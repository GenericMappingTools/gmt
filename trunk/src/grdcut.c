/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * API functions to support the grdcut application.
 *
 * Author:	Walter Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: Reads a grid file and writes a portion within it
 * to a new file.
 */
 
#define THIS_MODULE k_mod_grdcut /* I am grdcut */

#include "gmt.h"

/* Control structure for grdcontour */

struct GRDCUT_CTRL {
	struct In {
		bool active;
		char *file;
	} In;
	struct G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct S {	/* -S[n]<lon>/<lat>/[-|=|+]<radius>[d|e|f|k|m|M|n] */
		bool active;
		bool set_nan;
		int mode;	/* Could be negative */
		char unit;
		double lon, lat, radius;
	} S;
	struct Z {	/* -Z[min/max] */
		bool active;
		unsigned int mode;	/* 1 means NaN */
		double min, max;
	} Z;
};

#define NAN_IS_INSIDE	0
#define NAN_IS_OUTSIDE	1

void *New_grdcut_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDCUT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GRDCUT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->Z.min = -DBL_MAX;	C->Z.max = DBL_MAX;			/* No limits on z-range */
	return (C);
}

void Free_grdcut_Ctrl (struct GMT_CTRL *GMT, struct GRDCUT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);	
	if (C->G.file) free (C->G.file);	
	GMT_free (GMT, C);	
}

