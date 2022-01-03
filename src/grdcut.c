/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2022 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * API functions to support the grdcut application.
 *
 * Author:	Walter Smith, Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: Reads a grid file and writes a portion within it
 * to a new file.
 *
 * Note on KEYS: FD(= means -F takes an optional input Dataset as argument which may be followed by optional modifiers.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdcut"
#define THIS_MODULE_MODERN_NAME	"grdcut"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Extract subregion from a grid or image"
#define THIS_MODULE_KEYS	"<G{,FD(=,>DD,G?}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-JRVf"

/* Control structure for grdcut */

struct GRDCUT_CTRL {
	struct GRDCUT_In {
		bool active;
		unsigned int type;
		char *file;
	} In;
	struct GRDCUT_D {	/* -D[+t] */
		bool active;
		bool text;
		bool quit;
	} D;
	struct GRDCUT_F {	/* -Fpolfile[+c][+i] */
		bool active;
		bool crop;
		unsigned int invert;
		char *file;
	} F;
	struct GRDCUT_G {	/* -G<output_grdfile> */
		bool active;
		char *file;
	} G;
	struct GRDCUT_N {	/* -N<nodata> */
		bool active;
		gmt_grdfloat value;
	} N;
	struct GRDCUT_S {	/* -S<lon>/<lat>/[-|=|+]<radius>[d|e|f|k|m|M|n][+n] */
		bool active;
		bool set_nan;
		int mode;	/* Could be negative */
		char unit;
		double lon, lat, radius;
	} S;
	struct GRDCUT_Z {	/* -Z[min/max][+n|N|r] */
		bool active;
		unsigned int mode;	/* 0-2, see below */
		double min, max;
	} Z;
};

#define NAN_IS_IGNORED	0
#define NAN_IS_INRANGE	1
#define NAN_IS_SKIPPED	2
#define NAN_IS_FRAME	3

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDCUT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDCUT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->In.type = GMT_IS_GRID;
	C->N.value = GMT->session.f_NaN;
	C->Z.min = -DBL_MAX;	C->Z.max = DBL_MAX;			/* No limits on z-range */
	C->Z.mode = NAN_IS_IGNORED;
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDCUT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->F.file);
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s %s -G%s %s [-D[+t]] [-F<polygontable>[+c][+i]] [%s] [-N[<nodata>]] [-S<lon>/<lat>/<radius>[+n]] [%s] [-Z[<min>/<max>][+n|N|r]] [%s] [%s]\n",
		name, GMT_INGRID, GMT_OUTGRID, GMT_Rgeo_OPT, GMT_J_OPT, GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	gmt_ingrid_syntax (API, 0, "Name of grid (or image) to extract a subset from");
	gmt_outgrid_syntax (API, 'G', "Set name of the output grid file");
	GMT_Option (API, "R");
	GMT_Usage (API, -2, "Typically, the w/e/s/n you specify must be within the region of the input "
		"grid.  If in doubt, run grdinfo first and check range of old file. "
		"Alternatively, see -N below.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-D[+t]");
	GMT_Usage (API, -2, "Dry-run mode. No grid is written but its domain and increment will be "
		"written to standard output in w e s n dx dy numerical format.  Append +t to instead receive text strings -Rw/e/s/n -Idx/dy.");
	GMT_Usage (API, 1, "\n-F<polygontable>[+c][+i]");
	GMT_Usage (API, -2, "Specify a multi-segment closed polygon table that describes the grid subset "
		"to be extracted (nodes between grid boundary and polygons will be set to NaN).");
	GMT_Usage (API, 3, "+c Crop the grid to the polygon bounding box [leave region as is].");
	GMT_Usage (API, 3, "+i Invert what is set to Nan, i.e., the inside of the polygon.");
	GMT_Usage (API, 1, "\n%s", GMT_J_OPT);
	GMT_Usage (API, -2, "Specify oblique projection and compute corresponding rectangular "
		"region that needs to be extracted.");
	GMT_Usage (API, 1, "\n-N[<nodata>]");
	GMT_Usage (API, -2, "Allow grid to be extended if new -R exceeds existing boundaries. "
		"Optionally, append value to initialize nodes outside current region [Default is NaN].");
	gmt_dist_syntax (API->GMT, "S<lon>/<lat>/<radius>[+n]", "Specify an origin and radius to find the corresponding rectangular area.");
	GMT_Usage (API, -2, "Note: All nodes on or inside the radius are contained in the subset grid. "
		"Append +n to set all nodes in the subset outside the circle to NaN.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-Z[<min>/<max>][+n|N|r]");
	GMT_Usage (API, -2, "Specify an optional range and determine the corresponding rectangular region "
		"so that all nodes outside this region are outside the range [-inf/+inf]. Modifiers related to the treatment of NaNs:");
	GMT_Usage (API, 3, "+n Consider NaNs to be outside the range. The resulting grid will be NaN-free.");
	GMT_Usage (API, 3, "+N Strip off outside rows and cols that are all populated with NaNs.");
	GMT_Usage (API, 3, "+r Consider NaNs to be within the range [Default just ignores NaNs in decision].");
	GMT_Option (API, "f,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDCUT_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	bool F_or_R_or_J, do_file_check = true;
	unsigned int n_errors = 0, k, n_files = 0;
	char za[GMT_LEN64] = {""}, zb[GMT_LEN64] = {""}, zc[GMT_LEN64] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next)
		if (opt->option == 'D') do_file_check = false;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) {n_errors++; continue; }
				Ctrl->In.active = true;
				if (opt->arg[0]) Ctrl->In.file = strdup (opt->arg);
				if (do_file_check && GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file))) n_errors++;
			break;

			/* Processes program-specific parameters */

			case 'D':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				if (strstr (opt->arg, "+t")) Ctrl->D.text = true;
				if (opt->arg[0] && strstr (opt->arg, "done-in-gmt_init_module")) {
					Ctrl->D.quit = true;	/* Reporting has already happened */
					gmt_M_str_free (opt->arg);	/* Free internal marker */
				}
				break;
			case 'F':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->F.active);
				Ctrl->F.active = true;
				if (opt->arg[0]) {
					if ((c = gmt_first_modifier (GMT, opt->arg, "ci"))) {	/* Process any modifiers */
						unsigned int pos = 0;	/* Reset to start of new word */
						char p[PATH_MAX] = {""};
						while (gmt_getmodopt (GMT, 'F', c, "ci", &pos, p, &n_errors) && n_errors == 0) {
							switch (p[0]) {
								case 'c': Ctrl->F.crop = true; break;
								case 'i': Ctrl->F.invert = 1; break;
								default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
							}
						}
						c[0] = '\0';	/* Chop off all modifiers so range can be determined */
					}
					Ctrl->F.file = strdup (opt->arg);
					if (c) c[0] = '+';	/* Restore modifier */
					if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->F.file))) n_errors++;
				}
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -F: No arguments given\n");
					n_errors++;
				}
				break;
			case 'G':	/* Output file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.file = strdup (opt->arg);
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file))) n_errors++;
				break;
			case 'N':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				if (opt->arg[0])
					Ctrl->N.value = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.f_NaN : (gmt_grdfloat)atof (opt->arg);
				break;
 			case 'S':	/* Origin and radius */
				Ctrl->S.active = true;
				k = 0;
				if ((c = strstr (opt->arg, "+n"))) {
					Ctrl->S.set_nan = true;
					c[0] = '\0';	/* Chop off modifier */
				}
				else if (opt->arg[k] == 'n' && gmt_M_compat_check (GMT, 5)) {
					Ctrl->S.set_nan = true;
					k = 1;
				}
				if (sscanf (&opt->arg[k], "%[^/]/%[^/]/%s", za, zb, zc) == 3) {
					n_errors += gmt_verify_expectations (GMT, GMT_IS_LON, gmt_scanf_arg (GMT, za, GMT_IS_LON, false, &Ctrl->S.lon), za);
					n_errors += gmt_verify_expectations (GMT, GMT_IS_LAT, gmt_scanf_arg (GMT, zb, GMT_IS_LAT, false, &Ctrl->S.lat), zb);
					Ctrl->S.mode = gmt_get_distance (GMT, zc, &(Ctrl->S.radius), &(Ctrl->S.unit));
				}
				if (c) c[0] = '+';	/* Restore modifier */
				break;
			case 'Z':	/* Detect region via z-range -Z[<min>/<max>][+n|N|r]*/
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.active = true;
				k = 0;
				if ((c = strstr (opt->arg, "+n")))
					Ctrl->Z.mode = NAN_IS_SKIPPED;
				else if ((c = strstr (opt->arg, "+N")))
					Ctrl->Z.mode = NAN_IS_FRAME;
				else if ((c = strstr (opt->arg, "+r")))
					Ctrl->Z.mode = NAN_IS_INRANGE;
				if (c) c[0] = '\0';	/* Chop off modifier */
				if (c == NULL && gmt_M_compat_check (GMT, 5)) {	/* Oldstyle -Zn|N|r[<min>/<max>] */
					if (opt->arg[k] == 'n') {
						Ctrl->Z.mode = NAN_IS_SKIPPED;
						k = 1;
					}
					else if (opt->arg[k] == 'N') {
						Ctrl->Z.mode = NAN_IS_FRAME;
						k = 1;
					}
					else if (opt->arg[k] == 'r') {
						Ctrl->Z.mode = NAN_IS_INRANGE;
						k = 1;
					}
				}
				if (sscanf (&opt->arg[k], "%[^/]/%s", za, zb) == 2) {
					if (!(za[0] == '-' && za[1] == '\0'))
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Z), gmt_scanf_arg (GMT, za, gmt_M_type (GMT, GMT_IN, GMT_Z), false, &Ctrl->Z.min), za);
					if (!(zb[0] == '-' && zb[1] == '\0'))
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Z), gmt_scanf_arg (GMT, zb, gmt_M_type (GMT, GMT_IN, GMT_Z), false, &Ctrl->Z.max), zb);
				}
				if (c) c[0] = '+';	/* Restore modifier */
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->G.file, "Option -D: Cannot specify -G since no grid will be returned\n");
	//n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && !GMT->common.J.active, "Option -D: Requires -R and -J\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.active[RSET] && Ctrl->F.crop, "Option -F: Modifier +c cannot be used with -R\n");
	F_or_R_or_J = GMT->common.R.active[RSET] | Ctrl->F.active | GMT->common.J.active;
	n_errors += gmt_M_check_condition (GMT, (F_or_R_or_J + Ctrl->S.active + Ctrl->Z.active) != 1,
	                                   "Must specify only one of the -F, -R, -S or the -Z options\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file && !Ctrl->D.active, "Option -G: Must specify output grid file\n");
	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Must specify one input grid file\n");
	if (n_errors == 0) {
		int ftype = gmt_raster_type (GMT, Ctrl->In.file, true);
		if (ftype == GMT_IS_IMAGE)	/* Must read file as an image */
			Ctrl->In.type = GMT_IS_IMAGE;
		else if (ftype == GMT_IS_GRID) {	/* Check extension in case of special case */
			if (strstr (Ctrl->G.file, ".tif"))	/* Want to write a single band (normally written as a grid) to geotiff instead */
				Ctrl->In.type = GMT_IS_IMAGE;
			else
				Ctrl->In.type = GMT_IS_GRID;
		}
		else	/* Just have to assume it is a grid */
			Ctrl->In.type = GMT_IS_GRID;
		if (Ctrl->In.type == GMT_IS_IMAGE) {
			n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active, "Option -N: Cannot be used with an image\n");
			n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active, "Option -Z: Cannot be used with an image\n");
		}
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL unsigned int grdcut_count_NaNs (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int row0, unsigned int row1, unsigned int col0, unsigned int col1, unsigned int count[], unsigned int mode, unsigned int *side, bool *all) {
	/* Loop around current perimeter and count # of nans, return sum and pass back which side had most nans */
	unsigned int col, row, sum = 0, k, dim[4] = {0, 0, 0, 0};
	uint64_t node;

	gmt_M_memset (count, 4, unsigned int);	/* Reset count */
	dim[GMT_X] = col1 - col0 + 1;	/* Number of remaining nodes in x */
	dim[GMT_Y] = row1 - row0 + 1;	/* Number of remaining nodes in y */
	*side = 0;
	/* South count: */
	for (col = col0, node = gmt_M_ijp (G->header, row1, col); col <= col1; col++, node++)
		if (gmt_M_is_fnan (G->data[node])) count[0]++;
	/* East count: */
	for (row = row0, node = gmt_M_ijp (G->header, row, col1); row <= row1; row++, node += G->header->mx)
		if (gmt_M_is_fnan (G->data[node])) count[1]++;
	/* North count: */
	for (col = col0, node = gmt_M_ijp (G->header, row0, col); col <= col1; col++, node++)
		if (gmt_M_is_fnan (G->data[node])) count[2]++;
	/* West count: */
	for (row = row0, node = gmt_M_ijp (G->header, row, col0); row <= row1; row++, node += G->header->mx)
		if (gmt_M_is_fnan (G->data[node])) count[3]++;
	for (k = 0; k < 4; k++) {	/* Time to sum up and determine side with most NaNs */
		sum += count[k];
		if (mode == NAN_IS_FRAME) {
			if (k && count[k] > dim[*side]) *side = k;
		}
		else {
			if (k && count[k] > count[*side]) *side = k;
		}
	}
	*all = (count[*side] == dim[*side%2]);	/* True if every node along size is NaN */
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Nans found: W = %d E = %d S = %d N = %d\n",
	            count[3], count[1], count[0], count[2]);
	return ((row0 == row1 && col0 == col1) ? 0 : sum);	/* Return 0 if we run out of grid, else the sum */
}

