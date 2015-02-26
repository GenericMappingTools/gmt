/*-----------------------------------------------------------------
 *	$Id$
 *
 *      Copyright (c) 1999-2015 by P. Wessel
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
 *      Contact info: gmt.soest.hawaii.edu
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

#define THIS_MODULE_NAME	"x2sys_solve"
#define THIS_MODULE_LIB		"x2sys"
#define THIS_MODULE_PURPOSE	"Determine least-squares systematic correction from crossovers"

#include "x2sys.h"

#define GMT_PROG_OPTIONS "->Vb"

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

/* The 6 different kinds of corrections coded so far */
#define F_IS_CONSTANT	1	/* Subtract a constant from each track */
#define F_IS_DRIFT_D	2	/* Subtract a trend with distance from each track */
#define F_IS_HEADING	3	/* Subtract a magnetic heading correction from each track */
#define F_IS_GRAV1930	4	/* Subtract a trend with latitude from each track */
#define F_IS_SCALE	5	/* Apply a scale to the observations for each track */
#define F_IS_DRIFT_T	6	/* Subtract a trend with time from each track */

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
#ifdef SAVEFORLATER
	struct X2S_SOLVE_I {	/* -I */
		bool active;
		char *file;
	} I;
#endif
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

double basis_constant (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a constant c*f = c*1 : == 1 */
	GMT_UNUSED(P); GMT_UNUSED(which); GMT_UNUSED(row);
	return (1.0);	/* which, row are not used here */
}

double basis_tdrift (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a linear drift rate in time c*f = c*t : t */
	return (P[COL_T1+which][row]);	/* When this is called the time columns have been corrected for t0 (start of track) */
}

double basis_ddrift (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a linear drift rate in dist c*f = c*d : d */
	return (P[COL_D1+which][row]);
}

double basis_cosh (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a dependence on cos(h)  c*f = c*cos(h) : cos(h) */
	return (cosd(P[COL_H1+which][row]));
}

double basis_cos2h (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a dependence on cos(2*h)  c*f = c*cos(2*h) : cos(2*h) */
	return (cosd(2.0*P[COL_H1+which][row]));
}

double basis_sinh (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a dependence on sin(h)  c*f = c*sin(h) : sin(h) */
	return (sind(P[COL_H1+which][row]));
}

double basis_sin2h (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a dependence on sin(2*h)  c*f = c*sin(2*h) : sin(2*h) */
	return (sind(2.0*P[COL_H1+which][row]));
}

double basis_siny2 (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a dependence on sin^2(y)  c*f = c*sin^2(y) : sin^2(y) */
	GMT_UNUSED(which);
	return (pow (sind(P[COL_YY][row]), 2.0));	/* which not used since y is common to both tracks */
}

double basis_z (double **P, unsigned int which, uint64_t row) {	/* Basis function f for a dependence on value c*f = c*z : z */
	return (P[COL_Z1+which][row]);
}


void *New_x2sys_solve_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_SOLVE_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_SOLVE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	return (C);
}

void Free_x2sys_solve_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_SOLVE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->In.file) free (C->In.file);
	if (C->C.col) free (C->C.col);
#ifdef SAVEFORLATER
	if (C->I.file) free (C->I.file);
#endif
	if (C->T.TAG) free (C->T.TAG);
	GMT_free (GMT, C);
}

int GMT_x2sys_solve_usage (struct GMTAPI_CTRL *API, int level) {
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
#ifdef SAVEFORLATER
	GMT_Message (API, GMT_TIME_NONE, "usage: x2sys_solve -C<column> -E<flag> -T<TAG> [<coedata>] [-I<tracklist>] [%s] [-W]\n\t[%s]\n\n", GMT_V_OPT, GMT_bi_OPT);
#else
	GMT_Message (API, GMT_TIME_NONE, "usage: x2sys_solve -C<column> -E<flag> -T<TAG> [<coedata>] [%s] [-W[u]]\n\t[%s]\n\n", GMT_V_OPT, GMT_bi_OPT);
#endif

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-C Specify the column name to process (e.g., faa, mag).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Equation to fit: specify <flag> as c (constant), d (drift over distance),\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     g (latitude), h (heading), s (scale with data), or t (drift over time) [c].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T <TAG> is the x2sys tag for the data set.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<coedata> is the ASCII data output file from x2sys_list [or we read stdin].\n");
#ifdef SAVEFORLATER
	GMT_Message (API, GMT_TIME_NONE, "\t-I List of tracks and their start date (required for -Et).\n");
#endif
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Weights are present in last column for weighted fit [no weights].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append 'u' to report unweighted mean/std [Default, report weighted stats].\n");
	GMT_Option (API, "bi,.");
	
	return (EXIT_FAILURE);
}

