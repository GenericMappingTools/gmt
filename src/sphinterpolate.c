/*--------------------------------------------------------------------
 *
 *	Copyright (c) 2008-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * Spherical gridding in tension.  We read input data and want to create
 * a grid using various interpolants on a sphere.  This program relies
 * on two Fortran F77 libraries by Renka:
 * Renka, R, J,, 1997, Algorithm 772: STRIPACK: Delaunay Triangulation
 *     and Voronoi Diagram on the Surface of a Sphere, AMC Trans. Math.
 *     Software, 23 (3), 416-434.
 * Renka, R, J,, 1997, Algorithm 773: SSRFPACK: Interpolation of scattered
 *     data on the Surface of a Sphere with a surface under tension, AMC
 *     Trans. Math. Software, 23 (3), 435-442.
 * We translated to C using f2c -r8 and and manually edited the code
 * so that f2c libs were not needed.  For any translation errors, blame me.
 *
 * Author:	Paul Wessel
 * Date:	1-AUG-2011
 * Version:	6 API
 */

#include "gmt_dev.h"
#include "gmt_sph.h"

#define THIS_MODULE_CLASSIC_NAME	"sphinterpolate"
#define THIS_MODULE_MODERN_NAME	"sphinterpolate"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Spherical gridding in tension of data on a sphere"
#define THIS_MODULE_KEYS	"<D{,GG}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVbdehiqrs" GMT_OPT("F")

