/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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

#define THIS_MODULE_NAME	"gmtmath"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Reverse Polish Notation (RPN) calculator for data tables"
#define THIS_MODULE_KEYS	"<DI,ADi,>DO"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>Vbdfghios" GMT_OPT("HMm")

EXTERN_MSC int gmt_load_macros (struct GMT_CTRL *GMT, char *mtype, struct MATH_MACRO **M);
EXTERN_MSC int gmt_find_macro (char *arg, unsigned int n_macros, struct MATH_MACRO *M);
EXTERN_MSC void gmt_free_macros (struct GMT_CTRL *GMT, unsigned int n_macros, struct MATH_MACRO **M);
EXTERN_MSC struct GMT_OPTION * gmt_substitute_macros (struct GMT_CTRL *GMT, struct GMT_OPTION *options, char *mfile);
	
#define GMTMATH_ARG_IS_OPERATOR	 0
#define GMTMATH_ARG_IS_FILE	-1
#define GMTMATH_ARG_IS_NUMBER	-2
#define GMTMATH_ARG_IS_PI	-3
#define GMTMATH_ARG_IS_E	-4
#define GMTMATH_ARG_IS_F_EPS	-5
#define GMTMATH_ARG_IS_D_EPS	-6
#define GMTMATH_ARG_IS_EULER	-7
#define GMTMATH_ARG_IS_TMIN	-8
#define GMTMATH_ARG_IS_TMAX	-9
#define GMTMATH_ARG_IS_TRANGE	-10
#define GMTMATH_ARG_IS_TINC	-11
#define GMTMATH_ARG_IS_N	-12
#define GMTMATH_ARG_IS_J_MATRIX	-13
#define GMTMATH_ARG_IS_T_MATRIX	-14
#define GMTMATH_ARG_IS_t_MATRIX	-15
#define GMTMATH_ARG_IS_STORE	-50
#define GMTMATH_ARG_IS_RECALL	-51
#define GMTMATH_ARG_IS_CLEAR	-52
#define GMTMATH_ARG_IS_BAD	-99

#define GMTMATH_STACK_SIZE	100
#define GMTMATH_STORE_SIZE	100

#define GMTMATH_STORE_CMD	"STO@"
#define GMTMATH_RECALL_CMD	"RCL@"
#define GMTMATH_CLEAR_CMD	"CLR@"

#define COL_T	0	/* These are the first 3 columns in the Time structure */
#define COL_TN	1
#define COL_TJ	2

#define GMTMATH_COEFFICIENTS	0
#define GMTMATH_EVALUATE	1
#define GMTMATH_WEIGHTS		1
#define GMTMATH_SIGMAS		2

#define DOUBLE_BIT_MASK (~(1023ULL << 54ULL))	/* This will be 00000000 00111111 11111111 .... and sets to 0 anything larger than 2^53 which is max integer in double */

struct GMTMATH_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct Out {	/* = <filename> */
		bool active;
		char *file;
	} Out;
	struct A {	/* -A[-]<t_f(t).d>[+e][+w|s] */
		bool active;
		bool null;
		unsigned int e_mode;	/* 0 save coefficients, 1 save predictions and residuals */
		unsigned int w_mode;	/* 0 no weights, 1 = got weights, 2 = got sigmas */
		char *file;
	} A;
	struct C {	/* -C<cols> */
		bool active;
		bool *cols;
	} C;
	struct E {	/* -E<min_eigenvalue> */
		bool active;
		double eigen;
	} E;
	struct I {	/* -I */
		bool active;
	} I;
	struct L {	/* -L */
		bool active;
	} L;
	struct N {	/* -N<n_col>/<t_col> */
		bool active;
		uint64_t ncol, tcol;
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
	struct S {	/* -S[f|l] */
		bool active;
		int mode;	/* -1 or +1 */
	} S;
	struct T {	/* -T[<tmin/tmax/t_inc>[+]] | -T<file> */
		bool active;
		bool notime;
		double min, max, inc;
		char *file;
	} T;
};

struct GMTMATH_INFO {
	bool irregular;	/* true if t_inc varies */
	bool roots_found;	/* true if roots have been solved for */
	bool local;		/* Per segment operation (true) or global operation (false) */
	bool notime;		/* No time-array avaible for operators who depend on that */
	unsigned int n_roots;	/* Number of roots found */
	unsigned int fit_mode;	/* Used for {LSQ|SVD}FIT */
	unsigned int w_mode;	/* Used for weighted fit */
	uint64_t r_col;	/* The column used to find roots */
	uint64_t n_col;	/* Number of columns */
	double t_min, t_max, t_inc;
	struct GMT_DATATABLE *T;	/* Table with all time information */
};

struct GMTMATH_STACK {
	struct GMT_DATASET *D;		/* The dataset */
	bool constant;			/* true if a constant (see factor) and S == NULL */
	double factor;			/* The value if constant is true */
	unsigned int alloc_mode;	/* 0 is not allocated yet, 1 is allocated locally in this program, 2 = allocated via API */
};

struct GMTMATH_STORED {
	char *label;	/* Name of this stored memory */
	struct GMTMATH_STACK stored;
};

void *New_gmtmath_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTMATH_CTRL *C = GMT_memory (GMT, NULL, 1, struct GMTMATH_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->C.cols = GMT_memory (GMT, NULL, GMT_MAX_COLUMNS, bool);
	C->E.eigen = 1e-7;	/* Default cutoff of small eigenvalues */
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

bool decode_columns (char *txt, bool *skip, uint64_t n_col, uint64_t t_col)
{
	uint64_t i, start, stop;
	unsigned int pos;
	char p[GMT_BUFSIZ];

	/* decode_columns is used to handle the parsing of -C<cols>.  */
	
	if (!txt[0]) {	/* Reset to default */
		for (i = 0; i < n_col; i++) skip[i] = false;
		skip[t_col] = true;
	}
	else if (txt[0] == 'r' && txt[1] == '\0') {	/* Reverse all settings */
		for (i = 0; i < n_col; i++) skip[i] = !skip[i];
	}
	else if (txt[0] == 'a') {	/* Select all columns */
		for (i = 0; i < n_col; i++) skip[i] = false;
	}
	else {	/* Set the selected columns */
		for (i = 0; i < n_col; i++) skip[i] = true;
		pos = 0;
		while ((GMT_strtok (txt, ",", &pos, p))) {
			if (strchr (p, '-'))
				sscanf (p, "%" PRIu64 "-%" PRIu64, &start, &stop);
			else {
				sscanf (p, "%" PRIu64, &start);
				stop = start;
			}
			stop = MIN (stop, n_col-1);
			for (i = start; i <= stop; i++) skip[i] = false;
		}
	}
	return (!skip[t_col]);	/* Returns true if we are changing the time column */
}

int gmtmath_find_stored_item (struct GMTMATH_STORED *recall[], int n_stored, char *label)
{	/* Linear search to find the named storage item */
	int k = 0;
	while (k < n_stored && strcmp (recall[k]->label, label)) k++;
	return (k == n_stored ? -1 : k);
}

void load_column (struct GMT_DATASET *to, uint64_t to_col, struct GMT_DATATABLE *from, uint64_t from_col)
{	/* Copies data from one column to another */
	uint64_t seg;
	for (seg = 0; seg < from->n_segments; seg++) {
		GMT_memcpy (to->table[0]->segment[seg]->coord[to_col], from->segment[seg]->coord[from_col], from->segment[seg]->n_rows, double);
	}
}

/* ---------------------- start convenience functions --------------------- */

int solve_LS_system (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S, uint64_t n_col, bool skip[], \
	char *file, bool svd, double eigen_min, struct GMT_OPTION *options, struct GMT_DATASET *A)
{
	/* Consider the current table the augmented matrix [A | b], making up the linear system Ax = b.
	 * We will set up the normal equations, solve for x, and output the solution before quitting.
	 * This function is special since it operates across columns and returns n_col scalars.
	 * We try to solve this positive definite & symmetric matrix with Cholesky methods; if that fails
	 * we do a full SVD decomposition and set small eigenvalues to zero, yielding an approximate solution.
	 * However, if svd == true then we do the full SVD.
	 */

	unsigned int i, j, k, k0, i2, j2, n;
	int ier;
	uint64_t row, seg, rhs, w_col = 0;
	double cond, w = 1.0;
	double *N = NULL, *r = NULL, *d = NULL, *x = NULL;
	struct GMT_DATATABLE *T = S->D->table[0];
	struct GMT_DATASET *D = NULL;

	for (i = n = 0; i < n_col; i++) if (!skip[i]) n++;	/* Need to find how many active columns we have */
	if (n < 2) {
		char *pre[2] = {"LSQ", "SVD"};
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error, %sFIT requires at least 2 active columns!\n", pre[svd]);
		return (EXIT_FAILURE);
	}
	rhs = n_col - 1;
	while (rhs > 0 && skip[rhs]) rhs--;	/* Get last active col number as the rhs vector b */
	n--;					/* Account for b, the rhs vector, to get row & col dimensions of normal matrix N */
	if (info->w_mode) {
		w_col = rhs - 1;		/* If there are weights, this is the column they are stored in */
		while (w_col > 0 && skip[w_col]) w_col--;	/* Get next active col number as the weight vector w */
		n--;					/* Account for w, the rhs vector, to get row & col dimensions of normal matrix N */
	}

	N = GMT_memory (GMT, NULL, n*n, double);
	r = GMT_memory (GMT, NULL, T->n_records, double);

#if 0
	fprintf (stderr, "Printout of A | b matrix\n");
	fprintf (stderr, "------------------------------------------------------------------\n");
	for (seg = 0; seg < info->T->n_segments; seg++) {
		for (row = 0; row < T->segment[seg]->n_rows; row++) {
			for (j = 0; j < rhs; j++) {
				if (skip[j]) continue;
				fprintf (stderr, "%g\t", T->segment[seg]->coord[j][row]);
			}
			fprintf (stderr, "|\t%g\n", T->segment[seg]->coord[rhs][row]);
		}
	}
	fprintf (stderr, "------------------------------------------------------------------\n");
#endif
	/* Here we build A^T*W*A*x = A^T*W*b ==> N*x = r, where W is the diagonal matrix with squared weights w */
	/* Do the row & col dot products, skipping inactive columns as we go along */
	for (j = j2 = 0; j < n; j2++) {	/* j2 is table column, j is row in N matrix */
		if (skip[j2]) continue;
		for (i = i2 = 0; i < n; i2++) {	/* i2 is table column, i is column in N matrix */
			if (skip[i2]) continue;
			k0 = j * n + i;
			N[k0] = 0.0;
			for (seg = k = 0; seg < info->T->n_segments; seg++) {
				for (row = 0; row < T->segment[seg]->n_rows; row++, k++) {
					if (info->w_mode) {
						w = pow (T->segment[seg]->coord[w_col][row], 2.0);
						if (info->w_mode == GMTMATH_SIGMAS) w = 1.0 / w;	/* Got sigma */
					}
					N[k0] += w * T->segment[seg]->coord[j2][row] * T->segment[seg]->coord[i2][row];
				}
			}
			i++;
		}
		r[j] = 0.0;
		for (seg = k = 0; seg < info->T->n_segments; seg++) {
			for (row = 0; row < T->segment[seg]->n_rows; row++, k++) {
				if (info->w_mode) {
					w = pow (T->segment[seg]->coord[w_col][row], 2.0);
					if (info->w_mode == GMTMATH_SIGMAS) w = 1.0 / w;	/* Got sigma */
				}
				r[j] += w * T->segment[seg]->coord[j2][row] * T->segment[seg]->coord[rhs][row];
			}
		}
		j++;
	}
	if (svd)
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Solve LS system via SVD decomposition and exclude eigenvalues < %g.\n", eigen_min);
	else
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Solve LS system via Cholesky decomposition\n");

#if 0
	fprintf (stderr, "Printout of N and r matrix\n");
	fprintf (stderr, "------------------------------------------------------------------\n");
	for (j = 0; j < n; j++) {
		for (k = 0; k < n; k++)
			fprintf (stderr, "%g\t", N[j*n+k]);
		fprintf (stderr, "\n");
	}
	for (k = 0; k < n; k++)
		fprintf (stderr, "%g\n", r[k]);
	fprintf (stderr, "------------------------------------------------------------------\n");
#endif
	d = GMT_memory (GMT, NULL, n, double);
	x = GMT_memory (GMT, NULL, n, double);
	if (svd || ((ier = GMT_chol_dcmp (GMT, N, d, &cond, n, n) ) != 0)) {	/* Cholesky decomposition failed, use SVD method, or use SVD if specified */
		unsigned int nrots;
		double *b = NULL, *z = NULL, *v = NULL, *lambda = NULL;
		if (!svd) {
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Cholesky decomposition failed, try SVD decomposition instead and exclude eigenvalues < %g.\n", eigen_min);
			GMT_chol_recover (GMT, N, d, n, n, ier, true);	/* Restore to former matrix N */
		}
		/* Solve instead using the SVD of a square matrix via GMT_jacobi */
		lambda = GMT_memory (GMT, NULL, n, double);
		b = GMT_memory (GMT, NULL, n, double);
		z = GMT_memory (GMT, NULL, n, double);
		v = GMT_memory (GMT, NULL, n*n, double);

		if (GMT_jacobi (GMT, N, n, n, lambda, v, b, z, &nrots)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Eigenvalue routine failed to converge in 50 sweeps.\n");
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "The solution might be inaccurate.\n");
		}
		/* Solution x = v * lambda^-1 * v' * r */

		/* First do d = V' * r, so x = v * lambda^-1 * d */
		for (j = 0; j < n; j++) for (k = 0, d[j] = 0.0; k < n; k++) d[j] += v[j*n+k] * r[k];
		/* Then do d = lambda^-1 * d by setting small lambda's to zero */
		for (j = k = 0; j < n; j++) {
			if (lambda[j] < eigen_min) {
				d[j] = 0.0;
				k++;
			}
			else
				d[j] /= lambda[j];
		}
		if (k) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "%d eigenvalues < %g set to zero to yield a stable solution\n", k, eigen_min);

		/* Finally do x = v * d */
		for (j = 0; j < n; j++) for (k = 0; k < n; k++) x[j] += v[k*n+j] * d[k];

		GMT_free (GMT, b);
		GMT_free (GMT, z);
		GMT_free (GMT, v);
		GMT_free (GMT, lambda);
	}
	else {	/* Decomposition worked, now solve system */
		GMT_chol_solv (GMT, N, x, r, n, n);
	}

	if (info->fit_mode == GMTMATH_COEFFICIENTS) {	/* Return coefficients only as a single vector */
		uint64_t dim[4] = {1, 1, 0, 1};
		dim[GMT_ROW] = n;
		if ((D = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (GMT->parent->error);
		for (k = 0; k < n; k++) D->table[0]->segment[0]->coord[GMT_X][k] = x[k];
		GMT_Set_Comment (GMT->parent, GMT_IS_DATASET, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, D);
		if (GMT->common.h.add_colnames) {
			char header[GMT_BUFSIZ] = {""};
			sprintf (header, "#coefficients");
			if (GMT_Set_Comment (GMT->parent, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, D)) return (GMT->parent->error);
		}
		if (GMT_Write_Data (GMT->parent, GMT_IS_DATASET, (file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_NONE, 0, NULL, file, D) != GMT_OK) {
			return (GMT->parent->error);
		}
#if 0
		if (GMT_Destroy_Data (GMT->parent, &D) != GMT_OK) {
			return (GMT->parent->error);
		}
#endif
	}
	else {	/* Return t, y, p(t), r(t), where p(t) is the predicted solution and r(t) is the residuals */
		double value;
		k = (unsigned int)S->D->dim[GMT_COL];
		S->D->dim[GMT_COL] = (info->w_mode) ? 5 : 4;	/* State we want a different set of columns on output */
		D = GMT_Duplicate_Data (GMT->parent, GMT_IS_DATASET, GMT_DUPLICATE_ALLOC, S->D);	/* Same table length as S->D, but with up to n_cols columns (lon, lat, dist, g1, g2, ...) */
		S->D->dim[GMT_COL] = k;	/* Reset the original columns */
		if (D->table[0]->n_segments > 1) GMT_set_segmentheader (GMT, GMT_OUT, true);	/* More than one segment triggers -mo */
		load_column (D, 0, info->T, COL_T);	/* Place the time-column in first ouput column */
		for (seg = k = 0; seg < info->T->n_segments; seg++) {
			for (row = 0; row < T->segment[seg]->n_rows; row++, k++) {
				D->table[0]->segment[seg]->coord[1][row] = T->segment[seg]->coord[rhs][row];
				value = 0.0;
				for (j2 = j = 0; j2 < n; j2++) {	/* j2 is table column, j is entry in x vector */
					if (skip[j2]) continue;		/* Not included in the fit */
					value += T->segment[seg]->coord[j2][row] * x[j];	/* Sum up the solution */
					j++;
				}
				D->table[0]->segment[seg]->coord[2][row] = value;
				D->table[0]->segment[seg]->coord[3][row] = T->segment[seg]->coord[rhs][row] - value;
				if (info->w_mode) D->table[0]->segment[seg]->coord[4][row] = T->segment[seg]->coord[w_col][row];
			}
		}
		if (GMT->common.h.add_colnames) {
			char header[GMT_BUFSIZ] = {""};
			sprintf (header, "#t[0]\tobserved(t)[1]\tpredict(t)[2]\tresidual(t)[3]");
			if (info->w_mode == GMTMATH_WEIGHTS) strcat (header, "\tweight(t)[4]");
			else if (info->w_mode == GMTMATH_SIGMAS) strcat (header, "\tsigma(t)[4]");
			if (GMT_Set_Comment (GMT->parent, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, D)) return (GMT->parent->error);
		}
		if (GMT_Write_Data (GMT->parent, GMT_IS_DATASET, (file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_NONE, 0, NULL, file, D) != GMT_OK) {
			return (GMT->parent->error);
		}
	}

	GMT_free (GMT, x);
	GMT_free (GMT, d);
	GMT_free (GMT, N);
	GMT_free (GMT, r);
	return (EXIT_SUCCESS);
}

int solve_LSQFIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S, uint64_t n_col, bool skip[], double eigen, char *file, struct GMT_OPTION *options, struct GMT_DATASET *A)
{
	return (solve_LS_system (GMT, info, S, n_col, skip, file, false, eigen, options, A));
}

int solve_SVDFIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S, uint64_t n_col, bool skip[], double eigen, char *file, struct GMT_OPTION *options, struct GMT_DATASET *A)
{
	return (solve_LS_system (GMT, info, S, n_col, skip, file, true, eigen, options, A));
}

void load_const_column (struct GMT_DATASET *to, uint64_t to_col, double factor)
{	/* Sets all rows in a column to a constant factor */
	uint64_t row, seg;
	for (seg = 0; seg < to->n_segments; seg++) {
		for (row = 0; row < to->table[0]->segment[seg]->n_rows; row++) to->table[0]->segment[seg]->coord[to_col][row] = factor;
	}
}

bool same_size (struct GMT_DATASET *A, struct GMT_DATASET *B)
{	/* Are the two dataset the same size */
	uint64_t seg;
	if (!(A->table[0]->n_segments == B->table[0]->n_segments && A->table[0]->n_columns == B->table[0]->n_columns)) return (false);
	for (seg = 0; seg < A->table[0]->n_segments; seg++) if (A->table[0]->segment[seg]->n_rows != B->table[0]->segment[seg]->n_rows) return (false);
	return (true);
}

bool same_domain (struct GMT_DATASET *A, uint64_t t_col, struct GMT_DATATABLE *B)
{	/* Are the two dataset the same domain */
	uint64_t seg;
	for (seg = 0; seg < A->table[0]->n_segments; seg++) {
		if (!(doubleAlmostEqualZero (A->table[0]->min[t_col], B->min[COL_T])
					&& doubleAlmostEqualZero (A->table[0]->max[t_col], B->max[COL_T])))
			return (false);
	}
	return (true);
}

int GMT_gmtmath_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmtmath [-A[-]<ftable>[+s]] [-C<cols>] [-E<eigen>] [-I] [-L] [-N<n_col>[/<t_col>]] [-Q] [-S[f|l]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T[<t_min>/<t_max>/<t_inc>[+]]] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\tA B op C op ... = [outfile]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_s_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE,
		"\tA, B, etc are table files, constants, or symbols (see below).\n"
		"\tTo read stdin give filename as STDIN (which can appear more than once).\n"
		"\tThe stack can hold up to %d entries (given enough memory).\n", GMTMATH_STACK_SIZE);
	GMT_Message (API, GMT_TIME_NONE,
		"\tTrigonometric operators expect radians unless noted otherwise.\n"
		"\tThe operators and number of input and output arguments:\n\n"
		"\tName       #args   Returns\n"
		"\t--------------------------\n");
