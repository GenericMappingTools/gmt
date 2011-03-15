/*--------------------------------------------------------------------
 *    $Id: grdblend_func.c,v 1.2 2011-03-15 02:06:36 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * grdblend reads any number of grid files that may partly overlap and
 * creates a blend of all the files given certain criteria.  Each input
 * grid is considered to have an "outer" and "inner" region.  The outer
 * region is the extent of the grid; the inner region is provided as
 * input in the form of a -Rw/e/s/n statement.  Finally, each grid can
 * be assigned its separate weight.  This information is given to the
 * program in ASCII format, one line per grid file; each line looks like
 *
 * grdfile	-Rw/e/s/n	weight
 *
 * The blending will use a 2-D cosine taper between the inner and outer
 * regions.  The output at any node is thus a weighted average of the
 * values from any grid that occupies that grid node.  Because the out-
 * put grid can be really huge (say global grids at fine resolution),
 * all grid input/output is done row by row so memory should not be a
 * limiting factor in making large grid.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#include "gmt.h"

struct GRDBLEND_CTRL {
	struct G {	/* -G<grdfile> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double xinc, yinc;
	} I;
	struct N {	/* -N<nodata> */
		GMT_LONG active;
		double nodata;
	} N;
	struct Q {	/* -Q */
		GMT_LONG active;
	} Q;
	struct Z {	/* -Z<scale> */
		GMT_LONG active;
		double scale;
	} Z;
	struct W {	/* -W */
		GMT_LONG active;
	} W;
};

struct GRDBLEND_INFO {	/* Structure with info about each input grid file */
	struct GMT_GRDFILE G;				/* I/O structure for grid files, including grd header */
	GMT_LONG in_i0, in_i1, out_i0, out_i1;		/* Indices of outer and inner x-coordinates (in output grid coordinates) */
	GMT_LONG in_j0, in_j1, out_j0, out_j1;		/* Indices of outer and inner y-coordinates (in output grid coordinates) */
	GMT_LONG offset;				/* grid offset when the grid extends beyond north */
	long skip;					/* Byte offset to skip in native binary files */
	GMT_LONG outside;				/* TRUE if the current output row is outside the range of this grid */
	GMT_LONG invert;					/* TRUE if weight was given as negative and we want to taper to zero INSIDE the grid region */
	GMT_LONG open;					/* TRUE if file is currently open */
	char file[GMT_LONG_TEXT];			/* Name of grid file */
	double weight, wt_y, wxr, wxl, wyu, wyd;	/* Various weighting factors used for cosine-taper weights */
	double wesn[4];					/* Boundaries of inner region */
	float *z;					/* Row vector holding the current row from this file */
};

EXTERN_MSC GMT_LONG GMT_update_grd_info (struct GMT_CTRL *C, char *file, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_grd_get_format (struct GMT_CTRL *C, char *file, struct GRD_HEADER *header, GMT_LONG magic);

void decode_R (struct GMT_CTRL *GMT, char *string, double wesn[]) {
	GMT_LONG i, pos, error = 0;
	char text[BUFSIZ];

	/* Needed to decode the inner region -Rw/e/s/n string */

	i = pos = 0;
	while (!error && (GMT_strtok (string, "/", &pos, text))) {
		error += GMT_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][i/2], GMT_scanf_arg (GMT, text, GMT->current.io.col_type[GMT_IN][i/2], &wesn[i]), text);
		i++;
	}
	if (error || (i != 4) || GMT_check_region (GMT, wesn)) {
		GMT_syntax (GMT, 'R');
	}
}

