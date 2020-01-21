/*-----------------------------------------------------------------
 *
 *      Copyright (c) 1999-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *      Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/* x2sys_solve will read the crossover data base and determine the least-
 * squares correction coefficients for the specified field.  The correction
 * for each track is a sum of basis functions scaled by unknown coefficients
 * which we will solve for using least squares.  The normal equation matrix
 * is built directly and we solve the square linear system using Gauss-
 * Jordan elimination.
 *
 * Author:	Paul Wessel
 * Date:	18-SEPT-2008
 * Version:	1.0, based on the spirit of the old x_system code x_solve_dc_drift
 *		but completely rewritten from the ground up.
 *
 *
 */

/* #define DEBUGX */	/* Uncomment for testing */

#include "gmt_dev.h"
#include "mgd77/mgd77.h"
#include "x2sys.h"

#define THIS_MODULE_CLASSIC_NAME	"x2sys_solve"
#define THIS_MODULE_MODERN_NAME	"x2sys_solve"
#define THIS_MODULE_LIB		"x2sys"
#define THIS_MODULE_PURPOSE	"Determine least-squares systematic correction from crossovers"
#define THIS_MODULE_KEYS	">D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "->Vbd" GMT_ADD_x_OPT

#define N_COE_PARS	12	/* Total number of items that might be known at each crossover */
#define COL_COE		0	/* The crossover value in whatever field we are studying */
#define COL_XX		1	/* lon or x at crossover */
#define COL_YY		1	/* lat or y at crossover */
#define COL_T1		3	/* Time along track one (in seconds) */
#define COL_T2		4	/* Time along track two (in seconds) */
#define COL_D1		5	/* Distance along track one */
#define COL_D2		6	/* Distance along track two */
#define COL_H1		7	/* Heading along track one (in degrees) */
#define COL_H2		8	/* Heading along track two (in degrees) */
#define COL_Z1		9	/* Observation value along track one */
#define COL_Z2		10	/* Observation value along track two */
#define COL_WW		11	/* Composite weight at crossover */

#define N_BASIS		7	/* Number of basis functions currently encoded */

/* The 7 different kinds of corrections coded so far */
#define F_IS_CONSTANT	1	/* Subtract a constant from each track */
#define F_IS_DRIFT_D	2	/* Subtract a trend with distance from each track */
#define F_IS_HEADING	3	/* Subtract a magnetic heading correction from each track */
#define F_IS_GRAV1930	4	/* Subtract a trend with latitude from each track */
#define F_IS_SCALE	5	/* Apply a scale to the observations for each track */
#define F_IS_DRIFT_T	6	/* Subtract a trend with time from each track */
#define F_IS_SCALE_OFF	7	/* Apply a scale and offset to the observations for each track */

struct X2SYS_SOLVE_CTRL {
	struct X2S_SOLVE_In {
		bool active;
		char *file;
	} In;
	struct X2S_SOLVE_C {	/* -C */
		bool active;
		char *col;
	} C;
	struct X2S_SOLVE_E {	/* -E */
		bool active;
		int mode;
	} E;
	struct X2S_SOLVE_T {	/* -T */
		bool active;
		char *TAG;
	} T;
	struct X2S_SOLVE_W {	/* -W */
		bool active;
		bool unweighted_stats;
	} W;
};

/* The data matrix holds the COE information in these predefined column entries.
 * Not all columns are necessarily used and allocated:
 * Col 0:	COE value [Always used]
 * Col 1:	lon or x
 * Col 2:	lat or y
 * Col 3:	time_1
 * Col 4:	time_2
 * Col 5:	dist_1
 * Col 6:	dist_2
 * Col 7:	head_1
 * Col 8:	head_2
 * Col 9:	z_1
 * Col 10:	z_2
 * The array active_col[N_COE_PARS] will be true for those columns actually used
 */

/* Available basis functions.  To add more basis functions:
 * 1. Add another one based on the examples below.
 * 2. Increase N_BASIS by one.
 * 3. If new data columns are needed you need to modify N_COE_PARS and comment above
 * The arguments to each basis functions are:
 * P:	  The 2-D array that holds all the parameters for all crossovers.  Each basis function
 *	  only uses those columns it requires.
 * which: Either 0 or 1 to indicate the first or second track in the crossover
 * col:	  The crossover number we are working on.
 */

GMT_LOCAL double basis_constant (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a constant c*f = c*1 : == 1 */
	gmt_M_unused(P); gmt_M_unused(which); gmt_M_unused(row);
	return (1.0);	/* which, row are not used here */
}

GMT_LOCAL double basis_tdrift (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a linear drift rate in time c*f = c*t : t */
	return (P[COL_T1+which][row]);	/* When this is called the time columns have been corrected for t0 (start of track) */
}

