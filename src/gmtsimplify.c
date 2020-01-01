/*
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
 * API functions to support the gmtsimplify application.
 *
 * Author:	Joaquim Luis
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: gmtsimplify applies the Douglas-Peucker algorithm to simplify a line
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

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtsimplify"
#define THIS_MODULE_MODERN_NAME	"gmtsimplify"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Line reduction using the Douglas-Peucker algorithm"
#define THIS_MODULE_KEYS	"<D{,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vbdefghio" GMT_OPT("HMm")

/* Control structure for gmtsimplify */

struct GMTSIMPLIFY_CTRL {
	struct Out {	/* ->[<outfile>] */
		bool active;
		char *file;
	} Out;
	struct T {	/* 	-T[-|=|+]<tolerance>[d|s|m|e|f|k|M|n] */
		bool active;
		int mode;	/* Can be negative */
		double tolerance;
		char unit;
	} T;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTSIMPLIFY_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GMTSIMPLIFY_CTRL);
	
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTSIMPLIFY_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -T<tolerance>[<unit>] [%s] [%s]\n", name, GMT_V_OPT, GMT_b_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	gmt_dist_syntax (API->GMT, 'T', "Set tolerance as the maximum distance mismatch.");
	GMT_Message (API, GMT_TIME_NONE, "\t   No units means we will do a Cartesian calculation instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Option (API, "V,bi2,bo,d,e,f,g,h,i,o,:,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMTSIMPLIFY_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtsimplify and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;
			case '>':	/* Write to named output file instead of stdout */
				Ctrl->Out.active = true;
				if (n_files++ == 0 && opt->arg[0]) Ctrl->Out.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */
			
			case 'T':	/* Set tolerance distance */
				Ctrl->T.active = true;
				Ctrl->T.mode = gmt_get_distance (GMT, opt->arg, &(Ctrl->T.tolerance), &(Ctrl->T.unit));
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.mode == -1, "Syntax error -T: Unrecognized unit.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.mode == -2, "Syntax error -T: Unable to decode tolerance distance.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.mode == -3, "Syntax error -T: Tolerance is negative.\n");
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 2, "Syntax error: Binary input data (-bi) must have at least 2 columns.\n");
	n_errors += gmt_M_check_condition (GMT, n_files > 1, "Syntax error: Only one output destination can be specified.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define GMT_sqr(x) ((x)*(x))

/* Stack-based Douglas Peucker line simplification routine */
/* returned value is the number of output points */

/* This implementation of the algorithm has been kindly provided by
   Dr. Gary J. Robinson, Environmental Systems Science Centre,
   University of Reading, Reading, UK (gazza@mail.nerc-essc.ac.uk); his
   subroutine forms the basis for this program.

  Note: The algorithm uses Cartesian distance calculations in determining
        which points to remove.  We should ideally replace that with a
	spherical operation.
 */

GMT_LOCAL uint64_t Douglas_Peucker_geog (struct GMT_CTRL *GMT, double x_source[], double y_source[], uint64_t n_source, double band, bool geo, uint64_t index[]) {
/* x/y_source	Input coordinates, n_source of them.  These are not changed */
/* band;	tolerance in Cartesian user units or degrees */
/* geo:		true if data is lon/lat */
/* index[]	output coordinates indices */

	uint64_t n_stack, n_dest, start, end, i, sig;
	uint64_t *sig_start = NULL, *sig_end = NULL;	/* indices of start&end of working section */

	double dev_sqr, max_dev_sqr, band_sqr;
	double x12, y12, d12, x13, y13, d13, x23, y23, d23;

	/* check for simple cases */

	if (n_source < 3) {     /* one or two points we return as is */
		for (i = 0; i < n_source; i++) index[i] = i;
		return (n_source);
	}

	/* more complex case. initialise stack */

	sig_start = gmt_M_memory (GMT, NULL, n_source, uint64_t);
	sig_end   = gmt_M_memory (GMT, NULL, n_source, uint64_t);

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

	gmt_M_free (GMT, sig_start);
	gmt_M_free (GMT, sig_end);

	return (n_dest);
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmtsimplify (void *V_API, int mode, void *args) {
	int error;
	unsigned int smode = GMT_NO_STRINGS;
	bool geo, poly, skip;
	uint64_t tbl, col, row, seg_in, seg_out, np_out, ns_in = 0, ns_out = 0, n_in_tbl, *index = NULL;
	uint64_t dim_out[4] = {1, 0, 0, 0}, n_saved;
	
	double tolerance;
	
	struct GMT_DATASET *D[2] = {NULL, NULL};
	struct GMT_DATASEGMENT *S[2] = {NULL, NULL};
	struct GMTSIMPLIFY_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);
	
	/*---------------------------- This is the gmtsimplify main code ----------------------------*/

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input table data\n");
	if (Ctrl->T.mode > 1) {
		GMT_Report (API, GMT_MSG_VERBOSE, "gmtsimplify only implemented using Flat-Earth calculations.\n");
		Ctrl->T.mode = 1;	/* Limited to Flat Earth calculations for now */
	}
	
	/* Now we are ready to take on some input values */
	/* Allocate memory and read in all the files; each file can have many lines */
	
	/* We read as lines even though some/all segments could be polygons. */
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if ((D[GMT_IN] = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}
	if (D[GMT_IN]->n_columns < 2) {
		GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least 2 are needed\n", (int)D[GMT_IN]->n_columns);
		Return (GMT_DIM_TOO_SMALL);
	}
	geo = gmt_M_is_geographic (GMT, GMT_IN);					/* true for lon/lat coordinates */
	if (!geo && strchr (GMT_LEN_UNITS, (int)Ctrl->T.unit)) geo = true;	/* Used units but did not set -fg; implicitly set -fg via geo */

	gmt_init_distaz (GMT, Ctrl->T.unit, Ctrl->T.mode, GMT_MAP_DIST);	/* Initialize distance scalings according to unit selected */
	
	/* Convert tolerance to degrees [or leave as Cartesian] */
	/* We must do this here since Douglas_Peucker_geog is doing its own thing and cannot use gmt_distance yet */
	
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
	
	dim_out[GMT_TBL] = D[GMT_IN]->n_tables;		/* Allocate at least as many tables as the input source */
	dim_out[GMT_COL] = D[GMT_IN]->n_columns;	/* Allocate same number of columns tables as the input source */
	if ((D[GMT_OUT] = GMT_Create_Data (API, GMT_IS_DATASET, D[GMT_IN]->geometry, 0, dim_out, NULL, NULL, 0, 0, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a data set for output segments\n");
		Return (API->error);
	}
	
	for (tbl = 0; tbl < D[GMT_IN]->n_tables; tbl++) {
		n_in_tbl = 0;
		D[GMT_OUT]->table[tbl]->segment = gmt_M_memory (GMT, NULL, D[GMT_IN]->table[tbl]->n_segments, struct GMT_DATASEGMENT *);	/* Initial (and max) allocation of segments */
		for (seg_in = seg_out = 0; seg_in < D[GMT_IN]->table[tbl]->n_segments; seg_in++) {
			S[GMT_IN]  = D[GMT_IN]->table[tbl]->segment[seg_in];
			/* If input segment is a closed polygon then the simplified segment must have at least 4 points, else 3 is enough */
			poly = (!gmt_polygon_is_open (GMT, S[GMT_IN]->data[GMT_X], S[GMT_IN]->data[GMT_Y], S[GMT_IN]->n_rows));
			index = gmt_M_memory (GMT, NULL, S[GMT_IN]->n_rows, uint64_t);
			np_out = Douglas_Peucker_geog (GMT, S[GMT_IN]->data[GMT_X], S[GMT_IN]->data[GMT_Y], S[GMT_IN]->n_rows, tolerance, geo, index);
			skip = ((poly && np_out < 4) || (np_out == 2 && S[GMT_IN]->data[GMT_X][index[0]] == S[GMT_IN]->data[GMT_X][index[1]] && S[GMT_IN]->data[GMT_Y][index[0]] == S[GMT_IN]->data[GMT_Y][index[1]]));
			if (!skip) {	/* Must allocate one segment for output */
				smode = (S[GMT_IN]->text) ? GMT_WITH_STRINGS : GMT_NO_STRINGS;
				S[GMT_OUT] = GMT_Alloc_Segment (GMT->parent, smode, np_out, S[GMT_IN]->n_columns, NULL, NULL);
				S[GMT_OUT]->n_rows = np_out;
				D[GMT_OUT]->table[tbl]->segment[seg_out] = S[GMT_OUT];
				for (row = 0; row < np_out; row++) {
					for (col = 0; col < S[GMT_IN]->n_columns; col++)	/* Copy coordinates via index lookup */
						S[GMT_OUT]->data[col][row] = S[GMT_IN]->data[col][index[row]];
					if (smode) S[GMT_OUT]->text[row] = strdup (S[GMT_IN]->text[row]);
				}
				seg_out++;		/* Move on to next output segment */
				ns_in++;		/* Input segment with points */
				if (np_out) ns_out++;	/* Output segment with points */
				n_in_tbl += np_out;
				n_saved = np_out;
			}
			else
				n_saved = 0;
			gmt_M_free (GMT, index);	/* No longer needed */
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Points in: %" PRIu64 " Points out: %" PRIu64 "\n", S[GMT_IN]->n_rows, n_saved);
		}
		if (seg_out < D[GMT_IN]->table[tbl]->n_segments) D[GMT_OUT]->table[tbl]->segment = gmt_M_memory (GMT, D[GMT_OUT]->table[tbl]->segment, seg_out, struct GMT_DATASEGMENT *);	/* Reduce allocation to # of segments */
		D[GMT_OUT]->table[tbl]->n_segments = seg_out;	/* Update segment count */
		D[GMT_OUT]->table[tbl]->n_records = n_in_tbl;	/* Update table count */
		D[GMT_OUT]->n_records += n_in_tbl;		/* Update dataset count of total records*/
		D[GMT_OUT]->n_segments += seg_out;		/* Update dataset count of total segments*/
	}
	
	if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, D[GMT_IN]->geometry, GMT_WRITE_SET, NULL, Ctrl->Out.file, D[GMT_OUT]) != GMT_NOERROR) {
		Return (API->error);
	}
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Segments in: %" PRIu64 " Segments out: %" PRIu64 "\n", ns_in, ns_out);
	
	Return (GMT_NOERROR);
}
