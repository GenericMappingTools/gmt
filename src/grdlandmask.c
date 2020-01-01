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
 * Brief synopsis: grdlandmask defines a grid based on region and xinc/yinc values,
 * reads a shoreline data base, and sets the grid nodes inside, on the
 * boundary, and outside of the polygons to the user-defined values
 * <in>, <on>, and <out>.  These may be any number, including NaN.
 *
 * Author:	P. Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */
 
#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdlandmask"
#define THIS_MODULE_MODERN_NAME	"grdlandmask"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Create a \"wet-dry\" mask grid from shoreline data base"
#define THIS_MODULE_KEYS	"GG}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-VRr" GMT_ADD_x_OPT GMT_OPT("F")

#define GRDLANDMASK_N_CLASSES	(2*GSHHS_MAX_LEVEL + 1)	/* Number of bands separated by the levels */
/* There are 4 GSHHG levels, but we may find points exactly on the border between levels.
 * To handle these cases we must double the number of levels.  The N.mask[] array will hold the
 * data values we wish to assign to each of these 9 potential cases.  As we determine where a
 * node fits into this hierarchy we assign values 0-9 to the grid.  Then, at the end we replace
 * these values with the corresponding entries in N.mask[].
 */

struct GRDLANDMASK_CTRL {	/* All control options for this program (except common args) */
	/* ctive is true if the option has been activated */
	struct GRDLNDM_A {	/* -A<min_area>[/<min_level>/<max_level>] */
		bool active;
		struct GMT_SHORE_SELECT info;
	} A;
	struct GRDLNDM_D {	/* -D<resolution>[+f] */
		bool active;
		bool force;	/* if true, select next highest level if current set is not available */
		char set;	/* One of f, h, i, l, c, or a for auto */
	} D;
	struct GRDLNDM_E {	/* -E[<border> | <cborder>/<lborder>/<iborder>/<pborder>] */
		bool active;
		bool linetrace;	/* True if -E was given arguments as we must trace the lines through the grid */
		unsigned int inside;	/* if 2, then a point exactly on a polygon boundary is considered OUTSIDE, else 1 */
	} E;
	struct GRDLNDM_G {	/* -G<maskfile> */
		bool active;
		char *file;
	} G;
	struct GRDLNDM_N {	/* -N<maskvalues> */
		bool active;
		unsigned int wetdry;	/* 1 if dry/wet only, 0 if 5 mask levels */
		gmt_grdfloat mask[GRDLANDMASK_N_CLASSES];	/* values for each level */
	} N;
};

struct BINCROSS {
	double x, y, d;
};

