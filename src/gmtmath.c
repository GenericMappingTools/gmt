/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * API functions to support the gmtmath application.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: gmtmath.c is a reverse polish calculator that operates
 * on table files (and constants) and perform basic mathematical operations
 * on them like add, multiply, etc.
 * Some operators only work on one operand (e.g., log, exp)
 *
 */

#include "gmt.h"

EXTERN_MSC GMT_LONG gmt_load_macros (struct GMT_CTRL *GMT, char *mtype, struct MATH_MACRO **M);
EXTERN_MSC GMT_LONG gmt_find_macro (char *arg, GMT_LONG n_macros, struct MATH_MACRO *M);
EXTERN_MSC void gmt_free_macros (struct GMT_CTRL *GMT, GMT_LONG n_macros, struct MATH_MACRO **M);
	
#define GMTMATH_ARG_IS_OPERATOR	 0
#define GMTMATH_ARG_IS_FILE	-1
#define GMTMATH_ARG_IS_NUMBER	-2
#define GMTMATH_ARG_IS_PI	-3
#define GMTMATH_ARG_IS_E	-4
#define GMTMATH_ARG_IS_EULER	-5
#define GMTMATH_ARG_IS_TMIN	-6
#define GMTMATH_ARG_IS_TMAX	-7
#define GMTMATH_ARG_IS_TINC	-8
#define GMTMATH_ARG_IS_N	-9
#define GMTMATH_ARG_IS_T_MATRIX	-10
#define GMTMATH_ARG_IS_t_MATRIX	-11
#define GMTMATH_ARG_IS_BAD	-99

#define GMTMATH_STACK_SIZE	100

#define COL_T	0	/* These are the first and 2nd columns in the Time structure */
#define COL_TN	1

struct GMTMATH_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct Out {	/* = <filename> */
		GMT_LONG active;
		char *file;
	} Out;
	struct A {	/* -A<t_f(t).d> */
		GMT_LONG active;
		char *file;
	} A;
	struct C {	/* -C<cols> */
		GMT_LONG active;
		GMT_LONG *cols;
	} C;
	struct I {	/* -I */
		GMT_LONG active;
	} I;
	struct L {	/* -L */
		GMT_LONG active;
	} L;
	struct N {	/* -N<n_col>/<t_col> */
		GMT_LONG active;
		GMT_LONG ncol, tcol;
	} N;
	struct Q {	/* -Q */
		GMT_LONG active;
	} Q;
	struct S {	/* -S[f|l] */
		GMT_LONG active;
		GMT_LONG mode;
	} S;
	struct T {	/* -T[<tmin/tmax/t_inc>] | -T<file> */
		GMT_LONG active;
		GMT_LONG notime;
		GMT_LONG mode;	/* = 1 if t_inc really is number of desired nodes */
		double min, max, inc;
		char *file;
	} T;
};

struct GMTMATH_INFO {
	GMT_LONG irregular;	/* TRUE if t_inc varies */
	GMT_LONG roots_found;	/* TRUE if roots have been solved for */
	GMT_LONG local;		/* Per segment operation (TRUE) or global operation (FALSE) */
	GMT_LONG n_roots;	/* Number of roots found */
	GMT_LONG r_col;		/* The column used to find roots */
	GMT_LONG n_col;		/* Number of columns */
	double t_min, t_max, t_inc;
	struct GMT_TABLE *T;	/* Table with all time information */
};

void *New_gmtmath_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTMATH_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct GMTMATH_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->C.cols = GMT_memory (GMT, NULL, GMT_MAX_COLUMNS, GMT_LONG);
	C->N.ncol = 2;

	return (C);
}

void Free_gmtmath_Ctrl (struct GMT_CTRL *GMT, struct GMTMATH_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free (C->Out.file);
	if (C->A.file) free (C->A.file);
	GMT_free (GMT, C->C.cols);
	if (C->T.file) free (C->T.file);
	GMT_free (GMT, C);
}

void new_table (struct GMT_CTRL *GMT, double ***s, GMT_LONG n_col, GMT_LONG n)
{	/* First time it is called for a table the the s pointer is NULL */
	GMT_LONG j;
	double **p = GMT_memory (GMT, *s, n_col, double *);
	for (j = 0; j < n_col; j++) p[j] = GMT_memory (GMT, p[j], n, double);
	*s = p;
}

void decode_columns (struct GMT_CTRL *GMT, char *txt, GMT_LONG *skip, GMT_LONG n_col, GMT_LONG t_col)
{
	GMT_LONG i, start, stop, pos, col;
	char p[GMT_BUFSIZ];

	/* decode_columns is used to handle the parsing of -C<cols>.  */
	
	if (!txt[0]) {	/* Reset to default */
		for (i = 0; i < n_col; i++) skip[i] = FALSE;
		skip[t_col] = TRUE;
	}
	else if (txt[0] == 'r' && txt[1] == '\0') {	/* Reverse all settings */
		for (i = 0; i < n_col; i++) skip[i] = !skip[i];
	}
	else if (txt[0] == 'a') {	/* Select all columns */
		for (i = 0; i < n_col; i++) skip[i] = FALSE;
	}
	else {	/* Set the selected columns */
		for (i = 0; i < n_col; i++) skip[i] = TRUE;
		pos = col = 0;
		while ((GMT_strtok (GMT, txt, ",", &pos, p))) {
			if (strchr (p, '-'))
				sscanf (p, "%" GMT_LL "d-%" GMT_LL "d", &start, &stop);
			else {
				sscanf (p, "%" GMT_LL "d", &start);
				stop = start;
			}
			stop = MIN (stop, n_col-1);
			for (i = start; i <= stop; i++) skip[i] = FALSE;
		}
	}
}

/* ---------------------- start convenience functions --------------------- */

GMT_LONG solve_LSQFIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *D, GMT_LONG n_col, GMT_LONG skip[], char *file)
{
	/* Consider the current table the augmented matrix [A | b], making up the linear system Ax = b.
	 * We will set up the normal equations, solve for x, and output the solution before quitting.
	 * This function is special since it operates across columns and returns n_col scalars.
	 * We try to solve this positive definite & symmetric matrix with Cholsky methods; if that fails
	 * we do a full SVD decomposition and set small eigenvalues to zero, yielding an approximate solution.
	 */

	GMT_LONG i, j, k0, i2, j2, rhs, k, n, ier, seg, row;
	double cond, *N = NULL, *B = NULL, *d = NULL, *x = NULL, *b = NULL, *z = NULL, *v = NULL, *lambda = NULL;
	FILE *fp = NULL;
	struct GMT_TABLE *T = D->table[0];

	for (i = n = 0; i < n_col; i++) if (!skip[i]) n++;	/* Need to find how many active columns we have */
	if (n < 2) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error, LSQFIT requires at least 2 active columns!\n");
		return (EXIT_FAILURE);
	}
	rhs = n_col - 1;
	while (skip[rhs] && rhs > 0) rhs--;	/* Get last active col number as the rhs vector b */
	n--;					/* Account for b, the rhs vector, to get row & col dimensions of normal matrix N */

	N = GMT_memory (GMT, NULL, n*n, double);
	B = GMT_memory (GMT, NULL, T->n_records, double);

	/* Do the row & col dot products, skipping inactive columns as we go along */
	for (j = j2 = 0; j < n; j2++) {	/* j2 is table column, j is row in N matrix */
		if (skip[j2]) continue;
		for (i = i2 = 0; i < n; i2++) {	/* i2 is table column, i is column in N matrix */
			if (skip[i2]) continue;
			k0 = j * n + i;
			N[k0] = 0.0;
			for (seg = k = 0; seg < info->T->n_segments; seg++) {
				for (row = 0; row < T->segment[seg]->n_rows; row++, k++) N[k0] += T->segment[seg]->coord[j2][row] * T->segment[seg]->coord[i2][row];
			}
			i++;
		}
		B[j] = 0.0;
		for (seg = k = 0; seg < info->T->n_segments; seg++) {
			for (row = 0; row < T->segment[seg]->n_rows; row++, k++) B[j] += T->segment[seg]->coord[j2][row] * T->segment[seg]->coord[rhs][row];
		}
		j++;
	}

	d = GMT_memory (GMT, NULL, n, double);
	x = GMT_memory (GMT, NULL, n, double);
	if ( (ier = GMT_chol_dcmp (GMT, N, d, &cond, n, n) ) != 0) {	/* Decomposition failed, use SVD method */
		GMT_LONG nrots;
		GMT_chol_recover (GMT, N, d, n, n, ier, TRUE);		/* Restore to former matrix N */
		/* Solve instead using GMT_jacobi */
		lambda = GMT_memory (GMT, NULL, n, double);
		b = GMT_memory (GMT, NULL, n, double);
		z = GMT_memory (GMT, NULL, n, double);
		v = GMT_memory (GMT, NULL, n*n, double);

		if (GMT_jacobi (GMT, N, &n, &n, lambda, v, b, z, &nrots)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Eigenvalue routine failed to converge in 50 sweeps.\n");
			GMT_report (GMT, GMT_MSG_FATAL, "The reported L2 positions might be garbage.\n");
		}
		/* Solution x = v * lambda^-1 * v' * B */

		/* First do d = V' * B, so x = v * lambda^-1 * d */
		for (j = 0; j < n; j++) for (k = 0, d[j] = 0.0; k < n; k++) d[j] += v[k*n+j] * B[k];
		/* Then do d = lambda^-1 * d by setting small lambda's to zero */
		for (j = k = 0; j < n; j++) {
			if (lambda[j] < 1.0e7) {
				d[j] = 0.0;
				k++;
			}
			else
				d[j] /= lambda[j];
		}
		if (k) GMT_report (GMT, GMT_MSG_FATAL, "%ld eigenvalues < 1.0e-7 set to zero to yield a stable solution\n", k);

		/* Finally do x = v * d */
		for (j = 0; j < n; j++) for (k = 0; k < n; k++) x[j] += v[j*n+k] * d[k];

		GMT_free (GMT, b);
		GMT_free (GMT, z);
		GMT_free (GMT, v);
	}
	else {	/* Decomposition worked, now solve system */
		GMT_chol_solv (GMT, N, x, B, n, n);
	}

	if (!file) {
		fp = GMT->session.std[GMT_OUT];
#ifdef SET_IO_MODE
		GMT_setmode (GMT, GMT_OUT);
#endif
	}
	else if ((fp = GMT_fopen (GMT, file, GMT->current.io.w_mode)) == NULL) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error creating file %s\n", file);
		return (EXIT_FAILURE);
	}

	GMT->current.io.output (GMT, fp, n, x);
	GMT_fclose (GMT, fp);

	GMT_free (GMT, x);
	GMT_free (GMT, d);
	GMT_free (GMT, N);
	GMT_free (GMT, B);
	return (EXIT_SUCCESS);
}

void load_column (struct GMT_DATASET *to, GMT_LONG to_col, struct GMT_TABLE *from, GMT_LONG from_col)
{	/* Copies data from one column to another */
	GMT_LONG seg;
	for (seg = 0; seg < from->n_segments; seg++) {
		GMT_memcpy (to->table[0]->segment[seg]->coord[to_col], from->segment[seg]->coord[from_col], from->segment[seg]->n_rows, double);
	}
}

void load_const_column (struct GMT_DATASET *to, GMT_LONG to_col, double factor)
{	/* Sets all rows in a column to a constant factor */
	GMT_LONG seg, row;
	for (seg = 0; seg < to->n_segments; seg++) {
		for (row = 0; row < to->table[0]->segment[seg]->n_rows; row++) to->table[0]->segment[seg]->coord[to_col][row] = factor;
	}
}

GMT_LONG same_size (struct GMT_DATASET *A, struct GMT_DATASET *B)
{	/* Are the two dataset the same size */
	GMT_LONG seg;
	if (!(A->table[0]->n_segments == B->table[0]->n_segments && A->table[0]->n_columns == B->table[0]->n_columns)) return (FALSE);
	for (seg = 0; seg < A->table[0]->n_segments; seg++) if (A->table[0]->segment[seg]->n_rows != B->table[0]->segment[seg]->n_rows) return (FALSE);
	return (TRUE);
}

GMT_LONG same_domain (struct GMT_DATASET *A, GMT_LONG t_col, struct GMT_TABLE *B)
{	/* Are the two dataset the same domain */
	GMT_LONG seg;
	for (seg = 0; seg < A->table[0]->n_segments; seg++) {
		if (!(GMT_IS_ZERO (A->table[0]->min[t_col] - B->min[COL_T]) && GMT_IS_ZERO (A->table[0]->max[t_col] - B->max[COL_T]))) return (FALSE);
	}
	return (TRUE);
}

GMT_LONG GMT_gmtmath_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "gmtmath %s [API] - Reverse Polish Notation (RPN) calculator for data tables\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: gmtmath [-A<ftable>] [-C<cols>] [-I] [-L] [-N<n_col>/<t_col>] [-Q]\n");
	GMT_message (GMT, "\t[-S[f|l]] [-T[<tmin>/<tmax>/<t_inc>[+]]] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\tA B op C op ... = [outfile]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_s_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT,
		"\tA, B, etc are table files, constants, or symbols (see below).\n"
		"\tTo read stdin give filename as STDIN (which can appear more than once).\n"
		"\tThe stack can hold up to %d entries (given enough memory).\n", GMTMATH_STACK_SIZE);
	GMT_message (GMT,
		"\tTrigonometric operators expect radians.\n"
		"\tThe operators and number of input and output arguments:\n\n"
		"\tName       #args   Returns\n"
		"\t--------------------------\n");
