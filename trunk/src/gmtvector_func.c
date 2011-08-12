/*--------------------------------------------------------------------
*    $Id$
*
*	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
*	See LICENSE.TXT file for copying and redistribution conditions.
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
 * Brief synopsis: gmtvector performs basic vector operations such as dot and
 * cross products, sums, and conversion between Cartesian and polar/spherical
 * coordinates systems.
 *
 * Author:	Paul Wessel
 * Date:	10-Aug-2010
 * Version:	5 API
 */

#include "gmt.h"

#define DO_NOTHING	0
#define DO_AVERAGE	1
#define DO_DOT2D	2
#define DO_DOT3D	3
#define DO_CROSS	4
#define DO_SUM		5
#define DO_ROT2D	6
#define DO_ROT3D	7
#define DO_DOT		8
#define DO_BISECTOR	9

struct GMTVECTOR_CTRL {
	struct Out {	/* -> */
		GMT_LONG active;
		char *file;
	} Out;
	struct In {	/* infile */
		GMT_LONG active;
		GMT_LONG n_args;
		char *arg;
	} In;
	struct A {	/* -A[m[<conf>]|<vec>] */
		GMT_LONG active;
		GMT_LONG mode;
		double conf;
		char *arg;
	} A;
	struct C {	/* -C[i|o] */
		GMT_LONG active[2];
	} C;
	struct D {	/* -D[dim] */
		GMT_LONG active;
		GMT_LONG mode;
	} D;
	struct E {	/* -E */
		GMT_LONG active;
	} E;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct S {	/* -S[vec] */
		GMT_LONG active;
		char *arg;
	} S;
	struct T {	/* -T[operator] */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG degree;
		double par[3];
	} T;
};

void *New_gmtvector_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTVECTOR_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GMTVECTOR_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->A.conf = 0.95;	/* 95% conf level */
	return ((void *)C);
}

