/*--------------------------------------------------------------------
 *	$Id: segy2grd_func.c,v 1.3 2011-03-25 22:17:42 guru Exp $
 *
 *	Copyright (c) 1991-2011 by T. Henstock
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
 * segy2grd was modified from xyz2grd by T. J. Henstock, June 2002
 * to read a segy file and generate a
 * corresponding grd-file.
 *
 * Author:	Tim Henstock (then@noc.soton.ac.uk)
 * Date:	30-JUN-2002
 * Version:	3.4.1
 */
 
#include "gmt_segy.h"
#include "segy_io.h"

#define COUNT	1
#define AVERAGE	2

#define X_ID	0
#define Y_ID	1

#define PLOT_CDP	1
#define PLOT_OFFSET	2

struct SEGY2GRD_CTRL {
	struct In {	/* -In */
		GMT_LONG active;
		char *file;
	} In;
	struct A {	/* -A */
		GMT_LONG active;
		GMT_LONG mode;
	} A;
	struct C {	/* -C<cpt> */
		GMT_LONG active;
		double value;
	} C;
	struct D {	/* -D */
		GMT_LONG active;
		char *text;
	} D;
	struct G {	/* -G */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct L {	/* -L */
		GMT_LONG active;
		GMT_LONG value;
	} L;
	struct M {	/* -M */
		GMT_LONG active;
		GMT_LONG value;
	} M;
	struct N {	/* -N */
		GMT_LONG active;
		double d_value;
		float f_value;
	} N;
	struct Q {	/* -Qx|y */
		GMT_LONG active[2];
		double value[2];
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
		GMT_LONG mode;
		GMT_LONG value;
	} S;
};

void *New_segy2grd_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SEGY2GRD_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct SEGY2GRD_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->A.mode = AVERAGE;
	C->M.value = 10000;
	C->N.f_value = GMT->session.f_NaN;
	C->N.d_value = GMT->session.d_NaN;
	C->Q.value[X_ID] = 1.0;
	return ((void *)C);
}

void Free_segy2grd_Ctrl (struct GMT_CTRL *GMT, struct SEGY2GRD_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C && C->In.file) free ((void *)C->In.file);
	if (C && C->D.text) free ((void *)C->D.text);
	if (C && C->G.file) free ((void *)C->G.file);
	GMT_free (GMT, C);
}