#include "gmtmath_explain.h"
	GMT_Message (API, GMT_TIME_NONE,
		"\n\tThe special symbols are:\n\n"
		"\tPI                  = 3.1415926...\n"
		"\tE                   = 2.7182818...\n"
		"\tEULER               = 0.5772156...\n"
		"\tF_EPS (single eps)   = 1.192092896e-07\n"
		"\tD_EPS (double eps)   = 2.2204460492503131e-16\n"
		"\tTMIN, TMAX, TRANGE, or TINC = the corresponding constant.\n"
		"\tN                   = number of records.\n"
		"\tT                   = table with t-coordinates.\n"
		"\tTNORM               = table with normalized [-1 to +1] t-coordinates.\n"
		"\tTROW                = table with row numbers 0, 1, ..., N-1.\n"
		"\n\tUse macros for frequently used long expressions; see the gmtmath man page.\n"
		"\tStore stack to named variable via STO@<label>, recall via [RCL]@<label>, clear via CLR@<label>.\n"
		"\n\tOPTIONS:\n\n"
		"\t-A Set up and solve a linear system A x = b, and return vector x.\n"
		"\t   Requires -N and initializes extended matrix [A | b] from <ftable> holding t and f(t) only.\n"
		"\t   t goes into column <t_col> while f(t) goes into column <n_col> - 1 (i.e., r.h.s. vector b).\n"
		"\t   Use -A-<ftable> to only place f(t) in b and leave A initialized to zeros.\n"
		"\t   No additional data files are read.  Output will be a single column with coefficients.\n"
		"\t   Append +w if 3rd column contains weights and +s if 3rd column contains 1-sigmas.\n"
		"\t   Append +e to evaluate solution and write t, f(t), the solution, residuals[, weight|sigma].\n"
		"\t   Use either LSQFIT or SVDFIT to solve the [weighted] linear system.\n"
		"\t-C Change which columns to operate on [Default is all except time].\n"
		"\t   -C reverts to the default, -Cr toggles current settings, and -Ca selects all columns.\n"
		"\t-E Set minimum eigenvalue used by LSQFIT and SVDFIT [1e-7].\n"
		"\t-I Reverse the output sequence into descending order [ascending].\n"
		"\t-L Apply operators on a per-segment basis [cumulates operations across file].\n"
		"\t-N Set the number of columns and optionally the id of the time column (0 is first) [2/0].\n"
		"\t   If input files are given, -N will add extra columns initialized to zero, if needed.\n"
		"\t-Q Quick scalar calculator. Shorthand for -Ca -N1/0 -T0/0/1.\n"
		"\t-S Only write first row upon completion of calculations [write all rows].\n"
		"\t   Optionally, append l for last row or f for first row [Default].\n"
		"\t-T Set domain from t_min to t_max in steps of t_inc.\n"
		"\t   Append + to t_inc to indicate the number of points instead.\n"
		"\t   If a filename is given instead we read t coordinates from first column.\n"
		"\t   If no domain is given we assume no time, i.e., only data columns are present.\n"
		"\t   This choice also implies -Ca.\n");
	GMT_Option (API, "V,bi,bo,d,f,g,h,i,o,s,.");

	return (EXIT_FAILURE);
}

int GMT_gmtmath_parse (struct GMT_CTRL *GMT, struct GMTMATH_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to gmtmath and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, n_files = 0;
	bool missing_equal = true;
	char *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	int gmt_parse_o_option (struct GMT_CTRL *GMT, char *arg);

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (opt->arg[0] == '=' && opt->arg[1] == 0) {	/* No it was an = [outfile] sequence */
					missing_equal = false;
					opt->option = GMT_OPT_OUTFILE;	/* Prevents further use later */
					if (opt->next && (opt->next->option == GMT_OPT_INFILE || opt->next->option == GMT_OPT_OUTFILE)) {
						Ctrl->Out.active = true;
						if (opt->next->arg[0]) Ctrl->Out.file = strdup (opt->next->arg);
						opt->next->option = GMT_OPT_OUTFILE;	/* Prevents further use later */
					}
				}
				n_files++;
				break;
			case '>':	/* Output file specified via an API; set output file here */
				opt->option = GMT_OPT_OUTFILE;
				if (opt->arg[0] && !Ctrl->Out.file) Ctrl->Out.file = strdup (opt->arg);
				missing_equal = false;
				break;
			case '#':	/* Skip numbers */
				break;
				
			/* Processes program-specific parameters */

			case 'A':	/* y(x) table for LSQFIT/SVDFIT operations */
				Ctrl->A.active = true;	k = 0;
				if (opt->arg[0] == '-') {
					Ctrl->A.null = true;
					k = 1;
				}
				if ((c = strchr (opt->arg, '+')) && strchr ("esw", c[1])) {	/* Got a valid modifier */
					unsigned int pos = 0;
					char p[GMT_LEN256] = {""};
					c[0] = '\0';	/* Temporarily chop off modifiers */
					Ctrl->A.file = strdup (&opt->arg[k]);
					c[0] = '+';	/* Restore the modifier */
					while (GMT_strtok (c, "+", &pos, p)) {
						switch (p[0]) {
							case 'e': Ctrl->A.e_mode = GMTMATH_EVALUATE; break;	/* Evaluate solution */
							case 's': Ctrl->A.w_mode = GMTMATH_SIGMAS;   break;	/* Got t,y,s */
							case 'w': Ctrl->A.w_mode = GMTMATH_WEIGHTS;  break;	/* Got t,y,w */
							default: n_errors++;	break;
						}
					}
				}
				else	/* No modifiers, selected default output of coefficient column */
					Ctrl->A.file = strdup (&opt->arg[k]);
				break;
			case 'C':	/* Processed in the main loop but not here; just skip */
				break;
			case 'E':	/* Set minimum eigenvalue cutoff */
				Ctrl->E.eigen = atof (opt->arg);
				break;
			case 'F':	/* Now obsolete, using -o instead */
				if (GMT_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -F is deprecated; use -o instead\n");
					gmt_parse_o_option (GMT, opt->arg);
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'I':	/* Reverse output order */
				Ctrl->I.active = true;
				break;
			case 'L':	/* Apply operator per segment basis */
				Ctrl->L.active = true;
				break;
			case 'N':	/* Sets no of columns and optionally the time column [0] */
				Ctrl->N.active = true;
				if (sscanf (opt->arg, "%" PRIu64 "/%" PRIu64, &Ctrl->N.ncol, &Ctrl->N.tcol) == 1) Ctrl->N.tcol = 0;
				break;
			case 'Q':	/* Quick for -Ca -N1/0 -T0/0/1 */
				Ctrl->Q.active = true;
				break;
			case 'S':	/* Only want one row (first or last) */
				Ctrl->S.active = true;
				switch (opt->arg[0]) {
					case 'f': case 'F': case '\0':
						Ctrl->S.mode = -1; break;
					case 'l': case 'L':
						Ctrl->S.mode = +1; break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Syntax is -S[f|l]\n");
						n_errors++;
					break;
				}
				break;
			case 'T':	/* Either get a file with time coordinate or a min/max/dt setting */
				Ctrl->T.active = true;
				if (!opt->arg[0])	/* Turn off default GMT NaN-handling in t column */
					Ctrl->T.notime = true;
				else if (!GMT_access (GMT, opt->arg, R_OK))	/* Argument given and file can be opened */
					Ctrl->T.file = strdup (opt->arg);
				else {	/* Presumably gave tmin/tmax/tinc */
					if (sscanf (opt->arg, "%lf/%lf/%lf", &Ctrl->T.min, &Ctrl->T.max, &Ctrl->T.inc) != 3) {
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Unable to decode arguments for -T\n");
						n_errors++;
					}
					if (opt->arg[strlen(opt->arg)-1] == '+') {	/* Gave number of points instead; calculate inc */
						Ctrl->T.inc = (Ctrl->T.max - Ctrl->T.min) / (Ctrl->T.inc - 1.0);
					}
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
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && (Ctrl->N.ncol == 0 || Ctrl->N.tcol >= Ctrl->N.ncol),
		"Syntax error: -N must have positive n_cols and 0 <= t_col < n_col\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

unsigned int gmt_assign_ptrs (struct GMT_CTRL *GMT, unsigned int last, struct GMTMATH_STACK *S[], struct GMT_DATATABLE **T, struct GMT_DATATABLE **T_prev)
{	/* Centralize the assignment of previous stack ID and the current and previous stack tables */
	unsigned int prev;
	if (last == 0) {	/* User error in requesting more items that presently on the stack */
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Fatal error: Not enough items on the stack\n");
		return UINT_MAX;	/* Error flag */
	}
	prev = last - 1;
	*T = (S[last]->constant && !S[last]->D) ? NULL : S[last]->D->table[0];
	*T_prev = S[prev]->D->table[0];
	return prev;
}

/* -----------------------------------------------------------------
 *              Definitions of all operator functions
 * -----------------------------------------------------------------*/
/* Note: The OPERATOR: **** lines are used to extract syntax for documentation */

int table_ABS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ABS 1 1 abs (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand == 0!\n");
	if (S[last]->constant) a = fabs (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : fabs (T->segment[s]->coord[col][row]);
	return 0;
}

int table_ACOS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ACOS 1 1 acos (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, |operand| > 1 for ACOS!\n");
	if (S[last]->constant) a = d_acos (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : d_acos (T->segment[s]->coord[col][row]);
	return 0;
}

int table_ACOSH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ACOSH 1 1 acosh (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand < 1 for ACOSH!\n");
	if (S[last]->constant) a = acosh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : acosh (T->segment[s]->coord[col][row]);
	return 0;
}

int table_ACSC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ACSC 1 1 acsc (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, |operand| > 1 for ACSC!\n");
	if (S[last]->constant) a = d_asin (1.0 / S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : d_asin (1.0 / T->segment[s]->coord[col][row]);
	return 0;
}

int table_ACOT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ACOT 1 1 acot (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, |operand| > 1 for ACOT!\n");
	if (S[last]->constant) a = atan (1.0 / S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : atan (1.0 / T->segment[s]->coord[col][row]);
	return 0;
}

int table_ADD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ADD 2 1 A + B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */
	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = a + b;
	}
	return 0;
}

int table_AND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: AND 2 1 B if A == NaN, else A.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	GMT_UNUSED(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = (GMT_is_dnan (a)) ? b : a;
	}
	return 0;
}

int table_ASEC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ASEC 1 1 asec (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, |operand| > 1 for ASEC!\n");
	if (S[last]->constant) a = d_acos (1.0 / S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : d_acos (1.0 / T->segment[s]->coord[col][row]);
	return 0;
}

int table_ASIN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ASIN 1 1 asin (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, |operand| > 1 for ASIN!\n");
	if (S[last]->constant) a = d_asin (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : d_asin (T->segment[s]->coord[col][row]);
	return 0;
}

int table_ASINH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ASINH 1 1 asinh (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = asinh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : asinh (T->segment[s]->coord[col][row]);
	return 0;
}

int table_ATAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ATAN 1 1 atan (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = atan (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : atan (T->segment[s]->coord[col][row]);
	return 0;
}

int table_ATAN2 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ATAN2 2 1 atan2 (A, B).  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0 for ATAN2!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for ATAN2!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = d_atan2 (a, b);
	}
	return 0;
}

int table_ATANH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ATANH 1 1 atanh (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) >= 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, |operand| >= 1 for ATANH!\n");
	if (S[last]->constant) a = atanh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : atanh (T->segment[s]->coord[col][row]);
	return 0;
}

int table_BCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BCDF 3 1 Binomial cumulative distribution function for p = A, n = B and x = C.  */
{
	unsigned int prev1 = last - 1, prev2 = last - 2;
	uint64_t s, row;
	double p, x, n;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev2]->constant && (S[prev2]->factor < 0.0 || S[prev2]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument p to BCDF must be a 0 <= p <= 1!\n");
		return -1;
	}
	if (S[prev1]->constant && S[prev1]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument n to BCDF must be a positive integer (n >= 0)!\n");
		return -1;
	}
	if (S[last]->constant && S[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument x to BCDF must be a positive integer (x >= 0)!\n");
		return -1;
	}
	if (S[prev2]->constant && S[prev1]->constant && S[last]->constant) {	/* BCDF is given constant arguments */
		double value;
		p = S[prev2]->factor;
		n = lrint (S[prev1]->factor);	x = lrint (S[last]->factor);
		value = GMT_binom_cdf (GMT, x, n, p);
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev2->segment[s]->coord[col][row] = value;
		return 0;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		p = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->coord[col][row];
		n = lrint ((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->coord[col][row]);
		x = lrint ((S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row]);
		T_prev2->segment[s]->coord[col][row] = GMT_binom_cdf (GMT, x, n, p);
		
	}
	return 0;
}

int table_BEI (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BEI 1 1 bei (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_bei (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_bei (GMT, fabs (T->segment[s]->coord[col][row]));
	return 0;
}

int table_BER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BER 1 1 ber (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_ber (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_ber (GMT, fabs (T->segment[s]->coord[col][row]));
	return 0;
}

int table_BPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BPDF 3 1 Binomial probability density function for p = A, n = B and x = C.  */
{
	unsigned int prev1 = last - 1, prev2 = last - 2;
	uint64_t s, row;
	double p, x, n;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev2]->constant && (S[prev2]->factor < 0.0 || S[prev2]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument p to BPDF must be a 0 <= p <= 1!\n");
		return -1;
	}
	if (S[prev1]->constant && S[prev1]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument n to BPDF must be a positive integer (n >= 0)!\n");
		return -1;
	}
	if (S[last]->constant && S[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument x to BPDF must be a positive integer (x >= 0)!\n");
		return -1;
	}
	if (S[prev2]->constant && S[prev1]->constant && S[last]->constant) {	/* BPDF is given constant arguments */
		double value;
		p = S[prev2]->factor;
		n = lrint (S[prev1]->factor);	x = lrint (S[last]->factor);
		value = GMT_binom_pdf (GMT, x, n, p);
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev2->segment[s]->coord[col][row] = value;
		return 0;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		p = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->coord[col][row];
		n = lrint ((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->coord[col][row]);
		x = lrint ((S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row]);
		T_prev2->segment[s]->coord[col][row] = GMT_binom_pdf (GMT, x, n, p);
		
	}
	return 0;
}

int table_BITAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BITAND 2 1 A & B (bitwise AND operator).  */
{
	uint64_t s, row, a = 0, b = 0, n_warn = 0, result, result_trunc;
	unsigned int prev;
	double ad = 0.0, bd = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) ad = S[prev]->factor;
	if (S[last]->constant) bd = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev]->constant) ad = T_prev->segment[s]->coord[col][row];
		if (!S[last]->constant) bd = T->segment[s]->coord[col][row];
		if (GMT_is_dnan (ad) || GMT_is_dnan (bd))	/* Any NaN in bitwise operations results in NaN output */
			T_prev->segment[s]->coord[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;	b = (uint64_t)bd;
			result = a & b;
			result_trunc = result & DOUBLE_BIT_MASK;
			if (result != result_trunc) n_warn++;
			T_prev->segment[s]->coord[col][row] = (double)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITAND resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

int table_BITLEFT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BITLEFT 2 1 A << B (bitwise left-shift operator).  */
{
	uint64_t s, row, a = 0, b = 0, n_warn = 0, result, result_trunc;
	int64_t b_signed;
	unsigned int prev;
	bool first = true;
	double ad = 0.0, bd = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) ad = S[prev]->factor;
	if (S[last]->constant) bd = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev]->constant) ad = T_prev->segment[s]->coord[col][row];
		if (!S[last]->constant) bd = T->segment[s]->coord[col][row];
		if (GMT_is_dnan (ad) || GMT_is_dnan (bd))	/* Any NaN in bitwise operations results in NaN output */
			T_prev->segment[s]->coord[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;	b_signed = (int64_t)bd;
			if (b_signed < 0) {	/* Bad bitshift */
				if (first) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "ERROR: Bit shift must be >= 0; other values yield NaN\n");
				T_prev->segment[s]->coord[col][row] = GMT->session.d_NaN;
				first = false;
			}
			else {		
				b = (uint64_t)b_signed;
				result = a << b;
				result_trunc = result & DOUBLE_BIT_MASK;
				if (result != result_trunc) n_warn++;
				T_prev->segment[s]->coord[col][row] = (double)result_trunc;
			}
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITLEFT resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

int table_BITNOT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BITNOT 1 1 ~A (bitwise NOT operator, i.e., return two's complement).  */
{
	uint64_t s, row, a = 0, n_warn = 0, result, result_trunc;
	double ad = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) ad = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[last]->constant) ad = T->segment[s]->coord[col][row];
		if (GMT_is_dnan (ad))	/* Any NaN in bitwise operations results in NaN output */
			T->segment[s]->coord[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;
			result = ~a;
			result_trunc = result & DOUBLE_BIT_MASK;
			if (result != result_trunc) n_warn++;
			T->segment[s]->coord[col][row] = (double)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITNOT resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

int table_BITOR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BITOR 2 1 A | B (bitwise OR operator).  */
{
	uint64_t s, row, a = 0, b = 0, n_warn = 0, result, result_trunc;
	unsigned int prev;
	double ad = 0.0, bd = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) ad = S[prev]->factor;
	if (S[last]->constant) bd = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev]->constant) ad = T_prev->segment[s]->coord[col][row];
		if (!S[last]->constant) bd = T->segment[s]->coord[col][row];
		if (GMT_is_dnan (ad) || GMT_is_dnan (bd))	/* Any NaN in bitwise operations results in NaN output */
			T_prev->segment[s]->coord[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;	b = (uint64_t)bd;
			result = a | b;
			result_trunc = result & DOUBLE_BIT_MASK;
			if (result != result_trunc) n_warn++;
			T_prev->segment[s]->coord[col][row] = (double)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITOR resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

int table_BITRIGHT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BITRIGHT 2 1 A >> B (bitwise right-shift operator).  */
{
	uint64_t s, row, a = 0, b = 0, n_warn = 0, result, result_trunc;
	int64_t b_signed;
	unsigned int prev;
	bool first = true;
	double ad = 0.0, bd = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) ad = S[prev]->factor;
	if (S[last]->constant) bd = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev]->constant) ad = T_prev->segment[s]->coord[col][row];
		if (!S[last]->constant) bd = T->segment[s]->coord[col][row];
		if (GMT_is_dnan (ad) || GMT_is_dnan (bd))	/* Any NaN in bitwise operations results in NaN output */
			T_prev->segment[s]->coord[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;	b_signed = (int64_t)bd;
			if (b_signed < 0) {	/* Bad bitshift */
				if (first) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "ERROR: Bit shift must be >= 0; other values yield NaN\n");
				T_prev->segment[s]->coord[col][row] = GMT->session.d_NaN;
				first = false;
			}
			else {		
				b = (uint64_t)b_signed;
				result = a >> b;
				result_trunc = result & DOUBLE_BIT_MASK;
				if (result != result_trunc) n_warn++;
				T_prev->segment[s]->coord[col][row] = (double)result_trunc;
			}
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITRIGHT resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

int table_BITTEST (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BITTEST 2 1 1 if bit B of A is set, else 0 (bitwise TEST operator).  */
{
	uint64_t s, row, a = 0, b = 0, n_warn = 0, result, result_trunc;
	int64_t b_signed;
	unsigned int prev;
	bool first = true;
	double ad = 0.0, bd = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) ad = S[prev]->factor;
	if (S[last]->constant) bd = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev]->constant) ad = T_prev->segment[s]->coord[col][row];
		if (!S[last]->constant) bd = T->segment[s]->coord[col][row];
		if (GMT_is_dnan (ad) || GMT_is_dnan (bd))	/* Any NaN in bitwise operations results in NaN output */
			T_prev->segment[s]->coord[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;	b_signed = (int64_t)bd;
			if (b_signed < 0) {	/* Bad bit */
				if (first) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "ERROR: Bit position range for BITTEST is 0-63 (since we are using do); other values yield NaN\n");
				T_prev->segment[s]->coord[col][row] = GMT->session.d_NaN;
				first = false;
			}
			else {
				b = (uint64_t)b_signed;
				b = 1ULL << b;
				result = a & b;
				result_trunc = result & DOUBLE_BIT_MASK;
				if (result != result_trunc) n_warn++;
				T_prev->segment[s]->coord[col][row] = (result_trunc) ? 1.0 : 0.0;
			}
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITTEST resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

int table_BITXOR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BITXOR 2 1 A ^ B (bitwise XOR operator).  */
{
	uint64_t s, row, a = 0, b = 0, n_warn = 0, result, result_trunc;
	unsigned int prev;
	double ad = 0.0, bd = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) ad = S[prev]->factor;
	if (S[last]->constant) bd = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev]->constant) ad = T_prev->segment[s]->coord[col][row];
		if (!S[last]->constant) bd = T->segment[s]->coord[col][row];
		if (GMT_is_dnan (ad) || GMT_is_dnan (bd))	/* Any NaN in bitwise operations results in NaN output */
			T_prev->segment[s]->coord[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;	b = (uint64_t)bd;
			result = a ^ b;
			result_trunc = result & DOUBLE_BIT_MASK;
			if (result != result_trunc) n_warn++;
			T_prev->segment[s]->coord[col][row] = (double)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITXOR resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

int table_CEIL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: CEIL 1 1 ceil (A) (smallest integer >= A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = ceil (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : ceil (T->segment[s]->coord[col][row]);
	return 0;
}

int table_CHICRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: CHICRIT 2 1 Chi-squared distribution critical value for alpha = A and nu = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0 for CHICRIT!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for CHICRIT!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = GMT_chi2crit (GMT, a, b);
	}
	return 0;
}

int table_CHICDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: CHICDF 2 1 Chi-squared cumulative distribution function for chi2 = A and nu = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b, q;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0 for CHICDF!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for CHICDF!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		GMT_chi2 (GMT, a, b, &q);
		T_prev->segment[s]->coord[col][row] = 1.0 - q;
	}
	return 0;
}

