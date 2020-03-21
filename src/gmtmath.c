/*--------------------------------------------------------------------
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
 * API functions to support the gmtmath application.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: gmtmath.c is a reverse polish calculator that operates
 * on table files (and constants) and perform basic mathematical operations
 * on them like add, multiply, etc.
 * Some operators only work on one operand (e.g., log, exp)
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"gmtmath"
#define THIS_MODULE_MODERN_NAME	"gmtmath"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Reverse Polish Notation (RPN) calculator for data tables"
#define THIS_MODULE_KEYS	"<D(,AD(=,TD(,>D}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-:>Vbdefghioqs" GMT_OPT("HMm")

#define SPECIFIC_OPTIONS "AEILNQST"	/* All non-common options except for -C which we will actually process in the loop over args */

EXTERN_MSC int gmt_load_macros (struct GMT_CTRL *GMT, char *mtype, struct GMT_MATH_MACRO **M);
EXTERN_MSC int gmt_find_macro (char *arg, unsigned int n_macros, struct GMT_MATH_MACRO *M);
EXTERN_MSC void gmt_free_macros (struct GMT_CTRL *GMT, unsigned int n_macros, struct GMT_MATH_MACRO **M);
EXTERN_MSC struct GMT_OPTION * gmt_substitute_macros (struct GMT_CTRL *GMT, struct GMT_OPTION *options, char *mfile);

#define GMTMATH_ARG_IS_OPERATOR	 0
#define GMTMATH_ARG_IS_FILE	-1
#define GMTMATH_ARG_IS_NUMBER	-2
#define GMTMATH_ARG_IS_PI	-3
#define GMTMATH_ARG_IS_E	-4
#define GMTMATH_ARG_IS_F_EPS	-5
#define GMTMATH_ARG_IS_D_EPS	-6
#define GMTMATH_ARG_IS_EULER	-7
#define GMTMATH_ARG_IS_PHI	-8
#define GMTMATH_ARG_IS_TMIN	-9
#define GMTMATH_ARG_IS_TMAX	-10
#define GMTMATH_ARG_IS_TRANGE	-11
#define GMTMATH_ARG_IS_TINC	-12
#define GMTMATH_ARG_IS_N	-13
#define GMTMATH_ARG_IS_J_MATRIX	-14
#define GMTMATH_ARG_IS_T_MATRIX	-15
#define GMTMATH_ARG_IS_t_MATRIX	-16
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
		struct GMT_ARRAY T;
	} T;
};

struct GMTMATH_INFO {
	bool irregular;	/* true if t_inc varies */
	bool roots_found;	/* true if roots have been solved for */
	bool local;		/* Per segment operation (true) or global operation (false) */
	bool notime;		/* No time-array available for operators who depend on that */
	bool scalar;		/* -Q in effect */
	unsigned int n_roots;	/* Number of roots found */
	unsigned int fit_mode;	/* Used for {LSQ|SVD}FIT */
	unsigned int w_mode;	/* Used for weighted fit */
	uint64_t r_col;	/* The column used to find roots */
	uint64_t n_col;	/* Number of columns */
	double t_min, t_max, t_inc;
	struct GMT_DATATABLE *T;	/* Table with all time information */
	struct GMT_ORDER **Q;		/* For sorting columns */
};

struct GMTMATH_STACK {
	struct GMT_DATASET *D;	/* The dataset */
	bool constant;			/* true if a constant (see factor) and S == NULL */
	double factor;			/* The value if constant is true */
};

struct GMTMATH_STORED {
	char *label;	/* Name of this stored memory */
	struct GMTMATH_STACK stored;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMTMATH_CTRL *C = gmt_M_memory (GMT, NULL, 1, struct GMTMATH_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->C.cols = gmt_M_memory (GMT, NULL, GMT_MAX_COLUMNS, bool);
	C->E.eigen = 1e-7;	/* Default cutoff of small eigenvalues */
	C->N.ncol = 2;

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMTMATH_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->Out.file);
	gmt_M_str_free (C->A.file);
	gmt_M_free (GMT, C->C.cols);
	gmt_free_array (GMT, &(C->T.T));
	gmt_M_free (GMT, C);
}

GMT_LOCAL bool decode_columns (char *txt, bool *skip, uint64_t n_col, uint64_t t_col) {
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
		while ((gmt_strtok (txt, ",", &pos, p))) {
			if (strchr (p, '-'))
				sscanf (p, "%" PRIu64 "-%" PRIu64, &start, &stop);
			else if (strchr (p, 'x')) /* -Cx is the x/lon column */
				start = stop = GMT_X;
			else if (strchr (p, 'y')) /* -Cy is the y/lat column */
				start = stop = GMT_Y;
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

GMT_LOCAL int gmtmath_find_stored_item (struct GMTMATH_STORED *recall[], int n_stored, char *label) {
	/* Linear search to find the named storage item */
	int k = 0;
	while (k < n_stored && strcmp (recall[k]->label, label)) k++;
	return (k == n_stored ? GMT_NOTSET : k);
}

GMT_LOCAL void load_column (struct GMT_DATASET *to, uint64_t to_col, struct GMT_DATATABLE *from, uint64_t from_col) {
	/* Copies data from one column to another */
	uint64_t seg;
	for (seg = 0; seg < from->n_segments; seg++) {
		gmt_M_memcpy (to->table[0]->segment[seg]->data[to_col], from->segment[seg]->data[from_col], from->segment[seg]->n_rows, double);
	}
}

/* ---------------------- start convenience functions --------------------- */

GMT_LOCAL int solve_LS_system (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S, uint64_t n_col, bool skip[], char *file, bool svd, double eigen_min, struct GMT_OPTION *options, struct GMT_DATASET *A) {

	/* Consider the current table the augmented matrix [A | b], making up the linear system Ax = b.
	 * We will set up the normal equations, solve for x, and output the solution before quitting.
	 * This function is special since it operates across columns and returns n_col scalars.
	 * We try to solve this positive definite & symmetric matrix with Cholesky methods; if that fails
	 * we do a full SVD decomposition and set small eigenvalues to zero, yielding an approximate solution.
	 * However, if svd == true then we do the full SVD.
	 */

	unsigned int i, j, k, k0, i2, j2, n;
	int ier = 0;
	uint64_t row, seg, rhs, w_col = 0;
	double cond, w = 1.0;
	double *N = NULL, *r = NULL, *d = NULL, *x = NULL;
	struct GMT_DATATABLE *T = S->D->table[0];
	struct GMT_DATASET *D = NULL;
	gmt_M_unused(A);

	for (i = n = 0; i < n_col; i++) if (!skip[i]) n++;	/* Need to find how many active columns we have */
	if (n < 2) {
		char *pre[2] = {"LSQ", "SVD"};
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "%sFIT requires at least 2 active columns!\n", pre[svd]);
		return (GMT_DIM_TOO_SMALL);
	}
	rhs = n_col - 1;
	while (rhs > 0 && skip[rhs]) rhs--;	/* Get last active col number as the rhs vector b */
	n--;					/* Account for b, the rhs vector, to get row & col dimensions of normal matrix N */
	if (info->w_mode) {
		w_col = rhs - 1;		/* If there are weights, this is the column they are stored in */
		while (w_col > 0 && skip[w_col]) w_col--;	/* Get next active col number as the weight vector w */
		n--;					/* Account for w, the rhs vector, to get row & col dimensions of normal matrix N */
	}

	N = gmt_M_memory (GMT, NULL, n*n, double);
	r = gmt_M_memory (GMT, NULL, T->n_records, double);

#if 0
	fprintf (stderr, "Printout of A | b matrix\n");
	fprintf (stderr, "------------------------------------------------------------------\n");
	for (seg = 0; seg < info->T->n_segments; seg++) {
		for (row = 0; row < T->segment[seg]->n_rows; row++) {
			for (j = 0; j < rhs; j++) {
				if (skip[j]) continue;
				fprintf (stderr, "%g\t", T->segment[seg]->data[j][row]);
			}
			fprintf (stderr, "|\t%g\n", T->segment[seg]->data[rhs][row]);
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
						w = pow (T->segment[seg]->data[w_col][row], 2.0);
						if (info->w_mode == GMTMATH_SIGMAS) w = 1.0 / w;	/* Got sigma */
					}
					N[k0] += w * T->segment[seg]->data[j2][row] * T->segment[seg]->data[i2][row];
				}
			}
			i++;
		}
		r[j] = 0.0;
		for (seg = k = 0; seg < info->T->n_segments; seg++) {
			for (row = 0; row < T->segment[seg]->n_rows; row++, k++) {
				if (info->w_mode) {
					w = pow (T->segment[seg]->data[w_col][row], 2.0);
					if (info->w_mode == GMTMATH_SIGMAS) w = 1.0 / w;	/* Got sigma */
				}
				r[j] += w * T->segment[seg]->data[j2][row] * T->segment[seg]->data[rhs][row];
			}
		}
		j++;
	}
	if (svd)
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Solve LS system via SVD decomposition and exclude eigenvalues < %g.\n", eigen_min);
	else
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Solve LS system via Cholesky decomposition\n");

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
	d = gmt_M_memory (GMT, NULL, n, double);
	x = gmt_M_memory (GMT, NULL, n, double);
	if (svd || ((ier = gmt_chol_dcmp (GMT, N, d, &cond, n, n) ) != 0)) {	/* Cholesky decomposition failed, use SVD method, or use SVD if specified */
		unsigned int nrots;
		double *b = NULL, *z = NULL, *v = NULL, *lambda = NULL;
		if (!svd) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING,
			            "Cholesky decomposition failed, try SVD decomposition instead and exclude eigenvalues < %g.\n", eigen_min);
			gmt_chol_recover (GMT, N, d, n, n, ier, true);	/* Restore to former matrix N */
		}
		/* Solve instead using the SVD of a square matrix via gmt_jacobi */
		lambda = gmt_M_memory (GMT, NULL, n, double);
		b = gmt_M_memory (GMT, NULL, n, double);
		z = gmt_M_memory (GMT, NULL, n, double);
		v = gmt_M_memory (GMT, NULL, n*n, double);

		if (gmt_jacobi (GMT, N, n, n, lambda, v, b, z, &nrots)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Eigenvalue routine failed to converge in 50 sweeps.\n");
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "The solution might be inaccurate.\n");
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
		if (k) GMT_Report (GMT->parent, GMT_MSG_WARNING, "%d eigenvalues < %g set to zero to yield a stable solution\n", k, eigen_min);

		/* Finally do x = v * d */
		for (j = 0; j < n; j++) for (k = 0; k < n; k++) x[j] += v[k*n+j] * d[k];

		gmt_M_free (GMT, b);
		gmt_M_free (GMT, z);
		gmt_M_free (GMT, v);
		gmt_M_free (GMT, lambda);
	}
	else {	/* Decomposition worked, now solve system */
		gmt_chol_solv (GMT, N, x, r, n, n);
	}

	if (info->fit_mode == GMTMATH_COEFFICIENTS) {	/* Return coefficients only as a single vector */
		uint64_t dim[GMT_DIM_SIZE] = {1, 1, 0, 1};
		dim[GMT_ROW] = n;
		if ((D = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL)
			return (GMT->parent->error);
		for (k = 0; k < n; k++) D->table[0]->segment[0]->data[GMT_X][k] = x[k];
		D->table[0]->segment[0]->n_rows = n;
		GMT_Set_Comment (GMT->parent, GMT_IS_DATASET, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, D);
		if (GMT->common.h.add_colnames) {
			char header[GMT_LEN16] = {""};
			sprintf (header, "#coefficients");
			if (GMT_Set_Comment (GMT->parent, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, D)) return (GMT->parent->error);
		}
		if (GMT_Write_Data (GMT->parent, GMT_IS_DATASET, (file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_NONE, 0, NULL, file, D) != GMT_NOERROR)
			return (GMT->parent->error);
	}
	else {	/* Return t, y, p(t), r(t), where p(t) is the predicted solution and r(t) is the residuals */
		double value;
		struct GMT_DATASET_HIDDEN *DH = gmt_get_DD_hidden (S->D);
		k = (unsigned int)DH->dim[GMT_COL];
		DH->dim[GMT_COL] = (info->w_mode) ? 5 : 4;	/* State we want a different set of columns on output */
		D = GMT_Duplicate_Data (GMT->parent, GMT_IS_DATASET, GMT_DUPLICATE_ALLOC, S->D);	/* Same table length as S->D, but with up to n_cols columns (lon, lat, dist, g1, g2, ...) */
		DH->dim[GMT_COL] = k;	/* Reset the original columns */
		if (D->table[0]->n_segments > 1) gmt_set_segmentheader (GMT, GMT_OUT, true);	/* More than one segment triggers -mo */
		load_column (D, 0, info->T, COL_T);	/* Place the time-column in first output column */
		for (seg = k = 0; seg < info->T->n_segments; seg++) {
			for (row = 0; row < T->segment[seg]->n_rows; row++, k++) {
				D->table[0]->segment[seg]->data[1][row] = T->segment[seg]->data[rhs][row];
				value = 0.0;
				for (j2 = j = 0; j2 < n; j2++) {	/* j2 is table column, j is entry in x vector */
					if (skip[j2]) continue;		/* Not included in the fit */
					value += T->segment[seg]->data[j2][row] * x[j];	/* Sum up the solution */
					j++;
				}
				D->table[0]->segment[seg]->data[2][row] = value;
				D->table[0]->segment[seg]->data[3][row] = T->segment[seg]->data[rhs][row] - value;
				if (info->w_mode) D->table[0]->segment[seg]->data[4][row] = T->segment[seg]->data[w_col][row];
			}
		}
		if (GMT->common.h.add_colnames) {
			char header[GMT_LEN128] = {""};
			sprintf (header, "#t[0]\tobserved(t)[1]\tpredict(t)[2]\tresidual(t)[3]");
			if (info->w_mode == GMTMATH_WEIGHTS) strcat (header, "\tweight(t)[4]");
			else if (info->w_mode == GMTMATH_SIGMAS) strcat (header, "\tsigma(t)[4]");
			if (GMT_Set_Comment (GMT->parent, GMT_IS_DATASET, GMT_COMMENT_IS_COLNAMES, header, D)) return (GMT->parent->error);
		}
		if (GMT_Write_Data (GMT->parent, GMT_IS_DATASET, (file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_NONE, 0, NULL, file, D) != GMT_NOERROR) {
			return (GMT->parent->error);
		}
	}

	gmt_M_free (GMT, x);
	gmt_M_free (GMT, d);
	gmt_M_free (GMT, N);
	gmt_M_free (GMT, r);
	return (GMT_NOERROR);
}

GMT_LOCAL int solve_LSQFIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S, uint64_t n_col, bool skip[], double eigen, char *file, struct GMT_OPTION *options, struct GMT_DATASET *A) {
	return (solve_LS_system (GMT, info, S, n_col, skip, file, false, eigen, options, A));
}

GMT_LOCAL int solve_SVDFIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S, uint64_t n_col, bool skip[], double eigen, char *file, struct GMT_OPTION *options, struct GMT_DATASET *A) {
	return (solve_LS_system (GMT, info, S, n_col, skip, file, true, eigen, options, A));
}

GMT_LOCAL void load_const_column (struct GMT_DATASET *to, uint64_t to_col, double factor) {
	/* Sets all rows in a column to a constant factor */
	uint64_t row, seg;
	for (seg = 0; seg < to->n_segments; seg++) {
		for (row = 0; row < to->table[0]->segment[seg]->n_rows; row++) to->table[0]->segment[seg]->data[to_col][row] = factor;
	}
}

GMT_LOCAL bool same_size (struct GMT_DATASET *A, struct GMT_DATASET *B) {
	/* Are the two dataset the same size */
	uint64_t seg;
	if (!(A->table[0]->n_segments == B->table[0]->n_segments && A->table[0]->n_columns == B->table[0]->n_columns)) return (false);
	for (seg = 0; seg < A->table[0]->n_segments; seg++) if (A->table[0]->segment[seg]->n_rows != B->table[0]->segment[seg]->n_rows) return (false);
	return (true);
}

