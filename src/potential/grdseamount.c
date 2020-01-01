/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Author:      Paul Wessel
 * Date: 	3-MAR-2013
 *
 *
 * grdseamount.c will create a grid made up from elliptical or circular
 * seamounts that can be Gaussian, Conical, Parabolic or Disc, with or without truncated
 * tops (not for dics, obviously, as already truncated). If time information
 * is provided we can also produce grids for each time step that shows either
 * the cumulative relief up until this time or just the incremental relief
 * for each time step, such as needed for time-dependent flexure.
 *
 * */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdseamount"
#define THIS_MODULE_MODERN_NAME	"grdseamount"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Create synthetic seamounts (Gaussian, parabolic, cone or disc, circular or elliptical)"
#define THIS_MODULE_KEYS	"<T{,GG},TD("
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVbdefhir" GMT_OPT("H")

#define SHAPE_GAUS	0
#define SHAPE_PARA	1
#define SHAPE_CONE	2
#define SHAPE_DISC	3

#define TRUNC_FILE	1
#define TRUNC_ARG	2

#define SMT_CUMULATIVE	0
#define SMT_INCREMENTAL	1
#define FLUX_GAUSSIAN	0
#define FLUX_LINEAR	1

struct GMT_MODELTIME {	/* Hold info about modeling time */
	double value;	/* Time in year */
	double scale;	/* Scale factor from year to given user unit */
	char unit;	/* Either M (Myr), k (kyr), or blank (implies y) */
	unsigned int u;	/* For labeling: Either 0 (yr), 1 (kyr), or 2 (Myr) */
	
};

struct GRDSEAMOUNT_CTRL {
	struct A {	/* -A[<out>/<in>] */
		bool active;
		gmt_grdfloat value[2];	/* Inside and outside value for mask */
	} A;
	struct C {	/* -C<shape> */
		bool active;
		unsigned int mode;	/* 0 = Gaussian, 1 = parabola, 2 = cone, 3 = disc */
	} C;
	struct D {	/* -De|f|k|M|n|u */
		bool active;
		char unit;
	} D;
	struct E {	/* -E */
		bool active;
	} E;
	struct F {	/* -F[<flattening>] */
		bool active;
		unsigned int mode;
		double value;
	} F;
	struct G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct L {	/* -L[<hcut>] */
		bool active;
		unsigned int mode;
		double value;
	} L;
	struct M {	/* -M<outlist> */
		bool active;
		char *file;
	} M;
	struct N {	/* -N<norm> */
		bool active;
		double value;
	} N;
	struct Q {	/* -Qc|i/g|l */
		bool active;
		unsigned int bmode;
		unsigned int fmode;
	} Q;
	struct S {	/* -S<r_scale> */
		bool active;
		double value;
	} S;
	struct T {	/* -T[l]<t0>[u]/<t1>[u]/<d0>[u]|n  */
		bool active, log;
		unsigned int n_times;
		struct GMT_MODELTIME *time;	/* The current sequence of times */
	} T;
	struct Z {	/* -Z<base> */
		bool active;
		double value;
	} Z;
};

