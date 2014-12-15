/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: grdproject reads a geographical grid file and evaluates the grid at new grid positions
 * specified by the map projection and new dx/dy values using a weighted average of all
 * points within the search radius. Optionally, grdproject may perform the inverse
 * transformation, going from rectangular coordinates to geographical.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Ver:		5 API
 */

#define THIS_MODULE_NAME	"grdproject"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Forward and inverse map transformation of grids"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-JRVnr" GMT_OPT("FS")

struct GRDPROJECT_CTRL {
	struct In {	/* Input grid */
		bool active;
		char *file;
	} In;
	struct A {	/* -A[k|m|n|i|c|p] */
		bool active;
		char unit;
	} A;
	struct C {	/* -C[<dx/dy>] */
		bool active;
		double easting, northing;
	} C;
	struct D {	/* -Ddx[/dy] */
		bool active;
		double inc[2];
	} D;
	struct E {	/* -E<dpi> */
		bool active;
		int dpi;
	} E;
	struct G {	/* -G */
		bool active;
		char *file;
	} G;
	struct I {	/* -I */
		bool active;
	} I;
	struct M {	/* -Mc|i|m */
		bool active;
		char unit;
	} M;
};

void *New_grdproject_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDPROJECT_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDPROJECT_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
		
	return (C);
}

void Free_grdproject_Ctrl (struct GMT_CTRL *GMT, struct GRDPROJECT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->G.file) free (C->G.file);	
	GMT_free (GMT, C);	
}

int GMT_grdproject_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdproject <ingrid> -G<outgrid> %s [-A[%s|%s]] [-C[<dx>/<dy>]]\n",
	             GMT_J_OPT, GMT_LEN_UNITS2_DISPLAY, GMT_DIM_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t[-D%s] [-E<dpi>] [-I] [-M%s]\n", GMT_inc_OPT, GMT_DIM_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_n_OPT, GMT_r_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is data set to be projected.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set name of output grid\n");
	GMT_Option (API, "J");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Force projected values to be in actual distance units [Default uses the given map scale].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify unit by appending e (meter), f (foot) k (km), M (mile), n (nautical mile), u (survey foot),\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or i (inch), c (cm), or p (points) [e].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Coordinates are relative to projection center [Default is relative to lower left corner].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally append dx/dy to add (or subtract if -I) (i.e., false easting & northing) [0/0].\n");
	GMT_inc_syntax (API->GMT, 'D', 0);
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set dpi for output grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Inverse transformation from rectangular to geographical.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Temporarily reset PROJ_LENGTH_UNIT to be c (cm), i (inch), or p (point).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be used if -A is set.\n");
	GMT_Option (API, "R");
	GMT_Option (API, "V,n,r,.");

	return (EXIT_FAILURE);
}

