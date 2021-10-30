/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Author:	Paul Wessel
 * Date:	1-NOV-2021
 * Version:	6 API
 *
 * Brief synopsis: grdselect reads one or more grid/cube files and determines the
 * union or intersection of the regions, modulated by other options. Alternatively
 * it just writes the names of the data sources that pass the tests.
 *
 * Notes: IDeas for other features:
 * -F<pol> Only consider grids that overlap this polygon
 * -L<line> Only consider grids that are crossed by this line
 * -P<point> Only consider grids that contain this point
 * 
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdselect"
#define THIS_MODULE_MODERN_NAME	"grdselect"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Determine common regions from 2-D grids or 3-D cubes"
#define THIS_MODULE_KEYS	"<?{+,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->RVfhor"

/* Control structure for grdselect */

enum Opt_A_modes {
	GRDSELECT_INTERSECTION	= 0,	/* -Ai */
	GRDSELECT_UNION			= 1,	/* -Au */
	GRDSELECT_NO_INC	= 0,
	GRDSELECT_MIN_INC	= 1,	/* +il */
	GRDSELECT_MAX_INC	= 2,	/* +ih */
	GRDSELECT_SET_INC	= 3};	/* +i<incs> */

enum Opt_N_modes {
	GRDSELECT_LESS_NANS	= 1,	/* -Nl */
	GRDSELECT_MORE_NANS	= 2};	/* -Nh */

enum GRDSELECT {	/* Indices for the various tests */
	GRD_SELECT_r = 0,
	GRD_SELECT_D,
	GRD_SELECT_N,
	GRD_SELECT_W,
	GRD_SELECT_Z,
	GRD_SELECT_N_TESTS	/* Number of specific tests available */
};

#define WLO	6
#define WHI	7

