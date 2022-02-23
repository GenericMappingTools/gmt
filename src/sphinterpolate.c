/*--------------------------------------------------------------------
 *
 *	Copyright (c) 2008-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
	struct SPHINTERPOLATE_D {	/* -D for duplicate checking */
		bool active;
		bool full;
		unsigned int mode;	/* 0 is full slow search, 1 is for specific east longitude */
		double east;	/* We will skip data with this exact longitude if -D<east> is given */
	} D;
	struct SPHINTERPOLATE_G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct SPHINTERPOLATE_I {	/* -I (for checking only) */
		bool active;
	} I;
	struct SPHINTERPOLATE_Q {	/* -Q<interpolation> */
		bool active;
		unsigned int mode;
		double value[3];
	} Q;
	struct SPHINTERPOLATE_T {	/* -T for variable tension */
		bool active;
	} T;
	struct SPHINTERPOLATE_Z {	/* -Z to scale data */
		bool active;
	} Z;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPHINTERPOLATE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct SPHINTERPOLATE_CTRL);
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct SPHINTERPOLATE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int sphinterpolate_get_args (struct GMT_CTRL *GMT, char *arg, double par[], char *msg) {
	int m;
	char txt_a[32], txt_b[32], txt_c[32];
	m = sscanf (arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
	if (m < 1) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "%s\n", msg);
		m = GMT_NOTSET;
	}
	par[0] = atof (txt_a);
	if (m >= 2) par[1] = atof (txt_b);
	if (m == 3) par[2] = atof (txt_c);
	return (m);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 1, "==> The hard work is done by algorithms 772 (STRIPACK) & 773 (SSRFPACK) by R. J. Renka [1997] <==\n");
	GMT_Usage (API, 0, "usage: %s [<table>] -G%s %s [-D[<east>]] [-Q<mode>[<args>]] [%s] [-T] [%s] "
		"[-Z] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_OUTGRID, GMT_I_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_h_OPT,
		GMT_i_OPT, GMT_qi_OPT, GMT_r_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	gmt_outgrid_syntax (API, 'G', "Sets name of the output grid file");
	GMT_Option (API, "I");

	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-D[<east>]");
	GMT_Usage (API, -2, "Delete exact duplicate points [Default deletes no duplicates except at poles]. "
		"Optionally, append <east> to exclude points with this particular longitude as repeating west "
		"[-D with no arg does a full comprehensive duplicate check - this may be very slow].");
	GMT_Usage (API, 1, "\n-Q<mode>[<args>]");
	GMT_Usage (API, -2, "Compute tension factors to achieve the following mode [Default is no tension]:");
	GMT_Usage (API, 3, "p: Piecewise linear interpolation; no tension [Default].");
	GMT_Usage (API, 3, "l: Smooth interpolation with local gradient estimates.");
	GMT_Usage (API, 3, "g: Smooth interpolation with global gradient estimates and tension.  Optionally append <N>/<M>/<U>, where");
	GMT_Usage (API, 4, "<N> = Number of iterations to converge solutions for gradients and variable tensions (-T only) [3].");
	GMT_Usage (API, 4, "<M> = Number of Gauss-Seidel iterations when determining gradients [10].");
	GMT_Usage (API, 4, "<U> = Maximum change in a gradient at the last iteration [0.01].");
	GMT_Usage (API, 3, "s: Smoothing.  Optionally append <E>/<U>/<N>, where");
	GMT_Usage (API, 4, "<E> = Expected squared error in a typical (scaled) data value [0.01].");
	GMT_Usage (API, 4, "<U> = Upper bound on  weighted sum of squares of deviations from data [npoints].");
	GMT_Usage (API, 4, "<N> = Number of iterations to converge solutions for gradients and variable tensions (-T only) [3].");
	GMT_Option (API, "Rg");
	if (gmt_M_showusage (API)) GMT_Usage (API, -2, "If no region is specified we default to the entire world [-Rg].");
	GMT_Usage (API, 1, "\n-T Use variable tension (ignored for -Qp) [constant].");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-Z Scale data by 1/(max-min) prior to gridding [no scaling].");
	GMT_Option (API, "bi3,di,e,h,i,qi,r,s,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct SPHINTERPOLATE_CTRL *Ctrl, struct GMT_OPTION *options) {
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
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;

			/* Processes program-specific parameters */

			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				if (opt->arg[0]) {	/* Limited to east-duplicates */
					Ctrl->D.mode = 1;
					n_errors += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf_arg (GMT, opt->arg, GMT_IS_LON, false, &Ctrl->D.east), opt->arg);
				}
				else	/* Do full search for duplicates */
					Ctrl->D.full = true;
				break;
			case 'G':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file))) n_errors++;
				break;
			case 'I':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'Q':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
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
							if ((m = sphinterpolate_get_args (GMT, &opt->arg[k], Ctrl->Q.value, "-Qg<N>[/<M>[/<U>]]")) < 0) n_errors++;
						}
						break;
					case 's':	case '3':	/* Smoothing s (3 is old mode) */
						Ctrl->Q.mode = 3;
						if (opt->arg[1]) {	/* Gave optional e/sm/niter */
							k = (opt->arg[1] == '/') ? 2 : 1;	/* Gave optional /e/sm/niter */
							if ((m = sphinterpolate_get_args (GMT, &opt->arg[k], Ctrl->Q.value, "-Qs<E>[/<U>[/<niter>]]")) < 0) n_errors++;
						}
						break;
					default:
						n_errors++;
						GMT_Report (API, GMT_MSG_ERROR, "-%c Mode must be one of p,l,g,s\n", (int)opt->option);
						break;
				}
				break;
			case 'T':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				Ctrl->T.active = true;
				break;
			case 'Z':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.active = true;
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 3;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 3, "Binary input data (-bi) must have at least 3 columns\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Option -I: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Option -G: Must specify output file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.mode > 3, "Option -T: Must specify a mode in the 0-3 range\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_sphinterpolate (void *V_API, int mode, void *args) {
	openmp_int row, col;
	int error = GMT_NOERROR;

	bool skip, got_N_pole = false, got_S_pole = false;

	size_t n_alloc = 0;
	uint64_t i, n = 0, ij, ij_f, n_read = 0, n_skip = 0, n_duplicates = 0, n_skip_S = 0, n_skip_N = 0, n_skip_E = 0;

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

		if (In->data == NULL) {
			gmt_quit_bad_record (API, In);
			Return (API->error);
		}

		/* Data record to process */
		in = In->data;	/* Only need to process numerical part here */

		/* Note: We always check for and exclude repeated N or S pole points.  Depending on -D we may exclude a east = west+360 point or do a full duplicate search */
		if (doubleAlmostEqual (in[GMT_Y], 90)) {	/* North pole point */
			if (got_N_pole) {
				n_skip_N++;
				n_read++;
				continue;
			}
			got_N_pole = true;
			in[GMT_X] = 0.5 * (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI]);
		}
		else if (doubleAlmostEqual (in[GMT_Y], -90)) {	/* South pole point */
			if (got_S_pole) {
				n_skip_S++;
				n_read++;
				continue;
			}
			got_S_pole = true;
			in[GMT_X] = 0.5 * (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI]);
		}
		else if (Ctrl->D.mode && doubleAlmostEqual (in[GMT_X], Ctrl->D.east)) {	/* Repeated east point */
			n_skip_E++;
			n_read++;
			continue;
		}
		/* Here we passed the basic duplicate point test */
		gmt_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], X, true);	/* Get unit vector */
		xx[n] = X[GMT_X];	yy[n] = X[GMT_Y];	zz[n] = X[GMT_Z];	ww[n] = in[GMT_Z];
		if (Ctrl->D.full) {	/* Run full check for duplicates */
			skip = false;
			for (i = 0; !skip && i < n; i++) {
				double c = xx[i] * xx[n] + yy[i] * yy[n] + zz[i] * zz[n];
				if (doubleAlmostEqual (c, 1.0)) {	/* Duplicates will give a dot product of 1 */
					if (doubleAlmostEqualZero (ww[n], ww[i])) {
						GMT_Report (API, GMT_MSG_WARNING,
						            "Data constraint %" PRIu64 " is identical to %" PRIu64 " and will be skipped\n", n_read, i);
						skip = true;
						n_skip++;
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR,
						            "Data constraint %" PRIu64 " and %" PRIu64 " occupy the same location but differ"
						            " in observation (%.12g vs %.12g)\n", n_read, i, ww[n], ww[i]);
						n_duplicates++;
					}
				}
			}
			n_read++;
			if (skip) continue;	/* Current point was a duplicate of a previous point */
		}
		else
			n_read++;

		if (Ctrl->Z.active) {
			if (ww[n] < w_min) w_min = ww[n];
			if (ww[n] > w_max) w_max = ww[n];
		}
		if (++n == n_alloc) gmt_M_malloc4 (GMT, xx, yy, zz, ww, n, &n_alloc, double);
	} while (true);
	if (n_skip_N) GMT_Report (API, GMT_MSG_WARNING, "Skipped %" PRIu64 " North pole duplicates\n", n_skip_N);
	if (n_skip_S) GMT_Report (API, GMT_MSG_WARNING, "Skipped %" PRIu64 " South pole duplicates\n", n_skip_S);
	if (n_skip_E) GMT_Report (API, GMT_MSG_WARNING, "Skipped %" PRIu64 " East longitude duplicates\n", n_skip_E);
	if (n_skip) GMT_Report (API, GMT_MSG_WARNING, "Skipped %" PRIu64 " data constraints as duplicates via -D check\n", n_skip);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR || n_duplicates) {	/* Disables further data input */
		if (n_duplicates) {
			GMT_Report (API, GMT_MSG_ERROR,
			            "Found %" PRIu64 " data constraint duplicates with different observation values\n", n_duplicates);
			GMT_Report (API, GMT_MSG_ERROR,
			            "You must reconcile duplicates before running sphinterpolate\n");
		}
		error = API->error;
		goto free_up;
	}
	if (Ctrl->D.full)	/* Report a null-result since we would already have exited above */
		GMT_Report (API, GMT_MSG_INFORMATION, "No duplicate points found in the input data\n");
	else
		GMT_Report (API, GMT_MSG_INFORMATION, "No duplicate check performed [-D was not activated]\n");

	n_alloc = n;
	gmt_M_malloc4 (GMT, xx, yy, zz, ww, 0, &n_alloc, double);
	GMT->session.min_meminc = GMT_MIN_MEMINC;		/* Reset to the default value */

	GMT_Report (API, GMT_MSG_INFORMATION, "Do spherical interpolation using %" PRIu64 " points\n", n);

	if (Ctrl->Z.active && w_max > w_min) {	/* Scale the data */
		sf = 1.0 / (w_max - w_min);
		for (i = 0; i < n; i++) ww[n] *= sf;
	}

	/* Set up output grid */

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, NULL, NULL,
		                         GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) {
		error = API->error;
		goto free_up;
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Evaluate output grid\n");
	surfd = gmt_M_memory (GMT, NULL, Grid->header->nm, double);

	/* Do the interpolation */

	if (gmt_ssrfpack_grid (GMT, xx, yy, zz, ww, n, Ctrl->Q.mode, Ctrl->Q.value, Ctrl->T.active, Grid->header, surfd)) {
		error = GMT_RUNTIME_ERROR;
		goto free_up;
	}

	/* Convert the doubles to gmt_grdfloat and unto the Fortran transpose order */

	sf = (w_max - w_min);
	if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Grid) == NULL) {
		error = API->error;
		goto free_up;
	}
	gmt_M_grd_loop (GMT, Grid, row, col, ij) {
		ij_f = (uint64_t)col * (uint64_t)Grid->header->n_rows + (uint64_t)row;	/* Fortran index */
		Grid->data[ij] = (gmt_grdfloat)surfd[ij_f];	/* ij is GMT C index */
		if (Ctrl->Z.active) Grid->data[ij] *= (gmt_grdfloat)sf;
	}

	/* Write solution */

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) {
		error = API->error;
		goto free_up;
	}
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
		error = API->error;
		goto free_up;
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Gridding completed\n");

free_up:	/* Free memory and return */

	gmt_M_free (GMT, xx);	gmt_M_free (GMT, yy);
	gmt_M_free (GMT, zz);	gmt_M_free (GMT, ww);
	gmt_M_free (GMT, surfd);

	Return (error);
}
