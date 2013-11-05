/*-----------------------------------------------------------------
 *	$Id$
 *
 *      Copyright (c) 1999-2012 by P. Wessel
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License as published by
 *      the Free Software Foundation; version 2 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
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

#include "x2sys.h"

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
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct C {	/* -C */
		GMT_LONG active;
		char *col;
	} C;
	struct E {	/* -E */
		GMT_LONG active;
		GMT_LONG mode;
	} E;
#ifdef SAVEFORLATER
	struct I {	/* -I */
		GMT_LONG active;
		char *file;
	} I;
#endif
	struct T {	/* -T */
		GMT_LONG active;
		char *TAG;
	} T;
	struct W {	/* -W */
		GMT_LONG active;
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
 * The array active_col[N_COE_PARS] will be TRUE for those columns actually used
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

double basis_constant (double **P, int which, int col) {	/* Basis function f for a constant c*f = c*1 : == 1 */
	return (1.0);	/* which, col are not used here */
}

double basis_tdrift (double **P, int which, int col) {	/* Basis function f for a linear drift rate in time c*f = c*t : t */
	return (P[COL_T1+which][col]);	/* When this is called the time columns have been corrected for t0 (start of track) */
}

double basis_ddrift (double **P, int which, int col) {	/* Basis function f for a linear drift rate in dist c*f = c*d : d */
	return (P[COL_D1+which][col]);
}

double basis_cosh (double **P, int which, int col) {	/* Basis function f for a dependence on cos(h)  c*f = c*cos(h) : cos(h) */
	return (cosd(P[COL_H1+which][col]));
}

double basis_cos2h (double **P, int which, int col) {	/* Basis function f for a dependence on cos(2*h)  c*f = c*cos(2*h) : cos(2*h) */
	return (cosd(2.0*P[COL_H1+which][col]));
}

double basis_sinh (double **P, int which, int col) {	/* Basis function f for a dependence on sin(h)  c*f = c*sin(h) : sin(h) */
	return (sind(P[COL_H1+which][col]));
}

double basis_sin2h (double **P, int which, int col) {	/* Basis function f for a dependence on sin(2*h)  c*f = c*sin(2*h) : sin(2*h) */
	return (sind(2.0*P[COL_H1+which][col]));
}

double basis_siny2 (double **P, int which, int col) {	/* Basis function f for a dependence on sin^2(y)  c*f = c*sin^2(y) : sin^2(y) */
	return (pow (sind(P[COL_YY][col]), 2.0));	/* which not used since y is common to both tracks */
}

double basis_z (double **P, int which, int col) {	/* Basis function f for a dependence on value c*f = c*z : z */
	return (P[COL_Z1+which][col]);
}


void *New_x2sys_solve_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct X2SYS_SOLVE_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct X2SYS_SOLVE_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return (C);
}

void Free_x2sys_solve_Ctrl (struct GMT_CTRL *GMT, struct X2SYS_SOLVE_CTRL *C) {	/* Deallocate control structure */
	if (C->In.file) free (C->In.file);
	if (C->C.col) free (C->C.col);
#ifdef SAVEFORLATER
	if (C->I.file) free (C->I.file);
#endif
	if (C->T.TAG) free (C->T.TAG);
	GMT_free (GMT, C);
}