int table_CHIPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: CHIPDF 2 1 Chi^2 probability density function for chi = A and nu = B.  */
{
	uint64_t s, row, nu;
	unsigned int prev;
	double c;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		nu = lrint ((S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row]);
		c  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = GMT_chi2_pdf (GMT, c, nu);
	}
	return 0;
}

int table_COL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COL 1 1 Places column A on the stack.  */
{
	uint64_t s, row;
	unsigned int k;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if (gmt_assign_ptrs (GMT, last, S, &T, &T_prev) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (!S[last]->constant || S[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error, argument to COL must be a constant column number (0 <= k < n_col)!\n");
		return -1;
	}
	k = urint (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		T->segment[s]->coord[col][row] = T_prev->segment[s]->coord[k][row];
	}
	return 0;
}

int table_COMB (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COMB 2 1 Combinations n_C_r, with n = A and r = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument n to COMB must be a positive integer (n >= 0)!\n");
		return -1;
	}
	if (S[last]->constant && S[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument r to COMB must be a positive integer (r >= 0)!\n");
		return -1;
	}
	if (S[prev]->constant && S[last]->constant) {	/* COMBO is given constant args */
		double value = GMT_combination (GMT, irint(S[prev]->factor), irint(S[last]->factor));
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->coord[col][row] = value;
		return 0;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = GMT_combination (GMT, irint(a), irint(b));
	}
	return 0;
}

int table_CORRCOEFF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: CORRCOEFF 2 1 Correlation coefficient r(A, B).  */
{
	uint64_t s, row, i;
	unsigned int prev;
	double *a, *b, coeff;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[last]->constant) {	/* Correlation is undefined */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->coord[col][row] = GMT->session.d_NaN;
		return 0;
	}

	if (!info->local) {	/* Compute correlation across the entire table */
		a = GMT_memory (GMT, NULL, info->T->n_records, double);
		b = GMT_memory (GMT, NULL, info->T->n_records, double);
		for (s = i = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++, i++) a[i] = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		for (s = i = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++, i++) b[i] = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		coeff = GMT_corrcoeff (GMT, a, b, info->T->n_records, 0);
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->coord[col][row] = coeff;
		return 0;
	}
	/* Local, or per-segment calculations */
	for (s = 0; s < info->T->n_segments; s++) {
		if (S[prev]->constant) {		/* Must create the missing (constant) column */
			a = GMT_memory (GMT, NULL, info->T->segment[s]->n_rows, double);
			for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) a[row] = S[prev]->factor;
			b = T->segment[s]->coord[col];
		}
		else if (S[last]->constant) {	/* Must create the missing (constant) column */
			a = T_prev->segment[s]->coord[col];
			b = GMT_memory (GMT, NULL, info->T->segment[s]->n_rows, double);
			for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) b[row] = S[last]->factor;
		}
		else {
			a = T_prev->segment[s]->coord[col];
			b = T->segment[s]->coord[col];
		}
		coeff = GMT_corrcoeff (GMT, a, b, info->T->segment[s]->n_rows, 0);
		for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->coord[col][row] = coeff;
		if (S[prev]->constant) GMT_free (GMT, a);
		if (S[last]->constant) GMT_free (GMT, b);
	}
	return 0;
}


int table_COS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COS 1 1 cos (A) (A in radians).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = cos (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		T->segment[s]->coord[col][row] = (S[last]->constant) ? a : cos (T->segment[s]->coord[col][row]);
	}
	return 0;
}

int table_COSD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COSD 1 1 cos (A) (A in degrees).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = cosd (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : cosd (T->segment[s]->coord[col][row]);
	return 0;
}

int table_COSH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COSH 1 1 cosh (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = cosh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : cosh (T->segment[s]->coord[col][row]);
	return 0;
}

int table_COT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COT 1 1 cot (A) (A in radians).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = (1.0 / tan (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		T->segment[s]->coord[col][row] = (S[last]->constant) ? a : (1.0 / tan (T->segment[s]->coord[col][row]));
	}
	return 0;
}


int table_COTD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COTD 1 1 cot (A) (A in degrees).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = (1.0 / tand (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		T->segment[s]->coord[col][row] = (S[last]->constant) ? a : (1.0 / tand (T->segment[s]->coord[col][row]));
	}
	return 0;
}

int table_CSC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: CSC 1 1 csc (A) (A in radians).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = (1.0 / sin (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		T->segment[s]->coord[col][row] = (S[last]->constant) ? a : (1.0 / sin (T->segment[s]->coord[col][row]));
	}
	return 0;
}

int table_CSCD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: CSCD 1 1 csc (A) (A in degrees).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = (1.0 / sind (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		T->segment[s]->coord[col][row] = (S[last]->constant) ? a : (1.0 / sind (T->segment[s]->coord[col][row]));
	}
	return 0;
}

int table_PCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PCDF 2 1 Poisson cumulative distribution function for x = A and lambda = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for PCDF!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		GMT_poisson_cdf (GMT, a, b, &T_prev->segment[s]->coord[col][row]);
	}
	return 0;
}

int table_DDT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: DDT 1 1 d(A)/dt Central 1st derivative.  */
{
	uint64_t s, row;
	double c, left, next_left;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	/* Central 1st difference in t */

	if (info->irregular) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning, DDT called on irregularly spaced data (not supported)!\n");
	if (S[last]->constant) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand to DDT is constant!\n");

	c = 0.5 / info->t_inc;
	for (s = 0; s < info->T->n_segments; s++) {
		row = 0;
		while (row < info->T->segment[s]->n_rows) {	/* Process each segment */
			next_left = 2.0 * T->segment[s]->coord[col][row] - T->segment[s]->coord[col][row+1];
			while (row < info->T->segment[s]->n_rows - 1) {
				left = next_left;
				next_left = T->segment[s]->coord[col][row];
				T->segment[s]->coord[col][row] = (S[last]->constant) ? 0.0 : c * (T->segment[s]->coord[col][row+1] - left);
				row++;
			}
			T->segment[s]->coord[col][row] = (S[last]->constant) ? 0.0 : 2.0 * c * (T->segment[s]->coord[col][row] - next_left);
			row++;
		}
	}
	return 0;
}

int table_D2DT2 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: D2DT2 1 1 d^2(A)/dt^2 2nd derivative.  */
{
	uint64_t s, row;
	double c, left, next_left;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	/* Central 2nd difference in t */

	if (info->irregular) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning, D2DT2 called on irregularly spaced data (not supported)!\n");
	if (S[last]->constant) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand to D2DT2 is constant!\n");

	c = 1.0 / (info->t_inc * info->t_inc);
	for (s = 0; s < info->T->n_segments; s++) {
		row = 0;
		while (row < info->T->segment[s]->n_rows) {	/* Process each segment */
			next_left = T->segment[s]->coord[col][row];
			T->segment[s]->coord[col][row] = (GMT_is_dnan (T->segment[s]->coord[col][row]) || GMT_is_dnan (T->segment[s]->coord[col][row+1])) ? GMT->session.d_NaN : 0.0;
			row++;
			while (row < info->T->segment[s]->n_rows - 1) {
				left = next_left;
				next_left = T->segment[s]->coord[col][row];
				T->segment[s]->coord[col][row] = (S[last]->constant) ? 0.0 : c * (T->segment[s]->coord[col][row+1] - 2 * T->segment[s]->coord[col][row] + left);
				row++;
			}
			T->segment[s]->coord[col][row] = (GMT_is_dnan (T->segment[s]->coord[col][row]) || GMT_is_dnan (T->segment[s]->coord[col][row-1])) ? GMT->session.d_NaN : 0.0;
			row++;
		}
	}
	return 0;
}

int table_D2R (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: D2R 1 1 Converts Degrees to Radians.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = S[last]->factor * D2R;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : T->segment[s]->coord[col][row] * D2R;
	return 0;
}

int table_DILOG (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: DILOG 1 1 dilog (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_dilog (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_dilog (GMT, T->segment[s]->coord[col][row]);
	return 0;
}

int table_DIFF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: DIFF 1 1 Difference between adjacent elements of A (A[1]-A[0], A[2]-A[1], ..., 0). */
{
	uint64_t s, row;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	/* Central 1st difference in t */
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows - 1; row++) 
			T->segment[s]->coord[col][row] = T->segment[s]->coord[col][row+1] - T->segment[s]->coord[col][row];

		T->segment[s]->coord[col][info->T->segment[s]->n_rows - 1] = 0;
	}
	return 0;
}

int table_DIV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: DIV 2 1 A / B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	int table_MUL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col);
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant && S[last]->factor == 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Divide by zero gives NaNs\n");
	}
	if (S[last]->constant) {	/* Turn divide into multiply */
		a = S[last]->factor;	/* Save old factor */
		S[last]->factor = 1.0 / S[last]->factor;
		table_MUL (GMT, info, S, last, col);
		S[last]->factor = a;	/* Restore factor to original value */
		return 0;
	}

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = a / b;
	}
	return 0;
}

int table_DUP (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: DUP 1 2 Places duplicate of A on the stack.  */
{
	uint64_t s, row;
	unsigned int next = last + 1;
	struct GMT_DATATABLE *T = S[last]->D->table[0], *T_next = S[next]->D->table[0];
	GMT_UNUSED(GMT);

	/* The next stack is an array no matter what S[last]->constant may be.
	   If S[last]->constant is true then gmtmath has just allocated space so we update that as well as next. */
	S[next]->constant = false;
	for (s = 0; s < info->T->n_segments; s++) {
		if (S[last]->constant) {	/* Constant, update both this and next */
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_next->segment[s]->coord[col][row] = T->segment[s]->coord[col][row] = S[last]->factor;
		}
		else
			GMT_memcpy (T_next->segment[s]->coord[col], T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
	}
	return 0;
}

int table_ECDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ECDF 2 1 Exponential cumulative distribution function for x = A and lambda = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double x, lambda;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		lambda = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		x  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = 1.0 - exp (-lambda * x);
	}
	return 0;
}

int table_ECRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ECRIT 2 1 Exponential distribution critical value for alpha = A and lambda = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double alpha, lambda;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		lambda = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		alpha  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = -log (1.0 - alpha)/lambda;
	}
	return 0;
}

int table_EPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: EPDF 2 1 Exponential probability density function for x = A and lambda = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double x, lambda;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		lambda = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		x  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = lambda * exp (-lambda * x);
	}
	return 0;
}

int table_ERF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ERF 1 1 Error function erf (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = erf (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : erf (T->segment[s]->coord[col][row]);
	return 0;
}

int table_ERFC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ERFC 1 1 Complementary Error function erfc (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = erfc (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : erfc (T->segment[s]->coord[col][row]);
	return 0;
}

int table_ERFINV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ERFINV 1 1 Inverse error function of A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_erfinv (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_erfinv (GMT, T->segment[s]->coord[col][row]);
	return 0;
}

int table_EQ (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: EQ 2 1 1 if A == B, else 0.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	GMT_UNUSED(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = (double)(a == b);
	}
	return 0;
}

int table_EXCH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: EXCH 2 2 Exchanges A and B on the stack.  */
{
	unsigned int prev;
	struct GMT_DATASET *D = NULL;
	GMT_UNUSED(GMT); GMT_UNUSED(info); GMT_UNUSED(col);
	assert (last > 0);
	prev = last - 1;
	D = S[last]->D;
	S[last]->D = S[prev]->D;	S[prev]->D = D;
	bool_swap (S[last]->constant, S[prev]->constant);
	uint_swap (S[last]->alloc_mode, S[prev]->alloc_mode);
	double_swap (S[last]->factor, S[prev]->factor);
	return 0;
}

int table_EXP (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: EXP 1 1 exp (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = exp (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : exp (T->segment[s]->coord[col][row]);
	return 0;
}

int table_FACT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FACT 1 1 A! (A factorial).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_factorial (GMT, irint(S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_factorial (GMT, irint(T->segment[s]->coord[col][row]));
	return 0;
}

int table_FCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FCRIT 3 1 F distribution critical value for alpha = A, nu1 = B, and nu2 = C.  */
{
	uint64_t s, row;
	int nu1, nu2;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	double alpha;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev2]->constant && S[prev2]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0 for FCRIT!\n");
	if (S[prev1]->constant && S[prev1]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for FCRIT!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand three == 0 for FCRIT!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		alpha = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->coord[col][row];
		nu1 = irint ((double)((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->coord[col][row]));
		nu2 = irint ((double)((S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row]));
		T_prev2->segment[s]->coord[col][row] = GMT_Fcrit (GMT, alpha, (double)nu1, (double)nu2);
	}
	return 0;
}

int table_FCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FCDF 3 1 F cumulative distribution function for F = A, nu1 = B, and nu2 = C.  */
{
	uint64_t s, row, nu1, nu2;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	double F;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev1]->constant && S[prev1]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for FCDF!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand three == 0 for FCDF!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		F = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->coord[col][row];
		nu1 = lrint ((double)((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->coord[col][row]));
		nu2 = lrint ((double)((S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row]));
		T_prev2->segment[s]->coord[col][row] = GMT_f_cdf (GMT, F, nu1, nu2);
	}
	return 0;
}

int table_FLIPUD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FLIPUD 1 1 Reverse order of each column.  */
{
	uint64_t s, row, k;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);
	/* Reverse the order of points in a column */
	if (S[last]->constant) return 0;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0, k = info->T->segment[s]->n_rows-1; row < info->T->segment[s]->n_rows/2; row++, k--) double_swap (T->segment[s]->coord[col][row], T->segment[s]->coord[col][k]);
	return 0;
}

int table_FLOOR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FLOOR 1 1 floor (A) (greatest integer <= A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = floor (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : floor (T->segment[s]->coord[col][row]);
	return 0;
}

int table_FMOD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FMOD 2 1 A % B (remainder after truncated division).  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, using FMOD 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = fmod (a, b);
	}
	return 0;
}

int table_FPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FPDF 3 1 F probability density distribution for F = A, nu1 = B and nu2 = C.  */
{
	unsigned int prev1 = last - 1, prev2 = last - 2;
	uint64_t s, row, nu1, nu2;
	double F;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev2]->constant && (S[prev2]->factor < 0.0 || S[prev2]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument F to FPDF must be a >= 0!\n");
		return -1;
	}
	if (S[prev1]->constant && S[prev1]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument nu1 to FPDF must be a positive integer (nu1 > 0)!\n");
		return -1;
	}
	if (S[last]->constant && S[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument nu2 to FPDF must be a positive integer (nu2 > 0)!\n");
		return -1;
	}
	if (S[prev2]->constant && S[prev1]->constant && S[last]->constant) {	/* FPDF is given constant arguments */
		double value;
		F = S[prev2]->factor;
		nu1 = (uint64_t)S[prev1]->factor;	nu2 = (uint64_t)S[last]->factor;
		value = GMT_f_pdf (GMT, F, nu1, nu2);
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev2->segment[s]->coord[col][row] = value;
		return 0;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		F = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->coord[col][row];
		nu1 = lrint ((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->coord[col][row]);
		nu2 = lrint ((S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row]);
		T_prev2->segment[s]->coord[col][row] = GMT_f_pdf (GMT, F, nu1, nu2);
	}
	return 0;
}

int table_GE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: GE 2 1 1 if A >= B, else 0.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	GMT_UNUSED(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = (double)(a >= b);
	}
	return 0;
}

int table_GT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: GT 2 1 1 if A > B, else 0.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	GMT_UNUSED(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = (double)(a > b);
	}
	return 0;
}