GMT_LONG GMT_segy2grd_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "segy2grd %s - Converting segy data to a GMT grid file\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: segy2grd <segyfile> -G<grdfile> %s %s\n", GMT_Id_OPT, GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[-A[n|z]] [%s] [-F] [-L<nsamp>] [-M<ntraces>] [-N<nodata>]\n", GMT_GRDEDIT);
	GMT_message (GMT, "\t[-Q<mode><value>] [-S<header>] [%s]\n\n", GMT_V_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\tsegyfile(s) is an IEEE floating point SEGY file. Traces are all assumed to start at 0 time/depth\n");
	GMT_message (GMT, "\t-G to name the output grid file.\n");
	GMT_message (GMT, "\t-I specifies grid size(s).\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A (or -Az): Add multiple entries at the same node.\n");
	GMT_message (GMT, "\t   Append n (-An): Count number of multiple entries per node instead.\n");
	GMT_message (GMT, "\t   [Default (no -A option) will compute mean values]\n");
	GMT_message (GMT, "\t-D to enter header information.  Specify '=' to get default value\n");
	GMT_message (GMT, "\t-F will force pixel registration [Default is grid registration]\n");
	GMT_message (GMT, "\t-L<nsamp> to override number of samples\n");
	GMT_message (GMT, "\t-M<ntraces> to fix number of traces. Default reads all traces.\n\t\t-M0 will read number in binary header, -Mn will attempt to read only n traces.\n");
	GMT_message (GMT, "\t-N set value for nodes without corresponding input sample [Default is NaN]\n");
	GMT_message (GMT, "\t-Q<mode><value> can be used to change two different settings:\n");
	GMT_message (GMT, "\t   -Qx<scl> applies scalar x-scale to coordinates in trace header to match the coordinates specified in -R\n");
	GMT_message (GMT, "\t   -Qy<s_int> specifies sample interval as <s_int> if incorrect in the SEGY file\n");
	GMT_message (GMT, "\t-S<header> to set variable spacing\n");
	GMT_message (GMT, "\t   <header> is c for cdp, o for offset, b<number> for 4-byte float starting at byte number\n");
	GMT_message (GMT, "\t\tIf -S not set, assumes even spacing of samples at dx, dy supplied with -I\n");
	GMT_explain_options (GMT, "V.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_segy2grd_parse (struct GMTAPI_CTRL *C, struct SEGY2GRD_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to segy2grd and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = TRUE;
				if (opt->arg[0] == 'n')
					Ctrl->A.mode = COUNT;
				else if (opt->arg[0] == '\0' || opt->arg[0] == 'z')
					Ctrl->A.mode = AVERAGE;
				else {
					GMT_message (GMT, "GMT SYNTAX ERROR -A option:  Select -An or -A[z]\n");
					n_errors++;
				}
				break;
			case 'D':
				Ctrl->D.active = TRUE;
				Ctrl->D.text = strdup (opt->arg);
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'N':
				if (!opt->arg[0]) {
					GMT_message (GMT, "GMT SYNTAX ERROR -N option:  Must specify value or NaN\n");
					n_errors++;
				}
				else {
					Ctrl->N.d_value = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.d_NaN : atof (opt->arg);
					Ctrl->N.f_value = (float)Ctrl->N.d_value;
					Ctrl->N.active = TRUE;
				}
				break;
			case 'Q':
				switch (opt->arg[0]) {
					case 'x': /* over-rides of header info */
						Ctrl->Q.active[X_ID] = TRUE;
						Ctrl->Q.value[X_ID] = atof (opt->arg);
						break;
					case 'y': /* over-rides of header info */
						Ctrl->Q.active[Y_ID] = TRUE;
						Ctrl->Q.value[Y_ID] = atof (opt->arg);
						break;
				}
				break;
			case 'L':
				Ctrl->L.active = TRUE;
				Ctrl->L.value = atoi (opt->arg);
				break;
			case 'M':
				Ctrl->M.active = TRUE;
				Ctrl->M.value = atoi (opt->arg);
				break;
			/* variable spacing */
			case 'S':
				if (Ctrl->S.active) {
					GMT_message (GMT, "GMT SYNTAX ERROR -S option:  Can only be set once\n");
					n_errors++;
				}
				Ctrl->S.active = TRUE;
				switch (opt->arg[0]) {
					case 'o':
						Ctrl->S.mode = PLOT_OFFSET;
						break;
					case 'c':
						Ctrl->S.mode = PLOT_CDP;
						break;
					case 'b':
						Ctrl->S.value = atoi (&opt->arg[1]);
						break;
				}
				break;
			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.active, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "GMT SYNTAX ERROR.  Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "GMT SYNTAX ERROR -I option.  Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.active || !Ctrl->G.file, "GMT SYNTAX ERROR -G:  Must specify output file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.active || !Ctrl->G.file, "GMT SYNTAX ERROR -G:  Must specify output file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_segy2grd_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_segy2grd (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, read_cont = FALSE, swap_bytes = !WORDS_BIGENDIAN;

	GMT_LONG ij, ii, jj, n_read = 0, n_filled = 0, n_used = 0, *flag = NULL;
	GMT_LONG n_empty = 0, n_stuffed = 0, n_bad = 0, n_confused = 0, check, n_samp=0, ix, isamp, ij0;

	double idy, x0, yval;

	char line[BUFSIZ], *head = NULL;

	FILE *fpi = NULL;

	struct GMT_GRID *Grid = NULL;

/* SEGY parameters */
	char reelhead[3200];
	float *data = NULL;
	SEGYHEAD *header = NULL;
	int head2;
	SEGYREEL binhead;

	struct SEGY2GRD_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_segy2grd_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_segy2grd_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_segy2grd", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRr", GMT_OPT("F"), options))) Return (error);
	Ctrl = (struct SEGY2GRD_CTRL *)New_segy2grd_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_segy2grd_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the segy2grd main code ----------------------------*/

	read_cont = (Ctrl->S.mode != PLOT_CDP && Ctrl->S.mode != PLOT_OFFSET && !Ctrl->S.value);

	Grid = GMT_create_grid (GMT);
	GMT_grd_init (GMT, Grid->header, options, FALSE);

	GMT_err_fail (GMT, GMT_init_newgrid (GMT, Grid, GMT->common.R.wesn, Ctrl->I.inc, GMT->common.r.active), Ctrl->G.file);

	/* Decode grd information given, if any */

	if (Ctrl->D.active) GMT_decode_grd_h_info (GMT, Ctrl->D.text, Grid->header);

	GMT_report (GMT, GMT_MSG_NORMAL, "nx = %d  ny = %d\n", Grid->header->nx, Grid->header->ny);

	Grid->data = GMT_memory (GMT, NULL, Grid->header->size, float);
	flag = GMT_memory (GMT, NULL, Grid->header->size, GMT_LONG);

	GMT_grd_pad_off (GMT, Grid);	/* Undo pad since algorithm does not expect on */

	idy = 1.0 / Grid->header->inc[GMT_Y];
	ij = -1;	/* Will be incremented to 0 or set first time around */

	/* read in reel headers from segy file */
	if (Ctrl->In.active) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Will read segy file %s\n", Ctrl->In.file);
		if ((fpi = fopen (Ctrl->In.file, "rb")) == NULL) {
			GMT_message (GMT, "Cannot find segy file %s\n", Ctrl->In.file);
			Return (EXIT_FAILURE);
		}
	}
	else {
		GMT_report (GMT, GMT_MSG_NORMAL, "Will read segy file from standard input\n");
		if (fpi == NULL) fpi = stdin;
	}
	if ((check = get_segy_reelhd (fpi, reelhead)) != TRUE) exit (1);
	if ((check = get_segy_binhd (fpi, &binhead)) != TRUE) exit (1);

	if (swap_bytes) {
		/* this is a little-endian system, and we need to byte-swap ints in the reel header - we only
		   use a few of these*/
		GMT_report (GMT, GMT_MSG_NORMAL, "swapping bytes for ints in the headers\n");
		binhead.num_traces = GMT_swab2 (binhead.num_traces);
		binhead.nsamp = GMT_swab2 (binhead.nsamp);
		binhead.dsfc = GMT_swab2 (binhead.dsfc);
		binhead.sr = GMT_swab2 (binhead.sr);
	}


	/* set parameters from the reel headers */
	if (!Ctrl->M.value) Ctrl->M.value = binhead.num_traces;

	GMT_report (GMT, GMT_MSG_NORMAL, "Number of traces in header is %ld\n", Ctrl->M.value);

	if (!Ctrl->L.value) {	/* number of samples not overridden*/
		Ctrl->L.value = binhead.nsamp;
		GMT_message (GMT, "Number of samples per trace is %ld\n", Ctrl->L.value);
	}
	else if ((Ctrl->L.value != binhead.nsamp) && (binhead.nsamp))
		GMT_message (GMT, "warning nsampr input %ld, nsampr in header %d\n", Ctrl->L.value,  binhead.nsamp);

	if (!Ctrl->L.value) { /* no number of samples still - a problem! */
		GMT_message (GMT, "Error, number of samples per trace unknown\n");
		Return (EXIT_FAILURE);
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "Number of samples for reel is %ld\n", Ctrl->L.value);

	if (binhead.dsfc != 5) GMT_message (GMT, "segy2grd: WARNING data not in IEEE format\n");

	if (!Ctrl->Q.value[Y_ID]) {
		Ctrl->Q.value[Y_ID] = (double) binhead.sr; /* sample interval of data (microseconds) */
		Ctrl->Q.value[Y_ID] /= 1000000.0;
		GMT_message (GMT,"Sample interval is %f s\n", Ctrl->Q.value[Y_ID]);
	}
	else if ((Ctrl->Q.value[Y_ID] != binhead.sr) && (binhead.sr)) /* value in header overridden by input */
		GMT_message (GMT, "Warning s_int input %f, s_int in header %f\n", Ctrl->Q.value[Y_ID], (float)binhead.sr);

	if (!Ctrl->Q.value[Y_ID]) { /* still no sample interval at this point is a problem! */
		GMT_message (GMT, "Error, no sample interval in reel header\n");
		exit(EXIT_FAILURE);
	}
	if (read_cont && (Ctrl->Q.value[Y_ID] != Grid->header->inc[GMT_Y])) {
		GMT_message (GMT, "Warning, grid spacing != sample interval, setting sample interval to grid spacing\n");
		Ctrl->Q.value[Y_ID] = Grid->header->inc[GMT_Y];
	}

	if (Grid->header->inc[GMT_Y] < Ctrl->Q.value[Y_ID]) GMT_message (GMT, "Warning, grid spacing < sample interval, expect gaps in output....\n");

	/* starts reading actual data here....... */

	if (read_cont) {	/* old-style segy2grd */
		ix = 0;
		for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] = Ctrl->N.f_value;
		if (Grid->header->nx < Ctrl->M.value) {
			GMT_message (GMT,"Warning, number of traces in header > size of grid. Reading may be truncated\n");
			Ctrl->M.value = Grid->header->nx;
		}
		while ((ix < Ctrl->M.value) && (header = get_segy_header (fpi))) {
			if (swap_bytes) {
/* need to permanently byte-swap number of samples in the trace header */
				header->num_samps = GMT_swab4 (header->num_samps);
			 	header->sampleLength = GMT_swab2 (header->sampleLength);
			}

			data = (float *) get_segy_data (fpi, header); /* read a trace */
			/* get number of samples in _this_ trace or set to number in reel header */
			if (!(n_samp = samp_rd (header))) n_samp = Ctrl->L.value;

			ij0 = (GMT_LONG)(GMT->common.R.wesn[YLO] * idy);
			if (n_samp - ij0 > Grid->header->ny) n_samp = Grid->header->ny + ij0;

			if (swap_bytes) { /* need to swap the order of the bytes in the data even though assuming IEEE format */
				int *intdata = (int *) data;
				for (isamp = 0; isamp < n_samp; isamp++) intdata[isamp] = GMT_swab4 (intdata[isamp]);
			}

			for (ij = ij0; ij < n_samp ; ij++) {  /* n*idy is index of first sample to be included in the grid */
				Grid->data[ix + Grid->header->nx*(Grid->header->ny+ij0-ij-1)] = data[ij];
			}

			free (data);
			free (header);
			ix++;
		}
	}
	else {	/* Get trace data and position by headers */
		ix = 0;
		while ((ix < Ctrl->M.value) && (header = get_segy_header (fpi))) {   /* read traces one by one */
			if (Ctrl->S.mode == PLOT_OFFSET) { /* plot traces by offset, cdp, or input order */
				int32_t offset = ((swap_bytes)? GMT_swab4 (header->sourceToRecDist) : header->sourceToRecDist);
				x0 = (double) offset;
			}
			else if (Ctrl->S.mode == PLOT_CDP) {
				int32_t cdpval = ((swap_bytes)? GMT_swab4 (header->cdpEns) : header->cdpEns);
				x0 = (double) cdpval;
			}
			else if (Ctrl->S.value) { /* ugly code - want to get value starting at Ctrl->S.value of header into a double... */
				head = (char *) header;
				memcpy(&head2, &head[Ctrl->S.value], 4); /* edited to fix bug where 8bytes were copied from head.
                                                Caused by casting to a long directly from char array*/ 
				x0 = (double) ((swap_bytes)? GMT_swab4 (head2): head2);
			}
			else
				x0 = (1.0 + (double) ix);

			x0 *= Ctrl->Q.value[X_ID];

			if (swap_bytes) {
				/* need to permanently byte-swap some things in the trace header 
				   do this after getting the location of where traces are plotted in case the general Ctrl->S.value case
				   overlaps a defined header in a strange way */
				header->sourceToRecDist = GMT_swab4 (header->sourceToRecDist);
				header->sampleLength = GMT_swab2 (header->sampleLength);
				header->num_samps = GMT_swab4 (header->num_samps);
			}

			data = (float *) get_segy_data (fpi, header); /* read a trace */
			/* get number of samples in _this_ trace (e.g. OMEGA has strange ideas about SEGY standard)
			   or set to number in reel header */
			if (!(n_samp = samp_rd (header))) n_samp = Ctrl->L.value;

			if (swap_bytes) { /* need to swap the order of the bytes in the data even though assuming IEEE format */
				int *intdata = (int *) data;
				for (isamp = 0; isamp < n_samp; isamp++) intdata[isamp] = GMT_swab4 (intdata[isamp]);
			}

			if (!(x0 < GMT->common.R.wesn[XLO] || x0 > GMT->common.R.wesn[XHI])) {	/* inside x-range */
				/* find horizontal grid pos of this trace */
				ii = GMT_grd_x_to_col (x0, Grid->header);
				if (ii == Grid->header->nx) ii--, n_confused++;
				for (isamp = 0; isamp< n_samp; isamp++) {
					yval = isamp*Ctrl->Q.value[Y_ID];
					if (!(yval < GMT->common.R.wesn[YLO] || yval > GMT->common.R.wesn[YHI])) {	/* inside y-range */
						jj = GMT_grd_y_to_row (yval, Grid->header);
						if (jj == Grid->header->ny) jj--, n_confused++;
						ij = GMT_IJ0 (Grid->header, jj, ii);
						Grid->data[ij] += data[isamp];	/* Add up incase we must average */
						flag[ij]++;
						n_used++;
					}
				}
			}
			free (data);
			free (header);
			ix++;
		}

		for (ij = 0; ij < Grid->header->nm; ij++) {	/* Check if all nodes got one value only */
			if (flag[ij] == 1) {
				if (Ctrl->A.mode == COUNT) Grid->data[ij] = 1.0;
				n_filled++;
			}
			else if (flag[ij] == 0) {
				n_empty++;
				Grid->data[ij] = Ctrl->N.f_value;
			}
			else {	/* More than 1 value went to this node */
				if (Ctrl->A.mode == COUNT)
					Grid->data[ij] = (float)flag[ij];
				else if (Ctrl->A.mode == AVERAGE)
					Grid->data[ij] /= (float)flag[ij];
				n_filled++;
				n_stuffed++;
			}
		}

		if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) {
			sprintf (line, "%s\n", GMT->current.setting.format_float_out);
			GMT_message (GMT, " n_read: %ld  n_used: %ld  n_filled: %ld  n_empty: %ld set to ",
				n_read, n_used, n_filled, n_empty);
			(GMT_is_dnan (Ctrl->N.d_value)) ? GMT_message (GMT, "NaN\n") : GMT_message (GMT, line, Ctrl->N.d_value);
			if (n_bad) GMT_message (GMT, "%ld records unreadable\n", n_bad);
			if (n_stuffed) GMT_message (GMT, "Warning - %ld nodes had multiple entries that were averaged\n", n_stuffed);
			if (n_confused) GMT_message (GMT, "Warning - %ld values gave bad indices: Pixel vs gridline confusion?\n", n_confused);
		}
	}

	GMT_grd_pad_on (GMT, Grid, GMT->current.io.pad);	/* Restore padding */
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if (GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&Ctrl->G.file, (void *)Grid)) Return (GMT_DATA_WRITE_ERROR);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);				/* Disables further data output */

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Grid);

	GMT_free (GMT, flag);

	Return (GMT_OK);
}
