/*--------------------------------------------------------------------
 *	$Id: grd2cpt_func.c,v 1.2 2011-03-15 02:06:36 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Brief synopsis: grd2cpt reads a 2d binary gridded grdfile and creates a continuous-color-
 * palette cpt file, with a non-linear histogram-equalized mapping between
 * hue and data value.  (The linear mapping can be made with grd2cpt.)
 *
 * Creates a cumulative distribution function f(z) describing the data
 * in the grdfile.  f(z) is sampled at z values supplied by the user
 * [with -S option] or guessed from the sample mean and standard deviation.
 * f(z) is then found by looping over the grd array for each z and counting
 * data values <= z.  Once f(z) is found then a master cpt table is resampled
 * based on a normalized f(z).
 *
 * Author:	Walter H. F. Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 */

#include "gmt.h"

struct GRD2CPT_CTRL {
	struct In {
		GMT_LONG active;
	} In;
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
	struct E {	/* -E<nlevels> */
		GMT_LONG active;
		GMT_LONG levels;
	} E;
	struct F {	/* -F[R|r|h|c] */
		GMT_LONG active;
		GMT_LONG model;
	} F;
	struct I {	/* -I */
		GMT_LONG active;
	} I;
	struct L {	/* -L<min_limit>/<max_limit> */
		GMT_LONG active;
		double min, max;
	} L;
	struct M {	/* -M */
		GMT_LONG active;
	} M;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct Q {	/* -Q[i|o */
		GMT_LONG active;
		GMT_LONG mode;
	} Q;
	struct S {	/* -S<z_start>/<z_stop>/<z_inc> */
		GMT_LONG active;
		GMT_LONG cpt;
		double low, high, inc;
		char *file;
	} S;
	struct T {	/* -T<kind> */
		GMT_LONG active;
		GMT_LONG kind; /* -1 symmetric +-zmin, +1 +-zmax, -2 = +-Minx(|zmin|,|zmax|), +2 = +-Max(|zmin|,|zmax|), 0 = min to max [Default] */
	} T;
	struct W {	/* -W */
		GMT_LONG active;
	} W;
	struct Z {	/* -Z */
		GMT_LONG active;
	} Z;
};

void *New_grd2cpt_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRD2CPT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GRD2CPT_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */
	return ((void *)C);
}

void Free_grd2cpt_Ctrl (struct GMT_CTRL *GMT, struct GRD2CPT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free ((void *)C->Out.file);
	if (C->C.file) free ((void *)C->C.file);
	if (C->S.file) free ((void *)C->S.file);
	GMT_free (GMT, C);
}