GMT_LONG init_blend_job (struct GMT_CTRL *GMT, struct GRD_HEADER *h, struct GRDBLEND_INFO **blend) {
	GMT_LONG n = 0, nr, one_or_zero = 0, n_alloc = 0, type, n_fields;
	struct GRDBLEND_INFO *B = NULL;
	char *line = NULL, r_in[GMT_LONG_TEXT], *sense[2] = {"normal", "inverse"};

	GMT_set_meminc (GMT, GMT_SMALL_CHUNK);
	while ((n_fields = GMT_Get_Record (GMT->parent, GMT_READ_TEXT, (void **)&line)) != EOF) {	/* Keep returning records until we have no more files */
		if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;	/* Skip comment lines or blank lines */

		if (n == n_alloc) n_alloc = GMT_malloc (GMT, B, n, n_alloc, struct GRDBLEND_INFO);
		GMT_memset (&B[n], 1, struct GRDBLEND_INFO);	/* Initialize memory */
		nr = sscanf (line, "%s %s %lf", B[n].file, r_in, &B[n].weight);
		if (nr != 3) {
			GMT_report (GMT, GMT_MSG_FATAL, "Read error for blending parameters near row %ld\n", n);
			return (EXIT_FAILURE);
		}
		GMT_err_fail (GMT, GMT_read_grd_info (GMT, B[n].file, &B[n].G.header), B[n].file);	/* Read header structure */
		if (!strcmp (r_in, "-")) {	/* Set inner = outer region */
			B[n].wesn[XLO] = B[n].G.header.wesn[XLO];	B[n].wesn[XHI] = B[n].G.header.wesn[XHI];
			B[n].wesn[YLO] = B[n].G.header.wesn[YLO];	B[n].wesn[YHI] = B[n].G.header.wesn[YHI];
		}
		else	/* Must decode the -R string */
			decode_R (GMT, &r_in[2], B[n].wesn);	/* Decode inner region */

		/* Skip the file if its outer region does not lie within the final grid region */
		if (h->wesn[XLO] > B[n].wesn[XHI] || h->wesn[XHI] < B[n].wesn[XLO] || h->wesn[YLO] > B[n].wesn[YHI] || h->wesn[YHI] < B[n].wesn[YLO]) {
			GMT_report (GMT, GMT_MSG_FATAL, "Warning: File %s entirely outside final grid region (skipped)\n", B[n].file);
			continue;
		}

		/* Various sanity checking - e.g., all grid of same registration type and grid spacing */

		if (n == 0) {
			h->registration = B[n].G.header.registration;
			one_or_zero = !h->registration;
			GMT_RI_prepare (GMT, h);	/* Ensure -R -I consistency and set nx, ny */
		}
		if (h->registration != B[n].G.header.registration){
			GMT_report (GMT, GMT_MSG_FATAL, "File %s has different registration than the first file given\n", B[n].file);
			return (EXIT_FAILURE);
		}
		if (!GMT_IS_ZERO (B[n].G.header.inc[GMT_X] - h->inc[GMT_X])){
			GMT_report (GMT, GMT_MSG_FATAL, "File %s has different x-increment (%g) than the chosen output increment (%g)\n", B[n].file, B[n].G.header.inc[GMT_X], h->inc[GMT_X]);
			return (EXIT_FAILURE);
		}
		if (!GMT_IS_ZERO (B[n].G.header.inc[GMT_Y] - h->inc[GMT_Y])){
			GMT_report (GMT, GMT_MSG_FATAL, "File %s has different y-increment (%g) than the chosen output increment (%g)\n", B[n].file, B[n].G.header.inc[GMT_Y], h->inc[GMT_Y]);
			return (EXIT_FAILURE);
		}
		if (B[n].weight < 0.0) {	/* Negative weight means invert sense of taper */
			B[n].weight = fabs (B[n].weight);
			B[n].invert = TRUE;
		}

		GMT_err_fail (GMT, GMT_open_grd (GMT, B[n].file, &B[n].G, 'r'), B[n].file);	/* Open the grid for incremental row reading */

		/* Here, i0, j0 is the very first col, row to read, while i1, j1 is the very last col, row to read .
		 * Weights at the outside i,j should be 0, and reach 1 at the edge of the inside block */

		/* The following works for both pixel and grid-registered grids since we are here using the i,j to measure the width of the
		 * taper zone in units of dx, dy. */
		 
		B[n].out_i0 = irint ((B[n].G.header.wesn[XLO] - h->wesn[XLO]) / h->inc[GMT_X]);
		B[n].in_i0  = irint ((B[n].wesn[XLO] - h->wesn[XLO]) / h->inc[GMT_X]) - 1;
		B[n].in_i1  = irint ((B[n].wesn[XHI] - h->wesn[XLO]) / h->inc[GMT_X]) + one_or_zero;
		B[n].out_i1 = irint ((B[n].G.header.wesn[XHI] - h->wesn[XLO]) / h->inc[GMT_X]) - B[n].G.header.registration;
		B[n].out_j0 = irint ((h->wesn[YHI] - B[n].G.header.wesn[YHI]) / h->inc[GMT_Y]);
		B[n].in_j0  = irint ((h->wesn[YHI] - B[n].wesn[YHI]) / h->inc[GMT_Y]) - 1;
		B[n].in_j1  = irint ((h->wesn[YHI] - B[n].wesn[YLO]) / h->inc[GMT_Y]) + one_or_zero;
		B[n].out_j1 = irint ((h->wesn[YHI] - B[n].G.header.wesn[YLO]) / h->inc[GMT_Y]) - B[n].G.header.registration;

		B[n].wxl = M_PI * h->inc[GMT_X] / (B[n].wesn[XLO] - B[n].G.header.wesn[XLO]);
		B[n].wxr = M_PI * h->inc[GMT_X] / (B[n].G.header.wesn[XHI] - B[n].wesn[XHI]);
		B[n].wyu = M_PI * h->inc[GMT_Y] / (B[n].G.header.wesn[YHI] - B[n].wesn[YHI]);
		B[n].wyd = M_PI * h->inc[GMT_Y] / (B[n].wesn[YLO] - B[n].G.header.wesn[YLO]);

		if (B[n].out_j0 < 0) {	/* Must skip to first row inside the present -R */
			type = GMT_grdformats[B[n].G.header.type][0];
			if (type == 'c')	/* Old-style, 1-D netcdf grid */
				B[n].offset = B[n].G.header.nx * GMT_abs (B[n].out_j0);
			else if (type == 'n')	/* New, 2-D netcdf grid */
				B[n].offset = B[n].out_j0;
			else
				B[n].skip = (long)(B[n].G.n_byte * GMT_abs (B[n].out_j0));	/* do the fseek when we are ready to read first row */
		}

		/* Allocate space for one entire row */

		B[n].z = GMT_memory (GMT, NULL, B[n].G.header.nx, float);
		GMT_report (GMT, GMT_MSG_NORMAL, "Blend file %s in %g/%g/%g/%g with %s weight %g [%ld-%ld]\n",
			B[n].G.header.name, B[n].wesn[XLO], B[n].wesn[XHI], B[n].wesn[YLO], B[n].wesn[YHI], sense[B[n].invert], B[n].weight, B[n].out_j0, B[n].out_j1);

		GMT_close_grd (GMT, &B[n].G);

		n++;
	}

	n_alloc = GMT_malloc (GMT, B, 0, n, struct GRDBLEND_INFO);
	*blend = B;
	GMT_reset_meminc (GMT);

	return (n);
}

