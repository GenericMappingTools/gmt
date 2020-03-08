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
 * API functions to support the blockmedian application.
 *
 * Author:	Walter H.F. Smith
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: reads records of x, y, data, [weight] and writes out median
 * value per cell, where cellular region is bounded by West East South North
 * and cell dimensions are delta_x, delta_y.
 */

#define BLOCKMEDIAN	/* Since mean, median, mode share near-similar macros we require this setting */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"blockmedian"
#define THIS_MODULE_MODERN_NAME	"blockmedian"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Block average (x,y,z) data tables by median estimation"
#define THIS_MODULE_KEYS	"<D{,>D},GG),A->"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:>RVabdefghioqr" GMT_OPT("FH")

#include "block_subs.h"

/* Note: For external calls to block* we do not allow explicit -G options; these should be added by examining -A which
 * is required for external calls to make grids, even if just z is requested.  This differs from the command line where
 * -Az is the default and -G is required to set file name format.  */

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] %s %s\n", name, GMT_I_OPT, GMT_Rgeo_OPT);
	if (API->external)
		GMT_Message (API, GMT_TIME_NONE, "\t[-A<fields>] [-C] [-E[b]] [-Er|s[+l|h]] [-Q] [-T<q>] [%s] [-W[i][o][+s]]\n", GMT_V_OPT);
	else
		GMT_Message (API, GMT_TIME_NONE, "\t[-A<fields>] [-C] [-E[b]] [-Er|s[+l|h]] [-G<grdfile>] [-Q] [-T<q>] [%s] [-W[i][o][+s]]\n", GMT_V_OPT);
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
	GMT_Message (API, GMT_TIME_NONE, "\t   z, s, l, q25, q75, h, and w. s|l|h requires -E; l|q25|q75|h requires -Eb, w requires -W[o].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be used with -Er|s [Default is z only].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Output center of block as location [Default is (median x, median y), but see -Q].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Extend output with L1 scale (s=MAD), low (l), and high (h) value per block, i.e.,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   output (x,y,z,s,l,h[,w]) [Default outputs (x,y,z[,w])]; see -W regarding w.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Eb for box-and-whisker output (x,y,z,l,25%%q,75%%q,h[,w]).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Er to report record number of the median value per block,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or -Es to report an unsigned integer source id (sid) taken from the x,y,z[,w],sid input.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For ties, report record number (or sid) of highest value (+h) or append +l for lowest [highest].\n");
	if (!API->external) {
		GMT_Message (API, GMT_TIME_NONE, "\t-G Specify output grid file name; no table results will be written to stdout.\n");
		GMT_Message (API, GMT_TIME_NONE, "\t   If more than one field is set via -A then <grdfile> must contain  %%s to format field code.\n");
	}
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Quicker; get median z and x,y at that z [Default gets median x, median y, median z].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set quantile (0 < q < 1) to report [Default is 0.5 which is the median of z].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set Weight options.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Wi reads Weighted Input (4 cols: x,y,z,w) but skips w on output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Wo reads unWeighted Input (3 cols: x,y,z) but weight sum on output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -W with no modifier has both weighted Input and Output; Default is no weights used.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +s read/write standard deviations instead, with w = 1/s.\n");
	GMT_Option (API, "a,bi");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   Default is 3 columns (or 4 if -W is set).\n");
	GMT_Option (API, "bo,d,e,f,h,i,o,q,r,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct BLOCKMEDIAN_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to blockmedian and sets parameters in CTRL.
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
					switch (p[0]) {	/* z,s,l,q25,q75,h,w */
						case 'z':	Ctrl->A.selected[0] = true;	break;
						case 's':	Ctrl->A.selected[1] = true;	break;
						case 'l':	Ctrl->A.selected[2] = true;	break;
						case 'q':
							if (!strcmp (p, "q25"))
								Ctrl->A.selected[3] = true;
							else if (!strcmp (p, "q75"))
								Ctrl->A.selected[4] = true;
							else {
								GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized field argument %s in -A!\n", p);
								n_errors++;
							}
							break;
						case 'h':	Ctrl->A.selected[5] = true;	break;
						case 'w':	Ctrl->A.selected[6] = true;	break;
						default:
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized field argument %s in -A!\n", p);
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
			case 'E':	/* Report extended statistics */
				Ctrl->E.active = true;		/* Report standard deviation, min, and max in cols 4-6 [Default] */
				if (opt->arg[0] == 'b')		/* Instead report min, 25%, 75% and max in cols 4-7 */
					Ctrl->E.mode = BLK_DO_EXTEND4;
				else if (opt->arg[0] == 'r' || opt->arg[0] == 's') {	/* Report row number or sid of median */
					switch (opt->arg[1]) {	/* Look for modifiers */
						case '+':	/* New syntax with +h or +l to parse */
							if (opt->arg[2] == 'l')
								Ctrl->E.mode = BLK_DO_INDEX_LO;
							else if (opt->arg[2] == 'h' || opt->arg[2] == '\0')	/* E.g., let Er+ be thought of as -Er+h */
								Ctrl->E.mode = BLK_DO_INDEX_HI;
							else {	/* Neither +l, +h, or just + is bad */
								GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized argument -E%s!\n", opt->arg);
								n_errors++;
							}
							break;
						case '-':	/* Old syntax -Er- or -Es- for reporting index/source of lower value */
							Ctrl->E.mode = BLK_DO_INDEX_LO;
							break;
						default:	/* Default reports index/source of higher value */
							Ctrl->E.mode = BLK_DO_INDEX_HI;
							break;
					}
					if (opt->arg[0] == 's') /* report sid, add in flag */
						Ctrl->E.mode |= BLK_DO_SRC_ID;
				}
				else if (opt->arg[0] == '\0')	/* Plain -E : Report L1scale, low, high in cols 4-6 */
					Ctrl->E.mode = BLK_DO_EXTEND3;
				else	/* WTF? */
					n_errors++;
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
			case 'Q':	/* Quick mode for median z */
				Ctrl->Q.active = true;		/* Get median z and (x,y) of that point */
				break;
			case 'T':	/* Select a particular quantile [0.5 (median)] */
				Ctrl->T.active = true;
				Ctrl->T.quantile = atof (opt->arg);
				break;
			case 'W':	/* Use in|out weights */
				Ctrl->W.active = true;
				if (gmt_validate_modifiers (GMT, opt->arg, 'W', "s")) n_errors++;
				sigma = (gmt_get_modifier (opt->arg, 's', arg)) ? true : false;
				switch (arg[0]) {
					case '\0':
						Ctrl->W.weighted[GMT_IN] = Ctrl->W.weighted[GMT_OUT] = true;
						Ctrl->W.sigma[GMT_IN] = Ctrl->W.sigma[GMT_OUT] = sigma;
						break;
					case 'i': case 'I':
						Ctrl->W.weighted[GMT_IN] = true;
						Ctrl->W.sigma[GMT_IN] = sigma;
						break;
					case 'o': case 'O':
						Ctrl->W.weighted[GMT_OUT] = true;
						Ctrl->W.sigma[GMT_OUT] = sigma;
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

	if (Ctrl->G.active && !Ctrl->E.active) {	/* See if we need to auto-supply -E or -Eb */
		if (Ctrl->A.selected[1]) {	/* s cannot go with the quantiles, so check */
			if (Ctrl->A.selected[3] || Ctrl->A.selected[4]) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot combine s with q25 or q75.\n");
				n_errors++;
			}
			else {	/* Plain -E is added */
				GMT_Report (GMT->parent, GMT_MSG_WARNING, "-E is required if -A specifies s, l, or h.  -E was added.\n");
				Ctrl->E.active = true;
				Ctrl->E.mode = BLK_DO_EXTEND3;		/* Report L1scale, low, high in cols 4-6 */
			}
		}
		else if (Ctrl->A.selected[3] || Ctrl->A.selected[4]) {	/* Need q25 or q75 and s not set, so add -Eb */
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "-Eb is required if -A specifies q25 or q75.  -Eb was added.\n");
			Ctrl->E.active = true;
			Ctrl->E.mode = BLK_DO_EXTEND4;		/* Report low, 25%, 75% and high in cols 4-7 */
		}
		else if (Ctrl->A.selected[2] || Ctrl->A.selected[5]) {	/* Plain -E is added */
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "-E is required if -A specifies s, l, or h.  -E was added.\n");
			Ctrl->E.active = true;
			Ctrl->E.mode = BLK_DO_EXTEND3;		/* Report L1scale, low, high in cols 4-6 */
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
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && (Ctrl->E.mode == BLK_DO_INDEX_LO ||
	                                   Ctrl->E.mode == BLK_DO_INDEX_HI),
	                                   "Option -Es|r are incompatible with -G\n");
	if (Ctrl->G.active) {	/* Make sure -A sets valid fields, some require -E */
		if (Ctrl->A.active && Ctrl->A.n_selected > 1 && !GMT->parent->external && !strstr (Ctrl->G.file[0], "%s")) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "-G file format must contain a %%s for field type substitution.\n");
			n_errors++;
		}
		else if (!Ctrl->A.active)	/* Set default z output grid */
			Ctrl->A.selected[0] = true, Ctrl->A.n_selected = 1;
		else {	/* Make sure -A choices are valid and that -E is set if extended fields are selected */
			if (Ctrl->A.selected[6] && !Ctrl->W.weighted[GMT_OUT]) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "-W or -Wo is required if -A specifies w.\n");
				n_errors++;
			}
		}
	}
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.quantile <= 0.0 || Ctrl->T.quantile >= 1.0,
	                                   "Option -T: Select 0 < q < 1 for quantile [0.5]\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0,
	                                   "Option -I: Must specify positive increment(s)\n");
	n_errors += gmt_check_binary_io (GMT, (Ctrl->W.weighted[GMT_IN]) ? 4 : 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void median_output (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, uint64_t first_in_cell, uint64_t first_in_new_cell, double weight_sum, double *out, double *extra, unsigned int go_quickly, unsigned int emode, double *quantile, unsigned int n_quantiles, struct BLK_DATA *data) {
	double weight_half, weight_count;
	uint64_t node, n_in_cell, node1;
	unsigned int k, k_for_xy;
	int way;
	gmt_M_unused(GMT);

	/* Remember: Data are already sorted on z for each cell */

	/* Step 1: Find the n_quantiles requested (typically only one unless -Eb was used) */

	n_in_cell = first_in_new_cell - first_in_cell;
	node = first_in_cell;
	weight_count = data[first_in_cell].a[BLK_W];
	k_for_xy = (n_quantiles == 3) ? 1 : 0;	/* If -Eb is set get get median location, else same as for z (unless -Q) */
	for (k = 0; k < n_quantiles; k++) {

		weight_half = quantile[k] * weight_sum;	/* Normally, quantile will be 0.5 (i.e., median), hence the name of the variable */

		/* Determine the point where we hit the desired quantile */

		while (weight_count < weight_half) weight_count += data[++node].a[BLK_W];	/* Wind up until weight_count hits the mark */

		if (weight_count == weight_half) {
			node1 = node + 1;
			extra[k] = 0.5 * (data[node].a[BLK_Z] + data[node1].a[BLK_Z]);
			if (k == k_for_xy && go_quickly == 1) {	/* Only get x,y at the z-quantile if requested [-Q] */
				out[GMT_X] = 0.5 * (data[node].a[GMT_X] + data[node1].a[GMT_X]);
				out[GMT_Y] = 0.5 * (data[node].a[GMT_Y] + data[node1].a[GMT_Y]);
			}
			if (k == 0 && emode) {	/* Return record number of median, selecting the high or the low value since it is a tie */
				way = (data[node].a[BLK_Z] >= data[node1].a[BLK_Z]) ? +1 : -1;
				if (emode & BLK_DO_INDEX_HI) extra[3] = (way == +1) ? (double)data[node].src_id : (double)data[node1].src_id;
				else extra[3] = (way == +1) ? (double)data[node1].src_id : (double)data[node].src_id;
			}
		}
		else {
			extra[k] = data[node].a[BLK_Z];
			if (k == k_for_xy && go_quickly == 1) {	/* Only get x,y at the z-quantile if requested [-Q] */
				out[GMT_X] = data[node].a[GMT_X];
				out[GMT_Y] = data[node].a[GMT_Y];
			}
			if (k == 0 && emode) extra[3] = (double)data[node].src_id;	/* Return record number of median */
		}
	}
	out[GMT_Z] = extra[k_for_xy];	/* The desired quantile is passed via z */

	if (go_quickly == 1) return;	/* Already have everything requested so we return */

	if (go_quickly == 2) {	/* Return center of block instead of computing a representative location */
		uint64_t row, col;
		row = gmt_M_row (h, data[node].ij);
		col = gmt_M_col (h, data[node].ij);
		out[GMT_X] = gmt_M_grd_col_to_x (GMT, col, h);
		out[GMT_Y] = gmt_M_grd_row_to_y (GMT, row, h);
		return;
	}

	/* We get here when we need separate quantile calculations for both x and y locations */

	weight_half = quantile[k_for_xy] * weight_sum;	/* We want the same quantile for locations as was used for z */

	if (n_in_cell > 2) qsort(&data[first_in_cell], n_in_cell, sizeof (struct BLK_DATA), BLK_compare_x);
	node = first_in_cell;
	weight_count = data[first_in_cell].a[BLK_W];
	while (weight_count < weight_half) weight_count += data[++node].a[BLK_W];
	out[GMT_X] = (weight_count == weight_half) ?  0.5 * (data[node].a[GMT_X] + data[node + 1].a[GMT_X]) : data[node].a[GMT_X];

	if (n_in_cell > 2) qsort (&data[first_in_cell], n_in_cell, sizeof (struct BLK_DATA), BLK_compare_y);
	node = first_in_cell;
	weight_count = data[first_in_cell].a[BLK_W];
	while (weight_count < weight_half) weight_count += data[++node].a[BLK_W];
	out[GMT_Y] = (weight_count == weight_half) ? 0.5 * (data[node].a[GMT_Y] + data[node + 1].a[GMT_Y]) : data[node].a[GMT_Y];
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {GMT_Destroy_Data (API, &Grid); gmt_M_free (GMT, Out); Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_blockmedian (void *V_API, int mode, void *args) {
	int error = 0;
	bool do_extra = false, duplicate_col, bail = false;
	uint64_t n_lost, node, first_in_cell, first_in_new_cell;
	uint64_t n_read, nz, n_pitched, n_cells_filled, w_col, i_col = 0, sid_col;
	size_t n_alloc = 0, nz_alloc = 0;
	unsigned int row, col, emode = 0, n_input, n_output, n_quantiles = 1, go_quickly = 0;
	unsigned int k, kk, NF = 0, fcol[BLK_N_FIELDS] = {2,3,4,5,6,7,0,0}, field[BLK_N_FIELDS];
	double out[8], wesn[4], quantile[3] = {0.25, 0.5, 0.75}, extra[8], weight, half_dx, *in = NULL, *z_tmp = NULL;
	char format[GMT_LEN512] = {""}, *old_format = NULL, *fcode[BLK_N_FIELDS] = {"z", "s", "l", "q25", "q75", "h", "w", ""}, *code[BLK_N_FIELDS];
	char file[PATH_MAX] = {""};

	struct GMT_OPTION *options = NULL;
	struct GMT_GRID *Grid = NULL, *G = NULL, *GridOut[BLK_N_FIELDS];
	struct GMT_RECORD *In = NULL, *Out = NULL;
	struct BLK_DATA *data = NULL;
	struct BLOCKMEDIAN_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the blockmedian main code ----------------------------*/

	gmt_M_memset (GridOut, BLK_N_FIELDS, struct GMT_GRID *);	/* Initialize all pointers to NULL */

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, NULL, NULL, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);

	duplicate_col = (gmt_M_360_range (Grid->header->wesn[XLO], Grid->header->wesn[XHI]) && Grid->header->registration == GMT_GRID_NODE_REG);	/* E.g., lon = 0 column should match lon = 360 column */
	half_dx = 0.5 * Grid->header->inc[GMT_X];
	go_quickly = (Ctrl->Q.active) ? 1 : 0;
	if (Ctrl->C.active && go_quickly == 1) {
		GMT_Report (API, GMT_MSG_WARNING, "-C overrides -Q\n");
		go_quickly = 0;
	}
	if (Ctrl->C.active) go_quickly = 2;			/* Flag used in output calculation */
	n_input = 3 + Ctrl->W.weighted[GMT_IN] + ((Ctrl->E.mode & BLK_DO_SRC_ID) ? 1 : 0);	/* 3 columns on output, plus 1 extra if -W and another if -Es  */
	n_output = (Ctrl->W.weighted[GMT_OUT]) ? 4 : 3;		/* 3 columns on output, plus 1 extra if -W  */
	if (Ctrl->E.active) {					/* One or more -E settings */
		if (Ctrl->E.mode & BLK_DO_EXTEND3) {	/* Add s,l,h cols */
			n_output += 3;
			do_extra = true;
			fcol[5] = 5;	/* Since no q25,75 */
		}
		else if (Ctrl->E.mode & BLK_DO_EXTEND4) {	/* Add l, 25%, 75%, h cols */
			n_output += 4;
			n_quantiles = 3;
			do_extra = true;
			fcol[2] = 3; fcol[3] = 4; fcol[4] = 5; fcol[5] = 6;
		}
		if (Ctrl->E.mode & BLK_DO_INDEX_LO || Ctrl->E.mode & BLK_DO_INDEX_HI) {	/* Add index */
			n_output++;
			emode = Ctrl->E.mode & (BLK_DO_INDEX_LO + BLK_DO_INDEX_HI);
		}
	}
	if (!(Ctrl->E.mode & BLK_DO_EXTEND4)) quantile[0] = Ctrl->T.quantile;	/* Just get the single quantile [median] */

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		snprintf (format, GMT_LEN512, "W: %s E: %s S: %s N: %s n_columns: %%d n_rows: %%d\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_INFORMATION, format, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->n_columns, Grid->header->n_rows);
	}

	gmt_set_xy_domain (GMT, wesn, Grid->header);	/* May include some padding if gridline-registered */

	/* Specify input and output expected columns */
	if ((error = GMT_Set_Columns (API, GMT_IN, n_input, GMT_COL_FIX)) != GMT_NOERROR) {
		Return (error);
	}
	if (!Ctrl->G.active && (error = GMT_Set_Columns (API, GMT_OUT, n_output, GMT_COL_FIX)) != GMT_NOERROR) {
		Return (error);
	}

	/* Register likely data sources unless the caller has already done so */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if (!Ctrl->G.active && GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}

	/* Initialize the i/o for doing record-by-record reading/writing */
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	sid_col = (Ctrl->W.weighted[GMT_IN]) ? 4 : 3;	/* Column with integer source id [if -Es is set] */
	n_read = n_pitched = 0;	/* Initialize counters */

	GMT->session.min_meminc = GMT_INITIAL_MEM_ROW_ALLOC;	/* Start by allocating a 32 MB chunk */

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

		n_read++;						/* Number of records read */

		if (gmt_M_y_is_outside (GMT, in[GMT_Y], wesn[YLO], wesn[YHI])) continue;		/* Outside y-range */
		if (gmt_x_is_outside (GMT, &in[GMT_X], wesn[XLO], wesn[XHI])) continue;		/* Outside x-range (or longitude) */

		/* We appear to be inside: Get row and col indices of this block */

		if (gmt_row_col_out_of_bounds (GMT, in, Grid->header, &row, &col)) continue;	/* Sorry, outside after all */
		if (duplicate_col && (wesn[XHI]-in[GMT_X] < half_dx)) {	/* Only compute median values for the west column and not the repeating east column with lon += 360 */
			in[GMT_X] -= 360.0;	/* Make this point be considered for the western block mean value */
			col = 0;
		}

		/* OK, this point is definitively inside and will be used */

		node = gmt_M_ijp (Grid->header, row, col);	/* Bin node */

		if (n_pitched == n_alloc) data = gmt_M_malloc (GMT, data, n_pitched, &n_alloc, struct BLK_DATA);
		data[n_pitched].ij = node;
		data[n_pitched].src_id = (Ctrl->E.mode & BLK_DO_SRC_ID) ? (uint64_t)lrint (in[sid_col]) : n_read;
		data[n_pitched].a[BLK_W] = ((Ctrl->W.weighted[GMT_IN]) ? ((Ctrl->W.sigma[GMT_IN]) ? 1.0 / in[3] : in[3]) : 1.0);
		if (!Ctrl->C.active) {	/* Need to store (x,y) so we can compute median location later */
			data[n_pitched].a[GMT_X] = in[GMT_X];
			data[n_pitched].a[GMT_Y] = in[GMT_Y];
		}
		data[n_pitched].a[BLK_Z] = in[GMT_Z];

		n_pitched++;
	} while (true);

	GMT->session.min_meminc = GMT_MIN_MEMINC;		/* Reset to the default value */

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}

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

	if (n_pitched < n_alloc) {
		n_alloc = n_pitched;
		data = gmt_M_malloc (GMT, data, 0, &n_alloc, struct BLK_DATA);
	}
	w_col = gmt_get_cols (GMT, GMT_OUT) - 1;	/* Weights always reported in last output column */
	fcol[6] = (unsigned int)w_col;				/* Since we don't know what it is until parsed */

	/* Ready to go. */

	if (Ctrl->G.active) {	/* Create the grid(s) */
		char *remarks[BLK_N_FIELDS] = {"Median value per bin", "L1 scale per bin", "Lowest value per bin", "25% quartile", "75% quartile", "Highest value per bin", "Weight per bin"};
		for (k = kk = 0; k < BLK_N_FIELDS; k++) {
			if (!Ctrl->A.selected[k]) continue;
			field[NF] = fcol[k];	/* Just keep record of which fields we are actually using */
			code[NF]  = fcode[k];
			if ((GridOut[NF] = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL, \
				GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_TITLE, "Grid produced by blockmedian", GridOut[NF]) != GMT_NOERROR) Return (API->error);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, GridOut[NF]) != GMT_NOERROR) Return (API->error);
			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_REMARK, remarks[k], GridOut[NF])) Return (API->error);
			if (G == NULL) G = GridOut[NF];	/* First grid header used to get node later */
			for (node = 0; node < G->header->size; node++)
				GridOut[NF]->data[node] = GMT->session.f_NaN;
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
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
			gmt_M_free (GMT, data);
			Return (API->error);
		}
		if (GMT_Set_Geometry (API, GMT_OUT, GMT_IS_POINT) != GMT_NOERROR) {	/* Sets output geometry */
			gmt_M_free (GMT, data);
			Return (API->error);
		}
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Calculating block medians\n");

	if (emode) {					/* Index column last, with weight col just before */
		i_col = w_col--;
		old_format = GMT->current.io.o_format[i_col];		/* Need to restore this at end */
		GMT->current.io.o_format[i_col] = strdup ("%.0f");	/* Integer format for src_id */
	}

	/* Sort on node and Z value */

	qsort (data, n_pitched, sizeof (struct BLK_DATA), BLK_compare_index_z);

	Out = gmt_new_record (GMT, out, NULL);	/* Since we only need to worry about numerics in this module */

	/* Find n_in_cell and write appropriate output  */

	first_in_cell = n_cells_filled = nz = 0;
	while (first_in_cell < n_pitched) {
		weight = data[first_in_cell].a[BLK_W];
		if (do_extra) {
			if (nz == nz_alloc) z_tmp = gmt_M_malloc (GMT, z_tmp, nz, &nz_alloc, double);
			z_tmp[0] = data[first_in_cell].a[BLK_Z];
			nz = 1;
		}
		first_in_new_cell = first_in_cell + 1;
		while ((first_in_new_cell < n_pitched) && (data[first_in_new_cell].ij == data[first_in_cell].ij)) {
			weight += data[first_in_new_cell].a[BLK_W];
			if (do_extra) {	/* Must get a temporary copy of the sorted z array */
				if (nz == nz_alloc) z_tmp = gmt_M_malloc (GMT, z_tmp, nz, &nz_alloc, double);
				z_tmp[nz++] = data[first_in_new_cell].a[BLK_Z];
			}
			first_in_new_cell++;
		}

		/* Now we have weight sum [and copy of z in case of -E]; now calculate the quantile(s): */

		median_output (GMT, Grid->header, first_in_cell, first_in_new_cell, weight, out, extra, go_quickly, emode, quantile, n_quantiles, data);
		/* Here, x,y,z are loaded into out */

		if (Ctrl->E.mode & BLK_DO_EXTEND4) {	/* Need 7 items: x, y, median, min, 25%, 75%, max [,weight] */
			out[3] = z_tmp[0];	/* 0% quantile (min value) */
			out[4] = extra[0];	/* 25% quantile */
			out[5] = extra[2];	/* 75% quantile */
			out[6] = z_tmp[nz-1];	/* 100% quantile (max value) */
		}
		else if (Ctrl->E.mode & BLK_DO_EXTEND3) {	/* Need 6 items: x, y, median, MAD, min, max [,weight] */
			out[4] = z_tmp[0];	/* Low value */
			out[5] = z_tmp[nz-1];	/* High value */
			/* Turn z_tmp into absolute deviations from the median (out[GMT_Z]) */
			if (nz > 1) {
				for (node = 0; node < nz; node++) z_tmp[node] = fabs (z_tmp[node] - out[GMT_Z]);
				gmt_sort_array (GMT, z_tmp, nz, GMT_DOUBLE);
				out[3] = (nz%2) ? z_tmp[nz/2] : 0.5 * (z_tmp[(nz-1)/2] + z_tmp[nz/2]);
				out[3] *= MAD_NORMALIZE;	/* This will be L1 MAD-based scale */
			}
			else
				out[3] = GMT->session.d_NaN;
		}
		if (Ctrl->W.weighted[GMT_OUT]) out[w_col] = (Ctrl->W.sigma[GMT_OUT]) ? 1.0 / weight : weight;
		if (emode) out[i_col] = extra[3];

		if (Ctrl->G.active) {
			row = gmt_M_grd_y_to_row (GMT, out[GMT_Y], Grid->header);
			col = gmt_M_grd_x_to_col (GMT, out[GMT_X], Grid->header);
			node = gmt_M_ijp (Grid->header, row, col);	/* Bin node */
			for (k = 0; k < NF; k++)
				GridOut[k]->data[node] = (gmt_grdfloat)out[field[k]];
		}
		else
			GMT_Put_Record (API, GMT_WRITE_DATA, Out);	/* Write this to output */

		n_cells_filled++;
		first_in_cell = first_in_new_cell;
	}

	gmt_M_free (GMT, data);
	if (do_extra) gmt_M_free (GMT, z_tmp);

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
	GMT_Report (API, GMT_MSG_INFORMATION, "N read: %" PRIu64 " N used: %" PRIu64 " outside_area: %" PRIu64 " N cells filled: %" PRIu64 "\n", n_read, n_pitched, n_lost, n_cells_filled);

	if (emode) {
		gmt_M_str_free (GMT->current.io.o_format[i_col]);	/* Free the temporary integer format */
		GMT->current.io.o_format[i_col] = old_format;		/* Restore previous format */
	}

	Return (GMT_NOERROR);
}