int GMT_grdcut_usage (struct GMTAPI_CTRL *C, int level)
{
	struct GMT_CTRL *GMT = C->GMT;

	gmt_module_show_name_and_purpose (THIS_MODULE);
	GMT_message (GMT, "usage: grdcut <ingrid> -G<outgrid> %s [%s]\n\t[-S[n]<lon>/<lat>/<radius>] [-Z[n][<min>/<max>]] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<ingrid> is file to extract a subset from.\n");
	GMT_message (GMT, "\t-G Specify output grid file.\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t   The WESN you specify must be within the WESN of the input grid.\n");
	GMT_message (GMT, "\t   If in doubt, run grdinfo first and check range of old file.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "V");
	GMT_dist_syntax (GMT, 'S', "Specify an origin and radius to find the corresponding rectangular area.");
	GMT_message (GMT, "\t   All nodes on or inside the radius are contained in the subset grid.\n");
	GMT_message (GMT, "\t   Use -Sn to set all nodes in the subset outside the circle to NaN.\n");
	GMT_message (GMT, "\t-Z Specify a range and determine the corresponding rectangular region so that\n");
	GMT_message (GMT, "\t   all values outside this region are outside the range [-inf/+inf].\n");
	GMT_message (GMT, "\t   Use -Zn to consider NaNs outside as well [Default just ignores NaNs].\n");
	GMT_explain_options (GMT, "f.");
	
	return (EXIT_FAILURE);
}

int GMT_grdcut_parse (struct GMTAPI_CTRL *C, struct GRDCUT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, n_files = 0;
	char za[GMT_TEXT_LEN64], zb[GMT_TEXT_LEN64], zc[GMT_TEXT_LEN64];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				Ctrl->In.active = true;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */
			
 			case 'G':	/* Output file */
				Ctrl->G.active = true;
				Ctrl->G.file = strdup (opt->arg);
				break;
 			case 'S':	/* Origin and radius */
				Ctrl->S.active = true;
				k = 0;
				if (opt->arg[k] == 'n') {
					Ctrl->S.set_nan = true;
					k = 1;
				}
				if (sscanf (&opt->arg[k], "%[^/]/%[^/]/%s", za, zb, zc) == 3) {
					n_errors += GMT_verify_expectations (GMT, GMT_IS_LON, GMT_scanf_arg (GMT, za, GMT_IS_LON, &Ctrl->S.lon), za);
					n_errors += GMT_verify_expectations (GMT, GMT_IS_LAT, GMT_scanf_arg (GMT, zb, GMT_IS_LAT, &Ctrl->S.lat), zb);
					Ctrl->S.mode = GMT_get_distance (GMT, zc, &(Ctrl->S.radius), &(Ctrl->S.unit));
				}
				break;
			case 'Z':	/* Detect region via z-range */
				Ctrl->Z.active = true;
				k = 0;
				if (opt->arg[k] == 'n') {
					Ctrl->Z.mode = NAN_IS_OUTSIDE;
					k = 1;
				}
				if (sscanf (&opt->arg[k], "%[^/]/%s", za, zb) == 2) {
					if (!(za[0] == '-' && za[1] == '\0')) n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Z], GMT_scanf_arg (GMT, za, GMT->current.io.col_type[GMT_IN][GMT_Z], &Ctrl->Z.min), za);
					if (!(zb[0] == '-' && zb[1] == '\0')) n_errors += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Z], GMT_scanf_arg (GMT, zb, GMT->current.io.col_type[GMT_IN][GMT_Z], &Ctrl->Z.max), zb);
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	n_errors += GMT_check_condition (GMT, (GMT->common.R.active + Ctrl->S.active + Ctrl->Z.active) != 1, "Syntax error: Must specify only one of the -R, -S or the -Z options\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error -G option: Must specify output grid file\n");
	n_errors += GMT_check_condition (GMT, n_files != 1, "Syntax error: Must specify one input grid file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_grdcut_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdcut (struct GMTAPI_CTRL *API, int mode, void *args)
{
	int error = 0;
	unsigned int nx_old, ny_old;
	uint64_t node;

	double wesn_new[4], wesn_old[4];
	double lon, lat, distance, radius;

	struct GRD_HEADER test_header;
	struct GRDCUT_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_grdcut_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_grdcut_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_gmt_module (API, THIS_MODULE, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, "-VfR:", "", options)) Return (API->error);
	Ctrl = New_grdcut_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdcut_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdcut main code ----------------------------*/

	if (Ctrl->Z.active) {	/* Must determine new region via -Z, so get entire grid first */
		unsigned int row0 = 0, row1 = 0, col0 = 0, col1 = 0, row, col;
		bool go;
		
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);	/* Get entire grid */
		}
		
		for (row = 0, go = true; go && row < G->header->nx; row++) {	/* Scan from xmin towards xmax */
			for (col = 0, node = GMT_IJP (G->header, 0, row); go && col < G->header->ny; col++, node += G->header->mx) {
				if (GMT_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_OUTSIDE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) row0 = row;	/* Found starting column */
			}
		}
		if (go) {
			GMT_report (GMT, GMT_MSG_NORMAL, "The sub-region implied by -Z is empty!\n");
			Return (EXIT_FAILURE);
		}
		for (row = G->header->nx-1, go = true; go && row > row0; row--) {	/* Scan from xmax towards xmin */
			for (col = 0, node = GMT_IJP (G->header, 0, row); go && col < G->header->ny; col++, node += G->header->mx) {
				if (GMT_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INSIDE) go = false;	/* Must stop since this value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) row1 = row;	/* Found stopping column */
			}
		}
		for (col = 0, go = true; go && col < G->header->ny; col++) {	/* Scan from ymin towards ymax */
			for (row = row0, node = GMT_IJP (G->header, col, row0); go && row < row1; row++, node++) {
				if (GMT_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INSIDE) go = false;	/* Must stop since this value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) col0 = col;	/* Found starting row */
			}
		}
		for (col = G->header->ny-1, go = true; go && col >= col0; col--) {	/* Scan from ymax towards ymin */
			for (row = row0, node = GMT_IJP (G->header, col, row0); go && row < row1; row++, node++) {
				if (GMT_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INSIDE) go = false;	/* Must stop since this value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) col1 = col;	/* Found starting row */
			}
		}
		if (row0 == 0 && col0 == 0 && row1 == (G->header->nx-1) && col1 == (G->header->ny-1)) {
			GMT_report (GMT, GMT_MSG_VERBOSE, "Your -Z limits produced no subset - output grid is identical to input grid\n");
			GMT_memcpy (wesn_new, G->header->wesn, 4, double);
		}
		else {	/* Adjust boundaries inwards */
			wesn_new[XLO] = G->header->wesn[XLO] + row0 * G->header->inc[GMT_X];
			wesn_new[XHI] = G->header->wesn[XHI] - (G->header->nx - 1 - row1) * G->header->inc[GMT_X];
			wesn_new[YLO] = G->header->wesn[YLO] + (G->header->ny - 1 - col1) * G->header->inc[GMT_Y];
			wesn_new[YHI] = G->header->wesn[YHI] - col0 * G->header->inc[GMT_Y];
		}
		GMT_free_aligned (GMT, G->data);	/* Free the grid array only as we need the header below */
	}
	else if (Ctrl->S.active) {	/* Must determine new region via -S, so only need header */
		int row, col;
		bool wrap;
		
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);	/* Get header only */
		}
		if (!GMT_is_geographic (GMT, GMT_IN)) {
			GMT_report (GMT, GMT_MSG_NORMAL, "The -S option requires a geographic grid\n");
			Return (EXIT_FAILURE);
		}
		GMT_init_distaz (GMT, Ctrl->S.unit, Ctrl->S.mode, GMT_MAP_DIST);
		wesn_new[XLO] = wesn_new[XHI] = Ctrl->S.lon;
		wesn_new[YLO] = wesn_new[YHI] = Ctrl->S.lat;
		/* First adjust the S and N boundaries */
		radius = R2D * (Ctrl->S.radius / GMT->current.map.dist[GMT_MAP_DIST].scale) / GMT->current.proj.mean_radius;	/* Approximate radius in degrees */
		wesn_new[YLO] -= radius;	/* Approximate south limit in degrees */
		if (wesn_new[YLO] <= -90.0) {	/* Way south, reset to S pole */
			wesn_new[YLO] = -90.0;
		}
		else {	/* Possibly adjust south a bit using chosen distance calculation */
			row = GMT_grd_y_to_row (GMT, wesn_new[YLO], G->header);		/* Nearest row with this latitude */
			lat = GMT_grd_row_to_y (GMT, row, G->header);			/* Latitude of that row */
			distance = GMT_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			while (distance <= Ctrl->S.radius) {	/* Extend region by one more row until we are outside */
				lat -= G->header->inc[GMT_Y];
				distance = GMT_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			}
			wesn_new[YLO] = lat + G->header->inc[GMT_Y];	/* Go one back since last row was outside */
		}
		wesn_new[YHI] += radius;	/* Approximate north limit in degrees */
		if (wesn_new[YHI] >= 90.0) {	/* Way north, reset to N pole */
			wesn_new[YHI] = 90.0;
		}
		else {	/* Possibly adjust north a bit using chosen distance calculation */
			row = GMT_grd_y_to_row (GMT, wesn_new[YHI], G->header);		/* Nearest row with this latitude */
			lat = GMT_grd_row_to_y (GMT, row, G->header);			/* Latitude of that row */
			distance = GMT_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			while (distance <= Ctrl->S.radius) {	/* Extend region by one more row until we are outside */
				lat += G->header->inc[GMT_Y];
				distance = GMT_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			}
			wesn_new[YHI] = lat - G->header->inc[GMT_Y];	/* Go one back since last row was outside */
		}
		if (doubleAlmostEqual (wesn_new[YLO], -90.0) || doubleAlmostEqual (wesn_new[YHI], 90.0)) {	/* Need all longitudes when a pole is included */
			wesn_new[XLO] = G->header->wesn[XLO];
			wesn_new[XHI] = G->header->wesn[XHI];
		}
		else {	/* Determine longitude limits */
			wrap = GMT_360_RANGE (G->header->wesn[XLO], G->header->wesn[XHI]);	/* true if grid is 360 global */
			radius /= cosd (Ctrl->S.lat);					/* Approximate e-w width in degrees longitude */
			wesn_new[XLO] -= radius;					/* Approximate west limit in degrees */
			if (!wrap && wesn_new[XLO] < G->header->wesn[XLO]) {		/* Outside non-periodic grid range */
				wesn_new[XLO] = G->header->wesn[XLO];
			}
			else {
				col = GMT_grd_x_to_col (GMT, wesn_new[XLO], G->header);		/* Nearest col with this longitude */
				lon = GMT_grd_col_to_x (GMT, col, G->header);			/* Longitude of that col */
				distance = GMT_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				while (distance <= Ctrl->S.radius) {	/* Extend region by one more col until we are outside */
					lon -= G->header->inc[GMT_X];
					distance = GMT_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				}
				wesn_new[XLO] = lon + G->header->inc[GMT_X];	/* Go one back since last col was outside */
			}
			wesn_new[XHI] += radius;					/* Approximate east limit in degrees */
			if (!wrap && wesn_new[XHI] > G->header->wesn[XHI]) {		/* Outside grid range */
				wesn_new[XHI] = G->header->wesn[XHI];
			}
			else {
				col = GMT_grd_x_to_col (GMT, wesn_new[XHI], G->header);		/* Nearest col with this longitude */
				lon = GMT_grd_col_to_x (GMT, col, G->header);			/* Longitude of that col */
				distance = GMT_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				while (distance <= Ctrl->S.radius) {	/* Extend region by one more col until we are outside */
					lon += G->header->inc[GMT_X];
					distance = GMT_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				}
				wesn_new[XHI] = lon - G->header->inc[GMT_X];	/* Go one back since last col was outside */
			}
			if (wesn_new[XHI] > 360.0) wesn_new[XLO] -= 360.0, wesn_new[XHI] -= 360.0;
			if (wesn_new[XHI] < 0.0)   wesn_new[XLO] += 360.0, wesn_new[XHI] += 360.0;
		}
	}
	else {	/* Just the usual subset selection via -R */
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);	/* Get header only */
		}
		GMT_memcpy (wesn_new, GMT->common.R.wesn, 4, double);
	}
	
	if (wesn_new[YLO] < G->header->wesn[YLO] || wesn_new[YLO] > G->header->wesn[YHI]) error++;
	if (wesn_new[YHI] < G->header->wesn[YLO] || wesn_new[YHI] > G->header->wesn[YHI]) error++;

	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Geographic data */
		if (wesn_new[XLO] < G->header->wesn[XLO] && wesn_new[XHI] < G->header->wesn[XLO]) {
			G->header->wesn[XLO] -= 360.0;
			G->header->wesn[XHI] -= 360.0;
		}
		if (wesn_new[XLO] > G->header->wesn[XHI] && wesn_new[XHI] > G->header->wesn[XHI]) {
			G->header->wesn[XLO] += 360.0;
			G->header->wesn[XHI] += 360.0;
		}
		if (!GMT_grd_is_global (GMT, G->header) && (wesn_new[XLO] < G->header->wesn[XLO] || wesn_new[XHI] > G->header->wesn[XHI])) error++;
	}
	else if (wesn_new[XLO] < G->header->wesn[XLO] || wesn_new[XHI] > G->header->wesn[XHI])
		error++;

	if (error) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Subset exceeds data domain!\n");
		Return (GMT_RUNTIME_ERROR);
	}

	/* Make sure output grid is kosher */

	GMT_adjust_loose_wesn (GMT, wesn_new, G->header);

	GMT_memcpy (test_header.wesn, wesn_new, 4, double);
	GMT_memcpy (test_header.inc, G->header->inc, 2, double);
	GMT_err_fail (GMT, GMT_grd_RI_verify (GMT, &test_header, 1), Ctrl->G.file);

	/* OK, so far so good. Check if new wesn differs from old wesn by integer dx/dy */

	if (GMT_minmaxinc_verify (GMT, G->header->wesn[XLO], wesn_new[XLO], G->header->inc[GMT_X], GMT_SMALL) == 1) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Old and new x_min do not differ by N * dx\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (GMT_minmaxinc_verify (GMT, wesn_new[XHI], G->header->wesn[XHI], G->header->inc[GMT_X], GMT_SMALL) == 1) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Old and new x_max do not differ by N * dx\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (GMT_minmaxinc_verify (GMT, G->header->wesn[YLO], wesn_new[YLO], G->header->inc[GMT_Y], GMT_SMALL) == 1) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Old and new y_min do not differ by N * dy\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (GMT_minmaxinc_verify (GMT, wesn_new[YHI], G->header->wesn[YHI], G->header->inc[GMT_Y], GMT_SMALL) == 1) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Old and new y_max do not differ by N * dy\n");
		Return (GMT_RUNTIME_ERROR);
	}

	GMT_grd_init (GMT, G->header, options, true);

	GMT_memcpy (wesn_old, G->header->wesn, 4, double);
	nx_old = G->header->nx;		ny_old = G->header->ny;
	
	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA, wesn_new, Ctrl->In.file, G) == NULL) {	/* Get subset */
		Return (API->error);
	}

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		char format[GMT_BUFSIZ];
		sprintf (format, "\t%s\t%s\t%s\t%s\t%s\t%s\t%%d\t%%d\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
			GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_VERBOSE, "File spec:\tW E S N dx dy nx ny:\n");
		GMT_report (GMT, GMT_MSG_VERBOSE, "Old:");
		GMT_report (GMT, GMT_MSG_VERBOSE, format, wesn_old[XLO], wesn_old[XHI], wesn_old[YLO], wesn_old[YHI], G->header->inc[GMT_X], G->header->inc[GMT_Y], nx_old, ny_old);
		GMT_report (GMT, GMT_MSG_VERBOSE, "New:");
		GMT_report (GMT, GMT_MSG_VERBOSE, format, wesn_new[XLO], wesn_new[XHI], wesn_new[YLO], wesn_new[YHI], G->header->inc[GMT_X], G->header->inc[GMT_Y], G->header->nx, G->header->ny);
	}

	if (Ctrl->S.set_nan) {	/* Set all nodes outside the circle to NaN */
		unsigned int row, col;
		uint64_t n_nodes = 0;
		double *grd_lon = GMT_grd_coord (GMT, G->header, GMT_X);
		
		for (row = 0; row < G->header->ny; row++) {
			lat = GMT_grd_row_to_y (GMT, row, G->header);
			for (col = 0; col < G->header->nx; col++) {
				distance = GMT_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, grd_lon[col], lat);
				if (distance > Ctrl->S.radius) {	/* Outside circle */
					node = GMT_IJP (G->header, row, col);
					G->data[node] = GMT->session.f_NaN;
					n_nodes++;
				}
			}
		}
		GMT_free (GMT, grd_lon);	
		GMT_report (GMT, GMT_MSG_VERBOSE, "Set %" PRIu64 " nodes outside circle to NaN\n", n_nodes);
	}
	
	/* Send the subset of the grid to the destination. */
	
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Ctrl->G.file, G) != GMT_OK) {
		Return (API->error);
	}

	Return (GMT_OK);
}
