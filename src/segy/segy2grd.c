/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by T. Henstock
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
 * segy2grd was modified from xyz2grd by T. J. Henstock, June 2002
 * to read a segy file and generate a
 * corresponding grd-file.
 *
 * Author:	Tim Henstock (then@noc.soton.ac.uk)
 * Date:	30-JUN-2002
 * Version:	3.4.1, ported to GMT5 by P. Wessel
 */

#include "gmt_dev.h"
#include "segy_io.h"

#define THIS_MODULE_CLASSIC_NAME	"segy2grd"
#define THIS_MODULE_MODERN_NAME	"segy2grd"
#define THIS_MODULE_LIB		"segy"
#define THIS_MODULE_PURPOSE	"Converting SEGY data to a grid"
#define THIS_MODULE_KEYS	"GG}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-VRr" GMT_OPT("F")

#define COUNT	1
#define AVERAGE	2

#define X_ID	0
#define Y_ID	1

#define PLOT_CDP	1
#define PLOT_OFFSET	2

struct SEGY2GRD_CTRL {
	struct In {	/* -In */
		bool active;
		char *file;
	} In;
	struct A {	/* -A */
		bool active;
		int mode;
	} A;
	struct C {	/* -C<cpt> */
		bool active;
		double value;
	} C;
	struct D {	/* -D */
		bool active;
		char *text;
	} D;
	struct G {	/* -G */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct L {	/* -L */
		bool active;
		int value;
	} L;
	struct M {	/* -M */
		bool active;
		unsigned int value;
	} M;
	struct N {	/* -N */
		bool active;
		double d_value;
		float f_value;
	} N;
	struct Q {	/* -Qx|y */
		bool active[2];
		double value[2];
	} Q;
	struct S {	/* -S */
		bool active;
		unsigned int mode;
		int value;
	} S;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SEGY2GRD_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct SEGY2GRD_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.mode = AVERAGE;
	C->M.value = 10000;
	C->N.f_value = GMT->session.f_NaN;
	C->N.d_value = GMT->session.d_NaN;
	C->Q.value[X_ID] = 1.0;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct SEGY2GRD_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->D.text);
	gmt_M_str_free (C->G.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <segyfile> -G<grdfile> %s\n", name, GMT_Id_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-A[n|z]]\n\t[%s] [-L<nsamp>]\n", GMT_Rgeo_OPT, GMT_GRDEDIT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-M<ntraces>] [-N<nodata>] [-Q<mode><value>] [-S<header>] [%s] [%s] [%s]\n\n", GMT_V_OPT, GMT_r_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tsegyfile(s) is an IEEE floating point SEGY file. Traces are all assumed to start at 0 time/depth.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set name the output grid file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Specify grid size(s).\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A (or -Az): Add multiple entries at the same node.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append n (-An): Count number of multiple entries per node instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default (no -A option) will compute mean values].\n");
	gmt_grd_info_syntax (API->GMT, 'D');
	GMT_Message (API, GMT_TIME_NONE, "\t-L Let <nsamp> override number of samples.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Fix number of traces. Default reads all traces.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -M0 will read number in binary header, -Mn will attempt to read only n traces.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set value for nodes without corresponding input sample [Default is NaN].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Append <mode><value> to change either of two different settings:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Qx<scl> applies scalar x-scale to coordinates in trace header to match the coordinates specified in -R.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Qy<s_int> specifies sample interval as <s_int> if incorrect in the SEGY file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Append <header> to set variable spacing\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <header> is c for cdp, o for offset, b<number> for 4-byte float starting at byte number.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -S not set, assumes even spacing of samples at dx, dy supplied with -I.\n");
	GMT_Option (API, "V,r,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SEGY2GRD_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to segy2grd and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0)
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				if (opt->arg[0] == 'n')
					Ctrl->A.mode = COUNT;
				else if (opt->arg[0] == '\0' || opt->arg[0] == 'z')
					Ctrl->A.mode = AVERAGE;
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -A: Select -An or -A[z]\n");
					n_errors++;
				}
				break;
			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.text = strdup (opt->arg);
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
			case 'N':
				if (!opt->arg[0]) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -N: Must specify value or NaN\n");
					n_errors++;
				}
				else {
					Ctrl->N.d_value = (opt->arg[0] == 'N' || opt->arg[0] == 'n') ? GMT->session.d_NaN : atof (opt->arg);
					Ctrl->N.f_value = (float)Ctrl->N.d_value;
					Ctrl->N.active = true;
				}
				break;
			case 'Q':
				switch (opt->arg[0]) {
					case 'x': /* over-rides of header info */
						Ctrl->Q.active[X_ID] = true;
						Ctrl->Q.value[X_ID] = atof (&opt->arg[1]);
						break;
					case 'y': /* over-rides of header info */
						Ctrl->Q.active[Y_ID] = true;
						Ctrl->Q.value[Y_ID] = atof (&opt->arg[1]);
						break;
				}
				break;
			case 'L':
				Ctrl->L.active = true;
				Ctrl->L.value = atoi (opt->arg);
				break;
			case 'M':
				Ctrl->M.active = true;
				Ctrl->M.value = atoi (opt->arg);
				break;
			/* variable spacing */
			case 'S':
				if (Ctrl->S.active) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -S: Can only be set once\n");
					n_errors++;
				}
				Ctrl->S.active = true;
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
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Option -I: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.active || !Ctrl->G.file, "Option -G: Must specify output file\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.active || !Ctrl->G.file, "Option -G: Must specify output file\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_segy2grd (void *V_API, int mode, void *args) {
	bool  read_cont = false, swap_bytes = !GMT_BIGENDIAN;
	int error = 0;
	unsigned int ii, jj, n_read = 0, n_filled = 0, n_used = 0, *flag = NULL;
	unsigned int n_empty = 0, n_stuffed = 0, n_confused = 0, check, ix;

	uint64_t ij, ij0, n_samp = 0, isamp;

	double idy, x0, yval;

	char line[GMT_BUFSIZ] = {""};

	FILE *fpi = NULL;

	struct GMT_GRID *Grid = NULL;

/* SEGY parameters */
	char reelhead[3200] = {""};
	float *data = NULL;
	SEGYHEAD *header = NULL;
	SEGYREEL binhead;

	struct SEGY2GRD_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the segy2grd main code ----------------------------*/

	read_cont = (Ctrl->S.mode != PLOT_CDP && Ctrl->S.mode != PLOT_OFFSET && !Ctrl->S.value);

	if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, NULL, NULL, \
		GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);

	/* Decode grd information given, if any */

	if (Ctrl->D.active && gmt_decode_grd_h_info (GMT, Ctrl->D.text, Grid->header))
		Return (GMT_PARSE_ERROR);

	GMT_Report (API, GMT_MSG_INFORMATION, "n_columns = %d  n_rows = %d\n", Grid->header->n_columns, Grid->header->n_rows);

	flag = gmt_M_memory (GMT, NULL, Grid->header->size, unsigned int);

	gmt_grd_pad_off (GMT, Grid);	/* Undo pad since algorithm does not expect on */

	idy = 1.0 / Grid->header->inc[GMT_Y];

	/* read in reel headers from segy file */
	if (Ctrl->In.active) {
		GMT_Report (API, GMT_MSG_INFORMATION, "Will read segy file %s\n", Ctrl->In.file);
		if ((fpi = gmt_fopen (GMT, Ctrl->In.file, "rb")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot find segy file %s\n", Ctrl->In.file);
			gmt_M_free (GMT, flag);
			Return (GMT_ERROR_ON_FOPEN);
		}
	}
	else {
		GMT_Report (API, GMT_MSG_INFORMATION, "Will read segy file from standard input\n");
		if (fpi == NULL) fpi = stdin;
	}
	if ((check = segy_get_reelhd (fpi, reelhead)) != true) {
		if (fpi != stdin) fclose (fpi);
		gmt_M_free (GMT, flag);
		GMT_exit (GMT, GMT_RUNTIME_ERROR);
		return GMT_RUNTIME_ERROR;
	}
	if ((check = segy_get_binhd (fpi, &binhead)) != true) {
		if (fpi != stdin) fclose (fpi);
		gmt_M_free (GMT, flag);
		GMT_exit (GMT, GMT_RUNTIME_ERROR);
		return GMT_RUNTIME_ERROR;
	}

	if (swap_bytes) {
		/* this is a little-endian system, and we need to byte-swap ints in the reel header - we only
		   use a few of these*/
		GMT_Report (API, GMT_MSG_INFORMATION, "Swapping bytes for ints in the headers\n");
		binhead.num_traces = bswap16 (binhead.num_traces);
		binhead.nsamp = bswap16 (binhead.nsamp);
		binhead.dsfc = bswap16 (binhead.dsfc);
		binhead.sr = bswap16 (binhead.sr);
	}


	/* set parameters from the reel headers */
	if (!Ctrl->M.value) Ctrl->M.value = binhead.num_traces;

	GMT_Report (API, GMT_MSG_INFORMATION, "Number of traces in header is %d\n", Ctrl->M.value);

	if (!Ctrl->L.value) {	/* number of samples not overridden*/
		Ctrl->L.value = binhead.nsamp;
		GMT_Report (API, GMT_MSG_INFORMATION, "Number of samples per trace is %d\n", Ctrl->L.value);
	}
	else if ((Ctrl->L.value != binhead.nsamp) && (binhead.nsamp))
		GMT_Report (API, GMT_MSG_INFORMATION, "nsampr input %d, nsampr in header %d\n", Ctrl->L.value,  binhead.nsamp);

	if (!Ctrl->L.value) { /* no number of samples still - a problem! */
		GMT_Report (API, GMT_MSG_ERROR, "Number of samples per trace unknown\n");
		if (fpi != stdin) fclose (fpi);
		gmt_M_free (GMT, flag);
		Return (GMT_RUNTIME_ERROR);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Number of samples for reel is %d\n", Ctrl->L.value);

	if (binhead.dsfc != 5) GMT_Report (API, GMT_MSG_WARNING, "Data not in IEEE format\n");

	if (!Ctrl->Q.value[Y_ID]) {
		Ctrl->Q.value[Y_ID] = (double) binhead.sr; /* sample interval of data (microseconds) */
		Ctrl->Q.value[Y_ID] /= 1000000.0;
		GMT_Report (API, GMT_MSG_INFORMATION,"Sample interval is %f s\n", Ctrl->Q.value[Y_ID]);
	}
	else if ((Ctrl->Q.value[Y_ID] != binhead.sr) && (binhead.sr)) /* value in header overridden by input */
		GMT_Report (API, GMT_MSG_INFORMATION, "s_int input %f, s_int in header %f\n", Ctrl->Q.value[Y_ID], (float)binhead.sr);

	if (!Ctrl->Q.value[Y_ID]) { /* still no sample interval at this point is a problem! */
		GMT_Report (API, GMT_MSG_ERROR, "No sample interval in reel header\n");
		if (fpi != stdin) fclose (fpi);
		gmt_M_free (GMT, flag);
		GMT_exit (GMT, GMT_RUNTIME_ERROR); return GMT_RUNTIME_ERROR;
	}
	if (read_cont && (Ctrl->Q.value[Y_ID] != Grid->header->inc[GMT_Y])) {
		GMT_Report (API, GMT_MSG_INFORMATION, "Grid spacing != sample interval, setting sample interval to grid spacing\n");
		Ctrl->Q.value[Y_ID] = Grid->header->inc[GMT_Y];
	}

	if (Grid->header->inc[GMT_Y] < Ctrl->Q.value[Y_ID])
		GMT_Report (API, GMT_MSG_WARNING, "Grid spacing < sample interval, expect gaps in output....\n");

	/* starts reading actual data here....... */

	if (read_cont) {	/* old-style segy2grd */
		ix = 0;
		for (ij = 0; ij < Grid->header->size; ij++) Grid->data[ij] = Ctrl->N.f_value;
		if (Grid->header->n_columns < Ctrl->M.value) {
			GMT_Report (API, GMT_MSG_WARNING, "Number of traces in header > size of grid. Reading may be truncated\n");
			Ctrl->M.value = Grid->header->n_columns;
		}
		while ((ix < Ctrl->M.value) && (header = segy_get_header (fpi)) != 0) {
			if (swap_bytes) {
/* need to permanently byte-swap number of samples in the trace header */
				header->num_samps = bswap32 (header->num_samps);
				header->sampleLength = bswap16 (header->sampleLength);
			}

			data = segy_get_data (fpi, header); /* read a trace */
			/* get number of samples in _this_ trace or set to number in reel header */
			if ((n_samp = segy_samp_rd (header)) != 0) n_samp = Ctrl->L.value;

			ij0 = lrint (GMT->common.R.wesn[YLO] * idy);
			if ((n_samp - ij0) > (uint64_t)Grid->header->n_rows) n_samp = Grid->header->n_rows + ij0;

			if (swap_bytes) {
				/* need to swap the order of the bytes in the data even though assuming IEEE format */
				uint32_t tmp;
				for (isamp = 0; isamp < n_samp; ++isamp) {
					memcpy (&tmp, &data[isamp], sizeof(uint32_t));
					tmp = bswap32 (tmp);
					memcpy (&data[isamp], &tmp, sizeof(uint32_t));
				}
			}

			for (ij = ij0; ij < n_samp ; ij++) {  /* n*idy is index of first sample to be included in the grid */
				Grid->data[ix + Grid->header->n_columns*(Grid->header->n_rows+ij0-ij-1)] = data[ij];
			}

			gmt_M_str_free (data);
			gmt_M_str_free (header);
			ix++;
		}
	}
	else {
		/* Get trace data and position by headers */
		ix = 0;
		while ((ix < Ctrl->M.value) && (header = segy_get_header (fpi)) != 0) {
			/* read traces one by one */
			if (Ctrl->S.mode == PLOT_OFFSET) {
				/* plot traces by offset, cdp, or input order */
				int32_t tmp = header->sourceToRecDist;
				if (swap_bytes) {
					uint32_t *p = (uint32_t *)&tmp;
					*p = bswap32 (*p);
				}
				x0 = (double) tmp;
			}
			else if (Ctrl->S.mode == PLOT_CDP) {
				int32_t tmp = header->cdpEns;
				if (swap_bytes) {
					uint32_t *p = (uint32_t *)&tmp;
					*p = bswap32 (*p);
				}
				x0 = (double) tmp;
			}
			else if (Ctrl->S.value) {
				/* get value starting at Ctrl->S.value of header into a double */
				uint32_t tmp;
				memcpy (&tmp, &header[Ctrl->S.value], sizeof (uint32_t));
				x0 = (double) bswap32 (tmp);
			}
			else
				x0 = (1.0 + (double) ix);

			x0 *= Ctrl->Q.value[X_ID];

			if (swap_bytes) {
				/* need to permanently byte-swap some things in the trace header
					 do this after getting the location of where traces are plotted
					 in case the general Ctrl->S.value case overlaps a defined header
					 in a strange way */
				uint32_t *p = (uint32_t *)&header->sourceToRecDist;
				*p = bswap32 (*p);
				header->sampleLength = bswap16 (header->sampleLength);
				header->num_samps = bswap32 (header->num_samps);
			}

			data = segy_get_data (fpi, header); /* read a trace */
			/* get number of samples in _this_ trace (e.g. OMEGA has strange ideas about SEGY standard)
			   or set to number in reel header */
			if ((n_samp = segy_samp_rd (header)) != 0) n_samp = Ctrl->L.value;

			if (swap_bytes) {
				/* need to swap the order of the bytes in the data even though assuming IEEE format */
				uint32_t tmp;
				for (isamp = 0; isamp < n_samp; ++isamp) {
					memcpy (&tmp, &data[isamp], sizeof(uint32_t));
					tmp = bswap32 (tmp);
					memcpy (&data[isamp], &tmp, sizeof(uint32_t));
				}
			}

			if (!(x0 < GMT->common.R.wesn[XLO] || x0 > GMT->common.R.wesn[XHI])) {	/* inside x-range */
				/* find horizontal grid pos of this trace */
				ii = (unsigned int)gmt_M_grd_x_to_col (GMT, x0, Grid->header);
				if (ii == Grid->header->n_columns) ii--, n_confused++;
				for (isamp = 0; isamp< n_samp; ++isamp) {
					yval = isamp*Ctrl->Q.value[Y_ID];
					if (!(yval < GMT->common.R.wesn[YLO] || yval > GMT->common.R.wesn[YHI])) {	/* inside y-range */
						jj = (unsigned int)gmt_M_grd_y_to_row (GMT, yval, Grid->header);
						if (jj == Grid->header->n_rows) jj--, n_confused++;
						ij = gmt_M_ij0 (Grid->header, jj, ii);
						Grid->data[ij] += data[isamp];	/* Add up in case we must average */
						flag[ij]++;
						n_used++;
					}
				}
			}
			gmt_M_str_free (data);
			gmt_M_str_free (header);
			ix++;
		}

		for (ij = 0; ij < Grid->header->nm; ij++) {	/* Check if all nodes got one value only */
			if (flag[ij] == 1) {
				if (Ctrl->A.mode == COUNT) Grid->data[ij] = 1.0f;
				n_filled++;
			}
			else if (flag[ij] == 0) {
				n_empty++;
				Grid->data[ij] = Ctrl->N.f_value;
			}
			else {	/* More than 1 value went to this node */
				if (Ctrl->A.mode == COUNT)
					Grid->data[ij] = (gmt_grdfloat)flag[ij];
				else if (Ctrl->A.mode == AVERAGE)
					Grid->data[ij] /= (gmt_grdfloat)flag[ij];
				n_filled++;
				n_stuffed++;
			}
		}

		if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
			if (gmt_M_is_dnan (Ctrl->N.d_value))
				strcpy (line, "NaN\n");
			else
				sprintf (line, GMT->current.setting.format_float_out, Ctrl->N.d_value);
			GMT_Report (API, GMT_MSG_INFORMATION, " n_read: %d  n_used: %d  n_filled: %d  n_empty: %d set to %s\n", n_read, n_used, n_filled, n_empty, line);
		}
		if (n_stuffed) GMT_Report (API, GMT_MSG_WARNING, "%d nodes had multiple entries that were averaged\n", n_stuffed);
		if (n_confused) GMT_Report (API, GMT_MSG_WARNING, "%d values gave bad indices: Pixel vs gridline confusion?\n", n_confused);
	}
	if (fpi != stdin) fclose (fpi);
	gmt_M_free (GMT, flag);

	gmt_grd_pad_on (GMT, Grid, GMT->current.io.pad);	/* Restore padding */
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, Grid)) Return (API->error);
	if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->G.file, Grid) != GMT_NOERROR) {
		Return (API->error);
	}

	Return (GMT_NOERROR);
}
