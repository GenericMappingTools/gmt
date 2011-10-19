/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 *
 * Authors:	Walter H.F. Smith & P. Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: Reads an existing cpt table and desired output grid
 * and produces a GMT cpt file.  Can be inverted [-I] or made to be
 * continuous [-Z].  Discrete color jumps in cpt tables are handled
 * correctly.  Default color table is "rainbow".
 *
 */

#include "gmt.h"

EXTERN_MSC GMT_LONG GMT_log_array (struct GMT_CTRL *C, double min, double max, double delta, double **array);

/* Control structure for makecpt */

struct MAKECPT_CTRL {
	struct Out {	/* -> */
		GMT_LONG active;
		char *file;
	} Out;
	struct A {	/* -A+ */
		GMT_LONG active;
		GMT_LONG mode;
		double value;
	} A;
	struct C {	/* -C<cpt> */
		GMT_LONG active;
		char *file;
	} C;
	struct D {	/* -D[i|o] */
		GMT_LONG active;
		GMT_LONG mode;
	} D;
	struct F {	/* -F[r|R|h|c] */
		GMT_LONG active;
		GMT_LONG model;
	} F;
	struct I {	/* -I */
		GMT_LONG active;
	} I;
	struct M {	/* -M */
		GMT_LONG active;
	} M;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct T {	/* -T<z0/z1/dz> */
		GMT_LONG active;
		GMT_LONG cpt;
		double low, high, inc;
		char *file;
	} T;
	struct Q {	/* -Q[i|o] */
		GMT_LONG active;
		GMT_LONG mode;
	} Q;
	struct W {	/* -W */
		GMT_LONG active;
	} W;
	struct Z {	/* -Z */
		GMT_LONG active;
	} Z;
};

void *New_makecpt_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MAKECPT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct MAKECPT_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */
	return (C);
}

void Free_makecpt_Ctrl (struct GMT_CTRL *GMT, struct MAKECPT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free (C->Out.file);
	if (C->C.file) free (C->C.file);
	if (C->T.file) free (C->T.file);
	GMT_free (GMT, C);
}

