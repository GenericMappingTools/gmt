 /*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: grdmath.c is a reverse polish calculator that operates on grid files
 * (and constants) and perform basic mathematical operations
 * on them like add, multiply, etc.
 * Some operators only work on one operand (e.g., log, exp)
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"grdmath"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Reverse Polish Notation (RPN) calculator for grids (element by element)"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:RVbdfghinrs" GMT_OPT("F")

EXTERN_MSC int gmt_load_macros (struct GMT_CTRL *GMT, char *mtype, struct MATH_MACRO **M);
EXTERN_MSC int gmt_find_macro (char *arg, unsigned int n_macros, struct MATH_MACRO *M);
EXTERN_MSC void gmt_free_macros (struct GMT_CTRL *GMT, unsigned int n_macros, struct MATH_MACRO **M);
EXTERN_MSC struct GMT_OPTION * gmt_substitute_macros (struct GMT_CTRL *GMT, struct GMT_OPTION *options, char *mfile);

#define GRDMATH_ARG_IS_OPERATOR		 0
#define GRDMATH_ARG_IS_FILE		-1
#define GRDMATH_ARG_IS_NUMBER		-2
#define GRDMATH_ARG_IS_PI		-3
#define GRDMATH_ARG_IS_E		-4
#define GRDMATH_ARG_IS_EULER		-5
#define GRDMATH_ARG_IS_XMIN		-6
#define GRDMATH_ARG_IS_XMAX		-7
#define GRDMATH_ARG_IS_XINC		-8
#define GRDMATH_ARG_IS_NX		-9
#define GRDMATH_ARG_IS_YMIN		-10
#define GRDMATH_ARG_IS_YMAX		-11
#define GRDMATH_ARG_IS_YINC		-12
#define GRDMATH_ARG_IS_NY		-13
#define GRDMATH_ARG_IS_X_MATRIX		-14
#define GRDMATH_ARG_IS_x_MATRIX		-15
#define GRDMATH_ARG_IS_Y_MATRIX		-16
#define GRDMATH_ARG_IS_y_MATRIX		-17
#define GRDMATH_ARG_IS_ASCIIFILE	-18
#define GRDMATH_ARG_IS_SAVE		-19
#define GRDMATH_ARG_IS_STORE		-50
#define GRDMATH_ARG_IS_RECALL		-51
#define GRDMATH_ARG_IS_CLEAR		-52
#define GRDMATH_ARG_IS_BAD		-99

#define GRDMATH_STACK_SIZE		100
#define GRDMATH_STORE_SIZE		100

#define GRDMATH_STORE_CMD		"STO@"
#define GRDMATH_RECALL_CMD		"RCL@"
#define GRDMATH_CLEAR_CMD		"CLR@"

#define FLOAT_BIT_MASK (~(127U << 25U))	/* This will be 00000001 11111111 11111111 11111111 and sets to 0 anything larger than 2^24 which is max integer in float */

struct GRDMATH_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct Out {	/* = <filename> */
		bool active;
		char *file;
	} Out;
	struct A {	/* -A<min_area>[/<min_level>/<max_level>][+ag|i|s][+r|l][+p<percent>] */
		bool active;
		struct GMT_SHORE_SELECT info;
	} A;
	struct D {	/* -D<resolution> */
		bool active;
		bool force;	/* if true, select next highest level if current set is not avaialble */
		char set;	/* One of f, h, i, l, c */
	} D;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct M {	/* -M */
		bool active;
	} M;
	struct N {	/* -N */
		bool active;
	} N;
};

struct GRDMATH_INFO {
	int error;
	uint64_t nm;
	size_t size;
	char *ASCII_file;
	char gshhg_res;	/* If -D is set */
	bool convert;		/* Reflects -M */
	double *d_grd_x,  *d_grd_y;
	double *d_grd_xn, *d_grd_yn;
	float *f_grd_x,  *f_grd_y;
	float *f_grd_xn, *f_grd_yn;
	double *dx, dy;		/* In flat-Earth m if -M is set */
	struct GMT_GRID *G;
	struct GMT_SHORE_SELECT *A;	/* If -A is processed */
};

struct GRDMATH_STACK {
	struct GMT_GRID *G;		/* The grid */
	bool constant;			/* true if a constant (see factor) and S == NULL */
	double factor;			/* The value if constant is true */
	unsigned int alloc_mode;	/* 0 is not allocated, 1 is allocated in this program, 2 = allocated elsewhere */
};

struct GRDMATH_STORE {
	char *label;	/* Name of this stored memory */
	struct GRDMATH_STACK stored;
};

void *New_grdmath_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDMATH_CTRL *C = GMT_memory (GMT, NULL, 1, struct GRDMATH_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.info.high = GSHHS_MAX_LEVEL;	/* Include all GSHHS levels (if LDISTG is used) */
	C->D.set = 'l';				/* Low-resolution coastline data */
	return (C);
}

void Free_grdmath_Ctrl (struct GMT_CTRL *GMT, struct GRDMATH_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->Out.file) free (C->Out.file);
	GMT_free (GMT, C);
}

int GMT_grdmath_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdmath [%s]\n\t[%s]\n\t[-D<resolution>][+] [%s]\n\t[-M] [-N] [%s] [%s] [%s]\n\t[%s]\n\t[%s]"
		" [%s]\n\t[%s] [%s] [%s] [%s]\n",	GMT_Rgeo_OPT, GMT_A_OPT, GMT_I_OPT, GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT,
		GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_n_OPT, GMT_r_OPT, GMT_s_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\tA B op C op D op ... = outfile\n\n");

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE,
		"\tA, B, etc are grid files, constants, or symbols (see below).\n"
		"\tThe stack can hold up to %d entries (given enough memory).\n", GRDMATH_STACK_SIZE);
	GMT_Message (API, GMT_TIME_NONE,
		"\tTrigonometric operators expect radians unless noted otherwise.\n"
		"\tThe operators and number of input and output arguments are:\n\n"
		"\tName       #args   Returns\n"
		"\t--------------------------\n");
#include "grdmath_explain.h"
	GMT_Message (API, GMT_TIME_NONE,
		"\n\tThe special symbols are:\n\n"
		"\tPI                     = 3.1415926...\n"
		"\tE                      = 2.7182818...\n"
		"\tEULER                  = 0.5772156...\n"
		"\tXMIN, XMAX, XINC or NX = the corresponding constants.\n"
		"\tYMIN, YMAX, YINC or NY = the corresponding constants.\n"
		"\tX                      = grid with x-coordinates.\n"
		"\tY                      = grid with y-coordinates.\n"
		"\tXn                     = grid with normalized [-1|+1] x-coordinates.\n"
		"\tYn                     = grid with normalized [-1|+1] y-coordinates.\n"
		"\n\tUse macros for frequently used long expressions; see the grdmath man page.\n"
		"\tStore stack to named variable via STO@<label>, recall via [RCL]@<label>, clear via CLR@<label>.\n"
		"\n\tOPTIONS: (only use -R|I|r|f if no grid files are passed as arguments).\n\n");
	GMT_GSHHG_syntax (API->GMT, 'A');
	GMT_Message (API, GMT_TIME_NONE, "\t   (-A is only relevant to the LDISTG operator)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Choose one of the following resolutions to use with the LDISTG operator:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f - full resolution (may be very slow for large regions).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   h - high resolution (may be slow for large regions).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i - intermediate resolution.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   l - low resolution [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   c - crude resolution, for busy plots that need crude continent outlines only.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append + to use a lower resolution should the chosen one not be available [abort].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (-A and -D apply only to operator LDISTG)\n");
	GMT_Option (API, "I");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Handle map units in derivatives.  In this case, dx,dy of grid\n"
		"\t   will be converted from degrees lon,lat into meters (Flat-earth approximation).\n"
		"\t   Default computes derivatives in units of data/grid_distance.\n"
		"\t-N Do not perform strict domain check if several grids are involved.\n"
		"\t   [Default checks that domain is within %g * [xinc or yinc] of each other].\n", GMT_SMALL);
	GMT_Option (API, "R,V");
	GMT_Option (API, "bi2,di,f,g,h,i");
	GMT_Message (API, GMT_TIME_NONE, "\t   (Only applies to the input files for operators LDIST, PDIST, and INSIDE).\n");
	GMT_Option (API, "n,r,s,.");

	return (EXIT_FAILURE);
}

int GMT_grdmath_parse (struct GMT_CTRL *GMT, struct GRDMATH_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdmath and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	bool missing_equal = true;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
			case '<':	/* Input files */
				if (opt->arg[0] == '=' && !opt->arg[1]) {
					missing_equal = false;
					if (opt->next && opt->next->option == GMT_OPT_INFILE) {
						Ctrl->Out.active = true;
						if (opt->next->arg) Ctrl->Out.file = strdup (opt->next->arg);
					}
				}
				break;
			case '#':	/* Numbers */
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Restrict GSHHS features */
				Ctrl->A.active = true;
				GMT_set_levels (GMT, opt->arg, &Ctrl->A.info);
				break;
			case 'D':	/* Set GSHHS resolution */
				Ctrl->D.active = true;
				Ctrl->D.set = opt->arg[0];
				Ctrl->D.force = (opt->arg[1] == '+');
				break;
			case 'I':	/* Grid spacings */
				Ctrl->I.active = true;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'M':	/* Map units */
				Ctrl->M.active = true;
				break;
			case 'N':	/* Relax domain check */
				Ctrl->N.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.registration, &Ctrl->I.active);

	if (missing_equal) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Usage is <operations> = [outfile]\n");
		n_errors++;
	}
	if (Ctrl->I.active && !GMT->common.R.active) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: -I requires the -R option\n");
		n_errors++;
	}
	if (Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -I option: Must specify positive increment(s)\n");
		n_errors++;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

struct GMT_GRID * alloc_stack_grid (struct GMT_CTRL *GMT, struct GMT_GRID *Template)
{	/* Allocate a new GMT_GRID structure based on dimensions etc of the Template */
	struct GMT_GRID *New = GMT_Create_Data (GMT->parent, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, Template->header->wesn, Template->header->inc, \
		Template->header->registration, GMT_NOTSET, NULL);
	return (New);
}

int grdmath_find_stored_item (struct GMT_CTRL *GMT, struct GRDMATH_STORE *recall[], int n_stored, char *label)
{
	int k = 0;
	while (k < n_stored && strcmp (recall[k]->label, label)) k++;
	return (k == n_stored ? -1 : k);
}

/* -----------------------------------------------------------------
 *              Definitions of all operator functions
 * -----------------------------------------------------------------*/
/* Note: The OPERATOR: **** lines are used to extract syntax for documentation */

void grd_ABS (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ABS 1 1 abs (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand == 0!\n");
	if (stack[last]->constant) a = (float)fabs (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : fabsf (stack[last]->G->data[node]);
	GMT_grd_pad_zero (GMT, stack[last]->G);	/* Reset the boundary pad, if needed */
}

void grd_ACOS (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACOS 1 1 acos (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, |operand| > 1 for ACOS!\n");
	if (stack[last]->constant) a = (float)d_acos (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : d_acosf (stack[last]->G->data[node]);
}

void grd_ACOSH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACOSH 1 1 acosh (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand < 1 for ACOSH!\n");
	if (stack[last]->constant) a = (float)acosh (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : acoshf (stack[last]->G->data[node]);
}

void grd_ACOT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACOT 1 1 acot (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, |operand| > 1 for ACOT!\n");
	if (stack[last]->constant) a = (float)atan (1.0 / stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : atanf (1.0f / stack[last]->G->data[node]);
}

void grd_ACSC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACSC 1 1 acsc (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, |operand| > 1 for ACSC!\n");
	if (stack[last]->constant) a = (float)d_asin (1.0 / stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : d_asinf (1.0f / stack[last]->G->data[node]);
}

void grd_ADD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ADD 2 1 A + B.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	double a, b;

	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)(a + b);
	}
}

void grd_AND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: AND 2 1 B if A == NaN, else A.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	double a, b;

	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)((GMT_is_dnan (a)) ? b : a);
	}
}

void grd_ASEC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ASEC 1 1 asec (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, |operand| > 1 for ASEC!\n");
	if (stack[last]->constant) a = (float)d_acos (1.0 / stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : d_acosf (1.0f / stack[last]->G->data[node]);
}

void grd_ASIN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ASIN 1 1 asin (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, |operand| > 1 for ASIN!\n");
	if (stack[last]->constant) a = (float)d_asin (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : d_asinf (stack[last]->G->data[node]);
}

void grd_ASINH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ASINH 1 1 asinh (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)asinh (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : asinhf (stack[last]->G->data[node]);
}

void grd_ATAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ATAN 1 1 atan (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)atan (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : atanf (stack[last]->G->data[node]);
}

void grd_ATAN2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ATAN2 2 1 atan2 (A, B).  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	double a, b;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0 for ATAN2!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for ATAN2!\n");
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)d_atan2 (a, b);
	}
}

void grd_ATANH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ATANH 1 1 atanh (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && fabs (stack[last]->factor) >= 1.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, |operand| >= 1 for ATANH!\n");
	if (stack[last]->constant) a = (float)atanh (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : atanhf (stack[last]->G->data[node]);
}

void grd_BEI (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BEI 1 1 bei (A).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = GMT_bei (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : GMT_bei (GMT, fabs((double)stack[last]->G->data[node])));
}

void grd_BER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BER 1 1 ber (A).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = GMT_ber (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : GMT_ber (GMT, fabs ((double)stack[last]->G->data[node])));
}

void grd_BITAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITAND 2 1 A & B (bitwise AND operator).  */
{
	uint64_t node, n_warn = 0;
	float af = 0.0f, bf = 0.0f;
	unsigned int prev, a = 0, b = 0, result, result_trunc;

	prev = last - 1;
	if (stack[prev]->constant) af = (float)stack[prev]->factor;
	if (stack[last]->constant) bf = (float)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) af = stack[prev]->G->data[node];
		if (!stack[last]->constant) bf = stack[last]->G->data[node];
		if (GMT_is_fnan (af) || GMT_is_fnan (bf))	/* Any NaN in bitwise operations results in NaN output */
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;	b = (unsigned int)bf;
			result = a & b;
			result_trunc = (result & FLOAT_BIT_MASK);
			if (result != result_trunc) n_warn++;
			stack[prev]->G->data[node] = (float)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITAND resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");

}

void grd_BITLEFT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITLEFT 2 1 A << B (bitwise left-shift operator).  */
{
	uint64_t node, n_warn = 0;
	unsigned int prev, a = 0, b = 0, result, result_trunc;
	int b_signed;
	bool first = true;
	float af = 0.0f, bf = 0.0f;

	prev = last - 1;
	if (stack[prev]->constant) af = (float)stack[prev]->factor;
	if (stack[last]->constant) bf = (float)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) af = stack[prev]->G->data[node];
		if (!stack[last]->constant) bf = stack[last]->G->data[node];
		if (GMT_is_fnan (af) || GMT_is_fnan (bf))	/* Any NaN in bitwise operations results in NaN output */
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;	b_signed = (int)bf;
			if (b_signed < 0) {	/* Bad bitshift */
				if (first) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "ERROR: Bit shift must be >= 0; other values yield NaN\n");
				stack[prev]->G->data[node] = GMT->session.f_NaN;
				first = false;
			}
			else {
				b = (unsigned int)b_signed;
				result = a << b;
				result_trunc = (result & FLOAT_BIT_MASK);
				if (result != result_trunc) n_warn++;
				stack[prev]->G->data[node] = (float) result_trunc;
			}
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITLEFT resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");
}

void grd_BITNOT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITNOT 1 1  ~A (bitwise NOT operator, i.e., return two's complement).  */
{
	uint64_t node, n_warn = 0;
	unsigned int a = 0, result, result_trunc;
	float af = 0.0f;

	if (stack[last]->constant) af = (float)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[last]->constant) af = stack[last]->G->data[node];
		if (GMT_is_fnan (af))	/* Any NaN in bitwise operations results in NaN output */
			stack[last]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;
			result = ~a;
			result_trunc = (result & FLOAT_BIT_MASK);
			if (result != result_trunc) n_warn++;
			stack[last]->G->data[node] = (float)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITNOT resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");
}

void grd_BITOR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITOR 2 1 A | B (bitwise OR operator).  */
{
	uint64_t node, n_warn = 0;
	unsigned int prev, a = 0, b = 0, result, result_trunc;
	float af = 0.0f, bf = 0.0f;

	prev = last - 1;
	if (stack[prev]->constant) af = (float)stack[prev]->factor;
	if (stack[last]->constant) bf = (float)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) af = stack[prev]->G->data[node];
		if (!stack[last]->constant) bf = stack[last]->G->data[node];
		if (GMT_is_fnan (af) || GMT_is_fnan (bf))	/* Any NaN in bitwise operations results in NaN output */
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;	b = (unsigned int)bf;
			result = a | b;
			result_trunc = (result & FLOAT_BIT_MASK);
			if (result != result_trunc) n_warn++;
			stack[prev]->G->data[node] = (float)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITOR resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");
}