struct GRDSELECT_CTRL {
	unsigned int n_files;	/* How many data sources given */
	struct GRDSELECT_A {	/* -A[i|u][+il|h|arg>]*/
		bool active;
		bool round;		/* True if +i is used to set rounding */
		unsigned int i_mode;	/* 0 = no increment rounding, 1 is use smallest inc, 2 = use largest inc, 3 = use appended inc */
		unsigned int mode;		/* 0 = intersection [i], 1 = union [r] */
		double inc[2];	/* Optional increments */
	} A;
	struct GRDSELECT_C {	/* -C[b] */
		bool active;
		bool box;
	} C;
	struct GRDSELECT_D {	/* -D<dx[/dy[/dz]] */
		bool active;
		double inc[3];
	} D;
	struct GRDSELECT_G {	/* -G */
		bool active;
	} G;
	struct GRDSELECT_I {	/* -Idnrwz */
		bool active;
		bool pass[GRD_SELECT_N_TESTS];	/* One flag for each setting */
	} I;
	struct GRDSELECT_M {	/* -M */
		bool active;
		double margin[4];	/* Optional margins [none] */
	} M;
	struct GRDSELECT_N {	/* -Nl|h<nans> */
		bool active;
		unsigned int mode;
		int64_t n_nans;	/* Limit on nans [0] */
	} N;
	struct GRDSELECT_Q {	/* -Q */
		bool active;
	} Q;
	struct GRDSELECT_W {	/* -Wwmin/wmax */
		bool active;
		double w_min, w_max;
	} W;
	struct GRDSELECT_Z {	/* -Zzmin/zmax */
		bool active;
		double z_min, z_max;
	} Z;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDSELECT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDSELECT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	for (unsigned int k = 0; k < GRD_SELECT_N_TESTS; k++) C->I.pass[k] = true;    /* Default is to include the grid if we pass the test */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDSELECT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s grid1 grid2 ... [-Ai|u[+il|h|<inc>]] [-C] [-D<inc>] [-G] [-Idnrwz] [-M<margins>] [-Nl|h[<n>]] "
		"[-Q] [%s] [-W[<min>]/[<max>]] [-Z[<min>]/[<max>]] [%s] [%s] [%s] [%s] [%s]\n", name, GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT, GMT_ho_OPT, GMT_o_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	gmt_ingrid_syntax (API, 0, "Name of one or more grid or cube files");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-Ai|u[+il|h|<inc>]");
	GMT_Usage (API, -2, "Report a unifying data region for all the data sources. Append:");
	GMT_Usage (API, 3, "i: Determine the intersection of all passed data source regions.");
	GMT_Usage (API, 3, "u: Determine the union of all passed data source regions.");
	GMT_Usage (API, -2, "Optionally, append +i to round region using the (l)owest, (h)ighest, or specified increments [no rounding]." 
		"Note: Without -A we simply report the list of data sources passing any imposed tests.");
	GMT_Usage (API, 1, "\n-C Report information in fields on a single line using the format "
		"<w e s n {b t} w0 w1> [Default reports a -R<w/e/s/n>[/b/t] string].");
	GMT_Usage (API, 1, "\n-D<inc>");
	GMT_Usage (API, -2, "Only consider data sources that match the given increments [Consider all input data sources].");
	GMT_Usage (API, 1, "\n-G Force possible download of all tiles for a remote <grid> if given as input [no report for tiled grids].");
	GMT_Usage (API, 1, "\n-Idnrwz");
	GMT_Usage (API, -2, "Reverse the tests, i.e., pass data sources when the test fails. "
		"Supply any combination of dnrwz where each flag means:");
	GMT_Usage (API, 3, "d: Pass data sources that do not match the increment set in -D.");
	GMT_Usage (API, 3, "n: Pass data sources that fail the NaN-test in -N.");
	GMT_Usage (API, 3, "r: Pass data sources with the opposite registration than given in -r.");
	GMT_Usage (API, 3, "w: Pass data sources outside the data range given in -W.");
	GMT_Usage (API, 3, "z: Pass cubes outside the z-coordinate range given in -Z (requires -Q).");
	GMT_Usage (API, -2, "Note: If no argument is given then we default to -Idnrwz.");
	GMT_Usage (API, 1, "\n-M<margins>");
	GMT_Usage (API, -2, "Add padding around the final (rounded) region. Append a uniform <margin>, separate <xmargin>/<ymargin>, "
		"or individual <wmargin>/<emargin>/<smargin>/<nmargin> for each side [no padding].");
	GMT_Usage (API, 1, "\n-Nl|h[<n>]");
	GMT_Usage (API, -2, "Only consider data sources that satisfy a NaN-condition [Consider all input data sources]:");
	GMT_Usage (API, 3, "l: Only data sources with lower than <n> NaNs will pass [0].");
	GMT_Usage (API, 3, "h: Only data sources with higher than <n>  NaNs will pass [0].");
	GMT_Usage (API, 1, "\n-Q Input file(s) is 3-D data cube(s), not grid(s) [2-D grids].");
	GMT_Option (API, "R");
	GMT_Usage (API, 1, "\n-W[<min>]/[<max>]");
	GMT_Usage (API, -2, "Only consider data sources that have data-values in the given range [Consider all input data sources]. "
		"At least one of <min> or <max> must be specified, as well as the slash [-infinity/+infinity].");
	GMT_Usage (API, 1, "\n-Z[<min>]/[<max>]");
	GMT_Usage (API, -2, "Only consider cubes that have z-coordinates in the given range [Consider all input cubes]. "
		"At least one of <min> or <max> must be specified, as well as the slash [-infinity/+infinity]. Requires -Q.");
	GMT_Usage (API, 1, "\n-r[g|p]");
	GMT_Usage (API, -2, "Only consider data sources that have the specified registration [Consider all input data sources].");
	GMT_Option (API, "V,f,h,o,r,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDSELECT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdselect and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k = 0;
	char string[GMT_LEN128] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input files */
				if (GMT_Get_FilePath (API, GMT_IS_GRID, GMT_IN, GMT_FILE_REMOTE, &(opt->arg)))
					n_errors++;
				else
					Ctrl->n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Area comparisons */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				Ctrl->A.active = true;
				switch (opt->arg[0]) {
					case 'u': Ctrl->A.mode = GRDSELECT_UNION; k = 1; break;
					case 'i': Ctrl->A.mode = GRDSELECT_INTERSECTION; k = 1; break;
					default: k = 0;	break;	/* No directive, default to intersection */
				}
				if (gmt_get_modifier (opt->arg, 'i', string)) {	/* Want to control rounding */
					Ctrl->A.round = true;
					switch (string[0]) {
						case 'l': Ctrl->A.i_mode = GRDSELECT_MIN_INC; break;
						case 'h': Ctrl->A.i_mode = GRDSELECT_MAX_INC; break;
						default:
							if (gmt_getinc (GMT, string, Ctrl->A.inc)) n_errors++;
							Ctrl->A.i_mode = GRDSELECT_SET_INC;
							break;
					}
				}
				break;

			case 'C':	/* Column format */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				if (opt->arg[0] == 'b') Ctrl->C.box = true;
				break;
			case 'D':	/* Specified grid increments */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				Ctrl->D.active = true;
				if (opt->arg[0] && gmt_getinc (GMT, opt->arg, Ctrl->D.inc)) {
					gmt_inc_syntax (GMT, 'D', 1);
					n_errors++;
				}
				break;
			case 'I':	/* Invert selected tests */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				if (opt->arg[0] == '\0') {   /* If -I is given then all tests are reversed */
					for (k = 0; k < GRD_SELECT_N_TESTS; k++) Ctrl->I.pass[k] = false;
				}
				else {	/* Reverse those requested only */
					for (k = 0; opt->arg[k]; k++) {
						switch (opt->arg[k]) {
							case 'd': Ctrl->I.pass[GRD_SELECT_D] = false; break;
							case 'n': Ctrl->I.pass[GRD_SELECT_N] = false; break;
							case 'r': Ctrl->I.pass[GRD_SELECT_r] = false; break;
							case 'w': Ctrl->I.pass[GRD_SELECT_W] = false; break;
							case 'z': Ctrl->I.pass[GRD_SELECT_Z] = false; break;
							default:
								GMT_Report (API, GMT_MSG_ERROR, "Option -I: Expects any combination of dnrwz (%c is not valid)\n", opt->arg[k]);
								n_errors++;
								break;
						}
					}
				}
				break;
			case 'M':	/* Extend the region */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				Ctrl->M.active = true;
				k = GMT_Get_Values (GMT->parent, opt->arg, Ctrl->M.margin, 4);
				if (k == 1)	/* Same increments in all directions */
					Ctrl->M.margin[XHI] = Ctrl->M.margin[YLO] = Ctrl->M.margin[YHI] = Ctrl->M.margin[XLO];
				else if (k == 2) {	/* Separate increments in x and y */
					Ctrl->M.margin[YLO] = Ctrl->M.margin[YHI] = Ctrl->M.margin[XHI];
					Ctrl->M.margin[XHI] = Ctrl->M.margin[XLO];
				}
				else if (k != 4) {	/* The only other option is 4 but somehow we failed */
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -M: Bad number of increments given (%s)\n", opt->arg);
					n_errors++;
				}
				break;
			case 'N':	/* NaN condition */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				Ctrl->N.active = true;
				switch (opt->arg[0]) {
					case 'l':	Ctrl->N.mode = GRDSELECT_LESS_NANS;	break;
					case 'h':	Ctrl->N.mode = GRDSELECT_MORE_NANS;	break;
					default:
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -M: Bad directive, must be -Ml or -Mh\n");
						n_errors++;
				}
				if (opt->arg[1]) Ctrl->N.n_nans = atoi (&opt->arg[1]);
				break;

			case 'Q':	/* Expect cubes */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				Ctrl->Q.active = true;
				break;
			case 'Z':	/* z range */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				Ctrl->Z.active = true;
				n_errors += gmt_get_limits (GMT, 'Z', opt->arg, 1, &Ctrl->Z.z_min, &Ctrl->Z.z_max);
				break;

			case 'W':	/* w range */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				Ctrl->W.active = true;
				n_errors += gmt_get_limits (GMT, 'W', opt->arg, 1, &Ctrl->W.w_min, &Ctrl->W.w_max);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->n_files == 0,
	                                   "Must specify one or more input files\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.i_mode ==  GRDSELECT_SET_INC && (Ctrl->A.inc[GMT_X] <= 0.0 || Ctrl->A.inc[GMT_Y] <= 0.0),
									   "Option -A: Must specify a positive increment(s) via +i<inc>\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && (Ctrl->D.inc[GMT_X] <= 0.0 || Ctrl->D.inc[GMT_Y] <= 0.0),
									   "Option -D: Must specify a positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.o.active && Ctrl->C.active,
	                                   "The -o option requires -C\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && Ctrl->N.n_nans < 0,
	                                   "The -N option argument cannot be negative\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && !Ctrl->Q.active,
	                                   "The -Z option requires -Q since the limits applies to the 3-D z-dimension (see -W for data range limits)\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_set_pad (GMT, GMT_PAD_DEFAULT); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grdselect (void *V_API, int mode, void *args) {
	int error = 0, k_data, k_tile_id;
	unsigned int n_cols = 0, cmode = GMT_COL_FIX, geometry = GMT_IS_TEXT;
	bool first_r = true, first = true, is_cube, pass;

	double wesn[8], out[8], *subset = NULL;

	char record[GMT_BUFSIZ] = {""};
	static char *type[2] = {"grid", "cube"};

	struct GRDSELECT_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL;
	struct GMT_GRID_HEADER *header = NULL;
	struct GMT_CUBE *U = NULL;
	struct GMT_RECORD *Out = NULL;
	struct GMT_OPTION *opt = NULL;
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

	/*---------------------------- This is the grdselect main code ----------------------------*/

	/* OK, done parsing, now process all input data sources in a loop */

	gmt_M_memset (wesn, 8, double);	/* Initialize */
	gmt_M_memset (out, 8, double);	/* Initialize */

	if (Ctrl->C.active) {
		if (Ctrl->C.box) {	/* Write polygon */
			n_cols = 2;
			cmode = GMT_COL_FIX_NO_TEXT;
			geometry = GMT_IS_POLYGON;
		}
		else {
			n_cols = (Ctrl->Q.active) ? 8 : 6;	/* w e s n [z0 z1] [w0 w1] */
			cmode = GMT_COL_FIX_NO_TEXT;
			geometry = GMT_IS_NONE;
		}
	}

	GMT_Set_Columns (GMT->parent, GMT_OUT, n_cols, cmode);

	if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		Return (API->error);
	}

