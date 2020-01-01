/*--------------------------------------------------------------------
 *
 *	Copyright (c) 2008-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Spherical nearest distances - via Voronoi polygons.  We read input
 * data, assumed to be things like coastlines, and want to create a grid
 * with distances to the nearest line.  The approach here is to break
 * the data into voronoi polygons and then visit all nodes inside each
 * polygon and use geodesic distance calculation from each node to the
 * unique Voronoi interior data node.
 * Relies on STRIPACK Fortran F77 library (Renka, 1997). Reference:
 * Renka, R, J,, 1997, Algorithm 772: STRIPACK: Delaunay Triangulation
 *     and Voronoi Diagram on the Surface of a Sphere, AMC Trans. Math.
 *     Software, 23 (3), 416-434.
 * We translated to C using f2c -r8 and and manually edited the code
 * so that f2c libs were not needed.  For any translation errors, blame me.
 *
 * Author:	Paul Wessel
 * Date:	1-AUG-2011
 * Version:	6 API
 *
 */

#include "gmt_dev.h"
#include "gmt_sph.h"

#define THIS_MODULE_CLASSIC_NAME	"sphdistance"
#define THIS_MODULE_MODERN_NAME	"sphdistance"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Create Voronoi distance, node, or natural nearest-neighbor grid on a sphere"
#define THIS_MODULE_KEYS	"<D{,ND(,QD(,GG},Q-("
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVbdehijrs" GMT_OPT("F")

enum sphdist_modes {
	SPHD_DIST = 0,
	SPHD_NODES = 1,
	SPHD_VALUES = 2};

struct SPHDISTANCE_CTRL {
	struct A {	/* -A[m|p|x|y|step] */
		bool active;
		unsigned int mode;
		double step;
	} A;
	struct C {	/* -C */
		bool active;
	} C;
	struct E {	/* -Ed|n|z[<dist>] */
		bool active;
		unsigned int mode;
		double dist;
	} E;
	struct G {	/* -G<maskfile> */
		bool active;
		char *file;
	} G;
	struct L {	/* -L<unit>] */
		bool active;
		char unit;
	} L;
	struct N {	/* -N */
		bool active;
		char *file;
	} N;
	struct Q {	/* -Q */
		bool active;
		char *file;
	} Q;
};

