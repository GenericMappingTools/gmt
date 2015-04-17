/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * API functions to support the blockmean application.
 *
 * Author:	Walter H.F. Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: reads records of x, y, data, [weight] and writes out mean
 * value per cell, where cellular region is bounded by West East South North
 * and cell dimensions are delta_x, delta_y.
 */

#define BLOCKMEAN	/* Since mean, median, mode share near-similar macros we require this setting */

#define THIS_MODULE_NAME	"blockmean"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Block average (x,y,z) data tables by L2 norm"
#define THIS_MODULE_KEYS	"<DI,>DO"

#include "gmt_dev.h"
#include "block_subs.h"

#define GMT_PROG_OPTIONS "-:>RVabdfghior" GMT_OPT("FH")

/* MEX: <DI >DO */

int GMT_blockmean_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: blockmean [<table>] %s\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-C] [-E[p]] [-S[m|n|s|w]] [%s] [-W[i][o]]\n", GMT_Rgeo_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_a_OPT, GMT_b_OPT, GMT_d_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Option (API, "I,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Output center of block and mean z-value [Default outputs the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (mean x, mean y) location].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Extend output with st.dev (s), low (l), and high (h) value per block,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i.e., output (x,y,z,s,l,h[,w]) [Default outputs (x,y,z[,w])];\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   see -W regarding the weight w.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -Ep is used we assume weights are 1/sigma^2 and s becomes the propagated error.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Set the quantity to be reported per block; choose among:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sm report mean values [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sn report number of data points.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ss report data sums.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sw reports weight sums.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set Weight options, select one:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Wi reads 4 cols (x,y,z,w) but writes only (x,y,z[,s,l,h]) output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Wo reads 3 cols (x,y,z) but writes sum (x,y,z[,s,l,h],w) output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -W with no modifier has both weighted Input and Output.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is no weights used].\n");
	GMT_Option (API, "a,bi");
	GMT_Message (API, GMT_TIME_NONE, "\t    Default is 3 columns (or 4 if -W is set), or 2 for -Sn.\n");
	GMT_Option (API, "bo,d,f,h,i,o,r,:,.");

	return (EXIT_FAILURE);
}