GMT_LOCAL bool same_domain (struct GMT_DATASET *A, uint64_t t_col, struct GMT_DATATABLE *B) {
	/* Are the two dataset the same domain */
	uint64_t seg;
	for (seg = 0; seg < A->table[0]->n_segments; seg++) {
		if (!(doubleAlmostEqualZero (A->table[0]->min[t_col], B->min[COL_T])
					&& doubleAlmostEqualZero (A->table[0]->max[t_col], B->max[COL_T])))
			return (false);
	}
	return (true);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [-A<ftable>[+e][+r][+s|w]] [-C<cols>] [-E<eigen>] [-I] [-L] [-N<n_col>[/<t_col>]] [-Q] [-S[f|l]]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-T[<min>/<max>/<inc>[+b|l|n]] | -T<file|list>] [%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s] A B op C op ... = [outfile]\n\n",
		GMT_V_OPT, GMT_b_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_q_OPT, GMT_s_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE,
		"\tA, B, etc. are table files, constants, or symbols (see below).\n"
		"\tTo read stdin give filename as STDIN (which can appear more than once).\n"
		"\tThe stack can hold up to %d entries (given enough memory).\n", GMTMATH_STACK_SIZE);
	GMT_Message (API, GMT_TIME_NONE,
		"\tTrigonometric operators expect radians unless noted otherwise.\n"
		"\tThe operators and number of input and output arguments:\n\n"
		"\tName       #args   Returns\n"
		"\t--------------------------\n");
	GMT_Message (API, GMT_TIME_NONE,
		"	ABS        1  1    abs (A)\n"
		"	ACOS       1  1    acos (A)\n"
		"	ACOSH      1  1    acosh (A)\n"
		"	ACOT       1  1    acot (A)\n"
		"	ACOTH      1  1    acoth (A)\n"
		"	ACSC       1  1    acsc (A)\n"
		"	ACSCH      1  1    acsch (A)\n"
		"	ADD        2  1    A + B\n"
		"	AND        2  1    B if A == NaN, else A\n"
		"	ASEC       1  1    asec (A)\n"
		"	ASECH      1  1    asech (A)\n"
		"	ASIN       1  1    asin (A)\n"
		"	ASINH      1  1    asinh (A)\n"
		"	ATAN       1  1    atan (A)\n"
		"	ATAN2      2  1    atan2 (A, B)\n"
		"	ATANH      1  1    atanh (A)\n"
		"	BCDF       3  1    Binomial cumulative distribution function for p = A, n = B and x = C\n"
		"	BEI        1  1    bei (A)\n"
		"	BER        1  1    ber (A)\n"
		"	BPDF       3  1    Binomial probability density function for p = A, n = B and x = C\n"
		"	BITAND     2  1    A & B (bitwise AND operator)\n"
		"	BITLEFT    2  1    A << B (bitwise left-shift operator)\n"
		"	BITNOT     1  1    ~A (bitwise NOT operator, i.e., return two's complement)\n"
		"	BITOR      2  1    A | B (bitwise OR operator)\n"
		"	BITRIGHT   2  1    A >> B (bitwise right-shift operator)\n"
		"	BITTEST    2  1    1 if bit B of A is set, else 0 (bitwise TEST operator)\n"
		"	BITXOR     2  1    A ^ B (bitwise XOR operator)\n"
		"	CEIL       1  1    ceil (A) (smallest integer >= A)\n"
		"	CHI2CRIT   2  1    Chi-squared distribution critical value for alpha = A and nu = B\n"
		"	CHI2CDF    2  1    Chi-squared cumulative distribution function for chi2 = A and nu = B\n"
		"	CHI2PDF    2  1    Chi-squared probability density function for chi = A and nu = B\n"
		"	COL        1  1    Places column A on the stack\n"
		"	COMB       2  1    Combinations n_C_r, with n = A and r = B\n"
		"	CORRCOEFF  2  1    Correlation coefficient r(A, B)\n"
		"	COS        1  1    cos (A) (A in radians)\n"
		"	COSD       1  1    cos (A) (A in degrees)\n"
		"	COSH       1  1    cosh (A)\n"
		"	COT        1  1    cot (A) (A in radians)\n"
		"	COTD       1  1    cot (A) (A in degrees)\n"
		"	COTH       1  1    coth (A)\n"
		"	CSC        1  1    csc (A) (A in radians)\n"
		"	CSCD       1  1    csc (A) (A in degrees)\n"
		"	CSCH       1  1    csch (A)\n"
		"	PCDF       2  1    Poisson cumulative distribution function for x = A and lambda = B\n"
		"	DDT        1  1    d(A)/dt Central 1st derivative\n"
		"	D2DT2      1  1    d^2(A)/dt^2 2nd derivative\n"
		"	D2R        1  1    Converts Degrees to Radians\n"
		"	DENAN      2  1    Replace NaNs in A with values from B\n"
		"	DILOG      1  1    dilog (A)\n"
		"	DIFF       1  1    Difference (forward) between adjacent elements of A (A[1]-A[0], A[2]-A[1], ..., NaN)\n"
		"	DIV        2  1    A / B\n"
		"	DUP        1  2    Places duplicate of A on the stack\n"
		"	ECDF       2  1    Exponential cumulative distribution function for x = A and lambda = B\n"
		"	ECRIT      2  1    Exponential distribution critical value for alpha = A and lambda = B\n"
		"	EPDF       2  1    Exponential probability density function for x = A and lambda = B\n"
		"	ERF        1  1    Error function erf (A)\n"
		"	ERFC       1  1    Complementary Error function erfc (A)\n"
		"	ERFINV     1  1    Inverse error function of A\n"
		"	EQ         2  1    1 if A == B, else 0\n"
		"	EXCH       2  2    Exchanges A and B on the stack\n"
		"	EXP        1  1    exp (A)\n"
		"	FACT       1  1    A! (A factorial)\n"
		"	FCRIT      3  1    F distribution critical value for alpha = A, nu1 = B, and nu2 = C\n"
		"	FCDF       3  1    F cumulative distribution function for F = A, nu1 = B, and nu2 = C\n"
		"	FLIPUD     1  1    Reverse order of each column\n"
		"	FLOOR      1  1    floor (A) (greatest integer <= A)\n"
		"	FMOD       2  1    A % B (remainder after truncated division)\n"
		"	FPDF       3  1    F probability density distribution for F = A, nu1 = B and nu2 = C\n"
		"	GE         2  1    1 if A >= B, else 0\n"
		"	GT         2  1    1 if A > B, else 0\n"
		"	HSV2LAB    3  3    Convert hsv to lab, with h = A, s = B and v = C\n"
		"	HSV2RGB    3  3    Convert hsv to rgb, with h = A, s = B and v = C\n"
		"	HSV2XYZ    3  3    Convert hsv to xyz, with h = A, s = B and v = C\n"
		"	HYPOT      2  1    hypot (A, B) = sqrt (A*A + B*B)\n"
		"	I0         1  1    Modified Bessel function of A (1st kind, order 0)\n"
		"	I1         1  1    Modified Bessel function of A (1st kind, order 1)\n"
		"	IFELSE     3  1    B if A != 0, else C\n"
		"	IN         2  1    Modified Bessel function of A (1st kind, order B)\n"
		"	INRANGE    3  1    1 if B <= A <= C, else 0\n"
		"	INT        1  1    Numerically integrate A\n"
		"	INV        1  1    1 / A\n"
		"	ISFINITE   1  1    1 if A is finite, else 0\n"
		"	ISNAN      1  1    1 if A == NaN, else 0\n"
		"	J0         1  1    Bessel function of A (1st kind, order 0)\n"
		"	J1         1  1    Bessel function of A (1st kind, order 1)\n"
		"	JN         2  1    Bessel function of A (1st kind, order B)\n"
		"	K0         1  1    Modified Kelvin function of A (2nd kind, order 0)\n"
		"	K1         1  1    Modified Bessel function of A (2nd kind, order 1)\n"
		"	KN         2  1    Modified Bessel function of A (2nd kind, order B)\n"
		"	KEI        1  1    kei (A)\n"
		"	KER        1  1    ker (A)\n"
		"	KURT       1  1    Kurtosis of A\n"
		"	LAB2HSV    3  3    Convert lab to hsv, with l = A, a = B and b = C\n"
		"	LAB2RGB    3  3    Convert lab to rgb, with l = A, a = B and b = C\n"
		"	LAB2XYZ    3  3    Convert lab to xyz, with l = A, a = B and b = C\n"
		"	LCDF       1  1    Laplace cumulative distribution function for z = A\n"
		"	LCRIT      1  1    Laplace distribution critical value for alpha = A\n"
		"	LE         2  1    1 if A <= B, else 0\n"
		"	LMSSCL     1  1    LMS scale estimate (LMS STD) of A\n"
		"	LMSSCLW    1  1    Weighted LMS scale estimate (LMS STD) of A for weights in B\n"
		"	LOG        1  1    log (A) (natural log)\n"
		"	LOG10      1  1    log10 (A) (base 10)\n"
		"	LOG1P      1  1    log (1+A) (accurate for small A)\n"
		"	LOG2       1  1    log2 (A) (base 2)\n"
		"	LOWER      1  1    The lowest (minimum) value of A\n"
		"	LPDF       1  1    Laplace probability density function for z = A\n"
		"	LRAND      2  1    Laplace random noise with mean A and std. deviation B\n"
		"	LSQFIT     1  0    Current table is [A | b]; return LS solution to A * x = b via Cholesky decomposition\n"
		"	LT         2  1    1 if A < B, else 0\n"
		"	MAD        1  1    Median Absolute Deviation (L1 STD) of A\n"
		"	MADW       2  1    Weighted Median Absolute Deviation (L1 STD) of A for weights in B\n"
		"	MAX        2  1    Maximum of A and B\n"
		"	MEAN       1  1    Mean value of A\n"
		"	MEANW      2  1    Weighted mean value of A for weights in B\n"
		"	MEDIAN     1  1    Median value of A\n"
		"	MEDIANW    2  1    Weighted median value of A for weights in B\n"
		"	MIN        2  1    Minimum of A and B\n"
		"	MOD        2  1    A mod B (remainder after floored division)\n"
		"	MODE       1  1    Mode value (Least Median of Squares) of A\n"
		"	MODEW      2  1    Weighted mode value of A for weights in B\n"
		"	MUL        2  1    A * B\n"
		"	NAN        2  1    NaN if A == B, else A\n"
		"	NEG        1  1    -A\n"
		"	NEQ        2  1    1 if A != B, else 0\n"
		"	NORM       1  1    Normalize (A) so max(A)-min(A) = 1\n"
		"	NOT        1  1    NaN if A == NaN, 1 if A == 0, else 0\n"
		"	NRAND      2  1    Normal, random values with mean A and std. deviation B\n"
		"	OR         2  1    NaN if B == NaN, else A\n"
		"	PERM       2  1    Permutations n_P_r, with n = A and r = B\n"
		"	PLM        3  1    Associated Legendre polynomial P(A) degree B order C\n"
		"	PLMg       3  1    Normalized associated Legendre polynomial P(A) degree B order C (geophysical convention)\n"
		"	POP        1  0    Delete top element from the stack\n"
		"	POW        2  1    A ^ B\n"
		"	PPDF       2  1    Poisson probability density function for x = A and lambda = B\n"
		"	PQUANT     2  1    The B'th Quantile (0-100%) of A\n"
		"	PQUANTW    3  1    The C'th Quantile (0-100%) of A for weights in B\n"
		"	PSI        1  1    Psi (or Digamma) of A\n"
		"	PV         3  1    Legendre function Pv(A) of degree v = real(B) + imag(C)\n"
		"	QV         3  1    Legendre function Qv(A) of degree v = real(B) + imag(C)\n"
		"	R2         2  1    R2 = A^2 + B^2\n"
		"	R2D        1  1    Convert Radians to Degrees\n"
		"	RAND       2  1    Uniform random values between A and B\n"
		"	RCDF       1  1    Rayleigh cumulative distribution function for z = A\n"
		"	RCRIT      1  1    Rayleigh distribution critical value for alpha = A\n"
		"	RGB2HSV    3  3    Convert rgb to hsv, with r = A, g = B and b = C\n"
		"	RGB2LAB    3  3    Convert rgb to lab, with r = A, g = B and b = C\n"
		"	RGB2XYZ    3  3    Convert rgb to xyz, with r = A, g = B and b = C\n"
		"	RPDF       1  1    Rayleigh probability density function for z = A\n"
		"	RINT       1  1    rint (A) (round to integral value nearest to A)\n"
		"	RMS        1  1    Root-mean-square of A\n"
		"	RMSW       2  1    Weighted Root-mean-square of A for weights in B\n"
		"	ROLL       2  0    Cyclicly shifts the top A stack items by an amount B\n"
		"	ROTT       2  1    Rotate A by the (constant) shift B in the t-direction\n"
		"	SEC        1  1    sec (A) (A in radians)\n"
		"	SECD       1  1    sec (A) (A in degrees)\n"
		"	SECH       1  1    sech (A)\n"
		"	SIGN       1  1    sign (+1 or -1) of A\n"
		"	SIN        1  1    sin (A) (A in radians)\n"
		"	SINC       1  1    sinc (A) (sin (pi*A)/(pi*A))\n"
		"	SIND       1  1    sin (A) (A in degrees)\n"
		"	SINH       1  1    sinh (A)\n"
		"	SKEW       1  1    Skewness of A\n"
		"	SORT       3  1    Sort all columns in stack based on column A in direction of B (-1 descending |+1 ascending)\n"
		"	SQR        1  1    A^2\n"
		"	SQRT       1  1    sqrt (A)\n"
		"	STD        1  1    Standard deviation of A\n"
		"	STDW       2  1    Weighted standard deviation of A for weights in B\n"
		"	STEP       1  1    Heaviside step function H(A)\n"
		"	STEPT      1  1    Heaviside step function H(t-A)\n"
		"	SUB        2  1    A - B\n"
		"	SUM        1  1    Cumulative sum of A\n"
		"	SVDFIT     1  0    Current table is [A | b]; return LS solution to A * x = B via SVD decomposition (see -E)\n"
		"	TAN        1  1    tan (A) (A in radians)\n"
		"	TAND       1  1    tan (A) (A in degrees)\n"
		"	TANH       1  1    tanh (A)\n"
		"	TAPER      1  1    Unit weights cosine-tapered to zero within A of end margins\n"
		"	TCDF       2  1    Student's t cumulative distribution function for t = A and nu = B\n"
		"	TN         2  1    Chebyshev polynomial Tn(-1<A<+1) of degree B\n"
		"	TPDF       2  1    Student's t probability density function for t = A and nu = B\n"
		"	TCRIT      2  1    Student's t distribution critical value for alpha = A and nu = B\n"
		"	UPPER      1  1    The highest (maximum) value of A\n"
		"	VAR        1  1    Variance of A\n"
		"	VARW       2  1    Weighted variance of A for weights in B\n"
		"	WCDF       3  1    Weibull cumulative distribution function for x = A, scale = B, and shape = C\n"
		"	WCRIT      3  1    Weibull distribution critical value for alpha = A, scale = B, and shape = C\n"
		"	WPDF       3  1    Weibull probability density function for x = A, scale = B and shape = C\n"
		"	XOR        2  1    B if A == NaN, else A\n"
		"	XYZ2HSV    3  3    Convert xyz to hsv, with x = A, y = B and z = C\n"
		"	XYZ2LAB    3  3    Convert xyz to lab, with x = A, y = B and z = C\n"
		"	XYZ2RGB    3  3    Convert xyz to rgb, with x = A, y = B and z = C\n"
		"	Y0         1  1    Bessel function of A (2nd kind, order 0)\n"
		"	Y1         1  1    Bessel function of A (2nd kind, order 1)\n"
		"	YN         2  1    Bessel function of A (2nd kind, order B)\n"
		"	ZCRIT      1  1    Normal distribution critical value for alpha = A\n"
		"	ZCDF       1  1    Normal cumulative distribution function for z = A\n"
		"	ZPDF       1  1    Normal probability density function for z = A\n"
		"	ROOTS      2  1    Treats col A as f(t) = 0 and returns its roots\n");
	GMT_Message (API, GMT_TIME_NONE,
		"\n\tThe special symbols are:\n\n"
		"\tPI                  = 3.1415926...\n"
		"\tE                   = 2.7182818...\n"
		"\tEULER               = 0.5772156...\n"
		"\tPHI (golden ratio)  = 1.6180339...\n"
		"\tF_EPS (single eps)  = 1.192092896e-07\n"
		"\tD_EPS (double eps)  = 2.2204460492503131e-16\n"
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
		"\t   No additional data files are read.  Output will be a single column with coefficients.\n"
		"\t   Append +r to only place f(t) in b and leave A initialized to zeros.\n"
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
		"\t-Q Quick scalar calculator (Shorthand for -Ca -N1/0 -T0/0/1).\n"
		"\t   Allows constants to have plot units (i.e., %s); if so the answer is converted using PROJ_LENGTH_UNIT.\n"
		"\t-S Only write first row upon completion of calculations [write all rows].\n"
		"\t   Optionally, append l for last row or f for first row [Default].\n"
		"\t-T Set domain from <min> to <max> in steps of <inc>. Append +n to <inc> if number of points was given instead.\n"
		"\t   Append +b for log2 spacing in <inc> and +l for log10 spacing via <inc> = 1,2,3.\n"
		"\t   Alternatively, give a file with output times in the first column, or a comma-separated list.\n"
		"\t   If no domain is given we assume no time, i.e., only data columns are present.\n"
		"\t   This choice also implies -Ca.\n", GMT_DIM_UNITS_DISPLAY);
	GMT_Option (API, "V,bi,bo,d,e,f,g,h,i,o,q,s,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMTMATH_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmtmath and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, n_files = 0;
	bool missing_equal = true;
	char *c = NULL, *t_arg = NULL;
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
				if (opt->arg[0] == '-') {	/* Old-style leading hyphen to the filename has been replaced by modifier +r */
					if (gmt_M_compat_check (GMT, 5)) {
						GMT_Report (API, GMT_MSG_COMPAT, "The leading hyphen in -A is deprecated.  Append modifier +r instead.\n");
						Ctrl->A.null = true;
						k = 1;
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -A: Unable to decode arguments\n");
						n_errors++;
					}
				}
				if ((c = strchr(opt->arg, '+')) != NULL && strchr("ersw", c[1]) != NULL) {	/* Got a valid modifier */
					unsigned int pos = 0;
					char p[GMT_LEN256] = {""};
					c[0] = '\0';	/* Temporarily chop off modifiers */
					Ctrl->A.file = strdup (&opt->arg[k]);
					c[0] = '+';	/* Restore the modifier */
					while (gmt_strtok (c, "+", &pos, p)) {
						switch (p[0]) {
							case 'e': Ctrl->A.e_mode = GMTMATH_EVALUATE; break;	/* Evaluate solution */
							case 'r': Ctrl->A.null = true; break;	/* Only set rhs of equation */
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
				if (gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Option -F is deprecated; use -o instead\n");
					gmt_parse_o_option (GMT, opt->arg);
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
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
						GMT_Report (API, GMT_MSG_ERROR, "Option -S: Syntax is -S[f|l]\n");
						n_errors++;
					break;
				}
				break;
			case 'T':	/* Either get a file with time coordinate or a min/max/dt setting */
				Ctrl->T.active = true;
				t_arg = opt->arg;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
		}
	}

	if (Ctrl->T.active) {	/* Do this one here since we need Ctrl->N.col to be set first, if selected */
		if (t_arg && t_arg[0]) {	/* Gave an argument, so we can parse and create the array */
			n_errors += gmt_parse_array (GMT, 'T', t_arg, &(Ctrl->T.T), GMT_ARRAY_TIME | GMT_ARRAY_SCALAR |
			                             GMT_ARRAY_RANGE, (unsigned int)Ctrl->N.tcol);
			n_errors += gmt_create_array (GMT, 'T', &(Ctrl->T.T), NULL, NULL);
		}
		else
			Ctrl->T.notime = true;
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && gmt_access (GMT, Ctrl->A.file, R_OK),
	                                   "Option -A: Cannot read file %s!\n", Ctrl->A.file);
	n_errors += gmt_M_check_condition (GMT, missing_equal, "Usage is <operations> = [outfile]\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && (Ctrl->T.active || Ctrl->N.active || Ctrl->C.active),
	                                   "Cannot use -T, -N, or -C when -Q has been set\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.active && (Ctrl->N.ncol == 0 || Ctrl->N.tcol >= Ctrl->N.ncol),
	                                   "Option -N must have positive n_cols and 0 <= t_col < n_col\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL unsigned int gmt_assign_ptrs (struct GMT_CTRL *GMT, unsigned int last, struct GMTMATH_STACK *S[], struct GMT_DATATABLE **T, struct GMT_DATATABLE **T_prev) {	/* Centralize the assignment of previous stack ID and the current and previous stack tables */
	unsigned int prev;
	if (last == 0) {	/* User error in requesting more items that presently on the stack */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Not enough items on the stack\n");
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
/* Note: The OPERATOR: **** lines were used to extract syntax for documentation in the old days.
 * the first number is input args and the second is the output args. */

GMT_LOCAL int table_ABS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ABS 1 1 abs (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused (GMT);

	if (S[last]->constant) a = fabs (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : fabs (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ACOS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ACOS 1 1 acos (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ACOS!\n");
	if (S[last]->constant) a = d_acos (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : d_acos (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ACOSH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ACOSH 1 1 acosh (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) < 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand < 1 for ACOSH!\n");
	if (S[last]->constant) a = acosh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : acosh (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ACOT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ACOT 1 1 acot (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ACOT!\n");
	if (S[last]->constant) a = atan (1.0 / S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : atan (1.0 / T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ACOTH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ACOTH 1 1 acoth (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) <= 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| <= 1 for ACOTH!\n");
	if (S[last]->constant) a = atanh (1.0/S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : atanh (1.0/T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ACSC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ACSC 1 1 acsc (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ACSC!\n");
	if (S[last]->constant) a = d_asin (1.0 / S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : d_asin (1.0 / T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ACSCH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ACSCH 1 1 acsch (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = asinh (1.0/S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : asinh (1.0/T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ADD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ADD 2 1 A + B.  */
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */
	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "ADD: Operand one == 0!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "ADD: Operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
			b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
			T_prev->segment[s]->data[col][row] = a + b;
		}
	}
	return 0;
}

GMT_LOCAL int table_AND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: AND 2 1 B if A == NaN, else A.  */
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	gmt_M_unused(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
			b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
			T_prev->segment[s]->data[col][row] = (gmt_M_is_dnan (a)) ? b : a;
		}
	}
	return 0;
}

GMT_LOCAL int table_ASEC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ASEC 1 1 asec (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ASEC!\n");
	if (S[last]->constant) a = d_acos (1.0 / S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : d_acos (1.0 / T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ASECH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ASECH 1 1 asech (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand > 1 for ASECH!\n");
	if (S[last]->constant) a = acosh (1.0/S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : acosh (1.0/T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ASIN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ASIN 1 1 asin (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ASIN!\n");
	if (S[last]->constant) a = d_asin (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : d_asin (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ASINH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ASINH 1 1 asinh (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = asinh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : asinh (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ATAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ATAN 1 1 atan (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = atan (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : atan (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ATAN2 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ATAN2 2 1 atan2 (A, B).  */
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for ATAN2!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for ATAN2!\n");
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
			b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
			T_prev->segment[s]->data[col][row] = d_atan2 (a, b);
		}
	}
	return 0;
}

GMT_LOCAL int table_ATANH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: ATANH 1 1 atanh (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && fabs (S[last]->factor) >= 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| >= 1 for ATANH!\n");
	if (S[last]->constant) a = atanh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : atanh (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_BCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: BCDF 3 1 Binomial cumulative distribution function for p = A, n = B and x = C.  */
	unsigned int prev1 = last - 1, prev2 = last - 2;
	uint64_t s, row, x, n;
	double p;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev2]->constant && (S[prev2]->factor < 0.0 || S[prev2]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument p to BCDF must be a 0 <= p <= 1!\n");
		return -1;
	}
	if (S[prev1]->constant && S[prev1]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument n to BCDF must be a positive integer (n >= 0)!\n");
		return -1;
	}
	if (S[last]->constant && S[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument x to BCDF must be a positive integer (x >= 0)!\n");
		return -1;
	}
	if (S[prev2]->constant && S[prev1]->constant && S[last]->constant) {	/* BCDF is given constant arguments */
		double value;
		p = S[prev2]->factor;
		n = lrint (S[prev1]->factor);	x = lrint (S[last]->factor);
		value = gmt_binom_cdf (GMT, x, n, p);
		for (s = 0; s < info->T->n_segments; s++)
			for (row = 0; row < info->T->segment[s]->n_rows; row++)
				T_prev2->segment[s]->data[col][row] = value;
		return 0;
	}
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			p = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->data[col][row];
			n = lrint ((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->data[col][row]);
			x = lrint ((S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row]);
			T_prev2->segment[s]->data[col][row] = gmt_binom_cdf (GMT, x, n, p);
		}
	}
	return 0;
}

GMT_LOCAL int table_BEI (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: BEI 1 1 bei (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_bei (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_bei (GMT, fabs (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_BER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: BER 1 1 ber (A).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_ber (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++)
			T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_ber (GMT, fabs (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_BPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BPDF 3 1 Binomial probability density function for p = A, n = B and x = C.  */
{
	unsigned int prev1 = last - 1, prev2 = last - 2;
	uint64_t s, row, x, n;
	double p;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev2]->constant && (S[prev2]->factor < 0.0 || S[prev2]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument p to BPDF must be a 0 <= p <= 1!\n");
		return -1;
	}
	if (S[prev1]->constant && S[prev1]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument n to BPDF must be a positive integer (n >= 0)!\n");
		return -1;
	}
	if (S[last]->constant && S[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument x to BPDF must be a positive integer (x >= 0)!\n");
		return -1;
	}
	if (S[prev2]->constant && S[prev1]->constant && S[last]->constant) {	/* BPDF is given constant arguments */
		double value;
		p = S[prev2]->factor;
		n = lrint (S[prev1]->factor);	x = lrint (S[last]->factor);
		value = gmt_binom_pdf (GMT, x, n, p);
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev2->segment[s]->data[col][row] = value;
		return 0;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		p = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->data[col][row];
		n = lrint ((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->data[col][row]);
		x = lrint ((S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row]);
		T_prev2->segment[s]->data[col][row] = gmt_binom_pdf (GMT, x, n, p);

	}
	return 0;
}

GMT_LOCAL int table_BITAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		if (!S[prev]->constant) ad = T_prev->segment[s]->data[col][row];
		if (!S[last]->constant) bd = T->segment[s]->data[col][row];
		if (gmt_M_is_dnan (ad) || gmt_M_is_dnan (bd))	/* Any NaN in bitwise operations results in NaN output */
			T_prev->segment[s]->data[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;	b = (uint64_t)bd;
			result = a & b;
			result_trunc = result & DOUBLE_BIT_MASK;
			if (result != result_trunc) n_warn++;
			T_prev->segment[s]->data[col][row] = (double)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITAND resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

GMT_LOCAL int table_BITLEFT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		if (!S[prev]->constant) ad = T_prev->segment[s]->data[col][row];
		if (!S[last]->constant) bd = T->segment[s]->data[col][row];
		if (gmt_M_is_dnan (ad) || gmt_M_is_dnan (bd))	/* Any NaN in bitwise operations results in NaN output */
			T_prev->segment[s]->data[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;	b_signed = (int64_t)bd;
			if (b_signed < 0) {	/* Bad bitshift */
				if (first) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Bit shift must be >= 0; other values yield NaN\n");
				T_prev->segment[s]->data[col][row] = GMT->session.d_NaN;
				first = false;
			}
			else {
				b = (uint64_t)b_signed;
				result = a << b;
				result_trunc = result & DOUBLE_BIT_MASK;
				if (result != result_trunc) n_warn++;
				T_prev->segment[s]->data[col][row] = (double)result_trunc;
			}
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITLEFT resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

GMT_LOCAL int table_BITNOT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: BITNOT 1 1 ~A (bitwise NOT operator, i.e., return two's complement).  */
{
	uint64_t s, row, a = 0, n_warn = 0, result, result_trunc;
	double ad = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) ad = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[last]->constant) ad = T->segment[s]->data[col][row];
		if (gmt_M_is_dnan (ad))	/* Any NaN in bitwise operations results in NaN output */
			T->segment[s]->data[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;
			result = ~a;
			result_trunc = result & DOUBLE_BIT_MASK;
			if (result != result_trunc) n_warn++;
			T->segment[s]->data[col][row] = (double)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITNOT resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

GMT_LOCAL int table_BITOR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		if (!S[prev]->constant) ad = T_prev->segment[s]->data[col][row];
		if (!S[last]->constant) bd = T->segment[s]->data[col][row];
		if (gmt_M_is_dnan (ad) || gmt_M_is_dnan (bd))	/* Any NaN in bitwise operations results in NaN output */
			T_prev->segment[s]->data[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;	b = (uint64_t)bd;
			result = a | b;
			result_trunc = result & DOUBLE_BIT_MASK;
			if (result != result_trunc) n_warn++;
			T_prev->segment[s]->data[col][row] = (double)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITOR resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

GMT_LOCAL int table_BITRIGHT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		if (!S[prev]->constant) ad = T_prev->segment[s]->data[col][row];
		if (!S[last]->constant) bd = T->segment[s]->data[col][row];
		if (gmt_M_is_dnan (ad) || gmt_M_is_dnan (bd))	/* Any NaN in bitwise operations results in NaN output */
			T_prev->segment[s]->data[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;	b_signed = (int64_t)bd;
			if (b_signed < 0) {	/* Bad bitshift */
				if (first) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Bit shift must be >= 0; other values yield NaN\n");
				T_prev->segment[s]->data[col][row] = GMT->session.d_NaN;
				first = false;
			}
			else {
				b = (uint64_t)b_signed;
				result = a >> b;
				result_trunc = result & DOUBLE_BIT_MASK;
				if (result != result_trunc) n_warn++;
				T_prev->segment[s]->data[col][row] = (double)result_trunc;
			}
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITRIGHT resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

GMT_LOCAL int table_BITTEST (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		if (!S[prev]->constant) ad = T_prev->segment[s]->data[col][row];
		if (!S[last]->constant) bd = T->segment[s]->data[col][row];
		if (gmt_M_is_dnan (ad) || gmt_M_is_dnan (bd))	/* Any NaN in bitwise operations results in NaN output */
			T_prev->segment[s]->data[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;	b_signed = (int64_t)bd;
			if (b_signed < 0) {	/* Bad bit */
				if (first) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Bit position range for BITTEST is 0-63 (since we are using do); other values yield NaN\n");
				T_prev->segment[s]->data[col][row] = GMT->session.d_NaN;
				first = false;
			}
			else {
				b = (uint64_t)b_signed;
				b = 1ULL << b;
				result = a & b;
				result_trunc = result & DOUBLE_BIT_MASK;
				if (result != result_trunc) n_warn++;
				T_prev->segment[s]->data[col][row] = (result_trunc) ? 1.0 : 0.0;
			}
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITTEST resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

GMT_LOCAL int table_BITXOR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		if (!S[prev]->constant) ad = T_prev->segment[s]->data[col][row];
		if (!S[last]->constant) bd = T->segment[s]->data[col][row];
		if (gmt_M_is_dnan (ad) || gmt_M_is_dnan (bd))	/* Any NaN in bitwise operations results in NaN output */
			T_prev->segment[s]->data[col][row] = GMT->session.d_NaN;
		else {
			a = (uint64_t)ad;	b = (uint64_t)bd;
			result = a ^ b;
			result_trunc = result & DOUBLE_BIT_MASK;
			if (result != result_trunc) n_warn++;
			T_prev->segment[s]->data[col][row] = (double)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITXOR resulted in %" PRIu64 " values truncated to fit in the 53 available bits\n");
	return 0;
}

GMT_LOCAL int table_CEIL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: CEIL 1 1 ceil (A) (smallest integer >= A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = ceil (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : ceil (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_CHI2CRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: CHI2CRIT 2 1 Chi-squared distribution critical value for alpha = A and nu = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for CHI2CRIT!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for CHI2CRIT!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = gmt_chi2crit (GMT, a, b);
	}
	return 0;
}

GMT_LOCAL int table_CHI2CDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: CHI2CDF 2 1 Chi-squared cumulative distribution function for chi2 = A and nu = B.  */
	uint64_t s, row;
	unsigned int prev;
	double a, b, q;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for CHI2CDF!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for CHI2CDF!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		gmt_chi2 (GMT, a, b, &q);
		T_prev->segment[s]->data[col][row] = 1.0 - q;
	}
	return 0;
}

GMT_LOCAL int table_CHI2PDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: CHI2PDF 2 1 Chi-squared probability density function for chi = A and nu = B.  */
	uint64_t s, row, nu;
	unsigned int prev;
	double c;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for CHI2PDF!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for CHI2PDF!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		nu = lrint ((S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row]);
		c  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = gmt_chi2_pdf (GMT, c, nu);
	}
	return 0;
}

GMT_LOCAL int table_COL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: COL 1 1 Places column A on the stack.  */
	uint64_t s, row;
	unsigned int k;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if (gmt_assign_ptrs (GMT, last, S, &T, &T_prev) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (!S[last]->constant || S[last]->factor < 0.0 || ((k = urint (S[last]->factor)) >= info->n_col)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument to COL must be a constant column number (0 <= k < n_col)!\n");
		return -1;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		T->segment[s]->data[col][row] = T_prev->segment[s]->data[k][row];
	}
	return 0;
}

GMT_LOCAL int table_COMB (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: COMB 2 1 Combinations n_C_r, with n = A and r = B.  */
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument n to COMB must be a positive integer (n >= 0)!\n");
		return -1;
	}
	if (S[last]->constant && S[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to COMB must be a positive integer (r >= 0)!\n");
		return -1;
	}
	if (S[prev]->constant && S[last]->constant) {	/* COMBO is given constant args */
		double value = gmt_combination (GMT, irint(S[prev]->factor), irint(S[last]->factor));
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = value;
		return 0;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = gmt_combination (GMT, irint(a), irint(b));
	}
	return 0;
}

GMT_LOCAL int table_CORRCOEFF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: CORRCOEFF 2 1 Correlation coefficient r(A, B).  */
	uint64_t s, row, i;
	unsigned int prev;
	double *a, *b, coeff;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[last]->constant) {	/* Correlation is undefined */
		for (s = 0; s < info->T->n_segments; s++)
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = GMT->session.d_NaN;
		return 0;
	}

	if (!info->local) {	/* Compute correlation across the entire table */
		a = gmt_M_memory (GMT, NULL, info->T->n_records, double);
		b = gmt_M_memory (GMT, NULL, info->T->n_records, double);
		for (s = i = 0; s < info->T->n_segments; s++)
			for (row = 0; row < info->T->segment[s]->n_rows; row++, i++) a[i] = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		for (s = i = 0; s < info->T->n_segments; s++)
			for (row = 0; row < info->T->segment[s]->n_rows; row++, i++) b[i] = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		coeff = gmt_corrcoeff (GMT, a, b, info->T->n_records, 0);
		for (s = 0; s < info->T->n_segments; s++)
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = coeff;
		gmt_M_free (GMT, a);		gmt_M_free (GMT, b);
		return 0;
	}
	/* Local, or per-segment calculations */
	for (s = 0; s < info->T->n_segments; s++) {
		if (S[prev]->constant) {		/* Must create the missing (constant) column */
			a = gmt_M_memory (GMT, NULL, info->T->segment[s]->n_rows, double);
			for (row = 0; row < info->T->segment[s]->n_rows; row++) a[row] = S[prev]->factor;
			b = T->segment[s]->data[col];
		}
		else if (S[last]->constant) {	/* Must create the missing (constant) column */
			a = T_prev->segment[s]->data[col];
			b = gmt_M_memory (GMT, NULL, info->T->segment[s]->n_rows, double);
			for (row = 0; row < info->T->segment[s]->n_rows; row++) b[row] = S[last]->factor;
		}
		else {
			a = T_prev->segment[s]->data[col];
			b = T->segment[s]->data[col];
		}
		coeff = gmt_corrcoeff (GMT, a, b, info->T->segment[s]->n_rows, 0);
		for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = coeff;
		if (S[prev]->constant) gmt_M_free (GMT, a);
		if (S[last]->constant) gmt_M_free (GMT, b);
	}
	return 0;
}


GMT_LOCAL int table_COS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COS 1 1 cos (A) (A in radians).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = cos (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		T->segment[s]->data[col][row] = (S[last]->constant) ? a : cos (T->segment[s]->data[col][row]);
	}
	return 0;
}

GMT_LOCAL int table_COSD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COSD 1 1 cos (A) (A in degrees).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = cosd (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : cosd (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_COSH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COSH 1 1 cosh (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = cosh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : cosh (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_COT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COT 1 1 cot (A) (A in radians).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = (1.0 / tan (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		T->segment[s]->data[col][row] = (S[last]->constant) ? a : (1.0 / tan (T->segment[s]->data[col][row]));
	}
	return 0;
}


GMT_LOCAL int table_COTD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COTD 1 1 cot (A) (A in degrees).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = (1.0 / tand (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		T->segment[s]->data[col][row] = (S[last]->constant) ? a : (1.0 / tand (T->segment[s]->data[col][row]));
	}
	return 0;
}

GMT_LOCAL int table_COTH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: COTH 1 1 coth (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = 1.0 / tanh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : 1.0 / tanh (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_CSC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: CSC 1 1 csc (A) (A in radians).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = (1.0 / sin (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		T->segment[s]->data[col][row] = (S[last]->constant) ? a : (1.0 / sin (T->segment[s]->data[col][row]));
	}
	return 0;
}

GMT_LOCAL int table_CSCD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: CSCD 1 1 csc (A) (A in degrees).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = (1.0 / sind (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		T->segment[s]->data[col][row] = (S[last]->constant) ? a : (1.0 / sind (T->segment[s]->data[col][row]));
	}
	return 0;
}

GMT_LOCAL int table_CSCH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: CSCH 1 1 csch (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = 1.0 / sinh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : 1.0 / sinh (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_PCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PCDF 2 1 Poisson cumulative distribution function for x = A and lambda = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for PCDF!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		gmt_poisson_cdf (GMT, a, b, &T_prev->segment[s]->data[col][row]);
	}
	return 0;
}

GMT_LOCAL int table_DDT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: DDT 1 1 d(A)/dt Central 1st derivative.  */
{
	uint64_t s, row;
	double c, left, next_left;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	/* Central 1st difference in t, using zero-curvature boundary conditions at the ends */

	if (info->irregular) GMT_Report (GMT->parent, GMT_MSG_WARNING, "DDT called on irregularly spaced data (not supported)!\n");
	if (S[last]->constant) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand to DDT is constant!\n");

	c = 0.5 / info->t_inc;
	for (s = 0; s < info->T->n_segments; s++) {
		row = 0;
		while (row < info->T->segment[s]->n_rows) {	/* Process each segment */
			next_left = 2.0 * T->segment[s]->data[col][row] - T->segment[s]->data[col][row+1];
			while (row < info->T->segment[s]->n_rows - 1) {
				left = next_left;
				next_left = T->segment[s]->data[col][row];
				T->segment[s]->data[col][row] = (S[last]->constant) ? 0.0 : c * (T->segment[s]->data[col][row+1] - left);
				row++;
			}
			T->segment[s]->data[col][row] = (S[last]->constant) ? 0.0 : 2.0 * c * (T->segment[s]->data[col][row] - next_left);
			row++;
		}
	}
	return 0;
}

GMT_LOCAL int table_D2DT2 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: D2DT2 1 1 d^2(A)/dt^2 2nd derivative.  */
{
	uint64_t s, row;
	double c, left, next_left;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	/* Central 2nd difference in t, using zero curvature boundary conditions */

	if (info->irregular) GMT_Report (GMT->parent, GMT_MSG_WARNING, "D2DT2 called on irregularly spaced data (not supported)!\n");
	if (S[last]->constant) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand to D2DT2 is constant!\n");

	c = 1.0 / (info->t_inc * info->t_inc);
	for (s = 0; s < info->T->n_segments; s++) {
		row = 0;
		while (row < info->T->segment[s]->n_rows) {	/* Process each segment */
			next_left = T->segment[s]->data[col][row];
			T->segment[s]->data[col][row] = (gmt_M_is_dnan (T->segment[s]->data[col][row]) || gmt_M_is_dnan (T->segment[s]->data[col][row+1])) ? GMT->session.d_NaN : 0.0;
			row++;
			while (row < info->T->segment[s]->n_rows - 1) {
				left = next_left;
				next_left = T->segment[s]->data[col][row];
				T->segment[s]->data[col][row] = (S[last]->constant) ? 0.0 : c * (T->segment[s]->data[col][row+1] - 2 * T->segment[s]->data[col][row] + left);
				row++;
			}
			T->segment[s]->data[col][row] = (gmt_M_is_dnan (T->segment[s]->data[col][row]) || gmt_M_is_dnan (T->segment[s]->data[col][row-1])) ? GMT->session.d_NaN : 0.0;
			row++;
		}
	}
	return 0;
}

GMT_LOCAL int table_D2R (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: D2R 1 1 Converts Degrees to Radians.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = S[last]->factor * D2R;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : T->segment[s]->data[col][row] * D2R;
	return 0;
}

GMT_LOCAL int table_DENAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: DENAN 2 1 Replace NaNs in A with values from B.  */
{	/* Just a more straightforward application of AND */
	return (table_AND (GMT, info, S, last, col));
}

GMT_LOCAL int table_DILOG (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: DILOG 1 1 dilog (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_dilog (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_dilog (GMT, T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_DIFF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: DIFF 1 1 Difference (forward) between adjacent elements of A (A[1]-A[0], A[2]-A[1], ..., NaN). */
{
	uint64_t s, row;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	/* Central 1st difference in t */
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows - 1; row++)
			T->segment[s]->data[col][row] = T->segment[s]->data[col][row+1] - T->segment[s]->data[col][row];

		T->segment[s]->data[col][info->T->segment[s]->n_rows - 1] = GMT->session.d_NaN;
	}
	return 0;
}

GMT_LOCAL int table_DIV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: DIV 2 1 A / B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant && S[last]->factor == 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Divide by zero gives NaNs\n");
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = a / b;
	}
	return 0;
}

GMT_LOCAL int table_DUP (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: DUP 1 2 Places duplicate of A on the stack.  */
{
	uint64_t s, row;
	unsigned int next = last + 1;
	struct GMT_DATATABLE *T = S[last]->D->table[0], *T_next = S[next]->D->table[0];
	gmt_M_unused(GMT);

	/* The next stack is an array no matter what S[last]->constant may be.
	   If S[last]->constant is true then gmtmath has just allocated space so we update that as well as next. */
	S[next]->constant = false;
	for (s = 0; s < info->T->n_segments; s++) {
		if (S[last]->constant) {	/* Constant, update both this and next */
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_next->segment[s]->data[col][row] = T->segment[s]->data[col][row] = S[last]->factor;
		}
		else
			gmt_M_memcpy (T_next->segment[s]->data[col], T->segment[s]->data[col], info->T->segment[s]->n_rows, double);
	}
	return 0;
}

GMT_LOCAL int table_ECDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ECDF 2 1 Exponential cumulative distribution function for x = A and lambda = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double x, lambda;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		lambda = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		x  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = 1.0 - exp (-lambda * x);
	}
	return 0;
}

GMT_LOCAL int table_ECRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ECRIT 2 1 Exponential distribution critical value for alpha = A and lambda = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double alpha, lambda;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		lambda = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		alpha  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = -log (1.0 - alpha)/lambda;
	}
	return 0;
}

GMT_LOCAL int table_EPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: EPDF 2 1 Exponential probability density function for x = A and lambda = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double x, lambda;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		lambda = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		x  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = lambda * exp (-lambda * x);
	}
	return 0;
}

GMT_LOCAL int table_ERF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ERF 1 1 Error function erf (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = erf (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : erf (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ERFC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ERFC 1 1 Complementary Error function erfc (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = erfc (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : erfc (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ERFINV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ERFINV 1 1 Inverse error function of A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_erfinv (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_erfinv (GMT, T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_EQ (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: EQ 2 1 1 if A == B, else 0.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	gmt_M_unused(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = (double)(a == b);
	}
	return 0;
}

GMT_LOCAL int table_EXCH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: EXCH 2 2 Exchanges A and B on the stack.  */
{
	unsigned int prev;
	struct GMT_DATASET *D = NULL;
	gmt_M_unused(GMT); gmt_M_unused(info); gmt_M_unused(col);
	assert (last > 0);
	prev = last - 1;
	D = S[last]->D;
	S[last]->D = S[prev]->D;	S[prev]->D = D;
	gmt_M_bool_swap (S[last]->constant, S[prev]->constant);
	gmt_M_double_swap (S[last]->factor, S[prev]->factor);
	return 0;
}

GMT_LOCAL int table_EXP (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: EXP 1 1 exp (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = exp (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : exp (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_FACT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FACT 1 1 A! (A factorial).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_factorial (GMT, irint(S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_factorial (GMT, irint(T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_FCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FCRIT 3 1 F distribution critical value for alpha = A, nu1 = B, and nu2 = C.  */
{
	uint64_t s, row;
	int nu1, nu2;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	double alpha;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev2]->constant && S[prev2]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for FCRIT!\n");
	if (S[prev1]->constant && S[prev1]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for FCRIT!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand three == 0 for FCRIT!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		alpha = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->data[col][row];
		nu1 = irint ((double)((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->data[col][row]));
		nu2 = irint ((double)((S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row]));
		T_prev2->segment[s]->data[col][row] = gmt_Fcrit (GMT, alpha, (double)nu1, (double)nu2);
	}
	return 0;
}

GMT_LOCAL int table_FCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FCDF 3 1 F cumulative distribution function for F = A, nu1 = B, and nu2 = C.  */
{
	uint64_t s, row, nu1, nu2;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	double F;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev1]->constant && S[prev1]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for FCDF!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand three == 0 for FCDF!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		F = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->data[col][row];
		nu1 = lrint ((double)((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->data[col][row]));
		nu2 = lrint ((double)((S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row]));
		T_prev2->segment[s]->data[col][row] = gmt_f_cdf (GMT, F, nu1, nu2);
	}
	return 0;
}

GMT_LOCAL int table_FLIPUD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FLIPUD 1 1 Reverse order of each column.  */
{
	uint64_t s, row, k;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);
	/* Reverse the order of points in a column */
	if (S[last]->constant) return 0;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0, k = info->T->segment[s]->n_rows-1; row < info->T->segment[s]->n_rows/2; row++, k--) gmt_M_double_swap (T->segment[s]->data[col][row], T->segment[s]->data[col][k]);
	return 0;
}

GMT_LOCAL int table_FLOOR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FLOOR 1 1 floor (A) (greatest integer <= A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = floor (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : floor (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_FMOD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FMOD 2 1 A % B (remainder after truncated division).  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "using FMOD 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = fmod (a, b);
	}
	return 0;
}

GMT_LOCAL int table_FPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: FPDF 3 1 F probability density distribution for F = A, nu1 = B and nu2 = C.  */
{
	unsigned int prev1 = last - 1, prev2 = last - 2;
	uint64_t s, row, nu1, nu2;
	double F;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev2]->constant && S[prev2]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument F to FPDF must be a >= 0!\n");
		return -1;
	}
	if (S[prev1]->constant && S[prev1]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument nu1 to FPDF must be a positive integer (nu1 > 0)!\n");
		return -1;
	}
	if (S[last]->constant && S[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument nu2 to FPDF must be a positive integer (nu2 > 0)!\n");
		return -1;
	}
	if (S[prev2]->constant && S[prev1]->constant && S[last]->constant) {	/* FPDF is given constant arguments */
		double value;
		F = S[prev2]->factor;
		nu1 = (uint64_t)S[prev1]->factor;	nu2 = (uint64_t)S[last]->factor;
		value = gmt_f_pdf (GMT, F, nu1, nu2);
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev2->segment[s]->data[col][row] = value;
		return 0;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		F = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->data[col][row];
		nu1 = lrint ((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->data[col][row]);
		nu2 = lrint ((S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row]);
		T_prev2->segment[s]->data[col][row] = gmt_f_pdf (GMT, F, nu1, nu2);
	}
	return 0;
}

GMT_LOCAL int table_GE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: GE 2 1 1 if A >= B, else 0.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	gmt_M_unused(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = (double)(a >= b);
	}
	return 0;
}

GMT_LOCAL int table_GT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: GT 2 1 1 if A > B, else 0.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	gmt_M_unused(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = (double)(a > b);
	}
	return 0;
}

GMT_LOCAL int table_HSV2LAB (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: HSV2LAB 3 3 Convert HSV to LAB, with h = A, s = B and v = C.  */
	uint64_t s, row;
	double hsv[4], rgb[4], lab[3];
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (info->scalar) {	/* Scalars have a stack of 3 constants */
		unsigned int prev1 = last - 1, prev2 = last - 2;
		struct GMT_DATATABLE *T_prev1 = S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
		if (S[prev2]->factor < 0.0 || S[prev2]->factor > 360.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument h to HSV2LAB must be a 0 <= h <= 360!\n");
			return -1;
		}
		if (S[prev1]->factor < 0.0 || S[prev1]->factor > 1.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument s to HSV2LAB must be a 0 <= s <= 1!\n");
			return -1;
		}
		if (S[last]->factor < 0.0 || S[last]->factor > 1.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument v to HSV2LAB must be a 0 <= v <= 1!\n");
			return -1;
		}
		rgb[3] = hsv[3] = 0.0;	/* No transparency involved */
		hsv[0] = S[prev2]->factor;
		hsv[1] = S[prev1]->factor;
		hsv[2] = S[last]->factor;
		gmt_hsv_to_rgb (rgb, hsv);	/* Must do this via RGB */
		gmt_rgb_to_lab (rgb, lab);
		T_prev2->segment[0]->data[col][0] = lab[0];
		T_prev1->segment[0]->data[col][0] = lab[1];
		T->segment[0]->data[col][0]       = lab[2];
		return 0;
	}
	/* Table input, only one item on stack but has 3 columns; we wait for col == 2 */
	if (col != 2) return 0;
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			hsv[0] = T->segment[s]->data[0][row];
			hsv[1] = T->segment[s]->data[1][row];
			hsv[2] = T->segment[s]->data[2][row];
			gmt_hsv_to_rgb (rgb, hsv);	/* Must do this via RGB */
			gmt_rgb_to_lab (rgb, lab);
			T->segment[s]->data[0][row] = lab[0];
			T->segment[s]->data[1][row] = lab[1];
			T->segment[s]->data[2][row] = lab[2];
		}
	}
	return 0;
}

GMT_LOCAL int table_HSV2RGB (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: HSV2RGB 3 3 Convert HSV to RGB, with h = A, s = B and v = C.  */
	uint64_t s, row;
	double rgb[4], hsv[4];
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (info->scalar) {	/* Scalars have a stack of 3 constants */
		unsigned int prev1 = last - 1, prev2 = last - 2;
		struct GMT_DATATABLE *T_prev1 = S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
		if (S[prev2]->factor < 0.0 || S[prev2]->factor > 360.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument h to HSV2RGB must be a 0 <= h <= 360!\n");
			return -1;
		}
		if (S[prev1]->factor < 0.0 || S[prev1]->factor > 1.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument s to HSV2RGB must be a 0 <= s <= 1!\n");
			return -1;
		}
		if (S[last]->factor < 0.0 || S[last]->factor > 1.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument v to HSV2RGB must be a 0 <= v <= 1!\n");
			return -1;
		}
		rgb[3] = hsv[3] = 0.0;	/* No transparency involved */
		hsv[0] = S[prev2]->factor;
		hsv[1] = S[prev1]->factor;
		hsv[2] = S[last]->factor;
		gmt_hsv_to_rgb (rgb, hsv);
		T_prev2->segment[0]->data[col][0] = gmt_M_s255 (rgb[0]);
		T_prev1->segment[0]->data[col][0] = gmt_M_s255 (rgb[1]);
		T->segment[0]->data[col][0]       = gmt_M_s255 (rgb[2]);
		return 0;
	}
	/* Table input, only one item on stack but has 3 columns; we wait for col == 2 */
	if (col != 2) return 0;
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			hsv[0] = T->segment[s]->data[0][row];
			hsv[1] = T->segment[s]->data[1][row];
			hsv[2] = T->segment[s]->data[2][row];
			gmt_hsv_to_rgb (rgb, hsv);
			T->segment[s]->data[0][row] = gmt_M_s255 (rgb[0]);
			T->segment[s]->data[1][row] = gmt_M_s255 (rgb[1]);
			T->segment[s]->data[2][row] = gmt_M_s255 (rgb[2]);
		}
	}
	return 0;
}

GMT_LOCAL int table_HSV2XYZ (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: HSV2XYZ 3 3 Convert HSV to XYZ, with h = A, s = B and v = C.  */
	uint64_t s, row;
	double hsv[4], xyz[4], rgb[4];
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (info->scalar) {	/* Scalars have a stack of 3 constants */
		unsigned int prev1 = last - 1, prev2 = last - 2;
		struct GMT_DATATABLE *T_prev1 = S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
		if (S[prev2]->factor < 0.0 || S[prev2]->factor > 360.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument h to HSV2XYZ must be a 0 <= h <= 360!\n");
			return -1;
		}
		if (S[prev1]->factor < 0.0 || S[prev1]->factor > 1.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument s to HSV2XYZ must be a 0 <= s <= 1!\n");
			return -1;
		}
		if (S[last]->factor < 0.0 || S[last]->factor > 1.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument b to HSV2XYZ must be a 0 <= b <= 1!\n");
			return -1;
		}
		rgb[3] = hsv[3] = 0.0;	/* No transparency involved */
		hsv[0] = S[prev2]->factor;
		hsv[1] = S[prev1]->factor;
		hsv[2] = S[last]->factor;
		gmt_hsv_to_rgb (rgb, hsv);	/* Must do this via RGB */
		gmt_rgb_to_xyz (rgb, xyz);
		T_prev2->segment[0]->data[col][0] = xyz[0];
		T_prev1->segment[0]->data[col][0] = xyz[1];
		T->segment[0]->data[col][0]       = xyz[2];
		return 0;
	}
	/* Table input, only one item on stack but has 3 columns; we wait for col == 2 */
	if (col != 2) return 0;
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			hsv[0] = T->segment[s]->data[0][row];
			hsv[1] = T->segment[s]->data[1][row];
			hsv[2] = T->segment[s]->data[2][row];
			gmt_hsv_to_rgb (rgb, hsv);	/* Must do this via RGB */
			gmt_rgb_to_xyz (rgb, xyz);
			T->segment[s]->data[0][row] = xyz[0];
			T->segment[s]->data[1][row] = xyz[1];
			T->segment[s]->data[2][row] = xyz[2];
		}
	}
	return 0;
}

GMT_LOCAL int table_HYPOT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: HYPOT 2 1 hypot (A, B) = sqrt (A*A + B*B).  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "HYPOT: Operand one == 0!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "HYPOT: Operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = hypot (a, b);
	}
	return 0;
}

GMT_LOCAL int table_I0 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: I0 1 1 Modified Bessel function of A (1st kind, order 0).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_i0 (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_i0 (GMT, T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_I1 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: I1 1 1 Modified Bessel function of A (1st kind, order 1).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_i1 (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_i1 (GMT, T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_IFELSE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: IFELSE 3 1 B if A != 0, else C.  */
{
	uint64_t s, row;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	double a = 0.0, b = 0.0, c = 0.0;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
	gmt_M_unused(GMT);

	/* last is C, prev1 is B, prev2 is A */

	/* Set to 1 where B <= A <= C, 0 elsewhere, except where
	 * A, B, or C = NaN, in which case we set answer to NaN */

	if (S[prev2]->constant) a = (double)S[prev2]->factor;
	if (S[prev1]->constant) b = (double)S[prev1]->factor;
	if (S[last]->constant)  c = (double)S[last]->factor;

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev2]->constant) a = T_prev2->segment[s]->data[col][row];
		if (!S[prev1]->constant) b = T_prev1->segment[s]->data[col][row];
		if (!S[last]->constant)  c = T->segment[s]->data[col][row];
		T_prev2->segment[s]->data[col][row] = (fabs (a) < GMT_CONV8_LIMIT) ? c : b;
	}
	return 0;
}

GMT_LOCAL int table_IN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: IN 2 1 Modified Bessel function of A (1st kind, order B).  */
{
	uint64_t s, row;
	unsigned int prev, order = 0;
	bool simple = false;
	double b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant) {
		if (S[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order < 0 for IN!\n");
		if (fabs (rint(S[last]->factor) - S[last]->factor) > GMT_CONV4_LIMIT) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order not an integer for IN!\n");
		order = urint (fabs (S[last]->factor));
		if (S[prev]->constant) {
			b = gmt_in (GMT, order, fabs (S[prev]->factor));
			simple = true;
		}
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (simple)
			T_prev->segment[s]->data[col][row] = b;
		else {
			if (!S[last]->constant) order = urint (fabs (T->segment[s]->data[col][row]));
			T_prev->segment[s]->data[col][row] = gmt_in (GMT, order, fabs (T_prev->segment[s]->data[col][row]));
		}
	}
	return 0;
}

GMT_LOCAL int table_INRANGE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		if (!S[prev2]->constant) a = T_prev2->segment[s]->data[col][row];
		if (!S[prev1]->constant) b = T_prev1->segment[s]->data[col][row];
		if (!S[last]->constant)  c = T->segment[s]->data[col][row];

		if (gmt_M_is_dnan (a) || gmt_M_is_dnan (b) || gmt_M_is_dnan (c)) {
			T_prev2->segment[s]->data[col][row] = GMT->session.d_NaN;
			continue;
		}

		inrange = (b <= a && a <= c) ? 1.0 : 0.0;
		T_prev2->segment[s]->data[col][row] = inrange;
	}
	return 0;
}

GMT_LOCAL int table_INT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: INT 1 1 Numerically integrate A.  */
{
	uint64_t s, row, k;
	double f = 0.0, left, right, sum;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) {	/* Trivial case */
		sum = S[last]->factor * info->t_inc;
		for (s = k = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++, k++) T->segment[s]->data[col][row] = ((info->local) ? row : k) * sum;
		return 0;
	}

	/* We use dumb trapezoidal rule - one day we will replace with more sophisticated rules */

	sum = 0.0;
	if (!info->irregular) f = 0.5 * info->t_inc;

	for (s = 0; s < info->T->n_segments; s++) {
		row = 0;
		if (info->local) sum = 0.0;	/* Reset integrated sum for each segment */
		while (row < info->T->segment[s]->n_rows) {
			left = T->segment[s]->data[col][row];
			T->segment[s]->data[col][row] = sum;
			row++;
			while (row < info->T->segment[s]->n_rows) {	/* Dumb trapezoidal rule */
				if (info->irregular) f = 0.5 * (info->T->segment[s]->data[COL_T][row] - info->T->segment[s]->data[COL_T][row-1]);
				right = T->segment[s]->data[col][row];
				sum += f * (left + right);
				T->segment[s]->data[col][row] = sum;
				left = right;
				row++;
			}
		}
	}
	return 0;
}

GMT_LOCAL int table_INV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: INV 1 1 1 / A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Inverse of zero gives NaNs\n");
	if (S[last]->constant) a = (S[last]->factor == 0) ? GMT->session.d_NaN : 1.0 / S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : 1.0 / T->segment[s]->data[col][row];
	return 0;
}

GMT_LOCAL int table_ISFINITE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ISFINITE 1 1 1 if A is finite, else 0.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = isfinite (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : isfinite (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ISNAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ISNAN 1 1 1 if A == NaN, else 0.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = (double)gmt_M_is_dnan (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : (double)gmt_M_is_dnan (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_J0 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: J0 1 1 Bessel function of A (1st kind, order 0).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = j0 (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : j0 (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_J1 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: J1 1 1 Bessel function of A (1st kind, order 1).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = j1 (fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : j1 (fabs (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_JN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: JN 2 1 Bessel function of A (1st kind, order B).  */
{
	uint64_t s, row;
	unsigned int prev, order = 0;
	bool simple = false;
	double b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant) {
		if (S[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order < 0 for JN!\n");
		if (fabs (rint(S[last]->factor) - S[last]->factor) > GMT_CONV4_LIMIT) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order not an integer for JN!\n");
		order = urint (fabs (S[last]->factor));
		if (S[prev]->constant) {
			b = jn ((int)order, fabs (S[prev]->factor));
			simple = true;
		}
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (simple)
			T_prev->segment[s]->data[col][row] = b;
		else {
			if (!S[last]->constant) order = urint (fabs (T->segment[s]->data[col][row]));
			T_prev->segment[s]->data[col][row] = jn ((int)order, fabs (T_prev->segment[s]->data[col][row]));
		}
	}
	return 0;
}

GMT_LOCAL int table_K0 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: K0 1 1 Modified Kelvin function of A (2nd kind, order 0).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_k0 (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_k0 (GMT, T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_K1 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: K1 1 1 Modified Bessel function of A (2nd kind, order 1).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_k1 (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_k1 (GMT, T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_KN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: KN 2 1 Modified Bessel function of A (2nd kind, order B).  */
{
	uint64_t s, row;
	unsigned int prev, order = 0;
	bool simple = false;
	double b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant) {
		if (S[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order < 0 for KN!\n");
		if (fabs (rint(S[last]->factor) - S[last]->factor) > GMT_CONV4_LIMIT) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order not an integer for KN!\n");
		order = urint (fabs (S[last]->factor));
		if (S[prev]->constant) {
			b = gmt_kn (GMT, order, fabs (S[prev]->factor));
			simple = true;
		}
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (simple)
			T_prev->segment[s]->data[col][row] = b;
		else {
			if (!S[last]->constant) order = urint (fabs (T->segment[s]->data[col][row]));
			T_prev->segment[s]->data[col][row] = gmt_kn (GMT, order, fabs (T_prev->segment[s]->data[col][row]));
		}
	}
	return 0;
}

GMT_LOCAL int table_KEI (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: KEI 1 1 kei (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_kei (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_kei (GMT, fabs (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_KER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: KER 1 1 ker (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_ker (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_ker (GMT, fabs (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_KURT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: KURT 1 1 Kurtosis of A.  */
{
	uint64_t s, row, n = 0;
	double mean = 0.0, sum2 = 0.0, kurt = 0.0, delta;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = GMT->session.d_NaN;
		return 0;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) {n = 0; mean = sum2 = kurt = 0.0;}	/* Reset for each segment */
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (gmt_M_is_dnan (T->segment[s]->data[col][row])) continue;
			n++;
			delta = T->segment[s]->data[col][row] - mean;
			mean += delta / n;
			sum2 += delta * (T->segment[s]->data[col][row] - mean);
		}
		if (info->local) {
			for (row = 0; row < info->T->segment[s]->n_rows; row++) {
				if (gmt_M_is_dnan (T->segment[s]->data[col][row])) continue;
				delta = T->segment[s]->data[col][row] - mean;
				kurt += pow (delta, 4.0);
			}
			if (n > 1) {
				sum2 /= (n - 1);
				kurt = kurt / (n * sum2 * sum2) - 3.0;
			}
			else
				kurt = GMT->session.d_NaN;
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = kurt;
		}
	}
	if (info->local) return 0;

	/* Here we do the global kurtosis */
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (gmt_M_is_dnan (T->segment[s]->data[col][row])) continue;
		delta = T->segment[s]->data[col][row] - mean;
		kurt += pow (delta, 4.0);
	}
	if (n > 1) {
		sum2 /= (n - 1);
		kurt = kurt / (n * sum2 * sum2) - 3.0;
	}
	else
		kurt = GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = kurt;
	return 0;
}

GMT_LOCAL int table_LAB2HSV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: LAB2HSV 3 3 Convert LAB to HSV, with L = A, a = B and b = C.  */
	uint64_t s, row;
	double hsv[4], lab[4], rgb[4];
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (info->scalar) {	/* Scalars have a stack of 3 constants */
		unsigned int prev1 = last - 1, prev2 = last - 2;
		struct GMT_DATATABLE *T_prev1 = S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
		if (S[prev2]->factor < 0.0 || S[prev2]->factor > 100.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument L to LAB2HSV must be a 0 <= L <= 100!\n");
			return -1;
		}
#if 0
		if (S[prev1]->factor < 0.0 || S[prev1]->factor > 1.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument S to LAB2HSV must be a 0 <= S <= 1!\n");
			return -1;
		}
		if (S[last]->factor < 0.0 || S[last]->factor > 1.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument V to LAB2HSV must be a 0 <= V <= 1!\n");
			return -1;
		}
#endif
		rgb[3] = hsv[3] = 0.0;	/* No transparency involved */
		lab[0] = S[prev2]->factor;
		lab[1] = S[prev1]->factor;
		lab[2] = S[last]->factor;
		gmt_lab_to_rgb (rgb, lab);	/* Must do this via RGB */
		gmt_rgb_to_hsv (rgb, hsv);
		T_prev2->segment[0]->data[col][0] = hsv[0];
		T_prev1->segment[0]->data[col][0] = hsv[1];
		T->segment[0]->data[col][0]       = hsv[2];
		return 0;
	}
	/* Table input, only one item on stack but has 3 columns; we wait for col == 2 */
	if (col != 2) return 0;
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			lab[0] = T->segment[s]->data[0][row];
			lab[1] = T->segment[s]->data[1][row];
			lab[2] = T->segment[s]->data[2][row];
			gmt_lab_to_rgb (rgb, lab);	/* Must do this via RGB */
			gmt_rgb_to_hsv (rgb, hsv);
			T->segment[s]->data[0][row] = hsv[0];
			T->segment[s]->data[1][row] = hsv[1];
			T->segment[s]->data[2][row] = hsv[2];
		}
	}
	return 0;
}

GMT_LOCAL int table_LAB2RGB (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: LAB2RGB 3 3 Convert LAB to RGB, with L = A, a = B and b = C.  */
	uint64_t s, row;
	double lab[3], rgb[3];
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (info->scalar) {	/* Scalars have a stack of 3 constants */
		unsigned int prev1 = last - 1, prev2 = last - 2;
		struct GMT_DATATABLE *T_prev1 = S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
		if (S[prev2]->factor < 0.0 || S[prev2]->factor > 100.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument L to LAB2HSV must be a 0 <= L <= 100!\n");
			return -1;
		}
#if 0
		if (S[prev1]->factor < 0.0 || S[prev1]->factor > 1.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument S to LAB2RGB must be a 0 <= S <= 1!\n");
			return -1;
		}
		if (S[last]->factor < 0.0 || S[last]->factor > 1.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument V to LAB2RGB must be a 0 <= V <= 1!\n");
			return -1;
		}
#endif
		lab[0] = S[prev2]->factor;
		lab[1] = S[prev1]->factor;
		lab[2] = S[last]->factor;
		gmt_lab_to_rgb (rgb, lab);
		T_prev2->segment[0]->data[col][0] = gmt_M_s255 (rgb[0]);
		T_prev1->segment[0]->data[col][0] = gmt_M_s255 (rgb[1]);
		T->segment[0]->data[col][0]       = gmt_M_s255 (rgb[2]);
		return 0;
	}
	/* Table input, only one item on stack but has 3 columns; we wait for col == 2 */
	if (col != 2) return 0;
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			lab[0] = T->segment[s]->data[0][row];
			lab[1] = T->segment[s]->data[1][row];
			lab[2] = T->segment[s]->data[2][row];
			gmt_lab_to_rgb (rgb, lab);
			T->segment[s]->data[0][row] = gmt_M_s255 (rgb[0]);
			T->segment[s]->data[1][row] = gmt_M_s255 (rgb[1]);
			T->segment[s]->data[2][row] = gmt_M_s255 (rgb[2]);
		}
	}
	return 0;
}

GMT_LOCAL int table_LAB2XYZ (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: LAB2XYZ 3 3 Convert LAB to XYZ, with L = A, a = B and b = C.  */
	uint64_t s, row;
	double lab[3], xyz[3];
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (info->scalar) {	/* Scalars have a stack of 3 constants */
		unsigned int prev1 = last - 1, prev2 = last - 2;
		struct GMT_DATATABLE *T_prev1 = S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
		if (S[prev2]->factor < 0.0 || S[prev2]->factor > 100.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument L to LAB2HSV must be a 0 <= L <= 100!\n");
			return -1;
		}
#if 0
		if (S[prev1]->factor < 0.0 || S[prev1]->factor > 1.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument S to LAB2XYZ must be a 0 <= S <= 1!\n");
			return -1;
		}
		if (S[last]->factor < 0.0 || S[last]->factor > 1.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument V to LAB2XYZ must be a 0 <= V <= 1!\n");
			return -1;
		}
#endif
		lab[0] = S[prev2]->factor;
		lab[1] = S[prev1]->factor;
		lab[2] = S[last]->factor;
		gmt_lab_to_xyz (xyz, lab);
		T_prev2->segment[0]->data[col][0] = xyz[0];
		T_prev1->segment[0]->data[col][0] = xyz[1];
		T->segment[0]->data[col][0]       = xyz[2];
		return 0;
	}
	/* Table input, only one item on stack but has 3 columns; we wait for col == 2 */
	if (col != 2) return 0;
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			lab[0] = T->segment[s]->data[0][row];
			lab[1] = T->segment[s]->data[1][row];
			lab[2] = T->segment[s]->data[2][row];
			gmt_lab_to_xyz (xyz, lab);
			T->segment[s]->data[0][row] = xyz[0];
			T->segment[s]->data[1][row] = xyz[1];
			T->segment[s]->data[2][row] = xyz[2];
		}
	}
	return 0;
}

/* Laplace stuff based on https://en.wikipedia.org/wiki/Laplace_distribution */

GMT_LOCAL int table_LCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LCDF 1 1 Laplace cumulative distribution function for z = A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = 0.5 + copysign (0.5, S[last]->factor) * (1.0 - exp (-fabs (S[last]->factor)));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a :  0.5 + copysign (0.5, T->segment[s]->data[col][row]) * (1.0 - exp (-fabs (T->segment[s]->data[col][row])));
	return 0;
}

GMT_LOCAL int table_LCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LCRIT 1 1 Laplace distribution critical value for alpha = A.  */
{
	uint64_t s, row;
	double a = 0.0, p;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) {
		p = (1.0 - S[last]->factor) - 0.5;
		a = -copysign (1.0, p) * log (1.0 - 2.0 * fabs (p));
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (S[last]->constant)
			T->segment[s]->data[col][row] = a;
		else {
			p = (1.0 - T->segment[s]->data[col][row]) - 0.5;
			T->segment[s]->data[col][row] = -copysign (1.0, p) * log (1.0 - 2.0 * fabs (p));
		}
	}
	return 0;
}

GMT_LOCAL int table_LE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LE 2 1 1 if A <= B, else 0.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	gmt_M_unused(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = (double)(a <= b);
	}
	return 0;
}

GMT_LOCAL int table_LMSSCL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LMSSCL 1 1 LMS scale estimate (LMS STD) of A.  */
{
	uint64_t s, row, k;
	unsigned int gmt_mode_selection = 0, GMT_n_multiples = 0;
	double lmsscl, mode, *z = NULL;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) gmt_M_memset (T->segment[s]->data[col], info->T->segment[s]->n_rows, double);
		return 0;
	}

	if (!info->local) z = gmt_M_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++)  {
		if (info->local) {
			gmt_sort_array (GMT, T->segment[s]->data[col], info->T->segment[s]->n_rows, GMT_DOUBLE);
			for (row = info->T->segment[s]->n_rows; row > 1 && gmt_M_is_dnan (T->segment[s]->data[col][row-1]); row--);
			if (row) {
				gmt_mode (GMT, T->segment[s]->data[col], row, row/2, 0, gmt_mode_selection, &GMT_n_multiples, &mode);
				gmt_getmad (GMT, T->segment[s]->data[col], row, mode, &lmsscl);
			}
			else
				lmsscl = GMT->session.d_NaN;

			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = lmsscl;
			if (GMT_n_multiples > 0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "%d Multiple modes found for segment %" PRIu64 "\n", GMT_n_multiples, s);
		}
		else {	/* Just accumulate the total table */
			gmt_M_memcpy (&z[k], T->segment[s]->data[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return 0;	/* Done with local */
	gmt_sort_array (GMT, z, info->T->n_records, GMT_DOUBLE);
	for (row = info->T->n_records; row > 1 && gmt_M_is_dnan (z[row-1]); row--);
	if (row) {
		gmt_mode (GMT, z, row, row/2, 0, gmt_mode_selection, &GMT_n_multiples, &mode);
		gmt_getmad (GMT, z, row, mode, &lmsscl);
	}
	else
		lmsscl = GMT->session.d_NaN;

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = lmsscl;
	if (GMT_n_multiples > 0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "%d Multiple modes found\n", GMT_n_multiples);
	gmt_M_free (GMT, z);
	return 0;
}

GMT_LOCAL int table_LMSSCLW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: LMSSCLW 1 1 Weighted LMS scale estimate (LMS STD) of A for weights in B.  */
	uint64_t s, row, k = 0;
	unsigned int prev;
	double wmode, lmsscl, w;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	struct GMT_OBSERVATION *pair = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++)
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = S[prev]->factor;
		return 0;
	}

	pair = gmt_M_memory (GMT, NULL, info->T->n_records, struct GMT_OBSERVATION);

	for (s = k = 0; s < info->T->n_segments; s++) {
		if (info->local) k = 0;	/* Reset count per segment */
		/* 1. Create array of value,weight pairs, skipping NaNs */
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (gmt_M_is_dnan (T_prev->segment[s]->data[col][row])) continue;
			if (S[last]->constant)
				w = S[last]->factor;
			else if (gmt_M_is_dnan (T->segment[s]->data[col][row]))
				continue;
			else
				w = T->segment[s]->data[col][row];
			pair[k].value  = (gmt_grdfloat)T_prev->segment[s]->data[col][row];
			pair[k].weight = (gmt_grdfloat)w;
			k++;
		}
		if (info->local) {	/* Report per segment */
			/* 2. Find the weighted mode */
			wmode = gmt_mode_weighted (GMT, pair, k);
			/* 3. Compute the absolute deviations from this mode */
			for (row = 0; row < k; row++) pair[row].value = (gmt_grdfloat)fabs (pair[row].value - wmode);
			/* 4. Find the weighted median absolue deviation and scale it */
			lmsscl = MAD_NORMALIZE * gmt_median_weighted (GMT, pair, k);
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = lmsscl;
		}
	}
	if (info->local) {		/* Done with local */
		gmt_M_free (GMT, pair);
		return 0;
	}
	/* 2. Find the weighted mode */
	wmode = gmt_mode_weighted (GMT, pair, k);
	/* 3. Compute the absolute deviations from this mode */
	for (row = 0; row < k; row++) pair[row].value = (gmt_grdfloat)fabs (pair[row].value - wmode);
	/* 4. Find the weighted median absolue deviation and scale it */
	lmsscl = MAD_NORMALIZE * gmt_median_weighted (GMT, pair, k);
	gmt_M_free (GMT, pair);

	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = lmsscl;
	return 0;
}

GMT_LOCAL int table_LOG (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: LOG 1 1 log (A) (natural log).  */
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "argument to log = 0!\n");

	if (S[last]->constant) a = d_log (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : d_log (GMT, fabs (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_LOG10 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LOG10 1 1 log10 (A) (base 10).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "argument to log10 = 0!\n");

	if (S[last]->constant) a = d_log10 (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : d_log10 (GMT, fabs (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_LOG1P (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LOG1P 1 1 log (1+A) (accurate for small A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "argument to log1p < 0!\n");

	if (S[last]->constant) a = d_log1p (GMT, fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : d_log1p (GMT, fabs (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_LOG2 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LOG2 1 1 log2 (A) (base 2).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "argument to log2 = 0!\n");

	if (S[last]->constant) a = d_log (GMT, fabs (S[last]->factor)) * M_LN2_INV;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : d_log (GMT, fabs (T->segment[s]->data[col][row])) * M_LN2_INV;
	return 0;
}

GMT_LOCAL int table_LOWER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LOWER 1 1 The lowest (minimum) value of A.  */
{
	uint64_t s, row;
	double low = DBL_MAX;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = S[last]->factor;
		return 0;
	}

	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) low = DBL_MAX;
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (gmt_M_is_dnan (T->segment[s]->data[col][row])) continue;
			if (T->segment[s]->data[col][row] < low) low = T->segment[s]->data[col][row];
		}
		if (low == DBL_MAX) low = GMT->session.d_NaN;
		if (info->local) for (row = 0; row < info->T->segment[s]->n_rows; row++) if (!gmt_M_is_dnan (T->segment[s]->data[col][row])) T->segment[s]->data[col][row] = low;
	}
	if (info->local) return 0;	/* Done with local */
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = low;
	return 0;
}

GMT_LOCAL int table_LPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LPDF 1 1 Laplace probability density function for z = A.  */
{
	uint64_t s, row;
	double z = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) z = 0.5 * exp (-fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? z : 0.5 *exp (-fabs (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_LRAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		if (!S[prev]->constant) a = T_prev->segment[s]->data[col][row];
		if (!S[last]->constant) b = T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = a + b * gmt_lrand (GMT);
	}
	return 0;
}

GMT_LOCAL int table_LSQFIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LSQFIT 1 0 Current table is [A | b]; return LS solution to A * x = b via Cholesky decomposition.  */
{
	gmt_M_unused(GMT); gmt_M_unused(info); gmt_M_unused(S); gmt_M_unused(last); gmt_M_unused(col);
	/* Dummy routine needed since the automatically generated include file will have table_LSQFIT
	 * with these parameters just like any other function.  However, when we find LSQFIT we will
	 * instead call solve_LSQFIT which can be found at the end of these functions */
	return 0;
}

GMT_LOCAL int table_LT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: LT 2 1 1 if A < B, else 0.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	gmt_M_unused(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = (double)(a < b);
	}
	return 0;
}

GMT_LOCAL int table_MAD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MAD 1 1 Median Absolute Deviation (L1 STD) of A.  */
{
	uint64_t s, row, k;
	double mad, med, *z = NULL;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) gmt_M_memset (T->segment[s]->data[col], info->T->segment[s]->n_rows, double);
		return 0;
	}

	if (!info->local) z = gmt_M_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++) {
		if (info->local) {
			gmt_sort_array (GMT, T->segment[s]->data[col], info->T->segment[s]->n_rows, GMT_DOUBLE);
			for (row = info->T->segment[s]->n_rows; row > 1 && gmt_M_is_dnan (T->segment[s]->data[col][row-1]); row--);
			if (row) {
				med = (row%2) ? T->segment[s]->data[col][row/2] : 0.5 * (T->segment[s]->data[col][(row-1)/2] + T->segment[s]->data[col][row/2]);
				gmt_getmad (GMT, T->segment[s]->data[col], row, med, &mad);
			}
			else
				mad = GMT->session.d_NaN;
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = mad;
		}
		else {	/* Just accumulate the total table */
			gmt_M_memcpy (&z[k], T->segment[s]->data[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return 0;	/* Done with local */
	gmt_sort_array (GMT, z, info->T->n_records, GMT_DOUBLE);
	for (row = info->T->n_records; row > 1 && gmt_M_is_dnan (z[row-1]); row--);
	if (row) {
		med = (row%2) ? z[row/2] : 0.5 * (z[(row-1)/2] + z[row/2]);
		gmt_getmad (GMT, z, row, med, &mad);
	}
	else
		mad = GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = mad;
	gmt_M_free (GMT, z);
	return 0;
}

GMT_LOCAL int table_MADW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: MADW 2 1 Weighted Median Absolute Deviation (L1 STD) of A for weights in B.  */
	uint64_t s, row, k = 0;
	unsigned int prev;
	double wmed, wmad, w;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	struct GMT_OBSERVATION *pair = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++)
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = S[prev]->factor;
		return 0;
	}

	pair = gmt_M_memory (GMT, NULL, info->T->n_records, struct GMT_OBSERVATION);

	for (s = k = 0; s < info->T->n_segments; s++) {
		if (info->local) k = 0;	/* Reset count per segment */
		/* 1. Create array of value,weight pairs, skipping NaNs */
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (gmt_M_is_dnan (T_prev->segment[s]->data[col][row])) continue;
			if (S[last]->constant)
				w = S[last]->factor;
			else if (gmt_M_is_dnan (T->segment[s]->data[col][row]))
				continue;
			else
				w = T->segment[s]->data[col][row];
			pair[k].value  = (gmt_grdfloat)T_prev->segment[s]->data[col][row];
			pair[k].weight = (gmt_grdfloat)w;
			k++;
		}
		if (info->local) {	/* Report per segment */
			/* 2. Find the weighted median */
			wmed = gmt_median_weighted (GMT, pair, k);
			/* 3. Compute the absolute deviations from this median */
			for (row = 0; row < k; row++) pair[row].value = (gmt_grdfloat)fabs (pair[row].value - wmed);
			/* 4. Find the weighted median absolue deviation */
			wmad = gmt_median_weighted (GMT, pair, k);
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = wmad;
		}
	}
	if (info->local) {		/* Done with local */
		gmt_M_free (GMT, pair);
		return 0;
	}
	/* 2. Find the weighted median */
	wmed = gmt_median_weighted (GMT, pair, k);
	/* 3. Compute the absolute deviations from this median */
	for (row = 0; row < k; row++) pair[row].value = (gmt_grdfloat)fabs (pair[row].value - wmed);
	/* 4. Find the weighted median absolue deviation */
	wmad = gmt_median_weighted (GMT, pair, k);
	gmt_M_free (GMT, pair);

	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = wmad;
	return 0;
}

GMT_LOCAL int table_MAX (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MAX 2 1 Maximum of A and B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = (gmt_M_is_dnan (a) || gmt_M_is_dnan (b)) ? GMT->session.d_NaN : MAX (a, b);
	}
	return 0;
}

GMT_LOCAL int table_MEAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MEAN 1 1 Mean value of A.  */
{
	uint64_t s, row, n_a = 0;
	double sum_a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = S[last]->factor;
		return 0;
	}

	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) sum_a = 0.0, n_a = 0;
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (gmt_M_is_dnan (T->segment[s]->data[col][row])) continue;
			sum_a += T->segment[s]->data[col][row];
			n_a++;
		}
		if (info->local) {
			sum_a = (n_a) ? sum_a / n_a : GMT->session.d_NaN;
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = sum_a;
		}
	}
	if (info->local) return 0;	/* Done with local */
	sum_a = (n_a) ? sum_a / n_a : GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = sum_a;
	return 0;
}

GMT_LOCAL int table_MEANW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MEANW 2 1 Weighted mean value of A for weights in B.  */
{
	uint64_t s, row, n_a = 0;
	unsigned int prev;
	double sum_zw = 0.0, sum_w = 0.0, zm, w;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = S[prev]->factor;
		return 0;
	}

	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) sum_zw = sum_w = 0.0, n_a = 0;
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (gmt_M_is_dnan (T_prev->segment[s]->data[col][row])) continue;
			if (S[last]->constant)
				w = S[last]->factor;
			else if (gmt_M_is_dnan (T->segment[s]->data[col][row]))
				continue;
			else
				w = T->segment[s]->data[col][row];
			sum_zw += T_prev->segment[s]->data[col][row] * w;
			sum_w += w;
			n_a++;
		}
		if (info->local) {
			zm = (n_a == 0 || sum_w == 0.0) ? GMT->session.d_NaN : (sum_zw / sum_w);
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = zm;
		}
	}
	if (info->local) return 0;	/* Done with local */
	zm = (n_a == 0 || sum_w == 0.0) ? GMT->session.d_NaN : (sum_zw / sum_w);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = zm;
	return 0;
}

GMT_LOCAL int table_MEDIAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MEDIAN 1 1 Median value of A.  */
{
	uint64_t s, row, k;
	double med, *z = NULL;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = S[last]->factor;
		return 0;
	}

	if (!info->local) z = gmt_M_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++) {
		if (info->local) {
			gmt_sort_array (GMT, T->segment[s]->data[col], info->T->segment[s]->n_rows, GMT_DOUBLE);
			for (row = info->T->segment[s]->n_rows; row > 1 && gmt_M_is_dnan (T->segment[s]->data[col][row-1]); row--);
			if (row)
				med = (row%2) ? T->segment[s]->data[col][row/2] : 0.5 * (T->segment[s]->data[col][(row-1)/2] + T->segment[s]->data[col][row/2]);
			else
				med = GMT->session.d_NaN;

			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = med;
		}
		else {	/* Just accumulate the total table */
			gmt_M_memcpy (&z[k], T->segment[s]->data[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return 0;	/* Done with local */
	gmt_sort_array (GMT, z, info->T->n_records, GMT_DOUBLE);
	for (row = info->T->n_records; row > 1 && gmt_M_is_dnan (z[row-1]); row--);
	if (row)
		med = (row%2) ? z[row/2] : 0.5 * (z[(row-1)/2] + z[row/2]);
	else
		med = GMT->session.d_NaN;

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = med;
	gmt_M_free (GMT, z);
	return 0;
}

GMT_LOCAL int table_MEDIANW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: MEDIANW 2 1 Weighted median value of A for weights in B.  */
	uint64_t s, row, k = 0;
	unsigned int prev;
	double wmed, w;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	struct GMT_OBSERVATION *pair = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++)
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = S[prev]->factor;
		return 0;
	}

	pair = gmt_M_memory (GMT, NULL, info->T->n_records, struct GMT_OBSERVATION);

	for (s = k = 0; s < info->T->n_segments; s++) {
		if (info->local) k = 0;
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (gmt_M_is_dnan (T_prev->segment[s]->data[col][row])) continue;
			if (S[last]->constant)
				w = S[last]->factor;
			else if (gmt_M_is_dnan (T->segment[s]->data[col][row]))
				continue;
			else
				w = T->segment[s]->data[col][row];
			pair[k].value  = (gmt_grdfloat)T_prev->segment[s]->data[col][row];
			pair[k].weight = (gmt_grdfloat)w;
			k++;
		}
		if (info->local) {
			wmed = (gmt_grdfloat)gmt_median_weighted (GMT, pair, k);
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = wmed;
		}
	}
	if (info->local) {		/* Done with local */
		gmt_M_free (GMT, pair);
		return 0;
	}
	wmed = (gmt_grdfloat)gmt_median_weighted (GMT, pair, k);
	gmt_M_free (GMT, pair);

	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = wmed;
	return 0;
}

GMT_LOCAL int table_MIN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MIN 2 1 Minimum of A and B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = (gmt_M_is_dnan (a) || gmt_M_is_dnan (b)) ? GMT->session.d_NaN : MIN (a, b);
	}
	return 0;
}

GMT_LOCAL int table_MOD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MOD 2 1 A mod B (remainder after floored division).  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "using MOD 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = MOD (a, b);
	}
	return 0;
}

GMT_LOCAL int table_MODE (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: MODE 1 1 Mode value (Least Median of Squares) of A.  */
	uint64_t s, row, k = 0;
	unsigned int gmt_mode_selection = 0, GMT_n_multiples = 0;
	double wmed, *z = NULL;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++)
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = S[last]->factor;
		return 0;
	}

	if (!info->local) z = gmt_M_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++) {
		if (info->local) {
			gmt_mode (GMT, T->segment[s]->data[col], info->T->segment[s]->n_rows, info->T->segment[s]->n_rows/2, true, gmt_mode_selection, &GMT_n_multiples, &wmed);
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = wmed;
		}
		else {	/* Just accumulate the total table */
			gmt_M_memcpy (&z[k], T->segment[s]->data[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return 0;	/* Done with local */
	gmt_mode (GMT, z, info->T->n_records, info->T->n_records/2, true, gmt_mode_selection, &GMT_n_multiples, &wmed);
	gmt_M_free (GMT, z);

	for (s = 0; s < info->T->n_segments; s++)
		for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = wmed;
	return 0;
}

GMT_LOCAL int table_MODEW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MODEW 2 1 Weighted mode value of A for weights in B.  */
{
	uint64_t s, row, k = 0;
	unsigned int prev;
	double wmode, w;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	struct GMT_OBSERVATION *pair = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = S[prev]->factor;
		return 0;
	}

	pair = gmt_M_memory (GMT, NULL, info->T->n_records, struct GMT_OBSERVATION);

	for (s = k = 0; s < info->T->n_segments; s++) {
		if (info->local) k = 0;
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (gmt_M_is_dnan (T_prev->segment[s]->data[col][row])) continue;
			if (S[last]->constant)
				w = S[last]->factor;
			else if (gmt_M_is_dnan (T->segment[s]->data[col][row]))
				continue;
			else
				w = T->segment[s]->data[col][row];
			pair[k].value  = (gmt_grdfloat)T_prev->segment[s]->data[col][row];
			pair[k].weight = (gmt_grdfloat)w;
			k++;
		}
		if (info->local) {
			wmode = (gmt_grdfloat)gmt_mode_weighted (GMT, pair, k);
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = wmode;
		}
	}
	if (info->local) {		/* Done with local */
		gmt_M_free (GMT, pair);
		return 0;
	}
	wmode = (gmt_grdfloat)gmt_mode_weighted (GMT, pair, k);
	gmt_M_free (GMT, pair);

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = wmode;
	return 0;
}

GMT_LOCAL int table_MUL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: MUL 2 1 A * B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "MUL: Operand one == 0!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "MUL: Operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = a * b;
	}
	return 0;
}

GMT_LOCAL int table_NAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		if (!S[prev]->constant) a = T_prev->segment[s]->data[col][row];
		if (!S[last]->constant) b = T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = ((a == b) ? GMT->session.d_NaN : a);
	}
	return 0;
}

GMT_LOCAL int table_NEG (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: NEG 1 1 -A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "NEG: Operand == 0!\n");
	if (S[last]->constant) a = -S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : -T->segment[s]->data[col][row];
	return 0;
}

GMT_LOCAL int table_NEQ (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: NEQ 2 1 1 if A != B, else 0.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	gmt_M_unused(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = (double)(a != b);
	}
	return 0;
}

GMT_LOCAL int table_NORM (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: NORM 1 1 Normalize (A) so max(A)-min(A) = 1.  */
{
	uint64_t s, row, n;
	double a, z, zmin = DBL_MAX, zmax = -DBL_MAX;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "NORM of a constant gives NaN!\n");
		a = GMT->session.d_NaN;
	}
	else {
		for (s = n = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			z = T->segment[s]->data[col][row];
			if (gmt_M_is_dnan (z)) continue;
			if (z < zmin) zmin = z;
			if (z > zmax) zmax = z;
			n++;
		}
		a = (n == 0 || zmax == zmin) ? GMT->session.d_NaN : 1.0 / (zmax - zmin);	/* Normalization scale */
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : a * (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_NOT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: NOT 1 1 NaN if A == NaN, 1 if A == 0, else 0.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "NOT: Operand == 0!\n");
	if (S[last]->constant) a = (fabs (S[last]->factor) > GMT_CONV8_LIMIT) ? 0.0 : 1.0;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : ((fabs (T->segment[s]->data[col][row]) > GMT_CONV8_LIMIT) ? 0.0 : 1.0);
	return 0;
}

GMT_LOCAL int table_NRAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		if (!S[prev]->constant) a = T_prev->segment[s]->data[col][row];
		if (!S[last]->constant) b = T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = a + b * gmt_nrand (GMT);
	}
	return 0;
}

GMT_LOCAL int table_OR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: OR 2 1 NaN if B == NaN, else A.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = (gmt_M_is_dnan (a) || gmt_M_is_dnan (b)) ? GMT->session.d_NaN : a;
	}
	return 0;
}

GMT_LOCAL int table_PERM (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PERM 2 1 Permutations n_P_r, with n = A and r = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument n to PERM must be a positive integer (n >= 0)!\n");
		return -1;
	}
	if (S[last]->constant && S[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to PERM must be a positive integer (r >= 0)!\n");
		return -1;
	}
	if (S[prev]->constant && S[last]->constant) {	/* PERM is given constant argument */
		double value = gmt_permutation (GMT, irint(S[prev]->factor), irint(S[last]->factor));
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = value;
		return 0;
	}
	/* Must run the full thing */
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = gmt_permutation (GMT, irint(a), irint(b));
	}
	return 0;
}

GMT_LOCAL int table_PLM (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "PLM: L and M must be constants!\n");
		return -1;
	}
	L = urint (S[prev]->factor);
	M = urint (S[last]->factor);

	if (S[first]->constant) a = gmt_plm (GMT, L, M, S[first]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_first->segment[s]->data[col][row] = (S[first]-> constant) ? a : gmt_plm (GMT, L, M, T_first->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_PLMg (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "PLMg: L and M must be constants!\n");
		return -1;
	}
	L = urint (S[prev]->factor);
	M = urint (S[last]->factor);

	if (S[first]->constant) a = gmt_plm_bar (GMT, L, M, S[first]->factor, false);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_first->segment[s]->data[col][row] = (S[first]-> constant) ? a : gmt_plm_bar (GMT, L, M, T_first->segment[s]->data[col][row], false);
	return 0;
}

GMT_LOCAL int table_POP (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: POP 1 0 Delete top element from the stack.  */
{
	gmt_M_unused(GMT); gmt_M_unused(info); gmt_M_unused(S); gmt_M_unused(last); gmt_M_unused(col);
	/* Dummy routine that does nothing but consume the top element of stack */
	return 0;
}

GMT_LOCAL int table_POW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: POW 2 1 A ^ B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "POW: Operand one == 0!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "POW: Operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = pow (a, b);
	}
	return 0;
}

GMT_LOCAL int table_PPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PPDF 2 1 Poisson probability density function for x = A and lambda = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double x, lambda;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		lambda = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		x  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = gmt_poissonpdf (GMT, x, lambda);
	}
	return 0;
}

GMT_LOCAL int table_PQUANT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "PQUANT must be given a constant quantile!\n");
		return -1;
	}
	if (S[last]->factor < 0.0 || S[last]->factor > 100.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "PQUANT must be given a constant quantile between 0-100%%!\n");
		return -1;
	}
	if (S[prev]->constant) {	/* Trivial case */
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "PQUANT of a constant is set to NaN\n");
		p = GMT->session.d_NaN;
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = p;
		return 0;
	}

	if (!info->local) z = gmt_M_memory (GMT, NULL, info->T->n_records, double);

	for (s = k = 0; s < info->T->n_segments; s++)  {
		if (info->local) {
			gmt_sort_array (GMT, T_prev->segment[s]->data[col], info->T->segment[s]->n_rows, GMT_DOUBLE);
			p = gmt_quantile (GMT, T_prev->segment[s]->data[col], S[last]->factor, info->T->segment[s]->n_rows);
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = p;
		}
		else {	/* Just accumulate the total table */
			gmt_M_memcpy (&z[k], T_prev->segment[s]->data[col], info->T->segment[s]->n_rows, double);
			k += info->T->segment[s]->n_rows;
		}
	}
	if (info->local) return 0;	/* Done with local */
	gmt_sort_array (GMT, z, info->T->n_records, GMT_DOUBLE);
	p = gmt_quantile (GMT, z, S[last]->factor, info->T->n_records);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = p;
	gmt_M_free (GMT, z);
	return 0;
}

GMT_LOCAL int table_PQUANTW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PQUANTW 3 1 The C'th Quantile (0-100%) of A for weights in B.  */
{
	uint64_t s, row, k = 0;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	double p, q, w;
	struct GMT_DATATABLE *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
	struct GMT_OBSERVATION *pair = NULL;

	/* last holds the selected quantile (0-100), prev the data % */
	if (!S[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "PQUANTW must be given a constant quantile!\n");
		return -1;
	}
	if (S[last]->factor < 0.0 || S[last]->factor > 100.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "PQUANTW must be given a constant quantile between 0-100%%!\n");
		return -1;
	}
	if (S[prev2]->constant) {	/* Trivial case */
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "PQUANTW of a constant is set to NaN\n");
		p = GMT->session.d_NaN;
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev2->segment[s]->data[col][row] = p;
		return 0;
	}

	pair = gmt_M_memory (GMT, NULL, info->T->n_records, struct GMT_OBSERVATION);
	q = 0.01 * S[last]->factor;

	for (s = k = 0; s < info->T->n_segments; s++) {
		if (info->local) k = 0;
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (gmt_M_is_dnan (T_prev2->segment[s]->data[col][row])) continue;
			if (S[prev1]->constant)
				w = S[prev1]->factor;
			else if (gmt_M_is_dnan (T_prev1->segment[s]->data[col][row]))
				continue;
			else
				w = T_prev1->segment[s]->data[col][row];
			pair[k].value  = (gmt_grdfloat)T_prev2->segment[s]->data[col][row];
			pair[k].weight = (gmt_grdfloat)w;

			k++;
		}
		if (info->local) {
			p = (gmt_grdfloat)gmt_quantile_weighted (GMT, pair, k, q);
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev2->segment[s]->data[col][row] = p;
		}
	}
	if (info->local) {		/* Done with local */
		gmt_M_free (GMT, pair);
		return 0;
	}
	p = (gmt_grdfloat)gmt_quantile_weighted (GMT, pair, k, q);
	gmt_M_free (GMT, pair);

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev2->segment[s]->data[col][row] = p;
	return 0;
}

GMT_LOCAL int table_PSI (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PSI 1 1 Psi (or Digamma) of A.  */
{
	uint64_t s, row;
	double a = 0.0, x[2];
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	x[1] = 0.0;	/* No imaginary part */
	if (S[last]->constant) {
		x[0] = S[last]->factor;
		a = gmt_psi (GMT, x, NULL);
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[last]->constant) {
			x[0] = T->segment[s]->data[col][row];
			a = gmt_psi (GMT, x, NULL);
		}
		T->segment[s]->data[col][row] = a;
	}
	return 0;
}

GMT_LOCAL int table_PVQV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col, unsigned int kind)
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
		if ((S[first]->factor < -1.0 || S[first]->factor > 1.0)) GMT_Report (GMT->parent, GMT_MSG_WARNING, "argument to %s outside domain!\n", name[kind]);
		gmt_PvQv (GMT, S[first]->factor, nu, pq, &n);
		a = pq[2*kind];
	}
	if (S[prev]->constant) nu[0] = S[prev]->factor;
	if (S[last]->constant) nu[1] = S[last]->factor;
	if (S[first]-> constant)    x = S[first]->factor;
	kind *= 2;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (calc){
			if (!S[prev]->constant) nu[0] = T_prev->segment[s]->data[col][row];
			if (!S[last]->constant) nu[1] = T->segment[s]->data[col][row];
			if (!S[first]-> constant)    x = T_first->segment[s]->data[col][row];
			gmt_PvQv (GMT, x, nu, pq, &n);
			a = pq[kind];
		}
		T_first->segment[s]->data[col][row] = a;
	}
	return 0;
}

GMT_LOCAL int table_PV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: PV 3 1 Legendre function Pv(A) of degree v = real(B) + imag(C).  */
{
	table_PVQV (GMT, info, S, last, col, 0);
	return 0;
}

GMT_LOCAL int table_QV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: QV 3 1 Legendre function Qv(A) of degree v = real(B) + imag(C).  */
{
	table_PVQV (GMT, info, S, last, col, 1);
	return 0;
}

GMT_LOCAL int table_R2 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: R2 2 1 R2 = A^2 + B^2.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "R2: Operand one == 0!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "R2: Operand two == 0!\n");
	if (S[prev]->constant) S[prev]->factor *= S[prev]->factor;
	if (S[last]->constant) S[last]->factor *= S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row] * T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row] * T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = a + b;
	}
	return 0;
}

GMT_LOCAL int table_R2D (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: R2D 1 1 Convert Radians to Degrees.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = S[last]->factor * R2D;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : T->segment[s]->data[col][row] * R2D;
	return 0;
}

GMT_LOCAL int table_RAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		if (!S[prev]->constant) a = T_prev->segment[s]->data[col][row];
		if (!S[last]->constant) b = T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = a + gmt_rand (GMT) * (b - a);
	}
	return 0;
}

