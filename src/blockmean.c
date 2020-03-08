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
 * API functions to support the blockmean application.
 *
 * Author:	Walter H.F. Smith
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: reads records of x, y, data, [weight] and writes out mean
 * value per cell, where cellular region is bounded by West East South North
 * and cell dimensions are delta_x, delta_y.
 */

#define BLOCKMEAN	/* Since mean, median, mode share near-similar macros we require this setting */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"blockmean"
#define THIS_MODULE_MODERN_NAME	"blockmean"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Block average (x,y,z) data tables by mean estimation"
#define THIS_MODULE_KEYS	"<D{,>D},GG),A->"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:>RVabdefghioqr" GMT_OPT("FH")

#include "block_subs.h"

enum Block_Modes {
	BLK_MODE_NOTSET = 0,	/* No -E+p|P (or -Ep) set */
	BLK_MODE_OBSOLETE = 1,	/* Old -Ep for backwards compatibility; assumes input weights are already set to 1/s^2 */
	BLK_MODE_WEIGHTED = 2,	/* -E+p computes weighted z means and error propagation on weighted z mean, using input s and w = 1/s^2 */
	BLK_MODE_SIMPLE   = 3	/* -E+P computes simple z means and error propagation on simple z mean, using input s and w = 1/s^2 */
};

enum S_Modes {
	BLK_OUT_MEAN  = 0,	/* -Sm for mean value [Default] */
	BLK_OUT_ZSUM  = 1,	/* -Ss for data sum */
	BLK_OUT_WSUM  = 2,	/* -Sw for weight sum */
	BLK_OUT_COUNT = 3	/* -Sn for count */
};

