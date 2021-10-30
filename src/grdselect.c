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
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: grdselect reads one or more grid/cube file and determines the
 * union or intersection of the regions, modulated by other options.
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"grdselect"
#define THIS_MODULE_MODERN_NAME	"grdselect"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Select from a list of 2-D grids or 3-D cubes"
#define THIS_MODULE_KEYS	"<?{+,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->RVfho"

/* Control structure for grdselect */

enum Opt_A_modes {
	GRDSELECT_INTERSECTION	= 0,
	GRDSELECT_UNION			= 1,
	GRDSELECT_NO_INC	= 0,
	GRDSELECT_MIN_INC	= 1,	/* +il */
	GRDSELECT_MAX_INC	= 2,	/* +ih */
	GRDSELECT_SET_INC	= 3};	/* +i<incs> */

enum Opt_N_modes {
	GRDSELECT_LESS_NANS	= 1,
	GRDSELECT_MORE_NANS	= 2};


#define WLO	6
#define WHI	7

struct GRDSELECT_CTRL {
	unsigned int n_files;	/* How many grids given */
	struct GRDSELECT_A {	/* -A[i|u][+il|h|arg>]*/
		bool active;
		bool round;		/* True if +i is used to set rounding */
		unsigned int i_mode;	/* 0 = no increment rounding, 1 is use smallest inc, 2 = use largest inc, 3 = use appended inc */
		unsigned int mode;		/* 0 = intersection [i], 1 = union [r] */
		double inc[2];	/* Optional increments */
	} A;
	struct GRDSELECT_C {	/* -C[n|t] */
		bool active;
		unsigned int mode;
	} C;
	struct GRDSELECT_I {	/* -I<dx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct GRDSELECT_L {	/* Just list the passing grids*/
		bool active;
	} L;
	struct GRDSELECT_M {	/* -M */
		bool active;
		double margin[4];	/* Optional margins [none] */
	} M;
	struct GRDSELECT_N {	/* -Nl|u<nans> */
		bool active;
		unsigned int mode;
		uint64_t n_nans;	/* Optional margins [none] */
	} N;
	struct GRDSELECT_Q {	/* -Q */
		bool active;
	} Q;
	struct GRDSELECT_Z {	/* -Zzmin/zmax[+i] */
		bool active;
		bool inverse;
		double z_min, z_high;
	} Z;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDSELECT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDSELECT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDSELECT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s %s [-Ai|u[+il|h|<incs>] [-C] [-I<xinc>[/<yinc>]] [-L] [-M<margins>] [-Nl|u<n_nans>] "
		"[-Q] [%s] [%s] [%s] [%s] [%s] [%s]\n", name, GMT_INGRID, GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT, GMT_ho_OPT, GMT_o_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	gmt_ingrid_syntax (API, 0, "Name of one or more grid files");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-Ai|u[+il|h|<incs>");
	GMT_Usage (API, -2, "Select i for intersection or u for union in determining the joint area [union]. "
		"Optionally, append +i to round region using the (l)oest, (h)ighest, or specified increments [none].");
	GMT_Usage (API, 1, "\n-C");
	GMT_Usage (API, -2, "Report information in fields on a single line using the format "
		"<w e s n> [Default reports a -R<w/e/s/n> string]");
	GMT_Usage (API, 1, "\n-I<xinc>[/<yinc>]");
	GMT_Usage (API, -2, "Only consider the grids that matches the given increments [Consider all input grids].");
	GMT_Usage (API, 1, "\n-L Just list the passing grids with no region iformation [Output the region only]");
	GMT_Usage (API, 1, "\n-M<margins>");
	GMT_Usage (API, -2, "Add space around the final (rounded) region. Append a uniform <margin>, separate <xmargin>/<ymargin>, "
		"or individual <wmargin>/<emargin>/<smargin>/<nmargin> for each side.");
	GMT_Usage (API, 1, "\n-Nl|u<n_nans>");
	GMT_Usage (API, -2, "Only consider the grids that satisfies a NaN-condition [Consider all input grids]:");
	GMT_Usage (API, 3, "l: Only grids with fewer than <n_nans> NaNs will pass");
	GMT_Usage (API, 3, "h: Only grids with more than <n_nans>  NaNs will pass");
	GMT_Usage (API, 1, "\n-Q Input file(s) is 3-D data cube(s), not grid(s) [2-D grids].");
	GMT_Option (API, "R");
	GMT_Usage (API, 1, "\n-Z[<min>]/[<max>]");
	GMT_Usage (API, -2, "Only consider the grids that have z-values in the given range [Consider all input grids]. "
		"At least one of <min> or <max> must be specified, as well as the slash.");
	GMT_Usage (API, 1, "\n-r[g|p]");
	GMT_Usage (API, -2, "Only consider the grids that have the specified registration [Consider all input grids].");
	GMT_Option (API, "V,f,h,o,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDSELECT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	bool no_file_OK, Ctrl->C.active;
	unsigned int n_errors = 0, k = 0;
	char text[GMT_LEN32] = {""}, string[GMT_LEN128] = {""}, *c = NULL;
	static char *M[2] = {"minimum", "maximum"}, *V[3] = {"negative", "all", "positive"}, *T[2] = {"column", "row"};
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
							if (gmt_getinc (GMT, &string[1], Ctrl->A.inc)) n_errors++;
							Ctrl->A.i_mode = GRDSELECT_SET_INC;
							break;
					}
				}
				break;

			case 'C':	/* Column format */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				Ctrl->C.active = true;
				if (opt->arg[0] == 'n')
					Ctrl->C.mode = GRDSELECT_NUMERICAL;
				else if (opt->arg[0] == 't')
					Ctrl->C.mode = GRDSELECT_TRAILING;
				if (GMT->parent->external && Ctrl->C.mode == GRDSELECT_TRADITIONAL) Ctrl->C.mode = GRDSELECT_NUMERICAL;
				break;
			case 'I':	/* Specified grid increments */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				Ctrl->I.active = true;
				if (opt->arg[0] && gmt_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					gmt_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':	/* Selects lilst output */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->L.active);
				Ctrl->L.active = true;
				break;
			case 'M':	/* Extend the region */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				Ctrl->M.active = true;
				k = GMT_Get_Values (GMT->parent, opt->arg, Ctrl->M.margin, 4);
				if (k == 1)	/* Same increments in all directions */
					Ctrl->M.margin[XHI] = Ctrl->M.margin[YLO] = Ctrl->M.margin[YHI] = inCtrl->M.marginc[XLO];
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
					case 'u':	Ctrl->N.mode = GRDSELECT_MORE_NANS;	break;
					default:
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -M: Bad directive, must be -Ml or -Mu\n");
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
				if ((c = strstr ("+i"))) {
					Ctrl->Z.invert = true;
					c[0] = '\0';	/* Hide modifier */
				}
				n_errors += gmt_get_limits (GMT, 'Z', opt->arg, 1, &Ctrl->Z.z_min, &Ctrl->Z.z_high);
				if (c) c[0] = '+';	/* Restore modifier */
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->n_files == 0,
	                                   "Must specify one or more input files\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0),
									   "Option -I: Must specify a positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.o.active && Ctrl->C.active,
	                                   "The -o option requires -C\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

struct GMT_TILES {
	double wesn[4];
};


#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_set_pad (GMT, GMT_PAD_DEFAULT); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_grdselect (void *V_API, int mode, void *args) {
	int error = 0, k_data, k_tile_id;
	unsigned int n_cols = 0, level, cmode = GMT_COL_FIX, geometry = GMT_IS_TEXT;
	unsigned int GMT_W = GMT_Z;
	bool first_r = true, first = true, is_cube;

	double wesn[8], out[8];

	char record[GMT_BUFSIZ] = {""},;

	struct GRDSELECT_CTRL *Ctrl = NULL;
	struct GMT_GRID *G = NULL, *W = NULL;
	struct GMT_GRID_HEADER *header = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
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

	/* OK, done parsing, now process all input grids in a loop */

	gmt_M_memset (wesn, 8, double);	/* Initialize */

	sep = GMT->current.setting.io_col_separator;

	if (Ctrl->C.active) {
		n_cols = (Ctrl->Q.active) ? 8 : 6;	/* w e s n [z0 z1] [w0 w1] */
		cmode = GMT_COL_FIX_NO_TEXT;
		geometry = GMT_IS_NONE;
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

	if (Ctrl->R.set[RSET]) {	/* Gave a sub region so set the first region limit */
		gmt_M_memcpy (wesn, GMT->R.common.wesn, 6U, double);
		first_r = false;
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

		if (GMT->common.R.set[GSET]) {	/* Specified -r to only keep those grids with the same registration */
			if (header->registration != common.R.registration) continue;
		}
		if (Ctrl->I.active) {	/* Only pass grids with the same increments */
			if (!doubleAlmostEqual (Ctrl->I.inc[GMT_X], header->inc[GMT_X]) || !doubleAlmostEqual (Ctrl->I.inc[GMT_Y], header->inc[GMT_Y])) continue;
		}
		if (Ctrl->Z.active) {	/* Skip grids outside the imposed z-range */
			outside = (header->z_max < Ctrl->Z.z_min || header->z_min > Ctrl->Z.z_maz);
			if (outside != Ctrl->Z.invert) continue;	/* Skip if outside (or inside via +i) the range */
		}
		if (Ctrl->N.active) {	/* Must read the data to know how many NaNs, then skip grids that fail the test */
			uint64_t level, here = 0, ij, n_nan = 0;
			unsigned int col, row;
			gmt_grdfloat *data = (is_cube) ? U->data : G->data;
			if (is_cube) {
				if (GMT_Read_Data (API, GMT_IS_CUBE, GMT_IS_FILE, GMT_IS_VOLUME, GMT_DATA_ONLY, wesn, opt->arg, U) == NULL) {
					Return (API->error);
				}
			}
			else {
				if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, opt->arg, G) == NULL) {
					Return (API->error);
				}
			}

			for (level = 0; level < header->n_bands; level++) {
				for (row = 0; row < header->n_rows; row++) {
					for (col = 0, ij = gmt_M_ijp (header, row, 0); col < header->n_columns; col++, ij++) {
						if (gmt_M_is_fnan (data[ij + here])) n_nan++;
					}
				}
				here += header->size;
			}
			if (is_cube && GMT_Destroy_Data (API, &U) != GMT_NOERROR) {
				Return (API->error);
			}
			else if (!is_cube && GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
				Return (API->error);
			}
			if (Ctrl->N.mode == GRDSELECT_LESS_NANS && n_nan > Ctrl->N.n_nans) continue;	/* Skip this item */
			if (Ctrl->N.mode == GRDSELECT_MORE_NANS && n_nan < Ctrl->N.n_nans) continue;	/* Skip this item */
		}

		/* OK, here the grid passed any other obstacles */
		if (first_r) {
			gmt_M_memcpy (wesn, header->wesn);	/* The first grid needs to set the extend of the region for now */
			first_r = false;
		}
		if (first) {
			if (Ctrl->Q.active) {
				wesn[ZLO] = U->level[0];
				wesn[ZHI] = U->level[header->n_bands-1];
			}
			wesn[WLO] = header->z_min;
			wesn[WHI] = header->z_max;
			if (Ctrl->I.mode == GRDSELECT_MIN_INC || Ctrl->I.mode == GRDSELECT_MAX_INC)	/* Copy the first grid increments */
				gmt_M_memcpy (Ctrl->I.inc, header->inc, 2U);
		}
		if (Ctrl->L.active) {	/* Only wanted to list the grid files */
			sprintf (record, "%%s", opt->arg);
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);
		}
		else {
			if (Ctrl->A.mode == GRDSELECT_INTERSECTION) {	/* Retain only region common to all grids (i.e., the intersection) */
				wesn[YLO] = MAX (wesn[YLO], header->wesn[YLO]);
				wesn[YHI] = MIN (wesn[YHI], header->wesn[YHI]);
				if (wesn[YHI] <= wesn[YLO]) {	/* No common region can be found - no point to continue */
					GMT_Report (API, GMT_MSG_WARNING, "No common area for all grids is possible.\n");
					gmt_M_memcpy (wesn, wesn_nothing, 8U, double);
					goto reporting;
				}
				if (is_cube) {
					wesn[ZLO] = MAX (wesn[ZLO], U->level[0]);
					wesn[ZHI] = MIN (wesn[ZHI], U->level[header->n_bands-1]);
					if (wesn[ZHI] <= wesn[ZLO]) {	/* No common cube region can be found - no point to continue */
						GMT_Report (API, GMT_MSG_WARNING, "No common area for all cubes is possible.\n");
						gmt_M_memcpy (wesn, wesn_nothing, 8U, double);
						goto reporting;
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
					GMT_Report (API, GMT_MSG_WARNING, "No common area for all grids is possible.\n");
					gmt_M_memcpy (wesn, wesn_nothing, 8U, double);
					goto reporting;
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
					wesn[ZLO] = MIN (wesn[ZLO], U->level[0]);
					wesn[ZHI] = MAX (wesn[ZHI], U->level[header->n_bands-1]);
				}
			}
		}
		if (Ctrl->I.mode == GRDSELECT_MIN_INC) {	/* Update the smallest increments found */
			if (header->inc[GMT_X] < Ctrl->I.inc[GMT_X]) Ctrl->I.inc[GMT_X] = header->inc[GMT_X];	/* Update the smallest x-increments found */
			if (header->inc[GMT_Y] < Ctrl->I.inc[GMT_Y]) Ctrl->I.inc[GMT_Y] = header->inc[GMT_Y];	/* Update the smallest y-increments found */
		}
		if (Ctrl->I.mode == GRDSELECT_MAX_INC) {	/* Update the largest increments found */
			if (header->inc[GMT_X] > Ctrl->I.inc[GMT_X]) Ctrl->I.inc[GMT_X] = header->inc[GMT_X];	/* Update the largest x-increments found */
			if (header->inc[GMT_Y] > Ctrl->I.inc[GMT_Y]) Ctrl->I.inc[GMT_Y] = header->inc[GMT_Y];	/* Update the largest y-increments found */
		}
	}

regporting:

	if (!Ctrl->L.active) {
		/* Possible round the region */
		if (Ctrl->I.mode != GRDSELECT_NO_INC) {
			wesn[XLO] = floor (wesn[XLO] / Ctrl->inc[GMT_X]) * Ctrl->inc[GMT_X];
			wesn[XHI] = ceil  (wesn[XHI] / Ctrl->inc[GMT_X]) * Ctrl->inc[GMT_X];
			wesn[YLO] = floor (wesn[YLO] / Ctrl->inc[GMT_Y]) * Ctrl->inc[GMT_Y];
			wesn[YHI] = ceil  (wesn[YHI] / Ctrl->inc[GMT_Y]) * Ctrl->inc[GMT_Y];
		}
		if (Ctrl->M.active) {	/* Extend by given amounts */
			wesn[XLO] -= Ctrl->M.margin[XLO];
			wesn[XHI] += Ctrl->M.margin[XHI];
			wesn[YLO] -= Ctrl->M.margin[YLO];
			wesn[YHI] += Ctrl->M.margin[YHI];
		}
		if (Ctrl->C.active) {
			if (is_cube) {
				gmt_M_memcpy (out, wesn, 6, double);
				gmt_M_memcpy (&out[6], &wesn[WLO], 2, double);
			}
			else {
				gmt_M_memcpy (out, wesn, 4, double);
				gmt_M_memcpy (&out[4], &wesn[WLO], 2, double);
			}
		}
		GMT_Put_Record (API, GMT_WRITE_DATA, Out);
	}

	gmt_M_free (GMT, Out);

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	Return (GMT_NOERROR);
}