void grd_BITRIGHT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITRIGHT 2 1 A >> B (bitwise right-shift operator).  */
{
	uint64_t node, n_warn = 0;
	unsigned int prev, a = 0, b = 0, result, result_trunc;
	int b_signed;
	bool first = true;
	float af = 0.0f, bf = 0.0f;

	prev = last - 1;
	if (stack[prev]->constant) af = (float)stack[prev]->factor;
	if (stack[last]->constant) bf = (float)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) af = stack[prev]->G->data[node];
		if (!stack[last]->constant) bf = stack[last]->G->data[node];
		if (GMT_is_fnan (af) || GMT_is_fnan (bf))	/* Any NaN in bitwise operations results in NaN output */
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;	b_signed = (int)bf;
			if (b_signed < 0) {	/* Bad bitshift */
				if (first) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "ERROR: Bit shift must be >= 0; other values yield NaN\n");
				stack[prev]->G->data[node] = GMT->session.f_NaN;
				first = false;
			}
			else {
				b = (unsigned int)b_signed;
				result = a >> b;
				result_trunc = (result & FLOAT_BIT_MASK);
				if (result != result_trunc) n_warn++;
				stack[prev]->G->data[node] = (float)result_trunc;
			}
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITRIGHT resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");
}

void grd_BITTEST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITTEST 2 1 1 if bit B of A is set, else 0 (bitwise TEST operator).  */
{
	uint64_t node, n_warn = 0;
	unsigned int prev, a = 0, b = 0, result, result_trunc;
	int b_signed;
	bool first = true;
	float af = 0.0f, bf = 0.0f;

	prev = last - 1;
	if (stack[prev]->constant) af = (float)stack[prev]->factor;
	if (stack[last]->constant) bf = (float)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) af = stack[prev]->G->data[node];
		if (!stack[last]->constant) bf = stack[last]->G->data[node];
		if (GMT_is_fnan (af) || GMT_is_fnan (bf))	/* Any NaN in bitwise operations results in NaN output */
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;	b_signed = (int)bf;
			if (b_signed < 0) {	/* Bad bit */
				if (first) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "ERROR: Bit position range for BITTEST is 0-31 ; other values yield NaN\n");
				stack[prev]->G->data[node] = GMT->session.f_NaN;
				first = false;
			}
			else {
				b = (unsigned int)b_signed;
				b = 1U << b;
				result = a & b;
				result_trunc = (result & FLOAT_BIT_MASK);
				if (result != result_trunc) n_warn++;
				stack[prev]->G->data[node] = (result_trunc) ? 1.0f : 0.0f;
			}
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITTEST resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");
}

void grd_BITXOR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITXOR 2 1 A ^ B (bitwise XOR operator).  */
{
	uint64_t node, n_warn = 0;
	unsigned int prev, a = 0, b = 0, result, result_trunc;
	float af = 0.0f, bf = 0.0f;

	prev = last - 1;
	if (stack[prev]->constant) af = (float)stack[prev]->factor;
	if (stack[last]->constant) bf = (float)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) af = stack[prev]->G->data[node];
		if (!stack[last]->constant) bf = stack[last]->G->data[node];
		if (GMT_is_fnan (af) || GMT_is_fnan (bf))	/* Any NaN in bitwise operations results in NaN output */
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;	b = (unsigned int)bf;
			result = a ^ b;
			result_trunc = (result & FLOAT_BIT_MASK);
			if (result != result_trunc) n_warn++;
			stack[prev]->G->data[node] = (float)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: BITXOR resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");
}

void grd_CAZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CAZ 2 1 Cartesian azimuth from grid nodes to stack x,y.  */
{
	uint64_t node, row, col;
	unsigned int prev = last - 1;
	double x, y, az;

	GMT_grd_padloop (GMT, info->G, row, col, node) {
		x = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		y = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		az = (90.0 - atan2d (y - info->d_grd_y[row], x - info->d_grd_x[col]));
		while (az < -180.0) az += 360.0;
		while (az > +180.0) az -= 360.0;
		stack[prev]->G->data[node] = (float)az;
	}
}

void grd_CBAZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CBAZ 2 1 Cartesian backazimuth from grid nodes to stack x,y.  */
{
	uint64_t node, row, col;
	unsigned int prev = last - 1;
	double x, y, az;

	GMT_grd_padloop (GMT, info->G, row, col, node) {
		x = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		y = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		az = (270.0 - atan2d (y - info->d_grd_y[row], x - info->d_grd_x[col]));
		while (az < -180.0) az += 360.0;
		while (az > +180.0) az -= 360.0;
		stack[prev]->G->data[node] = (float)az;
	}
}

void grd_CDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CDIST 2 1 Cartesian distance between grid nodes and stack x,y.  */
{
	uint64_t node, row, col;
	unsigned int prev = last - 1;
	double a, b;

	if (GMT_is_geographic (GMT, GMT_IN)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "CDIST Error: Grid must be Cartesian; see SDIST for geographic data.\n");
		return;
	}
	GMT_grd_padloop (GMT, info->G, row, col, node) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)hypot (a - info->d_grd_x[col], b - info->d_grd_y[row]);
	}
}

void grd_CDIST2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CDIST2 2 1 As CDIST but only to nodes that are != 0.  */
{
	uint64_t node, row, col;
	unsigned int prev = last - 1;
	double a, b;

	if (GMT_is_geographic (GMT, GMT_IN)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "CDIST2 Error: Grid must be Cartesian; see SDIST2 for geographic data.\n");
		return;
	}
	GMT_grd_padloop (GMT, info->G, row, col, node) {
		if (stack[prev]->G->data[node] == 0.0)
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
			stack[prev]->G->data[node] = (float)hypot (a - info->d_grd_x[col], b - info->d_grd_y[row]);
		}
	}
}

void grd_CEIL (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CEIL 1 1 ceil (A) (smallest integer >= A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)ceil (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : ceilf (stack[last]->G->data[node]);
}

void grd_CHICRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CHICRIT 2 1 Critical value for chi-squared-distribution, with alpha = A and n = B.  */
{
	uint64_t node;
	unsigned int prev = last - 1, row, col;
	double a, b;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0 for CHICRIT!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for CHICRIT!\n");
	GMT_grd_loop (GMT, info->G, row, col, node) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)GMT_chi2crit (GMT, a, b);
	}
}

void grd_CHIDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CHIDIST 2 1 chi-squared-distribution P(chi2,n), with chi2 = A and n = B.  */
{
	uint64_t node;
	unsigned int prev = last - 1, row, col;
	double a, b, prob;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0 for CHIDIST!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for CHIDIST!\n");
	GMT_grd_loop (GMT, info->G, row, col, node) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		GMT_chi2 (GMT, a, b, &prob);
		stack[prev]->G->data[node] = (float)prob;
	}
}

void grd_CORRCOEFF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CORRCOEFF 2 1 Correlation coefficient r(A, B).  */
{
	uint64_t node;
	unsigned int prev = last - 1, pad[4];
	double coeff;

	if (stack[prev]->constant || stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: constant operands for CORRCOEFF yields NaNs\n");
		for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = GMT->session.f_NaN;
		return;
	}
	GMT_memcpy (pad, stack[last]->G->header->pad, 4, unsigned int);	/* Save original pad */
	GMT_grd_pad_off (GMT, stack[prev]->G);				/* Undo pad if one existed so we can sort */
	GMT_grd_pad_off (GMT, stack[last]->G);				/* Undo pad if one existed so we can sort */
	coeff = GMT_corrcoeff_f (GMT, stack[prev]->G->data, stack[last]->G->data, info->nm, 0);
	GMT_grd_pad_on (GMT, stack[prev]->G, pad);		/* Reinstate the original pad */
	GMT_grd_pad_on (GMT, stack[last]->G, pad);		/* Reinstate the original pad */
	for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = (float)coeff;
}

void grd_COS (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: COS 1 1 cos (A) (A in radians).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)cos (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : cosf (stack[last]->G->data[node]);
}

void grd_COSD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: COSD 1 1 cos (A) (A in degrees).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = cosd (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : cosd (stack[last]->G->data[node]));
}

void grd_COSH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: COSH 1 1 cosh (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)cosh (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : coshf (stack[last]->G->data[node]);
}

void grd_COT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: COT 1 1 cot (A) (A in radians).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = 1.0 / tan (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : (1.0 / tan (stack[last]->G->data[node])));
}

void grd_COTD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: COTD 1 1 cot (A) (A in degrees).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = 1.0 / tand (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : 1.0 / tand (stack[last]->G->data[node]));
}

void grd_CPOISS (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CPOISS 2 1 Cumulative Poisson distribution F(x,lambda), with x = A and lambda = B.  */
{
	uint64_t node;
	unsigned int prev = last - 1, row, col;
	double a, b, prob;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for CHIDIST!\n");
	GMT_grd_loop (GMT, info->G, row, col, node) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		GMT_cumpoisson (GMT, a, b, &prob);
		stack[prev]->G->data[node] = (float)prob;
	}
}

void grd_CSC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CSC 1 1 csc (A) (A in radians).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = 1.0 / sin (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : 1.0 / sinf (stack[last]->G->data[node]));
}

void grd_CSCD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CSCD 1 1 csc (A) (A in degrees).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = 1.0 / sind (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : 1.0 / sind (stack[last]->G->data[node]));
}

void grd_CURV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CURV 1 1 Curvature of A (Laplacian).  */
{
	uint64_t node;
	unsigned int row, col, mx;
	double cy, *cx = NULL;
	float *z = NULL;

	/* Curvature (Laplacian). */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand to CURV is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	/* If grid does not have BC rows/cols assigned we apply reasonable conditions:
	 * If -fg we assume geographic grid and use geographic BCs, else we use natural BCs. If the grid
	 * as a BC == GMT_BC_IS_DATA then the pad already constains observations. */

	GMT_BC_init (GMT, stack[last]->G->header);	/* Initialize grid interpolation and boundary condition parameters */
	GMT_grd_BC_set (GMT, stack[last]->G, GMT_IN);	/* Set boundary conditions */

	/* Now, stack[last]->G->data has boundary rows/cols all set according to the boundary conditions (or actual data).
	 * We can then operate on the interior of the grid and temporarily assign values to the z grid */

	z = GMT_memory (GMT, NULL, info->size, float);
	cx = GMT_memory (GMT, NULL, info->G->header->ny, double);
	GMT_row_loop (GMT, info->G, row) cx[row] = 1.0 / (info->dx[row] * info->dx[row]);

	mx = info->G->header->mx;
	cy = 1.0 / (info->dy * info->dy);

	GMT_grd_loop (GMT, info->G, row, col, node) {
		z[node] = (float)(cx[row] * (stack[last]->G->data[node+1] - 2.0 * stack[last]->G->data[node] + stack[last]->G->data[node-1]) + \
			cy * (stack[last]->G->data[node+mx] - 2.0 * stack[last]->G->data[node] + stack[last]->G->data[node-mx]));
	}

	GMT_memcpy (stack[last]->G->data, z, info->size, float);
	GMT_free (GMT, z);
	GMT_free (GMT, cx);
}

void grd_D2DX2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: D2DX2 1 1 d^2(A)/dx^2 2nd derivative.  */
{
	uint64_t node, ij;
	unsigned int row, col;
	double c, left, next_left;

	/* Central 2nd difference in x */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand to D2DX2 is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	GMT_row_loop (GMT, info->G, row) {	/* Process d2/dx2 row by row since dx may change with row */
		c = 1.0 / (info->dx[row] * info->dx[row]);
		/* Unless pad has real data we assign outside col values via natural BCs */
		ij = GMT_IJP (info->G->header, row, 0);	/* First col */
		if (stack[last]->G->header->BC[XLO] != GMT_BC_IS_DATA)
			stack[last]->G->data[ij-1] = (float)(2.0 * stack[last]->G->data[ij] - stack[last]->G->data[ij+1]);	/* Set left node via BC curv = 0 */
		next_left = stack[last]->G->data[ij-1];
		ij = GMT_IJP (info->G->header, row, info->G->header->nx-1);	/* Last col */
		if (stack[last]->G->header->BC[XHI] != GMT_BC_IS_DATA)
			stack[last]->G->data[ij+1] = (float)(2.0 * stack[last]->G->data[ij] - stack[last]->G->data[ij-1]);	/* Set right node via BC curv = 0 */
		GMT_col_loop (GMT, info->G, row, col, node) {	/* Loop over cols; always save the next left before we update the array at that col */
			left = next_left;
			next_left = stack[last]->G->data[node];
			stack[last]->G->data[node] = (float)(c * (stack[last]->G->data[node+1] - 2.0 * stack[last]->G->data[node] + left));
		}
	}
	GMT_grd_pad_zero (GMT, stack[last]->G);	/* Reset the boundary pad */
}

void grd_D2DY2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: D2DY2 1 1 d^2(A)/dy^2 2nd derivative.  */
{
	uint64_t node, ij;
	unsigned int row, col, mx;
	double c, bottom, next_bottom;

	/* Central 2nd difference in y */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand to D2DY2 is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	c = 1.0 / (info->dy * info->dy);
	mx = info->G->header->mx;
	GMT_col_loop (GMT, info->G, 0, col, node) {	/* Process d2/dy2 column by column */
		/* Unless pad has real data we assign outside row values via natural BCs */
		if (stack[last]->G->header->BC[YHI] != GMT_BC_IS_DATA)
			stack[last]->G->data[node-mx] = (float)(2.0 * stack[last]->G->data[node] - stack[last]->G->data[node+mx]);	/* Set top node via BC curv = 0 */
		next_bottom = stack[last]->G->data[node-mx];
		ij = GMT_IJP (info->G->header, info->G->header->ny-1, col);	/* Last row for this column */
		if (stack[last]->G->header->BC[YLO] != GMT_BC_IS_DATA)
			stack[last]->G->data[ij+mx] = (float)(2.0 * stack[last]->G->data[ij] - stack[last]->G->data[ij-mx]);	/* Set bottom node via BC curv = 0 */
		GMT_row_loop (GMT, info->G, row) { /* Cannot use node inside here and must get ij separately */
			ij = GMT_IJP (info->G->header, row, col);	/* current node in this column */
			bottom = next_bottom;
			next_bottom = stack[last]->G->data[ij];
			stack[last]->G->data[ij] = (float)(c * (stack[last]->G->data[ij+mx] - 2.0 * stack[last]->G->data[ij] + bottom));
		}
	}
	GMT_grd_pad_zero (GMT, stack[last]->G);	/* Reset the boundary pad */
}

void grd_D2DXY (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: D2DXY 1 1 d^2(A)/dxdy 2nd derivative.  */
{
	uint64_t node;
	unsigned int row, col, mx;
	double *cx = NULL, cy;
	float *z = NULL;

	/* Cross derivative d2/dxy = d2/dyx  */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand to D2DXY is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	/* If grid does not have BC rows/cols assigned we apply reasonable conditions:
	 * If -fg we assume geographic grid and use geographic BCs, else we use natural BCs. If the grid
	 * as a BC == GMT_BC_IS_DATA then the pad already constains observations. */

	GMT_BC_init (GMT, stack[last]->G->header);	/* Initialize grid interpolation and boundary condition parameters */
	GMT_grd_BC_set (GMT, stack[last]->G, GMT_IN);	/* Set boundary conditions */

	/* Now, stack[last]->G->data has boundary rows/cols all set according to the boundary conditions (or actual data).
	 * We can then operate on the interior of the grid and temporarily assign values to the z grid */

	z = GMT_memory (GMT, NULL, info->size, float);
	cx = GMT_memory (GMT, NULL, info->G->header->ny, double);
	GMT_row_loop (GMT, info->G, row) cx[row] = 0.5 / (info->dx[row] * info->dx[row]);

	mx = info->G->header->mx;
	cy = 0.5 / (info->dy * info->dy);

	GMT_grd_loop (GMT, info->G, row, col, node) {
		z[node] = (float)(cx[row] * cy * (stack[last]->G->data[node-mx+1] - stack[last]->G->data[node-mx-1] + \
			stack[last]->G->data[node+mx-1] - stack[last]->G->data[node+mx+1]));
	}

	GMT_memcpy (stack[last]->G->data, z, info->size, float);
	GMT_free (GMT, z);
	GMT_free (GMT, cx);
}

void grd_D2R (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: D2R 1 1 Converts Degrees to Radians.  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = stack[last]->factor * D2R;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : (stack[last]->G->data[node] * D2R));
}