int GMT_x2sys_solve_parse (struct GMT_CTRL *GMT, struct X2SYS_SOLVE_CTRL *Ctrl, struct GMT_OPTION *options) {

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
				}
				break;
#ifdef SAVEFORLATER
			case 'I':	/* List of track names and their start time dateTclock */
				Ctrl->I.active = true;
				Ctrl->I.file = strdup (opt->arg);
				break;
#endif
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
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !Ctrl->T.active || !Ctrl->T.TAG, "Syntax error: -T must be used to set the TAG\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.mode < 0, "Syntax error -E: Choose among c, d, g, h, s and t\n");
#ifdef SAVEFORLATER
	n_errors += GMT_check_condition (GMT, Ctrl->E.mode == F_IS_DRIFT_T && !Ctrl->I.file, "Syntax error -Ed: Solution requires -I<tracklist>\n");
#endif

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#ifdef SAVEFORLATER
int x2sys_read_namedatelist (struct GMT_CTRL *GMT, char *file, char ***list, double **start, int *nf)
{
	/* Reads a list with track names and their origin times (needed for -Et) */
	size_t n_alloc = GMT_CHUNK;
	int n = 0;
	char **p, line[GMT_BUFSIZ] = {""}, name[GMT_LEN64] = {""}, date[GMT_LEN64] = {""};
	double *T = NULL;
	FILE *fp = NULL;

	if ((fp = x2sys_fopen (GMT, file, "r")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Cannot find track list file %s in either current or X2SYS_HOME directories\n", line);
		return (GMT_GRDIO_FILE_NOT_FOUND);
	}
	
	p = GMT_memory (GMT, NULL, n_alloc, char *);
	T = GMT_memory (GMT, NULL, n_alloc, double);

	while (fgets (line, GMT_BUFSIZ, fp)) {
		GMT_chop (line);	/* Remove trailing CR or LF */
		sscanf (line, "%s %s", name, date);
		p[n] = strdup (name);
		if (date[strlen(date)-1] != 'T') strcat (date, "T");
		if (GMT_scanf (GMT, date, GMT_IS_ABSTIME, &T[n]) == GMT_IS_NAN) T[n] = GMT->session.d_NaN;
		
		n++;
		if (n == n_alloc) {
			n_alloc <<= 1;
			p = GMT_memory (GMT, p, n_alloc, char *);
			T = GMT_memory (GMT, T, n_alloc, double);
		}
	}
	fclose (fp);

	p = GMT_memory (GMT, p, n, char *);
	T = GMT_memory (GMT, T, n, double);

	*list = p;
	*start = T;
	*nf = n;

	return (X2SYS_NOERROR);
}
#endif

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_x2sys_solve_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

uint64_t next_unused_track (uint64_t *cluster, uint64_t n)
{
	uint64_t k;
	for (k = 0; k < n; k++) if (cluster[k] == 0) return (k);
	return (n);
}

int GMT_x2sys_solve (void *V_API, int mode, void *args)
{
	char **trk_list = NULL;
	char trk[2][GMT_LEN64], t_txt[2][GMT_LEN64], z_txt[GMT_LEN64] = {""}, w_txt[GMT_LEN64] = {""}, line[GMT_BUFSIZ] = {""};
	bool grow_list = false, normalize = false, active_col[N_COE_PARS];
	int *ID[2] = {NULL, NULL}, ks, t, error = 0, ierror, expect;
	uint64_t n_par = 0, n, m, n_tracks = 0, n_active, n_expected_fields, n_constraints = 0;
	uint64_t i, p, j, k, r, s, row_off, row, n_COE = 0, *R = NULL, *col_off = NULL, *cluster = NULL;
	size_t n_alloc = GMT_INITIAL_MEM_ROW_ALLOC, n_alloc_t = GMT_CHUNK;
	double *N = NULL, *a = NULL, *b = NULL, *data[N_COE_PARS], sgn, old_mean, new_mean, sw2, C_i, C_j;
	double old_stdev, new_stdev, e_k, min_extent, max_extent, range = 0.0, Sw, Sx, Sxx, var[N_BASIS];
#ifdef SAVEFORLATER
	double *start = NULL;
#endif
	struct X2SYS_INFO *S = NULL;
	struct X2SYS_BIX B;
	FILE *fp = NULL;
	double (*basis[N_BASIS]) (double **, unsigned int, uint64_t);	/* Pointers to selected basis functions */
	
	struct X2SYS_SOLVE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_x2sys_solve_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_x2sys_solve_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_x2sys_solve_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_x2sys_solve_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_solve_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the x2sys_solve main code ----------------------------*/

#ifdef SAVEFORLATER
	if (Ctrl->I.file && x2sys_read_namedatelist (GMT, Ctrl->I.file, &trk_list, &start, &n_tracks) == X2SYS_NOERROR) {
		GMT_Report (API, GMT_MSG_NORMAL, "ERROR -I: Problems reading %s\n", Ctrl->I.file);
		Return (EXIT_FAILURE);	
	}
#endif
	
	/* Initialize system via the tag */
	
	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &S, &B, &GMT->current.io), Ctrl->T.TAG);

	/* Verify that the chosen column is known to the system */

	if (Ctrl->C.col) x2sys_err_fail (GMT, x2sys_pick_fields (GMT, Ctrl->C.col, S), "-C");
	if (S->n_out_columns != 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: -C must specify a single column name\n");
		Return (EXIT_FAILURE);
	}

	GMT_memset (active_col, N_COE_PARS, bool); /* Initialize array */

	active_col[COL_COE] = true;	/* Always used */
	switch (Ctrl->E.mode) {	/* Set up pointers to basis functions and assign constants */
		case F_IS_CONSTANT:
			n_par = 1;
			basis[0] = &basis_constant;
			break;
		case F_IS_DRIFT_T:
			active_col[COL_T1] = active_col[COL_T2] = true;
			n_par = 2;
			basis[0] = &basis_constant;
			basis[1] = &basis_tdrift;
			break;
		case F_IS_DRIFT_D:
			active_col[COL_D1] = active_col[COL_D2] = true;
			n_par = 2;
			basis[0] = &basis_constant;
			basis[1] = &basis_ddrift;
			break;
		case F_IS_GRAV1930:
			active_col[COL_YY] = true;
			n_par = 2;
			basis[0] = &basis_constant;
			basis[1] = &basis_siny2;
			break;
		case F_IS_HEADING:
			active_col[COL_H1] = active_col[COL_H2] = true;
			n_par = 5;
			basis[0] = &basis_constant;
			basis[1] = &basis_cosh;
			basis[2] = &basis_cos2h;
			basis[3] = &basis_sinh;
			basis[4] = &basis_sin2h;
			break;
		case F_IS_SCALE:
			active_col[COL_Z1] = active_col[COL_Z2] = true;
			n_par = 1;
			basis[0] = &basis_z;
			break;
	}
	
	/* Allocate memory for COE data */
	
	GMT_memset (data, N_COE_PARS, double *);
	for (i = n_active = 0; i < N_COE_PARS; i++) {
		if (active_col[i]) {
			data[i] = GMT_memory (GMT, NULL, n_alloc, double);
			n_active++;
		}
	}
	if (!data[COL_WW]) data[COL_WW] = GMT_memory (GMT, NULL, n_alloc, double);
	for (i = 0; i < 2; i++) ID[i] = GMT_memory (GMT, NULL, n_alloc, int);
	
	/* Read the crossover info */

	fp = GMT->session.std[GMT_IN];
	if (Ctrl->In.file && (fp = GMT_fopen (GMT, Ctrl->In.file, GMT->current.io.r_mode)) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: Cannot open file %s\n", Ctrl->In.file);
		Return (EXIT_FAILURE);	
	}
	
	if (n_tracks == 0 && !GMT->common.b.active[GMT_IN])	{	/* Create track list on the go */
		grow_list = true;
		trk_list = GMT_memory (GMT, NULL, n_alloc_t, char *);
	}
	
	n_expected_fields = 2 + n_active + Ctrl->W.active;
	expect = (int)n_expected_fields;

	n_COE = 0;
	if (GMT->common.b.active[GMT_IN]) {	/* Binary input */
		/* Here, first two cols have track IDs and we do not write track names */
		int min_ID, max_ID, n_fields;
		uint64_t n_tracks2, w_col = n_expected_fields - 1;
		double *in = NULL;
		char *check = NULL;
		
		if (GMT->common.b.ncol[GMT_IN] < (uint64_t)expect) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: Binary file has %" PRIu64 " columns but %d is required\n", GMT->common.b.ncol[GMT_IN], expect);
			Return (EXIT_FAILURE);	
		}
		min_ID = INT_MAX;	max_ID = -INT_MAX;
		while ((in = GMT->current.io.input (GMT, fp, &n_expected_fields, &n_fields)) && !(GMT->current.io.status & GMT_IO_EOF)) {	/* Not yet EOF */
			for (i = 0; i < 2; i++) {	/* Get IDs and keept track of min/max values */
				ID[i][n_COE] = irint (in[i]);
				if (ID[i][n_COE] < min_ID) min_ID = ID[i][n_COE];
				if (ID[i][n_COE] > max_ID) max_ID = ID[i][n_COE];
			}
			switch (Ctrl->E.mode) {	/* Handle input differently depending on what is expected */
				case F_IS_CONSTANT:
					data[COL_COE][n_COE] = in[2];
					break;
				case F_IS_DRIFT_T:
					data[COL_T1][n_COE] = in[2];
					data[COL_T2][n_COE] = in[3];
					data[COL_COE][n_COE] = in[4];
					break;
				case F_IS_DRIFT_D:
					data[COL_D1][n_COE] = in[2];
					data[COL_D2][n_COE] = in[3];
					data[COL_COE][n_COE] = in[4];
					break;
				case F_IS_GRAV1930:
					data[COL_YY][n_COE] = in[2];
					data[COL_COE][n_COE] = in[3];
					break;
				case F_IS_HEADING:
					data[COL_H1][n_COE] = in[2];
					data[COL_H2][n_COE] = in[3];
					data[COL_COE][n_COE] = in[4];
					break;
				case F_IS_SCALE:
					data[COL_Z1][n_COE] = in[2];
					data[COL_Z2][n_COE] = in[3];
					data[COL_COE][n_COE] = in[2] - in[3];
					break;
			}
			data[COL_WW][n_COE] = (Ctrl->W.active) ? in[w_col] : 1.0;	/* Weight */
			if (GMT_is_dnan (data[COL_COE][n_COE])) {
				GMT_Report (API, GMT_MSG_VERBOSE, "Warning: COE == NaN skipped during reading\n");
				continue;
			}
			n_COE++;
			if (n_COE == n_alloc) {
				n_alloc <<= 1;
				for (i = 0; i < N_COE_PARS; i++) if (active_col[i]) data[i] = GMT_memory (GMT, data[i], n_alloc, double);
				data[COL_WW] = GMT_memory (GMT, data[COL_WW], n_alloc, double);
			}
		}
		/* Check that IDs are all contained within 0 <= ID < n_tracks and that there are no gaps */
		n_tracks2 = max_ID - min_ID + 1;
		if (n_tracks && n_tracks2 != n_tracks) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: The ID numbers in the binary file %s are not compatible with the <trklist> length\n", Ctrl->In.file);
			error = true;	
		}
		else {	/* Either no tracks read before or the two numbers did match properly */
			/* Look for ID gaps */
			n_tracks = n_tracks2;
			check = GMT_memory (GMT, NULL, n_tracks, char);
			for (k = 0; k < n_COE; k++) for (i = 0; i < 2; i++) check[ID[i][k]] = true;	/* Flag those tracks that have been mentioned */
			for (k = 0; k < n_tracks && check[k]; k++);	/* Loop until end of until first non-used ID */
			GMT_free (GMT, check);
			if (k < n_tracks) {
				GMT_Report (API, GMT_MSG_NORMAL, "Error: The ID numbers in the binary file %s do not completely cover the range 0 <= ID < n_tracks!\n", Ctrl->In.file);
				error = true;
			}
		}
		if (error) {	/* Delayed the cleanup until here */
			for (i = 0; i < N_COE_PARS; i++) if (active_col[i]) GMT_free (GMT, data[i]);
			GMT_free (GMT, data[COL_WW]);
			for (i = 0; i < 2; i++) GMT_free (GMT, ID[i]);
			Return (EXIT_FAILURE);
		}
	}
	else {	/* Ascii input with track names */
		char file_TAG[GMT_LEN64] = {""}, file_column[GMT_LEN64] = {""}, unused1[GMT_LEN64] = {""}, unused2[GMT_LEN64] = {""};
		if (!GMT_fgets (GMT, line, GMT_BUFSIZ, fp)) {	/* Read first line with TAG and column */
			GMT_Report (API, GMT_MSG_NORMAL, "Read error in 1st line of track file\n");
			Return (EXIT_FAILURE);
		}
		sscanf (&line[7], "%s %s", file_TAG, file_column);
		if (strcmp (Ctrl->T.TAG, file_TAG) && strcmp (Ctrl->C.col, file_column)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: The TAG and column info in the ASCII file %s are not compatible with the -C -T options\n", Ctrl->In.file);
			Return (EXIT_FAILURE);	
		}
		while (GMT_fgets (GMT, line, GMT_BUFSIZ, fp)) {    /* Not yet EOF */
			if (line[0] == '#') continue;	/* Skip other comments */
			switch (Ctrl->E.mode) {	/* Handle input differently depending on what is expected */
				case F_IS_CONSTANT:
					ks = sscanf (line, "%s %s %s %s %s %s", trk[0], trk[1], z_txt, w_txt, unused1, unused2);
					if (ks < expect) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: -Ec expected %d columns but found %d for crossover %" PRIu64 "\n", expect, ks, n_COE);
					if (GMT_scanf (GMT, z_txt, GMT_IS_FLOAT, &data[COL_COE][n_COE]) == GMT_IS_NAN) data[COL_COE][n_COE] = GMT->session.d_NaN;
					break;
				case F_IS_DRIFT_T:
					ks = sscanf (line, "%s %s %s %s %s %s", trk[0], trk[1], t_txt[0], t_txt[1], z_txt, w_txt);
					if (ks < expect) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: -Et expected %d columns but found %d for crossover %" PRIu64 "\n", expect, ks, n_COE);
					if (GMT_scanf (GMT, t_txt[0], GMT_IS_ABSTIME, &data[COL_T1][n_COE]) == GMT_IS_NAN) data[COL_T1][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, t_txt[1], GMT_IS_ABSTIME, &data[COL_T2][n_COE]) == GMT_IS_NAN) data[COL_T2][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, z_txt, GMT_IS_FLOAT, &data[COL_COE][n_COE]) == GMT_IS_NAN) data[COL_COE][n_COE]    = GMT->session.d_NaN;
					break;
				case F_IS_DRIFT_D:
					ks = sscanf (line, "%s %s %s %s %s %s", trk[0], trk[1], t_txt[0], t_txt[1], z_txt, w_txt);
					if (ks < expect) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: -Ed expected %d columns but found %d for crossover %" PRIu64 "\n", expect, ks, n_COE);
					if (GMT_scanf (GMT, t_txt[0], GMT_IS_FLOAT, &data[COL_D1][n_COE]) == GMT_IS_NAN) data[COL_D1][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, t_txt[1], GMT_IS_FLOAT, &data[COL_D2][n_COE]) == GMT_IS_NAN) data[COL_D2][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, z_txt, GMT_IS_FLOAT, &data[COL_COE][n_COE]) == GMT_IS_NAN) data[COL_COE][n_COE]  = GMT->session.d_NaN;
					break;
				case F_IS_GRAV1930:
					ks = sscanf (line, "%s %s %s %s %s %s", trk[0], trk[1], t_txt[0], z_txt, w_txt, unused1);
					if (ks < expect) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: -Eg expected %d columns but found %d for crossover %" PRIu64 "\n", expect, ks, n_COE);
					if (GMT_scanf (GMT, t_txt[0], GMT_IS_LAT, &data[COL_YY][n_COE]) == GMT_IS_NAN) data[COL_YY][n_COE]  = GMT->session.d_NaN;
					if (GMT_scanf (GMT, z_txt, GMT_IS_FLOAT, &data[COL_COE][n_COE]) == GMT_IS_NAN) data[COL_COE][n_COE] = GMT->session.d_NaN;
					break;
				case F_IS_HEADING:
					ks = sscanf (line, "%s %s %s %s %s %s", trk[0], trk[1], t_txt[0], t_txt[1], z_txt, w_txt);
					if (ks < expect) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: -Eh expected %d columns but found %d for crossover %" PRIu64 "\n", expect, ks, n_COE);
					if (GMT_scanf (GMT, t_txt[0], GMT_IS_FLOAT, &data[COL_H1][n_COE]) == GMT_IS_NAN) data[COL_H1][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, t_txt[1], GMT_IS_FLOAT, &data[COL_H2][n_COE]) == GMT_IS_NAN) data[COL_H2][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, z_txt, GMT_IS_FLOAT, &data[COL_COE][n_COE]) == GMT_IS_NAN) data[COL_COE][n_COE]  = GMT->session.d_NaN;
					break;
				case F_IS_SCALE:
					ks = sscanf (line, "%s %s %s %s %s %s", trk[0], trk[1], t_txt[0], t_txt[1], w_txt, unused1);
					if (ks < expect) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: -Es expected %d columns but found %d for crossover %" PRIu64 "\n", expect, ks, n_COE);
					if (GMT_scanf (GMT, t_txt[0], GMT_IS_FLOAT, &data[COL_Z1][n_COE]) == GMT_IS_NAN) data[COL_Z1][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, t_txt[1], GMT_IS_FLOAT, &data[COL_Z2][n_COE]) == GMT_IS_NAN) data[COL_Z2][n_COE] = GMT->session.d_NaN;
					break;
			}
			if (Ctrl->W.active) {
				if (GMT_scanf (GMT, w_txt, GMT_IS_FLOAT, &data[COL_WW][n_COE]) == GMT_IS_NAN) data[COL_WW][n_COE] = GMT->session.d_NaN;
			}
			else
				data[COL_WW][n_COE] = 1.0;
			if (GMT_is_dnan (data[COL_COE][n_COE])) {
				GMT_Report (API, GMT_MSG_VERBOSE, "Warning: COE == NaN skipped during reading\n");
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
							trk_list = GMT_memory (GMT, trk_list, n_alloc_t, char *);
						}
					}
