/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 2008-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Spherical gridding in tension.  We read input data and want to create
 * a grid using various interpolants on a sphere.  This program relies
 * on two Fortan F77 libraries by Renka:
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
 * Version:	5 API
 */
 
#define THIS_MODULE_NAME	"sphinterpolate"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Spherical gridding in tension of data on a sphere"
#define THIS_MODULE_KEYS	"<DI,GGO,RGi"

#include "gmt_dev.h"
#include "gmt_sph.h"

#define GMT_PROG_OPTIONS "-:RVbdhirs" GMT_OPT("F")

struct SPHINTERPOLATE_CTRL {
	struct G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
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

void *New_sphinterpolate_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPHINTERPOLATE_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct SPHINTERPOLATE_CTRL);
	return (C);
}

void Free_sphinterpolate_Ctrl (struct GMT_CTRL *GMT, struct SPHINTERPOLATE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free (C->G.file);	
	GMT_free (GMT, C);	
}

int get_args (struct GMT_CTRL *GMT, char *arg, double par[], char *msg)
{
	int m;
	char txt_a[32], txt_b[32], txt_c[32];
	m = sscanf (arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
	if (m < 1) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "GMT Error: %s\n", msg);
		m = -1;
	}
	par[0] = atof (txt_a);
	if (m >= 2) par[1] = atof (txt_b);
	if (m == 3) par[2] = atof (txt_c);
	return (m);
}