void grd_DDX (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DDX 1 1 d(A)/dx Central 1st derivative.  */
{
	uint64_t node, ij;
	unsigned int row, col;
	double c, left, next_left;

	/* Central 1st difference in x */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand to DDX is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	GMT_row_loop (GMT, info->G, row) {	/* Process d/dx row by row since dx may change with row */
		c = 0.5 / info->dx[row];
		/* Unless pad has real data we assign outside col values via natural BCs */
		ij = GMT_IJP (info->G->header, row, 0);	/* First col */
		if (stack[last]->G->header->BC[XLO] != GMT_BC_IS_DATA)
			stack[last]->G->data[ij-1] = (float)(2.0 * stack[last]->G->data[ij] - stack[last]->G->data[ij+1]);	/* Set left node via BC curv = 0 */
		next_left = stack[last]->G->data[ij-1];
		ij = GMT_IJP (info->G->header, row, info->G->header->nx-1);	/* Last col */
		if (stack[last]->G->header->BC[XHI] != GMT_BC_IS_DATA)
			stack[last]->G->data[ij+1] = (float)(2.0 * stack[last]->G->data[ij] - stack[last]->G->data[ij-1]);	/* Set right node via BC curv = 0 */
		GMT_col_loop (GMT, info->G, row, col, node) {	/* Loop over cols; always save the next left before we update the array at that col */
			left = next_left;
			next_left = stack[last]->G->data[node];
			stack[last]->G->data[node] = (float)(c * (stack[last]->G->data[node+1] - left));
		}
	}
}

void grd_DDY (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DDY 1 1 d(A)/dy Central 1st derivative.  */
{
	uint64_t node, ij;
	unsigned int row, col, mx;
	double c, bottom, next_bottom;

	/* Central 1st difference in y */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand to DDY is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	c = -0.5 / info->dy;	/* Because the loop over j below goes from ymax to ymin we compensate with a minus sign here */
	mx = info->G->header->mx;
	GMT_col_loop (GMT, info->G, 0, col, node) {	/* Process d/dy column by column */
		/* Unless pad has real data we assign outside row values via natural BCs */
		if (stack[last]->G->header->BC[YHI] != GMT_BC_IS_DATA) 	/* Set top node via BC curv = 0 */
			stack[last]->G->data[node-mx] = (float)(2.0 * stack[last]->G->data[node] - stack[last]->G->data[node+mx]);
		next_bottom = stack[last]->G->data[node-mx];
		ij = GMT_IJP (info->G->header, info->G->header->ny-1, col);	/* Last row for this column */
		if (stack[last]->G->header->BC[YLO] != GMT_BC_IS_DATA) 	/* Set bottom node via BC curv = 0 */
			stack[last]->G->data[ij+mx] = (float)(2.0 * stack[last]->G->data[ij] - stack[last]->G->data[ij-mx]);
		GMT_row_loop (GMT, info->G, row) {
			ij = GMT_IJP (info->G->header, row, col);	/* current node in this column */
			bottom = next_bottom;
			next_bottom = stack[last]->G->data[ij];
			stack[last]->G->data[ij] = (float)(c * (stack[last]->G->data[ij+mx] - bottom));
		}
	}
	GMT_grd_pad_zero (GMT, stack[last]->G);	/* Reset the boundary pad */
}

void grd_DEG2KM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DEG2KM 1 1 Converts Spherical Degrees to Kilometers.  */
{
	uint64_t node;
	double a = 0.0;

	if (GMT_is_geographic (GMT, GMT_IN)) {
		if (GMT_sph_mode (GMT) == GMT_GEODESIC) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, DEG2KM is only exact when PROJ_ELLIPSOID == sphere\n");
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, DEG2KM used with Cartesian data\n");
	if (stack[last]->constant) a = stack[last]->factor * GMT->current.proj.DIST_KM_PR_DEG;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : stack[last]->G->data[node] * GMT->current.proj.DIST_KM_PR_DEG);
}

void grd_DILOG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DILOG 1 1 dilog (A).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = GMT_dilog (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : GMT_dilog (GMT, stack[last]->G->data[node]));
}

void grd_DIV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DIV 2 1 A / B.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	double a, b;
	void grd_MUL (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last);

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Divide by zero gives NaNs\n");
	if (stack[last]->constant) {	/* Turn divide into multiply */
		a = stack[last]->factor;	/* Save original factor */
		stack[last]->factor = 1.0 / stack[last]->factor;
		grd_MUL (GMT, info, stack, last);
		stack[last]->factor = a;	/* Restore factor */
		return;
	}

	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)(a / b);
	}
}

void grd_DUP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DUP 1 2 Places duplicate of A on the stack.  */
{
	uint64_t node;
	unsigned int next;

	next = last + 1;
	stack[next]->constant = stack[last]->constant;
	stack[next]->factor = stack[last]->factor;
	if (stack[last]->constant) {	/* Time to fess up */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)stack[last]->factor;
	}

	GMT_memcpy (stack[next]->G->data, stack[last]->G->data, info->size, float);
}

void grd_ERF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ERF 1 1 Error function erf (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)erf (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : erff (stack[last]->G->data[node]);
}

void grd_ERFC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ERFC 1 1 Complementary Error function erfc (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)erfc (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : erfcf (stack[last]->G->data[node]);
}

void grd_EQ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: EQ 2 1 1 if A == B, else 0.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	float a, b;

	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (float)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (float)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (GMT_is_fnan (a) || GMT_is_fnan (b)) ? GMT->session.f_NaN : (float)(a == b);
	}
}

void grd_ERFINV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ERFINV 1 1 Inverse error function of A.  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = GMT_erfinv (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : GMT_erfinv (GMT, stack[last]->G->data[node]));
}

void grd_EXCH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: EXCH 2 2 Exchanges A and B on the stack.  */
{
	uint64_t node;
	unsigned int prev = last - 1;

	for (node = 0; node < info->size; node++) {
		if (stack[prev]->constant) stack[prev]->G->data[node] = (float)stack[prev]->factor;
		if (stack[last]->constant) stack[last]->G->data[node] = (float)stack[last]->factor;
		float_swap (stack[last]->G->data[node], stack[prev]->G->data[node]);
	}
	double_swap (stack[last]->factor, stack[prev]->factor);
	uint_swap (stack[last]->alloc_mode, stack[prev]->alloc_mode);
	bool_swap (stack[last]->constant, stack[prev]->constant);
}

void grd_EXP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: EXP 1 1 exp (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)exp (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : expf (stack[last]->G->data[node]);
}

void grd_FACT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FACT 1 1 A! (A factorial).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant)
		a = GMT_factorial (GMT, irint(stack[last]->factor));
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : GMT_factorial (GMT, irint((double)stack[last]->G->data[node])));
}

/* Subroutines for grd_EXTREMA */

int do_derivative (float *z, uint64_t this_node, int off, unsigned int type)
{	/* Examine a line of 3-points centered on the current this_node.
	 * z is the data matrix.
	 * off is shift to add to get index of the next value and subtract to get previous node.
	 * type: 0 means x-, or -y derivative, 1 means diagonal (N45E or N135E direction)  */

	uint64_t next_node, prev_node;
	int nan_flag;

	nan_flag = (type == 0) ? -2 : 0;	/* Return -2 if we find two nans except for diagonals where we return 0 */

	/* Because of padding, all internal nodes have neighbors on either side (left, right, above, below) */

	prev_node = this_node - off;	/* Previous node in line */
	next_node = this_node + off;	/* Next node in line */
	if (GMT_is_fnan (z[prev_node])) {			/* At least one of the two neighbor points is a NaN */
		if (GMT_is_fnan (z[next_node])) return (nan_flag);	/* Both points are NaN, return -2 (or 0 if diagonal) */
		if (z[this_node] == z[next_node]) return (-2);	/* Flat line, no extrema possible */
		if (z[this_node] < z[next_node]) return (-1);	/* A local minimum */
		return (+1);					/* Else it must be a local maximum */
	}
	if (GMT_is_fnan (z[next_node])) {			/* One of the two neighbor points is a NaN */
		if (z[this_node] == z[prev_node]) return (-2);	/* Flat line, no extrema possible */
		if (z[this_node] < z[prev_node]) return (-1);	/* A local minimum */
		return (+1);					/* Else it must be a local maximum */
	}/* OK, no NaNs among the three nodes */
	if (z[this_node] == z[prev_node] && z[this_node] == z[next_node]) return (-2);	/* Flat line, no extrema possible */
	if (z[this_node] < z[prev_node] && z[this_node] < z[next_node]) return (-1);	/* A local minimum */
	if (z[this_node] > z[prev_node] && z[this_node] > z[next_node]) return (+1);	/* A local maximum */
	return (0);									/* No extrema found */
}

void grd_EXTREMA (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: EXTREMA 1 1 Local Extrema: +2/-2 is max/min, +1/-1 is saddle with max/min in x, 0 elsewhere.  */
{
	uint64_t node;
	unsigned int row, col;
	int dx, dy, diag, product, mx1;
	float *z = NULL;

	/* Find local extrema in grid */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand to EXTREMA is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	/* If grid does not have BC rows/cols assigned we apply reasonable conditions:
	 * If -fg we assume geographic grid and use geographic BCs, else we use natural BCs. If the grid
	 * as a BC == GMT_BC_IS_DATA then the pad already constains observations. */

	GMT_BC_init (GMT, stack[last]->G->header);	/* Initialize grid interpolation and boundary condition parameters */
	GMT_grd_BC_set (GMT, stack[last]->G, GMT_IN);	/* Set boundary conditions */

	/* Now, stack[last]->G->data has boundary rows/cols all set according to the boundary conditions (or actual data).
	 * We can then operate on the interior of the grid and temporarily assign values to the z grid */

	z = GMT_memory (GMT, NULL, info->size, float);

	/* We will visit each node on the grid and determine if there are extrema.  We do this
	 * by looking at the along-x and along-y profiles separately.  If both of them shows the
	 * central node in a min or max location with respect to its two neighbors (one if at the
	 * edge of the grid or in the presence of NaNs) we must do further checking.  If the two
	 * extrema are of different sign we call it a saddle point and we are done.  If both are
	 * min or max then we look at the two diagonal lines to see if they can confirm this.
	 * Here, NaNs are not held against you - it takes real values to overrule the dx,dy values.
	 *
	 * Min is given -2, Max is given +2, Saddle points are +1 (if max in x) or -1 (if max in y)
	 * Default is 0 which means no extrema.
	 */

	mx1 = info->G->header->mx + 1;

	GMT_grd_loop (GMT, info->G, row, col, node) {

		if (GMT_is_fnan (stack[last]->G->data[node])) continue;	/* No extrema if point is NaN */

		if ((dx = do_derivative (stack[last]->G->data, node, 1, 0)) == -2) continue;	/* Too many NaNs or flat x-line */
		if ((dy = do_derivative (stack[last]->G->data, node, info->G->header->mx, 0)) == -2) continue;	/* Too many NaNs or flat y-line */

		if ((product = dx * dy) == 0) continue;	/* No min or max possible */
		if (product < 0) {	/* Saddle point - don't need to check diagonals */
			z[node] = (float)((dx > 0) ? +1 : -1);
			continue;
		}

		/* Need to examine diagonal trends to verify min or max */

		if ((diag = do_derivative (stack[last]->G->data, node, -mx1, 1)) == -2) continue;	/* Sorry, no extrema along diagonal N45E */
		if (diag != 0 && diag != dx) continue;						/* Sorry, extrema of opposite sign along diagonal N45E  */
		if ((diag = do_derivative (stack[last]->G->data, node,  mx1, 1)) == -2) continue;	/* Sorry, no extrema along diagonal N135E */
		if (diag != 0 && diag != dx) continue;						/* Sorry, extrema of opposite sign along diagonal N135E  */

		/* OK, we have a min or max point; just use dx to check which kind */

		z[node] = (float)((dx > 0) ? +2 : -2);
	}

	GMT_memcpy (stack[last]->G->data, z, info->size, float);
	GMT_memset (stack[last]->G->header->BC, 4, unsigned int);	/* No BC padding in this array */
	GMT_free (GMT, z);
}

void grd_FCRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FCRIT 3 1 Critical value for F-distribution, with alpha = A, n1 = B, and n2 = C.  */
{
	uint64_t node;
	int nu1, nu2;
	unsigned int prev1, prev2, row, col;
	double alpha;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && stack[prev2]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0 for FCRIT!\n");
	if (stack[prev1]->constant && stack[prev1]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for FCRIT!\n");
	if (stack[last]->constant  && stack[last]->factor  == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand three == 0 for FCRIT!\n");
	GMT_grd_loop (GMT, info->G, row, col, node) {
		alpha = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		nu1 = irint ((stack[prev1]->constant) ? stack[prev1]->factor : (double)stack[prev1]->G->data[node]);
		nu2 = irint ((stack[last]->constant)  ? stack[last]->factor  : (double)stack[last]->G->data[node]);
		stack[prev2]->G->data[node] = (float)GMT_Fcrit (GMT, alpha, nu1, nu2);
	}
}

void grd_FDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FDIST 3 1 F-distribution Q(F,n1,n2), with F = A, n1 = B, and n2 = C.  */
{
	uint64_t node;
	int nu1, nu2;
	unsigned int prev1, prev2, row, col;
	double F, chisq1, chisq2 = 1.0, prob;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev1]->constant && stack[prev1]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for FDIST!\n");
	if (stack[last]->constant  && stack[last]->factor  == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand three == 0 for FDIST!\n");
	GMT_grd_loop (GMT, info->G, row, col, node) {
		F = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		nu1 = irint ((stack[prev1]->constant) ? stack[prev1]->factor : (double)stack[prev1]->G->data[node]);
		nu2 = irint ((stack[last]->constant)  ? stack[last]->factor  : (double)stack[last]->G->data[node]);
		/* Since GMT_f_q needs chisq1 and chisq2, we set chisq2 = 1 and solve for chisq1 */
		chisq1 = F * nu1 / nu2;
		(void) GMT_f_q (GMT, chisq1, nu1, chisq2, nu2, &prob);
		stack[prev2]->G->data[node] = (float)prob;
	}
}

void grd_FLIPLR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FLIPLR 1 1 Reverse order of values in each row.  */
{
	uint64_t node;
	unsigned int mx1, row, col_l, col_r, mx_half;

	/* Reverse order of all rows */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand to FLIPLR is constant!\n");
		return;
	}

	/* This must also apply to the pads since any BCs there must be flipped as well, hence a local loop is used */

	mx_half = info->G->header->mx / 2;
	mx1 = info->G->header->mx - 1;
	for (node = row = 0; row < info->G->header->my; row++, node += info->G->header->mx) {	/* Do this to all rows */
		for (col_l = 0, col_r = mx1; col_l < mx_half; col_l++, col_r--) float_swap (stack[last]->G->data[node+col_l], stack[last]->G->data[node+col_r]);
	}
}

void grd_FLIPUD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FLIPUD 1 1 Reverse order of values in each column.  */
{
	unsigned int my1, mx, row_t, row_b, col, my_half;

	/* Reverse order of all columns */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand to FLIPLR is constant!\n");
		return;
	}

	/* This must also apply to the pads since any BCs there must be flipped as well, hence a local loop is used */

	my_half = info->G->header->my / 2;
	my1 = info->G->header->my - 1;
	mx = info->G->header->mx;
	for (col = 0; col < mx; col++) {	/* Do this to all cols */
		for (row_t = 0, row_b = my1; row_t < my_half; row_t++, row_b--) float_swap (stack[last]->G->data[(uint64_t)row_t*(uint64_t)mx+(uint64_t)col], stack[last]->G->data[(uint64_t)row_b*(uint64_t)mx+(uint64_t)col]);
	}
}

void grd_FLOOR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FLOOR 1 1 floor (A) (greatest integer <= A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)floor (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : floorf (stack[last]->G->data[node]);
}

void grd_FMOD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FMOD 2 1 A % B (remainder after truncated division).  */
{
	uint64_t node;
	unsigned int prev;
	double a, b;

	prev = last - 1;
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, using FMOD 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)fmod (a, b);
	}
}

void grd_GE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: GE 2 1 1 if A >= B, else 0.  */
{
	uint64_t node;
	unsigned int prev;
	float a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (float)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (float)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (GMT_is_fnan (a) || GMT_is_fnan (b)) ? GMT->session.f_NaN : (float)(a >= b);
	}
}

void grd_GT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: GT 2 1 1 if A > B, else 0.  */
{
	uint64_t node;
	unsigned int prev;
	float a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (float)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (float)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (GMT_is_fnan (a) || GMT_is_fnan (b)) ? GMT->session.f_NaN : (float)(a > b);
	}
}

void grd_HYPOT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: HYPOT 2 1 hypot (A, B) = sqrt (A*A + B*B).  */
{
	uint64_t node;
	unsigned int prev;
	double a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)hypot (a, b);
	}
}

void grd_I0 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: I0 1 1 Modified Bessel function of A (1st kind, order 0).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = GMT_i0 (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : GMT_i0 (GMT, stack[last]->G->data[node]));
}

void grd_I1 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: I1 1 1 Modified Bessel function of A (1st kind, order 1).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = GMT_i1 (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : GMT_i1 (GMT, stack[last]->G->data[node]));
}

void grd_IFELSE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: IFELSE 3 1 B if A != 0, else C.  */
{
	uint64_t node;
	unsigned int prev1, prev2;
	float a = 0.0f, b = 0.0f, c = 0.0f;

	/* last is C */
	prev1 = last - 1;	/* This is B */
	prev2 = last - 2;	/* This is A */

	/* Set to B if A == 1 else set to C
	 * A, B, or C = NaN, in which case we set answer to NaN */

	if (stack[prev2]->constant) a = (float)stack[prev2]->factor;
	if (stack[prev1]->constant) b = (float)stack[prev1]->factor;
	if (stack[last]->constant)  c = (float)stack[last]->factor;

	for (node = 0; node < info->size; node++) {
		if (!stack[prev2]->constant) a = stack[prev2]->G->data[node];
		if (!stack[prev1]->constant) b = stack[prev1]->G->data[node];
		if (!stack[last]->constant)  c = stack[last]->G->data[node];

		stack[prev2]->G->data[node] = (fabsf (a) < GMT_CONV_LIMIT) ? c : b;
	}
}