int table_HYPOT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: HYPOT 2 1 hypot (A, B) = sqrt (A*A + B*B).  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = hypot (a, b);
	}
	return 0;
}

int table_I0 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: I0 1 1 Modified Bessel function of A (1st kind, order 0).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_i0 (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_i0 (GMT, T->segment[s]->coord[col][row]);
	return 0;
}

int table_I1 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: I1 1 1 Modified Bessel function of A (1st kind, order 1).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_i1 (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_i1 (GMT, T->segment[s]->coord[col][row]);
	return 0;
}

int table_IFELSE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: IFELSE 3 1 B if A != 0, else C.  */
{
	uint64_t s, row;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	double a = 0.0, b = 0.0, c = 0.0;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
	GMT_UNUSED(GMT);

	/* last is C, prev1 is B, prev2 is A */

	/* Set to 1 where B <= A <= C, 0 elsewhere, except where
	 * A, B, or C = NaN, in which case we set answer to NaN */

	if (S[prev2]->constant) a = (double)S[prev2]->factor;
	if (S[prev1]->constant) b = (double)S[prev1]->factor;
	if (S[last]->constant)  c = (double)S[last]->factor;

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev2]->constant) a = T_prev2->segment[s]->coord[col][row];
		if (!S[prev1]->constant) b = T_prev1->segment[s]->coord[col][row];
		if (!S[last]->constant)  c = T->segment[s]->coord[col][row];
		T_prev2->segment[s]->coord[col][row] = (fabs (a) < GMT_CONV8_LIMIT) ? c : b;
	}
	return 0;
}

int table_IN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: IN 2 1 Modified Bessel function of A (1st kind, order B).  */
{
	uint64_t s, row;
	unsigned int prev, order = 0;
	bool simple = false;
	double b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant) {
		if (S[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order < 0 for IN!\n");
		if (fabs (rint(S[last]->factor) - S[last]->factor) > GMT_CONV4_LIMIT) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order not an integer for IN!\n");
		order = urint (fabs (S[last]->factor));
		if (S[prev]->constant) {
			b = GMT_in (GMT, order, fabs (S[prev]->factor));
			simple = true;
		}
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (simple)
			T_prev->segment[s]->coord[col][row] = b;
		else {
			if (!S[last]->constant) order = urint (fabs (T->segment[s]->coord[col][row]));
			T_prev->segment[s]->coord[col][row] = GMT_in (GMT, order, fabs (T_prev->segment[s]->coord[col][row]));
		}
	}
	return 0;
}

int table_INRANGE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: INRANGE 3 1 1 if B <= A <= C, else 0.  */
{
	uint64_t s, row;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	double a = 0.0, b = 0.0, c = 0.0, inrange;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	/* last is C, prev1 is B, prev2 is A */

	/* Set to 1 where B <= A <= C, 0 elsewhere, except where
	 * A, B, or C = NaN, in which case we set answer to NaN */

	if (S[prev2]->constant) a = (double)S[prev2]->factor;
	if (S[prev1]->constant) b = (double)S[prev1]->factor;
	if (S[last]->constant)  c = (double)S[last]->factor;

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev2]->constant) a = T_prev2->segment[s]->coord[col][row];
		if (!S[prev1]->constant) b = T_prev1->segment[s]->coord[col][row];
		if (!S[last]->constant)  c = T->segment[s]->coord[col][row];

		if (GMT_is_dnan (a) || GMT_is_dnan (b) || GMT_is_dnan (c)) {
			T_prev2->segment[s]->coord[col][row] = GMT->session.d_NaN;
			continue;
		}

		inrange = (b <= a && a <= c) ? 1.0 : 0.0;
		T_prev2->segment[s]->coord[col][row] = inrange;
	}
	return 0;
}

int table_INT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: INT 1 1 Numerically integrate A.  */
{
	uint64_t s, row, k;
	double f = 0.0, left, right, sum;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) {	/* Trivial case */
		sum = S[last]->factor * info->t_inc;
		for (s = k = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++, k++) T->segment[s]->coord[col][row] = ((info->local) ? row : k) * sum;
		return 0;
	}

	/* We use dumb trapezoidal rule - one day we will replace with more sophisticated rules */

	sum = 0.0;
	if (!info->irregular) f = 0.5 * info->t_inc;

	for (s = 0; s < info->T->n_segments; s++) {
		row = 0;
		if (info->local) sum = 0.0;	/* Reset integrated sum for each segment */
		while (row < info->T->segment[s]->n_rows) {
			left = T->segment[s]->coord[col][row];
			T->segment[s]->coord[col][row] = sum;
			row++;
			while (row < info->T->segment[s]->n_rows) {	/* Dumb trapezoidal rule */
				if (info->irregular) f = 0.5 * (info->T->segment[s]->coord[COL_T][row] - info->T->segment[s]->coord[COL_T][row-1]);
				right = T->segment[s]->coord[col][row];
				sum += f * (left + right);
				T->segment[s]->coord[col][row] = sum;
				left = right;
				row++;
			}
		}
	}
	return 0;
}

int table_INV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: INV 1 1 1 / A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Inverse of zero gives NaNs\n");
	if (S[last]->constant) a = (S[last]->factor == 0) ? GMT->session.d_NaN : 1.0 / S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : 1.0 / T->segment[s]->coord[col][row];
	return 0;
}

int table_ISFINITE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ISFINITE 1 1 1 if A is finite, else 0.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = isfinite (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : isfinite (T->segment[s]->coord[col][row]);
	return 0;
}

int table_ISNAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ISNAN 1 1 1 if A == NaN, else 0.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = (double)GMT_is_dnan (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : (double)GMT_is_dnan (T->segment[s]->coord[col][row]);
	return 0;
}

int table_J0 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: J0 1 1 Bessel function of A (1st kind, order 0).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = j0 (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : j0 (T->segment[s]->coord[col][row]);
	return 0;
}

int table_J1 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: J1 1 1 Bessel function of A (1st kind, order 1).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = j1 (fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : j1 (fabs (T->segment[s]->coord[col][row]));
	return 0;
}

int table_JN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: JN 2 1 Bessel function of A (1st kind, order B).  */
{
	uint64_t s, row;
	unsigned int prev, order = 0;
	bool simple = false;
	double b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant) {
		if (S[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order < 0 for JN!\n");
		if (fabs (rint(S[last]->factor) - S[last]->factor) > GMT_CONV4_LIMIT) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order not an integer for JN!\n");
		order = urint (fabs (S[last]->factor));
		if (S[prev]->constant) {
			b = jn ((int)order, fabs (S[prev]->factor));
			simple = true;
		}
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (simple)
			T_prev->segment[s]->coord[col][row] = b;
		else {
			if (!S[last]->constant) order = urint (fabs (T->segment[s]->coord[col][row]));
			T_prev->segment[s]->coord[col][row] = jn ((int)order, fabs (T_prev->segment[s]->coord[col][row]));
		}
	}
	return 0;
}

int table_K0 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: K0 1 1 Modified Kelvin function of A (2nd kind, order 0).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_k0 (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_k0 (GMT, T->segment[s]->coord[col][row]);
	return 0;
}

int table_K1 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: K1 1 1 Modified Bessel function of A (2nd kind, order 1).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_k1 (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_k1 (GMT, T->segment[s]->coord[col][row]);
	return 0;
}

int table_KN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: KN 2 1 Modified Bessel function of A (2nd kind, order B).  */
{
	uint64_t s, row;
	unsigned int prev, order = 0;
	bool simple = false;
	double b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant) {
		if (S[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order < 0 for KN!\n");
		if (fabs (rint(S[last]->factor) - S[last]->factor) > GMT_CONV4_LIMIT) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order not an integer for KN!\n");
		order = urint (fabs (S[last]->factor));
		if (S[prev]->constant) {
			b = GMT_kn (GMT, order, fabs (S[prev]->factor));
			simple = true;
		}
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (simple)
			T_prev->segment[s]->coord[col][row] = b;
		else {
			if (!S[last]->constant) order = urint (fabs (T->segment[s]->coord[col][row]));
			T_prev->segment[s]->coord[col][row] = GMT_kn (GMT, order, fabs (T_prev->segment[s]->coord[col][row]));
		}
	}
	return 0;
}

int table_KEI (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: KEI 1 1 kei (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_kei (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_kei (GMT, fabs (T->segment[s]->coord[col][row]));
	return 0;
}

int table_KER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: KER 1 1 ker (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_ker (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_ker (GMT, fabs (T->segment[s]->coord[col][row]));
	return 0;
}

int table_KURT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: KURT 1 1 Kurtosis of A.  */
{
	uint64_t s, row, n = 0;
	double mean = 0.0, sum2 = 0.0, kurt = 0.0, delta;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = GMT->session.d_NaN;
		return 0;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) {n = 0; mean = sum2 = kurt = 0.0;}	/* Reset for each segment */
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][row])) continue;
			n++;
			delta = T->segment[s]->coord[col][row] - mean;
			mean += delta / n;
			sum2 += delta * (T->segment[s]->coord[col][row] - mean);
		}
		if (info->local) {
			for (row = 0; row < info->T->segment[s]->n_rows; row++) {
				if (GMT_is_dnan (T->segment[s]->coord[col][row])) continue;
				delta = T->segment[s]->coord[col][row] - mean;
				kurt += pow (delta, 4.0);
			}
			if (n > 1) {
				sum2 /= (n - 1);
				kurt = kurt / (n * sum2 * sum2) - 3.0;
			}
			else
				kurt = GMT->session.d_NaN;
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = kurt;
		}
	}
	if (info->local) return 0;

	/* Here we do the global kurtosis */
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (GMT_is_dnan (T->segment[s]->coord[col][row])) continue;
		delta = T->segment[s]->coord[col][row] - mean;
		kurt += pow (delta, 4.0);
	}
	if (n > 1) {
		sum2 /= (n - 1);
		kurt = kurt / (n * sum2 * sum2) - 3.0;
	}
	else
		kurt = GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = kurt;
	return 0;
}

/* Laplace stuff based on https://en.wikipedia.org/wiki/Laplace_distribution */

int table_LCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LCDF 1 1 Laplace cumulative distribution function for z = A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = 0.5 + copysign (0.5, S[last]->factor) * (1.0 - exp (-fabs (S[last]->factor)));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a :  0.5 + copysign (0.5, T->segment[s]->coord[col][row]) * (1.0 - exp (-fabs (T->segment[s]->coord[col][row])));
	return 0;
}

int table_LCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LCRIT 1 1 Laplace distribution critical value for alpha = A.  */
{
	uint64_t s, row;
	double a = 0.0, p;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {
		p = (1.0 - S[last]->factor) - 0.5;
		a = -copysign (1.0, p) * log (1.0 - 2.0 * fabs (p));
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (S[last]->constant)
			T->segment[s]->coord[col][row] = a;
		else {
			p = (1.0 - T->segment[s]->coord[col][row]) - 0.5;
			T->segment[s]->coord[col][row] = -copysign (1.0, p) * log (1.0 - 2.0 * fabs (p));
		}
	}
	return 0;
}

int table_LE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LE 2 1 1 if A <= B, else 0.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	GMT_UNUSED(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = (double)(a <= b);
	}
	return 0;
}

int table_LMSSCL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LMSSCL 1 1 LMS scale estimate (LMS STD) of A.  */
{
	uint64_t s, row, k;
	unsigned int GMT_mode_selection = 0, GMT_n_multiples = 0;
	double lmsscl, mode, *z = NULL;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) GMT_memset (T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
		return 0;
	}

	if (!info->local) z = GMT_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++)  {
		if (info->local) {
			GMT_sort_array (GMT, T->segment[s]->coord[col], info->T->segment[s]->n_rows, GMT_DOUBLE);
			for (row = info->T->segment[s]->n_rows; row > 1 && GMT_is_dnan (T->segment[s]->coord[col][row-1]); row--);
			if (row) {
				GMT_mode (GMT, T->segment[s]->coord[col], row, row/2, 0, GMT_mode_selection, &GMT_n_multiples, &mode);
				GMT_getmad (GMT, T->segment[s]->coord[col], row, mode, &lmsscl);
			}
			else
				lmsscl = GMT->session.d_NaN;

			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = lmsscl;
			if (GMT_n_multiples > 0) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: %d Multiple modes found for segment %" PRIu64 "\n", GMT_n_multiples, s);
		}
		else {	/* Just accumulate the total table */
			GMT_memcpy (&z[k], T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return 0;	/* Done with local */
	GMT_sort_array (GMT, z, info->T->n_records, GMT_DOUBLE);
	for (row = info->T->n_records; row > 1 && GMT_is_dnan (z[row-1]); row--);
	if (row) {
		GMT_mode (GMT, z, row, row/2, 0, GMT_mode_selection, &GMT_n_multiples, &mode);
		GMT_getmad (GMT, z, row, mode, &lmsscl);
	}
	else
		lmsscl = GMT->session.d_NaN;

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = lmsscl;
	if (GMT_n_multiples > 0) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: %d Multiple modes found\n", GMT_n_multiples);
	GMT_free (GMT, z);
	return 0;
}

int table_LOG (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LOG 1 1 log (A) (natural log).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, argument to log = 0!\n");

	if (S[last]->constant) a = d_log (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : d_log (GMT, fabs (T->segment[s]->coord[col][row]));
	return 0;
}

int table_LOG10 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LOG10 1 1 log10 (A) (base 10).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, argument to log10 = 0!\n");

	if (S[last]->constant) a = d_log10 (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : d_log10 (GMT, fabs (T->segment[s]->coord[col][row]));
	return 0;
}

int table_LOG1P (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LOG1P 1 1 log (1+A) (accurate for small A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, argument to log1p < 0!\n");

	if (S[last]->constant) a = d_log1p (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : d_log1p (GMT, fabs (T->segment[s]->coord[col][row]));
	return 0;
}

int table_LOG2 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LOG2 1 1 log2 (A) (base 2).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, argument to log2 = 0!\n");

	if (S[last]->constant) a = d_log (GMT, fabs (S[last]->factor)) * M_LN2_INV;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : d_log (GMT, fabs (T->segment[s]->coord[col][row])) * M_LN2_INV;
	return 0;
}

int table_LOWER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LOWER 1 1 The lowest (minimum) value of A.  */
{
	uint64_t s, row;
	double low = DBL_MAX;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = S[last]->factor;
		return 0;
	}

	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) low = DBL_MAX;
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][row])) continue;
			if (T->segment[s]->coord[col][row] < low) low = T->segment[s]->coord[col][row];
		}
		if (low == DBL_MAX) low = GMT->session.d_NaN;
		if (info->local) for (row = 0; row < info->T->segment[s]->n_rows; row++) if (!GMT_is_dnan (T->segment[s]->coord[col][row])) T->segment[s]->coord[col][row] = low;
	}
	if (info->local) return 0;	/* Done with local */
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = low;
	return 0;
}

int table_LPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LPDF 1 1 Laplace probability density function for z = A.  */
{
	uint64_t s, row;
	double z = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) z = 0.5 * exp (-fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? z : 0.5 *exp (-fabs (T->segment[s]->coord[col][row]));
	return 0;
}

int table_LRAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LRAND 2 1 Laplace random noise with mean A and std. deviation B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a = 0.0, b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) a = S[prev]->factor;
	if (S[last]->constant) b = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev]->constant) a = T_prev->segment[s]->coord[col][row];
		if (!S[last]->constant) b = T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = a + b * GMT_lrand (GMT);
	}
	return 0;
}

int table_LSQFIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LSQFIT 1 0 Current table is [A | b]; return LS solution to A * x = b via Cholesky decomposition.  */
{
	GMT_UNUSED(GMT); GMT_UNUSED(info); GMT_UNUSED(S); GMT_UNUSED(last); GMT_UNUSED(col);
	/* Dummy routine needed since the automatically generated include file will have table_LSQFIT
	 * with these parameters just like any other function.  However, when we find LSQFIT we will
	 * instead call solve_LSQFIT which can be found at the end of these functions */
	return 0;
}

int table_LT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LT 2 1 1 if A < B, else 0.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	GMT_UNUSED(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = (double)(a < b);
	}
	return 0;
}

int table_MAD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MAD 1 1 Median Absolute Deviation (L1 STD) of A.  */
{
	uint64_t s, row, k;
	double mad, med, *z = NULL;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) GMT_memset (T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
		return 0;
	}

	if (!info->local) z = GMT_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++) {
		if (info->local) {
			GMT_sort_array (GMT, T->segment[s]->coord[col], info->T->segment[s]->n_rows, GMT_DOUBLE);
			for (row = info->T->segment[s]->n_rows; row > 1 && GMT_is_dnan (T->segment[s]->coord[col][row-1]); row--);
			if (row) {
				med = (row%2) ? T->segment[s]->coord[col][row/2] : 0.5 * (T->segment[s]->coord[col][(row-1)/2] + T->segment[s]->coord[col][row/2]);
				GMT_getmad (GMT, T->segment[s]->coord[col], row, med, &mad);
			}
			else
				mad = GMT->session.d_NaN;
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = mad;
		}
		else {	/* Just accumulate the total table */
			GMT_memcpy (&z[k], T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return 0;	/* Done with local */
	GMT_sort_array (GMT, z, info->T->n_records, GMT_DOUBLE);
	for (row = info->T->n_records; row > 1 && GMT_is_dnan (z[row-1]); row--);
	if (row) {
		med = (row%2) ? z[row/2] : 0.5 * (z[(row-1)/2] + z[row/2]);
		GMT_getmad (GMT, z, row, med, &mad);
	}
	else
		mad = GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = mad;
	GMT_free (GMT, z);
	return 0;
}

int table_MAX (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MAX 2 1 Maximum of A and B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = (GMT_is_dnan (a) || GMT_is_dnan (b)) ? GMT->session.d_NaN : MAX (a, b);
	}
	return 0;
}

int table_MEAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MEAN 1 1 Mean value of A.  */
{
	uint64_t s, row, n_a = 0;
	double sum_a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = S[last]->factor;
		return 0;
	}

	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) {sum_a = 0.0; n_a = 0;}
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][row])) continue;
			sum_a += T->segment[s]->coord[col][row];
			n_a++;
		}
		if (info->local) {
			sum_a = (n_a) ? sum_a / n_a : GMT->session.d_NaN;
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = sum_a;
		}
	}
	if (info->local) return 0;	/* Done with local */
	sum_a = (n_a) ? sum_a / n_a : GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = sum_a;
	return 0;
}