GMT_LOCAL void prepare_polygon (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *P) {
	/* Set the min/max extent of this polygon and determine if it
	 * is a polar cap; if so set the required metadata flags */
	uint64_t row;
	double lon_sum = 0.0, lat_sum = 0.0, dlon;
	struct GMT_DATASEGMENT_HIDDEN *PH = gmt_get_DS_hidden (P);

	gmt_set_seg_minmax (GMT, GMT_IS_POLY, 0, P);	/* Set the domain of the segment */

	/* Then loop over points to accumulate sums */

	for (row = 1; row < P->n_rows; row++) {	/* Start at row = 1 since (a) 0'th point is repeated at end and (b) we are doing differences */
		gmt_M_set_delta_lon (P->data[GMT_X][row-1], P->data[GMT_X][row], dlon);
		lon_sum += dlon;
		lat_sum += P->data[GMT_Y][row];
	}
	PH->pole = 0;
	if (gmt_M_360_range (lon_sum, 0.0)) {	/* Contains a pole */
		if (lat_sum < 0.0) { /* S */
			PH->pole = -1;
			PH->lat_limit = P->min[GMT_Y];
			P->min[GMT_Y] = -90.0;
			
		}
		else {	/* N */
			PH->pole = +1;
			PH->lat_limit = P->max[GMT_Y];
			P->max[GMT_Y] = 90.0;
		}
		P->min[GMT_X] = 0.0;	P->max[GMT_X] = 360.0;
	}
}

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SPHDISTANCE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct SPHDISTANCE_CTRL);
	C->E.dist = 1.0;	/* Default is 1 degree Voronoi edge resampling */
	C->L.unit = 'e';	/* Default is meter distances */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct SPHDISTANCE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->N.file);
	gmt_M_str_free (C->Q.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "==> The hard work is done by algorithms 772 (STRIPACK) & 773 (SSRFPACK) by R. J. Renka [1997] <==\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -G<outgrid> %s [-C] [-En|z|d[<dr>]]\n", name, GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-L<unit>] [-N<nodetable>] [-Q<voronoitable>] [%s] [%s] [%s] [%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s] [%s]\n\n", GMT_e_OPT, GMT_h_OPT, GMT_i_OPT, GMT_j_OPT, GMT_r_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-G Specify file name for output distance grid file.\n");
	GMT_Option (API, "I");

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more data file (in ASCII, binary, netCDF) with (x,y,z[,w]).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read (but see -Q).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Suppress connecting Voronoi arcs using great circles, i.e., connect by straight lines,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   unless m or p is appended to first follow meridian then parallel, or vice versa.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Conserve memory (Converts lon/lat <--> x/y/z when needed) [store both in memory]. Not used with -Q.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Specify the quantity that should be assigned to the grid nodes:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -En The Voronoi polygon ID.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ez The z-value of the Voronoi center node (natural NN gridding).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ed The distance to the nearest data point [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally append resampling interval in spherical degrees for polygon arcs [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set distance unit arc (d)egree, m(e)ter, (f)eet, (k)m, arc (m)inute, (M)ile, (n)autical mile,\n\t   or arc (s)econd [e].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Specify node filename for the Voronoi polygons (sphtriangulate -N output).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Specify table with Voronoi polygons in sphtriangulate -Qv format\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default performs Voronoi construction on input data first].\n");
	GMT_Option (API, "Rg");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   If no region is specified we default to the entire world [-Rg].\n");
	GMT_Option (API, "V,bi2,di,e,h,i,j,r,s,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SPHDISTANCE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to sphdistance and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = true;
				break;
			case 'D':
				if (gmt_M_compat_check (GMT, 4))
					GMT_Report (API, GMT_MSG_COMPAT, "-D option is deprecated; duplicates are automatically removed.\n");
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'E':
				Ctrl->E.active = true;
				switch (opt->arg[0]) {	/* Select output grid mode */
					case 'n': Ctrl->E.mode = SPHD_NODES;  k = 1;	break;
					case 'z': Ctrl->E.mode = SPHD_VALUES; k = 1;	break;
					case 'd': Ctrl->E.mode = SPHD_DIST;   k = 1;	break;
					default:  Ctrl->E.mode = SPHD_DIST;   k = 0;	break;
				}
				if (opt->arg[k]) Ctrl->E.dist = atof (&opt->arg[k]);
				break;
			case 'G':
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'L':
				Ctrl->L.active = true;
				if (!(opt->arg && strchr (GMT_LEN_UNITS, opt->arg[0]))) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -L%s\n", GMT_LEN_UNITS_DISPLAY);
					n_errors++;
				}
				else
					Ctrl->L.unit = opt->arg[0];
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.file = strdup (opt->arg);
				break;
			case 'Q':
				Ctrl->Q.active = true;
				Ctrl->Q.file = strdup (opt->arg);
				break;
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 3;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 3, "Syntax error: Binary input data (-bi) must have at least 3 columns\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && GMT->common.b.active[GMT_IN] && !Ctrl->N.active, "Syntax error: Binary input data (-bi) with -Q also requires -N.\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Syntax error -G: Must specify output file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_sphdistance (void *V_API, int mode, void *args) {
	bool first = false, periodic, duplicate_col;
	int error = 0, s_row, south_row, north_row, w_col, e_col;

	unsigned int row, col, p_col, west_col, east_col, nx1, n_in = 0;
	uint64_t n_dup = 0, n_set = 0, side, ij, node, n_new, n = 0;
	uint64_t vertex, node_stop, node_new, vertex_new, node_last, vertex_last;

	size_t n_alloc, p_alloc = 0;

	double first_x = 0.0, first_y = 0.0, prev_x = 0.0, prev_y = 0.0, X[3];
	double *grid_lon = NULL, *grid_lat = NULL, *in = NULL;
	double *xx = NULL, *yy = NULL, *zz = NULL, *lon = NULL, *lat = NULL;
	
	gmt_grdfloat f_val = 0.0, *z_val = NULL;

	struct GMT_GRID *Grid = NULL;
	struct GMT_RECORD *In = NULL;
	struct SPHDISTANCE_CTRL *Ctrl = NULL;
	struct STRIPACK T;
	struct GMT_DATASEGMENT *P = NULL;
	struct GMT_DATASET *Qin = NULL;
	struct GMT_DATATABLE *Table = NULL;
	struct STRIPACK_VORONOI *V = NULL;
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
	gmt_parse_common_options (GMT, "f", 'f', "g"); /* Implicitly set -fg since this is spherical triangulation */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the sphdistance main code ----------------------------*/

	gmt_M_memset (&T, 1, struct STRIPACK);

	gmt_init_distaz (GMT, Ctrl->L.unit, gmt_M_sph_mode (GMT), GMT_MAP_DIST);

	if (!GMT->common.R.active[RSET]) {	/* Default to a global grid */
		GMT->common.R.wesn[XLO] = 0.0;	GMT->common.R.wesn[XHI] = 360.0;	GMT->common.R.wesn[YLO] = -90.0;	GMT->common.R.wesn[YHI] = 90.0;
	}

	/* Now we are ready to take on some input values */

	if (Ctrl->Q.active) {	/* Expect a single file with Voronoi polygons */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Read Volonoi polygons from %s ...", Ctrl->Q.file);
		gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -Q files */
		if ((Qin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_READ_NORMAL, NULL, Ctrl->Q.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (Qin->n_columns < 2) {
			GMT_Report (API, GMT_MSG_NORMAL, "Input file %s has %d column(s) but at least 2 are needed\n", Ctrl->Q.file, (int)Qin->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
		Table = Qin->table[0];	/* Only one table in a file */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Found %" PRIu64 " segments\n", Table->n_segments);
	 	lon = gmt_M_memory (GMT, NULL, Table->n_segments, double);
	 	lat = gmt_M_memory (GMT, NULL, Table->n_segments, double);
		if (Ctrl->N.active) {	/* Must get nodes from separate file */
			struct GMT_DATASET *Nin = NULL;
			struct GMT_DATATABLE *NTable = NULL;
			if ((error = GMT_Set_Columns (API, GMT_IN, 3, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
				Return (error);
			}
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Read Nodes from %s ...", Ctrl->N.file);
			gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -N files */
			if ((Nin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_READ_NORMAL, NULL, Ctrl->N.file, NULL)) == NULL) {
				Return (API->error);
			}
			if (Nin->n_columns < 2) {
				GMT_Report (API, GMT_MSG_NORMAL, "Input file %s has %d column(s) but at least 2 are needed\n", Ctrl->N.file, (int)Nin->n_columns);
				Return (GMT_DIM_TOO_SMALL);
			}
			gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
			NTable = Nin->table[0];	/* Only one table in a file with a single segment */
			if (NTable->n_segments != 1) {
				GMT_Report (API, GMT_MSG_NORMAL, "File %s can only have 1 segment!\n", Ctrl->N.file);
				Return (GMT_RUNTIME_ERROR);
			}
			if (Table->n_segments != NTable->n_records) {
				GMT_Report (API, GMT_MSG_NORMAL, "Files %s and %s do not have same number of items!\n", Ctrl->Q.file, Ctrl->N.file);
				Return (GMT_RUNTIME_ERROR);
			}
			gmt_M_memcpy (lon, NTable->segment[0]->data[GMT_X], NTable->n_records, double);
			gmt_M_memcpy (lat, NTable->segment[0]->data[GMT_Y], NTable->n_records, double);
			if (GMT_Destroy_Data (API, &Nin) != GMT_NOERROR) {
				Return (API->error);
			}
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Found %" PRIu64 " records\n", NTable->n_records);
		}
		else {	/* Get extract them from the segment header */
			for (node = 0; node < Table->n_segments; node++) {
				if (Table->segment[node]->header == NULL) {
					GMT_Report (API, GMT_MSG_NORMAL, "No node-information found in the segment headers - must abort\n");
					Return (GMT_RUNTIME_ERROR);
				}
				if (sscanf (Table->segment[node]->header, "%*s %*d %lf %lf", &lon[node], &lat[node]) != 2) {
					GMT_Report (API, GMT_MSG_NORMAL, "Could not obtain node-information from the segment headers - must abort\n");
					Return (GMT_RUNTIME_ERROR);
				}
			}
		}
	}
	else {	/* Must process input point/line data */
		n_in = (Ctrl->E.mode == SPHD_VALUES) ? 3 : 2;
		if ((error = GMT_Set_Columns (API, GMT_IN, n_in, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {
			Return (API->error);	/* Enables data input and sets access mode */
		}

		GMT->session.min_meminc = GMT_INITIAL_MEM_ROW_ALLOC;	/* Start by allocating a 32 Mb chunk */ 

		n_alloc = 0;
		if (!Ctrl->C.active) gmt_M_malloc2 (GMT, lon, lat, 0, &n_alloc, double);
		n_alloc = 0;
		gmt_M_malloc3 (GMT, xx, yy, zz, 0, &n_alloc, double);
		if (Ctrl->E.mode == SPHD_VALUES) z_val = gmt_M_memory (GMT, NULL, n_alloc, gmt_grdfloat);

		n = 0;
		do {	/* Keep returning records until we reach EOF */
			if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
					if (Ctrl->E.mode == SPHD_VALUES) gmt_M_free (GMT, z_val);
					if (!Ctrl->C.active) {gmt_M_free (GMT, lon);	gmt_M_free (GMT, lat);}
					if (!Ctrl->Q.active) {gmt_M_free (GMT, xx);	gmt_M_free (GMT, yy);	gmt_M_free (GMT, zz);}
					Return (GMT_RUNTIME_ERROR);
				}
				else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
					break;
				else if (gmt_M_rec_is_segment_header (GMT))			/* Parse segment headers */
					first = true;
				continue;	/* Go back and read the next record */
			}

			/* Data record to process - avoid duplicate points as gmt_stripack_lists cannot handle that */
			in = In->data;	/* Only need to process numerical part here */

			if (first) {	/* Beginning of new segment; keep track of the very first coordinate in case of duplicates */
				first_x = prev_x = in[GMT_X];	first_y = prev_y = in[GMT_Y];
			}
			else {	/* Look for duplicate point at end of segments that replicate start point */
				if (in[GMT_X] == first_x && in[GMT_Y] == first_y) {	/* If any point after the first matches the first */
					n_dup++;
					continue;
				}
				if (n && in[GMT_X] == prev_x && in[GMT_Y] == prev_y) {	/* Identical neighbors */
					n_dup++;
					continue;
				}
				prev_x = in[GMT_X];	prev_y = in[GMT_Y];
			}

			/* Convert lon,lat in degrees to Cartesian x,y,z triplets */
			gmt_geo_to_cart (GMT, in[GMT_Y], in[GMT_X], X, true);

			xx[n] = X[GMT_X];	yy[n] = X[GMT_Y];	zz[n] = X[GMT_Z];
			if (!Ctrl->C.active) {
				lon[n] = in[GMT_X];	lat[n] = in[GMT_Y];
			}
			if (Ctrl->E.mode == SPHD_VALUES) z_val[n] = (gmt_grdfloat)in[GMT_Z];

			if (++n == n_alloc) {	/* Get more memory */
				if (!Ctrl->C.active) { size_t n_tmp = n_alloc; gmt_M_malloc2 (GMT, lon, lat, n, &n_tmp, double); }
				gmt_M_malloc3 (GMT, xx, yy, zz, n, &n_alloc, double);
				if (Ctrl->E.mode == SPHD_VALUES) z_val = gmt_M_memory (GMT, z_val, n_alloc, gmt_grdfloat);
			}
			first = false;
		} while (true);

		n_alloc = n;
		if (!Ctrl->C.active) gmt_M_malloc2 (GMT, lon, lat, 0, &n_alloc, double);
		gmt_M_malloc3 (GMT, xx, yy, zz, 0, &n_alloc, double);

		if (n_dup) GMT_Report (API, GMT_MSG_VERBOSE, "Skipped %" PRIu64 " duplicate points in segments\n", n_dup);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Do Voronoi construction using %" PRIu64 " points\n", n);

		T.mode = VORONOI;
		gmt_stripack_lists (GMT, n, xx, yy, zz, &T);	/* Do the basic triangulation */
		gmt_M_free (GMT, T.D.tri);	/* Don't need the triangulation */
		if (Ctrl->C.active) {	/* Recompute lon,lat and set pointers */
			gmt_n_cart_to_geo (GMT, n, xx, yy, zz, xx, yy);	/* Revert to lon, lat */
			lon = xx;
			lat = yy;
		}
		gmt_M_free (GMT,  zz);
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}
		GMT->session.min_meminc = GMT_MIN_MEMINC;		/* Reset to the default value */
	}

	/* OK, time to create and work on the distance grid */

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Start processing distance grid\n");

	grid_lon = Grid->x;
	grid_lat = Grid->y;

	nx1 = (Grid->header->registration == GMT_GRID_PIXEL_REG) ? Grid->header->n_columns : Grid->header->n_columns - 1;
	periodic = gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]);
	duplicate_col = (periodic && Grid->header->registration == GMT_GRID_NODE_REG);	/* E.g., lon = 0 column should match lon = 360 column */
	gmt_set_inside_mode (GMT, NULL, GMT_IOO_SPHERICAL);

	if (Ctrl->Q.active)	/* Pre-chewed, just get number of nodes */
		n = Table->n_segments;
	else
		V = &T.V;

	for (node = 0; node < n; node++) {
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing polygon %7ld\r", node);
		if (Ctrl->Q.active) {	/* Just point to next polygon */
			P = Table->segment[node];
		}
		else {	/* Obtain current polygon from Voronoi listings */
			if (P == NULL) {	/* Need a single polygon structure that we reuse for each polygon */
				P = gmt_get_segment (GMT);	/* Needed as pointer below */
				P->data = gmt_M_memory (GMT, NULL, 2, double *);	/* Needed as pointers below */
				P->min = gmt_M_memory (GMT, NULL, 2, double);	/* Needed to hold min lon/lat */
				P->max = gmt_M_memory (GMT, NULL, 2, double);	/* Needed to hold max lon/lat */
				P->n_columns = 2;	p_alloc = 0;
				gmt_M_malloc2 (GMT, P->data[GMT_X], P->data[GMT_Y], GMT_TINY_CHUNK, &p_alloc, double);
			}
			node_new = node_stop = V->lend[node];
			vertex_new = V->listc[node_new];

			/* Each iteration of this do-loop walks along one side of the polygon,
			   considering the subtriangle NODE --> VERTEX_LAST --> VERTEX. */

			vertex = 0;
		    	do {
				node_last = node_new;
				node_new = V->lptr[node_last];
				vertex_last = vertex_new;
				vertex_new = V->listc[node_new];

				P->data[GMT_X][vertex] = V->lon[vertex_last];
				P->data[GMT_Y][vertex] = V->lat[vertex_last];
				if (P->data[GMT_X][vertex] < 0.0) P->data[GMT_X][vertex] += 360.0;
				if (P->data[GMT_X][vertex] == 360.0) P->data[GMT_X][vertex] = 0.0;
				vertex++;
				if (vertex == p_alloc) gmt_M_malloc2 (GMT, P->data[GMT_X], P->data[GMT_Y], vertex, &p_alloc, double);

				/* When we reach the vertex where we started, we are done with this polygon */
			} while (node_new != node_stop);
			P->data[GMT_X][vertex] = P->data[GMT_X][0];	/* Close polygon explicitly */
			P->data[GMT_Y][vertex] = P->data[GMT_Y][0];
			if ((++vertex) == p_alloc) gmt_M_malloc2 (GMT, P->data[GMT_X], P->data[GMT_Y], vertex, &p_alloc, double);
			P->n_rows = vertex;
			switch (Ctrl->E.mode) {
				case SPHD_NODES:	f_val = (gmt_grdfloat)node;	break;
				case SPHD_VALUES:	f_val = z_val[node];	break;
				default:	break;	/* Must compute distances below */
			}
			
		}

		/* Here we have the polygon in P */

		if ((n_new = gmt_fix_up_path (GMT, &P->data[GMT_X], &P->data[GMT_Y], P->n_rows, Ctrl->E.dist, GMT_STAIRS_OFF)) == 0) {
			gmt_M_free (GMT, P);
			Return (GMT_RUNTIME_ERROR);
		}
		P->n_rows = n_new;
		prepare_polygon (GMT, P);	/* Determine the enclosing sector */

		south_row = (int)gmt_M_grd_y_to_row (GMT, P->min[GMT_Y], Grid->header);
		north_row = (int)gmt_M_grd_y_to_row (GMT, P->max[GMT_Y], Grid->header);
		w_col  = (int)gmt_M_grd_x_to_col (GMT, P->min[GMT_X], Grid->header);
		while (w_col < 0) w_col += nx1;
		west_col = w_col;
		e_col = (int)gmt_M_grd_x_to_col (GMT, P->max[GMT_X], Grid->header);
		while (e_col < w_col) e_col += nx1;
		east_col = e_col;
		/* So here, any polygon will have a positive (or 0) west_col with an east_col >= west_col */
		for (s_row = north_row; s_row <= south_row; s_row++) {	/* For each scanline intersecting this polygon */
			if (s_row < 0) continue;	/* North of region */
			row = s_row; if (row >= Grid->header->n_rows) continue;	/* South of region */
			for (p_col = west_col; p_col <= east_col; p_col++) {	/* March along the scanline using col >= 0 */
				if (p_col >= Grid->header->n_columns) {	/* Off the east end of the grid */
					if (periodic)	/* Just shuffle to the corresponding point inside the global grid */
						col = p_col - nx1;
					else		/* Sorry, really outside the region */
						continue;
				}
				else
					col = p_col;
				side = gmt_inonout (GMT, grid_lon[col], grid_lat[row], P);
				
				if (side == 0) continue;	/* Outside spherical polygon */
				ij = gmt_M_ijp (Grid->header, row, col);
				if (Ctrl->E.mode == SPHD_DIST)
					f_val = (gmt_grdfloat)gmt_distance (GMT, grid_lon[col], grid_lat[row], lon[node], lat[node]);
				Grid->data[ij] = f_val;
				n_set++;
				if (duplicate_col) {	/* Duplicate the repeating column on the other side of this one */
					if (col == 0) Grid->data[ij+nx1] = Grid->data[ij], n_set++;
					else if (col == nx1) Grid->data[ij-nx1] = Grid->data[ij], n_set++;
				}
			}
		}
	}
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing polygon %7ld\n", node);

	if (!Ctrl->Q.active) {
		gmt_free_segment (GMT, &P);
		gmt_M_free (GMT, T.V.lon);
		gmt_M_free (GMT, T.V.lat);
		gmt_M_free (GMT, T.V.lend);
		gmt_M_free (GMT, T.V.listc);
		gmt_M_free (GMT, T.V.lptr);
		gmt_M_free (GMT, xx);
		gmt_M_free (GMT, yy);
	}
	if (!Ctrl->C.active) {
		gmt_M_free (GMT, lon);
		gmt_M_free (GMT, lat);
	}
	if (Ctrl->E.mode == SPHD_VALUES) gmt_M_free (GMT, z_val);

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
		Return (API->error);
	}

	if (n_set > Grid->header->nm) n_set = Grid->header->nm;	/* Not confuse the public */
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Spherical distance calculation completed, %" PRIu64 " nodes visited (at least once)\n", n_set);

	Return (GMT_NOERROR);
}