	Out = gmt_new_record (GMT, (n_cols) ? out : NULL, (cmode) == GMT_COL_FIX ? record : NULL);	/* the two items */

	gmt_set_pad (API->GMT, 0);	/* Not read pads to simplify operations */

	if (GMT->common.R.active[RSET]) {	/* Gave a sub region so set the first region limit */
		gmt_M_memcpy (wesn, GMT->common.R.wesn, 6U, double);
		first_r = false;
		subset = GMT->common.R.wesn;	/* Read subsets */
	}
	wesn[ZLO] = wesn[WLO] = DBL_MAX;	wesn[ZHI] = wesn[WHI] = -DBL_MAX;

	for (opt = options; opt; opt = opt->next) {	/* Loop over arguments, skip options */

		if (opt->option != '<') continue;	/* We are only processing filenames here */

		gmt_set_cartesian (GMT, GMT_IN);	/* Reset since we may get a bunch of files, some geo, some not */
		k_tile_id = k_data = GMT_NOTSET;
		if ((k_data = gmt_remote_dataset_id (API, opt->arg)) != GMT_NOTSET || (k_tile_id = gmt_get_tile_id (API, opt->arg)) != GMT_NOTSET) {
			if (k_tile_id != GMT_NOTSET && !Ctrl->G.active) {
				GMT_Report (API, GMT_MSG_WARNING, "Information on tiled remote global grids requires -G since download or all tiles may be required\n");
				continue;
			}
			gmt_set_geographic (GMT, GMT_IN);	/* Since this will be returned as a memory grid */
		}

		is_cube = gmt_nc_is_cube (API, opt->arg);	/* Determine if this file is a cube or not */
		if (Ctrl->Q.active != is_cube) {
			if (is_cube)
				GMT_Report (API, GMT_MSG_WARNING, "Detected a data cube (%s) but -Q not set - skipping\n", opt->arg);
			else
				GMT_Report (API, GMT_MSG_WARNING, "Detected a data grid (%s) but -Q is set - skipping\n", opt->arg);
			continue;
		}
		if (is_cube) {
			if ((U = GMT_Read_Data (API, GMT_IS_CUBE, GMT_IS_FILE, GMT_IS_VOLUME, GMT_CONTAINER_ONLY, NULL, opt->arg, NULL)) == NULL) {
				Return (API->error);
			}
			header = U->header;
		}
		else {
			if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, opt->arg, NULL)) == NULL) {
				Return (API->error);
			}
			header = G->header;
		}

		if (GMT->common.R.active[GSET]) {	/* Specified -r to only keep those data sources with the same registration */
			pass = (header->registration == GMT->common.R.registration);
			if (pass != Ctrl->I.pass[GRD_SELECT_r]) continue;	/* Skip if wrong (or right via -Ir) registration */
		}
		if (Ctrl->D.active) {	/* Only pass data sources with the same increments */
			pass = (doubleAlmostEqual (Ctrl->D.inc[GMT_X], header->inc[GMT_X]) && doubleAlmostEqual (Ctrl->D.inc[GMT_Y], header->inc[GMT_Y]));
			if (pass != Ctrl->I.pass[GRD_SELECT_D]) continue;	/* Skip if grid spacing is different (or the same via -Id) */
		}
		if (Ctrl->W.active) {	/* Skip data sources outside the imposed w-range */
			pass = !(header->z_max < Ctrl->W.w_min || header->z_min > Ctrl->W.w_max);
			if (pass != Ctrl->I.pass[GRD_SELECT_W]) continue;	/* Skip if outside (or inside via -Iw) the range */
		}
		if (Ctrl->Z.active) {	/* Skip cubes outside the imposed z-range */
			pass = !(U->z[header->n_bands-1] < Ctrl->Z.z_min || U->z[0] > Ctrl->Z.z_max);
			if (pass != Ctrl->I.pass[GRD_SELECT_Z]) continue;	/* Skip if outside (or inside via -Iz) the range */
		}
		if (Ctrl->N.active) {	/* Must read the data to know how many NaNs, then skip data sources that fail the test (or pass if -In) */
			uint64_t level, here = 0, ij, n_nan = 0;
			gmt_grdfloat *data = NULL;
			if (is_cube) {
				if (GMT_Read_Data (API, GMT_IS_CUBE, GMT_IS_FILE, GMT_IS_VOLUME, GMT_DATA_ONLY, subset, opt->arg, U) == NULL) {
					Return (API->error);
				}
			}
			else {
				if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, subset, opt->arg, G) == NULL) {
					Return (API->error);
				}
			}
			data = (is_cube) ? U->data : G->data;

			for (level = 0; level < header->n_bands; level++) {
				for (ij = 0; ij < header->size; ij++) {
					if (gmt_M_is_fnan (data[ij + here])) n_nan++;
				}
				here += header->size;
			}
			if (is_cube && GMT_Destroy_Data (API, &U) != GMT_NOERROR) {
				Return (API->error);
			}
			else if (!is_cube && GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
				Return (API->error);
			}
			pass = true;
			if (Ctrl->N.mode == GRDSELECT_LESS_NANS && n_nan > Ctrl->N.n_nans) pass = false;	/* Skip this item */
			else if (Ctrl->N.mode == GRDSELECT_MORE_NANS && n_nan < Ctrl->N.n_nans) pass = false;	/* Skip this item */
			if (pass != Ctrl->I.pass[GRD_SELECT_N]) continue;	/* Skip if outside (or inside via +i) the range */
		}

		/* OK, here the grid/cube passed any other obstacles */
		if (first_r) {	/* No w/e/s/n initialized via -R, use the first header to initialize wesn */
			gmt_M_memcpy (wesn, header->wesn, 4U, double);	/* The first grid needs to set the extent of the region for now */
			first_r = false;
		}
		if (first) {	/* This is the first grid/cube */
			if (Ctrl->Q.active) {	/* Cube: Keep track of z-dimension range */
				wesn[ZLO] = U->z[0];
				wesn[ZHI] = U->z[header->n_bands-1];
			}
			/* Set the initial data range */
			wesn[WLO] = header->z_min;
			wesn[WHI] = header->z_max;
			if (Ctrl->A.i_mode == GRDSELECT_MIN_INC || Ctrl->A.i_mode == GRDSELECT_MAX_INC)	/* Copy the first grid increments */
				gmt_M_memcpy (Ctrl->A.inc, header->inc, 2U, double);
			first = false;
		}
		if (Ctrl->A.active) {	/* Want region information */
			if (Ctrl->A.mode == GRDSELECT_INTERSECTION) {	/* Retain only region common to all data sources (i.e., their intersection) */
				wesn[YLO] = MAX (wesn[YLO], header->wesn[YLO]);
				wesn[YHI] = MIN (wesn[YHI], header->wesn[YHI]);
				if (wesn[YHI] <= wesn[YLO]) {	/* No common region can be found - no point to continue */
					GMT_Report (API, GMT_MSG_WARNING, "No common area for all %ss is possible.\n", type[is_cube]);
					goto nothing;
				}
				if (is_cube) {
					wesn[ZLO] = MAX (wesn[ZLO], U->z[0]);
					wesn[ZHI] = MIN (wesn[ZHI], U->z[header->n_bands-1]);
					if (wesn[ZHI] <= wesn[ZLO]) {	/* No common cube region can be found - no point to continue */
						GMT_Report (API, GMT_MSG_WARNING, "No common area for all cubes is possible.\n");
						goto nothing;
					}
				}
				if (gmt_M_is_cartesian (GMT, GMT_IN)) {	/* Easy, peasy */
					wesn[XLO] = MAX (wesn[XLO], header->wesn[XLO]);
					wesn[XHI] = MIN (wesn[XHI], header->wesn[XHI]);
				}
				else {	/* Must worry about 360 wrapping */
					double w = header->wesn[XLO] - 720.0, e = header->wesn[XHI] - 720.0;	/* Ensure we are far west */
					while (e < wesn[XLO]) w += 360.0, e += 360.0;	/* Wind to the west */
					wesn[XLO] = MAX (wesn[XLO], w);
					wesn[XHI] = MIN (wesn[XHI], e);
				}
				if (wesn[XHI] <= wesn[XLO]) {	/* No common region can be found - no point to continue */
					GMT_Report (API, GMT_MSG_WARNING, "No common area for all %ss is possible.\n", type[is_cube]);
					goto nothing;
				}
			}
			else {	/* Find the union of all regions */
				wesn[YLO] = MIN (wesn[YLO], header->wesn[YLO]);
				wesn[YHI] = MAX (wesn[YHI], header->wesn[YHI]);
				if (gmt_M_is_cartesian (GMT, GMT_IN)) {	/* Easy, peasy */
					wesn[XLO] = MIN (wesn[XLO], header->wesn[XLO]);
					wesn[XHI] = MAX (wesn[XHI], header->wesn[XHI]);
				}
				else {	/* Must worry about 360 wrapping */
					double w = header->wesn[XLO] - 720.0, e = header->wesn[XHI] - 720.0;	/* Ensure we are far west */
					while (e < wesn[XLO]) w += 360.0, e += 360.0;	/* Wind to the west */
					wesn[XLO] = MIN (wesn[XLO], w);
					wesn[XHI] = MAX (wesn[XHI], e);
				}
				if (is_cube) {
					wesn[ZLO] = MIN (wesn[ZLO], U->z[0]);
					wesn[ZHI] = MAX (wesn[ZHI], U->z[header->n_bands-1]);
				}
			}
			wesn[WLO] = MIN (wesn[WLO], header->z_min);
			wesn[WHI] = MAX (wesn[WHI], header->z_max);
			if (Ctrl->A.i_mode == GRDSELECT_MIN_INC) {	/* Update the smallest increments found */
				if (header->inc[GMT_X] < Ctrl->A.inc[GMT_X]) Ctrl->A.inc[GMT_X] = header->inc[GMT_X];	/* Update the smallest x-increments found */
				if (header->inc[GMT_Y] < Ctrl->A.inc[GMT_Y]) Ctrl->A.inc[GMT_Y] = header->inc[GMT_Y];	/* Update the smallest y-increments found */
			}
			if (Ctrl->A.i_mode == GRDSELECT_MAX_INC) {	/* Update the largest increments found */
				if (header->inc[GMT_X] > Ctrl->A.inc[GMT_X]) Ctrl->A.inc[GMT_X] = header->inc[GMT_X];	/* Update the largest x-increments found */
				if (header->inc[GMT_Y] > Ctrl->A.inc[GMT_Y]) Ctrl->A.inc[GMT_Y] = header->inc[GMT_Y];	/* Update the largest y-increments found */
			}
		}
		else {	/* Only wanted to list the grid files that passed the tests */
			sprintf (record, "%s", opt->arg);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
	}

	if (Ctrl->A.active) {	/* Finalize region via rounding/padding, then report it */
		if (Ctrl->A.round) {	/* Must round the region via the increments */
			wesn[XLO] = floor (wesn[XLO] / Ctrl->A.inc[GMT_X]) * Ctrl->A.inc[GMT_X];
			wesn[XHI] = ceil  (wesn[XHI] / Ctrl->A.inc[GMT_X]) * Ctrl->A.inc[GMT_X];
			wesn[YLO] = floor (wesn[YLO] / Ctrl->A.inc[GMT_Y]) * Ctrl->A.inc[GMT_Y];
			wesn[YHI] = ceil  (wesn[YHI] / Ctrl->A.inc[GMT_Y]) * Ctrl->A.inc[GMT_Y];
		}
		if (Ctrl->M.active) {	/* Extend region by given margins */
			wesn[XLO] -= Ctrl->M.margin[XLO];
			wesn[XHI] += Ctrl->M.margin[XHI];
			wesn[YLO] -= Ctrl->M.margin[YLO];
			wesn[YHI] += Ctrl->M.margin[YHI];
		}
		if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Trim to fit the sphere */
			if ((wesn[XHI] - wesn[XLO] > 360.0)) {	/* Trim to 360 range */
				if (wesn[XLO] >= 0.0 || wesn[XHI] > 300.0)	/* Mostly positive longitudes so use 0/360 */
					wesn[XLO] = 0.0, wesn[XHI] = 360.0;
				else	/* Use dateline as the jump */
					wesn[XLO] = -180.0, wesn[XHI] = 18.0;
			}
			/* Ensure latitudes are not out of bound */
			if (wesn[YLO] < -90.0) wesn[YLO] = -90.0;
			if (wesn[YHI] > +90.0) wesn[YHI] = +90.0;
		}
		if (Ctrl->C.box) {	/* Output closed polygon */
			out[GMT_X] = wesn[XLO];	out[GMT_Y] = wesn[YLO]; GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			out[GMT_X] = wesn[XHI];	GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			out[GMT_Y] = wesn[YHI]; GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			out[GMT_X] = wesn[XLO]; GMT_Put_Record (API, GMT_WRITE_DATA, Out);
			out[GMT_Y] = wesn[YLO]; GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		else if (Ctrl->C.active) {	/* Want numerical output */
			if (is_cube) {	/* Need w e s n b t w0 w1 */
				gmt_M_memcpy (out, wesn, 6, double);
				gmt_M_memcpy (&out[6], &wesn[WLO], 2, double);
			}
			else {	/* Need w e s n w0 w1 */
				gmt_M_memcpy (out, wesn, 4, double);
				gmt_M_memcpy (&out[4], &wesn[WLO], 2, double);
			}
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		else {	/* Give the string -Rw/e/s/n[/b/t] only */
			char text[GMT_LEN64] = {""};
			sprintf (record, "-R");
			gmt_ascii_format_col (GMT, text, wesn[XLO], GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, "/");
			gmt_ascii_format_col (GMT, text, wesn[XHI], GMT_OUT, GMT_X);	strcat (record, text);	strcat (record, "/");
			gmt_ascii_format_col (GMT, text, wesn[YLO], GMT_OUT, GMT_Y);	strcat (record, text);	strcat (record, "/");
			gmt_ascii_format_col (GMT, text, wesn[YHI], GMT_OUT, GMT_Y);	strcat (record, text);
			if (is_cube) {	/* Must also report the z-range */
				gmt_ascii_format_col (GMT, text, wesn[ZLO], GMT_OUT, GMT_Z);	strcat (record, text);	strcat (record, "/");
				gmt_ascii_format_col (GMT, text, wesn[ZHI], GMT_OUT, GMT_Z);	strcat (record, text);
			}
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
	}

nothing:

	gmt_M_free (GMT, Out);

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	Return (GMT_NOERROR);
}