#include "gmtmath_explain.h"
	GMT_message (GMT,
		"\n\tThe special symbols are:\n\n"
		"\tPI                  = 3.1415926...\n"
		"\tE                   = 2.7182818...\n"
		"\tEULER               = 0.5772156...\n"
		"\tTMIN, TMAX, or TINC = the corresponding constant.\n"
		"\tN                   = number of records.\n"
		"\tT                   = table with t-coordinates.\n"
		"\tTn                  = table with normalized [-1 to +1] t-coordinates.\n"
		"\n\tOPTIONS:\n\n"
		"\t-A Require -N and will initialize table with file <ftable> containing t and f(t) only.\n"
		"\t   t goes into column <t_col> while f(t) goes into column <n_col> - 1.\n"
		"\t   No additional data files may be specified.\n"
		"\t-C Change which columns to operate on [Default is all except time].\n"
		"\t   -C reverts to the default, -Cr toggles current settings, and -Ca selects all columns.\n"
		"\t-I Reverse the output sequence into descending order [ascending].\n"
		"\t-L Apply operators on a per-segment basis [cumulates operations across file].\n"
		"\t-N Set the number of columns and the id of the time column (0 is first) [2/0].\n"
		"\t-Q Quick scalar calculator. Shorthand for -Ca -N1/0 -T0/0/1.\n"
		"\t-S Only write first row upon completion of calculations [write all rows].\n"
		"\t   Optionally, append l for last row or f for first row [Default].\n"
		"\t-T Set domain from t_min to t_max in steps of t_inc.\n"
		"\t   Append + to t_inc to indicate the number of points instead.\n"
		"\t   If a filename is given instead we read t coordinates from first column.\n"
		"\t   If no domain is given we assume no time, i.e. only data columns are present.\n"
		"\t   This choice also implies -Ca.\n");
	GMT_explain_options (GMT, "VC0D0fghios.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_gmtmath_parse (struct GMTAPI_CTRL *C, struct GMTMATH_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtmath and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0, n_req, missing_equal = TRUE;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;
#ifdef GMT_COMPAT
	GMT_LONG gmt_parse_o_option (struct GMT_CTRL *GMT, char *arg);
#endif

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (opt->arg[0] == '=' && opt->arg[1] == 0) {
					missing_equal = FALSE;
					if (opt->next && opt->next->option == GMTAPI_OPT_INFILE) {
						Ctrl->Out.active = TRUE;
						if (opt->next->arg) Ctrl->Out.file = strdup (opt->next->arg);
					}
				}
				n_files++;
				break;

			case '#':	/* Skip numbers */
				break;
				
			/* Processes program-specific parameters */

			case 'A':	/* y(x) table for LSQFIT operations */
				Ctrl->A.active = TRUE;
				Ctrl->A.file = strdup (opt->arg);
				break;
			case 'C':	/* Processed in the main loop but not here; just skip */
				break;
#ifdef GMT_COMPAT
			case 'F':	/* Now obsolete due to -o */
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -F is deprecated; use -o instead\n");
				gmt_parse_o_option (GMT, opt->arg);
				break;
#endif
			case 'I':	/* Reverse output order */
				Ctrl->I.active = TRUE;
				break;
			case 'L':	/* Apply operator per segment basis */
				Ctrl->L.active = TRUE;
				break;
			case 'N':	/* Sets no of columns and the time column */
				Ctrl->N.active = TRUE;
				sscanf (opt->arg, "%" GMT_LL "d/%" GMT_LL "d", &Ctrl->N.ncol, &Ctrl->N.tcol);
				break;
			case 'Q':	/* Quick for -Ca -N1/0 -T0/0/1 */
				Ctrl->Q.active = TRUE;
				break;
			case 'S':	/* Only want one row (first or last) */
				Ctrl->S.active = TRUE;
				switch (opt->arg[0]) {
					case 'f': case 'F': case '\0':
						Ctrl->S.mode = -1; break;
					case 'l': case 'L':
						Ctrl->S.mode = +1; break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Syntax is -S[f|l]\n");
						n_errors++;
					break;
				}
				break;
			case 'T':	/* Either get a file with time coordinate or a min/max/dt setting */
				Ctrl->T.active = TRUE;
				if (!opt->arg[0])	/* Turn off default GMT NaN-handling in t column */
					Ctrl->T.notime = TRUE;
				else if (!GMT_access (GMT, opt->arg, R_OK))	/* Argument given and file can be opened */
					Ctrl->T.file = strdup (opt->arg);
				else {	/* Presumably gave tmin/tmax/tinc */
					if (sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->T.min, &Ctrl->T.max, &Ctrl->T.inc) != 3) {
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Unable to decode arguments for -T\n");
						n_errors++;
					}
					if (opt->arg[strlen(opt->arg)-1] == '+') Ctrl->T.mode = 1;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
		}
	}

	n_errors += GMT_check_condition (GMT, Ctrl->A.active && GMT_access (GMT, Ctrl->A.file, R_OK), "Syntax error -A: Cannot read file %s!\n", Ctrl->A.file);
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->T.min > Ctrl->T.max, "Syntax error -T: min > max!\n");
	n_errors += GMT_check_condition (GMT, missing_equal, "Syntax error: Usage is <operations> = [outfile]\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && (Ctrl->T.active || Ctrl->N.active || Ctrl->C.active), "Syntax error: Cannot use -T, -N, or -C when -Q has been set\n");
	n_req = (Ctrl->N.active) ? Ctrl->N.ncol : 0;
	n_errors += GMT_check_binary_io (GMT, n_req);
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && (Ctrl->N.ncol <= 0 || Ctrl->N.tcol < 0 || Ctrl->N.tcol >= Ctrl->N.ncol),
		"Syntax error: -N must have positive n_cols and 0 <= t_col < n_col\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

/* -----------------------------------------------------------------
 *              Definitions of all operator functions
 * -----------------------------------------------------------------*/
/* Note: The OPERATOR: **** lines are used to extract syntax for documentation */

void table_ABS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ABS 1 1 abs (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand == 0!\n");
	if (constant[last]) a = fabs (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : fabs (T->segment[s]->coord[col][i]);
}

void table_ACOS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ACOS 1 1 acos (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && fabs (factor[last]) > 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, |operand| > 1 for ACOS!\n");
	if (constant[last]) a = d_acos (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : d_acos (T->segment[s]->coord[col][i]);
}

void table_ACOSH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ACOSH 1 1 acosh (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && fabs (factor[last]) > 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand < 1 for ACOSH!\n");
	if (constant[last]) a = acosh (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : acosh (T->segment[s]->coord[col][i]);
}

void table_ACSC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ACSC 1 1 acsc (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && fabs (factor[last]) > 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, |operand| > 1 for ACSC!\n");
	if (constant[last]) a = d_asin (1.0 / factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : d_asin (1.0 / T->segment[s]->coord[col][i]);
}

void table_ACOT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ACOT 1 1 acot (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && fabs (factor[last]) > 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, |operand| > 1 for ACOT!\n");
	if (constant[last]) a = atan (1.0 / factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : atan (1.0 / T->segment[s]->coord[col][i]);
}

void table_ADD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ADD 2 1 A + B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = a + b;
	}
}

void table_AND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: AND 2 1 B if A == NaN, else A.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = (GMT_is_dnan (a)) ? b : a;
	}
}

void table_ASEC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ASEC 1 1 asec (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && fabs (factor[last]) > 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, |operand| > 1 for ASEC!\n");
	if (constant[last]) a = d_acos (1.0 / factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : d_acos (1.0 / T->segment[s]->coord[col][i]);
}

void table_ASIN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ASIN 1 1 asin (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && fabs (factor[last]) > 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, |operand| > 1 for ASIN!\n");
	if (constant[last]) a = d_asin (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : d_asin (T->segment[s]->coord[col][i]);
}

void table_ASINH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ASINH 1 1 asinh (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = asinh (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : asinh (T->segment[s]->coord[col][i]);
}

void table_ATAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ATAN 1 1 atan (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = atan (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : atan (T->segment[s]->coord[col][i]);
}

void table_ATAN2 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ATAN2 2 1 atan2 (A, B).  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0 for ATAN2!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for ATAN2!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = d_atan2 (a, b);
	}
}

void table_ATANH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ATANH 1 1 atanh (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && fabs (factor[last]) >= 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, |operand| >= 1 for ATANH!\n");
	if (constant[last]) a = atanh (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : atanh (T->segment[s]->coord[col][i]);
}

void table_BEI (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: BEI 1 1 bei (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_bei (GMT, fabs (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_bei (GMT, fabs (T->segment[s]->coord[col][i]));
}

void table_BER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: BER 1 1 ber (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_ber (GMT, fabs (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_ber (GMT, fabs (T->segment[s]->coord[col][i]));
}

void table_CEIL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: CEIL 1 1 ceil (A) (smallest integer >= A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = ceil (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : ceil (T->segment[s]->coord[col][i]);
}

void table_CHICRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: CHICRIT 2 1 Critical value for chi-squared-distribution, with alpha = A and n = B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0 for CHICRIT!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for CHICRIT!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = GMT_chi2crit (GMT, a, b);
	}
}

void table_CHIDIST (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: CHIDIST 2 1 chi-squared-distribution P(chi2,n), with chi2 = A and n = B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0 for CHIDIST!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for CHIDIST!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		GMT_chi2 (GMT, a, b, &T_prev->segment[s]->coord[col][i]);
	}
}

void table_COL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: COL 1 1 Places column A on the stack.  */
{
	GMT_LONG s, i, k, prev = last - 1;
	struct GMT_TABLE *T = S[last]->table[0], *T_prev = S[prev]->table[0];

	if (!constant[last]) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error, argument to COL must be a constant column number (0 <= k < n_col)!\n");
		return;
	}
	k = irint (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		T->segment[s]->coord[col][i] = T_prev->segment[s]->coord[k][i];
	}
}

void table_CORRCOEFF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: CORRCOEFF 2 1 Correlation coefficient r(A, B).  */
{
	GMT_LONG s, i, row, prev = last - 1;
	double *a, *b, coeff;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev] && constant[last]) {	/* Correlation is undefined */
		for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T_prev->segment[s]->coord[col][i] = GMT->session.d_NaN;
		return;
	}

	if (!info->local) {	/* Compute correlation across the entire table */
		a = GMT_memory (GMT, NULL, info->T->n_records, double);
		b = GMT_memory (GMT, NULL, info->T->n_records, double);
		for (s = i = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++, i++) a[i] = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][row];
		for (s = i = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++, i++) b[i] = (constant[last]) ? factor[last] : T->segment[s]->coord[col][row];
		coeff = GMT_corrcoeff (GMT, a, b, info->T->n_records, 0);
		for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T_prev->segment[s]->coord[col][i] = coeff;
		return;
	}
	/* Local, or per-segment calculations */
	for (s = 0; s < info->T->n_segments; s++) {
		if (constant[prev]) {		/* Must create the missing (constant) column */
			a = GMT_memory (GMT, NULL, info->T->segment[s]->n_rows, double);
			for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) a[i] = factor[prev];
			b = T->segment[s]->coord[col];
		}
		else if (constant[last]) {	/* Must create the missing (constant) column */
			a = T_prev->segment[s]->coord[col];
			b = GMT_memory (GMT, NULL, info->T->segment[s]->n_rows, double);
			for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) b[i] = factor[last];
		}
		else {
			a = T_prev->segment[s]->coord[col];
			b = T->segment[s]->coord[col];
		}
		coeff = GMT_corrcoeff (GMT, a, b, info->T->segment[s]->n_rows, 0);
		for (i = 0; i < info->T->segment[s]->n_rows; i++) T_prev->segment[s]->coord[col][i] = coeff;
		if (constant[prev]) GMT_free (GMT, a);
		if (constant[last]) GMT_free (GMT, b);
	}
}


void table_COS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: COS 1 1 cos (A) (A in radians).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = cos (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		T->segment[s]->coord[col][i] = (constant[last]) ? a : cos (T->segment[s]->coord[col][i]);
	}
}

void table_COSD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: COSD 1 1 cos (A) (A in degrees).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = cosd (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : cosd (T->segment[s]->coord[col][i]);
}

void table_COSH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: COSH 1 1 cosh (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = cosh (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : cosh (T->segment[s]->coord[col][i]);
}

void table_COT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: COT 1 1 cot (A) (A in radians).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = (1.0 / tan (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		T->segment[s]->coord[col][i] = (constant[last]) ? a : (1.0 / tan (T->segment[s]->coord[col][i]));
	}
}


void table_COTD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: COTD 1 1 cot (A) (A in degrees).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = (1.0 / tand (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		T->segment[s]->coord[col][i] = (constant[last]) ? a : (1.0 / tand (T->segment[s]->coord[col][i]));
	}
}

void table_CSC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: CSC 1 1 csc (A) (A in radians).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = (1.0 / sin (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		T->segment[s]->coord[col][i] = (constant[last]) ? a : (1.0 / sin (T->segment[s]->coord[col][i]));
	}
}

void table_CSCD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: CSCD 1 1 csc (A) (A in degrees).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = (1.0 / sind (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		T->segment[s]->coord[col][i] = (constant[last]) ? a : (1.0 / sind (T->segment[s]->coord[col][i]));
	}
}

void table_CPOISS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: CPOISS 2 1 Cumulative Poisson distribution F(x,lambda), with x = A and lambda = B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for CPOISS!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		GMT_cumpoisson (GMT, a, b, &T_prev->segment[s]->coord[col][i]);
	}
}