int table_MED (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MED 1 1 Median value of A.  */
{
	uint64_t s, row, k;
	double med, *z = NULL;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = S[last]->factor;
		return 0;
	}

	if (!info->local) z = GMT_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++) {
		if (info->local) {
			GMT_sort_array (GMT, T->segment[s]->coord[col], info->T->segment[s]->n_rows, GMT_DOUBLE);
			for (row = info->T->segment[s]->n_rows; row > 1 && GMT_is_dnan (T->segment[s]->coord[col][row-1]); row--);
			if (row)
				med = (row%2) ? T->segment[s]->coord[col][row/2] : 0.5 * (T->segment[s]->coord[col][(row-1)/2] + T->segment[s]->coord[col][row/2]);
			else
				med = GMT->session.d_NaN;

			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = med;
		}
		else {	/* Just accumulate the total table */
			GMT_memcpy (&z[k], T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return 0;	/* Done with local */
	GMT_sort_array (GMT, z, info->T->n_records, GMT_DOUBLE);
	for (row = info->T->n_records; row > 1 && GMT_is_dnan (z[row-1]); row--);
	if (row)
		med = (row%2) ? z[row/2] : 0.5 * (z[(row-1)/2] + z[row/2]);
	else
		med = GMT->session.d_NaN;

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = med;
	GMT_free (GMT, z);
	return 0;
}

int table_MIN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MIN 2 1 Minimum of A and B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = (GMT_is_dnan (a) || GMT_is_dnan (b)) ? GMT->session.d_NaN : MIN (a, b);
	}
	return 0;
}

int table_MOD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MOD 2 1 A mod B (remainder after floored division).  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, using MOD 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = MOD (a, b);
	}
	return 0;
}

int table_MODE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MODE 1 1 Mode value (Least Median of Squares) of A.  */
{
	uint64_t s, row, k;
	unsigned int GMT_mode_selection = 0, GMT_n_multiples = 0;
	double mode, *z = NULL;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = S[last]->factor;
		return 0;
	}

	if (!info->local) z = GMT_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++)  {
		if (info->local) {
			GMT_sort_array (GMT, T->segment[s]->coord[col], info->T->segment[s]->n_rows, GMT_DOUBLE);
			for (row = info->T->segment[s]->n_rows; row > 1 && GMT_is_dnan (T->segment[s]->coord[col][row-1]); row--);
			if (row)
				GMT_mode (GMT, T->segment[s]->coord[col], row, row/2, 0, GMT_mode_selection, &GMT_n_multiples, &mode);
			else
				mode = GMT->session.d_NaN;

			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = mode;
			if (GMT_n_multiples > 0) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: %d Multiple modes found for segment %" PRIu64 "\n", GMT_n_multiples, s);
		}
		else {	/* Just accumulate the total table */
			GMT_memcpy (&z[k], T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	GMT_sort_array (GMT, z, info->T->n_records, GMT_DOUBLE);
	for (row = info->T->n_records; row > 1 && GMT_is_dnan (z[row-1]); row--);
	if (row)
		GMT_mode (GMT, z, row, row/2, 0, GMT_mode_selection, &GMT_n_multiples, &mode);
	else
		mode = GMT->session.d_NaN;

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = mode;
	if (GMT_n_multiples > 0) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: %d Multiple modes found\n", GMT_n_multiples);
	GMT_free (GMT, z);
	return 0;
}

int table_MUL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MUL 2 1 A * B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = a * b;
	}
	return 0;
}

int table_NAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: NAN 2 1 NaN if A == B, else A.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a = 0.0, b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) a = S[prev]->factor;
	if (S[last]->constant) b = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev]->constant) a = T_prev->segment[s]->coord[col][row];
		if (!S[last]->constant) b = T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = ((a == b) ? GMT->session.d_NaN : a);
	}
	return 0;
}

int table_NEG (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: NEG 1 1 -A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand == 0!\n");
	if (S[last]->constant) a = -S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : -T->segment[s]->coord[col][row];
	return 0;
}

int table_NEQ (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: NEQ 2 1 1 if A != B, else 0.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	GMT_UNUSED(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = (double)(a != b);
	}
	return 0;
}

int table_NORM (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: NORM 1 1 Normalize (A) so max(A)-min(A) = 1.  */
{
	uint64_t s, row, n;
	double a, z, zmin = DBL_MAX, zmax = -DBL_MAX;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, NORM of a constant gives NaN!\n");
		a = GMT->session.d_NaN;
	}
	else {
		for (s = n = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			z = T->segment[s]->coord[col][row];
			if (GMT_is_dnan (z)) continue;
			if (z < zmin) zmin = z;
			if (z > zmax) zmax = z;
			n++;
		}
		a = (n == 0 || zmax == zmin) ? GMT->session.d_NaN : 1.0 / (zmax - zmin);	/* Normalization scale */
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : a * (T->segment[s]->coord[col][row]);
	return 0;
}

int table_NOT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: NOT 1 1 NaN if A == NaN, 1 if A == 0, else 0.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand == 0!\n");
	if (S[last]->constant) a = (fabs (S[last]->factor) > GMT_CONV8_LIMIT) ? 0.0 : 1.0;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : ((fabs (T->segment[s]->coord[col][row]) > GMT_CONV8_LIMIT) ? 0.0 : 1.0);
	return 0;
}

int table_NRAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: NRAND 2 1 Normal, random values with mean A and std. deviation B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a = 0.0, b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) a = S[prev]->factor;
	if (S[last]->constant) b = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev]->constant) a = T_prev->segment[s]->coord[col][row];
		if (!S[last]->constant) b = T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = a + b * GMT_nrand (GMT);
	}
	return 0;
}

int table_OR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: OR 2 1 NaN if B == NaN, else A.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = (GMT_is_dnan (a) || GMT_is_dnan (b)) ? GMT->session.d_NaN : a;
	}
	return 0;
}

int table_PERM (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PERM 2 1 Permutations n_P_r, with n = A and r = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error, argument n to PERM must be a positive integer (n >= 0)!\n");
		return -1;
	}
	if (S[last]->constant && S[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error, argument r to PERM must be a positive integer (r >= 0)!\n");
		return -1;
	}
	if (S[prev]->constant && S[last]->constant) {	/* PERM is given constant argument */
		double value = GMT_permutation (GMT, irint(S[prev]->factor), irint(S[last]->factor));
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->coord[col][row] = value;
		return 0;
	}
	/* Must run the full thing */
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = GMT_permutation (GMT, irint(a), irint(b));
	}
	return 0;
}

int table_PLM (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PLM 3 1 Associated Legendre polynomial P(A) degree B order C.  */
{
	uint64_t s, row;
	unsigned int prev, first, L, M;
	double a = 0.0;
	struct GMT_DATATABLE *T_first = NULL;
	assert (last > 1);
	prev = last - 1;	first = last - 2;
	T_first = S[first]->D->table[0];
	
	/* last holds the order M, prev holds the degree L, first holds the argument x = cos(colat) */

	if (!(S[prev]->constant && S[last]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "PLM: L and M must be constants!\n");
		return -1;
	}
	L = urint (S[prev]->factor);
	M = urint (S[last]->factor);

	if (S[first]->constant) a = GMT_plm (GMT, L, M, S[first]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_first->segment[s]->coord[col][row] = (S[first]-> constant) ? a : GMT_plm (GMT, L, M, T_first->segment[s]->coord[col][row]);
	return 0;
}

int table_PLMg (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PLMg 3 1 Normalized associated Legendre polynomial P(A) degree B order C (geophysical convention).  */
{
	uint64_t s, row;
	unsigned int prev, first, L, M;
	double a = 0.0;
	struct GMT_DATATABLE *T_first = NULL;
	assert (last > 1);
	prev = last - 1;	first = last - 2;
	T_first = S[first]->D->table[0];
	/* last holds the order M, prev holds the degree L, first holds the argument x = cos(colat) */

	if (!(S[prev]->constant && S[last]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "PLMg: L and M must be constants!\n");
		return -1;
	}
	L = urint (S[prev]->factor);
	M = urint (S[last]->factor);

	if (S[first]->constant) a = GMT_plm_bar (GMT, L, M, S[first]->factor, false);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_first->segment[s]->coord[col][row] = (S[first]-> constant) ? a : GMT_plm_bar (GMT, L, M, T_first->segment[s]->coord[col][row], false);
	return 0;
}

int table_POP (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: POP 1 0 Delete top element from the stack.  */
{
	GMT_UNUSED(GMT); GMT_UNUSED(info); GMT_UNUSED(S); GMT_UNUSED(last); GMT_UNUSED(col);
	/* Dummy routine that does nothing but consume the top element of stack */
	return 0;
}

int table_POW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: POW 2 1 A ^ B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = pow (a, b);
	}
	return 0;
}

int table_PPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PPDF 2 1 Poisson probability density function for x = A and lambda = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double x, lambda;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		lambda = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		x  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = GMT_poissonpdf (GMT, x, lambda);
	}
	return 0;
}

int table_PQUANT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PQUANT 2 1 The B'th Quantile (0-100%) of A.  */
{
	uint64_t s, row, k;
	unsigned int prev;
	double p, *z = NULL;
	struct GMT_DATATABLE *T_prev = NULL;
	assert (last > 0);
	prev = last - 1;
	T_prev = S[prev]->D->table[0];
	/* last holds the selected quantile (0-100), prev the data % */
	if (!S[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: PQUANT must be given a constant quantile!\n");
		return -1;
	}
	if (S[last]->factor < 0.0 || S[last]->factor > 100.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: PQUANT must be given a constant quantile between 0-100%%!\n");
		return -1;
	}
	if (S[prev]->constant) {	/* Trivial case */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: PQUANT of a constant is set to NaN\n");
		p = GMT->session.d_NaN;
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->coord[col][row] = p;
		return 0;
	}

	if (!info->local) z = GMT_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++)  {
		if (info->local) {
			GMT_sort_array (GMT, T_prev->segment[s]->coord[col], info->T->segment[s]->n_rows, GMT_DOUBLE);
			p = GMT_quantile (GMT, T_prev->segment[s]->coord[col], S[last]->factor, info->T->segment[s]->n_rows);
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->coord[col][row] = p;
		}
		else {	/* Just accumulate the total table */
			GMT_memcpy (&z[k], T_prev->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return 0;	/* Done with local */
	GMT_sort_array (GMT, z, info->T->n_records, GMT_DOUBLE);
	p = GMT_quantile (GMT, z, S[last]->factor, info->T->n_records);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->coord[col][row] = p;
	GMT_free (GMT, z);
	return 0;
}

int table_PSI (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PSI 1 1 Psi (or Digamma) of A.  */
{
	uint64_t s, row;
	double a = 0.0, x[2];
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	x[1] = 0.0;	/* No imaginary part */
	if (S[last]->constant) {
		x[0] = S[last]->factor;
		a = GMT_psi (GMT, x, NULL);
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[last]->constant) {
			x[0] = T->segment[s]->coord[col][row];
			a = GMT_psi (GMT, x, NULL);
		}
		T->segment[s]->coord[col][row] = a;
	}
	return 0;
}

int table_PVQV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col, unsigned int kind)
{	/* kind: 0 = Pv, 1 = Qv */
	uint64_t s, row;
	unsigned int n, prev, first, calc;
	double a = 0.0, x = 0.0, nu[2], pq[4];
	static char *name[2] = {"PV", "QV"};
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL, *T_first = NULL;
	assert (last > 1);
				/* last holds the imaginary order vi */
	prev  = last - 1;	/* prev holds the real order vr */
	first = prev - 1;	/* first holds the argument x = cos(colat) */
	T = (S[last]->constant) ? NULL : S[last]->D->table[0];
	T_prev = (S[prev]->constant) ? NULL : S[prev]->D->table[0];
	T_first = S[first]->D->table[0];

	calc = !(S[prev]->constant && S[last]->constant && S[first]-> constant);	/* Only constant it all args are constant */
	if (!calc) {	/* All constants */
		nu[0] = S[prev]->factor;
		nu[1] = S[last]->factor;
		if ((S[first]->factor < -1.0 || S[first]->factor > 1.0)) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, argument to %s outside domain!\n", name[kind]);
		GMT_PvQv (GMT, S[first]->factor, nu, pq, &n);
		a = pq[2*kind];
	}
	if (S[prev]->constant) nu[0] = S[prev]->factor;
	if (S[last]->constant) nu[1] = S[last]->factor;
	if (S[first]-> constant)    x = S[first]->factor;
	kind *= 2;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (calc){
			if (!S[prev]->constant) nu[0] = T_prev->segment[s]->coord[col][row];
			if (!S[last]->constant) nu[1] = T->segment[s]->coord[col][row];
			if (!S[first]-> constant)    x = T_first->segment[s]->coord[col][row];
			GMT_PvQv (GMT, x, nu, pq, &n);
			a = pq[kind];
		}
		T_first->segment[s]->coord[col][row] = a;
	}
	return 0;
}

int table_PV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PV 3 1 Legendre function Pv(A) of degree v = real(B) + imag(C).  */
{
	table_PVQV (GMT, info, S, last, col, 0);
	return 0;
}

int table_QV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: QV 3 1 Legendre function Qv(A) of degree v = real(B) + imag(C).  */
{
	table_PVQV (GMT, info, S, last, col, 1);
	return 0;
}

int table_R2 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: R2 2 1 R2 = A^2 + B^2.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0!\n");
	if (S[prev]->constant) S[prev]->factor *= S[prev]->factor;
	if (S[last]->constant) S[last]->factor *= S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row] * T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row] * T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = a + b;
	}
	return 0;
}

int table_R2D (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: R2D 1 1 Convert Radians to Degrees.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = S[last]->factor * R2D;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : T->segment[s]->coord[col][row] * R2D;
	return 0;
}

int table_RAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: RAND 2 1 Uniform random values between A and B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a = 0.0, b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) a = S[prev]->factor;
	if (S[last]->constant) b = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev]->constant) a = T_prev->segment[s]->coord[col][row];
		if (!S[last]->constant) b = T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = a + GMT_rand (GMT) * (b - a);
	}
	return 0;
}

int table_RCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: RCDF 1 1 Rayleigh cumulative distribution function for z = A.  */
{
	uint64_t s, row;
	double z;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		z = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T->segment[s]->coord[col][row] = 1.0 - exp (-0.5*z*z);
	}
	return 0;
}

int table_RCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: RCRIT 1 1 Rayleigh distribution critical value for alpha = A.  */
{
	uint64_t s, row;
	double alpha;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		alpha = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T->segment[s]->coord[col][row] = M_SQRT2 * sqrt (-log (1.0 - alpha));
	}
	return 0;
}

int table_RPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: RPDF 1 1 Rayleigh probability density function for z = A.  */
{
	uint64_t s, row;
	unsigned int prev;
	double z;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		z = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T->segment[s]->coord[col][row] = z * exp (-0.5 * z * z);
	}
	return 0;
}

int table_RINT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: RINT 1 1 rint (A) (round to integral value nearest to A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = rint (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : rint (T->segment[s]->coord[col][row]);
	return 0;
}

void assign_gmtstack (struct GMTMATH_STACK *Sto, struct GMTMATH_STACK *Sfrom)
{	/* Copy contents of Sfrom to Sto */
	Sto->D          = Sfrom->D;
	Sto->constant   = Sfrom->constant;
	Sto->alloc_mode = Sfrom->alloc_mode;
	Sto->factor     = Sfrom->factor;
}

int table_ROLL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ROLL 2 0 Cyclicly shifts the top A stack items by an amount B.  */
{
	unsigned int prev, top, bottom, k, kk, n_items;
	int n_shift;
	struct GMTMATH_STACK Stmp;
	GMT_UNUSED(GMT); GMT_UNUSED(info); GMT_UNUSED(col);
	assert (last > 2);	/* Must have at least 3 items on the stack: A single item plus the two roll arguments */
	prev = last - 1;	/* This gives the number of stack items to include in the cycle */
	if (!(S[last]->constant && S[prev]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: length and shift must be constants in ROLL!\n");
		return -1;
	}
	n_items = urint (S[prev]->factor);
	n_shift = irint (S[last]->factor);
	if (n_items > prev) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Items on stack is fewer than required by ROLL!\n");
		return -1;
	}
	top = prev - 1;
	bottom = prev - n_items;
	for (k = 0; k < (unsigned int)abs (n_shift); k++) {	/* Do the cyclical shift */
		if (n_shift > 0) {	/* Positive roll */
			assign_gmtstack (&Stmp, S[top]);	/* Keep copy of top item */
			for (kk = 1; kk < n_items; kk++)	/* Move all others up one step */
				assign_gmtstack (S[top-kk+1], S[top-kk]);
			assign_gmtstack (S[bottom], &Stmp);	/* Place copy on bottom */
		}
		else if (n_shift < 0) {	/* Negative roll */
			assign_gmtstack (&Stmp, S[bottom]);	/* Keep copy of bottom item */
			for (kk = 1; kk < n_items; kk++)	/* Move all others down one step */
				assign_gmtstack (S[bottom+kk-1], S[bottom+kk]);
			assign_gmtstack (S[top], &Stmp);	/* Place copy on top */
		}
	}
	return 0;
}

int table_ROTT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ROTT 2 1 Rotate A by the (constant) shift B in the t-direction.  */
{
	uint64_t s, row, j, k;
	unsigned int prev;
	int shift;
	double *z = NULL;
	struct GMT_DATATABLE *T_prev = NULL;
	
	assert (last > 0);
	prev = last - 1;
	T_prev = S[prev]->D->table[0];
	if (!S[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: T-shift must be a constant in ROTT!\n");
		return -1;
	}
	shift = irint (S[last]->factor / info->t_inc);
	if (S[prev]->constant || !shift) return 0;	/* Easy, constant or no shift */
	if (!info->local) {
		if (shift < 0) shift += (int)info->T->n_records;		/* Same thing */
		z = GMT_memory (GMT, NULL, info->T->n_records, double);
	}
	for (s = k = 0; s < info->T->n_segments; s++)  {
		if (info->local) {
			shift = irint (S[last]->factor / info->t_inc);
			if (shift < 0) shift += (int)info->T->segment[s]->n_rows;		/* Same thing */
			z = GMT_memory (GMT, NULL, info->T->segment[s]->n_rows, double);
		}

		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			j = (info->local) ? (row+shift)%info->T->segment[s]->n_rows : (k+shift)%info->T->n_records;
			z[j] = T_prev->segment[s]->coord[col][row];
		}
		if (info->local) {
			GMT_memcpy (T_prev->segment[s]->coord[col], z, info->T->segment[s]->n_rows, double);
			GMT_free (GMT, z);
		}
	}
	if (info->local) return 0;	/* Done with local */
	for (s = k = 0; s < info->T->n_segments; s++, k += info->T->segment[s]->n_rows) GMT_memcpy (T_prev->segment[s]->coord[col], &z[k], info->T->segment[s]->n_rows, double);
	GMT_free (GMT, z);
	return 0;
}

int table_SEC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SEC 1 1 sec (A) (A in radians).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = (1.0 / cos (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : (1.0 / cos (T->segment[s]->coord[col][row]));
	return 0;
}

int table_SECD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SECD 1 1 sec (A) (A in degrees).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = (1.0 / cosd (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : (1.0 / cosd (T->segment[s]->coord[col][row]));
	return 0;
}

int table_SIGN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SIGN 1 1 sign (+1 or -1) of A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand == 0!\n");
	if (S[last]->constant) a = copysign (1.0, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : copysign (1.0, T->segment[s]->coord[col][row]);
	return 0;
}

int table_SIN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SIN 1 1 sin (A) (A in radians).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = sin (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : sin (T->segment[s]->coord[col][row]);
	return 0;
}

int table_SINC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SINC 1 1 sinc (A) (sin (pi*A)/(pi*A)).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_sinc (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_sinc (GMT, T->segment[s]->coord[col][row]);
	return 0;
}

int table_SIND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SIND 1 1 sin (A) (A in degrees).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = sind (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : sind (T->segment[s]->coord[col][row]);
	return 0;
}

int table_SINH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SINH 1 1 sinh (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = sinh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : sinh (T->segment[s]->coord[col][row]);
	return 0;
}

int table_SKEW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SKEW 1 1 Skewness of A.  */
{
	uint64_t s, row, n = 0;
	double mean = 0.0, sum2 = 0.0, skew = 0.0, delta;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = GMT->session.d_NaN;
		return 0;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) {n = 0; mean = sum2 = skew = 0.0; }	/* Start anew for each segment */
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][row])) continue;
			n++;
			delta = T->segment[s]->coord[col][row] - mean;
			mean += delta / n;
			sum2 += delta * (T->segment[s]->coord[col][row] - mean);
		}
		if (info->local) {
			if (n > 1) {
				for (row = 0; row < info->T->segment[s]->n_rows; row++) {
					if (GMT_is_dnan (T->segment[s]->coord[col][row])) continue;
					delta = T->segment[s]->coord[col][row] - mean;
					skew += pow (delta, 3.0);
				}
				sum2 /= (n - 1);
				skew /= n * pow (sum2, 1.5);
			}
			else
				skew = GMT->session.d_NaN;
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = skew;
		}
	}
	if (info->local) return 0;	/* Done with local */
	if (n > 1) {
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][row])) continue;
			delta = T->segment[s]->coord[col][row] - mean;
			skew += pow (delta, 3.0);
		}
		sum2 /= (n - 1);
		skew /= n * pow (sum2, 1.5);
	}
	else
		skew = GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = skew;
	return 0;
}