void grd_IN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: IN 2 1 Modified Bessel function of A (1st kind, order B).  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	int order = 0;
	bool simple = false;
	float b = 0.0f;

	if (stack[last]->constant) {
		if (stack[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order < 0 for IN!\n");
		if (fabs (rint(stack[last]->factor) - stack[last]->factor) > GMT_SMALL) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order not an integer for IN!\n");
		order = urint (fabs (stack[last]->factor));
		if (stack[prev]->constant) {
			b = (float)GMT_in (GMT, order, fabs (stack[prev]->factor));
			simple = true;
		}
	}
	for (node = 0; node < info->size; node++) {
		if (simple)
			stack[prev]->G->data[node] = b;
		else {
			if (!stack[last]->constant) order = urint (fabs (stack[last]->G->data[node]));
			stack[last]->G->data[node] = (float)GMT_in (GMT, order, fabs ((double)stack[prev]->G->data[node]));
		}
	}
}

void grd_INRANGE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: INRANGE 3 1 1 if B <= A <= C, else 0.  */
{
	uint64_t node;
	unsigned int prev1, prev2;
	float a = 0.0f, b = 0.0f, c = 0.0f, inrange;

	/* last is C */
	prev1 = last - 1;	/* This is B */
	prev2 = last - 2;	/* This is A */

	/* Set to 1 where B <= A <= C, 0 elsewhere, except where
	 * A, B, or C = NaN, in which case we set answer to NaN */

	if (stack[prev2]->constant) a = (float)stack[prev2]->factor;
	if (stack[prev1]->constant) b = (float)stack[prev1]->factor;
	if (stack[last]->constant)  c = (float)stack[last]->factor;

	for (node = 0; node < info->size; node++) {
		if (!stack[prev2]->constant) a = stack[prev2]->G->data[node];
		if (!stack[prev1]->constant) b = stack[prev1]->G->data[node];
		if (!stack[last]->constant)  c = stack[last]->G->data[node];

		if (GMT_is_fnan (a) || GMT_is_fnan (b) || GMT_is_fnan (c)) {
			stack[prev2]->G->data[node] = GMT->session.f_NaN;
			continue;
		}

		inrange = (b <= a && a <= c) ? 1.0f : 0.0f;
		stack[prev2]->G->data[node] = inrange;
	}
}

void grd_INSIDE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: INSIDE 1 1 1 when inside or on polygon(s) in A, else 0.  */
{	/* Suitable for geographic (lon, lat) data and polygons */
	uint64_t node, seg, row, col;
	unsigned int inside;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;

	GMT_set_cols (GMT, GMT_IN, 2);
	GMT_skip_xy_duplicates (GMT, true);	/* Avoid repeating x/y points in polygons */
	if ((D = GMT_Read_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, GMT_READ_NORMAL, NULL, info->ASCII_file, NULL)) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in operator INSIDE reading file %s!\n", info->ASCII_file);
		info->error = GMT->parent->error;
		return;
	}
	GMT_skip_xy_duplicates (GMT, false);	/* Reset */
	T = D->table[0];	/* Only one table in a single file */
	GMT_grd_padloop (GMT, info->G, row, col, node) {	/* Visit each node */
		for (seg = inside = 0; !inside && seg < T->n_segments; seg++) {
			S = T->segment[seg];
			if (GMT_polygon_is_hole (S)) continue;	/* Holes are handled within GMT_inonout */
			inside = GMT_inonout (GMT, info->d_grd_x[col], info->d_grd_y[row], S);
		}
		stack[last]->G->data[node] = (inside) ? 1.0f : 0.0f;
	}

	/* Free memory used for pol */

	if (GMT_Destroy_Data (GMT->parent, &D) != GMT_OK) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in operator INSIDE destroying allocated data from %s!\n", info->ASCII_file);
		info->error = GMT->parent->error;
		return;
	}
}

void grd_INV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: INV 1 1 1 / A.  */
{
	uint64_t node;
	double a;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: inverse of zero gives NaNs\n");
	if (stack[last]->constant) stack[last]->factor = (stack[last]->factor == 0.0) ? GMT->session.f_NaN : 1.0 / stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		a = (stack[last]->constant) ? stack[last]->factor : 1.0 / stack[last]->G->data[node];
		stack[last]->G->data[node] = (float)a;
	}
}

void grd_ISFINITE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ISFINITE 1 1 1 if A is finite, else 0.  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)isfinite (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : isfinite (stack[last]->G->data[node]);
}

void grd_ISNAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ISNAN 1 1 1 if A == NaN, else 0.  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)GMT_is_fnan (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : GMT_is_fnan (stack[last]->G->data[node]);
}

void grd_J0 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: J0 1 1 Bessel function of A (1st kind, order 0).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)j0 (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (float)j0 (stack[last]->G->data[node]);
}

void grd_J1 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: J1 1 1 Bessel function of A (1st kind, order 1).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)j1 (fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (float)j1 (fabsf (stack[last]->G->data[node]));
}

void grd_JN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: JN 2 1 Bessel function of A (1st kind, order B).  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	int order = 0;
	bool simple = false;
	float b = 0.0f;

	if (stack[last]->constant) {
		if (stack[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order < 0 for JN!\n");
		if (fabs (rint(stack[last]->factor) - stack[last]->factor) > GMT_SMALL) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order not an integer for JN!\n");
		order = urint (fabs (stack[last]->factor));
		if (stack[prev]->constant) {
			b = (float)jn (order, fabs (stack[prev]->factor));
			simple = true;
		}
	}
	for (node = 0; node < info->size; node++) {
		if (simple)
			stack[prev]->G->data[node] = b;
		else {
			if (!stack[last]->constant) order = urint (fabsf (stack[last]->G->data[node]));
			stack[last]->G->data[node] = (float)jn (order, fabsf (stack[prev]->G->data[node]));
		}
	}
}

void grd_K0 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: K0 1 1 Modified Kelvin function of A (2nd kind, order 0).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)GMT_k0 (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (float)GMT_k0 (GMT, stack[last]->G->data[node]);
}

void grd_K1 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: K1 1 1 Modified Bessel function of A (2nd kind, order 1).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)GMT_k1 (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (float)GMT_k1 (GMT, stack[last]->G->data[node]);
}

void grd_KEI (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: KEI 1 1 kei (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)GMT_kei (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (float)GMT_kei (GMT, fabsf (stack[last]->G->data[node]));
}

void grd_KER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: KER 1 1 ker (A).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = GMT_ker (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : GMT_ker (GMT, fabsf (stack[last]->G->data[node])));
}

void grd_KM2DEG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: KM2DEG 1 1 Converts Kilometers to Spherical Degrees.  */
{
	uint64_t node;
	double a = 0.0, f = 1.0 / GMT->current.proj.DIST_KM_PR_DEG;

	if (GMT_is_geographic (GMT, GMT_IN)) {
		if (GMT_sph_mode (GMT) == GMT_GEODESIC) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, KM2DEG is only exact when PROJ_ELLIPSOID == sphere\n");
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, KM2DEG used with Cartesian data\n");
	if (stack[last]->constant) a = stack[last]->factor * f;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : (stack[last]->G->data[node] * f));
}

void grd_KN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: KN 2 1 Modified Bessel function of A (2nd kind, order B).  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	int order = 0;
	bool simple = false;
	float b = 0.0f;

	if (stack[last]->constant) {
		if (stack[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order < 0 for KN!\n");
		if (fabs (rint(stack[last]->factor) - stack[last]->factor) > GMT_SMALL) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order not an integer for KN!\n");
		order = urint (fabs (stack[last]->factor));
		if (stack[prev]->constant) {
			b = (float)GMT_kn (GMT, order, fabs (stack[prev]->factor));
			simple = true;
		}
	}
	for (node = 0; node < info->size; node++) {
		if (simple)
			stack[prev]->G->data[node] = b;
		else {
			if (!stack[last]->constant) order = urint (fabsf (stack[last]->G->data[node]));
			stack[last]->G->data[node] = (float)GMT_kn (GMT, order, fabsf (stack[prev]->G->data[node]));
		}
	}
}

void grd_KURT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: KURT 1 1 Kurtosis of A.  */
{
	uint64_t node, n = 0;
	unsigned int row, col;
	double mean = 0.0, sum2 = 0.0, kurt = 0.0, delta;
	float f_kurt;

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = GMT->session.f_NaN;
		return;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	GMT_grd_loop (GMT, info->G, row, col, node) {
		if (GMT_is_fnan (stack[last]->G->data[node])) continue;
		n++;
		delta = (double)stack[last]->G->data[node] - mean;
		mean += delta / n;
		sum2 += delta * ((double)stack[last]->G->data[node] - mean);
	}
	if (n > 1) {
		GMT_grd_loop (GMT, info->G, row, col, node) {
			if (GMT_is_fnan (stack[last]->G->data[node])) continue;
			delta = (double)stack[last]->G->data[node] - mean;
			kurt += pow (delta, 4.0);
		}
		sum2 /= (n - 1);
		kurt = kurt / (n * sum2 * sum2) - 3.0;
		f_kurt = (float)kurt;
	}
	else
		f_kurt = GMT->session.f_NaN;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = f_kurt;
}

/* Helper functions ASCII_read and ASCII_free are used in LDIST* and PDIST* */

struct GMT_DATASET *ASCII_read (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, int geometry, char *op)
{
	struct GMT_DATASET *D = NULL;
	if (GMT_is_geographic (GMT, GMT_IN))
		GMT_init_distaz (GMT, 'k', GMT_sph_mode (GMT), GMT_MAP_DIST);
	else
		GMT_init_distaz (GMT, 'X', 0, GMT_MAP_DIST);	/* Cartesian */

	GMT_set_cols (GMT, GMT_IN,  2);
	if ((D = GMT_Read_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, geometry, GMT_READ_NORMAL, NULL, info->ASCII_file, NULL)) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in operator %s reading file %s!\n", op, info->ASCII_file);
		info->error = GMT->parent->error;
		return NULL;
	}
	return (D);
}

int ASCII_free (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_DATASET **D, char *op)
{
	if (GMT_Destroy_Data (GMT->parent, D) != GMT_OK) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error in operator %s destroying allocated data from %s!\n", op, info->ASCII_file);
		info->error = GMT->parent->error;
		return 1;
	}
	return 0;
}

void grd_LDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LDIST 1 1 Compute minimum distance (in km if -fg) from lines in multi-segment ASCII file A.  */
{
	uint64_t node, row, col;
	double d;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;

	if ((D = ASCII_read (GMT, info, GMT_IS_LINE, "LDIST")) == NULL) return;
	T = D->table[0];	/* Only one table in a single file */

	GMT_grd_padloop (GMT, info->G, row, col, node) {	/* Visit each node */
		if (col == 0) GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Row %d\n", row);
		(void) GMT_near_lines (GMT, info->d_grd_x[col], info->d_grd_y[row], T, true, &d, NULL, NULL);
		stack[last]->G->data[node] = (float)d;
	}

	ASCII_free (GMT, info, &D, "LDIST");	/* Free memory used for line */
}

void grd_LDISTG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LDISTG 0 1 As LDIST, but operates on the GSHHG dataset (see -A, -D for options).  */
{
	uint64_t node, row, col, seg, tbl;
	int i, old_i = INT32_MAX, old_row = INT32_MAX;
	double lon, lon1, lat, x, y, hor, bin_size, slop, d;
	double max_hor = 0.0, wesn[4] = {0.0, 360.0, -90.0, 90.0};
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;

	if (!GMT_is_geographic (GMT, GMT_IN)) /* Set -fg implicitly since not set already via input grid or -fg */
		GMT_parse_common_options (GMT, "f", 'f', "g");
	GMT_init_distaz (GMT, 'k', GMT_sph_mode (GMT), GMT_MAP_DIST);

	/* We use the global GSHHG data set to construct distances to. Although we know that the
	 * max distance to a coastline is ~2700 km, we cannot anticipate the usage of any user.
	 * If (s)he's excluding small features, then the distance will be larger. So we do not
	 * limit the region of GSHHG */
	if ((D = GMT_get_gshhg_lines (GMT, wesn, info->gshhg_res, info->A)) == NULL) return;

	bin_size = info->A->bin_size;	/* Current GSHHG bin size in degrees */
	slop = 2 * GMT_distance (GMT, 0.0, 0.0, bin_size, 0.0);	/* Define slop in projected units (km) */
	if (last == UINT32_MAX) last = 0;	/* Was called the very first time when n_stack - 1 goes crazy since it is unsigned */
	GMT_grd_padloop (GMT, info->G, row, col, node) {	/* Visit each node */
		if (col == 0) GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Row %d\n", row);
		lon = info->d_grd_x[col], lat = info->d_grd_y[row];
		i = (int)floor(lon/bin_size);
		/* For any new bin along a row, find the closest center of coastline bins */
		if (i != old_i || row != old_row) {
			lon1 = (i + 0.5) * bin_size;
			for (tbl = 0, hor = DBL_MAX; tbl < D->n_tables; tbl++) {
				x = 0.5 * (D->table[tbl]->min[GMT_X] + D->table[tbl]->max[GMT_X]);
				y = 0.5 * (D->table[tbl]->min[GMT_Y] + D->table[tbl]->max[GMT_Y]);
				D->table[tbl]->dist = d = GMT_distance (GMT, lon1, lat, x, y);
				if (d < hor) hor = d;
			}
			/* Add 2 bin sizes to the closest distance to a bin as slop. This should always include the closest points in any bin */
			hor = hor + slop;
			old_i = i, old_row = (int)row;
			if (hor > max_hor) max_hor = hor;
		}

		/* Loop over each line segment in each bin that is closer than the horizon defined above */
		for (tbl = 0, d = DBL_MAX; tbl < D->n_tables; tbl++) {
			T = D->table[tbl];
			if (T->dist >= hor) continue;	/* Skip entire bins that are too far away */
			for (seg = 0; seg < T->n_segments; seg++) {
				(void) GMT_near_a_line (GMT, lon, lat, seg, T->segment[seg], true, &d, NULL, NULL);
			}
		}
		stack[last]->G->data[node] = (float)d;
	}
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Max LDISTG horizon distance used: %g\n", max_hor);
	GMT_free_dataset (GMT, &D);
}

void grd_LDIST2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LDIST2 2 1 As LDIST, from lines in ASCII file B but only to nodes where A != 0.  */
{
	uint64_t node, row, col;
	unsigned int prev;
	double d;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;

	if ((D = ASCII_read (GMT, info, GMT_IS_LINE, "LDIST2")) == NULL) return;
	T = D->table[0];	/* Only one table in a single file */
	prev = last - 1;

	GMT_grd_padloop (GMT, info->G, row, col, node) {	/* Visit each node */
		if (col == 0) GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Row %d\n", row);
		if (stack[prev]->G->data[node] == 0.0)
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			(void) GMT_near_lines (GMT, info->d_grd_x[col], info->d_grd_y[row], T, true, &d, NULL, NULL);
			stack[prev]->G->data[node] = (float)d;
		}
	}

	ASCII_free (GMT, info, &D, "LDIST2");	/* Free memory used for line */
}

void grd_LE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LE 2 1 1 if A <= B, else 0.  */
{
	uint64_t node;
	unsigned int prev;
	float a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (float)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (float)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (GMT_is_fnan (a) || GMT_is_fnan (b)) ? GMT->session.f_NaN : (float)(a <= b);
	}
}

void grd_LOG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LOG 1 1 log (A) (natural log).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, argument to log = 0\n");

	if (stack[last]->constant) a = (float)d_log (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : d_logf (GMT, fabsf (stack[last]->G->data[node]));
}

void grd_LOG10 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LOG10 1 1 log10 (A) (base 10).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, argument to log10 = 0\n");

	if (stack[last]->constant) a = (float)d_log10 (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : d_log10f (GMT, fabsf (stack[last]->G->data[node]));
}