void table_DDT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: DDT 1 1 d(A)/dt Central 1st derivative.  */
{
	GMT_LONG s, i;
	double c, left, next_left;
	struct GMT_TABLE *T = S[last]->table[0];

	/* Central 1st difference in t */

	if (info->irregular) GMT_report (GMT, GMT_MSG_FATAL, "Warning, DDT called on irregularly spaced data (not supported)!\n");
	if (constant[last]) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand to DDT is constant!\n");

	c = 0.5 / info->t_inc;
	for (s = 0; s < info->T->n_segments; s++) {
		i = 0;
		while (i < info->T->segment[s]->n_rows) {	/* Process each segment */
			next_left = 2.0 * T->segment[s]->coord[col][i] - T->segment[s]->coord[col][i+1];
			while (i < info->T->segment[s]->n_rows - 1) {
				left = next_left;
				next_left = T->segment[s]->coord[col][i];
				T->segment[s]->coord[col][i] = (constant[last]) ? 0.0 : c * (T->segment[s]->coord[col][i+1] - left);
				i++;
			}
			T->segment[s]->coord[col][i] = (constant[last]) ? 0.0 : 2.0 * c * (T->segment[s]->coord[col][i] - next_left);
			i++;
		}
	}
}

void table_D2DT2 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: D2DT2 1 1 d^2(A)/dt^2 2nd derivative.  */
{
	GMT_LONG s, i;
	double c, left, next_left;
	struct GMT_TABLE *T = S[last]->table[0];

	/* Central 2nd difference in t */

	if (info->irregular) GMT_report (GMT, GMT_MSG_FATAL, "Warning, D2DT2 called on irregularly spaced data (not supported)!\n");
	if (constant[last]) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand to D2DT2 is constant!\n");

	c = 1.0 / (info->t_inc * info->t_inc);
	for (s = 0; s < info->T->n_segments; s++) {
		i = 0;
		while (i < info->T->segment[s]->n_rows) {	/* Process each segment */
			next_left = T->segment[s]->coord[col][i];
			T->segment[s]->coord[col][i] = (GMT_is_dnan (T->segment[s]->coord[col][i]) || GMT_is_dnan (T->segment[s]->coord[col][i+1])) ? GMT->session.d_NaN : 0.0;
			i++;
			while (i < info->T->segment[s]->n_rows - 1) {
				left = next_left;
				next_left = T->segment[s]->coord[col][i];
				T->segment[s]->coord[col][i] = (constant[last]) ? 0.0 : c * (T->segment[s]->coord[col][i+1] - 2 * T->segment[s]->coord[col][i] + left);
				i++;
			}
			T->segment[s]->coord[col][i] = (GMT_is_dnan (T->segment[s]->coord[col][i]) || GMT_is_dnan (T->segment[s]->coord[col][i-1])) ? GMT->session.d_NaN : 0.0;
			i++;
		}
	}
}

void table_D2R (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: D2R 1 1 Converts Degrees to Radians.  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = factor[last] * D2R;
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : T->segment[s]->coord[col][i] * D2R;
}

void table_DILOG (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: DILOG 1 1 dilog (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_dilog (GMT, factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_dilog (GMT, T->segment[s]->coord[col][i]);
}

void table_DIFF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: DIFF 1 1 Difference between adjacent elements of A (A[1]-A[0], A[2]-A[1], ..., 0). */
{
	GMT_LONG s, i;
	struct GMT_TABLE *T = S[last]->table[0];

	/* Central 1st difference in t */
	for (s = 0; s < info->T->n_segments; s++) {
		for (i = 0; i < info->T->segment[s]->n_rows - 1; i++) 
			T->segment[s]->coord[col][i] = T->segment[s]->coord[col][i+1] - T->segment[s]->coord[col][i];

		T->segment[s]->coord[col][info->T->segment[s]->n_rows - 1] = 0;
	}
}

void table_DIV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: DIV 2 1 A / B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	void table_MUL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col);
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[last] && factor[last] == 0.0) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: Divide by zero gives NaNs\n");
	}
	if (constant[last]) {	/* Turn divide into multiply */
		a = factor[last];	/* Save old factor */
		factor[last] = 1.0 / factor[last];
		table_MUL (GMT, info, S, constant, factor, last, col);
		factor[last] = a;	/* Restore factor to original value */
		return;
	}

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = a / b;
	}
}

void table_DUP (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: DUP 1 2 Places duplicate of A on the stack.  */
{
	GMT_LONG next = last + 1, i, s;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_next = S[next]->table[0];

	factor[next] = factor[last];
	constant[next] = constant[last];
	for (s = 0; s < info->T->n_segments; s++) {
		if (constant[last]) {
			for (i = 0; i < info->T->segment[s]->n_rows; i++) T_next->segment[s]->coord[col][i] = T->segment[s]->coord[col][i] = factor[next];
		}
		else
			GMT_memcpy (T_next->segment[s]->coord[col], T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
	}
}

void table_ERF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ERF 1 1 Error function erf (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = erf (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : erf (T->segment[s]->coord[col][i]);
}

void table_ERFC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ERFC 1 1 Complementary Error function erfc (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = erfc (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : erfc (T->segment[s]->coord[col][i]);
}

void table_ERFINV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ERFINV 1 1 Inverse error function of A.  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_erfinv (GMT, factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_erfinv (GMT, T->segment[s]->coord[col][i]);
}

void table_EQ (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: EQ 2 1 1 if A == B, else 0.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = (double)(a == b);
	}
}

void table_EXCH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: EXCH 2 2 Exchanges A and B on the stack.  */
{
	GMT_LONG prev = last - 1;
	struct GMT_DATASET *D = S[last];
	S[last] = S[prev];	S[prev] = D;
	l_swap (constant[last], constant[prev]);
	d_swap (factor[last], factor[prev]);
}

void table_EXP (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: EXP 1 1 exp (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = exp (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : exp (T->segment[s]->coord[col][i]);
}

void table_FACT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: FACT 1 1 A! (A factorial).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_factorial (GMT, (GMT_LONG)irint(factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_factorial (GMT, (GMT_LONG)irint(T->segment[s]->coord[col][i]));
}

void table_FCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: FCRIT 3 1 Critical value for F-distribution, with alpha = A, n1 = B, and n2 = C.  */
{
	GMT_LONG s, i, nu1, nu2, prev1 = last - 1, prev2 = last - 2;
	double alpha;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev1 = (constant[prev1]) ? NULL : S[prev1]->table[0], *T_prev2 = S[prev2]->table[0];

	if (constant[prev2] && factor[prev2] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0 for FCRIT!\n");
	if (constant[prev1] && factor[prev1] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for FCRIT!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand three == 0 for FCRIT!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		alpha = (constant[prev2]) ? factor[prev2] : T_prev2->segment[s]->coord[col][i];
		nu1 = irint ((double)((constant[prev1]) ? factor[prev1] : T_prev1->segment[s]->coord[col][i]));
		nu2 = irint ((double)((constant[last]) ? factor[last] : T->segment[s]->coord[col][i]));
		T_prev2->segment[s]->coord[col][i] = GMT_Fcrit (GMT, alpha, (double)nu1, (double)nu2);
	}
}

void table_FDIST (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: FDIST 3 1 F-distribution Q(F,n1,n2), with F = A, n1 = B, and n2 = C.  */
{
	GMT_LONG s, i, nu1, nu2, prev1 = last - 1, prev2 = last - 2;
	double F, chisq1, chisq2 = 1.0;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev1 = (constant[prev1]) ? NULL : S[prev1]->table[0], *T_prev2 = S[prev2]->table[0];

	if (constant[prev1] && factor[prev1] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for FDIST!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand three == 0 for FDIST!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		F = (constant[prev2]) ? factor[prev2] : T_prev2->segment[s]->coord[col][i];
		nu1 = irint ((double)((constant[prev1]) ? factor[prev1] : T_prev1->segment[s]->coord[col][i]));
		nu2 = irint ((double)((constant[last]) ? factor[last] : T->segment[s]->coord[col][i]));
		/* Since GMT_f_q needs chisq1 and chisq2, we set chisq2 = 1 and solve for chisq1 */
		chisq1 = F * nu1 / nu2;
		(void) GMT_f_q (GMT, chisq1, nu1, chisq2, nu2, &T_prev2->segment[s]->coord[col][i]);
	}
}

void table_FLIPUD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: FLIPUD 1 1 Reverse order of each column.  */
{
	GMT_LONG s, i, k;
	struct GMT_TABLE *T = S[last]->table[0];
	/* Reverse the order of points in a column */
	if (constant[last]) return;
	for (s = 0; s < info->T->n_segments; s++) for (i = 0, k = info->T->segment[s]->n_rows-1; i < info->T->segment[s]->n_rows/2; i++, k--) d_swap (T->segment[s]->coord[col][i], T->segment[s]->coord[col][k]);
}

void table_FLOOR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: FLOOR 1 1 floor (A) (greatest integer <= A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = floor (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : floor (T->segment[s]->coord[col][i]);
}

void table_FMOD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: FMOD 2 1 A % B (remainder after truncated division).  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, using FMOD 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = fmod (a, b);
	}
}

void table_GE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: GE 2 1 1 if A >= B, else 0.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = (double)(a >= b);
	}
}

void table_GT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: GT 2 1 1 if A > B, else 0.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = (double)(a > b);
	}
}

void table_HYPOT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: HYPOT 2 1 hypot (A, B) = sqrt (A*A + B*B).  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = hypot (a, b);
	}
}

void table_I0 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: I0 1 1 Modified Bessel function of A (1st kind, order 0).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_i0 (GMT, factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_i0 (GMT, T->segment[s]->coord[col][i]);
}

void table_I1 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: I1 1 1 Modified Bessel function of A (1st kind, order 1).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_i1 (GMT, factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_i1 (GMT, T->segment[s]->coord[col][i]);
}

void table_IN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: IN 2 1 Modified Bessel function of A (1st kind, order B).  */
{
	GMT_LONG s, i, prev = last - 1, simple = FALSE, order = 0;
	double b = 0.0;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[last]) {
		if (factor[last] < 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order < 0 for IN!\n");
		if (fabs (rint(factor[last]) - factor[last]) > GMT_SMALL) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order not an integer for IN!\n");
		order = irint (fabs (factor[last]));
		if (constant[prev]) {
			b = GMT_in (GMT, order, fabs (factor[prev]));
			simple = TRUE;
		}
	}
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (simple)
			T_prev->segment[s]->coord[col][i] = b;
		else {
			if (!constant[last]) order = irint (fabs (T->segment[s]->coord[col][i]));
			T_prev->segment[s]->coord[col][i] = GMT_in (GMT, order, fabs (T_prev->segment[s]->coord[col][i]));
		}
	}
}

void table_INRANGE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: INRANGE 3 1 1 if B <= A <= C, else 0.  */
{
	GMT_LONG s, i, prev1 = last - 1, prev2 = last - 2, inrange;
	double a = 0.0, b = 0.0, c = 0.0;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev1 = (constant[prev1]) ? NULL : S[prev1]->table[0], *T_prev2 = S[prev2]->table[0];

	/* last is C, prev1 is B, prev2 is A */

	/* Set to 1 where B <= A <= C, 0 elsewhere, except where
	 * A, B, or C = NaN, in which case we set answer to NaN */

	if (constant[prev2]) a = (double)factor[prev2];
	if (constant[prev1]) b = (double)factor[prev1];
	if (constant[last])  c = (double)factor[last];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (!constant[prev2]) a = T_prev2->segment[s]->coord[col][i];
		if (!constant[prev1]) b = T_prev1->segment[s]->coord[col][i];
		if (!constant[last])  c = T->segment[s]->coord[col][i];

		if (GMT_is_dnan (a) || GMT_is_dnan (b) || GMT_is_dnan (c)) {
			T_prev2->segment[s]->coord[col][i] = GMT->session.d_NaN;
			continue;
		}

		inrange = (b <= a && a <= c);
		T_prev2->segment[s]->coord[col][i] = (double)inrange;
	}
}

void table_INT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: INT 1 1 Numerically integrate A.  */
{
	GMT_LONG s, i, k;
	double f = 0.0, left, right, sum;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) {	/* Trivial case */
		sum = factor[last] * info->t_inc;
		for (s = k = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++, k++) T->segment[s]->coord[col][i] = ((info->local) ? i : k) * sum;
		return;
	}

	/* We use dumb trapezoidal rule - one day we will replace with more sophisticated rules */

	sum = 0.0;
	if (!info->irregular) f = 0.5 * info->t_inc;

	for (s = 0; s < info->T->n_segments; s++) {
		i = 0;
		if (info->local) sum = 0.0;	/* Reset integrated sum for each segment */
		while (i < info->T->segment[s]->n_rows) {
			left = T->segment[s]->coord[col][i];
			T->segment[s]->coord[col][i] = sum;
			i++;
			while (i < info->T->segment[s]->n_rows) {	/* Dumb trapezoidal rule */
				if (info->irregular) f = 0.5 * (info->T->segment[s]->coord[COL_T][i] - info->T->segment[s]->coord[COL_T][i-1]);
				right = T->segment[s]->coord[col][i];
				sum += f * (left + right);
				T->segment[s]->coord[col][i] = sum;
				left = right;
				i++;
			}
		}
	}
}

void table_INV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: INV 1 1 1 / A.  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_FATAL, "Warning: Inverse of zero gives NaNs\n");
	if (constant[last]) a = (factor[last] == 0) ? GMT->session.d_NaN : 1.0 / factor[last];
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : 1.0 / T->segment[s]->coord[col][i];
}

void table_ISNAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ISNAN 1 1 1 if A == NaN, else 0.  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = (double)GMT_is_dnan (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : (double)GMT_is_dnan (T->segment[s]->coord[col][i]);
}

void table_J0 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: J0 1 1 Bessel function of A (1st kind, order 0).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = j0 (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : j0 (T->segment[s]->coord[col][i]);
}