void Free_gmtvector_Ctrl (struct GMT_CTRL *GMT, struct GMTVECTOR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.arg) free ((void *)C->In.arg);	
	if (C->Out.file) free ((void *)C->Out.file);	
	if (C->S.arg) free ((void *)C->S.arg);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_gmtvector_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "gmtvector %s [API] - Basic manipulation of Cartesian vectors\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: gmtvector [<table>] [-Am[<conf>]|<vector>] [-C[i|o]] [-E] [-N] [-S<vector>] [-Ta|b|d|D|s|r<rot>|x]\n");
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n",
		GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t<table> (in ASCII or binary) have 2 or more columns with (x,y[,z]), (r,theta) or (lon,lat) in first 2-3 columns.\n");
	GMT_message (GMT, "\t   If one item is given and it cannot be opened we will interpret it as x/y[/z], r/theta, or lon/lat.\n");
	GMT_message (GMT, "\t   If no file(s) is given, standard input is read.\n");
	GMT_message (GMT, "\t-A Single primary vector, given as lon/lat, r/theta, or x/y[/z].  No infiles will be read.\n");
	GMT_message (GMT, "\t   Alternatively, give -Am to compute a single primary vector as the mean of the input vectors.\n");
	GMT_message (GMT, "\t   The confidence ellipse for the mean vector is determined (95%% level);\n");
	GMT_message (GMT, "\t   optionally append a different confidence level in percent.\n");
	GMT_message (GMT, "\t-C Indicate Cartesian coordinates on input/output instead of lon,lat or r/theta.\n");
	GMT_message (GMT, "\t   Append i or o to only affect input or output coordinates.\n");
	GMT_message (GMT, "\t-E Automatically convert between geodetic and geocentric coordinates [no conversion].\n");
	GMT_message (GMT, "\t   Ignored unless -fg is given.\n");
	GMT_message (GMT, "\t-N Normalize the transformed vectors (only affects -Co output).\n");
	GMT_message (GMT, "\t-S The secondary vector (if needed by -T), given as lon/lat, r/theta, or x/y[/z].\n");
	GMT_message (GMT, "\t-T Specify the desired transformation of the input data.  Choose one of\n");
	GMT_message (GMT, "\t   -Ta gives the average of the input and secondary vector (see -S).\n");
	GMT_message (GMT, "\t   -Tb gives the bisector great circle pole(s) for input and secondary vector (see -S).\n");
	GMT_message (GMT, "\t   -Td gives dot-product(s) with secondary vector (see -S).\n");
	GMT_message (GMT, "\t   -TD same as -Td, but gives the angle in degrees between the vectors.\n");
	GMT_message (GMT, "\t   -Ts gives the sum of the secondary vector (see -S) and the input vector(s).\n");
	GMT_message (GMT, "\t   -Tr will rotate the input vectors. Depending on your input (2-D or 3-D), append\n");
	GMT_message (GMT, "\t      <angle> or <plon/plat/angle>, respectively, to define the rotation.\n");
	GMT_message (GMT, "\t   -Tx will compute cross-product(s) with secondary vector (see -S).\n");
	GMT_explain_options (GMT, "VfgC0");
	GMT_message (GMT, "\t   Default is 2 [or 3; see -C, -D] input columns.\n");
	GMT_explain_options (GMT, "D0hio:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_gmtvector_parse (struct GMTAPI_CTRL *C, struct GMTVECTOR_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdsample and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG  n, n_in, n_errors = 0, n_files = 0;
	char txt_a[GMT_TEXT_LEN64], txt_b[GMT_TEXT_LEN64], txt_c[GMT_TEXT_LEN64];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files or single point */
				Ctrl->In.active = TRUE;
				if (Ctrl->In.n_args++ == 0) Ctrl->In.arg = strdup (opt->arg);
				break;
			case '>':	/* Got named output file */
				if (n_files++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Secondary vector */
				Ctrl->A.active = TRUE;
				if (opt->arg[0] == 'm') {
					Ctrl->A.mode = 1;
					if (opt->arg[1]) Ctrl->A.conf = 0.01 * atof (&opt->arg[1]);
				}
				else
					Ctrl->A.arg = strdup (opt->arg);
				break;
			case 'C':	/* Cartesian coordinates on in|out */
				if (opt->arg[0] == 'i')
					Ctrl->C.active[GMT_IN] = TRUE;
				else if (opt->arg[0] == 'o')
					Ctrl->C.active[GMT_OUT] = TRUE;
				else if (opt->arg[0] == '\0')
					Ctrl->C.active[GMT_IN] = Ctrl->C.active[GMT_OUT] = TRUE;
				else {
					GMT_message (GMT, "Bad modifier given to -C (%s)\n", opt->arg);
					n_errors++;
				}
				break;
			case 'E':	/* geodetic/geocentric conversion */
			 	Ctrl->E.active = TRUE;
				break;
			case 'N':	/* Normalize vectors */
			 	Ctrl->N.active = TRUE;
				break;
			case 'T':	/* Selects transformation */
				Ctrl->T.active = TRUE;
				switch (opt->arg[0]) {
					case 'a':	/* Angle between vectors */
						Ctrl->T.mode = DO_AVERAGE;
						break;
					case 'b':	/* Pole of bisector great circle */
						Ctrl->T.mode = DO_BISECTOR;
						break;
					case 'D':	/* Angle between vectors */
						Ctrl->T.degree = TRUE;
					case 'd':	/* dot-product of two vectors */
						Ctrl->T.mode = DO_DOT;
						break;
					case 'x':	/* Cross-product between vectors */
						Ctrl->T.mode = DO_CROSS;
						break;
					case 's':	/* Sum of vectors */
						Ctrl->T.mode = DO_SUM;
						break;
					case 'r':	/* Rotate vectors */
						n = sscanf (&opt->arg[1], "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
						if (n == 1) {	/* 2-D Cartesian rotation */
							Ctrl->T.mode = DO_ROT2D;
							Ctrl->T.par[2] = atof (txt_a);
						}
						else if (n == 3) {	/* 3-D spherical rotation */
							Ctrl->T.mode = DO_ROT3D;
							n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], GMT_scanf_arg (GMT, txt_a, GMT->current.io.col_type[GMT_IN][GMT_X], &Ctrl->T.par[0]), txt_a);
							n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], GMT_scanf_arg (GMT, txt_b, GMT->current.io.col_type[GMT_IN][GMT_Y], &Ctrl->T.par[1]), txt_b);
							Ctrl->T.par[2] = atof (txt_c);
						}
						else {
							GMT_report (GMT, GMT_MSG_FATAL, "Bad arguments given to -Tr (%s)\n", opt->arg);
							n_errors++;
						}
						break;
				}
				break;
			case 'S':	/* Secondary vector */
				Ctrl->S.active = TRUE;
				Ctrl->S.arg = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_in = (Ctrl->C.active[GMT_IN] && GMT_is_geographic (GMT, GMT_IN)) ? 3 : 2;
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = n_in;
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < n_in, "Syntax error: Binary input data (-bi) must have at least %ld columns\n", n_in);
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && Ctrl->S.arg && !GMT_access (GMT, Ctrl->S.arg, R_OK), "Syntax error -S: Secondary vector cannot be a file!\n");
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");
	n_errors += GMT_check_condition (GMT, Ctrl->In.n_args && Ctrl->A.active && Ctrl->A.mode == 0, "Syntax error: Cannot give input files and -A<vec> at the same time\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

GMT_LONG decode_vector (struct GMT_CTRL *C, char *arg, double coord[], GMT_LONG cartesian, GMT_LONG geocentric) {
	GMT_LONG n, n_out, n_errors = 0, ix, iy;
	char txt_a[GMT_TEXT_LEN64], txt_b[GMT_TEXT_LEN64], txt_c[GMT_TEXT_LEN64];
	
	ix = (C->current.setting.io_lonlat_toggle[GMT_IN]);	iy = 1 - ix;
	n = n_out = sscanf (arg, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
	if (n == 2) {	/* Got lon/lat, r/theta, or x/y */
		if (GMT_is_geographic (C, GMT_IN)) {
			n_errors += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][ix], GMT_scanf_arg (C, txt_a, C->current.io.col_type[GMT_IN][ix], &coord[ix]), txt_a);
			n_errors += GMT_verify_expectations (C, C->current.io.col_type[GMT_IN][iy], GMT_scanf_arg (C, txt_b, C->current.io.col_type[GMT_IN][iy], &coord[iy]), txt_b);
			if (geocentric) coord[GMT_Y] = GMT_lat_swap (C, coord[GMT_Y], GMT_LATSWAP_G2O);
			GMT_geo_to_cart (C, coord[GMT_Y], coord[GMT_X], coord, TRUE);	/* get x/y/z */
			n_out = 3;
		}
		else if (cartesian) {	/* Cartsesian x/y */
			coord[GMT_X] = atof (txt_a);
			coord[GMT_Y] = atof (txt_b);
		}
		else	/* Cylindrical r/theta */
			GMT_polar_to_cart (C, atof (txt_a), atof (txt_b), coord, TRUE);
	}
	else if (n == 3) {	/* Got x/y/z */
		coord[GMT_X] = atof (txt_a);
		coord[GMT_Y] = atof (txt_b);
		coord[GMT_Z] = atof (txt_c);
	}
	else {
		GMT_report (C, GMT_MSG_FATAL, "Bad vector argument (%s)\n", arg);
		return (0);
	}
	if (n_errors) {
		GMT_report (C, GMT_MSG_FATAL, "Failed to decode the geographic coordinates (%s)\n", arg);
		return (0);	
	}
	return (n_out);
}

