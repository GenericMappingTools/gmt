/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#define THIS_MODULE_NAME	"grdsample"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Resample a grid onto a new lattice"
#define THIS_MODULE_KEYS	"<GI,GGO"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-RVfnr" GMT_ADD_x_OPT GMT_OPT("FQ")

struct GRDSAMPLE_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct T {	/* -T */
		bool active;
	} T;
};

void *New_grdsample_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDSAMPLE_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDSAMPLE_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	return (C);
}

void Free_grdsample_Ctrl (struct GMT_CTRL *GMT, struct GRDSAMPLE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->G.file) free (C->G.file);	
	GMT_free (GMT, C);	
}

int GMT_grdsample_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdsample <ingrid> -G<outgrid> [%s]\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-T] [%s] [%s]\n\t[%s] [%s]%s\n", GMT_Rgeo_OPT,
		GMT_V_OPT, GMT_f_OPT, GMT_n_OPT, GMT_r_OPT, GMT_x_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is data set to be resampled.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set the name of the interpolated output grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t   When omitted: grid spacing is copied from input grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-R Specify a subregion [Default is old region].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Toggle between grid registration and pixel registration.\n");
	GMT_Option (API, "V,f,n,r,x,.");

	return (EXIT_FAILURE);
}

int GMT_grdsample_parse (struct GMT_CTRL *GMT, struct GRDSAMPLE_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdsample and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int n_errors = 0, n_files = 0, ii = 0, jj = 0;
	char format[GMT_BUFSIZ];
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'G':	/* Output file */
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':	/* Grid spacings */
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':	/* BCs */
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -L is deprecated; -n+b%s was set instead, use this in the future.\n", opt->arg);
					strncpy (GMT->common.n.BC, opt->arg, 4U);
					/* We turn on geographic coordinates if -Lg is given by faking -fg */
					/* But since GMT_parse_f_option is private to gmt_init and all it does */
					/* in this case are 2 lines below we code it here */
					if (!strcmp (GMT->common.n.BC, "g")) {
						GMT_set_geographic (GMT, GMT_IN);
						GMT_set_geographic (GMT, GMT_OUT);
					}
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'N':	/* Backwards compatible.  nx/ny can now be set with -I */
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -N<nx>/<ny> is deprecated; use -I<nx>+/<ny>+ instead.\n");
					Ctrl->I.active = true;
					sscanf (opt->arg, "%d/%d", &ii, &jj);
					if (jj == 0) jj = ii;
					sprintf (format, "%d+/%d+", ii, jj);
					if (GMT_getinc (GMT, format, Ctrl->I.inc)) {
						GMT_inc_syntax (GMT, 'I', 1);
						n_errors++;
					}
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'T':	/* Convert from pixel file <-> gridfile */
				Ctrl->T.active = true;
				if (GMT->current.io.grd_info.active) GMT->common.r.active = false;	/* Override any implicit -r via -Rgridfile */
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.registration, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, (n_files != 1), "Syntax error: Must specify a single input grid file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, GMT->common.r.active && Ctrl->T.active && !GMT->current.io.grd_info.active, 
					"Syntax error: Only one of -r, -T may be specified\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0), 
					"Syntax error -I: Must specify positive increments\n");
	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdsample_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdsample (void *V_API, int mode, void *args) {

	int error = 0;
	unsigned int row, col, registration;
	
	uint64_t ij;
	
	char format[GMT_BUFSIZ];
	
	double *lon = NULL, lat, wesn[4], inc[2];

	struct GRDSAMPLE_CTRL *Ctrl = NULL;
	struct GMT_GRID *Gin = NULL, *Gout = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdsample_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdsample_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdsample_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdsample_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdsample_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdsample main code ----------------------------*/

	GMT_enable_threads (GMT);	/* Set number of active threads, if supported */
	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grid\n");
	GMT_set_pad (GMT, 2U);	/* Ensure space for BCs in case an API passed pad == 0 */
	if ((Gin = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	GMT_memcpy (wesn, (GMT->common.R.active ? GMT->common.R.wesn : Gin->header->wesn), 4, double);
	GMT_memcpy (inc, (Ctrl->I.active ? Ctrl->I.inc : Gin->header->inc), 2, double);

	if (Ctrl->T.active)
		registration = !Gin->header->registration;
	else if (GMT->common.r.active)
		registration = GMT->common.r.registration;
	else
		registration = Gin->header->registration;

	if (GMT->common.R.active) {	/* Make sure input grid and output -R has an overlap */
		if (wesn[YLO] < (Gin->header->wesn[YLO] - Gin->header->inc[GMT_Y]) || wesn[YHI] > (Gin->header->wesn[YHI] + Gin->header->inc[GMT_Y])) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: Selected region exceeds the Y-boundaries of the grid file by more than one y-increment!\n");
			Return (EXIT_FAILURE);
		}
		if (GMT_is_geographic (GMT, GMT_IN)) {	/* Must carefully check the longitude overlap */
			int shift = 0;
			if (Gin->header->wesn[XHI] < wesn[XLO]) shift += 360;
			if (Gin->header->wesn[XLO] > wesn[XHI]) shift -= 360;
			if (shift) {	/* Must modify header */
				Gin->header->wesn[XLO] += shift, Gin->header->wesn[XHI] += shift;
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "File %s region needed longitude adjustment to fit final grid region\n", Ctrl->In.file);
			}
		}
		else if (wesn[XLO] < (Gin->header->wesn[XLO] - Gin->header->inc[GMT_X]) || wesn[XHI] > (Gin->header->wesn[XHI] + Gin->header->inc[GMT_X])) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: Selected region exceeds the X-boundaries of the grid file by more than one x-increment!\n");
			return (EXIT_FAILURE);
		}
	}

	if ((Gout = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, wesn, inc, \
		registration, GMT_NOTSET, NULL)) == NULL) Return (API->error);

	sprintf (format, "Input  grid (%s/%s/%s/%s) nx = %%d ny = %%d dx = %s dy = %s registration = %%d\n", 
		GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, 
		GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);

	GMT_Report (API, GMT_MSG_VERBOSE, format, Gin->header->wesn[XLO], Gin->header->wesn[XHI], 
		Gin->header->wesn[YLO], Gin->header->wesn[YHI], Gin->header->nx, Gin->header->ny,
		Gin->header->inc[GMT_X], Gin->header->inc[GMT_Y], Gin->header->registration);

	memcpy (&format, "Output", 6);

	GMT_Report (API, GMT_MSG_VERBOSE, format, Gout->header->wesn[XLO], Gout->header->wesn[XHI], 
		Gout->header->wesn[YLO], Gout->header->wesn[YHI], Gout->header->nx, Gout->header->ny,
		Gout->header->inc[GMT_X], Gout->header->inc[GMT_Y], Gout->header->registration);

	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, Ctrl->In.file, Gin) == NULL) {	/* Get subset */
		Return (API->error);
	}

	if (Gout->header->inc[GMT_X] > Gin->header->inc[GMT_X]) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Output sampling interval in x exceeds input interval and may lead to aliasing.\n");
	if (Gout->header->inc[GMT_Y] > Gin->header->inc[GMT_Y]) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Output sampling interval in y exceeds input interval and may lead to aliasing.\n");

	/* Precalculate longitudes */

	lon = GMT_memory (GMT, NULL, Gout->header->nx, double);
	for (col = 0; col < Gout->header->nx; col++) {
		lon[col] = GMT_grd_col_to_x (GMT, col, Gout->header);
		if (!Gin->header->nxp)
			/* Nothing */;
		else if (lon[col] > Gin->header->wesn[XHI])
			lon[col] -= Gin->header->inc[GMT_X] * Gin->header->nxp;
		else if (lon[col] < Gin->header->wesn[XLO])
			lon[col] += Gin->header->inc[GMT_X] * Gin->header->nxp;
	}

	/* Loop over input point and estimate output values */
	
	Gout->header->z_min = FLT_MAX; Gout->header->z_max = -FLT_MAX;	/* Min/max for out */