int GMT_sphinterpolate_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "==> The hard work is done by algorithms 772 (STRIPACK) & 773 (SSRFPACK) by R. J. Renka [1997] <==\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: sphinterpolate [<table>] -G<outgrid> %s\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-Q<mode>][/<args>] [-T] [%s] [-Z] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_s_OPT, GMT_colon_OPT);
               
	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for the final gridded solution.\n");
	GMT_Option (API, "I");

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more data file (in ASCII, binary, netCDF) with (x,y,z[,w]).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Select tension factors to achive the following [Default is no tension]:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   0: Piecewise linear interpolation ; no tension [Default]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   1: Smooth interpolation with local gradient estimates.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   2: Smooth interpolation with global gradient estimates and tension.  Optionally append /N/M/U:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      N = Number of iterations to converge solutions for gradients and variable tensions (-T only) [3],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      M = Number of Gauss-Seidel iterations when determining gradients [10],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      U = Maximum change in a gradient at the last iteration [0.01].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   3: Smoothing.  Optionally append /<E>/<U>/<N>, where\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <E> = Expected squared error in a typical (scaled) data value [0.01],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <U> = Upper bound on  weighted sum of squares of deviations from data [npoints],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <N> = Number of iterations to converge solutions for gradients and variable tensions (-T only) [3].\n");
	GMT_Option (API, "Rg");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no region is specified we default to the entire world [-Rg].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Use variable tension (ignored for -Q0) [constant].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Scale data by 1/(max-min) prior to gridding [no scaling].\n");
	GMT_Option (API, "bi3,di,h,i,r,s,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_sphinterpolate_parse (struct GMT_CTRL *GMT, struct SPHINTERPOLATE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to sphinterpolate and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	int m;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'G':
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'Q':
				Ctrl->Q.active = true;
				switch (opt->arg[0]) {
					case '0':	/* Linear */
						Ctrl->Q.mode = 0;	break;
					case '1':
						Ctrl->Q.mode = 1;	break;
					case '2':
						Ctrl->Q.mode = 2;
						if (opt->arg[1] == '/') {	/* Gave optional n/m/dgmx */
							if ((m = get_args (GMT, &opt->arg[2], Ctrl->Q.value, "-Q3/N[/M[/U]]")) < 0) n_errors++;
						}
						break;
					case '3':
						Ctrl->Q.mode = 3;
						if (opt->arg[1] == '/') {	/* Gave optional e/sm/niter */
							if ((m = get_args (GMT, &opt->arg[2], Ctrl->Q.value, "-Q3/E[/U[/niter]]")) < 0) n_errors++;
						}
						break;
					default:
						n_errors++;
						GMT_Report (API, GMT_MSG_NORMAL, "Error: -%c Mode must be in 0-3 range\n", (int)opt->option);
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
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.registration, &Ctrl->I.active);

	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 3;
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 3, "Syntax error: Binary input data (-bi) must have at least 3 columns\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.mode > 3, "Syntax error -T: Must specify a mode in the 0-3 range\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_sphinterpolate_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_sphinterpolate (void *V_API, int mode, void *args)
{
	unsigned int row, col;
	int error = 0;

	size_t n_alloc = 0;
	uint64_t i, n = 0, ij, ij_f;
	
	double w_min, w_max, sf = 1.0, X[3];
	double *xx = NULL, *yy = NULL, *zz = NULL, *ww = NULL, *surfd = NULL, *in = NULL;

	struct GMT_GRID *Grid = NULL;
	struct SPHINTERPOLATE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_sphinterpolate_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_sphinterpolate_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_sphinterpolate_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	GMT_parse_common_options (GMT, "f", 'f', "g"); /* Implicitly set -fg since this is spherical triangulation */
	Ctrl = New_sphinterpolate_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	if ((error = GMT_sphinterpolate_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the sphinterpolate main code ----------------------------*/

	if (!GMT->common.R.active) {	/* Default is global region */
		GMT->common.R.wesn[XLO] = 0.0;	GMT->common.R.wesn[XHI] = 360.0;	GMT->common.R.wesn[YLO] = -90.0;	GMT->common.R.wesn[YHI] = 90.0;
	}

	/* Now we are ready to take on some input values */

	if ((error = GMT_set_cols (GMT, GMT_IN, 3)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	GMT->session.min_meminc = GMT_INITIAL_MEM_ROW_ALLOC;	/* Start by allocating a 32 Mb chunk */ 

	GMT_malloc4 (GMT, xx, yy, zz, ww, GMT_CHUNK, &n_alloc, double);
	n = 0;
	w_min = DBL_MAX;	w_max = -DBL_MAX;
	
	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */

		GMT_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], X, true);	/* Get unit vector */
		xx[n] = X[GMT_X];	yy[n] = X[GMT_Y];	zz[n] = X[GMT_Z];	ww[n] = in[GMT_Z];
		if (Ctrl->Z.active) {
			if (ww[n] < w_min) w_min = ww[n];
			if (ww[n] > w_max) w_max = ww[n];
		}
		if (++n == n_alloc) GMT_malloc4 (GMT, xx, yy, zz, ww, n, &n_alloc, double);
	} while (true);
	
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	n_alloc = n;
	GMT_malloc4 (GMT, xx, yy, zz, ww, 0, &n_alloc, double);
	GMT->session.min_meminc = GMT_MIN_MEMINC;		/* Reset to the default value */

	GMT_Report (API, GMT_MSG_VERBOSE, "Do spherical interpolation using %" PRIu64 " points\n", n);

	if (Ctrl->Z.active && w_max > w_min) {	/* Scale the data */
		sf = 1.0 / (w_max - w_min);
		for (i = 0; i < n; i++) ww[n] *= sf;
	}
	
	/* Set up output grid */
	
	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, NULL, Ctrl->I.inc, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);

	GMT_Report (API, GMT_MSG_VERBOSE, "Evaluate output grid\n");
	surfd = GMT_memory (GMT, NULL, Grid->header->nm, double);
	
	/* Do the interpolation */
	
	ssrfpack_grid (GMT, xx, yy, zz, ww, n, Ctrl->Q.mode, Ctrl->Q.value, Ctrl->T.active, Grid->header, surfd);

	GMT_free (GMT, xx);	GMT_free (GMT, yy);
	GMT_free (GMT, zz);	GMT_free (GMT, ww);
	
	/* Convert the doubles to float and unto the Fortran transpose order */
	
	sf = (w_max - w_min);
	if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, NULL, NULL, 0, 0, Grid) == NULL) Return (API->error);
	GMT_grd_loop (GMT, Grid, row, col, ij) {
		ij_f = (uint64_t)col * (uint64_t)Grid->header->ny + (uint64_t)row;	/* Fortran index */
		Grid->data[ij] = (float)surfd[ij_f];	/* ij is GMT C index */
		if (Ctrl->Z.active) Grid->data[ij] *= (float)sf;
	}
	GMT_free (GMT, surfd);
	
	/* Write solution */
	
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Grid) != GMT_OK) {
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "Gridding completed\n");

	Return (GMT_OK);
}
