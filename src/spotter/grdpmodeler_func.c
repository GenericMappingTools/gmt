/*--------------------------------------------------------------------
 *	$Id: grdpmodeler_func.c,v 1.10 2011-05-14 00:04:07 guru Exp $
 *
 *   Copyright (c) 1999-2011 by P. Wessel
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 or any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   Contact info: www.soest.hawaii.edu/pwessel
 *--------------------------------------------------------------------*/
/*
 * grdpmodeler will read an age grid file and a plate motion model and
 * evaluates chosen attributes such as speed, direction, lon/lat origin
 * of the grid location.
 *
 * Author:	Paul Wessel
 * Date:	1-NOV-2010
 * Ver:		5
 */

#include "spotter.h"
#include "gmt_proj.h"

#define PM_RATE		0
#define PM_AZIM		1
#define PM_OMEGA	2
#define PM_DLON		3
#define PM_LON		4
#define PM_DLAT		5
#define PM_LAT		6
#define PM_DIST		7

struct GRDROTATER_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct E {	/* -Erotfile */
		GMT_LONG active;
		char *file;
	} E;
	struct F {	/* -Fpolfile */
		GMT_LONG active;
		char *file;
	} F;
	struct G {	/* -Goutfile */
		GMT_LONG active;
		char *file;
	} G;
	struct S {	/* -Sa|d|r|w|x|y|X|Y */
		GMT_LONG active;
		GMT_LONG mode;
	} S;
	struct T {	/* -T<fixtime> */
		GMT_LONG active;
		double value;
	} T;
};

void *New_grdpmodeler_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDROTATER_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDROTATER_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	return ((void *)C);
}

void Free_grdpmodeler_Ctrl (struct GMT_CTRL *GMT, struct GRDROTATER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->E.file) free ((void *)C->E.file);	
	if (C->F.file) free ((void *)C->F.file);	
	if (C->G.file) free ((void *)C->G.file);	
	GMT_free (GMT, C);	
}


GMT_LONG GMT_grdpmodeler_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT,"grdpmodeler %s - Plate model evaluation on a geographic grid\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdpmodeler <agegrdfile> -E<euler.d> -G<results.nc> [-F<polygonfile>]\n");
	GMT_message (GMT, "\t[%s] [-Sa|d|r|w|x|y|X|Y] [-T<time>] [%s] [%s]\n\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_b_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<grdfile> is a gridded data file in geographic coordinates to be rotated\n");
	GMT_message (GMT, "\t-G is the output filename of the new, rotated grid.  The boundary of the\n");
	GMT_message (GMT, "\t   original grid (or a subset; see -F) after rotation is written to stdout\n");
	GMT_message (GMT, "\t-E specifies the rotations to be used (see man page for format)\n\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-D Destination file for rotated polygon or grid outline [stdout]\n");
	GMT_message (GMT, "\t-F specifies a multi-segment closed polygon file that describes the area of the grid\n");
	GMT_message (GMT, "\t   that should be projected [Default projects entire grid]\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-S Select a model prediction as a function of crustal age.  Choose among:\n");
	GMT_message (GMT, "\t   a : Plate spreading azimuth.\n");
	GMT_message (GMT, "\t   d : Distance to origin of crust in km.\n");
	GMT_message (GMT, "\t   r : Plate motion rate in mm/yr or km/Myr.\n");
	GMT_message (GMT, "\t   w : Rotation rate in degrees/Myr.\n");
	GMT_message (GMT, "\t   x : Change in longitude since formation.\n");
	GMT_message (GMT, "\t   y : Change in latitude since formation.\n");
	GMT_message (GMT, "\t   X : Congitude at origin of crust.\n");
	GMT_message (GMT, "\t   Y : Latitude at origin of crust.\n");
	GMT_message (GMT, "\t-T sets fixed time of reconstruction to override age grid.\n");
	GMT_explain_options (GMT, "V.");
	
	return (EXIT_FAILURE);

}

