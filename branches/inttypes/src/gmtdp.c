/*
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * API functions to support the gmtdp application.
 *
 * Author:	Joaquim Luis
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: gmtdp applies the Douglas-Peucker algorithm to simplify a line
 * segment given a tolerance.
 */

/* 
* The algorithm is based on the paper:
* Douglas, D. H., and T. K. Peucker, Algorithms for the reduction
*   of the number of points required to represent a digitized line
*   of its caricature, Can. Cartogr., 10, 112-122, 1973.
* The implementation of this algorithm has been kindly provided by
* Dr. Gary J. Robinson, Environmental Systems Science Centre,
* University of Reading, Reading, UK (gazza@mail.nerc-essc.ac.uk)
* NOTE: For geographic data we do Flat Earth; there is no spherical
* solution here.
*/

#include "gmt.h"

/* Control structure for gmtdp */

struct GMTDP_CTRL {
	struct Out {	/* ->[<outfile>] */
		GMT_LONG active;
		char *file;
	} Out;
	struct T {	/* 	-T[-|=|+]<tolerance>[d|s|m|e|f|k|M|n] */
		GMT_LONG active;
		GMT_LONG mode;
		double tolerance;
		char unit;
	} T;
};

void *New_gmtdp_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTDP_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMTDP_CTRL);
	
	return (C);
}