GMT_LOCAL int comp_bincross (const void *p1, const void *p2) {
	const struct BINCROSS *a = p1, *b = p2;

	if (a->d < b->d) return (-1);
	if (a->d > b->d) return (+1);
	return (0);
}

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDLANDMASK_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct GRDLANDMASK_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	
	C->A.info.high = GSHHS_MAX_LEVEL;				/* Include all GSHHS levels */
	C->D.set = 'l';							/* Low-resolution coastline data */
	C->E.inside = GMT_ONEDGE;					/* Default is that points on a boundary are inside */
	gmt_M_memset (C->N.mask, GRDLANDMASK_N_CLASSES, gmt_grdfloat);		/* Default "wet" value = 0 */
	C->N.mask[2] = C->N.mask[6] = 1.0f;				/* Default for "dry" areas = 1 (inside) */
	
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDLANDMASK_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s -G<outgrid> %s %s\n", name, GMT_I_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-D<resolution>][+f] [-E[<bordervalues>]]\n\t[-N<maskvalues>] [%s] [%s]%s [%s]\n\n", GMT_A_OPT, GMT_V_OPT, GMT_r_OPT, GMT_x_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for output mask grid file.\n");
	GMT_Option (API, "I,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	gmt_GSHHG_syntax (API->GMT, 'A');
	GMT_Message (API, GMT_TIME_NONE, "\t-D Choose one of the following resolutions:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     a - auto: select best resolution given selected region.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f - full resolution (may be very slow for large regions).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     h - high resolution (may be slow for large regions).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     i - intermediate resolution.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     l - low resolution [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c - crude resolution, for tasks that need crude continent outlines only.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +f to use a lower resolution should the chosen one not be available [abort].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Indicate that nodes exactly on a polygon boundary are outside [inside].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally append <border> or <cborder>/<lborder>/<iborder>/<pborder>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   We will then trace lines through the grid and reset the cells crossed by\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the lines to the indicated values [Default is no line tracing].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This is a new option and is experimental.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Give values to use if a node is outside or inside a feature.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify this information using 1 of 2 formats:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -N<wet>/<dry>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -N<ocean>/<land>/<lake>/<island>/<pond>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   NaN is a valid entry.  Default values are 0/1/0/1/0 (i.e., 0/1).\n");
	GMT_Option (API, "V,r,x.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDLANDMASK_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdlandmask and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, j, pos, n_files = 0;
	char line[GMT_LEN256] = {""}, ptr[GMT_BUFSIZ] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Restrict GSHHS features */
				Ctrl->A.active = true;
				gmt_set_levels (GMT, opt->arg, &Ctrl->A.info);
				break;
			case 'D':	/* Set GSHHS resolution */
				Ctrl->D.active = true;
				Ctrl->D.set = opt->arg[0];
				Ctrl->D.force = (opt->arg[1] == '+' && (opt->arg[2] == 'f' || opt->arg[2] == '\0'));
				break;
			case 'E':	/* On-boundary setting */
				Ctrl->E.active = true;
				if (opt->arg[0]) {	/* Trace lines through grid */
					GMT_Report (API, GMT_MSG_VERBOSE, "-E<values> is presently being tested and is considered experimental\n");
					Ctrl->E.linetrace = true;
					j = pos = 0;
					strncpy (line, opt->arg,  GMT_LEN256-1);
					while (j < 4 && (gmt_strtok (line, "/", &pos, ptr))) {
						Ctrl->N.mask[2*j+1] = (ptr[0] == 'N' || ptr[0] == 'n') ? GMT->session.f_NaN : (gmt_grdfloat)atof (ptr);
						j++;
					}
					if (!(j == 1 || j == 4)) {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -E option: Specify 1 or 4 border values\n");
						n_errors++;
					}
					if (j == 1) Ctrl->N.mask[3] = Ctrl->N.mask[5] = Ctrl->N.mask[7] = Ctrl->N.mask[1];	/* Duplicate border values */
				}
				else
					Ctrl->E.inside = GMT_INSIDE;
				break;
			case 'G':	/* Output filename */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)))
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':	/* Grid spacings */
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'N':	/* Mask values */
				Ctrl->N.active = true;
				strncpy (line, opt->arg,  GMT_LEN256);
				if (line[strlen(line)-1] == 'o' && gmt_M_compat_check (GMT, 4)) { /* Edge is considered outside */
					GMT_Report (API, GMT_MSG_COMPAT, "Option -N...o is deprecated; use -E instead\n");
					Ctrl->E.active = true;
					Ctrl->E.inside = GMT_INSIDE;
					line[strlen(line)-1] = 0;
				}
				j = pos = 0;
				while (j < 5 && (gmt_strtok (line, "/", &pos, ptr))) {
					Ctrl->N.mask[2*j] = (ptr[0] == 'N' || ptr[0] == 'n') ? GMT->session.f_NaN : (gmt_grdfloat)atof (ptr);
					j++;
				}
				if (!(j == 2 || j == 5)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N option: Specify 2 or 5 mask values\n");
					n_errors++;
				}
				Ctrl->N.wetdry = (j == 2);
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify an output file\n");
	n_errors += gmt_M_check_condition (GMT, n_files, "Syntax error: No input files allowed.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL bool inside (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *C, double x, double y) {
	/* Return true if this point is inside the grid */
	int row, col;
	gmt_M_unused(GMT);
	row = gmt_M_grd_y_to_row (GMT, y, C);
	if (row < 0 || row > (int)C->n_rows) return false;
	col = gmt_M_grd_x_to_col (GMT, x, C);
	if (col < 0 || col > (int)C->n_columns) return false;
	return true;
}

