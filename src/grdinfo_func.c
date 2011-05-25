/*--------------------------------------------------------------------
 *	$Id: grdinfo_func.c,v 1.19 2011-05-25 21:12:00 guru Exp $
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
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: grdinfo reads one or more grid file and [optionally] prints
 * out various statistics like mean/standard deviation and median/scale.
 *
 */

#include "gmt.h"

/* Control structure for grdinfo */

struct GRDINFO_CTRL {
	struct C {	/* -C */
		GMT_LONG active;
	} C;
	struct F {	/* -F */
		GMT_LONG active;
	} F;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		GMT_LONG status;
		double inc[2];
	} I;
	struct M {	/* -M */
		GMT_LONG active;
	} M;
	struct L {	/* -L[1|2] */
		GMT_LONG active;
		GMT_LONG norm;
	} L;
	struct T {	/* -T<dz> */
		GMT_LONG active;
		double inc;
	} T;
};

void *New_grdinfo_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDINFO_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GRDINFO_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return ((void *)C);
}

void Free_grdinfo_Ctrl (struct GMT_CTRL *GMT, struct GRDINFO_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);
}

GMT_LONG GMT_grdinfo_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdinfo %s [API] - Extract information from netCDF grid files\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdinfo <grdfiles> [-C] [-F] [-I[<dx>[/<dy>]]] [-L[0|1|2]] [-M]\n");
	GMT_message (GMT, "	[%s] [-T<dz>] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<grdfiles> may be one or more netCDF grid files\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-C Formats report in fields on a single line using the format\n");
	GMT_message (GMT, "\t   file w e s n z0 z1 dx dy nx ny [x0 y0 x1 y1] [med scale] [mean std rms] [n_nan].\n");
	GMT_message (GMT, "\t   (-M gives [x0 y0 x1 y1] and [n_nan]; -L1 gives [med scale]; -L2 gives [mean std rms]).\n");
	GMT_message (GMT, "\t-F Reports domain in world mapping format [Default is generic].\n");
	GMT_message (GMT, "\t-I Returns textstring -Rw/e/s/n to nearest multiple of dx/dy.\n");
	GMT_message (GMT, "\t   If -C is set then rounding off will occur but no -R string is issued.\n");
	GMT_message (GMT, "\t   If no argument is given then the -I<xinc>/<yinc> string is issued.\n");
	GMT_message (GMT, "\t   If -I- is given then the grid's -R string is issued.\n");
	GMT_message (GMT, "\t-L Sets report mode:\n");
	GMT_message (GMT, "\t   -L0 reports range of data by actually reading them (not from header).\n");
	GMT_message (GMT, "\t   -L1 reports median and L1-scale of data set.\n");
	GMT_message (GMT, "\t   -L[2] reports mean, standard deviation, and rms of data set.\n");
	GMT_message (GMT, "\t-M Searches for the global min and max locations (x0,y0) and (x1,y1).\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-T Given increment dz, return global -Tzmin/zmax/dz in multiples of dz.\n");
	GMT_explain_options (GMT, "Vf.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdinfo_parse (struct GMTAPI_CTRL *C, struct GRDINFO_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Column format */
				Ctrl->C.active = TRUE;
				break;
			case 'F':	/* Wolrd mapping format */
				Ctrl->F.active = TRUE;
				break;
			case 'I':	/* Increment rounding */
				Ctrl->I.active = TRUE;
				if (!opt->arg[0])	/* No args given, we want to output the -I string */
					Ctrl->I.status = 0;
				else if (opt->arg[0] == '-' && opt->arg[1] == '\0')	/* Dash given, we want to output the actual -R string */
					Ctrl->I.status = 1;
				else {	/* Report -R to nearest given multiple increment */
					Ctrl->I.status = 2;
					if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
						GMT_inc_syntax (GMT, 'I', 1);
						n_errors++;
					}
				}
				break;
			case 'L':	/* Selects norm */
				Ctrl->L.active = TRUE;
				switch (opt->arg[0]) {
					case '\0': case '2':
						Ctrl->L.norm |= 2; break;
					case '1':
						Ctrl->L.norm |= 1; break;
				}
				break;
			case 'M':	/* Global extrema */
				Ctrl->M.active = TRUE;
				break;
			case 'T':	/* CPT range */
				Ctrl->T.active = TRUE;
				Ctrl->T.inc = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, n_files == 0, "Syntax error: Must specify one or more input files\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->T.inc <= 0.0, "Syntax error -T: Must specify a positive increment\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && Ctrl->I.status == 2 && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0), "Syntax error -I: Must specify a positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->I.active || Ctrl->T.active) && Ctrl->M.active, "Syntax error -M: Not compatible with -I or -T\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->I.active || Ctrl->T.active) && Ctrl->L.active, "Syntax error -L: Not compatible with -I or -T\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->I.active, "Syntax error: Only one of -I -T can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdinfo_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdinfo (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG n_grds = 0, n_nan = 0, n = 0, ij, error, subset;

	double x_min = 0.0, y_min = 0.0, z_min = 0.0, x_max = 0.0, y_max = 0.0, z_max = 0.0, wesn[4];
	double global_xmin, global_xmax, global_ymin, global_ymax, global_zmin, global_zmax;
	double mean = 0.0, median = 0.0, sum2 = 0.0, stdev = 0.0, scale = 0.0, rms = 0.0, x;

	char format[GMT_BUFSIZ], text[GMT_TEXT_LEN64];
	char *type[2] = { "Gridline", "Pixel"};
	
	struct GRDINFO_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdinfo_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdinfo_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdinfo", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRf", ">", options))) Return (error);
	Ctrl = (struct GRDINFO_CTRL *)New_grdinfo_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdinfo_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdinfo main code ----------------------------*/

	/* OK, done parsing, now process all input grids in a loop */
	
	GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */
	global_xmin = global_ymin = global_zmin = +DBL_MAX;
	global_xmax = global_ymax = global_zmax = -DBL_MAX;
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Registers default output destination, unless already set */
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);				/* Enables data input and sets access mode */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);				/* Enables data output and sets access mode */

	for (opt = options; opt; opt = opt->next) {	/* Loop over arguments, skip options */ 

		if (opt->option != '<') continue;	/* We are only processing filenames here */

		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&opt->arg, (void **)&G)) Return (GMT_DATA_READ_ERROR);
		subset = GMT_is_subset (GMT, G->header, wesn);	/* Subset requested */
		if (subset) GMT_err_fail (GMT, GMT_adjust_loose_wesn (GMT, wesn, G->header), "");	/* Make sure wesn matches header spacing */

		GMT_report (GMT, GMT_MSG_NORMAL, "Processing file %s\n", G->header->name);
		
		for (n = 0; n < GMT_Z; n++) GMT->current.io.col_type[GMT_OUT][n] = GMT->current.io.col_type[GMT_IN][n];	/* Since grids may differ in types */
		
		n_grds++;

		if (Ctrl->M.active || Ctrl->L.active || subset) {	/* Must determine the location of global min and max values */
			GMT_LONG ij_min, ij_max, col, row;
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&opt->arg, (void **)&G)) Return (GMT_DATA_READ_ERROR);

			z_min = DBL_MAX;	z_max = -DBL_MAX;
			mean = median = sum2 = 0.0;
			ij_min = ij_max = n = 0;
			GMT_grd_loop (GMT, G, row, col, ij) {
				if (GMT_is_fnan (G->data[ij])) continue;
				if (G->data[ij] < z_min) {
					z_min = G->data[ij];	ij_min = ij;
				}
				if (G->data[ij] > z_max) {
					z_max = G->data[ij];	ij_max = ij;
				}
				n++;
				if (Ctrl->L.active) {	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
					x = G->data[ij] - mean;
					mean += x / n;
					sum2 += x * (G->data[ij] - mean);
				}
			}

			n_nan = G->header->nm - n;
			col = GMT_col (G->header, ij_min);
			row = GMT_row (G->header, ij_min);
			x_min = GMT_grd_col_to_x (GMT, col, G->header);
			y_min = GMT_grd_row_to_y (GMT, row, G->header);
			col = GMT_col (G->header, ij_max);
			row = GMT_row (G->header, ij_max);
			x_max = GMT_grd_col_to_x (GMT, col, G->header);
			y_max = GMT_grd_row_to_y (GMT, row, G->header);
		}

		if (Ctrl->L.norm & 1) {	/* Calculate the median and L1 scale */
			GMT_LONG new_grid;
			struct GMT_GRID *G2 = NULL;
			
			/* Note that this option rearranges the input grid, so if a memory location is passed then
			 * the grid in the calling program is no longer the original values */
			new_grid = GMT_set_outgrid (GMT, G, &G2);	/* TRUE if input is a read-only array */
			GMT_grd_pad_off (GMT, G2);	/* Undo pad if one existed */
			GMT_sort_array (GMT, (void *)G2->data, G2->header->nm, GMT_FLOAT_TYPE);
			median = (n%2) ? G2->data[n/2] : 0.5*(G2->data[n/2-1] + G2->data[n/2]);
			for (ij = 0; ij < n; ij++) G2->data[ij] = (float)fabs (G2->data[ij] - median);
			GMT_sort_array (GMT, (void *)G2->data, n, GMT_FLOAT_TYPE);
			scale = (n%2) ? 1.4826 * G2->data[n/2] : 0.7413 * (G2->data[n/2-1] + G2->data[n/2]);
			if (new_grid) {	/* Now preserve info and free the temporary grid */
				/* copy over stat info to G */
				GMT_free_grid (GMT, &G2, TRUE);
			}
		}
		if (Ctrl->L.norm & 2) {	/* Calculate the mean, standard deviation, and rms */
			x = (double)n;
			stdev = (n > 1) ? sqrt (sum2 / (x-1)) : GMT->session.d_NaN;
			rms = (n > 0) ? sqrt (sum2 / x + mean * mean) : GMT->session.d_NaN;
			mean = (n > 0) ? mean : GMT->session.d_NaN;
		}

		if (GMT_is_geographic (GMT, GMT_IN)) {
			if (GMT_360_RANGE (G->header->wesn[XHI], G->header->wesn[XLO]))
				GMT->current.io.geo.range = (G->header->wesn[XHI] < 0.0) ? GMT_IS_M180_TO_P180_RANGE : GMT_IS_0_TO_P360_RANGE;
			else if (G->header->wesn[XLO] < 0.0 && G->header->wesn[XHI] >= 0.0)
				GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;
			else
				GMT->current.io.geo.range = GMT_IS_0_TO_P360_RANGE;
		}

		/* OK, time to report results */

		if (Ctrl->I.active && Ctrl->I.status == 1) {
			GMT_fprintf (GMT->session.std[GMT_OUT], "-R");
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->wesn[XLO], GMT_X);
			GMT_fprintf (GMT->session.std[GMT_OUT], "/");
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->wesn[XHI], GMT_X);
			GMT_fprintf (GMT->session.std[GMT_OUT], "/");
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->wesn[YLO], GMT_Y);
			GMT_fprintf (GMT->session.std[GMT_OUT], "/");
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->wesn[YHI], GMT_Y);
			GMT_fprintf (GMT->session.std[GMT_OUT], "\n");
		} else if (Ctrl->I.active && Ctrl->I.status == 0) {
			GMT_fprintf (GMT->session.std[GMT_OUT], "-I");
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->inc[GMT_X], GMT_Z);
			GMT_fprintf (GMT->session.std[GMT_OUT], "/");
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->inc[GMT_Y], GMT_Z);
			GMT_fprintf (GMT->session.std[GMT_OUT], "\n");
		} else if (Ctrl->C.active && !Ctrl->I.active) {
			GMT_fprintf (GMT->session.std[GMT_OUT], "%s\t", G->header->name);
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->wesn[XLO], GMT_X);
			GMT_fprintf (GMT->session.std[GMT_OUT], "\t");
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->wesn[XHI], GMT_X);
			GMT_fprintf (GMT->session.std[GMT_OUT], "\t");
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->wesn[YLO], GMT_Y);
			GMT_fprintf (GMT->session.std[GMT_OUT], "\t");
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->wesn[YHI], GMT_Y);
			GMT_fprintf (GMT->session.std[GMT_OUT], "\t");
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->z_min, GMT_Z);
			GMT_fprintf (GMT->session.std[GMT_OUT], "\t");
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->z_max, GMT_Z);
			GMT_fprintf (GMT->session.std[GMT_OUT], "\t");
			GMT_ascii_format_col (GMT, text, G->header->inc[GMT_X], GMT_X);
			if (isalpha ((int)text[strlen(text)-1])) text[strlen(text)-1] = '\0';	/* Chop of trailing WESN flag here */
			GMT_fprintf (GMT->session.std[GMT_OUT], "%s\t", text);
			GMT_ascii_format_col (GMT, text, G->header->inc[GMT_Y], GMT_Y);
			if (isalpha ((int)text[strlen(text)-1])) text[strlen(text)-1] = '\0';	/* Chop of trailing WESN flag here */
			GMT_fprintf (GMT->session.std[GMT_OUT], "%s\t", text);
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], (double)G->header->nx, GMT_Z);
			GMT_fprintf (GMT->session.std[GMT_OUT], "\t");
			GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], (double)G->header->ny, GMT_Z);

			if (Ctrl->M.active) {
				GMT_fprintf (GMT->session.std[GMT_OUT], "\t");	GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], x_min, GMT_X);
				GMT_fprintf (GMT->session.std[GMT_OUT], "\t");	GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], y_min, GMT_Y);
				GMT_fprintf (GMT->session.std[GMT_OUT], "\t");	GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], x_max, GMT_X);
				GMT_fprintf (GMT->session.std[GMT_OUT], "\t");	GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], y_max, GMT_Y);
			}
			if (Ctrl->L.norm & 1) {
				GMT_fprintf (GMT->session.std[GMT_OUT], "\t");	GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], median, GMT_Z);
				GMT_fprintf (GMT->session.std[GMT_OUT], "\t");	GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], scale, GMT_Z);
			}
			if (Ctrl->L.norm & 2) {
				GMT_fprintf (GMT->session.std[GMT_OUT], "\t");	GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], mean, GMT_Z);
				GMT_fprintf (GMT->session.std[GMT_OUT], "\t");	GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], stdev, GMT_Z);
				GMT_fprintf (GMT->session.std[GMT_OUT], "\t");	GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], rms, GMT_Z);
			}
			if (Ctrl->M.active) GMT_fprintf (GMT->session.std[GMT_OUT], "\t%" GMT_LL "d", n_nan);
			GMT_fprintf (GMT->session.std[GMT_OUT], "\n");
		}
		else if (!(Ctrl->T.active || (Ctrl->I.active && Ctrl->I.status == 2))) {
			GMT_fprintf (GMT->session.std[GMT_OUT], "%s: Title: %s\n", G->header->name, G->header->title);
			GMT_fprintf (GMT->session.std[GMT_OUT], "%s: Command: %s\n", G->header->name, G->header->command);
			GMT_fprintf (GMT->session.std[GMT_OUT], "%s: Remark: %s\n", G->header->name, G->header->remark);
			if (G->header->registration == GMT_GRIDLINE_REG || G->header->registration == GMT_PIXEL_REG)
				GMT_fprintf (GMT->session.std[GMT_OUT], 
					"%s: %s node registration used\n", G->header->name, type[G->header->registration]);
			else
				GMT_fprintf (GMT->session.std[GMT_OUT], 
					"%s: Unknown registration! Probably not a GMT grid\n", G->header->name);
			if (G->header->type >= 0 && G->header->type < GMT_N_GRD_FORMATS)
				GMT_fprintf (GMT->session.std[GMT_OUT], 
					"%s: Grid file format: %s\n", G->header->name, GMT->session.grdformat[G->header->type]);
			else
				GMT_fprintf (GMT->session.std[GMT_OUT], 
					"%s: Unrecognized grid file format! Probably not a GMT grid\n", G->header->name);
			if (Ctrl->F.active) {
				if ((fabs (G->header->wesn[XLO]) < 500.0) && (fabs (G->header->wesn[XHI]) < 500.0) && (fabs (G->header->wesn[YLO]) < 500.0) && (fabs (G->header->wesn[YHI]) < 500.0)) {
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: x_min: %.7f\n", G->header->name, G->header->wesn[XLO]);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: x_max: %.7f\n", G->header->name, G->header->wesn[XHI]);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: x_inc: %.7f\n", G->header->name, G->header->inc[GMT_X]);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: name: %s\n",    G->header->name, G->header->x_units);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: nx: %d\n",      G->header->name, G->header->nx);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: y_min: %.7f\n", G->header->name, G->header->wesn[YLO]);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: y_max: %.7f\n", G->header->name, G->header->wesn[YHI]);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: y_inc: %.7f\n", G->header->name, G->header->inc[GMT_Y]);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: name: %s\n",    G->header->name, G->header->y_units);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: ny: %d\n",      G->header->name, G->header->ny);
				}
				else {
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: x_min: %.2f\n", G->header->name, G->header->wesn[XLO]);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: x_max: %.2f\n", G->header->name, G->header->wesn[XHI]);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: x_inc: %.2f\n", G->header->name, G->header->inc[GMT_X]);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: name: %s\n",    G->header->name, G->header->x_units);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: nx: %d\n",      G->header->name, G->header->nx);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: y_min: %.2f\n", G->header->name, G->header->wesn[YLO]);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: y_max: %.2f\n", G->header->name, G->header->wesn[YHI]);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: y_inc: %.2f\n", G->header->name, G->header->inc[GMT_Y]);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: name: %s\n",    G->header->name, G->header->y_units);
					GMT_fprintf (GMT->session.std[GMT_OUT], "%s: ny: %d\n",      G->header->name, G->header->ny);
				}
			}
			else {
				GMT_ascii_format_col (GMT, text, G->header->inc[GMT_X], GMT_X);
				if (isalpha ((int)text[strlen(text)-1])) text[strlen(text)-1] = '\0';	/* Chop of trailing WESN flag here */
				GMT_fprintf (GMT->session.std[GMT_OUT], "%s: x_min: ", G->header->name);
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->wesn[XLO], GMT_X);
				GMT_fprintf (GMT->session.std[GMT_OUT], " x_max: ");
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->wesn[XHI], GMT_X);
				GMT_fprintf (GMT->session.std[GMT_OUT], " x_inc: %s", text);
				GMT_fprintf (GMT->session.std[GMT_OUT], " name: %s nx: %d\n", G->header->x_units, G->header->nx);
				GMT_fprintf (GMT->session.std[GMT_OUT], "%s: y_min: ", G->header->name);
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->wesn[YLO], GMT_Y);
				GMT_fprintf (GMT->session.std[GMT_OUT], " y_max: ");
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->wesn[YHI], GMT_Y);
				GMT_ascii_format_col (GMT, text, G->header->inc[GMT_Y], GMT_Y);
				if (isalpha ((int)text[strlen(text)-1])) text[strlen(text)-1] = '\0';	/* Chop of trailing WESN flag here */
				GMT_fprintf (GMT->session.std[GMT_OUT], " y_inc: %s", text);
				GMT_fprintf (GMT->session.std[GMT_OUT], " name: %s ny: %d\n", G->header->y_units, G->header->ny);
			}

			if (Ctrl->M.active) {
				if (z_min == -DBL_MAX) z_min = GMT->session.d_NaN;
				if (z_max == +DBL_MAX) z_max = GMT->session.d_NaN;
				GMT_fprintf (GMT->session.std[GMT_OUT], "%s: z_min: ", G->header->name);
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], z_min, GMT_Z);
				GMT_fprintf (GMT->session.std[GMT_OUT], " at x = ");
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], x_min, GMT_X);
				GMT_fprintf (GMT->session.std[GMT_OUT], " y = ");
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], y_min, GMT_Y);
				GMT_fprintf (GMT->session.std[GMT_OUT], " z_max: ");
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], z_max, GMT_Z);
				GMT_fprintf (GMT->session.std[GMT_OUT], " at x = ");
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], x_max, GMT_X);
				GMT_fprintf (GMT->session.std[GMT_OUT], " y = ");
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], y_max, GMT_Y);
				GMT_fprintf (GMT->session.std[GMT_OUT], "\n");
			}
			else if (Ctrl->F.active) {
				GMT_fprintf (GMT->session.std[GMT_OUT], "%s: zmin: %g\n", G->header->name, G->header->z_min);
				GMT_fprintf (GMT->session.std[GMT_OUT], "%s: zmax: %g\n", G->header->name, G->header->z_max);
			 	GMT_fprintf (GMT->session.std[GMT_OUT], "%s: name: %s\n", G->header->name, G->header->z_units);
			}
			else {
				GMT_fprintf (GMT->session.std[GMT_OUT], "%s: z_min: ", G->header->name);
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->z_min, GMT_Z);
				GMT_fprintf (GMT->session.std[GMT_OUT], " z_max: ");
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], G->header->z_max, GMT_Z);
				GMT_fprintf (GMT->session.std[GMT_OUT], " name: %s\n", G->header->z_units);
			}

			GMT_ascii_format_col (GMT, text, G->header->z_add_offset, GMT_Z);
			if (isalpha ((int)text[strlen(text)-1])) text[strlen(text)-1] = '\0';	/* Chop of trailing WESN flag here */
			sprintf (format, "%s: scale_factor: %s add_offset: %%s\n", G->header->name, GMT->current.setting.format_float_out);
			GMT_fprintf (GMT->session.std[GMT_OUT], format, G->header->z_scale_factor, text);
			if (n_nan) GMT_fprintf (GMT->session.std[GMT_OUT], "%s: %" GMT_LL "d nodes set to NaN\n", G->header->name, n_nan);
			if (Ctrl->L.norm & 1) {
				GMT_fprintf (GMT->session.std[GMT_OUT], "%s: median: ", G->header->name);
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], median, GMT_Z);
				GMT_fprintf (GMT->session.std[GMT_OUT], " scale: ");
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], scale, GMT_Z);
				GMT_fprintf (GMT->session.std[GMT_OUT], "\n");
			}
			if (Ctrl->L.norm & 2) {
				GMT_fprintf (GMT->session.std[GMT_OUT], "%s: mean: ", G->header->name);
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], mean, GMT_Z);
				GMT_fprintf (GMT->session.std[GMT_OUT], " stdev: ");
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], stdev, GMT_Z);
				GMT_fprintf (GMT->session.std[GMT_OUT], " rms: ");
				GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], rms, GMT_Z);
				GMT_fprintf (GMT->session.std[GMT_OUT], "\n");
			}
		}
		else {
			if (G->header->z_min < global_zmin) global_zmin = G->header->z_min;
			if (G->header->z_max > global_zmax) global_zmax = G->header->z_max;
			if (G->header->wesn[XLO] < global_xmin) global_xmin = G->header->wesn[XLO];
			if (G->header->wesn[XHI] > global_xmax) global_xmax = G->header->wesn[XHI];
			if (G->header->wesn[YLO] < global_ymin) global_ymin = G->header->wesn[YLO];
			if (G->header->wesn[YHI] > global_ymax) global_ymax = G->header->wesn[YHI];
		}
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&G);
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (global_zmin == -DBL_MAX) global_zmin = GMT->session.d_NaN;	/* Never got set */
	if (global_zmax == +DBL_MAX) global_zmax = GMT->session.d_NaN;

	if (Ctrl->C.active && (Ctrl->I.active && Ctrl->I.status == 2)) {
		global_xmin = floor (global_xmin / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
		global_xmax = ceil  (global_xmax / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
		global_ymin = floor (global_ymin / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
		global_ymax = ceil  (global_ymax / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
		GMT_fprintf (GMT->session.std[GMT_OUT], "%" GMT_LL "d\t", n_grds);
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], global_xmin, GMT_X);	GMT_fputs ("\t", GMT->session.std[GMT_OUT]);
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], global_xmax, GMT_X);	GMT_fputs ("\t", GMT->session.std[GMT_OUT]);
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], global_ymin, GMT_Y);	GMT_fputs ("\t", GMT->session.std[GMT_OUT]);
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], global_ymax, GMT_Y);	GMT_fputs ("\t", GMT->session.std[GMT_OUT]);
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], global_zmin, GMT_Z);	GMT_fputs ("\t", GMT->session.std[GMT_OUT]);
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], global_zmax, GMT_Z);	GMT_fputs ("\n", GMT->session.std[GMT_OUT]);
	}
	else if (Ctrl->T.active) {
		global_zmin = floor (global_zmin / Ctrl->T.inc) * Ctrl->T.inc;
		global_zmax = ceil  (global_zmax / Ctrl->T.inc) * Ctrl->T.inc;
		GMT_fprintf (GMT->session.std[GMT_OUT], "-T");
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], global_zmin, GMT_Z);
		GMT_fprintf (GMT->session.std[GMT_OUT], "/");
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], global_zmax, GMT_Z);
		GMT_fprintf (GMT->session.std[GMT_OUT], "/");
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], Ctrl->T.inc, GMT_Z);
		GMT_fprintf (GMT->session.std[GMT_OUT], "\n");
	}
	else if ((Ctrl->I.active && Ctrl->I.status == 2)) {
		global_xmin = floor (global_xmin / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
		global_xmax = ceil  (global_xmax / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
		global_ymin = floor (global_ymin / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
		global_ymax = ceil  (global_ymax / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
		GMT_fprintf (GMT->session.std[GMT_OUT], "-R");
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], global_xmin, GMT_X);	GMT_fputs ("/", GMT->session.std[GMT_OUT]);
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], global_xmax, GMT_X);	GMT_fputs ("/", GMT->session.std[GMT_OUT]);
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], global_ymin, GMT_Y);	GMT_fputs ("/", GMT->session.std[GMT_OUT]);
		GMT_ascii_output_col (GMT, GMT->session.std[GMT_OUT], global_ymax, GMT_Y);	GMT_fputs ("\n", GMT->session.std[GMT_OUT]);
	}
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	GMT_report (GMT, GMT_MSG_NORMAL, "Done!\n");
	Return (GMT_OK);
}