void sync_input_rows (struct GMT_CTRL *GMT, GMT_LONG row, struct GRDBLEND_INFO *B, GMT_LONG n_blend, double half) {
	GMT_LONG k;

	for (k = 0; k < n_blend; k++) {	/* Get every input grid ready for the new row */
		if (row < B[k].out_j0 || row > B[k].out_j1) {	/* Either done with grid or haven't gotten to this range yet */
			B[k].outside = TRUE;
			if (B[k].open) {
				GMT_close_grd (GMT, &B[k].G);	/* Done with this file */
				B[k].open = FALSE;
				GMT_free (GMT, B[k].z);
			}
			continue;
		}
		B[k].outside = FALSE;
		if (row <= B[k].in_j0)		/* Top cosine taper weight */
			B[k].wt_y = 0.5 * (1.0 - cos ((row - B[k].out_j0 + half) * B[k].wyu));
		else if (row >= B[k].in_j1)	/* Bottom cosine taper weight */
			B[k].wt_y = 0.5 * (1.0 - cos ((B[k].out_j1 - row + half) * B[k].wyd));
		else				/* We are inside the inner region; y-weight = 1 */
			B[k].wt_y = 1.0;
		B[k].wt_y *= B[k].weight;

		if (!B[k].open) {
			GMT_err_fail (GMT, GMT_open_grd (GMT, B[k].file, &B[k].G, 'r'), B[k].file);	/* Open the grid for incremental row reading */
			if (B[k].skip) GMT_fseek (B[k].G.fp, B[k].skip, SEEK_CUR);	/* Position for native binary files */
			B[k].G.start[0] += B[k].offset;					/* Start position for netCDF files */
			B[k].open = TRUE;
		}

		GMT_err_fail (GMT, GMT_read_grd_row (GMT, &B[k].G, 0, B[k].z), B[k].file);	/* Get one row from this file */
	}
}