int GMT_blockmean_parse (struct GMT_CTRL *GMT, struct BLOCKMEAN_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to blockmean and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Report center of block instead */
				Ctrl->C.active = true;
				break;
			case 'E':	/* Extended report with standard deviation, min, and max in cols 4-6 */
				Ctrl->E.active = true;
				if (opt->arg[0] == 'p') Ctrl->E.mode = 1;
				break;
			case 'I':	/* Get block dimensions */
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'S':	/* Set report mode for z */
				Ctrl->S.active = true;
				switch (opt->arg[0]) {
					case '\0': case 'z':	/* GMT4 LEVEL: Report data sums */
						if (GMT_compat_check (GMT, 4)) {
							GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: -S and -Sz options are deprecated; use -Ss instead.\n");
							Ctrl->S.mode = 1;
						}
						else /* Not allowing backwards compatibility */
							n_errors++;
						break;
					case 's':	/* Report data sums */
						Ctrl->S.mode = 1; break;
					case 'w': 	/* Report weight sums */
						Ctrl->S.mode = 2; break;
					case 'm':		/* Report means */
						Ctrl->S.mode = 0; break;
					case 'n': 	/* Report number of points (i.e., weight sum with all weights == 1) */
						Ctrl->S.mode = 3; break;
					default:
						n_errors++; break;
				}
				break;
			case 'W':	/* Use in|out weights */
				Ctrl->W.active = true;
				switch (opt->arg[0]) {
					case '\0':
						Ctrl->W.weighted[GMT_IN] = Ctrl->W.weighted[GMT_OUT] = true; break;
					case 'i': case 'I':
						Ctrl->W.weighted[GMT_IN] = true; break;
					case 'o': case 'O':
						Ctrl->W.weighted[GMT_OUT] = true; break;
					default:
						n_errors++; break;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.registration, &Ctrl->I.active);	/* If -R<grdfile> was given we may get incs unless -I was used */

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.mode && !Ctrl->W.weighted[GMT_IN], "Syntax error: The -Ep option requires weights (= 1/sigma^2) on input\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_binary_io (GMT, (Ctrl->W.weighted[GMT_IN]) ? 4 : 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {GMT_Destroy_Data (API, &Grid); GMT_free (GMT, zw); if (xy) GMT_free (GMT, xy); if (np) GMT_free (GMT, np); if (slhg) GMT_free (GMT, slhg); Free_blockmean_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout(code);}

int GMT_blockmean (void *V_API, int mode, void *args)
{
	uint64_t node, n_cells_filled, n_read, n_lost, n_pitched, w_col, *np = NULL;
	unsigned int row, col, n_input;
	int error;
	bool use_xy, use_weight, duplicate_col;

	double weight, weighted_z, iw, half_dx, wesn[4], out[7], *in = NULL;

	char format[GMT_BUFSIZ] = {""};

	struct GMT_OPTION *options = NULL;
	struct GMT_GRID *Grid = NULL;
	struct BLK_PAIR *xy = NULL, *zw = NULL;
	struct BLK_SLHG *slhg = NULL;
	struct BLOCKMEAN_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/
	
	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_blockmean_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_blockmean_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_blockmean_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_blockmean_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_blockmean_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the blockmean main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, NULL, Ctrl->I.inc, \
		GMT_GRID_DEFAULT_REG, 0, NULL)) == NULL) Return (API->error);	/* Note: 0 for pad */

	duplicate_col = (GMT_360_RANGE (Grid->header->wesn[XLO], Grid->header->wesn[XHI]) && Grid->header->registration == GMT_GRID_NODE_REG);	/* E.g., lon = 0 column should match lon = 360 column */
	half_dx = 0.5 * Grid->header->inc[GMT_X];
	use_xy = !Ctrl->C.active;	/* If not -C then we must keep track of x,y locations */
	zw = GMT_memory (GMT, NULL, Grid->header->nm, struct BLK_PAIR);
	if (use_xy) xy = GMT_memory (GMT, NULL, Grid->header->nm, struct BLK_PAIR);
	if (Ctrl->E.active) slhg = GMT_memory (GMT, NULL, Grid->header->nm, struct BLK_SLHG);
	if (Ctrl->W.weighted[GMT_IN] && Ctrl->E.active) np = GMT_memory (GMT, NULL, Grid->header->nm, uint64_t);

	/* Specify input and output expected columns */
	n_input = (Ctrl->S.mode == 3) ? 2 : 3;
	if ((error = GMT_set_cols (GMT, GMT_IN, n_input + Ctrl->W.weighted[GMT_IN])) != GMT_OK) {
		Return (error);
	}
	if ((error = GMT_set_cols (GMT, GMT_OUT, ((Ctrl->W.weighted[GMT_OUT]) ? 4 : 3) + 3 * Ctrl->E.active)) != GMT_OK) {
		Return (error);
	}

	/* Register likely data sources unless the caller has already done so */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	GMT_set_xy_domain (GMT, wesn, Grid->header);	/* wesn may include some padding if gridline-registered */

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		sprintf (format, "W: %s E: %s S: %s N: %s nx: %%d ny: %%d\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (API, GMT_MSG_VERBOSE, format, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->nx, Grid->header->ny);
	}
	
	if (GMT_is_verbose (GMT, GMT_MSG_LONG_VERBOSE)) {	/* Memory reporting */
		unsigned int kind = 0;
		size_t n_bytes_per_record = sizeof (struct BLK_PAIR);
		double mem;
		char *unit = "KMG";	/* Kilo-, Mega-, Giga- */
		if (!Ctrl->C.active) n_bytes_per_record += sizeof (struct BLK_PAIR);
		if (Ctrl->E.active)  n_bytes_per_record += sizeof (struct BLK_SLHG);
		if (Ctrl->W.weighted[GMT_IN] && Ctrl->E.active) n_bytes_per_record += sizeof (uint64_t);
		mem = n_bytes_per_record * Grid->header->nm / 1024.0;	/* Report kbytes unless it is too much */
		while (mem > 1024.0 && kind < 2) { mem /= 1024.0;	kind++; }	/* Goto next higher unit */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Using a total of %.3g %cb for all arrays.\n", mem, unit[kind]);
	}

	/* Initialize the i/o for doing record-by-record reading/writing */
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	n_read = n_pitched = 0;	/* Initialize counters */
	weight = 1.0;		/* Set the default point weight */
	use_weight = (Ctrl->W.weighted[GMT_IN] && Ctrl->S.mode != 3);	/* Do not use weights if -Sn was set */

	GMT->session.min_meminc = GMT_INITIAL_MEM_ROW_ALLOC;	/* Start by allocating a 32 Mb chunk */ 
	
	/* Read the input data */

	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}
		
		if (GMT_is_dnan (in[GMT_Z])) 		/* Skip if z = NaN */
			continue;

		/* Clean data record to process */

		n_read++;							/* Number of records read */

		if (GMT_y_is_outside (GMT, in[GMT_Y],  wesn[YLO], wesn[YHI])) continue;		/* Outside y-range */
		if (GMT_x_is_outside (GMT, &in[GMT_X], wesn[XLO], wesn[XHI])) continue;		/* Outside x-range (or periodic longitude) */

		/* We appear to be inside: Get row and col indices of this block */

		if (GMT_row_col_out_of_bounds (GMT, in, Grid->header, &row, &col)) continue;	/* Sorry, outside after all */
		if (duplicate_col && (wesn[XHI]-in[GMT_X] < half_dx)) {	/* Only compute mean values for the west column and not the repeating east column with lon += 360 */
			in[GMT_X] -= 360.0;	/* Make this point be considered for the western block mean value */
			col = 0;
		}

		/* OK, this point is definitively inside and will be used */

		if (use_weight) weight = in[3];		/* Use provided weight instead of 1 */
		weighted_z = in[GMT_Z] * weight;			/* Weighted value */
		node = GMT_IJ0 (Grid->header, row, col);		/* Bin node */
		if (use_xy) {						/* Must keep track of weighted location */
			xy[node].a[GMT_X] += (in[GMT_X] * weight);
			xy[node].a[GMT_Y] += (in[GMT_Y] * weight);
		}
		if (Ctrl->E.active) {	/* Add up sum (w*z^2) and n for weighted stdev and keep track of min,max */
			slhg[node].a[BLK_S] += (weighted_z * in[GMT_Z]);
			if (Ctrl->W.weighted[GMT_IN]) np[node]++;
			if (zw[node].a[BLK_W] == 0.0) {	/* Initialize low,high the first time */
				slhg[node].a[BLK_L] = +DBL_MAX;
				slhg[node].a[BLK_H] = -DBL_MAX;
			}
			if (in[GMT_Z] < slhg[node].a[BLK_L]) slhg[node].a[BLK_L] = in[GMT_Z];
			if (in[GMT_Z] > slhg[node].a[BLK_H]) slhg[node].a[BLK_H] = in[GMT_Z];
			if (Ctrl->E.mode == 1) slhg[node].a[BLK_G] += 1.0 / weight;	/* Sum of sigma squared*/
		}
		zw[node].a[BLK_W] += weight;		/* Sum up the weights */
		zw[node].a[BLK_Z] += weighted_z;	/* Sum up the weighted values */
		n_pitched++;				/* Number of points actually used */
	} while (true);

	GMT->session.min_meminc = GMT_MIN_MEMINC;		/* Reset to the default value */
	
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	/* Done with reading (files are automatically closed by i/o-machinery) */
	
	if (n_read == 0) {	/* Blank/empty input files */
		GMT_Report (API, GMT_MSG_VERBOSE, "No data records found; no output produced");
		Return (EXIT_SUCCESS);
	}
	if (n_pitched == 0) {	/* No points inside region */
		GMT_Report (API, GMT_MSG_VERBOSE, "No data points found inside the region; no output produced");
		Return (EXIT_SUCCESS);
	}

	w_col = GMT_get_cols (GMT, GMT_OUT) - 1;	/* Index of weight column (the last output column) */
	n_cells_filled = 0;				/* Number of blocks with values */

	GMT_Report (API, GMT_MSG_VERBOSE, "Calculating block means\n");

	if (GMT->common.h.add_colnames) {	/* Create meaningful column header */
		unsigned int k = 3;
		char header[GMT_BUFSIZ] = {""}, txt[GMT_LEN16] = {""}, *names[4] = {"\tmean_z", "\tsum_z", "\twsum_z", "\tn_z"};
		GMT_set_xycolnames (GMT, header);
		strcat (header, names[Ctrl->S.mode]);	strcat (header, "[2]");
		if (Ctrl->E.active) { strcat (header, "\tstd_z[3]\tlow_z[4]\thigh_z[5]"); k = 6; }
		if (Ctrl->W.weighted[GMT_OUT]) {
			sprintf (txt, "\tweight[%d]", k);
			strcat (header, txt);
		}
		if (GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, NULL)) Return (API->error);
	}
	
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_OK) {	/* Enables data output and sets access mode */
		Return (API->error);
	}

	for (node = 0; node < Grid->header->nm; node++) {	/* Visit all possible blocks to see if they were visited */

		if (zw[node].a[BLK_W] == 0.0) continue;	/* No values in this block; skip */

		n_cells_filled++;	/* Increase number of blocks with values found so far */
		if (Ctrl->W.weighted[GMT_OUT]) out[w_col] = zw[node].a[BLK_W];
		iw = 1.0 / zw[node].a[BLK_W];	/* Inverse weight to avoid divisions later */
		if (use_xy) {	/* Determine and report mean point location */
			out[GMT_X] = xy[node].a[GMT_X] * iw;
			out[GMT_Y] = xy[node].a[GMT_Y] * iw;
		}
		else {		/* Report block center */
			col = (unsigned int)GMT_col (Grid->header, node);
			row = (unsigned int)GMT_row (Grid->header, node);
			out[GMT_X] = GMT_grd_col_to_x (GMT, col, Grid->header);
			out[GMT_Y] = GMT_grd_row_to_y (GMT, row, Grid->header);
		}
		if (Ctrl->S.mode)	/* Report block sums or weights */
			out[GMT_Z] = (Ctrl->S.mode >= 2) ? zw[node].a[BLK_W] : zw[node].a[BLK_Z];
		else			/* Report block means */
			out[GMT_Z] = zw[node].a[BLK_Z] * iw;
		if (Ctrl->E.active) {	/* Compute and report extended attributes */
			if (Ctrl->W.weighted[GMT_IN]) {	/* Weighted standard deviation */
				if (Ctrl->E.mode == 1) {	/* Error propagation assuming weights were 1/sigma^2 */
					out[3] = d_sqrt (slhg[node].a[BLK_G]) / np[node];
				}
				else {
					out[3] = (np[node] > 1) ? d_sqrt ((zw[node].a[BLK_W] * slhg[node].a[BLK_S] - zw[node].a[BLK_Z] * zw[node].a[BLK_Z]) \
					/ (zw[node].a[BLK_W] * zw[node].a[BLK_W] * ((np[node] - 1.0) / np[node]))) : GMT->session.d_NaN;
				}
			}
			else {					/* Normal standard deviation */
				out[3] = (zw[node].a[BLK_W] > 1.0) ? d_sqrt ((zw[node].a[BLK_W] * slhg[node].a[BLK_S] - zw[node].a[BLK_Z] * zw[node].a[BLK_Z]) \
				/ (zw[node].a[BLK_W] * (zw[node].a[BLK_W] - 1.0))) : GMT->session.d_NaN;
			}
			out[4] = slhg[node].a[BLK_L];	/* Minimum value in block */
			out[5] = slhg[node].a[BLK_H];	/* Maximum value in block */
		}
		GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}

	n_lost = n_read - n_pitched;	/* Number of points that did not get used */
	GMT_Report (API, GMT_MSG_VERBOSE, "N read: %" PRIu64 " N used: %" PRIu64 " N outside_area: %" PRIu64 " N cells filled: %" PRIu64 "\n", n_read, n_pitched, n_lost, n_cells_filled);

	if (Ctrl->W.weighted[GMT_IN] && Ctrl->E.active) GMT_free (GMT, np);
	
	Return (GMT_OK);
}