#ifdef _OPENMP
#pragma omp parallel for private(row,col,lat,ij) shared(GMT,Gin,Gout,lon)
#endif
	for (row = 0; row < Gout->header->ny; row++) {
		lat = GMT_grd_row_to_y (GMT, row, Gout->header);
		if (!Gin->header->nyp)
			/* Nothing */;
		else if (lat > Gin->header->wesn[YHI])
			lat -= Gin->header->inc[GMT_Y] * Gin->header->nyp;
		else if (lat < Gin->header->wesn[YLO])
			lat += Gin->header->inc[GMT_Y] * Gin->header->nyp;
		for (col = 0; col < Gout->header->nx; col++) {
			ij = GMT_IJP (Gout->header, row, col);
			Gout->data[ij] = (float)GMT_get_bcr_z (GMT, Gin, lon[col], lat);
			if (Gout->data[ij] < Gout->header->z_min) Gout->header->z_min = Gout->data[ij];
			if (Gout->data[ij] > Gout->header->z_max) Gout->header->z_max = Gout->data[ij];
		}
	}

	if (!GMT->common.n.truncate && (Gout->header->z_min < Gin->header->z_min || Gout->header->z_max > Gin->header->z_max)) {	/* Report and possibly truncate output to input extrama */
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Output grid extrema [%g/%g] exceeds extrema of input grid [%g/%g]; to clip output use -n...+c""\n",
			Gout->header->z_min, Gout->header->z_max, Gin->header->z_min, Gin->header->z_max);
	}

	GMT_set_pad (GMT, API->pad);	/* Reset to session default pad before output */

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Gout)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Gout) != GMT_OK) {
		Return (API->error);
	}

	GMT_free (GMT, lon);

	Return (GMT_OK);
}