GMT_LOCAL int table_RCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: RCDF 1 1 Rayleigh cumulative distribution function for z = A.  */
{
	uint64_t s, row;
	double z;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		z = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T->segment[s]->data[col][row] = 1.0 - exp (-0.5*z*z);
	}
	return 0;
}

GMT_LOCAL int table_RCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: RCRIT 1 1 Rayleigh distribution critical value for alpha = A.  */
{
	uint64_t s, row;
	double alpha;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		alpha = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T->segment[s]->data[col][row] = M_SQRT2 * sqrt (-log (1.0 - alpha));
	}
	return 0;
}

GMT_LOCAL int table_RPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: RPDF 1 1 Rayleigh probability density function for z = A.  */
{
	uint64_t s, row;
	double z;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		z = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T->segment[s]->data[col][row] = z * exp (-0.5 * z * z);
	}
	return 0;
}

GMT_LOCAL int table_RGB2HSV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: RGB2HSV 3 3 Convert RGB to HSV, with r = A, b = B and b = C.  */
	uint64_t s = 0, row;
	double rgb[4], hsv[4];
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (info->scalar) {	/* Three stacks given since three separate values on the stack */
		unsigned int prev1 = last - 1, prev2 = last - 2;
		struct GMT_DATATABLE *T_prev1 = S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
		if (S[prev2]->factor < 0.0 || S[prev2]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to RGB2HSV must be a 0 <= r <= 255!\n");
			return -1;
		}
		if (S[prev1]->factor < 0.0 || S[prev1]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument g to RGB2HSV must be a 0 <= g <= 255!\n");
			return -1;
		}
		if (S[last]->factor < 0.0 || S[last]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument b to RGB2HSV must be a 0 <= b <= 255!\n");
			return -1;
		}
		rgb[3] = hsv[3] = 0.0;	/* No transparency involved */
		rgb[0] = gmt_M_is255 (S[prev2]->factor);
		rgb[1] = gmt_M_is255 (S[prev1]->factor);
		rgb[2] = gmt_M_is255 (S[last]->factor);
		gmt_rgb_to_hsv (rgb, hsv);
		/* Only a single segment with one row */
		T_prev2->segment[0]->data[col][0] = hsv[0];
		T_prev1->segment[0]->data[col][0] = hsv[1];
		T->segment[0]->data[col][0] = hsv[2];
		return 0;
	}
	/* Operate across rows. col must be 2 */
	if (col != 2) return 0;	/* Do nothing */
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			rgb[0] = gmt_M_is255 (T->segment[s]->data[0][row]);
			rgb[1] = gmt_M_is255 (T->segment[s]->data[1][row]);
			rgb[2] = gmt_M_is255 (T->segment[s]->data[2][row]);
			gmt_rgb_to_hsv (rgb, hsv);
			T->segment[s]->data[0][row] = hsv[0];
			T->segment[s]->data[1][row] = hsv[1];
			T->segment[s]->data[2][row] = hsv[2];
		}
	}
	return 0;
}

