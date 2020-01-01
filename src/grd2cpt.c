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
 * Brief synopsis: grd2cpt reads a 2d binary gridded grdfile and creates a continuous-color-
 * palette CPT, with a non-linear histogram-equalized mapping between
 * hue and data value.  (The linear mapping can be made with grd2cpt.)
 *
 * Creates a cumulative distribution function f(z) describing the data
 * in the grdfile.  f(z) is sampled at z values supplied by the user
 * [with -S option] or guessed from the sample mean and standard deviation.
 * f(z) is then found by looping over the grd array for each z and counting
 * data values <= z.  Once f(z) is found then a master CPT is resampled
 * based on a normalized f(z).
 *
 * Author:	Walter H. F. Smith
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grd2cpt"
#define THIS_MODULE_MODERN_NAME	"grd2cpt"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Make linear or histogram-equalized color palette table from grid"
#define THIS_MODULE_KEYS	"<G{+,>C}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"->RVh"

#define GRD2CPT_N_LEVELS	11	/* The default number of levels if nothing is specified */

struct GRD2CPT_CTRL {
	struct In {
		bool active;
	} In;
	struct Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct A {	/* -A<transp>[+a] */
		bool active;
		unsigned int mode;
		double value;
	} A;
	struct C {	/* -C<cpt> or -C<color1>,<color2>[,<color3>,...] */
		bool active;
		char *file;
	} C;
	struct D {	/* -D[i|o] */
		bool active;
		unsigned int mode;
	} D;
	struct E {	/* -E<nlevels> */
		bool active;
		unsigned int levels;
	} E;
	struct F {	/* -F[r|R|h|c][+c] */
		bool active;
		bool cat;
		unsigned int model;
	} F;
	struct G {	/* -Glow/high for input CPT truncation */
		bool active;
		double z_low, z_high;
	} G;
	struct H {	/* -H */
		bool active;
	} H;
	struct I {	/* -I[z][c] */
		bool active;
		unsigned int mode;
	} I;
	struct L {	/* -L<min_limit>/<max_limit> */
		bool active;
		double min, max;
	} L;
	struct M {	/* -M */
		bool active;
	} M;
	struct N {	/* -N */
		bool active;
	} N;
	struct Q {	/* -Q[i|o] */
		bool active;
		unsigned int mode;
	} Q;
	struct T {	/* -T<start>/<stop>/<inc> or -T<n_levels> */
		bool active;
		unsigned int mode;	/* 0 or 1 (-Tn) */
		unsigned int n_levels;
		double low, high, inc;
		char *file;
	} T;
	struct S {	/* -S<kind> */
		bool active;
		int kind; /* -1 symmetric +-zmin, +1 +-zmax, -2 = +-Minx(|zmin|,|zmax|), +2 = +-Max(|zmin|,|zmax|), 0 = min to max [Default] */
	} S;
	struct W {	/* -W[w] */
		bool active;
		bool wrap;
	} W;
	struct Z {	/* -Z */
		bool active;
	} Z;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRD2CPT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRD2CPT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->G.z_low = C->G.z_high = GMT->session.d_NaN;	/* No truncation */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRD2CPT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->T.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	const char *H_OPT = (API->GMT->current.setting.run_mode == GMT_MODERN) ? " [-H]" : "";
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <grid> [-A<transparency>[+a]] [-C<cpt>] [-D[i]] [-E[<nlevels>]] [-F[R|r|h|c][+c]]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-G<zlo>/<zhi>]%s [-I[c][z]] [-L<min_limit>/<max_limit>] [-M] [-N] [-Q[i|o]]\n", H_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-T<start>/<stop>/<inc> or -T<n>]\n\t[-Sh|l|m|u] [%s] [-W[w]] [-Z] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<grid> is name of one or more grid files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Set constant transparency for all colors; append +a to also include back-, for-, and nan-colors [0]\n");
	if (gmt_list_cpt (API->GMT, 'C')) return (GMT_CPT_READ_ERROR);	/* Display list of available color tables */
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set back- and foreground color to match the bottom/top limits\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   in the output CPT [Default uses color table]. Append i to match the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   bottom/top values in the input CPT.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set CPT to go from grid zmin to zmax (i.e., a linear scale).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, append <nlevels> to sample equidistant color levels from zmin to zmax.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Select the color model for output (R for r/g/b or grayscale or colorname,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   r for r/g/b only, h for h-s-v, c for c/m/y/k) [Default uses the input model]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c to output a discrete CPT in categorical CPT format.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Truncate incoming CPT to be limited to the z-range <zlo>/<zhi>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To accept one of the incoming limits, set that limit to NaN.\n");
	if (API->GMT->current.setting.run_mode == GMT_MODERN)
		GMT_Message (API, GMT_TIME_NONE, "\t-H Also write CPT to stdout [Default just saves as current CPT].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Reverse sense of CPT in one or two ways:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ic Reverse sense of color table as well as back- and foreground color [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Iz Reverse sign of z-values in the color table (takes affect before -G, T are consulted).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Limit the range of the data.  Node values outside this range are set to NaN.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses actual min,max of data].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Use GMT defaults to set back-, foreground, and NaN colors [Default uses color table].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do not write back-, foreground, and NaN colors [Default will].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Assign a logarithmic colortable [Default is linear].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qi: z-values are actually log10(z). Assign colors and write z [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qo: z-values are z, but take log10(z), assign colors and write z.\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Force color tables to be symmetric about 0. Append one modifier:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   l (low)   for values symmetric about zero from -|zmin| to +|zmin|.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   u (upper) for values symmetric about zero from -|zmax| to +|zmax|.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m (min)   for values symmetric about zero -+min(|zmin|,|zmax|).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   h (high)  for values symmetric about zero -+max(|zmin|,|zmax|).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T CDF sample points should range from <start> to <stop> by <inc>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -T<n> to select <n> points from a cumulative normal distribution [%d].\n", GRD2CPT_N_LEVELS);
	GMT_Message (API, GMT_TIME_NONE, "\t   <start> maps to data min and <stop> maps to data max (but see -L).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses equidistant steps for a Gaussian CDF].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Do not interpolate color palette. Alternatively, append w for a wrapped CPT.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Create a continuous color palette [Default is discontinuous, i.e., constant color intervals].\n");
	GMT_Option (API, "h,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRD2CPT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int n;
	unsigned int n_errors = 0, n_files[2] = {0, 0};
	char txt_a[GMT_LEN32] = {""}, txt_b[GMT_LEN32] = {""}, *c;
	char *T_arg = NULL, *S_arg = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				Ctrl->In.active = true;
				n_files[GMT_IN]++;
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)) n_errors++;
				break;
			case '>':	/* Got named output file */
				if (n_files[GMT_OUT]++ == 0) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Sets transparency [-A<transp>[+a]] */
				Ctrl->A.active = true;
				if (opt->arg[0] == '+') {	/* Old syntax */
					Ctrl->A.mode = 1;
					Ctrl->A.value = 0.01 * atof (&opt->arg[1]);
				}
				else if ((c = strstr (opt->arg, "+a"))) {
					Ctrl->A.mode = 1;
					c[0] = '\0';
					Ctrl->A.value = 0.01 * atof (opt->arg);
					c[0] = '+';
				}
				else
					Ctrl->A.value = 0.01 * atof (opt->arg);
				break;
			case 'C':	/* Get CPT */
				Ctrl->C.active = true;
				Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Set fore/back-ground to match end-colors */
				Ctrl->D.active = true;
				Ctrl->D.mode = 1;
				if (opt->arg[0] == 'i') Ctrl->D.mode = 2;
				break;
			case 'E':	/* Use n levels */
				Ctrl->E.active = true;
				if (opt->arg[0] && sscanf (opt->arg, "%d", &Ctrl->E.levels) != 1) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -E option: Cannot decode value\n");
					n_errors++;
				}
				break;
			case 'F':	/* Set color model for output */
				if (gmt_validate_modifiers (GMT, opt->arg, 'F', "c")) n_errors++;
				if (gmt_get_modifier (opt->arg, 'c', txt_a)) {
					Ctrl->F.cat = true;
					if (txt_a[0] == '\0') break;
				}
				Ctrl->F.active = true;
				switch (txt_a[0]) {
					case 'r': Ctrl->F.model = GMT_RGB + GMT_NO_COLORNAMES; break;
					case 'h': Ctrl->F.model = GMT_HSV; break;
					case 'c': Ctrl->F.model = GMT_CMYK; break;
					default:  Ctrl->F.model = GMT_RGB; break;
				}
				break;
			case 'G':	/* truncate incoming CPT */
				Ctrl->G.active = true;
				n = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b);
				n_errors += gmt_M_check_condition (GMT, n < 2, "Syntax error -G option: Must specify z_low/z_high\n");
				if (!(txt_a[0] == 'N' || txt_a[0] == 'n') || !strcmp (txt_a, "-")) Ctrl->G.z_low = atof (txt_a);
				if (!(txt_b[0] == 'N' || txt_b[0] == 'n') || !strcmp (txt_b, "-")) Ctrl->G.z_high = atof (txt_b);
				n_errors += gmt_M_check_condition (GMT, gmt_M_is_dnan (Ctrl->G.z_low) && gmt_M_is_dnan (Ctrl->G.z_high),
								"Syntax error -G option: Both of z_low/z_high cannot be NaN\n");
				break;
			case 'H':	/* Modern mode only: write CPT to stdout */
				Ctrl->H.active = true;
				break;
			case 'I':	/* Invert table */
				Ctrl->I.active = true;
				if ((Ctrl->I.mode = gmt_parse_inv_cpt (GMT, opt->arg)) == UINT_MAX)
					n_errors++;
				break;
			case 'L':	/* Limit data range */
				Ctrl->L.active = true;
				if (sscanf (opt->arg, "%lf/%lf", &Ctrl->L.min, &Ctrl->L.max) != 2) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L option: Cannot decode limits\n");
					n_errors++;
				}
				break;
			case 'M':	/* Override fore/back/NaN using GMT defaults */
				Ctrl->M.active = true;
				break;
			case 'N':	/* Do not write F/B/N colors */
				Ctrl->N.active = true;
				break;
			case 'Q':	/* Logarithmic data */
				Ctrl->Q.active = true;
				if (opt->arg[0] == 'o')	/* Input data is z, but take log10(z) before interpolation colors */
					Ctrl->Q.mode = 2;
				else			/* Input is log10(z) */
					Ctrl->Q.mode = 1;
				break;
			case 'T':	/* Sets sample range */
				Ctrl->T.active = true;
				T_arg = opt->arg;
				break;
			case 'S':	/* Force symmetry */
				Ctrl->S.active = true;
				S_arg = opt->arg;
				break;
			case 'W':	/* Do not interpolate colors */
				if (opt->arg[0] == 'w')
					Ctrl->W.wrap = true;
				else
					Ctrl->W.active = true;
				break;
			case 'Z':	/* Continuous colors */
				Ctrl->Z.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (T_arg) {	/* Must handle old or new syntax */
		if (strchr ("-+_=", T_arg[0]) && T_arg[1] == '\0') {	/* Old -Targs for symmetry given */
			if (gmt_M_compat_check (GMT, 5)) {	/* OK to parse that? */
				GMT_Report (API, GMT_MSG_COMPAT, "Option -T-|+|_|= is deprecated; use -Sl|u|m|h instead.\n");
				Ctrl->S.active = true;
				if (!S_arg) Ctrl->T.active = false;
				switch (T_arg[0]) {
					case '-': Ctrl->S.kind = -1; break; /* Symmetric with |zmin| range */
					case '+': Ctrl->S.kind = +1; break; /* Symmetric with |zmax| range */
					case '_': Ctrl->S.kind = -2; break; /* Symmetric with min(|zmin|,|zmax|) range */
					case '=': Ctrl->S.kind = +2; break; /* Symmetric with max(|zmin|,|zmax|) range */
					default: break;
				}
			}
			else {
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Cannot decode values %s\n", T_arg);
				n_errors++;
			}
		}
		else {	/* Got correct modern args */
			if (strchr (T_arg, '/')) {	/* Gave low/high/inc */
				if (sscanf (T_arg, "%lf/%lf/%lf", &Ctrl->T.low, &Ctrl->T.high, &Ctrl->T.inc) != 3) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Cannot decode values\n");
					n_errors++;
				}
				Ctrl->T.mode = 0;
			}
			else if (T_arg[0]) {	/* Gave -T<nlevels> */
				Ctrl->T.n_levels = atoi (T_arg);
				Ctrl->T.mode = 1;
			}
		}
	}
	if (S_arg) {	/* Must handle old or new syntax */
		if (strchr ("hlmu", S_arg[0]) && S_arg[1] == '\0') {	/* New -S for symmetry */
			switch (S_arg[0]) {
				case 'l': Ctrl->S.kind = -1; break; /* Symmetric with |zmin| range */
				case 'u': Ctrl->S.kind = +1; break; /* Symmetric with |zmax| range */
				case 'm': Ctrl->S.kind = -2; break; /* Symmetric with min(|zmin|,|zmax|) range */
				case 'h': Ctrl->S.kind = +2; break; /* Symmetric with max(|zmin|,|zmax|) range */
				default: break;
			}
		}
		else if (gmt_M_compat_check (GMT, 5)) {	/* Old-style -S range */
			GMT_Report (API, GMT_MSG_COMPAT, "Option -S<start>/<stop>/<inc> or -S<n> is deprecated; use -T instead.\n");
			if (strchr (S_arg, '/')) {	/* Gave low/high/inc */
				if (sscanf (S_arg, "%lf/%lf/%lf", &Ctrl->T.low, &Ctrl->T.high, &Ctrl->T.inc) != 3) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Cannot decode values %s\n", S_arg);
					n_errors++;
				}
				Ctrl->T.mode = 0;
			}
			else if (S_arg[0]) {	/* Gave -S<nlevels> */
				Ctrl->T.n_levels = atoi (S_arg);
				Ctrl->T.mode = 1;
			}
			if (!T_arg) Ctrl->S.active = false;
			Ctrl->T.active = true;
		}
		else {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -S option: Cannot decode values %s\n", S_arg);
			n_errors++;
		}
	}
	
	if (Ctrl->H.active && GMT->current.setting.run_mode == GMT_CLASSIC) {
		n_errors++;
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unrecognized option -H\n");
	}
	n_errors += gmt_M_check_condition (GMT, n_files[GMT_IN] < 1, "Error: No grid name(s) specified.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->Z.active,
					"Syntax error: -W and -Z cannot be used simultaneously\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.cat && Ctrl->Z.active,
	                                "Syntax error: -F+c and -Z cannot be used simultaneously\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && Ctrl->L.min >= Ctrl->L.max,
					"Syntax error -L option: min_limit must be less than max_limit.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->T.mode == 0 && (Ctrl->T.high <= Ctrl->T.low || Ctrl->T.inc <= 0.0),
					"Syntax error -S option: Bad arguments\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->T.mode == 1  && Ctrl->T.n_levels == 0,
					"Syntax error -S option: Bad arguments\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && (Ctrl->S.active || Ctrl->E.active),
					"Syntax error -T option: Cannot be combined with -E nor -S option.\n");
	n_errors += gmt_M_check_condition (GMT, n_files[GMT_OUT] > 1, "Syntax error: Only one output destination can be specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && (Ctrl->A.value < 0.0 || Ctrl->A.value > 1.0),
					"Syntax error -A: Transparency must be n 0-100 range [0 or opaque]\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int free_them_grids (struct GMTAPI_CTRL *API, struct GMT_GRID **G, char **grdfile, uint64_t n) {
	/* Free what we are pointing to */
	uint64_t k;
	for (k = 0; k < n; k++) {
		gmt_M_str_free (grdfile[k]);
		if (GMT_Destroy_Data (API, &G[k]) != GMT_NOERROR)
			return (API->error);
	}
	return (GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grd2cpt (void *V_API, int mode, void *args) {
	uint64_t ij, k, ngrd = 0, nxyg, nfound, ngood;
	unsigned int row, col, j, cpt_flags = 0;
	int signed_levels, error = 0;
	size_t n_alloc = GMT_TINY_CHUNK;
	bool write = false;

	char format[GMT_BUFSIZ] = {""}, *l = NULL, **grdfile = NULL;

	double *z = NULL, wesn[4], mean, sd;

	struct CDF_CPT {
		double	z;	/* Data value  */
		double	f;	/* Cumulative distribution function f(z)  */
	} *cdf_cpt = NULL;

	struct GMT_OPTION *opt = NULL;
	struct GMT_PALETTE *Pin = NULL, *Pout = NULL;
	struct GMT_GRID **G;
	struct GRD2CPT_CTRL *Ctrl = NULL;
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
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grd2cpt main code ----------------------------*/

	if (Ctrl->C.active) {
		if (Ctrl->C.file[0] != '@' && (l = strstr (Ctrl->C.file, ".cpt")) != NULL) *l = 0;	/* Strip off .cpt if used */
	}
	else {	/* No table specified; set GMT_DEFAULT_CPT_NAME table */
		Ctrl->C.active = true;
		Ctrl->C.file = strdup (GMT->init.cpt[0]);
	}

	if (!Ctrl->E.active) Ctrl->E.levels = (Ctrl->T.n_levels > 0) ? Ctrl->T.n_levels : GRD2CPT_N_LEVELS;	/* Default number of levels */
	if (Ctrl->M.active) cpt_flags |= GMT_CPT_NO_BNF;		/* bit 0 controls if BFN is determined by parameters */
	if (Ctrl->D.mode == 2) cpt_flags |= GMT_CPT_EXTEND_BNF;		/* bit 1 controls if BF will be set to equal bottom/top rgb value */
	if (Ctrl->Z.active) cpt_flags |= GMT_CPT_CONTINUOUS;	/* Controls if final CPT should be continuous in case input is a list of colors */

	if ((Pin = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, cpt_flags, NULL, Ctrl->C.file, NULL)) == NULL) {
		Return (API->error);
	}
	if (Ctrl->I.mode & GMT_CPT_Z_REVERSE)	/* Must reverse the z-values before anything else */
		gmt_scale_cpt (GMT, Pin, -1.0);
	
	if (Ctrl->G.active) {	/* Attempt truncation */
		struct GMT_PALETTE *Ptrunc = gmt_truncate_cpt (GMT, Pin, Ctrl->G.z_low, Ctrl->G.z_high);	/* Possibly truncate the CPT */
		if (Ptrunc == NULL)
			Return (EXIT_FAILURE);
		Pin = Ptrunc;
	}
	if (Ctrl->W.wrap) Pin->is_wrapping = true;	/* A cyclic CPT has been requested */

	write = (GMT->current.setting.run_mode == GMT_CLASSIC || Ctrl->H.active);	/* Only output to stdout in classic mode and with -H in modern mode */

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input grid(s)\n");

	gmt_M_memset (wesn, 4, double);
	if (GMT->common.R.active[RSET]) gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Subset */

	G = gmt_M_memory (GMT, NULL, n_alloc, struct GMT_GRID *);	/* Potentially an array of grids */
	grdfile = gmt_M_memory (GMT, NULL, n_alloc, char *);	/* Potentially an array of gridfile names */

	for (opt = options, k = 0; opt; opt = opt->next) {
		if (opt->option != '<') continue;	/* We are only processing input files here */

		if ((G[k] = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, wesn, opt->arg, NULL)) == NULL) {
			error = free_them_grids (API, G, grdfile, k);
			gmt_M_free (GMT, G);
			gmt_M_free (GMT, grdfile);
			Return ((error) ? error : API->error);
		}
		grdfile[k] = strdup (opt->arg);
		if (k && !(G[k]->header->n_columns == G[k-1]->header->n_columns && G[k]->header->n_rows == G[k-1]->header->n_rows)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Grids do not have the same domain!\n");
			error = free_them_grids (API, G, grdfile, k);
			gmt_M_free (GMT, G);
			gmt_M_free (GMT, grdfile);
			Return ((error) ? error : API->error);
		}

		k++;
		if (k == n_alloc) {
			size_t old_n_alloc = n_alloc;
			n_alloc += GMT_TINY_CHUNK;
			G = gmt_M_memory (GMT, G, n_alloc, struct GMT_GRID *);
			gmt_M_memset (&(G[old_n_alloc]), n_alloc - old_n_alloc, struct GMT_GRID *);	/* Set to NULL */
			grdfile = gmt_M_memory (GMT, grdfile, n_alloc, char *);
			gmt_M_memset (&(grdfile[old_n_alloc]), n_alloc - old_n_alloc, char *);	/* Set to NULL */
		}
	}

	ngrd = k;
	if (ngrd < n_alloc) {
		G = gmt_M_memory (GMT, G, ngrd, struct GMT_GRID *);
		grdfile = gmt_M_memory (GMT, grdfile, ngrd, char *);
	}

	nxyg = G[0]->header->nm * ngrd;

	/* Loop over the files and find NaNs.  If set limits, may create more NaNs  */
	/* We use the G[0]->header to keep limits representing all the grids */

	nfound = 0;
	mean = sd = 0.0;
	if (Ctrl->L.active) {	/* Loop over the grdfiles, and set anything outside the limiting values to NaN.  */
		G[0]->header->z_min = Ctrl->L.min;
		G[0]->header->z_max = Ctrl->L.max;
		for (k = 0; k < ngrd; k++) {	/* For each grid */
			gmt_M_grd_loop (GMT, G[k], row, col, ij) {
				if (gmt_M_is_fnan (G[k]->data[ij]))
					nfound++;
				else {
					if (G[k]->data[ij] < Ctrl->L.min || G[k]->data[ij] > Ctrl->L.max) {
						nfound++;
						G[k]->data[ij] = GMT->session.f_NaN;
					}
					else {
						mean += G[k]->data[ij];
						sd += G[k]->data[ij] * G[k]->data[ij];
					}
				}
			}
		}
	}
	else {
		Ctrl->L.min = G[0]->header->z_max;	/* This is just to double check G[k]->header->z_min, G[k]->header->z_max  */
		Ctrl->L.max = G[0]->header->z_min;
		for (k = 0; k < ngrd; k++) {	/* For each grid */
			gmt_M_grd_loop (GMT, G[k], row, col, ij) {
				if (gmt_M_is_fnan (G[k]->data[ij]))
					nfound++;
				else {
					if (G[k]->data[ij] < Ctrl->L.min) Ctrl->L.min = G[k]->data[ij];
					if (G[k]->data[ij] > Ctrl->L.max) Ctrl->L.max = G[k]->data[ij];
					mean += G[k]->data[ij];
					sd += G[k]->data[ij] * G[k]->data[ij];
				}
			}
		}
		G[0]->header->z_min = Ctrl->L.min;
		G[0]->header->z_max = Ctrl->L.max;
	}
	
	if (Ctrl->E.active && Ctrl->E.levels == 0) {	/* Use existing CPT structure, just linearly change z */
		if ((Pout = GMT_Duplicate_Data (API, GMT_IS_PALETTE, GMT_DUPLICATE_ALLOC, Pin)) == NULL) return (API->error);
		gmt_stretch_cpt (GMT, Pout, Ctrl->L.min, Ctrl->L.max);
		if (Ctrl->I.mode & GMT_CPT_C_REVERSE)
			gmt_invert_cpt (GMT, Pout);	/* Also flip the colors */
		cpt_flags = 0;
		if (Ctrl->N.active) cpt_flags |= GMT_CPT_NO_BNF;	/* bit 0 controls if BFN will be written out */
		if (Ctrl->D.mode == 1) cpt_flags |= GMT_CPT_EXTEND_BNF;	/* bit 1 controls if BF will be set to equal bottom/top rgb value */
		if (Ctrl->F.active) Pout->model = Ctrl->F.model;
		if (Ctrl->F.cat) Pout->categorical = 1;
		if (write && GMT_Write_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, cpt_flags, NULL, Ctrl->Out.file, Pout) != GMT_NOERROR) {
			Return (API->error);
		}
		if (!write)
			gmt_save_current_cpt (GMT, Pout, cpt_flags);	/* Save for use by session, if modern */
		free_them_grids (API, G, grdfile, ngrd);
		gmt_M_free (GMT, G);
		gmt_M_free (GMT, grdfile);

		Return (GMT_NOERROR);
	}

	ngood = nxyg - nfound;	/* This is the number of non-NaN points for the cdf function  */
	mean /= ngood;
	sd /= ngood;
	sd = sqrt (sd - mean * mean);
	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		sprintf (format, "Mean and S.D. of data are %s %s\n",
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, format, mean, sd);
	}

	/* Decide how to make steps in z.  */
	if (Ctrl->T.active && Ctrl->T.mode == 0) {	/* Use predefined levels and interval */
		unsigned int i, j;

		Ctrl->E.levels = (G[0]->header->z_min < Ctrl->T.low) ? 1 : 0;
		Ctrl->E.levels += urint (floor((Ctrl->T.high - Ctrl->T.low)/Ctrl->T.inc)) + 1;
		if (G[0]->header->z_max > Ctrl->T.high) Ctrl->E.levels++;
		cdf_cpt = gmt_M_memory (GMT, NULL, Ctrl->E.levels, struct CDF_CPT);
		if (G[0]->header->z_min < Ctrl->T.low) {
			cdf_cpt[0].z = G[0]->header->z_min;
			cdf_cpt[1].z = Ctrl->T.low;
			i = 2;
		}
		else {
			cdf_cpt[0].z = Ctrl->T.low;
			i = 1;
		}
		j = (G[0]->header->z_max > Ctrl->T.high) ? Ctrl->E.levels - 1 : Ctrl->E.levels;
		while (i < j) {
			cdf_cpt[i].z = cdf_cpt[i-1].z + Ctrl->T.inc;
			i++;
		}
		if (j == Ctrl->E.levels-1) cdf_cpt[j].z = G[0]->header->z_max;
	}
	else if (Ctrl->S.active || Ctrl->E.active) {	/* Make a equaldistant color map from G[k]->header->z_min to G[k]->header->z_max */
		double start, range;

		switch (Ctrl->S.kind) {
			case -1:
				start = -fabs (G[0]->header->z_min);
				break;
			case 1:
				start = -fabs (G[0]->header->z_max);
				break;
			case -2:
				start = -MIN (fabs (G[0]->header->z_min), fabs (G[0]->header->z_max));
				break;
			case 2:
				start = -MAX (fabs (G[0]->header->z_min), fabs (G[0]->header->z_max));
				break;
			default:
				start = G[0]->header->z_min;
				break;
		}
		range = (Ctrl->S.kind) ? 2.0 * fabs (start) : G[0]->header->z_max - G[0]->header->z_min;
		range *= (1.0 + GMT_CONV8_LIMIT);	/* To ensure the max grid values do not exceed the CPT limit due to round-off issues */
		start -= fabs (start) * GMT_CONV8_LIMIT;	/* To ensure the start of cpt is less than min value due to roundoff  */
		Ctrl->T.inc = range / (double)(Ctrl->E.levels - 1);
		cdf_cpt = gmt_M_memory (GMT, NULL, Ctrl->E.levels, struct CDF_CPT);
		for (j = 0; j < Ctrl->E.levels; j++) cdf_cpt[j].z = start + j * Ctrl->T.inc;
	}

	else {	/* This is completely ad-hoc.  It chooses z based on equidistant steps [of 0.1 unless -Sn set] for a Gaussian CDF:  */
		double z_inc = 1.0 / (Ctrl->E.levels - 1);		/* Increment between selected points [0.1] */
		double zcrit_tail = gmt_zcrit (GMT, 1.0 - z_inc);	/* Get the +/- z-value containing bulk of distribution, with z_inc in each tail */
		cdf_cpt = gmt_M_memory (GMT, NULL, Ctrl->E.levels, struct CDF_CPT);
		if ((mean - zcrit_tail*sd) <= G[0]->header->z_min || (mean + zcrit_tail*sd) >= G[0]->header->z_max) {
			/* Adjust mean/std so that our critical locations are still inside the min/max of the data */
			mean = 0.5 * (G[0]->header->z_min + G[0]->header->z_max);
			sd = (G[0]->header->z_max - mean) / 1.5;	/* This factor of 1.5 probably needs to change since z_inc is no longer fixed at 0.1 */
			if (sd <= 0.0) {
				GMT_Report (API, GMT_MSG_NORMAL, "Min and Max data values are equal.\n");
				gmt_M_free (GMT, cdf_cpt);
				Return (GMT_RUNTIME_ERROR);
			}
		}	/* End of stupid bug fix  */

		/* So we go in steps of z_inc in the Gaussian CDF except we start and stop at actual min/max */
		cdf_cpt[0].z = G[0]->header->z_min;
		for (j = 1; j < (Ctrl->E.levels - 1); j++) cdf_cpt[j].z = mean + gmt_zcrit (GMT, j *z_inc) * sd;
		cdf_cpt[Ctrl->E.levels-1].z = G[0]->header->z_max;
	}

	/* Get here when we are ready to go.  cdf_cpt[].z contains the sample points.  */

	if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE)) sprintf (format, "z = %s and CDF(z) = %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	for (j = 0; j < Ctrl->E.levels; j++) {
		if (cdf_cpt[j].z <= G[0]->header->z_min)
			cdf_cpt[j].f = 0.0;
		else if (cdf_cpt[j].z >= G[0]->header->z_max)
			cdf_cpt[j].f = 1.0;
		else {
			nfound = 0;
			for (k = 0; k < ngrd; k++) {	/* For each grid */
				gmt_M_grd_loop (GMT, G[k], row, col, ij) {
					if (!gmt_M_is_fnan (G[k]->data[ij]) && G[k]->data[ij] <= cdf_cpt[j].z) nfound++;
				}
			}
			cdf_cpt[j].f = (double)(nfound-1)/(double)(ngood-1);
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, format, cdf_cpt[j].z, cdf_cpt[j].f);
	}

	/* Now the cdf function has been found.  We now resample the chosen CPT  */

	z = gmt_M_memory (GMT, NULL, Ctrl->E.levels, double);
	for (j = 0; j < Ctrl->E.levels; j++) z[j] = cdf_cpt[j].z;
	if (Ctrl->Q.mode == 2) for (j = 0; j < Ctrl->E.levels; j++) z[j] = d_log10 (GMT, z[j]);	/* Make log10(z) values for interpolation step */

	signed_levels = Ctrl->E.levels;
	Pout = gmt_sample_cpt (GMT, Pin, z, -signed_levels, Ctrl->Z.active, Ctrl->I.active, Ctrl->Q.mode, Ctrl->W.active);	/* -ve to keep original colors */

	/* Determine mode flags for output */
	cpt_flags = 0;
	if (Ctrl->N.active) cpt_flags |= GMT_CPT_NO_BNF;	/* bit 0 controls if BFN will be written out */
	if (Ctrl->D.mode == 1) cpt_flags |= GMT_CPT_EXTEND_BNF;	/* bit 1 controls if BF will be set to equal bottom/top rgb value */
	if (Ctrl->F.active) Pout->model = Ctrl->F.model;
	if (Ctrl->F.cat) Pout->categorical = 1;

	if (Ctrl->A.active) gmt_cpt_transparency (GMT, Pout, Ctrl->A.value, Ctrl->A.mode);	/* Set transparency */

	if (write && GMT_Write_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, cpt_flags, NULL, Ctrl->Out.file, Pout) != GMT_NOERROR)
		error = API->error;

	if (!write) 
		gmt_save_current_cpt (GMT, Pout, cpt_flags);	/* Save for use by session, if modern */

	gmt_M_free (GMT, cdf_cpt);
	gmt_M_free (GMT, z);
	if (error == GMT_NOERROR)
		error = free_them_grids (API, G, grdfile, ngrd);
	else
		free_them_grids (API, G, grdfile, ngrd);
	gmt_M_free (GMT, G);
	gmt_M_free (GMT, grdfile);

	Return ((error) ? error : GMT_NOERROR);
}
