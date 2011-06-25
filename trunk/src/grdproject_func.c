/*--------------------------------------------------------------------
 *	$Id: grdproject_func.c,v 1.25 2011-06-25 01:59:47 guru Exp $
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
 * Brief synopsis: grdproject reads a geographical grid file and evaluates the grid at new grid positions
 * specified by the map projection and new dx/dy values using a weighted average of all
 * points within the search radius. Optionally, grdproject may perform the inverse
 * transformation, going from rectangular coordinates to geographical.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Ver:		5 API
 */

#include "gmt.h"

struct GRDPROJECT_CTRL {
	struct In {	/* Input grid */
		GMT_LONG active;
		char *file;
	} In;
	struct A {	/* -A[k|m|n|i|c|p] */
		GMT_LONG active;
		char unit;
	} A;
	struct C {	/* -C[<dx/dy>] */
		GMT_LONG active;
		double easting, northing;
	} C;
	struct D {	/* -Ddx[/dy] */
		GMT_LONG active;
		double inc[2];
	} D;
	struct E {	/* -E<dpi> */
		GMT_LONG active;
		GMT_LONG dpi;
	} E;
	struct G {	/* -G */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -I */
		GMT_LONG active;
	} I;
	struct M {	/* -Mc|i|m */
		GMT_LONG active;
		char unit;
	} M;
};

void *New_grdproject_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDPROJECT_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDPROJECT_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
		
	return ((void *)C);
}

void Free_grdproject_Ctrl (struct GMT_CTRL *GMT, struct GRDPROJECT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->G.file) free ((void *)C->G.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdproject_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdproject %s [API] - Forward and inverse map transformation of grids\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdproject <ingrid> -G<outgrid> %s\n", GMT_J_OPT);
	GMT_message (GMT, "\t[-A[%s|%s]] [-C[<dx>/<dy>]] [-D%s] [-E<dpi>]\n", GMT_LEN_UNITS2_DISPLAY, GMT_DIM_UNITS_DISPLAY, GMT_inc_OPT);
	GMT_message (GMT, "\t[-I] [-M%s] [%s]\n", GMT_DIM_UNITS_DISPLAY, GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\n", GMT_V_OPT, GMT_n_OPT, GMT_r_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<ingrid> is data set to be projected.\n");
	GMT_message (GMT, "\t-G Set name of output grid\n");
	GMT_explain_options (GMT, "J");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Force projected values to be in actual meters [Default uses the given map scale].\n");
	GMT_message (GMT, "\t   Specify another unit by appending (f)eet, (k)m, (M)ile, (n)autical mile,\n");
	GMT_message (GMT, "\t   or i (inch), c (cm), or p (points) [e].\n");
	GMT_message (GMT, "\t-C Coordinates are relative to projection center [Default is relative to lower left corner].\n");
	GMT_message (GMT, "\t   Optionally append dx/dy to add (or subtract if -I) (i.e., false easting & northing) [0/0].\n");
	GMT_inc_syntax (GMT, 'D', 0);
	GMT_message (GMT, "\t-E Set dpi for output grid.\n");
	GMT_message (GMT, "\t-I Inverse transformation from rectangular to geographical.\n");
	GMT_message (GMT, "\t-M Temporarily reset PROJ_LENGTH_UNIT to be c (cm), i (inch), or p (point).\n");
	GMT_message (GMT, "\t   Cannot be used if -A is set.\n");
	GMT_explain_options (GMT, "R");
	GMT_explain_options (GMT, "VnF.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdproject_parse (struct GMTAPI_CTRL *C, struct GRDPROJECT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdproject and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0, set_n = FALSE;
#ifdef GMT_COMPAT
	GMT_LONG ii = 0, jj = 0;
	char format[GMT_BUFSIZ];
#endif
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Force meters */
				Ctrl->A.active = TRUE;
				Ctrl->A.unit = opt->arg[0];
				break;
			case 'C':	/* Coordinates relative to origin */
				Ctrl->C.active = TRUE;
				if (opt->arg[0]) 	/* Also gave shifts */
					n_errors += GMT_check_condition (GMT, sscanf (opt->arg, "%lf/%lf", &Ctrl->C.easting, &Ctrl->C.northing) != 2,
						 "Syntax error: Expected -C[<false_easting>/<false_northing>]\n");
				break;
			case 'D':	/* Grid spacings */
				Ctrl->D.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->D.inc)) {
					GMT_inc_syntax (GMT, 'D', 1);
					n_errors++;
				}
				break;
			case 'E':	/* Set dpi of grid */
				Ctrl->E.active = TRUE;
				Ctrl->E.dpi = atoi (opt->arg);
				break;
			case 'G':	/* Output file */
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':	/* Inverse projection */
				Ctrl->I.active = TRUE;
				break;
			case 'M':	/* Directly specify units */
				Ctrl->M.active = TRUE;
				Ctrl->M.unit = opt->arg[0];
				break;