GMT_LOCAL int table_RGB2LAB (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: RGB2LAB 3 3 Convert RGB to LAB, with r = A, g = B and b = C.  */
	uint64_t s = 0, row;
	double rgb[3], lab[3];
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (info->scalar) {	/* Three stacks given since three separate values on the stack */
		unsigned int prev1 = last - 1, prev2 = last - 2;
		struct GMT_DATATABLE *T_prev1 = S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
		if (S[prev2]->factor < 0.0 || S[prev2]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to RGB2HSV must be a 0 <= r <= 255!\n");
			return -1;
		}
		if (S[prev1]->factor < 0.0 || S[prev1]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument g to RGB2HSV must be a 0 <= g <= 255!\n");
			return -1;
		}
		if (S[last]->factor < 0.0 || S[last]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument b to RGB2HSV must be a 0 <= b <= 255!\n");
			return -1;
		}
		rgb[0] = gmt_M_is255 (S[prev2]->factor);
		rgb[1] = gmt_M_is255 (S[prev1]->factor);
		rgb[2] = gmt_M_is255 (S[last]->factor);
		gmt_rgb_to_lab (rgb, lab);
		/* Only a single segment with one row */
		T_prev2->segment[0]->data[col][0] = lab[0];
		T_prev1->segment[0]->data[col][0] = lab[1];
		T->segment[0]->data[col][0] = lab[2];
		return 0;
	}
	/* Operate across rows. col must be 2 */
	if (col != 2) return 0;	/* Do nothing */
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			rgb[0] = gmt_M_is255 (T->segment[s]->data[0][row]);
			rgb[1] = gmt_M_is255 (T->segment[s]->data[1][row]);
			rgb[2] = gmt_M_is255 (T->segment[s]->data[2][row]);
			gmt_rgb_to_lab (rgb, lab);
			T->segment[s]->data[0][row] = lab[0];
			T->segment[s]->data[1][row] = lab[1];
			T->segment[s]->data[2][row] = lab[2];
		}
	}
	return 0;
}

