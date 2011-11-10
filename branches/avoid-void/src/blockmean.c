/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
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

#include "gmt.h"
#include "block_subs.h"

GMT_LONG GMT_blockmean_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "blockmean %s [API] - Block average (x,y,z) data tables by L2 norm\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: blockmean [<table>] %s\n", GMT_I_OPT);
	GMT_message (GMT, "\t%s [-C] [-E] [-S[m|n|s|w]]\n", GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[%s] [-W[i][o]] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_inc_syntax (GMT, 'I', 0);
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-C Output center of block and mean z-value.  [Default outputs the\n");
	GMT_message (GMT, "\t   (mean x, mean y) location].\n");
	GMT_message (GMT, "\t-E Extend output with st.dev (s), low (l), and high (h) value per block,\n");
	GMT_message (GMT, "\t   i.e., output (x,y,z,s,l,h[,w]) [Default outputs (x,y,z[,w]);\n");
	GMT_message (GMT, "\t   see -W regarding the weight w.\n");
	GMT_message (GMT, "\t-S Set the quantity to be reported per block; choose among:\n");
	GMT_message (GMT, "\t   -Sm report mean values [Default].\n");
	GMT_message (GMT, "\t   -Sn report number of data points.\n");
	GMT_message (GMT, "\t   -Ss report data sums.\n");
	GMT_message (GMT, "\t   -Sw reports weight sums.\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Set Weight options, select one:\n");
	GMT_message (GMT, "\t   -Wi reads 4 cols (x,y,z,w) but writes only (x,y,z[,s,l,h]) output.\n");
	GMT_message (GMT, "\t   -Wo reads 3 cols (x,y,z) but writes sum (x,y,z[,s,l,h],w) output.\n");
	GMT_message (GMT, "\t   -W with no modifier has both weighted Input and Output.\n");
	GMT_message (GMT, "\t   [Default is no weights used].\n");
	GMT_explain_options (GMT, "C0");
	GMT_message (GMT, "\t    Default is 3 columns (or 4 if -W is set).\n");
	GMT_explain_options (GMT, "D0fhioF:.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_blockmean_parse (struct GMTAPI_CTRL *C, struct BLOCKMEAN_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to blockmean and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Report center of block instead */
				Ctrl->C.active = TRUE;
				break;
			case 'E':	/* Extended report with standard deviation, min, and max in cols 4-6 */
				Ctrl->E.active = TRUE;
				break;
			case 'I':	/* Get block dimensions */
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'S':	/* Set report mode for z */
				Ctrl->S.active = TRUE;
				switch (opt->arg[0]) {
#ifdef GMT_COMPAT
					case '\0': case 'z':	/* Report data sums */
						GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -S and -Sz options are deprecated; use -Ss instead.\n");
						Ctrl->S.mode = 1; break;
#endif
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
				Ctrl->W.active = TRUE;
				switch (opt->arg[0]) {
					case '\0':
						Ctrl->W.weighted[GMT_IN] = Ctrl->W.weighted[GMT_OUT] = TRUE; break;
					case 'i': case 'I':
						Ctrl->W.weighted[GMT_IN] = TRUE; break;
					case 'o': case 'O':
						Ctrl->W.weighted[GMT_OUT] = TRUE; break;
					default:
						n_errors++; break;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.active, &Ctrl->I.active);	/* If -R<grdfile> was given we may get incs unless -I was used */

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_binary_io (GMT, (Ctrl->W.weighted[GMT_IN]) ? 4 : 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {GMT_free (GMT, zw); GMT_free (GMT, xy); GMT_free (GMT, np); GMT_free (GMT, slh); Free_blockmean_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout(code);}

GMT_LONG GMT_blockmean (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG row, col, node, n_fields, w_col, error, use_xy, use_weight;
	GMT_LONG n_cells_filled, n_read, n_lost, n_pitched, *np = NULL;

	double weight, weighted_z, iw, wesn[4], out[7], *in = NULL;

	char format[GMT_BUFSIZ];

	struct GMT_OPTION *options = NULL;
	struct GMT_GRID *Grid = NULL;
	struct BLK_PAIR *xy = NULL, *zw = NULL;
	struct BLK_SLH *slh = NULL;
	struct BLOCKMEAN_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/
	
	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	if ((options = GMT_Prep_Options (API, mode, args)) == NULL) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_blockmean_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_blockmean_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_blockmean", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-VRbf:", "aghior>" GMT_OPT("FH"), options)) Return (API->error);
	Ctrl = New_blockmean_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_blockmean_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the blockmean main code ----------------------------*/

	GMT_set_pad (GMT, 0);	/* We are using grid indexing but have no actual grid so no padding is needed */
	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, NULL)) == NULL) Return (API->error);
	GMT_grd_init (GMT, Grid->header, options, FALSE);

	/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, Grid, GMT->common.R.wesn, Ctrl->I.inc, GMT->common.r.active), "stdout");

	use_xy = !Ctrl->C.active;	/* If not -C then we must keep track of x,y locations */
	if ((zw = GMT_memory (GMT, NULL, Grid->header->nm, struct BLK_PAIR)) == NULL) Return (GMT_MEMORY_ERROR);
	if (use_xy && (xy = GMT_memory (GMT, NULL, Grid->header->nm, struct BLK_PAIR)) == NULL) Return (GMT_MEMORY_ERROR);
	if (Ctrl->E.active && (slh = GMT_memory (GMT, NULL, Grid->header->nm, struct BLK_SLH)) == NULL) Return (GMT_MEMORY_ERROR);
	if (Ctrl->W.weighted[GMT_IN] && Ctrl->E.active && (np = GMT_memory (GMT, NULL, Grid->header->nm, GMT_LONG)) == NULL) Return (GMT_MEMORY_ERROR);

	/* Specify input and output expected columns */
	if ((error = GMT_set_cols (GMT, GMT_IN,  3 + Ctrl->W.weighted[GMT_IN])) != GMT_OK) {
		Return (error);
	}
	if ((error = GMT_set_cols (GMT, GMT_OUT, ((Ctrl->W.weighted[GMT_OUT]) ? 4 : 3) + 3 * Ctrl->E.active)) != GMT_OK) {
		Return (error);
	}

	/* Register likely data sources unless the caller has already done so */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, options) != GMT_OK) {	/* Registers default input sources, unless already set */
		Return (API->error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Registers default output destination, unless already set */
		Return (API->error);
	}
	GMT_set_xy_domain (GMT, wesn, Grid->header);	/* wesn may include some padding if gridline-registered */

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		sprintf (format, "W: %s E: %s S: %s N: %s nx: %%ld ny: %%ld\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, format, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->nx, Grid->header->ny);
	}
	
	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {	/* Memory reporting */
		GMT_LONG kind = 0;
		double mem = (double)sizeof (struct BLK_PAIR);
		char *unit = "KMG";	/* Kilo-, Mega-, Giga- */
		if (!Ctrl->C.active) mem += (double)sizeof (struct BLK_PAIR);
		if (Ctrl->E.active)  mem += (double)sizeof (struct BLK_SLH);
		if (Ctrl->W.weighted[GMT_IN] && Ctrl->E.active) mem += (double)sizeof (GMT_LONG);
		mem *= (double)Grid->header->nm;
		mem /= 1024.0;	/* Report kbytes unless it is too much */
		while (mem > 1024.0 && kind < 2) { mem /= 1024.0;	kind++; }	/* Goto next higher unit */
		GMT_report (GMT, GMT_MSG_VERBOSE, "Using a total of %.3g %cb for all arrays.\n", mem, unit[kind]);
	}

	/* Initialize the i/o for doing record-by-record reading/writing */
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_REC) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	n_read = n_pitched = 0;	/* Initialize counters */
	weight = 1.0;		/* Set the default point weight */
	use_weight = (Ctrl->W.weighted[GMT_IN] && !Ctrl->S.mode == 3);	/* Do not use weights if -Sn was set */
	
	/* Read the input data */

	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, &n_fields)) == NULL) {	/* Read next record, get NULL if special case */
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

		col = GMT_grd_x_to_col (GMT, in[GMT_X], Grid->header);
		if (col < 0 || col >= Grid->header->nx) continue;
		row = GMT_grd_y_to_row (GMT, in[GMT_Y], Grid->header);
		if (row < 0 || row >= Grid->header->ny) continue;

		/* OK, this point is definitively inside and will be used */

		if (use_weight) weight = in[3];		/* Use provided weight instead of 1 */
		weighted_z = in[GMT_Z] * weight;			/* Weighted value */
		node = GMT_IJ0 (Grid->header, row, col);		/* Bin node */
		if (use_xy) {						/* Must keep track of weighted location */
			xy[node].a[GMT_X] += (in[GMT_X] * weight);
			xy[node].a[GMT_Y] += (in[GMT_Y] * weight);
		}
		if (Ctrl->E.active) {	/* Add up sum (w*z^2) and n for weighted stdev and keep track of min,max */
			slh[node].a[BLK_S] += (weighted_z * in[GMT_Z]);
			if (Ctrl->W.weighted[GMT_IN]) np[node]++;
			if (zw[node].a[BLK_W] == 0.0) {	/* Initialize low,high the first time */
				slh[node].a[BLK_L] = +DBL_MAX;
				slh[node].a[BLK_H] = -DBL_MAX;
			}
			if (in[GMT_Z] < slh[node].a[BLK_L]) slh[node].a[BLK_L] = in[GMT_Z];
			if (in[GMT_Z] > slh[node].a[BLK_H]) slh[node].a[BLK_H] = in[GMT_Z];
		}
		zw[node].a[BLK_W] += weight;		/* Sum up the weights */
		zw[node].a[BLK_Z] += weighted_z;	/* Sum up the weighted values */
		n_pitched++;				/* Number of points actually used */
	} while (TRUE);
	
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	/* Done with reading (files are automatically closed by i/o-machinery) */
	
	if (n_read == 0) {	/* Blank/empty input files */
		GMT_report (GMT, GMT_MSG_NORMAL, "No data records found; no output produced");
		Return (EXIT_SUCCESS);
	}
	if (n_pitched == 0) {	/* No points inside region */
		GMT_report (GMT, GMT_MSG_NORMAL, "No data points found inside the region; no output produced");
		Return (EXIT_SUCCESS);
	}

	w_col = GMT_get_cols (GMT, GMT_OUT) - 1;	/* Index of weight column (the last output column) */
	n_cells_filled = 0;				/* Number of blocks with values */

	GMT_report (GMT, GMT_MSG_NORMAL, "Calculating block means\n");

	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC) != GMT_OK) {	/* Enables data output and sets access mode */
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
			col = GMT_col (Grid->header, node);
			row = GMT_row (Grid->header, node);
			out[GMT_X] = GMT_grd_col_to_x (GMT, col, Grid->header);
			out[GMT_Y] = GMT_grd_row_to_y (GMT, row, Grid->header);
		}
		if (Ctrl->S.mode)	/* Report block sums or weights */
			out[GMT_Z] = (Ctrl->S.mode >= 2) ? zw[node].a[BLK_W] : zw[node].a[BLK_Z];
		else			/* Report block means */
			out[GMT_Z] = zw[node].a[BLK_Z] * iw;
		if (Ctrl->E.active) {	/* Compute and report extended attributes */
			if (Ctrl->W.weighted[GMT_IN]) {	/* Weighted standard deviation */
				out[3] = (np[node] > 1) ? d_sqrt ((zw[node].a[BLK_W] * slh[node].a[BLK_S] - zw[node].a[BLK_Z] * zw[node].a[BLK_Z]) \
				/ (zw[node].a[BLK_W] * zw[node].a[BLK_W] * ((np[node] - 1.0) / np[node]))) : GMT->session.d_NaN;
			}
			else {					/* Normal standard deviation */
				out[3] = (zw[node].a[BLK_W] > 1.0) ? d_sqrt ((zw[node].a[BLK_W] * slh[node].a[BLK_S] - zw[node].a[BLK_Z] * zw[node].a[BLK_Z]) \
				/ (zw[node].a[BLK_W] * (zw[node].a[BLK_W] - 1.0))) : GMT->session.d_NaN;
			}
			out[4] = slh[node].a[BLK_L];	/* Minimum value in block */
			out[5] = slh[node].a[BLK_H];	/* Maximum value in block */
		}
		GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */
	}
	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_OK) {	/* Disables further data output */
		Return (API->error);
	}

	n_lost = n_read - n_pitched;	/* Number of points that did not get used */
	GMT_report (GMT, GMT_MSG_NORMAL, "N read: %ld N used: %ld N outside_area: %ld N cells filled: %ld\n", n_read, n_pitched, n_lost, n_cells_filled);

	GMT_free_grid (GMT, &Grid, FALSE);	/* Free directly since not registered as an i/o resource */
	GMT_set_pad (GMT, 2);			/* Restore to GMT padding defaults */

	Return (GMT_OK);
}