int table_SQR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SQR 1 1 A^2.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = S[last]->factor * S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : T->segment[s]->coord[col][row] *  T->segment[s]->coord[col][row];
	return 0;
}

int table_SQRT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SQRT 1 1 sqrt (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one < 0!\n");
	if (S[last]->constant) a = sqrt (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : sqrt (T->segment[s]->coord[col][row]);
	return 0;
}

int table_STD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: STD 1 1 Standard deviation of A.  */
{
	uint64_t s, row, n = 0;
	double mean = 0.0, sum2 = 0.0, delta;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) GMT_memset (T->segment[s]->coord[col], info->T->segment[s]->n_rows, double);
		return 0;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) {n = 0; mean = sum2 = 0.0;}	/* Start anew for each segment */
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][row])) continue;
			n++;
			delta = T->segment[s]->coord[col][row] - mean;
			mean += delta / n;
			sum2 += delta * (T->segment[s]->coord[col][row] - mean);
		}
		if (info->local) {
			sum2 = (n > 1) ? sqrt (sum2 / (n - 1)) : 0.0;
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = sum2;
		}
	}
	if (info->local) return 0;	/* Done with local */
	sum2 = (n > 1) ? sqrt (sum2 / (n - 1)) : GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = sum2;
	return 0;
}

int table_STEP (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: STEP 1 1 Heaviside step function H(A).  */
{
	uint64_t s, row;
	double a;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		if (a == 0.0)
			T->segment[s]->coord[col][row] = 0.5;
		else
			T->segment[s]->coord[col][row] = (a < 0.0) ? 0.0 : 1.0;
	}
	return 0;
}

int table_STEPT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: STEPT 1 1 Heaviside step function H(t-A).  */
{
	uint64_t s, row;
	double a;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = info->T->segment[s]->coord[COL_T][row] - ((S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row]);
		if (a == 0.0)
			T->segment[s]->coord[col][row] = 0.5;
		else
			T->segment[s]->coord[col][row] = (a < 0.0) ? 0.0 : 1.0;
	}
	return 0;
}

int table_SUB (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SUB 2 1 A - B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = a - b;
	}
	return 0;
}

int table_SUM (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SUM 1 1 Cumulative sum of A.  */
{
	uint64_t s, row;
	double a = 0.0, sum = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) sum = 0.0;	/* Reset for each segment */
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (!S[last]->constant) a = T->segment[s]->coord[col][row];
			if (!GMT_is_dnan (a)) sum += a;
			T->segment[s]->coord[col][row] = sum;
		}
	}
	return 0;
}

int table_SVDFIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SVDFIT 1 0 Current table is [A | b]; return LS solution to A * x = B via SVD decomposition (see -E).  */
{
	GMT_UNUSED(GMT); GMT_UNUSED(info); GMT_UNUSED(S); GMT_UNUSED(last); GMT_UNUSED(col);
	/* Dummy routine needed since the automatically generated include file will have table_SVDFIT
	 * with these parameters just like any other function.  However, when we find SVDFIT we will
	 * instead call solve_SVDFIT which can be found at the end of these functions */
	return 0;
}

int table_TAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TAN 1 1 tan (A) (A in radians).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = tan (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : tan (T->segment[s]->coord[col][row]);
	return 0;
}

int table_TAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TAND 1 1 tan (A) (A in degrees).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = tand (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : tand (T->segment[s]->coord[col][row]);
	return 0;
}

int table_TANH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TANH 1 1 tanh (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	GMT_UNUSED(GMT);

	if (S[last]->constant) a = tanh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : tanh (T->segment[s]->coord[col][row]);
	return 0;
}

int table_TAPER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TAPER 1 1 Unit weights cosine-tapered to zero within A of end margins.  */
{
	/* If no time, then A is interpreted to mean number of nodes instead */
	uint64_t s, row;
	double strip, scale, t_min, t_max, start, stop, from_start, from_stop, t, w_t;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (!S[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "TAPER: Argument A must be a constant!\n");
		return -1;
	}
	strip = S[last]->factor;
	scale = M_PI / strip;
	if (!info->notime) stop = strip - info->t_max;
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->notime) {	/* If no time then A refers to number of rows and min and max are in rows */
			t_min = 0.0;
			t_max = info->T->segment[s]->n_rows - 1.0;
		}
		else {	/* Here, A is in time units and the min/max are start/stop time per segment */
			t_min = info->T->segment[s]->coord[COL_T][0];	/* Start of time for this segment */
			t_max = info->T->segment[s]->coord[COL_T][info->T->segment[s]->n_rows-1];	/* End of time for this segment */
		}
		start = strip + t_min;
		stop  = strip - t_max;
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			t = (info->notime) ? (double)row : info->T->segment[s]->coord[COL_T][row];
			from_start = start - t;
			if (from_start > 0.0) w_t = 0.5 * (1.0 + cos (from_start * scale));
			else if ((from_stop = stop + t) > 0.0) w_t = 0.5 * (1.0 + cos (from_stop * scale));
			else w_t = 1.0;	/* Inside non-tapered t-range */
			T->segment[s]->coord[col][row] = w_t;
		}
	}
	return 0;
}

int table_TCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TCDF 2 1 Student's t cumulative distribution function for t = A and nu = B.  */
{
	uint64_t s, row, nu;
	unsigned int prev;
	double t;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		nu = lrint ((S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row]);
		t  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = GMT_t_cdf (GMT, t, nu);
	}
	return 0;
}

int table_TN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TN 2 1 Chebyshev polynomial Tn(-1<A<+1) of degree B.  */
{
	uint64_t s, row;
	unsigned int prev;
	int n;
	double a;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		n = irint ((S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row]);
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		GMT_chebyshev (GMT, a, n, &T_prev->segment[s]->coord[col][row]);
	}
	return 0;
}

int table_TPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TPDF 2 1 Student's t probability density function for t = A and nu = B.  */
{
	uint64_t s, row, nu;
	unsigned int prev;
	double t;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		nu = lrint ((S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row]);
		t  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = GMT_t_pdf (GMT, t, nu);
	}
	return 0;
}

int table_TCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TCRIT 2 1 Student's t distribution critical value for alpha = A and nu = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0 for TCRIT!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for TCRIT!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->coord[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = GMT_tcrit (GMT, a, b);
	}
	return 0;
}

int table_UPPER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: UPPER 1 1 The highest (maximum) value of A.  */
{
	uint64_t s, row;
	double high = -DBL_MAX;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = S[last]->factor;
		return 0;
	}

	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) high = -DBL_MAX;
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (GMT_is_dnan (T->segment[s]->coord[col][row])) continue;
			if (T->segment[s]->coord[col][row] > high) high = T->segment[s]->coord[col][row];
		}
		if (high == -DBL_MAX) high = GMT->session.d_NaN;
		if (info->local) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = high;
	}
	if (info->local) return 0;	/* Done with local */
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) if (!GMT_is_dnan (T->segment[s]->coord[col][row])) T->segment[s]->coord[col][row] = high;
	return 0;
}

int table_WCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: WCDF 3 1 Weibull cumulative distribution function for x = A, scale = B, and shape = C.  */
{
	uint64_t s, row;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	double x, a, b;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev1]->constant && S[prev1]->factor <= 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two <=0 for WCDF!\n");
	if (S[last]->constant && S[last]->factor <= 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand three <= 0 for WCDF!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		x = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->coord[col][row];
		a = (double)((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->coord[col][row]);
		b = (double)((S[last]->constant) ? S[last]->factor : T->segment[s]->coord[col][row]);
		T_prev2->segment[s]->coord[col][row] = GMT_weibull_cdf (GMT, x, a, b);
	}
	return 0;
}

int table_WCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: WCRIT 3 1 Weibull distribution critical value for alpha = A, scale = B, and shape = C.  */
{
	uint64_t s, row;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	double alpha, a, b;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev1]->constant && S[prev1]->factor <= 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two <=0 for WCRIT!\n");
	if (S[last]->constant && S[last]->factor <= 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand three <= 0 for WCRIT!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		alpha = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->coord[col][row];
		a = (double)((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->coord[col][row]);
		b = (double)((S[last]->constant)  ? S[last]->factor  : T->segment[s]->coord[col][row]);
		T_prev2->segment[s]->coord[col][row] = GMT_weibull_crit (GMT, alpha, a, b);
	}
	return 0;
}

int table_WPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: WPDF 3 1 Weibull probability density function for x = A, scale = B and shape = C.  */
{
	unsigned int prev1 = last - 1, prev2 = last - 2;
	uint64_t s, row;
	double x, a, b;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev2]->constant && S[prev2]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument x to WPDF must be x >= 0!\n");
		return -1;
	}
	if (S[prev1]->constant && S[prev1]->factor <= 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument a to WPDF must be a positive (a > 0)!\n");
		return -1;
	}
	if (S[last]->constant && S[last]->factor <= 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error. Argument b to WPDF must be a positive (b > 0)!\n");
		return -1;
	}
	if (S[prev2]->constant && S[prev1]->constant && S[last]->constant) {	/* WPDF is given constant arguments */
		double value;
		x = S[prev2]->factor;
		a = S[prev1]->factor;	b = S[last]->factor;
		value = GMT_weibull_pdf (GMT, x, a, b);
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev2->segment[s]->coord[col][row] = value;
		return 0;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		x = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->coord[col][row];
		a = (S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->coord[col][row];
		b = (S[last]->constant)  ? S[last]->factor : T->segment[s]->coord[col][row];
		T_prev2->segment[s]->coord[col][row] = GMT_weibull_pdf (GMT, x, a, b);
	}
	return 0;
}

int table_XOR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: XOR 2 1 B if A == NaN, else A.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a = 0.0, b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	GMT_UNUSED(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) a = S[prev]->factor;
	if (S[last]->constant) b = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev]->constant) a = T_prev->segment[s]->coord[col][row];
		if (!S[last]->constant) b = T->segment[s]->coord[col][row];
		T_prev->segment[s]->coord[col][row] = (GMT_is_dnan (a)) ? b : a;
	}
	return 0;
}

int table_Y0 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: Y0 1 1 Bessel function of A (2nd kind, order 0).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand = 0 for Y0!\n");
	if (S[last]->constant) a = y0 (fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : y0 (fabs (T->segment[s]->coord[col][row]));
	return 0;
}

int table_Y1 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: Y1 1 1 Bessel function of A (2nd kind, order 1).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand = 0 for Y1!\n");
	if (S[last]->constant) a = y1 (fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : y1 (fabs (T->segment[s]->coord[col][row]));
	return 0;
}

