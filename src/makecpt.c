/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2019 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 *
 * Authors:	Walter H.F. Smith & P. Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: Reads an existing CPT and desired output grid
 * and produces a GMT CPT.  Can be inverted [-I] or made to be
 * continuous [-Z].  Discrete color jumps in CPTs are handled
 * correctly.  Default color table is "rainbow".
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"makecpt"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Make GMT color palette tables"
#define THIS_MODULE_KEYS	">C},ED(,TD("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->Vbdhi"

EXTERN_MSC unsigned int gmtlib_log_array (struct GMT_CTRL *GMT, double min, double max, double delta, double **array);

/* Control structure for makecpt */

struct MAKECPT_CTRL {
	struct Out {	/* -> */
		bool active;
		char *file;
	} Out;
	struct A {	/* -A+ */
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
	struct T {	/* -T<z_min/z_max[/z_inc>[+n]]|<zfile>|<z0,z1,...,zn> */
		bool active;
		bool interpolate;
		double low, high, inc;
		char *file;
		char *list;
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
	gmt_M_str_free (C->T.file);
	gmt_M_str_free (C->T.list);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: makecpt [-A[+]<transparency>] [-C<cpt>|colors] [-D[i|o]] [-E<nlevels>] [-F[R|r|h|c][+c]] [-G<zlo>/<zhi>]\n");
	GMT_Message (API, GMT_TIME_NONE, "	[-I[c][z]] [-L] [-M] [-N] [-Q[i|o]] [-T<z_min>/<z_max>[/<z_inc>[+n]] | -T<table> | -T<z1,z2,...zn>] [%s] [-W[w]]\n\t[-Z] [%s] [%s] [%s]\n\t[%s]\n\n", GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_i_OPT, GMT_ho_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Set constant transparency for all colors; prepend + to also include back-, for-, and nan-colors [0]\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t-I Reverse sense of CPT in one or two ways:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ic Reverse sense of color table as well as back- and foreground color [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Iz Reverse sign of z-values in the color table (takes affect before -G, T are consulted).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Use GMT defaults to set back-, foreground, and NaN colors\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default uses the settings in the color table].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do not write back-, foreground, and NaN colors [Default will].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Assign a logarithmic colortable [Default is linear].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qi: z-values are log10(z). Assign colors and write z [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qo: z-values are z; take log10(z), assign colors and write z.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        If -T<z_min/z_max/z_inc> is given, then z_inc must be 1, 2, or 3\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        (as in logarithmic annotations; see -B in psbasemap).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Give <z_min>/<z_max> to change the z-range for the colorscale in z-units.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append /<z_inc> to sample the cpt discretely instead, or append +n to <z_inc>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to let <z_inc> indicate the number of z-values to produce instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, supply (1) a filename with custom z-values or\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (2) a comma-separated list of custom z-values.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Do not interpolate color palette. Alternatively, append w for a wrapped CPT.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Create a continuous color palette [Default is discontinuous,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i.e., constant color intervals]. Without -T or when using -T<z_min>/<z_max> this\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   has no effect; the input palette table is used untouched with possible scaling.\n");
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
	char txt_a[GMT_LEN32] = {""}, txt_b[GMT_LEN32] = {""};
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

			case 'A':	/* Sets transparency */
				Ctrl->A.active = true;
				if (opt->arg[0] == '+') Ctrl->A.mode = 1;
				Ctrl->A.value = 0.01 * atof (&opt->arg[Ctrl->A.mode]);
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
			case 'T':	/* Sets up color z values */
				Ctrl->T.active = Ctrl->T.interpolate = true;
				if (!gmt_access (GMT, opt->arg, R_OK))
					Ctrl->T.file = strdup (opt->arg);
				else if (strchr (opt->arg, ','))
					Ctrl->T.list = strdup (opt->arg);
				else {
					n = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->T.low, &Ctrl->T.high, &Ctrl->T.inc);
					n_errors += gmt_M_check_condition (GMT, n < 2, "Syntax error -T option: Must specify z_min/z_max[/z_inc[+n]]\n");
					if (n == 3 && (strstr(opt->arg, "+n") || opt->arg[strlen(opt->arg)-1] == '+')) {	/* Gave number of levels instead; calculate inc */
						Ctrl->T.inc = (Ctrl->T.high - Ctrl->T.low) / (Ctrl->T.inc - 1.0);
					}
					if (n == 2) Ctrl->T.interpolate = false;
				}
				break;
			case 'Q':	/* Logarithmic scale */
				Ctrl->Q.active = true;
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

	n_errors += gmt_M_check_condition (GMT, n_files[GMT_IN] > 0 && !Ctrl->E.active,
	                                   "Syntax error: No input files expected unless -E is used\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->Z.active,
	                                   "Syntax error: -W and -Z cannot be used simultaneously\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.cat && Ctrl->Z.active,
	                                   "Syntax error: -F+c and -Z cannot be used simultaneously\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && !(Ctrl->T.file || Ctrl->T.list) && (Ctrl->T.low >= Ctrl->T.high),
	                                   "Syntax error -T option: Give z_min < z_max\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && Ctrl->T.file == NULL && Ctrl->T.list == NULL && Ctrl->T.interpolate && Ctrl->T.inc <= 0.0,
	                                   "Syntax error -T option: For interpolation, give z_inc > 0\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.file && gmt_access (GMT, Ctrl->T.file, R_OK),
	                                   "Syntax error -T option: Cannot access file %s\n", Ctrl->T.file);
	n_errors += gmt_M_check_condition (GMT, n_files[GMT_OUT] > 1, "Syntax error: Only one output destination can be specified\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && (Ctrl->A.value < 0.0 || Ctrl->A.value > 1.0),
	                                   "Syntax error -A: Transparency must be n 0-100 range [0 or opaque]\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && Ctrl->T.active, "Syntax error -E: Cannot be combined with -T\n");
	if (Ctrl->T.active && !Ctrl->T.interpolate && Ctrl->Z.active && (Ctrl->C.file == NULL || strchr (Ctrl->C.file, ',') == NULL)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning -T option: Without z_inc, -Z has no effect (ignored)\n");
		Ctrl->Z.active = false;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_makecpt (void *V_API, int mode, void *args) {
	int i, nz = 0, error = 0;
	unsigned int cpt_flags = 0, pos = 0;

	double *z = NULL;

	char *l = NULL, *kind[2] = {"discrete", "continuous"}, p[GMT_LEN128] = {""};

	struct MAKECPT_CTRL *Ctrl = NULL;
	struct GMT_PALETTE *Pin = NULL, *Pout = NULL;
	struct GMT_DATASET *T = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the makecpt main code ----------------------------*/

	if (Ctrl->C.active) {
		if ((l = strstr (Ctrl->C.file, ".cpt"))) *l = 0;	/* Strip off .cpt if used */
	}
	else {	/* No table specified; set default rainbow table */
		Ctrl->C.active = true;
		Ctrl->C.file = strdup ("rainbow");
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "Prepare CPT via the master file %s\n", Ctrl->C.file);

	/* OK, we can now do the resampling */

	if (Ctrl->M.active) cpt_flags |= GMT_CPT_NO_BNF;	/* bit 0 controls if BFN is determined by parameters */
	if (Ctrl->D.mode == 1) cpt_flags |= GMT_CPT_EXTEND_BNF;	/* bit 1 controls if BF will be set to equal bottom/top rgb value */
	if (Ctrl->Z.active) cpt_flags |= GMT_CPT_CONTINUOUS;	/* Controls if final CPT should be continuous in case input is a list of colors */

	if ((Pin = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, cpt_flags, NULL, Ctrl->C.file, NULL)) == NULL) {
		Return (API->error);
	}
	if (Ctrl->I.mode & GMT_CPT_Z_REVERSE)	/* Must reverse the z-values before anything else */
		gmt_scale_cpt (GMT, Pin, -1.0);
	
	GMT_Report (API, GMT_MSG_VERBOSE, "CPT is %s\n", kind[Pin->is_continuous]);
	if (Ctrl->G.active) {	/* Attempt truncation */
		struct GMT_PALETTE *Ptrunc = gmt_truncate_cpt (GMT, Pin, Ctrl->G.z_low, Ctrl->G.z_high);	/* Possibly truncate the CPT */
		if (Ptrunc == NULL)
			Return (API->error);
		Pin = Ptrunc;
	}

	if (Pin->categorical) Ctrl->W.active = true;	/* Do not want to sample a categorical table */
	if (Ctrl->E.active && Ctrl->E.levels == 0) Ctrl->E.levels = Pin->n_colors + 1;	/* Default number of levels */
	if (Ctrl->W.wrap) Pin->is_wrapping = true;	/* A cyclic CPT has been requested */

	/* Set up arrays */

	if (Ctrl->T.file) {	/* Array passed as a data file */
		if ((T = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->T.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (T->n_tables != 1 || T->table[0]->n_segments != 1) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: More than one table or segment in file %s\n", Ctrl->T.file);
			Return (GMT_RUNTIME_ERROR);
		}
		if (T->table[0]->segment[0]->n_rows == 0) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: No intervals in file %s\n", Ctrl->T.file);
			Return (GMT_RUNTIME_ERROR);
		}
		z = T->table[0]->segment[0]->data[GMT_X];
		nz = (int)T->table[0]->segment[0]->n_rows;
	}
	else if (Ctrl->T.list) {	/* Array passed as a comma-separated list of z-values */
		for (i = 0, nz = 1; i < (int)strlen (Ctrl->T.list); i++) if (Ctrl->T.list[i] == ',') nz++;	/* Count the commas */
		z = gmt_M_memory (GMT, NULL, nz, double);
		i = 0;
		while ((gmt_strtok (Ctrl->T.list, ",", &pos, p)))
			z[i++] = atof (p);
		if (Pin->mode & GMT_CPT_TEMPORARY) {	/* Got -Zcolor,color,... and -Tz,z,z */
			int k;
			extern void gmtlib_init_cpt (struct GMT_CTRL *GMT, struct GMT_PALETTE *P);
			if (nz != (int)(Pin->n_colors + 1)) {
				GMT_Report (API, GMT_MSG_NORMAL, "Error: Mistmatch between number of entries in color list and z list\n");
				gmt_M_free (GMT, z);
				Return (GMT_RUNTIME_ERROR);
			}
			if ((Pout = GMT_Duplicate_Data (API, GMT_IS_PALETTE, GMT_DUPLICATE_ALLOC, Pin)) == NULL) return (API->error);
			for (i = k = 0; i < (int)Pout->n_colors; i++) {
				Pout->data[i].z_low = z[k];
				Pout->data[i].z_high = z[++k];
			}
			gmtlib_init_cpt (GMT, Pin);	/* Recalculate delta rgb's */
			if (Ctrl->I.mode & GMT_CPT_C_REVERSE)	/* Also flip the colors */
				gmt_invert_cpt (GMT, Pout);
		}
	}
	else if (Ctrl->T.active && Ctrl->Q.mode == 2) {	/* Establish a log10 grid */
		if (!(Ctrl->T.inc == 1.0 || Ctrl->T.inc == 2.0 || Ctrl->T.inc == 3.0)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: For -Qo logarithmic spacing, z_inc must be 1, 2, or 3\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (Ctrl->T.low <= 0.0) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: For -Qo logarithmic spacing, z_start must be > 0\n");
			Return (GMT_RUNTIME_ERROR);
		}
		nz = gmtlib_log_array (GMT, Ctrl->T.low, Ctrl->T.high, Ctrl->T.inc, &z);
	}
	else if (Ctrl->T.active && Ctrl->T.interpolate) {	/* Establish discrete linear grid */
		nz = irint ((Ctrl->T.high - Ctrl->T.low) / Ctrl->T.inc) + 1;
		z = gmt_M_memory (GMT, NULL, nz, double);
		for (i = 0; i < nz; i++) z[i] = Ctrl->T.low + i * Ctrl->T.inc;	/* Desired z values */
	}
	else if (Ctrl->E.active) {
		struct GMT_DATASET *D = NULL;
		double inc;
		unsigned int col;
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
			Return (API->error);
		}
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
			Return (API->error);
		}
		col = (unsigned int)(D->n_columns - 1);	/* Use the last column of the input as z */
		nz = Ctrl->E.levels;
		inc = (D->max[col] - D->min[col]) / (nz - 1);
		z = gmt_M_memory (GMT, NULL, nz, double);
		for (i = 0; i < nz; i++) z[i] = D->min[col] + i * inc;	/* Desired z values */
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			gmt_M_free (GMT, z);
			Return (API->error);
		}
	}
	else {	/* Just copy what was in the CPT */
		if ((Pout = GMT_Duplicate_Data (API, GMT_IS_PALETTE, GMT_DUPLICATE_ALLOC, Pin)) == NULL) return (API->error);
		gmt_stretch_cpt (GMT, Pout, Ctrl->T.low, Ctrl->T.high);	/* Stretch to given range or use natural range if 0/0 */
		if (Ctrl->I.mode & GMT_CPT_C_REVERSE)	/* Also flip the colors */
			gmt_invert_cpt (GMT, Pout);
	}

