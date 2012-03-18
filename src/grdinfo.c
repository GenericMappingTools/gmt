/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
	struct T {	/* -T[s]<dz> */
		GMT_LONG active;
		GMT_LONG mode;
		double inc;
	} T;
};

void *New_grdinfo_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDINFO_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GRDINFO_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return (C);
}

void Free_grdinfo_Ctrl (struct GMT_CTRL *GMT, struct GRDINFO_CTRL *C) {	/* Deallocate control structure */
	GMT_free (GMT, C);
}

GMT_LONG GMT_grdinfo_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdinfo %s [API] - Extract information from grids\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdinfo <grid> [-C] [-F] [-I[<dx>[/<dy>]]] [-L[0|1|2]] [-M]\n");
	GMT_message (GMT, "	[%s] [-T[s]<dz>] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<grid> may be one or more grid files.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-C Format report in fields on a single line using the format\n");
	GMT_message (GMT, "\t   file w e s n z0 z1 dx dy nx ny [x0 y0 x1 y1] [med scale] [mean std rms] [n_nan].\n");
	GMT_message (GMT, "\t   (-M gives [x0 y0 x1 y1] and [n_nan]; -L1 gives [med scale]; -L2 gives [mean std rms]).\n");
	GMT_message (GMT, "\t-F Report domain in world mapping format [Default is generic].\n");
	GMT_message (GMT, "\t-I Return textstring -Rw/e/s/n to nearest multiple of dx/dy.\n");
	GMT_message (GMT, "\t   If -C is set then rounding off will occur but no -R string is issued.\n");
	GMT_message (GMT, "\t   If no argument is given then the -I<xinc>/<yinc> string is issued.\n");
	GMT_message (GMT, "\t   If -I- is given then the grid's -R string is issued.\n");
	GMT_message (GMT, "\t-L Set report mode:\n");
	GMT_message (GMT, "\t   -L0 reports range of data by actually reading them (not from header).\n");
	GMT_message (GMT, "\t   -L1 reports median and L1-scale of data set.\n");
	GMT_message (GMT, "\t   -L[2] reports mean, standard deviation, and rms of data set.\n");
	GMT_message (GMT, "\t-M Search for the global min and max locations (x0,y0) and (x1,y1).\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-T Given increment dz, return global -Tzmin/zmax/dz in multiples of dz.\n");
	GMT_message (GMT, "\t   To get a symmetrical range about zero, use -Ts<dz> insteda.\n");
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
				if (opt->arg[0] == 's') {	/* Want symmetrical range about 0, i.e., -3500/3500/500 */
					Ctrl->T.mode = 1;
					Ctrl->T.inc = atof (&opt->arg[1]);
				}
				else
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

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdinfo_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_grdinfo (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG n_grds = 0, n_nan = 0, n = 0, ij, error, subset;

	double x_min = 0.0, y_min = 0.0, z_min = 0.0, x_max = 0.0, y_max = 0.0, z_max = 0.0, wesn[4];
	double global_xmin, global_xmax, global_ymin, global_ymax, global_zmin, global_zmax;
	double mean = 0.0, median = 0.0, sum2 = 0.0, stdev = 0.0, scale = 0.0, rms = 0.0, x;

	char format[GMT_BUFSIZ], text[GMT_TEXT_LEN64], record[GMT_BUFSIZ];
	char *type[2] = { "Gridline", "Pixel"};
	
	struct GRDINFO_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_grdinfo_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_grdinfo_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdinfo", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-VRf", ">", options)) Return (API->error);
	Ctrl = New_grdinfo_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdinfo_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdinfo main code ----------------------------*/

	/* OK, done parsing, now process all input grids in a loop */
	
	GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Current -R setting, if any */
	global_xmin = global_ymin = global_zmin = +DBL_MAX;
	global_xmax = global_ymax = global_zmax = -DBL_MAX;
	if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_TEXT, GMT_OUT, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}

	for (opt = options; opt; opt = opt->next) {	/* Loop over arguments, skip options */ 

		if (opt->option != '<') continue;	/* We are only processing filenames here */

		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, opt->arg, NULL)) == NULL) {
			Return (API->error);
		}
		subset = GMT_is_subset (GMT, G->header, wesn);	/* Subset requested */
		if (subset) GMT_err_fail (GMT, GMT_adjust_loose_wesn (GMT, wesn, G->header), "");	/* Make sure wesn matches header spacing */

		GMT_report (GMT, GMT_MSG_NORMAL, "Processing file %s\n", G->header->name);
		
		for (n = 0; n < GMT_Z; n++) GMT->current.io.col_type[GMT_OUT][n] = GMT->current.io.col_type[GMT_IN][n];	/* Since grids may differ in types */
		
		n_grds++;

		if (Ctrl->M.active || Ctrl->L.active || subset) {	/* Need to read the data (all or subset) */
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, opt->arg, G) == NULL) {
				Return (API->error);
			}
		}
		
		if (Ctrl->M.active || Ctrl->L.active) {	/* Must determine the location of global min and max values */
			GMT_LONG ij_min, ij_max, col, row;

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
			GMT_sort_array (GMT, G2->data, G2->header->nm, GMTAPI_FLOAT);
			median = (n%2) ? G2->data[n/2] : 0.5*(G2->data[n/2-1] + G2->data[n/2]);
			for (ij = 0; ij < n; ij++) G2->data[ij] = (float)fabs (G2->data[ij] - median);
			GMT_sort_array (GMT, G2->data, n, GMTAPI_FLOAT);
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
			if (GMT_grd_is_global (GMT, G->header))
				GMT->current.io.geo.range = (G->header->wesn[XLO] < 0.0) ? GMT_IS_M180_TO_P180_RANGE : GMT_IS_0_TO_P360_RANGE;
			else if (G->header->wesn[XLO] < 0.0 && G->header->wesn[XHI] >= 0.0)
				GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;
			else
				GMT->current.io.geo.range = GMT_IS_0_TO_P360_RANGE;
		}

		/* OK, time to report results */

		if (Ctrl->I.active && Ctrl->I.status == 1) {
			sprintf (record, "-R");
			GMT_ascii_format_col (GMT, text, G->header->wesn[XLO], GMT_X);	strcat (record, text);	strcat (record, "/");
			GMT_ascii_format_col (GMT, text, G->header->wesn[XHI], GMT_X);	strcat (record, text);	strcat (record, "/");
			GMT_ascii_format_col (GMT, text, G->header->wesn[YLO], GMT_Y);	strcat (record, text);	strcat (record, "/");
			GMT_ascii_format_col (GMT, text, G->header->wesn[YHI], GMT_Y);	strcat (record, text);
			GMT_Put_Record (API, GMT_WRITE_TEXT, record);
		} else if (Ctrl->I.active && Ctrl->I.status == 0) {
			sprintf (record, "-I");
			GMT_ascii_format_col (GMT, text, G->header->inc[GMT_X], GMT_Z);	strcat (record, text);	strcat (record, "/");
			GMT_ascii_format_col (GMT, text, G->header->inc[GMT_Y], GMT_Z);
			GMT_Put_Record (API, GMT_WRITE_TEXT, record);
		} else if (Ctrl->C.active && !Ctrl->I.active) {
			sprintf (record, "%s%s", G->header->name, GMT->current.setting.io_col_separator);
			GMT_ascii_format_col (GMT, text, G->header->wesn[XLO], GMT_X);	strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
			GMT_ascii_format_col (GMT, text, G->header->wesn[XHI], GMT_X);	strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
			GMT_ascii_format_col (GMT, text, G->header->wesn[YLO], GMT_Y);	strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
			GMT_ascii_format_col (GMT, text, G->header->wesn[YHI], GMT_Y);	strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
			GMT_ascii_format_col (GMT, text, G->header->z_min, GMT_Z);	strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
			GMT_ascii_format_col (GMT, text, G->header->z_max, GMT_Z);	strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
			GMT_ascii_format_col (GMT, text, G->header->inc[GMT_X], GMT_X);
			if (isalpha ((int)text[strlen(text)-1])) text[strlen(text)-1] = '\0';	/* Chop of trailing WESN flag here */
			strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
			GMT_ascii_format_col (GMT, text, G->header->inc[GMT_Y], GMT_Y);
			if (isalpha ((int)text[strlen(text)-1])) text[strlen(text)-1] = '\0';	/* Chop of trailing WESN flag here */
			strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
			GMT_ascii_format_col (GMT, text, (double)G->header->nx, GMT_Z);	strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
			GMT_ascii_format_col (GMT, text, (double)G->header->ny, GMT_Z);	strcat (record, text);

			if (Ctrl->M.active) {
				strcat (record, GMT->current.setting.io_col_separator);	GMT_ascii_format_col (GMT, text, x_min, GMT_X);	strcat (record, text);
				strcat (record, GMT->current.setting.io_col_separator);	GMT_ascii_format_col (GMT, text, y_min, GMT_Y);	strcat (record, text);
				strcat (record, GMT->current.setting.io_col_separator);	GMT_ascii_format_col (GMT, text, x_max, GMT_X);	strcat (record, text);
				strcat (record, GMT->current.setting.io_col_separator);	GMT_ascii_format_col (GMT, text, y_max, GMT_Y);	strcat (record, text);
			}
			if (Ctrl->L.norm & 1) {
				strcat (record, GMT->current.setting.io_col_separator);	GMT_ascii_format_col (GMT, text, median, GMT_Z);	strcat (record, text);
				strcat (record, GMT->current.setting.io_col_separator);	GMT_ascii_format_col (GMT, text, scale, GMT_Z);		strcat (record, text);
			}
			if (Ctrl->L.norm & 2) {
				strcat (record, GMT->current.setting.io_col_separator);	GMT_ascii_format_col (GMT, text, mean, GMT_Z);	strcat (record, text);
				strcat (record, GMT->current.setting.io_col_separator);	GMT_ascii_format_col (GMT, text, stdev, GMT_Z);	strcat (record, text);
				strcat (record, GMT->current.setting.io_col_separator);	GMT_ascii_format_col (GMT, text, rms, GMT_Z);	strcat (record, text);
			}
			if (Ctrl->M.active) { sprintf (text, "%s%" GMT_LL "d", GMT->current.setting.io_col_separator, n_nan);	strcat (record, text); }
			GMT_Put_Record (API, GMT_WRITE_TEXT, record);
		}
		else if (!(Ctrl->T.active || (Ctrl->I.active && Ctrl->I.status == 2))) {
			char *gtype[2] = {"Cartesian grid", "Geographic grid"};
			sprintf (record, "%s: Title: %s", G->header->name, G->header->title);		GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			sprintf (record, "%s: Command: %s", G->header->name, G->header->command);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			sprintf (record, "%s: Remark: %s", G->header->name, G->header->remark);		GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			if (G->header->registration == GMT_GRIDLINE_REG || G->header->registration == GMT_PIXEL_REG)
				sprintf (record, "%s: %s node registration used [%s]", G->header->name, type[G->header->registration], gtype[GMT_is_geographic (GMT, GMT_IN)]);
			else
				sprintf (record, "%s: Unknown registration! Probably not a GMT grid", G->header->name);
			GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			if (G->header->type >= 0 && G->header->type < GMT_N_GRD_FORMATS)
				sprintf (record, "%s: Grid file format: %s", G->header->name, GMT->session.grdformat[G->header->type]);
			else
				sprintf (record, "%s: Unrecognized grid file format! Probably not a GMT grid", G->header->name);
			GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			if (Ctrl->F.active) {
				if ((fabs (G->header->wesn[XLO]) < 500.0) && (fabs (G->header->wesn[XHI]) < 500.0) && (fabs (G->header->wesn[YLO]) < 500.0) && (fabs (G->header->wesn[YHI]) < 500.0)) {
					sprintf (record, "%s: x_min: %.7f", G->header->name, G->header->wesn[XLO]);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: x_max: %.7f", G->header->name, G->header->wesn[XHI]);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: x_inc: %.7f", G->header->name, G->header->inc[GMT_X]);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: name: %s",    G->header->name, G->header->x_units);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: nx: %dn",     G->header->name, G->header->nx);		GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: y_min: %.7f", G->header->name, G->header->wesn[YLO]);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: y_max: %.7f", G->header->name, G->header->wesn[YHI]);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: y_inc: %.7f", G->header->name, G->header->inc[GMT_Y]);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: name: %s",    G->header->name, G->header->y_units);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: ny: %d",      G->header->name, G->header->ny);		GMT_Put_Record (API, GMT_WRITE_TEXT, record);
				}
				else {
					sprintf (record, "%s: x_min: %.2f", G->header->name, G->header->wesn[XLO]);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: x_max: %.2f", G->header->name, G->header->wesn[XHI]);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: x_inc: %.2f", G->header->name, G->header->inc[GMT_X]);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: name: %s",    G->header->name, G->header->x_units);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: nx: %d",      G->header->name, G->header->nx);		GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: y_min: %.2f", G->header->name, G->header->wesn[YLO]);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: y_max: %.2f", G->header->name, G->header->wesn[YHI]);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: y_inc: %.2f", G->header->name, G->header->inc[GMT_Y]);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: name: %s",    G->header->name, G->header->y_units);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
					sprintf (record, "%s: ny: %d",      G->header->name, G->header->ny);		GMT_Put_Record (API, GMT_WRITE_TEXT, record);
				}
			}
			else {
				sprintf (record, "%s: x_min: ", G->header->name);
				GMT_ascii_format_col (GMT, text, G->header->wesn[XLO], GMT_X);	strcat (record, text);
				strcat (record, " x_max: ");
				GMT_ascii_format_col (GMT, text, G->header->wesn[XHI], GMT_X);	strcat (record, text);
				GMT_ascii_format_col (GMT, text, G->header->inc[GMT_X], GMT_X);
				if (isalpha ((int)text[strlen(text)-1])) text[strlen(text)-1] = '\0';	/* Chop of trailing WESN flag here */
				strcat (record, " x_inc: ");	strcat (record, text);
				strcat (record, " name: ");	strcat (record, G->header->x_units);
				sprintf (text, " nx: %d", G->header->nx);	strcat (record, text);
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);
				sprintf (record, "%s: y_min: ", G->header->name);
				GMT_ascii_format_col (GMT, text, G->header->wesn[YLO], GMT_Y);	strcat (record, text);
				strcat (record, " y_max: ");
				GMT_ascii_format_col (GMT, text, G->header->wesn[YHI], GMT_Y);	strcat (record, text);
				GMT_ascii_format_col (GMT, text, G->header->inc[GMT_Y], GMT_Y);
				if (isalpha ((int)text[strlen(text)-1])) text[strlen(text)-1] = '\0';	/* Chop of trailing WESN flag here */
				strcat (record, " y_inc: ");	strcat (record, text);
				strcat (record, " name: ");	strcat (record, G->header->y_units);
				sprintf (text, " ny: %d", G->header->ny);	strcat (record, text);
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			}

			if (Ctrl->M.active) {
				if (z_min == -DBL_MAX) z_min = GMT->session.d_NaN;
				if (z_max == +DBL_MAX) z_max = GMT->session.d_NaN;
				sprintf (record, "%s: z_min: ", G->header->name);
				GMT_ascii_format_col (GMT, text, z_min, GMT_Z);	strcat (record, text);
				strcat (record, " at x = ");
				GMT_ascii_format_col (GMT, text, x_min, GMT_X);	strcat (record, text);
				strcat (record, " y = ");
				GMT_ascii_format_col (GMT, text, y_min, GMT_Y);	strcat (record, text);
				strcat (record, " z_max: ");
				GMT_ascii_format_col (GMT, text, z_max, GMT_Z);	strcat (record, text);
				strcat (record, " at x = ");
				GMT_ascii_format_col (GMT, text, x_max, GMT_X);	strcat (record, text);
				strcat (record, " y = ");
				GMT_ascii_format_col (GMT, text, y_max, GMT_Y);	strcat (record, text);
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			}
			else if (Ctrl->F.active) {
				sprintf (record, "%s: zmin: %g", G->header->name, G->header->z_min);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
				sprintf (record, "%s: zmax: %g", G->header->name, G->header->z_max);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			 	sprintf (record, "%s: name: %s", G->header->name, G->header->z_units);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			}
			else {
				sprintf (record, "%s: z_min: ", G->header->name);
				GMT_ascii_format_col (GMT, text, G->header->z_min, GMT_Z);	strcat (record, text);
				strcat (record, " z_max: ");
				GMT_ascii_format_col (GMT, text, G->header->z_max, GMT_Z);	strcat (record, text);
				strcat (record, " name: ");	strcat (record, G->header->z_units);
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			}

			GMT_ascii_format_col (GMT, text, G->header->z_add_offset, GMT_Z);
			if (isalpha ((int)text[strlen(text)-1])) text[strlen(text)-1] = '\0';	/* Chop of trailing WESN flag here */
			sprintf (format, "%s: scale_factor: %s add_offset: %%s", G->header->name, GMT->current.setting.format_float_out);
			sprintf (record, format, G->header->z_scale_factor, text);	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			if (n_nan) { sprintf (record, "%s: %" GMT_LL "d nodes set to NaN", G->header->name, n_nan);	GMT_Put_Record (API, GMT_WRITE_TEXT, record); }
			if (Ctrl->L.norm & 1) {
				sprintf (record, "%s: median: ", G->header->name);
				GMT_ascii_format_col (GMT, text, median, GMT_Z);	strcat (record, text);
				strcat (record, " scale: ");
				GMT_ascii_format_col (GMT, text, scale, GMT_Z);	strcat (record, text);
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);
			}
			if (Ctrl->L.norm & 2) {
				sprintf (record, "%s: mean: ", G->header->name);
				GMT_ascii_format_col (GMT, text, mean, GMT_Z);	strcat (record, text);
				strcat (record, " stdev: ");
				GMT_ascii_format_col (GMT, text, stdev, GMT_Z);	strcat (record, text);
				strcat (record, " rms: ");
				GMT_ascii_format_col (GMT, text, rms, GMT_Z);	strcat (record, text);
				GMT_Put_Record (API, GMT_WRITE_TEXT, record);
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
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &G) != GMT_OK) {
			Return (API->error);
		}
	}

	if (global_zmin == -DBL_MAX) global_zmin = GMT->session.d_NaN;	/* Never got set */
	if (global_zmax == +DBL_MAX) global_zmax = GMT->session.d_NaN;

	if (Ctrl->C.active && (Ctrl->I.active && Ctrl->I.status == 2)) {
		global_xmin = floor (global_xmin / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
		global_xmax = ceil  (global_xmax / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
		global_ymin = floor (global_ymin / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
		global_ymax = ceil  (global_ymax / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
		sprintf (record, "%" GMT_LL "d%s", n_grds, GMT->current.setting.io_col_separator);
		GMT_ascii_format_col (GMT, text, global_xmin, GMT_X);	strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
		GMT_ascii_format_col (GMT, text, global_xmax, GMT_X);	strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
		GMT_ascii_format_col (GMT, text, global_ymin, GMT_Y);	strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
		GMT_ascii_format_col (GMT, text, global_ymax, GMT_Y);	strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
		GMT_ascii_format_col (GMT, text, global_zmin, GMT_Z);	strcat (record, text);	strcat (record, GMT->current.setting.io_col_separator);
		GMT_ascii_format_col (GMT, text, global_zmax, GMT_Z);	strcat (record, text);
		GMT_Put_Record (API, GMT_WRITE_TEXT, record);
	}
	else if (Ctrl->T.active) {
		if (Ctrl->T.mode) {	/* Get a symemtrical range */
			global_zmin = floor (global_zmin / Ctrl->T.inc) * Ctrl->T.inc;
			global_zmax = ceil  (global_zmax / Ctrl->T.inc) * Ctrl->T.inc;
			global_zmax = MAX (fabs (global_zmin), fabs (global_zmax));
			global_zmin = -global_zmax;
		}
		else {
			global_zmin = floor (global_zmin / Ctrl->T.inc) * Ctrl->T.inc;
			global_zmax = ceil  (global_zmax / Ctrl->T.inc) * Ctrl->T.inc;
		}
		sprintf (record, "-T");
		GMT_ascii_format_col (GMT, text, global_zmin, GMT_Z);	strcat (record, text);	strcat (record, "/");
		GMT_ascii_format_col (GMT, text, global_zmax, GMT_Z);	strcat (record, text);	strcat (record, "/");
		GMT_ascii_format_col (GMT, text, Ctrl->T.inc, GMT_Z);	strcat (record, text);
		GMT_Put_Record (API, GMT_WRITE_TEXT, record);
	}
	else if ((Ctrl->I.active && Ctrl->I.status == 2)) {
		global_xmin = floor (global_xmin / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
		global_xmax = ceil  (global_xmax / Ctrl->I.inc[GMT_X]) * Ctrl->I.inc[GMT_X];
		global_ymin = floor (global_ymin / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
		global_ymax = ceil  (global_ymax / Ctrl->I.inc[GMT_Y]) * Ctrl->I.inc[GMT_Y];
		sprintf (record, "-R");
		GMT_ascii_format_col (GMT, text, global_xmin, GMT_X);	strcat (record, text);	strcat (record, "/");
		GMT_ascii_format_col (GMT, text, global_xmax, GMT_X);	strcat (record, text);	strcat (record, "/");
		GMT_ascii_format_col (GMT, text, global_ymin, GMT_Y);	strcat (record, text);	strcat (record, "/");
		GMT_ascii_format_col (GMT, text, global_ymax, GMT_Y);	strcat (record, text);
		GMT_Put_Record (API, GMT_WRITE_TEXT, record);
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "Done!\n");
	Return (GMT_OK);
}