void table_J1 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: J1 1 1 Bessel function of A (1st kind, order 1).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = j1 (fabs (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : j1 (fabs (T->segment[s]->coord[col][i]));
}

void table_JN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: JN 2 1 Bessel function of A (1st kind, order B).  */
{
	GMT_LONG s, i, prev = last - 1, order = 0, simple = FALSE;
	double b = 0.0;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[last]) {
		if (factor[last] < 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order < 0 for JN!\n");
		if (fabs (rint(factor[last]) - factor[last]) > GMT_SMALL) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order not an integer for JN!\n");
		order = irint (fabs (factor[last]));
		if (constant[prev]) {
			b = jn ((int)order, fabs (factor[prev]));
			simple = TRUE;
		}
	}
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (simple)
			T_prev->segment[s]->coord[col][i] = b;
		else {
			if (!constant[last]) order = irint (fabs (T->segment[s]->coord[col][i]));
			T_prev->segment[s]->coord[col][i] = jn ((int)order, fabs (T_prev->segment[s]->coord[col][i]));
		}
	}
}

void table_K0 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: K0 1 1 Modified Kelvin function of A (2nd kind, order 0).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_k0 (GMT, factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_k0 (GMT, T->segment[s]->coord[col][i]);
}

void table_K1 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: K1 1 1 Modified Bessel function of A (2nd kind, order 1).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_k1 (GMT, factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_k1 (GMT, T->segment[s]->coord[col][i]);
}

void table_KN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: KN 2 1 Modified Bessel function of A (2nd kind, order B).  */
{
	GMT_LONG s, i, prev = last - 1, order = 0, simple = FALSE;
	double b = 0.0;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[last]) {
		if (factor[last] < 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order < 0 for KN!\n");
		if (fabs (rint(factor[last]) - factor[last]) > GMT_SMALL) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order not an integer for KN!\n");
		order = irint (fabs (factor[last]));
		if (constant[prev]) {
			b = GMT_kn (GMT, order, fabs (factor[prev]));
			simple = TRUE;
		}
	}
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (simple)
			T_prev->segment[s]->coord[col][i] = b;
		else {
			if (!constant[last]) order = irint (fabs (T->segment[s]->coord[col][i]));
			T_prev->segment[s]->coord[col][i] = GMT_kn (GMT, order, fabs (T_prev->segment[s]->coord[col][i]));
		}
	}
}

void table_KEI (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: KEI 1 1 kei (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_kei (GMT, fabs (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_kei (GMT, fabs (T->segment[s]->coord[col][i]));
}

void table_KER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: KER 1 1 ker (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_ker (GMT, fabs (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_ker (GMT, fabs (T->segment[s]->coord[col][i]));
}

void table_KURT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: KURT 1 1 Kurtosis of A.  */
{
	GMT_LONG s, i, n = 0;
	double mean = 0.0, sum2 = 0.0, kurt = 0.0, delta;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = GMT->session.d_NaN;
		return;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) {n = 0; mean = sum2 = kurt = 0.0;}	/* Reset for each segment */
		for (i = 0; i < info->T->segment[s]->n_rows; i++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][i])) continue;
			n++;
			delta = T->segment[s]->coord[col][i] - mean;
			mean += delta / n;
			sum2 += delta * (T->segment[s]->coord[col][i] - mean);
		}
		if (info->local) {
			for (i = 0; i < info->T->segment[s]->n_rows; i++) {
				if (GMT_is_dnan (T->segment[s]->coord[col][i])) continue;
				delta = T->segment[s]->coord[col][i] - mean;
				kurt += pow (delta, 4.0);
			}
			if (n > 1) {
				sum2 /= (n - 1);
				kurt = kurt / (n * sum2 * sum2) - 3.0;
			}
			else
				kurt = GMT->session.d_NaN;
			for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = kurt;
		}
	}
	if (info->local) return;

	/* Here we do the global kurtosis */
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (GMT_is_dnan (T->segment[s]->coord[col][i])) continue;
		delta = T->segment[s]->coord[col][i] - mean;
		kurt += pow (delta, 4.0);
	}
	if (n > 1) {
		sum2 /= (n - 1);
		kurt = kurt / (n * sum2 * sum2) - 3.0;
	}
	else
		kurt = GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = kurt;
}

void table_LE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: LE 2 1 1 if A <= B, else 0.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = (double)(a <= b);
	}
}

void table_LMSSCL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: LMSSCL 1 1 LMS scale estimate (LMS STD) of A.  */
{
	GMT_LONG s, i, k, GMT_mode_selection = 0, GMT_n_multiples = 0;
	double lmsscl, mode, *z = NULL;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) GMT_memset (T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
		return;
	}

	if (!info->local) z = GMT_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++)  {
		if (info->local) {
			GMT_sort_array (GMT, T->segment[s]->coord[col], info->T->segment[s]->n_rows, GMTAPI_DOUBLE);
			for (i = info->T->segment[s]->n_rows; GMT_is_dnan (T->segment[s]->coord[col][i-1]) && i > 1; i--);
			if (i) {
				GMT_mode (GMT, T->segment[s]->coord[col], i, i/2, 0, GMT_mode_selection, &GMT_n_multiples, &mode);
				GMT_getmad (GMT, T->segment[s]->coord[col], i, mode, &lmsscl);
			}
			else
				lmsscl = GMT->session.d_NaN;

			for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = lmsscl;
			if (GMT_n_multiples > 0) GMT_report (GMT, GMT_MSG_FATAL, "Warning: %ld Multiple modes found for segment %ld\n", GMT_n_multiples, s);
		}
		else {	/* Just accumulate the total table */
			GMT_memcpy (&z[k], T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return;	/* Done with local */
	GMT_sort_array (GMT, z, info->T->n_records, GMTAPI_DOUBLE);
	for (i = info->T->n_records; GMT_is_dnan (z[i-1]) && i > 1; i--);
	if (i) {
		GMT_mode (GMT, z, i, i/2, 0, GMT_mode_selection, &GMT_n_multiples, &mode);
		GMT_getmad (GMT, z, i, mode, &lmsscl);
	}
	else
		lmsscl = GMT->session.d_NaN;

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = lmsscl;
	if (GMT_n_multiples > 0) GMT_report (GMT, GMT_MSG_FATAL, "Warning: %ld Multiple modes found\n", GMT_n_multiples);
	GMT_free (GMT, z);
}

void table_LOG (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: LOG 1 1 log (A) (natural log).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, argument to log = 0!\n");

	if (constant[last]) a = d_log (GMT, fabs (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : d_log (GMT, fabs (T->segment[s]->coord[col][i]));
}

void table_LOG10 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: LOG10 1 1 log10 (A) (base 10).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, argument to log10 = 0!\n");

	if (constant[last]) a = d_log10 (GMT, fabs (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : d_log10 (GMT, fabs (T->segment[s]->coord[col][i]));
}

void table_LOG1P (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: LOG1P 1 1 log (1+A) (accurate for small A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && factor[last] < 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, argument to log1p < 0!\n");

	if (constant[last]) a = d_log1p (GMT, fabs (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : d_log1p (GMT, fabs (T->segment[s]->coord[col][i]));
}

void table_LOG2 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: LOG2 1 1 log2 (A) (base 2).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, argument to log2 = 0!\n");

	if (constant[last]) a = d_log (GMT, fabs (factor[last])) * M_LN2_INV;
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : d_log (GMT, fabs (T->segment[s]->coord[col][i])) * M_LN2_INV;
}

void table_LOWER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: LOWER 1 1 The lowest (minimum) value of A.  */
{
	GMT_LONG s, i;
	double low = DBL_MAX;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = factor[last];
		return;
	}

	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) low = DBL_MAX;
		for (i = 0; i < info->T->segment[s]->n_rows; i++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][i])) continue;
			if (T->segment[s]->coord[col][i] < low) low = T->segment[s]->coord[col][i];
		}
		if (info->local) for (i = 0; i < info->T->segment[s]->n_rows; i++) if (!GMT_is_dnan (T->segment[s]->coord[col][i])) T->segment[s]->coord[col][i] = low;
	}
	if (info->local) return;	/* Done with local */
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) if (!GMT_is_dnan (T->segment[s]->coord[col][i])) T->segment[s]->coord[col][i] = low;
}

void table_LRAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: LRAND 2 1 Laplace random noise with mean A and std. deviation B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a = 0.0, b = 0.0;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev]) a = factor[prev];
	if (constant[last]) b = factor[last];
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (!constant[prev]) a = T_prev->segment[s]->coord[col][i];
		if (!constant[last]) b = T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = a + b * GMT_lrand (GMT);
	}
}

void table_LSQFIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: LSQFIT 1 0 Let current table be [A | b]; return least squares solution x = A \\ b.  */
{
	/* Dummy routine needed since the automatically generated include file will have table_LSQFIT
	 * with these parameters just like any other function.  However, when we find LSQFIT we will
	 * instead call solve_LSQFIT which can be found at the end of these functions */
}

void table_LT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: LT 2 1 1 if A < B, else 0.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = (double)(a < b);
	}
}

void table_MAD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: MAD 1 1 Median Absolute Deviation (L1 STD) of A.  */
{
	GMT_LONG s, i, k;
	double mad, med, *z = NULL;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) GMT_memset (T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
		return;
	}

	if (!info->local) z = GMT_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++) {
		if (info->local) {
			GMT_sort_array (GMT, T->segment[s]->coord[col], info->T->segment[s]->n_rows, GMTAPI_DOUBLE);
			for (i = info->T->segment[s]->n_rows; GMT_is_dnan (T->segment[s]->coord[col][i-1]) && i > 1; i--);
			if (i) {
				med = (i%2) ? T->segment[s]->coord[col][i/2] : 0.5 * (T->segment[s]->coord[col][(i-1)/2] + T->segment[s]->coord[col][i/2]);
				GMT_getmad (GMT, T->segment[s]->coord[col], i, med, &mad);
			}
			else
				mad = GMT->session.d_NaN;
			for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = mad;
		}
		else {	/* Just accumulate the total table */
			GMT_memcpy (&z[k], T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return;	/* Done with local */
	GMT_sort_array (GMT, z, info->T->n_records, GMTAPI_DOUBLE);
	for (i = info->T->n_records; GMT_is_dnan (z[i-1]) && i > 1; i--);
	if (i) {
		med = (i%2) ? z[i/2] : 0.5 * (z[(i-1)/2] + z[i/2]);
		GMT_getmad (GMT, z, i, med, &mad);
	}
	else
		mad = GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = mad;
	GMT_free (GMT, z);
}

void table_MAX (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: MAX 2 1 Maximum of A and B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = MAX (a, b);
	}
}

void table_MEAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: MEAN 1 1 Mean value of A.  */
{
	GMT_LONG s, i, n_a = 0;
	double sum_a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = factor[last];
		return;
	}

	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) {sum_a = 0.0; n_a = 0;}
		for (i = 0; i < info->T->segment[s]->n_rows; i++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][i])) continue;
			sum_a += T->segment[s]->coord[col][i];
			n_a++;
		}
		if (info->local) {
			sum_a = (n_a) ? sum_a / n_a : 0.0;
			for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = sum_a;
		}
	}
	if (info->local) return;	/* Done with local */
	sum_a = (n_a) ? sum_a / n_a : 0.0;
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = sum_a;
}

void table_MED (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: MED 1 1 Median value of A.  */
{
	GMT_LONG s, i, k;
	double med, *z = NULL;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = factor[last];
		return;
	}

	if (!info->local) z = GMT_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++) {
		if (info->local) {
			GMT_sort_array (GMT, T->segment[s]->coord[col], info->T->segment[s]->n_rows, GMTAPI_DOUBLE);
			for (i = info->T->segment[s]->n_rows; GMT_is_dnan (T->segment[s]->coord[col][i-1]) && i > 1; i--);
			if (i)
				med = (i%2) ? T->segment[s]->coord[col][i/2] : 0.5 * (T->segment[s]->coord[col][(i-1)/2] + T->segment[s]->coord[col][i/2]);
			else
				med = GMT->session.d_NaN;

			for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = med;
		}
		else {	/* Just accumulate the total table */
			GMT_memcpy (&z[k], T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return;	/* Done with local */
	GMT_sort_array (GMT, z, info->T->n_records, GMTAPI_DOUBLE);
	for (i = info->T->n_records; GMT_is_dnan (z[i-1]) && i > 1; i--);
	if (i)
		med = (i%2) ? z[i/2] : 0.5 * (z[(i-1)/2] + z[i/2]);
	else
		med = GMT->session.d_NaN;

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = med;
	GMT_free (GMT, z);
}

void table_MIN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: MIN 2 1 Minimum of A and B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = MIN (a, b);
	}
}

void table_MOD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: MOD 2 1 A mod B (remainder after floored division).  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	prev = last - 1;
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, using MOD 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = MOD (a, b);
	}
}

void table_MODE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: MODE 1 1 Mode value (Least Median of Squares) of A.  */
{
	GMT_LONG s, i, k, GMT_mode_selection = 0, GMT_n_multiples = 0;
	double mode, *z = NULL;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = factor[last];
		return;
	}

	if (!info->local) z = GMT_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++)  {
		if (info->local) {
			GMT_sort_array (GMT, T->segment[s]->coord[col], info->T->segment[s]->n_rows, GMTAPI_DOUBLE);
			for (i = info->T->segment[s]->n_rows; GMT_is_dnan (T->segment[s]->coord[col][i-1]) && i > 1; i--);
			if (i)
				GMT_mode (GMT, T->segment[s]->coord[col], i, i/2, 0, GMT_mode_selection, &GMT_n_multiples, &mode);
			else
				mode = GMT->session.d_NaN;

			for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = mode;
			if (GMT_n_multiples > 0) GMT_report (GMT, GMT_MSG_FATAL, "Warning: %ld Multiple modes found for segment %ld\n", GMT_n_multiples, s);
		}
		else {	/* Just accumulate the total table */
			GMT_memcpy (&z[k], T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	GMT_sort_array (GMT, z, info->T->n_records, GMTAPI_DOUBLE);
	for (i = info->T->n_records; GMT_is_dnan (z[i-1]) && i > 1; i--);
	if (i)
		GMT_mode (GMT, z, i, i/2, 0, GMT_mode_selection, &GMT_n_multiples, &mode);
	else
		mode = GMT->session.d_NaN;

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = mode;
	if (GMT_n_multiples > 0) GMT_report (GMT, GMT_MSG_FATAL, "Warning: %ld Multiple modes found\n", GMT_n_multiples);
	GMT_free (GMT, z);
}

void table_MUL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: MUL 2 1 A * B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = a * b;
	}
}

