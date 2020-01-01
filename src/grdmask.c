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
 * Brief synopsis: grdmask defines a grid based on region and xinc/yinc values,
 * reads xy polygon files, and sets the grid nodes inside, on the
 * boundary, and outside of the polygons to the user-defined values
 * <in>, <on>, and <out>.  These may be any number, including NaN.
 *
 * Author:	Walter. H. F. Smith
 * Date:	1-JAN-2010
 * Version:	6 API
 */
 
#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdmask"
#define THIS_MODULE_MODERN_NAME	"grdmask"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Create mask grid from polygons or point coverage"
#define THIS_MODULE_KEYS	"<D{,GG}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVabdefghijnqrs" GMT_ADD_x_OPT GMT_OPT("FHMm")

#define GRDMASK_N_CLASSES	3	/* outside, on edge, and inside */
#define GRDMASK_N_CART_MASK	9

struct GRDMASK_CTRL {
	struct GRDMASK_A {	/* -A[m|p|x|y|step] */
		bool active;
		unsigned int mode;
		double step;
	} A;
	struct GRDMASK_G {	/* -G<maskfile> */
		bool active;
		char *file;
	} G;
	struct GRDMASK_N {	/* -N<maskvalues> */
		bool active;
		unsigned int mode;	/* 0 for out/on/in, 1 for polygon ID inside, 2 for polygon ID inside+path */
		double mask[GRDMASK_N_CLASSES];	/* values for each level */
	} N;
	struct GRDMASK_S {	/* -S[-|=|+]<radius|z>[d|e|f|k|m|M|n] */
		bool active;
		bool variable_radius;	/* true when radii is read in on a per-record basis [false] */
		int mode;	/* Could be negative */
		double radius;
		double limit[2];
		char unit;
	} S;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDMASK_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct GRDMASK_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->N.mask[GMT_INSIDE] = 1.0;	/* Default inside value */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDMASK_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -G<outgrid> %s\n", name, GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-A[m|p|x|y]] [-N[z|Z|p|P][<values>]] [-S%s | <xlim>/<ylim>]\n", GMT_Rgeo_OPT, GMT_RADIUS_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]%s[%s] [%s]\n\n",
		GMT_V_OPT, GMT_a_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT,
		GMT_h_OPT, GMT_i_OPT, GMT_j_OPT, GMT_n_OPT, GMT_qi_OPT, GMT_r_OPT, GMT_s_OPT, GMT_x_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for output mask grid file.\n");
	GMT_Option (API, "I,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Suppress connecting geographic points using great circle arcs, i.e., connect by straight lines,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   unless m or p is appended to first follow meridian then parallel, or vice versa.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For Cartesian data, use -Ax or -Ay to connect first in x, then y, or vice versa.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Ignored if -S is used since input data are then considered to be points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set <out>/<edge>/<in> to use if node is outside, on the path, or inside.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   NaN is a valid entry.  Default values are 0/0/1.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, use -Nz (inside) or -NZ (inside & edge) to set the inside\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   nodes of a polygon to a z-value obtained as follows (in this order):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     a) If OGR/GMT files, get z-value via -aZ=<name> for attribute <name>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     b) Interpret segment z-values (-Z<zval>) as the z-value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c) Interpret segment labels (-L<label>) as the z-value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Finally, use -Np|P and append origin for running polygon IDs [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For -Nz|Z|p|P you may optionally append /<out [0].\n");
	gmt_dist_syntax (API->GMT, 'S', "Set search radius to identify inside points.");
	GMT_Message (API, GMT_TIME_NONE, "\t   Mask nodes are set to <in> or <out> depending on whether they are\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   inside the circle of specified radius [0] from the nearest data point.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give radius as 'z' if individual radii are provided via the 3rd data column\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and append a fixed unit unless Cartesian.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For Cartesian grids with different x and y units you may append <xlim>/<ylim>;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   this sets all nodes within the rectangular area of the given half-widths to inside.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   One can also achieve the rectangular selection effect by using the -S<n_cells>c\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   form. Here n_cells means the number of cells around each data point. As an example,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -S0c means that only the cell where point lies is masked, -S1c masks one cell\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   beyond that (i.e. makes a 3x3 neighborhood), and so on.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is to assume <table> contains polygons and use inside/outside searching].\n");
	GMT_Option (API, "V,a,bi2,di,e,f,g,h,i,j");
	if (gmt_M_showusage (API)) {
		GMT_Message (API, GMT_TIME_NONE, "\t-n+b<BC> Set boundary conditions.  <BC> can be either:\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   g for geographic, p for periodic, and n for natural boundary conditions.\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   For p and n you may optionally append x or y [default is both]:\n");
		GMT_Message (API, GMT_TIME_NONE, "\t     x applies the boundary condition for x only\n");
		GMT_Message (API, GMT_TIME_NONE, "\t     y applies the boundary condition for y only\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   [Default: Natural conditions, unless grid is geographic].\n");
	}
	GMT_Option (API, "qi,r,s,x,:,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL double grdmask_assign (struct GMT_CTRL *GMT, char *p) {
	/* Handle the parsing of NaN|<value> */
	double value = (p[0] == 'N' || p[0] == 'n') ? GMT->session.d_NaN : atof (p);
	return (value);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDMASK_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdmask and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, j, pos;
	char ptr[PATH_MAX] = {""}, *c = NULL, *S_copy = NULL;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Turn off draw_arc mode */
				Ctrl->A.active = true;
				switch (opt->arg[0]) {
					case 'm': case 'y': Ctrl->A.mode = GMT_STAIRS_Y; break;
					case 'p': case 'x': Ctrl->A.mode = GMT_STAIRS_X; break;
#ifdef DEBUG
					default: Ctrl->A.step = atof (opt->arg); break; /* Undocumented test feature; requires step in degrees */
#endif
				}
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
				switch (opt->arg[0]) {
					case 'z':	/* Z-value from file (inside only) */
						Ctrl->N.mode = 1;
						if (opt->arg[1] == '/' && opt->arg[2]) Ctrl->N.mask[GMT_OUTSIDE] = grdmask_assign (GMT, &opt->arg[2]);	/* Change the outside value */
						break;
					case 'Z':	/* Z-value from file (inside + edge) */
						Ctrl->N.mode = 2;
						if (opt->arg[1] == '/' && opt->arg[2]) Ctrl->N.mask[GMT_OUTSIDE] = grdmask_assign (GMT, &opt->arg[2]);	/* Change the outside value */
						break;
					case 'p':	/* Polygon ID from running number (inside only) */
						Ctrl->N.mode = 3;
						Ctrl->N.mask[GMT_INSIDE] = (opt->arg[1]) ? atof (&opt->arg[1]) - 1.0 : -1.0;	/* Running ID start [0] (we increment first) */
						if ((c = strchr (opt->arg, '/')) && c[1]) Ctrl->N.mask[GMT_OUTSIDE] = grdmask_assign (GMT, &c[1]);	/* Change the outside value */
						break;
					case 'P':	/* Polygon ID from running number (inside + edge) */
						Ctrl->N.mode = 4;
						Ctrl->N.mask[GMT_INSIDE] = (opt->arg[1]) ? atof (&opt->arg[1]) - 1.0 : -1.0;	/* Running ID start [0] (we increment first) */
						if ((c = strchr (opt->arg, '/')) && c[1]) Ctrl->N.mask[GMT_OUTSIDE] = grdmask_assign (GMT, &c[1]);	/* Change the outside value */
						break;
					default:	/* Standard out/on/in constant values */
						j = pos = 0;
						while (j < GRDMASK_N_CLASSES && (gmt_strtok (opt->arg, "/", &pos, ptr))) {
							Ctrl->N.mask[j] = grdmask_assign (GMT, ptr);
							j++;
						}
						break;
				}
				break;
			case 'S':	/* Search radius */
				Ctrl->S.active = true;
				if ((c = strchr (opt->arg, 'z'))) {	/* Gave -S[-|=|+]z[d|e|f|k|m|M|n] which means read radii from file */
					c[0] = '0';	/* Replace the z with 0 temporarily */
					Ctrl->S.mode = gmt_get_distance (GMT, opt->arg, &(Ctrl->S.radius), &(Ctrl->S.unit));
					Ctrl->S.variable_radius = true;
					c[0] = 'z';	/* Restore v */
				}
				else if (strchr (opt->arg, '/')) {	/* Gave -S<xlim>/<ylim> for Cartesian masking instead */
					if (sscanf (opt->arg, "%lg/%lg", &Ctrl->S.limit[GMT_X], &Ctrl->S.limit[GMT_Y]) != 2) {
						n_errors++;
					}
					if (Ctrl->S.limit[GMT_Y] == 0.0) Ctrl->S.limit[GMT_Y] = Ctrl->S.limit[GMT_X];
					Ctrl->S.mode = GRDMASK_N_CART_MASK;
				}
				else {		/* Gave -S[-|=|+]<radius>[d|e|f|k|m|M|n|c] which means radius is fixed or 0 */ 
					if (opt->arg[strlen(opt->arg)-1] == 'c') { 	/* A n of cells request for radius. The problem is that */
						if (S_copy) free (S_copy);
						S_copy = strdup (opt->arg);		/* we can't process it yet because we need -I. So, delay it */
					}
					else
						Ctrl->S.mode = gmt_get_distance (GMT, opt->arg, &(Ctrl->S.radius), &(Ctrl->S.unit));
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (S_copy) {	/* OK, time to process a -S<n_cells>c option. We had to wait till be sure that the -I option was parsed */
		char txt[64] = {""};
		int n_cells;
		S_copy[strlen(S_copy)-1] = '\0';		/* Drop the 'c' */
		n_cells = atoi(S_copy) + 1;				/* + 1 so that 0 means cell with point only */
		sprintf(txt, "%.12g/%.12g", n_cells * GMT->common.R.inc[GMT_X], n_cells * GMT->common.R.inc[GMT_Y]);
		Ctrl->S.mode = gmt_get_distance (GMT, txt, &(Ctrl->S.radius), &(Ctrl->S.unit));
		free (S_copy);
	}

	if (Ctrl->S.mode && Ctrl->S.mode != GRDMASK_N_CART_MASK && gmt_M_is_cartesian (GMT, GMT_IN))	/* Gave a geographic search radius but not -fg so do that automatically */
		gmt_parse_common_options (GMT, "f", 'f', "g");
		
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0,
	                                        "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -1, "Syntax error -S: Unrecognized unit\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -2, "Syntax error -S: Unable to decode radius\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -3, "Syntax error -S: Radius is negative\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == GRDMASK_N_CART_MASK && (Ctrl->S.limit[GMT_X] <= 0.0 ||
	                                        Ctrl->S.limit[GMT_Y] <= 0.0), "Syntax error -S: x-limit or y-limit is negative\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && Ctrl->N.mode, "Syntax error -S, -N: Cannot specify -Nz|Z|p|P for points\n");
	n_errors += gmt_check_binary_io (GMT, 2);
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdmask (void *V_API, int mode, void *args) {
	bool periodic = false, periodic_grid = false, do_test = true;
	bool wrap_180, replicate_x, replicate_y, worry_about_jumps;
	unsigned int side = 0, known_side, *d_col = NULL, d_row = 0;
	unsigned int tbl, gmode, n_pol = 0, max_d_col = 0, n_cols = 2, rowu, colu, x_wrap, y_wrap;
	int row, col, row_end, col_end, ii, jj, n_columns, n_rows, error = 0, col_0, row_0;
	
	uint64_t ij, k, seg;
	
	char text_item[GMT_LEN64] = {""};

	gmt_grdfloat mask_val[3], value;
	
	double distance, xx, yy, z_value, xtmp, radius = 0.0, last_radius = -DBL_MAX, *grd_x0 = NULL, *grd_y0 = NULL;

	struct GMT_GRID *Grid = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_DATASET *Din = NULL, *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_DATASET_HIDDEN *DH = NULL;
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	struct GRDMASK_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the grdmask main code ----------------------------*/

	gmt_enable_threads (GMT);	/* Set number of active threads, if supported */
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input table data\n");
	/* Create the empty grid and allocate space */
	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
	
	for (k = 0; k < 3; k++) mask_val[k] = (gmt_grdfloat)Ctrl->N.mask[k];	/* Copy over the mask values for perimeter polygons */
	z_value = Ctrl->N.mask[GMT_INSIDE];	/* Starting value if using running IDs */
	HH = gmt_get_H_hidden (Grid->header);

	if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE)) {
		char line[GMT_BUFSIZ] = {""}, *msg[2] = {"polygons", "search radius"};
		k = (Ctrl->S.active) ? 1 : 0; 
		if (Ctrl->N.mode == 1) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes completely inside the polygons will be set to the chosen z-value\n");
		}
		else if (Ctrl->N.mode == 2) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes completely inside the polygons or on the edge will be set to the chosen z-value\n");
		}
		else if (Ctrl->N.mode == 3) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes completely inside the polygons will be set to a polygon ID starting at %ld\n", lrint (z_value + 1.0));
		}
		else if (Ctrl->N.mode == 4) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes completely inside the polygons or on the edge will be set to a polygon ID starting at %ld\n", lrint (z_value + 1.0));
		}
		else {
			sprintf (line, "%s\n", GMT->current.setting.format_float_out);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes completely outside the %s will be set to ", msg[k]);
			(gmt_M_is_dnan (Ctrl->N.mask[GMT_OUTSIDE])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[GMT_OUTSIDE]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes completely inside the %s will be set to ", msg[k]);
			(gmt_M_is_dnan (Ctrl->N.mask[GMT_INSIDE])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[GMT_INSIDE]);
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Nodes on the %s boundary will be set to ", msg[k]);
			(gmt_M_is_dnan (Ctrl->N.mask[GMT_ONEDGE])) ? GMT_Message (API, GMT_TIME_NONE, "NaN\n") : GMT_Message (API, GMT_TIME_NONE, line, Ctrl->N.mask[GMT_ONEDGE]);
		}
	}

	n_columns = Grid->header->n_columns;	n_rows = Grid->header->n_rows;	/* Signed versions */
	replicate_x = (HH->nxp && Grid->header->registration == GMT_GRID_NODE_REG);	/* Gridline registration has duplicate column */
	replicate_y = (HH->nyp && Grid->header->registration == GMT_GRID_NODE_REG);	/* Gridline registration has duplicate row */
	x_wrap = Grid->header->n_columns - 1;				/* Add to node index to go to right column */
	y_wrap = (Grid->header->n_rows - 1) * Grid->header->n_columns;	/* Add to node index to go to bottom row */

	if (Ctrl->S.active) {	/* Need distance calculations in correct units, and the d_row/d_col machinery */
		if (Ctrl->S.mode == GRDMASK_N_CART_MASK) {
			max_d_col = urint (Ctrl->S.limit[GMT_X] / Grid->header->inc[GMT_X]);
			d_row = urint (Ctrl->S.limit[GMT_Y] / Grid->header->inc[GMT_Y]);
			d_col = gmt_M_memory (GMT, NULL, Grid->header->n_rows, double);
			for (rowu = 0; rowu < Grid->header->n_rows; rowu++) d_col[rowu] = max_d_col;
		}
		else {
			gmt_init_distaz (GMT, Ctrl->S.unit, Ctrl->S.mode, GMT_MAP_DIST);
			if (!Ctrl->S.variable_radius) {	/* Read x,y, fixed radius from -S */
				radius = Ctrl->S.radius;
				d_col = gmt_prep_nodesearch (GMT, Grid, radius, Ctrl->S.mode, &d_row, &max_d_col);	/* Init d_row/d_col etc */
			}
			else	/* Read x, y, radius */
				n_cols = 3;
		}
		grd_x0 = Grid->x;
		grd_y0 = Grid->y;
	}	
	
	periodic = gmt_M_is_geographic (GMT, GMT_IN);	/* Dealing with geographic coordinates */
	gmt_set_line_resampling (GMT, Ctrl->A.active, Ctrl->A.mode);	/* Possibly change line resampling mode */

	/* Initialize all nodes (including pad) to the 'outside' value */

	for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] = mask_val[GMT_OUTSIDE];

	if ((error = GMT_Set_Columns (API, GMT_IN, n_cols, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		if (Ctrl->S.active)
			gmt_M_free (GMT, d_col);
		Return (error);
	}
	gmode = (Ctrl->S.active) ? GMT_IS_POINT : GMT_IS_POLY;
	gmt_skip_xy_duplicates (GMT, true);	/* Skip repeating x/y points in polygons */
	if (GMT_Init_IO (API, GMT_IS_DATASET, gmode, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR)	/* Registers default input sources, unless already set */
		error = API->error;
	if (!Ctrl->S.active && (error = GMT_Set_Columns (API, GMT_IN, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		/* We don't want trailing text because we may need to resample lines below */
		Return (API->error);
	}
	if ((Din = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL)
		error = API->error;
	if (error && Ctrl->S.active) {
		gmt_M_free (GMT, d_col);
		Return (error);
	}
	gmt_skip_xy_duplicates (GMT, false);	/* Reset */

	D = Din;	/* The default is to work with the input data as is */
	DH = gmt_get_DD_hidden (D);
	
	if (!Ctrl->S.active) {	/* Make sure we were given at least one polygon */
		for (tbl = n_pol = 0; tbl < D->n_tables; tbl++)
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++)
				if (D->table[tbl]->segment[seg]->n_rows > 2) n_pol++;		/* Count polytons */
		if (n_pol == 0) {
			GMT_Report (API, GMT_MSG_NORMAL, "Without -S, we expect to read polygons but none found\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}
	if (!Ctrl->S.active && GMT->current.map.path_mode == GMT_RESAMPLE_PATH) {	/* Resample all polygons to desired resolution, once and for all */
		uint64_t n_new;
		if (DH->alloc_mode == GMT_ALLOC_EXTERNALLY) {
			D = GMT_Duplicate_Data (API, GMT_IS_DATASET, GMT_DUPLICATE_ALLOC + GMT_ALLOC_NORMAL, Din);
			DH = gmt_get_DD_hidden (D);
		}
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */
				S = D->table[tbl]->segment[seg];	/* Current segment */
				if ((n_new = gmt_fix_up_path (GMT, &S->data[GMT_X], &S->data[GMT_Y], S->n_rows, Ctrl->A.step, Ctrl->A.mode)) == 0) {
					if (DH->alloc_mode == GMT_ALLOC_EXTERNALLY) GMT_Destroy_Data (API, &D);
					Return (GMT_RUNTIME_ERROR);
				}
				S->n_rows = n_new;
				gmt_set_seg_minmax (GMT, D->geometry, 2, S);	/* Update min/max or x/y only */
			}
		}
	}

	if (!Ctrl->S.active)
		gmt_set_inside_mode (GMT, D, GMT_IOO_UNKNOWN);

	if (Ctrl->S.mode == GRDMASK_N_CART_MASK) radius = 1;	/* radius not used in this case and this avoids another if test */
	worry_about_jumps = (gmt_M_is_geographic (GMT, GMT_IN) && !gmt_grd_is_global (GMT, Grid->header));

	for (tbl = n_pol = 0; tbl < D->n_tables; tbl++) {
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++, n_pol++) {	/* For each segment in the table */
			S = D->table[tbl]->segment[seg];		/* Current data segment */
			if (Ctrl->S.active) {	/* Assign 'inside' to nodes within given distance of data constraints */
				for (k = 0; k < S->n_rows; k++) {
					if (gmt_M_y_is_outside (GMT, S->data[GMT_Y][k], Grid->header->wesn[YLO], Grid->header->wesn[YHI])) continue;	/* Outside y-range */
					xtmp = S->data[GMT_X][k];	/* Make copy since we may have to adjust by +-360 */
					if (gmt_x_is_outside (GMT, &xtmp, Grid->header->wesn[XLO], Grid->header->wesn[XHI])) continue;	/* Outside x-range (or longitude) */

					if (Ctrl->S.variable_radius) radius = S->data[GMT_Z][k];
					if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Make special checks for N and S poles */
						if (gmt_M_is_Npole (S->data[GMT_Y][k])) {	/* N pole */
							if (radius == 0.0) {	/* Only set the N pole row */
								gmt_M_col_loop (GMT, Grid, 0, col, ij)	/* Set this entire N row */
									Grid->data[ij] = mask_val[GMT_INSIDE];
								continue;
							}
							for (row = 0; row < (int)Grid->header->n_rows && (distance = gmt_distance (GMT, 0.0, 90.0, grd_x0[0], grd_y0[row])) <= radius; row++) {
								value = (doubleAlmostEqualZero (distance, radius)) ? mask_val[GMT_ONEDGE] : mask_val[GMT_INSIDE];	/* The onedge or inside value */
								gmt_M_col_loop (GMT, Grid, row, col, ij)	/* Set this entire row */
									Grid->data[ij] = value;
							}
							continue;
						}
						else if (gmt_M_is_Spole (S->data[GMT_Y][k])) {	/* S pole */
							if (radius == 0.0) {	/* Only set the S pole row */
								gmt_M_col_loop (GMT, Grid, Grid->header->n_rows - 1, col, ij)	/* Set this entire S row */
									Grid->data[ij] = mask_val[GMT_INSIDE];
								continue;
							}
							for (row = (int)(Grid->header->n_rows - 1); row >= 0 && (distance = gmt_distance (GMT, 0.0, -90.0, grd_x0[0], grd_y0[row])) <= radius; row--) {
								value = (doubleAlmostEqualZero (distance, radius)) ? mask_val[GMT_ONEDGE] : mask_val[GMT_INSIDE];	/* The onedge or inside value */
								gmt_M_col_loop (GMT, Grid, row, col, ij)	/* Set this entire row */
									Grid->data[ij] = value;
							}
							continue;
						}
					}

					/* OK, not a pole and this point is within bounds, but may be exactly on the border */

					row_0 = gmt_M_grd_y_to_row (GMT, S->data[GMT_Y][k], Grid->header);
					if (row_0 == (int)Grid->header->n_rows) row_0--;	/* Was exactly on the ymin edge */
					if (gmt_y_out_of_bounds (GMT, &row_0, Grid->header, &wrap_180)) continue;	/* Outside y-range.  This call must happen BEFORE gmt_x_out_of_bounds as it sets wrap_180 */
					col_0 = gmt_M_grd_x_to_col (GMT, xtmp, Grid->header);
					if (col_0 == (int)Grid->header->n_columns) col_0--;	/* Was exactly on the xmax edge */
					if (gmt_x_out_of_bounds (GMT, &col_0, Grid->header, wrap_180)) continue;	/* Outside x-range,  This call must happen AFTER gmt_y_out_of_bounds which sets wrap_180 */ 
					ij = gmt_M_ijp (Grid->header, row_0, col_0);
					Grid->data[ij] = mask_val[GMT_INSIDE];	/* This is the nearest node */
					if (Grid->header->registration == GMT_GRID_NODE_REG &&
					    (col_0 == 0 || col_0 == (int)(Grid->header->n_columns-1)) && periodic_grid) {
						/* Must duplicate the entry at periodic point */
						col = (col_0 == 0) ? Grid->header->n_columns-1 : 0;
						ij = gmt_M_ijp (Grid->header, row_0, col);
						Grid->data[ij] = mask_val[GMT_INSIDE];	/* This is also the nearest node */
					}
					if (radius == 0.0) continue;	/* Only consider the nearest node */
					/* Here we also include all the nodes within the search radius */
					if (Ctrl->S.variable_radius && !doubleAlmostEqual (radius, last_radius)) {	/* Init d_row/d_col etc */
						gmt_M_free (GMT, d_col);
						d_col = gmt_prep_nodesearch (GMT, Grid, radius, Ctrl->S.mode, &d_row, &max_d_col);
						last_radius = radius;
					}

					row_end = row_0 + d_row;
#ifdef _OPENMP
#pragma omp parallel for private(row,col,rowu,colu,col_end,jj,ii,ij,wrap_180,distance) shared(Grid,HH,row_0,d_row,col_0,d_col,row_end,xtmp,S,grd_x0,grd_y0,replicate_x,replicate_y,x_wrap,y_wrap,radius,mask_val)
#endif
					for (row = row_0 - d_row; row <= row_end; row++) {
						jj = row;
						if (gmt_y_out_of_bounds (GMT, &jj, Grid->header, &wrap_180)) continue;	/* Outside y-range.  This call must happen BEFORE gmt_x_out_of_bounds as it sets wrap_180 */
						rowu = jj;
						col_end = col_0 + d_col[jj];
						for (col = col_0 - d_col[row]; col <= col_end; col++) {
							ii = col;
							if (gmt_x_out_of_bounds (GMT, &ii, Grid->header, wrap_180)) continue;	/* Outside x-range,  This call must happen AFTER gmt_y_out_of_bounds which sets wrap_180 */ 
							colu = ii;
							ij = gmt_M_ijp (Grid->header, rowu, colu);
							if (Ctrl->S.mode == GRDMASK_N_CART_MASK)	/* Rectangular are for Cartesian so no need to check radius */
								Grid->data[ij] = mask_val[GMT_INSIDE];	/* The inside value */
							else {
								distance = gmt_distance (GMT, xtmp, S->data[GMT_Y][k], grd_x0[colu], grd_y0[rowu]);
								if (distance > radius) continue;	/* Clearly outside */
								Grid->data[ij] = (doubleAlmostEqualZero (distance, radius)) ? mask_val[GMT_ONEDGE] : mask_val[GMT_INSIDE];	/* The onedge or inside value */
							}
							/* With periodic, gridline-registered grids there are duplicate rows and/or columns
							   so we may have to assign the point to more than one node.  The next section deals
							   with this situation.
							*/

							if (replicate_x) {	/* Must check if we have to replicate a column */
								if (colu == 0) 	/* Must replicate left to right column */
									Grid->data[ij+x_wrap] = Grid->data[ij];
								else if (colu == HH->nxp)	/* Must replicate right to left column */
									Grid->data[ij-x_wrap] = Grid->data[ij];
							}
							if (replicate_y) {	/* Must check if we have to replicate a row */
								if (rowu == 0)	/* Must replicate top to bottom row */
									Grid->data[ij+y_wrap] = Grid->data[ij];
								else if (rowu == HH->nyp)	/* Must replicate bottom to top row */
									Grid->data[ij-y_wrap] = Grid->data[ij];
							}
						}
					}
				}
			}
			else if (S->n_rows > 2) {	/* Assign 'inside' to nodes if they are inside any of the given polygons (Need at least 3 vertices) */
				if (gmt_polygon_is_hole (GMT, S)) continue;	/* Holes are handled within gmt_inonout */
				SH = gmt_get_DS_hidden (S);
				if (Ctrl->N.mode == 1 || Ctrl->N.mode == 2) {	/* Look for z-values in the data headers */
					if (SH->ogr)	/* OGR data */
						z_value = gmt_get_aspatial_value (GMT, GMT_IS_Z, S);
					else if (gmt_parse_segment_item (GMT, S->header, "-Z", text_item))	/* Look for zvalue option */
						z_value = atof (text_item);
					else if (gmt_parse_segment_item (GMT, S->header, "-L", text_item))	/* Look for segment header ID */
						z_value = atof (text_item);
					else
						GMT_Report (API, GMT_MSG_NORMAL, "No z-value found; z-value set to NaN\n");
				}
				else if (Ctrl->N.mode)	/* 3 or 4; Increment running polygon ID */
					z_value += 1.0;

				if (worry_about_jumps) gmt_eliminate_lon_jumps (GMT, S->data[GMT_X], S->n_rows);	/* Since many segments may have been read we cannot be sure there are no junps */

				for (row = 0; row < n_rows; row++) {	/* Loop over grid rows */
					yy = gmt_M_grd_row_to_y (GMT, row, Grid->header);
					
					/* First check if y/latitude is outside, then there is no need to check all the x/lon values */
					if (periodic) {	/* Containing annulus test */
						do_test = true;
						switch (SH->pole) {
							case 0:	/* Not a polar cap */
								if (yy < S->min[GMT_Y] || yy > S->max[GMT_Y]) continue;	/* Outside, no need to check */
								break;
							case -1:	/* S polar cap */
								if (yy > S->max[GMT_Y]) continue;	/* Outside, no need to check */
								if (yy < SH->lat_limit) known_side = GMT_INSIDE, do_test = false;	/* Guaranteed inside, set answer */
								break;
							case +1:	/* N polar cap */
								if (yy < S->min[GMT_Y]) continue;	/* Outside, no need to check */
								if (yy > SH->lat_limit) known_side = GMT_INSIDE, do_test = false;	/* Guaranteed inside, set answer */
								break;
						}
					}
					else if (yy < S->min[GMT_Y] || yy > S->max[GMT_Y])	/* Cartesian case */
						continue;	/* Outside, no need to check */

					/* Here we will have to consider the x coordinates as well (or known_side is set) */
#ifdef _OPENMP
#pragma omp parallel for private(col,xx,side,ij) shared(Grid,n_columns,do_test,known_side,yy,S,row,Ctrl,z_value,mask_val)
#endif
					for (col = 0; col < n_columns; col++) {	/* Loop over grid columns */
						xx = gmt_M_grd_col_to_x (GMT, col, Grid->header);
						if (do_test) {	/* Must consider xx to determine if we are inside */
							if ((side = gmt_inonout (GMT, xx, yy, S)) == GMT_OUTSIDE)
								continue;	/* Outside polygon, go to next point */
						}
						else	/* Already know the answer */
							side = known_side;
						/* Here, point is inside or on edge, we must assign value */

						ij = gmt_M_ijp (Grid->header, row, col);
						
						if (Ctrl->N.mode%2 && side == GMT_ONEDGE) continue;	/* Not counting the edge as part of polygon for ID tagging for mode 1 | 3 */
						Grid->data[ij] = (Ctrl->N.mode) ? (gmt_grdfloat)z_value : mask_val[side];
					}
					GMT_Report (API, GMT_MSG_DEBUG, "Polygon %d scanning row %05d\n", n_pol, row);
				}
			}
			else {	/* 2 or fewer points in the "polygon" */
				GMT_Report (API, GMT_MSG_VERBOSE, "Segment %" PRIu64 " is not a polygon - skipped\n", seg);
			}
		}
	}
	if (D != Din && GMT_Destroy_Data (API, &D) != GMT_NOERROR) Return (API->error);	/* Free the duplicate dataset */
	
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
		Return (API->error);
	}

	if (Ctrl->S.active)
		gmt_M_free (GMT, d_col);

	Return (GMT_NOERROR);
}