/* Note: For external calls to block* we do not allow explicit -G options; these should be added by examining -A which
 * is required for external calls to make grids, even if just z is requested.  This differs from the command line where
 * -Az is the default and -G is required to set file name format.  */

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] %s %s\n", name, GMT_I_OPT, GMT_Rgeo_OPT);
	if (API->external)
		GMT_Message (API, GMT_TIME_NONE, "\t[-A<fields>] [-C] [-E[+p|P]] [-S[m|n|s|w]] [%s] [-W[i][o][+s]]\n", GMT_V_OPT);
	else
		GMT_Message (API, GMT_TIME_NONE, "\t[-A<fields>] [-C] [-E[+p|P]] [-G<grdfile>] [-S[m|n|s|w]] [%s] [-W[i][o][+s]]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_a_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_q_OPT, GMT_r_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "I,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	if (API->external)
		GMT_Message (API, GMT_TIME_NONE, "\t-A List of comma-separated fields to be written as grids. Choose from\n");
	else
		GMT_Message (API, GMT_TIME_NONE, "\t-A List of comma-separated fields to be written as grids (requires -G). Choose from\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   z, s, l, h, and w. s|l|h requires -E; w requires -W[o] [Default is z only].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Output center of block and mean z-value [Default outputs (mean x, mean y) location]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Extend output with st.dev (s), low (l), and high (h) value per block, i.e.,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   output (x,y,z,s,l,h[,w]) [Default is (x,y,z[,w])]; see -W regarding the weight w.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -E+p is used it implies -Wi+s and s becomes the propagated error of the weighted mean z.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -E+P to instead obtain the propagated error on the simple mean z.\n");
	if (!API->external) {
		GMT_Message (API, GMT_TIME_NONE, "\t-G Specify output grid file name; no table results will be written to stdout.\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   If more than one field is set via -A then <grdfile> must contain  %%s to format field code.\n");
	}
	GMT_Message (API, GMT_TIME_NONE, "\t-S Set the quantity to be reported per block as z; choose among:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Sm report [weighted if -W] mean values [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Sn report number of data points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Ss report data sums.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Sw reports weight sums.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set Weight options, select one:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Wi reads 4 cols (x,y,z,w) but writes only (x,y,z[,s,l,h]) output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Wo reads 3 cols (x,y,z) but writes sum (x,y,z[,s,l,h],w) output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -W with no modifier has both weighted Input and Output; Default is no weights used.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append +s to read standard deviations s instead and compute w = 1/s^2.\n");
	GMT_Option (API, "a,bi");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   Default is 3 columns (or 4 if -W[+s] is set), or 2 for -Sn.\n");
	GMT_Option (API, "bo,d,e,f,h,i,o,q,r,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct BLOCKMEAN_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to blockmean and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, pos = 0;
	bool sigma;
	char arg[GMT_LEN16] = {""}, p[GMT_LEN16] = {""};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Requires -G and selects which fields should be written as grids */
				Ctrl->A.active = true;
				while ((gmt_strtok (opt->arg, ",", &pos, p)) && Ctrl->A.n_selected < BLK_N_FIELDS) {
					switch (p[0]) {
						case 'z':	Ctrl->A.selected[0] = true;	break;
						case 's':	Ctrl->A.selected[1] = true;	break;
						case 'l':	Ctrl->A.selected[2] = true;	break;
						case 'h':	Ctrl->A.selected[3] = true;	break;
						case 'w':	Ctrl->A.selected[4] = true;	break;
						default:
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized field argument %s in -A.!\n", p);
							n_errors++;
							break;
					}
					Ctrl->A.n_selected++;
				}
				if (Ctrl->A.n_selected == 0) {	/* Let -Az be the default */
					GMT_Report (GMT->parent, GMT_MSG_DEBUG, "-A interpreted to mean -Az.\n");
					Ctrl->A.selected[0] = true;
					Ctrl->A.n_selected = 1;
				}
				break;
			case 'C':	/* Report center of block instead */
				Ctrl->C.active = true;
				break;
			case 'E':	/* Extended report with standard deviation, min, and max in cols 4-6 */
				Ctrl->E.active = true;
				if (opt->arg[0] == 'p') {	/* Old -Ep option */
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "-Ep is deprecated; see -E+p|P instead.\n");
					Ctrl->E.mode = BLK_MODE_OBSOLETE;
				}
				else if (strstr (opt->arg, "+p"))	/* Error propagation on simple mean */
					Ctrl->E.mode = BLK_MODE_WEIGHTED;
				else if (strstr (opt->arg, "+P"))	/* Error propagation on weighted mean */
					Ctrl->E.mode = BLK_MODE_SIMPLE;
				break;
			case 'G':	/* Write output grid(s) */
				if (!GMT->parent->external && Ctrl->G.n) {	/* Command line interface */
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "-G can only be set once!\n");
					n_errors++;
				}
				else if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file[Ctrl->G.n++] = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':	/* Get block dimensions */
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'S':	/* Set report mode for z */
				Ctrl->S.active = true;
				switch (opt->arg[0]) {
					case '\0': case 'z':	/* GMT4 LEVEL: Report data sums */
						if (gmt_M_compat_check (GMT, 4)) {
							GMT_Report (GMT->parent, GMT_MSG_COMPAT, "-S and -Sz options are deprecated; use -Ss instead.\n");
							Ctrl->S.mode = BLK_OUT_ZSUM;
						}
						else /* Not allowing backwards compatibility */
							n_errors++;
						break;
					case 's':	/* Report data sums */
						Ctrl->S.mode = BLK_OUT_ZSUM; break;
					case 'w': 	/* Report weight sums */
						Ctrl->S.mode = BLK_OUT_WSUM; break;
					case 'm':		/* Report means */
						Ctrl->S.mode = BLK_OUT_MEAN; break;
					case 'n': 	/* Report number of points (i.e., weight sum with all weights == 1) */
						Ctrl->S.mode = BLK_OUT_COUNT; break;
					default:
						n_errors++; break;
				}
				break;
			case 'W':	/* Use in|out weights */
				Ctrl->W.active = true;
				if (gmt_validate_modifiers (GMT, opt->arg, 'W', "s")) n_errors++;
				sigma = (gmt_get_modifier (opt->arg, 's', arg)) ? true : false;
				switch (opt->arg[0]) {
					case '\0':	case '+':
						Ctrl->W.weighted[GMT_IN] = Ctrl->W.weighted[GMT_OUT] = true;
						Ctrl->W.sigma[GMT_IN] = sigma;
						break;
					case 'i': case 'I':
						Ctrl->W.weighted[GMT_IN] = true;
						Ctrl->W.sigma[GMT_IN] = sigma;
						break;
					case 'o': case 'O':
						Ctrl->W.weighted[GMT_OUT] = true;
						break;
					default:
						n_errors++;
						break;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if ((Ctrl->E.mode == BLK_MODE_WEIGHTED || Ctrl->E.mode == BLK_MODE_SIMPLE) && !(Ctrl->W.active && Ctrl->W.sigma[GMT_IN])) {	/* For -E+p|P and no -W we implicitly set -Wi+s */
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "-E+p|P given without -W or -W+s sets -Wi+s.\n");
		Ctrl->W.active = Ctrl->W.weighted[GMT_IN] = Ctrl->W.sigma[GMT_IN] = true;
	}
	if (Ctrl->G.active) {	/* Make sure -A sets valid fields, some require -E */
		if (!Ctrl->E.active && (Ctrl->A.selected[1] || Ctrl->A.selected[2] || Ctrl->A.selected[3])) {
			/* -E is required if -A specifies l or h */
			Ctrl->E.active = true;			/* Extended report with standard deviation, min, and max in cols 4-6 */
		}
		if (GMT->parent->external && !Ctrl->A.active) {		/* From externals let -G equals -Az */
			Ctrl->A.active = true;
			Ctrl->A.selected[0] = true;
			Ctrl->A.n_selected = 1;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !GMT->parent->external && Ctrl->A.active && !Ctrl->G.active, "Option -A requires -G\n");
	n_errors += gmt_M_check_condition (GMT, GMT->parent->external && Ctrl->G.active && !Ctrl->A.active,
	                                   "Option -G requires -A\n");
	if (Ctrl->G.active) {	/* Make sure -A sets valid fields, some require -E */
		if (Ctrl->A.active && Ctrl->A.n_selected > 1 && !GMT->parent->external && !strstr (Ctrl->G.file[0], "%s")) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "-G file format must contain a %%s for field type substitution.\n");
			n_errors++;
		}
		else if (!Ctrl->A.active)	/* Set default z output grid */
			Ctrl->A.selected[0] = true, Ctrl->A.n_selected = 1;
		else {	/* Make sure -A choices are valid and that -E is set if extended fields are selected */
			if (!Ctrl->E.active && (Ctrl->A.selected[1] || Ctrl->A.selected[2] || Ctrl->A.selected[3])) {
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "-E is required if -A specifies s, l, or h.  -E was added.\n");
				Ctrl->E.active = true;
			}
			if (Ctrl->A.selected[4] && !Ctrl->W.weighted[GMT_OUT]) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "-W or -Wo is required if -A specifies w.\n");
				n_errors++;
			}
		}
	}
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.mode == BLK_MODE_OBSOLETE && !Ctrl->W.weighted[GMT_IN],
	                                   "The deprecated -Ep option requires -W (not -W+s) with precomputed weights (= 1/sigma^2) on input\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.mode == BLK_MODE_OBSOLETE && Ctrl->W.sigma[GMT_IN],
	                                   "The deprecated -Ep option requires plain -W (not -W+s) with precomputed weights (= 1/sigma^2) on input\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0,
	                                   "Option -I: Must specify positive increment(s)\n");
	n_errors += gmt_check_binary_io (GMT, (Ctrl->W.weighted[GMT_IN]) ? 4 : 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {GMT_Destroy_Data (API, &Grid); gmt_M_free (GMT, zw); gmt_M_free (GMT, xy); gmt_M_free (GMT, np); gmt_M_free (GMT, slhg); gmt_M_free (GMT, Out); Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout(code);}

int GMT_blockmean (void *V_API, int mode, void *args) {
	uint64_t node, n_cells_filled, n_read, n_lost, n_pitched, w_col, *np = NULL;
	unsigned int row, col, n_input, k, kk, NF = 0, fcol[BLK_N_FIELDS] = {2,3,4,5,6,0,0,0}, field[BLK_N_FIELDS];
	int error;
	bool use_xy, use_weight, duplicate_col, bail = false;
	double weight, weight_s2 = 0, weight_pos, weighted_z, iw, half_dx, wesn[4], out[7], *in = NULL;
	char format[GMT_LEN512] = {""}, *fcode[BLK_N_FIELDS] = {"z", "s", "l", "h", "w", "", "", ""}, *code[BLK_N_FIELDS];
	char file[PATH_MAX] = {""};

	struct GMT_OPTION *options = NULL;
	struct GMT_GRID *Grid = NULL, *G = NULL, *GridOut[BLK_N_FIELDS];
	struct GMT_RECORD *In = NULL, *Out = NULL;
	struct BLK_PAIR *xy = NULL, *zw = NULL;
	struct BLK_SLHG *slhg = NULL;
	struct BLOCKMEAN_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
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

	/*---------------------------- This is the blockmean main code ----------------------------*/

	gmt_M_memset (GridOut, BLK_N_FIELDS, struct GMT_GRID *);	/* Initialize all pointers to NULL */

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, NULL, NULL, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);

	duplicate_col = (gmt_M_360_range (Grid->header->wesn[XLO], Grid->header->wesn[XHI]) && Grid->header->registration == GMT_GRID_NODE_REG);	/* E.g., lon = 0 column should match lon = 360 column */
	half_dx = 0.5 * Grid->header->inc[GMT_X];
	use_xy = !Ctrl->C.active;	/* If not -C then we must keep track of x,y locations */
	zw = gmt_M_memory (GMT, NULL, Grid->header->size, struct BLK_PAIR);
	if (use_xy) xy = gmt_M_memory (GMT, NULL, Grid->header->size, struct BLK_PAIR);
	if (Ctrl->E.active) slhg = gmt_M_memory (GMT, NULL, Grid->header->size, struct BLK_SLHG);
	if (Ctrl->W.weighted[GMT_IN] && ((Ctrl->W.sigma[GMT_IN] && use_xy) || Ctrl->E.active)) np = gmt_M_memory (GMT, NULL, Grid->header->size, uint64_t);

	/* Specify input and output expected columns */
	n_input = (Ctrl->S.mode == BLK_OUT_COUNT) ? 2 : 3;
	if ((error = GMT_Set_Columns (API, GMT_IN, n_input + Ctrl->W.weighted[GMT_IN], GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}
	if (!Ctrl->G.active && (error = GMT_Set_Columns (API, GMT_OUT, ((Ctrl->W.weighted[GMT_OUT]) ? 4 : 3) + 3 * Ctrl->E.active, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		Return (error);
	}

	/* Register likely data sources unless the caller has already done so */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if (!Ctrl->G.active && GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	gmt_set_xy_domain (GMT, wesn, Grid->header);	/* wesn may include some padding if gridline-registered */

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		snprintf (format, GMT_LEN512, "W: %s E: %s S: %s N: %s n_columns: %%d n_rows: %%d\n",
		          GMT->current.setting.format_float_out, GMT->current.setting.format_float_out,
				  GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, format, Grid->header->wesn[XLO], Grid->header->wesn[XHI],
		            Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->n_columns, Grid->header->n_rows);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG)) {	/* Memory reporting */
		size_t n_bytes, n_bytes_per_record = sizeof (struct BLK_PAIR);
		if (!Ctrl->C.active) n_bytes_per_record += sizeof (struct BLK_PAIR);
		if (Ctrl->E.active)  n_bytes_per_record += sizeof (struct BLK_SLHG);
		if (Ctrl->W.weighted[GMT_IN] && Ctrl->E.active) n_bytes_per_record += sizeof (uint64_t);
		n_bytes = n_bytes_per_record * Grid->header->size;	/* Report kbytes unless it is too much */
		GMT_Report (API, GMT_MSG_DEBUG, "Using a total of %s for all arrays.\n", gmt_memory_use (n_bytes, 1));
	}

	/* Initialize the i/o for doing record-by-record reading/writing */
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	n_read = n_pitched = 0;	/* Initialize counters */
	weight = weight_pos = 1.0;		/* Set the default point weight for position and z value */
	use_weight = (Ctrl->W.weighted[GMT_IN] && Ctrl->S.mode != BLK_OUT_COUNT);	/* Do not use weights if -Sn was set */

	GMT->session.min_meminc = GMT_INITIAL_MEM_ROW_ALLOC;	/* Start by allocating a 32 Mb chunk */

	/* Read the input data */

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;							/* Go back and read the next record */
		}
		in = In->data;	/* Only need to process numerical part here */
		if (gmt_M_is_dnan (in[GMT_Z])) 		/* Skip if z = NaN */
			continue;

		/* Clean data record to process */

		n_read++;							/* Number of records read */

		if (gmt_M_y_is_outside (GMT, in[GMT_Y],  wesn[YLO], wesn[YHI])) continue;		/* Outside y-range */
		if (gmt_x_is_outside (GMT, &in[GMT_X], wesn[XLO], wesn[XHI])) continue;		/* Outside x-range (or periodic longitude) */

		/* We appear to be inside: Get row and col indices of this block */

		if (gmt_row_col_out_of_bounds (GMT, in, Grid->header, &row, &col)) continue;	/* Sorry, outside after all */
		if (duplicate_col && (wesn[XHI]-in[GMT_X] < half_dx)) {	/* Only compute mean values for the west column and not the repeating east column with lon += 360 */
			in[GMT_X] -= 360.0;	/* Make this point be considered for the western block mean value */
			col = 0;
		}

		/* OK, this point is definitively inside and will be used */

		if (use_weight) {	/* Assign weights */
			if (Ctrl->W.sigma[GMT_IN]) {	/* Got sigma on z, use 1/s^ as data weight and plain means for position */
				weight_s2 = 1.0 / (in[3]*in[3]);	/* weight for error propagation */
				weight = (!Ctrl->E.active || Ctrl->E.mode == BLK_MODE_WEIGHTED) ? weight_s2 : 1.0;	/* Weight for mean z calculation */
			}
			else {	/* Generic weights given applies to x,y,z */
				weight = weight_s2 = in[3];
				if (Ctrl->E.mode != BLK_MODE_OBSOLETE) weight_pos = weight;
			}

		}
		weighted_z = in[GMT_Z] * weight;			/* Weighted value */
		node = gmt_M_ijp (Grid->header, row, col);		/* Bin node */
		if (use_xy) {						/* Must keep track of weighted location */
			xy[node].a[GMT_X] += (in[GMT_X] * weight_pos);
			xy[node].a[GMT_Y] += (in[GMT_Y] * weight_pos);
		}
		if (Ctrl->E.active) {	/* Add up sum (w*z^2) and n for weighted stdev and keep track of min,max */
			slhg[node].a[BLK_S] += (weighted_z * in[GMT_Z]);
			if (zw[node].a[BLK_W] == 0.0) {	/* Initialize low,high the first time */
				slhg[node].a[BLK_L] = +DBL_MAX;
				slhg[node].a[BLK_H] = -DBL_MAX;
			}
			if (in[GMT_Z] < slhg[node].a[BLK_L]) slhg[node].a[BLK_L] = in[GMT_Z];
			if (in[GMT_Z] > slhg[node].a[BLK_H]) slhg[node].a[BLK_H] = in[GMT_Z];
			if (Ctrl->E.mode == BLK_MODE_WEIGHTED) slhg[node].a[BLK_G] += weight_s2;	/* Sum of 1/sigma squared*/
			else if (Ctrl->E.mode) slhg[node].a[BLK_G] += 1.0 / weight_s2;	/* Sum of sigma squared*/
		}
		if (np) np[node]++;			/* Sum of number of points in this bin */
		zw[node].a[BLK_W] += weight;		/* Sum up the weights */
		zw[node].a[BLK_Z] += weighted_z;	/* Sum up the weighted values */
		n_pitched++;				/* Number of points actually used */
	} while (true);

	GMT->session.min_meminc = GMT_MIN_MEMINC;		/* Reset to the default value */

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

	/* Done with reading (files are automatically closed by i/o-machinery) */

	if (n_read == 0) {	/* Blank/empty input files */
		GMT_Report (API, GMT_MSG_WARNING, "No data records found; no output produced\n");
		if (!(API->external && Ctrl->G.active))
			bail = true;
	}
	else if (n_pitched == 0) {	/* No points inside region */
		GMT_Report (API, GMT_MSG_WARNING, "No data points found inside the region; no output produced\n");
		if (!(API->external && Ctrl->G.active))
			bail = true;
	}

	if (bail) {	/* Time to quit */
		Return (GMT_NOERROR);
	}

	w_col = gmt_get_cols (GMT, GMT_OUT) - 1;	/* Index of weight column (the last output column) */
	n_cells_filled = 0;				/* Number of blocks with values */
	fcol[4] = (unsigned int)w_col;				/* Since we don't know what it is until parsed */

	if (Ctrl->G.active) {	/* Create the grid(s) */
		char *remarks[BLK_N_FIELDS] = {"Mean value per bin", "Standard deviation per bin", "Lowest value per bin",
		                               "Highest value per bin", "Weight per bin"};
		for (k = kk = 0; k < BLK_N_FIELDS; k++) {
			if (!Ctrl->A.selected[k]) continue;
			field[NF] = fcol[k];	/* Just keep record of which fields we are actually using */
			code[NF]  = fcode[k];
			if ((GridOut[NF] = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL,
			                                    GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_TITLE, "Grid produced by blockmean", GridOut[NF]) != GMT_NOERROR) Return (API->error);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, GridOut[NF]) != GMT_NOERROR) Return (API->error);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, remarks[k], GridOut[NF])) Return (API->error);
			if (G == NULL) G = GridOut[NF];	/* First grid header used to get node later */
			for (node = 0; node < G->header->size; node++)
				GridOut[NF]->data[node] = GMT->session.f_NaN;	/* Initialize with NaNs */
			if (API->external && n_read == 0) {	/* Write the empty grids back to the external caller */
				if (strstr (Ctrl->G.file[kk], "%s"))
					sprintf (file, Ctrl->G.file[kk], code[k]);
				else
					strncpy (file, Ctrl->G.file[kk], PATH_MAX-1);
				if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, GridOut[k]) != GMT_NOERROR) {
					Return (API->error);
				}
			}
			if (Ctrl->G.n > 1) kk++;	/* Only true for APIs */
			NF++;	/* Number of actual field grids */
		}
		if (API->external && n_read == 0) {	/* Delayed return */
			Return (GMT_NOERROR);
		}
	}
	else {	/* Get ready for rec-by-rec output */
		if (GMT->common.h.add_colnames) {	/* Create meaningful column header */
			unsigned int k = 3;
			char header[GMT_BUFSIZ] = {""}, txt[GMT_LEN16] = {""}, *names[4] = {"\tmean_z", "\tsum_z", "\twsum_z", "\tn_z"};
			gmt_set_xycolnames (GMT, header);
			strcat (header, names[Ctrl->S.mode]);	strcat (header, "[2]");
			if (Ctrl->E.active) { strcat (header, "\tstd_z[3]\tlow_z[4]\thigh_z[5]"); k = 6; }
			if (Ctrl->W.weighted[GMT_OUT]) {
				snprintf (txt, GMT_LEN16, "\tweight[%d]", k);
				strcat (header, txt);
			}
			if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, NULL)) Return (API->error);
		}

		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			Return (API->error);
		}
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Calculating block means\n");
	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */

	for (node = 0; node < Grid->header->size; node++) {	/* Visit all possible blocks to see if they were visited */

		if (zw[node].a[BLK_W] == 0.0) continue;	/* No values in this block; skip */

		n_cells_filled++;	/* Increase number of blocks with values found so far */
		if (Ctrl->W.weighted[GMT_OUT]) out[w_col] = zw[node].a[BLK_W];
		iw = 1.0 / zw[node].a[BLK_W];	/* Inverse weight to avoid divisions later */
		if (use_xy) {	/* Determine and report mean point location */
			double iw_xy = (np) ? 1.0 / np[node] : iw;	/* Plain or weighted means */
			out[GMT_X] = xy[node].a[GMT_X] * iw_xy;
			out[GMT_Y] = xy[node].a[GMT_Y] * iw_xy;
		}
		else {		/* Report block center instead  */
			col = (unsigned int)gmt_M_col (Grid->header, node);
			row = (unsigned int)gmt_M_row (Grid->header, node);
			out[GMT_X] = gmt_M_grd_col_to_x (GMT, col, Grid->header);
			out[GMT_Y] = gmt_M_grd_row_to_y (GMT, row, Grid->header);
		}
		if (Ctrl->S.mode)	/* Report block sums or weights */
			out[GMT_Z] = (Ctrl->S.mode >= BLK_OUT_WSUM) ? zw[node].a[BLK_W] : zw[node].a[BLK_Z];
		else			/* Report block means */
			out[GMT_Z] = zw[node].a[BLK_Z] * iw;
		if (Ctrl->E.active) {	/* Compute and report extended attributes */
			if (Ctrl->W.weighted[GMT_IN]) {	/* Weighted standard deviation */
				if (Ctrl->E.mode == BLK_MODE_NOTSET) {	/* Weighted Std.dev of the values in this bin */
					out[3] = (np[node] > 1) ? d_sqrt ((zw[node].a[BLK_W] * slhg[node].a[BLK_S] - zw[node].a[BLK_Z] * zw[node].a[BLK_Z]) \
					/ (zw[node].a[BLK_W] * zw[node].a[BLK_W] * ((np[node] - 1.0) / np[node]))) : GMT->session.d_NaN;
				}
				else if (Ctrl->E.mode == BLK_MODE_WEIGHTED)	/* Error propagation on weighted mean assuming weights were 1/sigma^2 */
					out[3] = 1.0 / d_sqrt (slhg[node].a[BLK_G]);
				else /* BLK_MODE_OBSOLETE or BLK_MODE_SIMPLE: Error propagation on simple mean assuming weights were 1/sigma^2 */
					out[3] = d_sqrt (slhg[node].a[BLK_G]) / np[node];
			}
			else {		/* Normal standard deviation of the values in this bin */
				out[3] = (zw[node].a[BLK_W] > 1.0) ? d_sqrt ((zw[node].a[BLK_W] * slhg[node].a[BLK_S] - zw[node].a[BLK_Z] *
				                                             zw[node].a[BLK_Z]) / (zw[node].a[BLK_W] * (zw[node].a[BLK_W] -
															 1.0))) : GMT->session.d_NaN;
			}
			out[4] = slhg[node].a[BLK_L];	/* Minimum value in block */
			out[5] = slhg[node].a[BLK_H];	/* Maximum value in block */
		}
		if (Ctrl->G.active) {	/* Fill in one or more grids */
			if (use_xy) {	/* row/col was not set earlier */
				col = (unsigned int)gmt_M_col (Grid->header, node);
				row = (unsigned int)gmt_M_row (Grid->header, node);
			}
			for (k = 0; k < NF; k++)
				GridOut[k]->data[node] = (gmt_grdfloat)out[field[k]];
		}
		else
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */
	}
	if (Ctrl->G.active) {	/* Writes the grid(s) */
		for (k = kk = 0; k < NF; k++) {
			if (strstr (Ctrl->G.file[kk], "%s"))
				sprintf (file, Ctrl->G.file[kk], code[k]);
			else
				strncpy (file, Ctrl->G.file[kk], PATH_MAX-1);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, file, GridOut[k]) != GMT_NOERROR) {
				Return (API->error);
			}
			if (Ctrl->G.n > 1) kk++;	/* Only true for APIs */
		}
	}
	else if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	n_lost = n_read - n_pitched;	/* Number of points that did not get used */
	GMT_Report (API, GMT_MSG_INFORMATION, "N read: %" PRIu64 " N used: %" PRIu64 " N outside_area: %" PRIu64 " N cells filled: %" PRIu64 "\n", n_read, n_pitched, n_lost, n_cells_filled);

	if (Ctrl->W.weighted[GMT_IN] && Ctrl->E.active) gmt_M_free (GMT, np);

	Return (GMT_NOERROR);
}