GMT_LOCAL int table_RGB2XYZ (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: RGB2XYZ 3 3 Convert RGB to LAB, with r = A, g = B and b = C.  */
	uint64_t s = 0, row;
	double rgb[3], xyz[3];
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (info->scalar) {	/* Three stacks given since three separate values on the stack */
		unsigned int prev1 = last - 1, prev2 = last - 2;
		struct GMT_DATATABLE *T_prev1 = S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
		if (S[prev2]->factor < 0.0 || S[prev2]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to RGB2XYZ must be a 0 <= r <= 255!\n");
			return -1;
		}
		if (S[prev1]->factor < 0.0 || S[prev1]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument g to RGB2XYZ must be a 0 <= g <= 255!\n");
			return -1;
		}
		if (S[last]->factor < 0.0 || S[last]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument b to RGB2XYZ must be a 0 <= b <= 255!\n");
			return -1;
		}
		rgb[0] = gmt_M_is255 (S[prev2]->factor);
		rgb[1] = gmt_M_is255 (S[prev1]->factor);
		rgb[2] = gmt_M_is255 (S[last]->factor);
		gmt_rgb_to_xyz (rgb, xyz);
		/* Only a single segment with one row */
		T_prev2->segment[0]->data[col][0] = xyz[0];
		T_prev1->segment[0]->data[col][0] = xyz[1];
		T->segment[0]->data[col][0] = xyz[2];
		return 0;
	}
	/* Operate across rows. col must be 2 */
	if (col != 2) return 0;	/* Do nothing */
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			rgb[0] = gmt_M_is255 (T->segment[s]->data[0][row]);
			rgb[1] = gmt_M_is255 (T->segment[s]->data[1][row]);
			rgb[2] = gmt_M_is255 (T->segment[s]->data[2][row]);
			gmt_rgb_to_xyz (rgb, xyz);
			T->segment[s]->data[0][row] = xyz[0];
			T->segment[s]->data[1][row] = xyz[1];
			T->segment[s]->data[2][row] = xyz[2];
		}
	}
	return 0;
}

GMT_LOCAL int table_RINT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: RINT 1 1 rint (A) (round to integral value nearest to A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = rint (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : rint (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_RMS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: RMS 1 1 Root-mean-square of A.  */
{
	uint64_t s, row, n = 0;
	double sum2 = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) sum2 = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) {
		if (!S[last]->constant) {
			if (info->local) {n = 0; sum2 = 0.0;}	/* Start anew for each segment */
			for (row = 0; row < info->T->segment[s]->n_rows; row++) {
				if (gmt_M_is_dnan (T->segment[s]->data[col][row])) continue;
				n++;
				sum2 += (T->segment[s]->data[col][row] * T->segment[s]->data[col][row]);
			}
			if (info->local) {
				sum2 = (n > 0) ? sqrt (sum2 / n) : GMT->session.d_NaN;
			}
		}
		if (info->local) {
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = sum2;
		}
	}
	if (info->local) return 0;	/* Done with local */
	if (!S[last]->constant) sum2 = (n > 0) ? sqrt (sum2 / n) : GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = sum2;
	return 0;
}

GMT_LOCAL int table_RMSW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: RMSW 2 1 Weighted Root-mean-square of A for weights in B.  */
{
	uint64_t s, row, n = 0;
	unsigned int prev;
	double sum2 = 0.0, sumw = 0.0, w;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) sum2 = S[prev]->factor;
	for (s = 0; s < info->T->n_segments; s++) {
		if (!S[prev]->constant) {
			if (info->local) {n = 0; sum2 = sumw = 0.0;}	/* Start anew for each segment */
			for (row = 0; row < info->T->segment[s]->n_rows; row++) {
				if (gmt_M_is_dnan (T_prev->segment[s]->data[col][row])) continue;
				if (S[last]->constant)
					w = S[last]->factor;
				else if (gmt_M_is_dnan (T->segment[s]->data[col][row]))
					continue;
				else
					w = T->segment[s]->data[col][row];
				n++;
				sum2 += w * (T_prev->segment[s]->data[col][row] * T_prev->segment[s]->data[col][row]);
				sumw += w;
			}
			if (info->local) {
				sum2 = (sumw > 0.0) ? sqrt (sum2 / sumw) : GMT->session.d_NaN;
			}
		}
		if (info->local) {
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = sum2;
		}
	}
	if (info->local) return 0;	/* Done with local */
	if (!S[prev]->constant) sum2 = (n > 0) ? sqrt (sum2 / sumw) : GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = sum2;
	return 0;
}

GMT_LOCAL void assign_gmtstack (struct GMTMATH_STACK *Sto, struct GMTMATH_STACK *Sfrom)
{	/* Copy contents of Sfrom to Sto */
	Sto->D          = Sfrom->D;
	Sto->constant   = Sfrom->constant;
	Sto->factor     = Sfrom->factor;
}

GMT_LOCAL int table_ROLL (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ROLL 2 0 Cyclicly shifts the top A stack items by an amount B.  */
{
	unsigned int prev, top, bottom, k, kk, n_items;
	int n_shift;
	struct GMTMATH_STACK Stmp;
	gmt_M_unused(GMT); gmt_M_unused(info); gmt_M_unused(col);
	assert (last > 2);	/* Must have at least 3 items on the stack: A single item plus the two roll arguments */
	prev = last - 1;	/* This gives the number of stack items to include in the cycle */
	if (!(S[last]->constant && S[prev]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "length and shift must be constants in ROLL!\n");
		return -1;
	}
	n_items = urint (S[prev]->factor);
	n_shift = irint (S[last]->factor);
	if (n_items > prev) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Items on stack is fewer than required by ROLL!\n");
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

GMT_LOCAL int table_ROTT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
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
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "T-shift must be a constant in ROTT!\n");
		return -1;
	}
	shift = irint (S[last]->factor / info->t_inc);
	if (S[prev]->constant || !shift) return 0;	/* Easy, constant or no shift */
	if (!info->local) {
		if (shift < 0) shift += (int)info->T->n_records;		/* Same thing */
		z = gmt_M_memory (GMT, NULL, info->T->n_records, double);
	}
	for (s = k = 0; s < info->T->n_segments; s++)  {
		if (info->local) {
			shift = irint (S[last]->factor / info->t_inc);
			if (shift < 0) shift += (int)info->T->segment[s]->n_rows;		/* Same thing */
			z = gmt_M_memory (GMT, NULL, info->T->segment[s]->n_rows, double);
		}

		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			j = (info->local) ? (row+shift)%info->T->segment[s]->n_rows : (k+shift)%info->T->n_records;
			z[j] = T_prev->segment[s]->data[col][row];
		}
		if (info->local) {
			gmt_M_memcpy (T_prev->segment[s]->data[col], z, info->T->segment[s]->n_rows, double);
			gmt_M_free (GMT, z);
		}
	}
	if (info->local) return 0;	/* Done with local */
	for (s = k = 0; s < info->T->n_segments; s++, k += info->T->segment[s]->n_rows) gmt_M_memcpy (T_prev->segment[s]->data[col], &z[k], info->T->segment[s]->n_rows, double);
	gmt_M_free (GMT, z);
	return 0;
}

