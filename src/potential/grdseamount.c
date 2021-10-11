/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * tops (not for discs, obviously, as already truncated). If time information
 * is provided we can also produce grids for each time step that shows either
 * the cumulative relief up until this time or just the incremental relief
 * for each time step, such as needed for time-dependent flexure. These estimates
 * can be either exact or approximated via constant-thickness discs. Seamounts
 * can use different models so you can mix and match cones and Gaussians, for example.
 *
 * */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdseamount"
#define THIS_MODULE_MODERN_NAME	"grdseamount"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Create synthetic seamounts (Gaussian, parabolic, polynomial, cone or disc; circular or elliptical)"
#define THIS_MODULE_KEYS	"<T{,GG},LD),MD),TD("
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVbdefhir" GMT_OPT("H")

#define SHAPE_GAUS	0
#define SHAPE_PARA	1
#define SHAPE_CONE	2
#define SHAPE_POLY	3
#define SHAPE_DISC	4
#define N_SHAPES	5

#define TRUNC_FILE	1
#define TRUNC_ARG	2

#define SMT_CUMULATIVE	0
#define SMT_INCREMENTAL	1
#define FLUX_GAUSSIAN	0
#define FLUX_LINEAR	1
#define ONETHIRD 0.333333333333333333333333333333333333333333333333333333333333

struct GMT_MODELTIME {	/* Hold info about modeling time */
	double value;	/* Time in year */
	double scale;	/* Scale factor from year to given user unit */
	char unit;	/* Either M (Myr), k (kyr), or blank (implies y) */
	unsigned int u;	/* For labeling: Either 0 (yr), 1 (kyr), or 2 (Myr) */

};

struct GRDSEAMOUNT_CTRL {
	struct GRDSEAMOUNT_A {	/* -A[<out>/<in>] */
		bool active;
		gmt_grdfloat value[2];	/* Inside and outside value for mask */
	} A;
	struct GRDSEAMOUNT_C {	/* -C[<shape>] */
		bool active;
		bool input;
		char code;
		unsigned int mode;	/* 0 = Gaussian, 1 = parabola, 2 = cone, 3 = poly, 4 = disc */
	} C;
	struct GRDSEAMOUNT_D {	/* -De|f|k|M|n|u */
		bool active;
		char unit;
	} D;
	struct GRDSEAMOUNT_E {	/* -E */
		bool active;
	} E;
	struct GRDSEAMOUNT_F {	/* -F[<flattening>] */
		bool active;
		unsigned int mode;
		double value;
	} F;
	struct GRDSEAMOUNT_G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct GRDSEAMOUNT_I {	/* -I (for checking only) */
		bool active;
	} I;
	struct GRDSEAMOUNT_L {	/* -L[<hcut>] */
		bool active;
		unsigned int mode;
		double value;
	} L;
	struct GRDSEAMOUNT_M {	/* -M<outlist> */
		bool active;
		char *file;
	} M;
	struct GRDSEAMOUNT_N {	/* -N<norm> */
		bool active;
		double value;
	} N;
	struct GRDSEAMOUNT_Q {	/* -Qc|i/g|c  Note: c was l previously */
		bool active;
		bool disc;	/* True if incremental volumes should be served as disks [Default is exact] */
		unsigned int bmode;
		unsigned int fmode;
	} Q;
	struct GRDSEAMOUNT_S {	/* -S<r_scale> */
		bool active;
		double value;
	} S;
	struct GRDSEAMOUNT_T {	/* -T[l]<t0>/<t1>/<d0>|n  */
		bool active, log;
		unsigned int n_times;
		struct GMT_MODELTIME *time;	/* The current sequence of times */
	} T;
	struct GRDSEAMOUNT_Z {	/* -Z<base> [0] */
		bool active;
		double value;
	} Z;
};