GMT_LONG GMT_x2sys_solve_usage (struct GMTAPI_CTRL *C, GMT_LONG level) {
	struct GMT_CTRL *GMT = C->GMT;
	
	GMT_message (GMT, "x2sys_solve %s - Determine least-squares systematic correction from crossovers\n\n", X2SYS_VERSION);
#ifdef SAVEFORLATER
	GMT_message (GMT, "usage: x2sys_solve -C<column> -E<flag> -T<TAG> [<coedata>] [-I<tracklist>] [%s] [-W] [%s]\n\n", GMT_V_OPT, GMT_bi_OPT);
#else
	GMT_message (GMT, "usage: x2sys_solve -C<column> -E<flag> -T<TAG> [<coedata>] [%s] [-W] [%s]\n\n", GMT_V_OPT, GMT_bi_OPT);
#endif

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-C Specify the column name to process (e.g., faa, mag).\n");
	GMT_message (GMT, "\t-E Equation to fit: specify <flag> as c (constant), d (drift over distance),\n");
	GMT_message (GMT, "\t     g (latitude), h (heading), s (scale with data), or t (drift over time) [c].\n");
	GMT_message (GMT, "\t-T <TAG> is the x2sys tag for the data set.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t<coedata> is the ASCII data output file from x2sys_list [or we read stdin].\n");
#ifdef SAVEFORLATER
	GMT_message (GMT, "\t-I List of tracks and their start date (required for -Et).\n");
#endif
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-W Weights are present in last column for weighted fit [no weights].\n");
	GMT_explain_options (GMT, "C");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_x2sys_solve_parse (struct GMTAPI_CTRL *C, struct X2SYS_SOLVE_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */

			case '<':	/* Input files */
				Ctrl->In.active = TRUE;
				if (n_files == 0) Ctrl->In.file = strdup (opt->arg);
				n_files++;
				break;

			/* Processes program-specific parameters */
			
			case 'C':	/* Needed to report correctly */
				Ctrl->C.active = TRUE;
				Ctrl->C.col = strdup (opt->arg);
				break;
			case 'E':	/* Which model to fit */
				Ctrl->E.active = TRUE;
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
				Ctrl->I.active = TRUE;
				Ctrl->I.file = strdup (opt->arg);
				break;
#endif
			case 'T':
				Ctrl->T.active = TRUE;
				Ctrl->T.TAG = strdup (opt->arg);
				break;
			case 'W':
				Ctrl->W.active = TRUE;
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
	int n_alloc = GMT_CHUNK, n = 0;
	char **p, line[GMT_BUFSIZ], name[GMT_TEXT_LEN64], date[GMT_TEXT_LEN64];
	double *T;
	FILE *fp;

	if ((fp = x2sys_fopen (GMT, file, "r")) == NULL) {
		GMT_report (GMT, GMT_MSG_FATAL, "Cannot find track list file %s in either current or X2SYS_HOME directories\n", line);
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

GMT_LONG GMT_x2sys_solve (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	char **trk_list = NULL;
	char trk[2][GMT_TEXT_LEN64], t_txt[2][GMT_TEXT_LEN64], z_txt[GMT_TEXT_LEN64], w_txt[GMT_TEXT_LEN64], line[GMT_BUFSIZ];
	GMT_LONG error = FALSE, grow_list = FALSE, normalize = FALSE, active_col[N_COE_PARS];
	int *ID[2] = {NULL, NULL};
	GMT_LONG n_par = 0, n, m, t, n_tracks = 0, n_active;
	GMT_LONG i, p, j, k, r, s, off, row, n_COE = 0, n_alloc = GMT_CHUNK, n_alloc_t = GMT_CHUNK, ierror;
	double *N = NULL, *a = NULL, *b = NULL, *data[N_COE_PARS], sgn, zero_test = 1.0e-08, old_mean, new_mean, sw2;
	double old_stdev, new_stdev, e_k, min_extent, max_extent, range = 0.0, Sw, Sx, Sxx;
#ifdef SAVEFORLATER
	double *start = NULL;
#endif
	struct X2SYS_INFO *S = NULL;
	struct X2SYS_BIX B;
	FILE *fp = NULL;
	PFD basis[N_BASIS];
	struct X2SYS_SOLVE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_x2sys_solve_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_x2sys_solve_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_x2sys_solve", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-Vb", ">", options)) Return (API->error);
	Ctrl = New_x2sys_solve_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_x2sys_solve_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the x2sys_solve main code ----------------------------*/

#ifdef SAVEFORLATER
	if (Ctrl->I.file && x2sys_read_namedatelist (GMT, Ctrl->I.file, &trk_list, &start, &n_tracks) == X2SYS_NOERROR) {
		GMT_report (GMT, GMT_MSG_FATAL, "ERROR -I: Problems reading %s\n", Ctrl->I.file);
		Return (EXIT_FAILURE);	
	}
#endif
	
	/* Initialize system via the tag */
	
	x2sys_err_fail (GMT, x2sys_set_system (GMT, Ctrl->T.TAG, &S, &B, &GMT->current.io), Ctrl->T.TAG);

	/* Verify that the chosen column is known to the system */
	
	if (Ctrl->C.col) x2sys_err_fail (GMT, x2sys_pick_fields (GMT, Ctrl->C.col, S), "-C");
	if (S->n_out_columns != 1) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: -C must specify a single column name\n");
		Return (EXIT_FAILURE);
	}

	active_col[COL_COE] = TRUE;	/* Always used */
	switch (Ctrl->E.mode) {	/* Set up pointers to basis functions and assign constants */
		case F_IS_CONSTANT:
			n_par = 1;
			basis[0] = basis_constant;
			break;
		case F_IS_DRIFT_T:
			active_col[COL_T1] = active_col[COL_T2] = TRUE;
			n_par = 2;
			basis[0] = basis_constant;
			basis[1] = basis_tdrift;
			break;
		case F_IS_DRIFT_D:
			active_col[COL_D1] = active_col[COL_D2] = TRUE;
			n_par = 2;
			basis[0] = basis_constant;
			basis[1] = basis_ddrift;
			break;
		case F_IS_GRAV1930:
			active_col[COL_YY] = TRUE;
			n_par = 2;
			basis[0] = basis_constant;
			basis[1] = basis_siny2;
			break;
		case F_IS_HEADING:
			active_col[COL_H1] = active_col[COL_H2] = TRUE;
			n_par = 5;
			basis[0] = basis_constant;
			basis[1] = basis_cosh;
			basis[2] = basis_cos2h;
			basis[3] = basis_sinh;
			basis[4] = basis_sin2h;
			break;
		case F_IS_SCALE:
			active_col[COL_Z1] = active_col[COL_Z2] = TRUE;
			n_par = 1;
			basis[0] = basis_z;
			break;
	}
	
	/* Allocate memory for COE data */
	
	for (i = n_active = 0; i < N_COE_PARS; i++) {
		data[i] = NULL;
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
		GMT_report (GMT, GMT_MSG_FATAL, "Error: Cannot open file %s\n", Ctrl->In.file);
		Return (EXIT_FAILURE);	
	}
	
	if (n_tracks == 0)	{	/* Create track list on the go */
		grow_list = TRUE;
		trk_list = GMT_memory (GMT, NULL, n_alloc_t, char *);
	}
	
	n_COE = 0;
	if (GMT->common.b.active[GMT_IN]) {	/* Binary input */
		/* Here, first two cols have track IDs and we do not write track names */
		int min_ID, max_ID;
		GMT_LONG n_fields, n_tracks2, n_expected_fields;
		double *in = NULL;
		char *check = NULL;
		
		min_ID = INT_MAX;	max_ID = -INT_MAX;
		n_expected_fields = n_active + Ctrl->W.active;
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
			data[COL_WW][n_COE] = (Ctrl->W.active) ? in[n_active] : 1.0;	/* Weight */
			if (GMT_is_dnan (data[COL_COE][n_COE])) {
				GMT_report (GMT, GMT_MSG_NORMAL, "Warning: COE == NaN skipped during reading\n");
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
			GMT_report (GMT, GMT_MSG_FATAL, "Error: The ID numbers in the binary file %s are not compatible with the <trklist> length\n", Ctrl->In.file);
			error = TRUE;	
		}
		else {	/* Either no tracks read before or the two numbers did match properly */
			/* Look for ID gaps */
			n_tracks = n_tracks2;
			check = GMT_memory (GMT, NULL, n_tracks, char);
			for (k = 0; k < n_COE; k++) for (i = 0; i < 2; i++) check[ID[i][k]] = TRUE;
			for (k = 0; k < n_tracks && check[k]; k++);
			GMT_free (GMT, check);
			if (k < n_tracks) {
				GMT_report (GMT, GMT_MSG_FATAL, "Error: The ID numbers in the binary file %s to not completely cover the range 0 <= ID < n_tracks!\n", Ctrl->In.file);
				error = TRUE;
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
		char file_TAG[GMT_TEXT_LEN64], file_column[GMT_TEXT_LEN64], *not_used = NULL;
		not_used = GMT_fgets (GMT, line, GMT_BUFSIZ, fp);	/* Read first line with TAG and column */
		sscanf (&line[7], "%s %s", file_TAG, file_column);
		if (strcmp (Ctrl->T.TAG, file_TAG) && strcmp (Ctrl->C.col, file_column)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: The TAG and column info in the ASCII file %s are not compatible with the -C -T options\n", Ctrl->In.file);
			Return (EXIT_FAILURE);	
		}
		while (GMT_fgets (GMT, line, GMT_BUFSIZ, fp)) {    /* Not yet EOF */
			if (line[0] == '#') continue;	/* Skip other comments */
			switch (Ctrl->E.mode) {	/* Handle input differently depending on what is expected */
				case F_IS_CONSTANT:
					sscanf (line, "%s %s %s %s", trk[0], trk[1], z_txt, w_txt);
					if (GMT_scanf (GMT, z_txt, GMT_IS_FLOAT, &data[COL_COE][n_COE]) == GMT_IS_NAN) data[COL_COE][n_COE] = GMT->session.d_NaN;
					break;
				case F_IS_DRIFT_T:
					sscanf (line, "%s %s %s %s %s %s", trk[0], trk[1], t_txt[0], t_txt[1], z_txt, w_txt);
					if (GMT_scanf (GMT, t_txt[0], GMT_IS_ABSTIME, &data[COL_T1][n_COE]) == GMT_IS_NAN) data[COL_T1][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, t_txt[1], GMT_IS_ABSTIME, &data[COL_T2][n_COE]) == GMT_IS_NAN) data[COL_T2][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, z_txt, GMT_IS_FLOAT, &data[COL_COE][n_COE]) == GMT_IS_NAN) data[COL_COE][n_COE] = GMT->session.d_NaN;
					break;
				case F_IS_DRIFT_D:
					sscanf (line, "%s %s %s %s %s %s", trk[0], trk[1], t_txt[0], t_txt[1], z_txt, w_txt);
					if (GMT_scanf (GMT, t_txt[0], GMT_IS_FLOAT, &data[COL_D1][n_COE]) == GMT_IS_NAN) data[COL_D1][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, t_txt[1], GMT_IS_FLOAT, &data[COL_D2][n_COE]) == GMT_IS_NAN) data[COL_D2][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, z_txt, GMT_IS_FLOAT, &data[COL_COE][n_COE]) == GMT_IS_NAN) data[COL_COE][n_COE] = GMT->session.d_NaN;
					break;
				case F_IS_GRAV1930:
					sscanf (line, "%s %s %s %s %s", trk[0], trk[1], t_txt[0], z_txt, w_txt);
					if (GMT_scanf (GMT, t_txt[0], GMT_IS_LAT, &data[COL_YY][n_COE]) == GMT_IS_NAN) data[COL_YY][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, z_txt, GMT_IS_FLOAT, &data[COL_COE][n_COE]) == GMT_IS_NAN) data[COL_COE][n_COE] = GMT->session.d_NaN;
					break;
				case F_IS_HEADING:
					sscanf (line, "%s %s %s %s %s %s", trk[0], trk[1], t_txt[0], t_txt[1], z_txt, w_txt);
					if (GMT_scanf (GMT, t_txt[0], GMT_IS_FLOAT, &data[COL_H1][n_COE]) == GMT_IS_NAN) data[COL_H1][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, t_txt[1], GMT_IS_FLOAT, &data[COL_H2][n_COE]) == GMT_IS_NAN) data[COL_H2][n_COE] = GMT->session.d_NaN;
					if (GMT_scanf (GMT, z_txt, GMT_IS_FLOAT, &data[COL_COE][n_COE]) == GMT_IS_NAN) data[COL_COE][n_COE] = GMT->session.d_NaN;
					break;
				case F_IS_SCALE:
					sscanf (line, "%s %s %s %s %s", trk[0], trk[1], t_txt[0], t_txt[1], w_txt);
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
				GMT_report (GMT, GMT_MSG_NORMAL, "Warning: COE == NaN skipped during reading\n");
				continue;
			}
			
			for (i = 0; i < 2; i++) {	/* Look up track IDs */
				ID[i][n_COE] = (int)x2sys_find_track (GMT, trk[i], trk_list, n_tracks);	/* Return track id # for this leg */
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
						GMT_report (GMT, GMT_MSG_FATAL, "Error: Track %s not in specified list of tracks [%s]\n", trk[i], Ctrl->I.file);
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
	GMT_report (GMT, GMT_MSG_NORMAL, "Found %ld COE records\n", n_COE);
	for (i = 0; i < N_COE_PARS; i++) if (active_col[i]) data[i] = GMT_memory (GMT, data[i], n_COE, double);
	data[COL_WW] = GMT_memory (GMT, data[COL_WW], n_COE, double);
	
	normalize = (Ctrl->E.mode == F_IS_DRIFT_T || Ctrl->E.mode == F_IS_DRIFT_D);
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
		for (k = 0; k < n_COE; k++) for (i = 0; i < 2; i++) data[j+i][k] /= range;
	}
	
	/* Estimate old weighted mean and std.dev */
	
	for (k = 0, Sw = Sx = Sxx = 0.0; k < n_COE; k++) {	/* For each crossover */
		Sw += data[COL_WW][k];
		Sx += (data[COL_WW][k] * data[COL_COE][k]);
		Sxx += (data[COL_WW][k] * data[COL_COE][k] * data[COL_COE][k]);
	}
	old_mean = Sx / Sw;
	old_stdev = sqrt ((n_COE * Sxx - Sx * Sx) / (Sw*Sw*(n_COE - 1.0)/n_COE));
	
	/* Set up matrix and column vectors */
	
	n = n_tracks * n_par;	/* Total number of unknowns */
	m = (Ctrl->E.mode == F_IS_SCALE) ? n : n + 1;	/* Need extra row/column to handle Lagrange's multiplier for unknown absolute level */
	N = GMT_memory (GMT, NULL, m*m, double);
	b = GMT_memory (GMT, NULL, m, double);

	/* Build A'A and A'b ==> N*x = b normal equation matrices directly since A may be too big to do A'A by multiplication.
	 * For all corrections that involve a constant shift we must add the constraint that such shifts sum to zero; this
	 * is implemented by adding an extra row/column with the appropriate 0s and 1s and a Lagrange multiplier. */
	
	for (p = row = 0; p < n_tracks; p++) {	/* For each track */
		for (s = 0; s < n_par; s++, row++) {	/* For each track's unknown parameter  */
			for (k = 0; k < n_COE; k++) {	/* For each crossover */
				i = ID[0][k];	/* Get track # 1 ID */
				j = ID[1][k];	/* Get track # 2 ID */
				if (i == p) {
					sgn = +1.0;	t = 0;
				} else if (p == j) {
					sgn = -1.0;	t = 1;
				} else continue;
				sw2 = sgn * data[COL_WW][k] * data[COL_WW][k];
				for (r = 0, off = m * row; r < n_par; r++) {	/* For each track's parameter in f(p)  */
					N[off+i*n_par+r] += sw2 * (basis[r](data,0,k) * basis[s](data,t,k));
					N[off+j*n_par+r] -= sw2 * (basis[r](data,1,k) * basis[s](data,t,k));
				}
				b[row] += sw2 * (data[COL_COE][k] * basis[s](data,t,k));
			}
			if (Ctrl->E.mode != F_IS_SCALE && s == 0) N[m*row+m-1] = 1.0;	/* Augmented column entry for Lagrange multiplier */
		}
	}
	if (Ctrl->E.mode != F_IS_SCALE) {	/* Augmented row for Lagrange multiplier for constants */
		for (i = 0, off = m*n; i < n; i += n_par) N[off+i] = 1.0;
	}
	