EXTERN_MSC double gmt_get_modeltime (char *A, char *unit, double *scale);
EXTERN_MSC unsigned int gmt_modeltime_array (struct GMT_CTRL *GMT, char *arg, bool *log, struct GMT_MODELTIME **T_array);
EXTERN_MSC char *gmt_modeltime_unit (unsigned int u);
EXTERN_MSC void gmt_modeltime_name (struct GMT_CTRL *GMT, char *file, char *format, struct GMT_MODELTIME *T);

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDSEAMOUNT_CTRL *C = NULL;
	
	C = gmt_M_memory (GMT, NULL, 1, struct GRDSEAMOUNT_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->A.value[GMT_IN] = GMT->session.f_NaN;
	C->A.value[GMT_OUT] = 1.0f;
	C->Q.bmode = SMT_CUMULATIVE;
	C->Q.fmode = FLUX_GAUSSIAN;
	C->S.value = 1.0;
	C->T.n_times = 1;
	
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDSEAMOUNT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);	
	gmt_M_str_free (C->M.file);	
	gmt_M_free (GMT, C->T.time);
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [infile(s)] -G<outgrid> %s\n\t%s [-A[<out>/<in>]] [-Cc|d|g|p] [-D%s]\n", name, GMT_I_OPT, GMT_Rgeo_OPT, GMT_LEN_UNITS2_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t[-E] [-F[<flattening>]] [-L[<hcut>]] [-M<list>] [-N<norm>] [-Q<bmode><fmode>] [-S<r_scale>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T<t0>[/<t1>/<dt>|<file>|<n>[+l]]] [-Z<base>] [%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\n",
		GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\tInput contains x (or lon), y (or lat), radius, height for each seamount.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   With -E we expect lon, lat, azimuth, semi-major, semi-minor, radius, height instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -F (with no argument) is given then an extra column with flattening (0-1) is expected.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -T is given then two extra columns with start and stop times are expected.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Build a mAsk grid, append outside/inside values [1/NaN].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Here, height is ignored and -L, -N, -Q, -T and -Z are disallowed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Choose between c(one), d(isc), p(arabola) or g(aussian) model [cone].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -C is not given the we default to Gaussian features.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Specify horizontal distance unit used by input file if -fg is not used.  Choose among\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   e (meter), f (foot) k (km), M (mile), n (nautical mile), or u (survey foot) [e].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Elliptical data format [Default is Circular].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Read lon, lat, azimuth, major, minor, height (m) for each seamount.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Seamounts are truncated.  Append flattening or expect it in an extra input column [no truncation].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G filename for output grdfile with constructed surface.  If -T is set then <outgrid>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   must be a filename template that contains a floating point format (C syntax) and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   we use the corresponding time (in units specified in -T) to generate the file names.\n");
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t-L List area, volume, and mean height for each seamount; NO grid is created.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append the noise-floor cutoff level [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Give filename for output table with names of all grids produced.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no filename is given then we write the list to stdout.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Normalize grid so maximum grid height equals <norm>. Not allowed with -T.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Only used in conjunction with -T.  Append the two modes:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <bmode> to compute either (c)umulative or (i)ncremental volume through time.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <fmode> to assume a (g)aussian or (l)inear volume flux distribution.\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Sets ad hoc scale factor for radii [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify start, stop, and time increments for sequence of calculations [one step, no time dependency].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For a single specific time, just give <start>. unit is years; append k for kyr and M for Myr.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For a logarithmic time scale, append +l and specify n steps instead of time increment.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To read a list of times from the first column in a file instead, use -T<tfile>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This option implies two extra input columns with start and stop time for each seamount's life span.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Q to select cumulative versus incremental loads.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Set the reference depth [0].  Not allowed for -Qi.\n");
	GMT_Option (API, "V,bi,di,e");
	GMT_Message (API, GMT_TIME_NONE, "\t-fg Map units (lon, lat in degree, radius, major, minor in km).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is Cartesian - no units are implied; but see -D].\n");
	GMT_Option (API, "h,i,r,:,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDSEAMOUNT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdseamount and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	uint64_t n_expected_fields, k;
	int n;
	char T1[GMT_LEN32] = {""}, T2[GMT_LEN32] = {""};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input file(s) */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Mask option */
				Ctrl->A.active = true;
				if (opt->arg[0]) {
					if ((n = sscanf (opt->arg, "%[^/]/%s", T1, T2)) == 2) {
						Ctrl->A.value[GMT_OUT] = (T1[0] == 'N') ? GMT->session.f_NaN : (gmt_grdfloat)atof (T1);
						Ctrl->A.value[GMT_IN]  = (T2[0] == 'N') ? GMT->session.f_NaN : (gmt_grdfloat)atof (T2);
					}
					else {
						GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Syntax error -A: Must specify two values\n");
						n_errors++;
					}
				}
				break;
			case 'C':	/* Shape option */
				Ctrl->C.active = true;
				switch (opt->arg[0]) {
					case 'c': Ctrl->C.mode = SHAPE_CONE; break;
					case 'd': Ctrl->C.mode = SHAPE_DISC; break;
					case 'p': Ctrl->C.mode = SHAPE_PARA; break;
					case 'g': Ctrl->C.mode = SHAPE_GAUS; break;
					default:  Ctrl->C.mode = SHAPE_CONE; break;
				}
				break;
			case 'D':	/* Cartesian unit option */
				Ctrl->D.active = true;
				Ctrl->D.unit = opt->arg[0];
				break;
			case 'E':	/* Elliptical shapes */
				Ctrl->E.active = true;
				break;
			case 'F':	/* Truncation fraction */
				Ctrl->F.active = true;
				Ctrl->F.mode = TRUNC_FILE;
				if (opt->arg[0]) {
					Ctrl->F.value = atof (opt->arg);
					Ctrl->F.mode = TRUNC_ARG;
				}
				break;
			case 'G':	/* Output file name or name template */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':	/* Grid spacing */
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'L':	/* List area, volume and mean height only, then exit */
				Ctrl->L.active = true;
				if (opt->arg[0]) {
					Ctrl->L.mode = 1;
					Ctrl->L.value = atof (opt->arg);
				}
				break;
			case 'M':	/* Output file name with list of generated grids */
				Ctrl->M.active = true;
				if (opt->arg[0]) Ctrl->M.file = strdup (opt->arg);
				break;
			case 'N':	/* Normalization to max height */
				Ctrl->N.active = true;
				Ctrl->N.value = atof (opt->arg);
				break;
			case 'Q':	/* Set two modes: build mode and flux mode */
				Ctrl->Q.active = true;
				for (k = 0; opt->arg[k]; k++) {
					if (opt->arg[k] == 'i') Ctrl->Q.bmode = SMT_INCREMENTAL;
					if (opt->arg[k] == 'c') Ctrl->Q.bmode = SMT_CUMULATIVE;
					if (opt->arg[k] == 'g') Ctrl->Q.fmode = FLUX_GAUSSIAN;
					if (opt->arg[k] == 'l') Ctrl->Q.fmode = FLUX_LINEAR;
				}
				break;
			case 'S':	/* Ad hoc radial scale */
				Ctrl->S.active = true;
				Ctrl->S.value = atof (opt->arg);
				break;
			case 'T':	/* Time grid */
				Ctrl->T.active = true;
				if ((Ctrl->T.n_times = gmt_modeltime_array (GMT, opt->arg, &Ctrl->T.log, &Ctrl->T.time)) == 0)
					n_errors++;
				break;
			case 'Z':	/* Background relief level */
				Ctrl->Z.active = true;
				Ctrl->Z.value = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->C.mode == SHAPE_DISC && Ctrl->F.active, "Warning: Cannot specify -F for discs; ignored\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && (Ctrl->N.active || Ctrl->Z.active || Ctrl->L.active || Ctrl->T.active), "Syntax error -A option: Cannot use -L, -N, -T or -Z with -A\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, !(Ctrl->G.active || Ctrl->G.file), "Syntax error option -G: Must specify output file or template\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->Q.bmode == SMT_INCREMENTAL, "Syntax error option -Z: Cannot be used with -Qi\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && !strchr (Ctrl->G.file, '%'), "Syntax error -G option: Filename template must contain format specifier when -T is used\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && !Ctrl->T.active, "Syntax error -M option: Requires time information via -T\n");
	n_expected_fields = ((Ctrl->E.active) ? 6 : 4) + ((Ctrl->F.mode == TRUNC_FILE) ? 1 : 0);
	if (Ctrl->T.active) n_expected_fields += 2;	/* The two cols with start and stop time */
	n_errors += gmt_check_binary_io (GMT, n_expected_fields);
	if (Ctrl->C.mode == SHAPE_DISC && Ctrl->F.active) {Ctrl->F.active = false; Ctrl->F.mode = 0; Ctrl->F.value = 0.0;}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void disc_area_volume_height (double a, double b, double h, double hc, double f, double *A, double *V, double *z) {
	/* Compute area and volume of circular or elliptical disc "seamounts" (more like plateaus).
	 * Here, f is not used; ignore compiler warning. */

	double r2;
	gmt_M_unused(f);

	r2 = a * b;
	*A = M_PI * r2;
	*z = h - hc;
	*V = *A * (*z);
}

GMT_LOCAL void para_area_volume_height (double a, double b, double h, double hc, double f, double *A, double *V, double *z) {
	/* Compute area and volume of circular or elliptical parabolic seamounts. */
	double e, r2, rc2;

	r2 = a * b;
	e = 1.0 - f*f;
	rc2 = r2 * (1.0 - e * hc / h);	/* product of a*b where h = hc */
	*A = M_PI * rc2;
	*V = 0.5 * M_PI * r2 * h * (e * pow ((1.0/e) - (hc/h), 2.0) - f*f*((1.0/e)-1.0));
	*z = (*V) / (*A);
}

GMT_LOCAL void cone_area_volume_height (double a, double b, double h, double hc, double f, double *A, double *V, double *z) {
	/* Compute area and volume of circular or elliptical conical seamounts */

	double e, r2;

	r2 = a * b;
	e = 1.0 - f;
	*A = M_PI * r2 * (1.0 - e * hc / h);
	*V = (M_PI / (3 * e)) * r2 * h * (pow (e, 3.0) * pow ((1.0 / e) - (hc / h), 3.0) - pow (f, 3.0));
	*z = (*V) / (*A);
}

GMT_LOCAL void gaussian_area_volume_height (double a, double b, double h, double hc, double f, double *A, double *V, double *z) {
	/* Compute area and volume of circular or elliptical Gaussian seamounts */

	bool circular = doubleAlmostEqual (a, b);
	double r2, t, c, logt;

	c = (9.0 / 2.0) * f * f;
	r2 = (circular) ? a * a : a * b;
	if (fabs (hc) < GMT_CONV8_LIMIT) {	/* Exact, no noise floor */
		*A = M_PI * r2;
		*V = (2.0 / 9.0) * M_PI * r2 * h * (1.0 + c);
	}
	else {			/* Noise floor at hc */
		t = hc / h;
		logt = log (t);
		c = 1.0 + (9.0 / 2.0) * f * f;
		*A = (2.0 / 9.0) * M_PI * r2 * (c - logt);
		*V = (2.0 / 9.0) * M_PI * r2 * h * (1.0 + c - t * (1.0 + c - logt));
	}
	*z = (*V) / (*A);
}

GMT_LOCAL double cone_solver (double in[], double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction */
	double A, V0, phi, r02, h0;
	r02 = (elliptical) ? in[3] * in[4] : in[2] * in[2];
	h0 = (elliptical) ? in[5] : in[3];
	A = M_PI * r02 * h0 / (3.0 * (1 - f));
	V0 = M_PI * r02 * h0  * (1 + f + f*f) / 3.0;
	phi = pow (1.0 - V0 * (1.0 - v) / A, (1.0/3.0));
	return (phi);
}

GMT_LOCAL double para_solver (double in[], double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction */
	double A, V0, phi, r02, h0;
	r02 = (elliptical) ? in[3] * in[4] : in[2] * in[2];
	h0 = (elliptical) ? in[5] : in[3];
	A = M_PI * r02 * h0 / (2.0 * (1 - f*f));
	V0 = M_PI * r02 * h0  * (1 + f*f) / 2.0;
	phi = pow (1.0 - V0 * (1.0 - v)/A, 0.25);
	return (phi);
}

GMT_LOCAL double gauss_solver (double in[], double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction */
	int n = 0;
	double A, B, V0, phi, phi0, r02, h0;
	r02 = (elliptical) ? in[3] * in[4] : in[2] * in[2];
	h0 = (elliptical) ? in[5] : in[3];
	A = 2.0 * M_PI * r02 * h0 * exp (9.0 * f * f / 2.0) / 9.0;
	V0 = 2.0 * M_PI * r02 * h0 * (1.0 + 9.0 * f * f / 2.0) / 9.0;
	B = V0 * (1.0 - v) / A;
	phi = v * (1 - f) + f;	/* Initial guess */
	do {
		phi0 = phi;
		phi = M_SQRT2 * sqrt (-log (B/(1 + 4.5 * phi0*phi0))) / 3.0;
		n++;
	} while (fabs (phi-phi0) > 1e-6);
	return (phi);
}

GMT_LOCAL int parse_the_record (struct GMT_CTRL *GMT, struct GRDSEAMOUNT_CTRL *Ctrl, double **data, char **text, uint64_t rec, uint64_t n_expected, bool map, double inv_scale, double *in) {
	uint64_t col;
	double s_scale;
	char txt_x[GMT_LEN64], txt_y[GMT_LEN64], s_unit;
	gmt_M_unused (GMT);
	
	for (col = 0; col < n_expected; col++) in[col] = data[col][rec];	/* Just copy over this numerical part of the record */
	if (Ctrl->T.active && text) {	/* Force start and stop times to be multiples of the increment */
		sscanf (text[rec], "%s %s", txt_x, txt_y);
		in[n_expected-2] = gmt_get_modeltime (txt_x, &s_unit, &s_scale);
		in[n_expected-1] = gmt_get_modeltime (txt_y, &s_unit, &s_scale);
	}
	
	if (!map) {	/* Scale horizontal units to meters */
		in[GMT_X] *= inv_scale;
		in[GMT_Y] *= inv_scale;
		if (Ctrl->E.active) {	/* Elliptical seamount axes */
			in[3] *= inv_scale;
			in[4] *= inv_scale;
		}
		else	/* Radius */
			in[2] *= inv_scale;
	}
	
	if (Ctrl->T.active && in[n_expected-2] < in[n_expected-1])
		gmt_M_double_swap (in[n_expected-2], in[n_expected-1]);	/* Ensure start time is always larger */
	return 0;	/* OK */	
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdseamount (void *V_API, int mode, void *args) {
	int error, scol, srow, scol_0, srow_0;
	
	unsigned int n_out, nx1, d_mode, row, col, row_0, col_0;
	unsigned int max_d_col, d_row, *d_col = NULL, t, t_use, build_mode, t0_col = 0, t1_col = 0;
	
	uint64_t n_expected_fields, n_smts = 0, tbl, seg, rec, ij;
	
	bool map = false, periodic = false, replicate, first, empty;
	
	char unit, unit_name[8], file[PATH_MAX] = {""}, time_fmt[GMT_LEN64] = {""};
	
	gmt_grdfloat *data = NULL;
	double x, y, r, c, in[9], this_r, A = 0.0, B = 0.0, C = 0.0, e, e2, ca, sa, ca2, sa2, r_in, dx, dy, dV;
	double add, f, max, r_km, amplitude, h_scale = 0.0, z_assign, h_scl = 0.0, noise = 0.0, this_user_time = 0.0, life_span, t_mid, v_curr, v_prev;
	double r_mean, h_mean, wesn[4], rr, out[12], a, b, area, volume, height, DEG_PR_KM = 0.0, *V = NULL;
	double fwd_scale, inv_scale = 0.0, inch_to_unit, unit_to_inch, prev_user_time = 0.0, h_curr = 0.0, h_prev = 0.0, h0, phi_prev, phi_curr;
	double *V_sum = NULL, *h_sum = NULL, *h = NULL;
	void (*shape_func) (double a, double b, double h, double hc, double f, double *A, double *V, double *z);
	double (*phi_solver) (double in[], double f, double v, bool elliptical);
	
	struct GMT_GRID *Grid = NULL;
	struct GMT_DATASET *D = NULL;	/* Pointer to GMT multisegment text table(s) */
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_DATASET *L = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GRDSEAMOUNT_CTRL *Ctrl = NULL;
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
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != GMT_NOERROR) Return (error);

	/*---------------------------- This is the grdseamount main code ----------------------------*/
	
	/* Specify inputexpected columns */
	n_expected_fields = ((Ctrl->E.active) ? 6 : 4) + ((Ctrl->F.mode == TRUNC_FILE) ? 1 : 0);
	if (Ctrl->T.active) n_expected_fields += 2;	/* The two cols with start and stop time */
	if ((error = GMT_Set_Columns (API, GMT_IN, (unsigned int)n_expected_fields, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}

	/* Register likely data sources unless the caller has already done so */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	switch (Ctrl->C.mode) {
		case SHAPE_CONE:  shape_func = cone_area_volume_height;
				  phi_solver = cone_solver; break;
		case SHAPE_DISC:  shape_func = disc_area_volume_height;
				  phi_solver = NULL; break;
		case SHAPE_PARA:  shape_func = para_area_volume_height;
		  		  phi_solver = para_solver; break;
		case SHAPE_GAUS:  shape_func = gaussian_area_volume_height;
		  		  phi_solver = gauss_solver; break;
		default:
			GMT_Report (API, GMT_MSG_DEBUG, "Internal error: Shape not set?\n");
			shape_func = gaussian_area_volume_height;
			phi_solver = gauss_solver;
			break;
	}

	build_mode = (Ctrl->T.active) ? SHAPE_DISC : Ctrl->C.mode;	/* For incremental building we use disc increments regardless of shape */
	
	map = gmt_M_is_geographic (GMT, GMT_IN);
	if (map) {	/* Geographic data */
		DEG_PR_KM = 1.0 / GMT->current.proj.DIST_KM_PR_DEG;
		d_mode = 2, unit = 'k';	/* Select km and great-circle distances */
	}	
	else {	/* Cartesian scaling */
		unsigned int s_unit;
		s_unit = gmt_check_scalingopt (GMT, 'D', Ctrl->D.unit, unit_name);
		/* We only need inv_scale here which scales input data in these units to m */
		gmt_init_scales (GMT, s_unit, &fwd_scale, &inv_scale, &inch_to_unit, &unit_to_inch, unit_name);
		d_mode = 0, unit = 'X';	/* Select Cartesian distances */
	}
	gmt_init_distaz (GMT, unit, d_mode, GMT_MAP_DIST);
	V = gmt_M_memory (GMT, NULL, D->n_records, double);	/* Allocate volume array */
	V_sum = gmt_M_memory (GMT, NULL, D->n_records, double);	/* Allocate volume array */
	h_sum = gmt_M_memory (GMT, NULL, D->n_records, double);	/* Allocate volume array */
	h = gmt_M_memory (GMT, NULL, D->n_records, double);	/* Allocate volume array */
	if (build_mode == SHAPE_GAUS) {
		noise = exp (-4.5);		/* Normalized height of a unit Gaussian at basal radius; we must subtract this to truly get 0 at r = rbase */
		h_scl = 1.0 / (1.0 - noise);	/* Compensation scale to make the peak amplitude = 1 given our adjustment for noise above */
	}

	if (Ctrl->L.active) {	/* Just list area, volume, etc. for each seamount; no grid needed */
		n_out = (unsigned int)n_expected_fields + 3;
		if ((error = GMT_Set_Columns (API, GMT_OUT, n_out, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			gmt_M_free (GMT, V);		gmt_M_free (GMT, V_sum);
			gmt_M_free (GMT, h);		gmt_M_free (GMT, h_sum);
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			gmt_M_free (GMT, V);		gmt_M_free (GMT, V_sum);
			gmt_M_free (GMT, h);		gmt_M_free (GMT, h_sum);
			Return (API->error);
		}
	}

	/* 0. DETERMINE THE NUMBER OF TIME STEPS */
	
	if (Ctrl->T.active) {	/* Have requested a time series of bathymetry */
		t0_col = (unsigned int)(n_expected_fields - 2);
		t1_col = (unsigned int)(n_expected_fields - 1);
	}
	
	if (Ctrl->M.active) {	/* Must create dataset to hold names of all output grids */
		uint64_t dim[GMT_DIM_SIZE] = {1, 1, Ctrl->T.n_times, 0};
		unsigned int k, j;
		if ((L = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Error creating text set for file %s\n", Ctrl->M.file);
			gmt_M_free (GMT, V);		gmt_M_free (GMT, V_sum);
			gmt_M_free (GMT, h);		gmt_M_free (GMT, h_sum);
			Return (GMT_RUNTIME_ERROR);
		}
		L->table[0]->segment[0]->n_rows = Ctrl->T.n_times;
		for (k = j = 0; Ctrl->G.file[k] && Ctrl->G.file[k] != '%'; k++);	/* Find first % */
		while (Ctrl->G.file[k] && !strchr ("efg", Ctrl->G.file[k])) time_fmt[j++] = Ctrl->G.file[k++];
		time_fmt[j++] = Ctrl->G.file[k];
		strcat (time_fmt, "%c");	/* Append the unit */
		GMT_Report (API, GMT_MSG_DEBUG, "Format for time will be %s\n", time_fmt);
	}
	/* Calculate the area, volume, height for each shape; if -L then also write the results */
	
	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */
	gmt_M_memset (in, 9, double);
	for (tbl = n_smts = 0; tbl < D->n_tables; tbl++) {
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
			S = D->table[tbl]->segment[seg];	/* Set shortcut to current segment */
			for (rec = 0; rec < S->n_rows; rec++, n_smts++) {
				if (parse_the_record (GMT, Ctrl, S->data, S->text, rec, n_expected_fields, map, inv_scale, in)) continue;
				for (col = 0; col < n_expected_fields; col++) out[col] = in[col];	/* Copy of record before any scalings */
				if (Ctrl->E.active) {	/* Elliptical seamount parameters */
					a = in[3];		/* Semi-major axis */
					b = in[4];		/* Semi-minor axis */
					amplitude = in[5];	/* Seamount max height from base */
					if (Ctrl->F.mode == TRUNC_FILE) Ctrl->F.value = in[6];	/* Flattening given via input file */
				}
				else {	/* Circular features */
					a = b = in[2];		/* Radius in m */
					amplitude = in[3];	/* Seamount max height from base */
					if (Ctrl->F.mode == TRUNC_FILE) Ctrl->F.value = in[4];	/* Flattening given via input file */
				}
				c = (map) ? cosd (in[GMT_Y]) : 1.0;	/* Flat Earth scaling factor */
				/* Compute area, volume, mean amplitude */
				shape_func (a, b, amplitude, Ctrl->L.value, Ctrl->F.value, &area, &volume, &height);
				V[n_smts] = volume;
				h[n_smts] = amplitude;
				if (map) {	/* Report values in km^2, km^3, and m */
					area   *= GMT->current.proj.DIST_KM_PR_DEG * GMT->current.proj.DIST_KM_PR_DEG * c;
					volume *= GMT->current.proj.DIST_KM_PR_DEG * GMT->current.proj.DIST_KM_PR_DEG * c;
					volume *= 1.0e-3;	/* Use km^3 as unit for volume */
				}
				if (Ctrl->L.active) {	/* Only want to add back out area, volume */
					col = (unsigned int)n_expected_fields;
					out[col++] = area;
					out[col++] = volume;
					out[col++] = height;
					GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
				}
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Seamount %" PRIu64 " area, volume, mean height: %g %g %g\n", n_smts, area, volume, height);
			}
		}
	}
	gmt_M_free (GMT, Out);
	if (Ctrl->L.active) {	/* OK, that was all we wanted */
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
			gmt_M_free (GMT, V);		gmt_M_free (GMT, V_sum);
			gmt_M_free (GMT, h);		gmt_M_free (GMT, h_sum);
			Return (API->error);
		}
		gmt_M_free (GMT, V);		gmt_M_free (GMT, V_sum);
		gmt_M_free (GMT, h);		gmt_M_free (GMT, h_sum);
		Return (GMT_NOERROR);
	}
				
	/* Set up and allocate output grid */
	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL,
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
		
	gmt_set_xy_domain (GMT, wesn, Grid->header);	/* May include some padding if gridline-registered */
	nx1 = Grid->header->n_columns + Grid->header->registration - 1;
	if (map && gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])) periodic = true;
	replicate = (periodic && Grid->header->registration == GMT_GRID_NODE_REG);
	if (Ctrl->A.active) for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] = Ctrl->A.value[GMT_OUT];
	if (Ctrl->Z.active) {	/* Start with the background depth */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Set the background level to %g\r", Ctrl->Z.value);
		for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] = (gmt_grdfloat)Ctrl->Z.value;
	}
	data = gmt_M_memory (GMT, NULL, Grid->header->size, gmt_grdfloat);	/* tmp */

	for (t = t_use = 0; t < Ctrl->T.n_times; t++) {	/* For each time step (or just once) */

		/* 1. SET THE CURRENT TIME VALUE (IF USED) */
		if (Ctrl->T.active) {	/* Set the current time in user units as well as years */
			this_user_time = Ctrl->T.time[t].value;	/* In years */
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Evaluating bathymetry for time %g %s\n", Ctrl->T.time[t].value * Ctrl->T.time[t].scale, gmt_modeltime_unit (Ctrl->T.time[t].u));
		}
		if (Ctrl->Q.bmode == SMT_INCREMENTAL) gmt_M_memset (Grid->data, Grid->header->size, gmt_grdfloat);	/* Wipe clean for next increment */
		max = -DBL_MAX;
		empty = true;	/* So far, no seamounts have left a contribution */
				
		/* 2. VISIT ALL SEAMOUNTS */
		for (tbl = n_smts = 0; tbl < D->n_tables; tbl++) {
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
				S = D->table[tbl]->segment[seg];	/* Set shortcut to current segment */
				for (rec = 0; rec < S->n_rows; rec++,  n_smts++) {
					if (parse_the_record (GMT, Ctrl, S->data, S->text, rec, n_expected_fields, map, inv_scale, in)) continue;
					
					if (Ctrl->T.active && (this_user_time >= in[t0_col] || this_user_time < in[t1_col])) continue;	/* Outside time-range */
					if (gmt_M_y_is_outside (GMT, in[GMT_Y],  wesn[YLO], wesn[YHI])) continue;	/* Outside y-range */
					if (gmt_x_is_outside (GMT, &in[GMT_X], wesn[XLO], wesn[XHI])) continue;	/* Outside x-range */

					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Evaluate seamount # %6d\n", n_smts);
					/* Ok, we are inside the region - process data */
					
					if (Ctrl->T.active) {	/* Must compute volume fractions v_curr, v_prev of an evolving seamount */
						life_span = in[t0_col] - in[t1_col];	/* Total life span of this seamount */
						if (Ctrl->Q.fmode == FLUX_GAUSSIAN) {	/* Gaussian volume flux */
							t_mid = 0.5 * (in[t0_col] + in[t1_col]);	/* time at mid point in evolution */
							v_curr = 0.5 * (1.0 + erf (-6.0 * (this_user_time - t_mid) / (M_SQRT2 * life_span)));	/* Normalized volume fraction at end of this time step */
							v_prev = 0.5 * (1.0 + erf (-6.0 * (prev_user_time - t_mid) / (M_SQRT2 * life_span)));	/* Normalized volume fraction at start of this time step */
							if (v_prev < 0.0015) v_prev = 0.0;	/* Deal with the 3-sigma truncation, i.e., throw first tail in with first slice */
							if (v_curr > 0.9985) v_curr = 1.0;	/* Deal with the 3-sigma truncation, i.e., throw last tail in with last slice */
						}
						else {	/* Linear volume flux */
							v_curr = (in[t0_col] - this_user_time) / life_span;	/* Normalized volume fraction at end of this time step */
							v_prev = (in[t0_col] - prev_user_time) / life_span;	/* Normalized volume fraction at start of this time step */
						}
						dV = V[n_smts] * (v_curr - v_prev);	/* Incremental volume produced */
						V_sum[n_smts] += dV;			/* Keep track of volume sum so we can compare with truth later */
						if (Ctrl->F.mode == TRUNC_FILE)
							f = (Ctrl->E.active) ? in[6] : in[4];
						else
							f = Ctrl->F.value;
						phi_curr = phi_solver (in, f, v_curr, Ctrl->E.active);
						phi_prev = phi_solver (in, f, v_prev, Ctrl->E.active);
						h0 = (Ctrl->E.active) ? in[5] : in[3];
						switch (Ctrl->C.mode) {	/* Given the phi values, evaluate the corresponding heights */
							case SHAPE_CONE:  h_curr = h0 * (1 - phi_curr) / (1 - f); h_prev = h0 * (1 - phi_prev) / (1 - f); break;
							case SHAPE_PARA:  h_curr = h0 * (1 - phi_curr * phi_curr) / (1 - f * f); h_prev = h0 * (1 - phi_prev * phi_prev) / (1 - f * f); break;
							case SHAPE_GAUS:  h_curr = h0 * exp (4.5 * (f*f - phi_curr * phi_curr)); h_prev = h0 * exp (4.5 * (f*f - phi_prev * phi_prev)); break;
						}
						h_mean = fabs (h_curr - h_prev);	/* This is our disc layer thickness */
						r_mean = sqrt (dV / (M_PI * h_mean));	/* Radius given by volume and height */
						h_sum[n_smts] += h_mean;		/* Keep track of height sum so we can compare with truth later */

						GMT_Report (API, GMT_MSG_DEBUG, "r_mean = %.1f h_mean = %.1f v_prev = %.3f v_curr = %.3f phi_prev = %.3f phi_curr = %.3f h_prev = %.1f h_curr = %.1f V_sum = %g h_sum = %g\n",
							r_mean, h_mean, v_prev, v_curr, phi_prev, phi_curr, h_prev, h_curr, V_sum[n_smts], h_sum[n_smts]);
						/* Replace the values in the in array with these incremental values instead */
						if (Ctrl->E.active) {	/* Elliptical parameters */
							e = in[4] / in[3];	/* Eccentricity */
							in[3] = r_mean / sqrt (e);	/* Since we solved for sqrt (a*b) above */
							in[4] = in[3] * e;
							in[5] = h_mean;
						}
						else {	/* Circular */
							in[GMT_Z] = r_mean;
							in[3] = h_mean;
						}
					}
					
					scol_0 = (int)gmt_M_grd_x_to_col (GMT, in[GMT_X], Grid->header);	/* Center column */
					if (scol_0 < 0) continue;	/* Still outside x-range */
					if ((col_0 = scol_0) >= Grid->header->n_columns) continue;	/* Still outside x-range */
					srow_0 = (int)gmt_M_grd_y_to_row (GMT, in[GMT_Y], Grid->header);	/* Center row */
					if (srow_0 < 0) continue;	/* Still outside y-range */
					if ((row_0 = srow_0) >= Grid->header->n_rows) continue;	/* Still outside y-range */
					
					if (Ctrl->E.active) {	/* Elliptical seamount parameters */
						sincos ((90.0 - in[GMT_Z]) * D2R, &sa, &ca);	/* in[GMT_Z] is azimuth in degrees, convert to direction, get sin/cos */
						a = in[3];			/* Semi-major axis */
						b = in[4];			/* Semi-minor axis */
						e = b / a;		/* Eccentricity */
						e2 = e * e;
						ca2 = ca * ca;
						sa2 = sa * sa;
						r_km = b * Ctrl->S.value;	/* Scaled semi-minor axis in user units (Cartesian or km) */
						r = r_km;
						if (map) r *= DEG_PR_KM;	/* Was in km so now it is in degrees, same units as grid coordinates */
						f = -4.5 / (r_km * r_km);	/* So we can take exp (f * radius_in_km^2) */
						A = f * (e2 * ca2 + sa2);	/* Elliptical components A, B, C needed to evaluate radius(az) */
						B = -f * (sa * ca * (1.0 - e2));
						C = f * (e2 * sa2 + ca2);
						r_in = a;			/* Semi-major axis in user units (Cartesian or km)*/
						r_km = r_in * Ctrl->S.value;	/* Scaled semi-major axis in user units (Cartesian or km) */
						r = r_km;			/* Copy of r_km */
						if (map) r *= DEG_PR_KM;	/* Was in km so now it is in degrees, same units as grid coordinates */
						amplitude = in[5];		/* Seamount max height from base */
						if (Ctrl->F.mode == TRUNC_FILE) Ctrl->F.value = in[6];	/* Flattening given by input file */
					}
					else {	/* Circular features */
						r_in = a = b = in[GMT_Z];	/* Radius in user units */
						r_km = r_in * Ctrl->S.value;	/* Scaled up by user scale */
						r = r_km;			/* Copy of r_km */
						if (map) r *= DEG_PR_KM;	/* Was in km so now it is in degrees, same units as grid coordinates */
						f = (build_mode == SHAPE_CONE) ? 1.0 / r_km : -4.5 / (r_km * r_km);	/* So we can take exp (f * radius_in_km^2) */
						amplitude = in[3];		/* Seamount max height from base */
						if (Ctrl->F.mode == TRUNC_FILE) Ctrl->F.value = in[4];	/* Flattening given by input file */
					}
					c = (map) ? cosd (in[GMT_Y]) : 1.0;	/* Flat Earth area correction */
					switch (build_mode) {
						case SHAPE_CONE:  h_scale = 1.0 / (1.0 - Ctrl->F.value); break;
						case SHAPE_DISC:  h_scale = 1.0; break;
						case SHAPE_PARA:  h_scale = 1.0 / (1.0 - Ctrl->F.value * Ctrl->F.value); break;
						case SHAPE_GAUS:  h_scale = 1.0 / exp (-4.5 * Ctrl->F.value * Ctrl->F.value); break;
					}
					if (build_mode == SHAPE_GAUS) h_scale *= h_scl;
					if (!(Ctrl->A.active || amplitude > 0.0)) continue;	/* No contribution from this seamount */

					/* Initialize local search machinery, i.e., what is the range of rows and cols we need to search */
					gmt_M_free (GMT, d_col);
					d_col = gmt_prep_nodesearch (GMT, Grid, r_km, d_mode, &d_row, &max_d_col);
		
					for (srow = srow_0 - (int)d_row; srow <= (srow_0 + (int)d_row); srow++) {
						if (srow < 0) continue;
						if ((row = srow) >= Grid->header->n_rows) continue;
						y = gmt_M_grd_row_to_y (GMT, row, Grid->header);
						first = replicate;	/* Used to help us deal with duplicate columns for grid-line registered global grids */
						for (scol = scol_0 - (int)d_col[row]; scol <= (scol_0 + (int)d_col[row]); scol++) {
							if (!periodic) {
								if (scol < 0) continue;
								if ((col = scol) >= Grid->header->n_columns) continue;
							}
							if (scol < 0)	/* Periodic grid: Break on through to other side! */
								col = scol + nx1;
							else if ((col = scol) >= Grid->header->n_columns) 	/* Periodic grid: Wrap around to other side */
								col -= nx1;
							/* "silent" else we are inside w/e */
							x = gmt_M_grd_col_to_x (GMT, col, Grid->header);
							this_r = gmt_distance (GMT, in[GMT_X], in[GMT_Y], x, y);	/* In Cartesian units or km (if map is true) */
							if (this_r > r_km) continue;	/* Beyond the base of the seamount */
							if (Ctrl->E.active) {	/* For Gaussian we must deal with direction etc */
								dx = (map) ? (x - in[GMT_X]) * GMT->current.proj.DIST_KM_PR_DEG * c : (x - in[GMT_X]);
								dy = (map) ? (y - in[GMT_Y]) * GMT->current.proj.DIST_KM_PR_DEG : (y - in[GMT_Y]);
								this_r = A * dx * dx + 2.0 * B * dx * dy + C * dy * dy;
								/* this_r is now r^2 in the 0 to -4.5 range expected for the Gaussian case */
								rr = sqrt (-this_r/4.5);	/* Convert this r^2 to a normalized radius 0-1 inside cone */
								if (Ctrl->A.active && rr > 1.0) continue;	/* Beyond the seamount base so nothing to do for a mask */
								if (build_mode == SHAPE_CONE) {	/* Elliptical cone case */
									if (rr < 1.0)	/* Since in minor direction rr may exceed 1 and be outside the ellipse */
										add = (rr < Ctrl->F.value) ? 1.0 : (1.0 - rr) * h_scale;
									else
										add = 0.0;
								}
								else if (build_mode == SHAPE_DISC)	/* Elliptical disc/plateau case */
									add = (rr <= 1.0) ? 1.0 : 0.0;
								else if (build_mode == SHAPE_PARA)	/* Elliptical parabolic case */
									add = (rr < Ctrl->F.value) ? 1.0 : (1.0 - rr*rr) * h_scale;
								else	/* Elliptical Gaussian case */
									add = (rr < Ctrl->F.value) ? 1.0 : exp (this_r) * h_scale - noise;
							}
							else {	/* Circular features */
								rr = this_r / r_km;	/* Now in 0-1 range */
								if (build_mode == SHAPE_CONE)	/* Circular cone case */
									add = (rr < Ctrl->F.value) ? 1.0 : (1.0 - rr) * h_scale;
								else if (build_mode == SHAPE_DISC)	/* Circular disc/plateau case */
									add = (rr <= 1.0) ? 1.0 : 0.0;
								else if (build_mode == SHAPE_PARA)	/* Circular parabolic case */
									add = (rr < Ctrl->F.value) ? 1.0 : (1.0 - rr*rr) * h_scale;
								else	/* Circular Gaussian case */
									add = (rr < Ctrl->F.value) ? 1.0 : exp (f * this_r * this_r) * h_scale - noise;
							}
							if (add <= 0.0) continue;	/* No amplitude, so skip */
							ij = gmt_M_ijp (Grid->header, row, col);	/* Current node location */
							z_assign = amplitude * add;		/* height to be added */
							if (Ctrl->A.active)	/* Just set inside value for mask */
								Grid->data[ij] = Ctrl->A.value[GMT_IN];
							else {	/* Add in contribution and keep track of max height */
								Grid->data[ij] += (gmt_grdfloat)z_assign;
								if (Grid->data[ij] > max) max = Grid->data[ij];
							}
							if (first) {	/* May have to copy over to repeated column in global gridline-registered grids */
								if (col == 0) {	/* Must copy from x_min to repeated column at x_max */
									if (Ctrl->A.active) Grid->data[ij+nx1] = Ctrl->A.value[GMT_IN]; else Grid->data[ij+nx1] += (gmt_grdfloat)z_assign;
									first = false;
								}
								else if (col == nx1) {	/* Must copy from x_max to repeated column at x_min */
									if (Ctrl->A.active) Grid->data[ij-nx1] = Ctrl->A.value[GMT_IN]; else Grid->data[ij-nx1] += (gmt_grdfloat)z_assign;
									first = false;
								}
							}
							empty = false;	/* We have made a mark on this grid */
						}
					}
				}
			}
			prev_user_time = this_user_time;	/* Make this the previous time */
		}
		if (empty) {
			GMT_Report (API, GMT_MSG_VERBOSE, "No contribution made for time %g %s\n",
			            Ctrl->T.time[t].value * Ctrl->T.time[t].scale, gmt_modeltime_unit (Ctrl->T.time[t].u));
			continue;
		}
		
		/* Time to write the grid */
		if (Ctrl->T.active)
			gmt_modeltime_name (GMT, file, Ctrl->G.file, &(Ctrl->T.time[t]));
		else
			strcpy (file, Ctrl->G.file);
		if (Ctrl->M.active) {
			char record[GMT_BUFSIZ] = {""}, tmp[GMT_LEN64] = {""};
			if (Ctrl->T.active) {
				sprintf (record, "%s\t", file);
				sprintf (tmp, time_fmt, Ctrl->T.time[t].value * Ctrl->T.time[t].scale, Ctrl->T.time[t].unit);
				strcat (record, tmp);
			}
			else
				strcpy (record, file);
			L->table[0]->segment[0]->text[t_use++] = strdup (record);
			L->table[0]->segment[0]->n_rows++;
		}
		
		if (Ctrl->N.active) {	/* Normalize so max height == N.value */
			double n_scl = Ctrl->N.value / max;
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Normalize seamount amplitude so max height is %g\r", Ctrl->N.value);
			for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] *= (gmt_grdfloat)n_scl;
		}

		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) {
			gmt_M_free (GMT, d_col);	gmt_M_free (GMT, V);		gmt_M_free (GMT, h);
			gmt_M_free (GMT, V_sum);	gmt_M_free (GMT, h_sum);	gmt_M_free (GMT, data);
			Return (API->error);
		}
		gmt_M_memcpy (data, Grid->data, Grid->header->size, gmt_grdfloat);	/* This will go away once gmt_nc.c is fixed to leave array alone */
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, Grid) != GMT_NOERROR) {
			gmt_M_free (GMT, d_col);	gmt_M_free (GMT, V);		gmt_M_free (GMT, h);
			gmt_M_free (GMT, V_sum);	gmt_M_free (GMT, h_sum);	gmt_M_free (GMT, data);
			Return (API->error);
		}
		gmt_M_memcpy (Grid->data, data, Grid->header->size, gmt_grdfloat);
	}
	if (Ctrl->M.active) L->table[0]->n_records = t_use;
	if (Ctrl->M.active && GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, 0, NULL, Ctrl->M.file, L) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error writing list of grid files to %s\n", Ctrl->M.file);
		gmt_M_free (GMT, d_col);	gmt_M_free (GMT, V);		gmt_M_free (GMT, h);
		gmt_M_free (GMT, V_sum);	gmt_M_free (GMT, h_sum);	gmt_M_free (GMT, data);
		Return (API->error);
	}

	gmt_M_free (GMT, d_col);	gmt_M_free (GMT, V);		gmt_M_free (GMT, h);
	gmt_M_free (GMT, V_sum);	gmt_M_free (GMT, h_sum);	gmt_M_free (GMT, data);
	
	Return (GMT_NOERROR);
}