void make_rot_matrix (struct GMT_CTRL *C, double lonp, double latp, double w, double R[3][3])
{
	/* lonp, latp	Euler pole in degrees
 	 * w		angular rotation in degrees
 	 * R		the rotation matrix
	 * Based on Cox and Hart, 1986.  Plate Tectonics - How it Works.
	 * All coordinates are assumed to be geocentric.
 	 */

	double E[3], sin_w, cos_w, c, E_x, E_y, E_z, E_12c, E_13c, E_23c;

	GMT_geo_to_cart (C, latp, lonp, E, TRUE);

	sincosd (w, &sin_w, &cos_w);
	c = 1.0 - cos_w;

	E_x = E[0] * sin_w;
	E_y = E[1] * sin_w;
	E_z = E[2] * sin_w;
	E_12c = E[0] * E[1] * c;
	E_13c = E[0] * E[2] * c;
	E_23c = E[1] * E[2] * c;

	R[0][0] = E[0] * E[0] * c + cos_w;
	R[0][1] = E_12c - E_z;
	R[0][2] = E_13c + E_y;

	R[1][0] = E_12c + E_z;
	R[1][1] = E[1] * E[1] * c + cos_w;
	R[1][2] = E_23c - E_x;

	R[2][0] = E_13c - E_y;
	R[2][1] = E_23c + E_x;
	R[2][2] = E[2] * E[2] * c + cos_w;
}