struct SPHINTERPOLATE_CTRL {
	struct G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct Q {	/* -Q<interpolation> */
		bool active;
		unsigned int mode;
		double value[3];
	} Q;
	struct T {	/* -T for variable tension */
		bool active;
	} T;
	struct Z {	/* -Z to scale data */
		bool active;
	} Z;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPHINTERPOLATE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct SPHINTERPOLATE_CTRL);
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct SPHINTERPOLATE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int get_args (struct GMT_CTRL *GMT, char *arg, double par[], char *msg) {
	int m;
	char txt_a[32], txt_b[32], txt_c[32];
	m = sscanf (arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
	if (m < 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "%s\n", msg);
		m = -1;
	}
	par[0] = atof (txt_a);
	if (m >= 2) par[1] = atof (txt_b);
	if (m == 3) par[2] = atof (txt_c);
	return (m);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "==> The hard work is done by algorithms 772 (STRIPACK) & 773 (SSRFPACK) by R. J. Renka [1997] <==\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -G<outgrid> %s\n", name, GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Q<mode>][<args>] [%s] [-T] [%s] [-Z] [%s]\n\t[%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_Rgeo_OPT, GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_qi_OPT, GMT_r_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);
              
	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for the final gridded solution.\n");
	GMT_Option (API, "I");

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more data file (in ASCII, binary, netCDF) with (x,y,z[,w]).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Compute tension factors to achieve the following [Default is no tension]:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   p: Piecewise linear interpolation ; no tension [Default]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   l: Smooth interpolation with local gradient estimates.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g: Smooth interpolation with global gradient estimates and tension.  Optionally append <N>/<M>/<U>:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <N> = Number of iterations to converge solutions for gradients and variable tensions (-T only) [3],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <M> = Number of Gauss-Seidel iterations when determining gradients [10],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <U> = Maximum change in a gradient at the last iteration [0.01].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s: Smoothing.  Optionally append <E>/<U>/<N>, where\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <E> = Expected squared error in a typical (scaled) data value [0.01],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <U> = Upper bound on  weighted sum of squares of deviations from data [npoints],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <N> = Number of iterations to converge solutions for gradients and variable tensions (-T only) [3].\n");
	GMT_Option (API, "Rg");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   If no region is specified we default to the entire world [-Rg].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Use variable tension (ignored for -Qp) [constant].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Scale data by 1/(max-min) prior to gridding [no scaling].\n");
	GMT_Option (API, "bi3,di,e,h,i,qi,r,s,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SPHINTERPOLATE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to sphinterpolate and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k;
	int m;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'G':
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'Q':
				Ctrl->Q.active = true;
				switch (opt->arg[0]) {
					case 'p':	case '0':	/* Piecewise Linear p (0 is old mode)*/
						Ctrl->Q.mode = 0;	break;
					case 'l':	case '1':	/* Local gradients l (1 is old mode) */
						Ctrl->Q.mode = 1;	break;
					case 'g':	case '2':	/* Global gradients g (2 is old mode) */
						Ctrl->Q.mode = 2;
						if (opt->arg[1]) {	/* Gave optional n/m/dgmx */
							k = (opt->arg[1] == '/') ? 2 : 1;	/* Gave optional /n/m/dgmx */
							if ((m = get_args (GMT, &opt->arg[k], Ctrl->Q.value, "-Qg<N>[/<M>[/<U>]]")) < 0) n_errors++;
						}
						break;
					case 's':	case '3':	/* Smoothing s (3 is old mode) */
						Ctrl->Q.mode = 3;
						if (opt->arg[1]) {	/* Gave optional e/sm/niter */
							k = (opt->arg[1] == '/') ? 2 : 1;	/* Gave optional /e/sm/niter */
							if ((m = get_args (GMT, &opt->arg[k], Ctrl->Q.value, "-Qs<E>[/<U>[/<niter>]]")) < 0) n_errors++;
						}
						break;
					default:
						n_errors++;
						GMT_Report (API, GMT_MSG_NORMAL, "-%c Mode must be one of p,l,g,s\n", (int)opt->option);
						break;
				}
				break;
			case 'T':
				Ctrl->T.active = true;
				break;
			case 'Z':
				Ctrl->Z.active = true;
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 3;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 3, "Syntax error: Binary input data (-bi) must have at least 3 columns\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.mode > 3, "Syntax error -T: Must specify a mode in the 0-3 range\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_sphinterpolate (void *V_API, int mode, void *args) {
	unsigned int row, col;
	int error = 0;

	bool skip;

	size_t n_alloc = 0;
	uint64_t i, n = 0, ij, ij_f, n_read = 0, n_skip = 0, n_duplicates = 0;

	double w_min, w_max, sf = 1.0, X[3];
	double *xx = NULL, *yy = NULL, *zz = NULL, *ww = NULL, *surfd = NULL, *in = NULL;

	struct GMT_GRID *Grid = NULL;
	struct GMT_RECORD *In = NULL;
	struct SPHINTERPOLATE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	gmt_parse_common_options (GMT, "f", 'f', "g"); /* Implicitly set -fg since this is spherical triangulation */
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the sphinterpolate main code ----------------------------*/

	if (!GMT->common.R.active[RSET]) {	/* Default is global region */
		GMT->common.R.wesn[XLO] = 0.0;	GMT->common.R.wesn[XHI] = 360.0;	GMT->common.R.wesn[YLO] = -90.0;	GMT->common.R.wesn[YHI] = 90.0;
	}

	/* Now we are ready to take on some input values */

	if ((error = GMT_Set_Columns (API, GMT_IN, 3, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	GMT->session.min_meminc = GMT_INITIAL_MEM_ROW_ALLOC;	/* Start by allocating a 32 Mb chunk */

	gmt_M_malloc4 (GMT, xx, yy, zz, ww, GMT_CHUNK, &n_alloc, double);
	n = 0;
	w_min = DBL_MAX;	w_max = -DBL_MAX;

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) { 		/* Bail if there are any read errors */
				gmt_M_free (GMT, xx);	gmt_M_free (GMT, yy);
				gmt_M_free (GMT, zz);	gmt_M_free (GMT, ww);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}

		/* Data record to process */
		in = In->data;	/* Only need to process numerical part here */

		gmt_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], X, true);	/* Get unit vector */
		xx[n] = X[GMT_X];	yy[n] = X[GMT_Y];	zz[n] = X[GMT_Z];	ww[n] = in[GMT_Z];
		/* Check for duplicates */
		skip = false;
		for (i = 0; !skip && i < n; i++) {
			double c = xx[i] * xx[n] + yy[i] * yy[n] + zz[i] * zz[n];
			if (doubleAlmostEqual (c, 1.0)) {	/* Duplicates will give a dot product of 1 */
				if (doubleAlmostEqualZero (ww[n], ww[i])) {
					GMT_Report (API, GMT_MSG_VERBOSE,
					            "Data constraint %" PRIu64 " is identical to %" PRIu64 " and will be skipped\n", n_read, i);
					skip = true;
					n_skip++;
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL,
					            "Data constraint %" PRIu64 " and %" PRIu64 " occupy the same location but differ"
					            " in observation (%.12g vs %.12g)\n", n_read, i, ww[n], ww[i]);
					n_duplicates++;
				}
			}
		}
		n_read++;
		if (skip) continue;	/* Current point was a duplicate of a previous point */
	
		if (Ctrl->Z.active) {
			if (ww[n] < w_min) w_min = ww[n];
			if (ww[n] > w_max) w_max = ww[n];
		}
		if (++n == n_alloc) gmt_M_malloc4 (GMT, xx, yy, zz, ww, n, &n_alloc, double);
	} while (true);
	if (n_skip) GMT_Report (API, GMT_MSG_VERBOSE, "Skipped %" PRIu64 " data constraints as duplicates\n", n_skip);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR || n_duplicates) {	/* Disables further data input */
		gmt_M_free (GMT, xx);	gmt_M_free (GMT, yy);
		gmt_M_free (GMT, zz);	gmt_M_free (GMT, ww);
		if (n_duplicates) {
			GMT_Report (API, GMT_MSG_NORMAL,
			            "Found %" PRIu64 " data constraint duplicates with different observation values\n", n_duplicates);
			GMT_Report (API, GMT_MSG_NORMAL,
			            "You must reconcile duplicates before running sphinterpolate\n");
		}
		Return (API->error);
	}

	n_alloc = n;
	gmt_M_malloc4 (GMT, xx, yy, zz, ww, 0, &n_alloc, double);
	GMT->session.min_meminc = GMT_MIN_MEMINC;		/* Reset to the default value */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Do spherical interpolation using %" PRIu64 " points\n", n);

	if (Ctrl->Z.active && w_max > w_min) {	/* Scale the data */
		sf = 1.0 / (w_max - w_min);
		for (i = 0; i < n; i++) ww[n] *= sf;
	}

	/* Set up output grid */

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, NULL, NULL,
		                         GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) {
		gmt_M_free (GMT, xx);	gmt_M_free (GMT, yy);
		gmt_M_free (GMT, zz);	gmt_M_free (GMT, ww);
		Return (API->error);
	}
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Evaluate output grid\n");
	surfd = gmt_M_memory (GMT, NULL, Grid->header->nm, double);

	/* Do the interpolation */

	gmt_ssrfpack_grid (GMT, xx, yy, zz, ww, n, Ctrl->Q.mode, Ctrl->Q.value, Ctrl->T.active, Grid->header, surfd);

	gmt_M_free (GMT, xx);	gmt_M_free (GMT, yy);
	gmt_M_free (GMT, zz);	gmt_M_free (GMT, ww);

	/* Convert the doubles to gmt_grdfloat and unto the Fortran transpose order */

	sf = (w_max - w_min);
	if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Grid) == NULL) {
		gmt_M_free (GMT, surfd);
		Return (API->error);
	}
	gmt_M_grd_loop (GMT, Grid, row, col, ij) {
		ij_f = (uint64_t)col * (uint64_t)Grid->header->n_rows + (uint64_t)row;	/* Fortran index */
		Grid->data[ij] = (gmt_grdfloat)surfd[ij_f];	/* ij is GMT C index */
		if (Ctrl->Z.active) Grid->data[ij] *= (gmt_grdfloat)sf;
	}
	gmt_M_free (GMT, surfd);

	/* Write solution */

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Gridding completed\n");

	Return (GMT_NOERROR);
}
