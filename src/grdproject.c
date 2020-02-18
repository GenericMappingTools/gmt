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
 * Brief synopsis: grdproject reads a geographical grid file and evaluates the grid at new grid positions
 * specified by the map projection and new dx/dy values using a weighted average of all
 * points within the search radius. Optionally, grdproject may perform the inverse
 * transformation, going from rectangular coordinates to geographical.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Ver:		6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdproject"
#define THIS_MODULE_MODERN_NAME	"grdproject"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Forward and inverse map transformation of grids"
#define THIS_MODULE_KEYS	"<G{,GG}"
#define THIS_MODULE_NEEDS	"J"
#define THIS_MODULE_OPTIONS "-JRVnr" GMT_OPT("S")

struct GRDPROJECT_CTRL {
	struct GRDPRJ_In {	/* Input grid */
		bool active;
		char *file;
	} In;
	struct GRDPRJ_C {	/* -C[<dx/dy>] */
		bool active;
		double easting, northing;
	} C;
	struct GRDPRJ_E {	/* -E<dpi> */
		bool active;
		int dpi;
	} E;
	struct GRDPRJ_F {	/* -F[k|m|n|i|c|p] */
		bool active;
		char unit;
	} F;
	struct GRDPRJ_G {	/* -G */
		bool active;
		char *file;
	} G;
	struct GRDPRJ_I {	/* -I */
		bool active;
	} I;
	struct GRDPRJ_M {	/* -Mc|i|m */
		bool active;
		char unit;
	} M;
};

EXTERN_MSC void gmtlib_ellipsoid_name_convert (char *inname, char outname[]);

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDPROJECT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDPROJECT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDPROJECT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <ingrid> -G<outgrid> %s [-C[<dx>/<dy>]] [-D%s]\n",
		name, GMT_J_OPT, GMT_inc_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-E<dpi>] [-F[%s|%s]] [-I] [-M%s] [%s] [%s]\n", GMT_LEN_UNITS2_DISPLAY,
		GMT_DIM_UNITS_DISPLAY, GMT_DIM_UNITS_DISPLAY, GMT_Rgeo_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\n", GMT_n_OPT, GMT_r_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<ingrid> is data set to be projected.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set name of output grid\n");
	GMT_Option (API, "J");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Coordinates are relative to projection center [Default is relative to lower left corner].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally append dx/dy to add (or subtract if -I) (i.e., false easting & northing) [0/0].\n");
	gmt_inc_syntax (API->GMT, 'D', 0);
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set dpi for output grid.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Force projected values to be in actual distance units [Default uses the given map scale].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify unit by appending e (meter), f (foot) k (km), M (mile), n (nautical mile), u (survey foot),\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or i (inch), c (cm), or p (points) [e].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Inverse transformation from rectangular to geographical.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Temporarily reset PROJ_LENGTH_UNIT to be c (cm), i (inch), or p (point).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be used if -F is set.\n");
	GMT_Option (API, "R");
	GMT_Option (API, "V,n,r,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDPROJECT_CTRL *Ctrl, struct GMT_OPTION *options) {
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
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Coordinates relative to origin */
				Ctrl->C.active = true;
				if (opt->arg[0]) 	/* Also gave shifts */
					n_errors += gmt_M_check_condition (GMT, sscanf (opt->arg, "%lf/%lf", &Ctrl->C.easting, &Ctrl->C.northing) != 2,
						 "Expected -C[<false_easting>/<false_northing>]\n");
				break;
			case 'D':	/* Grid spacings */
				n_errors += gmt_parse_inc_option (GMT, 'D', opt->arg);
				break;
			case 'E':	/* Set dpi of grid */
				Ctrl->E.active = true;
				sval = atoi (opt->arg);
				n_errors += gmt_M_check_condition (GMT, sval <= 0, "Option -E: Must specify positive dpi\n");
				Ctrl->E.dpi = sval;
				break;
			case 'A':	/* Old Force specific unit option */
				if (gmt_M_compat_check (GMT, 5))	/* Honor old -A[<unit>] option */
					GMT_Report (API, GMT_MSG_COMPAT, "Option -A is deprecated; use -F instead.\n");
				else {
					n_errors += gmt_default_error (GMT, opt->option);
					break;
				}
				/* Fall through on purpose to get -F */
			case 'F':	/* Force specific unit */
				Ctrl->F.active = true;
				Ctrl->F.unit = opt->arg[0];
				break;
			case 'G':	/* Output file */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
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
				n_errors += gmt_M_check_condition (GMT, !Ctrl->M.unit,
							"Option -M: projected measure unit must be one of 'c', i', or 'p'\n");
				break;
			case 'N':	/* GMT4 Backwards compatible.  n_columns/n_rows can now be set with -D */
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "-N option is deprecated; use -D instead.\n");
					sscanf (opt->arg, "%d/%d", &ii, &jj);
					if (jj == 0) jj = ii;
					sprintf (format, "%d+/%d+", ii, jj);
					n_errors += gmt_parse_inc_option (GMT, 'D', format);
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	/* See if scale == 1:1 and if yes set -C -F */
	if (GMT->current.proj.pars[14] == 1)
		Ctrl->C.active = Ctrl->F.active = true;

	n_errors += gmt_M_check_condition (GMT, !Ctrl->In.file, "Must specify input file\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Option -G: Must specify output file\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active,
	                                   "Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition (GMT, (Ctrl->M.active + Ctrl->F.active) == 2,
	                                   "Can specify only one of -F and -M\n");
	n_errors += gmt_M_check_condition (GMT, (GMT->common.R.active[ISET] + Ctrl->E.active) > 1,
	                                   "Must specify only one of -D or -E\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.active[ISET] && (GMT->common.R.inc[GMT_X] <= 0.0 ||
	                                   GMT->common.R.inc[GMT_Y] < 0.0),
	                                   "Option -D: Must specify positive increment(s)\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