EXTERN_MSC double gmt_get_modeltime (char *A, char *unit, double *scale);
EXTERN_MSC unsigned int gmt_modeltime_array (struct GMT_CTRL *GMT, char *arg, bool *log, struct GMT_MODELTIME **T_array);
EXTERN_MSC char *gmt_modeltime_unit (unsigned int u);
EXTERN_MSC void gmt_modeltime_name (struct GMT_CTRL *GMT, char *file, char *format, struct GMT_MODELTIME *T);

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDSEAMOUNT_CTRL *C = NULL;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDSEAMOUNT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->A.value[GMT_IN] = GMT->session.f_NaN;
	C->A.value[GMT_OUT] = 1.0f;
	C->C.mode = SHAPE_GAUS;
	C->C.code = 'g';
	C->Q.bmode = SMT_CUMULATIVE;
	C->Q.fmode = FLUX_GAUSSIAN;
	C->S.value = 1.0;
	C->T.n_times = 1;

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDSEAMOUNT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->M.file);
	gmt_M_free (GMT, C->T.time);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] -G%s %s %s [-A[<out>/<in>]] [-C[c|d|g|o|p]] [-D%s] "
		"[-E] [-F[<flattening>]] [-L[<hcut>]] [-M[<list>]] [-N<norm>] [-Q<bmode><fmode>[+d]] [-S<r_scale>] "
		"[-T<t0>[/<t1>/<dt>|<file>|<n>[+l]]] [%s] [-Z<base>] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_OUTGRID, GMT_I_OPT, GMT_Rgeo_OPT, GMT_LEN_UNITS2_DISPLAY, GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT,
		GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS (unless -L is used):\n");
	GMT_Usage (API, 1, "\n<table>");
	GMT_Usage (API, -2, "One or more data files (in ASCII, binary, netCDF). If no files are given, standard "
		"input is read. Records contain x (or lon), y (or lat), radius, height for each seamount. "
		"With -E we expect lon, lat, azimuth, semi-major, semi-minor, height instead. "
		"If -F (with no argument) is given then an extra column with flattening (0-1) is expected. "
		"If -T is given then two extra columns with start and stop times are expected.");
	gmt_outgrid_syntax (API, 'G', "Filename for output grid with constructed surface. If -T is set then <outgrid> "
		"must be a filename template that contains a floating point format (C syntax) and "
		"we use the corresponding time (in units specified in -T) to generate the file names.");
	GMT_Option (API, "I,R");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A[<out>/<in>]");
	GMT_Usage (API, -2, "Build a mAsk grid, append outside/inside values [1/NaN]. "
		"Here, height is ignored and -L, -N, -Q, -T and -Z are disallowed.");
	GMT_Usage (API, 1, "\n-C[c|d|g|o|p]");
	GMT_Usage (API, -2, "Choose between a variety of shapes [Default is Gaussian]:");
	GMT_Usage (API, 3, "c: Cone radial shape.");
	GMT_Usage (API, 3, "d: Disc radial shape.");
	GMT_Usage (API, 3, "g: Gaussian radial shape.");
	GMT_Usage (API, 3, "o: Polynomial radial shape.");
	GMT_Usage (API, 3, "p: Parabolic radial shape.");
	GMT_Usage (API, -2, "Note: If -C is given without argument then we expect to read c,d,g,o or p from the trailing input text.");
	GMT_Usage (API, 1, "\n-D%s", GMT_LEN_UNITS2_DISPLAY);
	GMT_Usage (API, -2, "Specify horizontal distance unit used by input file if -fg is not used.  Choose among "
		"e (meter), f (foot) k (km), M (mile), n (nautical mile), or u (survey foot) [e].");
	GMT_Usage (API, 1, "\n-E Elliptical data format [Default is Circular]. "
		"Read lon, lat, azimuth, major, minor, height (m) for each seamount.");
	GMT_Usage (API, 1, "\n-F[<flattening>]");
	GMT_Usage (API, -2, "Seamounts are truncated. Append flattening or expect it in an extra input column [no truncation]. ");
	GMT_Usage (API, 1, "\n-L[<hcut>]");
	GMT_Usage (API, -2, "List area, volume, and mean height for each seamount; NO grid is created so -R -I are not required. "
		"Optionally, append the noise-floor cutoff level [0].");
	GMT_Usage (API, 1, "\n-M[<list>]");
	GMT_Usage (API, -2, "Give filename for output table with names of all grids produced. "
		"If no filename is given then we write the list to standard output.");
	GMT_Usage (API, 1, "\n-N<norm>");
	GMT_Usage (API, -2, "Normalize grid so maximum grid height equals <norm>. Not allowed with -T.");
	GMT_Usage (API, 1, "\n-Q<bmode><fmode>[+d]");
	GMT_Usage (API, -2, "Only used in conjunction with -T. Append the two modes:");
	GMT_Usage (API, 3, "<bmode>: Build either (c)umulative [Default] or (i)ncremental volume through time.");
	GMT_Usage (API, 3, "<fmode>: Assume a (g)aussian [Default] or (c)onstant volume flux distribution.");
	GMT_Usage (API, -2, "Append +d to build grids with increments as uniform discs [exact shapes].");
	GMT_Usage (API, 1, "\n-S<r_scale>");
	GMT_Usage (API, -2, "Set ad hoc scale factor for radii [1].");
	GMT_Usage (API, 1, "\n-T<t0>[/<t1>/<dt>[+l]]|<file>");
	GMT_Usage (API, -2, "Specify start, stop, and time increments for sequence of calculations [one step, no time dependency]. "
		"For a single specific time, just give <t0> (in years; append k for kyr and M for Myr).");
	GMT_Usage (API, 3, "+l For a logarithmic time scale, interpret <dt> as n_steps instead of time increment.");
	GMT_Usage (API, -2, "Alternatively, read a list of times from the first column in a file instead by appending <tfile>. "
		"Note: This option implies two extra input columns with <start> and <stop> time for each seamount's life span. "
		"Use -Q to select cumulative versus incremental construction.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-Z<base>");
	GMT_Usage (API, -2, "Set the reference depth [0].  Not allowed for -Qi.");
	GMT_Option (API, "bi,di,e");
	GMT_Usage (API, 1, "\n-fg Map units (lon, lat in degree, radius, major, minor in km) "
		"[Default is Cartesian - no units are implied; but see -D].");
	GMT_Option (API, "h,i,r,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDSEAMOUNT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdseamount and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	uint64_t n_expected_fields;
	int n;
	char T1[GMT_LEN32] = {""}, T2[GMT_LEN32] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input file(s) */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Mask option */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				if (opt->arg[0]) {
					if ((n = sscanf (opt->arg, "%[^/]/%s", T1, T2)) == 2) {
						Ctrl->A.value[GMT_OUT] = (T1[0] == 'N') ? GMT->session.f_NaN : (gmt_grdfloat)atof (T1);
						Ctrl->A.value[GMT_IN]  = (T2[0] == 'N') ? GMT->session.f_NaN : (gmt_grdfloat)atof (T2);
					}
					else {
						GMT_Report (API, GMT_MSG_WARNING, "Option -A: Must specify two values\n");
						n_errors++;
					}
				}
				break;
			case 'C':	/* Shape option */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				Ctrl->C.code = opt->arg[0];
				switch (opt->arg[0]) {
					case 'c': Ctrl->C.mode = SHAPE_CONE; break;
					case 'd': Ctrl->C.mode = SHAPE_DISC; break;
					case 'g': Ctrl->C.mode = SHAPE_GAUS; break;
					case 'o': Ctrl->C.mode = SHAPE_POLY; break;
					case 'p': Ctrl->C.mode = SHAPE_PARA; break;
					default:
						if (opt->arg[0]) {
							GMT_Report (API, GMT_MSG_WARNING, "Option -G: Unrecognized shape %s\n", opt->arg);
							n_errors++;
						}
						else	/* Read from trailing text instead */
							Ctrl->C.input = true;
						break;
				}
				break;
			case 'D':	/* Cartesian unit option */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				Ctrl->D.unit = opt->arg[0];
				break;
			case 'E':	/* Elliptical shapes */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				Ctrl->E.active = true;
				break;
			case 'F':	/* Truncation fraction */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				Ctrl->F.mode = TRUNC_FILE;
				if (opt->arg[0]) {
					Ctrl->F.value = atof (opt->arg);
					Ctrl->F.mode = TRUNC_ARG;
				}
				break;
			case 'G':	/* Output file name or name template */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file))) n_errors++;
				break;
			case 'I':	/* Grid spacing */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'L':	/* List area, volume and mean height only, then exit */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.active = true;
				if (opt->arg[0]) {
					Ctrl->L.mode = 1;
					Ctrl->L.value = atof (opt->arg);
				}
				break;
			case 'M':	/* Output file name with list of generated grids */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				Ctrl->M.active = true;
				if (opt->arg[0]) Ctrl->M.file = strdup (opt->arg);
				break;
			case 'N':	/* Normalization to max height */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				Ctrl->N.value = atof (opt->arg);
				break;
			case 'Q':	/* Set two modes: build mode and flux mode */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				Ctrl->Q.active = true;
				if ((c = strstr (opt->arg, "+d"))) {
					Ctrl->Q.disc = true;
					c[0] = '\0';
				}
				if (opt->arg[0]) {	/* Gave mbode/fmode */
					char b, f;
					if (sscanf (opt->arg, "%c/%c", &b, &f) == 2) {
						Ctrl->Q.bmode = (b == 'i' || b == 'l') ? SMT_INCREMENTAL : SMT_CUMULATIVE;	/* Allow l for backwards compatibility */
						Ctrl->Q.fmode = (f == 'c') ? FLUX_LINEAR : FLUX_GAUSSIAN;
					}
					else {
						GMT_Report (API, GMT_MSG_WARNING, "Option -Q: Unable to parse the two modes\n");
						n_errors++;
					}
				}
				if (c) c[0] = '+';
				break;
			case 'S':	/* Ad hoc radial scale */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				Ctrl->S.active = true;
				Ctrl->S.value = atof (opt->arg);
				break;
			case 'T':	/* Time grid */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				Ctrl->T.active = true;
				if ((Ctrl->T.n_times = gmt_modeltime_array (GMT, opt->arg, &Ctrl->T.log, &Ctrl->T.time)) == 0)
					n_errors++;
				break;
			case 'Z':	/* Background relief level */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.active = true;
				Ctrl->Z.value = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->C.mode == SHAPE_DISC && Ctrl->F.active, "Cannot specify -F for discs; ignored\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && (Ctrl->N.active || Ctrl->Z.active || Ctrl->L.active || Ctrl->T.active), "Option -A: Cannot use -L, -N, -T or -Z with -A\n");
	if (!Ctrl->L.active) {	/* Just a listing, cannot use -R -I -r -G -M -Z */
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
		n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Option -I: Must specify positive increment(s)\n");
		n_errors += gmt_M_check_condition (GMT, !(Ctrl->G.active || Ctrl->G.file), "Option -G: Must specify output file or template\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && !strchr (Ctrl->G.file, '%'), "Option -G: Filename template must contain format specifier when -T is used\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->Q.bmode == SMT_INCREMENTAL, "Option -Z: Cannot be used with -Qi\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && !Ctrl->T.active, "Option -M: Requires time information via -T\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->F.mode == TRUNC_ARG && (Ctrl->F.value < 0.0 || Ctrl->F.value >= 1.0), "Option -F: Flattening must be in 0-1 range\n");
	}
	n_expected_fields = ((Ctrl->E.active) ? 6 : 4) + ((Ctrl->F.mode == TRUNC_FILE) ? 1 : 0);
	if (Ctrl->T.active) n_expected_fields += 2;	/* The two cols with start and stop time */
	n_errors += gmt_check_binary_io (GMT, n_expected_fields);
	if (Ctrl->C.mode == SHAPE_DISC && Ctrl->F.active) {Ctrl->F.active = false; Ctrl->F.mode = 0; Ctrl->F.value = 0.0;}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void grdseamount_disc_area_volume_height (double a, double b, double h, double hc, double f, double *A, double *V, double *z) {
	/* Compute area and volume of circular or elliptical disc "seamounts" (more like plateaus).
	 * Here, f is not used; ignore compiler warning. */

	double r2;
	gmt_M_unused(f);

	r2 = a * b;
	*A = M_PI * r2;
	*z = h - hc;
	*V = *A * (*z);
}