#ifdef DEBUGX	
	GMT_message (GMT, "Matrix equation N * a = b: (N = %ld)\n", m);
	for (i = 0; i < m; i++) {
		for (j = 0; j < m; j++) GMT_message (GMT, "%8.2f\t", N[i*m+j]);
		GMT_message (GMT, "\t%8.2f\n", b[i]);
	}
#endif

	/* Get LS solution */

	GMT_gauss (GMT, N, b, m, m, zero_test, &ierror, 1);
	GMT_free (GMT, N);
	a = b;	/* Convenience since the solution is called a in the notes */

	/* Estimate new st.dev. */
	
	for (k = 0, Sw = Sx = Sxx = 0.0; k < n_COE; k++) {	/* For each crossover */
		i = ID[0][k];	/* Get track # 1 ID */
		j = ID[1][k];	/* Get track # 2 ID */
		e_k = data[COL_COE][k];
		for (r = 0; r < n_par; r++) {	/* Correct crossover for track adjustments  */
			e_k += a[j*n_par+r]*basis[r](data,1,k) - a[i*n_par+r]*basis[r](data,0,k);
		}
		Sw += data[COL_WW][k];
		Sx += (data[COL_WW][k] * e_k);
		Sxx += (data[COL_WW][k] * e_k * e_k);
#ifdef DEBUGX	
		GMT_message (GMT, "COE # %ld: Was %g Is %g\n", k, data[COL_COE][k], e_k);
#endif
	}
	new_mean = Sx / Sw;
	new_stdev = sqrt ((n_COE * Sxx - Sx * Sx) / (Sw*Sw*(n_COE - 1.0)/n_COE));
	
	GMT_report (GMT, GMT_MSG_NORMAL, "Old mean and st.dev.: %g %g New mean and st.dev.: %g %g\n", old_mean, old_stdev, new_mean, new_stdev);
	
	/* Write correction table */
	
	for (p = 0; p < n_tracks; p++) {
		if (normalize) a[p*n_par+1] /= range;	/* Unnormalize slopes */
		(GMT->common.b.active[GMT_IN]) ? printf ("%ld", p) : printf ("%s", trk_list[p]);
		printf ("\t%s", Ctrl->C.col);
		switch (Ctrl->E.mode) {	/* Set up pointers to basis functions and assign constants */
			case F_IS_CONSTANT:
				printf ("\t%g\n", a[p]);
				break;
			case F_IS_DRIFT_T:
				printf ("\t%g\t%g*((time-T))\n", a[p*n_par], a[p*n_par+1]);
				break;
			case F_IS_DRIFT_D:
				printf ("\t%g\t%g*((dist))\n", a[p*n_par], a[p*n_par+1]);
				break;
			case F_IS_GRAV1930:
				printf ("\t%g\t%g*sin((lat))^2\n", a[p*n_par], a[p*n_par+1]);
				break;
			case F_IS_HEADING:
				printf ("\t%g\t%g*cos((azim))\t%g*cos(2*(azim))\t%g*sin((azim))\t%g*sin(2*(azim))\n", a[p*n_par], a[p*n_par+1], a[p*n_par+2], a[p*n_par+3], a[p*n_par+4]);
				break;
			case F_IS_SCALE:
				printf ("\t%g*((%s))\n", 1.0 - a[p], Ctrl->C.col);
				break;
		}
	}
	
	/* Free up memory */
	
	for (i = 0; i < N_COE_PARS; i++) if (active_col[i]) GMT_free (GMT, data[i]);
	GMT_free (GMT, data[COL_WW]);
	for (i = 0; i < 2; i++) GMT_free (GMT, ID[i]);
	GMT_free (GMT, b);
	x2sys_free_list (GMT, trk_list, n_tracks);
#ifdef SAVEFORLATER
	if (start) GMT_free (GMT, start);
#endif

	x2sys_end (GMT, S);

	Return (GMT_OK);
}
