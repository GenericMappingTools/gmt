/*--------------------------------------------------------------------
 *	$Id: sphinterpolate_func.c,v 1.14 2011-06-07 21:38:30 guru Exp $
 *
 *	Copyright (c) 2008-2011 by P. Wessel
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Spherical gridding in tension.  We read input data and want to create
 * a grid using various interpolants on a sphere.  This program relies
 * on two Fortan F77 libraries by Renka:
 * Renka, R, J,, 1997, Algorithm 772: STRIPACK: Delaunay Triangulation
 *  and Voronoi Diagram on the Surface of a Sphere, AMC Trans. Math.
 *  Software, 23 (3), 416-434.
 * Renka, R, J,, 1997, Algorithm 773: SSRFPACK: Interpolation of scattered
 *  data on the Surface of a Sphere with a surface under tension, AMC
 *  Trans. Math. Software, 23 (3), 435-442.
 * We translate to C using f2c -r8 and link with -lf2c
 *
 * Author:	Paul Wessel
 * Date:	9-APR-2008
 *
 */
 
#include "gmt_sph.h"
#include "sph.h"

struct SPHINTERPOLATE_CTRL {
	struct G {	/* -G<grdfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct Q {	/* -Q<interpolation> */
		GMT_LONG active;
		int mode;
		double value[2];
	} Q;
	struct T {	/* -T for variable tension */
		GMT_LONG active;
	} T;
	struct Z {	/* -Z to scale data */
		GMT_LONG active;
	} Z;
};

void *New_sphinterpolate_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPHINTERPOLATE_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct SPHINTERPOLATE_CTRL);
	return ((void *)C);
}

void Free_sphinterpolate_Ctrl (struct GMT_CTRL *GMT, struct SPHINTERPOLATE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free ((void *)C->G.file);	
	GMT_free (GMT, C);	
}