GMT_LOCAL int table_SEC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SEC 1 1 sec (A) (A in radians).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = (1.0 / cos (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : (1.0 / cos (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_SECD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SECD 1 1 sec (A) (A in degrees).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = (1.0 / cosd (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : (1.0 / cosd (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_SECH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SECH 1 1 sech (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = 1.0 / cosh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : 1.0 / cosh (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_SIGN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SIGN 1 1 sign (+1 or -1) of A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "SIGN: Operand == 0!\n");
	if (S[last]->constant) a = copysign (1.0, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : copysign (1.0, T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_SIN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SIN 1 1 sin (A) (A in radians).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = sin (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : sin (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_SINC (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SINC 1 1 sinc (A) (sin (pi*A)/(pi*A)).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_sinc (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_sinc (GMT, T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_SIND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SIND 1 1 sin (A) (A in degrees).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = sind (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : sind (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_SINH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SINH 1 1 sinh (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = sinh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : sinh (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_SKEW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SKEW 1 1 Skewness of A.  */
{
	uint64_t s, row, n = 0;
	double mean = 0.0, sum2 = 0.0, skew = 0.0, delta;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = GMT->session.d_NaN;
		return 0;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) {n = 0; mean = sum2 = skew = 0.0; }	/* Start anew for each segment */
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (gmt_M_is_dnan (T->segment[s]->data[col][row])) continue;
			n++;
			delta = T->segment[s]->data[col][row] - mean;
			mean += delta / n;
			sum2 += delta * (T->segment[s]->data[col][row] - mean);
		}
		if (info->local) {
			if (n > 1) {
				for (row = 0; row < info->T->segment[s]->n_rows; row++) {
					if (gmt_M_is_dnan (T->segment[s]->data[col][row])) continue;
					delta = T->segment[s]->data[col][row] - mean;
					skew += pow (delta, 3.0);
				}
				sum2 /= (n - 1);
				skew /= n * pow (sum2, 1.5);
			}
			else
				skew = GMT->session.d_NaN;
			for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = skew;
		}
	}
	if (info->local) return 0;	/* Done with local */
	if (n > 1) {
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (gmt_M_is_dnan (T->segment[s]->data[col][row])) continue;
			delta = T->segment[s]->data[col][row] - mean;
			skew += pow (delta, 3.0);
		}
		sum2 /= (n - 1);
		skew /= n * pow (sum2, 1.5);
	}
	else
		skew = GMT->session.d_NaN;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = skew;
	return 0;
}

GMT_LOCAL void free_sort_list (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info) {
	/* Free any column sorting helper arrays */
	uint64_t s;
	if (info->Q == NULL) return;
	for (s = 0; s < ((info->local) ? info->T->n_segments : 1); s++)
		gmt_M_free (GMT, info->Q[s]);
	gmt_M_free (GMT, info->Q);
}

GMT_LOCAL int table_SORT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SORT 3 1 Sort all columns in stack based on column A in direction of B (-1 descending |+1 ascending).  */
{
	uint64_t s, seg, row, k0 = 0, k = 0;
	unsigned int scol;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	int dir;
	struct GMT_DATATABLE *T_prev2 = S[prev2]->D->table[0];
	struct GMT_ORDER *Z = NULL;

	if (!S[prev1]->constant || S[prev1]->factor < 0.0 || (scol = urint (S[prev1]->factor)) >= info->n_col) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "SORT: Column must be a constant column number (0 <= k < n_col)!\n");
		return -1;
	}
	if (!S[last]->constant || !((dir = lrint (S[last]->factor)) == -1 || dir == +1)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "SORT: Direction must be -1 (decreasing) or +1 (increasing)!\n");
		return 0;
	}

	if (info->Q == NULL) {	/* First time we must determine the sorting key */
		info->Q = gmt_M_memory (GMT, NULL, (info->local) ? info->T->n_segments : 1, struct GMT_ORDER *);
		if (!info->local) Z = info->Q[0] = gmt_M_memory (GMT, NULL, info->T->n_records, struct GMT_ORDER);
		for (s = k = 0; s < info->T->n_segments; s++) {
			if (info->local) {	/* Sort each segment independently */
				Z = info->Q[s] = gmt_M_memory (GMT, NULL, info->T->segment[s]->n_rows, struct GMT_ORDER);
				seg = s;
				k = 0;	/* Reset for new segment */
			}
			else	/* Sort the whole enchilada */
				seg = 0;
			for (row = 0; row < info->T->segment[s]->n_rows; row++, k++) {
				Z[k].value = T_prev2->segment[s]->data[scol][row];
				Z[k].order = k;
			}
			if (info->local) /* Sort per segment */
				gmt_sort_order (GMT, info->Q[seg], k, dir);
		}
		if (!info->local)	/* Sort the whole enchilada */
			gmt_sort_order (GMT, info->Q[0], k, dir);
	}

	/* OK now we can deal with shuffling of rows based on how the selected column was sorted */
	if (!info->local) gmt_prep_tmp_arrays (GMT, GMT_IN, info->T->n_records, 1);	/* Init or reallocate tmp vectors once if the entire table */
	for (s = k0 = 0; s < info->T->n_segments; s++) {
		if (info->local) {	/* Do the shuffle on a segment-by-segment basis */
			seg = s;
			k0 = 0;	/* Reset for new segment */
		}
		else	/* Just do everything at once, so k0 increases by n_rows after each segment */
			seg = 0;
		Z = info->Q[seg];	/* Pointer to this segment's (or all) order scheme */
		if (info->local) gmt_prep_tmp_arrays (GMT, GMT_IN, info->T->segment[s]->n_rows, 1);	/* Init or reallocate tmp vectors if a segment is longer */
		for (col = 0; col < info->n_col; col++) {
			k = k0;	/* Reset for each column */
			for (row = 0; row < info->T->segment[s]->n_rows; row++, k++) /* Do the shuffle via a temp vector */
				GMT->hidden.mem_coord[GMT_X][row] = T_prev2->segment[s]->data[col][Z[k].order];
			gmt_M_memcpy (T_prev2->segment[s]->data[col], GMT->hidden.mem_coord[GMT_X], info->T->segment[s]->n_rows, double);
		}
		k0 += info->T->segment[s]->n_rows;	/* May be reset above if local */
	}
	return 0;
}

GMT_LOCAL int table_SQR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SQR 1 1 A^2.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = S[last]->factor * S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : T->segment[s]->data[col][row] *  T->segment[s]->data[col][row];
	return 0;
}

GMT_LOCAL int table_SQRT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SQRT 1 1 sqrt (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one < 0!\n");
	if (S[last]->constant) a = sqrt (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : sqrt (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_STD (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: STD 1 1 Standard deviation of A.  */
{
	uint64_t s, row;
	double std;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && info->T->n_records < 2)	/* Trivial case: std is undefined */
		std = GMT->session.d_NaN;
	else {	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
		uint64_t n = 0;
		double mean = 0.0, sum2 = 0.0, delta;
		for (s = 0; s < info->T->n_segments; s++) {
			if (info->local) {n = 0; mean = sum2 = 0.0;}	/* Start anew for each segment */
			for (row = 0; row < info->T->segment[s]->n_rows; row++) {
				if (gmt_M_is_dnan (T->segment[s]->data[col][row])) continue;
				n++;
				delta = T->segment[s]->data[col][row] - mean;
				mean += delta / n;
				sum2 += delta * (T->segment[s]->data[col][row] - mean);
			}
			if (info->local) {
				std = (n > 1) ? sqrt (sum2 / (n - 1)) : GMT->session.d_NaN;
				for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = std;
			}
		}
		if (info->local) return 0;	/* Done with local */
		std = (n > 1) ? sqrt (sum2 / (n - 1)) : GMT->session.d_NaN;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = std;
	return 0;
}

GMT_LOCAL int table_STDW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: STDW 2 1 Weighted standard deviation of A for weights in B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double std;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && info->T->n_records < 2)	/* Trivial case: std is undefined */
		std = GMT->session.d_NaN;
	else {	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
		uint64_t n = 0;
		double temp, mean = 0.0, sumw = 0.0, delta, R, M2 = 0.0, w;
		for (s = 0; s < info->T->n_segments; s++) {
			if (info->local) {n = 0; mean = sumw = M2 = 0.0;}	/* Start anew for each segment */
			for (row = 0; row < info->T->segment[s]->n_rows; row++) {
				if (gmt_M_is_dnan (T_prev->segment[s]->data[col][row])) continue;
				if (S[last]->constant)
					w = S[last]->factor;
				else if (gmt_M_is_dnan (T->segment[s]->data[col][row]))
					continue;
				else
					w = T->segment[s]->data[col][row];
				temp = w + sumw;
				delta = T_prev->segment[s]->data[col][row] - mean;
				R = delta * w / temp;
				mean += R;
				M2 += sumw * delta * R;
				sumw = temp;
				n++;
			}
			if (info->local) {
				std = (n > 1) ? sqrt ((n * M2 / sumw) / (n - 1.0)) : GMT->session.d_NaN;
				for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = std;
			}
		}
		if (info->local) return 0;	/* Done with local */
		std = (n > 1) ? sqrt ((n * M2 / sumw) / (n - 1.0)) : GMT->session.d_NaN;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = std;
	return 0;
}

GMT_LOCAL int table_STEP (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: STEP 1 1 Heaviside step function H(A).  */
{
	uint64_t s, row;
	double a;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		if (a == 0.0)
			T->segment[s]->data[col][row] = 0.5;
		else
			T->segment[s]->data[col][row] = (a < 0.0) ? 0.0 : 1.0;
	}
	return 0;
}

GMT_LOCAL int table_STEPT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: STEPT 1 1 Heaviside step function H(t-A).  */
{
	uint64_t s, row;
	double a;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = info->T->segment[s]->data[COL_T][row] - ((S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row]);
		if (a == 0.0)
			T->segment[s]->data[col][row] = 0.5;
		else
			T->segment[s]->data[col][row] = (a < 0.0) ? 0.0 : 1.0;
	}
	return 0;
}

GMT_LOCAL int table_SUB (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SUB 2 1 A - B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "SUB: Operand one == 0!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "SUB: Operand two == 0!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = a - b;
	}
	return 0;
}

GMT_LOCAL int table_SUM (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SUM 1 1 Cumulative sum of A.  */
{
	uint64_t s, row;
	double a = 0.0, sum = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) sum = 0.0;	/* Reset for each segment */
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (!S[last]->constant) a = T->segment[s]->data[col][row];
			if (!gmt_M_is_dnan (a)) sum += a;
			T->segment[s]->data[col][row] = sum;
		}
	}
	return 0;
}

GMT_LOCAL int table_SVDFIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: SVDFIT 1 0 Current table is [A | b]; return LS solution to A * x = B via SVD decomposition (see -E).  */
{
	gmt_M_unused(GMT); gmt_M_unused(info); gmt_M_unused(S); gmt_M_unused(last); gmt_M_unused(col);
	/* Dummy routine needed since the automatically generated include file will have table_SVDFIT
	 * with these parameters just like any other function.  However, when we find SVDFIT we will
	 * instead call solve_SVDFIT which can be found at the end of these functions */
	return 0;
}

GMT_LOCAL int table_TAN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TAN 1 1 tan (A) (A in radians).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = tan (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : tan (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_TAND (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TAND 1 1 tan (A) (A in degrees).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = tand (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : tand (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_TANH (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TANH 1 1 tanh (A).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) a = tanh (S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : tanh (T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_TAPER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TAPER 1 1 Unit weights cosine-tapered to zero within A of end margins.  */
{
	/* If no time, then A is interpreted to mean number of nodes instead */
	uint64_t s, row;
	double strip, scale, t_min, t_max, start, stop, from_start, from_stop, t, w_t;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (!S[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "TAPER: Argument A must be a constant!\n");
		return -1;
	}
	strip = S[last]->factor;
	scale = M_PI / strip;
	for (s = 0; s < info->T->n_segments; s++) {
		if (info->notime) {	/* If no time then A refers to number of rows and min and max are in rows */
			t_min = 0.0;
			t_max = info->T->segment[s]->n_rows - 1.0;
		}
		else {	/* Here, A is in time units and the min/max are start/stop time per segment */
			t_min = info->T->segment[s]->data[COL_T][0];	/* Start of time for this segment */
			t_max = info->T->segment[s]->data[COL_T][info->T->segment[s]->n_rows-1];	/* End of time for this segment */
		}
		start = strip + t_min;
		stop  = strip - t_max;
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			t = (info->notime) ? (double)row : info->T->segment[s]->data[COL_T][row];
			from_start = start - t;
			if (from_start > 0.0) w_t = 0.5 * (1.0 + cos (from_start * scale));
			else if ((from_stop = stop + t) > 0.0) w_t = 0.5 * (1.0 + cos (from_stop * scale));
			else w_t = 1.0;	/* Inside non-tapered t-range */
			T->segment[s]->data[col][row] = w_t;
		}
	}
	return 0;
}

GMT_LOCAL int table_TCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TCDF 2 1 Student's t cumulative distribution function for t = A and nu = B.  */
{
	uint64_t s, row, nu;
	unsigned int prev;
	double t;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		nu = lrint ((S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row]);
		t  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = gmt_t_cdf (GMT, t, nu);
	}
	return 0;
}

GMT_LOCAL int table_TN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TN 2 1 Chebyshev polynomial Tn(-1<A<+1) of degree B.  */
{
	uint64_t s, row;
	unsigned int prev;
	int n;
	double a;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		n = irint ((S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row]);
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		gmt_chebyshev (GMT, a, n, &T_prev->segment[s]->data[col][row]);
	}
	return 0;
}

GMT_LOCAL int table_TPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TPDF 2 1 Student's t probability density function for t = A and nu = B.  */
{
	uint64_t s, row, nu;
	unsigned int prev;
	double t;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		nu = lrint ((S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row]);
		t  = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = gmt_t_pdf (GMT, t, nu);
	}
	return 0;
}

GMT_LOCAL int table_TCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: TCRIT 2 1 Student's t distribution critical value for alpha = A and nu = B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a, b;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for TCRIT!\n");
	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for TCRIT!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		a = (S[prev]->constant) ? S[prev]->factor : T_prev->segment[s]->data[col][row];
		b = (S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = gmt_tcrit (GMT, a, b);
	}
	return 0;
}

GMT_LOCAL int table_UPPER (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: UPPER 1 1 The highest (maximum) value of A.  */
{
	uint64_t s, row;
	double high = -DBL_MAX;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) {	/* Trivial case */
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = S[last]->factor;
		return 0;
	}

	for (s = 0; s < info->T->n_segments; s++) {
		if (info->local) high = -DBL_MAX;
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			if (gmt_M_is_dnan (T->segment[s]->data[col][row])) continue;
			if (T->segment[s]->data[col][row] > high) high = T->segment[s]->data[col][row];
		}
		if (high == -DBL_MAX) high = GMT->session.d_NaN;
		if (info->local) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = high;
	}
	if (info->local) return 0;	/* Done with local */
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = high;
	return 0;
}

GMT_LOCAL int table_VAR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: VAR 1 1 Variance of A.  */
{
	uint64_t s, row;
	double var;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && info->T->n_records < 2)	/* Trivial case: variance is undefined */
		var = GMT->session.d_NaN;
	else {	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
		uint64_t n = 0;
		double mean = 0.0, sum2 = 0.0, delta;
		for (s = 0; s < info->T->n_segments; s++) {
			if (info->local) {n = 0; mean = sum2 = 0.0;}	/* Start anew for each segment */
			for (row = 0; row < info->T->segment[s]->n_rows; row++) {
				if (gmt_M_is_dnan (T->segment[s]->data[col][row])) continue;
				n++;
				delta = T->segment[s]->data[col][row] - mean;
				mean += delta / n;
				sum2 += delta * (T->segment[s]->data[col][row] - mean);
			}
			if (info->local) {
				sum2 = (n > 1) ? sum2 / (n - 1) : GMT->session.d_NaN;
				for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = sum2;
			}
		}
		if (info->local) return 0;	/* Done with local */
		var = (n > 1) ? sum2 / (n - 1) : GMT->session.d_NaN;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = var;
	return 0;
}

GMT_LOCAL int table_VARW (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: VARW 2 1 Weighted variance of A for weights in B.  */
{
	uint64_t s, row;
	unsigned int prev;
	double var;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant && info->T->n_records < 2)	/* Trivial case: variance is undefined */
		var = GMT->session.d_NaN;
	else {	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
		uint64_t n = 0;
		double temp, mean = 0.0, sumw = 0.0, delta, R, M2 = 0.0, w;
		for (s = 0; s < info->T->n_segments; s++) {
			if (info->local) {n = 0; mean = sumw = M2 = 0.0;}	/* Start anew for each segment */
			for (row = 0; row < info->T->segment[s]->n_rows; row++) {
				if (gmt_M_is_dnan (T_prev->segment[s]->data[col][row])) continue;
				if (S[last]->constant)
					w = S[last]->factor;
				else if (gmt_M_is_dnan (T->segment[s]->data[col][row]))
					continue;
				else
					w = T->segment[s]->data[col][row];
				temp = w + sumw;
				delta = T_prev->segment[s]->data[col][row] - mean;
				R = delta * w / temp;
				mean += R;
				M2 += sumw * delta * R;
				sumw = temp;
				n++;
			}
			if (info->local) {
				var = (n > 1) ? (n * M2 / sumw) / (n - 1.0) : GMT->session.d_NaN;
				for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = var;
			}
		}
		if (info->local) return 0;	/* Done with local */
		var = (n > 1) ? (n * M2 / sumw) / (n - 1.0) : GMT->session.d_NaN;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev->segment[s]->data[col][row] = var;
	return 0;
}

GMT_LOCAL int table_WCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: WCDF 3 1 Weibull cumulative distribution function for x = A, scale = B, and shape = C.  */
{
	uint64_t s, row;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	double x, a, b;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev1]->constant && S[prev1]->factor <= 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two <=0 for WCDF!\n");
	if (S[last]->constant && S[last]->factor <= 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand three <= 0 for WCDF!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		x = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->data[col][row];
		a = (double)((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->data[col][row]);
		b = (double)((S[last]->constant) ? S[last]->factor : T->segment[s]->data[col][row]);
		T_prev2->segment[s]->data[col][row] = gmt_weibull_cdf (GMT, x, a, b);
	}
	return 0;
}

GMT_LOCAL int table_WCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: WCRIT 3 1 Weibull distribution critical value for alpha = A, scale = B, and shape = C.  */
{
	uint64_t s, row;
	unsigned int prev1 = last - 1, prev2 = last - 2;
	double alpha, a, b;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev1]->constant && S[prev1]->factor <= 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two <=0 for WCRIT!\n");
	if (S[last]->constant && S[last]->factor <= 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand three <= 0 for WCRIT!\n");
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		alpha = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->data[col][row];
		a = (double)((S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->data[col][row]);
		b = (double)((S[last]->constant)  ? S[last]->factor  : T->segment[s]->data[col][row]);
		T_prev2->segment[s]->data[col][row] = gmt_weibull_crit (GMT, alpha, a, b);
	}
	return 0;
}

GMT_LOCAL int table_WPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: WPDF 3 1 Weibull probability density function for x = A, scale = B and shape = C.  */
{
	unsigned int prev1 = last - 1, prev2 = last - 2;
	uint64_t s, row;
	double x, a, b;
	struct GMT_DATATABLE *T = (S[last]->constant) ? NULL : S[last]->D->table[0], *T_prev1 = (S[prev1]->constant) ? NULL : S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];

	if (S[prev2]->constant && S[prev2]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument x to WPDF must be x >= 0!\n");
		return -1;
	}
	if (S[prev1]->constant && S[prev1]->factor <= 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument a to WPDF must be a positive (a > 0)!\n");
		return -1;
	}
	if (S[last]->constant && S[last]->factor <= 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument b to WPDF must be a positive (b > 0)!\n");
		return -1;
	}
	if (S[prev2]->constant && S[prev1]->constant && S[last]->constant) {	/* WPDF is given constant arguments */
		double value;
		x = S[prev2]->factor;
		a = S[prev1]->factor;	b = S[last]->factor;
		value = gmt_weibull_pdf (GMT, x, a, b);
		for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T_prev2->segment[s]->data[col][row] = value;
		return 0;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		x = (S[prev2]->constant) ? S[prev2]->factor : T_prev2->segment[s]->data[col][row];
		a = (S[prev1]->constant) ? S[prev1]->factor : T_prev1->segment[s]->data[col][row];
		b = (S[last]->constant)  ? S[last]->factor : T->segment[s]->data[col][row];
		T_prev2->segment[s]->data[col][row] = gmt_weibull_pdf (GMT, x, a, b);
	}
	return 0;
}

GMT_LOCAL int table_XOR (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: XOR 2 1 B if A == NaN, else A.  */
{
	uint64_t s, row;
	unsigned int prev;
	double a = 0.0, b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	gmt_M_unused(GMT);

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[prev]->constant) a = S[prev]->factor;
	if (S[last]->constant) b = S[last]->factor;
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (!S[prev]->constant) a = T_prev->segment[s]->data[col][row];
		if (!S[last]->constant) b = T->segment[s]->data[col][row];
		T_prev->segment[s]->data[col][row] = (gmt_M_is_dnan (a)) ? b : a;
	}
	return 0;
}

GMT_LOCAL int table_XYZ2HSV (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: XYZ2HSV 3 3 Convert XYZ to HSV, with x = A, y = B and z = C.  */
	uint64_t s = 0, row;
	double rgb[4], hsv[4], xyz[3];
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused (GMT);

	if (info->scalar) {	/* Three stacks given since three separate values on the stack */
		unsigned int prev1 = last - 1, prev2 = last - 2;
		struct GMT_DATATABLE *T_prev1 = S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
#if 0
		if (S[prev2]->factor < 0.0 || S[prev2]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to XYZ2HSV must be a 0 <= r <= 255!\n");
			return -1;
		}
		if (S[prev1]->factor < 0.0 || S[prev1]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument g to XYZ2HSV must be a 0 <= g <= 255!\n");
			return -1;
		}
		if (S[last]->factor < 0.0 || S[last]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument b to XYZ2HSV must be a 0 <= b <= 255!\n");
			return -1;
		}
#endif
		xyz[0] = S[prev2]->factor;
		xyz[1] = S[prev1]->factor;
		xyz[2] = S[last]->factor;
		gmt_xyz_to_rgb (rgb, xyz);
		gmt_rgb_to_hsv (rgb, hsv);
		/* Only a single segment with one row */
		T_prev2->segment[0]->data[col][0] = hsv[0];
		T_prev1->segment[0]->data[col][0] = hsv[1];
		T->segment[0]->data[col][0]       = hsv[2];
		return 0;
	}
	/* Operate across rows. col must be 2 */
	if (col != 2) return 0;	/* Do nothing */
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			xyz[0] = T->segment[s]->data[0][row];
			xyz[1] = T->segment[s]->data[1][row];
			xyz[2] = T->segment[s]->data[2][row];
			gmt_xyz_to_rgb (rgb, xyz);
			gmt_rgb_to_hsv (rgb, hsv);
			T->segment[s]->data[0][row] = hsv[0];
			T->segment[s]->data[1][row] = hsv[1];
			T->segment[s]->data[2][row] = hsv[2];
		}
	}
	return 0;
}

GMT_LOCAL int table_XYZ2LAB (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: XYZ2LAB 3 3 Convert XYZ to LAB, with x = A, y = B and z = C.  */
	uint64_t s = 0, row;
	double lab[3], xyz[3];
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused (GMT);

	if (info->scalar) {	/* Three stacks given since three separate values on the stack */
		unsigned int prev1 = last - 1, prev2 = last - 2;
		struct GMT_DATATABLE *T_prev1 = S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
#if 0
		if (S[prev2]->factor < 0.0 || S[prev2]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to XYZ2LAB must be a 0 <= r <= 255!\n");
			return -1;
		}
		if (S[prev1]->factor < 0.0 || S[prev1]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument g to XYZ2LAB must be a 0 <= g <= 255!\n");
			return -1;
		}
		if (S[last]->factor < 0.0 || S[last]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument b to XYZ2LAB must be a 0 <= b <= 255!\n");
			return -1;
		}
#endif
		xyz[0] = S[prev2]->factor;
		xyz[1] = S[prev1]->factor;
		xyz[2] = S[last]->factor;
		gmt_xyz_to_lab (xyz, lab);
		/* Only a single segment with one row */
		T_prev2->segment[0]->data[col][0] = lab[0];
		T_prev1->segment[0]->data[col][0] = lab[1];
		T->segment[0]->data[col][0]       = lab[2];
		return 0;
	}
	/* Operate across rows. col must be 2 */
	if (col != 2) return 0;	/* Do nothing */
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			xyz[0] = T->segment[s]->data[0][row];
			xyz[1] = T->segment[s]->data[1][row];
			xyz[2] = T->segment[s]->data[2][row];
			gmt_xyz_to_lab (xyz, lab);
			T->segment[s]->data[0][row] = lab[0];
			T->segment[s]->data[1][row] = lab[1];
			T->segment[s]->data[2][row] = lab[2];
		}
	}
	return 0;
}

GMT_LOCAL int table_XYZ2RGB (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col) {
/*OPERATOR: XYZ2RGB 3 3 Convert XYZ to RGB, with x = A, y = B and z = C.  */
	uint64_t s = 0, row;
	double rgb[3], xyz[3];
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused (GMT);

	if (info->scalar) {	/* Three stacks given since three separate values on the stack */
		unsigned int prev1 = last - 1, prev2 = last - 2;
		struct GMT_DATATABLE *T_prev1 = S[prev1]->D->table[0], *T_prev2 = S[prev2]->D->table[0];
#if 0
		if (S[prev2]->factor < 0.0 || S[prev2]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to XYZ2RGB must be a 0 <= r <= 255!\n");
			return -1;
		}
		if (S[prev1]->factor < 0.0 || S[prev1]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument g to XYZ2RGB must be a 0 <= g <= 255!\n");
			return -1;
		}
		if (S[last]->factor < 0.0 || S[last]->factor > 255.0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument b to XYZ2RGB must be a 0 <= b <= 255!\n");
			return -1;
		}
#endif
		xyz[0] = S[prev2]->factor;
		xyz[1] = S[prev1]->factor;
		xyz[2] = S[last]->factor;
		gmt_xyz_to_rgb (rgb, xyz);
		/* Only a single segment with one row */
		T_prev2->segment[0]->data[col][0] = gmt_M_s255 (rgb[0]);
		T_prev1->segment[0]->data[col][0] = gmt_M_s255 (rgb[1]);
		T->segment[0]->data[col][0]       = gmt_M_s255 (rgb[2]);
		return 0;
	}
	/* Operate across rows. col must be 2 */
	if (col != 2) return 0;	/* Do nothing */
	for (s = 0; s < info->T->n_segments; s++) {
		for (row = 0; row < info->T->segment[s]->n_rows; row++) {
			xyz[0] = T->segment[s]->data[0][row];
			xyz[1] = T->segment[s]->data[1][row];
			xyz[2] = T->segment[s]->data[2][row];
			gmt_xyz_to_rgb (rgb, xyz);
			T->segment[s]->data[0][row] = gmt_M_s255 (rgb[0]);
			T->segment[s]->data[1][row] = gmt_M_s255 (rgb[1]);
			T->segment[s]->data[2][row] = gmt_M_s255 (rgb[2]);
		}
	}
	return 0;
}

GMT_LOCAL int table_Y0 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: Y0 1 1 Bessel function of A (2nd kind, order 0).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand = 0 for Y0!\n");
	if (S[last]->constant) a = y0 (fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : y0 (fabs (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_Y1 (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: Y1 1 1 Bessel function of A (2nd kind, order 1).  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant && S[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand = 0 for Y1!\n");
	if (S[last]->constant) a = y1 (fabs (S[last]->factor));
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : y1 (fabs (T->segment[s]->data[col][row]));
	return 0;
}

GMT_LOCAL int table_YN (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: YN 2 1 Bessel function of A (2nd kind, order B).  */
{
	uint64_t s, row;
	unsigned int prev, order = 0;
	bool simple = false;
	double b = 0.0;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;

	if ((prev = gmt_assign_ptrs (GMT, last, S, &T, &T_prev)) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	if (S[last]->constant && S[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order < 0 for YN!\n");
	if (S[last]->constant && fabs (rint(S[last]->factor) - S[last]->factor) > GMT_CONV4_LIMIT) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order not an integer for YN!\n");
	if (S[prev]->constant && S[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "argument = 0 for YN!\n");
	if (S[last]->constant) order = urint (fabs (S[last]->factor));
	if (S[last]->constant && S[prev]->constant) {
		b = yn ((int)order, fabs (S[prev]->factor));
		simple = true;
	}
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) {
		if (simple)
			T_prev->segment[s]->data[col][row] = b;
		else {
			if (!S[last]->constant) order = urint (fabs (T->segment[s]->data[col][row]));
			T_prev->segment[s]->data[col][row] = yn ((int)order, fabs (T_prev->segment[s]->data[col][row]));
		}
	}
	return 0;
}

GMT_LOCAL int table_ZCRIT (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ZCRIT 1 1 Normal distribution critical value for alpha = A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_zcrit (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_zcrit (GMT, T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ZCDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ZCDF 1 1 Normal cumulative distribution function for z = A.  */
{
	uint64_t s, row;
	double a = 0.0;
	struct GMT_DATATABLE *T = S[last]->D->table[0];

	if (S[last]->constant) a = gmt_zdist (GMT, S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? a : gmt_zdist (GMT, T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ZPDF (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ZPDF 1 1 Normal probability density function for z = A.  */
{
	uint64_t s, row;
	double z = 0.0, f = 1.0 / sqrt (TWO_PI);
	struct GMT_DATATABLE *T = S[last]->D->table[0];
	gmt_M_unused(GMT);

	if (S[last]->constant) z = f * exp (-0.5 * S[last]->factor * S[last]->factor);
	for (s = 0; s < info->T->n_segments; s++) for (row = 0; row < info->T->segment[s]->n_rows; row++) T->segment[s]->data[col][row] = (S[last]->constant) ? z : f * exp (-0.5 * T->segment[s]->data[col][row] * T->segment[s]->data[col][row]);
	return 0;
}

GMT_LOCAL int table_ROOTS (struct GMT_CTRL *GMT, struct GMTMATH_INFO *info, struct GMTMATH_STACK *S[], unsigned int last, unsigned int col)
/*OPERATOR: ROOTS 2 1 Treats col A as f(t) = 0 and returns its roots.  */
{
	uint64_t seg, row;
	unsigned int i;
	int s_arg;
	double *roots = NULL;
	struct GMT_DATATABLE *T = NULL, *T_prev = NULL;
	gmt_M_unused(col);

	if (gmt_assign_ptrs (GMT, last, S, &T, &T_prev) == UINT_MAX) return -1;	/* Set up pointers and prev; exit if running out of stack */

	/* Treats the chosen column (at there is only one) as f(t) and solves for t that makes f(t) == 0.
	 * For now we only solve using a linear spline but in the future this should depend on the users
	 * choice of INTERPOLANT. */

	if (info->roots_found) return 0;	/* Already been here */
	if (!S[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument to operator ROOTS must be a constant: the column number. Reset to 0\n");
		s_arg = 0;
	}
	else
		s_arg = irint (S[last]->factor);
	if (s_arg < 0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument to operator ROOTS must be a column number 0 < col < %d. Reset to 0\n", info->n_col);
		s_arg = 0;
	}
	info->r_col = s_arg;
	if (info->r_col >= info->n_col) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument to operator ROOTS must be a column number 0 < col < %d. Reset to 0\n", info->n_col);
		info->r_col = 0;
	}
	roots = gmt_M_memory (GMT, NULL, T->n_records, double);
	info->n_roots = 0;
	if (T_prev->segment[0]->data[info->r_col][0] == 0.0) roots[info->n_roots++] = info->T->segment[0]->data[COL_T][0];
	for (seg = 0; seg < info->T->n_segments; seg++) {
		for (row = 1; row < info->T->segment[seg]->n_rows; row++) {
			if (T_prev->segment[seg]->data[info->r_col][row] == 0.0) {
				roots[info->n_roots++] = info->T->segment[seg]->data[COL_T][row];
				continue;
			}

			if ((T_prev->segment[seg]->data[info->r_col][row-1] * T_prev->segment[seg]->data[info->r_col][row]) < 0.0) {	/* Crossing 0 */
				roots[info->n_roots] = info->T->segment[seg]->data[COL_T][row-1] - T_prev->segment[seg]->data[info->r_col][row-1] * (info->T->segment[seg]->data[COL_T][row] - info->T->segment[seg]->data[COL_T][row-1]) / (T_prev->segment[seg]->data[info->r_col][row] - T_prev->segment[seg]->data[info->r_col][row-1]);
				info->n_roots++;
			}
		}
	}
	for (i = 0; i < info->n_roots; i++) T_prev->segment[0]->data[info->r_col][i] = roots[i];
	gmt_M_free (GMT, roots);
	info->roots_found = true;
	return 0;
}

/* ---------------------- end operator functions --------------------- */

#define GMTMATH_N_OPERATORS 197

GMT_LOCAL void gmtmath_init (int (*ops[])(struct GMT_CTRL *, struct GMTMATH_INFO *, struct GMTMATH_STACK **S, unsigned int, unsigned int), unsigned int n_args[], unsigned int n_out[])
{
	/* Operator function	# of operands	# of outputs */

	ops[0] = table_ABS;	n_args[0] = 1;	n_out[0] = 1;
	ops[1] = table_ACOS;	n_args[1] = 1;	n_out[1] = 1;
	ops[2] = table_ACOSH;	n_args[2] = 1;	n_out[2] = 1;
	ops[3] = table_ACOT;	n_args[3] = 1;	n_out[3] = 1;
	ops[4] = table_ACOTH;	n_args[4] = 1;	n_out[4] = 1;
	ops[5] = table_ACSC;	n_args[5] = 1;	n_out[5] = 1;
	ops[6] = table_ACSCH;	n_args[6] = 1;	n_out[6] = 1;
	ops[7] = table_ADD;	n_args[7] = 2;	n_out[7] = 1;
	ops[8] = table_AND;	n_args[8] = 2;	n_out[8] = 1;
	ops[9] = table_ASEC;	n_args[9] = 1;	n_out[9] = 1;
	ops[10] = table_ASECH;	n_args[10] = 1;	n_out[10] = 1;
	ops[11] = table_ASIN;	n_args[11] = 1;	n_out[11] = 1;
	ops[12] = table_ASINH;	n_args[12] = 1;	n_out[12] = 1;
	ops[13] = table_ATAN;	n_args[13] = 1;	n_out[13] = 1;
	ops[14] = table_ATAN2;	n_args[14] = 2;	n_out[14] = 1;
	ops[15] = table_ATANH;	n_args[15] = 1;	n_out[15] = 1;
	ops[16] = table_BCDF;	n_args[16] = 3;	n_out[16] = 1;
	ops[17] = table_BEI;	n_args[17] = 1;	n_out[17] = 1;
	ops[18] = table_BER;	n_args[18] = 1;	n_out[18] = 1;
	ops[19] = table_BPDF;	n_args[19] = 3;	n_out[19] = 1;
	ops[20] = table_BITAND;	n_args[20] = 2;	n_out[20] = 1;
	ops[21] = table_BITLEFT;	n_args[21] = 2;	n_out[21] = 1;
	ops[22] = table_BITNOT;	n_args[22] = 1;	n_out[22] = 1;
	ops[23] = table_BITOR;	n_args[23] = 2;	n_out[23] = 1;
	ops[24] = table_BITRIGHT;	n_args[24] = 2;	n_out[24] = 1;
	ops[25] = table_BITTEST;	n_args[25] = 2;	n_out[25] = 1;
	ops[26] = table_BITXOR;	n_args[26] = 2;	n_out[26] = 1;
	ops[27] = table_CEIL;	n_args[27] = 1;	n_out[27] = 1;
	ops[28] = table_CHI2CRIT;	n_args[28] = 2;	n_out[28] = 1;
	ops[29] = table_CHI2CDF;	n_args[29] = 2;	n_out[29] = 1;
	ops[30] = table_CHI2PDF;	n_args[30] = 2;	n_out[30] = 1;
	ops[31] = table_COL;	n_args[31] = 1;	n_out[31] = 1;
	ops[32] = table_COMB;	n_args[32] = 2;	n_out[32] = 1;
	ops[33] = table_CORRCOEFF;	n_args[33] = 2;	n_out[33] = 1;
	ops[34] = table_COS;	n_args[34] = 1;	n_out[34] = 1;
	ops[35] = table_COSD;	n_args[35] = 1;	n_out[35] = 1;
	ops[36] = table_COSH;	n_args[36] = 1;	n_out[36] = 1;
	ops[37] = table_COT;	n_args[37] = 1;	n_out[37] = 1;
	ops[38] = table_COTD;	n_args[38] = 1;	n_out[38] = 1;
	ops[39] = table_COTH;	n_args[39] = 1;	n_out[39] = 1;
	ops[40] = table_CSC;	n_args[40] = 1;	n_out[40] = 1;
	ops[41] = table_CSCD;	n_args[41] = 1;	n_out[41] = 1;
	ops[42] = table_CSCH;	n_args[42] = 1;	n_out[42] = 1;
	ops[43] = table_PCDF;	n_args[43] = 2;	n_out[43] = 1;
	ops[44] = table_DDT;	n_args[44] = 1;	n_out[44] = 1;
	ops[45] = table_D2DT2;	n_args[45] = 1;	n_out[45] = 1;
	ops[46] = table_D2R;	n_args[46] = 1;	n_out[46] = 1;
	ops[47] = table_DENAN;	n_args[47] = 2;	n_out[47] = 1;
	ops[48] = table_DILOG;	n_args[48] = 1;	n_out[48] = 1;
	ops[49] = table_DIFF;	n_args[49] = 1;	n_out[49] = 1;
	ops[50] = table_DIV;	n_args[50] = 2;	n_out[50] = 1;
	ops[51] = table_DUP;	n_args[51] = 1;	n_out[51] = 2;
	ops[52] = table_ECDF;	n_args[52] = 2;	n_out[52] = 1;
	ops[53] = table_ECRIT;	n_args[53] = 2;	n_out[53] = 1;
	ops[54] = table_EPDF;	n_args[54] = 2;	n_out[54] = 1;
	ops[55] = table_ERF;	n_args[55] = 1;	n_out[55] = 1;
	ops[56] = table_ERFC;	n_args[56] = 1;	n_out[56] = 1;
	ops[57] = table_ERFINV;	n_args[57] = 1;	n_out[57] = 1;
	ops[58] = table_EQ;	n_args[58] = 2;	n_out[58] = 1;
	ops[59] = table_EXCH;	n_args[59] = 2;	n_out[59] = 2;
	ops[60] = table_EXP;	n_args[60] = 1;	n_out[60] = 1;
	ops[61] = table_FACT;	n_args[61] = 1;	n_out[61] = 1;
	ops[62] = table_FCRIT;	n_args[62] = 3;	n_out[62] = 1;
	ops[63] = table_FCDF;	n_args[63] = 3;	n_out[63] = 1;
	ops[64] = table_FLIPUD;	n_args[64] = 1;	n_out[64] = 1;
	ops[65] = table_FLOOR;	n_args[65] = 1;	n_out[65] = 1;
	ops[66] = table_FMOD;	n_args[66] = 2;	n_out[66] = 1;
	ops[67] = table_FPDF;	n_args[67] = 3;	n_out[67] = 1;
	ops[68] = table_GE;	n_args[68] = 2;	n_out[68] = 1;
	ops[69] = table_GT;	n_args[69] = 2;	n_out[69] = 1;
	ops[70] = table_HYPOT;	n_args[70] = 2;	n_out[70] = 1;
	ops[71] = table_I0;	n_args[71] = 1;	n_out[71] = 1;
	ops[72] = table_I1;	n_args[72] = 1;	n_out[72] = 1;
	ops[73] = table_IFELSE;	n_args[73] = 3;	n_out[73] = 1;
	ops[74] = table_IN;	n_args[74] = 2;	n_out[74] = 1;
	ops[75] = table_INRANGE;	n_args[75] = 3;	n_out[75] = 1;
	ops[76] = table_INT;	n_args[76] = 1;	n_out[76] = 1;
	ops[77] = table_INV;	n_args[77] = 1;	n_out[77] = 1;
	ops[78] = table_ISFINITE;	n_args[78] = 1;	n_out[78] = 1;
	ops[79] = table_ISNAN;	n_args[79] = 1;	n_out[79] = 1;
	ops[80] = table_J0;	n_args[80] = 1;	n_out[80] = 1;
	ops[81] = table_J1;	n_args[81] = 1;	n_out[81] = 1;
	ops[82] = table_JN;	n_args[82] = 2;	n_out[82] = 1;
	ops[83] = table_K0;	n_args[83] = 1;	n_out[83] = 1;
	ops[84] = table_K1;	n_args[84] = 1;	n_out[84] = 1;
	ops[85] = table_KN;	n_args[85] = 2;	n_out[85] = 1;
	ops[86] = table_KEI;	n_args[86] = 1;	n_out[86] = 1;
	ops[87] = table_KER;	n_args[87] = 1;	n_out[87] = 1;
	ops[88] = table_KURT;	n_args[88] = 1;	n_out[88] = 1;
	ops[89] = table_LCDF;	n_args[89] = 1;	n_out[89] = 1;
	ops[90] = table_LCRIT;	n_args[90] = 1;	n_out[90] = 1;
	ops[91] = table_LE;	n_args[91] = 2;	n_out[91] = 1;
	ops[92] = table_LMSSCL;	n_args[92] = 1;	n_out[92] = 1;
	ops[93] = table_LMSSCLW;	n_args[93] = 1;	n_out[93] = 1;
	ops[94] = table_LOG;	n_args[94] = 1;	n_out[94] = 1;
	ops[95] = table_LOG10;	n_args[95] = 1;	n_out[95] = 1;
	ops[96] = table_LOG1P;	n_args[96] = 1;	n_out[96] = 1;
	ops[97] = table_LOG2;	n_args[97] = 1;	n_out[97] = 1;
	ops[98] = table_LOWER;	n_args[98] = 1;	n_out[98] = 1;
	ops[99] = table_LPDF;	n_args[99] = 1;	n_out[99] = 1;
	ops[100] = table_LRAND;	n_args[100] = 2;	n_out[100] = 1;
	ops[101] = table_LSQFIT;	n_args[101] = 1;	n_out[101] = 0;
	ops[102] = table_LT;	n_args[102] = 2;	n_out[102] = 1;
	ops[103] = table_MAD;	n_args[103] = 1;	n_out[103] = 1;
	ops[104] = table_MADW;	n_args[104] = 2;	n_out[104] = 1;
	ops[105] = table_MAX;	n_args[105] = 2;	n_out[105] = 1;
	ops[106] = table_MEAN;	n_args[106] = 1;	n_out[106] = 1;
	ops[107] = table_MEANW;	n_args[107] = 2;	n_out[107] = 1;
	ops[108] = table_MEDIAN;	n_args[108] = 1;	n_out[108] = 1;
	ops[109] = table_MEDIANW;	n_args[109] = 2;	n_out[109] = 1;
	ops[110] = table_MIN;	n_args[110] = 2;	n_out[110] = 1;
	ops[111] = table_MOD;	n_args[111] = 2;	n_out[111] = 1;
	ops[112] = table_MODE;	n_args[112] = 1;	n_out[112] = 1;
	ops[113] = table_MODEW;	n_args[113] = 2;	n_out[113] = 1;
	ops[114] = table_MUL;	n_args[114] = 2;	n_out[114] = 1;
	ops[115] = table_NAN;	n_args[115] = 2;	n_out[115] = 1;
	ops[116] = table_NEG;	n_args[116] = 1;	n_out[116] = 1;
	ops[117] = table_NEQ;	n_args[117] = 2;	n_out[117] = 1;
	ops[118] = table_NORM;	n_args[118] = 1;	n_out[118] = 1;
	ops[119] = table_NOT;	n_args[119] = 1;	n_out[119] = 1;
	ops[120] = table_NRAND;	n_args[120] = 2;	n_out[120] = 1;
	ops[121] = table_OR;	n_args[121] = 2;	n_out[121] = 1;
	ops[122] = table_PERM;	n_args[122] = 2;	n_out[122] = 1;
	ops[123] = table_PLM;	n_args[123] = 3;	n_out[123] = 1;
	ops[124] = table_PLMg;	n_args[124] = 3;	n_out[124] = 1;
	ops[125] = table_POP;	n_args[125] = 1;	n_out[125] = 0;
	ops[126] = table_POW;	n_args[126] = 2;	n_out[126] = 1;
	ops[127] = table_PPDF;	n_args[127] = 2;	n_out[127] = 1;
	ops[128] = table_PQUANT;	n_args[128] = 2;	n_out[128] = 1;
	ops[129] = table_PQUANTW;	n_args[129] = 3;	n_out[129] = 1;
	ops[130] = table_PSI;	n_args[130] = 1;	n_out[130] = 1;
	ops[131] = table_PV;	n_args[131] = 3;	n_out[131] = 1;
	ops[132] = table_QV;	n_args[132] = 3;	n_out[132] = 1;
	ops[133] = table_R2;	n_args[133] = 2;	n_out[133] = 1;
	ops[134] = table_R2D;	n_args[134] = 1;	n_out[134] = 1;
	ops[135] = table_RAND;	n_args[135] = 2;	n_out[135] = 1;
	ops[136] = table_RCDF;	n_args[136] = 1;	n_out[136] = 1;
	ops[137] = table_RCRIT;	n_args[137] = 1;	n_out[137] = 1;
	ops[138] = table_RPDF;	n_args[138] = 1;	n_out[138] = 1;
	ops[139] = table_RINT;	n_args[139] = 1;	n_out[139] = 1;
	ops[140] = table_RMS;	n_args[140] = 1;	n_out[140] = 1;
	ops[141] = table_RMSW;	n_args[141] = 2;	n_out[141] = 1;
	ops[142] = table_ROLL;	n_args[142] = 2;	n_out[142] = 0;
	ops[143] = table_ROTT;	n_args[143] = 2;	n_out[143] = 1;
	ops[144] = table_SEC;	n_args[144] = 1;	n_out[144] = 1;
	ops[145] = table_SECD;	n_args[145] = 1;	n_out[145] = 1;
	ops[146] = table_SECH;	n_args[146] = 1;	n_out[146] = 1;
	ops[147] = table_SIGN;	n_args[147] = 1;	n_out[147] = 1;
	ops[148] = table_SIN;	n_args[148] = 1;	n_out[148] = 1;
	ops[149] = table_SINC;	n_args[149] = 1;	n_out[149] = 1;
	ops[150] = table_SIND;	n_args[150] = 1;	n_out[150] = 1;
	ops[151] = table_SINH;	n_args[151] = 1;	n_out[151] = 1;
	ops[152] = table_SKEW;	n_args[152] = 1;	n_out[152] = 1;
	ops[153] = table_SORT;	n_args[153] = 3;	n_out[153] = 1;
	ops[154] = table_SQR;	n_args[154] = 1;	n_out[154] = 1;
	ops[155] = table_SQRT;	n_args[155] = 1;	n_out[155] = 1;
	ops[156] = table_STD;	n_args[156] = 1;	n_out[156] = 1;
	ops[157] = table_STDW;	n_args[157] = 2;	n_out[157] = 1;
	ops[158] = table_STEP;	n_args[158] = 1;	n_out[158] = 1;
	ops[159] = table_STEPT;	n_args[159] = 1;	n_out[159] = 1;
	ops[160] = table_SUB;	n_args[160] = 2;	n_out[160] = 1;
	ops[161] = table_SUM;	n_args[161] = 1;	n_out[161] = 1;
	ops[162] = table_SVDFIT;	n_args[162] = 1;	n_out[162] = 0;
	ops[163] = table_TAN;	n_args[163] = 1;	n_out[163] = 1;
	ops[164] = table_TAND;	n_args[164] = 1;	n_out[164] = 1;
	ops[165] = table_TANH;	n_args[165] = 1;	n_out[165] = 1;
	ops[166] = table_TAPER;	n_args[166] = 1;	n_out[166] = 1;
	ops[167] = table_TCDF;	n_args[167] = 2;	n_out[167] = 1;
	ops[168] = table_TN;	n_args[168] = 2;	n_out[168] = 1;
	ops[169] = table_TPDF;	n_args[169] = 2;	n_out[169] = 1;
	ops[170] = table_TCRIT;	n_args[170] = 2;	n_out[170] = 1;
	ops[171] = table_UPPER;	n_args[171] = 1;	n_out[171] = 1;
	ops[172] = table_VAR;	n_args[172] = 1;	n_out[172] = 1;
	ops[173] = table_VARW;	n_args[173] = 2;	n_out[173] = 1;
	ops[174] = table_WCDF;	n_args[174] = 3;	n_out[174] = 1;
	ops[175] = table_WCRIT;	n_args[175] = 3;	n_out[175] = 1;
	ops[176] = table_WPDF;	n_args[176] = 3;	n_out[176] = 1;
	ops[177] = table_XOR;	n_args[177] = 2;	n_out[177] = 1;
	ops[178] = table_Y0;	n_args[178] = 1;	n_out[178] = 1;
	ops[179] = table_Y1;	n_args[179] = 1;	n_out[179] = 1;
	ops[180] = table_YN;	n_args[180] = 2;	n_out[180] = 1;
	ops[181] = table_ZCRIT;	n_args[181] = 1;	n_out[181] = 1;
	ops[182] = table_ZCDF;	n_args[182] = 1;	n_out[182] = 1;
	ops[183] = table_ZPDF;	n_args[183] = 1;	n_out[183] = 1;
	ops[184] = table_ROOTS;	n_args[184] = 2;	n_out[184] = 1;
	ops[185] = table_HSV2LAB;	n_args[185] = 3;	n_out[185] = 3;
	ops[186] = table_HSV2RGB;	n_args[186] = 3;	n_out[186] = 3;
	ops[187] = table_HSV2XYZ;	n_args[187] = 3;	n_out[187] = 3;
	ops[188] = table_LAB2HSV;	n_args[188] = 3;	n_out[188] = 3;
	ops[189] = table_LAB2RGB;	n_args[189] = 3;	n_out[189] = 3;
	ops[190] = table_LAB2XYZ;	n_args[190] = 3;	n_out[190] = 3;
	ops[191] = table_RGB2HSV;	n_args[191] = 3;	n_out[191] = 3;
	ops[192] = table_RGB2LAB;	n_args[192] = 3;	n_out[192] = 3;
	ops[193] = table_RGB2XYZ;	n_args[193] = 3;	n_out[193] = 3;
	ops[194] = table_XYZ2HSV;	n_args[194] = 3;	n_out[194] = 3;
	ops[195] = table_XYZ2LAB;	n_args[195] = 3;	n_out[195] = 3;
	ops[196] = table_XYZ2RGB;	n_args[196] = 3;	n_out[196] = 3;
}

GMT_LOCAL void Free_Stack (struct GMTAPI_CTRL *API, struct GMTMATH_STACK **stack) {
	unsigned int k, error = 0;
	bool is_object;
	struct GMT_DATASET_HIDDEN *DH = NULL;
	for (k = 0; k < GMTMATH_STACK_SIZE; k++) {
		if (stack[k]->D) {	/* Must free unless being passed back out */
			is_object = gmtlib_is_an_object (API->GMT, &stack[k]->D);
			if (is_object && (error = GMT_Destroy_Data (API, &stack[k]->D)) == GMT_NOERROR)
				GMT_Report (API, GMT_MSG_DEBUG, "GMT_Destroy_Data freed stack item %d\n", k);
			else if (!is_object || error == GMT_OBJECT_NOT_FOUND) {
				/* Failures should only be from local objects not passed up */
				DH = gmt_get_DD_hidden (stack[k]->D);
				if (gmt_this_alloc_level (API->GMT, DH->alloc_level)) {
					gmt_free_dataset (API->GMT, &stack[k]->D);
					GMT_Report (API, GMT_MSG_DEBUG, "gmt_free_dataset freed stack item %d\n", k);
				}
				else
					GMT_Report (API, GMT_MSG_DEBUG, "No freeing, returning stack item %d\n", k);
			}
			else
				GMT_Report (API, GMT_MSG_ERROR, "Failed to free stack item %d, error = %d\n", k, error);
		}
		gmt_M_free (API->GMT, stack[k]);
	}
}

GMT_LOCAL void Free_Store (struct GMTAPI_CTRL *API, struct GMTMATH_STORED **recall) {
	unsigned int k;
	for (k = 0; k < GMTMATH_STORE_SIZE; k++) {
		if (recall[k] && !recall[k]->stored.constant) {
			gmt_free_dataset (API->GMT, &recall[k]->stored.D);
			gmt_M_str_free (recall[k]->label);
			gmt_M_free (API->GMT, recall[k]);
		}
	}
}

#define Free_Misc {if (T_in) GMT_Destroy_Data (API, &T_in); GMT_Destroy_Data (API, &Template); GMT_Destroy_Data (API, &Time); if (read_stdin) GMT_Destroy_Data (API, &D_stdin); }
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return1(code) {GMT_Destroy_Options (API, &list); Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code); }
#define Return(code) {GMT_Destroy_Options (API, &list); Free_Ctrl (GMT, Ctrl); Free_Stack(API,stack); Free_Store(API,recall); Free_Misc;  gmt_end_module (GMT, GMT_cpy); bailout (code); }

GMT_LOCAL int decode_gmt_argument (struct GMT_CTRL *GMT, char *txt, double *value, bool *dimension, struct GMT_HASH *H) {
	unsigned int expect;
	int key;
	size_t last;
	bool check = GMT_IS_NAN, possible_number = false;
	char copy[GMT_LEN256] = {""};
	char *mark = NULL;
	double tmp = 0.0;

	if (!txt) return (GMTMATH_ARG_IS_BAD);

	if (gmt_M_file_is_memory (txt)) return GMTMATH_ARG_IS_FILE;	/* Deal with memory references first */
	if (gmt_M_file_is_cache (txt)) return GMTMATH_ARG_IS_FILE;	/* Deal with cache file first */

	/* Check if argument is operator */

	if ((key = gmt_hash_lookup (GMT, txt, H, GMTMATH_N_OPERATORS, GMTMATH_N_OPERATORS)) >= GMTMATH_ARG_IS_OPERATOR) return (key);

	/* Next look for symbols with special meaning */

	if (!strcmp (txt, "STDIN")) return GMTMATH_ARG_IS_FILE;	/* read from stdin */
	if (!strncmp (txt, GMTMATH_STORE_CMD, strlen(GMTMATH_STORE_CMD))) return GMTMATH_ARG_IS_STORE;		/* store into mem location @<label>*/
	if (!strncmp (txt, GMTMATH_CLEAR_CMD, strlen(GMTMATH_CLEAR_CMD))) return GMTMATH_ARG_IS_CLEAR;		/* free mem location @<label>*/
	if (!strncmp (txt, GMTMATH_RECALL_CMD, strlen(GMTMATH_RECALL_CMD))) return GMTMATH_ARG_IS_RECALL;	/* load from mem location @<label>*/
	if (!(strcmp (txt, "PI") && strcmp (txt, "pi"))) return GMTMATH_ARG_IS_PI;
	if (!(strcmp (txt, "E") && strcmp (txt, "e"))) return GMTMATH_ARG_IS_E;
	if (!strcmp (txt, "F_EPS")) return GMTMATH_ARG_IS_F_EPS;
	if (!strcmp (txt, "D_EPS")) return GMTMATH_ARG_IS_D_EPS;
	if (!strcmp (txt, "EULER")) return GMTMATH_ARG_IS_EULER;
	if (!strcmp (txt, "PHI")) return GMTMATH_ARG_IS_PHI;
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

	strncpy (copy, txt, GMT_LEN256-1);
	if (!gmt_not_numeric (GMT, copy)) {	/* Only check if we are not sure this is NOT a number */
		expect = (strchr (copy, 'T')) ? GMT_IS_ABSTIME : GMT_IS_UNKNOWN;	/* Watch out for dateTclock-strings */
		check = gmt_scanf (GMT, copy, expect, &tmp);
		possible_number = true;
	}

	/* Determine if argument is file. Remove possible question mark. */

	mark = strchr (copy, '?');
	if (mark) *mark = '\0';
	if (!gmt_access (GMT, copy, R_OK)) {	/* Yes it is a file */
		if (check != GMT_IS_NAN && possible_number) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Your argument %s is both a file and a number.  File is selected\n", txt);
		return GMTMATH_ARG_IS_FILE;
	}

	/* Check if it is a dimension (which would fail the gmt_not_numeric check above) */

	last = strlen (txt) - 1;	/* Position of last character in string */
	if (strchr (GMT_DIM_UNITS, txt[last])) {	/* Yes, ends in c, i, or p */
		*value = gmt_M_to_inch (GMT, txt);
		*dimension = true;
		return GMTMATH_ARG_IS_NUMBER;
	}

	if (check != GMT_IS_NAN) {	/* OK it is a number */
		*value = tmp;
		return GMTMATH_ARG_IS_NUMBER;
	}

	if (txt[0] == '-') {	/* Probably a bad commandline option */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option %s not recognized\n", txt);
		return (GMTMATH_ARG_IS_BAD);
	}

	GMT_Report (GMT->parent, GMT_MSG_ERROR, "%s is not a number, operator or file name\n", txt);
	return (GMTMATH_ARG_IS_BAD);	/* Dummy return to satisfy some compilers */
}

GMT_LOCAL char *gmtmath_setlabel (struct GMT_CTRL *GMT, char *arg) {
	char *label = strchr (arg, '@') + 1;	/* Label that follows @ */
	if (!label || label[0] == '\0') {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "No label appended to STO|RCL|CLR operator!\n");
		return (NULL);
	}
	return (label);
}

GMT_LOCAL void gmtmath_backwards_fixing (struct GMT_CTRL *GMT, char **arg)
{	/* Handle backwards compatible operator names */
	char *t = NULL, old[GMT_LEN16] = {""};
	if (!gmt_M_compat_check (GMT, 6)) return;	/* No checking so we may fail later */
	if (!strcmp (*arg, "CHIDIST"))      {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("CHI2CDF");  }
	else if (!strcmp (*arg, "CHICRIT")) {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("CHI2CRIT"); }
	else if (!strcmp (*arg, "CPOISS"))  {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("PCDF");     }
	else if (!strcmp (*arg, "FDIST"))   {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("FCDF");     }
	else if (!strcmp (*arg, "MED"))     {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("MEDIAN");   }
	else if (!strcmp (*arg, "TDIST"))   {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("TCDF");     }
	else if (!strcmp (*arg, "Tn"))      {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("TNORM");    }
	else if (!strcmp (*arg, "ZDIST"))   {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("ZCDF");     }

	if (t)
		GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Operator %s is deprecated; use %s instead.\n", old, t);
}

GMT_LOCAL void gmtmath_expand_recall_cmd (struct GMT_OPTION *list) {
	/* If users doing STO, RCL, CLR on memory items then the shorthand @item needs to
	 * be expanded to the full syntax RCL@item, otherwise it interferes with remote
	 * cache files. */
	struct GMT_OPTION *opt = NULL, *opt2 = NULL;
	char target[GMT_LEN64] = {""};

	for (opt = list; opt; opt = opt->next) {
		if (opt->option == GMT_OPT_INFILE && !strncmp (opt->arg, "STO@", 4U)) {	/* Found a STO@item */
			for (opt2 = opt->next; opt2; opt2 = opt2->next) {	/* Loop over all remaining options */
				if (!strcmp (opt2->arg, &opt->arg[3])) {	/* Found an implicit recall item, expand to full syntax */
					sprintf (target, "RCL%s", opt2->arg);
					gmt_M_str_free (opt2->arg);	/* Remove the old shorthand */
					opt2->arg = strdup (target);
				}
			}
		}
	}
}

bool color_operator_on_table (bool scalar, char *arg) {
	if (scalar) return false;
	if (strlen (arg) < 7) return false;
	if (arg[3] != '2') return false;
	if (!strcmp (arg, "HSV2LAB")) return true;
	if (!strcmp (arg, "HSV2RGB")) return true;
	if (!strcmp (arg, "HSV2XYZ")) return true;
	if (!strcmp (arg, "LAB2HSV")) return true;
	if (!strcmp (arg, "LAB2RGB")) return true;
	if (!strcmp (arg, "LAB2XYZ")) return true;
	if (!strcmp (arg, "RGB2HSV")) return true;
	if (!strcmp (arg, "RGB2LAB")) return true;
	if (!strcmp (arg, "RGB2XYZ")) return true;
	if (!strcmp (arg, "XYZ2HSV")) return true;
	if (!strcmp (arg, "XYZ2LAB")) return true;
	if (!strcmp (arg, "XYZ2RGB")) return true;
	return false;
}

int GMT_gmtmath (void *V_API, int mode, void *args) {
	int i, k, op = 0, status = 0, last;
	unsigned int consumed_operands[GMTMATH_N_OPERATORS], produced_operands[GMTMATH_N_OPERATORS], new_stack = INT_MAX;
	unsigned int j, nstack = 0, n_stored = 0, kk, eaten, created;
	bool error = false, set_equidistant_t = false, got_t_from_file = false, free_time = false, dimension = false;
	bool read_stdin = false, t_check_required = true, touched_t_col = false, done, no_C = true;
	uint64_t use_t_col = 0, row, n_records, n_rows = 0, n_columns = 0, seg;

	uint64_t dim[GMT_DIM_SIZE] = {1, 1, 0, 0};

	double t_noise = 0.0, value, off, scale, special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_N+1];

	char *label = NULL;

	/* Declare operator array */

	static char *operator[GMTMATH_N_OPERATORS + 1] = {

		"ABS",	/* id = 0 */
		"ACOS",	/* id = 1 */
		"ACOSH",	/* id = 2 */
		"ACOT",	/* id = 3 */
		"ACOTH",	/* id = 4 */
		"ACSC",	/* id = 5 */
		"ACSCH",	/* id = 6 */
		"ADD",	/* id = 7 */
		"AND",	/* id = 8 */
		"ASEC",	/* id = 9 */
		"ASECH",	/* id = 10 */
		"ASIN",	/* id = 11 */
		"ASINH",	/* id = 12 */
		"ATAN",	/* id = 13 */
		"ATAN2",	/* id = 14 */
		"ATANH",	/* id = 15 */
		"BCDF",	/* id = 16 */
		"BEI",	/* id = 17 */
		"BER",	/* id = 18 */
		"BPDF",	/* id = 19 */
		"BITAND",	/* id = 20 */
		"BITLEFT",	/* id = 21 */
		"BITNOT",	/* id = 22 */
		"BITOR",	/* id = 23 */
		"BITRIGHT",	/* id = 24 */
		"BITTEST",	/* id = 25 */
		"BITXOR",	/* id = 26 */
		"CEIL",	/* id = 27 */
		"CHI2CRIT",	/* id = 28 */
		"CHI2CDF",	/* id = 29 */
		"CHI2PDF",	/* id = 30 */
		"COL",	/* id = 31 */
		"COMB",	/* id = 32 */
		"CORRCOEFF",	/* id = 33 */
		"COS",	/* id = 34 */
		"COSD",	/* id = 35 */
		"COSH",	/* id = 36 */
		"COT",	/* id = 37 */
		"COTD",	/* id = 38 */
		"COTH",	/* id = 39 */
		"CSC",	/* id = 40 */
		"CSCD",	/* id = 41 */
		"CSCH",	/* id = 42 */
		"PCDF",	/* id = 43 */
		"DDT",	/* id = 44 */
		"D2DT2",	/* id = 45 */
		"D2R",	/* id = 46 */
		"DENAN",	/* id = 47 */
		"DILOG",	/* id = 48 */
		"DIFF",	/* id = 49 */
		"DIV",	/* id = 50 */
		"DUP",	/* id = 51 */
		"ECDF",	/* id = 52 */
		"ECRIT",	/* id = 53 */
		"EPDF",	/* id = 54 */
		"ERF",	/* id = 55 */
		"ERFC",	/* id = 56 */
		"ERFINV",	/* id = 57 */
		"EQ",	/* id = 58 */
		"EXCH",	/* id = 59 */
		"EXP",	/* id = 60 */
		"FACT",	/* id = 61 */
		"FCRIT",	/* id = 62 */
		"FCDF",	/* id = 63 */
		"FLIPUD",	/* id = 64 */
		"FLOOR",	/* id = 65 */
		"FMOD",	/* id = 66 */
		"FPDF",	/* id = 67 */
		"GE",	/* id = 68 */
		"GT",	/* id = 69 */
		"HYPOT",	/* id = 70 */
		"I0",	/* id = 71 */
		"I1",	/* id = 72 */
		"IFELSE",	/* id = 73 */
		"IN",	/* id = 74 */
		"INRANGE",	/* id = 75 */
		"INT",	/* id = 76 */
		"INV",	/* id = 77 */
		"ISFINITE",	/* id = 78 */
		"ISNAN",	/* id = 79 */
		"J0",	/* id = 80 */
		"J1",	/* id = 81 */
		"JN",	/* id = 82 */
		"K0",	/* id = 83 */
		"K1",	/* id = 84 */
		"KN",	/* id = 85 */
		"KEI",	/* id = 86 */
		"KER",	/* id = 87 */
		"KURT",	/* id = 88 */
		"LCDF",	/* id = 89 */
		"LCRIT",	/* id = 90 */
		"LE",	/* id = 91 */
		"LMSSCL",	/* id = 92 */
		"LMSSCLW",	/* id = 93 */
		"LOG",	/* id = 94 */
		"LOG10",	/* id = 95 */
		"LOG1P",	/* id = 96 */
		"LOG2",	/* id = 97 */
		"LOWER",	/* id = 98 */
		"LPDF",	/* id = 99 */
		"LRAND",	/* id = 100 */
		"LSQFIT",	/* id = 101 */
		"LT",	/* id = 102 */
		"MAD",	/* id = 103 */
		"MADW",	/* id = 104 */
		"MAX",	/* id = 105 */
		"MEAN",	/* id = 106 */
		"MEANW",	/* id = 107 */
		"MEDIAN",	/* id = 108 */
		"MEDIANW",	/* id = 109 */
		"MIN",	/* id = 110 */
		"MOD",	/* id = 111 */
		"MODE",	/* id = 112 */
		"MODEW",	/* id = 113 */
		"MUL",	/* id = 114 */
		"NAN",	/* id = 115 */
		"NEG",	/* id = 116 */
		"NEQ",	/* id = 117 */
		"NORM",	/* id = 118 */
		"NOT",	/* id = 119 */
		"NRAND",	/* id = 120 */
		"OR",	/* id = 121 */
		"PERM",	/* id = 122 */
		"PLM",	/* id = 123 */
		"PLMg",	/* id = 124 */
		"POP",	/* id = 125 */
		"POW",	/* id = 126 */
		"PPDF",	/* id = 127 */
		"PQUANT",	/* id = 128 */
		"PQUANTW",	/* id = 129 */
		"PSI",	/* id = 130 */
		"PV",	/* id = 131 */
		"QV",	/* id = 132 */
		"R2",	/* id = 133 */
		"R2D",	/* id = 134 */
		"RAND",	/* id = 135 */
		"RCDF",	/* id = 136 */
		"RCRIT",	/* id = 137 */
		"RPDF",	/* id = 138 */
		"RINT",	/* id = 139 */
		"RMS",	/* id = 140 */
		"RMSW",	/* id = 141 */
		"ROLL",	/* id = 142 */
		"ROTT",	/* id = 143 */
		"SEC",	/* id = 144 */
		"SECD",	/* id = 145 */
		"SECH",	/* id = 146 */
		"SIGN",	/* id = 147 */
		"SIN",	/* id = 148 */
		"SINC",	/* id = 149 */
		"SIND",	/* id = 150 */
		"SINH",	/* id = 151 */
		"SKEW",	/* id = 152 */
		"SORT",	/* id = 153 */
		"SQR",	/* id = 154 */
		"SQRT",	/* id = 155 */
		"STD",	/* id = 156 */
		"STDW",	/* id = 157 */
		"STEP",	/* id = 158 */
		"STEPT",	/* id = 159 */
		"SUB",	/* id = 160 */
		"SUM",	/* id = 161 */
		"SVDFIT",	/* id = 162 */
		"TAN",	/* id = 163 */
		"TAND",	/* id = 164 */
		"TANH",	/* id = 165 */
		"TAPER",	/* id = 166 */
		"TCDF",	/* id = 167 */
		"TN",	/* id = 168 */
		"TPDF",	/* id = 169 */
		"TCRIT",	/* id = 170 */
		"UPPER",	/* id = 171 */
		"VAR",	/* id = 172 */
		"VARW",	/* id = 173 */
		"WCDF",	/* id = 174 */
		"WCRIT",	/* id = 175 */
		"WPDF",	/* id = 176 */
		"XOR",	/* id = 177 */
		"Y0",	/* id = 178 */
		"Y1",	/* id = 179 */
		"YN",	/* id = 180 */
		"ZCRIT",	/* id = 181 */
		"ZCDF",	/* id = 182 */
		"ZPDF",	/* id = 183 */
		"ROOTS",	/* id = 184 */
		"HSV2LAB",	/* id = 185 */
		"HSV2RGB",	/* id = 186 */
		"HSV2XYZ",	/* id = 187 */
		"LAB2HSV",	/* id = 188 */
		"LAB2RGB",	/* id = 189 */
		"LAB2XYZ",	/* id = 190 */
		"RGB2HSV",	/* id = 191 */
		"RGB2LAB",	/* id = 192 */
		"RGB2XYZ",	/* id = 193 */
		"XYZ2HSV",	/* id = 194 */
		"XYZ2LAB",	/* id = 195 */
		"XYZ2RGB",	/* id = 196 */
		"" /* last element is intentionally left blank */
	};

	int (*call_operator[GMTMATH_N_OPERATORS]) (struct GMT_CTRL *, struct GMTMATH_INFO *, struct GMTMATH_STACK **S, unsigned int, unsigned int);

	struct GMTMATH_STACK *stack[GMTMATH_STACK_SIZE];
	struct GMTMATH_STORED *recall[GMTMATH_STORE_SIZE];
	struct GMT_DATASET *A_in = NULL, *D_stdin = NULL, *D_in = NULL;
	struct GMT_DATASET *T_in = NULL, *Template = NULL, *Time = NULL, *R = NULL;
	struct GMT_DATATABLE *rhs = NULL, *D = NULL, *I = NULL;
	struct GMT_DATASET_HIDDEN *DH = NULL;
	struct GMT_HASH localhashnode[GMTMATH_N_OPERATORS];
	struct GMT_OPTION *opt = NULL, *list = NULL;
	struct GMTMATH_INFO info;
	struct GMTMATH_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */
	gmtmath_expand_recall_cmd (options);	/* Avoid any conflicts with [RCL]@item and remote cache files */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if ((list = gmt_substitute_macros (GMT, options, "gmtmath.macros")) == NULL) Return1 (GMT_DATA_READ_ERROR);
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, list)) Return1 (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, list)) != 0) Return1 (error);

	/*---------------------------- This is the gmtmath main code ----------------------------*/

	GMT_Report (API, GMT_MSG_INFORMATION, "Perform reverse Polish notation calculations on data tables\n");

	if (Ctrl->Q.active || Ctrl->S.active) {	/* Turn off table and segment headers in calculator or one-record mode */
		gmt_set_tableheader (GMT, GMT_OUT, false);
		gmt_set_segmentheader (GMT, GMT_OUT, false);
	}

	gmt_M_memset (&info, 1, struct GMTMATH_INFO);
	gmt_M_memset (recall, GMTMATH_STORE_SIZE, struct GMTMATH_STORED *);

	t_check_required = !Ctrl->T.notime;	/* Turn off default GMT NaN-handling in t column */

	gmt_hash_init (GMT, localhashnode, operator, GMTMATH_N_OPERATORS, GMTMATH_N_OPERATORS);

	for (i = 0; i < GMTMATH_STACK_SIZE; i++) stack[i] = gmt_M_memory (GMT, NULL, 1, struct GMTMATH_STACK);

	GMT->current.io.skip_if_NaN[GMT_X] = GMT->current.io.skip_if_NaN[GMT_Y] = false;	/* Turn off default GMT NaN-handling of x/y (e.g. lon/lat columns) */
	GMT->current.io.skip_if_NaN[Ctrl->N.tcol] = t_check_required;	/* Determines if the t-column may have NaNs */
	info.scalar = Ctrl->Q.active;

	/* Because of how gmtmath works we do not use GMT_Init_IO to register inputfiles */

	/* Read the first file we encounter so we may allocate space */

	/* Check sanity of all arguments and also look for an input file to get t from */
	for (opt = list, got_t_from_file = 0; got_t_from_file == 0 && opt; opt = opt->next) {
		if (!(opt->option == GMT_OPT_INFILE))	continue;	/* Skip command line options and output */
		/* Filenames,  operators, some numbers and = will all have been flagged as files by the parser */
		gmtmath_backwards_fixing (GMT, &(opt->arg));	/* Possibly exchange obsolete operator name for new one unless compatibility is off */
		op = decode_gmt_argument (GMT, opt->arg, &value, &dimension, localhashnode);	/* Determine what this is */
		if (op == GMTMATH_ARG_IS_BAD) Return (GMT_RUNTIME_ERROR);		/* Horrible */
		if (op != GMTMATH_ARG_IS_FILE) continue;				/* Skip operators and numbers */
		if (!got_t_from_file) {
			/* Passing GMT_VIA_MODULE_INPUT since these are command line file arguments but processed here instead of by GMT_Init_IO */
			if (!strcmp (opt->arg, "STDIN")) {	/* Special stdin name.  We store this input in a special struct since we may need it again and it can only be read once! */
				if ((D_stdin = GMT_Read_Data (API, GMT_IS_DATASET|GMT_VIA_MODULE_INPUT, GMT_IS_STREAM, GMT_IS_NONE, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
					Return (API->error);
				}
				read_stdin = true;
				D_in = D_stdin;
				I = D_stdin->table[0];
			}
			else if ((D_in = GMT_Read_Data (API, GMT_IS_DATASET|GMT_VIA_MODULE_INPUT, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL | GMT_IO_RESET, NULL, opt->arg, NULL)) == NULL) {
				/* Read but request IO reset since the file (which may be a memory reference) will be read again later */
				Return (API->error);
			}
			got_t_from_file = 1;
		}
	}
	if (D_in) {	/* Read a file, update columns */
		if (Ctrl->N.active && Ctrl->N.ncol > D_in->n_columns) gmt_adjust_dataset (GMT, D_in, Ctrl->N.ncol);	/* Add more input columns */
		use_t_col  = Ctrl->N.tcol;
		n_columns  = D_in->n_columns;
		if (n_columns == 1) Ctrl->T.notime = true;	/* Only one column to work with implies -T */
	}
	set_equidistant_t = (Ctrl->T.active && !Ctrl->T.T.file && !Ctrl->T.notime);	/* We were given -Tmin/max/inc */
	if (D_in && set_equidistant_t) {
		GMT_Report (API, GMT_MSG_ERROR, "Cannot use -T when data files are specified\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (Ctrl->A.active) {	/* Always stored as t and f(t) in cols 0 and 1 */
		if (D_in) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot have data files when -A is specified\n");
			Return (GMT_RUNTIME_ERROR);
		}
		gmt_disable_bhi_opts (GMT);	/* Do not want any -b -h -i to affect the reading from -A files */
		if ((A_in = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->A.file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Failure while reading file %s\n", Ctrl->A.file);
			Return (API->error);
		}
		gmt_reenable_bhi_opts (GMT);	/* Recover settings provided by user (if -b -h -i were used at all) */
		rhs = A_in->table[0];	/* Only one table */
		if (Ctrl->A.w_mode) {	/* Need at least 3 columns */
			if (rhs->n_columns < 3) {
				GMT_Report (API, GMT_MSG_ERROR, "Option -A: Requires a file with at least 3 (t,f(t),w(t)|s(t)) columns\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}
		else {	/* Need at least 2 columns */
			if (rhs->n_columns < 2) {
				GMT_Report (API, GMT_MSG_ERROR, "Option -A: Requires a file with at least 2 (t,f(t)) columns\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}
	}
	if (Ctrl->Q.active) {	/* Shorthand for -N1/0 -T0/0/1 -Ca */
		Ctrl->N.ncol = 1;
		Ctrl->N.tcol = 0;
		Ctrl->N.active = set_equidistant_t = true;
		Ctrl->T.T.min = Ctrl->T.T.max = 0.0;
		Ctrl->T.T.inc = 1.0;
		Ctrl->T.T.n = 1;
		Ctrl->T.T.array = gmt_M_memory (GMT, NULL, 1, double);
		Ctrl->T.notime = true;
		n_rows = n_columns = 1;
	}
	if (Ctrl->N.active) {
		GMT->current.io.skip_if_NaN[GMT_X] = GMT->current.io.skip_if_NaN[GMT_Y] = false;
		GMT->current.io.skip_if_NaN[Ctrl->N.tcol] = true;
		n_columns = Ctrl->N.ncol;
	}
	if (Ctrl->T.active && Ctrl->T.T.file) {	/* Gave a file that we will use to obtain the T vector only */
		if (D_in) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot use -T when data files are specified\n");
			Return (GMT_RUNTIME_ERROR);
		}
		use_t_col = 0;
	}

	if (set_equidistant_t && !Ctrl->Q.active) {
		n_rows = Ctrl->T.T.n;
		n_columns = Ctrl->N.ncol;
	}

	if (Ctrl->A.active) {	/* Get number of rows and time from the file, but not n_cols (that takes -N, which defaults to 2) */
		n_rows = rhs->n_records;
		n_columns = Ctrl->N.ncol;
		Ctrl->T.T.min = rhs->min[0];
		Ctrl->T.T.max = rhs->max[0];
		Ctrl->T.T.inc = (rhs->max[0] - rhs->min[0]) / (rhs->n_records - 1);
		if (gmt_create_array (GMT, 'T', &(Ctrl->T.T), NULL, NULL))
			Return (GMT_RUNTIME_ERROR);
	}
	if (Ctrl->T.T.file) n_columns = Ctrl->N.ncol;

	if (!(D_in || T_in || A_in || set_equidistant_t)) {	/* Neither a file nor -T given; must read data from stdin */
		GMT_Report (API, GMT_MSG_ERROR, "Expression must contain at least one table file or -T [and -N]\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (D_in)	/* Obtained file structure from an input file, use this to create new stack entry */
		Template = GMT_Duplicate_Data (API, GMT_IS_DATASET, GMT_DUPLICATE_DATA, D_in);

	else {		/* Must use -N -T etc to create single segment */
		dim[GMT_COL] = n_columns;	dim[GMT_ROW] = n_rows;
		if ((Template = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (GMT_MEMORY_ERROR);
		Template->table[0]->segment[0]->n_rows = n_rows;
	}
	Ctrl->N.ncol = n_columns;
	if (!Ctrl->T.notime && n_columns > 1) Ctrl->C.cols[Ctrl->N.tcol] = (Ctrl->Q.active) ? false : true;

	/* Create the Time data structure with 3 cols: 0 is t, 1 is normalized tn, 2 is row numbers */
	if (D_in) {	/* Either D_in or D_stdin */
		Time = gmt_alloc_dataset (GMT, D_in, 0, 3, GMT_ALLOC_NORMAL);
		free_time = true;
		info.T = Time->table[0];	D = D_in->table[0];
		for (seg = 0, done = false; seg < D->n_segments; seg++) {
			gmt_M_memcpy (info.T->segment[seg]->data[COL_T], D->segment[seg]->data[use_t_col], D->segment[seg]->n_rows, double);
			if (!done) {
				for (row = 1; row < info.T->segment[seg]->n_rows && (gmt_M_is_dnan (info.T->segment[seg]->data[COL_T][row-1]) || gmt_M_is_dnan (info.T->segment[seg]->data[COL_T][row])); row++);	/* Find the first real two records in a row */
				Ctrl->T.T.inc = (row == info.T->segment[seg]->n_rows) ? GMT->session.d_NaN : info.T->segment[seg]->data[COL_T][row] - info.T->segment[seg]->data[COL_T][row-1];
				t_noise = fabs (GMT_CONV4_LIMIT * Ctrl->T.T.inc);
			}
			for (row = 1; row < info.T->segment[seg]->n_rows && !info.irregular; row++) if (fabs (fabs (info.T->segment[seg]->data[COL_T][row] - info.T->segment[seg]->data[COL_T][row-1]) - fabs (Ctrl->T.T.inc)) > t_noise) info.irregular = true;
		}
		if (!read_stdin && GMT_Destroy_Data (API, &D_in) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	else {	/* Create orderly output */
		dim[GMT_COL] = 3;	dim[GMT_ROW] = n_rows;
		if ((Time = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (GMT_MEMORY_ERROR);
		info.T = Time->table[0];
        	info.T->segment[0]->n_rows = n_rows;
		gmt_M_memcpy (info.T->segment[0]->data[COL_T], Ctrl->T.T.array, n_rows, double);
	}

	for (seg = n_records = 0; seg < info.T->n_segments; seg++) {	/* Create normalized times and possibly reverse time (-I) */
		off = 0.5 * (info.T->segment[seg]->data[COL_T][info.T->segment[seg]->n_rows-1] + info.T->segment[seg]->data[COL_T][0]);
		scale = 2.0 / (info.T->segment[seg]->data[COL_T][info.T->segment[seg]->n_rows-1] - info.T->segment[seg]->data[COL_T][0]);
		if (Ctrl->I.active) for (row = 0; row < info.T->segment[seg]->n_rows/2; row++) gmt_M_double_swap (info.T->segment[seg]->data[COL_T][row], info.T->segment[seg]->data[COL_T][info.T->segment[seg]->n_rows-1-row]);	/* Reverse time series */
		for (row = 0; row < info.T->segment[seg]->n_rows; row++) {
			info.T->segment[seg]->data[COL_TN][row] = (info.T->segment[seg]->data[COL_T][row] - off) * scale;
			info.T->segment[seg]->data[COL_TJ][row] = (unsigned int)((Ctrl->I.active) ? info.T->segment[seg]->n_rows - row - 1 : row);
		}
		n_records += info.T->segment[seg]->n_rows;
	}
	info.t_min = Ctrl->T.T.min;	info.t_max = Ctrl->T.T.max;	info.t_inc = Ctrl->T.T.inc;
	info.n_col = n_columns;		info.local = Ctrl->L.active;
	info.notime = Ctrl->T.notime;
	gmt_set_tbl_minmax (GMT, GMT_IS_POINT, info.T);

	if (Ctrl->A.active) {	/* Set up A * x = b, with the table holding the extended matrix [ A | [w | ] b ], with w the optional weights */
		if (!stack[0]->D)
			stack[0]->D = gmt_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
		load_column (stack[0]->D, n_columns-1, rhs, 1);		/* Always put the r.h.s of the Ax = b equation in the last column of the item on the stack */
		if (!Ctrl->A.null) load_column (stack[0]->D, Ctrl->N.tcol, rhs, 0);	/* Optionally, put the t vector in the time column of the item on the stack */
		gmt_set_tbl_minmax (GMT, stack[0]->D->geometry, stack[0]->D->table[0]);
		nstack = 1;
		info.fit_mode = Ctrl->A.e_mode;
		info.w_mode = Ctrl->A.w_mode;
	}
	else
		nstack = 0;

	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_PI]     = M_PI;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_E]      = M_E;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_F_EPS]  = FLT_EPSILON;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_D_EPS]  = DBL_EPSILON;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_EULER]  = M_EULER;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_PHI]    = M_PHI;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_TMIN]   = Ctrl->T.T.min;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_TMAX]   = Ctrl->T.T.max;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_TRANGE] = Ctrl->T.T.max - Ctrl->T.T.min;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_TINC]   = Ctrl->T.T.inc;
	special_symbol[GMTMATH_ARG_IS_PI-GMTMATH_ARG_IS_N]      = (double)n_records;

	gmtmath_init (call_operator, consumed_operands, produced_operands);
	op = decode_gmt_argument (GMT, "EXCH", &value, &dimension, localhashnode);
	if (op == GMTMATH_ARG_IS_BAD) {
		GMT_Report (API, GMT_MSG_ERROR, "Bad input argument!\n");
		Return (GMT_RUNTIME_ERROR);
	}
	consumed_operands[op] = produced_operands[op] = 0;	/* Modify items since we simply swap pointers */

	for (opt = list, error = false; !error && opt; opt = opt->next) {

		/* First check if we should skip optional arguments */

		if (strchr (SPECIFIC_OPTIONS THIS_MODULE_OPTIONS GMT_OPT("F"), opt->option)) continue;
		if (opt->option == 'C') {	/* Change affected columns */
			no_C = false;
			if (decode_columns (opt->arg, Ctrl->C.cols, n_columns, Ctrl->N.tcol)) touched_t_col = true;
			continue;
		}
		if (opt->option == GMT_OPT_OUTFILE) continue;	/* We do output after the loop */

		gmtmath_backwards_fixing (GMT, &(opt->arg));	/* Possibly exchange obsolete operator name for new one unless compatibility is off */

		op = decode_gmt_argument (GMT, opt->arg, &value, &dimension, localhashnode);

		if (op != GMTMATH_ARG_IS_FILE && !gmt_access (GMT, opt->arg, R_OK)) GMT_Message (API, GMT_TIME_NONE, "The number or operator %s may be confused with an existing file %s!  The file will be ignored.\n", opt->arg, opt->arg);

		if (op < GMTMATH_ARG_IS_OPERATOR) {	/* File name or factor */

			if (nstack == GMTMATH_STACK_SIZE) {	/* Stack overflow */
				GMT_Report (API, GMT_MSG_ERROR, "Stack overflow!\n");
				Return (GMT_RUNTIME_ERROR);
			}

			if (op == GMTMATH_ARG_IS_NUMBER) {
				stack[nstack]->constant = true;
				stack[nstack]->factor = value;
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "%g ", stack[nstack]->factor);
				nstack++;
				continue;
			}
			else if (op <= GMTMATH_ARG_IS_PI && op >= GMTMATH_ARG_IS_N) {
				stack[nstack]->constant = true;
				stack[nstack]->factor = special_symbol[GMTMATH_ARG_IS_PI-op];
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "%g ", stack[nstack]->factor);
				nstack++;
				continue;
			}
			else if (op == GMTMATH_ARG_IS_STORE) {
				/* Duplicate stack into stored memory location */
				bool new = false;
				last = nstack - 1;
				if (nstack == 0) {
					GMT_Report (API, GMT_MSG_ERROR, "No items on stack to put into stored memory!\n");
					Return (GMT_RUNTIME_ERROR);
				}
				if ((label = gmtmath_setlabel (GMT, opt->arg)) == NULL) Return (GMT_RUNTIME_ERROR);
				if ((k = gmtmath_find_stored_item (recall, n_stored, label)) != GMT_NOTSET) {
					if (!stack[last]->constant) for (j = 0; j < n_columns; j++) if (no_C || !Ctrl->C.cols[j]) load_column (recall[k]->stored.D, j, stack[last]->D->table[0], j);
					GMT_Report (API, GMT_MSG_DEBUG, "Stored memory cell %d named %s is overwritten with new information\n", k, label);
				}
				else {	/* Need new named storage place; use gmt_duplicate_dataset/gmt_free_dataset since no point adding to registered resources when internal use only */
					k = n_stored;
					recall[k] = gmt_M_memory (GMT, NULL, 1, struct GMTMATH_STORED);
					recall[k]->label = strdup (label);
					if (!stack[last]->constant) recall[k]->stored.D = gmt_duplicate_dataset (GMT, stack[last]->D, GMT_ALLOC_NORMAL, NULL);
					new = true;
					GMT_Report (API, GMT_MSG_DEBUG, "Stored memory cell %d named %s is created with new information\n", k, label);
				}
				recall[k]->stored.constant = stack[last]->constant;
				recall[k]->stored.factor = stack[last]->factor;
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "[--> %s] ", recall[k]->label);
				if (new) n_stored++;	/* We added a new item */
				continue;	/* Just go back and process next item */
			}
			else if (op == GMTMATH_ARG_IS_RECALL) {
				/* Add to stack from stored memory location */
				if ((label = gmtmath_setlabel (GMT, opt->arg)) == NULL) Return (GMT_RUNTIME_ERROR);
				if ((k = gmtmath_find_stored_item (recall, n_stored, label)) == GMT_NOTSET) {
					GMT_Report (API, GMT_MSG_ERROR, "No stored memory item with label %s exists!\n", label);
					Return (GMT_RUNTIME_ERROR);
				}
				if (recall[k]->stored.constant) {	/* Place a stored constant on the stack */
					stack[nstack]->constant = true;
					stack[nstack]->factor = recall[k]->stored.factor;
				}
				else {/* Place the stored dataset on the stack */
					stack[nstack]->constant = false;
					if (!stack[nstack]->D)
						stack[nstack]->D = gmt_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
					for (j = 0; j < n_columns; j++) if (no_C || !Ctrl->C.cols[j]) load_column (stack[nstack]->D, j, recall[k]->stored.D->table[0], j);
					DH = gmt_get_DD_hidden (stack[nstack]->D);
					gmt_set_tbl_minmax (GMT, stack[nstack]->D->geometry, stack[nstack]->D->table[0]);
				}
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "@%s ", recall[k]->label);
				nstack++;
				continue;
			}
			else if (op == GMTMATH_ARG_IS_CLEAR) {
				/* Free stored memory location */
				if ((label = gmtmath_setlabel (GMT, opt->arg)) == NULL) Return (GMT_RUNTIME_ERROR);
				if ((k = gmtmath_find_stored_item (recall, n_stored, label)) == GMT_NOTSET) {
					GMT_Report (API, GMT_MSG_ERROR, "No stored memory item with label %s exists!\n", label);
					Return (GMT_RUNTIME_ERROR);
				}
				if (recall[k]->stored.D) gmt_free_dataset (GMT, &recall[k]->stored.D);
				gmt_M_str_free (recall[k]->label);
				gmt_M_free (GMT, recall[k]);
				while (k && k == (int)(n_stored-1) && !recall[k]) k--, n_stored--;	/* Chop off trailing NULL cases */
				continue;	/* Just go back and process next item */
			}

			/* Here we need a matrix */

			stack[nstack]->constant = false;

			if (op == GMTMATH_ARG_IS_T_MATRIX) {	/* Need to set up matrix of t-values */
				if (Ctrl->T.notime) {
					GMT_Report (API, GMT_MSG_ERROR, "T is not defined for plain data files!\n");
					Return (GMT_RUNTIME_ERROR);
				}
				if (!stack[nstack]->D)
					stack[nstack]->D = gmt_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "T ");
				for (j = 0; j < n_columns; j++) if (no_C || !Ctrl->C.cols[j]) load_column (stack[nstack]->D, j, info.T, COL_T);
				DH = gmt_get_DD_hidden (stack[nstack]->D);
				gmt_set_tbl_minmax (GMT, stack[nstack]->D->geometry, stack[nstack]->D->table[0]);
			}
			else if (op == GMTMATH_ARG_IS_t_MATRIX) {	/* Need to set up matrix of normalized t-values */
				if (Ctrl->T.notime) {
					GMT_Report (API, GMT_MSG_ERROR, "TNORM is not defined for plain data files!\n");
					Return (GMT_RUNTIME_ERROR);
				}
				if (!stack[nstack]->D)
					stack[nstack]->D = gmt_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "TNORM ");
				for (j = 0; j < n_columns; j++) if (no_C || !Ctrl->C.cols[j]) load_column (stack[nstack]->D, j, info.T, COL_TN);
				DH = gmt_get_DD_hidden (stack[nstack]->D);
				gmt_set_tbl_minmax (GMT, stack[nstack]->D->geometry, stack[nstack]->D->table[0]);
			}
			else if (op == GMTMATH_ARG_IS_J_MATRIX) {	/* Need to set up matrix of row numbers */
				if (!stack[nstack]->D)
					stack[nstack]->D = gmt_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "TROW ");
				for (j = 0; j < n_columns; j++) if (no_C || !Ctrl->C.cols[j]) load_column (stack[nstack]->D, j, info.T, COL_TJ);
				DH = gmt_get_DD_hidden (stack[nstack]->D);
				gmt_set_tbl_minmax (GMT, stack[nstack]->D->geometry, stack[nstack]->D->table[0]);
			}
			else if (op == GMTMATH_ARG_IS_FILE) {		/* Filename given */
				struct GMT_DATASET *F = NULL;
				struct GMT_DATATABLE *T_in = NULL;
				if (!stack[nstack]->D)
					stack[nstack]->D = gmt_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
				if (!strcmp (opt->arg, "STDIN")) {	/* stdin file */
					T_in = I;
					if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "<stdin> ");
				}
				else {
					if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "%s ", opt->arg);
					/* Passing GMT_VIA_MODULE_INPUT since these are command line file arguments but processed here instead of by GMT_Init_IO */
					if ((F = GMT_Read_Data (API, GMT_IS_DATASET|GMT_VIA_MODULE_INPUT, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, opt->arg, NULL)) == NULL) {
						GMT_Report (API, GMT_MSG_ERROR, "Failure while reading file %s\n", opt->arg);
						Return (API->error);
					}
					if (Ctrl->N.ncol > F->n_columns) gmt_adjust_dataset (GMT, F, Ctrl->N.ncol);	/* Add more input columns */
					T_in = F->table[0];	/* Only one table since only a single file */
				}
				for (j = 0; j < n_columns; j++) if (no_C || !Ctrl->C.cols[j]) load_column (stack[nstack]->D, j, T_in, j);
				DH = gmt_get_DD_hidden (stack[nstack]->D);
				gmt_set_tbl_minmax (GMT, stack[nstack]->D->geometry, stack[nstack]->D->table[0]);
				if (!same_size (stack[nstack]->D, Template)) {
					GMT_Report (API, GMT_MSG_ERROR, "tables not of same size!\n");
					Return (GMT_RUNTIME_ERROR);
				}
				else if (!Ctrl->C.cols[Ctrl->N.tcol] && !(Ctrl->T.notime || same_domain (stack[nstack]->D, Ctrl->N.tcol, info.T))) {
					GMT_Report (API, GMT_MSG_ERROR, "tables do not cover the same domain!\n");
					Return (GMT_RUNTIME_ERROR);
				}
				if (T_in != I && GMT_Destroy_Data (API, &F) != GMT_NOERROR) {
					Return (API->error);
				}
			}
			nstack++;
			continue;
		}

		/* Here we have an operator */

		eaten = consumed_operands[op];	created = produced_operands[op];
		if (color_operator_on_table (info.scalar, opt->arg)) {	/* These operators read across 3 columns instead */
			if (info.n_col != 3) {
				GMT_Report (API, GMT_MSG_ERROR, "Input tables must have 3 columns for using the color triplet operator %s!\n", opt->arg);
				Return (GMT_RUNTIME_ERROR);
			}
			eaten = created = 1;
		}

		if (!strncmp (opt->arg, "ROOTS", 5U) && !(opt->next && opt->next->arg[0] == '=')) {
			GMT_Report (API, GMT_MSG_ERROR, "Only = may follow operator ROOTS\n");
			Return (GMT_RUNTIME_ERROR);
		}

		if ((new_stack = nstack - eaten + created) >= GMTMATH_STACK_SIZE) {
			GMT_Report (API, GMT_MSG_ERROR, "Stack overflow (%s)\n", opt->arg);
			Return (GMT_RUNTIME_ERROR);
		}

		if (nstack < eaten) {
			GMT_Report (API, GMT_MSG_ERROR, "Operation \"%s\" requires %d operands\n", operator[op], eaten);
			Return (GMT_RUNTIME_ERROR);
		}

		if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "%s ", operator[op]);

		for (i = created - eaten; i > 0; i--) {
			if (stack[nstack+i-1]->D) continue;

			/* Must make space for more */
			stack[nstack+i-1]->D = gmt_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
		}

		/* If operators operates on constants only we may have to make space as well */

		for (j = 0, i = nstack - eaten; j < created; j++, i++) {
			if (stack[i]->constant && !stack[i]->D) {
				stack[i]->D = gmt_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
				if (!Ctrl->T.notime) load_column (stack[i]->D, COL_T, info.T, COL_T);	/* Make sure t-column is copied if needed */
			}
		}

		if (!strcmp (operator[op], "LSQFIT")) {	/* Special case, solve LSQ system and return */
			solve_LSQFIT (GMT, &info, stack[nstack-1], n_columns, Ctrl->C.cols, Ctrl->E.eigen, Ctrl->Out.file, options, A_in);
			Return (GMT_NOERROR);
		}
		else if (!strcmp (operator[op], "SVDFIT")) {	/* Special case, solve SVD system and return */
			solve_SVDFIT (GMT, &info, stack[nstack-1], n_columns, Ctrl->C.cols, Ctrl->E.eigen, Ctrl->Out.file, options, A_in);
			Return (GMT_NOERROR);
		}
		else if (!strcmp (operator[op], "SORT")) {	/* Special case, sort all columns inside operator */
			status = (*call_operator[op]) (GMT, &info, stack, nstack - 1, 0);	/* Do it for all columns, the col = 0 is not used inside function */
		}
		else {
			for (j = 0; j < n_columns; j++) {
				if (Ctrl->C.cols[j]) continue;
				status = (*call_operator[op]) (GMT, &info, stack, nstack - 1, j);	/* Do it */
				if (status == -1) {	/* Serious problem, need to bail */
					GMT_exit (GMT, GMT_RUNTIME_ERROR); Return (GMT_RUNTIME_ERROR);
				}
			}
		}
		free_sort_list (GMT, &info);	/* Frees helper array if SORT was called */

		nstack = new_stack;

		for (kk = 1; kk <= created; kk++) if (stack[nstack-kk]->D) stack[nstack-kk]->constant = false;	/* Now filled with table */
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		(Ctrl->Out.file) ? GMT_Message (API, GMT_TIME_NONE, "= %s", Ctrl->Out.file) : GMT_Message (API, GMT_TIME_NONE,  "= <stdout>");
	}

	if (nstack == 0) {
		GMT_Report (API, GMT_MSG_ERROR, "No operands left on the stack!\n");
		Return (API->error);
	}
	last = nstack - 1;	/* Index of top of stack (also hopefully bottom of stack) */

	if (stack[last]->constant) {	/* Only a constant provided, set table accordingly */
		if (!stack[last]->D)
			stack[last]->D = gmt_alloc_dataset (GMT, Template, 0, n_columns, GMT_ALLOC_NORMAL);
		for (j = 0; j < n_columns; j++) {
			if (j == COL_T && !Ctrl->Q.active && Ctrl->C.cols[j])
				load_column (stack[last]->D, j, info.T, COL_T);
			else if (!Ctrl->C.cols[j])
				load_const_column (stack[last]->D, j, stack[last]->factor);
		}
		DH = gmt_get_DD_hidden (stack[last]->D);
		gmt_set_tbl_minmax (GMT, stack[last]->D->geometry, stack[last]->D->table[0]);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "\n");

	if (info.roots_found) {	/* Special treatment of root finding */
		struct GMT_DATASEGMENT *S = stack[0]->D->table[0]->segment[0];
		uint64_t dim[GMT_DIM_SIZE] = {1, 1, 0, 1};

		dim[GMT_ROW] = info.n_roots;
		if ((R = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error)
		for (kk = 0; kk < info.n_roots; kk++) R->table[0]->segment[0]->data[GMT_X][kk] = S->data[info.r_col][kk];
		R->table[0]->segment[0]->n_rows = info.n_roots;
		DH = gmt_get_DD_hidden (stack[0]->D);
		GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, R);
		if (GMT_Write_Data (API, GMT_IS_DATASET, (Ctrl->Out.file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_NONE, DH->io_mode, NULL, Ctrl->Out.file, R) != GMT_NOERROR) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &R) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	else {	/* Regular table result */
		bool template_used = false, place_t_col = (Ctrl->T.active && t_check_required && !Ctrl->Q.active && !touched_t_col);

		if (stack[last]->D)	/* There is an output stack, select it */
			R = stack[last]->D;
		else {		/* Can happen if only -T [-N] was specified with no operators */
			R = Template;
			template_used = true;
		}
		DH = gmt_get_DD_hidden (R);
		if (dimension && Ctrl->Q.active) {	/* Encountered dimensioned items on the command line, must return in current units */
			R->table[0]->segment[0]->data[0][0] *= GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];
		}
		if (place_t_col && Ctrl->N.tcol < R->n_columns) {
			load_column (R, Ctrl->N.tcol, info.T, COL_T);	/* Put T in the time column of the item on the stack if possible */
			gmt_set_tbl_minmax (GMT, R->geometry, R->table[0]);
		}
		if ((error = GMT_Set_Columns (API, GMT_OUT, (unsigned int)R->n_columns, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) Return (error);	/* Since -bo might have been used */
		if (Ctrl->S.active) {	/* Only get one record per segment */
			uint64_t row, c;
			uint64_t dim[GMT_DIM_SIZE] = {1, 0, 1, 0};	/* One table, 1 row per table, need to set segments and columns below */
			struct GMT_DATASET *N = NULL;
			dim[GMT_SEG] = R->table[0]->n_segments;
			dim[GMT_COL] = n_columns;
			if ((N = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL)
				return (GMT->parent->error);
			for (seg = 0; seg < R->table[0]->n_segments; seg++) {
				row = (Ctrl->S.mode == -1) ? 0 : R->table[0]->segment[seg]->n_rows - 1;
				for (c = 0; c < n_columns; c++) N->table[0]->segment[seg]->data[c][0] = R->table[0]->segment[seg]->data[c][row];
                N->table[0]->segment[seg]->n_rows = 1;
			}
			DH = gmt_get_DD_hidden (N);
			GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, N);
			if (GMT_Write_Data (API, GMT_IS_DATASET, (Ctrl->Out.file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_NONE, DH->io_mode, NULL, Ctrl->Out.file, N) != GMT_NOERROR) {
				Return (API->error);
			}
		}
		else {	/* Write the whole enchilada */
			GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, R);
			if (GMT_Write_Data (API, GMT_IS_DATASET, (Ctrl->Out.file ? GMT_IS_FILE : GMT_IS_STREAM), GMT_IS_NONE, DH->io_mode, NULL, Ctrl->Out.file, R) != GMT_NOERROR) {
				Return (API->error);
			}
		}
		if (template_used) Template = NULL;	/* This prevents it from being freed twice (once from API registration via GMT_Write_Data and then again in Free_Misc) */
	}

	/* Clean-up time */

	if (Ctrl->A.active && GMT_Destroy_Data (API, &A_in) != GMT_NOERROR) {
		Return (API->error);
	}

	if (free_time) gmt_free_dataset (GMT, &Time);
	for (kk = 0; kk < n_stored; kk++) {	/* Free up stored STO/RCL memory */
		if (recall[kk]->stored.D) gmt_free_dataset (GMT, &recall[kk]->stored.D);
		gmt_M_str_free (recall[kk]->label);
		gmt_M_free (GMT, recall[kk]);
	}

	if (nstack > 1) GMT_Report (API, GMT_MSG_WARNING, "%d more operands left on the stack!\n", nstack-1);

	Return (GMT_NOERROR);
}