#ifdef SAVEFORLATER
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Error: Track %s not in specified list of tracks [%s]\n", trk[i], Ctrl->I.file);
						Return (EXIT_FAILURE);					
					}
#endif
				}
			}
		
			n_COE++;
			if (n_COE == n_alloc) {
				n_alloc <<= 1;
				for (i = 0; i < N_COE_PARS; i++) if (active_col[i]) data[i] = GMT_memory (GMT, data[i], n_alloc, double);
				data[COL_WW] = GMT_memory (GMT, data[COL_WW], n_alloc, double);
				for (i = 0; i < 2; i++) ID[i] = GMT_memory (GMT, ID[i], n_alloc, int);
			}
		}
	}
	GMT_fclose (GMT, fp);
	GMT_Report (API, GMT_MSG_VERBOSE, "Found %d COE records\n", n_COE);
	for (i = 0; i < N_COE_PARS; i++) if (active_col[i]) data[i] = GMT_memory (GMT, data[i], n_COE, double);
	data[COL_WW] = GMT_memory (GMT, data[COL_WW], n_COE, double);
	
	normalize = (Ctrl->E.mode == F_IS_DRIFT_T || Ctrl->E.mode == F_IS_DRIFT_D);	/* Only when the linear drift term is in effect */
	if (normalize) {	/* For numerical stability, normalize distances or times to fall in 0-1 range */
		min_extent = DBL_MAX;	max_extent = -DBL_MAX;
		j = (Ctrl->E.mode == F_IS_DRIFT_T) ? COL_T1 : COL_D1;	/* Which variable we are working on */
		for (k = 0; k < n_COE; k++) {
			for (i = 0; i < 2; i++) {
#ifdef SAVEFORLATER
				if (Ctrl->E.mode == F_IS_DRIFT_T) data[COL_T1+i][k] -= start[ID[i][k]];	/* Remove origin time for track */
#endif
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
	R = GMT_memory (GMT, NULL, n_tracks, uint64_t);		/* Number of parameter for each track p, so that 1 <= R(p) < n_par */
	col_off = GMT_memory (GMT, NULL, n_tracks, uint64_t);	/* Since R(p) may vary, need cumulative sum of R(p) to get correct column */
	for (k = 0; k < n_COE; k++) {	/* Count number of crossovers per track */
		i = ID[0][k];	/* Get track # 1 ID */
		j = ID[1][k];	/* Get track # 2 ID */
		R[i]++;		/* Increase COE count for track i */
		R[j]++;		/* Increase COE count for track j */
	}
	for (p = n = 0; p < n_tracks; p++) {	/* For each track, determine R[track], total number of parameters, and the column offsets */
		(GMT->common.b.active[GMT_IN]) ? sprintf (trk[0], "%" PRIu64, p) : sprintf (trk[0], "%s", trk_list[p]);
		if (R[p] < n_par)	/* Came up short */
			GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Track %s only has %" PRIu64 " crossings so can only solve for %" PRIu64 " of the %" PRIu64 " parameters\n", trk[0], R[p], R[p], n_par);
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
	
	if (Ctrl->E.mode != F_IS_SCALE) {	/* Constraint equations are needed when there is an offset in the model (all but the scaling model) */
		char *C = NULL, *used = NULL;
		uint64_t ij, g, n_in_cluster, *member = NULL;
		
		GMT_Report (API, GMT_MSG_VERBOSE, "Determine number of independent track clusters\n");
		C = GMT_memory (GMT, NULL, n_tracks*n_tracks, double);	/* For the connectivity matrix that shows which tracks cross */
		used = GMT_memory (GMT, NULL, n_tracks, double);	/* Array to remember which tracks we have dealt with so far */
		cluster = GMT_memory (GMT, NULL, n_tracks, uint64_t);	/* Array to remember which cluster a track belongs to */
		/* First build the adjacency matrix C */
		for (k = 0; k < n_COE; k++) {	/* Identify crossing pairs (the # of crossings doesn't matter) */
			i = ID[0][k];	/* Get track # 1 ID */
			j = ID[1][k];	/* Get track # 2 ID */
			ij = i * n_tracks + j;	/* Index in upper triangular matrix */
			if (C[ij] == 0) C[ij] = C[j*n_tracks+i] = 1;	/* Make C symmetric */
		}
		
		member = GMT_memory (GMT, NULL, n_tracks, uint64_t);	/* Temp array to keep all members of current cluster */
		n_constraints = 0;
		/* Below, cluster[] array will use counting from 1 to n_constraints, but later we reset to run from 0 instead */
		while ((p = next_unused_track (cluster, n_tracks)) < n_tracks) {	/* Still more clusters to form */
			n_constraints++;		/* Increment number of constraints, this will happen at least once */
			GMT_memset (member, n_tracks, uint64_t);	/* Cluster starts off with no members */
			member[0] = p;			/* This is the first member of this cluster */
			cluster[p] = n_constraints;	/* So we set this cluster number right away */
			n_in_cluster = 1;		/* It is the first member of this cluster so far */
			for (i = 0; i < n_tracks; i++) {	/* Scan adjacency matrix for other direct members than crossed p */
				if (cluster[i] || C[p*n_tracks+i] == 0) continue;	/* Skip those already dealt with or if no crossing */
				member[n_in_cluster++] = i;	/* Add track #i to p's cluster */
				cluster[i] = cluster[p];	/* And set it to same cluster id as p */
			}
			g = 1;	/* Start at 2nd entry since 1st (p) has already been given its direct cluster numbers */
			while (g < n_in_cluster) {	/* Since n_in_cluster may grow we must check via a test at the top */
				k = member[g];	/* We will examine the k't row in the adjacency matrix for tracks that crossed k */
				for (i = 0; i < n_tracks; i++) {
					if (cluster[i] || C[k*n_tracks+i] == 0) continue;	/* Skip those already dealt with or if no crossing */
					member[n_in_cluster++] = i;	/* Add track #i to p's cluster */
					cluster[i] = cluster[p];	/* And set it to same cluster */
				}
				g++;	/* Go to next cluster member */
			}
		}
		for (g = 0; g < n_tracks; g++) cluster[g]--;	/* Shift cluster numbers to start at zero instead of 1 */
		GMT_free (GMT, member);
		GMT_free (GMT, C);
		GMT_free (GMT, used);
		
		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
			if (n_constraints > 1) {	/* Report on the clusters */
				GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " tracks are grouped into %" PRIu64 " independent clusters:\n", n, n_constraints);
				for (k = 0; k < n_constraints; k++) {
					for (p = i = 0; p < n_tracks; p++) if (cluster[p] == k) i++;
					GMT_Report (API, GMT_MSG_VERBOSE, "===> Cluster # %" PRIu64 " [%" PRIu64 " tracks]:\n", k, i);
					for (p = 0; p < n_tracks; p++) if (cluster[p] == k) {
						(GMT->common.b.active[GMT_IN]) ? sprintf (trk[0], "%" PRIu64, p) : sprintf (trk[0], "%s", trk_list[p]);
						GMT_Report (API, GMT_MSG_VERBOSE, "\tTrack %s\n", trk[0]);
					}
				}
			}
			else
				GMT_Report (API, GMT_MSG_VERBOSE, "%" PRIu64 " tracks form a single connected cluster\n", n);
		}
	}
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Number of unknowns is %" PRIu64 " and number of constraints is %" PRIu64 "\n", n, n_constraints);

	/* Set up matrix and column vectors */

	m = n + n_constraints;	/* Need extra row(s)/column(s) to handle Lagrange's multiplier(s) for unknown absolute level(s) */
	/* Allocate arrays for N*x = b, the m x m linear system */
	N = GMT_memory (GMT, NULL, m*m, double);
	b = GMT_memory (GMT, NULL, m, double);

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
		GMT_free (GMT, cluster);
	}

	GMT_Report (API, GMT_MSG_VERBOSE, "Matrix equation N * a = b: (N = %" PRIu64 " x %" PRIu64 ")\n", m, m);

#ifdef DEBUGX	
	for (i = 0; i < m; i++) {
		for (j = 0; j < m; j++) GMT_Message (API, GMT_TIME_NONE, "%8.2f\t", N[i*m+j]);
		GMT_Message (API, GMT_TIME_NONE, "\t%8.2f\n", b[i]);
	}
#endif

	/* Get LS solution */

	if ((ierror = GMT_gauss (GMT, N, b, (unsigned int)m, (unsigned int)m, true)))
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: Divisions by a small number (< DBL_EPSILON) occurred in GMT_gauss()!\n");

	GMT_free (GMT, N);
	a = b;	/* Convenience since the solution is called a in the notes and Wessel [2010] */

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
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Before correction, mean and st.dev.: %g %g \n", old_mean, old_stdev);
	GMT_Report (API, GMT_MSG_VERBOSE, "After correction, mean and st.dev. : %g %g\n", new_mean, new_stdev);
	
	/* Write correction table */
	
	for (p = 0; p < n_tracks; p++) {
		if (normalize) a[col_off[p]+1] /= range;	/* Unnormalize slopes */
		(GMT->common.b.active[GMT_IN]) ? printf ("%" PRIu64, p) : printf ("%s", trk_list[p]);
		printf ("\t%s", Ctrl->C.col);
		GMT_memset (var, N_BASIS, double);	/* Reset all parameters to zero */
		for (r = 0; r < R[p]; r++) var[r] = a[col_off[p]+r];	/* Just get the first R(p) items; the rest are set to 0 */
		switch (Ctrl->E.mode) {	/* Set up pointers to basis functions and assign constants */
			case F_IS_CONSTANT:
				printf ("\t%g\n", var[0]);
				break;
			case F_IS_DRIFT_T:
				printf ("\t%g\t%g*((time-T))\n", var[0], var[1]);
				break;
			case F_IS_DRIFT_D:
				printf ("\t%g\t%g*((dist))\n", var[0], var[1]);
				break;
			case F_IS_GRAV1930:
				printf ("\t%g\t%g*sin((lat))^2\n", var[0], var[1]);
				break;
			case F_IS_HEADING:
				printf ("\t%g\t%g*cos((azim))\t%g*cos(2*(azim))\t%g*sin((azim))\t%g*sin(2*(azim))\n", var[0], var[1], var[2], var[3], var[4]);
				break;
			case F_IS_SCALE:
				printf ("\t%g*((%s))\n", 1.0 - var[0], Ctrl->C.col);
				break;
		}
	}
	
	/* Free up memory */
	
	for (i = 0; i < N_COE_PARS; i++) if (active_col[i]) GMT_free (GMT, data[i]);
	if (data[COL_WW]) GMT_free (GMT, data[COL_WW]);
	for (i = 0; i < 2; i++) GMT_free (GMT, ID[i]);
	GMT_free (GMT, b);
	GMT_free (GMT, R);
	GMT_free (GMT, col_off);
	if (!GMT->common.b.active[GMT_IN]) x2sys_free_list (GMT, trk_list, n_tracks);
#ifdef SAVEFORLATER
	if (start) GMT_free (GMT, start);
#endif

	x2sys_end (GMT, S);

	Return (GMT_OK);
}
