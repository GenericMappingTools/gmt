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
 *
 * Authors:	Walter H.F. Smith & P. Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: Reads an existing CPT and desired output grid
 * and produces a GMT CPT.  Can be inverted [-I] or made to be
 * continuous [-Z].  Discrete color jumps in CPTs are handled
 * correctly.  Default color table is GMT_DEFAULT_CPT_NAME.
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"makecpt"
#define THIS_MODULE_MODERN_NAME	"makecpt"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Make GMT color palette tables"
#define THIS_MODULE_KEYS	">C},ED(,SD(,TD(,<D("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"->Vbdhi"

EXTERN_MSC unsigned int gmtlib_log_array (struct GMT_CTRL *GMT, double min, double max, double delta, double **array);

enum makecpt_enum_mode {DO_RANGE = 0,		/* Use actual data range in -T */
	DO_ROUNDED_RANGE,			/* Round data range using inc */
	DO_ROUNDED_RANGE_E,			/* Round data range using inc via -E*/
	DO_MEAN,				/* Use mean +/- n*std */
	DO_MEDIAN,				/* Use median +/- n*L1_scale  */
	DO_MODE,				/* Use mode +/- n*LMS_scale  */
	DO_TRIM};				/* Use alpha-trimmed range */

/* Control structure for makecpt */