#ifdef GMT_COMPAT
			case 'N':	/* Backwards compatible.  nx/ny can now be set with -D */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -N option is deprecated; use -D instead.\n");
				Ctrl->D.active = TRUE;
				sscanf (opt->arg, "%" GMT_LL "d/%" GMT_LL "d", &ii, &jj);
				if (jj == 0) jj = ii;
				sprintf (format, "%" GMT_LL "d+/%" GMT_LL "d+", ii, jj);
				GMT_getinc (GMT, format, Ctrl->D.inc);
				break;
#endif
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if ((Ctrl->D.active + Ctrl->E.active) == 0) set_n = TRUE;


	GMT_check_lattice (GMT, Ctrl->D.inc, &GMT->common.r.active, &Ctrl->D.active);

	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->M.active + Ctrl->A.active) == 2, "Syntax error: Can specify only one of -A and -M\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->D.active + Ctrl->E.active) > 1, "Syntax error: Must specify only one of -D or -E\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && (Ctrl->D.inc[GMT_X] <= 0.0 || Ctrl->D.inc[GMT_Y] < 0.0), "Syntax error -D option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && Ctrl->E.dpi <= 0, "Syntax error -E option: Must specify positive dpi\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdproject_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdproject (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, set_n = FALSE, shift_xy = FALSE, offset, k, unit = 0;
	GMT_LONG use_nx = 0, use_ny = 0;

	char format[GMT_BUFSIZ], unit_name[GRD_UNIT_LEN80], scale_unit_name[GRD_UNIT_LEN80];

	double wesn[4];
	double xmin, xmax, ymin, ymax, inch_to_unit, unit_to_inch, fwd_scale, inv_scale;

	struct GMT_GRID *Geo = NULL, *Rect = NULL;
	struct GRDPROJECT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdproject_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdproject_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdproject", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJR", "nr" GMT_OPT("FS"), options))) Return (error);
	Ctrl = (struct GRDPROJECT_CTRL *) New_grdproject_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdproject_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdproject main code ----------------------------*/

	if ((Ctrl->D.active + Ctrl->E.active) == 0) set_n = TRUE;
	if (Ctrl->M.active) GMT_err_fail (GMT, GMT_set_measure_unit (GMT, Ctrl->M.unit), "-M");
	shift_xy = !(Ctrl->C.easting == 0.0 && Ctrl->C.northing == 0.0);
	
	unit = GMT_check_scalingopt (GMT, 'A', Ctrl->A.unit, scale_unit_name);
	GMT_init_scales (GMT, unit, &fwd_scale, &inv_scale, &inch_to_unit, &unit_to_inch, unit_name);

	if (Ctrl->I.active) {	/* Must flip the column types since in is Cartesian and out is geographic */
		GMT->current.io.col_type[GMT_OUT][GMT_X] = GMT_IS_LON;	GMT->current.io.col_type[GMT_OUT][GMT_Y] = GMT_IS_LAT;	/* Inverse projection expects x,y and gives lon, lat */
		GMT->current.io.col_type[GMT_IN][GMT_X] = GMT->current.io.col_type[GMT_IN][GMT_Y] = GMT_IS_FLOAT;
	}
	
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) Return (error);		/* Enables data input and sets access mode */
	if (GMT->common.R.active)	/* Load the w/e/s/n from -R */
		GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);
	else {	/* If -R was not given we infer the option via the input grid */
		char opt_R[GMT_BUFSIZ];
		struct GMT_GRID *G = NULL;
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->In.file), (void **)&G)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
		GMT_memcpy (wesn, G->header->wesn, 4, double);
		if (!Ctrl->I.active) {
			sprintf (opt_R, "%.12f/%.12f/%.12f/%.12f", wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
			GMT_parse_common_options (GMT, "R", 'R', opt_R);
			if (GMT_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_RUNTIME_ERROR);
		}
		else {			/* Do inverse transformation */
			double x_c, y_c, lon_t, lat_t, ww, ee, ss, nn;
			/* Obtain a first crude estimation of the good -R */
			x_c = (wesn[XLO] + wesn[XHI]) / 2.0; 		/* mid point of projected coords */
			y_c = (wesn[YLO] + wesn[YHI]) / 2.0; 
			if (GMT->current.proj.projection == GMT_UTM && !GMT->current.proj.north_pole && y_c > 0) y_c *= -1;
			if (y_c > 0)
				GMT_parse_common_options (GMT, "R", 'R', "-180/180/0/80");
			else
				GMT_parse_common_options (GMT, "R", 'R', "-180/180/-80/0");
			if (GMT->current.proj.projection == GMT_UTM && !GMT->current.proj.north_pole && y_c < 0) y_c *= -1;	/* Undo the *-1 (only for the UTM case) */ 
			if (shift_xy) {
				x_c -= Ctrl->C.easting;
				y_c -= Ctrl->C.northing;
			}
			/* Convert from 1:1 scale */ 
			if (unit) {
				x_c *= fwd_scale;
				y_c *= fwd_scale;
			}

			if (GMT_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_RUNTIME_ERROR);

			x_c *= GMT->current.proj.scale[GMT_X];
			y_c *= GMT->current.proj.scale[GMT_Y];

			if (Ctrl->C.active) {	/* Then correct so lower left corner is (0,0) */
				x_c += GMT->current.proj.origin[GMT_X];
				y_c += GMT->current.proj.origin[GMT_Y];
			}
			GMT_xy_to_geo (GMT, &lon_t, &lat_t, x_c, y_c);
			sprintf (opt_R, "%.12f/%.12f/%.12f/%.12f", lon_t-1, lon_t+1, lat_t-1, lat_t+1);
			if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "First opt_R\t %s\t%g\t%g\n", opt_R, x_c, y_c);
			GMT->common.R.active = FALSE;	/* We need to reset this to not fall into non-wanted branch deeper down */
			GMT_parse_common_options (GMT, "R", 'R', opt_R);
			if (GMT_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_RUNTIME_ERROR);

			/* Finally obtain the good limits */
			if (shift_xy) {
				wesn[XLO] -= Ctrl->C.easting;	wesn[XHI] -= Ctrl->C.easting;
				wesn[YLO] -= Ctrl->C.northing;	wesn[YHI] -= Ctrl->C.northing;
			}
			if (unit) for (k = 0; k < 4; k++) wesn[k] *= fwd_scale;
			
			wesn[XLO] *= GMT->current.proj.scale[GMT_X];	wesn[XHI] *= GMT->current.proj.scale[GMT_X];
			wesn[YLO] *= GMT->current.proj.scale[GMT_Y];	wesn[YHI] *= GMT->current.proj.scale[GMT_Y];

			if (Ctrl->C.active) {
				wesn[XLO] += GMT->current.proj.origin[GMT_X];	wesn[XHI] += GMT->current.proj.origin[GMT_X];
				wesn[YLO] += GMT->current.proj.origin[GMT_Y];	wesn[YHI] += GMT->current.proj.origin[GMT_Y];
			}

			GMT_xy_to_geo (GMT, &ww, &ss, wesn[XLO], wesn[YLO]);		/* SW corner */
			GMT_xy_to_geo (GMT, &ee, &nn, wesn[XHI], wesn[YHI]);		/* NE corner */
			sprintf (opt_R, "%.12f/%.12f/%.12f/%.12fr", ww, ss, ee, nn);
			if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "Second opt_R\t %s\n", opt_R);
			GMT->common.R.active = FALSE;
			GMT_parse_common_options (GMT, "R", 'R', opt_R);
		}
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&G);
	}

	if (GMT_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_RUNTIME_ERROR);

	xmin = (Ctrl->C.active) ? GMT->current.proj.rect[XLO] - GMT->current.proj.origin[GMT_X] : GMT->current.proj.rect[XLO];
	xmax = (Ctrl->C.active) ? GMT->current.proj.rect[XHI] - GMT->current.proj.origin[GMT_X] : GMT->current.proj.rect[XHI];
	ymin = (Ctrl->C.active) ? GMT->current.proj.rect[YLO] - GMT->current.proj.origin[GMT_Y] : GMT->current.proj.rect[YLO];
	ymax = (Ctrl->C.active) ? GMT->current.proj.rect[YHI] - GMT->current.proj.origin[GMT_Y] : GMT->current.proj.rect[YHI];
	if (Ctrl->A.active) {	/* Convert to chosen units */
		strncpy (unit_name, scale_unit_name, (size_t)GRD_UNIT_LEN80);
		xmin /= GMT->current.proj.scale[GMT_X];
		xmax /= GMT->current.proj.scale[GMT_X];
		ymin /= GMT->current.proj.scale[GMT_Y];
		ymax /= GMT->current.proj.scale[GMT_Y];
		if (unit) {	/* Change the 1:1 unit used */
			xmin *= fwd_scale;
			xmax *= fwd_scale;
			ymin *= fwd_scale;
			ymax *= fwd_scale;
		}
	}
	else {	/* Convert inches to chosen MEASURE */
		xmin *= inch_to_unit;
		xmax *= inch_to_unit;
		ymin *= inch_to_unit;
		ymax *= inch_to_unit;
	}
	if (shift_xy) {
		xmin += Ctrl->C.easting;
		xmax += Ctrl->C.easting;
		ymin += Ctrl->C.northing;
		ymax += Ctrl->C.northing;
	}

	sprintf (format, "(%s/%s/%s/%s)", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if (Ctrl->I.active) {	/* Transforming from rectangular projection to geographical */

		/* if (GMT->common.R.oblique) d_swap (s, e); */  /* Got w/s/e/n, make into w/e/s/n */

		Geo = GMT_create_grid (GMT);
		GMT_memcpy (Geo->header->wesn, wesn, 4, double);

		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&(Ctrl->In.file), (void **)&Rect)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

		offset = Rect->header->registration;	/* Same as input */
		if (GMT->common.r.active) offset = !offset;	/* Toggle */
		if (set_n) {
			use_nx = Rect->header->nx;
			use_ny = Rect->header->ny;
		}
		GMT_err_fail (GMT, GMT_project_init (GMT, Geo->header, Ctrl->D.inc, use_nx, use_ny, Ctrl->E.dpi, offset), Ctrl->G.file);
		GMT_set_grddim (GMT, Geo->header);
		Geo->data = GMT_memory (GMT, NULL, Geo->header->size, float);
		GMT_grd_init (GMT, Geo->header, options, TRUE);

		if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Transform ");
			GMT_message (GMT, format, Geo->header->wesn[XLO], Geo->header->wesn[XHI], Geo->header->wesn[YLO], Geo->header->wesn[YHI]);
			GMT_message (GMT, " <-- ");
			GMT_message (GMT, format, xmin, xmax, ymin, ymax);
			GMT_message (GMT, " [%s]\n", unit_name);
		}

		/* Modify input rect header if -A, -C, -M have been set */

		if (shift_xy) {
			Rect->header->wesn[XLO] -= Ctrl->C.easting;
			Rect->header->wesn[XHI] -= Ctrl->C.easting;
			Rect->header->wesn[YLO] -= Ctrl->C.northing;
			Rect->header->wesn[YHI] -= Ctrl->C.northing;

		}
		if (Ctrl->A.active) {	/* Convert from 1:1 scale */
			if (unit) for (k = 0; k < 4; k++) Rect->header->wesn[k] *= inv_scale;	/* Undo the 1:1 unit used */
			Rect->header->wesn[XLO] *= GMT->current.proj.scale[GMT_X];
			Rect->header->wesn[XHI] *= GMT->current.proj.scale[GMT_X];
			Rect->header->wesn[YLO] *= GMT->current.proj.scale[GMT_Y];
			Rect->header->wesn[YHI] *= GMT->current.proj.scale[GMT_Y];
		}
		else if (GMT->current.setting.proj_length_unit != GMT_INCH) {	/* Convert from inch to whatever */
			for (k = 0; k < 4; k++) Rect->header->wesn[k] *= unit_to_inch;
		}
		if (Ctrl->C.active) {	/* Then correct so lower left corner is (0,0) */
			Rect->header->wesn[XLO] += GMT->current.proj.origin[GMT_X];
			Rect->header->wesn[XHI] += GMT->current.proj.origin[GMT_X];
			Rect->header->wesn[YLO] += GMT->current.proj.origin[GMT_Y];
			Rect->header->wesn[YHI] += GMT->current.proj.origin[GMT_Y];
		}
		Rect->header->inc[GMT_X] = GMT_get_inc (GMT, Rect->header->wesn[XLO], Rect->header->wesn[XHI], Rect->header->nx, Rect->header->registration);
		Rect->header->inc[GMT_Y] = GMT_get_inc (GMT, Rect->header->wesn[YLO], Rect->header->wesn[YHI], Rect->header->ny, Rect->header->registration);
		sprintf (Geo->header->x_units, "longitude [degrees_east]");
		sprintf (Geo->header->y_units, "latitude [degrees_north]");

		GMT_grd_project (GMT, Rect, Geo, TRUE);

		GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->G.file, (void *)Geo);
	}
	else {	/* Forward projection from geographical to rectangular grid */

		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&(Ctrl->In.file), (void **)&Geo)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

		Rect = GMT_create_grid (GMT);
		GMT_memcpy (Rect->header->wesn, GMT->current.proj.rect, 4, double);
		if (Ctrl->A.active) {	/* Convert from 1:1 scale */
			if (unit) {	/* Undo the 1:1 unit used */
				Ctrl->D.inc[GMT_X] *= inv_scale;
				Ctrl->D.inc[GMT_Y] *= inv_scale;
			}
			Ctrl->D.inc[GMT_X] *= GMT->current.proj.scale[GMT_X];
			Ctrl->D.inc[GMT_Y] *= GMT->current.proj.scale[GMT_Y];
		}
		else if (GMT->current.setting.proj_length_unit != GMT_INCH) {	/* Convert from inch to whatever */
			Ctrl->D.inc[GMT_X] *= unit_to_inch;
			Ctrl->D.inc[GMT_Y] *= unit_to_inch;
		}
		if (set_n) {
			use_nx = Geo->header->nx;
			use_ny = Geo->header->ny;
		}

		if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Transform ");
			GMT_message (GMT, format, Geo->header->wesn[XLO], Geo->header->wesn[XHI], Geo->header->wesn[YLO], Geo->header->wesn[YHI]);
			GMT_message (GMT, " --> ");
			GMT_message (GMT, format, xmin, xmax, ymin, ymax);
			GMT_message (GMT, " [%s]\n", unit_name);
		}

		offset = Geo->header->registration;	/* Same as input */
		if (GMT->common.r.active) offset = !offset;	/* Toggle */

		GMT_err_fail (GMT, GMT_project_init (GMT, Rect->header, Ctrl->D.inc, use_nx, use_ny, Ctrl->E.dpi, offset), Ctrl->G.file);
		GMT_set_grddim (GMT, Rect->header);
		Rect->data = GMT_memory (GMT, NULL, Rect->header->size, float);
		GMT_grd_project (GMT, Geo, Rect, FALSE);
		GMT_grd_init (GMT, Rect->header, options, TRUE);

		/* Modify output rect header if -A, -C, -M have been set */

		if (Ctrl->C.active) {	/* Change origin from lower left to projection center */
			Rect->header->wesn[XLO] -= GMT->current.proj.origin[GMT_X];
			Rect->header->wesn[XHI] -= GMT->current.proj.origin[GMT_X];
			Rect->header->wesn[YLO] -= GMT->current.proj.origin[GMT_Y];
			Rect->header->wesn[YHI] -= GMT->current.proj.origin[GMT_Y];
		}
		if (Ctrl->A.active) {	/* Convert to 1:1 scale */
			Rect->header->wesn[XLO] /= GMT->current.proj.scale[GMT_X];
			Rect->header->wesn[XHI] /= GMT->current.proj.scale[GMT_X];
			Rect->header->wesn[YLO] /= GMT->current.proj.scale[GMT_Y];
			Rect->header->wesn[YHI] /= GMT->current.proj.scale[GMT_Y];
			if (unit) for (k = 0; k < 4; k++) Rect->header->wesn[k] *= fwd_scale;	/* Change the 1:1 unit used */
		}
		else if (GMT->current.setting.proj_length_unit != GMT_INCH) {	/* Convert from inch to whatever */
			for (k = 0; k < 4; k++) Rect->header->wesn[k] /= unit_to_inch;
		}
		if (shift_xy) {
			Rect->header->wesn[XLO] += Ctrl->C.easting;
			Rect->header->wesn[XHI] += Ctrl->C.easting;
			Rect->header->wesn[YLO] += Ctrl->C.northing;
			Rect->header->wesn[YHI] += Ctrl->C.northing;

		}
		Rect->header->inc[GMT_X] = GMT_get_inc (GMT, Rect->header->wesn[XLO], Rect->header->wesn[XHI], Rect->header->nx, Rect->header->registration);
		Rect->header->inc[GMT_Y] = GMT_get_inc (GMT, Rect->header->wesn[YLO], Rect->header->wesn[YHI], Rect->header->ny, Rect->header->registration);
		strcpy (Rect->header->x_units, unit_name);
		strcpy (Rect->header->y_units, unit_name);

		/* rect xy values are here in GMT projected units chosen by user */

		GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->G.file, (void *)Rect);
	}
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	Return (GMT_OK);
}
