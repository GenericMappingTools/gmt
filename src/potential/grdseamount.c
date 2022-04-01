/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * seamounts that can be Gaussian, Conical, Parabolic, Polynomial or Disc, with
 * or without truncated tops (not for discs, obviously, as already truncated).
 * Optional variable density grid can be written as well.  If time information
 * is provided we can also produce grids for each time step that shows either
 * the cumulative relief up until this time or just the incremental relief
 * for each time step, such as needed for time-dependent flexure. These estimates
 * can be either exact or approximated via constant-thickness discs. Seamounts
 * can use different models so you can mix and match cones and Gaussians, for example.
 * The density option allows output of vertically-averaged density grids.
 * With -S we can also allow for flank collapse of seamounts via parameters that
 * specify one or more slides per seamount that can occur at different times.
 * This option can be used to study the isostatic rebound of seamounts and islands
 * after such mass redistributions.
 *
 * */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdseamount"
#define THIS_MODULE_MODERN_NAME	"grdseamount"
#define THIS_MODULE_LIB		"potential"
#define THIS_MODULE_PURPOSE	"Create synthetic seamounts (Gaussian, parabolic, polynomial, cone or disc; circular or elliptical)"
#define THIS_MODULE_KEYS	"<T{,GG},LD),MD),TD("
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVbdefhir"

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
#define ONE_THIRD 0.333333333333333333333333333333333333333333333333333333333333

#define N_MAX_SLIDES	5	/* Max number of slides for any given seamount */
#define N_MAX_COLS		59	/* Max possible number of input columns (7 + 2 +5*10) */

struct GMT_MODELTIME {	/* Hold info about modeling time */
	double value;	/* Time in year */
	double scale;	/* Scale factor from year to given user unit */
	char unit;	/* Either M (Myr), k (kyr), or blank (implies y) */
	unsigned int u;	/* For labeling: Either 0 (yr), 1 (kyr), or 2 (Myr) */

};

struct SLIDE {	/* Hold information for one slide */
	/* Input parameters (some have defaults): */
	double h1, h2;		/* Two escarpment heights, with h2 > h1 */
	double hc;			/* Height of deposit at toe of seamount [h1/2] */
	double az1, az2;	/* Azimuthal range of the slide [0/360] */
	double u0;			/* Radial slide shape parameter */
	double t0, t1;		/* Start and end time of slide */
	double p;			/* Azimuthal power parameter */
	double phi;			/* Desired volume fraction (%) of the whole */
	/* Computed parameters from h1, h2, hc and Vs based on shape: */
	double r1, r2;		/* The corresponding radii for h1, h2 */
	double rc;			/* Radius to start of distal deposit */
	double rd;			/* Radius to end of distal deposit */
	double noise;
};