int table_YN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: YN 2 1 Bessel function of A (2nd kind, order B).  */
{
	uint64_t s, row;
	unsigned int prev, order = 0;
	bool simple = false;
	double b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant && S[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order < 0 for YN!\n");
	if (S[last]->constant && fabs (rint(S[last]->factor) - S[last]->factor) > GMT_CONV4_LIMIT) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order not an integer for YN!\n");
	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, argument = 0 for YN!\n");
	if (S[last]->constant) order = urint (fabs (S[last]->factor));
	if (S[last]->constant && S[prev]->constant) {
		b = yn ((int)order, fabs (S[prev]->factor));
		simple = true;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (simple)
			T_prev->segment[s]->coord[col][row] = b;
		else {
			if (!S[last]->constant) order = urint (fabs (T->segment[s]->coord[col][row]));
			T_prev->segment[s]->coord[col][row] = yn ((int)order, fabs (T_prev->segment[s]->coord[col][row]));
		}
	}
	return 0;
}

int table_ZCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ZCRIT 1 1 Normal distribution critical value for alpha = A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_zcrit (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_zcrit (GMT, T->segment[s]->coord[col][row]);
	return 0;
}

int table_ZCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ZCDF 1 1 Normal cumulative distribution function for z = A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = GMT_zdist (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? a : GMT_zdist (GMT, T->segment[s]->coord[col][row]);
	return 0;
}

int table_ZPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ZPDF 1 1 Normal probability density function for z = A.  */
{
	uint64_t s, row;
	double z = 0.0, f = 1.0 / sqrt (TWO_PI);
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) z = f * exp (-0.5 * S[last]->factor * S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->coord[col][row] = (S[last]->constant) ? z : f * exp (-0.5 * T->segment[s]->coord[col][row] * T->segment[s]->coord[col][row]);
	return 0;
}

int table_ROOTS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ROOTS 2 1 Treats col A as f(t) = 0 and returns its roots.  */
{
	uint64_t seg, row;
	unsigned int i;
	int s_arg;
	double *roots = NULL;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	GMT_UNUSED(col);

	if (gmt_assign_ptrs (GMT, last, S, &T, &T_prev) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	/* Treats the chosen column (at there is only one) as f(t) and solves for t that makes f(t) == 0.
	 * For now we only solve using a linear spline but in the future this should depend on the users
	 * choice of INTERPOLANT. */

	if (info->roots_found) return 0;	/* Already been here */
	if (!S[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Argument to operator ROOTS must be a constant: the column number. Reset to 0\n");
		s_arg = 0;
	}
	else
		s_arg = irint (S[last]->factor);
	if (s_arg < 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Argument to operator ROOTS must be a column number 0 < col < %d. Reset to 0\n", info->n_col);
		s_arg = 0;
	}
	info->r_col = s_arg;
	if (info->r_col >= info->n_col) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Argument to operator ROOTS must be a column number 0 < col < %d. Reset to 0\n", info->n_col);
		info->r_col = 0;
	}
	roots = GMT_memory (GMT, NULL, T->n_records, double);
	info->n_roots = 0;
	if (T_prev->segment[0]->coord[info->r_col][0] == 0.0) roots[info->n_roots++] = info->T->segment[0]->coord[COL_T][0];
	for (seg = 0; seg < info->T->n_segments; seg++) {
		for (row = 1; row < info->T->segment[seg]->n_rows; row++) {
			if (T_prev->segment[seg]->coord[info->r_col][row] == 0.0) {
				roots[info->n_roots++] = info->T->segment[seg]->coord[COL_T][row];
				continue;
			}

			if ((T_prev->segment[seg]->coord[info->r_col][row-1] * T_prev->segment[seg]->coord[info->r_col][row]) < 0.0) {	/* Crossing 0 */
				roots[info->n_roots] = info->T->segment[seg]->coord[COL_T][row-1] - T_prev->segment[seg]->coord[info->r_col][row-1] * (info->T->segment[seg]->coord[COL_T][row] - info->T->segment[seg]->coord[COL_T][row-1]) / (T_prev->segment[seg]->coord[info->r_col][row] - T_prev->segment[seg]->coord[info->r_col][row-1]);
				info->n_roots++;
			}
		}
	}
	for (i = 0; i < info->n_roots; i++) T_prev->segment[0]->coord[info->r_col][i] = roots[i];
	GMT_free (GMT, roots);
	info->roots_found = true;
	return 0;
}

/* ---------------------- end operator functions --------------------- */

#include "gmtmath.h"

void Free_Stack (struct GMTAPI_CTRL *API, struct GMTMATH_STACK **stack)
{	unsigned int i;
	for (i = 0; i < GMTMATH_STACK_SIZE; i++) {
		if (stack[i]->alloc_mode == 2)
			GMT_Destroy_Data (API, &stack[i]->D);
		else if (stack[i]->alloc_mode == 1 && stack[i]->D)
			GMT_free_dataset (API->GMT, &stack[i]->D);
		GMT_free (API->GMT, stack[i]);
	}
}

void Free_Store (struct GMTAPI_CTRL *API, struct GMTMATH_STORED **recall)
{	unsigned int i;
	for (i = 0; i < GMTMATH_STORE_SIZE; i++) {
		if (recall[i] && !recall[i]->stored.constant) {
			GMT_free_dataset (API->GMT, &recall[i]->stored.D);
			GMT_free (API->GMT, recall[i]);
		}
	}
}

#define Free_Misc {if (T_in) GMT_Destroy_Data (API, &T_in); GMT_Destroy_Data (API, &Template); GMT_Destroy_Data (API, &Time); if (read_stdin) GMT_Destroy_Data (API, &D_stdin); }
#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return1(code) {GMT_Destroy_Options (API, &list); Free_gmtmath_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code); }
#define Return(code) {GMT_Destroy_Options (API, &list); Free_gmtmath_Ctrl (GMT, Ctrl); Free_Stack(API,stack); Free_Store(API,recall); Free_Misc;  GMT_end_module (GMT, GMT_cpy); bailout (code); }

int decode_gmt_argument (struct GMT_CTRL *GMT, char *txt, double *value, struct GMT_HASH *H) {
	unsigned int expect;
	int key;
	bool check = GMT_IS_NAN, possible_number = false;
	char copy[GMT_LEN256] = {""};
	char *mark = NULL;
	double tmp = 0.0;

	if (!txt) return (GMTMATH_ARG_IS_BAD);

	if (GMT_File_Is_Memory (txt)) return GMTMATH_ARG_IS_FILE;	/* Deal with memory references first */
	
	/* Check if argument is operator */

	if ((key = GMT_hash_lookup (GMT, txt, H, GMTMATH_N_OPERATORS, GMTMATH_N_OPERATORS)) >= GMTMATH_ARG_IS_OPERATOR) return (key);

	/* Next look for symbols with special meaning */

	if (!strcmp (txt, "STDIN")) return GMTMATH_ARG_IS_FILE;	/* read from stdin */
	if (!strncmp (txt, GMTMATH_STORE_CMD, strlen(GMTMATH_STORE_CMD))) return GMTMATH_ARG_IS_STORE;		/* store into mem location @<label>*/
	if (!strncmp (txt, GMTMATH_CLEAR_CMD, strlen(GMTMATH_CLEAR_CMD))) return GMTMATH_ARG_IS_CLEAR;		/* free mem location @<label>*/
	if (!strncmp (txt, GMTMATH_RECALL_CMD, strlen(GMTMATH_RECALL_CMD))) return GMTMATH_ARG_IS_RECALL;	/* load from mem location @<label>*/
	if (txt[0] == '@') return GMTMATH_ARG_IS_RECALL;							/* load from mem location @<label> */
	if (!(strcmp (txt, "PI") && strcmp (txt, "pi"))) return GMTMATH_ARG_IS_PI;
	if (!(strcmp (txt, "E") && strcmp (txt, "e"))) return GMTMATH_ARG_IS_E;
	if (!strcmp (txt, "F_EPS")) return GMTMATH_ARG_IS_F_EPS;
	if (!strcmp (txt, "D_EPS")) return GMTMATH_ARG_IS_D_EPS;
	if (!strcmp (txt, "EULER")) return GMTMATH_ARG_IS_EULER;
	if (!strcmp (txt, "TMIN")) return GMTMATH_ARG_IS_TMIN;
	if (!strcmp (txt, "TMAX")) return GMTMATH_ARG_IS_TMAX;
	if (!strcmp (txt, "TRANGE")) return GMTMATH_ARG_IS_TRANGE;
	if (!strcmp (txt, "TINC")) return GMTMATH_ARG_IS_TINC;
	if (!strcmp (txt, "N")) return GMTMATH_ARG_IS_N;
	if (!strcmp (txt, "TROW")) return GMTMATH_ARG_IS_J_MATRIX;
	if (!(strcmp (txt, "T") && strcmp (txt, "t"))) return GMTMATH_ARG_IS_T_MATRIX;
	if (!strcmp (txt, "TNORM")) return GMTMATH_ARG_IS_t_MATRIX;
	if (!strcmp (txt, "NaN")) {*value = GMT->session.d_NaN; return GMTMATH_ARG_IS_NUMBER;}

	/* Preliminary test-conversion to a number */

	strncpy (copy, txt, GMT_LEN256);
	if (!GMT_not_numeric (GMT, copy)) {	/* Only check if we are not sure this is NOT a number */
		expect = (strchr (copy, 'T')) ? GMT_IS_ABSTIME : GMT_IS_UNKNOWN;	/* Watch out for dateTclock-strings */
		check = GMT_scanf (GMT, copy, expect, &tmp);
		possible_number = true;
	}

	/* Determine if argument is file. Remove possible question mark. */

	mark = strchr (copy, '?');
	if (mark) *mark = '\0';
	if (!GMT_access (GMT, copy, R_OK)) {	/* Yes it is a file */
		if (check != GMT_IS_NAN && possible_number) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Your argument %s is both a file and a number.  File is selected\n", txt);
		return GMTMATH_ARG_IS_FILE;
	}

	if (check != GMT_IS_NAN) {	/* OK it is a number */
		*value = tmp;
		return GMTMATH_ARG_IS_NUMBER;
	}

	if (txt[0] == '-') {	/* Probably a bad commandline option */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Option %s not recognized\n", txt);
		return (GMTMATH_ARG_IS_BAD);
	}

	GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: %s is not a number, operator or file name\n", txt);
	return (GMTMATH_ARG_IS_BAD);	/* Dummy return to satisfy some compilers */
}

char *gmtmath_setlabel (struct GMT_CTRL *GMT, char *arg)
{
	char *label = strchr (arg, '@') + 1;	/* Label that follows @ */
	if (!label || label[0] == '\0') {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No label appended to STO|RCL|CLR operator!\n");
		return (NULL);
	}
	return (label);
}

void gmtmath_backwards_fixing (struct GMT_CTRL *GMT, char **arg)
{	/* Handle backwards compatible operator names */
	char *t = NULL, old[GMT_LEN16] = {""};
	if (!GMT_compat_check (GMT, 6)) return;	/* No checking so we may fail later */
	if (!strcmp (*arg, "CPOISS"))  {strcpy (old, *arg); free (*arg); *arg = t = strdup ("PCDF");   }
	if (!strcmp (*arg, "FDIST"))   {strcpy (old, *arg); free (*arg); *arg = t = strdup ("FCDF");   }
	if (!strcmp (*arg, "TDIST"))   {strcpy (old, *arg); free (*arg); *arg = t = strdup ("TCDF");   }
	if (!strcmp (*arg, "ZDIST"))   {strcpy (old, *arg); free (*arg); *arg = t = strdup ("ZCDF");   }
	if (!strcmp (*arg, "CHIDIST")) {strcpy (old, *arg); free (*arg); *arg = t = strdup ("CHICDF"); }
	if (!strcmp (*arg, "Tn"))      {strcpy (old, *arg); free (*arg); *arg = t = strdup ("TNORM");  }
	
	if (t)
		GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Operator %s is deprecated; use %s instead.\n", old, t);
}

int GMT_gmtmath (void *V_API, int mode, void *args)
{
	int i, k, op = 0, status = 0;
	unsigned int consumed_operands[GMTMATH_N_OPERATORS], produced_operands[GMTMATH_N_OPERATORS], new_stack = INT_MAX;
	unsigned int j, nstack = 0, n_stored = 0, kk;
	bool error = false, set_equidistant_t = false, got_t_from_file = false, free_time = false;
	bool read_stdin = false, t_check_required = true, touched_t_col = false, done, no_C = true;
	uint64_t use_t_col = 0, row, n_records, n_rows = 0, n_columns = 0, seg;
	
	uint64_t dim[4] = {1, 1, 0, 0};

	double t_noise = 0.0, value, off, scale, special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_N+1];

	char *label = NULL;
#include "gmtmath_op.h"

	int (*call_operator[GMTMATH_N_OPERATORS]) (struct GMT_CTRL *, struct GMTMATH_INFO *, struct GMTMATH_STACK **S, unsigned int, unsigned int);
	
	struct GMTMATH_STACK *stack[GMTMATH_STACK_SIZE];
	struct GMTMATH_STORED *recall[GMTMATH_STORE_SIZE];
	struct GMT_DATASET *A_in = NULL, *D_stdin = NULL, *D_in = NULL;
	struct GMT_DATASET *T_in = NULL, *Template = NULL, *Time = NULL, *R = NULL;
	struct GMT_DATATABLE *rhs = NULL, *D = NULL, *I = NULL;
	struct GMT_HASH localhashnode[GMTMATH_N_OPERATORS];
	struct GMT_OPTION *opt = NULL, *list = NULL;
	struct GMTMATH_INFO info;
	struct GMTMATH_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_gmtmath_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_gmtmath_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_gmtmath_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if ((list = gmt_substitute_macros (GMT, options, "gmtmath.macros")) == NULL) Return1 (EXIT_FAILURE);
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, list)) Return1 (API->error);
	Ctrl = New_gmtmath_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_gmtmath_parse (GMT, Ctrl, list))) Return1 (error);

	/*---------------------------- This is the gmtmath main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Perform reverse Polish notation calculations on data tables\n");

	if (Ctrl->Q.active || Ctrl->S.active) {	/* Turn off table and segment headers in calculator or one-record mode */
		GMT_set_tableheader (GMT, GMT_OUT, false);
		GMT_set_segmentheader (GMT, GMT_OUT, false);
	}

	GMT_memset (&info, 1, struct GMTMATH_INFO);
	GMT_memset (recall, GMTMATH_STORE_SIZE, struct GMTMATH_STORED *);

	t_check_required = !Ctrl->T.notime;	/* Turn off default GMT NaN-handling in t column */

	GMT_hash_init (GMT, localhashnode, operator, GMTMATH_N_OPERATORS, GMTMATH_N_OPERATORS);

	for (i = 0; i < GMTMATH_STACK_SIZE; i++) stack[i] = GMT_memory (GMT, NULL, 1, struct GMTMATH_STACK);

	GMT->current.io.skip_if_NaN[GMT_X] = GMT->current.io.skip_if_NaN[GMT_Y] = false;	/* Turn off default GMT NaN-handling of x/y (e.g. lon/lat columns) */
	GMT->current.io.skip_if_NaN[Ctrl->N.tcol] = t_check_required;	/* Determines if the t-column may have NaNs */

	/* Because of how gmtmath works we do not use GMT_Init_IO to register inputfiles */
	
	/* Read the first file we encounter so we may allocate space */

	/* Check sanity of all arguments and also look for an input file to get t from */
	for (opt = list, got_t_from_file = 0; got_t_from_file == 0 && opt; opt = opt->next) {
		if (!(opt->option == GMT_OPT_INFILE))	continue;	/* Skip command line options and output */
		/* Filenames,  operators, some numbers and = will all have been flagged as files by the parser */
		gmtmath_backwards_fixing (GMT, &(opt->arg));	/* Possibly exchange obsolete operator name for new one unless compatibility is off */
		op = decode_gmt_argument (GMT, opt->arg, &value, localhashnode);	/* Determine what this is */
		if (op == GMTMATH_ARG_IS_BAD) Return (EXIT_FAILURE);		/* Horrible */
		if (op != GMTMATH_ARG_IS_FILE) continue;				/* Skip operators and numbers */
		if (!got_t_from_file) {
			if (!strcmp (opt->arg, "STDIN")) {	/* Special stdin name.  We store this input in a special struct since we may need it again and it can only be read once! */
				if ((D_stdin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_STREAM, GMT_IS_NONE, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
					Return (API->error);
				}
				read_stdin = true;
				D_in = D_stdin;
				I = D_stdin->table[0];
			}
			else if ((D_in = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL | GMT_IO_RESET, NULL, opt->arg, NULL)) == NULL) {
				/* Read but request IO reset since the file (which may be a memory reference) will be read again later */
				Return (API->error);
			}
			got_t_from_file = 1;
		}
	}
	if (D_in) {	/* Read a file, update columns */
		if (Ctrl->N.active && Ctrl->N.ncol > D_in->n_columns) GMT_adjust_dataset (GMT, D_in, Ctrl->N.ncol);	/* Add more input columns */
		use_t_col  = Ctrl->N.tcol;
		n_columns  = D_in->n_columns;
		if (n_columns == 1) Ctrl->T.notime = true;	/* Only one column to work with implies -T */
	}
	set_equidistant_t = (Ctrl->T.active && !Ctrl->T.file && !Ctrl->T.notime);	/* We were given -Tmin/max/inc */
	if (D_in && set_equidistant_t) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Cannot use -T when data files are specified\n");
		Return (EXIT_FAILURE);
	}
	if (Ctrl->A.active) {	/* Always stored as t and f(t) in cols 0 and 1 */
		if (D_in) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Cannot have data files when -A is specified\n");
			Return (EXIT_FAILURE);
		}
		if ((A_in = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->A.file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error reading file %s\n", Ctrl->A.file);
			Return (API->error);
		}
		rhs = A_in->table[0];	/* Only one table */
		if (Ctrl->A.w_mode) {	/* Need at least 3 columns */
			if (rhs->n_columns < 3) {
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: -A requires a file with at least 3 (t,f(t),w(t)|s(t)) columns\n");
				Return (EXIT_FAILURE);
			}
		}
		else {	/* Need at least 2 columns */
			if (rhs->n_columns < 2) {
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: -A requires a file with at least 2 (t,f(t)) columns\n");
				Return (EXIT_FAILURE);
			}
		}
	}
	if (Ctrl->Q.active) {	/* Shorthand for -N1/0 -T0/0/1 -Ca */
		Ctrl->N.ncol = 1;
		Ctrl->N.tcol = 0;
		Ctrl->N.active = set_equidistant_t = true;
		Ctrl->T.min = Ctrl->T.max = 0.0;
		Ctrl->T.inc = 1.0;
		Ctrl->T.notime = true;
		n_rows = n_columns = 1;
	}
	if (Ctrl->N.active) {
		GMT->current.io.skip_if_NaN[GMT_X] = GMT->current.io.skip_if_NaN[GMT_Y] = false;
		GMT->current.io.skip_if_NaN[Ctrl->N.tcol] = true;
		n_columns = Ctrl->N.ncol;
	}
	if (Ctrl->T.active && Ctrl->T.file) {	/* Gave a file that we will use to obtain the T vector only */
		if (D_in) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Cannot use -T when data files are specified\n");
			Return (EXIT_FAILURE);
		}
		if ((T_in = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->T.file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error reading file %s\n", Ctrl->T.file);
			Return (API->error);
		}
		use_t_col = 0;
		got_t_from_file = 2;
	}

	if (set_equidistant_t && !Ctrl->Q.active) {
		/* Make sure the min/man/inc values harmonize */
		switch (GMT_minmaxinc_verify (GMT, Ctrl->T.min, Ctrl->T.max, Ctrl->T.inc, GMT_CONV4_LIMIT)) {
			case 1:
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T options: (max - min) is not a whole multiple of inc\n");
				Return (EXIT_FAILURE);
				break;
			case 2:
				if (Ctrl->T.inc != 1.0) {	/* Allow for somebody explicitly saying -T0/0/1 */
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T options: (max - min) is <= 0\n");
					Return (EXIT_FAILURE);
				}
				break;
			case 3:
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T options: inc is <= 0\n");
				Return (EXIT_FAILURE);
				break;
			default:	/* OK */
				break;
		}

		n_rows = lrint ((Ctrl->T.max - Ctrl->T.min) / Ctrl->T.inc) + 1;
		n_columns = Ctrl->N.ncol;
	}

	if (Ctrl->A.active) {	/* Get number of rows and time from the file, but not n_cols (that takes -N, which defaults to 2) */
		n_rows = rhs->n_records;
		n_columns = Ctrl->N.ncol;
		Ctrl->T.min = rhs->min[0];
		Ctrl->T.max = rhs->max[0];
		Ctrl->T.inc = (rhs->max[0] - rhs->min[0]) / (rhs->n_records - 1);
	}
	if (Ctrl->T.file) n_columns = Ctrl->N.ncol;

	if (!(D_in || T_in || A_in || set_equidistant_t)) {	/* Neither a file nor -T given; must read data from stdin */
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expression must contain at least one table file or -T [and -N]\n");
		Return (EXIT_FAILURE);
	}
	if (D_in)	/* Obtained file structure from an input file, use this to create new stack entry */
		Template = GMT_Duplicate_Data (API, GMT_IS_DATASET, GMT_DUPLICATE_DATA, D_in);
		
	else {		/* Must use -N -T etc to create single segment */
		dim[GMT_COL] = n_columns;	dim[GMT_ROW] = n_rows;
		if ((Template = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (GMT_MEMORY_ERROR);
	}
	stack[0]->alloc_mode = 1;	/* Allocated locally */
	Ctrl->N.ncol = n_columns;
	if (!Ctrl->T.notime && n_columns > 1) Ctrl->C.cols[Ctrl->N.tcol] = (Ctrl->Q.active) ? false : true;
	
	/* Create the Time data structure with 3 cols: 0 is t, 1 is normalized tn, 2 is row numbers */
	if (D_in) {	/* Either D_in or D_stdin */
		Time = GMT_alloc_dataset (GMT, D_in, 0, 3, GMT_ALLOC_NORMAL);
		free_time = true;
		info.T = Time->table[0];	D = D_in->table[0];
		for (seg = 0, done = false; seg < D->n_segments; seg++) {
			GMT_memcpy (info.T->segment[seg]->coord[COL_T], D->segment[seg]->coord[use_t_col], D->segment[seg]->n_rows, double);
			if (!done) {
				for (row = 1; row < info.T->segment[seg]->n_rows && (GMT_is_dnan (info.T->segment[seg]->coord[COL_T][row-1]) || GMT_is_dnan (info.T->segment[seg]->coord[COL_T][row])); row++);	/* Find the first real two records in a row */
				Ctrl->T.inc = (row == info.T->segment[seg]->n_rows) ? GMT->session.d_NaN : info.T->segment[seg]->coord[COL_T][row] - info.T->segment[seg]->coord[COL_T][row-1];
				t_noise = fabs (GMT_CONV4_LIMIT * Ctrl->T.inc);
			}
			for (row = 1; row < info.T->segment[seg]->n_rows && !info.irregular; row++) if (fabs (fabs (info.T->segment[seg]->coord[COL_T][row] - info.T->segment[seg]->coord[COL_T][row-1]) - fabs (Ctrl->T.inc)) > t_noise) info.irregular = true;
		}
		if (!read_stdin && GMT_Destroy_Data (API, &D_in) != GMT_OK) {
			Return (API->error);
		}
	}
	else {	/* Create orderly output */
		dim[GMT_COL] = 3;	dim[GMT_ROW] = n_rows;
		if ((Time = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (GMT_MEMORY_ERROR);
		info.T = Time->table[0];
		for (row = 0; row < info.T->segment[0]->n_rows; row++) info.T->segment[0]->coord[COL_T][row] = (row == (info.T->segment[0]->n_rows-1)) ? Ctrl->T.max: Ctrl->T.min + row * Ctrl->T.inc;
		t_noise = fabs (GMT_CONV4_LIMIT * Ctrl->T.inc);
	}

	for (seg = n_records = 0; seg < info.T->n_segments; seg++) {	/* Create normalized times and possibly reverse time (-I) */
		off = 0.5 * (info.T->segment[seg]->coord[COL_T][info.T->segment[seg]->n_rows-1] + info.T->segment[seg]->coord[COL_T][0]);
		scale = 2.0 / (info.T->segment[seg]->coord[COL_T][info.T->segment[seg]->n_rows-1] - info.T->segment[seg]->coord[COL_T][0]);
		if (Ctrl->I.active) for (row = 0; row < info.T->segment[seg]->n_rows/2; row++) double_swap (info.T->segment[seg]->coord[COL_T][row], info.T->segment[seg]->coord[COL_T][info.T->segment[seg]->n_rows-1-row]);	/* Reverse time-series */
		for (row = 0; row < info.T->segment[seg]->n_rows; row++) {
			info.T->segment[seg]->coord[COL_TN][row] = (info.T->segment[seg]->coord[COL_T][row] - off) * scale;
			info.T->segment[seg]->coord[COL_TJ][row] = (unsigned int)((Ctrl->I.active) ? info.T->segment[seg]->n_rows - row - 1 : row);
		}
		n_records += info.T->segment[seg]->n_rows;
	}
	info.t_min = Ctrl->T.min;	info.t_max = Ctrl->T.max;	info.t_inc = Ctrl->T.inc;
	info.n_col = n_columns;		info.local = Ctrl->L.active;
	info.notime = Ctrl->T.notime;
	GMT_set_tbl_minmax (GMT, info.T);

	if (Ctrl->A.active) {	/* Set up A * x = b, with the table holding the extended matrix [ A | [w | ] b ], with w the optional weights */
		if (!stack[0]->D) {
			stack[0]->D = GMT_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
			stack[0]->alloc_mode = 1;
		}
		load_column (stack[0]->D, n_columns-1, rhs, 1);		/* Always put the r.h.s of the Ax = b equation in the last column of the item on the stack */
		if (!Ctrl->A.null) load_column (stack[0]->D, Ctrl->N.tcol, rhs, 0);	/* Optionally, put the t vector in the time column of the item on the stack */
		GMT_set_tbl_minmax (GMT, stack[0]->D->table[0]);
		nstack = 1;
		info.fit_mode = Ctrl->A.e_mode;
		info.w_mode = Ctrl->A.w_mode;
	}
	else
		nstack = 0;

	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_PI]     = M_PI;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_E]      = M_E;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_F_EPS]   = FLT_EPSILON;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_D_EPS]   = DBL_EPSILON;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_EULER]  = M_EULER;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_TMIN]   = Ctrl->T.min;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_TMAX]   = Ctrl->T.max;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_TRANGE] = Ctrl->T.max - Ctrl->T.min;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_TINC]   = Ctrl->T.inc;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_N]      = (double)n_records;

	gmtmath_init (call_operator, consumed_operands, produced_operands);
	op = decode_gmt_argument (GMT, "EXCH", &value, localhashnode);
	consumed_operands[op] = produced_operands[op] = 0;	/* Modify items since we simply swap pointers */

	for (opt = list, error = false; !error && opt; opt = opt->next) {

		/* First check if we should skip optional arguments */

		if (strchr ("AEINQSTVbfghios-" GMT_OPT("FHMm"), opt->option)) continue;
		if (opt->option == 'C') {	/* Change affected columns */
			no_C = false;
			if (decode_columns (opt->arg, Ctrl->C.cols, n_columns, Ctrl->N.tcol)) touched_t_col = true;
			continue;
		}
		if (opt->option == GMT_OPT_OUTFILE) continue;	/* We do output after the loop */

		gmtmath_backwards_fixing (GMT, &(opt->arg));	/* Possibly exchange obsolete operator name for new one unless compatibility is off */

		op = decode_gmt_argument (GMT, opt->arg, &value, localhashnode);

		if (op != GMTMATH_ARG_IS_FILE && !GMT_access (GMT, opt->arg, R_OK)) GMT_Message (API, GMT_TIME_NONE, "Warning: The number or operator %s may be confused with an existing file %s!\n", opt->arg, opt->arg);

		if (op < GMTMATH_ARG_IS_OPERATOR) {	/* File name or factor */

			if (nstack == GMTMATH_STACK_SIZE) {	/* Stack overflow */
				GMT_Report (API, GMT_MSG_NORMAL, "Stack overflow!\n");
				Return (EXIT_FAILURE);
			}

			if (op == GMTMATH_ARG_IS_NUMBER) {
				stack[nstack]->constant = true;
				stack[nstack]->factor = value;
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "%g ", stack[nstack]->factor);
				nstack++;
				continue;
			}
			else if (op <= GMTMATH_ARG_IS_PI && op >= GMTMATH_ARG_IS_N) {
				stack[nstack]->constant = true;
				stack[nstack]->factor = special_symbol[GMTMATH_ARG_IS_PI-op];
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "%g ", stack[nstack]->factor);
				nstack++;
				continue;
			}
			else if (op == GMTMATH_ARG_IS_STORE) {
				/* Duplicate stack into stored memory location */
				int last = nstack - 1;
				bool new = false;
				if (nstack == 0) {
					GMT_Report (API, GMT_MSG_NORMAL, "No items on stack to put into stored memory!\n");
					Return (EXIT_FAILURE);
				}
				if ((label = gmtmath_setlabel (GMT, opt->arg)) == NULL) Return (EXIT_FAILURE);
				if ((k = gmtmath_find_stored_item (recall, n_stored, label)) != -1) {
					if (!stack[last]->constant) for (j = 0; j < n_columns; j++) if (no_C || !Ctrl->C.cols[j]) load_column (recall[k]->stored.D, j, stack[last]->D->table[0], j);
					GMT_Report (API, GMT_MSG_DEBUG, "Stored memory cell %d named %s is overwritten with new information\n", k, label);
				}
				else {	/* Need new named storage place; use GMT_duplicate_dataset/GMT_free_dataset since no point adding to registered resources since internal use only */
					k = n_stored;
					recall[k] = GMT_memory (GMT, NULL, 1, struct GMTMATH_STORED);
					recall[k]->label = strdup (label);
					if (!stack[last]->constant) recall[k]->stored.D = GMT_duplicate_dataset (GMT, stack[last]->D, GMT_ALLOC_NORMAL, NULL);
					new = true;
					GMT_Report (API, GMT_MSG_DEBUG, "Stored memory cell %d named %s is created with new information\n", k, label);
				}
				recall[k]->stored.constant = stack[last]->constant;
				recall[k]->stored.factor = stack[last]->factor;
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "[--> %s] ", recall[k]->label);
				if (new) n_stored++;	/* We added a new item */
				continue;	/* Just go back and process next item */
			}
			else if (op == GMTMATH_ARG_IS_RECALL) {
				/* Add to stack from stored memory location */
				if ((label = gmtmath_setlabel (GMT, opt->arg)) == NULL) Return (EXIT_FAILURE);
				if ((k = gmtmath_find_stored_item (recall, n_stored, label)) == -1) {
					GMT_Report (API, GMT_MSG_NORMAL, "No stored memory item with label %s exists!\n", label);
					Return (EXIT_FAILURE);
				}
				if (recall[k]->stored.constant) {	/* Place a stored constant on the stack */
					stack[nstack]->constant = true;
					stack[nstack]->factor = recall[k]->stored.factor;
				}
				else {/* Place the stored dataset on the stack */
					stack[nstack]->constant = false;
					if (!stack[nstack]->D) {
						stack[nstack]->D = GMT_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
						stack[nstack]->alloc_mode = 1;
					}
					for (j = 0; j < n_columns; j++) if (no_C || !Ctrl->C.cols[j]) load_column (stack[nstack]->D, j, recall[k]->stored.D->table[0], j);
					GMT_set_tbl_minmax (GMT, stack[nstack]->D->table[0]);
				}
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "@%s ", recall[k]->label);
				nstack++;
				continue;
			}
			else if (op == GMTMATH_ARG_IS_CLEAR) {
				/* Free stored memory location */
				if ((label = gmtmath_setlabel (GMT, opt->arg)) == NULL) Return (EXIT_FAILURE);
				if ((k = gmtmath_find_stored_item (recall, n_stored, label)) == -1) {
					GMT_Report (API, GMT_MSG_NORMAL, "No stored memory item with label %s exists!\n", label);
					Return (EXIT_FAILURE);
				}
				if (recall[k]->stored.D) GMT_free_dataset (GMT, &recall[k]->stored.D);
				free ((void *)recall[k]->label);
				GMT_free (GMT, recall[k]);
				while (k && k == (int)(n_stored-1) && !recall[k]) k--, n_stored--;	/* Chop off trailing NULL cases */
				continue;	/* Just go back and process next item */
			}

			/* Here we need a matrix */

			stack[nstack]->constant = false;

			if (op == GMTMATH_ARG_IS_T_MATRIX) {	/* Need to set up matrix of t-values */
				if (Ctrl->T.notime) {
					GMT_Report (API, GMT_MSG_NORMAL, "T is not defined for plain data files!\n");
					Return (EXIT_FAILURE);
				}
				if (!stack[nstack]->D) {
					stack[nstack]->D = GMT_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
					stack[nstack]->alloc_mode = 1;
				}
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "T ");
				for (j = 0; j < n_columns; j++) if (no_C || !Ctrl->C.cols[j]) load_column (stack[nstack]->D, j, info.T, COL_T);
				GMT_set_tbl_minmax (GMT, stack[nstack]->D->table[0]);
			}
			else if (op == GMTMATH_ARG_IS_t_MATRIX) {	/* Need to set up matrix of normalized t-values */
				if (Ctrl->T.notime) {
					GMT_Report (API, GMT_MSG_NORMAL, "TNORM is not defined for plain data files!\n");
					Return (EXIT_FAILURE);
				}
				if (!stack[nstack]->D) {
					stack[nstack]->D = GMT_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
					stack[nstack]->alloc_mode = 1;
				}
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "TNORM ");
				for (j = 0; j < n_columns; j++) if (no_C || !Ctrl->C.cols[j]) load_column (stack[nstack]->D, j, info.T, COL_TN);
				GMT_set_tbl_minmax (GMT, stack[nstack]->D->table[0]);
			}
			else if (op == GMTMATH_ARG_IS_J_MATRIX) {	/* Need to set up matrix of row numbers */
				if (!stack[nstack]->D) {
					stack[nstack]->D = GMT_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
					stack[nstack]->alloc_mode = 1;
				}
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "TROW ");
				for (j = 0; j < n_columns; j++) if (no_C || !Ctrl->C.cols[j]) load_column (stack[nstack]->D, j, info.T, COL_TJ);
				GMT_set_tbl_minmax (GMT, stack[nstack]->D->table[0]);
			}
			else if (op == GMTMATH_ARG_IS_FILE) {		/* Filename given */
				struct GMT_DATASET *F = NULL;
				struct GMT_DATATABLE *T_in = NULL;
				if (!stack[nstack]->D) {
					stack[nstack]->D = GMT_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
					stack[nstack]->alloc_mode = 1;
				}
				if (!strcmp (opt->arg, "STDIN")) {	/* stdin file */
					T_in = I;
					if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "<stdin> ");
				}
				else {
					if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "%s ", opt->arg);
					if ((F = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, opt->arg, NULL)) == NULL) {
						GMT_Report (API, GMT_MSG_NORMAL, "Error reading file %s\n", opt->arg);
						Return (API->error);
					}
					if (Ctrl->N.ncol > F->n_columns) GMT_adjust_dataset (GMT, F, Ctrl->N.ncol);	/* Add more input columns */
					T_in = F->table[0];	/* Only one table since only a single file */
				}
				for (j = 0; j < n_columns; j++) if (no_C || !Ctrl->C.cols[j]) load_column (stack[nstack]->D, j, T_in, j);
				GMT_set_tbl_minmax (GMT, stack[nstack]->D->table[0]);
				if (!same_size (stack[nstack]->D, Template)) {
					GMT_Report (API, GMT_MSG_NORMAL, "tables not of same size!\n");
					Return (EXIT_FAILURE);
				}
				else if (!Ctrl->C.cols[Ctrl->N.tcol] && !(Ctrl->T.notime || same_domain (stack[nstack]->D, Ctrl->N.tcol, info.T))) {
					GMT_Report (API, GMT_MSG_NORMAL, "tables do not cover the same domain!\n");
					Return (EXIT_FAILURE);
				}
				if (T_in != I && GMT_Destroy_Data (API, &F) != GMT_OK) {
					Return (API->error);
				}
			}
			nstack++;
			continue;
		}

		/* Here we have an operator */

		if (!strncmp (opt->arg, "ROOTS", 5U) && !(opt->next && opt->next->arg[0] == '=')) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Only = may follow operator ROOTS\n");
			Return (EXIT_FAILURE);
		}

		if ((new_stack = nstack - consumed_operands[op] + produced_operands[op]) >= GMTMATH_STACK_SIZE) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Stack overflow (%s)\n", opt->arg);
			Return (EXIT_FAILURE);
		}

		if (nstack < consumed_operands[op]) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Operation \"%s\" requires %d operands\n", operator[op], consumed_operands[op]);
			Return (EXIT_FAILURE);
		}

		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "%s ", operator[op]);

		for (i = produced_operands[op] - consumed_operands[op]; i > 0; i--) {
			if (stack[nstack+i-1]->D)	continue;

			/* Must make space for more */

			stack[nstack+i-1]->D = GMT_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
			stack[nstack+i-1]->alloc_mode = 1;
		}

		/* If operators operates on constants only we may have to make space as well */

		for (j = 0, i = nstack - consumed_operands[op]; j < produced_operands[op]; j++, i++) {
			if (stack[i]->constant && !stack[i]->D) {
				stack[i]->D = GMT_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
				stack[i]->alloc_mode = 1;
				if (!Ctrl->T.notime) load_column (stack[i]->D, COL_T, info.T, COL_T);	/* Make sure t-column is copied if needed */
			}
		}

		if (!strcmp (operator[op], "LSQFIT")) {	/* Special case, solve LSQ system and return */
			solve_LSQFIT (GMT, &info, stack[nstack-1], n_columns, Ctrl->C.cols, Ctrl->E.eigen, Ctrl->Out.file, options, A_in);
			Return (EXIT_SUCCESS);
		}
		else if (!strcmp (operator[op], "SVDFIT")) {	/* Special case, solve SVD system and return */
			solve_SVDFIT (GMT, &info, stack[nstack-1], n_columns, Ctrl->C.cols, Ctrl->E.eigen, Ctrl->Out.file, options, A_in);
			Return (EXIT_SUCCESS);
		}

		for (j = 0; j < n_columns; j++) {
			if (Ctrl->C.cols[j]) continue;
			status = (*call_operator[op]) (GMT, &info, stack, nstack - 1, j);	/* Do it */
			if (status == -1) {	/* Serious problem, need to bail */
				GMT_exit (GMT, EXIT_FAILURE); Return (EXIT_FAILURE);
			}
		}

		nstack = new_stack;

		for (kk = 1; kk <= produced_operands[op]; kk++) if (stack[nstack-kk]->D) stack[nstack-kk]->constant = false;	/* Now filled with table */
	}

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		(Ctrl->Out.file) ? GMT_Message (API, GMT_TIME_NONE, "= %s", Ctrl->Out.file) : GMT_Message (API, GMT_TIME_NONE,  "= <stdout>");
	}

	if (stack[0]->constant) {	/* Only a constant provided, set table accordingly */
		if (!stack[0]->D) {
			stack[0]->D = GMT_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
			stack[0]->alloc_mode = 1;
		}
		for (j = 0; j < n_columns; j++) {
			if (j == COL_T && !Ctrl->Q.active && Ctrl->C.cols[j])
				load_column (stack[0]->D, j, info.T, COL_T);
			else if (!Ctrl->C.cols[j])
				load_const_column (stack[0]->D, j, stack[0]->factor);
		}
		GMT_set_tbl_minmax (GMT, stack[0]->D->table[0]);
	}

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "\n");

	if (info.roots_found) {	/* Special treatment of root finding */
		struct GMT_DATASEGMENT *S = stack[0]->D->table[0]->segment[0];
		uint64_t dim[4] = {1, 1, 0, 1};
		
		dim[GMT_ROW] = info.n_roots;
		if ((R = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error)
		for (kk = 0; kk < info.n_roots; kk++) R->table[0]->segment[0]->coord[GMT_X][kk] = S->coord[info.r_col][kk];
		GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, R);
		if (GMT_Write_Data (API, GMT_IS_DATASET, (Ctrl->Out.file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_NONE, stack[0]->D->io_mode, NULL, Ctrl->Out.file, R) != GMT_OK) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &R) != GMT_OK) {
			Return (API->error);
		}
	}
	else {	/* Regular table result */
		bool template_used = false, place_t_col = (Ctrl->T.active && t_check_required && !Ctrl->Q.active && !touched_t_col);
		
		if (stack[0]->D)	/* There is an output stack, select it */
			R = stack[0]->D;
		else {		/* Can happen if only -T [-N] was specified with no operators */
			R = Template;
			template_used = true;
		}
		if (place_t_col && Ctrl->N.tcol < R->n_columns) {
			load_column (R, Ctrl->N.tcol, info.T, COL_T);	/* Put T in the time column of the item on the stack if possible */
			GMT_set_tbl_minmax (GMT, R->table[0]);
		}
		if ((error = GMT_set_cols (GMT, GMT_OUT, R->n_columns))) Return (error);	/* Since -bo might have been used */
		if (Ctrl->S.active) {	/* Only get one record */
			uint64_t r, row;
			uint64_t nr, c;
			struct GMT_DATASET *N = NULL;
			nr = (Ctrl->S.active) ? 1 : 0;
			if ((N = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) return (GMT->parent->error);
			N = GMT_alloc_dataset (GMT, R, nr, n_columns, GMT_ALLOC_NORMAL);
			for (seg = 0; seg < R->table[0]->n_segments; seg++) {
				for (r = 0; r < N->table[0]->segment[seg]->n_rows; r++) {
					row = (Ctrl->S.active) ? ((Ctrl->S.mode == -1) ? 0 : R->table[0]->segment[seg]->n_rows - 1) : r;
					for (c = 0; c < n_columns; c++) N->table[0]->segment[seg]->coord[c][r] = R->table[0]->segment[seg]->coord[c][row];
				}
			}
			GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, N);
			if (GMT_Write_Data (API, GMT_IS_DATASET, (Ctrl->Out.file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_NONE, N->io_mode, NULL, Ctrl->Out.file, N) != GMT_OK) {
				Return (API->error);
			}
			GMT_free_dataset (API->GMT, &N);
		}
		else {	/* Write the whole enchilada */
			GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, R);
			if (GMT_Write_Data (API, GMT_IS_DATASET, (Ctrl->Out.file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_NONE, R->io_mode, NULL, Ctrl->Out.file, R) != GMT_OK) {
				Return (API->error);
			}
		}
		if (template_used) Template = NULL;	/* This prevents it from being freed twice (once from API registration via GMT_Write_Data and then again in Free_Misc) */
	}

	/* Clean-up time */

	if (Ctrl->A.active && GMT_Destroy_Data (API, &A_in) != GMT_OK) {
		Return (API->error);
	}

	if (free_time) GMT_free_dataset (GMT, &Time);
	for (kk = 0; kk < n_stored; kk++) {	/* Free up stored STO/RCL memory */
		if (recall[kk]->stored.D) GMT_free_dataset (GMT, &recall[kk]->stored.D);
		free ((void *)recall[kk]->label);
		GMT_free (GMT, recall[kk]);
	}
	
	if (nstack > 1) GMT_Report (API, GMT_MSG_NORMAL, "Warning: %d more operands left on the stack!\n", nstack-1);

	Return (EXIT_SUCCESS);
}