void grd_LOG1P (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LOG1P 1 1 log (1+A) (accurate for small A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && stack[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, argument to log1p < 0\n");

	if (stack[last]->constant) a = (float)d_log1p (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : d_log1pf (GMT, fabsf (stack[last]->G->data[node]));
}

void grd_LOG2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LOG2 1 1 log2 (A) (base 2).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, argument to log2 = 0\n");

	if (stack[last]->constant) a = d_log (GMT, fabs (stack[last]->factor)) * M_LN2_INV;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : d_logf (GMT, fabsf (stack[last]->G->data[node])) * M_LN2_INV);
}

void grd_LMSSCL (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LMSSCL 1 1 LMS scale estimate (LMS STD) of A.  */
{
	uint64_t node, n;
	unsigned int GMT_mode_selection = 0, GMT_n_multiples = 0, pad[4];
	double mode, lmsscl;
	float lmsscl_f;

	if (stack[last]->constant) {	/* Trivial case */
		GMT_memset (stack[last]->G->data, info->size, float);
		return;
	}

	/* Sort will put any NaNs to the end - we then count to find the real data */

	GMT_memcpy (pad, stack[last]->G->header->pad, 4, unsigned int);	/* Save original pad */
	GMT_grd_pad_off (GMT, stack[last]->G);				/* Undo pad if one existed so we can sort */
	GMT_sort_array (GMT, stack[last]->G->data, info->nm, GMT_FLOAT);
	for (n = info->nm; n > 1 && GMT_is_fnan (stack[last]->G->data[n-1]); n--);
	if (n) {
		GMT_mode_f (GMT, stack[last]->G->data, n, n/2, 0, GMT_mode_selection, &GMT_n_multiples, &mode);
		GMT_getmad_f (GMT, stack[last]->G->data, n, mode, &lmsscl);
		lmsscl_f = (float)lmsscl;
	}
	else
		lmsscl_f = GMT->session.f_NaN;

	GMT_memset (stack[last]->G->data, info->size, float);	/* Wipes everything */
	GMT_grd_pad_on (GMT, stack[last]->G, pad);		/* Reinstate the original pad */
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = lmsscl_f;

	if (GMT_n_multiples > 0) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: %d Multiple modes found\n", GMT_n_multiples);
}

void grd_LOWER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LOWER 1 1 The lowest (minimum) value of A.  */
{
	uint64_t node;
	unsigned int row, col;
	float low = FLT_MAX;

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)stack[last]->factor;
		return;
	}

	GMT_grd_loop (GMT, info->G, row, col, node) {	/* First we must find the lowest value in the grid */
		if (GMT_is_fnan (stack[last]->G->data[node])) continue;
		if (stack[last]->G->data[node] < low) low = stack[last]->G->data[node];
	}
	/* Now copy that low value everywhere */
	for (node = 0; node < info->size; node++) if (!GMT_is_fnan (stack[last]->G->data[node])) stack[last]->G->data[node] = low;
}

void grd_LRAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LRAND 2 1 Laplace random noise with mean A and std. deviation B.  */
{
	uint64_t node;
	unsigned int prev;
	double a = 0.0, b = 0.0;

	prev = last - 1;
	if (stack[prev]->constant) a = stack[prev]->factor;
	if (stack[last]->constant) b = stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) a = stack[prev]->G->data[node];
		if (!stack[last]->constant) b = stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)(a + b * GMT_lrand (GMT));
	}
}

void grd_LT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LT 2 1 1 if A < B, else 0.  */
{
	uint64_t node;
	unsigned int prev;
	float a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (float)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (float)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (GMT_is_fnan (a) || GMT_is_fnan (b)) ? GMT->session.f_NaN : (float)(a < b);
	}
}

void grd_MAD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: MAD 1 1 Median Absolute Deviation (L1 STD) of A.  */
{
	uint64_t node, n;
	unsigned int pad[4];
	double mad, med;
	float mad_f;

	if (stack[last]->constant) {	/* Trivial case */
		GMT_memset (stack[last], info->size, float);
		return;
	}

	/* Sort will put any NaNs to the end - we then count to find the real data */

	GMT_memcpy (pad, stack[last]->G->header->pad, 4U, unsigned int);	/* Save original pad */
	GMT_grd_pad_off (GMT, stack[last]->G);				/* Undo pad if one existed so we can sort */
	GMT_sort_array (GMT, stack[last]->G->data, info->nm, GMT_FLOAT);
	for (n = info->nm; n > 1 && GMT_is_fnan (stack[last]->G->data[n-1]); n--);
	if (n) {
		med = (n%2) ? stack[last]->G->data[n/2] : 0.5 * (stack[last]->G->data[(n-1)/2] + stack[last]->G->data[n/2]);
		GMT_getmad_f (GMT, stack[last]->G->data, n, med, &mad);
		mad_f = (float)mad;
	}
	else
		mad_f = GMT->session.f_NaN;

	GMT_grd_pad_on (GMT, stack[last]->G, pad);		/* Reinstate the original pad */
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = mad_f;
}

void grd_MAX (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: MAX 2 1 Maximum of A and B.  */
{
	uint64_t node;
	unsigned int prev;
	float a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (float)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (float)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (GMT_is_fnan (a) || GMT_is_fnan (b)) ? GMT->session.f_NaN : MAX (a, b);
	}
}

void grd_MEAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: MEAN 1 1 Mean value of A.  */
{
	uint64_t node, n_a = 0;
	unsigned int row, col;
	double sum_a = 0.0;

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)stack[last]->factor;
		return;
	}

	GMT_grd_loop (GMT, info->G, row, col, node) {
		if (GMT_is_fnan (stack[last]->G->data[node])) continue;
		sum_a += stack[last]->G->data[node];
		n_a++;
	}
	sum_a = (n_a) ? sum_a / (double)n_a : 0.0;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)sum_a;
}

void grd_MED (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: MED 1 1 Median value of A.  */
{
	uint64_t node, n;
	unsigned int pad[4];
	float med;

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)stack[last]->factor;
		return;
	}

	GMT_memcpy (pad, stack[last]->G->header->pad, 4, unsigned int);	/* Save original pad */
	GMT_grd_pad_off (GMT, stack[last]->G);				/* Undo pad if one existed so we can sort */
	GMT_sort_array (GMT, stack[last], info->nm, GMT_FLOAT);
	for (n = info->nm; n > 1 && GMT_is_fnan (stack[last]->G->data[n-1]); n--);
	if (n)
		med = (n%2) ? stack[last]->G->data[n/2] : 0.5f * (stack[last]->G->data[(n-1)/2] + stack[last]->G->data[n/2]);
	else
		med = GMT->session.f_NaN;

	GMT_grd_pad_on (GMT, stack[last]->G, pad);		/* Reinstate the original pad */
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = med;
}

void grd_MIN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: MIN 2 1 Minimum of A and B.  */
{
	uint64_t node;
	unsigned int prev;
	float a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (float)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (float)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (GMT_is_fnan (a) || GMT_is_fnan (b)) ? GMT->session.f_NaN : MIN (a, b);
	}
}

void grd_MOD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: MOD 2 1 A mod B (remainder after floored division).  */
{
	uint64_t node;
	unsigned int prev;
	double a, b;

	prev = last - 1;
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, using MOD 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (float)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (float)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)MOD (a, b);
	}
}

void grd_MODE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: MODE 1 1 Mode value (Least Median of Squares) of A.  */
{
	uint64_t node, n;
	unsigned int GMT_mode_selection = 0, GMT_n_multiples = 0, pad[4];
	double mode = 0.0;

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)stack[last]->factor;
		return;
	}

	GMT_memcpy (pad, stack[last]->G->header->pad, 4, unsigned int);	/* Save original pad */
	GMT_grd_pad_off (GMT, stack[last]->G);				/* Undo pad if one existed so we can sort */
	GMT_sort_array (GMT, stack[last]->G->data, info->nm, GMT_FLOAT);
	for (n = info->nm; n > 1 && GMT_is_fnan (stack[last]->G->data[n-1]); n--);
	if (n)
		GMT_mode_f (GMT, stack[last]->G->data, n, n/2, 0, GMT_mode_selection, &GMT_n_multiples, &mode);
	else
		mode = GMT->session.f_NaN;

	GMT_grd_pad_on (GMT, stack[last]->G, pad);		/* Reinstate the original pad */
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)mode;
	if (GMT_n_multiples > 0) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: %d Multiple modes found\n", GMT_n_multiples);
}

void grd_MUL (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: MUL 2 1 A * B.  */
{
	uint64_t node;
	unsigned int prev;
	double a, b;

	prev = last - 1;
	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)(a * b);
	}
}

void grd_NAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: NAN 2 1 NaN if A == B, else A.  */
{
	uint64_t node;
	unsigned int prev;
	float a = 0.0f, b = 0.0f;

	prev = last - 1;
	if (stack[prev]->constant) a = (float)stack[prev]->factor;
	if (stack[last]->constant) b = (float)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) a = stack[prev]->G->data[node];
		if (!stack[last]->constant) b = stack[last]->G->data[node];
		stack[prev]->G->data[node] = (a == b) ? GMT->session.f_NaN : a;
	}
}

void grd_NEG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: NEG 1 1 -A.  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand == 0!\n");
	if (stack[last]->constant) a = (float)-stack[last]->factor;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : -stack[last]->G->data[node];
}

void grd_NEQ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: NEQ 2 1 1 if A != B, else 0.  */
{
	uint64_t node;
	unsigned int prev;
	float a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (float)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (float)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)(a != b);
	}
}

void grd_NORM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: NORM 1 1 Normalize (A) so max(A)-min(A) = 1.  */
{
	uint64_t node, n = 0;
	unsigned int row, col;
	float z, zmin = FLT_MAX, zmax = -FLT_MAX;
	double a;

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, NORM of a constant gives NaN!\n");
		a = GMT->session.d_NaN;
	}
	else {
		GMT_grd_loop (GMT, info->G, row, col, node) {
			z = stack[last]->G->data[node];
			if (GMT_is_fnan (z)) continue;
			if (z < zmin) zmin = z;
			if (z > zmax) zmax = z;
			n++;
		}
		a = (n == 0 || zmax == zmin) ? GMT->session.f_NaN : (1.0 / (zmax - zmin));	/* Normalization scale */
	}
	GMT_grd_loop (GMT, info->G, row, col, node) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : a * stack[last]->G->data[node]);
}

void grd_NOT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: NOT 1 1 NaN if A == NaN, 1 if A == 0, else 0.  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand == 0!\n");
	if (stack[last]->constant) a = (fabs (stack[last]->factor) > GMT_CONV_LIMIT) ? 0.0f : 1.0f;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : ((fabsf (stack[last]->G->data[node]) > GMT_CONV_LIMIT) ? 0.0f : 1.0f);
}

void grd_NRAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: NRAND 2 1 Normal, random values with mean A and std. deviation B.  */
{
	uint64_t node;
	unsigned int prev;
	double a = 0.0, b = 0.0;

	prev = last - 1;
	if (stack[prev]->constant) a = stack[prev]->factor;
	if (stack[last]->constant) b = stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) a = stack[prev]->G->data[node];
		if (!stack[last]->constant) b = stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)(a + b * GMT_nrand (GMT));
	}
}

void grd_OR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: OR 2 1 NaN if B == NaN, else A.  */
{
	uint64_t node;
	unsigned int prev;
	float a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (float)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (float)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (GMT_is_fnan (a) || GMT_is_fnan (b)) ? GMT->session.f_NaN : a;
	}
}

void grd_PDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PDIST 1 1 Compute minimum distance (in km if -fg) from points in ASCII file A.  */
{
	uint64_t dummy[2], node, row, col;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;

	if ((D = ASCII_read (GMT, info, GMT_IS_POINT, "PDIST")) == NULL) return;

	T = D->table[0];	/* Only one table in a single file */

	GMT_grd_padloop (GMT, info->G, row, col, node) stack[last]->G->data[node] = (float)GMT_mindist_to_point (GMT, info->d_grd_x[col], info->d_grd_y[row], T, dummy);

	ASCII_free (GMT, info, &D, "PDIST");	/* Free memory used for points */
}

void grd_PDIST2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PDIST2 2 1 As PDIST, from points in ASCII file B but only to nodes where A != 0.  */
{
	uint64_t dummy[2], node, row, col;
	unsigned int prev;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;

	if ((D = ASCII_read (GMT, info, GMT_IS_POINT, "PDIST")) == NULL) return;

	T = D->table[0];	/* Only one table in a single file */
	prev = last - 1;

	GMT_grd_padloop (GMT, info->G, row, col, node) {
		if (stack[prev]->G->data[node] == 0.0)
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else
			stack[prev]->G->data[node] = (float)GMT_mindist_to_point (GMT, info->d_grd_x[col], info->d_grd_y[row], T, dummy);
	}

	ASCII_free (GMT, info, &D, "PDIST2");	/* Free memory used for points */
}

void grd_POP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: POP 1 0 Delete top element from the stack.  */
{
	/* Dummy routine that does nothing but consume the top element of stack */
}

void grd_PLM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PLM 3 1 Associated Legendre polynomial P(A) degree B order C.  */
{
	uint64_t node;
	unsigned int prev = last - 1, first = last - 2;
	int L, M;
	double a = 0.0;
	/* last holds the order M , prev holds the degree L, first holds the argument x = cos(colat) */

	if (!(stack[prev]->constant && stack[last]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "L and M must be constants in PLM (no calculations performed)\n");
		return;
	}

	L = irint (stack[prev]->factor);
	M = irint (stack[last]->factor);

	if (stack[first]->constant) a = GMT_plm (GMT, L, M, stack[first]->factor);
	for (node = 0; node < info->size; node++) stack[first]->G->data[node] = (float)((stack[first]->constant) ? a : GMT_plm (GMT, L, M, stack[first]->G->data[node]));
}


void grd_PLMg (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PLMg 3 1 Normalized associated Legendre polynomial P(A) degree B order C (geophysical convention).  */
{
	uint64_t node;
	unsigned int prev = last - 1, first = last - 2;
	int L, M;
	double a = 0.0;
	/* last holds the order M, prev holds the degree L, first holds the argument x = cos(colat) */

	if (!(stack[prev]->constant && stack[last]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "L and M must be constants in PLMg (no calculations performed)\n");
		return;
	}

	L = irint (stack[prev]->factor);
	M = irint (stack[last]->factor);

	if (stack[first]->constant) a = GMT_plm_bar (GMT, L, M, stack[first]->factor, false);
	for (node = 0; node < info->size; node++) stack[first]->G->data[node] = (float)((stack[first]->constant) ? a : GMT_plm_bar (GMT, L, M, stack[first]->G->data[node], false));
}

void grd_POW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: POW 2 1 A ^ B.  */
{
	uint64_t node;
	unsigned int prev;
	double a, b;

	prev = last - 1;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)pow (a, b);
	}
}

void grd_PQUANT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PQUANT 2 1 The B'th Quantile (0-100%) of A.  */
{
	uint64_t node;
	unsigned int prev, pad[4];
	float p;

	prev  = last - 1;	/* last holds the selected quantile (0-100), prev the data % */
	if (!stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: PQUANT must be given a constant quantile (no calculations performed)\n");
		return;
	}
	if (stack[last]->factor < 0.0 || stack[last]->factor > 100.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: PQUANT must be given a constant quantile between 0-100%% (no calculations performed)\n");
		return;
	}
	if (stack[prev]->constant) {	/* Trivial case */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: PQUANT of a constant is set to NaN\n");
		p = GMT->session.f_NaN;
	}
	else {
		GMT_memcpy (pad, stack[last]->G->header->pad, 4U, unsigned int);	/* Save original pad */
		GMT_grd_pad_off (GMT, stack[last]->G);				/* Undo pad if one existed so we can sort */
		GMT_sort_array (GMT, stack[prev]->G->data, info->nm, GMT_FLOAT);
		p = (float) GMT_quantile_f (GMT, stack[prev]->G->data, stack[last]->factor, info->nm);
		GMT_memset (stack[last]->G->data, info->size, float);	/* Wipes everything */
		GMT_grd_pad_on (GMT, stack[last]->G, pad);		/* Reinstate the original pad */
	}

	for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = p;
}

void grd_PSI (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PSI 1 1 Psi (or Digamma) of A.  */
{
	uint64_t node;
	float a = 0.0f;
	double x[2];

	x[1] = 0.0;	/* No imaginary part */
	if (stack[last]->constant) {
		x[0] = stack[last]->factor;
		a = (float)GMT_psi (GMT, x, NULL);
	}

	for (node = 0; node < info->size; node++) {
		if (!stack[last]->constant) {
			x[0] = stack[last]->G->data[node];
			a = (float)GMT_psi (GMT, x, NULL);
		}
		stack[last]->G->data[node] = a;
	}
}

void grd_PVQV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last, unsigned int kind)
{
	bool calc;
	unsigned int prev = last - 1, first = last - 2, n;
	uint64_t node;
	float a = 0.0f;
	double x = 0.0, nu[2], pq[4];
	static char *name[2] = {"PV", "QV"};
	/* last holds the imaginary order vi, prev holds the real order vr, first holds the argument x = cos(colat) */

	calc = !(stack[prev]->constant && stack[last]->constant && stack[first]->constant);	/* Only constant if all args are constant */
	if (!calc) {	/* All constants */
		nu[0] = stack[prev]->factor;
		nu[1] = stack[last]->factor;
		if ((stack[first]->factor < -1.0 || stack[first]->factor > 1.0)) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, argument to %s outside domain!\n", name[kind]);
		GMT_PvQv (GMT, stack[first]->factor, nu, pq, &n);
		a = (float)pq[2*kind];
	}
	if (stack[prev]->constant) nu[0] = stack[prev]->factor;
	if (stack[last]->constant) nu[1] = stack[last]->factor;
	if (stack[first]->constant)    x = stack[first]->factor;
	kind *= 2;
	for (node = 0; node < info->size; node++) {
		if (calc){
			if (!stack[prev]->constant) nu[0] = stack[prev]->G->data[node];
			if (!stack[last]->constant) nu[1] = stack[last]->G->data[node];
			if (!stack[first]->constant)    x = stack[first]->G->data[node];
			GMT_PvQv (GMT, x, nu, pq, &n);
			a = (float)pq[kind];
		}
		stack[first]->G->data[node] = a;
	}
}

