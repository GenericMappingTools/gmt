/*--------------------------------------------------------------------
 *    $Id$
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
 * API functions to support the blockmode application.
 *
 * Author:	Walter H.F. Smith
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: reads records of x, y, data, [weight] and writes out mode
 * value per cell, where cellular region is bounded by West East South North
 * and cell dimensions are delta_x, delta_y.
 */

#define BLOCKMODE	/* Since mean, median, mode share near-similar macros we require this setting */

#include "gmt.h"
#include "block_subs.h"

GMT_LONG GMT_blockmode_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "blockmode %s [API] - Block average (x,y,z) data tables by mode estimation\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: blockmode [<table>] %s %s\n", GMT_I_OPT, GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[-C] [-E] [-Q] [%s] [-W[i][o]] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_r_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_inc_syntax (GMT, 'I', 0);
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-C Output center of block and mode z-value  [Default is mode location (but see -Q)].\n");
	GMT_message (GMT, "\t-E Extend output with LMS scale (s), low (l), and high (h) value per block, i.e.,\n");
	GMT_message (GMT, "\t   output (x,y,z,s,l,h[,w]) [Default outputs (x,y,z[,w]); see -W regarding w.\n");
	GMT_message (GMT, "\t-Q Quicker; get mode z and mean x,y [Default gets mode x, mode y, mode z].\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Set Weight options.\n");
	GMT_message (GMT, "\t   -Wi reads Weighted Input (4 cols: x,y,z,w) but writes only (x,y,z[,s,l,h]) Output.\n");
	GMT_message (GMT, "\t   -Wo reads unWeighted Input (3 cols: x,y,z) but reports sum (x,y,z[,s,l,h],w) Output.\n");
	GMT_message (GMT, "\t   -W with no modifier has both weighted Input and Output; Default is no weights used.\n");
	GMT_explain_options (GMT, "C0");
	GMT_message (GMT, "\t    Default is 3 columns (or 4 if -W is set).\n");
	GMT_explain_options (GMT, "D0fhioF:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_blockmode_parse (struct GMTAPI_CTRL *C, struct BLOCKMODE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to blockmode and sets parameters in CTRL.
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
			case 'E':
				Ctrl->E.active = TRUE;		/* Extended report with standard deviation, min, and max in cols 4-6 */
				break;
			case 'I':	/* Get block dimensions */
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'Q':	/* Quick mode for modal z */
				Ctrl->Q.active = TRUE;
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

double weighted_mode (struct BLK_DATA *d, double wsum, GMT_LONG n, GMT_LONG k)
{
	/* Estimate mode by finding a maximum in the estimated
	   pdf of weighted data.  Estimate the pdf as the finite
	   difference of the cumulative frequency distribution
	   over points from i to j.  This has the form top/bottom,
	   where top is the sum of the weights from i to j, and
	   bottom is (data[j] - data[i]).  Strategy is to start
	   with i=0, j=n-1, and then move i or j toward middle
	   while j-i > n/2 and bottom > 0.  At end while, midpoint
	   of range from i to j is the mode estimate.  Choose
	   to move either i or j depending on which one will
	   cause greatest increase in pdf estimate.  If a tie,
	   move both.

	   Strictly, the pdf estimated this way would need to be
	   scaled by (1/wsum), but this is constant so we don't
	   use it here, as we are seeking a relative minimum.

	   I assumed n > 2 when I wrote this. [WHFS] */

	double top, topj, topi, bottomj, bottomi, pj, pi;
	GMT_LONG i = 0, j = n - 1, nh = n / 2;

	top = wsum;

	while ((j-i) > nh) {
		topi = top - d[i].a[BLK_W];
		topj = top - d[j].a[BLK_W];
		bottomi = d[j].a[k] - d[i+1].a[k];
		bottomj = d[j-1].a[k] - d[i].a[k];

		if (bottomj == 0.0) return (d[j-1].a[k]);
		if (bottomi == 0.0) return (d[i+1].a[k]);
		pi = topi / bottomi;
		pj = topj / bottomj;
		if (pi > pj) {
			i++;
			top = topi;
		}
		else if (pi < pj) {
			j--;
			top = topj;
		}
		else {
			top -= (d[i].a[BLK_W] + d[j].a[BLK_W]);
			i++;
			j--;
		}
	}
	return (0.5 * (d[j].a[k] + d[i].a[k]));
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_blockmode_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_blockmode (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG error = FALSE, mode_xy, col, row, n_fields, w_col, n_pitched;
	GMT_LONG node, first_in_cell, first_in_new_cell, n_lost, n_read;
	GMT_LONG n_cells_filled, n_in_cell, n_alloc = 0, nz_alloc = 0, nz;

	double out[7], wesn[4], i_n_in_cell, weight, *in = NULL, *z_tmp = NULL;

	char format[GMT_BUFSIZ];

	struct GMT_OPTION *options = NULL;
	struct GMT_GRID *Grid = NULL;
	struct BLK_DATA *data = NULL;
	struct BLOCKMODE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_blockmode_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_blockmode_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_blockmode", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRbf:", "ghior>" GMT_OPT("FH"), options))) Return (error);
	Ctrl = New_blockmode_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_blockmode_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the blockmode main code ----------------------------*/

	if (Ctrl->C.active && Ctrl->Q.active) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: -C overrides -Q\n");
		Ctrl->Q.active = FALSE;
	}

	GMT_set_pad (GMT, 0);	/* We are using grid indexing but have no actual grid so no padding is needed */
	Grid = GMT_Create_Data (API, GMT_IS_GRID, NULL);
	GMT_grd_init (GMT, Grid->header, options, FALSE);

	/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
	GMT_err_fail (GMT, GMT_init_newgrid (GMT, Grid, GMT->common.R.wesn, Ctrl->I.inc, GMT->common.r.active), "stdout");

	mode_xy = !Ctrl->C.active;

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		sprintf (format, "W: %s E: %s S: %s N: %s nx: %%ld ny: %%ld\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, format, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI], Grid->header->nx, Grid->header->ny);
	}

	GMT_set_xy_domain (GMT, wesn, Grid->header);	/* May include some padding if gridline-registered */

	/* Specify input and output expected columns */
	if ((error = GMT_set_cols (GMT, GMT_IN,  3 + Ctrl->W.weighted[GMT_IN]))) Return (error);
	if ((error = GMT_set_cols (GMT, GMT_OUT, ((Ctrl->W.weighted[GMT_OUT]) ? 4 : 3) + 3 * Ctrl->E.active))) Return (error);

	/* Register likely data sources unless the caller has already done so */
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN,  GMT_REG_DEFAULT, options))) Return (error);	/* Registers default input sources, unless already set */
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_OUT, GMT_REG_DEFAULT, options))) Return (error);	/* Registers default output destination, unless already set */

	/* Initialize the i/o for doing record-by-record reading/writing */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN,  GMT_BY_REC))) Return (error);				/* Enables data input and sets access mode */

	n_read = n_pitched = 0;	/* Initialize counters */

	/* Read the input data */

	while ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, &n_fields))) {	/* Keep returning records until we reach EOF */

		if (GMT_REC_IS_ERROR (GMT)) Return (GMT_RUNTIME_ERROR);	/* Bail if there are any read errors */
		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;		/* Skip all table and segment headers */
		if (GMT_is_dnan (in[GMT_Z])) continue;			/* Skip if z = NaN */

		/* Data record to process */

		n_read++;						/* Number of records read */

		if (GMT_y_is_outside (GMT, in[GMT_Y], wesn[YLO], wesn[YHI])) continue;	/* Outside y-range */
		if (GMT_x_is_outside (GMT, &in[GMT_X], wesn[XLO], wesn[XHI])) continue;	/* Outside x-range (or longitude) */

		/* We appear to be inside: Get row and col indices of this block */

		col = GMT_grd_x_to_col (GMT, in[GMT_X], Grid->header);
		if (col < 0 || col >= Grid->header->nx ) continue;
		row = GMT_grd_y_to_row (GMT, in[GMT_Y], Grid->header);
		if (row < 0 || row >= Grid->header->ny ) continue;

		/* OK, this point is definitively inside and will be used */

		node = GMT_IJP (Grid->header, row, col);		/* Bin node */

		if (n_pitched == n_alloc) data = GMT_malloc (GMT, data, n_pitched, &n_alloc, struct BLK_DATA);
		data[n_pitched].i = node;
		if (mode_xy) {	/* Need to store (x,y) so we can compute modal location later */
			data[n_pitched].a[GMT_X] = in[GMT_X];
			data[n_pitched].a[GMT_Y] = in[GMT_Y];
		}
		data[n_pitched].a[BLK_Z] = in[GMT_Z];
		data[n_pitched].a[BLK_W] = (Ctrl->W.weighted[GMT_IN]) ? in[3] : 1.0;

		n_pitched++;
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (n_read == 0) {	/* Blank/empty input files */
		GMT_report (GMT, GMT_MSG_NORMAL, "No data records found; no output produced");
		Return (EXIT_SUCCESS);
	}
	if (n_pitched == 0) {	/* No points inside region */
		GMT_report (GMT, GMT_MSG_NORMAL, "No data points found inside the region; no output produced");
		Return (EXIT_SUCCESS);
	}

	if (n_pitched < n_alloc) {
		n_alloc = n_pitched;
		data = GMT_malloc (GMT, data, 0, &n_alloc, struct BLK_DATA);
	}

	/* Ready to go. */

	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_REC))) Return (error);	/* Enables data output and sets access mode */

	w_col = GMT_get_cols (GMT, GMT_OUT) - 1;	/* Weights always reported in last output column */

	/* Sort on node and Z value */

	qsort (data, n_pitched, sizeof (struct BLK_DATA), BLK_compare_index_z);

	/* Find n_in_cell and write appropriate output  */

	first_in_cell = n_cells_filled = nz = 0;
	while (first_in_cell < n_pitched) {
		weight = data[first_in_cell].a[BLK_W];
		if (Ctrl->E.active) {
			if (nz == nz_alloc) z_tmp = GMT_malloc (GMT, z_tmp, nz, &nz_alloc, double);
			z_tmp[0] = data[first_in_cell].a[BLK_Z];
			nz = 1;
		}
		if (Ctrl->C.active) {	/* Use block center */
			row = GMT_row (Grid->header, data[first_in_cell].i);
			col = GMT_col (Grid->header, data[first_in_cell].i);
			out[GMT_X] = GMT_grd_col_to_x (GMT, col, Grid->header);
			out[GMT_Y] = GMT_grd_row_to_y (GMT, row, Grid->header);
		}
		else {
			out[GMT_X] = data[first_in_cell].a[GMT_X];
			out[GMT_Y] = data[first_in_cell].a[GMT_Y];
		}
		first_in_new_cell = first_in_cell + 1;
		while ((first_in_new_cell < n_pitched) && (data[first_in_new_cell].i == data[first_in_cell].i)) {
			weight += data[first_in_new_cell].a[BLK_W];	/* Summing up weights */
			if (mode_xy) {
				out[GMT_X] += data[first_in_new_cell].a[GMT_X];
				out[GMT_Y] += data[first_in_new_cell].a[GMT_Y];
			}
			if (Ctrl->E.active) {	/* Must get a temporary copy of the sorted z array */
				if (nz == nz_alloc) z_tmp = GMT_malloc (GMT, z_tmp, nz, &nz_alloc, double);
				z_tmp[nz] = data[first_in_new_cell].a[BLK_Z];
				nz++;
			}
			first_in_new_cell++;
		}
		n_in_cell = first_in_new_cell - first_in_cell;
		if (n_in_cell > 2) {	/* data are already sorted on z; get z mode  */
			out[GMT_Z] = weighted_mode (&data[first_in_cell], weight, n_in_cell, 2);
			if (Ctrl->Q.active) {
				i_n_in_cell = 1.0 / n_in_cell;
				out[GMT_X] *= i_n_in_cell;
				out[GMT_Y] *= i_n_in_cell;
			}
			else if (mode_xy) {
				qsort (&data[first_in_cell], (size_t)n_in_cell, sizeof (struct BLK_DATA), BLK_compare_x);
				out[GMT_X] = weighted_mode (&data[first_in_cell], weight, n_in_cell, 0);

				qsort (&data[first_in_cell], (size_t)n_in_cell, sizeof (struct BLK_DATA), BLK_compare_y);
				out[GMT_Y] = weighted_mode (&data[first_in_cell], weight, n_in_cell, 1);
			}
		}
		else if (n_in_cell == 2) {
			if (data[first_in_cell].a[BLK_W] > data[first_in_cell+1].a[BLK_W]) {
				out[GMT_Z] = data[first_in_cell].a[BLK_Z];
				if (Ctrl->Q.active) {
					out[GMT_X] *= 0.5;
					out[GMT_Y] *= 0.5;
				}
				else if (mode_xy) {
					out[GMT_X] = data[first_in_cell].a[GMT_X];
					out[GMT_Y] = data[first_in_cell].a[GMT_Y];
				}
			}
			else if (data[first_in_cell].a[BLK_W] < data[first_in_cell+1].a[BLK_W]) {
				out[GMT_Z] = data[first_in_cell+1].a[BLK_Z];
				if (Ctrl->Q.active) {
					out[GMT_X] *= 0.5;
					out[GMT_Y] *= 0.5;
				}
				else if (mode_xy) {
					out[GMT_X] = data[first_in_cell+1].a[GMT_X];
					out[GMT_Y] = data[first_in_cell+1].a[GMT_Y];
				}
			}
			else {
				if (mode_xy) {	/* Need average location */
					out[GMT_X] *= 0.5;
					out[GMT_Y] *= 0.5;
				}
				out[GMT_Z] = 0.5 * (data[first_in_cell].a[BLK_Z] + data[first_in_cell+1].a[BLK_Z]);
			}
		}
		else
			out[GMT_Z] = data[first_in_cell].a[BLK_Z];

		if (Ctrl->E.active) {
			out[4] = z_tmp[0];	/* Low value */
			out[5] = z_tmp[nz-1];	/* High value */
			/* Turn z_tmp into absolute deviations from the mode (out[GMT_Z]) */
			if (nz > 1) {
				for (node = 0; node < nz; node++) z_tmp[node] = fabs (z_tmp[node] - out[GMT_Z]);
				GMT_sort_array (GMT, z_tmp, nz, GMTAPI_DOUBLE);
				out[3] = (nz%2) ? z_tmp[nz/2] : 0.5 * (z_tmp[(nz-1)/2] + z_tmp[nz/2]);
				out[3] *= 1.4826;	/* This will be LMS MAD-based scale */
			}
			else
				out[3] = GMT->session.d_NaN;
		}
		if (Ctrl->W.weighted[GMT_OUT]) out[w_col] = weight;

		GMT_Put_Record (API, GMT_WRITE_DOUBLE, out);	/* Write this to output */

		n_cells_filled++;
		first_in_cell = first_in_new_cell;
	}
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */

	n_lost = n_read - n_pitched;	/* Number of points that did not get used */
	GMT_report (GMT, GMT_MSG_NORMAL, "N read: %ld N used: %ld N outside_area: %ld N cells filled: %ld\n", n_read, n_pitched, n_lost, n_cells_filled);

	GMT_free_grid (GMT, &Grid, FALSE);	/* Free directly since not registered as a i/o resource */
	GMT_free (GMT, data);
	GMT_free (GMT, z_tmp);

	GMT_set_pad (GMT, 2);			/* Restore to GMT padding defaults */

	Return (GMT_OK);
}