struct SEAMOUNT {	/* HOld information for one seamount */
	double lon, lat, height;
	double radius;	/* If circular */
	double azimuth, major, minor;	/* If elliptical */
	double f;		/* Flattening */
	double t0, t1;	/* Time span */
	double noise;	/* Only nonzero for Gaussian model */
	unsigned int n_slides;	/* How many land slides for this seamount */
	unsigned int build_mode;	/* Which seamount shape */
	struct SLIDE Slide[N_MAX_SLIDES];	/* Information about n_slides land slides */
	char code;
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
	struct GRDSEAMOUNT_H {	/* -H<H>/<rho_l>/<rho_h>[+d<densify>][+p<power>] */
		bool active;
		double H, rho_l, rho_h;
		double p, densify;
		double del_rho;	/* Computed as rho_h - rho_l */
		double p1;	/* Will be p + 1 to avoid addition in loops */
	} H;
	struct GRDSEAMOUNT_I {	/* -I (for checking only) */
		bool active;
	} I;
	struct GRDSEAMOUNT_L {	/* -L[<hn>] */
		bool active;
		unsigned int mode;
		double value;
	} L;
	struct GRDSEAMOUNT_K {	/* -K<densmodel> */
		bool active;
		char *file;
	} K;
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
	struct GRDSEAMOUNT_S {	/* -S[<h1>/<h2>][+a[<az1/az2>]][+d[<hc>]][+p[<pow>]][+t[<t0/t1>]][+u[<u0>]][+v[<phi>]] */
		bool active;
		bool slide;
		bool got_h;		/* True if <h1>/<h2> were provided */
		bool got_a;		/* True if +a was set */
		bool got_d;		/* True if +d was set */
		bool got_p;		/* True if +p was set */
		bool got_t;		/* True if +t was set */
		bool got_u;		/* True if +u was set */
		bool got_v;		/* True if +u was set */
		bool read_h;	/* True if we must read h1 h2 from file instead */
		bool read_a;	/* True if +a took no arguments and we read from file instead */
		bool read_d;	/* True if +d took no arguments and we read from file instead */
		bool read_p;	/* True if +p took no arguments and we read from file instead */
		bool read_t;	/* True if +t took no arguments and we read from file instead */
		bool read_u;	/* True if +u took no arguments and we read from file instead */
		bool read_v;	/* True if +v took no arguments and we read from file instead */
		unsigned int n;	/* Number of slides per seamount */
		double value;	/* Deprecated -S<r_scale> */
		struct SLIDE L;	/* Holds command-line settings only */
	} S;
	struct GRDSEAMOUNT_T {	/* -T[l]<t0>/<t1>/<d0>|n  */
		bool active, log;
		unsigned int n_times;
		struct GMT_MODELTIME *time;	/* The current sequence of times */
	} T;
	struct GRDSEAMOUNT_W {	/* -W<avedensgrid> */
		bool active;
		char *file;
	} W;
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
	C->S.L.az2 = 120.0;
	C->S.L.u0 = 0.2;
	C->S.n = 1;
	C->T.n_times = 1;
	C->H.p = 1.0;	/* Linear density increase */
	C->H.densify = 0.0;	/* No water-driven compaction on flanks */

	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDSEAMOUNT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->K.file);
	gmt_M_str_free (C->M.file);
	gmt_M_free (GMT, C->T.time);
	gmt_M_str_free (C->W.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] -G%s %s %s [-A[<out>/<in>]] [-C[c|d|g|o|p]] [-D%s] "
		"[-E] [-F[<flattening>]] [-H<H>/<rho_l>/<rho_h>[+d<densify>][+p<power>]] [-K<densmodel>] "
		"[-L[<hn>]] [-M[<list>]] [-N<norm>] [-Q<bmode><fmode>[+d]] [-S[<h1>/<h2>][+[a<az1/az2>]][+d[<hc>]][+p[<pow>]][+t[<t0/t1>]][+u[<u0>]][+v[<phi>]] [-T<t0>[/<t1>/<dt>|<file>|<n>[+l]]] "
		"[%s] [-W%s] [-Z<base>] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_OUTGRID, GMT_I_OPT, GMT_Rgeo_OPT, GMT_LEN_UNITS2_DISPLAY, GMT_V_OPT, GMT_OUTGRID, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT,
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
	GMT_Usage (API, -2, "Build a mAsk grid instead; append outside/inside values [1/NaN]. "
		"Here, height is ignored and -L, -N, -Q, -T and -Z are disallowed.");
	GMT_Usage (API, 1, "\n-C[c|d|g|o|p]");
	GMT_Usage (API, -2, "Choose between a variety of shapes [Default is Gaussian]:");
	GMT_Usage (API, 3, "c: Cone radial shape.");
	GMT_Usage (API, 3, "d: Disc radial shape.");
	GMT_Usage (API, 3, "g: Gaussian radial shape.");
	GMT_Usage (API, 3, "o: Polynomial radial shape.");
	GMT_Usage (API, 3, "p: Parabolic radial shape.");
	GMT_Usage (API, -2, "Note: If -C is given without argument then we expect to read c, d, g, o or p from the trailing input text.");
	GMT_Usage (API, 1, "\n-D%s", GMT_LEN_UNITS2_DISPLAY);
	GMT_Usage (API, -2, "Specify horizontal distance unit used by input file if -fg is not used.  Choose among "
		"e (meter), f (foot) k (km), M (mile), n (nautical mile), or u (survey foot) [e].");
	GMT_Usage (API, 1, "\n-E Elliptical data format [Default is Circular]. "
		"Read lon, lat, azimuth, major, minor, height (m) for each seamount.");
	GMT_Usage (API, 1, "\n-F[<flattening>]");
	GMT_Usage (API, -2, "Seamounts are truncated. Append flattening or expect it in an extra input column [no truncation]. ");
	GMT_Usage (API, 1, "\n-H<H>/<rho_l>/<rho_h>[+d<densify>][+p<power>]");
	GMT_Usage (API, -2, "Set parameters for the reference seamount height (in m), flank and deep core densities (kg/m^3 or g/cm^3). "
		"Control the density function by these modifiers:");
	GMT_Usage (API, 3, "+d Density increase (kg/m^3 or g/cm^3) due to water pressure over full depth implied by <H> [0].");
	GMT_Usage (API, 3, "+p Exponential <power> coefficient (> 0) for density change with burial depth [1 (linear)].");
	GMT_Usage (API, 1, "\n-K<densmodel>");
	GMT_Usage (API, -2, "Write a 2-D crossection showing variable seamount densities [no grid written].");
	GMT_Usage (API, 1, "\n-L[<hn>]");
	GMT_Usage (API, -2, "List area, volume, and mean height for each seamount; No grid is created so -R -I are not required. "
		"Optionally, append the noise-floor cutoff level <hn> [0].");
	GMT_Usage (API, 1, "\n-M[<list>]");
	GMT_Usage (API, -2, "Give filename for output table with names of all grids produced. "
		"If no filename is given then we write the list to standard output.");
	GMT_Usage (API, 1, "\n-N<norm>");
	GMT_Usage (API, -2, "Normalize grid so maximum grid height equals <norm>. Not allowed with -T.");
	GMT_Usage (API, 1, "\n-Q<bmode><fmode>[+d]");
	GMT_Usage (API, -2, "Only used in conjunction with -T. Append the two modes:");
	GMT_Usage (API, 3, "<bmode>: Build either (c)umulative [Default] or (i)ncremental volume through time.");
	GMT_Usage (API, 3, "<fmode>: Assume a (g)aussian [Default] or (c)onstant volume flux distribution.");
	GMT_Usage (API, -2, "Append +d to build grids with increments as uniform discs [Default gives exact shapes].");
	GMT_Usage (API, 1, "\n-S[<h1>/<h2>][+[a<az1/az2>]][+d[<hc>]][+p[<pow>]][+t[<t0/t1>]][+u[<u0>]][+v[<phi>]]");
	GMT_Usage (API, -2, "Control how a sectoral landslide should look like.  Append the height range affected (if not we read from file) and select optional modifiers. "
		"If no argument is given to a modifier it means we will read that parameter from the input file (order is alphabetical):");
	GMT_Usage (API, 3, "+a Set the azimuthal sector range [0/360].");
	GMT_Usage (API, 3, "+d Set the height of the start of the distal distributed deposit [h1/2].");
	GMT_Usage (API, 3, "+p Turn on azimuthal height variation and set power coefficient >= 2.");
	GMT_Usage (API, 3, "+t Set time range over which the slide will develop linearly.");
	GMT_Usage (API, 3, "+u Set positive slide parameter <u0> to affect slide profile [0.1].");
	GMT_Usage (API, 3, "+v Set desired volume fraction <phi> of the slide relative to the entire seamount (in %). Note: This will compute <u0>");
	GMT_Usage (API, 1, "\n-T<t0>[/<t1>/<dt>[+l]]|<file>");
	GMT_Usage (API, -2, "Specify start, stop, and time increments for sequence of calculations [one step, no time dependency]. "
		"For a single specific time, just give <t0> (in years; append k for kyr and M for Myr).");
	GMT_Usage (API, 3, "+l For a logarithmic time scale, interpret <dt> as <n_steps> instead of time increment.");
	GMT_Usage (API, -2, "Alternatively, read a list of times from the first column in a file by appending <tfile>. "
		"Note: This option implies two extra input columns with <start> and <stop> time for each seamount's life span. "
		"Use -Q to select cumulative versus incremental construction.");
	GMT_Option (API, "V");
	gmt_outgrid_syntax (API, 'W', "Filename for output grid with vertically averaged seamount densities. If -T is set then <outgrid> "
		"must be a filename template that contains a floating point format (C syntax) and "
		"we use the corresponding time (in units specified in -T) to generate the file names.");
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
			case 'H':	/* Reference seamount density parameters */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->H.active);
				Ctrl->H.active = true;
				if ((c = gmt_first_modifier (GMT, opt->arg, "dp"))) {
					unsigned int pos = 0;
					char txt[GMT_LEN256] = {""};
					while (gmt_getmodopt (GMT, 'H', c, "dp", &pos, txt, &n_errors) && n_errors == 0) {
						switch (txt[0]) {
							case 'd':	/* Get densify rate over reference water depth H */
								Ctrl->H.densify = atof (&txt[1]);
								if (Ctrl->H.densify < 10.0) Ctrl->H.densify *= 1000;	/* Gave units of g/cm^3 */
								break;
							case 'p':	/* Get power coefficient */
								Ctrl->H.p = atof (&txt[1]);
								break;
							default:
								n_errors++;
								break;
						}
					}
					c[0] = '\0';	/* Chop off all modifiers so range can be determined */
				}
				if (sscanf (opt->arg, "%lg/%lg/%lg", &Ctrl->H.H, &Ctrl->H.rho_l, &Ctrl->H.rho_h) != 3) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -H: Unable to parse the three values\n");
					n_errors++;
				}
				if (Ctrl->H.rho_l < 10.0) Ctrl->H.rho_l *= 1000;	/* Gave units of g/cm^3 */
				if (Ctrl->H.rho_h < 10.0) Ctrl->H.rho_h *= 1000;	/* Gave units of g/cm^3 */
				Ctrl->H.del_rho = Ctrl->H.rho_h - Ctrl->H.rho_l;
				Ctrl->H.p1 = Ctrl->H.p + 1;
				break;
			case 'I':	/* Grid spacing */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'K':	/* Output file name for density grid */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->K.active);
				Ctrl->K.active = true;
				if (opt->arg[0]) {
					Ctrl->K.file = strdup (opt->arg);
					if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->K.file))) n_errors++;
				}
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -K: No file name appended\n");
					n_errors++;
				}
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
				Ctrl->S.active = true;
				if (strchr (opt->arg, '+')) {	/* Slide settings */
					n_errors += gmt_M_repeated_module_option (API, Ctrl->S.slide);
					Ctrl->S.slide = true;
					if ((c = gmt_first_modifier (GMT, opt->arg, "adhptuv"))) {
						unsigned int pos = 0;
						char txt[GMT_LEN256] = {""};
						while (gmt_getmodopt (GMT, 'S', c, "adhptuv", &pos, txt, &n_errors) && n_errors == 0) {
							switch (txt[0]) {
								case 'a':	/* Get Azimuth band for slide */
									Ctrl->S.got_a = true;
									if (txt[1] == '\0')	/* Read them from file */
										Ctrl->S.read_a = true;
									else if (sscanf (&txt[1], "%lg/%lg", &Ctrl->S.L.az1, &Ctrl->S.L.az2) != 2) {
										GMT_Report (API, GMT_MSG_ERROR, "Option -S: Unable to parse the two azimuth values in +a\n");
										n_errors++;
									}
									break;
								case 'd':	/* Get distal start height */
									Ctrl->S.got_d = true;
									if (txt[1] == '\0')	/* Read it from file */
										Ctrl->S.read_d = true;
									else
										Ctrl->S.L.hc = atof (&txt[1]);
									break;
								case 'h':	/* Get escarpment heights for slide */
									Ctrl->S.got_h = true;
									if (txt[1] == '\0')	/* Read them from file */
										Ctrl->S.read_h = true;
									else if (sscanf (&txt[1], "%lg/%lg", &Ctrl->S.L.h1, &Ctrl->S.L.h2) != 2) {
										GMT_Report (API, GMT_MSG_ERROR, "Option -S: Unable to parse the two height values for slide in +h\n");
										n_errors++;
									}
									break;
								case 'p':	/* Get azimuthal power coefficient and turn on azimuthal variation */
									Ctrl->S.got_p = true;
									if (txt[1] == '\0')	/* Read it from file */
										Ctrl->S.read_p = true;
									else
										Ctrl->S.L.p = atof (&txt[1]);
									break;
								case 't':	/* Get time-window for slide */
									Ctrl->S.got_t = true;
									if (txt[1] == '\0')	/* Read them from file */
										Ctrl->S.read_t = true;
									else if (sscanf (&txt[1], "%lg/%lg", &Ctrl->S.L.t0, &Ctrl->S.L.t1) != 2) {
										GMT_Report (API, GMT_MSG_ERROR, "Option -S: Unable to parse the two time values in +t\n");
										n_errors++;
									}
									break;
								case 'u':	/* Get initial normalized offset u0 */
									Ctrl->S.got_u = true;
									if (txt[1] == '\0')	/* Read it from file */
										Ctrl->S.read_u = true;
									else
										Ctrl->S.L.u0 = atof (&txt[1]);
									break;
								case 'v':	/* Get volume percent of slide */
									Ctrl->S.got_v = true;
									if (txt[1] == '\0')	/* Read it from file */
										Ctrl->S.read_v = true;
									else
										Ctrl->S.L.phi = atof (&txt[1]);
									break;
								default:
									n_errors++;
									break;
							}
						}
						c[0] = '\0';	/* Chop off all modifiers so range can be determined */
					}
					if (!Ctrl->S.got_a)	/* Did not set +a, default to 0-360 (as initialized( */
						Ctrl->S.got_a = true;
					if (Ctrl->S.got_h && !Ctrl->S.got_d) {	/* Set the default value for hc */
						Ctrl->S.L.hc = 0.5 * Ctrl->S.L.h1;
						Ctrl->S.got_d = true;	/* Since we just set it */
					}
					if (!Ctrl->S.got_u)	/* Did not set +u, default to 0.1 (as initialized( */
						Ctrl->S.got_u = true;
					if (c) c[0] = '+';	/* Restore modifiers */
				}
				else {	/* Deprecated radial scaling */
					n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
					Ctrl->S.value = atof (opt->arg);
				}
				break;
			case 'T':	/* Time grid */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				Ctrl->T.active = true;
				if ((Ctrl->T.n_times = gmt_modeltime_array (GMT, opt->arg, &Ctrl->T.log, &Ctrl->T.time)) == 0)
					n_errors++;
				break;
			case 'W':	/* Output file name for density cube */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				if (opt->arg[0]) {
					Ctrl->W.file = strdup (opt->arg);
					if (GMT_Get_FilePath (API, GMT_IS_CUBE, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->W.file))) n_errors++;
				}
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -W: No file name appended\n");
					n_errors++;
				}
				break;
			case 'Z':	/* Background relief level */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.active = true;
				Ctrl->Z.value = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->C.mode == SHAPE_DISC && Ctrl->F.active, "Cannot specify -F for discs; ignored\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && (Ctrl->N.active || Ctrl->Z.active || Ctrl->L.active || Ctrl->T.active), "Option -A: Cannot use -L, -N, -T or -Z with -A\n");
	if (!Ctrl->L.active) {	/* Just a listing, cannot use -R -I -r -G -M -Z */
		if (!Ctrl->K.active) {
			n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
			n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Option -I: Must specify positive increment(s)\n");
			n_errors += gmt_M_check_condition (GMT, !(Ctrl->G.active || Ctrl->G.file), "Option -G: Must specify output file or template\n");
		}
		n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && !strchr (Ctrl->G.file, '%'), "Option -G: Filename template must contain format specifier when -T is used\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->Q.bmode == SMT_INCREMENTAL, "Option -Z: Cannot be used with -Qi\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && !Ctrl->T.active, "Option -M: Requires time information via -T\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->F.mode == TRUNC_ARG && (Ctrl->F.value < 0.0 || Ctrl->F.value >= 1.0), "Option -F: Flattening must be in 0-1 range\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->K.active && !Ctrl->H.active, "Option -K: Requires density model function via -H\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && !Ctrl->H.active, "Option -W: Requires density model function via -H\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->H.active && Ctrl->H.p <= 0.0, "Option -H: Exponent must be positive\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->H.active && Ctrl->H.densify < 0.0, "Option -H: Densify value must be positive or zero\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->H.active && Ctrl->H.rho_l > Ctrl->H.rho_h, "Option -H: Low density cannot exceed the high density\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->H.active && Ctrl->H.H <= 0.0, "Option -H: Reference seamount height must be positive\n");
		n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && Ctrl->C.input, "Option -C: Cannot read from trailing text if binary input is selected\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.got_p && Ctrl->S.L.p < 2.0, "Option -S: Azimuthal power coefficient set with +p must be >= 2\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.L.phi < 0.0 || Ctrl->S.L.phi >= 100.0, "Option -S: Volume fraction must be less than 100%%\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.got_h && Ctrl->S.L.h1 > Ctrl->S.L.h2 , "Option -S: Scarp height h2 must exceed h1\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.got_d && Ctrl->S.got_h && Ctrl->S.L.hc > Ctrl->S.L.h1, "Option -S: Distal slump height hc cannot exceed h1\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.got_v && Ctrl->S.got_u, "Option -S: Cannot set +u if +v is also set\n");
		if (GMT->common.b.active[GMT_IN] && Ctrl->T.active)
			GMT_Report (API, GMT_MSG_WARNING, "Option -T: Seamount start end times via binary input area assumed to be in years\n");
	}
	n_expected_fields = ((Ctrl->E.active) ? 6 : 4) + ((Ctrl->F.mode == TRUNC_FILE) ? 1 : 0);
	if (Ctrl->T.active) n_expected_fields += 2;	/* The two cols with start and stop time would be numerical if binary input */
	n_errors += gmt_check_binary_io (GMT, n_expected_fields);
	if (Ctrl->C.mode == SHAPE_DISC && Ctrl->F.active) {Ctrl->F.active = false; Ctrl->F.mode = 0; Ctrl->F.value = 0.0;}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void grdseamount_disc_area_volume_height (double a, double b, double h, double hc, double f, double *A, double *V, double *z) {
	/* Compute area and volume of circular or elliptical disc "seamounts" (more like plateaus).
	 * Here, f is not used. */

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
	return ((r <= -1.0 || r > 1.0) ? 0.0 : pow ((1.0 + r) * (1.0 - r), 3.0) / (1.0 + pow (r, 3.0)));
}