GMT_LONG GMT_makecpt_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "makecpt %s [API] - Make GMT color palette tables\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: makecpt [-A[+]<transparency>] [-C<cpt>] [-D[i|o]] [-F[R|r|h|c] [-I] [-M] [-N] [-Q[i|o]]\n");
	GMT_message (GMT, "	[-T<z0>/<z1>/<dz> | -T<table>] [%s] [-Z]\n", GMT_V_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Set constant transparency for all colors; prepend + to also include back-, for-, and nan-colors [0]\n");
	if (GMT_list_cpt (GMT, 'C')) return (EXIT_FAILURE);	/* Display list of available color tables */
	GMT_message (GMT, "\t-D Set back- and foreground color to match the bottom/top limits\n");
	GMT_message (GMT, "\t   in the output cpt file [Default uses color table]. Append i to match the\n");
	GMT_message (GMT, "\t   bottom/top values in the input cpt file.\n");
	GMT_message (GMT, "\t-F Select the color model for output (R for r/g/b or grayscale or colorname,\n");
	GMT_message (GMT, "\t   r for r/g/b only, h for h-s-v, c for c/m/y/k).\n");
	GMT_message (GMT, "\t-I Reverse sense of color table as well as back- and foreground color.\n");
	GMT_message (GMT, "\t-M Use GMT defaults to set back-, foreground, and NaN colors\n");
	GMT_message (GMT, "\t   [Default uses the settings in the color table].\n");
	GMT_message (GMT, "\t-N Do not write back-, foreground, and NaN colors [Default will].\n");
	GMT_message (GMT, "\t-Q Assign a logarithmic colortable [Default is linear].\n");
	GMT_message (GMT, "\t   -Qi: z-values are log10(z). Assign colors and write z [Default].\n");
	GMT_message (GMT, "\t   -Qo: z-values are z; take log10(z), assign colors and write z.\n");
	GMT_message (GMT, "\t        If -T<z0/z1/dz> is given, then dz must be 1, 2, or 3\n");
	GMT_message (GMT, "\t        (as in logarithmic annotations; see -B in psbasemap).\n");
	GMT_message (GMT, "\t-T Give start, stop, and increment for colorscale in z-units,\n");
	GMT_message (GMT, "\t   or filename with custom z-values.  If no -T option is given,\n");
	GMT_message (GMT, "\t   then the range in the master cptfile will be used.\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Do not interpolate color palette.\n");
	GMT_message (GMT, "\t-Z Create a continuous color palette [Default is discontinuous,\n");
	GMT_message (GMT, "\t   i.e., constant color intervals].\n");
	GMT_explain_options (GMT, ".");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_makecpt_parse (struct GMTAPI_CTRL *C, struct MAKECPT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to makecpt and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files[2] = {0, 0};
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

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
				Ctrl->A.active = TRUE;
				if (opt->arg[0] == '+') Ctrl->A.mode = 1;
				Ctrl->A.value = 0.01 * atof (&opt->arg[Ctrl->A.mode]);
				break;
			case 'C':	/* CTP table */
				Ctrl->C.active = TRUE;
				Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Set BNF to match cpt ends */
				Ctrl->D.active = TRUE;
				Ctrl->D.mode = 1;
				if (opt->arg[0] == 'i') Ctrl->D.mode = 2;
				break;
			case 'F':	/* Sets format for color reporting */
				Ctrl->F.active = TRUE;
				switch (opt->arg[0]) {
					case 'r': Ctrl->F.model = GMT_RGB + GMT_NO_COLORNAMES; break;
					case 'h': Ctrl->F.model = GMT_HSV; break;
					case 'c': Ctrl->F.model = GMT_CMYK; break;
					default: Ctrl->F.model = GMT_RGB; break;
				}
				break;
			case 'I':	/* Invert table */
				Ctrl->I.active = TRUE;
				break;
			case 'M':	/* Use GMT defaults for BNF colors */
				Ctrl->M.active = TRUE;
				break;
			case 'N':	/* Do not output BNF colors */
				Ctrl->N.active = TRUE;
				break;
			case 'T':	/* Sets up color z values */
				Ctrl->T.active = TRUE;
				if (!access (opt->arg, R_OK))
					Ctrl->T.file = strdup (opt->arg);
				else {
					GMT_LONG n;
					Ctrl->T.inc = 0.0;
					n = sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->T.low, &Ctrl->T.high, &Ctrl->T.inc);
					n_errors += GMT_check_condition (GMT, n < 2, "Syntax error -T option: Must specify start/stop[/inc]\n");
				}
				break;
			case 'Q':	/* Logarithmic scale */
				Ctrl->Q.active = TRUE;
				if (opt->arg[0] == 'o')	/* Input data is z, but take log10(z) before interpolation colors */
					Ctrl->Q.mode = 2;
				else			/* Input is log10(z) */
					Ctrl->Q.mode = 1;
				break;
			case 'W':	/* Do not interpolate colors */
				Ctrl->W.active = TRUE;
				break;
			case 'Z':	/* Continuous colors */
				Ctrl->Z.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files[GMT_IN] > 0, "Syntax error: No input files expected\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->Z.active, "Syntax error: -W and -Z cannot be used simultaneously\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && !Ctrl->T.file && (Ctrl->T.low >= Ctrl->T.high || Ctrl->T.inc < 0.0), "Syntax error -T option: Give start < stop and inc > 0\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.file && GMT_access (GMT, Ctrl->T.file, R_OK), "Syntax error -T option: Cannot access file %s\n", Ctrl->T.file);
	n_errors += GMT_check_condition (GMT, n_files[GMT_OUT] > 1, "Syntax error: Only one output destination can be specified\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && (Ctrl->A.value < 0.0 || Ctrl->A.value > 1.0), "Syntax error -A: Transparency must be n 0-100 range [0 or opaque]\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_makecpt_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_makecpt (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG i, nz, cpt_flags = 0, error = 0;

	double *z = NULL;

	char CPT_file[GMT_BUFSIZ], *file = NULL, *l = NULL;

	struct MAKECPT_CTRL *Ctrl = NULL;
	struct GMT_PALETTE *Pin = NULL, *Pout = NULL;
	struct GMT_DATASET *T = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_makecpt_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_makecpt_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_makecpt", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-V", ">", options))) Return (error);
	Ctrl = (struct MAKECPT_CTRL *) New_makecpt_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_makecpt_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the makecpt main code ----------------------------*/

	if (Ctrl->C.active) {
		if ((l = strstr (Ctrl->C.file, ".cpt"))) *l = 0;	/* Strip off .cpt if used */
	}
	else {	/* No table specified; set default rainbow table */
		Ctrl->C.active = TRUE;
		Ctrl->C.file = strdup ("rainbow");
	}

	error += GMT_check_condition (GMT, !GMT_getsharepath (GMT, "cpt", Ctrl->C.file, ".cpt", CPT_file), "Error: Cannot find colortable %s\n", Ctrl->C.file);
	if (error) Return (GMT_RUNTIME_ERROR);	/* Bail on run-time errors */

	/* OK, we can now do the resampling */

	if (Ctrl->M.active) cpt_flags |= 1;	/* bit 0 controls if BFN is determined by parameters */
	if (Ctrl->D.mode == 1) cpt_flags |= 2;	/* bit 1 controls if BF will be set to equal bottom/top rgb value */

	file = CPT_file;

	/* if ((error = GMT_Init_IO (API, GMT_IS_CPT, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error); */	/* Establishes cpt output [stdout] */
	if ((error = GMT_Begin_IO (API, 0, GMT_IN,  GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if ((error = GMT_Begin_IO (API, 0, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */

	if ((error = GMT_Get_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, cpt_flags, file, &Pin))) Return (error);
	if (Pin->categorical) Ctrl->W.active = TRUE;	/* Do not want to sample a categorical table */

	/* Set up arrays */

	if (Ctrl->T.file) {	/* Array passed as a data file */
		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, Ctrl->T.file, &T)) Return (GMT_DATA_READ_ERROR);
		if (T->n_tables != 1 || T->table[0]->n_segments != 1) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: More than one table or segment in file %s\n", Ctrl->T.file);
			Return (GMT_RUNTIME_ERROR);
		}
		if (T->table[0]->segment[0]->n_rows == 0) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: No intervals in file %s\n", Ctrl->T.file);
			Return (GMT_RUNTIME_ERROR);
		}
		z = T->table[0]->segment[0]->coord[GMT_X];
		nz = T->table[0]->segment[0]->n_rows;
	}
	else if (Ctrl->T.active && Ctrl->Q.mode == 2) {	/* Establish a log10 grid */
		if (!(Ctrl->T.inc == 1.0 || Ctrl->T.inc == 2.0 || Ctrl->T.inc == 3.0)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: For -Qo logarithmic spacing, dz must be 1, 2, or 3\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (Ctrl->T.low <= 0.0) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: For -Qo logarithmic spacing, z_start must be > 0\n");
			Return (GMT_RUNTIME_ERROR);
		}
		nz = GMT_log_array (GMT, Ctrl->T.low, Ctrl->T.high, Ctrl->T.inc, &z);
	}
	else if (Ctrl->T.active) {	/* Establish linear grid */
		if (Ctrl->T.inc == 0 && Ctrl->C.active) {	/* Compute interval from number of colors in palette */
			nz = Pin->n_colors + 1;
			Ctrl->T.inc = (Ctrl->T.high - Ctrl->T.low) / Pin->n_colors;
		}
		else if (Ctrl->T.inc <= 0) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: Interval must be > 0\n");
			Return (GMT_RUNTIME_ERROR);
		}
		else
			nz = irint ((Ctrl->T.high - Ctrl->T.low) / Ctrl->T.inc) + 1;

		z = GMT_memory (GMT, NULL, nz, double);
		for (i = 0; i < nz; i++) z[i] = Ctrl->T.low + i * Ctrl->T.inc;	/* Desired z values */
	}
	else {	/* Just copy what was in the cpt file */
		nz = Pin->n_colors + 1;
		z = GMT_memory (GMT, NULL, nz, double);
		if (Ctrl->I.active) {
			/* Reverse the intervals (only relavant for non-equidistant color maps) */
			for (i = 0; i < nz-1; i++) z[i] = Pin->range[0].z_low + Pin->range[Pin->n_colors-1].z_high - Pin->range[Pin->n_colors-1-i].z_high;
		}
		else {
			for (i = 0; i < nz-1; i++) z[i] = Pin->range[i].z_low;
		}
		z[i] = Pin->range[i-1].z_high;
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (Ctrl->Q.mode == 2) for (i = 0; i < nz; i++) z[i] = d_log10 (GMT, z[i]);	/* Make log10(z) values for interpolation step */

	/* Now we can resample the cpt table and write out the result */

	GMT_sample_cpt (GMT, Pin, z, nz, Ctrl->Z.active, Ctrl->I.active, Ctrl->Q.mode, Ctrl->W.active, &Pout);

	if (!Ctrl->T.file) GMT_free (GMT, z);

	if (Ctrl->A.active) GMT_cpt_transparency (GMT, Pout, Ctrl->A.value, Ctrl->A.mode);	/* Set transparency */

	/* Determine mode flags for output */
	cpt_flags = 0;
	if (Ctrl->N.active) cpt_flags |= 1;	/* bit 0 controls if BFN will be written out */
	if (Ctrl->D.mode == 1) cpt_flags |= 2;	/* bit 1 controls if BF will be set to equal bottom/top rgb value */
	if (Ctrl->F.active) Pout->model = Ctrl->F.model;

	if (GMT_Put_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, cpt_flags, Ctrl->Out.file, Pout)) Return (GMT_DATA_WRITE_ERROR);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	Return (GMT_OK);
}