GMT_LONG GMT_grd2cpt_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grd2cpt %s [API] - Make a linear or histogram-equalized color palette table from a grdfile\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grd2cpt <grdfiles> [-A[+]<transparency>] [-C<table>] [-D[i|o]] [-F[R|r|h|c] [-E<nlevels> [-I] [-L<min_limit>/<max_limit>]\n");
	GMT_message (GMT, "\t[-M] [-N] [-Q[i|o]] [%s] [-S<z_start>/<z_stop>/<z_inc>] [-T<-|+|=|_>] [%s] [-Z]\n", GMT_Rgeo_OPT, GMT_V_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<grdfiles> names of one or more 2-D binary data sets\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Set constant transparency for all colors; prepend + to also include back-, for-, and nan-colors [0]\n");
	if (GMT_list_cpt (GMT, 'C')) return (EXIT_FAILURE);	/* Display list of available color tables */
	GMT_message (GMT, "\t-D Set back- and foreground color to match the bottom/top limits\n");
	GMT_message (GMT, "\t   in the output cpt file [Default uses color table]. Append i to match the\n");
	GMT_message (GMT, "\t   bottom/top values in the input cpt file.\n");
	GMT_message (GMT, "\t-E Use <nlevels> equidistant color levels from zmin to zmax.\n");
	GMT_message (GMT, "\t-F Select the color model for output (R for r/g/b or grayscale or colorname,\n");
	GMT_message (GMT, "\t   r for r/g/b only, h for h-s-v, c for c/m/y/k)\n");
	GMT_message (GMT, "\t-I Reverses the sense of the color table as well as back- and foreground color.\n");
	GMT_message (GMT, "\t-L Limit the range of the data [Default uses actual min,max of data].\n");
	GMT_message (GMT, "\t-M Use GMT defaults to set back-, foreground, and NaN colors [Default uses color table].\n");
	GMT_message (GMT, "\t-N Do not write back-, foreground, and NaN colors [Default will].\n");
	GMT_message (GMT, "\t-Q Assign a logarithmic colortable [Default is linear]\n");
	GMT_message (GMT, "\t   -Qi: z-values are actually log10(z). Assign colors and write z. [Default]\n");
	GMT_message (GMT, "\t   -Qo: z-values are z, but take log10(z), assign colors and write z.\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-S Sample points should Step from z_start to z_stop by z_inc [Default guesses some values].\n");
	GMT_message (GMT, "\t-T Force color tables to be symmetric about 0. Append one modifier:\n");
	GMT_message (GMT, "\t   - for values symmetric about zero from -|zmin| to +|zmin|\n");
	GMT_message (GMT, "\t   + for values symmetric about zero from -|zmax| to +|zmax|\n");
	GMT_message (GMT, "\t   _ for values symmetric about zero -+min(|zmin|,|zmax|)\n");
	GMT_message (GMT, "\t   = for values symmetric about zero -+max(|zmin|,|zmax|)\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Do not interpolate color paletter\n");
	GMT_message (GMT, "\t-Z Create a continuous color palette [Default is discontinuous, i.e., constant color intervals]\n");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_grd2cpt_parse (struct GMTAPI_CTRL *C, struct GRD2CPT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files[2] = {0, 0};
	char kind;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				Ctrl->In.active = TRUE;
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
			case 'C':	/* Get cpt table */
				Ctrl->C.active = TRUE;
				Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Set fore/back-ground to match end-colors */
				Ctrl->D.active = TRUE;
				Ctrl->D.mode = 1;
				if (opt->arg[0] == 'i') Ctrl->D.mode = 2;
				break;
			case 'E':	/* Use n levels */
				Ctrl->E.active = TRUE;
				if (sscanf (opt->arg, "%ld", &Ctrl->E.levels) != 1) {
					GMT_report (GMT, GMT_MSG_FATAL, "GMT SYNTAX ERROR -E option:  Cannot decode value\n");
					n_errors++;
				}
				break;
			case 'F':	/* Set color model for output */
				Ctrl->F.active = TRUE;
				switch (opt->arg[0]) {
					case 'r': Ctrl->F.model = GMT_RGB + GMT_NO_COLORNAMES; break;
					case 'h': Ctrl->F.model = GMT_HSV; break;
					case 'c': Ctrl->F.model = GMT_CMYK; break;
					default:  Ctrl->F.model = GMT_RGB; break;
				}
				break;
			case 'I':	/* Reverse scale */
				Ctrl->I.active = TRUE;
				break;
			case 'L':	/* Limit data range */
				Ctrl->L.active = TRUE;
				if (sscanf (opt->arg, "%lf/%lf", &Ctrl->L.min, &Ctrl->L.max) != 2) {
					GMT_report (GMT, GMT_MSG_FATAL, "GMT SYNTAX ERROR -L option:  Cannot decode limits\n");
					n_errors++;
				}
				break;
			case 'M':	/* Override fore/back/NaN using GMT defaults */
				Ctrl->M.active = TRUE;
				break;
			case 'N':	/* Do not write F/B/N colors */
				Ctrl->N.active = TRUE;
				break;
			case 'Q':	/* Logarithmic data */
				Ctrl->Q.active = TRUE;
				if (opt->arg[0] == 'o')	/* Input data is z, but take log10(z) before interpolation colors */
					Ctrl->Q.mode = 2;
				else			/* Input is log10(z) */
					Ctrl->Q.mode = 1;
				break;
			case 'S':	/* Sets sample range */
				Ctrl->S.active = TRUE;
				if (sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->S.low, &Ctrl->S.high, &Ctrl->S.inc) != 3) {
					GMT_report (GMT, GMT_MSG_FATAL, "GMT SYNTAX ERROR -S option:  Cannot decode values\n");
					n_errors++;
				}
				break;
			case 'T':	/* Force symmetry */
				Ctrl->T.active = TRUE;
				kind = '\0';
				if (sscanf (opt->arg, "%c", &kind) != 1) {
					GMT_report (GMT, GMT_MSG_FATAL, "GMT SYNTAX ERROR -T option:  Cannot decode option\n");
					n_errors++;
				}
				switch (kind) {
					case '-': Ctrl->T.kind = -1; break; /* Symmetric with |zmin| range */
					case '+': Ctrl->T.kind = +1; break; /* Symmetric with |zmax| range */
					case '_': Ctrl->T.kind = -2; break; /* Symmetric with min(|zmin|,|zmax|) range */
					case '=': Ctrl->T.kind = +2; break; /* Symmetric with max(|zmin|,|zmax|) range */
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "GMT SYNTAX ERROR -T option:  Must append modifier -, +, _, or =\n");
						n_errors++;
						break;
				}
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

	n_errors += GMT_check_condition (GMT, n_files[GMT_IN] < 1, "GMT USAGE ERROR:  No grid name(s) specified.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->Z.active, "GMT SYNTAX ERROR:  -W and -Z cannot be used simultaneously\n");
	n_errors += GMT_check_condition (GMT, Ctrl->L.active && Ctrl->L.min >= Ctrl->L.max, "GMT SYNTAX ERROR -L option:  min_limit must be less than max_limit.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && (Ctrl->S.high <= Ctrl->S.low || Ctrl->S.inc <= 0.0),
		"GMT SYNTAX ERROR -S option:  Bad arguments\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.active && (Ctrl->T.active || Ctrl->E.active),
		"GMT USAGE ERROR -S option:  Cannot be combined with -E nor -T option.\n");
	n_errors += GMT_check_condition (GMT, n_files[GMT_OUT] > 1, "GMT SYNTAX ERROR:  Only one output destination can be specified\n");
	n_errors += GMT_check_condition (GMT, Ctrl->A.active && (Ctrl->A.value < 0.0 || Ctrl->A.value > 1.0), "GMT SYNTAX ERROR -A:  Transparency must be n 0-100 range [0 or opaque]\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grd2cpt_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grd2cpt (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG row, col, j, k, ij, ngrd = 0, nxyg, nfound, ngood, cpt_flags = 0;
	GMT_LONG n_alloc = GMT_TINY_CHUNK, error = FALSE;

	char CPT_file[BUFSIZ], format[BUFSIZ], *file = NULL, **grdfile = NULL;

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

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grd2cpt_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grd2cpt_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grd2cpt", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VR", ">", options))) Return (error);
	Ctrl = (struct GRD2CPT_CTRL *) New_grd2cpt_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grd2cpt_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grd2cpt main code ----------------------------*/

	if (!Ctrl->C.active) {	/* Must assign default table [rainbow] */
		Ctrl->C.active = TRUE;
		Ctrl->C.file = strdup ("rainbow");
	}

	/* Open the specified master color table */

	error += GMT_check_condition (GMT, GMT_set_cpt_path (GMT, CPT_file, Ctrl->C.file), "Error: Could not open file %s\n", Ctrl->C.file);
	if (error) Return (GMT_RUNTIME_ERROR);	/* Bail on run-time errors */

	if (!Ctrl->E.active) Ctrl->E.levels = 11;	/* Default number of levels */
	if (Ctrl->M.active) cpt_flags |= 1;		/* bit 0 controls if BFN is determined by parameters */
	if (Ctrl->D.mode == 2) cpt_flags |= 2;		/* bit 1 controls if BF will be set to equal bottom/top rgb value */

	file = CPT_file;
	if ((error = GMT_Begin_IO (API, 0, GMT_IN, GMT_BY_SET))) Return (error);				/* Enables data input and sets access mode */
	if ((error = GMT_Get_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, cpt_flags, (void **)&file, (void **)&Pin))) Return (error);
	if ((error = GMT_Init_IO (API, GMT_IS_CPT, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Registers default output destination, unless already set */

	GMT_memset (wesn, 4, double);
	if (GMT->common.R.active) GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Subset */

	G = GMT_memory (GMT, NULL, n_alloc, struct GMT_GRID *);	/* Potentially an array of grids */
	grdfile = GMT_memory (GMT, NULL, n_alloc, char *);	/* Potentially an array of gridfile names */

	for (opt = options, k = 0; opt; opt = opt->next) {
		if (opt->option != '<') continue;	/* We are only processing input files here */

		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_ALL, (void **)&opt->arg, (void **)&G[k])) Return (GMT_DATA_READ_ERROR);
		grdfile[k] = strdup (opt->arg);
		if (k && !(G[k]->header->nx == G[k-1]->header->nx && G[k]->header->ny == G[k-1]->header->ny)) {
			GMT_report (GMT, GMT_MSG_FATAL, "GMT ERROR: Grids do not have the same domain!\n");
			while (k >= 0) GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&G[k--]);
			Return (GMT_RUNTIME_ERROR);
		}

		k++;
		if (k == n_alloc) {
			n_alloc += GMT_TINY_CHUNK;
			G = GMT_memory (GMT, (void *)G, n_alloc, struct GMT_GRID *);
			grdfile = GMT_memory (GMT, (void *)grdfile, n_alloc, char *);
		}
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);				/* Disables further data input */

	ngrd = k;
	if (ngrd < n_alloc) G = GMT_memory (GMT, (void *)G, ngrd, struct GMT_GRID *);

	nxyg = G[0]->header->nm * ngrd;

	/* Loop over the files and find NaNs.  If set limits, may create more NaNs  */
	/* We use the G[0]->header to keep limits representing all the grids */

	nfound = 0;
	mean = sd = 0.0;
	if (Ctrl->L.active) {	/* Loop over the grdfiles, and set anything outside the limiting values to NaN.  */
		G[0]->header->z_min = Ctrl->L.min;
		G[0]->header->z_max = Ctrl->L.max;
		for (k = 0; k < ngrd; k++) {	/* For each grid */
			GMT_grd_loop (G[k], row, col, ij) {
				if (GMT_is_fnan (G[k]->data[ij]))
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
			GMT_grd_loop (G[k], row, col, ij) {
				if (GMT_is_fnan (G[k]->data[ij]))
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
	ngood = nxyg - nfound;	/* This is the number of non-NaN points for the cdf function  */
	mean /= ngood;
	sd /= ngood;
	sd = sqrt (sd - mean * mean);
	if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) {
		sprintf (format, "Mean and S.D. of data are %s %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, format, mean, sd);
	}

	/* Decide how to make steps in z.  */
	if (Ctrl->S.active) {	/* Use predefined levels and interval */
		GMT_LONG i, j;

		Ctrl->E.levels = (G[0]->header->z_min < Ctrl->S.low) ? 1 : 0;
		Ctrl->E.levels += (GMT_LONG)floor((Ctrl->S.high - Ctrl->S.low)/Ctrl->S.inc) + 1;
		if (G[0]->header->z_max > Ctrl->S.high) Ctrl->E.levels++;
		cdf_cpt = GMT_memory (GMT, NULL, Ctrl->E.levels, struct CDF_CPT);
		if (G[0]->header->z_min < Ctrl->S.low) {
			cdf_cpt[0].z = G[0]->header->z_min;
			cdf_cpt[1].z = Ctrl->S.low;
			i = 2;
		}
		else {
			cdf_cpt[0].z = Ctrl->S.low;
			i = 1;
		}
		j = (G[0]->header->z_max > Ctrl->S.high) ? Ctrl->E.levels - 1 : Ctrl->E.levels;
		while (i < j) {
			cdf_cpt[i].z = cdf_cpt[i-1].z + Ctrl->S.inc;
			i++;
		}
		if (j == Ctrl->E.levels-1) cdf_cpt[j].z = G[0]->header->z_max;
	}
	else if (Ctrl->T.active || Ctrl->E.active) {	/* Make a equaldistant color map from G[k]->header->z_min to G[k]->header->z_max */
		double start, range;

		switch (Ctrl->T.kind) {
			case -1:
				start = -fabs ((double)G[0]->header->z_min);
				break;
			case 1:
				start = -fabs ((double)G[0]->header->z_max);
				break;
			case -2:
				start = -MIN (fabs ((double)G[0]->header->z_min), fabs ((double)G[0]->header->z_max));
				break;
			case 2:
				start = -MAX (fabs ((double)G[0]->header->z_min), fabs ((double)G[0]->header->z_max));
				break;
			default:
				start = G[0]->header->z_min;
				break;
		}
		range = (Ctrl->T.kind) ? 2.0 * fabs (start) : G[0]->header->z_max - G[0]->header->z_min;
		Ctrl->S.inc = range / (double)(Ctrl->E.levels - 1);
		cdf_cpt = GMT_memory (GMT, NULL, Ctrl->E.levels, struct CDF_CPT);
		for (k = 0; k < Ctrl->E.levels; k++) cdf_cpt[k].z = start + k * Ctrl->S.inc;
	}

	else {	/* This is completely ad-hoc.  It chooses z based on steps of 0.1 for a Gaussian CDF:  */
		cdf_cpt = GMT_memory (GMT, NULL, Ctrl->E.levels, struct CDF_CPT);
		if ((mean - 1.28155*sd) <= G[0]->header->z_min || (mean + 1.28155*sd) >= G[0]->header->z_max) {
			mean = 0.5 * (G[0]->header->z_min + G[0]->header->z_max);
			sd = (G[0]->header->z_max - mean) / 1.5;
			if (sd <= 0.0) {
				GMT_report (GMT, GMT_MSG_FATAL, " ERROR.  Min and Max data values are equal.\n");
				Return (EXIT_FAILURE);
			}
		}	/* End of stupid bug fix  */

		cdf_cpt[0].z = G[0]->header->z_min;
		cdf_cpt[1].z = mean - 1.28155 * sd;
		cdf_cpt[2].z = mean - 0.84162 * sd;
		cdf_cpt[3].z = mean - 0.52440 * sd;
		cdf_cpt[4].z = mean - 0.25335 * sd;
		cdf_cpt[5].z = mean;
		cdf_cpt[6].z = mean + 0.25335 * sd;
		cdf_cpt[7].z = mean + 0.52440 * sd;
		cdf_cpt[8].z = mean + 0.84162 * sd;
		cdf_cpt[9].z = mean + 1.28155 * sd;
		cdf_cpt[10].z = G[0]->header->z_max;
	}

	/* Get here when we are ready to go.  cdf_cpt[].z contains the sample points.  */

	if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) sprintf (format, "z = %s and CDF(z) = %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	for (j = 0; j < Ctrl->E.levels; j++) {
		if (cdf_cpt[j].z <= G[0]->header->z_min)
			cdf_cpt[j].f = 0.0;
		else if (cdf_cpt[j].z >= G[0]->header->z_max)
			cdf_cpt[j].f = 1.0;
		else {
			nfound = 0;
			for (k = 0; k < ngrd; k++) {	/* For each grid */
				GMT_grd_loop (G[k], row, col, ij) {
					if (!GMT_is_fnan (G[k]->data[ij]) && G[k]->data[ij] <= cdf_cpt[j].z) nfound++;
				}
			}
			cdf_cpt[j].f = (double)(nfound-1)/(double)(ngood-1);
		}
		GMT_report (GMT, GMT_MSG_NORMAL, format, cdf_cpt[j].z, cdf_cpt[j].f);
	}

	/* Now the cdf function has been found.  We now resample the chosen cptfile  */

	z = GMT_memory (GMT, NULL, Ctrl->E.levels, double);
	for (k = 0; k < Ctrl->E.levels; k++) z[k] = cdf_cpt[k].z;
	if (Ctrl->Q.mode == 2) for (k = 0; k < Ctrl->E.levels; k++) z[k] = d_log10 (GMT, z[k]);	/* Make log10(z) values for interpolation step */

	GMT_sample_cpt (GMT, Pin, z, -Ctrl->E.levels, Ctrl->Z.active, Ctrl->I.active, Ctrl->Q.mode, Ctrl->W.active, &Pout);	/* -ve to keep original colors */
	if ((error = GMT_Begin_IO (API, 0, GMT_OUT, GMT_BY_SET))) Return (error);				/* Enables data output and sets access mode */

	/* Determine mode flags for output */
	cpt_flags = 0;
	if (Ctrl->N.active) cpt_flags |= 1;	/* bit 0 controls if BFN will be written out */
	if (Ctrl->D.mode == 1) cpt_flags |= 2;	/* bit 1 controls if BF will be set to equal bottom/top rgb value */
	if (Ctrl->F.active) Pout->model = Ctrl->F.model;

	if (Ctrl->A.active) GMT_cpt_transparency (GMT, Pout, Ctrl->A.value, Ctrl->A.mode);	/* Set transparency */

	if (GMT_Put_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, cpt_flags, (void **)&Ctrl->Out.file, (void *)Pout)) Return (GMT_DATA_WRITE_ERROR);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	GMT_free (GMT, cdf_cpt);
	GMT_free (GMT, z);
	for (k = 0; k < ngrd; k++) {
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&G[k]);	/* Destroy all grids */
		free ((void *)grdfile[k]);
	}
	GMT_free (GMT, G);
	GMT_free (GMT, grdfile);
	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Pin);
	GMT_free_palette (GMT, &Pout);	/* Since we know we allocaed this one herein */

	Return (EXIT_SUCCESS);
}