GMT_LONG get_args (struct GMT_CTRL *GMT, char *arg, double par[], char *msg)
{
	GMT_LONG m;
	char txt_a[32], txt_b[32], txt_c[32];
	m = sscanf (arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
	if (m < 1) {
		GMT_report (GMT, GMT_MSG_FATAL, "GMT Error: %s\n", msg);
		m = -1;
	}
	par[0] = atof (txt_a);
	if (m >= 2) par[1] = atof (txt_b);
	if (m == 3) par[2] = atof (txt_c);
	return (m);
}

GMT_LONG GMT_sphinterpolate_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "sphinterpolate %s - Spherical gridding in tension of data on a sphere\n", GMT_VERSION);
	GMT_message (GMT, "==> The hard work is done by algorithms 772 (STRIPACK) & 773 (SSRFPACK) by R. J. Renka [1997] <==\n\n");
	GMT_message (GMT, "usage: sphinterpolate [<table>] -G<outgrid> %s\n", GMT_I_OPT);
	GMT_message (GMT, "\t[-Q<mode>][/<args>] [-T] [-V] [-Z] [%s]\n\t[%s] [%s] [%s] [%s]\n\n", GMT_b_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_colon_OPT);
               
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-G Specify file name for the final gridded solution.\n");
	GMT_inc_syntax (GMT, 'I', 0);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t<table> is one or more data file (in ASCII, binary, netCDF) with (x,y,z[,w]).\n");
	GMT_message (GMT, "\t   If no files are given, standard input is read.\n");
	GMT_message (GMT, "\t-Q Select tension factors to achive the following [Default is no tension]:\n");
	GMT_message (GMT, "\t   0: Piecewise linear interpolation ; no tension [Default]\n");
	GMT_message (GMT, "\t   1: Smooth interpolation with local gradient estimates.\n");
	GMT_message (GMT, "\t   2: Smooth interpolation with global gradient estimates and tension.  Optionally append /N/M/U:\n");
	GMT_message (GMT, "\t      N = Number of iterations to converge solutions for gradients and variable tensions (-T only) [3]\n");
	GMT_message (GMT, "\t      M = Number of Gauss-Seidel iterations when determining gradients [10]\n");
	GMT_message (GMT, "\t      U = Maximum change in a gradient at the last iteration [0.01]\n");
	GMT_message (GMT, "\t   3: Smoothing.  Optionally append /<E>/<U>/<N>, where\n");
	GMT_message (GMT, "\t      <E> = Expected squared error in a typical (scaled) data value [0.01]\n");
	GMT_message (GMT, "\t      <U> = Upper bound on  weighted sum of squares of deviations from data [npoints]\n");
	GMT_message (GMT, "\t      <N> = Number of iterations to converge solutions for gradients and variable tensions (-T only) [3]\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t   If no region is specified we default to the entire world [-Rg]\n");
	GMT_message (GMT, "\t-T Use variable tension (ignored for -Q0) [constant]\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-Z Scale data by 1/(max-min) prior to gridding [no scaling]\n");
	GMT_explain_options (GMT, "C3hiF:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_sphinterpolate_parse (struct GMTAPI_CTRL *C, struct SPHINTERPOLATE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to sphinterpolate and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, m;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'Q':
				Ctrl->Q.active = TRUE;
				switch (opt->arg[0]) {
					case '0':	/* Linear */
						Ctrl->Q.mode = 0;
						break;
					case '1':
						Ctrl->Q.mode = 1;
						break;
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
						GMT_report (GMT, GMT_MSG_FATAL, "Error: -%c Mode must be in 0-3 range\n", (int)opt->option);
						break;
				}
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				break;
			case 'Z':
				Ctrl->Z.active = TRUE;
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.active, &Ctrl->I.active);

	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 3;
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 3, "Syntax error: Binary input data (-bi) must have at least 3 columns\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.mode < 0 || Ctrl->Q.mode > 3, "Syntax error -T: Must specify a mode in the 0-3 range\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_sphinterpolate_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_sphinterpolate (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG row, col, i, ij, ij_f, n = 0, n_alloc = GMT_CHUNK, n_fields, error = FALSE;

	double w_min, w_max, sf = 1.0, X[3];
	double *xx = NULL, *yy = NULL, *zz = NULL, *ww = NULL, *surfd = NULL, *in = NULL;

	struct GMT_GRID *Grid = NULL;
	struct SPHINTERPOLATE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_sphinterpolate_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_sphinterpolate_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_sphinterpolate", &GMT_cpy);		/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRbr:", "hm" GMT_OPT("F"), options))) Return (error);
	Ctrl = (struct SPHINTERPOLATE_CTRL *) New_sphinterpolate_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_sphinterpolate_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the sphinterpolate main code ----------------------------*/

	Grid = GMT_create_grid (GMT);
	GMT_grd_init (GMT, Grid->header, options, FALSE);

	if (!GMT->common.R.active) {	/* Default is global region */
		Grid->header->wesn[XLO] = 0.0;	Grid->header->wesn[XHI] = 360.0;	Grid->header->wesn[YLO] = -90.0;	Grid->header->wesn[YHI] = 90.0;
	}

	/* Now we are ready to take on some input values */

	if ((error = GMT_set_cols (GMT, GMT_IN, 3))) Return (error);
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Registers default input sources, unless already set */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_REC))) Return (error);				/* Enables data input and sets access mode */

	n_alloc = GMT_malloc4 (GMT, xx, yy, zz, ww, 0, 0, double);
	n = 0;
	w_min = DBL_MAX;	w_max = -DBL_MAX;
	while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, (void **)&in)) != EOF) {	/* Keep returning records until we reach EOF */

		if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);
		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Skip all headers */

		GMT_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], X, TRUE);
		xx[n] = X[0];	yy[n] = X[1];	zz[n] = X[2];
		ww[n] = in[GMT_Z];
		if (Ctrl->Z.active) {
			if (ww[n] < w_min) w_min = ww[n];
			if (ww[n] > w_max) w_max = ww[n];
		}
		n++;

		if (n == n_alloc) n_alloc = GMT_malloc4 (GMT, xx, yy, zz, ww, n, n_alloc, double);
	}
	n_alloc = GMT_malloc4 (GMT, xx, yy, zz, ww, 0, n, double);
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */

	GMT_report (GMT, GMT_MSG_NORMAL, "Do Delaunay triangulation using %ld points\n", n);

	if (Ctrl->Z.active && w_max > w_min) {	/* Scale the data */
		sf = 1.0 / (w_max - w_min);
		for (i = 0; i < n; i++) ww[n] *= sf;
	}
	
	/* Set up output grid */
	
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, Grid, GMT->common.R.wesn, Ctrl->I.inc, GMT->common.r.active), Ctrl->G.file);
	GMT_report (GMT, GMT_MSG_NORMAL, "Evaluate output grid\n");
	surfd = GMT_memory (GMT, NULL, Grid->header->nm, double);
	
	/* Do the interpolation */
	
	ssrfpack_grid (GMT, xx, yy, zz, ww, n, Ctrl->Q.mode, Ctrl->Q.value, Ctrl->T.active, Grid->header, surfd);
	GMT_free (GMT, xx);
	GMT_free (GMT, yy);
	GMT_free (GMT, zz);
	GMT_free (GMT, ww);
	
	/* Convert the doubles to float and unto the Fortran transpose order */
	
	sf = (w_max - w_min);
	Grid->data = GMT_memory (GMT, NULL, Grid->header->size, float);
	for (row = 0; row < Grid->header->ny; row++) {
		for (col = 0; col < Grid->header->nx; col++) {
			ij_f = col * Grid->header->ny + row;
			ij = GMT_IJP (Grid->header, row, col);
			Grid->data[ij] = (float)surfd[ij_f];
			if (Ctrl->Z.active) Grid->data[ij] *= (float)sf;
		}
	}
	GMT_free (GMT, surfd);
	
	/* Write solution */
	
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&Ctrl->G.file, (void *)Grid)) Return (GMT_DATA_WRITE_ERROR);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */

	GMT_report (GMT, GMT_MSG_NORMAL, "Gridding completed\n");

	Return (GMT_OK);
}