void matrix_vect_mult (struct GMT_CTRL *C, GMT_LONG dim, double a[3][3], double b[3], double c[3])
{	/* c = A * b */
	GMT_LONG i, j;

	for (i = 0; i < dim; i++) for (j = 0, c[i] = 0.0; j < dim; j++) c[i] += a[i][j] * b[j];
}

void get_bisector (struct GMT_CTRL *C, double A[3], double B[3], double P[3])
{	/* Given points in A and B, return the bisector pole via P */
	 
	GMT_LONG i;
	double Pa[3], M[3];
	 
	/* Get mid point between A and B */
	
	for (i = 0; i < 3; i++) M[i] = A[i] + B[i];
	GMT_normalize3v (C, M);

	/* Get pole for great circle through A and B */
	
	GMT_cross3v (C, A, B, Pa);
	GMT_normalize3v (C, Pa);

	/* Then get pole for bisector of A-B through Pa */
	
	GMT_cross3v (C, M, Pa, P);
	GMT_normalize3v (C, P);
}

void mean_vector (struct GMT_CTRL *GMT, struct GMT_DATASET *D, GMT_LONG cartesian, double conf, double *M, double *E)
{
	/* Determines the mean vector M and the covariance matrix C */
	
	GMT_LONG i, j, k, p, tbl, seg, row, nv, n, nrots;
	double lambda[3], V[9], work1[3], work2[3], lon, lat, lon2, lat2, scl, L, Y;
	double *P[3], X[3], B[3], C[9];
	struct GMT_LINE_SEGMENT *S = NULL;
	
	GMT_memset (M, 3, double);
	nv = (GMT_is_geographic (GMT, GMT_IN) || D->n_columns == 3) ? 3 : 2;
	for (k = 0; k < nv; k++) P[k] = GMT_memory (GMT, NULL, D->n_records, double);
	for (tbl = n = 0; tbl < D->n_tables; tbl++) {
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
			S = D->table[tbl]->segment[seg];
			for (row = 0; row < S->n_rows; row++) {
				if (!cartesian) {	/* Want to turn geographic or polar into Cartesian */
					if (GMT_is_geographic (GMT, GMT_IN))
						GMT_geo_to_cart (GMT, S->coord[GMT_Y][row], S->coord[GMT_X][row], X, TRUE);	/* get x/y/z */
					else
						GMT_polar_to_cart (GMT, S->coord[GMT_X][row], S->coord[GMT_Y][row], X, TRUE);
					for (k = 0; k < nv; k++) P[k][n] = X[k];
				}
				else for (k = 0; k < nv; k++) P[k][n] = S->coord[k][row];
				for (k = 0; k < nv; k++) M[k] += P[k][n];
				n++;
			}
		}
	}
	for (k = 0; k < nv; k++) M[k] /= n;	/* Compute mean resultant vector [not normalized] */
	GMT_memcpy (X, M, 3, double);
	
	/* Get covariance matrix */
	
	GMT_memset (C, 9, double);
	for (j = 0; j < nv; j++) for (i = 0; i < nv; i++) {
		k = nv*j + i;
		C[k] = 0.0;
		for (p = 0; p < n; p++) C[k] += (P[j][p] - M[j]) * (P[i][p] - M[i]);
		C[k] /= (n - 1.0);
	}
	for (k = 0; k < nv; k++) GMT_free (GMT, P[k]);

	if (GMT_jacobi (GMT, C, &nv, &nv, lambda, V, work1, work2, &nrots)) {	/* Solve eigen-system */
		GMT_message (GMT, "Warning: Eigenvalue routine failed to converge in 50 sweeps.\n");
	}
	if (nv == 3) {
		GMT_cart_to_geo (GMT, &lat, &lon, X, TRUE);
		if (lon < 0.0) lon += 360.0;
	}
	else { lon = X[GMT_X]; lat = X[GMT_Y]; }

	/* Find the xy[z] point of end of eigenvector 1: */
	GMT_memset (B, 3, double);
	for (k = 0; k < nv; k++) B[k] = X[k] + sqrt (lambda[0]) * V[k];
	L = sqrt (B[0] * B[0] + B[1] * B[1] + B[2] * B[2]);
	for (k = 0; k < nv; k++) B[k] /= L;
	if (nv == 3) {
		GMT_cart_to_geo (GMT, &lat2, &lon2, B, TRUE);
		if (lon2 < 0.0) lon2 += 360.0;
		L = lon2 - lon;
		if (fabs (L) > 180.0) L = copysign (360.0 - fabs (L), L);
	}
	else {
		lon2 = B[GMT_X];
		lat2 = B[GMT_Y];
	}
	Y = lat2 - lat;

	E[0] = 90.0 - atan2 (Y, L * cos (lat * D2R)) * R2D;
	/* Convert to 95% confidence in km */

	scl = (nv == 3) ? GMT->current.proj.DIST_KM_PR_DEG * R2D * sqrt (GMT_chi2crit (GMT, conf, 3)) : sqrt (GMT_chi2crit (GMT, conf, 2));
	E[1] = 2.0 * sqrt (lambda[0]) * scl;	/* 2* since we need the major axis not semi-major */
	E[2] = 2.0 * sqrt (lambda[1]) * scl;

	GMT_report (GMT, GMT_MSG_NORMAL, "%g%% confidence ellipse on mean position: Major axis = %g Minor axis = %g Major axis azimuth = %g\n", 100.0 * conf, E[1], E[2], E[0]);
}