void table_NAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: NAN 2 1 NaN if A == B, else A.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a = 0.0, b = 0.0;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev]) a = factor[prev];
	if (constant[last]) b = factor[last];
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (!constant[prev]) a = T_prev->segment[s]->coord[col][i];
		if (!constant[last]) b = T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = ((a == b) ? GMT->session.d_NaN : a);
	}
}

void table_NEG (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: NEG 1 1 -A.  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand == 0!\n");
	if (constant[last]) a = -factor[last];
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : -T->segment[s]->coord[col][i];
}

void table_NEQ (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: NEQ 2 1 1 if A != B, else 0.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = (double)(a != b);
	}
}

void table_NORM (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: NORM 1 1 Normalize (A) so max(A)-min(A) = 1.  */
{
	GMT_LONG s, i, n;
	double a, z, zmin = DBL_MAX, zmax = -DBL_MAX;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning, NORM of a constant gives NaN!\n");
		a = GMT->session.d_NaN;
	}
	else {
		for (s = n = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
			z = T->segment[s]->coord[col][i];
			if (GMT_is_dnan (z)) continue;
			if (z < zmin) zmin = z;
			if (z > zmax) zmax = z;
			n++;
		}
		a = (n == 0 || zmax == zmin) ? GMT->session.d_NaN : 1.0 / (zmax - zmin);	/* Normalization scale */
	}
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : a * (T->segment[s]->coord[col][i]);
}

void table_NOT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: NOT 1 1 NaN if A == NaN, 1 if A == 0, else 0.  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand == 0!\n");
	if (constant[last]) a = (fabs (factor[last]) > GMT_CONV_LIMIT) ? 0.0 : 1.0;
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : ((fabs (T->segment[s]->coord[col][i]) > GMT_CONV_LIMIT) ? 0.0 : 1.0);
}

void table_NRAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: NRAND 2 1 Normal, random values with mean A and std. deviation B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a = 0.0, b = 0.0;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev]) a = factor[prev];
	if (constant[last]) b = factor[last];
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (!constant[prev]) a = T_prev->segment[s]->coord[col][i];
		if (!constant[last]) b = T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = a + b * GMT_nrand (GMT);
	}
}

void table_OR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: OR 2 1 NaN if B == NaN, else A.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = (GMT_is_dnan (a) || GMT_is_dnan (b)) ? GMT->session.d_NaN : a;
	}
}

void table_PLM (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: PLM 3 1 Associated Legendre polynomial P(A) degree B order C.  */
{
	GMT_LONG s, i, prev = last - 1, first = last - 2, L, M;
	double a = 0.0;
	struct GMT_TABLE *T_first = S[first]->table[0];
	/* last holds the order M ,prev holds the degree L, first holds the argument x = cos(colat) */

	if (!(constant[prev] && constant[last])) {
		GMT_report (GMT, GMT_MSG_FATAL, "L and M must be constants in PLM (no calculations performed)\n");
		return;
	}
	L = irint (factor[prev]);
	M = irint (factor[last]);

	if (constant[first]) a = GMT_plm (GMT, L, M, factor[first]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T_first->segment[s]->coord[col][i] = (constant[first]) ? a : GMT_plm (GMT, L, M, T_first->segment[s]->coord[col][i]);
}

void table_PLMg (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: PLMg 3 1 Normalized associated Legendre polynomial P(A) degree B order C (geophysical convention).  */
{
	GMT_LONG s, i, prev = last - 1, first = last - 2, L, M;
	double a = 0.0;
	struct GMT_TABLE *T_first = S[first]->table[0];
	/* last holds the order M, prev holds the degree L, first holds the argument x = cos(colat) */

	if (!(constant[prev] && constant[last])) {
		GMT_report (GMT, GMT_MSG_FATAL, "L and M must be constants in PLMg (no calculations performed)\n");
		return;
	}
	L = irint (factor[prev]);
	M = irint (factor[last]);

	if (constant[first]) a = GMT_plm_bar (GMT, L, M, factor[first], FALSE);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T_first->segment[s]->coord[col][i] = (constant[first]) ? a : GMT_plm_bar (GMT, L, M, T_first->segment[s]->coord[col][i], FALSE);
}

void table_POP (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: POP 1 0 Delete top element from the stack.  */
{
	/* Dummy routine that does nothing but consume the top element of stack */
}

void table_POW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: POW 2 1 A ^ B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = pow (a, b);
	}
}

void table_PQUANT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: PQUANT 2 1 The B'th Quantile (0-100%) of A.  */
{
	GMT_LONG s, i, k, prev = last - 1;
	double p, *z = NULL;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	/* last holds the selected quantile (0-100), prev the data % */
	if (!constant[last]) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: PQUANT must be given a constant quantile (no calculations performed)\n");
		return;
	}
	if (factor[last] < 0.0 || factor[last] > 100.0) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: PQUANT must be given a constant quantile between 0-100%% (no calculations performed)\n");
		return;
	}
	if (constant[prev]) {	/* Trivial case */
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: PQUANT of a constant is set to NaN\n");
		p = GMT->session.d_NaN;
		for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T_prev->segment[s]->coord[col][i] = p;
		return;
	}

	if (!info->local) z = GMT_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++)  {
		if (info->local) {
			GMT_sort_array (GMT, T_prev->segment[s]->coord[col], info->T->segment[s]->n_rows, GMTAPI_DOUBLE);
			p = GMT_quantile (GMT, T_prev->segment[s]->coord[col], factor[last], info->T->segment[s]->n_rows);
			for (i = 0; i < info->T->segment[s]->n_rows; i++) T_prev->segment[s]->coord[col][i] = p;
		}
		else {	/* Just accumulate the total table */
			GMT_memcpy (&z[k], T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return;	/* Done with local */
	GMT_sort_array (GMT, z, info->T->n_records, GMTAPI_DOUBLE);
	p = GMT_quantile (GMT, z, factor[last], info->T->n_records);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T_prev->segment[s]->coord[col][i] = p;
	GMT_free (GMT, z);
}

void table_PSI (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: PSI 1 1 Psi (or Digamma) of A.  */
{
	GMT_LONG s, i;
	double a = 0.0, x[2];
	struct GMT_TABLE *T = S[last]->table[0];

	x[1] = 0.0;	/* No imaginary part */
	if (constant[last]) {
		x[0] = factor[last];
		a = GMT_psi (GMT, x, NULL);
	}
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (!constant[last]) {
			x[0] = T->segment[s]->coord[col][i];
			a = GMT_psi (GMT, x, NULL);
		}
		T->segment[s]->coord[col][i] = a;
	}
}

void table_PVQV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col, GMT_LONG kind)
{	/* kind: 0 = Pv, 1 = Qv */
	GMT_LONG s, i, prev = last - 1, first = last - 2, n, calc;
	double a = 0.0, x = 0.0, nu[2], pq[4];
	static char *name[2] = {"PV", "QV"};
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = (constant[prev]) ? NULL : S[prev]->table[0], *T_first = S[first]->table[0];
				/* last holds the imaginary order vi */
	prev  = last - 1;	/* prev holds the real order vr */
	first = prev - 1;	/* first holds the argument x = cos(colat) */

	calc = !(constant[prev] && constant[last] && constant[first]);	/* Only constant it all args are constant */
	if (!calc) {	/* All constants */
		nu[0] = factor[prev];
		nu[1] = factor[last];
		if ((factor[first] < -1.0 || factor[first] > 1.0)) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, argument to %s outside domain!\n", name[kind]);
		GMT_PvQv (GMT, factor[first], nu, pq, &n);
		a = pq[2*kind];
	}
	if (constant[prev]) nu[0] = factor[prev];
	if (constant[last]) nu[1] = factor[last];
	if (constant[first])    x = factor[first];
	kind *= 2;
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (calc){
			if (!constant[prev]) nu[0] = T_prev->segment[s]->coord[col][i];
			if (!constant[last]) nu[1] = T->segment[s]->coord[col][i];
			if (!constant[first])    x = T_first->segment[s]->coord[col][i];
			GMT_PvQv (GMT, x, nu, pq, &n);
			a = pq[kind];
		}
		T_first->segment[s]->coord[col][i] = a;
	}
}

void table_PV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: PV 3 1 Legendre function Pv(A) of degree v = real(B) + imag(C).  */
{
	table_PVQV (GMT, info, S, constant, factor, last, col, 0);
}

void table_QV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: QV 3 1 Legendre function Qv(A) of degree v = real(B) + imag(C).  */
{
	table_PVQV (GMT, info, S, constant, factor, last, col, 1);
}

void table_R2 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: R2 2 1 R2 = A^2 + B^2.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0!\n");
	if (constant[prev]) factor[prev] *= factor[prev];
	if (constant[last]) factor[last] *= factor[last];
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i] * T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i] * T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = a + b;
	}
}

void table_R2D (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: R2D 1 1 Convert Radians to Degrees.  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = factor[last] * R2D;
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : T->segment[s]->coord[col][i] * R2D;
}

void table_RAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: RAND 2 1 Uniform random values between A and B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a = 0.0, b = 0.0;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev]) a = factor[prev];
	if (constant[last]) b = factor[last];
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (!constant[prev]) a = T_prev->segment[s]->coord[col][i];
		if (!constant[last]) b = T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = a + GMT_rand (GMT) * (b - a);
	}
}

void table_RINT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: RINT 1 1 rint (A) (nearest integer).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = rint (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : rint (T->segment[s]->coord[col][i]);
}

void table_ROTT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ROTT 2 1 Rotate A by the (constant) shift B in the t-direction.  */
{
	GMT_LONG s, i, j, k, shift, prev = last - 1;
	double *z = NULL;
	struct GMT_TABLE *T_prev = S[prev]->table[0];

	if (!constant[last]) {
		GMT_report (GMT, GMT_MSG_FATAL, "T-shift must be a constant in ROTT (no rotation performed)\n");
		return;
	}
	shift = irint (factor[last] / info->t_inc);
	if (constant[prev] || !shift) return;	/* Easy, constant or no shift */
	if (!info->local) {
		if (shift < 0) shift += info->T->n_records;		/* Same thing */
		z = GMT_memory (GMT, NULL, info->T->n_records, double);
	}
	for (s = k = 0; s < info->T->n_segments; s++)  {
		if (info->local) {
			shift = irint (factor[last] / info->t_inc);
			if (shift < 0) shift += info->T->segment[s]->n_rows;		/* Same thing */
			z = GMT_memory (GMT, NULL, info->T->segment[s]->n_rows, double);
		}

		for (i = 0; i < info->T->segment[s]->n_rows; i++) {
			j = (info->local) ? (i+shift)%info->T->segment[s]->n_rows : (k+shift)%info->T->n_records;
			z[j] = T_prev->segment[s]->coord[col][i];
		}
		if (info->local) {
			GMT_memcpy (T_prev->segment[s]->coord[col], z, info->T->segment[s]->n_rows, double);
			GMT_free (GMT, z);
		}
	}
	if (info->local) return;	/* Done with local */
	for (s = k = 0; s < info->T->n_segments; s++, k += info->T->segment[s]->n_rows) GMT_memcpy (T_prev->segment[s]->coord[col], &z[k], info->T->segment[s]->n_rows, double);
	GMT_free (GMT, z);
}

void table_SEC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: SEC 1 1 sec (A) (A in radians).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = (1.0 / cos (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : (1.0 / cos (T->segment[s]->coord[col][i]));
}

void table_SECD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: SECD 1 1 sec (A) (A in degrees).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = (1.0 / cosd (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : (1.0 / cosd (T->segment[s]->coord[col][i]));
}

void table_SIGN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: SIGN 1 1 sign (+1 or -1) of A.  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand == 0!\n");
	if (constant[last]) a = copysign (1.0, factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : copysign (1.0, T->segment[s]->coord[col][i]);
}

void table_SIN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: SIN 1 1 sin (A) (A in radians).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = sin (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : sin (T->segment[s]->coord[col][i]);
}

void table_SINC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: SINC 1 1 sinc (A) (sin (pi*A)/(pi*A)).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_sinc (GMT, factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_sinc (GMT, T->segment[s]->coord[col][i]);
}

void table_SIND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: SIND 1 1 sin (A) (A in degrees).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = sind (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : sind (T->segment[s]->coord[col][i]);
}

void table_SINH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: SINH 1 1 sinh (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = sinh (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : sinh (T->segment[s]->coord[col][i]);
}

void table_SKEW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: SKEW 1 1 Skewness of A.  */
{
	GMT_LONG s, i, n = 0;
	double mean = 0.0, sum2 = 0.0, skew = 0.0, delta;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = GMT->session.d_NaN;
		return;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) {n = 0; mean = sum2 = skew = 0.0; }	/* Start anew for each segment */
		for (i = 0; i < info->T->segment[s]->n_rows; i++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][i])) continue;
			n++;
			delta = T->segment[s]->coord[col][i] - mean;
			mean += delta / n;
			sum2 += delta * (T->segment[s]->coord[col][i] - mean);
		}
		if (info->local) {
			if (n > 1) {
				for (i = 0; i < info->T->segment[s]->n_rows; i++) {
					if (GMT_is_dnan (T->segment[s]->coord[col][i])) continue;
					delta = T->segment[s]->coord[col][i] - mean;
					skew += pow (delta, 3.0);
				}
				sum2 /= (n - 1);
				skew /= n * pow (sum2, 1.5);
			}
			else
				skew = GMT->session.d_NaN;
			for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = skew;
		}
	}
	if (info->local) return;	/* Done with local */
	if (n > 1) {
		for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][i])) continue;
			delta = T->segment[s]->coord[col][i] - mean;
			skew += pow (delta, 3.0);
		}
		sum2 /= (n - 1);
		skew /= n * pow (sum2, 1.5);
	}
	else
		skew = GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = skew;
}