void grd_PV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PV 3 1 Legendre function Pv(A) of degree v = real(B) + imag(C).  */
{
	grd_PVQV (GMT, info, stack, last, 0);
}

void grd_QV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: QV 3 1 Legendre function Qv(A) of degree v = real(B) + imag(C).  */
{
	grd_PVQV (GMT, info, stack, last, 1);
}

void grd_R2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: R2 2 1 R2 = A^2 + B^2.  */
{
	uint64_t node;
	unsigned int prev;
	double a = 0.0, b = 0.0;

	prev = last - 1;
	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0!\n");
	if (stack[prev]->constant) a = stack[prev]->factor * stack[prev]->factor;
	if (stack[last]->constant) b = stack[last]->factor * stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) a = stack[prev]->G->data[node] * stack[prev]->G->data[node];
		if (!stack[last]->constant) b = stack[last]->G->data[node] * stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)(a + b);
	}
}

void grd_R2D (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: R2D 1 1 Convert Radians to Degrees.  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = R2D * stack[last]->factor;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : R2D * stack[last]->G->data[node]);
}

void grd_RAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: RAND 2 1 Uniform random values between A and B.  */
{
	uint64_t node;
	unsigned int prev;
	double a = 0.0, b = 0.0;

	prev = last - 1;
	if (stack[prev]->constant) a = stack[prev]->factor;
	if (stack[last]->constant) b = stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) a = stack[prev]->G->data[node];
		if (!stack[last]->constant) b = stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)(a + GMT_rand (GMT) * (b - a));
	}
}

void grd_RINT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: RINT 1 1 rint (A) (round to integral value nearest to A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)rint (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : rintf (stack[last]->G->data[node]);
}

void grd_ROTX (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ROTX 2 1 Rotate A by the (constant) shift B in x-direction.  */
{
	uint64_t node;
	unsigned int col, row, prev = last - 1, *new_col = NULL, nx;
	int colx, shift;
	float *z = NULL;

	/* Shift grid A by the x-shift B.  B must be a constant */

	if (!stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "DX shift (B) must be a constant in ROTX (no calculations performed)\n");
		return;
	}
	shift = irint (stack[last]->factor * info->G->header->r_inc[GMT_X]);	/* Shift of nodes */

	if (stack[prev]->constant || !shift) return;	/* Trivial since A is a constant or shift is zero */
	if (shift < 0) shift += info->G->header->nx;	/* Same thing */
	nx = info->G->header->nx;
	/* Set up permutation vector */

	new_col = GMT_memory (GMT, NULL, nx, unsigned int);
	z = GMT_memory (GMT, NULL, nx, float);
	for (col = colx = 0; col < info->G->header->nx; col++, colx++) new_col[colx] = (colx + shift) % info->G->header->nx;	/* Move by shift but rotate around */
	GMT_row_loop (GMT, info->G, row) {	/* For each row */
		GMT_col_loop (GMT, info->G, row, col, node) z[new_col[col]] = stack[prev]->G->data[node];	/* Copy one row of data to z with shift */
		node = GMT_IJP (info->G->header, row, 0);		/* First col */
		GMT_memcpy (&stack[prev]->G->data[node], z, nx, float);	/* Replace this row */
	}
	GMT_free (GMT, z);
	GMT_free (GMT, new_col);
}

void grd_ROTY (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ROTY 2 1 Rotate A by the (constant) shift B in y-direction.  */
{
	unsigned int row, col, prev = last - 1, *new_row = NULL;
	int rowx, shift;
	float *z = NULL;

	/* Shift grid A by the y-shift B.  B must be a constant */

	if (!stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "DY shift (B) must be a constant in ROTY (no calculations performed)\n");
		return;
	}
	shift = irint (stack[last]->factor / info->G->header->inc[GMT_Y]);	/* Shift of nodes */

	if (stack[prev]->constant || !shift) return;	/* Trivial since A is a constant or shift is zero */
	if (shift < 0) shift += info->G->header->ny;	/* Same thing */
	/* Set up permutation vector */

	new_row = GMT_memory (GMT, NULL, info->G->header->ny, unsigned int);
	z = GMT_memory (GMT, NULL, info->G->header->ny, float);
	for (row = rowx = 0; row < info->G->header->ny; row++, rowx++) new_row[rowx] = (rowx + info->G->header->ny - shift) % info->G->header->ny;	/* Move by shift but rotate around */
	for (col = 0; col < info->G->header->nx; col++) {	/* For each column */
		for (row = 0; row < info->G->header->ny; row++) z[new_row[row]] = stack[prev]->G->data[GMT_IJP(info->G->header, row, col)];	/* Copy one column of data to z with shift */
		for (row = 0; row < info->G->header->ny; row++) stack[prev]->G->data[GMT_IJP(info->G->header, row, col)] = z[row];	/* Replace this column */
	}
	GMT_free (GMT, z);
	GMT_free (GMT, new_row);
}

void grd_SDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SDIST 2 1 Spherical distance (in km) between grid nodes and stack lon,lat (A, B).  */
{
	uint64_t node, row, col;
	unsigned int prev = last - 1;
	double a, b;

	if (GMT_is_geographic (GMT, GMT_IN))
		GMT_init_distaz (GMT, 'k', GMT_sph_mode (GMT), GMT_MAP_DIST);
	else {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "SDIST Error: Grid must be geographic; see CDIST for Cartesian data.\n");
		return;
	}

	GMT_grd_padloop (GMT, info->G, row, col, node) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float) GMT_distance (GMT, a, b, info->d_grd_x[col], info->d_grd_y[row]);
	}
}

void grd_SDIST2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SDIST2 2 1 As SDIST but only to nodes that are != 0.  */
{
	uint64_t node, row, col;
	unsigned int prev = last - 1;
	double a, b;

	if (GMT_is_geographic (GMT, GMT_IN))
		GMT_init_distaz (GMT, 'k', GMT_sph_mode (GMT), GMT_MAP_DIST);
	else {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "SDIST2 Error: Grid must be geographic; see CDIST2 for Cartesian data.\n");
		return;
	}

	GMT_grd_padloop (GMT, info->G, row, col, node) {
		if (stack[prev]->G->data[node] == 0.0)
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
			stack[prev]->G->data[node] = (float) GMT_distance (GMT, a, b, info->d_grd_x[col], info->d_grd_y[row]);
		}
	}
}

void grd_AZ_sub (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last, bool reverse)
{
	uint64_t node, row, col;
	unsigned int prev = last - 1;
	double x0 = 0.0, y0 = 0.0, az;

	GMT_init_distaz (GMT, 'd', GMT_sph_mode (GMT), GMT_MAP_DIST);
	if (stack[prev]->constant) x0 = stack[prev]->factor;
	if (stack[last]->constant) y0 = stack[last]->factor;
	GMT_grd_padloop (GMT, info->G, row, col, node) {
		if (!stack[prev]->constant) x0 = stack[prev]->G->data[node];
		if (!stack[last]->constant) y0 = stack[last]->G->data[node];
		az = GMT_az_backaz (GMT, info->d_grd_x[col], info->d_grd_y[row], x0, y0, reverse);
		while (az < -180.0) az += 360.0;
		while (az > +180.0) az -= 360.0;
		stack[prev]->G->data[node] = (float)az;
	}
}

void grd_SAZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SAZ 2 1 Spherical azimuth from grid nodes to stack x,y.  */
/* Azimuth from grid ones to stack point */
{
	grd_AZ_sub (GMT, info, stack, last, false);
}

void grd_SBAZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SBAZ 2 1 Spherical backazimuth from grid nodes to stack x,y.  */
/* Azimuth from stack point to grid ones (back azimuth) */
{
	grd_AZ_sub (GMT, info, stack, last, true);
}

void grd_SEC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SEC 1 1 sec (A) (A in radians).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)(1.0 / cos (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (1.0f / cosf (stack[last]->G->data[node]));
}

void grd_SECD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SECD 1 1 sec (A) (A in degrees).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = 1.0 / cosd (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : 1.0 / cosd (stack[last]->G->data[node]));
}

void grd_SIGN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SIGN 1 1 sign (+1 or -1) of A.  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand == 0!\n");
	if (stack[last]->constant) a = (float)copysign (1.0, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : copysignf (1.0f, stack[last]->G->data[node]);
}

void grd_SIN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SIN 1 1 sin (A) (A in radians).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)sin (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : sinf (stack[last]->G->data[node]);
}

void grd_SINC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SINC 1 1 sinc (A) (sin (pi*A)/(pi*A)).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = GMT_sinc (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : GMT_sinc (GMT, stack[last]->G->data[node]));
}

void grd_SIND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SIND 1 1 sin (A) (A in degrees).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = sind (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : sind (stack[last]->G->data[node]));
}

void grd_SINH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SINH 1 1 sinh (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)sinh (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : sinhf (stack[last]->G->data[node]);
}

void grd_SKEW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SKEW 1 1 Skewness of A.  */
{
	uint64_t node, n = 0;
	unsigned int row, col;
	double mean = 0.0, sum2 = 0.0, skew = 0.0, delta;
	float f_skew;

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = GMT->session.f_NaN;
		return;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	GMT_grd_loop (GMT, info->G, row, col, node) {
		if (GMT_is_fnan (stack[last]->G->data[node])) continue;
		n++;
		delta = stack[last]->G->data[node] - mean;
		mean += delta / n;
		sum2 += delta * (stack[last]->G->data[node] - mean);
	}
	if (n > 1) {
		GMT_grd_loop (GMT, info->G, row, col, node) {
			if (GMT_is_fnan (stack[last]->G->data[node])) continue;
			delta = stack[last]->G->data[node] - mean;
			skew += pow (delta, 3.0);
		}
		sum2 /= (n - 1);
		skew /= n * pow (sum2, 1.5);
		f_skew = (float)skew;
	}
	else
		f_skew = GMT->session.f_NaN;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = f_skew;
}

void grd_SQR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SQR 1 1 A^2.  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = stack[last]->factor * stack[last]->factor;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : stack[last]->G->data[node] * stack[last]->G->data[node]);
}

void grd_SQRT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SQRT 1 1 sqrt (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant && stack[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one < 0!\n");
	if (stack[last]->constant) a = (float)sqrt (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : sqrtf (stack[last]->G->data[node]);
}

void grd_STD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: STD 1 1 Standard deviation of A.  */
{
	uint64_t node, n = 0;
	unsigned int row, col;
	double mean = 0.0, sum2 = 0.0, delta;

	if (stack[last]->constant) {	/* Trivial case */
		GMT_memset (stack[last], info->size, float);
		return;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	GMT_grd_loop (GMT, info->G, row, col, node) {
		if (GMT_is_fnan (stack[last]->G->data[node])) continue;
		n++;
		delta = stack[last]->G->data[node] - mean;
		mean += delta / n;
		sum2 += delta * (stack[last]->G->data[node] - mean);
	}
	sum2 = (n > 1) ? sqrt (sum2 / (n - 1)) : 0.0;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)sum2;
}

void grd_STEP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: STEP 1 1 Heaviside step function: H(A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[last]->constant) a = stack[last]->G->data[node];
		if (a == 0.0f)
			stack[last]->G->data[node] = 0.5f;
		else
			stack[last]->G->data[node] = (a < 0.0) ? 0.0f : 1.0f;
	}
}

void grd_STEPX (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: STEPX 1 1 Heaviside step function in x: H(x-A).  */
{
	uint64_t node, row, col;
	double a;

	GMT_grd_padloop (GMT, info->G, row, col, node) {
		a = info->d_grd_x[col] - ((stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node]);
		if (a == 0.0)
			stack[last]->G->data[node] = 0.5f;
		else
			stack[last]->G->data[node] = (a < 0.0) ? 0.0f : 1.0f;
	}
}

void grd_STEPY (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: STEPY 1 1 Heaviside step function in y: H(y-A).  */
{
	uint64_t node, row, col;
	double a;

	GMT_grd_padloop (GMT, info->G, row, col, node) {
		a = info->d_grd_y[row] - ((stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node]);
		if (a == 0.0)
			stack[last]->G->data[node] = 0.5f;
		else
			stack[last]->G->data[node] = (a < 0.0) ? 0.0f : 1.0f;
	}
}

void grd_SUB (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SUB 2 1 A - B.  */
{
	uint64_t node;
	unsigned int prev;
	double a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (float)(a - b);
	}
}

void grd_SUM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SUM 1 1 Sum of all values in A.  */
{
	uint64_t node, n_used = 0;
	double sum = 0.0;

	if (stack[last]->constant)
		sum = stack[last]->factor * stack[last]->G->header->nm;
	else {
		uint64_t row, col;
		GMT_grd_loop (GMT, info->G, row, col, node) {
			if (GMT_is_fnan (stack[last]->G->data[node])) continue;
			sum += stack[last]->G->data[node];
			n_used++;
		}
		if (n_used == 0) sum = GMT->session.d_NaN;
	}
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)sum;
}

void grd_TAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TAN 1 1 tan (A) (A in radians).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)tan (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : tanf (stack[last]->G->data[node]);
}

void grd_TAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TAND 1 1 tan (A) (A in degrees).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = tand (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : tand (stack[last]->G->data[node]));
}

void grd_TANH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TANH 1 1 tanh (A).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)tanh (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : tanhf (stack[last]->G->data[node]);
}

void grd_TAPER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TAPER 2 1 Unit weights cosine-tapered to zero within A and B of x and y grid margins.  */
{
	uint64_t node;
	unsigned int prev = last - 1, row, col;
	double strip, scale, start, stop, from_start, from_stop, w_y, *w_x = NULL;

	if (!(stack[last]->constant && stack[prev]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "TAPER: Arguments A and B must both be constants\n");
		return;
	}
	if (stack[last]->factor < 0.0 || stack[prev]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "TAPER: Arguments A and B must both be >= 0\n");
		return;
	}

	/* First compute and store x taper weights: Ramp 0 to 1 for left margin, constant 1, then ramp 1 to 0 for right margin */
	w_x = GMT_memory (GMT, NULL, info->G->header->mx, double);
	if (stack[prev]->factor == 0.0) {	/* No taper in x so set weights to 1 */
		GMT_col_padloop2 (GMT, info->G, col) w_x[col] = 1.0;
	}
	else {
		strip = stack[prev]->factor;
		scale = M_PI / strip;
		start = strip + info->G->header->wesn[XLO];
		stop  = strip - info->G->header->wesn[XHI];
		GMT_col_padloop2 (GMT, info->G, col) {
			from_start = start - info->d_grd_x[col];
			if (from_start > 0.0) w_x[col] = 0.5 * (1.0 + cos (from_start * scale));
			else if ((from_stop = stop + info->d_grd_x[col]) > 0.0) w_x[col] = 0.5 * (1.0 + cos (from_stop * scale));
			else w_x[col] = 1.0;	/* Inside non-tapered x-range */
		}
	}

	/* Now compute y taper weights: Ramp 0 to 1 for left margin, constant 1, then ramp 1 to 0 for right margin.
	 * We apply these as we loop over rows and do the w_x * w_y taper */
	strip = stack[last]->factor;
	scale = (strip > 0.0) ? M_PI / strip : 0.0;
	start = strip + info->G->header->wesn[YLO];
	stop  = strip - info->G->header->wesn[YHI];

	GMT_row_padloop (GMT, info->G, row, node) {
		from_start = start - info->d_grd_y[row];
		if (stack[last]->factor == 0.0) w_y = 1.0;	/* No taper in y-range */
		else if (from_start > 0.0) w_y = 0.5 * (1.0 + cos (from_start * scale));
		else if ((from_stop = stop + info->d_grd_y[row]) > 0.0) w_y = 0.5 * (1.0 + cos (from_stop * scale));
		else w_y = 1.0;	/* Inside non-tapered y-range */
		GMT_col_padloop (GMT, info->G, col, node) {
			stack[prev]->G->data[node] = (float)(w_y * w_x[col]);
		}
	}
	GMT_free (GMT, w_x);
}

void grd_TN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TN 2 1 Chebyshev polynomial Tn(-1<t<+1,n), with t = A, and n = B.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	int n;
	double a = 0.0, t;

	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		n = irint ((stack[last]->constant) ? stack[last]->factor : (double)stack[last]->G->data[node]);
		GMT_chebyshev (GMT, a, n, &t);
		stack[prev]->G->data[node] = (float)t;
	}
}

void grd_TCRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TCRIT 2 1 Critical value for Student's t-distribution, with alpha = A and n = B.  */
{
	uint64_t node;
	int b;
	unsigned int prev, row, col;
	double a;

	prev = last - 1;
	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0 for TCRIT!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for TCRIT!\n");
	GMT_grd_loop (GMT, info->G, row, col, node) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = irint ((stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node]);
		stack[prev]->G->data[node] = (float)GMT_tcrit (GMT, a, (double)b);
	}
}

void grd_TDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TDIST 2 1 Student's t-distribution A(t,n), with t = A, and n = B.  */
{
	uint64_t node;
	int b;
	unsigned int prev, row, col;
	double a, prob;

	prev = last - 1;
	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand one == 0 for TDIST!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, operand two == 0 for TDIST!\n");
	GMT_grd_loop (GMT, info->G, row, col, node) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = irint ((stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node]);
		(void) GMT_student_t_a (GMT, a, b, &prob);
		stack[prev]->G->data[node] = (float)prob;
	}
}