GMT_LOCAL void assign_node (struct GMT_CTRL *GMT, struct GMT_GRID *G, struct GMT_GRID_HEADER *C, double x, double y, gmt_grdfloat f_level, int64_t *ij) {
	/* Set node value and return node index if this point is inside the grid */
	int row, col;
	gmt_M_unused(GMT);
	row = gmt_M_grd_y_to_row (GMT, y, C);
	if (row < 0 || row > (int)C->n_rows) return;
	col = gmt_M_grd_x_to_col (GMT, x, C);
	if (col < 0 || col > (int)C->n_columns) return;
	*ij = gmt_M_ijp (G->header, row, col);
	G->data[*ij] = f_level;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdlandmask (void *V_API, int mode, void *args) {
	bool temp_shift = false, wrap, used_polygons, double_dip;
	unsigned int base = 3, k, bin, np, side, np_new;
	int row, row_min, row_max, ii, col, col_min, col_max, i, direction, err, ind, nx1, ny1, error = 0;
	
	int64_t ij;
	uint64_t count[GRDLANDMASK_N_CLASSES];
	
	size_t nx_alloc = GMT_SMALL_CHUNK;

	char line[GMT_LEN256] = {""};
	char *shore_resolution[5] = {"full", "high", "intermediate", "low", "crude"};

	double xmin, xmax, ymin, ymax, west_border, east_border, i_dx_inch, i_dy_inch, inc_inch[2];
	double dummy, *x = NULL, *y = NULL;
	
	gmt_grdfloat f_level = 0.0f;

	struct GMT_SHORE c;
	struct GMT_GRID *Grid = NULL, *Cart = NULL;
	struct GMT_GRID_HEADER *C = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_GSHHS_POL *p = NULL;
	struct BINCROSS *X = NULL;
	struct GRDLANDMASK_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
    if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdlandmask main code ----------------------------*/

	/* We know coastline data are geographic so we hardwire this here so parsing of -R works: */
	gmt_set_geographic (GMT, GMT_IN);
	gmt_enable_threads (GMT);	/* Set number of active threads, if supported */

	/* Create the empty grid and allocate space */
	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
	
	if (Grid->header->wesn[XLO] < 0.0 && Grid->header->wesn[XHI] < 0.0) {	/* Shift longitudes */
		temp_shift = true;
		Grid->header->wesn[XLO] += 360.0;
		Grid->header->wesn[XHI] += 360.0;
	}
	HH = gmt_get_H_hidden (Grid->header);
	
	if (Ctrl->D.force) Ctrl->D.set = gmt_shore_adjust_res (GMT, Ctrl->D.set);
	base = gmt_set_resolution (GMT, &Ctrl->D.set, 'D');
	gmt_M_memset (count, GRDLANDMASK_N_CLASSES, uint64_t);		/* Counts of each level */
	
	if (Ctrl->N.wetdry) {	/* Must duplicate wet/dry settings */
		Ctrl->N.mask[6] = Ctrl->N.mask[2];
		Ctrl->N.mask[4] = Ctrl->N.mask[8] = Ctrl->N.mask[0];
	}

	if ((err = gmt_init_shore (GMT, Ctrl->D.set, &c, Grid->header->wesn, &Ctrl->A.info))) {
		GMT_Report (API, GMT_MSG_NORMAL, "%s [GSHHG %s resolution shorelines]\n", GMT_strerror(err), shore_resolution[base]);
		Return (GMT_RUNTIME_ERROR);
	}
	if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE)) {
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "GSHHG version %s\n", c.version);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%s\n", c.title);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%s\n", c.source);

		sprintf (line, "%s\n", GMT->current.setting.format_float_out);
		if (Ctrl->N.wetdry) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes in water will be set to ");
			(gmt_M_is_fnan (Ctrl->N.mask[0])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[0]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes on land will be set to ");
			(gmt_M_is_fnan (Ctrl->N.mask[2])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[2]);
		}
		else {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes in the oceans will be set to ");
			(gmt_M_is_fnan (Ctrl->N.mask[0])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[0]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes on land will be set to ");
			(gmt_M_is_fnan (Ctrl->N.mask[2])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[2]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes in lakes will be set to ");
			(gmt_M_is_fnan (Ctrl->N.mask[4])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[4]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes in islands will be set to ");
			(gmt_M_is_fnan (Ctrl->N.mask[6])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[6]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes in ponds will be set to ");
			(gmt_M_is_fnan (Ctrl->N.mask[8])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[8]);
		}
		if (Ctrl->E.linetrace) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes near shoreline will be set to ");
			(gmt_M_is_fnan (Ctrl->N.mask[1])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[1]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes near lakeline will be set to ");
			(gmt_M_is_fnan (Ctrl->N.mask[3])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[3]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes near islandline will be set to ");
			(gmt_M_is_fnan (Ctrl->N.mask[5])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[5]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes near pondline will be set to ");
			(gmt_M_is_fnan (Ctrl->N.mask[7])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[7]);
		}
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Apply linear scale to avoid wrap-around: -Jx100id; this temporarily changes the domain\n");
	gmt_parse_common_options (GMT, "J", 'J', "x100id");	/* Fake linear projection so the shore machinery will work. Used bo be -Jx1id but had trouble with roundoff */
	if (gmt_M_err_pass (GMT, gmt_proj_setup (GMT, Grid->header->wesn), "")) Return (GMT_PROJECTION_ERROR);
	GMT->current.map.parallel_straight = GMT->current.map.meridian_straight = 2;	/* No resampling along bin boundaries */
	wrap = GMT->current.map.is_world = gmt_grd_is_global (GMT, Grid->header);
	double_dip = (wrap && Grid->header->registration == GMT_GRID_NODE_REG);	/* Must duplicate the west nodes to east */
	/* Using -Jx1d means output is Cartesian but we want to force geographic */
	gmt_set_geographic (GMT, GMT_OUT);
	/* All data nodes are thus initialized to 0 */
	x = gmt_M_memory (GMT, NULL, Grid->header->n_columns, double);
	y = gmt_M_memory (GMT, NULL, Grid->header->n_rows, double);
	if (Ctrl->E.linetrace)
		X = gmt_M_memory (GMT, X, nx_alloc, struct BINCROSS);

	nx1 = Grid->header->n_columns - 1;	ny1 = Grid->header->n_rows - 1;

	/* Fill out gridnode coordinates and apply the implicit linear projection */

	for (col = 0; col <= nx1; col++) gmt_geo_to_xy (GMT, gmt_M_grd_col_to_x (GMT, col, Grid->header), 0.0, &x[col], &dummy);
	for (row = 0; row <= ny1; row++) gmt_geo_to_xy (GMT, 0.0, gmt_M_grd_row_to_y (GMT, row, Grid->header), &dummy, &y[row]);
	inc_inch[GMT_X] = fabs (x[1] - x[0]);
	inc_inch[GMT_Y] = fabs (y[1] - y[0]);
	i_dx_inch = 1.0 / inc_inch[GMT_X];
	i_dy_inch = 1.0 / inc_inch[GMT_Y];

	if ((Cart = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, GMT->current.proj.rect, inc_inch, \
		Grid->header->registration, GMT_NOTSET, NULL)) == NULL) Return (API->error);
	C = Cart->header;
	
	west_border = floor (GMT->common.R.wesn[XLO] / c.bsize) * c.bsize;
	east_border =  ceil (GMT->common.R.wesn[XHI] / c.bsize) * c.bsize;
	
	for (ind = 0; ind < c.nb; ind++) {	/* Loop over necessary bins only */

		bin = c.bins[ind];
		GMT_Report (API, GMT_MSG_DEBUG, "Working on block # %5ld\n", bin);

		if ((err = gmt_get_shore_bin (GMT, ind, &c))) {
			GMT_Report (API, GMT_MSG_NORMAL, "%s [%s resolution shoreline]\n", GMT_strerror(err), shore_resolution[base]);
			Return (GMT_RUNTIME_ERROR);
		}

		/* Use polygons, if any.  Go in both directions to cover both land and sea */

		used_polygons = false;

		for (direction = -1; c.ns > 0 && direction < 2; direction += 2) {

			/* Assemble one or more segments into polygons */

			np = gmt_assemble_shore (GMT, &c, direction, true, west_border, east_border, &p);

			/* Get clipped polygons in x,y inches that can be processed */

			np_new = gmt_prep_shore_polygons (GMT, &p, np, false, 0.0, -1);

			for (k = 0; k < np_new; k++) {

				if (p[k].n == 0) continue;

				used_polygons = true;	/* At least some points made it to here */

				/* Find min/max of polygon in inches */

				xmin = xmax = p[k].lon[0];
				ymin = ymax = p[k].lat[0];
				for (i = 1; i < p[k].n; i++) {
					if (p[k].lon[i] < xmin) xmin = p[k].lon[i];
					if (p[k].lon[i] > xmax) xmax = p[k].lon[i];
					if (p[k].lat[i] < ymin) ymin = p[k].lat[i];
					if (p[k].lat[i] > ymax) ymax = p[k].lat[i];
				}
				col_min = MAX (0, irint (ceil (xmin * i_dx_inch - Grid->header->xy_off - GMT_CONV8_LIMIT)));
				if (col_min > nx1) col_min = 0;
				/* So col_min is in range [0,nx1] */
				col_max = MIN (nx1, irint (floor (xmax * i_dx_inch - Grid->header->xy_off + GMT_CONV8_LIMIT)));
				if (col_max <= 0 || col_max < col_min) col_max = nx1;
				/* So col_max is in range [1,nx1] */
				row_min = MAX (0, irint (ceil ((GMT->current.proj.rect[YHI] - ymax) * i_dy_inch - Grid->header->xy_off - GMT_CONV8_LIMIT)));
				/* So row_min is in range [0,?] */
				row_max = MIN (ny1, irint (floor ((GMT->current.proj.rect[YHI] - ymin) * i_dy_inch - Grid->header->xy_off + GMT_CONV8_LIMIT)));
				/* So row_max is in range [?,ny1] */
				f_level = (gmt_grdfloat)(2*p[k].level);

#ifdef _OPENMP
#pragma omp parallel for private(row,col,side,ij) shared(row_min,row_max,col_min,col_max,GMT,x,y,p,k,Ctrl,Grid,f_level)
#endif
				for (row = row_min; row <= row_max; row++) {
					assert (row >= 0);	/* Just in case we have a logic bug somewhere */
					for (col = col_min; col <= col_max; col++) {

						if ((side = gmt_non_zero_winding (GMT, x[col], y[row], p[k].lon, p[k].lat, p[k].n)) < Ctrl->E.inside) continue;	/* Outside */

						/* Here, point is inside, we must assign value */

						ij = gmt_M_ijp (Grid->header, row, col);
						if (f_level > Grid->data[ij]) Grid->data[ij] = f_level;
					}
				}
			}

			gmt_free_shore_polygons (GMT, p, np_new);
			gmt_M_free (GMT, p);

			if (Ctrl->E.linetrace) {	/* Deal with tracing the lines and resetting gridnodes */
				int64_t last_ij;
				uint64_t nx;
				unsigned int curr_x_pt, prev_x_pt, pt;
				int last_col, last_row, start_col, end_col, brow, bcol;
				double dx, del_x, del_y, xc, yc, xb, yb;
				bool last_not_set;
				
				if ((np = gmt_assemble_shore (GMT, &c, 1, false, west_border, east_border, &p)) == 0) {	/* Just get segments */
					gmt_free_shore (GMT, &c);
					continue;
				}
				for (k = 0; k < np; k++) {
					if (p[k].n == 0) continue;
					for (pt = 0; pt < (unsigned int)p[k].n; pt++) {	/* Convert geo coordinates to inches */
						gmt_geo_to_xy (GMT, p[k].lon[pt], p[k].lat[pt], &xc, &yc);
						p[k].lon[pt] = xc;		p[k].lat[pt] = yc;
					}
					f_level = (gmt_grdfloat)(2*p[k].level-1);	/* Border index is half-way between the two levels */
					last_ij = UINT_MAX;
					last_not_set = true;
					last_col = last_row = -1;

					/* To handle lines that exit the grid we pursue the entire line even if outside.
					 * We only check if (row,col) is inside when filling in between points and assigning nodes */
					
					for (pt = 0; pt < (unsigned int)p[k].n; pt++) {
						/* Get (row,col) and index to nearest node for this point */
						row = gmt_M_grd_y_to_row (GMT, p[k].lat[pt], C);
						col = gmt_M_grd_x_to_col (GMT, p[k].lon[pt], C);
						ij = gmt_M_ijp (Grid->header, row, col);

						if (last_not_set) {	/* Initialize last node to this node the first time */
							last_ij = ij;
							last_not_set = false;
						}

						if (ij != last_ij) {	/* Crossed into another cell */
							start_col = MIN (last_col, col) + 1;
							end_col = MAX (last_col, col);
							dx = p[k].lon[pt] - p[k].lon[pt-1];

							/* Find all the bin-line intersections - modified from x2sys_binlist */

							/* Add the start and stop coordinates to the xc/yc arrays so we can get mid-points of each interval */

							nx = 0;
							if (inside (GMT, C, last_row, last_col)) {	/* Previous point is inside, add it to our list with zero distance */
								X[nx].x = p[k].lon[pt-1];	X[nx].y = p[k].lat[pt-1];	X[nx++].d = 0.0;
							}
							if (inside (GMT, C, row, col)) {	/* This point is inside, add it to our list with max distance */
								X[nx].x = p[k].lon[pt];	X[nx].y = p[k].lat[pt];	X[nx++].d = hypot (dx, p[k].lat[pt] - p[k].lat[pt-1]);
							}
							
							/* Now add all crossings between this line segment and the gridlines outlining the cells centered on the nodes */
							
							for (brow = MIN (last_row, row) + 1; brow <= MAX (last_row, row); brow++) {	/* If we go in here we know dy is non-zero */
								if (brow < 0 || brow > (int)C->n_rows) continue;	/* Outside grid */
								/* Determine intersections between the line segment and parallels */
								yb = gmt_M_grd_row_to_y (GMT, brow, C) + 0.5 * inc_inch[GMT_Y];
								del_y = yb - p[k].lat[pt-1];
								del_x = del_y * dx / (p[k].lat[pt] - p[k].lat[pt-1]);
								xc = p[k].lon[pt-1] + del_x;
								X[nx].x = xc;	X[nx].y = yb;	X[nx].d = hypot (del_x , del_y);
								nx++;
								if (nx == nx_alloc) {
									nx_alloc <<= 1;
									X = gmt_M_memory (GMT, X, nx_alloc, struct BINCROSS);
								}
							}
							for (bcol = start_col; bcol <= end_col; bcol++) {	/* If we go in here we think dx is non-zero (we do a last-ditch dx check just in case) */
								if (bcol < 0 || bcol > (int)C->n_columns) continue;
								/* Determine intersections between the line segment and meridians */
								xb = gmt_M_grd_col_to_x (GMT, bcol, C) - 0.5 * inc_inch[GMT_X];
								del_x = xb - p[k].lon[pt-1];
								del_y = (dx == 0.0) ? 0.5 * (p[k].lat[pt] - p[k].lat[pt-1]) : del_x * (p[k].lat[pt] - p[k].lat[pt-1]) / dx;
								yc = p[k].lat[pt-1] + del_y;
								X[nx].x = xb;	X[nx].y = yc;	X[nx].d = hypot (del_x, del_y);
								nx++;
								if (nx == nx_alloc) {
									nx_alloc <<= 1;
									X = gmt_M_memory (GMT, X, nx_alloc, struct BINCROSS);
								}
							}

							if (nx) {	/* Here we have intersections */
								qsort (X, nx, sizeof (struct BINCROSS), comp_bincross);	/* Sort on distances along the line segment */
								assign_node (GMT, Grid, C, X[0].x, X[0].y, f_level, &ij);	/* Possibly assign f_level to nearest node */
								for (curr_x_pt = 1, prev_x_pt = 0; curr_x_pt < nx; curr_x_pt++, prev_x_pt++) {	/* Process the intervals, getting mid-points and using that to get bin */
									assign_node (GMT, Grid, C, X[curr_x_pt].x, X[curr_x_pt].y, f_level, &ij);	/* Possibly assign f_level to nearest node */
									/* Add mid-point between the cuts */
									xc = 0.5 * (X[curr_x_pt].x + X[prev_x_pt].x);
									yc = 0.5 * (X[curr_x_pt].y + X[prev_x_pt].y);
									assign_node (GMT, Grid, C, xc, yc, f_level, &ij);	/* Possibly assign f_level to nearest node */
								}
							}
						}
						/* Update last row/col and index */
						last_ij = ij;
						last_col = col;
						last_row = row;
					}

				}
				gmt_free_shore_polygons (GMT, p, np);
				gmt_M_free (GMT, p);
			}
		}

		if (!used_polygons) {	/* Lack of polygons or clipping etc resulted in no polygons after all, must deal with background */
			k = INT_MAX;	/* Initialize to outside range of levels (4 is highest) */
			/* Visit each of the 4 nodes, test if it is inside -R, and if so update lowest level found so far */

			if (!gmt_map_outside (GMT, c.lon_sw, c.lat_sw)) k = MIN (k, c.node_level[0]);				/* SW */
			if (!gmt_map_outside (GMT, c.lon_sw + c.bsize, c.lat_sw)) k = MIN (k, c.node_level[1]);			/* SE */
			if (!gmt_map_outside (GMT, c.lon_sw + c.bsize, c.lat_sw - c.bsize)) k = MIN (k, c.node_level[2]);	/* NE */
			if (!gmt_map_outside (GMT, c.lon_sw, c.lat_sw - c.bsize)) k = MIN (k, c.node_level[3]);			/* NW */

			/* If k is still INT_MAX we must assume this patch should have the min level of the bin */

			if (k == INT_MAX) k = MIN (MIN (c.node_level[0], c.node_level[1]) , MIN (c.node_level[2], c.node_level[3]));
			f_level = (gmt_grdfloat)(2*k);

			/* Determine nodes to initialize */

			row_min = MAX (0, irint (ceil ((Grid->header->wesn[YHI] - c.lat_sw - c.bsize) * HH->r_inc[GMT_Y] - Grid->header->xy_off)));
			row_max = MIN (ny1, irint (floor ((Grid->header->wesn[YHI] - c.lat_sw) * HH->r_inc[GMT_Y] - Grid->header->xy_off)));
			if (wrap) {	/* Handle jumps */
				col_min = irint (ceil (fmod (c.lon_sw - Grid->header->wesn[XLO], 360.0) * HH->r_inc[GMT_X] - Grid->header->xy_off));
				col_max = irint (floor (fmod (c.lon_sw + c.bsize - Grid->header->wesn[XLO], 360.0) * HH->r_inc[GMT_X] - Grid->header->xy_off));
				if (col_max < col_min) col_max += Grid->header->n_columns;
			}
			else {	/* Make sure we are inside our grid */
				double lon_w, lon_e;
				lon_w = c.lon_sw - Grid->header->wesn[XLO];	lon_e = c.lon_sw + c.bsize - Grid->header->wesn[XLO];
				if (lon_w < Grid->header->wesn[XLO] && (lon_w+360.0) < Grid->header->wesn[XHI]) {
					lon_w += 360.0;	lon_e += 360.0;
				}
				else if (lon_e > Grid->header->wesn[XHI] && (lon_e-360.0) > Grid->header->wesn[XLO]) {
					lon_w -= 360.0;	lon_e -= 360.0;
				}
				col_min = irint (ceil (lon_w * HH->r_inc[GMT_X] - Grid->header->xy_off));
				col_max = irint (floor (lon_e * HH->r_inc[GMT_X] - Grid->header->xy_off));
				if (col_min < 0) col_min = 0;
				if (col_max > nx1) col_max = nx1;
			}
#ifdef _OPENMP
#pragma omp parallel for private(row,col,ii,ij) shared(row_min,row_max,col_min,col_max,wrap,nx1,Grid,f_level)
#endif
			for (row = row_min; row <= row_max; row++) {
				for (col = col_min; col <= col_max; col++) {
					ii = (wrap) ? col % (int)Grid->header->n_columns : col;
					if (ii < 0 || ii > nx1) continue;
					ij = gmt_M_ijp (Grid->header, row, ii);
					Grid->data[ij] = f_level;
				}
			}
		}

		gmt_free_shore (GMT, &c);
	}

	gmt_shore_cleanup (GMT, &c);
	gmt_M_free (GMT, x);
	gmt_M_free (GMT, y);
	if (Ctrl->E.linetrace) gmt_M_free (GMT, X);
	if (GMT_Destroy_Data (API, &Cart) != GMT_NOERROR) {
		Return (API->error);
	}

#ifdef _OPENMP
#pragma omp parallel for private(row,col,k,ij) shared(GMT,Grid,Ctrl)
#endif
	
	gmt_M_grd_loop (GMT, Grid, row, col, ij) {	/* Turn levels into mask values */
		k = urint (Grid->data[ij]);
		Grid->data[ij] = Ctrl->N.mask[k];
		count[k]++;
		if (col == 0 && double_dip) count[k]++;	/* Count these guys twice */
	}

	if (double_dip) { /* Copy over values to the repeating right column */
		unsigned int row_l;
		for (row_l = 0, ij = gmt_M_ijp (Grid->header, row_l, 0); row_l < Grid->header->n_rows; row_l++, ij += Grid->header->mx) Grid->data[ij+nx1] = Grid->data[ij];
	}
	
	if (temp_shift) {
		Grid->header->wesn[XLO] -= 360.0;
		Grid->header->wesn[XHI] -= 360.0;
	}

	sprintf (line, "Derived from the %s resolution shorelines", shore_resolution[base]);
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, line, Grid)) return (API->error);
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
		Return (API->error);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		for (k = 0; k < GRDLANDMASK_N_CLASSES; k++) {
			if (count[k] == 0) continue;
			if (k%2 == 0)
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Level %d set for %" PRIu64 " nodes\n", k/2, count[k]);
			else
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Border between Levels %d-%d set for %" PRIu64 " nodes\n", (k-1)/2, (k+1)/2, count[k]);
		}
	}

	Return (GMT_NOERROR);
}