void table_SQR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: SQR 1 1 A^2.  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = factor[last] * factor[last];
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : T->segment[s]->coord[col][i] *  T->segment[s]->coord[col][i];
}

void table_SQRT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: SQRT 1 1 sqrt (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && factor[last] < 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one < 0!\n");
	if (constant[last]) a = sqrt (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : sqrt (T->segment[s]->coord[col][i]);
}

void table_STD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: STD 1 1 Standard deviation of A.  */
{
	GMT_LONG s, i, n = 0;
	double mean = 0.0, sum2 = 0.0, delta;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) GMT_memset (T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
		return;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) {n = 0; mean = sum2 = 0.0;}	/* Start anew for each segment */
		for (i = 0; i < info->T->segment[s]->n_rows; i++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][i])) continue;
			n++;
			delta = T->segment[s]->coord[col][i] - mean;
			mean += delta / n;
			sum2 += delta * (T->segment[s]->coord[col][i] - mean);
		}
		if (info->local) {
			sum2 = (n > 1) ? sqrt (sum2 / (n - 1)) : 0.0;
			for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = sum2;
		}
	}
	if (info->local) return;	/* Done with local */
	sum2 = (n > 1) ? sqrt (sum2 / (n - 1)) : 0.0;
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = sum2;
}

void table_STEP (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: STEP 1 1 Heaviside step function H(A).  */
{
	GMT_LONG s, i;
	double a;
	struct GMT_TABLE *T = S[last]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		if (a == 0.0)
			T->segment[s]->coord[col][i] = 0.5;
		else
			T->segment[s]->coord[col][i] = (a < 0.0) ? 0.0 : 1.0;
	}
}

void table_STEPT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: STEPT 1 1 Heaviside step function H(t-A).  */
{
	GMT_LONG s, i;
	double a;
	struct GMT_TABLE *T = S[last]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = info->T->segment[s]->coord[COL_T][i] - ((constant[last]) ? factor[last] : T->segment[s]->coord[col][i]);
		if (a == 0.0)
			T->segment[s]->coord[col][i] = 0.5;
		else
			T->segment[s]->coord[col][i] = (a < 0.0) ? 0.0 : 1.0;
	}
}

void table_SUB (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: SUB 2 1 A - B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = a - b;
	}
}

void table_SUM (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: SUM 1 1 Cumulative sum of A.  */
{
	GMT_LONG s, i;
	double a = 0.0, sum = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = factor[last];
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) sum = 0.0;	/* Reset for each segment */
		for (i = 0; i < info->T->segment[s]->n_rows; i++) {
			if (!constant[last]) a = T->segment[s]->coord[col][i];
			if (!GMT_is_dnan (a)) sum += a;
			T->segment[s]->coord[col][i] = sum;
		}
	}
}

void table_TAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: TAN 1 1 tan (A) (A in radians).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = tan (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : tan (T->segment[s]->coord[col][i]);
}

void table_TAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: TAND 1 1 tan (A) (A in degrees).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = tand (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : tand (T->segment[s]->coord[col][i]);
}

void table_TANH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: TANH 1 1 tanh (A).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = tanh (factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : tanh (T->segment[s]->coord[col][i]);
}

void table_TN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: TN 2 1 Chebyshev polynomial Tn(-1<A<+1) of degree B.  */
{
	GMT_LONG s, i, prev = last - 1, n;
	double a;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		n = irint ((constant[last]) ? factor[last] : T->segment[s]->coord[col][i]);
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		GMT_chebyshev (GMT, a, n, &T_prev->segment[s]->coord[col][i]);
	}
}

void table_TCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: TCRIT 2 1 Critical value for Student's t-distribution, with alpha = A and n = B.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a, b;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0 for TCRIT!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for TCRIT!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = (constant[last]) ? factor[last] : T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = GMT_tcrit (GMT, a, b);
	}
}

void table_TDIST (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: TDIST 2 1 Student's t-distribution A(t,n), with t = A, and n = B.  */
{
	GMT_LONG s, i, b, prev = last - 1;
	double a;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0 for TDIST!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for TDIST!\n");
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		a = (constant[prev]) ? factor[prev] : T_prev->segment[s]->coord[col][i];
		b = irint ((constant[last]) ? factor[last] : T->segment[s]->coord[col][i]);
		(void) GMT_student_t_a (GMT, a, b, &T_prev->segment[s]->coord[col][i]);
	}
}

void table_UPPER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: UPPER 1 1 The highest (maximum) value of A.  */
{
	GMT_LONG s, i;
	double high = -DBL_MAX;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = factor[last];
		return;
	}

	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) high = -DBL_MAX;
		for (i = 0; i < info->T->segment[s]->n_rows; i++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][i])) continue;
			if (T->segment[s]->coord[col][i] > high) high = T->segment[s]->coord[col][i];
		}
		if (info->local) for (i = 0; i < info->T->segment[s]->n_rows; i++) if (!GMT_is_dnan (T->segment[s]->coord[col][i])) T->segment[s]->coord[col][i] = high;
	}
	if (info->local) return;	/* DOne with local */
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) if (!GMT_is_dnan (T->segment[s]->coord[col][i])) T->segment[s]->coord[col][i] = high;
}

void table_XOR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: XOR 2 1 B if A == NaN, else A.  */
{
	GMT_LONG s, i, prev = last - 1;
	double a = 0.0, b = 0.0;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[prev]) a = factor[prev];
	if (constant[last]) b = factor[last];
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (!constant[prev]) a = T_prev->segment[s]->coord[col][i];
		if (!constant[last]) b = T->segment[s]->coord[col][i];
		T_prev->segment[s]->coord[col][i] = (GMT_is_dnan (a)) ? b : a;
	}
}

void table_Y0 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: Y0 1 1 Bessel function of A (2nd kind, order 0).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand = 0 for Y0!\n");
	if (constant[last]) a = y0 (fabs (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : y0 (fabs (T->segment[s]->coord[col][i]));
}

void table_Y1 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: Y1 1 1 Bessel function of A (2nd kind, order 1).  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand = 0 for Y1!\n");
	if (constant[last]) a = y1 (fabs (factor[last]));
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : y1 (fabs (T->segment[s]->coord[col][i]));
}

void table_YN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: YN 2 1 Bessel function of A (2nd kind, order B).  */
{
	GMT_LONG s, i, prev = last - 1, order = 0, simple = FALSE;
	double b = 0.0;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	if (constant[last] && factor[last] < 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order < 0 for YN!\n");
	if (constant[last] && fabs (rint(factor[last]) - factor[last]) > GMT_SMALL) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order not an integer for YN!\n");
	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, argument = 0 for YN!\n");
	if (constant[last]) order = irint (fabs (factor[last]));
	if (constant[last] && constant[prev]) {
		b = yn ((int)order, fabs (factor[prev]));
		simple = TRUE;
	}
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) {
		if (simple)
			T_prev->segment[s]->coord[col][i] = b;
		else {
			if (!constant[last]) order = irint (fabs (T->segment[s]->coord[col][i]));
			T_prev->segment[s]->coord[col][i] = yn ((int)order, fabs (T_prev->segment[s]->coord[col][i]));
		}
	}
}

void table_ZCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ZCRIT 1 1 Critical value for the normal-distribution, with alpha = A.  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_zcrit (GMT, factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_zcrit (GMT, T->segment[s]->coord[col][i]);
}

void table_ZDIST (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ZDIST 1 1 Cumulative normal-distribution C(x), with x = A.  */
{
	GMT_LONG s, i;
	double a = 0.0;
	struct GMT_TABLE *T = S[last]->table[0];

	if (constant[last]) a = GMT_zdist (GMT, factor[last]);
	for (s = 0; s < info->T->n_segments; s++) for (i = 0; i < info->T->segment[s]->n_rows; i++) T->segment[s]->coord[col][i] = (constant[last]) ? a : GMT_zdist (GMT, T->segment[s]->coord[col][i]);
}

void table_ROOTS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMT_DATASET *S[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG col)
/*OPERATOR: ROOTS 2 1 Treats col A as f(t) = 0 and returns its roots.  */
{
	GMT_LONG s, i, prev = last - 1;
	double *roots = NULL;
	struct GMT_TABLE *T = (constant[last]) ? NULL : S[last]->table[0], *T_prev = S[prev]->table[0];

	/* Treats the chosen column (at there is only one) as f(t) and solves for t that makes f(t) == 0.
	 * For now we only solve using a linear spline but in the future this should depend on the users
	 * choice of INTERPOLANT. */

	if (info->roots_found) return;	/* Already been here */
	if (!constant[last]) {
		GMT_report (GMT, GMT_MSG_FATAL, "Argument to operator ROOTS must be a constant: the column number. Reset to 0\n");
		info->r_col = 0;
	}
	else
		info->r_col = irint (factor[last]);
	if (info->r_col < 0 || info->r_col >= info->n_col) {
		GMT_report (GMT, GMT_MSG_FATAL, "Argument to operator ROOTS must be a column number 0 < col < %ld. Reset to 0\n", info->n_col);
		info->r_col = 0;
	}
	roots = GMT_memory (GMT, NULL, T->n_records, double);
	info->n_roots = 0;
	if (T_prev->segment[0]->coord[info->r_col][0] == 0.0) roots[info->n_roots++] = info->T->segment[0]->coord[COL_T][0];
	for (s = 0; s < info->T->n_segments; s++) {
		for (i = 1; i < info->T->segment[s]->n_rows; i++) {
			if (T_prev->segment[s]->coord[info->r_col][i] == 0.0) {
				roots[info->n_roots++] = info->T->segment[s]->coord[COL_T][i];
				continue;
			}

			if ((T_prev->segment[s]->coord[info->r_col][i-1] * T_prev->segment[s]->coord[info->r_col][i]) < 0.0) {	/* Crossing 0 */
				roots[info->n_roots] = info->T->segment[s]->coord[COL_T][i-1] - T_prev->segment[s]->coord[info->r_col][i-1] * (info->T->segment[s]->coord[COL_T][i] - info->T->segment[s]->coord[COL_T][i-1]) / (T_prev->segment[s]->coord[info->r_col][i] - T_prev->segment[s]->coord[info->r_col][i-1]);
				info->n_roots++;
			}
		}
	}
	for (i = 0; i < info->n_roots; i++) T_prev->segment[0]->coord[info->r_col][i] = roots[i];
	GMT_free (GMT, roots);
	info->roots_found = TRUE;
}

/* ---------------------- end operator functions --------------------- */

#include "gmtmath.h"

#define Free_Hash { for (i = 0; i < GMTMATH_N_OPERATORS; i++) { p = localhashnode[i].next; while ((current = p)) { p = p->next; GMT_free (GMT, current); } } }
#define Free_Stack { for (i = 0; i < GMTMATH_STACK_SIZE; i++) if (alloc_mode[i] == 2) GMT_Destroy_Data (API, GMT_ALLOCATED, &stack[i]); else if (alloc_mode[i] == 1) GMT_free_dataset (GMT, &stack[i]); }
#define Free_Misc {if (T_in) GMT_Destroy_Data (API, GMT_ALLOCATED, &T_in); GMT_free_dataset (GMT, &Template); GMT_free_dataset (GMT, &Time); if (read_stdin) GMT_Destroy_Data (API, GMT_ALLOCATED, &D_stdin); }
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return1(code) {GMT_Destroy_Options (API, &list); Free_gmtmath_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code); }
#define Return(code) {GMT_Destroy_Options (API, &list); Free_gmtmath_Ctrl (GMT, Ctrl); Free_Hash; Free_Stack; Free_Misc;  GMT_end_module (GMT, GMT_cpy); bailout (code); }

GMT_LONG decode_gmt_argument (struct GMT_CTRL *GMT, char *txt, double *value, struct GMT_HASH *H) {
	GMT_LONG expect, i, check = GMT_IS_NAN, possible_number = FALSE;
	char copy[GMT_TEXT_LEN256];
	char *mark = NULL;
	double tmp = 0.0;

	if (!txt) return (GMTMATH_ARG_IS_BAD);

	/* Check if argument is operator */

	if ((i = GMT_hash_lookup (GMT, txt, H, GMTMATH_N_OPERATORS, GMTMATH_N_OPERATORS)) >= GMTMATH_ARG_IS_OPERATOR) return (i);

	/* Next look for symbols with special meaning */

	if (!(strcmp (txt, "STDIN"))) return GMTMATH_ARG_IS_FILE;	/* read from stdin */
	if (!(strcmp (txt, "PI") && strcmp (txt, "pi"))) return GMTMATH_ARG_IS_PI;
	if (!(strcmp (txt, "E") && strcmp (txt, "e"))) return GMTMATH_ARG_IS_E;
	if (!strcmp (txt, "EULER")) return GMTMATH_ARG_IS_EULER;
	if (!strcmp (txt, "TMIN")) return GMTMATH_ARG_IS_TMIN;
	if (!strcmp (txt, "TMAX")) return GMTMATH_ARG_IS_TMAX;
	if (!strcmp (txt, "TINC")) return GMTMATH_ARG_IS_TINC;
	if (!strcmp (txt, "N")) return GMTMATH_ARG_IS_N;
	if (!(strcmp (txt, "T") && strcmp (txt, "t"))) return GMTMATH_ARG_IS_T_MATRIX;
	if (!(strcmp (txt, "Tn") && strcmp (txt, "tn"))) return GMTMATH_ARG_IS_t_MATRIX;

	/* Preliminary test-conversion to a number */

	strcpy (copy, txt);
	if (!GMT_not_numeric (GMT, copy)) {	/* Only check if we are not sure this is NOT a number */
		expect = (strchr (copy, 'T')) ? GMT_IS_ABSTIME : GMT_IS_UNKNOWN;	/* Watch out for dateTclock-strings */
		check = GMT_scanf (GMT, copy, expect, &tmp);
		possible_number = TRUE;
	}

	/* Determine if argument is file. Remove possible question mark. */

	mark = strchr (copy, '?');
	if (mark) *mark = '\0';
	if (!GMT_access (GMT, copy, R_OK)) {	/* Yes it is a file */
		if (check != GMT_IS_NAN && possible_number) GMT_report (GMT, GMT_MSG_FATAL, "Warning: Your argument %s is both a file and a number.  File is selected\n", txt);
		return GMTMATH_ARG_IS_FILE;
	}

	if (check != GMT_IS_NAN) {	/* OK it is a number */
		*value = tmp;
		return GMTMATH_ARG_IS_NUMBER;
	}

	if (txt[0] == '-') {	/* Probably a bad commandline option */
		GMT_report (GMT, GMT_MSG_FATAL, "Error: Option %s not recognized\n", txt);
		return (GMTMATH_ARG_IS_BAD);
	}

	GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: %s is not a number, operator or file name\n", txt);
	return (GMTMATH_ARG_IS_BAD);	/* Dummy return to satisfy some compilers */
}