#define Return(code) {Free_gmtvector_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_gmtvector (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG tbl, seg, row, error = 0, k, n, nv, n_out, add = 0, single = FALSE;

	double out[3], vector_1[3], vector_2[3], vector_3[3], R[3][3], E[3];

	struct GMT_DATASET *Din = NULL, *Dout = NULL;
	struct GMT_LINE_SEGMENT *Sin = NULL,  *Sout = NULL;
	struct GMTVECTOR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_gmtvector_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_gmtvector_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_gmtvector", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-Vbf:", "ghios>" GMT_OPT("HMm"), options))) Return (error);
	Ctrl = (struct GMTVECTOR_CTRL *) New_gmtvector_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtvector_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the gmtvector main code ----------------------------*/
			
	if (Ctrl->T.mode == DO_ROT3D)
		make_rot_matrix (GMT, Ctrl->T.par[0], Ctrl->T.par[1], Ctrl->T.par[2], R);
	else if (Ctrl->T.mode == DO_ROT2D) {
		double s, c;
		GMT_memset (R, 9, double);
		sincosd (Ctrl->T.par[2], &s, &c);
		R[0][0] = c;	R[0][1] = -s;
		R[1][0] = s;	R[1][1] = c;
	}
	else if (!Ctrl->T.mode == DO_NOTHING) {	/* Will need secondary vector, get that first before input file */
		n = decode_vector (GMT, Ctrl->S.arg, vector_2, Ctrl->C.active[GMT_IN], Ctrl->E.active);
		if (n == 0) Return (EXIT_FAILURE);
		if (Ctrl->T.mode == DO_DOT) {	/* Must normalize to turn dot-product into angle */
			if (n == 2)
				GMT_normalize2v (GMT, vector_2);
			else
				GMT_normalize3v (GMT, vector_2);
			Ctrl->C.active[GMT_OUT] = TRUE;	/* Since we just want to return the angle */
		}
	}
	
	/* Read input data set */
	
	GMT_memset (vector_1, 3, double);
	GMT_memset (vector_3, 3, double);
	if (Ctrl->A.active) {	/* Want a single primary vector */
		if (Ctrl->A.mode) {	/* Compute the mean of all input vectors */
			if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Registers default input sources, unless already set */
			if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
			if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, (void **)&Din)) Return ((error = GMT_DATA_READ_ERROR));
			if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
			n = n_out = (Ctrl->C.active[GMT_OUT] && (Din->n_columns == 3 || GMT_is_geographic (GMT, GMT_IN))) ? 3 : 2;
			mean_vector (GMT, Din, Ctrl->C.active[GMT_IN], Ctrl->A.conf, vector_1, E);	/* Get mean vector and confidence ellispe parameters */
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Din);
			add = 3;	/* Make space for angle major minor */
		}
		else {	/* Decode a single vector */
			n = decode_vector (GMT, Ctrl->A.arg, vector_1, Ctrl->C.active[GMT_IN], Ctrl->E.active);
			if (n == 0) Return (EXIT_FAILURE);
			if (Ctrl->T.mode == DO_DOT) {	/* Must normalize before we turn dot-product into angle */
				if (n == 2)
					GMT_normalize2v (GMT, vector_1);
				else
					GMT_normalize3v (GMT, vector_1);
			}
			n_out = (Ctrl->C.active[GMT_OUT] && n == 3) ? 3 : 2;
		}
		Din = GMT_create_dataset (GMT, 1, 1, 3, 1);
		nv = (n == 3 || GMT_is_geographic (GMT, GMT_IN)) ? 3 : 2;	/* Number of Cartesian vector components */
		for (k = 0; k < nv; k++) Din->table[0]->segment[0]->coord[k][0] = vector_1[k];
		single = TRUE;
	}
	else {	/* Read input files or stdin */
		if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Registers default input sources, unless already set */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, (void **)&Din)) Return ((error = GMT_DATA_READ_ERROR));
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
		n = n_out = (Ctrl->C.active[GMT_OUT] && (Din->n_columns == 3 || GMT_is_geographic (GMT, GMT_IN))) ? 3 : 2;
	}
	
	if (Ctrl->T.mode == DO_DOT) {
		n_out = 1;	/* Override prior setting since we just will report an angle in one column */
		GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_FLOAT;
	}
	else if (Ctrl->C.active[GMT_OUT] || !GMT_is_geographic (GMT, GMT_OUT))	/* Override types since output is Cartesian or polar coordinates, not lon/lat */
		GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_FLOAT;

	GMT_alloc_dataset (GMT, Din, &Dout, n_out + add, 0, GMT_ALLOC_NORMAL);
	GMT_memset (out, 3, double);

	/* OK, with data in hand we can do some damage */
	
	if (Ctrl->T.mode == DO_DOT) Ctrl->T.mode = (n == 3 || GMT_is_geographic (GMT, GMT_IN)) ? DO_DOT3D : DO_DOT2D;
	nv = (n == 3 || GMT_is_geographic (GMT, GMT_IN)) ? 3 : 2;	/* Number of Cartesian vector components */
	
	for (tbl = 0; tbl < Din->n_tables; tbl++) {
		for (seg = 0; seg < Din->table[tbl]->n_segments; seg++) {
			Sin = Din->table[tbl]->segment[seg];
			Sout = Dout->table[tbl]->segment[seg];
			if (Sin->header) Sout->header = strdup (Sin->header);
			for (row = 0; row < Sin->n_rows; row++) {
				if (!single && !Ctrl->C.active[GMT_IN]) {	/* Want to turn geographic or polar into Cartesian */
					if (GMT_is_geographic (GMT, GMT_IN))
						GMT_geo_to_cart (GMT, Sin->coord[GMT_Y][row], Sin->coord[GMT_X][row], vector_1, TRUE);	/* get x/y/z */
					else
						GMT_polar_to_cart (GMT, Sin->coord[GMT_X][row], Sin->coord[GMT_Y][row], vector_1, TRUE);
				}
				else for (k = 0; k < nv; k++) vector_1[k] = Sin->coord[k][row];
				
				switch (Ctrl->T.mode) {
					case DO_AVERAGE:	/* Get sum of 2-D or 3-D vectors and compute average */
						for (k = 0; k < nv; k++) vector_3[k] = 0.5 * (vector_1[k] + vector_2[k]);
						break;
					case DO_BISECTOR:	/* Compute pole or bisector of vector 1 and 2 */
						get_bisector (GMT, vector_1, vector_2, vector_3);
						break;
					case DO_DOT2D:	/* Get angle between 2-D vectors */
						if (!single) GMT_normalize2v (GMT, vector_1);
						vector_3[0] = GMT_dot2v (GMT, vector_1, vector_2);
						if (Ctrl->T.degree) vector_3[0] = acosd (vector_3[0]);
						break;
					case DO_DOT3D:	/* Get angle between 3-D vectors */
						if (!single) GMT_normalize3v (GMT, vector_1);
						vector_3[0] = GMT_dot3v (GMT, vector_1, vector_2);
						if (Ctrl->T.degree) vector_3[0] = acosd (vector_3[0]);
						break;
					case DO_CROSS:	/* Get cross-product of 3-D vectors */
						GMT_cross3v (GMT, vector_1, vector_2, vector_3);
						break;
					case DO_SUM:	/* Get sum of 2-D or 3-D vectors */
						for (k = 0; k < nv; k++) vector_3[k] = vector_1[k] + vector_2[k];
						break;
					case DO_ROT2D:	/* Rotate a 2-D vector about the z_axis */
						matrix_vect_mult (GMT, 2, R, vector_1, vector_3);
						break;
					case DO_ROT3D:	/* Rotate a 3-D vector about an arbitrary pole encoded in 3x3 matrix R */
						matrix_vect_mult (GMT, 3, R, vector_1, vector_3);
						break;
					case DO_NOTHING:	/* Probably just want the effect of -C, -E, -N */
						GMT_memcpy (vector_3, vector_1, nv, double);
						break;
				}
				if (Ctrl->C.active[GMT_OUT]) {	/* Report Cartesian output... */
					if (Ctrl->N.active) {	/* ...but normalize first */
						if (n_out == 3)
							GMT_normalize3v (GMT, vector_3);
						else if (n_out == 2)
							GMT_normalize2v (GMT, vector_3);
					}
					GMT_memcpy (out, vector_3, n_out, double);
				}
				else {	/* Want to turn Cartesian output into something else */
					if (GMT_is_geographic (GMT, GMT_OUT)) {
						GMT_normalize3v (GMT, vector_3);	/* Must always normalize before calling cart_to_geo */
						GMT_cart_to_geo (GMT, &(out[GMT_Y]), &(out[GMT_X]), vector_3, TRUE);	/* Get lon/lat */
						if (Ctrl->E.active) out[GMT_Y] = GMT_lat_swap (GMT, out[GMT_Y], GMT_LATSWAP_G2O + 1);	/* Convert to geodetic */
					}
					else {
						if (Ctrl->N.active) GMT_normalize2v (GMT, vector_3);	/* Optional for cart_to_polar */
						GMT_cart_to_polar (GMT, &(out[GMT_X]), &(out[GMT_Y]), vector_3, TRUE);	/* Get r/theta */
					}
				}
				for (k = 0; k < n_out; k++) Sout->coord[k][row] = out[k];
			}
		}
	}

	if (Ctrl->A.mode) for (k = 0; k < 3; k++) Sout->coord[k+n_out][0] = E[k];	/* Place az, major, minor in the single output record */
	
	/* Time to write out the results */
	
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Establishes data output */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if (GMT_Put_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, (void **)&Ctrl->Out.file, (void **)Dout)) Return ((error = GMT_DATA_READ_ERROR));
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */
	if (single) GMT_free_dataset (GMT, &Din);
	
	Return (EXIT_SUCCESS);
}