struct MAKECPT_CTRL {
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
	struct M {	/* -M */
		bool active;
	} M;
	struct N {	/* -N */
		bool active;
	} N;
	struct S {	/* -S */
		bool active;
		bool discrete;
		unsigned int mode;
		double scale;
		double q[2];
	} S;
	struct T {	/* -T<min/max[/inc>[+n]]|<file>|<z0,z1,...,zn> */
		bool active;
		bool interpolate;
		struct GMT_ARRAY T;
	} T;
	struct Q {	/* -Q[i|o] */
		bool active;
		unsigned int mode;
	} Q;
	struct W {	/* -W[w] */
		bool active;
		bool wrap;
	} W;
	struct Z {	/* -Z */
		bool active;
	} Z;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MAKECPT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct MAKECPT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->G.z_low = C->G.z_high = GMT->session.d_NaN;	/* No truncation */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct MAKECPT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_M_str_free (C->C.file);
	gmt_free_array (GMT, &(C->T.T));
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	const char *H_OPT = (API->GMT->current.setting.run_mode == GMT_MODERN) ? " [-H]" : "";
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [-A<transparency>[+a]] [-C<cpt>|colors] [-D[i|o]] [-E<nlevels>] [-F[R|r|h|c][+c]] [-G<zlo>/<zhi>]%s\n", name, H_OPT);
	GMT_Message (API, GMT_TIME_NONE, "	[-I[c][z]] [-M] [-N] [-Q] [-S<mode>] [-T<min>/<max>[/<inc>[+b|l|n]] | -T<table> | -T<z1,z2,...zn>] [%s] [-W[w]]\n\t[-Z] [%s] [%s] [%s]\n\t[%s] [%s]\n\n",
		GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_i_OPT, GMT_ho_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Set constant transparency for all colors; append +a to also include back-, for-, and nan-colors [0]\n");
	if (gmt_list_cpt (API->GMT, 'C')) return (GMT_CPT_READ_ERROR);	/* Display list of available color tables */
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set back- and foreground color to match the bottom/top limits\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   in the output CPT [Default uses color table]. Append i to match the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   bottom/top values in the input CPT.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Use <nlevels> equidistant color levels from zmin to zmax.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This option implies we read data from given command-line files [or stdin] to\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   determine data range (use -i to select a data column, else last column is used).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <nlevels> is not set we use the number of color slices in the chosen CPT.\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t-M Use GMT defaults to set back-, foreground, and NaN colors\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses the settings in the color table].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do not write back-, foreground, and NaN colors [Default will].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q The z-values given to -T are log10(z). Assign colors via log10(z) but write z.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Determine range in -T from input data table(s) instead.  Choose operation:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sa<scl> Make symmetric range around average (i.e., mean) and +/- <scl> * sigma.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sm<scl> Make symmetric range around median and +/- <scl> * L1_scale.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sp<scl> Make symmetric range around mode and +/- <scl> * LMS_scale.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sq<low>/<high> Set range from <low> quartile to <high> quartile.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -S<inc>[+d] Read data and round range to nearest <inc>; append +d for discrete CPT.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sr	Read data and use min/max as range.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Last data column is used in the calculation; see -i to arrange columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Make evenly spaced color boundaries from <min> to <max> in steps of <inc>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +b for log2 spacing in integer <inc> or +l for log10 spacing via <inc> = 1,2,3.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +n to indicate <inc> is the number of color boundaries to produce instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For absolute time series, append a valid time unit (%s) to the increment.\n", GMT_TIME_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, give a file with color boundaries in the first column, or a comma-separate list of values.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Do not interpolate color palette. Alternatively, append w for a wrapped CPT.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Force a continuous color palette when derived from color and z-lists [discrete].\n");
	GMT_Option (API, "bi,di,h,i,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct MAKECPT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to makecpt and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int n;
	unsigned int n_errors = 0, n_files[2] = {0, 0};
	char txt_a[GMT_LEN32] = {""}, txt_b[GMT_LEN32] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files (none expected so throw error ) */
				n_files[GMT_IN]++;
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
			case 'C':	/* CTP table */
				Ctrl->C.active = true;
				Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Set BNF to match cpt ends */
				Ctrl->D.active = true;
				Ctrl->D.mode = 1;
				if (opt->arg[0] == 'i') Ctrl->D.mode = 2;
				break;
			case 'E':	/* Use n levels */
				Ctrl->E.active = true;
				if (opt->arg[0] && sscanf (opt->arg, "%d", &Ctrl->E.levels) != 1) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error -E option: Cannot decode value\n");
					n_errors++;
				}
				break;
			case 'F':	/* Sets format for color reporting */
				Ctrl->F.active = true;
				if (gmt_validate_modifiers (GMT, opt->arg, 'F', "c")) n_errors++;
				if (gmt_get_modifier (opt->arg, 'c', txt_a)) Ctrl->F.cat = true;
				switch (opt->arg[0]) {
					case 'r': Ctrl->F.model = GMT_RGB + GMT_NO_COLORNAMES; break;
					case 'h': Ctrl->F.model = GMT_HSV; break;
					case 'c': Ctrl->F.model = GMT_CMYK; break;
					default: Ctrl->F.model = GMT_RGB; break;
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
			case 'M':	/* Use GMT defaults for BNF colors */
				Ctrl->M.active = true;
				break;
			case 'N':	/* Do not output BNF colors */
				Ctrl->N.active = true;
				break;
			case 'S':	/* Derive -T arguments from input data */
				Ctrl->S.active = true;
				switch (opt->arg[0]) {
					case 'a':
						Ctrl->S.mode = DO_MEAN;	Ctrl->S.scale = atof (&opt->arg[1]);	break;
					case 'm':
						Ctrl->S.mode = DO_MEDIAN;	Ctrl->S.scale = atof (&opt->arg[1]);	break;
					case 'p':
						Ctrl->S.mode = DO_MODE;	Ctrl->S.scale = atof (&opt->arg[1]);	break;
					case 'q':
						Ctrl->S.mode = DO_TRIM;
						sscanf (&opt->arg[1], "%[^/]/%s", txt_a, txt_b);
						gmt_scanf_float (GMT, txt_a, &Ctrl->S.q[0]);
						gmt_scanf_float (GMT, txt_b, &Ctrl->S.q[1]);
						break;
					case 'r':
					case '\0':
						Ctrl->S.mode = DO_RANGE;	break;
					default:
						Ctrl->S.mode = DO_ROUNDED_RANGE;
						if ((c = strstr (opt->arg, "+d"))) {
							c[0] = '\0';	/* Temporarily chop off modifier */
							Ctrl->S.discrete = true;
						}
						Ctrl->S.scale = atof (opt->arg);
						if (c) c[0] = '+';	/* Restore modifier */
						break;
				}
				break;
			case 'T':	/* Sets up color z values */
				Ctrl->T.active = Ctrl->T.interpolate = true;
				n_errors += gmt_parse_array (GMT, 'T', opt->arg, &(Ctrl->T.T), GMT_ARRAY_TIME | GMT_ARRAY_DIST | GMT_ARRAY_RANGE | GMT_ARRAY_NOINC, GMT_Z);
				if (Ctrl->T.T.set == 2) Ctrl->T.interpolate = false;	/* Did not give increment, just min/max */
				break;
			case 'Q':	/* Logarithmic scale */
				Ctrl->Q.active = true;
				if (opt->arg[0] != '\0' && gmt_M_compat_check (GMT, 5))
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Option -Qi or -Qo are deprecated; Use -T+l for old -Qo and -Q for old -Qi.\n");
				if (opt->arg[0] == 'o')	/* Input data is z, but take log10(z) before interpolation colors */
					Ctrl->Q.mode = 2;
				else			/* Input is log10(z) */
					Ctrl->Q.mode = 1;
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

	/* Given -Qo and -T...+l are the same, make sure both ways are defined since code uses both */
	if (Ctrl->Q.active && Ctrl->Q.mode == 2) Ctrl->T.T.logarithmic = true;
	else if (Ctrl->T.T.logarithmic) Ctrl->Q.active = true, Ctrl->Q.mode = 2;

	if (Ctrl->H.active && GMT->current.setting.run_mode == GMT_CLASSIC) {
		n_errors++;
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unrecognized option -H\n");
	}
	n_errors += gmt_M_check_condition (GMT, n_files[GMT_IN] > 0 && !(Ctrl->E.active || Ctrl->S.active),
	                                   "Syntax error: No input files expected unless -E or -S are used\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->Z.active,
	                                   "Syntax error: -W and -Z cannot be used simultaneously\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.cat && Ctrl->Z.active,
	                                   "Syntax error: -F+c and -Z cannot be used simultaneously\n");
	if (!Ctrl->S.active) {
		if (Ctrl->T.active && !Ctrl->T.interpolate && Ctrl->Z.active && (Ctrl->C.file == NULL || strchr (Ctrl->C.file, ',') == NULL)) {
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning -T option: Without inc, -Z has no effect (ignored)\n");
			Ctrl->Z.active = false;
		}
	}
	n_errors += gmt_M_check_condition (GMT, n_files[GMT_OUT] > 1, "Syntax error: Only one output destination can be specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && (Ctrl->A.value < 0.0 || Ctrl->A.value > 1.0),
	                                   "Syntax error -A: Transparency must be n 0-100 range [0 or opaque]\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && Ctrl->T.active, "Syntax error -E: Cannot be combined with -T\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_makecpt (void *V_API, int mode, void *args) {
	int i, nz = 0, error = 0;
	unsigned int cpt_flags = 0;

	bool write = false;

	double *z = NULL;

	char *l = NULL, *kind[2] = {"discrete", "continuous"};

	struct MAKECPT_CTRL *Ctrl = NULL;
	struct GMT_PALETTE *Pin = NULL, *Pout = NULL;
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

	/*---------------------------- This is the makecpt main code ----------------------------*/

	if (Ctrl->C.active) {
		if (Ctrl->C.file[0] != '@' && (l = strstr (Ctrl->C.file, ".cpt"))) *l = 0;	/* Strip off .cpt if used */
	}
	else {	/* No table specified; set default table */
		Ctrl->C.active = true;
		Ctrl->C.file = strdup (GMT->init.cpt[0]);
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Prepare CPT via the master file %s\n", Ctrl->C.file);

	/* OK, we can now do the resampling */

	if (Ctrl->M.active) cpt_flags |= GMT_CPT_NO_BNF;	/* bit 0 controls if BFN is determined by parameters */
	if (Ctrl->D.mode == 1) cpt_flags |= GMT_CPT_EXTEND_BNF;	/* bit 1 controls if BF will be set to equal bottom/top rgb value */
	if (Ctrl->Z.active) cpt_flags |= GMT_CPT_CONTINUOUS;	/* Controls if final CPT should be continuous in case input is a list of colors */

	if ((Pin = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, cpt_flags, NULL, Ctrl->C.file, NULL)) == NULL) {
		Return (API->error);
	}
	if (Ctrl->T.active && (API->error = gmt_validate_cpt_parameters (GMT, Pin, Ctrl->C.file, &(Ctrl->T.interpolate), &(Ctrl->Z.active))))
			Return (API->error)

	if (Ctrl->Q.active && Pin->has_hinge)
		GMT_Report (API, GMT_MSG_VERBOSE, "CPT %s has a hinge but you selected a logarithmic scale\n", Ctrl->C.file);
	if (Ctrl->I.mode & GMT_CPT_Z_REVERSE)	/* Must reverse the z-values before anything else */
		gmt_scale_cpt (GMT, Pin, -1.0);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "CPT is %s\n", kind[Pin->is_continuous]);
	if (Ctrl->G.active) {	/* Attempt truncation */
		struct GMT_PALETTE *Ptrunc = gmt_truncate_cpt (GMT, Pin, Ctrl->G.z_low, Ctrl->G.z_high);	/* Possibly truncate the CPT */
		if (Ptrunc == NULL)
			Return (API->error);
		Pin = Ptrunc;
	}

	if (Pin->categorical) Ctrl->W.active = true;	/* Do not want to sample a categorical table */
	if (Ctrl->E.active) {
		if (Ctrl->E.levels == 0)
			Ctrl->E.levels = Pin->n_colors + 1;	/* Default number of levels */
		else
			Ctrl->T.interpolate = true;
	}
	if (Ctrl->W.wrap) Pin->is_wrapping = true;	/* A cyclic CPT has been requested */

	if (Ctrl->S.active || Ctrl->E.active) {	/* Must read a data set and do statistics first, and then set -T values accordingly */
		unsigned int gmt_mode_selection = 0, GMT_n_multiples = 0;
		uint64_t n = 0, zcol, tbl, seg;
		enum makecpt_enum_mode tmode;
		struct GMT_DATASET *D = NULL;
		double *zz = NULL, mean_z, sig_z;
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_IN, 0, GMT_COL_FIX)) != GMT_NOERROR) Return (error);
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
			Return (API->error);
		}
		zcol = D->n_columns - 1;	/* Always the last column */
		if (!(Ctrl->E.active || Ctrl->S.mode == DO_RANGE || Ctrl->S.mode == DO_ROUNDED_RANGE)) {
			/* Must sort the data to do the L1, LMS statistics */
			zz = gmt_M_memory (GMT, NULL, D->n_records, double);
			for (tbl = n = 0; tbl < D->n_tables; tbl++) {
				for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
					gmt_M_memcpy (&zz[n], D->table[tbl]->segment[seg]->data[zcol], D->table[tbl]->segment[seg]->n_rows, double);
					n += D->table[tbl]->segment[seg]->n_rows;
				}
			}
			gmt_sort_array (GMT, zz, n, GMT_DOUBLE);
		}
		Ctrl->T.T.set = 2;	/* For most, we set the range only */
		Ctrl->T.active = true;
		tmode = (Ctrl->E.active) ? DO_ROUNDED_RANGE_E : Ctrl->S.mode;
		switch (tmode) {
			case DO_RANGE:
				Ctrl->T.T.min = D->min[zcol];	Ctrl->T.T.max = D->max[zcol];
				break;
			case DO_ROUNDED_RANGE:
				Ctrl->T.T.set = 3;	/* Here we also set inc */
				Ctrl->T.T.inc = Ctrl->S.scale;
				Ctrl->T.interpolate = Ctrl->S.discrete;
				Ctrl->T.T.min = floor (D->min[zcol] / Ctrl->T.T.inc) * Ctrl->T.T.inc;
				Ctrl->T.T.max = ceil (D->max[zcol] / Ctrl->T.T.inc) * Ctrl->T.T.inc;
				break;
			case DO_ROUNDED_RANGE_E:	/* -E only */
				Ctrl->T.T.inc = (D->max[zcol] - D->min[zcol]) / (Ctrl->E.levels - 1);
				Ctrl->T.T.set = 3;	/* Here we also set inc */
				Ctrl->T.T.min = D->min[zcol];
				Ctrl->T.T.max = D->max[zcol];
				break;
			case DO_MEAN:
				mean_z = gmt_mean_and_std (GMT, zz, n, &sig_z);
				Ctrl->T.T.min = mean_z - Ctrl->S.scale * sig_z;
				Ctrl->T.T.max = mean_z + Ctrl->S.scale * sig_z;
				break;
			case DO_MEDIAN:
				mean_z = (n % 2) ? zz[n/2] : 0.5 * (zz[n/2] + zz[(n/2)-1]);
				gmt_getmad (GMT, zz, n, mean_z, &sig_z);
				Ctrl->T.T.min = mean_z - Ctrl->S.scale * sig_z;
				Ctrl->T.T.max = mean_z + Ctrl->S.scale * sig_z;
				break;
			case DO_MODE:
				gmt_mode (GMT, zz, n, n/2, 0, gmt_mode_selection, &GMT_n_multiples, &mean_z);
				gmt_getmad (GMT, zz, n, mean_z, &sig_z);
				Ctrl->T.T.min = mean_z - Ctrl->S.scale * sig_z;
				Ctrl->T.T.max = mean_z + Ctrl->S.scale * sig_z;
				break;
			case DO_TRIM:
				Ctrl->T.T.min = gmt_quantile (GMT, zz, Ctrl->S.q[0], n);	/* "Left" quantile */
				Ctrl->T.T.max = gmt_quantile (GMT, zz, Ctrl->S.q[1], n);	/* "Left" quantile */
				break;
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			Return (API->error);
		}
		gmt_M_free (GMT, zz);
		if (Ctrl->T.T.set == 3)
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Input data and -E|S implies -T%g/%g/%g\n", Ctrl->T.T.min, Ctrl->T.T.max, Ctrl->T.T.inc);
		else
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Input data and -S implies -T%g/%g\n", Ctrl->T.T.min, Ctrl->T.T.max);
	}
	
	/* Set up arrays */

	if (Ctrl->T.active && gmt_create_array (GMT, 'T', &(Ctrl->T.T), NULL, NULL)) Return (GMT_RUNTIME_ERROR);
	nz = (int)Ctrl->T.T.n;		z = Ctrl->T.T.array;

	if (Ctrl->T.T.list && (Pin->mode & GMT_CPT_TEMPORARY)) {	/* Array was passed as a comma-separated list of z-values so make sure it matches a list of colors, if given */
		/* Got -Zcolor,color,... and -Tz,z,z */
		int k;
		extern void gmtlib_init_cpt (struct GMT_CTRL *GMT, struct GMT_PALETTE *P);
		if (nz != (int)(Pin->n_colors + 1)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Mismatch between number of entries in color and z lists\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if ((Pout = GMT_Duplicate_Data (API, GMT_IS_PALETTE, GMT_DUPLICATE_ALLOC, Pin)) == NULL) {
			Return (API->error);
		}
		for (i = k = 0; i < (int)Pout->n_colors; i++) {
			Pout->data[i].z_low  = z[k];
			Pout->data[i].z_high = z[++k];
		}
		gmtlib_init_cpt (GMT, Pin);	/* Recalculate delta rgb's */
		if (Ctrl->I.mode & GMT_CPT_C_REVERSE)	/* Also flip the colors */
			gmt_invert_cpt (GMT, Pout);
	}
	else if (Ctrl->T.active && Ctrl->Q.mode == 2) {	/* Must establish a log10 lattice instead of linear */
		if (!(Ctrl->T.T.inc == 1.0 || Ctrl->T.T.inc == 2.0 || Ctrl->T.T.inc == 3.0)) {
			GMT_Report (API, GMT_MSG_NORMAL, "For -Qo logarithmic spacing, inc must be 1, 2, or 3\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (Ctrl->T.T.min <= 0.0) {
			GMT_Report (API, GMT_MSG_NORMAL, "For -Qo logarithmic spacing, min must be > 0\n");
			Return (GMT_RUNTIME_ERROR);
		}
		gmt_M_free (GMT, Ctrl->T.T.array);	/* Free and rebuild using log scheme */
		nz = gmtlib_log_array (GMT, Ctrl->T.T.min, Ctrl->T.T.max, Ctrl->T.T.inc, &(Ctrl->T.T.array));
		z = Ctrl->T.T.array;
	}
	if (!Ctrl->T.interpolate) {	/* Just copy what was in the CPT but stretch to given range min/max */
		if ((Pout = GMT_Duplicate_Data (API, GMT_IS_PALETTE, GMT_DUPLICATE_ALLOC, Pin)) == NULL) return (API->error);
		gmt_stretch_cpt (GMT, Pout, Ctrl->T.T.min, Ctrl->T.T.max);	/* Stretch to given range or use natural range if 0/0 */
		if (Ctrl->I.mode & GMT_CPT_C_REVERSE)	/* Also flip the colors */
			gmt_invert_cpt (GMT, Pout);
	}

	if (Pout == NULL) {	/* Meaning it was not created above */
		if (Ctrl->Q.mode == 2)
			for (i = 0; i < nz; i++) z[i] = d_log10 (GMT, z[i]);	/* Make log10(z) values for interpolation step */

		/* Now we can resample the CPT and write out the result */

		Pout = gmt_sample_cpt (GMT, Pin, z, nz, Ctrl->Z.active, Ctrl->I.mode & GMT_CPT_C_REVERSE, Ctrl->Q.mode, Ctrl->W.active);
	}

	if (Ctrl->A.active) gmt_cpt_transparency (GMT, Pout, Ctrl->A.value, Ctrl->A.mode);	/* Set transparency */

	/* Determine mode flags for output */
	cpt_flags = 0;
	if (Ctrl->N.active) cpt_flags |= GMT_CPT_NO_BNF;	/* bit 0 controls if BFN will be written out */
	if (Ctrl->D.mode == 1) cpt_flags |= GMT_CPT_EXTEND_BNF;	/* bit 1 controls if BF will be set to equal bottom/top rgb value */
	if (Ctrl->F.active) Pout->model = Ctrl->F.model;
	if (Ctrl->F.cat) Pout->categorical = 1;

	write = (GMT->current.setting.run_mode == GMT_CLASSIC || Ctrl->H.active);	/* Only output to stdout in classic mode and with -H in modern mode */

	if (write && GMT_Write_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, cpt_flags, NULL, Ctrl->Out.file, Pout) != GMT_NOERROR) {
		Return (API->error);
	}

	if (!write)
		gmt_save_current_cpt (GMT, Pout, cpt_flags);	/* Save for use by session, if modern */
	
	Return (GMT_NOERROR);
}