EXTERN_MSC int gmtlib_get_grdtype (struct GMT_CTRL *GMT, unsigned int direction, struct GMT_GRID_HEADER *h);

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdproject (void *V_API, int mode, void *args) {
	bool set_n = false, shift_xy = false;
	unsigned int use_nx = 0, use_ny = 0, offset, k, unit = 0;
	int error = 0;

	char format[GMT_LEN128] = {""}, unit_name[GMT_GRID_UNIT_LEN80] = {""}, scale_unit_name[GMT_GRID_UNIT_LEN80] = {""};

	double wesn[4];
	double xmin, xmax, ymin, ymax, inch_to_unit, unit_to_inch, fwd_scale, inv_scale;

	struct GMT_GRID *Geo = NULL, *Rect = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GRDPROJECT_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the grdproject main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input grid\n");
	gmt_set_pad (GMT, 2U);	/* Ensure space for BCs in case an API passed pad == 0 */
	if ((GMT->common.R.active[ISET] + Ctrl->E.active) == 0) set_n = true;
	if (Ctrl->M.active) gmt_M_err_fail (GMT, gmt_set_measure_unit (GMT, Ctrl->M.unit), "-M");
	shift_xy = !(Ctrl->C.easting == 0.0 && Ctrl->C.northing == 0.0);

	unit = gmt_check_scalingopt (GMT, 'A', Ctrl->F.unit, scale_unit_name);
	gmt_init_scales (GMT, unit, &fwd_scale, &inv_scale, &inch_to_unit, &unit_to_inch, unit_name);

	if (GMT->common.R.active[RSET])	/* Load the w/e/s/n from -R */
		gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);
	else {	/* If -R was not given we infer the option via the input grid */
		char opt_R[GMT_BUFSIZ];
		struct GMT_GRID *G = NULL;
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
		gmt_M_memcpy (wesn, G->header->wesn, 4, double);
		if (!Ctrl->I.active) {
			sprintf (opt_R, "%.12f/%.12f/%.12f/%.12f", wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
			gmt_parse_common_options (GMT, "R", 'R', opt_R);
			if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);
		}
		else {			/* Do inverse transformation */
			double x_c, y_c, lon_t, lat_t, xSW, ySW, xNW, yNW, xNE, yNE, xSE, ySE, x, y, xb, yT, yB, dx;
			/* Obtain a first crude estimation of the good -R */
			x_c = (wesn[XLO] + wesn[XHI]) / 2.0; 		/* mid point of projected coords */
			y_c = (wesn[YLO] + wesn[YHI]) / 2.0;
			if (GMT->current.proj.projection_GMT == GMT_UTM && GMT->current.proj.utm_hemisphere == -1 && y_c > 0) y_c *= -1;
			if (y_c > 0)
				gmt_parse_common_options (GMT, "R", 'R', "-180/180/0/80");
			else
				gmt_parse_common_options (GMT, "R", 'R', "-180/180/-80/0");
			if (GMT->current.proj.projection_GMT == GMT_UTM && GMT->current.proj.utm_hemisphere == -1 && y_c < 0) y_c *= -1;	/* Undo the *-1 (only for the UTM case) */
			if (shift_xy) {
				x_c -= Ctrl->C.easting;
				y_c -= Ctrl->C.northing;
			}
			/* Convert from 1:1 scale */
			if (unit) {
				x_c *= fwd_scale;
				y_c *= fwd_scale;
			}

			if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

			x_c *= GMT->current.proj.scale[GMT_X];
			y_c *= GMT->current.proj.scale[GMT_Y];

			if (Ctrl->C.active) {	/* Then correct so lower left corner is (0,0) */
				x_c += GMT->current.proj.origin[GMT_X];
				y_c += GMT->current.proj.origin[GMT_Y];
			}
			gmt_xy_to_geo (GMT, &lon_t, &lat_t, x_c, y_c);
			sprintf (opt_R, "%.12f/%.12f/%.12f/%.12f", lon_t-1, lon_t+1, lat_t-1, lat_t+1);
			if (gmt_M_is_verbose (GMT, GMT_MSG_WARNING)) GMT_Message (API, GMT_TIME_NONE, "First opt_R\t %s\t%g\t%g\n", opt_R, x_c, y_c);
			GMT->common.R.active[RSET] = false;	/* We need to reset this to not fall into non-wanted branch deeper down */
			gmt_parse_common_options (GMT, "R", 'R', opt_R);
			if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

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

			gmt_xy_to_geo (GMT, &xSW, &ySW, wesn[XLO], wesn[YLO]);		/* SW corner */
			gmt_xy_to_geo (GMT, &xNW, &yNW, wesn[XLO], wesn[YHI]);		/* NW corner */
			gmt_xy_to_geo (GMT, &xNE, &yNE, wesn[XHI], wesn[YHI]);		/* NE corner */
			gmt_xy_to_geo (GMT, &xSE, &ySE, wesn[XHI], wesn[YLO]);		/* SE corner */
			dx = (wesn[XHI] - wesn[XLO]) / 20;
			yT = MAX(yNW, yNE);		yB = MIN(ySW, ySE);
			for (k = 0; k < 20; k++) {                                  /* Run along the North and South boundary */
				xb = wesn[XLO] + k * dx;
				gmt_xy_to_geo (GMT, &x,  &y, xb, wesn[YHI]);
				yT = MAX(yT, y);
				gmt_xy_to_geo (GMT, &x,  &y, xb, wesn[YLO]);
				yB = MIN(yB, y);
			}
			sprintf (opt_R, "%.12f/%.12f/%.12f/%.12fr", MIN(xSW, xNW), yB, MAX(xNE, xSE), yT);

			if (gmt_M_is_verbose (GMT, GMT_MSG_WARNING)) GMT_Message (API, GMT_TIME_NONE, "Second opt_R\t %s\n", opt_R);
			GMT->common.R.active[RSET] = false;
			gmt_parse_common_options (GMT, "R", 'R', opt_R);
			gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);	/* Load up our best wesn setting - it will be used below if -I */
		}
		if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	if (Ctrl->I.active) {			/* Must flip the column types since in is Cartesian and out is geographic */
		gmt_set_geographic (GMT, GMT_OUT);	/* Inverse projection expects x,y and gives lon, lat */
		gmt_set_cartesian (GMT, GMT_IN);
	}
	else {
		gmt_set_geographic (GMT, GMT_IN);	/* Forward projection expects lon, lat and gives x,y */
		gmt_set_cartesian (GMT, GMT_OUT);
	}

	xmin = (Ctrl->C.active) ? GMT->current.proj.rect[XLO] - GMT->current.proj.origin[GMT_X] : GMT->current.proj.rect[XLO];
	xmax = (Ctrl->C.active) ? GMT->current.proj.rect[XHI] - GMT->current.proj.origin[GMT_X] : GMT->current.proj.rect[XHI];
	ymin = (Ctrl->C.active) ? GMT->current.proj.rect[YLO] - GMT->current.proj.origin[GMT_Y] : GMT->current.proj.rect[YLO];
	ymax = (Ctrl->C.active) ? GMT->current.proj.rect[YHI] - GMT->current.proj.origin[GMT_Y] : GMT->current.proj.rect[YHI];
	if (Ctrl->F.active) {	/* Convert to chosen units */
		strncpy (unit_name, scale_unit_name, GMT_GRID_UNIT_LEN80-1);
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
		char buf[GMT_LEN128] = {""}, gdal_ellipsoid_name[GMT_LEN16] = {""};

		/* if (GMT->common.R.oblique) gmt_M_double_swap (s, e); */  /* Got w/s/e/n, make into w/e/s/n */

		if ((Rect = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);
		}

		if ((Geo = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, Rect)) == NULL) Return (API->error);	/* Just to get a header we can change */
		HH = gmt_get_H_hidden (Geo->header);	/* Get the hidden info structure */

		if (gmt_M_is_azimuthal(GMT) && GMT->current.proj.polar) {	/* Watch out for polar cap grids */
			double px, py;
			/* Get projected pole point and determine if it is inside the grid */
			gmt_geo_to_xy (GMT, 0.0, GMT->current.proj.pole, &px, &py);	/* Projected coordinates of the relevant pole */
			if (!gmt_M_y_is_outside (GMT, py,  Rect->header->wesn[YLO], Rect->header->wesn[YHI]) &&
				!gmt_x_is_outside (GMT, &px,  Rect->header->wesn[XLO], Rect->header->wesn[XHI])) {	/* Not outside both projected x or y-range */
				wesn[XLO] = -180.0;	wesn[XHI] = +180.0;	/* Need a full 360 longitude range */
				if (GMT->current.proj.north_pole) {	/* And one of the latitude bounds must extend to the right pole */
					wesn[YLO] = MIN (wesn[YLO], wesn[YHI]);	wesn[YHI] = +90.0;
				}
				else {
					wesn[YHI] = MAX (wesn[YLO], wesn[YHI]);	wesn[YLO] = -90.0;
				}
			}
		}
		gmt_M_memcpy (Geo->header->wesn, wesn, 4, double);

		offset = Rect->header->registration;	/* Same as input */
		if (GMT->common.R.active[GSET]) offset = !offset;	/* Toggle */
		if (set_n) {
			use_nx = Rect->header->n_columns;
			use_ny = Rect->header->n_rows;
		}
		gmt_M_err_fail (GMT, gmt_project_init (GMT, Geo->header, GMT->common.R.inc, use_nx, use_ny, Ctrl->E.dpi, offset), Ctrl->G.file);
		gmt_set_grddim (GMT, Geo->header);
		if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Geo) == NULL) Return (API->error);
		gmt_grd_init (GMT, Geo->header, options, true);
		gmt_BC_init (GMT, Geo->header);

		if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
			GMT_Report (API, GMT_MSG_INFORMATION, "Transform ");
			GMT_Message (API, GMT_TIME_NONE, format, Geo->header->wesn[XLO], Geo->header->wesn[XHI], Geo->header->wesn[YLO], Geo->header->wesn[YHI]);
			GMT_Message (API, GMT_TIME_NONE, " <-- ");
			GMT_Message (API, GMT_TIME_NONE, format, xmin, xmax, ymin, ymax);
			GMT_Message (API, GMT_TIME_NONE, " [%s]\n", unit_name);
		}

		/* Modify input rect header if -F, -C, -M have been set */

		if (shift_xy) {
			Rect->header->wesn[XLO] -= Ctrl->C.easting;
			Rect->header->wesn[XHI] -= Ctrl->C.easting;
			Rect->header->wesn[YLO] -= Ctrl->C.northing;
			Rect->header->wesn[YHI] -= Ctrl->C.northing;

		}
		if (Ctrl->F.active) {	/* Convert from 1:1 scale */
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
		gmt_set_grdinc (GMT, Rect->header);	/* Update inc and r_inc given changes to wesn */

		sprintf (Geo->header->x_units, "longitude [degrees_east]");
		sprintf (Geo->header->y_units, "latitude [degrees_north]");

		/* Need to convert between GMT and GDAL ellipsoid names. But all are available in both sides. */
		k = GMT->current.setting.proj_ellipsoid;
		if (!strcmp (GMT->current.setting.ref_ellipsoid[k].name, "Sphere"))
			sprintf (buf, "+proj=longlat +a=%f +b%f +no_defs", GMT->current.setting.ref_ellipsoid[k].eq_radius,
			         GMT->current.setting.ref_ellipsoid[k].eq_radius);
		else {
			gmtlib_ellipsoid_name_convert (GMT->current.setting.ref_ellipsoid[k].name, gdal_ellipsoid_name);
			if (!strcmp (gdal_ellipsoid_name, "unknown"))
				sprintf (buf, "+proj=longlat +ellps=%s +no_defs", gdal_ellipsoid_name);
			else if (!strcmp (gdal_ellipsoid_name, "Web-Mercator"))
				sprintf (buf, "+proj=longlat +a=%f +b%f +no_defs", GMT->current.setting.ref_ellipsoid[k].eq_radius,
				         GMT->current.setting.ref_ellipsoid[k].eq_radius);
			else {
				sprintf (buf, "+proj=longlat +a=%f +b%f +no_defs", GMT->current.setting.ref_ellipsoid[k].eq_radius,
				         GMT->current.setting.ref_ellipsoid[k].eq_radius);
				GMT_Report (API, GMT_MSG_WARNING, "Unknown conversion between GMT and GDAL ellipsoid names. Using a generic spherical body.");
			}
		}

		gmt_grd_project (GMT, Rect, Geo, true);

		HH->grdtype = gmtlib_get_grdtype (GMT, GMT_OUT, Geo->header);	/* Determine grid type */

		gmt_set_pad (GMT, API->pad);	/* Reset to session default pad before output */
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Geo)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Geo) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	else {	/* Forward projection from geographical to rectangular grid */

		if ((Geo = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);
		}

		if ((Rect = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, Geo)) == NULL) Return (API->error);	/* Just to get a header we can change */
		HH = gmt_get_H_hidden (Rect->header);	/* Get the hidden info structure */

		gmt_M_memcpy (Rect->header->wesn, GMT->current.proj.rect, 4, double);
		if (Ctrl->F.active) {	/* Convert from 1:1 scale */
			if (unit) {	/* Undo the 1:1 unit used */
				GMT->common.R.inc[GMT_X] *= inv_scale;
				GMT->common.R.inc[GMT_Y] *= inv_scale;
			}
			GMT->common.R.inc[GMT_X] *= GMT->current.proj.scale[GMT_X];
			GMT->common.R.inc[GMT_Y] *= GMT->current.proj.scale[GMT_Y];
		}
		else if (GMT->current.setting.proj_length_unit != GMT_INCH) {	/* Convert from inch to whatever */
			GMT->common.R.inc[GMT_X] *= unit_to_inch;
			GMT->common.R.inc[GMT_Y] *= unit_to_inch;
		}
		if (set_n) {
			use_nx = Geo->header->n_columns;
			use_ny = Geo->header->n_rows;
		}

		if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
			GMT_Report (API, GMT_MSG_INFORMATION, "Transform ");
			GMT_Message (API, GMT_TIME_NONE, format, Geo->header->wesn[XLO], Geo->header->wesn[XHI], Geo->header->wesn[YLO], Geo->header->wesn[YHI]);
			GMT_Message (API, GMT_TIME_NONE, " --> ");
			GMT_Message (API, GMT_TIME_NONE, format, xmin, xmax, ymin, ymax);
			GMT_Message (API, GMT_TIME_NONE, " [%s]\n", unit_name);
		}

		offset = Geo->header->registration;	/* Same as input */
		if (GMT->common.R.active[GSET]) offset = !offset;	/* Toggle */

		gmt_M_err_fail (GMT, gmt_project_init (GMT, Rect->header, GMT->common.R.inc, use_nx, use_ny, Ctrl->E.dpi, offset), Ctrl->G.file);
		gmt_set_grddim (GMT, Rect->header);
		if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Rect) == NULL) Return (API->error);
		gmt_BC_init (GMT, Rect->header);
		gmt_grd_project (GMT, Geo, Rect, false);
		gmt_grd_init (GMT, Rect->header, options, true);

		/* Modify output rect header if -F, -C, -M have been set */

		if (Ctrl->C.active) {	/* Change origin from lower left to projection center */
			Rect->header->wesn[XLO] -= GMT->current.proj.origin[GMT_X];
			Rect->header->wesn[XHI] -= GMT->current.proj.origin[GMT_X];
			Rect->header->wesn[YLO] -= GMT->current.proj.origin[GMT_Y];
			Rect->header->wesn[YHI] -= GMT->current.proj.origin[GMT_Y];
		}
		if (Ctrl->F.active) {	/* Convert to 1:1 scale */
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
		gmt_set_grdinc (GMT, Rect->header);	/* Update inc and r_inc given changes to wesn */
		strncpy (Rect->header->x_units, unit_name, GMT_GRID_UNIT_LEN80-1);
		strncpy (Rect->header->y_units, unit_name, GMT_GRID_UNIT_LEN80-1);

		if (GMT->common.J.proj4string[0])
			Rect->header->ProjRefPROJ4 = strdup (GMT->common.J.proj4string);
		else if (GMT->common.J.WKTstring[0])
			Rect->header->ProjRefWKT = strdup (GMT->common.J.WKTstring);
		else
			Rect->header->ProjRefPROJ4 = gmt_export2proj4 (GMT);	/* Convert the GMT -J<...> into a proj4 string and save it in the header */

		/* rect xy values are here in GMT projected units chosen by user */

		HH->grdtype = gmtlib_get_grdtype (GMT, GMT_OUT, Rect->header);	/* Determine grid type */
		gmt_set_pad (GMT, API->pad);	/* Reset to session default pad before output */
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Rect)) Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Rect) != GMT_NOERROR) {
			Return (API->error);
		}
		if (Rect->header->ProjRefPROJ4) gmt_M_str_free (Rect->header->ProjRefPROJ4);
		if (Rect->header->ProjRefWKT)   gmt_M_str_free (Rect->header->ProjRefWKT);
	}

	Return (GMT_NOERROR);
}