	if (Pout == NULL) {	/* Meaning it was not created above */
		if (Ctrl->Q.mode == 2)
			for (i = 0; i < nz; i++) z[i] = d_log10 (GMT, z[i]);	/* Make log10(z) values for interpolation step */

		/* Now we can resample the CPT and write out the result */

		Pout = gmt_sample_cpt (GMT, Pin, z, nz, Ctrl->Z.active, Ctrl->I.mode & GMT_CPT_C_REVERSE, Ctrl->Q.mode, Ctrl->W.active);
	}

	if (!Ctrl->T.file) gmt_M_free (GMT, z);	/* It may also have been allocated inside gmtlib_log_array() */

	if (Ctrl->A.active) gmt_cpt_transparency (GMT, Pout, Ctrl->A.value, Ctrl->A.mode);	/* Set transparency */

	/* Determine mode flags for output */
	cpt_flags = 0;
	if (Ctrl->N.active) cpt_flags |= GMT_CPT_NO_BNF;	/* bit 0 controls if BFN will be written out */
	if (Ctrl->D.mode == 1) cpt_flags |= GMT_CPT_EXTEND_BNF;	/* bit 1 controls if BF will be set to equal bottom/top rgb value */
	if (Ctrl->F.active) Pout->model = Ctrl->F.model;
	if (Ctrl->F.cat) Pout->categorical = 1;

	if (GMT_Write_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, cpt_flags, NULL, Ctrl->Out.file, Pout) != GMT_NOERROR) {
		Return (API->error);
	}

	Return (GMT_NOERROR);
}