GMT_LOCAL int grdcut_set_rectangular_subregion (struct GMT_CTRL *GMT, double wesn[], struct GMT_GRID_HEADER *h) {
	/* Select a subset either via -R or combination or -R -J typically for non-rectangular projections */
	double *inc = h->inc;
	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double); /* Default is to take the -R if given */
	if (GMT->current.proj.projection == GMT_NO_PROJ) return GMT_NOERROR;	/* Nothing else to do */
	if (!GMT->common.R.active[RSET])	/* Need a -R with -J if no -R given */
		gmt_M_memcpy (wesn, h->wesn, 4, double);

	/* Here we got an oblique area and a projection; find the corresponding rectangular -R that covers this oblique area */

	if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, wesn), "")) return (GMT_PROJECTION_ERROR);

	/* If geographic and we got an oblique -R -J setup or a global region then we search for a subset.  Else, -R was
	 * set to a specific subset and we keep that as what the user specified. */
	if (gmt_M_is_geographic (GMT, GMT_IN) && (GMT->common.R.oblique ||
		(gmt_M_180_range (wesn[YHI], wesn[YLO]) && gmt_M_360_range (wesn[XHI], wesn[XLO])))) {
		gmt_wesn_search (GMT, GMT->current.proj.rect[XLO], GMT->current.proj.rect[XHI], GMT->current.proj.rect[YLO],
			GMT->current.proj.rect[YHI], &GMT->common.R.wesn[XLO], &GMT->common.R.wesn[XHI],
			&GMT->common.R.wesn[YLO], &GMT->common.R.wesn[YHI], false);
		/* Round to nearest multiple of grid increment */
		wesn[XLO] = floor (GMT->common.R.wesn[XLO] / inc[GMT_X]) * inc[GMT_X];
		wesn[XHI] = ceil  (GMT->common.R.wesn[XHI] / inc[GMT_X]) * inc[GMT_X];
		wesn[YLO] = floor (GMT->common.R.wesn[YLO] / inc[GMT_Y]) * inc[GMT_Y];
		wesn[YHI] = ceil  (GMT->common.R.wesn[YHI] / inc[GMT_Y]) * inc[GMT_Y];
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION) && rint (inc[GMT_X] * 60.0) == (inc[GMT_X] * 60.0)) {	/* Spacing in whole arc minutes */
		int w, e, s, n, wm, em, sm, nm;

		w = irint (floor (wesn[XLO]));	wm = irint ((wesn[XLO] - w) * 60.0);
		e = irint (floor (wesn[XHI]));	em = irint ((wesn[XHI] - e) * 60.0);
		s = irint (floor (wesn[YLO]));	sm = irint ((wesn[YLO] - s) * 60.0);
		n = irint (floor (wesn[YHI]));	nm = irint ((wesn[YHI] - n) * 60.0);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "%s -> -R%d:%02d/%d:%02d/%d:%02d/%d:%02d\n",
		            GMT->common.R.string, w, wm, e, em, s, sm, n, nm);
	}
	else if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION) && rint (inc[GMT_X] * 3600.0) == (inc[GMT_X] * 3600.0)) {	/* Spacing in whole arc seconds */
		int w, e, s, n, wm, em, sm, nm, ws, es, ss, ns;

		w = irint (floor (wesn[XLO]));	wm = irint (floor ((wesn[XLO] - w) * 60.0));
		ws = irint (floor ((wesn[XLO] - w - wm/60.0) * 3600.0));
		e = irint (floor (wesn[XHI]));	em = irint (floor ((wesn[XHI] - e) * 60.0));
		es = irint (floor ((wesn[XHI] - e - em/60.0) * 3600.0));
		s = irint (floor (wesn[YLO]));	sm = irint (floor ((wesn[YLO] - s) * 60.0));
		ss = irint (floor ((wesn[YLO] - s - sm/60.0) * 3600.0));
		n = irint (floor (wesn[YHI]));	nm = irint (floor ((wesn[YHI] - n) * 60.0));
		ns = irint (floor ((wesn[YHI] - n - nm/60.0) * 3600.0));
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "%s -> -R%d:%02d:%02d/%d:%02d:%02d/%d:%02d:%02d/%d:%02d:%02d\n",
		            GMT->common.R.string, w, wm, ws, e, em, es, s, sm, ss, n, nm, ns);
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "%s -> -R%g/%g/%g/%g\n",
		            GMT->common.R.string, wesn[XLO], wesn[XHI], wesn[YLO], wesn[YHI]);
	return GMT_NOERROR;
}