void grd_UPPER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: UPPER 1 1 The highest (maximum) value of A.  */
{
	uint64_t node;
	unsigned int row, col;
	float high = -FLT_MAX;

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)stack[last]->factor;
		return;
	}

	GMT_grd_loop (GMT, info->G, row, col, node) {
		if (GMT_is_fnan (stack[last]->G->data[node])) continue;
		if (stack[last]->G->data[node] > high) high = stack[last]->G->data[node];
	}
	for (node = 0; node < info->size; node++) if (!GMT_is_fnan (stack[last]->G->data[node])) stack[last]->G->data[node] = high;
}

void grd_XOR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: XOR 2 1 0 if A == NaN and B == NaN, NaN if B == NaN, else A.  */
{
	uint64_t node;
	unsigned int prev;
	float a = 0.0f, b = 0.0f;

	prev = last - 1;
	if (stack[prev]->constant) a = (float)stack[prev]->factor;
	if (stack[last]->constant) b = (float)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) a = stack[prev]->G->data[node];
		if (!stack[last]->constant) b = stack[last]->G->data[node];
		stack[prev]->G->data[node] = (GMT_is_fnan (a) && GMT_is_fnan (b)) ? 0.0f : (GMT_is_fnan (b) ? GMT->session.f_NaN : a);
	}
}

void grd_Y0 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: Y0 1 1 Bessel function of A (2nd kind, order 0).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)y0 (fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (float)y0 ((double)fabsf (stack[last]->G->data[node]));
}

void grd_Y1 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: Y1 1 1 Bessel function of A (2nd kind, order 1).  */
{
	uint64_t node;
	float a = 0.0f;

	if (stack[last]->constant) a = (float)y1 (fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (float)y1 ((double)fabsf (stack[last]->G->data[node]));
}

void grd_YLM_sub (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last, bool ortho)
{
	/* Returns geophysical normalization, unless M < 0, then orthonormalized form */
	uint64_t node, row, col;
	unsigned int prev = last - 1;
	int L, M;
	double x, z, P, C, S;

	if (!(stack[prev]->constant && stack[last]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "L and M must be constants in YLM[g] (no calculations performed)\n");
		return;
	}

	L = irint (stack[prev]->factor);
	M = irint (stack[last]->factor);
	z = abs (M) * D2R;	/* abs() just in case routine is called with -M to add (-1)^M */

	GMT_row_padloop (GMT, info->G, row, node) {	/* For each latitude */

		x = sind (info->d_grd_y[row]);	/* Plm takes cos(colatitude) = sin(latitude) */
		P = GMT_plm_bar (GMT, L, M, x, ortho);
		if (M == 0) {
			GMT_col_padloop (GMT, info->G, col, node) {
				stack[prev]->G->data[node] = (float)P;
				stack[last]->G->data[node] = 0.0f;
			}
		}
		else {
			GMT_col_padloop (GMT, info->G, col, node) {
				sincos (z * info->d_grd_x[col], &S, &C);
				stack[prev]->G->data[node] = (float)(P * C);
				stack[last]->G->data[node] = (float)(P * S);
			}
		}
	}
}

void grd_YLM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: YLM 2 2 Re and Im orthonormalized spherical harmonics degree A order B.  */
{
	grd_YLM_sub (GMT, info, stack, last, true);
}

void grd_YLMg (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: YLMg 2 2 Cos and Sin normalized spherical harmonics degree A order B (geophysical convention).  */
{
	grd_YLM_sub (GMT, info, stack, last, false);
}

void grd_YN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: YN 2 1 Bessel function of A (2nd kind, order B).  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	int order = 0;
	bool simple = false;
	float b = 0.0f;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, argument = 0 for YN!\n");
	if (stack[last]->constant) {
		if (stack[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order < 0 for YN!\n");
		if ((rint(stack[last]->factor) != stack[last]->factor)) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Warning, order not an integer for YN!\n");
		order = urint (fabs (stack[last]->factor));
		if (stack[prev]->constant) {
			b = (float)yn (order, fabs (stack[prev]->factor));
			simple = true;
		}
	}
	for (node = 0; node < info->size; node++) {
		if (simple)
			stack[prev]->G->data[node] = b;
		else {
			if (!stack[last]->constant) order = urint (fabsf (stack[last]->G->data[node]));
			stack[last]->G->data[node] = (float)yn (order, (double)fabsf (stack[prev]->G->data[node]));
		}
	}
}

void grd_ZCRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ZCRIT 1 1 Critical value for the normal-distribution, with alpha = A.  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = GMT_zcrit (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : GMT_zcrit (GMT, stack[last]->G->data[node]));
}

void grd_ZDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ZDIST 1 1 Cumulative normal-distribution C(x), with x = A.  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = GMT_zdist (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (float)((stack[last]->constant) ? a : GMT_zdist (GMT, stack[last]->G->data[node]));
}

/* ---------------------- end operator functions --------------------- */

#include "grdmath.h"

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return1(code) {GMT_Destroy_Options (API, &list); Free_grdmath_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}
#define Return(code) {GMT_Destroy_Options (API, &list); Free_grdmath_Ctrl (GMT, Ctrl); grdmath_free (GMT, stack, recall, &info); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int decode_grd_argument (struct GMT_CTRL *GMT, struct GMT_OPTION *opt, double *value, struct GMT_HASH *H)
{
	int i, expect, check = GMT_IS_NAN;
	bool possible_number = false;
	double tmp = 0.0;

	if (opt->option == GMT_OPT_OUTFILE) return GRDMATH_ARG_IS_SAVE;	/* Time to save stack; arg is filename */

	/* Check if argument is operator */

	if ((i = GMT_hash_lookup (GMT, opt->arg, H, GRDMATH_N_OPERATORS, GRDMATH_N_OPERATORS)) >= GRDMATH_ARG_IS_OPERATOR) return (i);

	/* Next look for symbols with special meaning */

	if (!strncmp (opt->arg, GRDMATH_STORE_CMD, strlen(GRDMATH_STORE_CMD))) return GRDMATH_ARG_IS_STORE;	/* store into mem location @<label> */
	if (!strncmp (opt->arg, GRDMATH_CLEAR_CMD, strlen(GRDMATH_CLEAR_CMD))) return GRDMATH_ARG_IS_CLEAR;	/* clear mem location @<label> */
	if (!strncmp (opt->arg, GRDMATH_RECALL_CMD, strlen(GRDMATH_RECALL_CMD))) return GRDMATH_ARG_IS_RECALL;	/* load from mem location @<label> */
	if (opt->arg[0] == '@') return GRDMATH_ARG_IS_RECALL;							/* load from mem location @<label> */
	if (!(strcmp (opt->arg, "PI") && strcmp (opt->arg, "pi"))) return GRDMATH_ARG_IS_PI;
	if (!(strcmp (opt->arg, "E") && strcmp (opt->arg, "e"))) return GRDMATH_ARG_IS_E;
	if (!strcmp (opt->arg, "EULER")) return GRDMATH_ARG_IS_EULER;
	if (!strcmp (opt->arg, "XMIN")) return GRDMATH_ARG_IS_XMIN;
	if (!strcmp (opt->arg, "XMAX")) return GRDMATH_ARG_IS_XMAX;
	if (!strcmp (opt->arg, "XINC")) return GRDMATH_ARG_IS_XINC;
	if (!strcmp (opt->arg, "NX")) return GRDMATH_ARG_IS_NX;
	if (!strcmp (opt->arg, "YMIN")) return GRDMATH_ARG_IS_YMIN;
	if (!strcmp (opt->arg, "YMAX")) return GRDMATH_ARG_IS_YMAX;
	if (!strcmp (opt->arg, "YINC")) return GRDMATH_ARG_IS_YINC;
	if (!strcmp (opt->arg, "NY")) return GRDMATH_ARG_IS_NY;
	if (!strcmp (opt->arg, "X")) return GRDMATH_ARG_IS_X_MATRIX;
	if (!strcmp (opt->arg, "Xn")) return GRDMATH_ARG_IS_x_MATRIX;
	if (!strcmp (opt->arg, "Y")) return GRDMATH_ARG_IS_Y_MATRIX;
	if (!strcmp (opt->arg, "Yn")) return GRDMATH_ARG_IS_y_MATRIX;
	if (!strcmp (opt->arg, "NaN")) {*value = GMT->session.d_NaN; return GRDMATH_ARG_IS_NUMBER;}

	/* Preliminary test-conversion to a number */

	if (!GMT_not_numeric (GMT, opt->arg)) {	/* Only check if we are not sure this is NOT a number */
		expect = (strchr (opt->arg, 'T')) ? GMT_IS_ABSTIME : GMT_IS_UNKNOWN;	/* Watch out for dateTclock-strings */
		check = GMT_scanf (GMT, opt->arg, expect, &tmp);
		possible_number = true;
	}

	/* Determine if argument is file. But first strip off suffix */

	if (!GMT_access (GMT, opt->arg, F_OK)) {	/* Yes it is */
		if (check != GMT_IS_NAN && possible_number) GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Your argument %s is both a file and a number.  File is selected\n", opt->arg);
		return GRDMATH_ARG_IS_FILE;
	}

	if (check != GMT_IS_NAN) {	/* OK it is a number */
		*value = tmp;
		return GRDMATH_ARG_IS_NUMBER;
	}

	if (opt->arg[0] == '-') {	/* Probably a bad commandline option */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Option %s not recognized\n", opt->arg);
		return GRDMATH_ARG_IS_BAD;
	}

	GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error: %s is not a number, operator or file name\n", opt->arg);
	return GRDMATH_ARG_IS_BAD;
}

char *grdmath_setlabel (struct GMT_CTRL *GMT, char *arg)
{
	char *label = strchr (arg, '@') + 1;	/* Label that follows @ */
	if (!label || label[0] == '\0') {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No label appended to STO|RCL|CLR operator!\n");
		return (NULL);
	}
	return (label);
}

void grdmath_free (struct GMT_CTRL *GMT, struct GRDMATH_STACK *stack[], struct GRDMATH_STORE *recall[], struct GRDMATH_INFO *info) {
	/* Free allocated memory before quitting */
	unsigned int k;

	for (k = 0; k < GRDMATH_STACK_SIZE; k++) {
		if (GMT_Destroy_Data (GMT->parent, &stack[k]->G) != GMT_OK) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to free stack item %d\n", k);
		}

		GMT_free (GMT, stack[k]);
	}
	for (k = 0; k < GRDMATH_STORE_SIZE; k++) {
		if (recall[k] == NULL) continue;
		if (recall[k] && !recall[k]->stored.constant) {
			if (GMT_Destroy_Data (GMT->parent, &recall[k]->stored.G) != GMT_OK) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to free recall item %d\n", k);
			}
		}
		GMT_free (GMT, recall[k]);
	}
	if (GMT_Destroy_Data (GMT->parent, &info->G) != GMT_OK) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to free info.G\n");
	}
	if (info->d_grd_x) GMT_free (GMT, info->d_grd_x);
	if (info->d_grd_y) GMT_free (GMT, info->d_grd_y);
	if (info->d_grd_xn) GMT_free (GMT, info->d_grd_xn);
	if (info->d_grd_yn) GMT_free (GMT, info->d_grd_yn);
	if (info->f_grd_x) GMT_free (GMT, info->f_grd_x);
	if (info->f_grd_y) GMT_free (GMT, info->f_grd_y);
	if (info->f_grd_xn) GMT_free (GMT, info->f_grd_xn);
	if (info->f_grd_yn) GMT_free (GMT, info->f_grd_yn);
	if (info->dx) GMT_free (GMT, info->dx);
	if (info->ASCII_file) free (info->ASCII_file);
}

int GMT_grdmath (void *V_API, int mode, void *args)
{
	int k, op = 0, new_stack = -1, rowx, colx, status, start, error = 0;
	unsigned int kk, nstack = 0, n_stored = 0, n_items = 0, this_stack;
	unsigned int consumed_operands[GRDMATH_N_OPERATORS], produced_operands[GRDMATH_N_OPERATORS];
	bool subset;
	char *in_file = NULL, *label = NULL;

	uint64_t node, row, col;

	struct GRDMATH_STACK *stack[GRDMATH_STACK_SIZE];
	struct GRDMATH_STORE *recall[GRDMATH_STORE_SIZE];
	struct GMT_GRID *G_in = NULL;

	double value, x_noise, y_noise, off, scale;
	double wesn[4], special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_NY+1];

