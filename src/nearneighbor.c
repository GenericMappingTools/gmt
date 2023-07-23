/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: Based on a specified grid size, nearneighbor reads an xyz file and
 * determines the nearest points to each node in sectors.  The default
 * looks for the nearest point for each quadrant.  The points must also
 * be within a maximum search-radius from the node.  For the nodes that
 * have a full set of nearest neighbors, a weighted average value is
 * computed.  New feature is full support for boundary conditions so
 * that geographic or periodic conditions are explicitly dealt with
 * in the sense that a data point may wrap around to serve as a
 * constraint on the other side of the periodic boundary.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"
#include "longopt/nearneighbor_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"nearneighbor"
#define THIS_MODULE_MODERN_NAME	"nearneighbor"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Grid table data using a \"Nearest neighbor\" algorithm"
#define THIS_MODULE_KEYS	"<D{,GG}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVabdefhinqrw" GMT_OPT("FH")

#define NN_DEF_SECTORS	4
#define GMT_n_OPT2 "-n+b<BC>"

struct NEARNEIGHBOR_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct NEARNEIGHBOR_E {	/* -E<empty> */
		bool active;
		double value;
	} E;
	struct NEARNEIGHBOR_G {	/* -G<grdfile> */
		bool active;
		char *file;
	} G;
	struct NEARNEIGHBOR_I {	/* -I (for checking only) */
		bool active;
	} I;
	struct NEARNEIGHBOR_N {	/* -N<sectors>[+m<min_sectors>] | -Nn */
		bool active;
		unsigned int sectors, min_sectors;
		unsigned int mode;
	} N;
	struct NEARNEIGHBOR_S {	/* -S[-|=|+]<radius>[d|e|f|k|m|M|n] */
		bool active;
		int mode;	/* May be negative */
		double radius;
		char unit;
	} S;
	struct NEARNEIGHBOR_W {	/* -W */
		bool active;
	} W;
};

struct NEARNEIGHBOR_NODE {	/* Structure with point id and distance pairs for all sectors */
	gmt_grdfloat *distance;	/* Distance of nearest datapoint to this node per sector */
	int64_t *datum;		/* Point id of this data point */
};