GMT_LONG GMT_gmtmath (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG i, j, k, kk, op = 0, nstack = 0, new_stack = -1, use_t_col = 0, status, n_macros;
	GMT_LONG consumed_operands[GMTMATH_N_OPERATORS], produced_operands[GMTMATH_N_OPERATORS];
	GMT_LONG n_records, n_rows = 0, n_columns = 0, n_segments, seg, alloc_mode[GMTMATH_STACK_SIZE];
	GMT_LONG constant[GMTMATH_STACK_SIZE], error = FALSE, set_equidistant_t = FALSE, dim[4] = {1, 1, 0, 0};
	GMT_LONG read_stdin = FALSE, t_check_required = TRUE, got_t_from_file = FALSE, done;

	double factor[GMTMATH_STACK_SIZE], t_noise = 0.0, value, off, scale, special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_N+1];

	char *outfile = CNULL;
#include "gmtmath_op.h"

	PFV call_operator[GMTMATH_N_OPERATORS];

	struct GMT_DATASET *stack[GMTMATH_STACK_SIZE], *A_in = NULL, *D_stdin = NULL, *D_in = NULL;
	struct GMT_DATASET *T_in = NULL, *Template = NULL, *Time = NULL, *R = NULL;
	struct GMT_TABLE *rhs = NULL, *D = NULL, *I = NULL;
	struct GMT_HASH *p = NULL, *current = NULL, localhashnode[GMTMATH_N_OPERATORS];
	struct GMT_OPTION *opt = NULL, *list = NULL, *ptr = NULL;
	struct GMTMATH_INFO info;
	struct MATH_MACRO *M = NULL;
	struct GMTMATH_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_gmtmath_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_gmtmath_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_gmtmath", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-Vbf:", "ghios>" GMT_OPT("HMm"), options)) Return1 (API->error);
	Ctrl = New_gmtmath_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtmath_parse (API, Ctrl, options))) Return1 (error);

	/*---------------------------- This is the gmtmath main code ----------------------------*/

	n_macros = gmt_load_macros (GMT, ".gmtmath", &M);	/* Load in any macros */
	if (n_macros) GMT_report (GMT, GMT_MSG_NORMAL, "Found and loaded %ld user macros.\n", n_macros);
	
	if (Ctrl->Q.active || Ctrl->S.active) GMT->current.io.multi_segments[GMT_OUT] = FALSE;	/* Turn off segment headers in calculator or one-record mode */

	/* Internally replace the = [file] sequence with a single output option ->file */

	for (i = 0, opt = options; opt; opt = opt->next) {
		if (opt->option == GMTAPI_OPT_INFILE && !strcmp (opt->arg, "=")) {	/* Found the output sequence */
			if (opt->next) {	/* opt->next->arg may be NULL if stdout is implied */
				ptr = GMT_Make_Option (API, GMTAPI_OPT_OUTFILE, opt->next->arg);
				opt = opt->next;	/* Now we must skip that option */
			}
			else	/* Standard output */
				ptr = GMT_Make_Option (API, GMTAPI_OPT_OUTFILE, NULL);
		}
		else if (opt->option == GMTAPI_OPT_INFILE && (k = gmt_find_macro (opt->arg, n_macros, M)) != GMTAPI_NOTSET) {
			/* Add in the replacement commands from the macro */
			for (kk = 0; kk < M[k].n_arg; kk++) {
				ptr = GMT_Make_Option (API, GMTAPI_OPT_INFILE, M[k].arg[kk]);
				if ((list = GMT_Append_Option (API, ptr, list)) == NULL) Return1 (EXIT_FAILURE);
			}
			continue;
		}
		else
			ptr = GMT_Make_Option (API, opt->option, opt->arg);

		if (ptr == NULL || (list = GMT_Append_Option (API, ptr, list)) == NULL) Return1 (EXIT_FAILURE);
		if (ptr->option == GMTAPI_OPT_OUTFILE) i++;
	}
	gmt_free_macros (GMT, n_macros, &M);

	if (i != 1) {
		GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: No output destination specified or implied\n");
		Return1 (EXIT_FAILURE);
	}
	
	GMT_memset (&info, 1, struct GMTMATH_INFO);

	t_check_required = !Ctrl->T.notime;	/* Turn off default GMT NaN-handling in t column */

	GMT_hash_init (GMT, localhashnode, operator, GMTMATH_N_OPERATORS, GMTMATH_N_OPERATORS);

	GMT_memset (constant, GMTMATH_STACK_SIZE, GMT_LONG);
	GMT_memset (factor, GMTMATH_STACK_SIZE, double);
	GMT_memset (alloc_mode, GMTMATH_STACK_SIZE, GMT_LONG);
	GMT_memset (stack, GMTMATH_STACK_SIZE, struct GMT_DATASET *);

	GMT->current.io.skip_if_NaN[GMT_X] = GMT->current.io.skip_if_NaN[GMT_Y] = FALSE;	/* Turn off default GMT NaN-handling of x/y (e.g. lon/lat columns) */
	GMT->current.io.skip_if_NaN[Ctrl->N.tcol] = t_check_required;	/* Determines if the t-column may have NaNs */

	/* Because of how gmtmath works we do not use GMT_Init_IO to register inputfiles */
	
	/* Read the first file we encounter so we may allocate space */

	/* Check sanity of all arguments and also look for an input file to get t from */
	for (opt = list, got_t_from_file = 0; got_t_from_file == 0 && opt; opt = opt->next) {
		if (!(opt->option == GMTAPI_OPT_INFILE))	continue;	/* Skip command line options and output */
		/* Filenames,  operators, some numbers and = will all have been flagged as files by the parser */
		status = decode_gmt_argument (GMT, opt->arg, &value, localhashnode);	/* Determine what this is */
		if (status == GMTMATH_ARG_IS_BAD) Return (EXIT_FAILURE);		/* Horrible */
		if (status != GMTMATH_ARG_IS_FILE) continue;				/* Skip operators and numbers */
		if (!got_t_from_file) {
			if (!strcmp (opt->arg, "STDIN")) {	/* Special stdin name.  We store this input in a special struct since we may need it again and it can only be read once! */
				if ((D_stdin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_STREAM, GMT_IS_POINT, NULL, 0, NULL, NULL)) == NULL) {
					Return (API->error);
				}
				read_stdin = TRUE;
				D_in = D_stdin;
				I = D_stdin->table[0];
			}
			else {
				if ((D_in = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, opt->arg, NULL)) == NULL) {
					Return (API->error);
				}
			}
			got_t_from_file = 1;
		}
	}
	if (D_in) {	/* Read a file, update columns */
		use_t_col  = Ctrl->N.tcol;
		n_segments = D_in->n_segments;
		n_columns  = D_in->n_columns;
	}
	set_equidistant_t = (Ctrl->T.active && !Ctrl->T.file && !Ctrl->T.notime);	/* We were given -Tmin/max/inc */
	if (D_in && set_equidistant_t) {
		GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Cannot use -T when data files are specified\n");
		Return (EXIT_FAILURE);
	}
	if (Ctrl->A.active) {	/* Always stored as t and f(t) in cols 0 and 1 */
		if (D_in) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Cannot have data files when -A is specified\n");
			Return (EXIT_FAILURE);
		}
		if ((A_in = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, Ctrl->A.file, NULL)) == NULL) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading file %s\n", Ctrl->A.file);
			Return (API->error);
		}
		rhs = A_in->table[0];	/* Only one table */
		if (rhs->n_columns != 2) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: -A must take a file with 2 (t,f(t)) columns\n");
			Return (EXIT_FAILURE);
		}
	}
	if (Ctrl->Q.active) {	/* Shorthand for -N1/0 -T0/0/1 -Ca */
		Ctrl->N.ncol = 1;
		Ctrl->N.tcol = 0;
		Ctrl->N.active = set_equidistant_t = TRUE;
		Ctrl->T.min = Ctrl->T.max = 0.0;
		Ctrl->T.inc = 1.0;
		n_rows = n_columns = 1;
	}
	if (Ctrl->N.active) {
		GMT->current.io.skip_if_NaN[GMT_X] = GMT->current.io.skip_if_NaN[GMT_Y] = FALSE;
		GMT->current.io.skip_if_NaN[Ctrl->N.tcol] = TRUE;
		n_columns = Ctrl->N.ncol;
	}
	if (Ctrl->T.active && Ctrl->T.file) {	/* Gave a file that we will use to obtain the T vector only */
		if (D_in) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Cannot use -T when data files are specified\n");
			Return (EXIT_FAILURE);
		}
		if ((T_in = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, Ctrl->T.file, NULL)) == NULL) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error reading file %s\n", Ctrl->T.file);
			Return (API->error);
		}
		use_t_col = 0;
		got_t_from_file = 2;
	}

	if (set_equidistant_t && !Ctrl->Q.active) {
		if (Ctrl->T.mode == 1) {	/* Got n instead of t_inc, use it to reset t_inc */
			Ctrl->T.inc = (Ctrl->T.max - Ctrl->T.min) / (Ctrl->T.inc - 1.0);
		}
		/* Make sure the min/man/inc values harmonize */
		switch (GMT_minmaxinc_verify (GMT, Ctrl->T.min, Ctrl->T.max, Ctrl->T.inc, GMT_SMALL)) {
			case 1:
				GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -T options: (max - min) is not a whole multiple of inc\n");
				Return (EXIT_FAILURE);
				break;
			case 2:
				if (Ctrl->T.inc != 1.0) {	/* Allow for somebody explicitly saying -T0/0/1 */
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -T options: (max - min) is <= 0\n");
					Return (EXIT_FAILURE);
				}
				break;
			case 3:
				GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -T options: inc is <= 0\n");
				Return (EXIT_FAILURE);
				break;
			default:	/* OK */
				break;
		}

		n_rows = irint ((Ctrl->T.max - Ctrl->T.min) / Ctrl->T.inc) + 1;
		n_columns = Ctrl->N.ncol;
	}

	if (Ctrl->A.active) {	/* Get number of rows and time from the file, but not n_cols (that takes -N, which defaults to 2) */
		n_rows = rhs->n_records;
		n_columns = Ctrl->N.ncol;
		Ctrl->T.min = rhs->min[0];
		Ctrl->T.max = rhs->max[1];
		Ctrl->T.inc = (rhs->max[1] - rhs->min[0]) / (rhs->n_records - 1);
	}
	if (Ctrl->T.file) n_columns = Ctrl->N.ncol;

	if (!(D_in || T_in || A_in || set_equidistant_t)) {	/* Neither a file nor -T given; must read data from stdin */
		GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Expression must contain at least one table file or -T [and -N]\n");
		Return (EXIT_FAILURE);
	}
	if (D_in)	/* Obtained file structure from an input file, use this to create new stack entry */
		Template = GMT_alloc_dataset (GMT, D_in, n_columns, 0, GMT_ALLOC_NORMAL);
	else {		/* Must use -N -T etc to create single segment */
		dim[2] = n_columns;	dim[3] = n_rows;
		if ((Template = GMT_Create_Data (API, GMT_IS_DATASET, dim)) == NULL) Return (GMT_MEMORY_ERROR);
	}
	alloc_mode[0] = 1;	/* Allocated locally */
	Ctrl->N.ncol = n_columns;
	if (!Ctrl->T.notime && n_columns > 1) Ctrl->C.cols[Ctrl->N.tcol] = (Ctrl->Q.active) ? FALSE : TRUE;
	
	/* Create the Time data structure with 2 cols: 0 is t, 1 is normalized tn */
	if (D_in) {	/* Either D_in or D_stdin */
		Time = GMT_alloc_dataset (GMT, D_in, 2, 0, GMT_ALLOC_NORMAL);
		info.T = Time->table[0];	D = D_in->table[0];
		for (seg = 0, done = FALSE; seg < D->n_segments; seg++) {
			GMT_memcpy (info.T->segment[seg]->coord[0], D->segment[seg]->coord[use_t_col], D->segment[seg]->n_rows, double);
			if (!done) {
				for (i = 1; i < info.T->segment[seg]->n_rows && (GMT_is_dnan (info.T->segment[seg]->coord[0][i-1]) || GMT_is_dnan (info.T->segment[seg]->coord[0][i])); i++);	/* Find the first real two records in a row */
				Ctrl->T.inc = (i == info.T->segment[seg]->n_rows) ? GMT->session.d_NaN : info.T->segment[seg]->coord[0][i] - info.T->segment[seg]->coord[0][i-1];
				t_noise = fabs (GMT_SMALL * Ctrl->T.inc);
			}
			for (i = 1; i < info.T->segment[seg]->n_rows && !info.irregular; i++) if (fabs (fabs (info.T->segment[seg]->coord[0][i] - info.T->segment[seg]->coord[0][i-1]) - fabs (Ctrl->T.inc)) > t_noise) info.irregular = TRUE;
		}
		if (!read_stdin && GMT_Destroy_Data (API, GMT_ALLOCATED, &D_in) != GMT_OK) {
			Return (API->error);
		}
	}
	else {	/* Create orderly output */
		dim[2] = 2;	dim[3] = n_rows;
		if ((Time = GMT_Create_Data (API, GMT_IS_DATASET, dim)) == NULL) Return (GMT_MEMORY_ERROR);
		info.T = Time->table[0];
		for (i = 0; i < info.T->segment[0]->n_rows; i++) info.T->segment[0]->coord[0][i] = (i == (info.T->segment[0]->n_rows-1)) ? Ctrl->T.max: Ctrl->T.min + i * Ctrl->T.inc;
		t_noise = fabs (GMT_SMALL * Ctrl->T.inc);
	}

	for (seg = n_records = 0; seg < info.T->n_segments; seg++) {	/* Create normalized times and possibly reverse time (-I) */
		off = 0.5 * (info.T->segment[seg]->coord[0][info.T->segment[seg]->n_rows-1] + info.T->segment[seg]->coord[0][0]);
		scale = 2.0 / (info.T->segment[seg]->coord[0][info.T->segment[seg]->n_rows-1] - info.T->segment[seg]->coord[0][0]);
		if (Ctrl->I.active) for (i = 0; i < info.T->segment[seg]->n_rows/2; i++) d_swap (info.T->segment[seg]->coord[0][i], info.T->segment[seg]->coord[0][info.T->segment[seg]->n_rows-1-i]);	/* Reverse time-series */
		for (i = 0; i < info.T->segment[seg]->n_rows; i++) info.T->segment[seg]->coord[1][i] = (info.T->segment[seg]->coord[0][i] - off) * scale;
		n_records += info.T->segment[seg]->n_rows;
	}
	info.t_min = Ctrl->T.min;	info.t_max = Ctrl->T.max;	info.t_inc = Ctrl->T.inc;
	info.n_col = n_columns;		info.local = Ctrl->L.active;
	GMT_set_tbl_minmax (GMT, info.T);

	if (Ctrl->A.active) {
		load_column (stack[0], n_columns-1, rhs, 1);	/* Put the r.h.s of the Ax = b equation in the last column of the item on the stack */
		GMT_set_tbl_minmax (GMT, stack[0]->table[0]);
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &A_in) != GMT_OK) {
			Return (API->error);
		}
		nstack = 1;
	}
	else
		nstack = 0;

	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_PI] = M_PI;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_E] = M_E;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_EULER] = M_EULER;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_TMIN] = Ctrl->T.min;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_TMAX] = Ctrl->T.max;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_TINC] = Ctrl->T.inc;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_N] = (double)n_records;

	gmtmath_init (call_operator, consumed_operands, produced_operands);

	for (opt = list, error = FALSE; !error && opt; opt = opt->next) {

		/* First check if we should skip optional arguments */

		if (strchr ("AINQSTVbfghios" GMT_OPT("FHMm"), opt->option)) continue;
		if (opt->option == 'C') {	/* Change affected columns */
			decode_columns (GMT, opt->arg, Ctrl->C.cols, n_columns, Ctrl->N.tcol);
			continue;
		}
		if (opt->option == GMTAPI_OPT_OUTFILE) continue;	/* We do output after the loop */

		op = decode_gmt_argument (GMT, opt->arg, &value, localhashnode);

		if (op != GMTMATH_ARG_IS_FILE && !GMT_access (GMT, opt->arg, R_OK)) GMT_message (GMT, "Warning: The number or operator %s may be confused with an existing file %s!\n", opt->arg, opt->arg);

		if (op < GMTMATH_ARG_IS_OPERATOR) {	/* File name or factor */

			if (nstack == GMTMATH_STACK_SIZE) {	/* Stack overflow */
				GMT_report (GMT, GMT_MSG_FATAL, "Stack overflow!\n");
				Return (EXIT_FAILURE);
			}

			if (op == GMTMATH_ARG_IS_NUMBER) {
				constant[nstack] = TRUE;
				factor[nstack] = value;
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "%g ", factor[nstack]);
				nstack++;
				continue;
			}
			else if (op <= GMTMATH_ARG_IS_PI && op >= GMTMATH_ARG_IS_N) {
				constant[nstack] = TRUE;
				factor[nstack] = special_symbol[GMTMATH_ARG_IS_PI-op];
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "%g ", factor[nstack]);
				nstack++;
				continue;
			}

			/* Here we need a matrix */

			constant[nstack] = FALSE;

			if (op == GMTMATH_ARG_IS_T_MATRIX) {	/* Need to set up matrix of t-values */
				if (Ctrl->T.notime) {
					GMT_report (GMT, GMT_MSG_FATAL, "T is not defined for plain data files!\n");
					Return (EXIT_FAILURE);
				}
				if (!stack[nstack]) {
					stack[nstack] = GMT_alloc_dataset (GMT, Template, n_columns, 0, GMT_ALLOC_NORMAL);
					alloc_mode[nstack] = 1;
				}
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "T ");
				for (j = 0; j < n_columns; j++) load_column (stack[nstack], j, info.T, COL_T);
				GMT_set_tbl_minmax (GMT, stack[nstack]->table[0]);
			}
			else if (op == GMTMATH_ARG_IS_t_MATRIX) {	/* Need to set up matrix of normalized t-values */
				if (Ctrl->T.notime) {
					GMT_report (GMT, GMT_MSG_FATAL, "Tn is not defined for plain data files!\n");
					Return (EXIT_FAILURE);
				}
				if (!stack[nstack]) {
					stack[nstack] = GMT_alloc_dataset (GMT, Template, n_columns, 0, GMT_ALLOC_NORMAL);
					alloc_mode[nstack] = 1;
				}
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "Tn ");
				for (j = 0; j < n_columns; j++) load_column (stack[nstack], j, info.T, COL_TN);
				GMT_set_tbl_minmax (GMT, stack[nstack]->table[0]);
			}
			else if (op == GMTMATH_ARG_IS_FILE) {		/* Filename given */
				if (!strcmp (opt->arg, "STDIN")) {	/* stdin file */
					if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "<stdin> ");
					if (!stack[nstack]) {
						stack[nstack] = GMT_alloc_dataset (GMT, Template, n_columns, 0, GMT_ALLOC_NORMAL);
						alloc_mode[nstack] = 1;
					}
					for (j = 0; j < n_columns; j++) load_column (stack[nstack], j, I, j);
					GMT_set_tbl_minmax (GMT, stack[nstack]->table[0]);
				}
				else {
					if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "%s ", opt->arg);
					if ((stack[nstack] = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, opt->arg, NULL)) == NULL) {
						GMT_report (GMT, GMT_MSG_FATAL, "Error reading file %s\n", opt->arg);
						Return (API->error);
					}
					alloc_mode[nstack] = 2;
				}
				if (!same_size (stack[nstack], Template)) {
					GMT_report (GMT, GMT_MSG_FATAL, "tables not of same size!\n");
					Return (EXIT_FAILURE);
				}
				else if (!(Ctrl->T.notime || same_domain (stack[nstack], Ctrl->N.tcol, info.T))) {
					GMT_report (GMT, GMT_MSG_FATAL, "tables do not cover the same domain!\n");
					Return (EXIT_FAILURE);
				}
			}
			nstack++;
			continue;
		}

		/* Here we have an operator */

		if (!strncmp (opt->arg, "ROOTS", (size_t)5) && !(opt->next && opt->next->arg[0] == '=')) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Only = may follow operator ROOTS\n");
			Return (EXIT_FAILURE);
		}

		if ((new_stack = nstack - consumed_operands[op] + produced_operands[op]) >= GMTMATH_STACK_SIZE) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Stack overflow (%s)\n", opt->arg);
			Return (EXIT_FAILURE);
		}

		if (nstack < consumed_operands[op]) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Operation \"%s\" requires %ld operands\n", operator[op], consumed_operands[op]);
			Return (EXIT_FAILURE);
		}

		if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "%s ", operator[op]);

		for (i = produced_operands[op] - consumed_operands[op]; i > 0; i--) {
			if (stack[nstack+i-1])	continue;

			/* Must make space for more */

			stack[nstack+i-1] = GMT_alloc_dataset (GMT, Template, n_columns, 0, GMT_ALLOC_NORMAL);
			alloc_mode[nstack+i-1] = 1;
		}

		/* If operators operates on constants only we may have to make space as well */

		for (j = 0, i = nstack - consumed_operands[op]; j < produced_operands[op]; j++, i++) {
			if (constant[i] && !stack[i]) {
				stack[i] = GMT_alloc_dataset (GMT, Template, n_columns, 0, GMT_ALLOC_NORMAL);
				alloc_mode[i] = 1;
			}
		}

		if (!strcmp (operator[op], "LSQFIT")) {	/* Special case, solve LSQ system and return */
			solve_LSQFIT (GMT, &info, stack[nstack-1], n_columns, Ctrl->C.cols, outfile);
			Return (EXIT_SUCCESS);
		}

		for (j = 0; j < n_columns; j++) {
			if (Ctrl->C.cols[j]) continue;
			(*call_operator[op]) (GMT, &info, stack, constant, factor, nstack - 1, j);	/* Do it */
		}

		nstack = new_stack;

		for (i = 1; i <= produced_operands[op]; i++) if (stack[nstack-i]) constant[nstack-i] = FALSE;	/* Now filled with table */
	}

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		(outfile) ? GMT_message (GMT, "= %s", outfile) : GMT_message (GMT,  "= <stdout>");
	}

	if (new_stack < 0 && constant[0]) {	/* Only a constant provided, set table accordingly */
		if (!stack[0]) {
			stack[0] = GMT_alloc_dataset (GMT, Template, n_columns, 0, GMT_ALLOC_NORMAL);
			alloc_mode[0] = 1;
		}
		for (j = 0; j < n_columns; j++) {
			if (j == COL_T && !Ctrl->Q.active)
				load_column (stack[0], j, info.T, COL_T);
			else
				load_const_column (stack[0], j, factor[0]);
		}
		GMT_set_tbl_minmax (GMT, stack[0]->table[0]);
	}

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "\n");

	if (info.roots_found) {	/* Special treatment of root finding */
		struct GMT_LINE_SEGMENT *S = stack[0]->table[0]->segment[0];
		GMT_LONG dim[4] = {1, 1, 1, 0};
		
		dim[3] = info.n_roots;
		if ((R = GMT_Create_Data (API, GMT_IS_DATASET, dim)) == NULL) Return (API->error)
		for (i = 0; i < info.n_roots; i++) R->table[0]->segment[0]->coord[GMT_X][i] = S->coord[info.r_col][i];
		if (GMT_Write_Data (API, GMT_IS_DATASET, (Ctrl->Out.file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_POINT, NULL, stack[0]->io_mode, Ctrl->Out.file, R) != GMT_OK) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, GMT_ALLOCATED, &R) != GMT_OK) {
			Return (API->error);
		}
	}
	else {	/* Regular table result */
		GMT_LONG template_used = FALSE;
		if (stack[0])	/* There is an output stack, select it */
			R = stack[0];
		else {		/* Can happen if only -T [-N] was specified with no operators */
			R = Template;
			template_used = TRUE;
			if (Ctrl->N.tcol < R->n_columns) {
				load_column (R, Ctrl->N.tcol, info.T, COL_T);	/* Put T in the time column of the item on the stack if possible */
				GMT_set_tbl_minmax (GMT, R->table[0]);
			}
		}
		if (Ctrl->S.active) {	/* Only get one record */
			GMT_LONG nr, r, c, row;
			struct GMT_DATASET *N = NULL;
			nr = (Ctrl->S.active) ? 1 : 0;
			N = GMT_alloc_dataset (GMT, R, n_columns, nr, GMT_ALLOC_NORMAL);
			for (seg = 0; seg < R->table[0]->n_segments; seg++) {
				for (r = 0; r < N->table[0]->segment[seg]->n_rows; r++) {
					row = (Ctrl->S.active) ? ((Ctrl->S.mode == -1) ? 0 : R->table[0]->segment[seg]->n_rows - 1) : r;
					for (c = 0; c < n_columns; c++) N->table[0]->segment[seg]->coord[c][r] = R->table[0]->segment[seg]->coord[c][row];
				}
			}
			if (GMT_Write_Data (API, GMT_IS_DATASET, (Ctrl->Out.file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_POINT, NULL, N->io_mode, Ctrl->Out.file, N) != GMT_OK) {
				Return (API->error);
			}
			if (GMT_Destroy_Data (API, GMT_ALLOCATED, &N) != GMT_OK) {
				Return (API->error);
			}
		}
		else {	/* Write the whole enchilada */
			if (R != Template) alloc_mode[0] = 2;	/* Since GMT_Write_Data will register it */
			if (GMT_Write_Data (API, GMT_IS_DATASET, (Ctrl->Out.file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_POINT, NULL, R->io_mode, Ctrl->Out.file, R) != GMT_OK) {
				Return (API->error);
			}
		}
		if (template_used) Template = NULL;	/* This prevents it from being freed twice (once from API registration via GMT_Write_Data and then again in Free_Misc) */
	}

	/* Clean-up time */

	if (nstack > 1) GMT_report (GMT, GMT_MSG_FATAL, "Warning: %ld more operands left on the stack!\n", nstack-1);

	Return (EXIT_SUCCESS);
}
