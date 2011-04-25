/*--------------------------------------------------------------------
 *	$Id: grdsample_func.c,v 1.13 2011-04-25 00:04:09 remko Exp $
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
 * API functions to support the grdsample application.
 *
 * Brief synopsis: grdsample reads a grid file and evaluates the grid at new grid
 * positions specified by new dx/dy values using a 2-D Taylor expansion of order 3.
 * In order to evaluate derivatives along the edges of the surface, I assume 
 * natural bicubic spline conditions, i.e. both the second and third normal 
 * derivatives are zero, and that the dxdy derivative in the corners are zero, too.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#include "gmt.h"

struct GRDSAMPLE_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct G {	/* -G<grdfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct L {	/* -L<flag> */
		GMT_LONG active;
		char mode[4];
	} L;
	struct T {	/* -T */
		GMT_LONG active;
	} T;
};

void *New_grdsample_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDSAMPLE_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDSAMPLE_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	return ((void *)C);
}

void Free_grdsample_Ctrl (struct GMT_CTRL *GMT, struct GRDSAMPLE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->G.file) free ((void *)C->G.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdsample_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {

	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdsample %s [API] - Resample a grid file onto a new grid\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdsample <old_grdfile> -G<new_grdfile> [%s] [-L<flag>]\n", GMT_I_OPT);
	GMT_message (GMT, "\t[%s] [-T] [%s] [%s] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT, GMT_n_OPT, GMT_r_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<old_grdfile> is data set to be resampled.\n");
	GMT_message (GMT, "\t-G Sets the name of the interpolated output grid file.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_message (GMT, "\t   When omitted: grid spacing is copied from input grid.\n");
	GMT_message (GMT, "\t-L Sets boundary conditions.  <flag> can be either\n");
	GMT_message (GMT, "\t   g for geographic boundary conditions or one or both of\n");
	GMT_message (GMT, "\t   x for periodic boundary conditions on x.\n");
	GMT_message (GMT, "\t   y for periodic boundary conditions on y.\n");
	GMT_message (GMT, "\t-R Specifies a subregion [Default is old region].\n");
	GMT_message (GMT, "\t-T Toggles between grid registration and pixel registration.\n");
	GMT_explain_options (GMT, "VfnF.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdsample_parse (struct GMTAPI_CTRL *C, struct GRDSAMPLE_CTRL *Ctrl, struct GMT_EDGEINFO *edgeinfo, struct GMT_OPTION *options) {

	/* This parses the options provided to grdsample and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
#ifdef GMT_COMPAT
	GMT_LONG ii = 0, jj = 0;
	char format[BUFSIZ];
#endif
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	GMT_boundcond_init (GMT, edgeinfo);

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'G':	/* Output file */
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':	/* Grid spacings */
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':	/* BCs */
				Ctrl->L.active = TRUE;
				strncpy (Ctrl->L.mode, opt->arg, (size_t)4);
				break;
#ifdef GMT_COMPAT
			case 'N':	/* Backwards compatible.  nx/ny can now be set with -I */
				Ctrl->I.active = TRUE;
				sscanf (opt->arg, "%" GMT_LL "d/%" GMT_LL "d", &ii, &jj);
				if (jj == 0) jj = ii;
				sprintf (format, "%" GMT_LL "d+/%" GMT_LL "d+", ii, jj);
				GMT_getinc (GMT, format, Ctrl->I.inc);
				break;
#endif

			case 'T':	/* Convert from pixel file <-> gridfile */
				Ctrl->T.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.active, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single input grid file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, GMT->common.r.active && Ctrl->T.active, 
					"Syntax error: Only one of -r, -T may be specified\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0), 
					"Syntax error -I: Must specify positive increments\n");
	if (Ctrl->L.active && GMT_boundcond_parse (GMT, edgeinfo, Ctrl->L.mode)) n_errors++;
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdsample_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdsample (struct GMTAPI_CTRL *API, struct GMT_OPTION *options) {

	GMT_LONG error = 0, ij, row, col;
	
	char format[BUFSIZ];
	
	double *lon = NULL, lat;

	struct GMT_EDGEINFO edgeinfo;
	struct GMT_BCR bcr;
	struct GRDSAMPLE_CTRL *Ctrl = NULL;
	struct GMT_GRID *Gin = NULL, *Gout = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdsample_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdsample_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdsample", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRf", "nr" GMT_OPT("FQ"), options))) Return (error);
	Ctrl = (struct GRDSAMPLE_CTRL *) New_grdsample_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdsample_parse (API, Ctrl, &edgeinfo, options))) Return (error);

	/*---------------------------- This is the grdsample main code ----------------------------*/

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->In.file), (void **)&Gin)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
	GMT_boundcond_init (GMT, &edgeinfo);

	Gout = GMT_create_grid (GMT);
	GMT_memcpy (Gout->header->wesn, (GMT->common.R.active ? GMT->common.R.wesn : Gin->header->wesn), 4, double);

	if (Ctrl->I.active)
		GMT_memcpy (Gout->header->inc, Ctrl->I.inc, 2, double);
	else
		GMT_memcpy (Gout->header->inc, Gin->header->inc, 2, double);

	if (Ctrl->T.active)
		Gout->header->registration = !Gin->header->registration;
	else if (GMT->common.r.active)
		Gout->header->registration = GMT_PIXEL_REG;
	else
		Gout->header->registration = Gin->header->registration;

	GMT_RI_prepare (GMT, Gout->header);	/* Ensure -R -I consistency and set nx, ny */
	GMT_set_grddim (GMT, Gout->header);
	GMT_boundcond_param_prep (GMT, Gin->header, &edgeinfo);

	if (GMT->common.R.active) {
		if (!edgeinfo.nxp && (Gout->header->wesn[XLO] < Gin->header->wesn[XLO] - GMT_SMALL || Gout->header->wesn[XHI] > Gin->header->wesn[XHI] + GMT_SMALL)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: Selected region exceeds the X-boundaries of the grid file!\n");
			return (EXIT_FAILURE);
		}
		else if (!edgeinfo.nyp && (Gout->header->wesn[YLO] < Gin->header->wesn[YLO] - GMT_SMALL || Gout->header->wesn[YHI] > Gin->header->wesn[YHI] + GMT_SMALL)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: Selected region exceeds the Y-boundaries of the grid file!\n");
			return (EXIT_FAILURE);
		}
	}

	if (!Ctrl->I.active) {
		Gout->header->inc[GMT_X] = GMT_get_inc (Gout->header->wesn[XLO], Gout->header->wesn[XHI], Gout->header->nx, Gout->header->registration);
		Gout->header->inc[GMT_Y] = GMT_get_inc (Gout->header->wesn[YLO], Gout->header->wesn[YHI], Gout->header->ny, Gout->header->registration);
	}

	GMT_err_fail (GMT, GMT_grd_RI_verify (GMT, Gout->header, 1), Ctrl->G.file);

	Gout->data = GMT_memory (GMT, NULL, Gout->header->size, float);

	GMT_grd_init (GMT, Gin->header, options, TRUE);

	sprintf (format, "New grid (%s/%s/%s/%s) nx = %%d ny = %%d dx = %s dy = %s registration = %%d\n", 
		GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, 
		GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);

	GMT_report (GMT, GMT_MSG_NORMAL, format, Gout->header->wesn[XLO], Gout->header->wesn[XHI], 
		Gout->header->wesn[YLO], Gout->header->wesn[YHI], Gout->header->nx, Gout->header->ny, Gout->header->inc[GMT_X], Gout->header->inc[GMT_Y], Gout->header->registration);

	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, (void **)&(Ctrl->In.file), (void **)&Gin)) Return (GMT_DATA_READ_ERROR);	/* Get subset */
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (Gout->header->inc[GMT_X] > Gin->header->inc[GMT_X] || Gout->header->inc[GMT_Y] > Gin->header->inc[GMT_Y]) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning: A coarser sampling interval may lead to aliasing");
	}
	/* Initialize bcr structure */

	GMT_bcr_init (GMT, Gin->header, GMT->common.n.interpolant, GMT->common.n.threshold, &bcr);

	/* Set boundary conditions  */

	GMT_boundcond_grid_set (GMT, Gin, &edgeinfo);

	/* Precalculate longitudes */

	lon = GMT_memory (GMT, NULL, Gout->header->nx, double);
	for (col = 0; col < Gout->header->nx; col++) {
		lon[col] = GMT_grd_col_to_x (col, Gout->header);
		if (!edgeinfo.nxp)
			/* Nothing */;
		else if (lon[col] > Gin->header->wesn[XHI])
			lon[col] -= Gin->header->inc[GMT_X] * edgeinfo.nxp;
		else if (lon[col] < Gin->header->wesn[XLO])
			lon[col] += Gin->header->inc[GMT_X] * edgeinfo.nxp;
	}

	/* Loop over input point and estinate output values */
	
	GMT_row_loop (Gout, row) {
		lat = GMT_grd_row_to_y (row, Gout->header);
		if (!edgeinfo.nyp)
			/* Nothing */;
		else if (lat > Gin->header->wesn[YHI])
			lat -= Gin->header->inc[GMT_Y] * edgeinfo.nyp;
		else if (lat < Gin->header->wesn[YLO])
			lat += Gin->header->inc[GMT_Y] * edgeinfo.nyp;
		GMT_col_loop (Gout, row, col, ij) Gout->data[ij] = (float)GMT_get_bcr_z (GMT, Gin, lon[col], lat, &bcr);
	}

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->G.file, (void *)Gout);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	GMT_free (GMT, lon);

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Gin);
	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Gout);

	Return (GMT_OK);
}