struct NEARNEIGHBOR_POINT {	/* Structure with input data constraints */
	gmt_grdfloat x, y, z, w;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct NEARNEIGHBOR_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1U, struct NEARNEIGHBOR_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->N.sectors = C->N.min_sectors = NN_DEF_SECTORS;
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct NEARNEIGHBOR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL struct NEARNEIGHBOR_NODE *nearneighbor_add_new_node (struct GMT_CTRL *GMT, unsigned int n) {
	/* Allocate and initialize a new node to have GMT_NOTSET in all the n datum sectors */
	struct NEARNEIGHBOR_NODE *new_node = gmt_M_memory (GMT, NULL, 1U, struct NEARNEIGHBOR_NODE);
	new_node->distance = gmt_M_memory (GMT, NULL, n, gmt_grdfloat);
	new_node->datum = gmt_M_memory (GMT, NULL, n, int64_t);
	while (n > 0) new_node->datum[--n] = GMT_NOTSET;

	return (new_node);
}

GMT_LOCAL void nearneighbor_assign_node (struct GMT_CTRL *GMT, struct NEARNEIGHBOR_NODE **node, unsigned int n_sector, unsigned int sector, double distance, uint64_t id) {
	/* Allocates node space if not already used and updates the value if closer to node than the current value */

	if (!(*node)) *node = nearneighbor_add_new_node (GMT, n_sector);
	if ((*node)->datum[sector] == GMT_NOTSET || (*node)->distance[sector] > distance) {
		(*node)->distance[sector] = (gmt_grdfloat)distance;
		(*node)->datum[sector] = id;
	}
}

GMT_LOCAL void nearneighbor_free_node (struct GMT_CTRL *GMT, struct NEARNEIGHBOR_NODE *node) {
	/* Frees allocated node space */

	if (!node) return;	/* Nothing to do */
	gmt_M_free (GMT, node->distance);
	gmt_M_free (GMT, node->datum);
	gmt_M_free (GMT, node);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [<table>] -G%s %s %s -S%s [-E<empty>] [-N<sectors>[+m<min_sectors>]|n] "
		"[%s] [-W] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s] [%s]\n",
		name, GMT_OUTGRID, GMT_I_OPT, GMT_Rgeo_OPT, GMT_RADIUS_OPT, GMT_V_OPT, GMT_a_OPT, GMT_bi_OPT, GMT_di_OPT,
		GMT_e_OPT,GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_n_OPT2, GMT_qi_OPT, GMT_r_OPT, GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	GMT_Option (API, "<");
	gmt_outgrid_syntax (API, 'G', "Set name of the output grid file");
	GMT_Option (API, "I");
	GMT_Option (API, "R");
	gmt_dist_syntax (API->GMT, "S" GMT_RADIUS_OPT, "Only consider points inside this search radius.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-E<empty>");
	GMT_Usage (API, -2, "Value to use for empty nodes [Default is NaN].");
	GMT_Usage (API, 1, "\n-N<sectors>[+m<min_sectors>]|n");
	GMT_Usage (API, -2, "Set number of sectors and the minimum number of sectors with data required for averaging. "
		"If modifier +m<min_sectors> is omitted it defaults to ~50%% of <sectors>. "
		"Default is -N%d+m%d, i.e., a quadrant search, requiring all sectors to be filled. "
		"Alternatively, supply -Nn for plain NN algorithm via GDAL.", NN_DEF_SECTORS, NN_DEF_SECTORS);
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-W Input <table> has observation weights in 4th column.");
	GMT_Option (API, "a,bi");
	GMT_Usage (API, -2, "Default is 3 (or 4 if -W is set) columns.");
	GMT_Option (API, "di,e,f,h,i");
	if (gmt_M_showusage (API)) {
		GMT_Usage (API, 1, "\n%s", GMT_n_OPT2);
		GMT_Usage (API, -2, "Set boundary conditions.  <BC> can be either:");
		GMT_Usage (API, 3, "g: Set geographic boundary conditions.");
		GMT_Usage (API, 3, "p: Set periodic boundary conditions.");
		GMT_Usage (API, 3, "n: Set natural boundary conditions.");
		GMT_Usage (API, -2, "Note: For p and n you may optionally append x or y [Default is both]:");
		GMT_Usage (API, 3, "x applies the boundary condition for x only.");
		GMT_Usage (API, 3, "y applies the boundary condition for y only.");
		GMT_Usage (API, -2, "[Default: Natural conditions, unless grid is geographic].");
	}
	GMT_Option (API, "qi,r,w,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct NEARNEIGHBOR_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to nearneighbor and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	int n;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input file(s) */
				if (GMT_Get_FilePath (API, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'E':	/* NaN value */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				if (opt->arg[0])
					Ctrl->E.value = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.d_NaN : atof (opt->arg);
				else {
					n_errors++;
					GMT_Report (API, GMT_MSG_ERROR, "Option -E: Must specify value or NaN\n");
				}
				break;
			case 'G':	/* Output file */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_GRID, GMT_OUT, GMT_FILE_LOCAL, &(Ctrl->G.file));
				break;
			case 'I':	/* Grid spacings */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'L':	/* BCs */
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -L is deprecated; -n+b%s was set instead, use this in the future.\n", opt->arg);
					gmt_strncpy (GMT->common.n.BC, opt->arg, 4U);
					/* We turn on geographic coordinates if -Lg is given by faking -fg */
					/* But since GMT_parse_f_option is private to gmt_init and all it does */
					/* in this case are 2 lines below we code it here */
					if (!strcmp (GMT->common.n.BC, "g")) {
						gmt_set_geographic (GMT, GMT_IN);
						gmt_set_geographic (GMT, GMT_OUT);
					}
				}
				else
					n_errors += gmt_default_option_error (GMT, opt);
				break;
			case 'N':	/* -N<sectors>[+m<minsectors>]] or -Nn */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				if (opt->arg[0] == 'n')
					Ctrl->N.mode = 1;
				else if (opt->arg[0]) {	/* Override default -N4+m4 */
					if (isalpha(opt->arg[0])) {
						GMT_Report (API, GMT_MSG_ERROR, "-N%c is not a valid option\n", opt->arg[0]);
						n_errors++;
						break;
					}
					if ((c = strstr (opt->arg, "+m"))) {	/* Set sectors and min sectors using current syntax */
						Ctrl->N.min_sectors = atoi (&c[2]);
						c[0] = '\0';
						Ctrl->N.sectors = atoi (opt->arg);
						c[0] = '+';
					}
					else {	/* Either no modifier or old slash-based syntax */
						n = sscanf (opt->arg, "%d/%d", &Ctrl->N.sectors, &Ctrl->N.min_sectors);
						if (n < 2) Ctrl->N.min_sectors = irint (Ctrl->N.sectors / 2.0);	/* Giving just -N<sectors> means -N<sectors>/(<sectors>/2) */
					}
				}
				if (Ctrl->N.sectors < Ctrl->N.min_sectors) Ctrl->N.min_sectors = Ctrl->N.sectors;	/* Minimum cannot be larger than desired */
				break;
			case 'S':	/* Search radius */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S.active);
				Ctrl->S.mode = gmt_get_distance (GMT, opt->arg, &(Ctrl->S.radius), &(Ctrl->S.unit));
				break;
			case 'W':	/* Use weights */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Option -I: Must specify positive increment(s)\n");
	if (Ctrl->N.active && Ctrl->N.mode) {	/* For Natural NN gridding we cannot have stray options */
		n_errors += gmt_M_check_condition (GMT, Ctrl->E.active, "Option -Nn: Cannot specify -E option\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->W.active, "Option -Nn: Cannot specify -W option\n");
	}
	else {
		n_errors += gmt_M_check_condition (GMT, Ctrl->N.sectors <= 0, "Option -N: Must specify a positive number of sectors\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -1, "Option -S: Unrecognized unit\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -2, "Option -S: Unable to decode radius\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -3, "Option -S: Radius is negative\n");
	}
	n_errors += gmt_check_binary_io (GMT, (Ctrl->W.active) ? 4 : 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_nearneighbor (void *V_API, int mode, void *args) {
	openmp_int rowu, colu, max_d_col, d_row, *d_col = NULL;
	int col_0, row_0, row, col, row_end, col_end, ii, jj, error = 0;
	unsigned int k, sector, y_wrap, x_wrap;
	bool wrap_180, replicate_x, replicate_y;
	size_t n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;

	uint64_t ij, ij0, kk, n, n_read, n_almost, n_none, n_set, n_filled;

	double weight, weight_sum, grd_sum, dx, dy, delta, distance = 0.0;
	double x_left, x_right, y_top, y_bottom, factor, three_over_radius;
	double half_y_width, y_width, half_x_width, x_width;
	double *x0 = NULL, *y0 = NULL, *in = NULL;

	struct GMT_GRID *Grid = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_RECORD *In = NULL;
	struct NEARNEIGHBOR_NODE **grid_node = NULL;
	struct NEARNEIGHBOR_POINT *point = NULL;
	struct NEARNEIGHBOR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the nearneighbor main code ----------------------------*/

#if ((GDAL_VERSION_MAJOR >= 2) && (GDAL_VERSION_MINOR >= 1)) || (GDAL_VERSION_MAJOR >= 3)
	if (Ctrl->N.mode) {	/* Pass over to GDAL */
		char buf[GMT_LEN128] = {""};
		struct GMT_OPTION *opt = NULL;
		struct GMT_GDALLIBRARIFIED_CTRL *st = gmt_M_memory (GMT, NULL, 1, struct GMT_GDALLIBRARIFIED_CTRL);
		for (opt = options; opt; opt = opt->next) {	/* Loop over arguments, skip options */
			if (opt->option != GMT_OPT_INFILE) continue;	/* We are only processing filenames here */
			st->fname_in  = opt->arg;
			st->fname_out = Ctrl->G.file;
			sprintf(buf,"-a nearest:radius1=%f:radius2=%f:nodata=NaN", Ctrl->S.radius, Ctrl->S.radius);
			st->opts = buf;
			if ((error = gmt_gdal_grid (GMT, st))) {
				GMT_Report (API, GMT_MSG_ERROR, "GDAL nearest neighbor returned error %d\n", error);
			}
			break;
		}
		gmt_M_free (GMT, st);
		Return (error);
	}
#else
	GMT_Report (API, GMT_MSG_ERROR, "Option -Nn: Requires GDAL 2.1 or later\n");
#endif

	/* Regular nearest neighbor moving average operation */

	if (gmt_init_distaz (GMT, Ctrl->S.unit, Ctrl->S.mode, GMT_MAP_DIST) == GMT_NOT_A_VALID_TYPE)
		Return (GMT_NOT_A_VALID_TYPE);

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, NULL, NULL, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
	HH = gmt_get_H_hidden (Grid->header);

	/* Initialize the input since we are doing record-by-record reading/writing */
	if ((error = GMT_Set_Columns (API, GMT_IN, 3 + Ctrl->W.active, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Grid dimensions are n_columns = %d, n_rows = %d\n", Grid->header->n_columns, Grid->header->n_rows);
	GMT_Report (API, GMT_MSG_INFORMATION, "Number of sectors = %d, minimum number of filled sectors = %d\n", Ctrl->N.sectors, Ctrl->N.min_sectors);

	grid_node = gmt_M_memory (GMT, NULL, Grid->header->nm, struct NEARNEIGHBOR_NODE *);
	point = gmt_M_memory (GMT, NULL, n_alloc, struct NEARNEIGHBOR_POINT);

	x0 = gmt_grd_coord (GMT, Grid->header, GMT_X);
	y0 = gmt_grd_coord (GMT, Grid->header, GMT_Y);

	d_col = gmt_prep_nodesearch (GMT, Grid, Ctrl->S.radius, Ctrl->S.mode, &d_row, &max_d_col);	/* Init d_row/d_col etc */

	factor = Ctrl->N.sectors / (2.0 * M_PI);

	/* To allow data points falling outside -R but within the search radius we extend the data domain in all directions */

	x_left = Grid->header->wesn[XLO];	x_right = Grid->header->wesn[XHI];	/* This is what -R says */
	if (gmt_M_is_cartesian (GMT, GMT_IN) || !gmt_grd_is_global (GMT, Grid->header)) {
		x_left  -= max_d_col * Grid->header->inc[GMT_X];	/* OK to extend x-domain since not a periodic geographic grid */
		x_right += max_d_col * Grid->header->inc[GMT_X];
	}
	y_top = Grid->header->wesn[YHI] + d_row * Grid->header->inc[GMT_Y];	y_bottom = Grid->header->wesn[YLO] - d_row * Grid->header->inc[GMT_Y];
	if (gmt_M_y_is_lat (GMT, GMT_IN)) {	/* For geographic grids we must ensure the extended y-domain is physically possible */
		if (y_bottom < -90.0) y_bottom = -90.0;
		if (y_top > 90.0) y_top = 90.0;
	}
	x_width = Grid->header->wesn[XHI] - Grid->header->wesn[XLO];		y_width = Grid->header->wesn[YHI] - Grid->header->wesn[YLO];
	half_x_width = 0.5 * x_width;			half_y_width = 0.5 * y_width;
	n = n_read = 0;
	replicate_x = (HH->nxp && Grid->header->registration == GMT_GRID_NODE_REG);	/* Gridline registration has duplicate column */
	replicate_y = (HH->nyp && Grid->header->registration == GMT_GRID_NODE_REG);	/* Gridline registration has duplicate row */
	x_wrap = Grid->header->n_columns - 1;				/* Add to node index to go to right column */
	y_wrap = (Grid->header->n_rows - 1) * Grid->header->n_columns;	/* Add to node index to go to bottom row */
	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		gmt_M_free (GMT, x0);		gmt_M_free (GMT, y0);		gmt_M_free (GMT, grid_node);
		gmt_M_free (GMT, point);	gmt_M_free (GMT, d_col);
		Return (API->error);
	}

	do {	/* Keep returning records until we reach EOF */
		n_read++;
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				gmt_M_free (GMT, x0);		gmt_M_free (GMT, y0);		gmt_M_free (GMT, grid_node);
				gmt_M_free (GMT, point);	gmt_M_free (GMT, d_col);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}
		if (In->data == NULL) {
			gmt_quit_bad_record (API, In);
			Return (API->error);
		}

		in = In->data;	/* Only need to process numerical part here */

		if (gmt_M_is_dnan (in[GMT_Z])) continue;					/* Skip if z = NaN */
		if (gmt_M_y_is_outside (GMT, in[GMT_Y], y_bottom, y_top)) continue;	/* Outside y-range */
		if (gmt_x_is_outside (GMT, &in[GMT_X], x_left, x_right)) continue;	/* Outside x-range (or longitude) */

		/* Data record to process */

		/* Store this point in memory */

		point[n].x = (gmt_grdfloat)in[GMT_X];
		point[n].y = (gmt_grdfloat)in[GMT_Y];
		point[n].z = (gmt_grdfloat)in[GMT_Z];
		if (Ctrl->W.active) point[n].w = (gmt_grdfloat)in[3];

		/* Find row/col indices of the node closest to this data point.  Note: These may be negative */

		col_0 = (int)gmt_M_grd_x_to_col (GMT, in[GMT_X], Grid->header);
		row_0 = (int)gmt_M_grd_y_to_row (GMT, in[GMT_Y], Grid->header);

		/* Loop over all nodes within radius of this node */

		row_end = row_0 + (int)d_row;
		for (row = row_0 - (int)d_row; row <= row_end; row++) {

			jj = row;
			if (gmt_y_out_of_bounds (GMT, &jj, Grid->header, &wrap_180)) continue;	/* Outside y-range.  This call must happen BEFORE gmt_x_out_of_bounds as it sets wrap_180 */
			rowu = (openmp_int)jj;
			col_end = col_0 + (int)d_col[jj];
			for (col = col_0 - (int)d_col[jj]; col <= col_end; col++) {

				ii = col;
				if (gmt_x_out_of_bounds (GMT, &ii, Grid->header, wrap_180)) continue;	/* Outside x-range,  This call must happen AFTER gmt_y_out_of_bounds which sets wrap_180 */

				/* Here, (ii,jj) [both are >= 0] is index of a node (kk) inside the grid */
				colu = (openmp_int)ii;

				distance = gmt_distance (GMT, x0[colu], y0[rowu], in[GMT_X], in[GMT_Y]);

				if (distance > Ctrl->S.radius) continue;	/* Data constraint is too far from this node */
				kk = gmt_M_ij0 (Grid->header, rowu, colu);	/* No padding used for gridnode array */
				dx = in[GMT_X] - x0[colu];	dy = in[GMT_Y] - y0[rowu];

				/* Check for wrap-around in x or y.  This should only occur if the
				   search radius is larger than 1/2 the grid width/height so that
				   the shortest distance is going through the periodic boundary.
				   For longitudes the dx obviously cannot exceed 180 (half_x_width)
				   since we could then go the other direction instead.
				*/
				if (HH->nxp && fabs (dx) > half_x_width) dx -= copysign (x_width, dx);
				if (HH->nyp && fabs (dy) > half_y_width) dy -= copysign (y_width, dy);

				/* OK, this point should constrain this node.  Calculate which sector and assign the value */

				sector = urint (floor (((d_atan2 (dy, dx) + M_PI) * factor))) % Ctrl->N.sectors;
				nearneighbor_assign_node (GMT, &grid_node[kk], Ctrl->N.sectors, sector, distance, n);

				/* With periodic, gridline-registered grids there are duplicate rows and/or columns
				   so we may have to assign the point to more than one node.  The next section deals
				   with this situation.
				*/

				if (replicate_x) {	/* Must check if we have to replicate a column */
					if (colu == 0) 	/* Must replicate left to right column */
						nearneighbor_assign_node (GMT, &grid_node[kk+x_wrap], Ctrl->N.sectors, sector, distance, n);
					else if (colu == (openmp_int)HH->nxp)	/* Must replicate right to left column */
						nearneighbor_assign_node (GMT, &grid_node[kk-x_wrap], Ctrl->N.sectors, sector, distance, n);
				}
				if (replicate_y) {	/* Must check if we have to replicate a row */
					if (rowu == 0)	/* Must replicate top to bottom row */
						nearneighbor_assign_node (GMT, &grid_node[kk+y_wrap], Ctrl->N.sectors, sector, distance, n);
					else if (rowu == (openmp_int)HH->nyp)	/* Must replicate bottom to top row */
						nearneighbor_assign_node (GMT, &grid_node[kk-y_wrap], Ctrl->N.sectors, sector, distance, n);
				}
			}
		}
		n++;
		if (!(n % 16384)) GMT_Report (API, GMT_MSG_INFORMATION, "Processed record %10ld\r", n);	/* 16384 = 2^14 */
		if (n == n_alloc) {
			size_t old_n_alloc = n_alloc;
			n_alloc <<= 1;
			point = gmt_M_memory (GMT, point, n_alloc, struct NEARNEIGHBOR_POINT);
			gmt_M_memset (&(point[old_n_alloc]), n_alloc - old_n_alloc, struct NEARNEIGHBOR_POINT);	/* Set to NULL/0 */
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	GMT_Report (API, GMT_MSG_INFORMATION, "Processed record %10ld\n", n);

	if (n < n_alloc) point = gmt_M_memory (GMT, point, n, struct NEARNEIGHBOR_POINT);
	/* Compute weighted averages based on the nearest neighbors */

	if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, Grid) == NULL) {
		gmt_M_free (GMT, point);	gmt_M_free (GMT, grid_node);	gmt_M_free (GMT, d_col);
		gmt_M_free (GMT, x0);		gmt_M_free (GMT, y0);
		Return (API->error);
	}

	n_set = n_almost = n_none = 0;

	if (!Ctrl->E.active) Ctrl->E.value = GMT->session.d_NaN;
	three_over_radius = 3.0 / Ctrl->S.radius;

	ij0 = 0;
	gmt_M_row_loop (GMT, Grid, rowu) {
		gmt_M_col_loop (GMT, Grid, rowu, colu, ij) {
			if (!grid_node[ij0]) {	/* No nearest neighbors, set to empty and goto next node */
				n_none++;
				Grid->data[ij] = (gmt_grdfloat)Ctrl->E.value;
				ij0++;
				continue;
			}

			for (k = 0, n_filled = 0; k < Ctrl->N.sectors; k++)
				if (grid_node[ij0]->datum[k] >= 0) n_filled++;
			if (n_filled < Ctrl->N.min_sectors) { 	/* Not minimum set of neighbors in all sectors, set to empty and goto next node */
				n_almost++;
				Grid->data[ij] = (gmt_grdfloat)Ctrl->E.value;
				nearneighbor_free_node (GMT, grid_node[ij0]);
				ij0++;
				continue;
			}

			/* OK, here we have enough data and need to calculate the weighted value */

			n_set++;
			weight_sum = grd_sum = 0.0;	/* Initialize sums */
			for (k = 0; k < Ctrl->N.sectors; k++) {
				if (grid_node[ij0]->datum[k] >= 0) {
					delta = three_over_radius * grid_node[ij0]->distance[k];
					weight = 1.0 / (1.0 + delta * delta);	/* This is distance weight */
					if (Ctrl->W.active) weight *= point[grid_node[ij0]->datum[k]].w;	/* This is observation weight */
					grd_sum += weight * point[grid_node[ij0]->datum[k]].z;
					weight_sum += weight;
				}
			}
			Grid->data[ij] = (gmt_grdfloat)(grd_sum / weight_sum);
			nearneighbor_free_node (GMT, grid_node[ij0]);
			ij0++;
		}
		if ((rowu % 128) == 0) GMT_Report (API, GMT_MSG_INFORMATION, "Gridded row %10ld\r", rowu);
	}
	GMT_Report (API, GMT_MSG_INFORMATION, "Gridded row %10ld\n", rowu);

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) {
		gmt_M_free (GMT, point);	gmt_M_free (GMT, grid_node);	gmt_M_free (GMT, d_col);
		gmt_M_free (GMT, x0);		gmt_M_free (GMT, y0);
		Return (API->error);
	}

	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
		gmt_M_free (GMT, point);	gmt_M_free (GMT, grid_node);	gmt_M_free (GMT, d_col);
		gmt_M_free (GMT, x0);		gmt_M_free (GMT, y0);
		Return (API->error);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		char line[GMT_BUFSIZ];
		sprintf (line, "%s)\n", GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " nodes were assigned an average value\n", n_set);
		GMT_Report (API, GMT_MSG_INFORMATION, "%" PRIu64 " nodes failed sector criteria and %" PRIu64 " nodes had no neighbor points (all set to ", n_almost, n_none);
		(gmt_M_is_dnan (Ctrl->E.value)) ? GMT_Message (API, GMT_TIME_NONE, "NaN)\n") : GMT_Message (API, GMT_TIME_NONE,  line, Ctrl->E.value);
	}

	gmt_M_free (GMT, point);	gmt_M_free (GMT, grid_node);	gmt_M_free (GMT, d_col);
	gmt_M_free (GMT, x0);		gmt_M_free (GMT, y0);

	Return (GMT_NOERROR);
}