GMT_LOCAL unsigned int grdcut_node_is_outside (struct GMT_CTRL *GMT, struct GMT_DATASET *D, double lon, double lat) {
	/* Returns 1 if the selected point is outside the polygon, 0 if inside */
	uint64_t seg;
	unsigned int inside = 0;
	for (seg = 0; seg < D->table[0]->n_segments && !inside; seg++) {	/* Use degrees since function expects it */
		if (gmt_polygon_is_hole (GMT, D->table[0]->segment[seg])) continue;	/* Holes are handled within gmt_inonout */
		inside = (gmt_inonout (GMT, lon, lat, D->table[0]->segment[seg]) > GMT_OUTSIDE);
	}
	return ((inside) ? 0 : 1);	/* 1 if outside */
}


#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grdcut (void *V_API, int mode, void *args) {
	int error = 0;
	unsigned int nx_old, ny_old, add_mode = 0U, side, extend, type = 0U, def_pad[4], pad[4];
	uint64_t node;
	bool outside[4] = {false, false, false, false}, all, bail = false, geo_to_cart = false, do_via_gdal = false;

	char *name[2][4] = {{"left", "right", "bottom", "top"}, {"west", "east", "south", "north"}};

	double wesn_new[4], wesn_old[4], wesn_requested[4];
	double lon, lat, distance, radius;

	struct GMT_DATASET *D = NULL;
	struct GMT_GRID_HEADER test_header, *h = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GRDCUT_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_IMAGE *I = NULL;
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

	/*---------------------------- This is the grdcut main code ----------------------------*/

	if (Ctrl->D.quit)	/* Already reported information deep inside gmt_init_module */
		Return (GMT_NOERROR);

	gmt_grd_set_datapadding (GMT, true);	/* Turn on gridpadding when reading a subset */

	if (!Ctrl->Z.active) {	/* All of -F, -R, -S selections first needs the header */
		if (Ctrl->In.type == GMT_IS_IMAGE) {
			GMT_Report (API, GMT_MSG_INFORMATION, "Processing input image\n");
			if ((I = GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY | GMT_GRID_IS_IMAGE, NULL, Ctrl->In.file, NULL)) == NULL) {
				Return (API->error);	/* Get header only */
			}
			h = I->header;
			if ((I->type == GMT_FLOAT || h->n_bands > 4) && !gmt_M_file_is_memory (Ctrl->In.file) && !gmt_M_file_is_memory (Ctrl->G.file) && strstr (Ctrl->In.file, ".tif")) {
				do_via_gdal = true;	/* Use gdal_translate for this multi-layer data subset */
				if (!Ctrl->D.active && strstr (Ctrl->G.file, ".tif") == NULL && strstr (Ctrl->G.file, ".nc") == NULL && strstr (Ctrl->G.file, ".grd") == NULL)  {
					GMT_Report (API, GMT_MSG_INFORMATION, "Option -G: Must give an output file name with extensions .tiff (geotiff) or .nc or .grd (netCDF) when selecting multiband output from a geotiff file\n");
					Return (GMT_RUNTIME_ERROR);	/* Get header only */
				}
			}
		}
		else {
			GMT_Report (API, GMT_MSG_INFORMATION, "Processing input grid\n");
			if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {
				Return (API->error);	/* Get header only */
			}
			h = G->header;
		}
	}

	if (Ctrl->Z.active) {	/* Must determine new region via -Z, so get entire grid first */
		unsigned int row0 = 0, row1 = 0, col0 = 0, col1 = 0, row, col, sum, side, count[4];
		bool go;
		struct GMT_GRID_HIDDEN *GH = NULL;

		GMT_Report (API, GMT_MSG_INFORMATION, "Processing input grid\n");
		if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->In.file, NULL)) == NULL) {
			Return (API->error);	/* Get entire grid */
		}
		h = G->header;
		row1 = h->n_rows - 1;	col1 = h->n_columns - 1;
		if (Ctrl->Z.mode == NAN_IS_SKIPPED) {	/* Must scan in from outside to the inside, one side at the time, remove side with most Nans */
			sum = grdcut_count_NaNs (GMT, G, row0, row1, col0, col1, count, NAN_IS_SKIPPED, &side, &all);	/* Initial border count */
			while (sum) {	/* Must eliminate the row or col with most NaNs, and move grid boundary inwards */
				if (side == 3 && col0 < col1) {	/* Need to move in from the left */
					col0++;
					GMT_Report (API, GMT_MSG_INFORMATION, "Stip off a leftmost column\n");
				}
				else if (side == 1 && col1 > col0) {	/* Need to move in from the right */
					col1--;
					GMT_Report (API, GMT_MSG_INFORMATION, "Stip off rightmost column\n");
				}
				else if (side == 0 && row1 > row0) {	/* Need to move up from the bottom */
					row1--;
					GMT_Report (API, GMT_MSG_INFORMATION, "Stip off bottom row\n");
				}
				else if (side == 2 && row0 < row1) {	/* Need to move down from the top */
					row0++;
					GMT_Report (API, GMT_MSG_INFORMATION, "Stip off top row\n");
				}
				sum = grdcut_count_NaNs (GMT, G, row0, row1, col0, col1, count, NAN_IS_SKIPPED, &side, &all);
			}
			if (col0 == col1 || row0 == row1) {
				GMT_Report (API, GMT_MSG_ERROR, "The sub-region implied by -Z+n is empty!\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}
		else if (Ctrl->Z.mode == NAN_IS_FRAME) {	/* Must scan in from outside to the inside, one side at the time, remove sides with all Nans */
			sum = grdcut_count_NaNs (GMT, G, row0, row1, col0, col1, count, NAN_IS_FRAME, &side, &all);	/* Initial border count */
			while (all) {	/* Must eliminate the row or col with most NaNs, and move grid boundary inwards */
				if (side == 3 && col0 < col1) {	/* Need to move in from the left */
					col0++;
					GMT_Report (API, GMT_MSG_INFORMATION, "Stip off a leftmost column\n");
				}
				else if (side == 1 && col1 > col0) {	/* Need to move in from the right */
					col1--;
					GMT_Report (API, GMT_MSG_INFORMATION, "Stip off rightmost column\n");
				}
				else if (side == 0 && row1 > row0) {	/* Need to move up from the bottom */
					row1--;
					GMT_Report (API, GMT_MSG_INFORMATION, "Stip off bottom row\n");
				}
				else if (side == 2 && row0 < row1) {	/* Need to move down from the top */
					row0++;
					GMT_Report (API, GMT_MSG_INFORMATION, "Stip off top row\n");
				}
				sum = grdcut_count_NaNs (GMT, G, row0, row1, col0, col1, count, NAN_IS_FRAME, &side, &all);
			}
			if (col0 == col1 || row0 == row1) {
				GMT_Report (API, GMT_MSG_ERROR, "The sub-region implied by -Z+N is empty!\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}
		/* Here NaNs have either been skipped by inward search, or will be ignored (NAN_IS_IGNORED)
		   or will be considered to be within range (NAN_IS_INRANGE) */
		for (row = row0, go = true; go && row <= row1; row++) {	/* Scan from ymax towards ymin */
			for (col = col0, node = gmt_M_ijp (h, row, 0); go && col <= col1; col++, node++) {
				if (gmt_M_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INRANGE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) row0 = row;	/* Found starting row */
			}
		}
		if (go) {
			GMT_Report (API, GMT_MSG_ERROR, "The sub-region implied by -Z is empty!\n");
			Return (GMT_RUNTIME_ERROR);
		}
		for (row = row1, go = true; go && row > row0; row--) {	/* Scan from ymin towards ymax */
			for (col = col0, node = gmt_M_ijp (h, row, 0); go && col <= col1; col++, node++) {
				if (gmt_M_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INRANGE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) row1 = row;	/* Found stopping row */
			}
		}
		for (col = col0, go = true; go && col <= col1; col++) {	/* Scan from xmin towards xmax */
			for (row = row0, node = gmt_M_ijp (h, row0, col); go && row <= row1; row++, node += h->mx) {
				if (gmt_M_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INRANGE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) col0 = col;	/* Found starting col */
			}
		}
		for (col = col1, go = true; go && col >= col0; col--) {	/* Scan from xmax towards xmin */
			for (row = row0, node = gmt_M_ijp (h, row0, col); go && row <= row1; row++, node += h->mx) {
				if (gmt_M_is_fnan (G->data[node])) {
					if (Ctrl->Z.mode == NAN_IS_INRANGE) go = false;	/* Must stop since this NaN value defines the inner box */
				}
				else if (G->data[node] >= Ctrl->Z.min && G->data[node] <= Ctrl->Z.max)
					go = false;
				if (!go) col1 = col;	/* Found stopping col */
			}
		}
		if (row0 == 0 && col0 == 0 && row1 == (h->n_rows-1) && col1 == (h->n_columns-1)) {
			GMT_Report (API, GMT_MSG_WARNING, "Your -Z limits produced no subset - output grid is identical to input grid\n");
			gmt_M_memcpy (wesn_new, h->wesn, 4, double);
		}
		else {	/* Adjust boundaries inwards */
			wesn_new[XLO] = h->wesn[XLO] + col0 * h->inc[GMT_X];
			wesn_new[XHI] = h->wesn[XHI] - (h->n_columns - 1 - col1) * h->inc[GMT_X];
			wesn_new[YLO] = h->wesn[YLO] + (h->n_rows - 1 - row1) * h->inc[GMT_Y];
			wesn_new[YHI] = h->wesn[YHI] - row0 * h->inc[GMT_Y];
		}
		GH = gmt_get_G_hidden (G);
		if (GH->alloc_mode == GMT_ALLOC_INTERNALLY) gmt_M_free_aligned (GMT, G->data);	/* Free the grid array only as we need the header below */
		add_mode = GMT_IO_RESET;	/* Pass this to allow reading the data again. */
	}
	else if (Ctrl->S.active) {	/* Must determine new region via -S, so only need header */
		/* Note: The use of g and gmt_M_grd_row_to_y is correct since lon and lat args are not
		 * coordinates computed from west or south in whole increments of dx dy. */
		int row, col;
		bool wrap = gmt_M_360_range (h->wesn[XLO], h->wesn[XHI]);	/* true if grid is 360 global */

		if (gmt_M_is_cartesian (GMT, GMT_IN)) {
			GMT_Report (API, GMT_MSG_ERROR, "The -S option requires a geographic grid or image\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (gmt_init_distaz (GMT, Ctrl->S.unit, Ctrl->S.mode, GMT_MAP_DIST) == GMT_NOT_A_VALID_TYPE)
			Return (GMT_NOT_A_VALID_TYPE);
		/* Set w/e to center and adjust in case of -/+ 360 stuff */
		wesn_new[XLO] = wesn_new[XHI] = Ctrl->S.lon;
		if (wrap) {
			while (wesn_new[XLO] < h->wesn[XLO]) wesn_new[XLO] += 360.0, wesn_new[XHI] += 360.0;
			while (wesn_new[XLO] > h->wesn[XHI]) wesn_new[XLO] -= 360.0, wesn_new[XHI] -= 360.0;
		}
		wesn_new[YLO] = wesn_new[YHI] = Ctrl->S.lat;
		/* First adjust the S and N boundaries */
		if (GMT->current.map.dist[GMT_MAP_DIST].arc)	/* Got arc distance */
			radius = (Ctrl->S.radius / GMT->current.map.dist[GMT_MAP_DIST].scale);	/* Radius in degrees */
		else
			radius = R2D * (Ctrl->S.radius / GMT->current.map.dist[GMT_MAP_DIST].scale) / GMT->current.proj.mean_radius;	/* Approximate radius in degrees */
		wesn_new[YLO] -= radius;	/* Approximate south limit in degrees */
		if (wesn_new[YLO] <= h->wesn[YLO]) {	/* Way south, reset to grid S limit */
			wesn_new[YLO] = h->wesn[YLO];
		}
		else {	/* Possibly adjust south a bit using chosen distance calculation */
			row = (int)gmt_M_grd_y_to_row (GMT, wesn_new[YLO], h);		/* Nearest row with this latitude */
			lat = gmt_M_grd_row_to_y (GMT, row, h);			/* Latitude of that row */
			distance = gmt_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			while (distance <= Ctrl->S.radius) {	/* Extend region by one more row until we are outside */
				lat -= h->inc[GMT_Y];
				distance = gmt_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			}
			wesn_new[YLO] = lat + (1.0 - h->xy_off) * h->inc[GMT_Y];	/* Go one back since last row was outside */
			/* The (1-xy_off) adjust since grid boundaries for pixel grids do not coincide with pixel node coordinates - they are a half off */
			if (wesn_new[YLO] <= h->wesn[YLO]) wesn_new[YLO] = h->wesn[YLO];
		}
		wesn_new[YHI] += radius;	/* Approximate north limit in degrees */
		if (wesn_new[YHI] >= h->wesn[YHI]) {	/* Way north, reset to grid N limit */
			wesn_new[YHI] = h->wesn[YHI];
		}
		else {	/* Possibly adjust north a bit using chosen distance calculation */
			row = (int)gmt_M_grd_y_to_row (GMT, wesn_new[YHI], h);		/* Nearest row with this latitude */
			lat = gmt_M_grd_row_to_y (GMT, row, h);			/* Latitude of that row */
			distance = gmt_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			while (distance <= Ctrl->S.radius) {	/* Extend region by one more row until we are outside */
				lat += h->inc[GMT_Y];
				distance = gmt_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, Ctrl->S.lon, lat);
			}
			wesn_new[YHI] = lat - (1.0 - h->xy_off) * h->inc[GMT_Y];	/* Go one back since last row was outside */
			/* The (1-xy_off) adjust since grid boundaries for pixel grids do not coincide with pixel node coordinates - they are a half off */
			if (wesn_new[YHI] >= h->wesn[YHI]) wesn_new[YHI] = h->wesn[YHI];
		}
		if (doubleAlmostEqual (wesn_new[YLO], -90.0) || doubleAlmostEqual (wesn_new[YHI], 90.0)) {	/* Need all longitudes when a pole is included */
			wesn_new[XLO] = h->wesn[XLO];
			wesn_new[XHI] = h->wesn[XHI];
		}
		else {	/* Determine longitude limits */
			radius /= cosd (Ctrl->S.lat);	/* Approximate e-w width in degrees longitude at center point */
			wesn_new[XLO] -= radius;	/* Approximate west limit in degrees */
			if (!wrap && wesn_new[XLO] < h->wesn[XLO]) {	/* Outside non-periodic grid range */
				wesn_new[XLO] = h->wesn[XLO];
			}
			else {
				col = (int)gmt_M_grd_x_to_col (GMT, wesn_new[XLO], h);		/* Nearest col with this longitude */
				lon = gmt_M_grd_col_to_x (GMT, col, h);			/* Longitude of that col */
				distance = gmt_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				while (distance <= Ctrl->S.radius) {	/* Extend region by one more col until we are outside */
					lon -= h->inc[GMT_X];
					distance = gmt_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				}
				wesn_new[XLO] = lon + (1.0 - h->xy_off) * h->inc[GMT_X];	/* Go one back since last col was outside */
			}
			wesn_new[XHI] += radius;					/* Approximate east limit in degrees */
			if (!wrap && wesn_new[XHI] > h->wesn[XHI]) {		/* Outside grid range */
				wesn_new[XHI] = h->wesn[XHI];
			}
			else {
				col = (int)gmt_M_grd_x_to_col (GMT, wesn_new[XHI], h);		/* Nearest col with this longitude */
				lon = gmt_M_grd_col_to_x (GMT, col, h);			/* Longitude of that col */
				distance = gmt_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				while (distance <= Ctrl->S.radius) {	/* Extend region by one more col until we are outside */
					lon += h->inc[GMT_X];
					distance = gmt_distance (GMT, lon, Ctrl->S.lat, Ctrl->S.lon, Ctrl->S.lat);
				}
				wesn_new[XHI] = lon - (1.0 - h->xy_off) * h->inc[GMT_X];	/* Go one back since last col was outside */
			}
			if ((wesn_new[XHI] - wesn_new[XLO]) > 360.0) wesn_new[XHI] -= 360.0;	/* One of them got off by 360 */
			if (wesn_new[XHI] > 360.0) wesn_new[XLO] -= 360.0, wesn_new[XHI] -= 360.0;
			if (wesn_new[XHI] < 0.0)   wesn_new[XLO] += 360.0, wesn_new[XHI] += 360.0;
		}
	}
	else if (Ctrl->F.active) {	/* Only read in subset corresponding to the bounding box of the polygon */
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_READ_NORMAL, NULL, Ctrl->F.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (D->n_records == 0) {
			GMT_Report (API, GMT_MSG_ERROR, "No data records found - exiting\n");
			Return (GMT_RUNTIME_ERROR);			
		}
		gmt_M_memcpy (wesn_new, h->wesn, 4U, double);
		if (GMT->common.R.active[RSET])	/* Let -R override what the grid says */
			gmt_M_memcpy (wesn_new, GMT->common.R.wesn, 4U, double);
		if (Ctrl->F.crop) {	/* Get new grid region from polygon bounding box */
			wesn_new[YLO] = MAX (wesn_new[YLO], floor (D->min[GMT_Y] / h->inc[GMT_Y]) * h->inc[GMT_Y]);
			wesn_new[YHI] = MIN (wesn_new[YHI], ceil  (D->max[GMT_Y] / h->inc[GMT_Y]) * h->inc[GMT_Y]);
			if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must worry about longitude */
				if (gmt_M_360_range (wesn_new[XLO], wesn_new[XHI])) {
					wesn_new[XLO] = floor (D->min[GMT_X] / h->inc[GMT_X]) * h->inc[GMT_X];
					wesn_new[XHI] = ceil  (D->max[GMT_X] / h->inc[GMT_X]) * h->inc[GMT_X];
				}
				else {	/* The two domains could be off by 360 */
					if (D->min[GMT_X] > wesn_new[XHI]) wesn_new[XLO] += 360.0, wesn_new[XHI] += 360.0;
					else if (D->max[GMT_X] < wesn_new[XLO]) wesn_new[XLO] -= 360.0, wesn_new[XHI] -= 360.0;
					wesn_new[XLO] = MAX (wesn_new[XLO], floor (D->min[GMT_X] / h->inc[GMT_X]) * h->inc[GMT_X]);
					wesn_new[XHI] = MIN (wesn_new[XHI], ceil  (D->max[GMT_X] / h->inc[GMT_X]) * h->inc[GMT_X]);
				}
			}
			else {	/* Plain Cartesian case */
				wesn_new[XLO] = MAX (wesn_new[XLO], floor (D->min[GMT_X] / h->inc[GMT_X]) * h->inc[GMT_X]);
				wesn_new[XHI] = MIN (wesn_new[XHI], ceil  (D->max[GMT_X] / h->inc[GMT_X]) * h->inc[GMT_X]);
			}
		}
	}
	else {	/* Just the usual subset selection via -R.  */
		if (grdcut_set_rectangular_subregion (GMT, wesn_new, h)) {
			Return (API->error);	/* Get header only */
		}
	}

	/* Basic sanity checking that the requested region has at least some overlap with the actual region */

	if (wesn_new[YLO] >= h->wesn[YHI] || wesn_new[YHI] <= h->wesn[YLO]) {	/* y-check is simple */
		GMT_Report (API, GMT_MSG_ERROR, "Requested subset is entirely below or above the current grid region\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (gmt_M_x_is_lon (GMT, GMT_IN)) {	/* Geographic data required more trickery */
		double we[2] = {wesn_new[XLO], wesn_new[XHI]};
		gmt_set_column_type (GMT, GMT_OUT, GMT_X, GMT_IS_LON);
		while (we[XHI] > h->wesn[XLO]) we[XLO] -= 360.0, we[XHI] -= 360.0;	/* Wind past on the left */
		we[XLO] += 360.0, we[XHI] += 360.0;	/* Now we either overlap or we are past on the right */
		if (we[XLO] >= h->wesn[XHI])
			bail = true;	/* Outside original w/e extent */
	}
	else if (wesn_new[XLO] >= h->wesn[XHI] || wesn_new[XHI] <= h->wesn[XLO])	/* Cartesian x-check is simple */
		bail = true;
	if (gmt_M_y_is_lat (GMT, GMT_IN) && (wesn_new[YLO] < -90.0 || wesn_new[YHI] > 90.0)) {
		gmt_set_column_type (GMT, GMT_OUT, GMT_Y, GMT_IS_FLOAT);
		gmt_grd_set_cartesian (GMT, h, 2);
		geo_to_cart = true;
	}

	if (bail) {
		GMT_Report (API, GMT_MSG_ERROR, "Requested subset is entirely to the left or to the right of the current data region\n");
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->D.active) {	/* Report tight region given -R */
		double out[6];
		char record[GMT_LEN256] = {""};
		struct GMT_RECORD *Out = NULL;
		GMT_Report (API, GMT_MSG_INFORMATION, "Extracted region will be %g/%g/%g/%g for increments %lg/%lg\n", wesn_new[XLO], wesn_new[XHI], wesn_new[YLO], wesn_new[YHI], h->inc[GMT_X], h->inc[GMT_Y]);
		if (GMT_Set_Columns (API, GMT_OUT, Ctrl->D.text ? 0 : 6, Ctrl->D.text ? GMT_COL_FIX : GMT_COL_FIX_NO_TEXT) != GMT_NOERROR)
			Return (API->error);
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (gmt_M_x_is_lon (GMT, GMT_IN)) {
			if (wesn_new[XLO] < 0.0 && wesn_new[XHI] <= 0.0)
				GMT->current.io.geo.range = GMT_IS_GIVEN_RANGE;
			else if ((wesn_new[XHI] - wesn_new[XLO]) > 180.0)
				GMT->current.io.geo.range = GMT_IS_GIVEN_RANGE;
			else if (wesn_new[XLO] < 0.0 && wesn_new[XHI] >= 0.0)
				GMT->current.io.geo.range = GMT_IS_M180_TO_P180_RANGE;
			else
				GMT->current.io.geo.range = GMT_IS_0_TO_P360_RANGE;
		}
		if (Ctrl->D.text) {
			enum gmt_col_enum type = gmt_get_column_type (GMT, GMT_IN, GMT_X);
			char inc[GMT_LEN64] = {""};
			Out = gmt_new_record (GMT, NULL, record);	/* The trailing text output record */
			gmt_format_region (GMT, record, wesn_new);	/* Typeset the -Rw/e/s/n part */
			gmt_ascii_format_inc (GMT, inc, h->inc[GMT_X], type);
			strcat (record, " -I"); strcat (record, inc);
			type = gmt_get_column_type (GMT, GMT_IN, GMT_Y);
			gmt_ascii_format_inc (GMT, inc, h->inc[GMT_Y], type);
			strcat (record, "/");	strcat (record, inc);	/* dlat or yinc */
		}
		else {
			Out = gmt_new_record (GMT, out, NULL);	/* The numerical output record */
			gmt_M_memcpy (out, wesn_new, 4U, double);	/* Place the grid boundaries */
			out[4] = h->inc[GMT_X];
			out[5] = h->inc[GMT_Y];
		}
		GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR)	/* Disables further data output */
			Return (API->error);
		gmt_M_free (GMT, Out);
		Return (GMT_NOERROR);
	}

	gmt_M_memcpy (wesn_requested, wesn_new, 4, double);
	if (wesn_new[YLO] < h->wesn[YLO]) wesn_new[YLO] = h->wesn[YLO], outside[YLO] = true;
	if (wesn_new[YHI] > h->wesn[YHI]) wesn_new[YHI] = h->wesn[YHI], outside[YHI] = true;
	HH = gmt_get_H_hidden (h);

	if (gmt_M_x_is_lon (GMT, GMT_IN)) {	/* Geographic longitude */
		if (wesn_new[XLO] < h->wesn[XLO] && wesn_new[XHI] < h->wesn[XLO]) {
			h->wesn[XLO] -= 360.0;
			h->wesn[XHI] -= 360.0;
		}
		if (wesn_new[XLO] > h->wesn[XHI] && wesn_new[XHI] > h->wesn[XHI]) {
			h->wesn[XLO] += 360.0;
			h->wesn[XHI] += 360.0;
		}
		if (!gmt_grd_is_global (GMT, h)) {
			if (wesn_new[XLO] < h->wesn[XLO]) wesn_new[XLO] = h->wesn[XLO], outside[XLO] = true;
			if (wesn_new[XHI] > h->wesn[XHI]) wesn_new[XHI] = h->wesn[XHI], outside[XHI] = true;
		}
		type = 1U;
	}
	else {
		if (wesn_new[XLO] < h->wesn[XLO]) wesn_new[XLO] = h->wesn[XLO], outside[XLO] = true;
		if (wesn_new[XHI] > h->wesn[XHI]) wesn_new[XHI] = h->wesn[XHI], outside[XHI] = true;
	}

	for (side = extend = 0; side < 4; side++) {
		if (!outside[side]) continue;
		extend++;
		if (Ctrl->N.active)
			GMT_Report (API, GMT_MSG_INFORMATION, "Requested subset exceeds data domain on the %s side - nodes in the extra area will be initialized to %g\n", name[type][side], Ctrl->N.value);
		else
			GMT_Report (API, GMT_MSG_INFORMATION, "Requested subset exceeds data domain on the %s side - truncated to match data bounds\n", name[type][side]);
	}

	/* Make sure output grid is kosher */

	gmt_adjust_loose_wesn (GMT, wesn_new, h);

	gmt_M_memcpy (test_header.wesn, wesn_new, 4, double);
	gmt_M_memcpy (test_header.inc, h->inc, 2, double);
	if ((error = gmt_M_err_fail (GMT, gmt_grd_RI_verify (GMT, &test_header, 1), Ctrl->G.file)))
		Return (error);

	/* OK, so far so good. Check if new wesn differs from old wesn by integer dx/dy */

	if (gmt_minmaxinc_verify (GMT, h->wesn[XLO], wesn_new[XLO], h->inc[GMT_X], GMT_CONV4_LIMIT) == 1) {
		GMT_Report (API, GMT_MSG_ERROR, "Old and new x_min do not differ by N * dx\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (gmt_minmaxinc_verify (GMT, wesn_new[XHI], h->wesn[XHI], h->inc[GMT_X], GMT_CONV4_LIMIT) == 1) {
		GMT_Report (API, GMT_MSG_ERROR, "Old and new x_max do not differ by N * dx\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (gmt_minmaxinc_verify (GMT, h->wesn[YLO], wesn_new[YLO], h->inc[GMT_Y], GMT_CONV4_LIMIT) == 1) {
		GMT_Report (API, GMT_MSG_ERROR, "Old and new y_min do not differ by N * dy\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (gmt_minmaxinc_verify (GMT, wesn_new[YHI], h->wesn[YHI], h->inc[GMT_Y], GMT_CONV4_LIMIT) == 1) {
		GMT_Report (API, GMT_MSG_ERROR, "Old and new y_max do not differ by N * dy\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (!GMT->common.R.active[RSET]) {	/* Got a new region indirectly via -S or -Z */
		gmt_M_memcpy (GMT->common.R.wesn, wesn_new, 4, double);
		GMT->common.R.active[RSET] = true;
	}

	gmt_grd_init (GMT, h, options, true);

	gmt_M_memcpy (wesn_old, h->wesn, 4, double);
	nx_old = h->n_columns;		ny_old = h->n_rows;

	if (Ctrl->F.active && Ctrl->In.type == GMT_IS_GRID) {	/* Must reset nodes outside the polygon to NaN */
		unsigned int row, col, set;
		uint64_t n_nodes = 0;
		gmt_set_inside_mode (GMT, D, GMT_IOO_UNKNOWN);
		if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY | add_mode, wesn_new, Ctrl->In.file, G) == NULL) {	/* Get subset (unless memory file) */
			Return (API->error);
		}
		gmt_M_grd_loop (GMT, G, row, col, node) {
			/* set is either 0, 1, or 2, but only the case 1 means we set the node to NaN */
			set = Ctrl->F.invert + grdcut_node_is_outside (GMT, D, G->x[col], G->y[row]);
			if (set == 1) G->data[node] = GMT->session.f_NaN;
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Set %" PRIu64 " grid nodes outside polygon to NaN\n", n_nodes);
	}
	if (Ctrl->N.active && extend) {	/* Determine the pad needed for the extended area */
		gmt_M_memcpy (def_pad, GMT->current.io.pad, 4, unsigned int);	/* Default pad */
		gmt_M_memcpy (pad, def_pad, 4, unsigned int);			/* Starting pad */
		if (outside[XLO]) pad[XLO] += urint ((h->wesn[XLO] - wesn_requested[XLO]) * HH->r_inc[GMT_X]);
		if (outside[XHI]) pad[XHI] += urint ((wesn_requested[XHI] - h->wesn[XHI]) * HH->r_inc[GMT_X]);
		if (outside[YLO]) pad[YLO] += urint ((h->wesn[YLO] - wesn_requested[YLO]) * HH->r_inc[GMT_Y]);
		if (outside[YHI]) pad[YHI] += urint ((wesn_requested[YHI] - h->wesn[YHI]) * HH->r_inc[GMT_Y]);
		gmt_M_memcpy (GMT->current.io.pad, pad, 4, unsigned int);	/* Change default pad */
		if (!gmt_M_file_is_memory (Ctrl->In.file)) {	/* If a memory grid we end up duplicating below so only do this in the other cases */
			gmt_M_grd_setpad (GMT, h, pad);	/* Set the active pad before reading */
			gmt_set_grddim (GMT, h);	/* Update dimensions given the change of pad */
		}
	}
	if (!Ctrl->F.active) {	/* Read in the subset grid or image */
		if (Ctrl->In.type == GMT_IS_GRID) {
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY | add_mode, wesn_new, Ctrl->In.file, G) == NULL) {	/* Get subset (unless memory file) */
				Return (API->error);
			}
		}
		else if (!do_via_gdal) {
			if (GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn_new, Ctrl->In.file, I) == NULL) { 	/* Get subset */
				Return (API->error);
			}
		}
	}
	if (gmt_M_file_is_memory (Ctrl->In.file) && (Ctrl->N.active || gmt_M_file_is_memory (Ctrl->G.file))) {
		/* Cannot manipulate the same grid in two different ways so we must make a duplicate of the input grid */
		if (Ctrl->In.type == GMT_IS_GRID) {
			struct GMT_GRID *G_dup = NULL;	/* For the duplicate; we eliminate any unnecessary padding using GMT_DUPLICATE_RESET */
			if ((G_dup = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_DATA | GMT_DUPLICATE_RESET, G)) == NULL) {
				Return (API->error);
			}
			G = G_dup;	/* Since G was not allocated here anyway - it came from the outside and will be deleted there */
		}
		else {
			struct GMT_IMAGE *I_dup = NULL;	/* For the duplicate; we eliminate any unnecessary padding using GMT_DUPLICATE_RESET */
			if ((I_dup = GMT_Duplicate_Data (API, GMT_IS_IMAGE, GMT_DUPLICATE_DATA | GMT_DUPLICATE_RESET, I)) == NULL) {
				Return (API->error);
			}
			I = I_dup;	/* Since G was not allocated here anyway - it came from the outside and will be deleted there */
		}
	}
	if (Ctrl->N.active && extend) {	/* Now shrink pad back to default and simultaneously extend region and apply nodata values */
		unsigned int xlo, xhi, ylo, yhi, row, col, n_zero, n_zero_e;
		double dx, dy;
		n_zero = 0;	/* Count zeros in the grid before extension */
		gmt_M_grd_loop (GMT, G, row, col, node) {
			if (G->data[node] == 0.0) n_zero++;
		}
		gmt_M_memcpy (h->wesn, wesn_requested, 4, double);
		gmt_M_memcpy (GMT->current.io.pad, def_pad, 4, unsigned int);	/* Reset default pad */
		gmt_M_grd_setpad (GMT, h, GMT->current.io.pad);	/* Set the default pad */
		gmt_set_grddim (GMT, h);			/* Update dimensions given the change of wesn and pad */
		gmt_M_memcpy (wesn_new, wesn_requested, 4, double);	/* So reporting below is accurate */
		/* dx,dy are needed when the grid is pixel-registered as the w/e/s/n bounds are off by 0.5 {dx,dy} relative to node coordinates */
		dx = h->inc[GMT_X] * h->xy_off;	dy = h->inc[GMT_Y] * h->xy_off;

		xlo = outside[XLO] ? (unsigned int)gmt_M_grd_x_to_col (GMT, wesn_old[XLO] + dx, h) : 0;
		xhi = outside[XHI] ? (unsigned int)gmt_M_grd_x_to_col (GMT, wesn_old[XHI] - dx, h) : h->n_columns - 1;
		ylo = outside[YLO] ? (unsigned int)gmt_M_grd_y_to_row (GMT, wesn_old[YLO] + dy, h) : h->n_rows - 1;
		yhi = outside[YHI] ? (unsigned int)gmt_M_grd_y_to_row (GMT, wesn_old[YHI] - dy, h) : 0;
		if (outside[XLO]) {
			for (row = 0; row < h->n_rows; row++)
				for (col = 0; col < xlo; col++) G->data[gmt_M_ijp(h,row,col)] = Ctrl->N.value;
		}
		if (outside[XHI]) {
			for (row = 0; row < h->n_rows; row++)
				for (col = xhi+1; col < h->n_columns; col++) G->data[gmt_M_ijp(h,row,col)] = Ctrl->N.value;
		}
		if (outside[YLO]) {
			for (row = ylo+1; row < h->n_rows; row++)
				for (col = xlo; col <= xhi; col++) G->data[gmt_M_ijp(h,row,col)] = Ctrl->N.value;
		}
		if (outside[YHI]) {
			for (row = 0; row < yhi; row++)
				for (col = xlo; col <= xhi; col++) G->data[gmt_M_ijp(h,row,col)] = Ctrl->N.value;
		}
		n_zero_e = 0;	/* Count zeros in the grid after extension */
		gmt_M_grd_loop (GMT, G, row, col, node) {
			if (G->data[node] == 0.0) n_zero_e++;
		}
		if (n_zero_e != n_zero)
			GMT_Report (API, GMT_MSG_WARNING, "Something went wrong - %d extended nodes not set to NaN\n", n_zero_e-n_zero);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		char format[GMT_BUFSIZ];
		sprintf (format, "\t%s\t%s\t%s\t%s\t%s\t%s\t%%d\t%%d\n", GMT->current.setting.format_float_out,
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
		         GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
		         GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, "File spec:\tW E S N dx dy n_columns n_rows:\n");
		GMT_Report (API, GMT_MSG_INFORMATION, "Old:");
		GMT_Report (API, GMT_MSG_INFORMATION, format, wesn_old[XLO], wesn_old[XHI], wesn_old[YLO],
		            wesn_old[YHI], h->inc[GMT_X], h->inc[GMT_Y], nx_old, ny_old);
		GMT_Report (API, GMT_MSG_INFORMATION, "New:");
		GMT_Report (API, GMT_MSG_INFORMATION, format, wesn_new[XLO], wesn_new[XHI], wesn_new[YLO],
		            wesn_new[YHI], h->inc[GMT_X], h->inc[GMT_Y], h->n_columns, h->n_rows);
	}
	if (geo_to_cart) {
		GMT_Report (API, GMT_MSG_WARNING, "Expanded grid region implies the grid is no longer geographic\n");
		gmt_set_cartesian (GMT, GMT_OUT);
	}

	if (Ctrl->S.set_nan && Ctrl->In.type == GMT_IS_GRID) {	/* Set all nodes outside the circle to NaN */
		unsigned int row, col;
		uint64_t n_nodes = 0;

		for (row = 0; row < h->n_rows; row++) {
			for (col = 0; col < h->n_columns; col++) {
				distance = gmt_distance (GMT, Ctrl->S.lon, Ctrl->S.lat, G->x[col], G->y[row]);
				if (distance > Ctrl->S.radius) {	/* Outside circle */
					node = gmt_M_ijp (h, row, col);
					G->data[node] = GMT->session.f_NaN;
					n_nodes++;
				}
			}
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Set %" PRIu64 " nodes outside circle to NaN\n", n_nodes);
	}

	/* Send the subset of the grid or image to the gridfile destination. */

	if (Ctrl->In.type == GMT_IS_GRID) {	/* Write a grid */
		if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, G))
			Return (API->error);
		if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, G) != GMT_NOERROR)
			Return (API->error);
	}
	else {	/* Write an image */
		if (do_via_gdal) {	/* Special case of multi-band geotiff */
			char driver[GMT_LEN128] = {""}, cmd[GMT_LEN256] = {""},the_band[GMT_LEN32] = {""},  *c = strchr (Ctrl->In.file, '='), *b = strstr (Ctrl->In.file, "+b");
			if (c) c[0] = '\0';	/* Chop off =gd request after that so that we only write the actual file name in the command */
			if (b) b[0] = '\0';	/* Chop offany band request after that so that we only write the actual file name in the command */
			if (wesn_new[XLO] > 180.0) wesn_new[XLO] -= 360.0, wesn_new[XHI] -= 360.0;	/* GDAL expects -180/+180 */
			if (strstr (Ctrl->G.file, ".tif"))
				sprintf (driver, "-of GTiff -co COMPRESS=DEFLATE");
			else
				sprintf (driver, "-of netCDF -co COMPRESS=DEFLATE -co FORMAT=NC4 -co ZLEVEL=%d -a_nodata NaN", GMT->current.setting.io_nc4_deflation_level);
			sprintf (cmd, "gdal_translate -projwin %.10lg %.10lg %.10lg %.10lg %s %s %s", wesn_new[XLO], wesn_new[YHI], wesn_new[XHI], wesn_new[YLO], driver, Ctrl->In.file, Ctrl->G.file);
			if (c) c[0] = '=';	/* Restore full file name */
			if (b) b[0] = '+';	/* Restore band requests */
			if (b) {	/* Parse and add specific band request(s) to gdal_translate */
				char p[GMT_LEN64] = {""};
				unsigned int pos = 0, band;
				while ((gmt_strtok (&b[2], ",", &pos, p))) {
					band = atoi (p) + 1;	/* We start counting at 0, GDAL at 1 */
					sprintf (the_band, " -b %d", band);
					strcat (cmd, the_band);
				}
			}
			GMT_Report (API, GMT_MSG_INFORMATION, "The gdal_translate command: \n%s\n", cmd);
			if (system (cmd)) {	/* Execute the gdal_translate command */
				GMT_Report (API, GMT_MSG_ERROR, "Error calling %s", cmd);
				Return (GMT_RUNTIME_ERROR);
			}
		}
		else if (GMT_Write_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, I) != GMT_NOERROR) {
			Return (API->error);
		}
	}

	Return (GMT_NOERROR);
}