GMT_LOCAL void grdseamount_para_area_volume_height (double a, double b, double h, double hc, double f, double *A, double *V, double *z) {
	/* Compute area and volume of circular or elliptical parabolic seamounts. */
	double e, r2, rc2;

	r2 = a * b;
	e = 1.0 - f*f;
	rc2 = r2 * (1.0 - e * hc / h);	/* product of a*b where h = hc */
	*A = M_PI * rc2;
	*V = 0.5 * M_PI * r2 * h * (e * pow ((1.0/e) - (hc/h), 2.0) - f*f*((1.0/e)-1.0));
	*z = (*V) / (*A);
}

GMT_LOCAL void grdseamount_cone_area_volume_height (double a, double b, double h, double hc, double f, double *A, double *V, double *z) {
	/* Compute area and volume of circular or elliptical conical seamounts */

	double e, r2;

	r2 = a * b;
	e = 1.0 - f;
	*A = M_PI * r2 * (1.0 - e * hc / h);
	*V = (M_PI / (3 * e)) * r2 * h * (pow (e, 3.0) * pow ((1.0 / e) - (hc / h), 3.0) - pow (f, 3.0));
	*z = (*V) / (*A);
}

GMT_LOCAL void grdseamount_gaussian_area_volume_height (double a, double b, double h, double hc, double f, double *A, double *V, double *z) {
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

GMT_LOCAL double poly_smt_func (double r) {
	/* Polynomial radial function similar to a Gaussian */
	return (pow ((1.0 + r) * (1.0 - r), 3.0) / (1.0 + pow (r, 3.0)));
}

GMT_LOCAL double ddr_poly_smt_func (double r) {
	/* Radial derivative of polynomial radial function similar to a Gaussian */
	double r2 = r * r;
	return (-(3.0 * r * pow (r - 1.0, 2.0) * (r2 * r + r + 2.0)) / pow (r2 - r + 1.0, 2.0));
}

GMT_LOCAL double poly_smt_rc (double hc) {
	/* Determine the radius where h(rc) == hc via Newton-Rhapson */
	unsigned int k = 0;
	double r0 = 0.5, r1, dr;
	do {
		k++;
    	r1 = r0 - (poly_smt_func (r0) - hc) / ddr_poly_smt_func (r0);
    	dr = fabs (r1 - r0);
    	r0 = r1;
    }
    while (k < 20 && dr > GMT_CONV12_LIMIT);
    return (r0);
}

GMT_LOCAL double poly_smt_vol (double r) {
	/* Volume of polynomial seamount of unit amplitude from normalized r' = 0 to r */
	double r2 = r * r, r3 = r2 * r, r5 = r2 * r3, V, root3 = sqrt (3.0);
	V = TWO_PI * (root3 * (M_PI / 6.0 + atan (root3 * (2.0 * r - 1.0) / 3.0)) - 1.5 * log (r2 - r + 1.0) - 3.0 * r + 0.5 * r2 + r3 - 0.2 * r5);
	return (V);
}

GMT_LOCAL void grdseamount_poly_area_volume_height (double a, double b, double h, double hc, double f, double *A, double *V, double *z) {
	/* Compute area and volume of circular or elliptical polynomial seamounts. */
	bool circular = doubleAlmostEqual (a, b);
	double r2, r, rc = (hc > 0.0) ? poly_smt_rc (hc / h) : 1.0;	/* Fraction of normalized radius at noise floor */
	double beta = (poly_smt_vol (rc) - poly_smt_vol (f)) / poly_smt_func (f);
	r2 = (circular) ? a * a : a * b;
	r = sqrt (r2);	/* Mean radius */
	*A = M_PI * r2;
	*V = r2 * h * (beta + M_PI * f * f) - M_PI * pow (rc * r, 2.0) * hc;
	*z = (*V) / (*A);
}

/* Below, phi is a factor that we scale with r (or a,b) for a seamount matching the volume fraction */
GMT_LOCAL double grdseamount_cone_solver (double in[], double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction */
	double A, V0, phi, r02, h0;
	r02 = (elliptical) ? in[3] * in[4] : in[2] * in[2];
	h0  = (elliptical) ? in[5] : in[3];
	A = M_PI * r02 * h0 / (3.0 * (1 - f));
	V0 = M_PI * r02 * h0  * (1 + f + f*f) / 3.0;
	phi = pow (1.0 - V0 * (1.0 - v) / A, ONETHIRD);
	return (phi);
}

GMT_LOCAL double grdseamount_disc_solver (double in[], double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction fro a disc is trivial */
	gmt_M_unused (in), gmt_M_unused (f), gmt_M_unused (elliptical);
	return (v);
}

GMT_LOCAL double grdseamount_para_solver (double in[], double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction */
	double A, V0, phi, r02, h0;
	r02 = (elliptical) ? in[3] * in[4] : in[2] * in[2];
	h0  = (elliptical) ? in[5] : in[3];
	A = M_PI * r02 * h0 / (2.0 * (1 - f*f));
	V0 = M_PI * r02 * h0  * (1 + f*f) / 2.0;
	phi = pow (1.0 - V0 * (1.0 - v)/A, 0.25);
	return (phi);
}

GMT_LOCAL double grdseamount_gauss_solver (double in[], double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction */
	int n = 0;
	double A, B, V0, phi, phi0, r02, h0;
	r02 = (elliptical) ? in[3] * in[4] : in[2] * in[2];
	h0  = (elliptical) ? in[5] : in[3];
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

#define DELTA_PHI 0.01
GMT_LOCAL double grdseamount_poly_solver (double in[], double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction from a polynomial seamount */
	double I1 = TWO_PI * (M_PI * sqrt(3.0) / 3.0 - 1.7);	/* I(1) definite integral */
	double b = (1.0 - v) * (M_PI * f * f * poly_smt_func (f) - poly_smt_vol (f)) - v * I1;
	double t = 0.0, phi = 0.0, lhs = 0.0, last_lhs;
	gmt_M_unused (in);
	gmt_M_unused (elliptical);
	while (lhs >= b) {
    	t += DELTA_PHI;
    	last_lhs = lhs;
    	lhs = M_PI * t * t * poly_smt_func (t) - poly_smt_vol (t);
    }
	if (lhs < 0.0)
		phi = (t - DELTA_PHI) + DELTA_PHI * (b - last_lhs) / (lhs - last_lhs);	/* Linear interpolation */
	return (phi);
}

GMT_LOCAL int grdseamount_parse_the_record (struct GMT_CTRL *GMT, struct GRDSEAMOUNT_CTRL *Ctrl, double **data, char **text, uint64_t rec, uint64_t n_expected, bool map, double inv_scale, double *in, char *code) {
	uint64_t col;
	int n;
	double s_scale;
	char txt_x[GMT_LEN64], txt_y[GMT_LEN64], m[GMT_LEN16], s_unit;
	gmt_M_unused (GMT);

	/* There are two scenarios under which there is trailing text (and text is not NULL):
	 * 1. User gave times with units, e.g., 50M or 10k, so there are two words with time info
	 * 2. User gave -C so the model code g,c,p,d is the last input word. */

	for (col = 0; col < n_expected; col++) in[col] = data[col][rec];	/* Just copy over this numerical part of the record */
	if (Ctrl->T.active && text) {	/* Force start and stop times to be multiples of the increment */
		n = sscanf (text[rec], "%s %s %s", txt_x, txt_y, m);
		if (n == 1)	/* Only shape code given as trailing text */
			*code = txt_x[0];
		else if (n >= 2) {	/* Model time given, and possibly shape code */
			in[n_expected-2] = gmt_get_modeltime (txt_x, &s_unit, &s_scale);
			in[n_expected-1] = gmt_get_modeltime (txt_y, &s_unit, &s_scale);
			if (n == 3) *code = m[0];	/* Also gave shape code */
		}
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

EXTERN_MSC int GMT_grdseamount (void *V_API, int mode, void *args) {
	int error, scol, srow, scol_0, srow_0;

	unsigned int n_out, nx1, d_mode, row, col, row_0, col_0, rmode, inc_mode;
	unsigned int max_d_col, d_row, *d_col = NULL, t, t_use, build_mode, t0_col = 0, t1_col = 0;

	uint64_t n_expected_fields, n_smts = 0, tbl, seg, rec, ij;

	bool map = false, periodic = false, replicate, first, empty, exact, exact_increments, cone_increments;

	char unit, code, unit_name[8], file[PATH_MAX] = {""}, time_fmt[GMT_LEN64] = {""};

	gmt_grdfloat *data = NULL, *current = NULL, *previous = NULL;

	double x, y, r, c, in[9], this_r, A = 0.0, B = 0.0, C = 0.0, e, e2, ca, sa, ca2, sa2, r_in, dx, dy, dV, scale = 1.0;
	double add, f, max, r_km, amplitude, h_scale = 0.0, z_assign, noise = 0.0, this_user_time = 0.0, life_span, t_mid;
	double r_mean, h_mean, wesn[4], rr, out[12], a, b, area, volume, height, DEG_PR_KM = 0.0, v_curr, v_prev, *V = NULL;
	double fwd_scale, inv_scale = 0.0, inch_to_unit, unit_to_inch, prev_user_time = 0.0, h_curr = 0.0, h_prev = 0.0, h0, pf;
	double *V_sum = NULL, *h_sum = NULL, *h = NULL, g_noise = exp (-4.5), g_scl = 1.0 / (1.0 - g_noise), phi_prev, phi_curr;
	void (*shape_func[N_SHAPES]) (double a, double b, double h, double hc, double f, double *A, double *V, double *z);
	double (*phi_solver[N_SHAPES]) (double in[], double f, double v, bool elliptical);

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

	code = Ctrl->C.code;
	/* Specify expected columns */
	n_expected_fields = ((Ctrl->E.active) ? 6 : 4) + ((Ctrl->F.mode == TRUNC_FILE) ? 1 : 0);
	if (Ctrl->T.active) n_expected_fields += 2;	/* The two cols with start and stop time */
	rmode = (Ctrl->C.input) ? GMT_COL_FIX : GMT_COL_FIX_NO_TEXT;
	if ((error = GMT_Set_Columns (API, GMT_IN, (unsigned int)n_expected_fields, rmode)) != GMT_NOERROR) {
		Return (error);
	}

	/* Register likely data sources unless the caller has already done so */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}

	/* Assign functions to shape_func and phi_solver array of functions */
	shape_func[SHAPE_CONE] = grdseamount_cone_area_volume_height;
	phi_solver[SHAPE_CONE] = grdseamount_cone_solver;
	shape_func[SHAPE_DISC] = grdseamount_disc_area_volume_height;
	phi_solver[SHAPE_DISC] = grdseamount_disc_solver;
	shape_func[SHAPE_PARA] = grdseamount_para_area_volume_height;
	phi_solver[SHAPE_PARA] = grdseamount_para_solver;
	shape_func[SHAPE_GAUS] = grdseamount_gaussian_area_volume_height;
	phi_solver[SHAPE_GAUS] = grdseamount_gauss_solver;
	shape_func[SHAPE_POLY] = grdseamount_poly_area_volume_height;
	phi_solver[SHAPE_POLY] = grdseamount_poly_solver;

	build_mode = inc_mode = Ctrl->C.mode;
	cone_increments = (Ctrl->T.active && Ctrl->Q.disc);
	if (cone_increments) inc_mode = SHAPE_DISC;

	map = gmt_M_is_geographic (GMT, GMT_IN);
	if (map) {	/* Geographic data */
		DEG_PR_KM = 1.0 / GMT->current.proj.DIST_KM_PR_DEG;
		d_mode = 2, unit = 'k';	/* Select km and great-circle distances */
	}
	else {	/* Cartesian scaling */
		unsigned int s_unit;
		int is;
		if ((is = gmt_check_scalingopt (GMT, 'D', Ctrl->D.unit, unit_name)) == -1) {
			Return (GMT_PARSE_ERROR);
		}
		else
			s_unit = (unsigned int)is;
		/* We only need inv_scale here which scales input data in these units to m */
		if ((error = gmt_init_scales (GMT, s_unit, &fwd_scale, &inv_scale, &inch_to_unit, &unit_to_inch, unit_name))) {
			Return (error);
		}
		d_mode = 0, unit = 'X';	/* Select Cartesian distances */
	}
	if (gmt_init_distaz (GMT, unit, d_mode, GMT_MAP_DIST) == GMT_NOT_A_VALID_TYPE)
		Return (GMT_NOT_A_VALID_TYPE);
	V = gmt_M_memory (GMT, NULL, D->n_records, double);	/* Allocate volume array */
	V_sum = gmt_M_memory (GMT, NULL, D->n_records, double);	/* Allocate volume array */
	h_sum = gmt_M_memory (GMT, NULL, D->n_records, double);	/* Allocate volume array */
	h = gmt_M_memory (GMT, NULL, D->n_records, double);	/* Allocate volume array */
	if (inc_mode == SHAPE_GAUS)
		noise = g_noise;		/* Normalized height of a unit Gaussian at basal radius; we must subtract this to truly get 0 at r = rbase */

	if (Ctrl->L.active) {	/* Just list area, volume, etc. for each seamount; no grid needed */
		n_out = (unsigned int)n_expected_fields + 3;
		if ((error = GMT_Set_Columns (API, GMT_OUT, n_out, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
			goto wrap_up;
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR)	/* Registers default output destination, unless already set */
			goto wrap_up;
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR)	/* Enables data output and sets access mode */
			goto wrap_up;
	}

	/* 0. DETERMINE THE NUMBER OF TIME STEPS */

	if (Ctrl->T.active) {	/* Have requested a time series of bathymetry */
		t0_col = (unsigned int)(n_expected_fields - 2);
		t1_col = (unsigned int)(n_expected_fields - 1);
	}

	if (Ctrl->M.active) {	/* Must create dataset to hold names of all output grids */
		uint64_t dim[GMT_DIM_SIZE] = {1, 1, Ctrl->T.n_times, 1};
		unsigned int k, j;
		if ((L = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_WITH_STRINGS, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_INFORMATION, "Failure while creating text set for file %s\n", Ctrl->M.file);
			API->error = GMT_RUNTIME_ERROR;
			goto wrap_up;
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
				if (grdseamount_parse_the_record (GMT, Ctrl, S->data, S->text, rec, n_expected_fields, map, inv_scale, in, &code)) continue;
				for (col = 0; col < n_expected_fields; col++) out[col] = in[col];	/* Copy of record before any scalings */
				if (Ctrl->C.input) {
					noise = 0.0;
					switch (code) {
						case 'c': build_mode = SHAPE_CONE; break;
						case 'd': build_mode = SHAPE_DISC; break;
						case 'o': build_mode = SHAPE_POLY; break;
						case 'p': build_mode = SHAPE_PARA; break;
						case 'g': build_mode = SHAPE_GAUS;
							noise = g_noise;	/* Normalized height of a unit Gaussian at basal radius; we must subtract this to truly get 0 at r = rbase */
							break;
						default:
							GMT_Report (API, GMT_MSG_ERROR, "Unrecognized shape code %c\n", code);
							API->error = GMT_RUNTIME_ERROR;
							goto wrap_up;
					}
				}
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
				if (Ctrl->F.mode == TRUNC_FILE && (Ctrl->F.value < 0.0 || Ctrl->F.value >= 1.0)) {
					GMT_Report (API, GMT_MSG_ERROR, "Flattening outside valid range 0-1 (%g)!\n", Ctrl->F.value);
					API->error = GMT_RUNTIME_ERROR;
					goto wrap_up;
				}
				c = (map) ? cosd (in[GMT_Y]) : 1.0;	/* Flat Earth scaling factor */
				/* Compute area, volume, mean amplitude */
				shape_func[build_mode] (a, b, amplitude, Ctrl->L.value, Ctrl->F.value, &area, &volume, &height);
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
				GMT_Report (API, GMT_MSG_INFORMATION, "Seamount %" PRIu64 " [%c] area, volume, mean height: %g %g %g\n", n_smts, code, area, volume, height);
			}
		}
	}
	gmt_M_free (GMT, Out);
	if (Ctrl->L.active) {	/* OK, that was all we wanted */
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR)	/* Disables further data output */
			goto wrap_up;
		goto wrap_up;
	}

	/* Set up and allocate output grid */
	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL,
	                             GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) {
		gmt_M_free (GMT, h_sum);	gmt_M_free (GMT, V);		gmt_M_free (GMT, V_sum);	gmt_M_free (GMT, h);
		Return (API->error);
	}

	gmt_set_xy_domain (GMT, wesn, Grid->header);	/* May include some padding if gridline-registered */
	nx1 = Grid->header->n_columns + Grid->header->registration - 1;
	if (map && gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI])) periodic = true;
	replicate = (periodic && Grid->header->registration == GMT_GRID_NODE_REG);
	if (Ctrl->A.active) for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] = Ctrl->A.value[GMT_OUT];
	if (Ctrl->Z.active) {	/* Start with the background depth */
		GMT_Report (API, GMT_MSG_INFORMATION, "Set the background level to %g\r", Ctrl->Z.value);
		for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] = (gmt_grdfloat)Ctrl->Z.value;
	}
	exact = (Ctrl->T.active && !Ctrl->Q.disc);
	exact_increments = (exact && Ctrl->Q.bmode == SMT_INCREMENTAL);
	data = gmt_M_memory (GMT, NULL, Grid->header->size, gmt_grdfloat);	/* tmp */
	if (exact) current  = gmt_M_memory (GMT, NULL, Grid->header->size, gmt_grdfloat);	/* current time solution */
	if (exact_increments) previous = gmt_M_memory (GMT, NULL, Grid->header->size, gmt_grdfloat);	/* previous time solution */

	for (t = t_use = 0; t < Ctrl->T.n_times; t++) {	/* For each time step (or just once) */

		/* 1. SET THE CURRENT TIME VALUE (IF USED) */
		if (Ctrl->T.active) {	/* Set the current time in user units as well as years */
			this_user_time = Ctrl->T.time[t].value;	/* In years */
			GMT_Report (API, GMT_MSG_INFORMATION, "Evaluating bathymetry for time %g %s\n", Ctrl->T.time[t].value * Ctrl->T.time[t].scale, gmt_modeltime_unit (Ctrl->T.time[t].u));
		}
		if (Ctrl->Q.bmode == SMT_INCREMENTAL || exact) gmt_M_memset (Grid->data, Grid->header->size, gmt_grdfloat);	/* Wipe clean for next increment */
		max = -DBL_MAX;
		empty = true;	/* So far, no seamounts have left a contribution */

		/* 2. VISIT ALL SEAMOUNTS */
		for (tbl = n_smts = 0; tbl < D->n_tables; tbl++) {
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
				S = D->table[tbl]->segment[seg];	/* Set shortcut to current segment */
				for (rec = 0; rec < S->n_rows; rec++,  n_smts++) {
					if (grdseamount_parse_the_record (GMT, Ctrl, S->data, S->text, rec, n_expected_fields, map, inv_scale, in, &code)) continue;

					if (Ctrl->T.active) {	/* Check if outside time-range */
						if (this_user_time >= in[t0_col]) continue;	/* Not started growing yet */
						if (this_user_time < in[t1_col] && !exact) continue;	/* Completed so making no contribution to incremental discs */
					}
					if (gmt_M_y_is_outside (GMT, in[GMT_Y],  wesn[YLO], wesn[YHI])) continue;	/* Outside y-range */
					if (gmt_x_is_outside (GMT, &in[GMT_X], wesn[XLO], wesn[XHI])) continue;	/* Outside x-range */

					if (Ctrl->C.input) {
						noise = 0.0;
						switch (code) {
							case 'c': build_mode = SHAPE_CONE; break;
							case 'd': build_mode = SHAPE_DISC; break;
							case 'p': build_mode = SHAPE_PARA; break;
							case 'o': build_mode = SHAPE_POLY; break;
							case 'g': build_mode = SHAPE_GAUS;
								noise = g_noise;	/* Normalized height of a unit Gaussian at basal radius; we must subtract this to truly get 0 at r = rbase */
								break;
							default:
								GMT_Report (API, GMT_MSG_ERROR, "Unrecognized shape code %c\n", code);
								API->error = GMT_RUNTIME_ERROR;
								goto wrap_up;
						}
						if (!cone_increments) inc_mode = build_mode;
					}

					/* Ok, we are inside the region - process data */
					GMT_Report (API, GMT_MSG_INFORMATION, "Evaluate seamount # %6d [%c]\n", n_smts, code);

					if (Ctrl->T.active) {	/* Must compute volume fractions v_curr, v_prev of an evolving seamount */
						life_span = in[t0_col] - in[t1_col];	/* Total life span of this seamount */
						if (Ctrl->Q.fmode == FLUX_GAUSSIAN) {	/* Gaussian volume flux */
							if (t == 0) prev_user_time = DBL_MAX;
							t_mid = 0.5 * (in[t0_col] + in[t1_col]);	/* time at mid point in evolution */
							v_curr = 0.5 * (1.0 + erf (-6.0 * (this_user_time - t_mid) / (M_SQRT2 * life_span)));	/* Normalized volume fraction at end of this time step */
							v_prev = 0.5 * (1.0 + erf (-6.0 * (prev_user_time - t_mid) / (M_SQRT2 * life_span)));	/* Normalized volume fraction at start of this time step */
							if (v_prev < 0.0015) v_prev = 0.0;	/* Deal with the 3-sigma truncation, i.e., throw first tail in with first slice */
							if (v_curr > 0.9985) v_curr = 1.0;	/* Deal with the 3-sigma truncation, i.e., throw last tail in with last slice */
						}
						else {	/* Linear volume flux */
							if (t == 0) prev_user_time = in[t0_col];
							v_curr = (in[t0_col] - this_user_time) / life_span;	/* Normalized volume fraction at end of this time step */
							v_prev = (in[t0_col] - prev_user_time) / life_span;	/* Normalized volume fraction at start of this time step */
						}
						/* Since we don't skip seamounts after they have completed when we want cumulative heights, we add a few further checks */
						if (v_curr > 1.0) v_curr = 1.0;	/* When we pass the end-life of the volcano we stop at 1 since the volcano is now fully grown */
						if (v_prev > 1.0) v_prev = 1.0;	/* When we pass the end-life of the volcano we stop at 1 since the volcano is now fully grown */
						/* When v_curr == v_prev after a volcano has stopped growing the incremental change will be zero */
						dV = V[n_smts] * (v_curr - v_prev);	/* Incremental volume produced */
						V_sum[n_smts] += dV;			/* Keep track of volume sum so we can compare with truth later */
						if (Ctrl->F.mode == TRUNC_FILE) {
							f = (Ctrl->E.active) ? in[6] : in[4];
							if (Ctrl->F.mode == TRUNC_FILE && (f < 0.0 || f >= 1.0)) {
								GMT_Report (API, GMT_MSG_ERROR, "Flattening outside valid range 0-1 (%g)!\n", f);
								API->error = GMT_RUNTIME_ERROR;
								goto wrap_up;
							}
						}
						else
							f = Ctrl->F.value;
						if (Ctrl->Q.disc) {	/* Obtain the equivalent disc parameters given the volumes found */
							phi_curr = phi_solver[build_mode] (in, f, v_curr, Ctrl->E.active);
							phi_prev = phi_solver[build_mode] (in, f, v_prev, Ctrl->E.active);
							h0 = (Ctrl->E.active) ? in[5] : in[3];
							switch (build_mode) {	/* Given the phi values, evaluate the corresponding heights */
								case SHAPE_CONE:  h_curr = h0 * (1 - phi_curr) / (1 - f); h_prev = h0 * (1 - phi_prev) / (1 - f); break;
								case SHAPE_PARA:  h_curr = h0 * (1 - phi_curr * phi_curr) / (1 - f * f); h_prev = h0 * (1 - phi_prev * phi_prev) / (1 - f * f); break;
								case SHAPE_GAUS:  h_curr = h0 * exp (4.5 * (f*f - phi_curr * phi_curr)); h_prev = h0 * exp (4.5 * (f*f - phi_prev * phi_prev)); break;
								case SHAPE_POLY:  pf = poly_smt_func (f); h_curr = h0 * poly_smt_func (phi_curr) / pf; h_prev = h0 * poly_smt_func (phi_prev) / pf;	break;
								//case SHAPE_POLY:  pf = poly_smt_func (f); h_curr = MIN (h0, h0 * poly_smt_func (phi_curr) / pf); h_prev = MIN (h0, h0 * poly_smt_func (phi_prev) / pf);	break;
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
							scale = 1.0;
						}
						else	/* Here we want the exact surface at the reduced radius and height given by the scale related to the volume fraction */
							scale = pow (v_curr, ONETHIRD);
					}

					scol_0 = (int)gmt_M_grd_x_to_col (GMT, in[GMT_X], Grid->header);	/* Center column */
					if (scol_0 < 0) continue;	/* Still outside x-range */
					if ((col_0 = scol_0) >= Grid->header->n_columns) continue;	/* Still outside x-range */
					srow_0 = (int)gmt_M_grd_y_to_row (GMT, in[GMT_Y], Grid->header);	/* Center row */
					if (srow_0 < 0) continue;	/* Still outside y-range */
					if ((row_0 = srow_0) >= Grid->header->n_rows) continue;	/* Still outside y-range */

					c = (map) ? cosd (in[GMT_Y]) : 1.0;	/* Flat Earth area correction */

					if (Ctrl->E.active) {	/* Elliptical seamount parameters */
						sincos ((90.0 - in[GMT_Z]) * D2R, &sa, &ca);	/* in[GMT_Z] is azimuth in degrees, convert to direction, get sin/cos */
						a = scale * in[3];			/* Semi-major axis */
						b = scale * in[4];			/* Semi-minor axis */
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
						amplitude = scale * in[5];		/* Seamount max height from base */
						if (Ctrl->F.mode == TRUNC_FILE) Ctrl->F.value = in[6];	/* Flattening given by input file */
					}
					else {	/* Circular features */
						r_in = a = b = scale * in[GMT_Z];	/* Radius in user units */
						r_km = r_in * Ctrl->S.value;	/* Scaled up by user scale */
						r = r_km;			/* Copy of r_km */
						if (map) r *= DEG_PR_KM;	/* Was in km so now it is in degrees, same units as grid coordinates */
						f = (inc_mode == SHAPE_CONE) ? 1.0 / r_km : -4.5 / (r_km * r_km);	/* So we can take exp (f * radius_in_km^2) */
						amplitude = scale * in[3];		/* Seamount max height from base */
						if (Ctrl->F.mode == TRUNC_FILE) Ctrl->F.value = in[4];	/* Flattening given by input file */
					}
					switch (inc_mode) {	/* This adjusts hight scaling for truncated features. If f = 0 then h_scale == 1 */
						case SHAPE_CONE:  h_scale = 1.0 / (1.0 - Ctrl->F.value); break;
						case SHAPE_DISC:  h_scale = 1.0; break;
						case SHAPE_PARA:  h_scale = 1.0 / (1.0 - Ctrl->F.value * Ctrl->F.value); break;
						case SHAPE_POLY:  h_scale = 1.0 / poly_smt_func (Ctrl->F.value); break;
						case SHAPE_GAUS:  h_scale = 1.0 / exp (-4.5 * Ctrl->F.value * Ctrl->F.value); break;
					}
					if (inc_mode == SHAPE_GAUS) h_scale *= g_scl;	/* Adjust for the fact we only go to -/+ 3 sigma and not infinity */
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
#if 0
							if (t == (Ctrl->T.n_times-1) && doubleAlmostEqualZero (this_r, 0.0)) {
								dx = 0.0;	/* Break point here if debugging */
							}
#endif
							if (Ctrl->E.active) {	/* For elliptical bases we must deal with direction etc */
								dx = (map) ? (x - in[GMT_X]) * GMT->current.proj.DIST_KM_PR_DEG * c : (x - in[GMT_X]);
								dy = (map) ? (y - in[GMT_Y]) * GMT->current.proj.DIST_KM_PR_DEG : (y - in[GMT_Y]);
								this_r = A * dx * dx + 2.0 * B * dx * dy + C * dy * dy;
								/* this_r is now r^2 in the 0 to -4.5 range expected for the Gaussian case */
								rr = sqrt (-this_r/4.5);	/* Convert this r^2 to a normalized radius 0-1 inside cone */
								if (Ctrl->A.active && rr > 1.0) continue;	/* Beyond the seamount base so nothing to do for a mask */
								if (inc_mode == SHAPE_CONE) {	/* Elliptical cone case */
									if (rr < 1.0)	/* Since in minor direction rr may exceed 1 and be outside the ellipse */
										add = (rr < Ctrl->F.value) ? 1.0 : (1.0 - rr) * h_scale;
									else
										add = 0.0;
								}
								else if (inc_mode == SHAPE_DISC)	/* Elliptical disc/plateau case */
									add = (rr <= 1.0) ? 1.0 : 0.0;
								else if (inc_mode == SHAPE_PARA)	/* Elliptical parabolic case */
									add = (rr < Ctrl->F.value) ? 1.0 : (1.0 - rr*rr) * h_scale;
								else if (inc_mode == SHAPE_POLY)	/* Elliptical parabolic case */
									add = (rr < Ctrl->F.value) ? 1.0 : poly_smt_func (rr) * h_scale;
								else	/* Elliptical Gaussian case */
									add = (rr < Ctrl->F.value) ? 1.0 : exp (this_r) * h_scale - noise;
							}
							else {	/* Circular features are simpler */
								rr = this_r / r_km;	/* Now in 0-1 range */
								if (inc_mode == SHAPE_CONE)	/* Circular cone case */
									add = (rr < Ctrl->F.value) ? 1.0 : (1.0 - rr) * h_scale;
								else if (inc_mode == SHAPE_DISC)	/* Circular disc/plateau case */
									add = (rr <= 1.0) ? 1.0 : 0.0;
								else if (inc_mode == SHAPE_PARA)	/* Circular parabolic case */
									add = (rr < Ctrl->F.value) ? 1.0 : (1.0 - rr*rr) * h_scale;
								else if (inc_mode == SHAPE_POLY)	/* Circular parabolic case */
									add = (rr < Ctrl->F.value) ? 1.0 : poly_smt_func (rr) * h_scale;
								else	/* Circular Gaussian case */
									add = (rr < Ctrl->F.value) ? 1.0 : exp (f * this_r * this_r) * h_scale - noise;
							}
							if (add <= 0.0) continue;	/* No amplitude, so skip */
							ij = gmt_M_ijp (Grid->header, row, col);	/* Current node location */
							z_assign = amplitude * add;		/* height to be added to grid */
							if (Ctrl->A.active)	/* No amplitude, just set inside value for mask */
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
							empty = false;	/* We have made a change to this grid */
						}
					}
				}
			}
			prev_user_time = this_user_time;	/* Make this the previous time */
		}
		if (empty && Ctrl->T.active) {	/* No contribution made */
			GMT_Report (API, GMT_MSG_INFORMATION, "No contribution made for time %g %s\n",
			            Ctrl->T.time[t].value * Ctrl->T.time[t].scale, gmt_modeltime_unit (Ctrl->T.time[t].u));
			if (exact && !exact_increments) gmt_M_memcpy (Grid->data, current, Grid->header->size, float);	/* Nothing new added so same cumulative surface as last step */
		}

		/* Time to write the grid */
		if (Ctrl->T.active)
			gmt_modeltime_name (GMT, file, Ctrl->G.file, &(Ctrl->T.time[t]));
		else
			strcpy (file, Ctrl->G.file);
		if (Ctrl->M.active) {	/* Write list of grids to file */
			char record[GMT_BUFSIZ] = {""}, tmp[GMT_LEN64] = {""};
			if (Ctrl->T.active) {
				sprintf (record, "%s\t", file);
				sprintf (tmp, time_fmt, Ctrl->T.time[t].value * Ctrl->T.time[t].scale, Ctrl->T.time[t].unit);
				strcat (record, tmp);
			}
			else
				strcpy (record, file);
			L->table[0]->segment[0]->data[GMT_X][t_use] = Ctrl->T.time[t].value;
			L->table[0]->segment[0]->text[t_use++] = strdup (record);
			L->table[0]->segment[0]->n_rows++;
		}

		if (Ctrl->N.active) {	/* Normalize so max height == N.value */
			double n_scl = Ctrl->N.value / max;
			GMT_Report (API, GMT_MSG_INFORMATION, "Normalize seamount amplitude so max height is %g\r", Ctrl->N.value);
			for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] *= (gmt_grdfloat)n_scl;
		}
		if (exact) gmt_M_memcpy (current, Grid->data, Grid->header->size, float);	/* Store solution for the current time */

		if (exact_increments) {	/* Need to subtract current solution from the previous solution */
			for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] -= previous[ij];	/* Get increments relative to previous time */
			if (t < (Ctrl->T.n_times-1)) gmt_M_memcpy (previous, current, Grid->header->size, float);	/* Now update previous */
		}

		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid))
			goto wrap_up;
		gmt_M_memcpy (data, Grid->data, Grid->header->size, gmt_grdfloat);	/* This will go away once gmt_nc.c is fixed to leave array alone */
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, Grid) != GMT_NOERROR)
			goto wrap_up;
		gmt_M_memcpy (Grid->data, data, Grid->header->size, gmt_grdfloat);
	}
	if (Ctrl->M.active) L->table[0]->n_records = L->table[0]->segment[0]->n_rows = t_use;
	if (Ctrl->M.active && GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_WRITE_NORMAL, NULL, Ctrl->M.file, L) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_ERROR, "Failure while writing list of grid files to %s\n", Ctrl->M.file);
		goto wrap_up;
	}

wrap_up:

	gmt_M_free (GMT, current);	gmt_M_free (GMT, previous);
	gmt_M_free (GMT, d_col);	gmt_M_free (GMT, V);		gmt_M_free (GMT, h);
	gmt_M_free (GMT, V_sum);	gmt_M_free (GMT, h_sum);	gmt_M_free (GMT, data);

	Return (API->error);
}