void *New_grdblend_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDBLEND_CTRL *C = NULL;
	
	C = GMT_memory (GMT, NULL, 1, struct GRDBLEND_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	
	C->N.nodata = GMT->session.d_NaN;
	C->Z.scale = 1.0;
	
	return ((void *)C);
}

void Free_grdblend_Ctrl (struct GMT_CTRL *GMT, struct GRDBLEND_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free ((void *)C->G.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdblend_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdblend %s [API] - Blend several partially over-lapping grid files onto one grid\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdblend [<blendfile>] -G<grdfile> %s\n", GMT_I_OPT);
	GMT_message (GMT, "\t%s [-N<nodata>] [-Q] [%s] [-W] [-Z<scale>]\n\t[%s]\n", GMT_Rgeo_OPT, GMT_V_OPT, GMT_f_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<blendfile> is an ASCII file (or stdin) with blending parameters for each input grid\n");
	GMT_message (GMT, "\t   Each record has three items:  filename -Rw/e/s/n weight\n");
	GMT_message (GMT, "\t   Relative weights are <weight> inside the given -R and cosine taper to 0 at actual grid -R.\n");
	GMT_message (GMT, "\t   Give filename - weight if inner region should equal the actual region\n");
	GMT_message (GMT, "\t   Give a negative weight to invert the sense of the taper (i.e., |<weight>| outside given R.\n");
	GMT_message (GMT, "\t-G <grdfile> is the name of the final 2-D binary data set\n");
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-N Set value for nodes without constraints [Default is NaN]\n");
	GMT_message (GMT, "\t-Q Grdraster-compatible output without leading grd header [Default writes GMT grid file]\n");
	GMT_message (GMT, "\t   Output grid must be in native binary format (i.e., not netCDF).\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Write out weights only (only applies to a single input file) [make blend grid]\n");
	GMT_message (GMT, "\t-Z Multiply z-values by this scale before writing to file [1]\n");
	GMT_explain_options (GMT, "f.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdblend_parse (struct GMTAPI_CTRL *C, struct GRDBLEND_CTRL *Ctrl, struct GMT_GRDFILE *S, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdblend and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, err;
	char type;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'G':	/* Output filename */
				Ctrl->G.file = strdup (opt->arg);
				Ctrl->G.active = TRUE;
				break;
			case 'I':	/* Grid spacings */
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, &Ctrl->I.xinc, &Ctrl->I.yinc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'N':	/* NaN-value */
				Ctrl->N.active = TRUE;
				if (opt->arg[0])
					Ctrl->N.nodata = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.d_NaN : atof (opt->arg);
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "GMT SYNTAX ERROR -N option:  Must specify value or NaN\n");
					n_errors++;
				}
				break;
			case 'Q':	/* No header on output */
				Ctrl->Q.active = TRUE;
				break;
			case 'W':	/* Write weights instead */
				Ctrl->W.active = TRUE;
				break;
			case 'Z':	/* z-multiplier */
				Ctrl->Z.active = TRUE;
				Ctrl->Z.scale = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, &Ctrl->I.xinc, &Ctrl->I.yinc, NULL, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "GMT SYNTAX ERROR -R:  Must specify region\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.xinc <= 0.0 || Ctrl->I.yinc <= 0.0, "GMT SYNTAX ERROR -I:  Must specify positive dx, dy\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "GMT SYNTAX ERROR -G:  Must specify output file\n");
	if ((err = GMT_grd_get_format (GMT, Ctrl->G.file, &(S->header), FALSE)) != GMT_NOERROR){
		GMT_report (GMT, GMT_MSG_FATAL, "GMT SYNTAX ERROR: %s [%s]\n", GMT_strerror(err), Ctrl->G.file); n_errors++;
	}
	type = (char)GMT_grdformats[S->header.type][0];
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && (type == 'c' || type == 'n'), "GMT SYNTAX ERROR -Q:  Output grid file cannot be netCDF format\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdblend_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdblend (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG col, pcol, row, nx_360 = 0, k, kk, m, n_blend, error, n_fill, n_tot, ij, wrap_x;
	
	double wt_x, w, wt;
	
	float *z = NULL, no_data_f;
	
	char mode[2] = {'w', 'W'};
	
	struct GRDBLEND_INFO *blend = NULL;
	struct GMT_GRDFILE S;
	struct GMT_GRID *Grid = NULL;
	struct GRDBLEND_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdblend_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdblend_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdblend", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRf:", "", options))) Return (error);
	GMT_grd_init (GMT, &S.header, options, FALSE);
	Ctrl = (struct GRDBLEND_CTRL *) New_grdblend_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdblend_parse (API, Ctrl, &S, options))) Return (error);
	
	/*---------------------------- This is the grdblend main code ----------------------------*/

	n_fill = n_tot = 0;

	S.header.inc[GMT_X] = Ctrl->I.xinc;
	S.header.inc[GMT_Y] = Ctrl->I.yinc;
	
	GMT_RI_prepare (GMT, &S.header);	/* Ensure -R -I consistency and set nx, ny */

	/* Process blend parameters and populate blend structure and open input files and seek to first row inside the output grid */

	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Register data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_REC))) Return (error);				/* Enables data input and sets access mode */

	n_blend = init_blend_job (GMT, &S.header, &blend);

	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (Ctrl->W.active && n_blend > 1) {
		GMT_report (GMT, GMT_MSG_FATAL, "GMT SYNTAX ERROR -W:  Only applies when there is a single input grid file\n");
		Return (EXIT_FAILURE);
	}

	no_data_f = (float)Ctrl->N.nodata;

	/* Initialize header structure for output blend grid */

	S.header.z_min = S.header.z_max = 0.0;

	GMT_err_fail (GMT, GMT_grd_RI_verify (GMT, &S.header, 1), Ctrl->G.file);

	n_tot = GMT_get_nm (S.header.nx, S.header.ny);

	z = GMT_memory (GMT, NULL, S.header.nx, float);	/* Memory for one output row */

	if (GMT_File_Is_Memory (Ctrl->G.file)) {	/* GMT_grdblend is called by another module; must return as GMT_GRID */
		Grid = GMT_create_grid (GMT);
		GMT_grd_init (GMT, Grid->header, options, FALSE);

		/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
		GMT_err_fail (GMT, GMT_init_newgrid (GMT, Grid, GMT->common.R.wesn, Ctrl->I.xinc, Ctrl->I.yinc, S.header.registration), Ctrl->G.file);
		Grid->data = GMT_memory (GMT, NULL, Grid->header->size, float);	/* Memory for the entire padded grid */
	}
	else {
		if (!Ctrl->Q.active) GMT_err_fail (GMT, GMT_write_grd_info (GMT, Ctrl->G.file, &S.header), Ctrl->G.file);

		GMT_err_fail (GMT, GMT_open_grd (GMT, Ctrl->G.file, &S, mode[Ctrl->Q.active]), Ctrl->G.file);
	}
	
	if (Ctrl->Z.active) GMT_report (GMT, GMT_MSG_NORMAL, "Output data will be scaled by %g\n", Ctrl->Z.scale);

	S.header.z_min = DBL_MAX;	/* These will be updated in the loop below */
	S.header.z_max = -DBL_MAX;
	wrap_x = (GMT_is_geographic (GMT, GMT_OUT));	/* Periodic geographic grid */
	if (wrap_x) nx_360 = irint (360.0 / S.header.inc[GMT_X]);

	for (row = 0; row < S.header.ny; row++) {	/* For every output row */

		GMT_memset (z, S.header.nx, float);	/* Start from scratch */

		sync_input_rows (GMT, row, blend, n_blend, S.header.xy_off);	/* Wind each input file to current record and read each of the overlapping rows */

		for (col = 0; col < S.header.nx; col++) {	/* For each output node on the current row */

			w = 0.0;
			for (k = m = 0; k < n_blend; k++) {	/* Loop over every input grid */
				if (blend[k].outside) continue;					/* This grid is currently outside the s/n range */
				if (wrap_x) {	/* Special testing for periodic x coordinates */
					pcol = col + nx_360;
					while (pcol > blend[k].out_i1) pcol -= nx_360;
					if (pcol < blend[k].out_i0) continue;	/* This grid is currently outside the w/e range */
				}
				else {	/* Not periodic */
					if (col < blend[k].out_i0 || col > blend[k].out_i1) continue;	/* This grid is currently outside the xmin/xmax range */
					pcol = col;
				}
				kk = pcol - blend[k].out_i0;					/* kk is the local column variable for this grid */
				if (GMT_is_fnan (blend[k].z[kk])) continue;			/* NaNs do not contribute */
				if (pcol <= blend[k].in_i0)					/* Left cosine-taper weight */
					wt_x = 0.5 * (1.0 - cos ((pcol - blend[k].out_i0 + S.header.xy_off) * blend[k].wxl));
				else if (pcol >= blend[k].in_i1)					/* Right cosine-taper weight */
					wt_x = 0.5 * (1.0 - cos ((blend[k].out_i1 - pcol + S.header.xy_off) * blend[k].wxr));
				else								/* Inside inner region, weight = 1 */
					wt_x = 1.0;
				wt = wt_x * blend[k].wt_y;					/* Actual weight is 2-D cosine taper */
				if (blend[k].invert) wt = blend[k].weight - wt;			/* Invert the sense of the tapering */
				z[col] += (float)(wt * blend[k].z[kk]);				/* Add up weighted z*w sum */
				w += wt;							/* Add up the weight sum */
				m++;								/* Add up the number of contributing grids */
			}

			if (m) {	/* OK, at least one grid contributed to an output value */
				if (!Ctrl->W.active) {		/* Want output z blend */
					z[col] = (float)((w == 0.0) ? 0.0 : z[col] / w);	/* Get weighted average z */
					if (Ctrl->Z.active) z[col] *= (float)Ctrl->Z.scale;		/* Apply the global scale here */
				}
				else		/* Get the weight only */
					z[col] = (float)w;				/* Only interested in the weights */
				n_fill++;						/* One more cell filled */
				if (z[col] < S.header.z_min) S.header.z_min = z[col];	/* Update the extrema for output grid */
				if (z[col] > S.header.z_max) S.header.z_max = z[col];
			}
			else			/* No grids covered this node, defaults to the no_data value */
				z[col] = no_data_f;
		}
		if (Grid) {	/* Must copy entire row to grid */
			ij = GMT_IJP (Grid->header, row, 0);
			GMT_memcpy (&(Grid->data[ij]), z, S.header.nx, float);
		}
		else
			GMT_err_fail (GMT, GMT_write_grd_row (GMT, &S, 0, z), Ctrl->G.file);

		if (row%10 == 0)  GMT_report (GMT, GMT_MSG_NORMAL, "Processed row %7ld of %d\r", row, S.header.ny);

	}
	GMT_report (GMT, GMT_MSG_NORMAL, "Processed row %7ld\n", row);

	if (Grid) {	/* Must write entire grid */
		if ((error = GMT_Begin_IO (API, 0, GMT_OUT, GMT_BY_SET))) Return (error);		/* Enables data output and sets access mode */
		GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&Ctrl->G.file, (void *)Grid);
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid);
	}
	else {	/* Finish the line-by-line writing */
		GMT_close_grd (GMT, &S);	/* Close the output gridfile */
		if (!Ctrl->Q.active) GMT_err_fail (GMT, GMT_update_grd_info (GMT, Ctrl->G.file, &S.header), Ctrl->G.file);
	}
	GMT_free (GMT, z);

	for (k = 0; k < n_blend; k++) if (blend[k].open) {
		GMT_free (GMT, blend[k].z);
		GMT_close_grd (GMT, &blend[k].G);	/* Close all input grd files still open */
	}

	if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) {
		char empty[GMT_TEXT_LEN];
		GMT_report (GMT, GMT_MSG_NORMAL, "Blended grid size of %s is %d x %d\n", Ctrl->G.file, S.header.nx, S.header.ny);
		if (n_fill == n_tot)
			GMT_report (GMT, GMT_MSG_NORMAL, "All nodes assigned values\n");
		else {
			if (GMT_is_fnan (no_data_f))
				strcpy (empty, "NaN");
			else
				sprintf (empty, "%g", no_data_f);
			GMT_report (GMT, GMT_MSG_NORMAL, "%ld nodes assigned values, %ld set to %s\n", (GMT_LONG)n_fill, (GMT_LONG)(n_tot - n_fill), empty);
		}
	}

	GMT_free (GMT, blend);

	Return (GMT_OK);
}