GMT_LOCAL double ddr_poly_smt_func (double r) {
	/* Radial derivative of polynomial radial function similar to a Gaussian */
	double r2 = r * r;
	return ((fabs(r) > 1.0) ? 0.0 : -(3.0 * r * pow (r - 1.0, 2.0) * (r2 * r + r + 2.0)) / pow (r2 - r + 1.0, 2.0));
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

GMT_LOCAL double grdseamount_r_from_h (unsigned int inc_mode, double h, double r0, double h0, double f) {
	/* Return the radius where height is h */
	double r;
	switch (inc_mode) {	/* This adjusts hight scaling for truncated features. If f = 0 then h_scale == 1 */
		case SHAPE_CONE: r = (1.0 - (h * (1.0 - f)/h0)); break;
		case SHAPE_DISC: r = 1.0; break;
		case SHAPE_PARA: r = sqrt (1.0 - (h * (1.0 - f)/h0)); break;
		case SHAPE_POLY: r = poly_smt_rc (h * poly_smt_func (f) /h0); break;
		case SHAPE_GAUS: r = sqrt (f * f - 2.0 * log (h/h0) / 9.0); break;
	}
	return (r * r0);
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
GMT_LOCAL double grdseamount_cone_solver (struct SEAMOUNT *S, double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction */
	double A, V0, phi, r02, h0;
	r02 = (elliptical) ? S->major * S->minor : S->radius * S->radius;
	h0  = S->height;
	A = M_PI * r02 * h0 / (3.0 * (1 - f));
	V0 = M_PI * r02 * h0  * (1 + f + f*f) / 3.0;
	phi = pow (1.0 - V0 * (1.0 - v) / A, ONE_THIRD);
	return (phi);
}

GMT_LOCAL double grdseamount_disc_solver (struct SEAMOUNT *S, double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction fro a disc is trivial */
	gmt_M_unused (S), gmt_M_unused (f), gmt_M_unused (elliptical);
	return (v);
}

GMT_LOCAL double grdseamount_para_solver (struct SEAMOUNT *S, double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction */
	double A, V0, phi, r02, h0;
	r02 = (elliptical) ? S->major * S->minor : S->radius * S->radius;
	h0  = S->height;
	A = M_PI * r02 * h0 / (2.0 * (1 - f*f));
	V0 = M_PI * r02 * h0  * (1 + f*f) / 2.0;
	phi = pow (1.0 - V0 * (1.0 - v)/A, 0.25);
	return (phi);
}

GMT_LOCAL double grdseamount_gauss_solver (struct SEAMOUNT *S, double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction */
	int n = 0;
	double A, B, V0, phi, phi0, r02, h0;
	r02 = (elliptical) ? S->major * S->minor : S->radius * S->radius;
	h0  = S->height;
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
GMT_LOCAL double grdseamount_poly_solver (struct SEAMOUNT *S, double f, double v, bool elliptical) {
	/* Return effective phi given volume fraction from a polynomial seamount */
	double I1 = TWO_PI * (M_PI * sqrt(3.0) / 3.0 - 1.7);	/* I(1) definite integral */
	double b = (1.0 - v) * (M_PI * f * f * poly_smt_func (f) - poly_smt_vol (f)) - v * I1;
	double t = 0.0, phi = 0.0, lhs = 0.0, last_lhs;
	gmt_M_unused (S);
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

GMT_LOCAL double grdseamount_density (struct GRDSEAMOUNT_CTRL *Ctrl, double z, double h) {
	/* Passing in the current seamount's normalized height, h(r) and the current normalized radial point z(r).
	 * We return the density at this point inside the seamount with reference height 1 */
	double u = (h - z);	/* Depth into seamount. Note: h and z are both in the 0-1 range here */
	double q = (1.0 - z);	/* Water depth to flank point */

	double rho = Ctrl->H.rho_l + Ctrl->H.densify * q + Ctrl->H.del_rho * pow (u, Ctrl->H.p);
	return (rho);
}

GMT_LOCAL double grdseamount_mean_density (struct GRDSEAMOUNT_CTRL *Ctrl, double h, double z1, double z2) {
	/* Passing in the current seamounts height h(r) and the two depths z1(r) and z2(r) defining a layer.
	 * When doing the whole seamount we pass z2 = 0 and z1 = h(r).
	 * The vertically averaged density for this radial position is returned */
	double u1 = (h - z1) / Ctrl->H.H;
	double u2 = (h - z2) / Ctrl->H.H;
	double q = (Ctrl->H.H - h) / Ctrl->H.H;
	double dz = z2 - z1;

	double rho = Ctrl->H.rho_l + Ctrl->H.densify * q + Ctrl->H.del_rho * Ctrl->H.H * (pow (u1, Ctrl->H.p1) - pow (u2, Ctrl->H.p1)) / (dz * (Ctrl->H.p1));
	return (rho);
}

GMT_LOCAL bool grdseamount_in_sector (struct GMT_CTRL *GMT, struct GRDSEAMOUNT_CTRL *Ctrl, struct SLIDE *S, struct GMT_GRID *G, double *in, unsigned int row, unsigned int col, double *s) {
	/* Return true if the azimuth from seamount center to this point is inside the wedge selected via -S+a.
	 * If inside we also compute the azimuthal scale s if +p was set */
	bool in_sector = false;
	double dx = G->x[col] - in[GMT_X], dy = G->y[row] - in[GMT_Y];
	double az = (dx == 0.0 && dy == 0.0) ? 0.0 : R2D * atan2 (dy, dx) - 360.0;	/* Make sure we start negative */
	gmt_M_unused (GMT);
	*s = 0.0;	/* Default */
	while (az < S->az1) az += 360.0;	/* Keep wrapping until we pass az1 */
	if (az <= S->az2) {	/* Inside sector, compute s */
		in_sector = true;
		if (Ctrl->S.got_p) {	/* Evaluate scale */
			double gamma = fabs (2.0 * (az - S->az1) / (S->az2 - S->az1) - 1.0);
			*s = pow (gamma, S->p);
		}
	}

	return (in_sector);
}

GMT_LOCAL double grdseamount_slide (struct GMT_CTRL *GMT, struct SLIDE *S, double r_km, double r, double hf, double s) {
	/* r is normalized radial position (0-1), and input hf and output height h are as well.
	 * s is the azimuthal scale which is 0 if no azimuth variation was requested. */
	double u, this_r = r * r_km, q, hs, h;
	gmt_M_unused (GMT);
	if (this_r < S->r2) return hf;	/* Before slide range, return the regular height */
	if (this_r > S->rd) return hf;	/* Beyond distal range, return the regular height  (likely zero) */
	if (this_r > S->rc) {	/* In the distal range for redeposit of slide material with thickness h_d(r) */
		h = S->hc * (1.0 - (this_r - S->rc) / (S->rd - S->rc));
		return (h);
	}
	if (this_r > S->r1) return hf;	/* Between foot of slide and toe of deposit, return the regular height */
	/* Here we are within the slide radial range */
	u = ((this_r - S->r2) / (S->r1 - S->r2));	/* Normalized radial u inside the slide range of 0-1 */
	q = S->u0 * ((1.0 + S->u0) / (u + S->u0) - 1.0);	/* Normalized slide shape function q(u) */
	hs = S->h1 + (S->h2 - S->h1) * q;	/* Slide height scaled to actual topography */
	h = hf * s + (1.0 - s) * hs;	/* Handle azimuthal variation (if any) by blending flank and slide heights using s(alpha) */
	return(h);
}

GMT_LOCAL double grdseamount_height (struct GRDSEAMOUNT_CTRL *Ctrl, double this_r, double rr, double h_scale, double f, double f_exp, double noise, double *orig_add) {
	/* Return the height of the seamount at this normalized radius rr */
	double add;
	if (Ctrl->C.mode == SHAPE_CONE) {	/* Circular cone case */
		if (rr < 1.0) {	/* Since in minor direction rr may exceed 1 and be outside the ellipse */
			*orig_add = (1.0 - rr) * h_scale;
			add = (rr < f) ? 1.0 : *orig_add;
		}
		else
			*orig_add = add = 0.0;
	}
	else if (Ctrl->C.mode == SHAPE_DISC)	/* Circular disc/plateau case */
		*orig_add = add = (rr <= 1.0) ? 1.0 : 0.0;
	else if (Ctrl->C.mode == SHAPE_PARA) {	/* Circular parabolic case */
		*orig_add = (1.0 - rr*rr) * h_scale;
		add = (rr < f) ? 1.0 : *orig_add;
	}
	else if (Ctrl->C.mode == SHAPE_POLY) {	/* Circular parabolic case */
		*orig_add = poly_smt_func (rr) * h_scale;
		add = (rr < f) ? 1.0 : *orig_add;
	}
	else {	/* Circular Gaussian case */
		*orig_add = exp (f_exp * this_r * this_r) * h_scale - noise;
		add = (rr < f) ? 1.0 : *orig_add;
	}
	return (add);
}

/* The derivations behind these expressions are in sandbox/gurudocs/slides.tex */

GMT_LOCAL double grdseamount_cone_pappas (double r0, double h0, double f, double r1, double r2) {
	/* Compute flank crossectional area and centroid distance from axis for a cone model and return volume via Pappas */
	double K, Af, rf, uf, u1 = r1 / r0, u2 = r2 / r0;
	K = u1 - u2 - 0.5 * (pow (u1, 2.0) - pow (u2, 2.0));
	uf = (3.0 * (pow (u1, 2.0) - pow (u2, 2.0)) - 2.0 * (pow (u1, 3.0) - pow (u2, 3.0))) / (6.0 * K);
	Af = K * h0 * r0 / (1 - f);
	rf = r0 * uf;
	return (TWO_PI * Af * rf);
}

GMT_LOCAL double grdseamount_para_pappas (double r0, double h0, double f, double r1, double r2) {
	/* Compute flank crossectional area and centroid distance from axis for a parabolic model and return volume via Pappas */
	double K, Af, rf, uf, u1 = r1 / r0, u2 = r2 / r0;
	K = u1 - u2 - (pow (u1, 3.0) - pow (u2, 3.0)) / 3.0;
	uf = (2.0 * (pow (u1, 2.0) - pow (u2, 2.0)) - (pow (u1, 4.0) - pow (u2, 4.0))) / (4.0 * K);
	Af = K * h0 * r0 / (1 - f*f);
	rf = r0 * uf;
	return (TWO_PI * Af * rf);
}

GMT_LOCAL double grdseamount_gauss_pappas (double r0, double h0, double f, double r1, double r2) {
	/* Compute flank crossectional area and centroid distance from axis for a Gaussian model and return volume via Pappas */
	double K, Af, rf, uf, u1 = r1 / r0, u2 = r2 / r0, c = 3.0 * sqrt (2) / 2.0;
	K = sqrt (TWO_PI) * (erf (c * u1) - erf (c * u2)) / 6.0;
	uf = (exp (-4.5 * pow (u2, 2.0)) - exp (-4.5 * pow (u1, 2.0))) / (9.0 * K);
	Af = K * h0 * r0 * exp (4.5 * f * f);
	rf = r0 * uf;
	return (TWO_PI * Af * rf);
}

GMT_LOCAL double grdseamount_poly_pappas (double r0, double h0, double f, double r1, double r2) {
	/* Compute flank crossectional area and centroid distance from axis for a polynomial model and return volume via Pappas */
	double K, Af, rf, uf, u1 = r1 / r0, u2 = r2 / r0, c = sqrt (3) / 3.0, L, T;
	L = 1.5 * log ((pow (u1, 2.0) - u1 + 1) / (pow (u2, 2.0) - u2 + 1));
	T = sqrt (3) * (atan (c * (2 * u1 - 1)) - atan (c * (2 * u2 - 1)));
	K = u1 - u2 + 1.5 * (pow (u1, 2.0) - pow (u2, 2.0)) - 0.25 * (pow (u1, 4.0) - pow (u2, 4.0)) - L - T;
	uf = (-3.0 * (u1 - u2) + 0.5 * (pow (u1, 2.0) - pow (u2, 2.0)) + (pow (u1, 3.0) - pow (u2, 3.0)) - 0.2 * (pow (u1, 5.0) - pow (u2, 5.0)) - L + T) / K;
	Af = K * h0 * r0 / poly_smt_func (f);
	rf = r0 * uf;
	return (TWO_PI * Af * rf);
}

GMT_LOCAL double ubar (double u0) {
	/* Compute mean u_s^u via equation (6) */
	double u0_1 = 1.0 + u0;
	return (2.0 * u0_1 * (1.0 - u0 * log (u0_1 / u0)) - 1.0) / (u0_1 * log (u0_1 / u0) - 1.0);
}

GMT_LOCAL double grdseamount_pappas_slide (struct GRDSEAMOUNT_CTRL *Ctrl, double u0) {
	/* Compute flank crossectional area and centroid distance from axis for the slide model and return volume via Pappas.
	 * Here dh = h2 - h1. We split solution into upper (u) and lower (l) portions */
	double dr = Ctrl->S.L.r1 - Ctrl->S.L.r2, dh = Ctrl->S.L.h2 - Ctrl->S.L.h1, u0_1 = 1.0 + u0, As_u, rs_u, As_l, rs_l;
	As_u = dh * dr * u0 * (u0_1 * log (u0_1 / u0) - 1.0);
	rs_u = Ctrl->S.L.r2 + dr * ubar (u0);
	As_l = dr * Ctrl->S.L.h1;	/* Area of lower pedestal */
	rs_l = 0.5 * (Ctrl->S.L.r1 + Ctrl->S.L.r2);
	return (TWO_PI * (As_u * rs_u + As_l * rs_l));
}

GMT_LOCAL double grdseamount_distal_r (struct GRDSEAMOUNT_CTRL *Ctrl, double r0, double Vs) {
	/* Return the radial distance to the end of the distal slide deposit */
	double c = 3.0 * Vs / (M_PI * Ctrl->S.L.hc) + r0 * (r0 + Ctrl->S.L.rc);
	double rd = 0.5 * (sqrt (Ctrl->S.L.rc*Ctrl->S.L.rc + 4.0 * c) - Ctrl->S.L.rc);
	return (rd);
}

GMT_LOCAL double grdseamount_slide_u0 (struct GMT_CTRL *GMT, struct GRDSEAMOUNT_CTRL *Ctrl, double Vf, double V0) {
	/* Determine the tuning parameter u0 that yields the specified slide fraction */

	double dr = Ctrl->S.L.r1 - Ctrl->S.L.r2, dh = Ctrl->S.L.h2 - Ctrl->S.L.h1;
	double rhs = 0.5 * ((Vf - Ctrl->S.L.phi * V0) / (M_PI * dr) - Ctrl->S.L.h1 * (Ctrl->S.L.r1 + Ctrl->S.L.r2)) / dh;
	double u0_next, u0_1, du, u0 = 0.1;	/* Trial value */
	int n_iter = 0;
	gmt_M_unused (GMT);

	/* Iterate on equation (16) to solve for optimal u0 */
	do {
		u0_1 = 1.0 + u0;
		u0_next = rhs / ((Ctrl->S.L.r2 + dr * ubar (u0)) * (u0_1 * log (u0_1 / u0) - 1.0));
		du = fabs (u0_next - u0);
		n_iter++;
	} while (du > GMT_CONV12_LIMIT && n_iter < 50);
	return (u0_next);
}

struct SEAMOUNT *grdseamount_read_input (struct GMTAPI_CTRL *API, struct GRDSEAMOUNT_CTRL *Ctrl, struct GMT_OPTION *options, uint64_t *n_smt) {
	char txt_x[GMT_LEN64], txt_y[GMT_LEN64], m[GMT_LEN16], s_unit;
	uint64_t n = 0;
	int n_fields;
	unsigned int n_expected, n_time, n_per_slide = 0, f_col, k, col;
	double s_scale, g_noise = exp (-4.5), *in = NULL;
	size_t n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;
	struct SEAMOUNT *S = NULL;
	struct GMT_RECORD *In = NULL;

	/* Specify expected columns */
	n_expected = ((Ctrl->E.active) ? 6 : 4) + ((Ctrl->F.mode == TRUNC_FILE) ? 1 : 0);

	if (Ctrl->S.slide) {	/* Need to read extra groups of columns per slide */
		if (Ctrl->S.read_a) n_per_slide += 2;	/* +a: Read az1 and az2 */
		if (Ctrl->S.read_d) n_per_slide += 1;	/* +d: Read hc */
		if (Ctrl->S.read_h) n_per_slide += 2;	/* +h: Read h1 and h2 */
		if (Ctrl->S.read_p) n_per_slide += 1;	/* +p: Read p */
		if (Ctrl->S.read_t) n_per_slide += 2;	/* +t: Read t0 and t1 */
		if (Ctrl->S.read_u) n_per_slide += 1;	/* +u: Read u0 */
		if (Ctrl->S.read_v) n_per_slide += 1;	/* +v: Read phi */
	}

	if (GMT_Set_Columns (API, GMT_IN, 0, GMT_COL_VAR) != GMT_NOERROR) {
		return (NULL);
	}

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
		return (NULL);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {		/* Enables data input and sets access mode */
		return (NULL);
	}

	S = gmt_M_memory (API->GMT, NULL, n_alloc, struct SEAMOUNT);

	f_col = (Ctrl->E.active) ? 6 : 4;

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, &n_fields)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (API->GMT)) {		/* Bail if there are any read errors */
				return (NULL);
			}
			else if (gmt_M_rec_is_eof (API->GMT)) 		/* Reached end of file */
				break;
			else if (gmt_M_rec_is_any_header (API->GMT))			/* Skip segment and table headers */
				continue;
		}
		in = In->data;
		S[n].lon = in[GMT_X];	S[n].lat = in[GMT_Y];
		if (Ctrl->E.active) {	/* Elliptical parameters */
			S[n].azimuth = in[GMT_Z];
			S[n].major = in[3];
			S[n].minor = in[4];
			S[n].height = in[5];
		}
		else {	/* Circular parameters */
			S[n].radius = in[GMT_Z];
			S[n].height = in[3];
		}
		S[n].f = (Ctrl->F.mode == TRUNC_FILE) ? in[f_col] : Ctrl->F.value;
		if (Ctrl->F.mode == TRUNC_FILE && (S[n].f < 0.0 || S[n].f >= 1.0)) {
			GMT_Report (API, GMT_MSG_ERROR, "Flattening outside valid range 0-1 (%g)!\n", S[n].f);
			API->error = GMT_RUNTIME_ERROR;
			gmt_M_free (API->GMT, S);
			return NULL;
		}
		n_time = 0;	/* Number of numerical columns used for time */
		S[n].code = Ctrl->C.code;
		if (In->text) {	/* Have information passed via trailing text */
			int ns = sscanf (In->text, "%s %s %s", txt_x, txt_y, m);
			if (Ctrl->T.active) {	/* Force start and stop times to be multiples of the increment */
				if (ns == 1)	{	/* Only shape code given as trailing text */
					if (Ctrl->C.input) S[n].code = txt_x[0];
					S[n].t0 = in[n_expected];
					S[n].t1 = in[n_expected+1];
					n_time = 2;
				}
				else if (ns >= 2) {	/* Model time given, and possibly shape code */
					S[n].t0 = gmt_get_modeltime (txt_x, &s_unit, &s_scale);
					S[n].t1 = gmt_get_modeltime (txt_y, &s_unit, &s_scale);
					if (Ctrl->C.input) S[n].code = m[0];
				}
			}
			else if (Ctrl->C.input) 	/* Only shape code given as trailing text */
				S[n].code = txt_x[0];
		}
		switch (S[n].code) {
			case 'c': S[n].build_mode = SHAPE_CONE; break;
			case 'd': S[n].build_mode = SHAPE_DISC; break;
			case 'o': S[n].build_mode = SHAPE_POLY; break;
			case 'p': S[n].build_mode = SHAPE_PARA; break;
			case 'g': S[n].build_mode = SHAPE_GAUS;
				S[n].noise = g_noise;	/* Normalized height of a unit Gaussian at basal radius; we must subtract this to truly get 0 at r = rbase */
				break;
			default:
				GMT_Report (API, GMT_MSG_ERROR, "Unrecognized shape code %c\n", S[n].code);
				API->error = GMT_RUNTIME_ERROR;
				gmt_M_free (API->GMT, S);
				return NULL;
		}

		if (Ctrl->S.slide) {	/* Determine how many slide groups */
			S[n].n_slides = (n_fields - n_expected) / n_per_slide;
			col = n_expected + n_time;	/* Start of slide columns for this record */
			for (k = 0; k < S[n].n_slides; k++) {
				if (Ctrl->S.read_a) {	/* Read the azimuths from file */
					S[n].Slide[k].az1 = in[col++];
					S[n].Slide[k].az2 = in[col++];
				}
				else {	/* Get fixed azimuths from -S */
					S[n].Slide[k].az1 = Ctrl->S.L.az1;
					S[n].Slide[k].az2 = Ctrl->S.L.az2;					
				}
				if (Ctrl->S.read_d)	/* Read hc from file */
					S[n].Slide[k].hc = in[col++];
				else	/* Get fixed hc from -S */
					S[n].Slide[k].hc = Ctrl->S.L.hc;
				if (Ctrl->S.read_h) {	/* Read the heights from file */
					S[n].Slide[k].h1 = in[col++];
					S[n].Slide[k].h2 = in[col++];
				}
				else {	/* Get fixed azimuths from -S */
					S[n].Slide[k].h1 = Ctrl->S.L.h1;
					S[n].Slide[k].h2 = Ctrl->S.L.h2;					
				}
				if (Ctrl->S.read_p)	/* Read p from file */
					S[n].Slide[k].p = in[col++];
				else	/* Get fixed p from -S */
					S[n].Slide[k].p = Ctrl->S.L.p;
				if (Ctrl->S.read_t) {	/* Read the slide times from file */
					S[n].Slide[k].t0 = in[col++];
					S[n].Slide[k].t1 = in[col++];
				}
				else {	/* Get fixed azimuths from -S */
					S[n].Slide[k].t0 = Ctrl->S.L.t0;
					S[n].Slide[k].t1 = Ctrl->S.L.t1;					
				}
				if (Ctrl->S.read_u)	/* Read u0 from file */
					S[n].Slide[k].u0 = in[col++];
				else	/* Get fixed u0 from -S */
					S[n].Slide[k].u0 = Ctrl->S.L.u0;
				if (Ctrl->S.read_v)	/* Read phi from file */
					S[n].Slide[k].phi = in[col++];
				else	/* Get fixed phi from -S */
					S[n].Slide[k].phi = Ctrl->S.L.phi;
			}
		}
		n++;	/* One more seamount ingested */
		if (n == n_alloc) {
			n_alloc <<= 1;	/* Double it */
			S = gmt_M_memory (API->GMT, S, n_alloc, struct SEAMOUNT);
		}
	} while (true);
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		gmt_M_free (API->GMT, S);
		return NULL;
	}

	S = gmt_M_memory (API->GMT, S, n, struct SEAMOUNT);
	*n_smt = n;

	return (S);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grdseamount (void *V_API, int mode, void *args) {
	int scol, srow, scol_0, srow_0, error;

	openmp_int row, col, max_d_col, d_row, nx1, *d_col = NULL;

	unsigned int n_out, d_mode, inc_mode, k, t, t_use, build_mode;

	uint64_t ij, n_smts, smt;

	bool map = false, periodic = false, replicate, first, empty, exact, exact_increments, cone_increments;

	char unit, unit_name[8], gfile[PATH_MAX] = {""}, wfile[PATH_MAX] = {""}, time_fmt[GMT_LEN64] = {""};

	gmt_grdfloat *data = NULL, *current = NULL, *previous = NULL, *rho_weight = NULL, *prev_z = NULL;

	double x, y, c, in[60], this_r, A = 0.0, B = 0.0, C = 0.0, e, e2, ca, sa, ca2, sa2, r_in, dx, dy, dV, scale_curr = 1.0;
	double add, f, max, r_km, amplitude = 0.0, h_scale = 0.0, z_assign, lon, this_user_time = 0.0, life_span, t_mid, rho;
	double r_mean, h_mean, wesn[4], rr, out[12], a, b, area, volume, height, v_curr, v_prev, *V = NULL;
	double fwd_scale, inv_scale = 0.0, inch_to_unit, unit_to_inch, prev_user_time = 0.0, h_curr = 0.0, h_prev = 0.0, h0, pf;
	double *V_sum = NULL, *h_sum = NULL, *h = NULL, g_noise = exp (-4.5), g_scl = 1.0 / (1.0 - g_noise), phi_prev, phi_curr;
	double orig_add, dz, rho_z, sum_rz, sum_z, exp_f, radius, minor, major;
	double (*pappas_func[N_SHAPES]) (double r0, double h0, double f, double r1, double r2);
	double (*phi_solver[N_SHAPES]) (struct SEAMOUNT *S, double f, double v, bool elliptical);
	void (*shape_func[N_SHAPES]) (double a, double b, double h, double hc, double f, double *A, double *V, double *z);

	struct SLIDE Slide[N_MAX_SLIDES];	/* Allocate of the stack info for up to these many slides per seamount */
	struct GMT_GRID *Grid = NULL;
	struct GMT_GRID *Ave = NULL;
	struct SEAMOUNT *S = NULL;
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

	gmt_M_memset (Slide, N_MAX_SLIDES, struct SLIDE);	/* Initialize to zero */
	f = Ctrl->F.value;
	inc_mode = Ctrl->C.mode;
	if (Ctrl->K.active) {
		/* Here we create the cross-section of a normalized reference seamount that goes from
		 * -1 to +1 in x and 0-1 in y (height).  Using steps of 0.005 to yield a 401 x 201 grid
		 * with densities inside the seamount and NaN outside */
		double range[4] = {-1.0, 1.0, 0.0, 1.0}, inc[2] = {0.005, 0.005};
		struct GMT_GRID *Model = NULL;
		if ((Model = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, range, inc,
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) {
			gmt_M_free (GMT, h_sum);	gmt_M_free (GMT, V);		gmt_M_free (GMT, V_sum);	gmt_M_free (GMT, h);
			Return (API->error);
		}
		switch (inc_mode) {	/* This adjusts hight scaling for truncated features. If f = 0 then h_scale == 1 */
			case SHAPE_CONE:  h_scale = 1.0 / (1.0 - f); break;
			case SHAPE_DISC:  h_scale = 1.0; break;
			case SHAPE_PARA:  h_scale = 1.0 / (1.0 - f * f); break;
			case SHAPE_POLY:  h_scale = 1.0 / poly_smt_func (f); break;
			case SHAPE_GAUS:  h_scale = 1.0 / exp (-4.5 * f * f); break;
		}
		/* Note: We use the un-flattened height since the guyot shape happened due to erosion later */
		for (col = 0; col < Model->header->n_columns; col++) {
			rr = fabs (Model->x[col]);	/* Now in 0-1 range */
			if (inc_mode == SHAPE_CONE)	/* Circular cone case */
				add = (1.0 - rr) * h_scale;
			else if (inc_mode == SHAPE_DISC)	/* Circular disc/plateau case */
				add = (rr <= 1.0) ? 1.0 : 0.0;
			else if (inc_mode == SHAPE_PARA)	/* Circular parabolic case */
				add = (1.0 - rr*rr) * h_scale;
			else if (inc_mode == SHAPE_POLY)	/* Circular parabolic case */
				add = poly_smt_func (rr) * h_scale;
			else	/* Circular Gaussian case */
				add = exp (-4.5 * rr * rr) * h_scale - g_noise;
			for (row = 0; row < Model->header->n_rows; row++) {
				ij = gmt_M_ijp (Model->header, row, col);
				if (Model->y[row] > add)	/* Outside the seamount */
					Model->data[ij] = GMT->session.f_NaN;
				else	/* Evaluate the density at this depth */
					Model->data[ij] = (gmt_grdfloat)grdseamount_density (Ctrl, Model->y[row], add);
			}
		}

		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_WRITE_NORMAL, NULL, Ctrl->K.file, Model) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while writing seamount density model grid to %s\n", Ctrl->K.file);
			goto wrap_up;
		}
		if (!Ctrl->L.active && !Ctrl->G.active)
			Return (GMT_NOERROR);
	}

	if ((S = grdseamount_read_input (API, Ctrl, options, &n_smts)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Failure while reading seamount file\n");
		goto wrap_up;
	}

	/* Assign functions to shape_func, phi_solver and pappas_func array of function pointers */
	shape_func[SHAPE_CONE]  = grdseamount_cone_area_volume_height;
	phi_solver[SHAPE_CONE]  = grdseamount_cone_solver;
	pappas_func[SHAPE_CONE] = grdseamount_cone_pappas;
	shape_func[SHAPE_DISC]  = grdseamount_disc_area_volume_height;
	phi_solver[SHAPE_DISC]  = grdseamount_disc_solver;
	shape_func[SHAPE_PARA]  = grdseamount_para_area_volume_height;
	phi_solver[SHAPE_PARA]  = grdseamount_para_solver;
	pappas_func[SHAPE_PARA] = grdseamount_para_pappas;
	shape_func[SHAPE_GAUS]  = grdseamount_gaussian_area_volume_height;
	phi_solver[SHAPE_GAUS]  = grdseamount_gauss_solver;
	pappas_func[SHAPE_GAUS] = grdseamount_gauss_pappas;
	shape_func[SHAPE_POLY]  = grdseamount_poly_area_volume_height;
	phi_solver[SHAPE_POLY]  = grdseamount_poly_solver;
	pappas_func[SHAPE_POLY] = grdseamount_poly_pappas;

	build_mode = Ctrl->C.mode;
	cone_increments = (Ctrl->T.active && Ctrl->Q.disc);
	if (cone_increments) inc_mode = SHAPE_DISC;

	map = gmt_M_is_geographic (GMT, GMT_IN);
	if (map) {	/* Geographic data */
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
	V = gmt_M_memory (GMT, NULL, n_smts, double);	/* Allocate volume array */
	V_sum = gmt_M_memory (GMT, NULL, n_smts, double);	/* Allocate volume array */
	h_sum = gmt_M_memory (GMT, NULL, n_smts, double);	/* Allocate volume array */
	h = gmt_M_memory (GMT, NULL, n_smts, double);	/* Allocate volume array */

	if (Ctrl->L.active) {	/* Just list area, volume, etc. for each seamount; no grid needed */
		n_out = (unsigned int)3;
		if ((error = GMT_Set_Columns (API, GMT_OUT, n_out, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR)
			goto wrap_up;
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR)	/* Registers default output destination, unless already set */
			goto wrap_up;
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR)	/* Enables data output and sets access mode */
			goto wrap_up;
	}

	/* 0. DETERMINE THE NUMBER OF TIME STEPS */

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
	gmt_M_memset (in, N_MAX_COLS, double);
	for (smt = 0; smt < n_smts; smt++) {
		if (Ctrl->E.active) {	/* Elliptical seamount parameters */
			a = S[smt].major;		/* Semi-major axis */
			b = S[smt].minor;		/* Semi-minor axis */
		}
		else	/* Circular features */
			a = b = S[smt].radius;		/* Radius in m */
		c = (map) ? cosd (S[smt].lat) : 1.0;	/* Flat Earth scaling factor */
		/* Compute area, volume, mean amplitude */
		shape_func[S[smt].build_mode] (a, b, S[smt].height, Ctrl->L.value, S[smt].f, &area, &volume, &height);
		V[smt] = volume;
		h[smt] = amplitude;
		if (map) {	/* Report values in km^2, km^3, and m */
			area   *= GMT->current.proj.DIST_KM_PR_DEG * GMT->current.proj.DIST_KM_PR_DEG * c;
			volume *= GMT->current.proj.DIST_KM_PR_DEG * GMT->current.proj.DIST_KM_PR_DEG * c;
			volume *= 1.0e-3;	/* Use km^3 as unit for volume */
		}
		if (Ctrl->L.active) {	/* Only want to add back out area, volume */
			col = 0;
			out[col++] = area;
			out[col++] = volume;
			out[col++] = height;
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Seamount %" PRIu64 " [%c] area, volume, mean height: %g %g %g\n", n_smts, S[smt].code, area, volume, height);
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

	/* Set up and allocate output grid */
	if (Ctrl->W.active) {
		if ((Ave = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL,
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) {
			gmt_M_free (GMT, h_sum);	gmt_M_free (GMT, V);		gmt_M_free (GMT, V_sum);	gmt_M_free (GMT, h);
			Return (API->error);
		}
		rho_weight = gmt_M_memory (GMT, NULL, Ave->header->size, gmt_grdfloat);	/* weights for blending average densities */
	}

	gmt_set_xy_domain (GMT, wesn, Grid->header);	/* May include some padding if gridline-registered */
	nx1 = (openmp_int)Grid->header->n_columns + Grid->header->registration - 1;
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
	if (Ctrl->W.active && exact_increments) prev_z = gmt_M_memory (GMT, NULL, Grid->header->size, gmt_grdfloat);	/* Keep track of previous height */

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
		for (smt = 0; smt < n_smts; smt++) {
			if (Ctrl->T.active) {	/* Check if outside time-range */
				if (this_user_time >= S[smt].t0) continue;	/* Not started growing yet */
				if (this_user_time <  S[smt].t1 && !exact) continue;	/* Completed so making no contribution to incremental discs */
			}
			if (gmt_M_y_is_outside (GMT, S[smt].lat, wesn[YLO], wesn[YHI])) continue;	/* Outside y-range */
			lon = S[smt].lon;
			if (gmt_x_is_outside (GMT, &lon,  wesn[XLO], wesn[XHI])) continue;	/* Outside x-range */

			if (!cone_increments) inc_mode = S[smt].build_mode;
			major = S[smt].major;	minor = S[smt].minor; radius = S[smt].radius;

			/* Ok, we are inside the region - process data */
			GMT_Report (API, GMT_MSG_INFORMATION, "Evaluate seamount # %6d [%c]\n", smt, S[smt].code);

			f = S[smt].f;	/* Flattening for this seamount */
			sum_rz = sum_z = 0.0;	/* Reset counters for this seamount */
			if (Ctrl->T.active) {	/* Must compute volume fractions v_curr, v_prev of an evolving seamount */
				life_span = S[smt].t0 - S[smt].t1;	/* Total life span of this seamount */
				if (Ctrl->Q.fmode == FLUX_GAUSSIAN) {	/* Gaussian volume flux */
					if (t == 0) prev_user_time = DBL_MAX;
					t_mid  = 0.5 * (S[smt].t0 + S[smt].t1);	/* time at mid point in evolution */
					v_curr = 0.5 * (1.0 + erf (-6.0 * (this_user_time - t_mid) / (M_SQRT2 * life_span)));	/* Normalized volume fraction at end of this time step */
					v_prev = 0.5 * (1.0 + erf (-6.0 * (prev_user_time - t_mid) / (M_SQRT2 * life_span)));	/* Normalized volume fraction at start of this time step */
					if (v_prev < 0.0015) v_prev = 0.0;	/* Deal with the 3-sigma truncation, i.e., throw first tail in with first slice */
					if (v_curr > 0.9985) v_curr = 1.0;	/* Deal with the 3-sigma truncation, i.e., throw last tail in with last slice */
				}
				else {	/* Linear volume flux */
					if (t == 0) prev_user_time = S[smt].t0;
					v_curr = (S[smt].t0 - this_user_time) / life_span;	/* Normalized volume fraction at end of this time step */
					v_prev = (S[smt].t0 - prev_user_time) / life_span;	/* Normalized volume fraction at start of this time step */
				}
				/* Since we don't skip seamounts after they have completed when we want cumulative heights, we add a few further checks */
				if (v_curr > 1.0) v_curr = 1.0;	/* When we pass the end-life of the volcano we stop at 1 since the volcano is now fully grown */
				if (v_prev > 1.0) v_prev = 1.0;	/* When we pass the end-life of the volcano we stop at 1 since the volcano is now fully grown */
				/* When v_curr == v_prev after a volcano has stopped growing the incremental change will be zero */
				dV = V[smt] * (v_curr - v_prev);	/* Incremental volume produced */
				V_sum[smt] += dV;			/* Keep track of volume sum so we can compare with truth later */
				h0 = S[smt].height;
				if (Ctrl->Q.disc) {	/* Obtain the equivalent disc parameters given the volumes found */
					phi_curr = phi_solver[build_mode] (&S[smt], f, v_curr, Ctrl->E.active);
					phi_prev = phi_solver[build_mode] (&S[smt], f, v_prev, Ctrl->E.active);
					switch (build_mode) {	/* Given the phi values, evaluate the corresponding heights */
						case SHAPE_CONE:  h_curr = h0 * (1 - phi_curr) / (1 - f); h_prev = h0 * (1 - phi_prev) / (1 - f); break;
						case SHAPE_PARA:  h_curr = h0 * (1 - phi_curr * phi_curr) / (1 - f * f); h_prev = h0 * (1 - phi_prev * phi_prev) / (1 - f * f); break;
						case SHAPE_GAUS:  h_curr = h0 * exp (4.5 * (f*f - phi_curr * phi_curr)); h_prev = h0 * exp (4.5 * (f*f - phi_prev * phi_prev)); break;
						case SHAPE_POLY:  pf = poly_smt_func (f); h_curr = h0 * poly_smt_func (phi_curr) / pf; h_prev = h0 * poly_smt_func (phi_prev) / pf;	break;
						//case SHAPE_POLY:  pf = poly_smt_func (f); h_curr = MIN (h0, h0 * poly_smt_func (phi_curr) / pf); h_prev = MIN (h0, h0 * poly_smt_func (phi_prev) / pf);	break;
					}
					h_mean = fabs (h_curr - h_prev);	/* This is our disc layer thickness */
					r_mean = sqrt (dV / (M_PI * h_mean));	/* Radius given by volume and height */
					h_sum[smt] += h_mean;		/* Keep track of height sum so we can compare with truth later */

					GMT_Report (API, GMT_MSG_DEBUG, "r_mean = %.1f h_mean = %.1f v_prev = %.3f v_curr = %.3f phi_prev = %.3f phi_curr = %.3f h_prev = %.1f h_curr = %.1f V_sum = %g h_sum = %g\n",
						r_mean, h_mean, v_prev, v_curr, phi_prev, phi_curr, h_prev, h_curr, V_sum[smt], h_sum[smt]);
					/* Replace the values in the in array with these incremental values instead */
					if (Ctrl->E.active) {	/* Elliptical parameters */
						e = S[smt].major / S[smt].minor;	/* Eccentricity */
						major = r_mean / sqrt (e);	/* Since we solved for sqrt (a*b) above */
						minor = major * e;
					}
					else {	/* Circular */
						radius = r_mean;
					}
					scale_curr = 1.0;
				}
				else	/* Here we want the exact surface at the reduced radius and height given by the scale related to the volume fraction */
					scale_curr = pow (v_curr, ONE_THIRD);
			}

			scol_0 = (int)gmt_M_grd_x_to_col (GMT, lon, Grid->header);	/* Center column */
			if (scol_0 < 0) continue;	/* Still outside x-range */
			if (scol_0 >= (int)Grid->header->n_columns) continue;	/* Still outside x-range */
			srow_0 = (int)gmt_M_grd_y_to_row (GMT, S[smt].lat, Grid->header);	/* Center row */
			if (srow_0 < 0) continue;	/* Still outside y-range */
			if (srow_0 >= (int)Grid->header->n_rows) continue;	/* Still outside y-range */

			c = (map) ? cosd (S[smt].lat) : 1.0;	/* Flat Earth area correction */

			if (Ctrl->E.active) {	/* Elliptical seamount parameters */
				sincos ((90.0 - S[smt].azimuth) * D2R, &sa, &ca);	/* azimuth in degrees, convert to direction, get sin/cos */
				a = scale_curr * major;			/* Semi-major axis */
				b = scale_curr * minor;			/* Semi-minor axis */
				e = b / a;		/* Eccentricity */
				e2 = e * e;
				ca2 = ca * ca;
				sa2 = sa * sa;
				r_km = b * Ctrl->S.value;	/* Scaled semi-minor axis in user units (Cartesian or km) */
				exp_f = -4.5 / (r_km * r_km);	/* So we can take exp (f * radius_in_km^2) */
				A = exp_f * (e2 * ca2 + sa2);	/* Elliptical components A, B, C needed to evaluate radius(az) */
				B = -exp_f * (sa * ca * (1.0 - e2));
				C = exp_f * (e2 * sa2 + ca2);
				r_in = a;			/* Semi-major axis in user units (Cartesian or km)*/
				r_km = r_in * Ctrl->S.value;	/* Scaled semi-major axis in user units (Cartesian or km) */
			}
			else {	/* Circular features */
				r_in = a = b = scale_curr * radius;	/* Radius in user units */
				r_km = r_in * Ctrl->S.value;	/* Scaled up by user scale */
				exp_f = (inc_mode == SHAPE_CONE) ? 1.0 / r_km : -4.5 / (r_km * r_km);	/* So we can take exp (f * radius_in_km^2) */
			}
			amplitude = scale_curr * S[smt].height;		/* Seamount max height from base */

			switch (inc_mode) {	/* This adjusts hight scaling for truncated features. If f = 0 then h_scale == 1 */
				case SHAPE_CONE:  h_scale = 1.0 / (1.0 - f); break;
				case SHAPE_DISC:  h_scale = 1.0; break;
				case SHAPE_PARA:  h_scale = 1.0 / (1.0 - f * f); break;
				case SHAPE_POLY:  h_scale = 1.0 / poly_smt_func (f); break;
				case SHAPE_GAUS:  h_scale = 1.0 / exp (-4.5 * f * f); break;
			}
			if (inc_mode == SHAPE_GAUS) h_scale *= g_scl;	/* Adjust for the fact we only go to -/+ 3 sigma and not infinity */
			if (!(Ctrl->A.active || amplitude > 0.0)) continue;	/* No contribution from this seamount */

			if (Ctrl->S.slide) {	/* Compute the radii needed for slide calculations below. ASSUMES CIRCULAR FOR NOW */
				double Vf, Vs, Vq, u0 = Ctrl->S.L.u0;
				for (k = 0; k < S[smt].n_slides; k++) {
					S[smt].Slide[k].r1 = grdseamount_r_from_h (build_mode, S[smt].Slide[k].h1, r_in, S[smt].height, f);
					S[smt].Slide[k].r2 = grdseamount_r_from_h (build_mode, S[smt].Slide[k].h2, r_in, S[smt].height, f);
					S[smt].Slide[k].rc = grdseamount_r_from_h (build_mode, S[smt].Slide[k].hc, r_in, S[smt].height, f);
					Vf = pappas_func[build_mode] (r_in, S[smt].height, f, S[smt].Slide[k].r1, S[smt].Slide[k].r2);	/* Volume beneath flank surface for given shape */
					if (Ctrl->S.got_v) {	/* Must compute u0 to match specified slide volume (else we use given u0) */
						shape_func[build_mode] (a, b, amplitude, 0.0, f, &area, &volume, &height);	/* Get full seamount volume */
						u0 = grdseamount_slide_u0 (GMT, Ctrl, Vf, volume);	/* Corresponding u0 to use */
					}
					Vq = grdseamount_pappas_slide (Ctrl, u0);	/* Volume beneath slide surface */
					Vs = Vf - Vq;	/* Actual slide volume (over 0-360 so not specific yet) */
					S[smt].Slide[k].rd = grdseamount_distal_r (Ctrl, r_in, Vs);
					GMT_Report (API, GMT_MSG_INFORMATION, "Seamount # %d Slide %d: r2 = %lg r1 = %lg rc = %lg rd = %lg, u0 = %lg, Vs = %lg\n",
					smt, k, S[smt].Slide[k].r2, S[smt].Slide[k].r1, S[smt].Slide[k].rc, S[smt].Slide[k].rd, u0, Vs);
				}
			}

			/* Initialize local search machinery, i.e., what is the range of rows and cols we need to search */
			gmt_M_free (GMT, d_col);
			d_col = gmt_prep_nodesearch (GMT, Grid, r_km, d_mode, &d_row, &max_d_col);

			for (srow = srow_0 - (int)d_row; srow <= (srow_0 + (int)d_row); srow++) {
				if (srow < 0) continue;
				if ((row = (openmp_int)srow) >= (openmp_int)Grid->header->n_rows) continue;
				y = gmt_M_grd_row_to_y (GMT, row, Grid->header);
				first = replicate;	/* Used to help us deal with duplicate columns for grid-line registered global grids */
				for (scol = scol_0 - (int)d_col[row]; scol <= (scol_0 + (int)d_col[row]); scol++) {
					if (!periodic) {
						if (scol < 0) continue;
						if ((col = (openmp_int)scol) >= (openmp_int)Grid->header->n_columns) continue;
					}
					if (scol < 0)	/* Periodic grid: Break on through to other side! */
						col = scol + nx1;
					else if ((col = (openmp_int)scol) >= (openmp_int)Grid->header->n_columns) 	/* Periodic grid: Wrap around to other side */
						col -= nx1;
					/* "silent" else we are inside w/e */
					x = gmt_M_grd_col_to_x (GMT, col, Grid->header);
					this_r = gmt_distance (GMT, lon, S[smt].lat, x, y);	/* In Cartesian units or km (if map is true) */
					if (this_r > r_km && !Ctrl->S.slide) continue;	/* Beyond the base of the seamount */
#if 0
					if (doubleAlmostEqualZero (this_r, 0.0)) {
						dx = 0.0;	/* Break point here if debugging peak of seamount location */
					}
#endif
					/* In the following, orig_add is the height of the seamount prior to any truncation, while add is the current value (subject to truncation) */
					if (Ctrl->E.active) {	/* For elliptical bases we must deal with direction etc */
						dx = (map) ? (x - lon) * GMT->current.proj.DIST_KM_PR_DEG * c : (x - lon);
						dy = (map) ? (y - S[smt].lat) * GMT->current.proj.DIST_KM_PR_DEG : (y - S[smt].lat);
						this_r = A * dx * dx + 2.0 * B * dx * dy + C * dy * dy;
						/* this_r is now r^2 in the 0 to -4.5 range expected for the Gaussian case */
						rr = sqrt (-this_r/4.5);	/* Convert this r^2 to a normalized radius 0-1 inside cone */
						if (Ctrl->A.active && rr > 1.0) continue;	/* Beyond the seamount base so nothing to do for a mask */
					}
					else	/* Circular features are simpler */
						rr = this_r / r_km;	/* Now in 0-1 range */

					/* Compute next height (add) it the untruncated version if -F is set (orig_add) */
					add = grdseamount_height (Ctrl, this_r, rr, h_scale, f, exp_f, S[smt].noise, &orig_add);
					/* Both add and orig_add are normalized fractions of full seamount height */
					z_assign = amplitude * add;		/* Height to be added to grid if no slide */
					if (Ctrl->S.slide) {	/* Must handle the sector variation */
						double s;
						for (k = 0; k < S[smt].n_slides; k++) {	/* See if we are inside any of the slide sectors */
							if (grdseamount_in_sector (GMT, Ctrl, &S[smt].Slide[k], Grid, in, row, col, &s)) {	/* Inside slide sector */
								/* If we are outside the radial slide range then grdseamount_slide returns z_assign so the flank height is selected */
								z_assign = grdseamount_slide (GMT, &S[smt].Slide[k], r_in, rr, z_assign, s);
							}
						}
					}
					if (z_assign <= 0.0) continue;	/* No amplitude, so skip */
					/* z_assign is the height to be added to the grid */
					ij = gmt_M_ijp (Grid->header, row, col);	/* Current node location */
					if (Ctrl->A.active)	/* No amplitude, just set inside value for mask */
						Grid->data[ij] = Ctrl->A.value[GMT_IN];
					else {	/* Add in contribution and keep track of max height */
						Grid->data[ij] += (gmt_grdfloat)z_assign;
						if (Grid->data[ij] > max) max = Grid->data[ij];
					}
					if (Ctrl->W.active) {	/* Must accumulate the average vertical density */
						/* When -T is not active then we build the complete seamount as is.  In that case
						 * we want to use z1 = 0 and z2 = h(r) in the mean density solution.  However, if
						 * -T is active and this is just an incremental layer (or cumulative surface), then we
						 * must know the final h(r) as well as the two z-values for the previous and current height at r */
						double h_at_r = S[smt].height * orig_add;	/* This is fully-built height at this radius */
						double z1_at_r = (exact_increments) ? prev_z[ij] : 0.0;	/* Should be previous height or 0.0 */
						double z2_at_r = amplitude * orig_add;	/* Current or final height */
						dz = z2_at_r - z1_at_r;	/* Range of integrated depths */
						rho = grdseamount_mean_density (Ctrl, h_at_r, z1_at_r, z2_at_r);
						rho_z = rho * dz;
						Ave->data[ij]  += (gmt_grdfloat)rho_z;	/* Must add up these since seamounts may overlap */
						rho_weight[ij] += (gmt_grdfloat)dz;
						sum_rz += rho_z;
						sum_z += dz;
						if (exact_increments) prev_z[ij] = amplitude * orig_add;
					}
					if (first) {	/* May have to copy over to repeated column in global gridline-registered grids */
						if (col == 0) {	/* Must copy from x_min to repeated column at x_max */
							if (Ctrl->A.active) Grid->data[ij+nx1] = Ctrl->A.value[GMT_IN]; else Grid->data[ij+nx1] += (gmt_grdfloat)z_assign;
							if (Ctrl->W.active) {	/* Must accumulate the average vertical density */
								Ave->data[ij+nx1]  += (gmt_grdfloat)rho_z;
								rho_weight[ij+nx1] += (gmt_grdfloat)rho_z;
							}
							first = false;
						}
						else if (col == nx1) {	/* Must copy from x_max to repeated column at x_min */
							if (Ctrl->A.active) Grid->data[ij-nx1] = Ctrl->A.value[GMT_IN]; else Grid->data[ij-nx1] += (gmt_grdfloat)z_assign;
							if (Ctrl->W.active) {	/* Must accumulate the average vertical density */
								Ave->data[ij-nx1]  += (gmt_grdfloat)rho_z;
								rho_weight[ij-nx1] += (gmt_grdfloat)rho_z;
							}
							first = false;
						}
					}
					empty = false;	/* We have made a change to this grid */
				}
			}
			if (sum_z > 0.0) {	/* This is only true when -H -W are used */
				double mean_rho = sum_rz / sum_z;
				GMT_Report (API, GMT_MSG_INFORMATION, "Seamount # %d mean density: %g\n", n_smts, mean_rho);
			}
			prev_user_time = this_user_time;	/* Make this the previous time */
		}
		if (empty && Ctrl->T.active) {	/* No contribution made */
			GMT_Report (API, GMT_MSG_INFORMATION, "No contribution made for time %g %s\n",
			            Ctrl->T.time[t].value * Ctrl->T.time[t].scale, gmt_modeltime_unit (Ctrl->T.time[t].u));
			if (exact && !exact_increments) gmt_M_memcpy (Grid->data, current, Grid->header->size, float);	/* Nothing new added so same cumulative surface as last step */
		}

		/* Time to write the shape grid */
		if (Ctrl->T.active) {	/* Create the temporary file name */
			gmt_modeltime_name (GMT, gfile, Ctrl->G.file, &(Ctrl->T.time[t]));
			if (Ctrl->W.active)
				gmt_modeltime_name (GMT, wfile, Ctrl->W.file, &(Ctrl->T.time[t]));
		}
		else {	/* Only one grid, use given name */
			strcpy (gfile, Ctrl->G.file);
			if (Ctrl->W.active)
				strcpy (wfile, Ctrl->W.file);
		}
		if (Ctrl->M.active) {	/* Write list of grids to file */
			char record[GMT_BUFSIZ] = {""}, tmp[GMT_LEN64] = {""};
			if (Ctrl->T.active) {
				sprintf (record, "%s\t", gfile);	/* First word is increment/cumulative grid name */
				if (Ctrl->W.active) {	/* Second word holds density grid name */
					strcat (record, wfile);
					strcat (record, "\t");
				}
				sprintf (tmp, time_fmt, Ctrl->T.time[t].value * Ctrl->T.time[t].scale, Ctrl->T.time[t].unit);
				strcat (record, tmp);
			}
			else {	/* Just one file since no time-stepping */
				strcpy (record, gfile);	/* First word holds the relief grid name */
				if (Ctrl->W.active) {	/* Second word holds density grid name */
					strcat (record, "\t");
					strcat (record, wfile);
				}
			}
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
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, gfile, Grid) != GMT_NOERROR)
			goto wrap_up;
		gmt_M_memcpy (Grid->data, data, Grid->header->size, gmt_grdfloat);
		if (Ctrl->W.active) {	/* Must average in case multiple seamounts overprint on the same node */
			double rs = 0.0, ws = 0.0;
			char remark[GMT_GRID_REMARK_LEN160] = {""};
			for (ij = 0; ij < Ave->header->size; ij++) {
				if (rho_weight[ij] > 0.0) {
					rs += Ave->data[ij];
					ws += rho_weight[ij];
					Ave->data[ij] /= rho_weight[ij];
				}
				else
					Ave->data[ij] = GMT->session.f_NaN;
			}
			if (ws > 0.0) rs /= ws;	/* Mean load density for this feature */
			sprintf (remark, "Mean Load Density: %lg", rs);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, remark, Ave)) Return (API->error);
			if (Ctrl->W.active && GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_WRITE_NORMAL, NULL, wfile, Ave) != GMT_NOERROR) {
				GMT_Report (API, GMT_MSG_ERROR, "Failure while writing average density grid to %s\n", wfile);
				goto wrap_up;
			}
			if (Ctrl->T.active) {	/* Reset density arrays for next model time */
				gmt_M_memset (Ave->data, Ave->header->size, gmt_grdfloat);
				gmt_M_memset (rho_weight, Ave->header->size, gmt_grdfloat);
			}
		}
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
	if (Ctrl->W.active) gmt_M_free (GMT, rho_weight);
	if (Ctrl->W.active && exact_increments) gmt_M_free (GMT, prev_z);

	Return (API->error);
}