GMT_LOCAL double basis_ddrift (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a linear drift rate in dist c*f = c*d : d */
	return (P[COL_D1+which][row]);
}

GMT_LOCAL double basis_cosh (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a dependence on cos(h)  c*f = c*cos(h) : cos(h) */
	return (cosd(P[COL_H1+which][row]));
}

GMT_LOCAL double basis_cos2h (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a dependence on cos(2*h)  c*f = c*cos(2*h) : cos(2*h) */
	return (cosd(2.0*P[COL_H1+which][row]));
}

GMT_LOCAL double basis_sinh (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a dependence on sin(h)  c*f = c*sin(h) : sin(h) */
	return (sind(P[COL_H1+which][row]));
}

GMT_LOCAL double basis_sin2h (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a dependence on sin(2*h)  c*f = c*sin(2*h) : sin(2*h) */
	return (sind(2.0*P[COL_H1+which][row]));
}

GMT_LOCAL double basis_siny2 (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a dependence on sin^2(y)  c*f = c*sin^2(y) : sin^2(y) */
	gmt_M_unused(which);
	return (pow (sind(P[COL_YY][row]), 2.0));	/* which not used since y is common to both tracks */
}

GMT_LOCAL double basis_z (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a dependence on value c*f = c*z : z */
	return (P[COL_Z1+which][row]);
}


GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_SOLVE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct X2SYS_SOLVE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_SOLVE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->C.col);
	gmt_M_str_free (C->T.TAG);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s -C<column> -Ec|d|g|h|s|t|z -T<TAG> [<coedata>] [%s] [-W[u]]\n\t[%s] [%s]%s[%s]\n\n",
		name, GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_x_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-C Specify the column name to process (e.g., faa, mag).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Equation to fit: specify <flag> to indicate model to fit per track:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c (constant offset):   Determine offset [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     d (drift by distance): Determine offset and drift-vs-distance rate.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     g (gravity latitude):  Determine amplitude of latitude gravity function.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     h (magnetic heading):  Determine amplitude of heading magnetic function.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     s (data scale):        Determine scaling factor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     t (drift over time):   Determine offset and drift-vs-time rate.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     z (data scale/offset): Determine offset and scaling factor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T <TAG> is the x2sys tag for the data set.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<coedata> is the ASCII data output file from x2sys_list [or we read stdin].\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Weights are present in last column for weighted fit [no weights].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append 'u' to report unweighted mean/std [Default, report weighted stats].\n");
	GMT_Option (API, "bi,di,x,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct X2SYS_SOLVE_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input files */
				Ctrl->In.active = true;
				if (n_files == 0) Ctrl->In.file = strdup (opt->arg);
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Needed to report correctly */
				Ctrl->C.active = true;
				Ctrl->C.col = strdup (opt->arg);
				break;
			case 'E':	/* Which model to fit */
				Ctrl->E.active = true;
				switch (opt->arg[0]) {
					case 'c':
						Ctrl->E.mode = F_IS_CONSTANT;
						break;
					case 'd':
						Ctrl->E.mode = F_IS_DRIFT_D;
						break;
					case 'g':
						Ctrl->E.mode = F_IS_GRAV1930;
						break;
					case 'h':
						Ctrl->E.mode = F_IS_HEADING;
						break;
					case 's':
						Ctrl->E.mode = F_IS_SCALE;
						break;
					case 't':
						Ctrl->E.mode = F_IS_DRIFT_T;
						break;
					case 'z':
						Ctrl->E.mode = F_IS_SCALE_OFF;
						break;
					default:
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unrecognized model: %s\n", opt->arg);
						n_errors++;
						break;
				}
				break;
			case 'T':
				Ctrl->T.active = true;
				Ctrl->T.TAG = strdup (opt->arg);
				break;
			case 'W':
				Ctrl->W.active = true;
				if (opt->arg[0] == 'u')		/* Report unweighted statistics anyway */
					Ctrl->W.unweighted_stats = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.mode < 0, "Syntax error -E: Choose among c, d, g, h, s, t and z\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}


#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LOCAL uint64_t next_unused_track (uint64_t *cluster, uint64_t n) {
	/* Determines the next track not yet assigned to a cluster */
	uint64_t k;
	for (k = 0; k < n; k++) if (cluster[k] == 0) return (k);
	return (n);	/* Found nothing so we are done */
}

int GMT_x2sys_solve (void *V_API, int mode, void *args) {
	char **trk_list = NULL, text[GMT_BUFSIZ] = {""}, frmt_name[16] = {""};
	char trk[2][GMT_LEN64], line[GMT_BUFSIZ] = {""};
	char file_TAG[GMT_LEN64] = {""}, file_column[GMT_LEN64] = {""};
	bool grow_list = false, normalize = false, first = true, active_col[N_COE_PARS];
	int *ID[2] = {NULL, NULL}, ks, t, error = GMT_NOERROR, max_len;
	int min_ID, max_ID;
	unsigned int rec_mode;
	uint64_t n_par = 0, n_in = 0, n, m, n_tracks = 0, n_active, n_constraints = 0;
	uint64_t i, p, j, k, r, s, row_off, row, n_COE = 0, w_col, id_col, bin_expect, *R = NULL, *col_off = NULL, *cluster = NULL;
	size_t n_alloc = GMT_INITIAL_MEM_ROW_ALLOC, n_alloc_t = GMT_CHUNK;

	double *N = NULL, *a = NULL, *b = NULL, *in = NULL, *data[N_COE_PARS], sgn, old_mean, new_mean, sw2, C_i, C_j;
	double old_stdev, new_stdev, e_k, min_extent, max_extent, range = 0.0, Sw, Sx, Sxx, var[N_BASIS];
	struct GMT_RECORD *In = NULL, *Out = NULL;
	struct X2SYS_INFO *S = NULL;
	struct X2SYS_BIX B;
	double (*basis[N_BASIS]) (double **, unsigned int, uint64_t);	/* Pointers to selected basis functions */

	struct X2SYS_SOLVE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the x2sys_solve main code ----------------------------*/

	gmt_enable_threads (GMT);	/* Set number of active threads, if supported */

	/* Initialize system via the tag */

	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &S, &B, &GMT->current.io), Ctrl->T.TAG);

	/* Verify that the chosen column is known to the system */

	if (Ctrl->C.col) x2sys_err_fail (GMT, x2sys_pick_fields (GMT, Ctrl->C.col, S), "-C");
	if (S->n_out_columns != 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: -C must specify a single column name\n");
		x2sys_end (GMT, S);
		Return (GMT_RUNTIME_ERROR);
	}

	gmt_M_memset (active_col, N_COE_PARS, bool); /* Initialize array */

	active_col[COL_COE] = true;	/* Always used */
	switch (Ctrl->E.mode) {	/* Set up pointers to basis functions and assign constants */
		case F_IS_CONSTANT:
			n_par = n_in = 1;
			basis[0] = &basis_constant;
			break;
		case F_IS_DRIFT_T:
			active_col[COL_T1] = active_col[COL_T2] = true;
			n_par = 2;	n_in = 3;
			basis[0] = &basis_constant;
			basis[1] = &basis_tdrift;
			break;
		case F_IS_DRIFT_D:
			active_col[COL_D1] = active_col[COL_D2] = true;
			n_par = 2;	n_in = 3;
			basis[0] = &basis_constant;
			basis[1] = &basis_ddrift;
			break;
		case F_IS_GRAV1930:
			active_col[COL_YY] = true;
			n_par = 2;	n_in = 2;
			basis[0] = &basis_constant;
			basis[1] = &basis_siny2;
			break;
		case F_IS_HEADING:
			active_col[COL_H1] = active_col[COL_H2] = true;
			n_par = 5;	n_in = 3;
			basis[0] = &basis_constant;
			basis[1] = &basis_cosh;
			basis[2] = &basis_cos2h;
			basis[3] = &basis_sinh;
			basis[4] = &basis_sin2h;
			break;
		case F_IS_SCALE:
			active_col[COL_Z1] = active_col[COL_Z2] = true;
			n_par = 1;	n_in = 2;
			basis[0] = &basis_z;
			break;
		case F_IS_SCALE_OFF:
			active_col[COL_Z1] = active_col[COL_Z2] = true;
			n_par = 2;	n_in = 2;
			basis[0] = &basis_constant;
			basis[1] = &basis_z;
			break;
	}
	if (Ctrl->W.active) n_in++;

	/* Allocate memory for COE data */

	gmt_M_memset (data, N_COE_PARS, double *);
	for (i = n_active = 0; i < N_COE_PARS; i++) {
		if (active_col[i]) {
			data[i] = gmt_M_memory (GMT, NULL, n_alloc, double);
			n_active++;
		}
	}
	if (!data[COL_WW]) data[COL_WW] = gmt_M_memory (GMT, NULL, n_alloc, double);
	for (i = 0; i < 2; i++) ID[i] = gmt_M_memory (GMT, NULL, n_alloc, int);

	if (n_tracks == 0 && !GMT->common.b.active[GMT_IN])	{	/* Create track list on the go */
		grow_list = true;
		trk_list = gmt_M_memory (GMT, NULL, n_alloc_t, char *);
	}

	bin_expect = n_in + 2;
	w_col = n_in - 1;
	id_col = w_col;		/* For binary files, ID columns start at end of record */
	min_ID = INT_MAX;	max_ID = -INT_MAX;

	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < bin_expect) {
		GMT_Report (API, GMT_MSG_NORMAL, "Binary file has %" PRIu64 " columns but %d is required\n", GMT->common.b.ncol[GMT_IN], (int)bin_expect);
		x2sys_end (GMT, S);
		Return (GMT_RUNTIME_ERROR);
	}

	for (k = 0; k < bin_expect; k++)	/* All input columns are floating point numbers here */
		gmt_set_column (GMT, GMT_IN, k, GMT_IS_FLOAT);

	/* Open the crossover info */

	rec_mode = (GMT->common.b.ncol[GMT_IN]) ? GMT_READ_DATA : GMT_READ_MIXED;
	if (GMT_Init_IO (API, GMT_IS_DATASET, rec_mode, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data inputs */
		x2sys_end (GMT, S);
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	/* Read the crossover file */

	n_COE = 0;
	do {	/* Keep returning records until we have no more files */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Keep returning records until we have no more files */
			if (gmt_M_rec_is_error (GMT)) {
				gmt_M_free (GMT, trk_list);
				x2sys_end (GMT, S);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_table_header (GMT)) {
				if (first) {	/* Must parse the first header to see if content matches tag and selected column */
					unsigned int bad = 0;
					sscanf (&GMT->current.io.curr_text[6], "%s %s", file_TAG, file_column);
					if (strcmp (Ctrl->T.TAG, file_TAG)) {
						GMT_Report (API, GMT_MSG_NORMAL, "The TAG in ASCII file %s is not compatible with -T\n", Ctrl->In.file);
						bad++;
					}
					if (strcmp (Ctrl->C.col, file_column)) {
						GMT_Report (API, GMT_MSG_NORMAL, "The column in ASCII file %s is not compatible with -Cs\n", Ctrl->In.file);
						bad++;
					}
					if (bad) {	/* Must bail */
						gmt_M_free (GMT, trk_list);
						x2sys_end (GMT, S);
						Return (GMT_RUNTIME_ERROR);
					}
					first = false;
				}
			}
			else if (gmt_M_rec_is_eof (GMT))	/* Reached end of file */
				break;
			continue;
		}
		in = In->data;
		if (In->text) {
			if ((ks = sscanf (In->text, "%s %s", trk[0], trk[1])) != 2) {
				GMT_Report (API, GMT_MSG_NORMAL, "Could not decode two track IDs - skipping record\n");
				continue;
			}
			for (i = 0; i < 2; i++) {	/* Look up track IDs */
				ID[i][n_COE] = x2sys_find_track (GMT, trk[i], trk_list, (unsigned int)n_tracks);	/* Return track id # for this leg */
				if (ID[i][n_COE] == -1) {	/* Leg not in the data base yet */
					if (grow_list) {	/* Add it */
						trk_list[n_tracks] = strdup (trk[i]);
						ID[i][n_COE] = (int)n_tracks++;
						if (n_tracks == n_alloc_t) {
							n_alloc_t <<= 1;
							trk_list = gmt_M_memory (GMT, trk_list, n_alloc_t, char *);
						}
					}
				}
			}
		
		}
		else {	/* Binary file with integer IDs */
			for (i = 0; i < 2; i++) {	/* Get IDs and keep track of min/max values */
				ID[i][n_COE] = irint (in[i+id_col]);
				if (ID[i][n_COE] < min_ID) min_ID = ID[i][n_COE];
				if (ID[i][n_COE] > max_ID) max_ID = ID[i][n_COE];
			}
		}
		/* Handle input order differently depending on what is expected */
		switch (Ctrl->E.mode) {
			case F_IS_CONSTANT:
				data[COL_COE][n_COE] = in[0];
				break;
			case F_IS_DRIFT_T:
				data[COL_T1][n_COE]  = in[0];
				data[COL_T2][n_COE]  = in[1];
				data[COL_COE][n_COE] = in[2];
				break;
			case F_IS_DRIFT_D:
				data[COL_D1][n_COE]  = in[0];
				data[COL_D2][n_COE]  = in[1];
				data[COL_COE][n_COE] = in[2];
				break;
			case F_IS_GRAV1930:
				data[COL_YY][n_COE]  = in[0];
				data[COL_COE][n_COE] = in[1];
				break;
			case F_IS_HEADING:
				data[COL_H1][n_COE]  = in[0];
				data[COL_H2][n_COE]  = in[1];
				data[COL_COE][n_COE] = in[2];
				break;
			case F_IS_SCALE:
			case F_IS_SCALE_OFF:
				data[COL_Z1][n_COE]  = in[0];
				data[COL_Z2][n_COE]  = in[1];
				data[COL_COE][n_COE] = in[0] - in[1];
				break;
		}
		data[COL_WW][n_COE] = (Ctrl->W.active) ? in[w_col] : 1.0;	/* Weight */
		if (gmt_M_is_dnan (data[COL_COE][n_COE])) {
			GMT_Report (API, GMT_MSG_VERBOSE, "COE == NaN skipped during reading\n");
			continue;
		}
		if (++n_COE == n_alloc) {
			n_alloc <<= 1;
			for (i = 0; i < N_COE_PARS; i++) if (active_col[i]) data[i] = gmt_M_memory (GMT, data[i], n_alloc, double);
			data[COL_WW] = gmt_M_memory (GMT, data[COL_WW], n_alloc, double);
			for (i = 0; i < 2; i++) ID[i] = gmt_M_memory (GMT, ID[i], n_alloc, int);
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	
	if (GMT->common.b.active[GMT_IN]) {	/* Binary input */
		/* Here, first two cols have track IDs and we do not write track names */
		uint64_t n_tracks2;
		char *check = NULL;

		/* Check that IDs are all contained within 0 <= ID < n_tracks and that there are no gaps */
		n_tracks2 = max_ID - min_ID + 1;
		if (n_tracks && n_tracks2 != n_tracks) {
			GMT_Report (API, GMT_MSG_NORMAL, "The ID numbers in the binary file %s are not compatible with the <trklist> length\n", Ctrl->In.file);
			error = true;
		}
		else {	/* Either no tracks read before or the two numbers did match properly */
			/* Look for ID gaps */
			n_tracks = n_tracks2;
			check = gmt_M_memory (GMT, NULL, n_tracks, char);
			for (k = 0; k < n_COE; k++) for (i = 0; i < 2; i++) check[ID[i][k]] = true;	/* Flag those tracks that have been mentioned */
			for (k = 0; k < n_tracks && check[k]; k++);	/* Loop until end of until first non-used ID */
			gmt_M_free (GMT, check);
			if (k < n_tracks) {
				GMT_Report (API, GMT_MSG_NORMAL,
				            "The ID numbers in the binary file %s do not completely cover the range 0 <= ID < n_tracks!\n", Ctrl->In.file);
				error = true;
			}
		}
		if (error) {	/* Delayed the cleanup until here */
			for (i = 0; i < N_COE_PARS; i++) if (active_col[i]) gmt_M_free (GMT, data[i]);
			gmt_M_free (GMT, data[COL_WW]);
			for (i = 0; i < 2; i++) gmt_M_free (GMT, ID[i]);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Found %d COE records\n", n_COE);
	for (i = 0; i < N_COE_PARS; i++) if (active_col[i]) data[i] = gmt_M_memory (GMT, data[i], n_COE, double);
	data[COL_WW] = gmt_M_memory (GMT, data[COL_WW], n_COE, double);

	normalize = (Ctrl->E.mode == F_IS_DRIFT_T || Ctrl->E.mode == F_IS_DRIFT_D);	/* Only when the linear drift term is in effect */
	if (normalize) {	/* For numerical stability, normalize distances or times to fall in 0-1 range */
		min_extent = DBL_MAX;	max_extent = -DBL_MAX;
		j = (Ctrl->E.mode == F_IS_DRIFT_T) ? COL_T1 : COL_D1;	/* Which variable we are working on */
		for (k = 0; k < n_COE; k++) {
			for (i = 0; i < 2; i++) {
				if (data[j+i][k] < min_extent) min_extent = data[j+i][k];
				if (data[j+i][k] > max_extent) max_extent = data[j+i][k];
			}
		}
		range = max_extent - min_extent;
		for (k = 0; k < n_COE; k++) for (i = 0; i < 2; i++) data[j+i][k] /= range;	/* Get normalized time or distance */
	}

	/* Estimate old weighted mean and std.dev */

	if (Ctrl->W.unweighted_stats) {
		for (k = 0, Sw = Sx = Sxx = 0.0; k < n_COE; k++) {	/* For each crossover */
			Sx += data[COL_COE][k];
			Sxx += (data[COL_COE][k] * data[COL_COE][k]);
		}
		Sw = (double)n_COE;
	}
	else {
		for (k = 0, Sw = Sx = Sxx = 0.0; k < n_COE; k++) {	/* For each crossover */
			Sw  += data[COL_WW][k];
			Sx  += (data[COL_WW][k] * data[COL_COE][k]);
			Sxx += (data[COL_WW][k] * data[COL_COE][k] * data[COL_COE][k]);
		}
	}
	old_mean = Sx / Sw;
	old_stdev = sqrt ((n_COE * Sxx - Sx * Sx) / (Sw*Sw*(n_COE - 1.0)/n_COE));

	/* Because it can happen that a track has less crossovers than n_par, we must determine R(track), the max parameters per track we can fit */
	R = gmt_M_memory (GMT, NULL, n_tracks, uint64_t);	/* Number of parameter for each track p, so that 1 <= R(p) < n_par */
	col_off = gmt_M_memory (GMT, NULL, n_tracks, uint64_t);	/* Since R(p) may vary, need cumulative sum of R(p) to get correct column */
	for (k = 0; k < n_COE; k++) {	/* Count number of crossovers per track */
		i = ID[0][k];	/* Get track # 1 ID */
		j = ID[1][k];	/* Get track # 2 ID */
		R[i]++;		/* Increase COE count for track i */
		R[j]++;		/* Increase COE count for track j */
	}
	for (p = n = 0; p < n_tracks; p++) {	/* For each track, determine R[track], total number of parameters, and the column offsets */
		(GMT->common.b.active[GMT_IN]) ? sprintf (trk[0], "%" PRIu64, p) : sprintf (trk[0], "%s", trk_list[p]);
		if (R[p] < n_par)	/* Came up short */
			GMT_Report (API, GMT_MSG_VERBOSE, "Track %s only has %" PRIu64 " crossings so can only solve for %" PRIu64 " of the %" PRIu64 " parameters\n", trk[0], R[p], R[p], n_par);
		else
			R[p] = n_par;
		n += R[p];	/* Add up total number of unknowns across all tracks */
		if (p) col_off[p] = col_off[p-1] + R[p-1];	/* Offset along a row given not every track may have n_par columns. Obviously, col_off[0] = 0 */
	}

	/* Determine how many extra constraints we need to add.  This is required because if the model has a constant
	 * as part of the parameters, then we have an unconstrained absolute level.  We deal with this by requiring the sum of
	 * the constants to be zero.  However, it may happen that not all tracks can be connected to other tracks (and thus form
	 * independent crossing clusters) and then we need such a constraint for each cluster of tracks that intersect.  Here
	 * we find the number of such clusters and which cluster each track belongs to. */

	if (Ctrl->E.mode != F_IS_SCALE) {	/* Constraint equations are needed when there is an offset in the model (this means all but the scaling model) */
		char *C = NULL;
		uint64_t ij, g, n_in_cluster, *member = NULL;

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Determine number of independent track clusters\n");
		C = gmt_M_memory (GMT, NULL, n_tracks*n_tracks, char);		/* For the connectivity matrix that shows which tracks cross */
		cluster = gmt_M_memory (GMT, NULL, n_tracks, uint64_t);		/* Array to remember which cluster a track belongs to */
		/* First build the symmetric adjacency matrix C */
		for (k = 0; k < n_COE; k++) {	/* Identify crossing pairs (the # of crossings doesn't matter) */
			i = ID[0][k];	/* Get track # 1 ID */
			j = ID[1][k];	/* Get track # 2 ID */
			ij = i * n_tracks + j;	/* Index in upper triangular matrix */
			if (C[ij] == 0) C[ij] = C[j*n_tracks+i] = 1;	/* These cross, set C_ij = C_ji = 1 */
		}

		member = gmt_M_memory (GMT, NULL, n_tracks, uint64_t);	/* Temp array to keep all members of current cluster */
		n_constraints = 0;
		/* Below, cluster[] array will use counting from 1 to n_constraints, but later we reset to run from 0 instead */
		while ((p = next_unused_track (cluster, n_tracks)) < n_tracks) {	/* Still more clusters to form */
			n_constraints++;		/* Increment number of constraints, this will happen at least once (i.e., for each cluster) */
			gmt_M_memset (member, n_tracks, uint64_t);	/* Cluster starts off with no members */
			member[0] = p;			/* This is the first member of this cluster */
			cluster[p] = n_constraints;	/* So we set this cluster number right away */
			n_in_cluster = 1;		/* Only one track in this cluster so far */
			for (i = 0; i < n_tracks; i++) {	/* Scan adjacency matrix for other tracks than directly crossed track p */
				if (cluster[i] || C[p*n_tracks+i] == 0) continue;	/* Skip tracks already dealt with or not crossing track p */
				member[n_in_cluster++] = i;	/* i crossed p! Add track #i to p's cluster... */
				cluster[i] = cluster[p];	/* ...and assign it to the same cluster as p */
			}
			g = 1;	/* Start at 2nd entry since 1st (p) has already been given its direct cluster numbers */
			while (g < n_in_cluster) {	/* Since n_in_cluster may grow we must check via a test at the top */
				k = member[g];	/* Now we examine the k't row in the adjacency matrix for other tracks that crossed k */
				for (i = 0; i < n_tracks; i++) {
					if (cluster[i] || C[k*n_tracks+i] == 0) continue;	/* Skip tracks already dealt with or not crossing track k */
					member[n_in_cluster++] = i;	/* Add track #i to p's growing cluster */
					/* coverity[copy_paste_error] */		/* For Coverity analysis. Do not remove this comment */
					cluster[i] = cluster[p];	/* And set it to same cluster as p */
				}
				g++;	/* Go to next cluster member */
			}
			/* Done determining the cluster for track p, go to next track */
		}
		for (g = 0; g < n_tracks; g++) cluster[g]--;	/* Shift cluster numbers to start at zero instead of 1 */
		gmt_M_free (GMT, member);
		gmt_M_free (GMT, C);

		if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
			if (n_constraints > 1) {	/* Report on the clusters */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " tracks are grouped into %" PRIu64 " independent clusters:\n", n, n_constraints);
				for (k = 0; k < n_constraints; k++) {
					for (p = i = 0; p < n_tracks; p++) if (cluster[p] == k) i++;
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "===> Cluster # %" PRIu64 " [%" PRIu64 " tracks]:\n", k, i);
					for (p = 0; p < n_tracks; p++) if (cluster[p] == k) {
						(GMT->common.b.active[GMT_IN]) ? sprintf (trk[0], "%" PRIu64, p) : sprintf (trk[0], "%s", trk_list[p]);
						GMT_Report (API, GMT_MSG_LONG_VERBOSE, "\tTrack %s\n", trk[0]);
					}
				}
			}
			else
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%" PRIu64 " tracks form a single connected cluster\n", n);
		}
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Number of unknowns is %" PRIu64 " and number of constraints is %" PRIu64 "\n", n, n_constraints);

	/* Set up matrix and column vectors */

	m = n + n_constraints;	/* Need extra row(s)/column(s) to handle Lagrange's multiplier(s) for unknown absolute level(s) */
	/* Allocate arrays for N*x = b, the m x m linear system */
	N = gmt_M_memory (GMT, NULL, m*m, double);
	b = gmt_M_memory (GMT, NULL, m, double);

	/* Build A'A and A'b ==> N*x = b normal equation matrices directly since A may be too big to do A'A by multiplication.
	 * For all corrections that involve a constant shift we must add the constraint that such shifts sum to zero; this
	 * is implemented by adding extra rows/columns with the appropriate 0s and 1s and solve for Lagrange multipliers. */

	for (p = row = 0; p < n_tracks; p++) {	/* For each track */
		for (s = 0; s < R[p]; s++, row++) {	/* For each of this track's unknown parameters  */
			row_off = m * row;	/* Start of current row in N */
			for (k = 0; k < n_COE; k++) {	/* For each crossover */
				i = ID[0][k];	/* Get track # 1 ID */
				j = ID[1][k];	/* Get track # 2 ID */
				if (i == p) {		/* p equals track # 1 */
					sgn = +1.0;	t = 0;
				} else if (p == j) {	/* p equals track # 2 */
					sgn = -1.0;	t = 1;
				} else continue;	/* COE did not involve track p, goto next crossover */
				sw2 = sgn * data[COL_WW][k] * data[COL_WW][k];
				for (r = 0; r < R[i]; r++)	/* For track i's parameters in f(p)  */
					N[row_off+col_off[i]+r] += sw2 * (basis[r](data,0,k) * basis[s](data,t,k));
				for (r = 0; r < R[j]; r++)	/* For track j's parameters in f(p)  */
					N[row_off+col_off[j]+r] -= sw2 * (basis[r](data,1,k) * basis[s](data,t,k));
				b[row] += sw2 * (data[COL_COE][k] * basis[s](data,t,k));
			}
			/* Augmented column entry for Lagrange multiplier for this cluster */
			if (n_constraints) N[row_off+n+cluster[p]] = 1.0;
		}
	}
	if (n_constraints) {	/* Append the constraint equations that each cluster's offset corrections add to zero */
		for (k = 0, row_off = m * n; k < n_constraints; k++, row_off += m) {	/* For each cluster */
			for (p = 0; p < n_tracks; p++) if (cluster[p] == k) N[row_off+col_off[p]] = 1.0;
		}
		gmt_M_free (GMT, cluster);
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Matrix equation N * a = b: (N = %" PRIu64 " x %" PRIu64 ")\n", m, m);

#ifdef DEBUGX
	for (i = 0; i < m; i++) {
		for (j = 0; j < m; j++) GMT_Message (API, GMT_TIME_NONE, "%8.2f\t", N[i*m+j]);
		GMT_Message (API, GMT_TIME_NONE, "\t%8.2f\n", b[i]);
	}
#endif

	/* Get LS solution */

	if ((error = gmt_gaussjordan (GMT, N, (unsigned int)m, b)) != 0) {
		GMT_Report (API, GMT_MSG_NORMAL, "Singular matrix - unable to solve!\n");
		gmt_M_free (GMT, N);
		error = GMT_RUNTIME_ERROR;
		goto END;
	}

	gmt_M_free (GMT, N);
	a = b;	/* Convenience since the solution is called a in the notes and in Wessel [2010] */

	/* Estimate new st.dev. */

	for (k = 0, Sw = Sx = Sxx = 0.0; k < n_COE; k++) {	/* For each crossover */
		i = ID[0][k];	/* Get track # 1 ID */
		j = ID[1][k];	/* Get track # 2 ID */
		for (r = 0, C_j = 0.0; r < R[j]; r++)	/* Correct adjustment for track j  */
			C_j += a[col_off[j]+r]*basis[r](data,1,k);
		for (r = 0, C_i = 0.0; r < R[i]; r++)	/* Correct adjustment for track i  */
			C_i += a[col_off[i]+r]*basis[r](data,0,k);
		e_k = data[COL_COE][k] + (C_j - C_i);	/* Adjusted crossover error */

		if (Ctrl->W.unweighted_stats) {
			Sx  += e_k;
			Sxx += (e_k * e_k);
		}
		else {
			Sw += data[COL_WW][k];
			Sx += (data[COL_WW][k] * e_k);
			Sxx += (data[COL_WW][k] * e_k * e_k);
		}
#ifdef DEBUGX
		GMT_Message (API, GMT_TIME_NONE, "COE # %d: Before: %g After: %g\n", k, data[COL_COE][k], e_k);
#endif
	}
	if (Ctrl->W.unweighted_stats) Sw = (double)n_COE;
	new_mean = Sx / Sw;
	new_stdev = sqrt ((n_COE * Sxx - Sx * Sx) / (Sw*Sw*(n_COE - 1.0)/n_COE));

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Before correction, mean and st.dev.: %g %g \n", old_mean, old_stdev);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "After correction, mean and st.dev. : %g %g\n", new_mean, new_stdev);

	/* Write correction table */

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		error = API->error;
		goto END;
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data output and sets access mode */
		error = API->error;
		goto END;
	}
	Out = gmt_new_record (GMT, NULL, line);

	/* Calculate the format string for the cruise name so that the printed table is better formatted */
	max_len = 0;
	for (p = 0; p < n_tracks; p++)
		max_len = MAX(max_len, (int)strlen(trk_list[p])+1);
	sprintf (frmt_name, "%%-%ds", max_len+2);

	for (p = 0; p < n_tracks; p++) {
		if (normalize) a[col_off[p]+1] /= range;	/* Unnormalize slopes */
		(GMT->common.b.active[GMT_IN]) ? sprintf (line, "%" PRIu64, p) : sprintf (line, frmt_name, trk_list[p]);
		strcat (line, "\t");
		strcat (line, Ctrl->C.col);
		gmt_M_memset (var, N_BASIS, double);	/* Reset all parameters to zero */
		for (r = 0; r < R[p]; r++) var[r] = a[col_off[p]+r];	/* Just get the first R(p) items; the rest are set to 0 */
		switch (Ctrl->E.mode) {	/* Set up pointers to basis functions and assign constants */
			case F_IS_CONSTANT:
				sprintf (text, "\t%g", var[0]);
				break;
			case F_IS_DRIFT_T:
				sprintf (text, "\t%g\t%g*((time-T))", var[0], var[1]);
				break;
			case F_IS_DRIFT_D:
				sprintf (text, "\t% 10.4f\t% g*((dist))", var[0], var[1]);
				break;
			case F_IS_GRAV1930:
				sprintf (text, "\t%g\t%g*sin((lat))^2", var[0], var[1]);
				break;
			case F_IS_HEADING:
				sprintf (text, "\t%g\t%g*cos((azim))\t%g*cos(2*(azim))\t%g*sin((azim))\t%g*sin(2*(azim))", var[0], var[1], var[2], var[3], var[4]);
				break;
			case F_IS_SCALE:
				sprintf (text, "\t%g*((%s))", 1.0 - var[0], Ctrl->C.col);
				break;
			case F_IS_SCALE_OFF:
				sprintf (text, "\t%g\t%g*((%s))", var[0], 1.0 - var[1], Ctrl->C.col);
				break;
		}
		strcat (line, text);
		GMT_Put_Record (API, GMT_WRITE_DATA, Out);
	}

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}
	/* Free up memory */

END:
	for (i = 0; i < N_COE_PARS; i++) if (active_col[i]) gmt_M_free (GMT, data[i]);
	gmt_M_free (GMT, data[COL_WW]);
	for (i = 0; i < 2; i++) gmt_M_free (GMT, ID[i]);
	gmt_M_free (GMT, b);
	gmt_M_free (GMT, R);
	gmt_M_free (GMT, col_off);
	if (!GMT->common.b.active[GMT_IN]) x2sys_free_list (GMT, trk_list, n_tracks);
	gmt_M_free (GMT, Out);

	x2sys_end (GMT, S);

	Return (error);
}