#include "grdmath_op.h"

	void (*call_operator[GRDMATH_N_OPERATORS]) (struct GMT_CTRL *, struct GRDMATH_INFO *, struct GRDMATH_STACK **, unsigned int);

	struct GMT_HASH localhashnode[GRDMATH_N_OPERATORS];
	struct GRDMATH_INFO info;
	struct GRDMATH_CTRL *Ctrl = NULL;
	struct GMT_OPTION *opt = NULL, *list = NULL, *ptr = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_grdmath_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_grdmath_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_grdmath_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if ((list = gmt_substitute_macros (GMT, options, "grdmath.macros")) == NULL) Return1 (EXIT_FAILURE);
	Ctrl = New_grdmath_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return1 (API->error);
	if ((error = GMT_grdmath_parse (GMT, Ctrl, options))) Return1 (error);

	/*---------------------------- This is the grdmath main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Perform reverse Polish notation calculations on grids\n");
	GMT_memset (&info, 1, struct GRDMATH_INFO);		/* Initialize here to not crash when Return gets called */
	GMT_memset (recall, GRDMATH_STORE_SIZE, struct GRDMATH_STORE *);
	GMT_memset (localhashnode, GRDMATH_N_OPERATORS, struct GMT_HASH);
	for (k = 0; k < GRDMATH_STACK_SIZE; k++) stack[k] = GMT_memory (GMT, NULL, 1, struct GRDMATH_STACK);
	GMT_set_pad (GMT, 2U);	/* Ensure space for BCs in case an API passed pad == 0 */

	/* Internally replace the = file sequence with an output option ->file*/

	GMT_Destroy_Options (API, &options);
	options = list;	list = NULL;
	for (opt = options; opt; opt = opt->next) {
		if (opt->option == GMT_OPT_INFILE && !strcmp (opt->arg, "=")) {	/* Found the output sequence */
			if (opt->next) {
				ptr = GMT_Make_Option (API, GMT_OPT_OUTFILE, opt->next->arg);
				opt = opt->next;	/* Now we must skip that option */
			}
			else {	/* Standard output */
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: No output file specified via = file mechanism\n");
				Return (EXIT_FAILURE);
			}
		}
		else
		 	ptr = GMT_Make_Option (API, opt->option, opt->arg);

		if (ptr == NULL || (list = GMT_Append_Option (API, ptr, list)) == NULL) Return (API->error);
	}

	GMT_hash_init (GMT, localhashnode, operator, GRDMATH_N_OPERATORS, GRDMATH_N_OPERATORS);

	GMT_memset (wesn, 4, double);

	/* Read the first file we encounter so we may allocate space */

	for (opt = list; !G_in && opt; opt = opt->next) {	/* Look for a grid file, if given */
		if (!(opt->option == GMT_OPT_INFILE))	continue;	/* Skip command line options and output file */
		if (opt->next && !(strncmp (opt->next->arg, "LDIST", 5U) && strncmp (opt->next->arg, "PDIST", 5U) && strncmp (opt->next->arg, "INSIDE", 6U))) continue;	/* Not grids */
		/* Filenames,  operators, some numbers and = will all have been flagged as files by the parser */
		status = decode_grd_argument (GMT, opt, &value, localhashnode);		/* Determine what this is */
		if (status == GRDMATH_ARG_IS_BAD) Return (EXIT_FAILURE);		/* Horrible */
		if (status != GRDMATH_ARG_IS_FILE) continue;				/* Skip operators and numbers */
		in_file = opt->arg;
		if ((G_in = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, in_file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
	}

	subset = GMT->common.R.active;

	if (G_in) {	/* We read a gridfile header above, now update columns */
		if (GMT->common.R.active && Ctrl->I.active) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Cannot use -R, -I when grid files are specified\n");
			Return (EXIT_FAILURE);
		}
		else if  (GMT->common.r.active) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Cannot use -r when grid files are specified\n");
			Return (EXIT_FAILURE);
		}
		if (subset) {	/* Gave -R and files: Read the subset to set the header properly */
			GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);
			if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, in_file, G_in) == NULL) {	/* Get subset only */
				Return (API->error);
			}
		}
		if ((info.G = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, G_in)) == NULL) Return (API->error);
		if (GMT_Destroy_Data (API, &G_in) != GMT_OK) {
			Return (API->error);
		}
	}
	else if (GMT->common.R.active && Ctrl->I.active) {	/* Must create from -R -I [-r] */
		/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
		if ((info.G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, NULL, Ctrl->I.inc, \
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
	}
	else {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expression must contain at least one grid file or -R, -I\n");
		Return (EXIT_FAILURE);
	}
	info.nm = info.G->header->nm;	info.size = info.G->header->size;

	/* Get x and y vectors (these extend onto the pad) */

	info.d_grd_x  = GMT_memory (GMT, NULL, info.G->header->mx, double);
	info.d_grd_y  = GMT_memory (GMT, NULL, info.G->header->my, double);
	info.d_grd_xn = GMT_memory (GMT, NULL, info.G->header->mx, double);
	info.d_grd_yn = GMT_memory (GMT, NULL, info.G->header->my, double);
	info.f_grd_x  = GMT_memory (GMT, NULL, info.G->header->mx, float);
	info.f_grd_y  = GMT_memory (GMT, NULL, info.G->header->my, float);
	info.f_grd_xn = GMT_memory (GMT, NULL, info.G->header->mx, float);
	info.f_grd_yn = GMT_memory (GMT, NULL, info.G->header->my, float);
	for (k = 0, start = info.G->header->pad[XLO], colx = -start; k < (int)info.G->header->mx; colx++, k++) info.d_grd_x[k] = GMT_grd_col_to_x (GMT, colx, info.G->header);
	for (k = 0, start = info.G->header->pad[YHI], rowx = -start; k < (int)info.G->header->my; rowx++, k++) info.d_grd_y[k] = GMT_grd_row_to_y (GMT, rowx, info.G->header);
	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Make sure latitudes remain in range; if not apply geographic BC */
		for (kk = 0; kk < info.G->header->pad[YHI]; kk++)
			if (info.d_grd_y[kk] > 90.0) info.d_grd_y[kk] = 2.0 * 90.0 - info.d_grd_y[kk];
		for (kk = 0, k = info.G->header->my - info.G->header->pad[YLO]; kk < info.G->header->pad[YLO]; kk++, k++)
			if (info.d_grd_y[k] < -90.0) info.d_grd_y[k] = -2.0 * 90.0 - info.d_grd_y[k];
	}
	for (k = 0; k < (int)info.G->header->mx; k++) info.f_grd_x[k] = (float)info.d_grd_x[k];
	for (k = 0; k < (int)info.G->header->my; k++) info.f_grd_y[k] = (float)info.d_grd_y[k];
	off = 0.5 * (info.G->header->wesn[XHI] + info.G->header->wesn[XLO]);
	scale = 2.0 / (info.G->header->wesn[XHI] - info.G->header->wesn[XLO]);
	for (kk = 0; kk < info.G->header->mx; kk++) {
		info.d_grd_xn[kk] = (info.d_grd_x[kk] - off) * scale;
		info.f_grd_xn[kk] = (float)info.d_grd_xn[kk];
	}
	off = 0.5 * (info.G->header->wesn[YHI] + info.G->header->wesn[YLO]);
	scale = 2.0 / (info.G->header->wesn[YHI] - info.G->header->wesn[YLO]);
	for (kk = 0; kk < info.G->header->my; kk++) {
		info.d_grd_yn[kk] = (info.d_grd_y[kk] - off) * scale;
		info.f_grd_yn[kk] = (float)info.d_grd_yn[kk];
	}
	x_noise = GMT_SMALL * info.G->header->inc[GMT_X];	y_noise = GMT_SMALL * info.G->header->inc[GMT_Y];
	info.dx = GMT_memory (GMT, NULL, info.G->header->my, double);
	if (Ctrl->D.force) Ctrl->D.set = GMT_shore_adjust_res (GMT, Ctrl->D.set);
	info.gshhg_res = Ctrl->D.set;	/* Selected GSHHG resolution, if used */
	info.A = &Ctrl->A.info;		/* Selected GSHHG flags, if used */

	if (Ctrl->M.active) {	/* Use flat earth distances for gradients */
		for (kk = 0; kk < info.G->header->my; kk++) info.dx[kk] = GMT->current.proj.DIST_M_PR_DEG * info.G->header->inc[GMT_X] * cosd (info.d_grd_y[kk]);
		info.dy = GMT->current.proj.DIST_M_PR_DEG * info.G->header->inc[GMT_Y];
		info.convert = true;
	}
	else {	/* Constant increments in user units */
		for (kk = 0; kk < info.G->header->my; kk++) info.dx[kk] = info.G->header->inc[GMT_X];
		info.dy = info.G->header->inc[GMT_Y];
	}

	grdmath_init (call_operator, consumed_operands, produced_operands);

	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_PI] = M_PI;
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_E] = M_E;
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_EULER] = M_EULER;
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_XMIN] = info.G->header->wesn[XLO];
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_XMAX] = info.G->header->wesn[XHI];
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_XINC] = info.G->header->inc[GMT_X];
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_NX] = info.G->header->nx;
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_YMIN] = info.G->header->wesn[YLO];
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_YMAX] = info.G->header->wesn[YHI];
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_YINC] = info.G->header->inc[GMT_Y];
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_NY] = info.G->header->ny;

	GMT_Report (API, GMT_MSG_VERBOSE, " ");

	nstack = 0;

	for (opt = list, error = false; !error && opt; opt = opt->next) {

		/* First check if we should skip optional arguments */

		if (strchr ("ADIMNRVbfnr-" GMT_OPT("F"), opt->option)) continue;
		/* if (opt->option == GMT_OPT_OUTFILE) continue; */	/* We do output after the loop */

		op = decode_grd_argument (GMT, opt, &value, localhashnode);

		if (op == GRDMATH_ARG_IS_SAVE) {	/* Time to save the current stack to output and pop the stack */
			if (nstack <= 0) {
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: No items on stack available for output!\n");
				Return (EXIT_FAILURE);
			}

			if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "= %s", opt->arg);

			if (n_items && new_stack < 0 && stack[nstack-1]->constant) {	/* Only a constant provided, set grid accordingly */
				if (!stack[nstack-1]->G) stack[nstack-1]->G = alloc_stack_grid (GMT, info.G);
				stack[nstack-1]->alloc_mode = 1;
				GMT_grd_loop (GMT, info.G, row, col, node) stack[nstack-1]->G->data[node] = (float)stack[nstack-1]->factor;
			}
			this_stack = nstack - 1;
			GMT_grd_init (GMT, stack[this_stack]->G->header, options, true);	/* Update command history only */

			GMT_set_pad (GMT, API->pad);	/* Reset to session default pad before output */

			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, stack[this_stack]->G)) Return (API->error);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, opt->arg, stack[this_stack]->G) != GMT_OK) {
				Return (API->error);
			}
			GMT_set_pad (GMT, 2U);			/* Ensure space for BCs in case an API passed pad == 0 */
			stack[this_stack]->alloc_mode = 2;	/* Since it now is registered */
			if (n_items) nstack--;	/* Pop off the current stack if there is one */
			new_stack = nstack;
			continue;
		}

		if (op != GRDMATH_ARG_IS_FILE && !GMT_access (GMT, opt->arg, R_OK)) GMT_Message (API, GMT_TIME_NONE, "Warning: The number or operator %s may be confused with an existing file %s!\n", opt->arg, opt->arg);

		if (op < GRDMATH_ARG_IS_OPERATOR) {	/* File name or factor */

			if (op == GRDMATH_ARG_IS_FILE && !(strncmp (opt->next->arg, "LDIST", 5U) && strncmp (opt->next->arg, "PDIST", 5U) && strncmp (opt->next->arg, "INSIDE", 6U))) op = GRDMATH_ARG_IS_ASCIIFILE;

			if (nstack == GRDMATH_STACK_SIZE) {	/* Stack overflow */
				error = true;
				continue;
			}
			n_items++;
			if (op == GRDMATH_ARG_IS_NUMBER) {
				stack[nstack]->constant = true;
				stack[nstack]->factor = value;
				error = false;
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "%g ", stack[nstack]->factor);
				nstack++;
				continue;
			}
			else if (op <= GRDMATH_ARG_IS_PI && op >= GRDMATH_ARG_IS_NY) {
				stack[nstack]->constant = true;
				stack[nstack]->factor = special_symbol[GRDMATH_ARG_IS_PI-op];
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "%g ", stack[nstack]->factor);
				nstack++;
				continue;
			}
			else if (op == GRDMATH_ARG_IS_STORE) {
				/* Duplicate stack into stored memory location associated with specified label */
				int last = nstack - 1;
				bool added_new = false;
				if (nstack == 0) {
					GMT_Report (API, GMT_MSG_NORMAL, "No items on stack to put into stored memory!\n");
					Return (EXIT_FAILURE);
				}
				if ((label = grdmath_setlabel (GMT, opt->arg)) == NULL) Return (EXIT_FAILURE);
				if ((k = grdmath_find_stored_item (GMT, recall, n_stored, label)) != -1) {
					GMT_Report (API, GMT_MSG_DEBUG, "Stored memory cell %d named %s is overwritten with new information\n", k, label);
					if (!stack[last]->constant) GMT_memcpy (recall[k]->stored.G->data, stack[last]->G->data, info.G->header->size, float);
				}
				else {	/* Need new named storage place */
					k = n_stored;
					recall[k] = GMT_memory (GMT, NULL, 1, struct GRDMATH_STORE);
					recall[k]->label = strdup (label);
					if (!stack[last]->constant) recall[k]->stored.G = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_DATA, stack[last]->G);
					added_new = true;
					GMT_Report (API, GMT_MSG_DEBUG, "Stored memory cell %d named %s is created with new information\n", k, label);
				}
				recall[k]->stored.constant = stack[last]->constant;
				recall[k]->stored.factor = stack[last]->factor;
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "[--> %s] ", recall[n_stored]->label);
				if (added_new) n_stored++;	/* We added a new item */
				continue;	/* Just go back and process next item */
			}
			else if (op == GRDMATH_ARG_IS_RECALL) {
				/* Add to stack from stored memory location */
				if ((label = grdmath_setlabel (GMT, opt->arg)) == NULL) Return (EXIT_FAILURE);
				if ((k = grdmath_find_stored_item (GMT, recall, n_stored, label)) == -1) {
					GMT_Report (API, GMT_MSG_NORMAL, "No stored memory item with label %s exists!\n", label);
					Return (EXIT_FAILURE);
				}
				if (recall[k]->stored.constant) {	/* Place a stored constant on the stack */
					stack[nstack]->constant = true;
					stack[nstack]->factor = recall[k]->stored.factor;
				}
				else {	/* Place the stored grid on the stack */
					stack[nstack]->constant = false;
					if (!stack[nstack]->G) {
						stack[nstack]->G = alloc_stack_grid (GMT, info.G);
						stack[nstack]->alloc_mode = 1;
					}
					GMT_memcpy (stack[nstack]->G->data, recall[k]->stored.G->data, info.G->header->size, float);
				}
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "@%s ", recall[k]->label);
				nstack++;
				continue;
			}
			else if (op == GRDMATH_ARG_IS_CLEAR) {
				/* Free stored memory location */
				if ((label = grdmath_setlabel (GMT, opt->arg)) == NULL) Return (EXIT_FAILURE);
				if ((k = grdmath_find_stored_item (GMT, recall, n_stored, label)) == -1) {
					GMT_Report (API, GMT_MSG_NORMAL, "No stored memory item with label %s exists!\n", label);
					Return (EXIT_FAILURE);
				}
				if (recall[k]->stored.G && GMT_Destroy_Data (API, &recall[k]->stored.G) != GMT_OK) {
					GMT_Report (API, GMT_MSG_NORMAL, "Failed to free recall item %d\n", k);
				}

				free ((void *)recall[k]->label);
				GMT_free (GMT, recall[k]);
				while (k && k == (int)(n_stored-1) && !recall[k]) k--, n_stored--;	/* Chop off trailing NULL cases */
				continue;
			}

			/* Here we need a matrix */

			stack[nstack]->constant= false;

			if (op == GRDMATH_ARG_IS_X_MATRIX) {		/* Need to set up matrix of x-values */
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "X ");
				if (!stack[nstack]->G) stack[nstack]->G = alloc_stack_grid (GMT, info.G), stack[nstack]->alloc_mode = 1;
				GMT_row_padloop (GMT, info.G, row, node) {
					node = row * info.G->header->mx;
					GMT_memcpy (&stack[nstack]->G->data[node], info.f_grd_x, info.G->header->mx, float);
				}
			}
			else if (op == GRDMATH_ARG_IS_x_MATRIX) {		/* Need to set up matrix of normalized x-values */
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "Xn ");
				if (!stack[nstack]->G) stack[nstack]->G = alloc_stack_grid (GMT, info.G), stack[nstack]->alloc_mode = 1;
				GMT_row_padloop (GMT, info.G, row, node) {
					node = row * info.G->header->mx;
					GMT_memcpy (&stack[nstack]->G->data[node], info.f_grd_xn, info.G->header->mx, float);
				}
			}
			else if (op == GRDMATH_ARG_IS_Y_MATRIX) {	/* Need to set up matrix of y-values */
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "Y ");
				if (!stack[nstack]->G) stack[nstack]->G = alloc_stack_grid (GMT, info.G), stack[nstack]->alloc_mode = 1;
				GMT_grd_padloop (GMT, info.G, row, col, node) stack[nstack]->G->data[node] = info.f_grd_y[row];
			}
			else if (op == GRDMATH_ARG_IS_y_MATRIX) {	/* Need to set up matrix of normalized y-values */
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "Yn ");
				if (!stack[nstack]->G) stack[nstack]->G = alloc_stack_grid (GMT, info.G), stack[nstack]->alloc_mode = 1;
				GMT_grd_padloop (GMT, info.G, row, col, node) stack[nstack]->G->data[node] = info.f_grd_yn[row];
			}
			else if (op == GRDMATH_ARG_IS_ASCIIFILE) {
				if (info.ASCII_file) free (info.ASCII_file);
				if (!stack[nstack]->G) stack[nstack]->G = alloc_stack_grid (GMT, info.G), stack[nstack]->alloc_mode = 1;
				info.ASCII_file = strdup (opt->arg);
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "(%s) ", opt->arg);
			}
			else if (op == GRDMATH_ARG_IS_FILE) {		/* Filename given */
				if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "%s ", opt->arg);
				if ((stack[nstack]->G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, wesn, opt->arg, NULL)) == NULL) {	/* Get header only */
					Return (API->error);
				}
				if (!subset && !GMT_grd_same_shape (GMT, stack[nstack]->G, info.G)) {
					GMT_Report (API, GMT_MSG_NORMAL, "grid files not of same size!\n");
					Return (EXIT_FAILURE);
				}
				else if (!Ctrl->N.active && (!subset && (fabs (stack[nstack]->G->header->wesn[XLO] - info.G->header->wesn[XLO]) > x_noise || fabs (stack[nstack]->G->header->wesn[XHI] - info.G->header->wesn[XHI]) > x_noise ||
					fabs (stack[nstack]->G->header->wesn[YLO] - info.G->header->wesn[YLO]) > y_noise || fabs (stack[nstack]->G->header->wesn[YHI] - info.G->header->wesn[YHI]) > y_noise))) {
					GMT_Report (API, GMT_MSG_NORMAL, "grid files do not cover the same area!\n");
					Return (EXIT_FAILURE);
				}
				if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, wesn, opt->arg, stack[nstack]->G) == NULL) {	/* Get header only */
					Return (API->error);
				}
				stack[nstack]->alloc_mode = 2;
			}
			nstack++;
			continue;
		}

		/* Here we have an operator */

		if ((new_stack = nstack - consumed_operands[op] + produced_operands[op]) >= GRDMATH_STACK_SIZE) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Stack overflow (%s)\n", opt->arg);
			Return (EXIT_FAILURE);
		}

		if (nstack < consumed_operands[op]) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Operation \"%s\" requires %d operands\n", operator[op], consumed_operands[op]);
			Return (EXIT_FAILURE);
		}

		n_items++;
		if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "%s ", operator[op]);

		for (k = produced_operands[op] - consumed_operands[op]; k > 0; k--) {
			if (stack[nstack+k-1]->G) continue;

			/* Must make space for more */

			stack[nstack+k-1]->G = alloc_stack_grid (GMT, info.G);
			stack[nstack+k-1]->alloc_mode = 1;
		}

		/* If operators operates on constants only we may have to make space as well */

		for (kk = 0, k = nstack - consumed_operands[op]; kk < produced_operands[op]; kk++, k++) {
			if (stack[k]->constant && !stack[k]->G) {
				stack[k]->G = alloc_stack_grid (GMT, info.G);
				stack[k]->alloc_mode = 1;
			}
		}

		(*call_operator[op]) (GMT, &info, stack, nstack - 1);	/* Do it */

		if (info.error) Return (info.error);	/* Got an error inside the operator */

		nstack = new_stack;
		for (kk = 1; kk <= produced_operands[op]; kk++) stack[nstack-kk]->constant = false;	/* Now filled with grid */
	}

	/* Clean-up time */

	for (kk = 0; kk < n_stored; kk++) {	/* Free up stored STO/RCL memory */
		if (recall[kk]->stored.G && GMT_Destroy_Data (API, &recall[kk]->stored.G) != GMT_OK) {
			GMT_Report (API, GMT_MSG_NORMAL, "Failed to free recall item %d\n", kk);
		}
		free ((void *)recall[kk]->label);
		GMT_free (GMT, recall[kk]);
	}

	if (GMT_is_verbose (GMT, GMT_MSG_VERBOSE)) GMT_Message (API, GMT_TIME_NONE, "\n");

	if (nstack > 0) GMT_Report (API, GMT_MSG_NORMAL, "Warning: %d more operands left on the stack!\n", nstack);

	Return (EXIT_SUCCESS);
}