GMT_LONG GMT_grdpmodeler_parse (struct GMTAPI_CTRL *C, struct GRDROTATER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdpmodeler and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Supplemental parameters */
			
			case 'E':	/* File with stage poles */
				Ctrl->E.active = TRUE;	n = 0;
				Ctrl->E.file  = strdup (&opt->arg[n]);
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				Ctrl->F.file = strdup (opt->arg);
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				switch (opt->arg[0]) {
					case 'a':	/* Plate spreading azimuth */
						Ctrl->S.mode = PM_AZIM;
						break;
					case 'd':	/* Distance from point to origin at ridge */
						Ctrl->S.mode = PM_DIST;
						break;
					case 's':	/* Plate spreading rate */
						Ctrl->S.mode = PM_RATE;
						break;
					case 'w':	/* Plate rotation omega */
						Ctrl->S.mode = PM_OMEGA;
						break;
					case 'x':	/* Change in longitude since origin */
						Ctrl->S.mode = PM_DLON;
						break;
					case 'y':	/* Change in latitude since origin */
						Ctrl->S.mode = PM_DLAT;
						break;
					case 'X':	/* Plate longitude at crust origin */
						Ctrl->S.mode = PM_LON;
						break;
					case 'Y':	/* Plate latitude at crust origin */
						Ctrl->S.mode = PM_LAT;
						break;
					default:
						n_errors++;
						break;
				}
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				Ctrl->T.value = atof (opt->arg);
				break;
				
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->E.active, "Syntax error: Must specify -E\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->S.active, "Syntax error: Must specify -S\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.value < 0.0, "Syntax error -T: Must specify positive age.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdpmodeler_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdpmodeler (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG col, row, node, inside, seg, k, n_stages, error = FALSE;
	
	double lon, lat, d, value = 0.0, t_max, age, *grd_x = NULL, *grd_y = NULL, *grd_yc = NULL;

	struct GMT_DATASET *D = NULL;
	struct GMT_TABLE *pol = NULL;
	struct EULER *p = NULL;			/* Pointer to array of stage poles */
	struct GMT_OPTION *ptr = NULL;
	struct GMT_GRID *G_age = NULL, *G_mod = NULL;
	struct GRDROTATER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdpmodeler_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdpmodeler_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdpmodeler", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRf:", "", options))) Return (error);
	if (GMT_Find_Option (API, 'f', options, &ptr)) GMT_parse_common_options (GMT, "f", 'f', "g"); /* Did not set -f, implicitly set -fg */
	Ctrl = (struct GRDROTATER_CTRL *) New_grdpmodeler_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdpmodeler_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the grdpmodeler main code ----------------------------*/

	GMT_lat_swap_init (GMT);	/* Initialize auxiliary latitude machinery */

	/* Check limits and get data file */

	if ((error = GMT_Begin_IO (API, 0, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&(Ctrl->In.file), (void **)&G_age)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);			/* Disables further data input */

	if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, G_age->header->wesn, 4, double);	/* -R was not set so we use the grid domain */

	/* Determine the wesn to be used to read the Ctrl->In.file; or exit if file is outside -R */

	if (!GMT_grd_setregion (GMT, G_age->header, GMT->common.R.wesn, BCR_BILINEAR)) {
		GMT_report (GMT, GMT_MSG_FATAL, "No grid values inside selected region - aborting\n");
		Return (EXIT_FAILURE);
	}
		
	if (Ctrl->F.active) {	/* Read the user's clip polygon file */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, NULL, 0, (void **)&Ctrl->F.file, (void **)&D)) Return ((error = GMT_DATA_READ_ERROR));
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
		pol = D->table[0];	/* Since it is a single file */
		GMT_report (GMT, GMT_MSG_NORMAL, "Restrict evalution to within polygons in file %s\n", Ctrl->F.file);
	}

	n_stages = spotter_init (GMT, Ctrl->E.file, &p, FALSE, FALSE, 0, &t_max);
	for (k = 0; k < n_stages; k++) {
		if (p[k].omega < 0.0) {	/* Ensure all stages have positive rotation angles */
			p[k].omega = -p[k].omega;
			p[k].lat = -p[k].lat;
			p[k].lon += 180.0;
			if (p[k].lon > 360.0) p[k].lon -= 360.0;
		}
	}
	if (Ctrl->T.active && Ctrl->T.value > t_max) {
		GMT_report (GMT, GMT_MSG_FATAL, "Requested a fixed reconstruction time outside range of rotation table\n");
		GMT_free (GMT, p);
		Return (EXIT_FAILURE);
	}
	
	G_mod = GMT_create_grid (GMT);
	GMT_grd_init (GMT, G_mod->header, options, FALSE);
	
	/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, G_mod, GMT->common.R.wesn, G_age->header->inc, G_age->header->registration), Ctrl->G.file);
	
	G_mod->data = GMT_memory (GMT, NULL, G_mod->header->size, float);
	grd_x = GMT_memory (GMT, NULL, G_mod->header->nx, double);
	grd_y = GMT_memory (GMT, NULL, G_mod->header->ny, double);
	grd_yc = GMT_memory (GMT, NULL, G_mod->header->ny, double);
	/* Precalculate node coordinates in both degrees and radians */
	for (row = 0; row < G_mod->header->ny; row++) grd_y[row] = GMT_grd_row_to_y (row, G_mod->header);
	for (row = 0; row < G_mod->header->ny; row++) grd_yc[row] = GMT_lat_swap (GMT, grd_y[row], GMT_LATSWAP_G2O);
	for (col = 0; col < G_mod->header->nx; col++) grd_x[col] = GMT_grd_col_to_x (col, G_mod->header);

	/* Loop over all nodes in the new rotated grid and find those inside the reconstructed polygon */
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Evalute model prediction grid\n");

	GMT_init_distaz (GMT, 'd', GMT_GREATCIRCLE, GMT_MAP_DIST);	/* Great circle distances in degrees */
	if (Ctrl->S.mode == PM_DLON) GMT->current.io.geo.range = 2;	/* Need +- around 0 here */

	GMT_grd_loop (G_mod, row, col, node) {
		G_mod->data[node] = GMT->session.f_NaN;
		if (Ctrl->F.active) {
			for (seg = inside = 0; seg < pol->n_segments && !inside; seg++) {	/* Use degrees since function expects it */
				if (GMT_polygon_is_hole (pol->segment[seg])) continue;	/* Holes are handled within GMT_inonout */
				inside = (GMT_inonout (GMT, grd_x[col], grd_y[row], pol->segment[seg]) > 0);
			}
			if (!inside) continue;	/* Outside the polygon(s) */
		}
		/* Here we are inside; get the coordinates and rotate back to original grid coordinates */
		age = (Ctrl->T.active) ? Ctrl->T.value : G_age->data[node];
		if (GMT_is_dnan (age)) continue;	/* No crustal age */
		if ((k = spotter_stage (GMT, age, p, n_stages)) < 0) continue;	/* Outside valid stage rotation range */
		switch (Ctrl->S.mode) {
			case PM_RATE:	/* Compute plate motion speed at this point in time/space */
				d = GMT_distance (GMT, grd_x[col], grd_yc[row], p[k].lon, p[k].lat);
				value = sind (d) * p[k].omega * GMT->current.proj.DIST_KM_PR_DEG;	/* km/Myr or mm/yr */
				break;
			case PM_AZIM:	/* Compute plate motion direction at this point in time/space */
				value = GMT_az_backaz (GMT, grd_x[col], grd_yc[row], p[k].lon, p[k].lat, FALSE) - 90.0;
				GMT_lon_range_adjust (GMT, GMT->current.io.geo.range, &value);
				break;
			case PM_OMEGA:	/* Compute plate rotation rate omega */
				value = p[k].omega;	/* degree/Myr  */
				break;
			case PM_DLAT:	/* Compute latitude where this point was formed in the model */
				lon = grd_x[col] * D2R;	lat = grd_yc[row] * D2R;
				(void)spotter_backtrack (GMT, &lon, &lat, &age, 1, p, n_stages, 0.0, 0.0, FALSE, NULL, NULL);
				value = grd_y[row] - GMT_lat_swap (GMT, lat * R2D, GMT_LATSWAP_O2G);	/* Convert back to geodetic */
				break;
			case PM_LAT:	/* Compute latitude where this point was formed in the model */
				lon = grd_x[col] * D2R;	lat = grd_yc[row] * D2R;
				(void)spotter_backtrack (GMT, &lon, &lat, &age, 1, p, n_stages, 0.0, 0.0, FALSE, NULL, NULL);
				value = GMT_lat_swap (GMT, lat * R2D, GMT_LATSWAP_O2G);			/* Convert back to geodetic */
				break;
			case PM_DLON:	/* Compute latitude where this point was formed in the model */
				lon = grd_x[col] * D2R;	lat = grd_yc[row] * D2R;
				(void)spotter_backtrack (GMT, &lon, &lat, &age, 1, p, n_stages, 0.0, 0.0, FALSE, NULL, NULL);
				value = grd_x[col] - lon * R2D;
				if (fabs (value) > 180.0) value = copysign (360.0 - fabs (value), -value);
				break;
			case PM_LON:	/* Compute latitude where this point was formed in the model */
				lon = grd_x[col] * D2R;	lat = grd_yc[row] * D2R;
				(void)spotter_backtrack (GMT, &lon, &lat, &age, 1, p, n_stages, 0.0, 0.0, FALSE, NULL, NULL);
				value = lon * R2D;
				break;
			case PM_DIST:	/* Compute distance between node and point of origin at ridge */
				lon = grd_x[col] * D2R;	lat = grd_yc[row] * D2R;
				(void)spotter_backtrack (GMT, &lon, &lat, &age, 1, p, n_stages, 0.0, 0.0, FALSE, NULL, NULL);
				value = GMT_distance (GMT, grd_x[col], grd_yc[row], lon * R2D, lat * R2D) * GMT->current.proj.DIST_KM_PR_DEG;
				break;
		}
		G_mod->data[node] = (float)value;
	}	
	
	/* Now write model prediction grid */
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Write predicted grid\n");

	strcpy (G_mod->header->x_units, "degrees_east");
	strcpy (G_mod->header->y_units, "degrees_north");
	switch (Ctrl->S.mode) {
		case PM_RATE:	/* Compute plate motion speed at this point in time/space */
			strcpy (G_mod->header->z_units, "mm/yr");
			break;
		case PM_AZIM:	/* Compute plate motion direction at this point in time/space */
			strcpy (G_mod->header->z_units, "degree");
			break;
		case PM_OMEGA:	/* Compute plate rotation rate omega */
			strcpy (G_mod->header->z_units, "degree/Myr");
			break;
		case PM_DLAT:	/* Difference in latitude relative to where this point was formed in the model */
		case PM_LAT:	/* Latitude where this point was formed in the model */
			strcpy (G_mod->header->z_units, "degrees_north");
			break;
		case PM_DLON:	/* Difference in longitude relative to where this point was formed in the model */
		case PM_LON:	/* Longitude where this point was formed in the model */
			strcpy (G_mod->header->z_units, "degrees_east");
			break;
		case PM_DIST:	/* Distance to origin in km */
			strcpy (G_mod->header->z_units, "km");
			break;
	}
	sprintf (G_mod->header->remark, "Plate Model predictions for model %s", Ctrl->E.file);
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&Ctrl->G.file, (void *)G_mod)) Return (GMT_DATA_WRITE_ERROR);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */

	GMT_free (GMT, grd_x);
	GMT_free (GMT, grd_y);
	GMT_free (GMT, grd_yc);
	GMT_free (GMT, p);
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Done!\n");

	Return (GMT_OK);
}