void Free_gmtdp_Ctrl (struct GMT_CTRL *GMT, struct GMTDP_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free (C->Out.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_gmtdp_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;
	GMT_message (GMT, "gmtdp %s [API] - Line reduction using the Douglas-Peucker algorithm\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: gmtdp [<table>] -T<tolerance>[<unit>] [-G<outtable>] [%s]\n", GMT_V_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n",
		GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_dist_syntax (GMT, 'T', "Set tolerance as the maximum distance mismatch.");
	GMT_message (GMT, "\t   No units means we will do a Cartesian calculation instead.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "<");
	GMT_message (GMT, "\t-G Specify an output file [Default writes to stdout].\n");
	GMT_explain_options (GMT, "VC2D0fghio:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_gmtdp_parse (struct GMTAPI_CTRL *C, struct GMTDP_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtdp and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;
			case '>':	/* Write to named output file instead of stdout */
				Ctrl->Out.active = TRUE;
				if (n_files++ == 0 && opt->arg[0]) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */
			
			case 'T':	/* Set tolerance distance */
				Ctrl->T.active = TRUE;
				Ctrl->T.mode = GMT_get_distance (GMT, opt->arg, &(Ctrl->T.tolerance), &(Ctrl->T.unit));
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}
	
	n_errors += GMT_check_condition (GMT, Ctrl->T.mode == -1, "Syntax error -T: Unrecognized unit\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.mode == -2, "Syntax error -T: Unable to decode tolarance distance.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.mode == -3, "Syntax error -T: Tolarance is negative\n");
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 2, "Syntax error: Binary input data (-bi) must have at least 2 columns.\n");
	n_errors += GMT_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define GMT_sqr(x) ((x)*(x))

/* Stack-based Douglas Peucker line simplification routine */
/* returned value is the number of output points */

uint64_t Douglas_Peucker_geog (struct GMT_CTRL *GMT, double x_source[], double y_source[], uint64_t n_source, double band, GMT_LONG geo, uint64_t index[]) {
/* x/y_source	Input coordinates, n_source of them.  These are not changed */
/* band;	tolerance in Cartesian user units or degrees */
/* geo:		TRUE if data is lon/lat */
/* index[]	output co-ordinates indices */

	uint64_t n_stack, n_dest, start, end, i, sig;
	uint64_t *sig_start = NULL, *sig_end = NULL;	/* indices of start&end of working section */

	double dev_sqr, max_dev_sqr, band_sqr;
	double x12, y12, d12, x13, y13, d13, x23, y23, d23;

	/* check for simple cases */

	if (n_source < 3) {     /* one or two points */
		for (i = 0; i < n_source; i++) index[i] = i;
		return (n_source);
	}

	/* more complex case. initialise stack */

	sig_start = GMT_memory (GMT, NULL, n_source, uint64_t);
	sig_end   = GMT_memory (GMT, NULL, n_source, uint64_t);

	/* All calculations uses the original units, either Cartesian or FlatEarth */
	/* The tolerance (band) must be in the same units as the data */
	
	band_sqr = GMT_sqr (band);

	n_dest = sig_start[0] = 0;
	sig_end[0] = n_source - 1;

	n_stack = 1;

	/* while the stack is not empty  ... */

	while (n_stack > 0) {
		/* ... pop the top-most entries off the stacks */

		start = sig_start[n_stack-1];
		end = sig_end[n_stack-1];

		n_stack--;

		if ((end - start) > 1) { /* any intermediate points ? */
			/* ... yes, so find most deviant intermediate point to
			   either side of line joining start & end points */

			x12 = x_source[end] - x_source[start];
			if (geo && fabs (x12) > 180.0) x12 = 360.0 - fabs (x12);
			y12 = y_source[end] - y_source[start];
			if (geo) x12 *= cosd (0.5 * (y_source[end] + y_source[start]));
			d12 = GMT_sqr (x12) + GMT_sqr (y12);
			
			for (i = start + 1, sig = start, max_dev_sqr = -1.0; i < end; i++) {
				x13 = x_source[i] - x_source[start];
				if (geo && fabs (x13) > 180.0) x13 = 360.0 - fabs (x13);
				y13 = y_source[i] - y_source[start];

				x23 = x_source[i] - x_source[end];
				if (geo && fabs (x23) > 180.0) x23 = 360.0 - fabs (x23);
				y23 = y_source[i] - y_source[end];

				if (geo) {	/* Do the Flat Earth thingy */
					x13 *= cosd (0.5 * (y_source[i] + y_source[start]));
					x23 *= cosd (0.5 * (y_source[i] + y_source[end]));
				}

				d13 = GMT_sqr (x13) + GMT_sqr (y13);
				d23 = GMT_sqr (x23) + GMT_sqr (y23);

				if (d13 >= (d12 + d23))
					dev_sqr = d23;
				else if (d23 >= (d12 + d13))
					dev_sqr = d13;
				else
					dev_sqr =  GMT_sqr (x13 * y12 - y13 * x12) / d12;

				if (dev_sqr > max_dev_sqr) {
					sig = i;
					max_dev_sqr = dev_sqr;
				}
			}

			if (max_dev_sqr < band_sqr) {  /* is there a sig.  intermediate point ? */
				/* ... no, so transfer current start point */
				index[n_dest] = start;
				n_dest++;
			}
			else {	/* ... yes, so push two sub-sections on stack for further processing */

				n_stack++;
				sig_start[n_stack-1] = sig;
				sig_end[n_stack-1] = end;

				n_stack++;
				sig_start[n_stack-1] = start;
				sig_end[n_stack-1] = sig;
			}
		}
		else {	/* ... no intermediate points, so transfer current start point */
			index[n_dest] = start;
			n_dest++;
		}
	}

	/* transfer last point */

	index[n_dest] = n_source-1;
	n_dest++;

	GMT_free (GMT, sig_start);
	GMT_free (GMT, sig_end);

	return (n_dest);
}

/* Must free allocated memory before returning */
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_gmtdp_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_gmtdp (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG tbl, col, geo, error;
	uint64_t row, seg, np_out, *index = NULL;
	
	double tolerance;
	
	struct GMT_DATASET *D[2] = {NULL, NULL};
	struct GMT_LINE_SEGMENT *S[2] = {NULL, NULL};
	struct GMTDP_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_gmtdp_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_gmtdp_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_gmtdp", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-Vbf:", "ghio>" GMT_OPT("HMm"), options)) Return (API->error);
	Ctrl = New_gmtdp_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtdp_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the gmtdp main code ----------------------------*/

	if (Ctrl->T.mode > 1) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning: gmtdp only implemented using Flat-Earth calculations.\n");
		Ctrl->T.mode = 1;	/* Limited to Flat Earth calculations for now */
	}
	
	/* Now we are ready to take on some input values */
	/* Allocate memory and read in all the files; each file can have many lines */
	
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options) != GMT_OK) {	/* Establishes data input */
		Return (API->error);
	}
	if ((D[GMT_IN] = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	
	D[GMT_OUT] = GMT_alloc_dataset (GMT, D[GMT_IN], 0, 0, GMT_ALLOC_NORMAL);	/* Allocate identical output tables; we reallocate memory below */

	geo = GMT_is_geographic (GMT, GMT_IN);					/* TRUE for lon/lat coordinates */
	if (!geo && strchr (GMT_LEN_UNITS, (int)Ctrl->T.unit)) geo = TRUE;	/* Used units but did not set -fg; implicitly set -fg via geo */

	GMT_init_distaz (GMT, Ctrl->T.unit, Ctrl->T.mode, GMT_MAP_DIST);	/* Initialize distance scalings according to unit selected */
	
	/* Convert tolerance to degrees [or leave as Cartesian] */
	/* We must do this here since Douglas_Peucker_geog is doing its own thing and cannot use GMT_distance yet */
	
	tolerance = Ctrl->T.tolerance;
	switch (Ctrl->T.unit) {
		case 'd':	/* Various arc angular distances */
		case 'm': 
		case 's':
			tolerance /= GMT->current.map.dist[GMT_MAP_DIST].scale; /* Get degrees */
			break;
		case 'e':	/* Various Earth distances */
		case 'f':
		case 'k':
		case 'M':
		case 'n':
			tolerance /= GMT->current.map.dist[GMT_MAP_DIST].scale;	/* Get meters... */
			tolerance /= GMT->current.proj.DIST_M_PR_DEG;		/* ...then convert to spherical degrees */
			break;
		default:	/* Cartesian; do nothing further */
			break;
	}
	
	/* Process all tables and segments */
	for (tbl = 0; tbl < D[GMT_IN]->n_tables; tbl++) {
		for (seg = 0; seg < D[GMT_IN]->table[tbl]->n_segments; seg++) {
			S[GMT_IN]  = D[GMT_IN]->table[tbl]->segment[seg];
			S[GMT_OUT] = D[GMT_OUT]->table[tbl]->segment[seg];
			index = GMT_memory (GMT, NULL, S[GMT_IN]->n_rows, uint64_t);
			np_out = Douglas_Peucker_geog (GMT, S[GMT_IN]->coord[GMT_X], S[GMT_IN]->coord[GMT_Y], S[GMT_IN]->n_rows, tolerance, geo, index);

			GMT_alloc_segment (GMT, S[GMT_OUT], np_out, S[GMT_OUT]->n_columns, FALSE);	/* Reallocate to get correct n_rows */
			for (row = 0; row < np_out; row++) for (col = 0; col < S[GMT_IN]->n_columns; col++) {
				S[GMT_OUT]->coord[col][row] = S[GMT_IN]->coord[col][index[row]];
			}
			GMT_free (GMT, index);
			GMT_report (GMT, GMT_MSG_NORMAL, "Points in: %ld Points out: %ld\n", S[GMT_IN]->n_rows, np_out);
		}
	}

	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, Ctrl->Out.file, D[GMT_OUT]) != GMT_OK) {
		Return (API->error);
	}
	
	Return (GMT_OK);
}