int GMT_grdproject_parse (struct GMT_CTRL *GMT, struct GRDPROJECT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdproject and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	int sval, ii = 0, jj = 0;
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

			case 'A':	/* Force meters */
				Ctrl->A.active = true;
				Ctrl->A.unit = opt->arg[0];
				break;
			case 'C':	/* Coordinates relative to origin */
				Ctrl->C.active = true;
				if (opt->arg[0]) 	/* Also gave shifts */
					n_errors += GMT_check_condition (GMT, sscanf (opt->arg, "%lf/%lf", &Ctrl->C.easting, &Ctrl->C.northing) != 2,
						 "Syntax error: Expected -C[<false_easting>/<false_northing>]\n");
				break;
			case 'D':	/* Grid spacings */
				Ctrl->D.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->D.inc)) {
					GMT_inc_syntax (GMT, 'D', 1);
					n_errors++;
				}
				break;
			case 'E':	/* Set dpi of grid */
				Ctrl->E.active = true;
				sval = atoi (opt->arg);
				n_errors += GMT_check_condition (GMT, sval <= 0, "Syntax error -E option: Must specify positive dpi\n");
				Ctrl->E.dpi = sval;
				break;
			case 'G':	/* Output file */
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':	/* Inverse projection */
				Ctrl->I.active = true;
				break;
			case 'M':	/* Directly specify units */
				Ctrl->M.active = true;
				Ctrl->M.unit = opt->arg[0];
				n_errors += GMT_check_condition (GMT, !Ctrl->M.unit,
							"Syntax error -M option: projected measure unit must be one of 'c', i', or 'p'\n");
				break;
			case 'N':	/* GMT4 Backwards compatible.  nx/ny can now be set with -D */
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -N option is deprecated; use -D instead.\n");
					Ctrl->D.active = true;
					sscanf (opt->arg, "%d/%d", &ii, &jj);
					if (jj == 0) jj = ii;
					sprintf (format, "%d+/%d+", ii, jj);
					if (GMT_getinc (GMT, format, Ctrl->D.inc)) {
						GMT_inc_syntax (GMT, 'D', 1);
						n_errors++;
					}
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->D.inc, &GMT->common.r.registration, &Ctrl->D.active);

	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->M.active + Ctrl->A.active) == 2, "Syntax error: Can specify only one of -A and -M\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->D.active + Ctrl->E.active) > 1, "Syntax error: Must specify only one of -D or -E\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && (Ctrl->D.inc[GMT_X] <= 0.0 || Ctrl->D.inc[GMT_Y] < 0.0),
	                                 "Syntax error -D option: Must specify positive increment(s)\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdproject_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdproject (void *V_API, int mode, void *args)
{
	bool set_n = false, shift_xy = false;
	unsigned int use_nx = 0, use_ny = 0, offset, k, unit = 0;
	int error = 0;

	char format[GMT_BUFSIZ] = {""}, unit_name[GMT_GRID_UNIT_LEN80] = {""}, scale_unit_name[GMT_GRID_UNIT_LEN80] = {""};

	double wesn[4];
	double xmin, xmax, ymin, ymax, inch_to_unit, unit_to_inch, fwd_scale, inv_scale;

	struct GMT_GRID *Geo = NULL, *Rect = NULL;
	struct GRDPROJECT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdproject_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdproject_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdproject_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_grdproject_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdproject_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdproject main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grid\n");
	GMT_set_pad (GMT, 2U);	/* Ensure space for BCs in case an API passed pad == 0 */
	if ((Ctrl->D.active + Ctrl->E.active) == 0) set_n = true;
	if (Ctrl->M.active) GMT_err_fail (GMT, GMT_set_measure_unit (GMT, Ctrl->M.unit), "-M");
	shift_xy = !(Ctrl->C.easting == 0.0 && Ctrl->C.northing == 0.0);
	
	unit = GMT_check_scalingopt (GMT, 'A', Ctrl->A.unit, scale_unit_name);
	GMT_init_scales (GMT, unit, &fwd_scale, &inv_scale, &inch_to_unit, &unit_to_inch, unit_name);

	if (GMT->common.R.active)	/* Load the w/e/s/n from -R */
		GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);
	else {	/* If -R was not given we infer the option via the input grid */
		char opt_R[GMT_BUFSIZ];
		struct GMT_GRID *G = NULL;
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
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
			if (GMT->current.proj.projection == GMT_UTM && GMT->current.proj.utm_hemisphere == -1 && y_c > 0) y_c *= -1;
			if (y_c > 0)
				GMT_parse_common_options (GMT, "R", 'R', "-180/180/0/80");
			else
				GMT_parse_common_options (GMT, "R", 'R', "-180/180/-80/0");
			if (GMT->current.proj.projection == GMT_UTM && GMT->current.proj.utm_hemisphere == -1 && y_c < 0) y_c *= -1;	/* Undo the *-1 (only for the UTM case) */ 
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
			if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "First opt_R\t %s\t%g\t%g\n", opt_R, x_c, y_c);
			GMT->common.R.active = false;	/* We need to reset this to not fall into non-wanted branch deeper down */
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
			if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "Second opt_R\t %s\n", opt_R);
			GMT->common.R.active = false;
			GMT_parse_common_options (GMT, "R", 'R', opt_R);
			GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Load up our best wesn setting - it will be used below if -I */
		}
		if (GMT_Destroy_Data (API, &G) != GMT_OK) {
			Return (API->error);
		}
	}

	if (GMT_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_RUNTIME_ERROR);

	if (Ctrl->I.active) {			/* Must flip the column types since in is Cartesian and out is geographic */
		GMT_set_geographic (GMT, GMT_OUT);	/* Inverse projection expects x,y and gives lon, lat */
		GMT_set_cartesian (GMT, GMT_IN);
	}

	xmin = (Ctrl->C.active) ? GMT->current.proj.rect[XLO] - GMT->current.proj.origin[GMT_X] : GMT->current.proj.rect[XLO];
	xmax = (Ctrl->C.active) ? GMT->current.proj.rect[XHI] - GMT->current.proj.origin[GMT_X] : GMT->current.proj.rect[XHI];
	ymin = (Ctrl->C.active) ? GMT->current.proj.rect[YLO] - GMT->current.proj.origin[GMT_Y] : GMT->current.proj.rect[YLO];
	ymax = (Ctrl->C.active) ? GMT->current.proj.rect[YHI] - GMT->current.proj.origin[GMT_Y] : GMT->current.proj.rect[YHI];
	if (Ctrl->A.active) {	/* Convert to chosen units */
		strncpy (unit_name, scale_unit_name, GMT_GRID_UNIT_LEN80);
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

	sprintf (format, "(%s/%s/%s/%s)", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
	                                  GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);

	if (Ctrl->I.active) {	/* Transforming from rectangular projection to geographical */

		/* if (GMT->common.R.oblique) double_swap (s, e); */  /* Got w/s/e/n, make into w/e/s/n */

		if ((Rect = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);
		}

		if ((Geo = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, Rect)) == NULL) Return (API->error);	/* Just to get a header we can change */

		if (GMT_IS_AZIMUTHAL(GMT) && GMT->current.proj.polar) {	/* Watch out for polar cap grids */
			if (doubleAlmostEqual (GMT->current.proj.pole, -90.0)) {	/* Covers S pole; implies 360 longitude range */
				wesn[XLO] = -180.0;	wesn[XHI] = +180.0;	wesn[YHI] = MAX(wesn[YLO], wesn[YHI]);	wesn[YLO] = -90.0;
			}
			else if (doubleAlmostEqual (GMT->current.proj.pole, +90.0)) {	/* Covers N pole; implies 360 longitude range */
				wesn[XLO] = -180.0;	wesn[XHI] = +180.0;	wesn[YLO] = MIN(wesn[YLO], wesn[YHI]);	wesn[YHI] = +90.0;
			}
		}
		GMT_memcpy (Geo->header->wesn, wesn, 4, double);

		offset = Rect->header->registration;	/* Same as input */
		if (GMT->common.r.active) offset = !offset;	/* Toggle */
		if (set_n) {
			use_nx = Rect->header->nx;
			use_ny = Rect->header->ny;
		}
		GMT_err_fail (GMT, GMT_project_init (GMT, Geo->header, Ctrl->D.inc, use_nx, use_ny, Ctrl->E.dpi, offset), Ctrl->G.file);
		GMT_set_grddim (GMT, Geo->header);
		if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, NULL, NULL, 0, 0, Geo) == NULL) Return (API->error);
		GMT_grd_init (GMT, Geo->header, options, true);
		GMT_BC_init (GMT, Geo->header);

		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Transform ");
			GMT_Message (API, GMT_TIME_NONE, format, Geo->header->wesn[XLO], Geo->header->wesn[XHI], Geo->header->wesn[YLO], Geo->header->wesn[YHI]);
			GMT_Message (API, GMT_TIME_NONE, " <-- ");
			GMT_Message (API, GMT_TIME_NONE, format, xmin, xmax, ymin, ymax);
			GMT_Message (API, GMT_TIME_NONE, " [%s]\n", unit_name);
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
		GMT_set_grdinc (GMT, Rect->header);	/* Update inc and r_inc given changes to wesn */

		sprintf (Geo->header->x_units, "longitude [degrees_east]");
		sprintf (Geo->header->y_units, "latitude [degrees_north]");

		Geo->header->ProjRefPROJ4 = "+proj=longlat +no_defs";	/* HOWEVER, this may be quite incorrect for we are ignoring the DATUM */

		GMT_grd_project (GMT, Rect, Geo, true);

		GMT_set_pad (GMT, API->pad);	/* Reset to session default pad before output */
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Geo)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Geo) != GMT_OK) {
			Return (API->error);
		}
	}
	else {	/* Forward projection from geographical to rectangular grid */

		if ((Geo = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);
		}

		if ((Rect = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, Geo)) == NULL) Return (API->error);	/* Just to get a header we can change */
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

		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Transform ");
			GMT_Message (API, GMT_TIME_NONE, format, Geo->header->wesn[XLO], Geo->header->wesn[XHI], Geo->header->wesn[YLO], Geo->header->wesn[YHI]);
			GMT_Message (API, GMT_TIME_NONE, " --> ");
			GMT_Message (API, GMT_TIME_NONE, format, xmin, xmax, ymin, ymax);
			GMT_Message (API, GMT_TIME_NONE, " [%s]\n", unit_name);
		}

		offset = Geo->header->registration;	/* Same as input */
		if (GMT->common.r.active) offset = !offset;	/* Toggle */

		GMT_err_fail (GMT, GMT_project_init (GMT, Rect->header, Ctrl->D.inc, use_nx, use_ny, Ctrl->E.dpi, offset), Ctrl->G.file);
		GMT_set_grddim (GMT, Rect->header);
		if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, NULL, NULL, 0, 0, Rect) == NULL) Return (API->error);
		GMT_BC_init (GMT, Rect->header);
		GMT_grd_project (GMT, Geo, Rect, false);
		GMT_grd_init (GMT, Rect->header, options, true);

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
		GMT_set_grdinc (GMT, Rect->header);	/* Update inc and r_inc given changes to wesn */
		strncpy (Rect->header->x_units, unit_name, GMT_GRID_UNIT_LEN80);
		strncpy (Rect->header->y_units, unit_name, GMT_GRID_UNIT_LEN80);

		Rect->header->ProjRefPROJ4 = GMT_export2proj4(GMT);	/* Convert the GMT -J<...> into a proj4 string and save it in the header */

		/* rect xy values are here in GMT projected units chosen by user */

		GMT_set_pad (GMT, API->pad);	/* Reset to session default pad before output */
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Rect)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, Rect) != GMT_OK) {
			Return (API->error);
		}
		free(Rect->header->ProjRefPROJ4);
	}

	Return (GMT_OK);
}
