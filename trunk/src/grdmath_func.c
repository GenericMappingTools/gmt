/*--------------------------------------------------------------------
 *	$Id: grdmath_func.c,v 1.24 2011-06-08 19:21:49 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
 * Brief synopsis: grdmath.c is a reverse polish calculator that operates on grid files
 * (and constants) and perform basic mathematical operations
 * on them like add, multiply, etc.
 * Some operators only work on one operand (e.g., log, exp)
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#include "gmt.h"

EXTERN_MSC GMT_LONG gmt_load_macros (struct GMT_CTRL *GMT, char *mtype, struct MATH_MACRO **M);
EXTERN_MSC GMT_LONG gmt_find_macro (char *arg, GMT_LONG n_macros, struct MATH_MACRO *M);
EXTERN_MSC void gmt_free_macros (struct GMT_CTRL *GMT, GMT_LONG n_macros, struct MATH_MACRO **M);	
EXTERN_MSC double GMT_mindist_to_point (struct GMT_CTRL *C, double lon, double lat, struct GMT_TABLE *T, GMT_LONG *id);

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
#define GRDMATH_ARG_IS_BAD		-99

#define GRDMATH_STACK_SIZE		100

struct GRDMATH_CTRL {	/* All control options for this program (except common args) */
	/* active is TRUE if the option has been activated */
	struct Out {	/* = <filename> */
		GMT_LONG active;
		char *file;
	} Out;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct M {	/* -M */
		GMT_LONG active;
	} M;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
};

struct GRDMATH_INFO {
	GMT_LONG nm, size, error;
	char *ASCII_file;
	GMT_LONG convert;		/* Reflects -M */
	float *grd_x, *grd_y;
	float *grd_xn, *grd_yn;
	float *dx, dy;		/* In flat-Earth m if -M is set */
	struct GMT_GRID *G;
};

/* External math-related functions from gmt*.c not in standard gmt*.h files */

void *New_grdmath_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDMATH_CTRL *C = NULL;

	C = GMT_memory (GMT, NULL, 1, struct GRDMATH_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	return ((void *)C);
}

void Free_grdmath_Ctrl (struct GMT_CTRL *GMT, struct GRDMATH_CTRL *C) {	/* Deallocate control structure */
	if (C->Out.file) free ((void *)C->Out.file);	
	GMT_free (GMT, C);
}

GMT_LONG GMT_grdmath_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "grdmath %s [API] - Reverse Polish Notation (RPN) calculator for grid files (element by element)\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdmath [%s] [%s]\n\t[-M] [-N] [%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s]\n",
		GMT_Rgeo_OPT, GMT_I_OPT, GMT_V_OPT, GMT_bi_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_n_OPT, GMT_r_OPT);
	GMT_message (GMT, "	A B op C op D op ... = outfile\n\n");

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\tA, B, etc are grid files, constants, or symbols (see below).\n");
	GMT_message (GMT, "\tThe stack can hold up to %d entries (given enough memory).\n", GRDMATH_STACK_SIZE);
	GMT_message (GMT, "\tTrigonometric operators expect radians.\n");
	GMT_message (GMT, "\tThe operators and number of input and output arguments are:\n\n");
	GMT_message (GMT, "\tName    #args   Returns\n");
	GMT_message (GMT, "\t-----------------------\n");
#include "grdmath_explain.h"
	GMT_message (GMT, "\n\tThe special symbols are:\n\n");
	GMT_message (GMT, "\t  PI	= 3.1415926...\n");
	GMT_message (GMT, "\t  E	= 2.7182818...\n");
	GMT_message (GMT, "\t  EULER	= 0.5772156...\n");
	GMT_message (GMT, "\t  XMIN, XMAX, XINC or NX	= the corresponding constants.\n");
	GMT_message (GMT, "\t  YMIN, YMAX, YINC or NY	= the corresponding constants.\n");
	GMT_message (GMT, "\t  X	= grid with x-coordinates.\n");
	GMT_message (GMT, "\t  Y	= grid with y-coordinates.\n");
	GMT_message (GMT, "\t  Xn	= grid with normalized [-1|+1] x-coordinates.\n");
	GMT_message (GMT, "\t  Yn	= grid with normalized [-1|+1] y-coordinates.\n");
	GMT_message (GMT, "\n\tOPTIONS: (only used if no grid files are passed as arguments).\n\n");
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_message (GMT, "\t-M Handle map units in derivatives.  In this case, dx,dy of grid\n");
	GMT_message (GMT, "\t   will be converted from degrees lon,lat into meters (Flat-earth approximation).\n");
	GMT_message (GMT, "\t   Default computes derivatives in units of data/grid_distance.\n");
	GMT_message (GMT, "\t-N Do not perform strict domain check if several grids are involved.\n");
	GMT_message (GMT, "\t   [Default checks that domain is within %g * [xinc or yinc] of each other].\n", GMT_SMALL);
	GMT_explain_options (GMT, "RV");
	GMT_explain_options (GMT, "fghi");
	GMT_message (GMT, "\t   (Only applies to the input files for operators LDIST, PDIST, and INSIDE).\n");
	GMT_explain_options (GMT, "nF.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdmath_parse (struct GMTAPI_CTRL *C, struct GRDMATH_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdmath and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	GMT_LONG missing_equal = TRUE;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {
			case '<':	/* Input files */
				if (opt->arg[0] == '=' && !opt->arg[1]) {
					missing_equal = FALSE;
					if (opt->next && opt->next->option == GMTAPI_OPT_INFILE) {
						Ctrl->Out.active = TRUE;
						if (opt->next->arg) Ctrl->Out.file = strdup (opt->next->arg);
					}
				}
				break;
			case '#':	/* Numbers */
				break;

			/* Processes program-specific parameters */
			
			case 'I':	/* Grid spacings */
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'M':	/* Map units */
				Ctrl->M.active = TRUE;
				break;
			case 'N':	/* Relax domain check */
				Ctrl->N.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, &GMT->common.r.active, &Ctrl->I.active);

	if (missing_equal) {
		GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Usage is <operations> = [outfile]\n");
		n_errors++;
	}
	if (Ctrl->I.active && !GMT->common.R.active) {
		GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: -I requires the -R option\n");
		n_errors++;
	}
	if (Ctrl->I.active && (Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -I option: Must specify positive increment(s)\n");
		n_errors++;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

void alloc_stack (struct GMT_CTRL *GMT, struct GMT_GRID **G, struct GMT_GRID *Template)
{	/* Allocate a new GMT_GRID structure based on dimensions etc of the Template */
	struct GMT_GRID *New = GMT_create_grid (GMT);
	GMT_memcpy (New->header, Template->header, 1, struct GRD_HEADER);
	New->data = GMT_memory (GMT, NULL, Template->header->size, float);
	*G = New;
}

/* -----------------------------------------------------------------
 *              Definitions of all operator functions
 * -----------------------------------------------------------------*/
/* Note: The OPERATOR: **** lines are used to extract syntax for documentation */

void grd_ABS (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ABS 1 1 abs (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand == 0!\n");
	if (constant[last]) a = fabs (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : fabs ((double)stack[last]->data[node]));
	GMT_grd_pad_zero (GMT, stack[last]);	/* Reset the boundary pad, if needed */
}

void grd_ACOS (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ACOS 1 1 acos (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && fabs (factor[last]) > 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, |operand| > 1 for ACOS!\n");
	if (constant[last]) a = d_acos (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : d_acos ((double)stack[last]->data[node]));
}

void grd_ACOSH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ACOSH 1 1 acosh (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && fabs (factor[last]) > 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand < 1 for ACOSH!\n");
	if (constant[last]) a = acosh (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : acosh ((double)stack[last]->data[node]));
}

void grd_ACOT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ACOT 1 1 acot (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && fabs (factor[last]) > 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, |operand| > 1 for ACOS!\n");
	if (constant[last]) a = atan (1.0 / factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : atan ((double)(1.0 / stack[last]->data[node])));
}

void grd_ACSC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ACSC 1 1 acsc (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && fabs (factor[last]) > 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, |operand| > 1 for ACOS!\n");
	if (constant[last]) a = d_asin (1.0 / factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : d_asin ((double)(1.0 / stack[last]->data[node])));
}

void grd_ADD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ADD 2 1 A + B.  */
{
	GMT_LONG prev = last - 1, node;
	double a, b;

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)(a + b);
	}
}

void grd_AND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: AND 2 1 B if A == NaN, else A.  */
{
	GMT_LONG prev = last - 1, node;
	double a, b;

	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)((GMT_is_dnan (a)) ? b : a);
	}
}

void grd_ASEC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ASEC 1 1 asec (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && fabs (factor[last]) > 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, |operand| > 1 for ACOS!\n");
	if (constant[last]) a = d_acos (1.0 / factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : d_acos ((double)(1.0 / stack[last]->data[node])));
}

void grd_ASIN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ASIN 1 1 asin (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && fabs (factor[last]) > 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, |operand| > 1 for ASIN!\n");
	if (constant[last]) a = d_asin (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : d_asin ((double)stack[last]->data[node]));
}

void grd_ASINH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ASINH 1 1 asinh (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = asinh (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : asinh ((double)stack[last]->data[node]));
}

void grd_ATAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ATAN 1 1 atan (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = atan (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : atan ((double)stack[last]->data[node]));
}

void grd_ATAN2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ATAN2 2 1 atan2 (A, B).  */
{
	GMT_LONG prev = last - 1, node;
	double a, b;

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0 for ATAN2!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for ATAN2!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)d_atan2 (a, b);
	}
}

void grd_ATANH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ATANH 1 1 atanh (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && fabs (factor[last]) >= 1.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, |operand| >= 1 for ATANH!\n");
	if (constant[last]) a = atanh (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : atanh ((double)stack[last]->data[node]));
}

void grd_BEI (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: BEI 1 1 bei (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_bei (GMT, fabs (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_bei (GMT, fabs((double)stack[last]->data[node])));
}

void grd_BER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: BER 1 1 ber (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_ber (GMT, fabs (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_ber (GMT, fabs ((double)stack[last]->data[node])));
}

void grd_CAZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: CAZ 2 1 Cartesian azimuth from grid nodes to stack x,y.  */
{
	GMT_LONG row, col, node, prev = last - 1;
	double x, y, az;

	GMT_grd_padloop (GMT, info->G, row, col, node) {
		x = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		y = (constant[last]) ? factor[last] : stack[last]->data[node];
		az = 90.0 - atan2d (y - (double)info->grd_y[row], x - (double)info->grd_x[col]);
		while (az < -180.0) az += 360.0;
		while (az > +180.0) az -= 360.0;
		stack[prev]->data[node] = (float)az;
	}
}

void grd_CBAZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: CBAZ 2 1 Cartesian backazimuth from grid nodes to stack x,y.  */
{
	GMT_LONG row, col, node, prev = last - 1;
	double x, y, az;

	GMT_grd_padloop (GMT, info->G, row, col, node) {
		x = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		y = (constant[last]) ? factor[last] : stack[last]->data[node];
		az = 270.0 - atan2d (y - (double)info->grd_y[row], x - (double)info->grd_x[col]);
		while (az < -180.0) az += 360.0;
		while (az > +180.0) az -= 360.0;
		stack[prev]->data[node] = (float)az;
	}
}

void grd_CDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: CDIST 2 1 Cartesian distance between grid nodes and stack x,y.  */
{
	GMT_LONG row, col, node, prev = last - 1;
	double a, b;

	GMT_grd_padloop (GMT, info->G, row, col, node) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)hypot (a - (double)info->grd_x[col], b - (double)info->grd_y[row]);
	}
}

void grd_CEIL (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: CEIL 1 1 ceil (A) (smallest integer >= A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = ceil (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : ceil ((double)stack[last]->data[node]));
}

void grd_CHICRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: CHICRIT 2 1 Critical value for chi-squared-distribution, with alpha = A and n = B.  */
{
	GMT_LONG prev = last - 1, node;
	double a, b;

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0 for CHICRIT!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for CHICRIT!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)GMT_chi2crit (GMT, a, b);
	}
}

void grd_CHIDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: CHIDIST 2 1 chi-squared-distribution P(chi2,n), with chi2 = A and n = B.  */
{
	GMT_LONG prev = last - 1, node;
	double a, b, prob;

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0 for CHIDIST!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for CHIDIST!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		GMT_chi2 (GMT, a, b, &prob);
		stack[prev]->data[node] = (float)prob;
	}
}

void grd_CORRCOEFF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: CORRCOEFF 2 1 Correlation coefficient r(A, B).  */
{
	GMT_LONG prev = last - 1, node, pad[4];
	double coeff;

	if (constant[prev] || constant[last]) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: constant operands for CORRCOEFF yields NaNs\n");
		for (node = 0; node < info->size; node++) stack[prev]->data[node] = GMT->session.f_NaN;
		return;
	}
	GMT_memcpy (pad, stack[last]->header->pad, 4, GMT_LONG);	/* Save original pad */
	GMT_grd_pad_off (GMT, stack[prev]);				/* Undo pad if one existed so we can sort */
	GMT_grd_pad_off (GMT, stack[last]);				/* Undo pad if one existed so we can sort */
	coeff = GMT_corrcoeff_f (GMT, stack[prev]->data, stack[last]->data, info->nm, 0);
	GMT_grd_pad_on (GMT, stack[prev], pad);		/* Reinstate the original pad */
	GMT_grd_pad_on (GMT, stack[last], pad);		/* Reinstate the original pad */
	for (node = 0; node < info->size; node++) stack[prev]->data[node] = (float)coeff;
}

void grd_COS (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: COS 1 1 cos (A) (A in radians).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = cos (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : cos ((double)stack[last]->data[node]));
}

void grd_COSD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: COSD 1 1 cos (A) (A in degrees).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = cosd (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : cosd ((double)stack[last]->data[node]));
}

void grd_COSH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: COSH 1 1 cosh (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = cosh (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : cosh ((double)stack[last]->data[node]));
}

void grd_COT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: COT 1 1 cot (A) (A in radians).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = (1.0 / tan (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : (1.0 / tan ((double)stack[last]->data[node])));
}

void grd_COTD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: COTD 1 1 cot (A) (A in degrees).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = (1.0 / tand (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : (1.0 / tand ((double)stack[last]->data[node])));
}

void grd_CPOISS (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: CPOISS 2 1 Cumulative Poisson distribution F(x,lambda), with x = A and lambda = B.  */
{
	GMT_LONG prev = last - 1, node;
	double a, b, prob;

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for CHIDIST!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		GMT_cumpoisson (GMT, a, b, &prob);
		stack[prev]->data[node] = (float)prob;
	}
}

void grd_CSC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: CSC 1 1 csc (A) (A in radians).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = (1.0 / sin (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : (1.0 / sin ((double)stack[last]->data[node])));
}

void grd_CSCD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: CSCD 1 1 csc (A) (A in degrees).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = (1.0 / sind (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : (1.0 / sind ((double)stack[last]->data[node])));
}

void grd_CURV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: CURV 1 1 Curvature of A (Laplacian).  */
{
	GMT_LONG row, col, node, mx;
	double cy;
	float *z = NULL, *cx = NULL;
	
	/* Curvature (Laplacian). */

	if (constant[last]) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand to CURV is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	/* If grid does not have BC rows/cols assigned we apply reasonable conditions:
	 * If -fg we assume geographic grid and use geographic BCs, else we use natural BCs. If the grid
	 * as a BC == GMT_BC_IS_DATA then the pad already constains observations. */

	GMT_BC_init (GMT, stack[last]->header);	/* Initialize grid interpolation and boundary condition parameters */
	GMT_grd_BC_set (GMT, stack[last]);	/* Set boundary conditions */
	
	/* Now, stack[last]->data has boundary rows/cols all set according to the boundary conditions (or actual data).
	 * We can then operate on the interior of the grid and temporarily assign values to the z grid */
	
	z = GMT_memory (GMT, NULL, info->size, float);
	cx = GMT_memory (GMT, NULL, info->G->header->ny, float);
	GMT_row_loop (GMT, info->G, row) cx[row] = (float)+0.5 / (info->dx[row] * info->dx[row]);

	mx = info->G->header->mx;
	cy = -0.5 / (info->dy * info->dy);	/* Because the loop over rows below goes from ymax to ymin we compensate with a minus sign here */

	GMT_grd_loop (GMT, info->G, row, col, node) {
		z[node] = (float)(cx[row] * (stack[last]->data[node+1] - 2.0 * stack[last]->data[node] + stack[last]->data[node-1]) + cy * (stack[last]->data[node+mx] - 2 * stack[last]->data[node] + stack[last]->data[node-mx]));
	}

	GMT_memcpy (stack[last]->data, z, info->size, float);
	GMT_free (GMT, z);
	GMT_free (GMT, cx);
}

void grd_D2DX2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: D2DX2 1 1 d^2(A)/dx^2 2nd derivative.  */
{
	GMT_LONG row, col, node;
	double c, left, next_left;

	/* Central 2nd difference in x */

	if (constant[last]) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand to D2DX2 is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	GMT_row_loop (GMT, info->G, row) {	/* Process d2/dx2 row by row since dx may change with row */
		c = 1.0 / (info->dx[row] * info->dx[row]);
		/* Unless pad has real data we assign outside col values via natural BCs */
		node = GMT_IJP (info->G->header, row, 0);	/* First col */
		if (stack[last]->header->BC[XLO] != GMT_BC_IS_DATA) 
			stack[last]->data[node-1] = (float)(2.0 * stack[last]->data[node] - stack[last]->data[node+1]);	/* Set left node via BC curv = 0 */
		next_left = stack[last]->data[node-1];
		node = GMT_IJP (info->G->header, row, info->G->header->nx-1);	/* Last col */
		if (stack[last]->header->BC[XHI] != GMT_BC_IS_DATA) 
			stack[last]->data[node+1] = (float)(2.0 * stack[last]->data[node] - stack[last]->data[node-1]);	/* Set right node via BC curv = 0 */
		GMT_col_loop (GMT, info->G, row, col, node) {	/* Loop over cols; always save the next left before we update the array at that col */
			left = next_left;
			next_left = stack[last]->data[node];
			stack[last]->data[node] = (float)(c * (stack[last]->data[node+1] - 2.0 * stack[last]->data[node] + left));
		}
	}
	GMT_grd_pad_zero (GMT, stack[last]);	/* Reset the boundary pad */
}

void grd_D2DY2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: D2DY2 1 1 d^2(A)/dy^2 2nd derivative.  */
{
	GMT_LONG row, col, node, mx;
	double c, bottom, next_bottom;

	/* Central 2nd difference in y */

	if (constant[last]) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand to D2DY2 is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	c = 1.0 / (info->dy * info->dy);
	mx = info->G->header->mx;
	GMT_col_loop (GMT, info->G, 0, col, node) {	/* Process d2/dy2 column by column */
		/* Unless pad has real data we assign outside row values via natural BCs */
		if (stack[last]->header->BC[YHI] != GMT_BC_IS_DATA) 
			stack[last]->data[node-mx] = (float)(2.0 * stack[last]->data[node] - stack[last]->data[node+mx]);	/* Set top node via BC curv = 0 */
		next_bottom = stack[last]->data[node-mx];
		node = GMT_IJP (info->G->header, info->G->header->ny-1, col);	/* Last row for this column */
		if (stack[last]->header->BC[YLO] != GMT_BC_IS_DATA) 
			stack[last]->data[node+mx] = (float)(2.0 * stack[last]->data[node] - stack[last]->data[node-mx]);	/* Set bottom node via BC curv = 0 */
		GMT_row_loop (GMT, info->G, row) {
			node = GMT_IJP (info->G->header, row, col);	/* current node in this column */
			bottom = next_bottom;
			next_bottom = stack[last]->data[node];
			stack[last]->data[node] = (float)(c * (stack[last]->data[node+mx] - 2 * stack[last]->data[node] + bottom));
		}
	}
	GMT_grd_pad_zero (GMT, stack[last]);	/* Reset the boundary pad */
}

void grd_D2DXY (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: D2DXY 1 1 d^2(A)/dxdy 2nd derivative.  */
{
	GMT_LONG row, col, node, mx;
	double *cx = NULL, cy;
	float *z = NULL;

	/* Cross derivative d2/dxy = d2/dyx  */

	if (constant[last]) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand to D2DXY is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	/* If grid does not have BC rows/cols assigned we apply reasonable conditions:
	 * If -fg we assume geographic grid and use geographic BCs, else we use natural BCs. If the grid
	 * as a BC == GMT_BC_IS_DATA then the pad already constains observations. */

	GMT_BC_init (GMT, stack[last]->header);	/* Initialize grid interpolation and boundary condition parameters */
	GMT_grd_BC_set (GMT, stack[last]);	/* Set boundary conditions */
	
	/* Now, stack[last]->data has boundary rows/cols all set according to the boundary conditions (or actual data).
	 * We can then operate on the interior of the grid and temporarily assign values to the z grid */
	
	z = GMT_memory (GMT, NULL, info->size, float);
	cx = GMT_memory (GMT, NULL, info->G->header->ny, double);
	GMT_row_loop (GMT, info->G, row) cx[row] = 0.5 / (info->dx[row] * info->dx[row]);

	mx = info->G->header->mx;
	cy = 0.5 / (info->dy * info->dy);

	GMT_grd_loop (GMT, info->G, row, col, node) {
		z[node] = (float)(cx[row] * cy * (stack[last]->data[node-mx+1] - stack[last]->data[node-mx-1] + stack[last]->data[node+mx-1] - stack[last]->data[node+mx+1]));
	}

	GMT_memcpy (stack[last]->data, z, info->size, float);
	GMT_free (GMT, z);
	GMT_free (GMT, cx);
}

void grd_D2R (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: D2R 1 1 Converts Degrees to Radians.  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = factor[last] * D2R;
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : (stack[last]->data[node] * D2R));
}

void grd_DDX (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: DDX 1 1 d(A)/dx Central 1st derivative.  */
{
	GMT_LONG row, col, node;
	double c, left, next_left;

	/* Central 1st difference in x */

	if (constant[last]) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand to DDX is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	GMT_row_loop (GMT, info->G, row) {	/* Process d2/dx2 row by row since dx may change with row */
		c = 0.5 / info->dx[row];
		/* Unless pad has real data we assign outside col values via natural BCs */
		node = GMT_IJP (info->G->header, row, 0);	/* First col */
		if (stack[last]->header->BC[XLO] != GMT_BC_IS_DATA) 
			stack[last]->data[node-1] = (float)(2.0 * stack[last]->data[node] - stack[last]->data[node+1]);	/* Set left node via BC curv = 0 */
		next_left = stack[last]->data[node-1];
		node = GMT_IJP (info->G->header, row, info->G->header->nx-1);	/* Last col */
		if (stack[last]->header->BC[XHI] != GMT_BC_IS_DATA) 
			stack[last]->data[node+1] = (float)(2.0 * stack[last]->data[node] - stack[last]->data[node-1]);	/* Set right node via BC curv = 0 */
		GMT_col_loop (GMT, info->G, row, col, node) {	/* Loop over cols; always save the next left before we update the array at that col */
			left = next_left;
			next_left = stack[last]->data[node];
			stack[last]->data[node] = (float)(c * (stack[last]->data[node+1] - left));
		}
	}
}

void grd_DDY (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: DDY 1 1 d(A)/dy Central 1st derivative.  */
{
	GMT_LONG row, col, node, mx;
	double c, bottom, next_bottom;

	/* Central 1st difference in y */

	if (constant[last]) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand to DDY is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	c = -0.5 / info->dy;	/* Because the loop over j below goes from ymax to ymin we compensate with a minus sign here */
	mx = info->G->header->mx;
	GMT_col_loop (GMT, info->G, 0, col, node) {	/* Process d2/dy2 column by column */
		/* Unless pad has real data we assign outside row values via natural BCs */
		if (stack[last]->header->BC[YHI] != GMT_BC_IS_DATA) 	/* Set top node via BC curv = 0 */
			stack[last]->data[node-mx] = (float)(2.0 * stack[last]->data[node] - stack[last]->data[node+mx]);
		next_bottom = stack[last]->data[node-mx];
		node = GMT_IJP (info->G->header, info->G->header->ny-1, col);	/* Last row for this column */
		if (stack[last]->header->BC[YLO] != GMT_BC_IS_DATA) 	/* Set bottom node via BC curv = 0 */
			stack[last]->data[node+mx] = (float)(2.0 * stack[last]->data[node] - stack[last]->data[node-mx]);
		GMT_row_loop (GMT, info->G, row) {
			node = GMT_IJP (info->G->header, row, col);	/* current node in this column */
			bottom = next_bottom;
			next_bottom = stack[last]->data[node];
			stack[last]->data[node] = (float)(c * (stack[last]->data[node+mx] - bottom));
		}
	}
	GMT_grd_pad_zero (GMT, stack[last]);	/* Reset the boundary pad */
}

void grd_DEG2KM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: DEG2KM 1 1 Converts Spherical Degrees to Kilometers.  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = factor[last] * GMT->current.proj.DIST_KM_PR_DEG;
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : (stack[last]->data[node] * GMT->current.proj.DIST_KM_PR_DEG));
}

void grd_DILOG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: DILOG 1 1 dilog (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_dilog (GMT, factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_dilog (GMT, stack[last]->data[node]));
}

void grd_DIV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: DIV 2 1 A / B.  */
{
	GMT_LONG prev = last - 1, node;
	double a, b;
	void grd_MUL (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last);

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_FATAL, "Warning: Divide by zero gives NaNs\n");
	if (constant[last]) {	/* Turn divide into multiply */
		a = factor[last];	/* Save original factor */
		factor[last] = 1.0 / factor[last];
		grd_MUL (GMT, info, stack, constant, factor, last);
		factor[last] = a;	/* Restore factor */
		return;
	}

	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)(a / b);
	}
}

void grd_DUP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: DUP 1 2 Places duplicate of A on the stack.  */
{
	GMT_LONG next, node;

	next = last + 1;
	constant[next] = constant[last];
	factor[next] = factor[last];
	if (constant[last]) {	/* Time to fess up */
		for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)factor[last];
	}

	GMT_memcpy (stack[next]->data, stack[last]->data, info->size, float);
}

void grd_ERF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ERF 1 1 Error function erf (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = erf (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : erf ((double)stack[last]->data[node]));
}

void grd_ERFC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ERFC 1 1 Complementary Error function erfc (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = erfc (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : erfc ((double)stack[last]->data[node]));
}

void grd_EQ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: EQ 2 1 1 if A == B, else 0.  */
{
	GMT_LONG prev = last - 1, node;
	double a, b;

	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (GMT_is_dnan (a) || GMT_is_dnan (b)) ? GMT->session.f_NaN : (float)(a == b);
	}
}

void grd_ERFINV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ERFINV 1 1 Inverse error function of A.  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_erfinv (GMT, factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_erfinv (GMT, (double)stack[last]->data[node]));
}

void grd_EXCH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: EXCH 2 2 Exchanges A and B on the stack.  */
{
	GMT_LONG prev = last - 1, node;

	for (node = 0; node < info->size; node++) {
		if (constant[prev]) stack[prev]->data[node] = (float)factor[prev];
		if (constant[last]) stack[last]->data[node] = (float)factor[last];
		f_swap (stack[last]->data[node], stack[prev]->data[node]);
	}
	d_swap (factor[last], factor[prev]);
	l_swap (constant[last], constant[prev]);
}

void grd_EXP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: EXP 1 1 exp (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = exp (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : exp ((double)stack[last]->data[node]));
}

void grd_FACT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: FACT 1 1 A! (A factorial).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_factorial (GMT, (GMT_LONG)irint(factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_factorial (GMT, (GMT_LONG)irint(stack[last]->data[node])));
}

/* Subroutines for grd_EXTREMA */

GMT_LONG do_derivative (float *z, GMT_LONG this_node, GMT_LONG off, GMT_LONG type)
{	/* Examine a line of 3-points centered on the current this_node.
	 * z is the data matrix.
	 * off is shift to add to get index of the next value and subtract to get previous node.
	 * type: 0 means x-, or -y derivative, 1 means diagonal (N45E or N135E direction)  */

	GMT_LONG next_node, prev_node, nan;

	nan = (type == 0) ? -2 : 0;	/* Return -2 if we find two nans except for diagonals where we return 0 */
	
	/* Because of padding, all internal nodes have neighbors on either side (left, right, above, below) */
	
	prev_node = this_node - off;	/* Previous node in line */
	next_node = this_node + off;	/* Next node in line */
	if (GMT_is_fnan (z[prev_node])) {			/* At least one of the two neighbor points is a NaN */
		if (GMT_is_fnan (z[next_node])) return (nan);	/* Both points are NaN, return -2 (or 0 if diagonal) */
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

void grd_EXTREMA (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: EXTREMA 1 1 Local Extrema: +2/-2 is max/min, +1/-1 is saddle with max/min in x, 0 elsewhere.  */
{
	GMT_LONG row, col, node, mx1, dx, dy, diag, product;
	float *z = NULL;

	/* Find local extrema in grid */

	if (constant[last]) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand to EXTREMA is constant!\n");
		GMT_memset (stack[last], info->size, float);
		return;
	}

	/* If grid does not have BC rows/cols assigned we apply reasonable conditions:
	 * If -fg we assume geographic grid and use geographic BCs, else we use natural BCs. If the grid
	 * as a BC == GMT_BC_IS_DATA then the pad already constains observations. */

	GMT_BC_init (GMT, stack[last]->header);	/* Initialize grid interpolation and boundary condition parameters */
	GMT_grd_BC_set (GMT, stack[last]);	/* Set boundary conditions */
	
	/* Now, stack[last]->data has boundary rows/cols all set according to the boundary conditions (or actual data).
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

		if (GMT_is_fnan (stack[last]->data[node])) continue;	/* No extrema if point is NaN */

		if ((dx = do_derivative (stack[last]->data, node, 1, 0)) == -2) continue;	/* Too many NaNs or flat x-line */
		if ((dy = do_derivative (stack[last]->data, node, info->G->header->mx, 0)) == -2) continue;	/* Too many NaNs or flat y-line */

		if ((product = dx * dy) == 0) continue;	/* No min or max possible */
		if (product < 0) {	/* Saddle point - don't need to check diagonals */
			z[node] = (float)((dx > 0) ? +1 : -1);
			continue;
		}

		/* Need to examine diagonal trends to verify min or max */

		if ((diag = do_derivative (stack[last]->data, node, -mx1, 1)) == -2) continue;	/* Sorry, no extrema along diagonal N45E */
		if (diag != 0 && diag != dx) continue;						/* Sorry, extrema of opposite sign along diagonal N45E  */
		if ((diag = do_derivative (stack[last]->data, node,  mx1, 1)) == -2) continue;	/* Sorry, no extrema along diagonal N135E */
		if (diag != 0 && diag != dx) continue;						/* Sorry, extrema of opposite sign along diagonal N135E  */

		/* OK, we have a min or max point; just use dx to check which kind */

		z[node] = (float)((dx > 0) ? +2 : -2);
	}

	GMT_memcpy (stack[last]->data, z, info->size, float);
	GMT_memset (stack[last]->header->BC, 4, GMT_LONG);	/* No BC padding in this array */
	GMT_free (GMT, z);
}

void grd_FCRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: FCRIT 3 1 Critical value for F-distribution, with alpha = A, n1 = B, and n2 = C.  */
{
	GMT_LONG node, nu1, nu2, prev1, prev2;
	double alpha;

	prev1 = last - 1;
	prev2 = last - 2;
	if (constant[prev2] && factor[prev2] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0 for FCRIT!\n");
	if (constant[prev1] && factor[prev1] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for FCRIT!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand three == 0 for FCRIT!\n");
	for (node = 0; node < info->size; node++) {
		alpha = (constant[prev2]) ? factor[prev2] : stack[prev2]->data[node];
		nu1 = irint ((double)((constant[prev1]) ? factor[prev1] : stack[prev1]->data[node]));
		nu2 = irint ((double)((constant[last]) ? factor[last] : stack[last]->data[node]));
		stack[prev2]->data[node] = (float)GMT_Fcrit (GMT, alpha, (double)nu1, (double)nu2);
	}
}

void grd_FDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: FDIST 3 1 F-distribution Q(F,n1,n2), with F = A, n1 = B, and n2 = C.  */
{
	GMT_LONG node, nu1, nu2, prev1, prev2;
	double F, chisq1, chisq2 = 1.0, prob;

	prev1 = last - 1;
	prev2 = last - 2;
	if (constant[prev1] && factor[prev1] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for FDIST!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand three == 0 for FDIST!\n");
	for (node = 0; node < info->size; node++) {
		F = (constant[prev2]) ? factor[prev2] : stack[prev2]->data[node];
		nu1 = (GMT_LONG)(irint ((double)((constant[prev1]) ? factor[prev1] : stack[prev1]->data[node])));
		nu2 = (GMT_LONG)(irint ((double)((constant[last]) ? factor[last] : stack[last]->data[node])));
		/* Since GMT_f_q needs chisq1 and chisq2, we set chisq2 = 1 and solve for chisq1 */
		chisq1 = F * nu1 / nu2;
		(void) GMT_f_q (GMT, chisq1, nu1, chisq2, nu2, &prob);
		stack[prev2]->data[node] = (float)prob;
	}
}

void grd_FLIPLR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: FLIPLR 1 1 Reverse order of values in each row.  */
{
	GMT_LONG node, mx1, row, col_l, col_r, mx_half;

	/* Reverse order of all rows */

	if (constant[last]) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand to FLIPLR is constant!\n");
		return;
	}

	/* This must also apply to the pads since any BCs there must be flipped as well, hence a local loop is used */
	
	mx_half = info->G->header->mx / 2;
	mx1 = info->G->header->mx - 1;
	for (row = node = 0; row < info->G->header->my; row++, node += info->G->header->mx) {	/* Do this to all rows */
		for (col_l = 0, col_r = mx1; col_l < mx_half; col_l++, col_r--) f_swap (stack[last]->data[node+col_l], stack[last]->data[node+col_r]);
	}
}

void grd_FLIPUD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: FLIPUD 1 1 Reverse order of values in each column.  */
{
	GMT_LONG my1, mx, row_t, row_b, col, my_half;

	/* Reverse order of all columns */

	if (constant[last]) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand to FLIPLR is constant!\n");
		return;
	}

	/* This must also apply to the pads since any BCs there must be flipped as well, hence a local loop is used */

	my_half = info->G->header->my / 2;
	my1 = info->G->header->my - 1;
	mx = info->G->header->mx;
	for (col = 0; col < mx; col++) {	/* Do this to all cols */
		for (row_t = 0, row_b = my1; row_t < my_half; row_t++, row_b--) f_swap (stack[last]->data[row_t*mx+col], stack[last]->data[row_b*mx+col]);
	}
}

void grd_FLOOR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: FLOOR 1 1 floor (A) (greatest integer <= A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = floor (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : floor ((double)stack[last]->data[node]));
}

void grd_FMOD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: FMOD 2 1 A % B (remainder after truncated division).  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, using FMOD 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)fmod (a, b);
	}
}

void grd_GE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: GE 2 1 1 if A >= B, else 0.  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (GMT_is_dnan (a) || GMT_is_dnan (b)) ? GMT->session.f_NaN : (float)(a >= b);
	}
}

void grd_GT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: GT 2 1 1 if A > B, else 0.  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (GMT_is_dnan (a) || GMT_is_dnan (b)) ? GMT->session.f_NaN : (float)(a > b);
	}
}

void grd_HYPOT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: HYPOT 2 1 hypot (A, B) = sqrt (A*A + B*B).  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)hypot (a, b);
	}
}

void grd_I0 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: I0 1 1 Modified Bessel function of A (1st kind, order 0).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_i0 (GMT, factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_i0 (GMT, (double)stack[last]->data[node]));
}

void grd_I1 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: I1 1 1 Modified Bessel function of A (1st kind, order 1).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_i1 (GMT, factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_i1 (GMT, (double)stack[last]->data[node]));
}

void grd_IN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: IN 2 1 Modified Bessel function of A (1st kind, order B).  */
{
	GMT_LONG node, prev = last - 1, order = 0, simple = FALSE;
	double b = 0.0;

	if (constant[last]) {
		if (factor[last] < 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order < 0 for IN!\n");
		if (fabs (rint(factor[last]) - factor[last]) > GMT_SMALL) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order not an integer for IN!\n");
		order = irint (fabs (factor[last]));
		if (constant[prev]) {
			b = GMT_in (GMT, order, fabs (factor[prev]));
			simple = TRUE;
		}
	}
	for (node = 0; node < info->size; node++) {
		if (simple)
			stack[prev]->data[node] = (float)b;
		else {
			if (!constant[last]) order = irint (fabs ((double)stack[last]->data[node]));
			stack[last]->data[node] = (float)GMT_in (GMT, order, fabs ((double)stack[prev]->data[node]));
		}
	}
}

void grd_INRANGE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: INRANGE 3 1 1 if B <= A <= C, else 0.  */
{
	GMT_LONG node, prev1, prev2, inrange;
	float a = 0.0, b = 0.0, c = 0.0;

	/* last is C */
	prev1 = last - 1;	/* This is B */
	prev2 = last - 2;	/* This is A */

	/* Set to 1 where B <= A <= C, 0 elsewhere, except where
	 * A, B, or C = NaN, in which case we set answer to NaN */

	if (constant[prev2]) a = (float)factor[prev2];
	if (constant[prev1]) b = (float)factor[prev1];
	if (constant[last])  c = (float)factor[last];

	for (node = 0; node < info->size; node++) {
		if (!constant[prev2]) a = stack[prev2]->data[node];
		if (!constant[prev1]) b = stack[prev1]->data[node];
		if (!constant[last])  c = stack[last]->data[node];

		if (GMT_is_fnan (a) || GMT_is_fnan (b) || GMT_is_fnan (c)) {
			stack[prev2]->data[node] = GMT->session.f_NaN;
			continue;
		}

		inrange = (b <= a && a <= c);
		stack[prev2]->data[node] = (float)inrange;
	}
}

void grd_INSIDE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: INSIDE 1 1 1 when inside or on polygon(s) in A, else 0.  */
{	/* Suitable for geographic (lon, lat) data and polygons */
	GMT_LONG row, col, node, P, inside;
	struct GMT_TABLE *T = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_LINE_SEGMENT *S = NULL;

	GMT_set_cols (GMT, GMT_IN, 2);
	GMT_skip_xy_duplicates (GMT, TRUE);	/* Avoid repeating x/y points in polygons */
	if (GMT_Get_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, NULL, 0, (void **)&(info->ASCII_file), (void **)&D)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error in operator INSIDE reading file %s!\n", info->ASCII_file);
		info->error = GMT_DATA_READ_ERROR;
		return;
	}
	GMT_skip_xy_duplicates (GMT, FALSE);	/* Reset */
	T = D->table[0];	/* Only one table in a single file */
	GMT_grd_padloop (GMT, info->G, row, col, node) {	/* Visit each node */
		for (P = inside = 0; !inside && P < T->n_segments; P++) {
			S = T->segment[P];
			if (GMT_polygon_is_hole (S)) continue;	/* Holes are handled within GMT_inonout */
			inside = GMT_inonout (GMT, (double)info->grd_x[col], (double)info->grd_y[row], S);
		}
		stack[last]->data[node] = (float)((inside) ? 1.0 : 0.0);
	}

	/* Free memory used for pol */

	GMT_Destroy_Data (GMT->parent, GMT_ALLOCATED, (void **)&D);
}

void grd_INV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: INV 1 1 1 / A.  */
{
	GMT_LONG node;
	double a;

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_FATAL, "Warning: inverse of zero gives NaNs\n");
	if (constant[last]) factor[last] = (factor[last] == 0.0) ? GMT->session.f_NaN : 1.0 / factor[last];
	for (node = 0; node < info->size; node++) {
		a = (constant[last]) ? factor[last] : 1.0 / stack[last]->data[node];
		stack[last]->data[node] = (float)a;
	}
}

void grd_ISNAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ISNAN 1 1 1 if A == NaN, else 0.  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_is_dnan (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_is_fnan (stack[last]->data[node]));
}

void grd_J0 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: J0 1 1 Bessel function of A (1st kind, order 0).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = j0 (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : j0 ((double)stack[last]->data[node]));
}

void grd_J1 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: J1 1 1 Bessel function of A (1st kind, order 1).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = j1 (fabs (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : j1 (fabs ((double)stack[last]->data[node])));
}

void grd_JN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: JN 2 1 Bessel function of A (1st kind, order B).  */
{
	GMT_LONG node, prev = last - 1, simple = FALSE, order = 0;
	double b = 0.0;

	if (constant[last]) {
		if (factor[last] < 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order < 0 for JN!\n");
		if (fabs (rint(factor[last]) - factor[last]) > GMT_SMALL) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order not an integer for JN!\n");
		order = irint (fabs (factor[last]));
		if (constant[prev]) {
			b = jn ((int)order, fabs (factor[prev]));
			simple = TRUE;
		}
	}
	for (node = 0; node < info->size; node++) {
		if (simple)
			stack[prev]->data[node] = (float)b;
		else {
			if (!constant[last]) order = irint (fabs ((double)stack[last]->data[node]));
			stack[last]->data[node] = (float)jn ((int)order, fabs ((double)stack[prev]->data[node]));
		}
	}
}

void grd_K0 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: K0 1 1 Modified Kelvin function of A (2nd kind, order 0).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_k0 (GMT, factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_k0 (GMT, (double)stack[last]->data[node]));
}

void grd_K1 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: K1 1 1 Modified Bessel function of A (2nd kind, order 1).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_k1 (GMT, factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_k1 (GMT, (double)stack[last]->data[node]));
}

void grd_KEI (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: KEI 1 1 kei (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_kei (GMT, fabs (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_kei (GMT, fabs ((double)stack[last]->data[node])));
}

void grd_KER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: KER 1 1 ker (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_ker (GMT, fabs (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_ker (GMT, fabs ((double)stack[last]->data[node])));
}

void grd_KM2DEG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: KM2DEG 1 1 Converts Kilometers to Spherical Degrees.  */
{
	GMT_LONG node;
	double a = 0.0, f = 1.0 / GMT->current.proj.DIST_KM_PR_DEG;

	if (constant[last]) a = factor[last] * f;
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : (stack[last]->data[node] * f));
}

void grd_KN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: KN 2 1 Modified Bessel function of A (2nd kind, order B).  */
{
	GMT_LONG node, prev = last - 1, order = 0, simple = FALSE;
	double b = 0.0;

	if (constant[last]) {
		if (factor[last] < 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order < 0 for KN!\n");
		if (fabs (rint(factor[last]) - factor[last]) > GMT_SMALL) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order not an integer for KN!\n");
		order = irint (fabs (factor[last]));
		if (constant[prev]) {
			b = GMT_kn (GMT, order, fabs (factor[prev]));
			simple = TRUE;
		}
	}
	for (node = 0; node < info->size; node++) {
		if (simple)
			stack[prev]->data[node] = (float)b;
		else {
			if (!constant[last]) order = irint (fabs ((double)stack[last]->data[node]));
			stack[last]->data[node] = (float)GMT_kn (GMT, order, fabs ((double)stack[prev]->data[node]));
		}
	}
}

void grd_KURT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: KURT 1 1 Kurtosis of A.  */
{
	GMT_LONG row, col, node, n = 0;
	double mean = 0.0, sum2 = 0.0, kurt = 0.0, delta;
	float f_kurt;

	if (constant[last]) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->data[node] = GMT->session.f_NaN;
		return;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	GMT_grd_loop (GMT, info->G, row, col, node) {
		if (GMT_is_fnan (stack[last]->data[node])) continue;
		n++;
		delta = (double)stack[last]->data[node] - mean;
		mean += delta / n;
		sum2 += delta * ((double)stack[last]->data[node] - mean);
	}
	if (n > 1) {
		GMT_grd_loop (GMT, info->G, row, col, node) {
			if (GMT_is_fnan (stack[last]->data[node])) continue;
			delta = (double)stack[last]->data[node] - mean;
			kurt += pow (delta, 4.0);
		}
		sum2 /= (n - 1);
		kurt = kurt / (n * sum2 * sum2) - 3.0;
		f_kurt = (float)kurt;
	}
	else
		f_kurt = GMT->session.f_NaN;
	for (node = 0; node < info->size; node++) stack[last]->data[node] = f_kurt;
}

void grd_LDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: LDIST 1 1 Compute distance (in degrees if -fg) from lines in multi-segment ASCII file A.  */
{
	GMT_LONG row, col, node;
	double d;
	struct GMT_TABLE *line = NULL;
	struct GMT_DATASET *D = NULL;

	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Spherical (in degrees) */
		GMT_init_distaz (GMT, 'd', 1 + GMT_sph_mode (GMT), GMT_MAP_DIST);
#ifdef GMT_COMPAT
		GMT_report (GMT, GMT_MSG_COMPAT, "Warning: LDIST returns distances in spherical degrees; in GMT4 it returned km.  Use DEG2KM for conversion, if needed.\n");
#endif
	}
	else
		GMT_init_distaz (GMT, 'X', 0, GMT_MAP_DIST);	/* Cartesian */

	GMT_set_cols (GMT, GMT_IN,  2);
	if (GMT_Get_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, NULL, 0, (void **)&(info->ASCII_file), (void **)&D)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error in operator LDIST reading file %s!\n", info->ASCII_file);
		info->error = GMT_DATA_READ_ERROR;
		return;
	}
	line = D->table[0];	/* Only one table in a single file */

	GMT_grd_padloop (GMT, info->G, row, col, node) {	/* Visit each node */
		(void) GMT_near_lines (GMT, (double)info->grd_x[col], (double)info->grd_y[row], line, TRUE, &d, NULL, NULL);
		stack[last]->data[node] = (float)d;
		if (col == 0) GMT_report (GMT, GMT_MSG_VERBOSE, "Row %ld\n", row);
	}

	/* Free memory used for line */

	GMT_Destroy_Data (GMT->parent, GMT_ALLOCATED, (void **)&D);
}

void grd_LE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: LE 2 1 1 if A <= B, else 0.  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (GMT_is_dnan (a) || GMT_is_dnan (b)) ? GMT->session.f_NaN : (float)(a <= b);
	}
}

void grd_LOG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: LOG 1 1 log (A) (natural log).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, argument to log = 0\n");

	if (constant[last]) a = d_log (GMT, fabs (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : d_log (GMT, fabs ((double)stack[last]->data[node])));
}

void grd_LOG10 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: LOG10 1 1 log10 (A) (base 10).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, argument to log10 = 0\n");

	if (constant[last]) a = d_log10 (GMT, fabs (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : d_log10 (GMT, fabs ((double)stack[last]->data[node])));
}

void grd_LOG1P (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: LOG1P 1 1 log (1+A) (accurate for small A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && factor[last] < 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, argument to log1p < 0\n");

	if (constant[last]) a = d_log1p (GMT, fabs (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : d_log1p (GMT, fabs ((double)stack[last]->data[node])));
}

void grd_LOG2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: LOG2 1 1 log2 (A) (base 2).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, argument to log2 = 0\n");

	if (constant[last]) a = d_log (GMT, fabs (factor[last])) * M_LN2_INV;
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)(((constant[last]) ? a : d_log (GMT, fabs ((double)stack[last]->data[node]))) * M_LN2_INV);
}

void grd_LMSSCL (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: LMSSCL 1 1 LMS scale estimate (LMS STD) of A.  */
{
	GMT_LONG GMT_mode_selection = 0, GMT_n_multiples = 0, n, node, pad[4];
	double mode, lmsscl;
	float lmsscl_f;

	if (constant[last]) {	/* Trivial case */
		GMT_memset (stack[last]->data, info->size, float);
		return;
	}

	/* Sort will put any NaNs to the end - we then count to find the real data */

	GMT_memcpy (pad, stack[last]->header->pad, 4, GMT_LONG);	/* Save original pad */
	GMT_grd_pad_off (GMT, stack[last]);				/* Undo pad if one existed so we can sort */
	GMT_sort_array (GMT, (void *)stack[last]->data, info->nm, GMT_FLOAT_TYPE);
	for (n = info->nm; GMT_is_fnan (stack[last]->data[n-1]) && n > 1; n--);
	if (n) {
		GMT_mode_f (GMT, stack[last]->data, n, n/2, 0, GMT_mode_selection, &GMT_n_multiples, &mode);
		GMT_getmad_f (GMT, stack[last]->data, n, mode, &lmsscl);
		lmsscl_f = (float)lmsscl;
	}
	else
		lmsscl_f = GMT->session.f_NaN;

	GMT_memset (stack[last]->data, info->size, float);	/* Wipes everything */
	GMT_grd_pad_on (GMT, stack[last], pad);		/* Reinstate the original pad */
	for (node = 0; node < info->size; node++) stack[last]->data[node] = lmsscl_f;
	
	if (GMT_n_multiples > 0) GMT_report (GMT, GMT_MSG_FATAL, "Warning: %ld Multiple modes found\n", GMT_n_multiples);
}

void grd_LOWER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: LOWER 1 1 The lowest (minimum) value of A.  */
{
	GMT_LONG row, col, node;
	float low = FLT_MAX;

	if (constant[last]) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)factor[last];
		return;
	}

	GMT_grd_loop (GMT, info->G, row, col, node) {	/* First we must find the lowest value in the grid */
		if (GMT_is_fnan (stack[last]->data[node])) continue;
		if (stack[last]->data[node] < low) low = stack[last]->data[node];
	}
	/* Now copy that low value everywhere */
	for (node = 0; node < info->size; node++) if (!GMT_is_fnan (stack[last]->data[node])) stack[last]->data[node] = low;
}

void grd_LRAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: LRAND 2 1 Laplace random noise with mean A and std. deviation B.  */
{
	GMT_LONG node, prev;
	double a = 0.0, b = 0.0;

	prev = last - 1;
	if (constant[prev]) a = factor[prev];
	if (constant[last]) b = factor[last];
	for (node = 0; node < info->size; node++) {
		if (!constant[prev]) a = (double)stack[prev]->data[node];
		if (!constant[last]) b = (double)stack[last]->data[node];
		stack[prev]->data[node] = (float)(a + b * GMT_lrand (GMT));
	}
}

void grd_LT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: LT 2 1 1 if A < B, else 0.  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (GMT_is_dnan (a) || GMT_is_dnan (b)) ? GMT->session.f_NaN : (float)(a < b);
	}
}

void grd_MAD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: MAD 1 1 Median Absolute Deviation (L1 STD) of A.  */
{
	GMT_LONG  node, n, pad[4];
	double mad, med;
	float mad_f;

	if (constant[last]) {	/* Trivial case */
		GMT_memset (stack[last], info->size, float);
		return;
	}

	/* Sort will put any NaNs to the end - we then count to find the real data */

	GMT_memcpy (pad, stack[last]->header->pad, 4, GMT_LONG);	/* Save original pad */
	GMT_grd_pad_off (GMT, stack[last]);				/* Undo pad if one existed so we can sort */
	GMT_sort_array (GMT, (void *)stack[last]->data, info->nm, GMT_FLOAT_TYPE);
	for (n = info->nm; GMT_is_fnan (stack[last]->data[n-1]) && n > 1; n--);
	if (n) {
		med = (n%2) ? stack[last]->data[n/2] : (float)(0.5 * (stack[last]->data[(n-1)/2] + stack[last]->data[n/2]));
		GMT_getmad_f (GMT, stack[last]->data, n, med, &mad);
		mad_f = (float)mad;
	}
	else
		mad_f = GMT->session.f_NaN;

	GMT_grd_pad_on (GMT, stack[last], pad);		/* Reinstate the original pad */
	for (node = 0; node < info->size; node++) stack[last]->data[node] = mad_f;
}

void grd_MAX (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: MAX 2 1 Maximum of A and B.  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)MAX (a, b);
	}
}

void grd_MEAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: MEAN 1 1 Mean value of A.  */
{
	GMT_LONG row, col, node, n_a = 0;
	double sum_a = 0.0;

	if (constant[last]) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)factor[last];
		return;
	}

	GMT_grd_loop (GMT, info->G, row, col, node) {
		if (GMT_is_fnan (stack[last]->data[node])) continue;
		sum_a += stack[last]->data[node];
		n_a++;
	}
	sum_a = (n_a) ? sum_a / (double)n_a : 0.0;
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)sum_a;
}

void grd_MED (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: MED 1 1 Median value of A.  */
{
	GMT_LONG node, n, pad[4];
	float med;

	if (constant[last]) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)factor[last];
		return;
	}

	GMT_memcpy (pad, stack[last]->header->pad, 4, GMT_LONG);	/* Save original pad */
	GMT_grd_pad_off (GMT, stack[last]);				/* Undo pad if one existed so we can sort */
	GMT_sort_array (GMT, (void *)stack[last], info->nm, GMT_FLOAT_TYPE);
	for (n = info->nm; GMT_is_fnan (stack[last]->data[n-1]) && n > 1; n--);
	if (n)
		med = (n%2) ? stack[last]->data[n/2] : (float)(0.5 * (stack[last]->data[(n-1)/2] + stack[last]->data[n/2]));
	else
		med = GMT->session.f_NaN;

	GMT_grd_pad_on (GMT, stack[last], pad);		/* Reinstate the original pad */
	for (node = 0; node < info->size; node++) stack[last]->data[node] = med;
}

void grd_MIN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: MIN 2 1 Minimum of A and B.  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)MIN (a, b);
	}
}

void grd_MOD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: MOD 2 1 A mod B (remainder after floored division).  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, using MOD 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)MOD (a, b);
	}
}

void grd_MODE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: MODE 1 1 Mode value (Least Median of Squares) of A.  */
{
	GMT_LONG GMT_mode_selection = 0, GMT_n_multiples = 0, node, n, pad[4];
	double mode = 0.0;

	if (constant[last]) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)factor[last];
		return;
	}

	GMT_memcpy (pad, stack[last]->header->pad, 4, GMT_LONG);	/* Save original pad */
	GMT_grd_pad_off (GMT, stack[last]);				/* Undo pad if one existed so we can sort */
	GMT_sort_array (GMT, (void *)stack[last]->data, info->nm, GMT_FLOAT_TYPE);
	for (n = info->nm; GMT_is_fnan (stack[last]->data[n-1]) && n > 1; n--);
	if (n)
		GMT_mode_f (GMT, stack[last]->data, n, n/2, 0, GMT_mode_selection, &GMT_n_multiples, &mode);
	else
		mode = GMT->session.f_NaN;

	GMT_grd_pad_on (GMT, stack[last], pad);		/* Reinstate the original pad */
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)mode;
	if (GMT_n_multiples > 0) GMT_report (GMT, GMT_MSG_FATAL, "Warning: %ld Multiple modes found\n", GMT_n_multiples);
}

void grd_MUL (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: MUL 2 1 A * B.  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)(a * b);
	}
}

void grd_NAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: NAN 2 1 NaN if A == B, else A.  */
{
	GMT_LONG node, prev;
	double a = 0.0, b = 0.0;

	prev = last - 1;
	if (constant[prev]) a = factor[prev];
	if (constant[last]) b = factor[last];
	for (node = 0; node < info->size; node++) {
		if (!constant[prev]) a = stack[prev]->data[node];
		if (!constant[last]) b = stack[last]->data[node];
		stack[prev]->data[node] = ((a == b) ? GMT->session.f_NaN : (float)a);
	}
}

void grd_NEG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: NEG 1 1 -A.  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand == 0!\n");
	if (constant[last]) a = -factor[last];
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : -stack[last]->data[node]);
}

void grd_NEQ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: NEQ 2 1 1 if A != B, else 0.  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)(a != b);
	}
}

void grd_NORM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: NORM 1 1 Normalize (A) so max(A)-min(A) = 1.  */
{
	GMT_LONG node, n = 0, row, col;
	float a, z, zmin = FLT_MAX, zmax = -FLT_MAX;

	if (constant[last]) {
		GMT_report (GMT, GMT_MSG_NORMAL, "Warning, NORM of a constant gives NaN!\n");
		a = GMT->session.f_NaN;
	}
	else {
		GMT_grd_loop (GMT, info->G, row, col, node) {
			z = stack[last]->data[node];
			if (GMT_is_fnan (z)) continue;
			if (z < zmin) zmin = z;
			if (z > zmax) zmax = z;
			n++;
		}
		a = (n == 0 || zmax == zmin) ? GMT->session.f_NaN : 1.0F / (zmax - zmin);	/* Normalization scale */
	}
	GMT_grd_loop (GMT, info->G, row, col, node) stack[last]->data[node] = (constant[last]) ? a : a * stack[last]->data[node];
}

void grd_NOT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: NOT 1 1 NaN if A == NaN, 1 if A == 0, else 0.  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand == 0!\n");
	if (constant[last]) a = (fabs (factor[last]) > GMT_CONV_LIMIT) ? 0.0 : 1.0;
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : ((fabs (stack[last]->data[node]) > GMT_CONV_LIMIT) ? 0.0 : 1.0));
}

void grd_NRAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: NRAND 2 1 Normal, random values with mean A and std. deviation B.  */
{
	GMT_LONG node, prev;
	double a = 0.0, b = 0.0;

	prev = last - 1;
	if (constant[prev]) a = factor[prev];
	if (constant[last]) b = factor[last];
	for (node = 0; node < info->size; node++) {
		if (!constant[prev]) a = (double)stack[prev]->data[node];
		if (!constant[last]) b = (double)stack[last]->data[node];
		stack[prev]->data[node] = (float)(a + b * GMT_nrand (GMT));
	}
}

void grd_OR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: OR 2 1 NaN if B == NaN, else A.  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)((GMT_is_dnan (a) || GMT_is_dnan (b)) ? GMT->session.f_NaN : a);
	}
}

void grd_PDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: PDIST 1 1 Compute distance (in degrees if -fg) from points in ASCII file A.  */
{
	GMT_LONG row, col, node, dummy[2];
	struct GMT_TABLE *T = NULL;
	struct GMT_DATASET *D = NULL;

	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Spherical, in degrees */
		GMT_init_distaz (GMT, 'd', 1 + GMT_sph_mode (GMT), GMT_MAP_DIST);
#ifdef GMT_COMPAT
		GMT_report (GMT, GMT_MSG_COMPAT, "Warning: PDIST returns distances in spherical degrees; in GMT4 it returned km.  Use DEG2KM for conversion, if needed.\n");
#endif
	}
	else
		GMT_init_distaz (GMT, 'X', 0, GMT_MAP_DIST);	/* Cartesian */

	GMT_set_cols (GMT, GMT_IN,  2);
	if (GMT_Get_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&(info->ASCII_file), (void **)&D)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error in operator grd_PDIST reading file %s!\n", info->ASCII_file);
		info->error = GMT_DATA_READ_ERROR;
		return;
	}
	T = D->table[0];	/* Only one table in a single file */

	GMT_grd_padloop (GMT, info->G, row, col, node) stack[last]->data[node] = (float)GMT_mindist_to_point (GMT, (double)info->grd_x[col], (double)info->grd_y[row], T, dummy);

	/* Free memory used for points */

	GMT_Destroy_Data (GMT->parent, GMT_ALLOCATED, (void **)&D);
}

void grd_POP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: POP 1 0 Delete top element from the stack.  */
{

	/* Dummy routine that does nothing but consume the top element of stack */
}

void grd_PLM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: PLM 3 1 Associated Legendre polynomial P(A) degree B order C.  */
{
	GMT_LONG prev = last - 1, first = last - 2, L, M, node;
	double a = 0.0;
	/* last holds the order M , prev holds the degree L, first holds the argument x = cos(colat) */

	if (!(constant[prev] && constant[last])) {
		GMT_report (GMT, GMT_MSG_FATAL, "L and M must be constants in PLM (no calculations performed)\n");
		return;
	}

	L = irint (factor[prev]);
	M = irint (factor[last]);

	if (constant[first]) a = GMT_plm (GMT, L, M, factor[first]);
	for (node = 0; node < info->size; node++) stack[first]->data[node] = (float)((constant[first]) ? a : GMT_plm (GMT, L, M, stack[first]->data[node]));
}


void grd_PLMg (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: PLMg 3 1 Normalized associated Legendre polynomial P(A) degree B order C (geophysical convention).  */
{
	GMT_LONG prev = last - 1, first = last - 2, L, M, node;
	double a = 0.0;
	/* last holds the order M, prev holds the degree L, first holds the argument x = cos(colat) */

	if (!(constant[prev] && constant[last])) {
		GMT_report (GMT, GMT_MSG_FATAL, "L and M must be constants in PLMg (no calculations performed)\n");
		return;
	}

	L = irint (factor[prev]);
	M = irint (factor[last]);

	if (constant[first]) a = GMT_plm_bar (GMT, L, M, factor[first], FALSE);
	for (node = 0; node < info->size; node++) stack[first]->data[node] = (float)((constant[first]) ? a : GMT_plm_bar (GMT, L, M, stack[first]->data[node], FALSE));
}

void grd_POW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: POW 2 1 A ^ B.  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)pow (a, b);
	}
}

void grd_PQUANT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: PQUANT 2 1 The B'th Quantile (0-100%) of A.  */
{
	GMT_LONG node, prev, pad[4];
	float p;

	prev  = last - 1;	/* last holds the selected quantile (0-100), prev the data % */
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
		p = GMT->session.f_NaN;
	}
	else {
		GMT_memcpy (pad, stack[last]->header->pad, 4, GMT_LONG);	/* Save original pad */
		GMT_grd_pad_off (GMT, stack[last]);				/* Undo pad if one existed so we can sort */
		GMT_sort_array (GMT, (void *)stack[prev]->data, info->nm, GMT_FLOAT_TYPE);
		p = (float) GMT_quantile_f (GMT, stack[prev]->data, factor[last], (GMT_LONG)info->nm);
		GMT_memset (stack[last]->data, info->size, float);	/* Wipes everything */
		GMT_grd_pad_on (GMT, stack[last], pad);			/* Reinstate the original pad */
	}

	for (node = 0; node < info->size; node++) stack[prev]->data[node] = p;
}

void grd_PSI (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: PSI 1 1 Psi (or Digamma) of A.  */
{
	GMT_LONG node;
	double a = 0.0, x[2];

	x[1] = 0.0;	/* No imaginary part */
	if (constant[last]) {
		x[0] = factor[last];
		a = GMT_psi (GMT, x, (double *)NULL);
	}

	for (node = 0; node < info->size; node++) {
		if (!constant[last]) {
			x[0] = (double)stack[last]->data[node];
			a = GMT_psi (GMT, x, (double *)NULL);
		}
		stack[last]->data[node] = (float)a;
	}
}

void grd_PVQV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG kind)
{
	GMT_LONG prev = last - 1, first = last - 2, n, node, calc;
	double a = 0.0, x = 0.0, nu[2], pq[4];
	static char *name[2] = {"PV", "QV"};
	/* last holds the imaginary order vi, prev holds the real order vr, first holds the argument x = cos(colat) */

	calc = !(constant[prev] && constant[last] && constant[first]);	/* Only constant if all args are constant */
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
	for (node = 0; node < info->size; node++) {
		if (calc){
			if (!constant[prev]) nu[0] = (double)stack[prev]->data[node];
			if (!constant[last]) nu[1] = (double)stack[last]->data[node];
			if (!constant[first])    x = (double)stack[first]->data[node];
			GMT_PvQv (GMT, x, nu, pq, &n);
			a = pq[kind];
		}
		stack[first]->data[node] = (float)a;
	}
}

void grd_PV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: PV 3 1 Legendre function Pv(A) of degree v = real(B) + imag(C).  */
{
	grd_PVQV (GMT, info, stack, constant, factor, last, 0);
}

void grd_QV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: QV 3 1 Legendre function Qv(A) of degree v = real(B) + imag(C).  */
{
	grd_PVQV (GMT, info, stack, constant, factor, last, 1);
}

void grd_R2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: R2 2 1 R2 = A^2 + B^2.  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0!\n");
	if (constant[prev]) factor[prev] *= factor[prev];
	if (constant[last]) factor[last] *= factor[last];
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node] * stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node] * stack[last]->data[node];
		stack[prev]->data[node] = (float)(a + b);
	}
}

void grd_R2D (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: R2D 1 1 Convert Radians to Degrees.  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = R2D * factor[last];
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : R2D * stack[last]->data[node]);
}

void grd_RAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: RAND 2 1 Uniform random values between A and B.  */
{
	GMT_LONG node, prev;
	double a = 0.0, b = 0.0;

	prev = last - 1;
	if (constant[prev]) a = factor[prev];
	if (constant[last]) b = factor[last];
	for (node = 0; node < info->size; node++) {
		if (!constant[prev]) a = (double)stack[prev]->data[node];
		if (!constant[last]) b = (double)stack[last]->data[node];
		stack[prev]->data[node] = (float)(a + GMT_rand (GMT) * (b - a));
	}
}

void grd_RINT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: RINT 1 1 rint (A) (nearest integer).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = rint (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : rint ((double)stack[last]->data[node]));
}

void grd_ROTX (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ROTX 2 1 Rotate A by the (constant) shift B in x-direction.  */
{
	GMT_LONG row, col, prev = last - 1, shift, *new_col = NULL, node, nx;
	float *z = NULL;

	/* Shift grid A by the x-shift B.  B must be a constant */

	if (!constant[last]) {
		GMT_report (GMT, GMT_MSG_FATAL, "DX shift (B) must be a constant in ROTX (no calculations performed)\n");
		return;
	}
	shift = irint (factor[last] * info->G->header->r_inc[GMT_X]);	/* Shift of nodes */

	if (constant[prev] || !shift) return;	/* Trivial since A is a constant or shift is zero */
	if (shift < 0) shift += info->G->header->nx;	/* Same thing */
	nx = (size_t) info->G->header->nx;
	/* Set up permutation vector */

	new_col = GMT_memory (GMT, NULL, nx, GMT_LONG);
	z =  (float *) GMT_memory (GMT, NULL, nx, float);
	for (col = 0; col < info->G->header->nx; col++) new_col[col] = (col + shift) % info->G->header->nx;	/* Move by shift but rotate around */
	GMT_row_loop (GMT, info->G, row) {	/* For each row */
		GMT_col_loop (GMT, info->G, row, col, node) z[new_col[col]] = stack[prev]->data[node];	/* Copy one row of data to z with shift */
		node = GMT_IJP (info->G->header, row, 0);		/* First col */
		GMT_memcpy (&stack[prev]->data[node], z, nx, float);	/* Replace this row */
	}
	GMT_free (GMT, z);
	GMT_free (GMT, new_col);
}

void grd_ROTY (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ROTY 2 1 Rotate A by the (constant) shift B in y-direction.  */
{
	GMT_LONG row, col, prev = last - 1, shift, *new_row = NULL, nx;
	float *z = NULL;

	/* Shift grid A by the y-shift B.  B must be a constant */

	if (!constant[last]) {
		GMT_report (GMT, GMT_MSG_FATAL, "DY shift (B) must be a constant in ROTY (no calculations performed)\n");
		return;
	}
	shift = irint (factor[last] / info->G->header->inc[GMT_Y]);	/* Shift of nodes */

	if (constant[prev] || !shift) return;	/* Trivial since A is a constant or shift is zero */
	if (shift < 0) shift += info->G->header->ny;	/* Same thing */
	nx = (size_t) info->G->header->nx;
	/* Set up permutation vector */

	new_row = GMT_memory (GMT, NULL, info->G->header->ny, GMT_LONG);
	z =  (float *) GMT_memory (GMT, NULL, info->G->header->ny, float);
	for (row = 0; row < info->G->header->ny; row++) new_row[row] = (row + info->G->header->ny - shift) % info->G->header->ny;	/* Move by shift but rotate around */
	for (col = 0; col < info->G->header->nx; col++) {	/* For each column */
		for (row = 0; row < info->G->header->ny; row++) z[new_row[row]] = stack[prev]->data[GMT_IJP(info->G->header, row, col)];	/* Copy one column of data to z with shift */
		for (row = 0; row < info->G->header->ny; row++) stack[prev]->data[GMT_IJP(info->G->header, row, col)] = z[row];	/* Replace this column */
	}
	GMT_free (GMT, z);
	GMT_free (GMT, new_row);
}

void grd_SDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SDIST 2 1 Spherical (Great circle) distance (in degrees) between grid nodes and stack lon,lat (A, B).  */
{
	GMT_LONG row, col, node, prev = last - 1;
	double a, b;

	GMT_init_distaz (GMT, 'd', 1 + GMT_sph_mode (GMT), GMT_MAP_DIST);	/* Spherical, in degrees */
	GMT_grd_padloop (GMT, info->G, row, col, node) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float) GMT_distance (GMT, a, b, (double)info->grd_x[col], (double)info->grd_y[row]);
	}
}

void grd_AZ_sub (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG reverse)
{
	GMT_LONG row, col, node, prev = last - 1;
	double x0 = 0.0, y0 = 0.0, az;

	GMT_init_distaz (GMT, 'd', 1 + GMT_sph_mode (GMT), GMT_MAP_DIST);
	if (constant[prev]) x0 = factor[prev];
	if (constant[last]) y0 = factor[last];
	GMT_grd_padloop (GMT, info->G, row, col, node) {
		if (!constant[prev]) x0 = (double)stack[prev]->data[node];
		if (!constant[last]) y0 = (double)stack[last]->data[node];
		az = GMT_az_backaz (GMT, info->grd_x[col], info->grd_y[row], x0, y0, reverse);
		while (az < -180.0) az += 360.0;
		while (az > +180.0) az -= 360.0;
		stack[prev]->data[node] = (float)az;
	}
}

void grd_SAZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SAZ 2 1 Spherical azimuth from grid nodes to stack x,y.  */
/* Azimuth from grid ones to stack point */
{
	grd_AZ_sub (GMT, info, stack, constant, factor, last, FALSE);
}

void grd_SBAZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SBAZ 2 1 Spherical backazimuth from grid nodes to stack x,y.  */
/* Azimuth from stack point to grid ones (back azimuth) */
{
	grd_AZ_sub (GMT, info, stack, constant, factor, last, TRUE);
}

void grd_SEC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SEC 1 1 sec (A) (A in radians).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = (1.0 / cos (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : (1.0 / cos ((double)stack[last]->data[node])));
}

void grd_SECD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SECD 1 1 sec (A) (A in degrees).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = (1.0 / cosd (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : (1.0 / cosd ((double)stack[last]->data[node])));
}

void grd_SIGN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SIGN 1 1 sign (+1 or -1) of A.  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand == 0!\n");
	if (constant[last]) a = copysign (1.0, factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : copysign (1.0, (double)stack[last]->data[node]));
}

void grd_SIN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SIN 1 1 sin (A) (A in radians).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = sin (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : sin ((double)stack[last]->data[node]));
}

void grd_SINC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SINC 1 1 sinc (A) (sin (pi*A)/(pi*A)).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_sinc (GMT, factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_sinc (GMT, (double)stack[last]->data[node]));
}

void grd_SIND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SIND 1 1 sin (A) (A in degrees).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = sind (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : sind ((double)stack[last]->data[node]));
}

void grd_SINH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SINH 1 1 sinh (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = sinh (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : sinh ((double)stack[last]->data[node]));
}

void grd_SKEW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SKEW 1 1 Skewness of A.  */
{
	GMT_LONG row, col, node, n = 0;
	double mean = 0.0, sum2 = 0.0, skew = 0.0, delta;
	float f_skew;

	if (constant[last]) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->data[node] = GMT->session.f_NaN;
		return;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	GMT_grd_loop (GMT, info->G, row, col, node) {
		if (GMT_is_fnan (stack[last]->data[node])) continue;
		n++;
		delta = (double)stack[last]->data[node] - mean;
		mean += delta / n;
		sum2 += delta * ((double)stack[last]->data[node] - mean);
	}
	if (n > 1) {
		GMT_grd_loop (GMT, info->G, row, col, node) {
			if (GMT_is_fnan (stack[last]->data[node])) continue;
			delta = (double)stack[last]->data[node] - mean;
			skew += pow (delta, 3.0);
		}
		sum2 /= (n - 1);
		skew /= n * pow (sum2, 1.5);
		f_skew = (float)skew;
	}
	else
		f_skew = GMT->session.f_NaN;
	for (node = 0; node < info->size; node++) stack[last]->data[node] = f_skew;
}

void grd_SQR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SQRT 1 1 A^2.  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = factor[last] * factor[last];
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : stack[last]->data[node] * stack[last]->data[node]);
}

void grd_SQRT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SQRT 1 1 sqrt (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last] && factor[last] < 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one < 0!\n");
	if (constant[last]) a = sqrt (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : sqrt ((double)stack[last]->data[node]));
}

void grd_STD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: STD 1 1 Standard deviation of A.  */
{
	GMT_LONG row, col, node, n = 0;
	double mean = 0.0, sum2 = 0.0, delta;

	if (constant[last]) {	/* Trivial case */
		GMT_memset (stack[last], info->size, float);
		return;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	GMT_grd_loop (GMT, info->G, row, col, node) {
		if (GMT_is_fnan (stack[last]->data[node])) continue;
		n++;
		delta = (double)stack[last]->data[node] - mean;
		mean += delta / n;
		sum2 += delta * ((double)stack[last]->data[node] - mean);
	}
	sum2 = (n > 1) ? sqrt (sum2 / (n - 1)) : 0.0;
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)sum2;
}

void grd_STEP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: STEP 1 1 Heaviside step function: H(A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = factor[last];
	for (node = 0; node < info->size; node++) {
		if (!constant[last]) a = (double)stack[last]->data[node];
		if (a == 0.0)
			stack[last]->data[node] = (float)0.5;
		else
			stack[last]->data[node] = (float)((a < 0.0) ? 0.0 : 1.0);
	}
}

void grd_STEPX (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: STEPX 1 1 Heaviside step function in x: H(x-A).  */
{
	GMT_LONG row, col, node;
	double a;

	GMT_grd_padloop (GMT, info->G, row, col, node) {
		a = info->grd_x[col] - ((constant[last]) ? factor[last] : stack[last]->data[node]);
		if (a == 0.0)
			stack[last]->data[node] = (float)0.5;
		else
			stack[last]->data[node] = (float)((a < 0.0) ? 0.0 : 1.0);
	}
}

void grd_STEPY (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: STEPY 1 1 Heaviside step function in y: H(y-A).  */
{
	GMT_LONG row, col, node;
	double a;

	GMT_grd_padloop (GMT, info->G, row, col, node) {
		a = info->grd_y[row] - ((constant[last]) ? factor[last] : stack[last]->data[node]);
		if (a == 0.0)
			stack[last]->data[node] = (float)0.5;
		else
			stack[last]->data[node] = (float)((a < 0.0) ? 0.0 : 1.0);
	}
}

void grd_SUB (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: SUB 2 1 A - B.  */
{
	GMT_LONG node, prev;
	double a, b;

	prev = last - 1;
	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = (constant[last]) ? factor[last] : stack[last]->data[node];
		stack[prev]->data[node] = (float)(a - b);
	}
}

void grd_TAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: TAN 1 1 tan (A) (A in radians).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = tan (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : tan ((double)stack[last]->data[node]));
}

void grd_TAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: TAND 1 1 tan (A) (A in degrees).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = tand (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : tand ((double)stack[last]->data[node]));
}

void grd_TANH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: TANH 1 1 tanh (A).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = tanh (factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : tanh ((double)stack[last]->data[node]));
}

void grd_TN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: TN 2 1 Chebyshev polynomial Tn(-1<t<+1,n), with t = A, and n = B.  */
{
	GMT_LONG node, prev = last - 1, n;
	double a = 0.0, t;

	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		n = irint ((double)((constant[last]) ? factor[last] : stack[last]->data[node]));
		GMT_chebyshev (GMT, a, n, &t);
		stack[prev]->data[node] = (float)t;
	}
}

void grd_TCRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: TCRIT 2 1 Critical value for Student's t-distribution, with alpha = A and n = B.  */
{
	GMT_LONG node, b, prev;
	double a;

	prev = last - 1;
	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0 for TCRIT!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for TCRIT!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = irint ((double)((constant[last]) ? factor[last] : stack[last]->data[node]));
		stack[prev]->data[node] = (float)GMT_tcrit (GMT, a, (double)b);
	}
}

void grd_TDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: TDIST 2 1 Student's t-distribution A(t,n), with t = A, and n = B.  */
{
	GMT_LONG node, b, prev;
	double a, prob;

	prev = last - 1;
	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand one == 0 for TDIST!\n");
	if (constant[last] && factor[last] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, operand two == 0 for TDIST!\n");
	for (node = 0; node < info->size; node++) {
		a = (constant[prev]) ? factor[prev] : stack[prev]->data[node];
		b = irint ((double)((constant[last]) ? factor[last] : stack[last]->data[node]));
		(void) GMT_student_t_a (GMT, a, (GMT_LONG)b, &prob);
		stack[prev]->data[node] = (float)prob;
	}
}

void grd_UPPER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: UPPER 1 1 The highest (maximum) value of A.  */
{
	GMT_LONG row, col, node;
	float high = -FLT_MAX;

	if (constant[last]) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)factor[last];
		return;
	}

	GMT_grd_loop (GMT, info->G, row, col, node) {
		if (GMT_is_fnan (stack[last]->data[node])) continue;
		if (stack[last]->data[node] > high) high = stack[last]->data[node];
	}
	for (node = 0; node < info->size; node++) if (!GMT_is_fnan (stack[last]->data[node])) stack[last]->data[node] = high;
}

void grd_XOR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: XOR 2 1 B if A == NaN, else A.  */
{
	GMT_LONG node, prev;
	double a = 0.0, b = 0.0;

	prev = last - 1;
	if (constant[prev]) a = factor[prev];
	if (constant[last]) b = factor[last];
	for (node = 0; node < info->size; node++) {
		if (!constant[prev]) a = stack[prev]->data[node];
		if (!constant[last]) b = stack[last]->data[node];
		stack[prev]->data[node] = (float)((GMT_is_dnan (a)) ? b : a);
	}
}

void grd_Y0 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: Y0 1 1 Bessel function of A (2nd kind, order 0).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = y0 (fabs (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : y0 (fabs ((double)stack[last]->data[node])));
}

void grd_Y1 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: Y1 1 1 Bessel function of A (2nd kind, order 1).  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = y1 (fabs (factor[last]));
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : y1 (fabs ((double)stack[last]->data[node])));
}

void grd_YLM_sub (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last, GMT_LONG ortho)
{
	/* Returns geophysical normalization, unless M < 0, then orthonormalized form */
	GMT_LONG row, col, node, prev = last - 1, L, M;
	double x, z, P, C, S;

	if (!(constant[prev] && constant[last])) {
		GMT_report (GMT, GMT_MSG_FATAL, "L and M must be constants in YLM[g] (no calculations performed)\n");
		return;
	}

	L = irint (factor[prev]);
	M = irint (factor[last]);
	z = GMT_abs (M) * D2R;	/* abs() just in case routine is called with -M to add (-1)^M */

	GMT_row_padloop (GMT, info->G, row, node) {	/* For each latitude */

		x = sind (info->grd_y[row]);	/* Plm takes cos(colatitude) = sin(latitude) */
		P = GMT_plm_bar (GMT, L, M, x, ortho);
		if (M == 0) {
			GMT_col_padloop (GMT, info->G, col, node) {
				stack[prev]->data[node] = (float)P;
				stack[last]->data[node] = 0.0;
			}
		}
		else {
			GMT_col_padloop (GMT, info->G, col, node) {
				sincos (z * info->grd_x[col], &S, &C);
				stack[prev]->data[node] = (float)(P * C);
				stack[last]->data[node] = (float)(P * S);
			}
		}
	}
}

void grd_YLM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: YLM 2 2 Re and Im orthonormalized spherical harmonics degree A order B.  */
{
	grd_YLM_sub (GMT, info, stack, constant, factor, last, TRUE);
}

void grd_YLMg (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: YLMg 2 2 Cos and Sin normalized spherical harmonics degree A order B (geophysical convention).  */
{
	grd_YLM_sub (GMT, info, stack, constant, factor, last, FALSE);
}

void grd_YN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: YN 2 1 Bessel function of A (2nd kind, order B).  */
{
	GMT_LONG node, prev = last - 1, order = 0, simple = FALSE;
	double b = 0.0;

	if (constant[prev] && factor[prev] == 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, argument = 0 for YN!\n");
	if (constant[last]) {
		if (factor[last] < 0.0) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order < 0 for YN!\n");
		if ((rint(factor[last]) != factor[last])) GMT_report (GMT, GMT_MSG_NORMAL, "Warning, order not an integer for YN!\n");
		order = irint (fabs (factor[last]));
		if (constant[prev]) {
			b = yn ((int)order, fabs (factor[prev]));
			simple = TRUE;
		}
	}
	for (node = 0; node < info->size; node++) {
		if (simple)
			stack[prev]->data[node] = (float)b;
		else {
			if (!constant[last]) order = irint (fabs ((double)stack[last]->data[node]));
			stack[last]->data[node] = (float)yn ((int)order, fabs ((double)stack[prev]->data[node]));
		}
	}
}

void grd_ZCRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ZCRIT 1 1 Critical value for the normal-distribution, with alpha = A.  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_zcrit (GMT, factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_zcrit (GMT, a));
}

void grd_ZDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *stack[], GMT_LONG *constant, double *factor, GMT_LONG last)
/*OPERATOR: ZDIST 1 1 Cumulative normal-distribution C(x), with x = A.  */
{
	GMT_LONG node;
	double a = 0.0;

	if (constant[last]) a = GMT_zdist (GMT, factor[last]);
	for (node = 0; node < info->size; node++) stack[last]->data[node] = (float)((constant[last]) ? a : GMT_zdist (GMT, a));
}

/* ---------------------- end operator functions --------------------- */

#include "grdmath_func.h"

#define Return(code) {GMT_Destroy_Options (API, &list); Free_grdmath_Ctrl (GMT, Ctrl); grdmath_free (GMT, stack, alloc_mode, &info, localhashnode); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG decode_grd_argument (struct GMT_CTRL *GMT, struct GMT_OPTION *opt, double *value, struct GMT_HASH *H)
{
	GMT_LONG i, expect, check = GMT_IS_NAN, possible_number = FALSE;
	double tmp = 0.0;

	if (opt->option == GMTAPI_OPT_OUTFILE) return GRDMATH_ARG_IS_SAVE;	/* Time to save stack; arg is filename */

	/* Check if argument is operator */

	if ((i = GMT_hash_lookup (GMT, opt->arg, H, GRDMATH_N_OPERATORS, GRDMATH_N_OPERATORS)) >= GRDMATH_ARG_IS_OPERATOR) return (i);

	/* Next look for symbols with special meaning */

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

	/* Preliminary test-conversion to a number */

	if (!GMT_not_numeric (GMT, opt->arg)) {	/* Only check if we are not sure this is NOT a number */
		expect = (strchr (opt->arg, 'T')) ? GMT_IS_ABSTIME : GMT_IS_UNKNOWN;	/* Watch out for dateTclock-strings */
		check = GMT_scanf (GMT, opt->arg, expect, &tmp);
		possible_number = TRUE;
	}

	/* Determine if argument is file. But first strip off suffix */

	if (!GMT_access (GMT, opt->arg, F_OK)) {	/* Yes it is */
		if (check != GMT_IS_NAN && possible_number) GMT_report (GMT, GMT_MSG_FATAL, "Warning: Your argument %s is both a file and a number.  File is selected\n", opt->arg);
		return GRDMATH_ARG_IS_FILE;
	}

	if (check != GMT_IS_NAN) {	/* OK it is a number */
		*value = tmp;
		return GRDMATH_ARG_IS_NUMBER;
	}

	if (opt->arg[0] == '-') {	/* Probably a bad commandline option */
		GMT_report (GMT, GMT_MSG_FATAL, "Error: Option %s not recognized\n", opt->arg);
		return GRDMATH_ARG_IS_BAD;
	}

	GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: %s is not a number, operator or file name\n", opt->arg);
	return GRDMATH_ARG_IS_BAD;
}

void grdmath_free (struct GMT_CTRL *GMT, struct GMT_GRID *stack[], GMT_LONG alloc_mode[], struct GRDMATH_INFO *info, struct GMT_HASH lnode[]) {
	/* Free allocated memory before quitting */
	GMT_LONG k;
	struct GMT_HASH *p = NULL, *current = NULL;
	
	for (k = 0; k < GRDMATH_STACK_SIZE; k++) {
		if (alloc_mode[k] == 1) GMT_free_grid (GMT, &stack[k], TRUE);
	}
	GMT_free_grid (GMT, &info->G, TRUE);
	GMT_free (GMT, info->grd_x);
	GMT_free (GMT, info->grd_y);
	GMT_free (GMT, info->grd_xn);
	GMT_free (GMT, info->grd_yn);
	GMT_free (GMT, info->dx);
	if (info->ASCII_file) free ((void *)info->ASCII_file);
	for (k = 0; k < GRDMATH_N_OPERATORS; k++) {
		p = lnode[k].next;
		while ((current = p)) { p = p->next; GMT_free (GMT, current); }
	}
}

GMT_LONG GMT_grdmath (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG row, col, k, kk, node, op = 0, nstack = 0, new_stack = -1, n_items = 0, this_stack;
	GMT_LONG consumed_operands[GRDMATH_N_OPERATORS], produced_operands[GRDMATH_N_OPERATORS];
	GMT_LONG alloc_mode[GRDMATH_STACK_SIZE], status, subset, n_macros;
	GMT_LONG constant[GRDMATH_STACK_SIZE], error = FALSE;

	struct GMT_GRID *stack[GRDMATH_STACK_SIZE], *G_in = NULL;

	double factor[GRDMATH_STACK_SIZE], value, x_noise, y_noise, off, scale;
	double wesn[4], special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_NY+1];

#include "grdmath_op.h"

	PFV call_operator[GRDMATH_N_OPERATORS];

	struct GMT_HASH localhashnode[GRDMATH_N_OPERATORS];
	struct GRDMATH_INFO info;
	struct MATH_MACRO *M = NULL;
	struct GRDMATH_CTRL *Ctrl = NULL;
	struct GMT_OPTION *opt = NULL, *list = NULL, *ptr = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdmath_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdmath_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdmath", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRbf:", "ghinrs" GMT_OPT("F"), options))) Return (error);
	Ctrl = (struct GRDMATH_CTRL *) New_grdmath_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdmath_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the grdmath main code ----------------------------*/

	GMT_memset (localhashnode, GRDMATH_N_OPERATORS, struct GMT_HASH);
	GMT_memset (&info, 1, struct GRDMATH_INFO);
	GMT_memset (alloc_mode, GRDMATH_STACK_SIZE, GMT_LONG);
	GMT_memset (stack, GRDMATH_STACK_SIZE, struct GMT_GRID *);

	n_macros = gmt_load_macros (GMT, ".grdmath", &M);	/* Load in any macros */
	if (n_macros) GMT_report (GMT, GMT_MSG_NORMAL, "Found and loaded %ld user macros.\n", n_macros);
	
	/* Internally replace the = [file] sequence with an output option */

	for (opt = options; opt; opt = opt->next) {
		if (opt->option == GMTAPI_OPT_INFILE && !strcmp (opt->arg, "=")) {	/* Found the output sequence */
			if (opt->next) {
				GMT_Make_Option (API, GMTAPI_OPT_OUTFILE, opt->next->arg, &ptr);
				opt = opt->next;	/* Now we must skip that option */
			}
			else {	/* Standard output */
				GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: No output file specified via = file mechanism\n");
				Return (EXIT_FAILURE);
			}
		}
		else if (opt->option == GMTAPI_OPT_INFILE && (k = gmt_find_macro (opt->arg, n_macros, M)) != GMTAPI_NOTSET) {
			/* Add in the replacement commands from the macro */
			for (kk = 0; kk < M[k].n_arg; kk++) {
				GMT_Make_Option (API, GMTAPI_OPT_INFILE, M[k].arg[kk], &ptr);
			 	if (list)
					GMT_Append_Option (API, ptr, &list);
				else
					list = ptr;
			}
			continue;
		}
		else
		 	GMT_Make_Option (API, opt->option, opt->arg, &ptr);

		if (list)
			GMT_Append_Option (API, ptr, &list);
		else
			list = ptr;
	}
	gmt_free_macros (GMT, n_macros, &M);

	GMT_hash_init (GMT, localhashnode, operator, GRDMATH_N_OPERATORS, GRDMATH_N_OPERATORS);

	GMT_memset (wesn, 4, double);
	GMT_memset (constant, GRDMATH_STACK_SIZE, GMT_LONG);
	GMT_memset (factor, GRDMATH_STACK_SIZE, double);

	/* Read the first file we encounter so we may allocate space */

	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_IN,  GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */

	for (opt = list; !G_in && opt; opt = opt->next) {	/* Look for a grid file, if given */
		if (!(opt->option == GMTAPI_OPT_INFILE || opt->option == GMTAPI_OPT_NUMBER))	continue;	/* Skip command line options and output file */
		if (opt->next && !(strncmp (opt->next->arg, "LDIST", (size_t)5) && strncmp (opt->next->arg, "PDIST", (size_t)5) && strncmp (opt->next->arg, "INSIDE", (size_t)6))) continue;	/* Not grids */
		/* Filenames,  operators, some numbers and = will all have been flagged as files by the parser */
		status = decode_grd_argument (GMT, opt, &value, localhashnode);		/* Determine what this is */
		if (status == GRDMATH_ARG_IS_BAD) Return (EXIT_FAILURE);		/* Horrible */
		if (status != GRDMATH_ARG_IS_FILE) continue;				/* Skip operators and numbers */
		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(opt->arg), (void **)&G_in)) Return (GMT_GRID_READ_ERROR);	/* Get header only */
	}

	info.G = GMT_create_grid (GMT);
	GMT_grd_init (GMT, info.G->header, options, TRUE);
	subset = (GMT->common.R.active && Ctrl->I.active);

	if (G_in) {	/* We read a gridfile header above, now update columns */
		if (GMT->common.R.active && Ctrl->I.active) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Cannot use -R, -I when grid files are specified\n");
			Return (EXIT_FAILURE);
		}
		else if  (GMT->common.r.active) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Cannot use -r when grid files are specified\n");
			Return (EXIT_FAILURE);
		}
		if (subset) {	/* Gave -R and files: Read the subset to set the header properly */
			GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(opt->arg), (void **)&G_in)) Return (GMT_DATA_READ_ERROR);	/* Get subset only */
		}
		GMT_memcpy (info.G->header, G_in->header, 1, struct GRD_HEADER);
		GMT_set_grddim (GMT, info.G->header);			/* To adjust for the pad */
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&G_in);
	}
	else if (GMT->common.R.active && Ctrl->I.active) {	/* Must create from -R -I [-r] */
		/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
		GMT_err_fail (GMT, GMT_init_newgrid (GMT, info.G, GMT->common.R.wesn, Ctrl->I.inc, GMT->common.r.active), Ctrl->Out.file);
	}
	else {
		GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Expression must contain at least one grid file or -R, -I\n");
		Return (EXIT_FAILURE);
	}
	info.nm = info.G->header->nm;	info.size = info.G->header->size;

	/* Get x and y vectors (these extend onto the pad) */

	info.grd_x = GMT_memory (GMT, NULL, info.G->header->mx, float);
	info.grd_y = GMT_memory (GMT, NULL, info.G->header->my, float);
	info.grd_xn = GMT_memory (GMT, NULL, info.G->header->mx, float);
	info.grd_yn = GMT_memory (GMT, NULL, info.G->header->my, float);
	for (k = 0, col = -info.G->header->pad[XLO]; k < info.G->header->mx; col++, k++) info.grd_x[k] = (float)GMT_grd_col_to_x (GMT, col, info.G->header);
	for (k = 0, row = -info.G->header->pad[YHI]; k < info.G->header->my; row++, k++) info.grd_y[k] = (float)GMT_grd_row_to_y (GMT, row, info.G->header);
	if (GMT_is_geographic (GMT, GMT_IN)) {	/* Make sure latitudes remain in range; if not apply geographic BC */
		for (k = 0; k < info.G->header->pad[YHI]; k++) 
			if (info.grd_y[k] > 90.0) info.grd_y[k] = (float)(2.0 * 90.0 - info.grd_y[k]);
		for (k = 0, kk = info.G->header->my - info.G->header->pad[YLO]; k < info.G->header->pad[YLO]; k++, k++) 
			if (info.grd_y[kk] < -90.0) info.grd_y[kk] = (float)(-2.0 * 90.0 - info.grd_y[kk]);
	}
	off = 0.5 * (info.G->header->wesn[XHI] + info.G->header->wesn[XLO]);
	scale = 2.0 / (info.G->header->wesn[XHI] - info.G->header->wesn[XLO]);
	for (k = 0; k < info.G->header->mx; k++) info.grd_xn[k] = (float)((info.grd_x[k] - off) * scale);
	off = 0.5 * (info.G->header->wesn[YHI] + info.G->header->wesn[YLO]);
	scale = 2.0 / (info.G->header->wesn[YHI] - info.G->header->wesn[YLO]);
	for (k = 0; k < info.G->header->my; k++) info.grd_yn[k] = (float)((info.grd_y[k] - off) * scale);
	x_noise = GMT_SMALL * info.G->header->inc[GMT_X];	y_noise = GMT_SMALL * info.G->header->inc[GMT_Y];
	info.dx = GMT_memory (GMT, NULL, info.G->header->my, float);

	if (Ctrl->M.active) {	/* Use flat earth distances for gradients */
		for (k = 0; k < info.G->header->my; k++) info.dx[k] = (float) (GMT->current.proj.DIST_M_PR_DEG * info.G->header->inc[GMT_X] * cosd (info.grd_y[k]));
		info.dy = (float)(GMT->current.proj.DIST_M_PR_DEG * info.G->header->inc[GMT_Y]);
		info.convert = TRUE;
	}
	else {	/* Constant increments in user units */
		for (k = 0; k < info.G->header->my; k++) info.dx[k] = (float)info.G->header->inc[GMT_X];
		info.dy = (float)info.G->header->inc[GMT_Y];
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
	GMT_report (GMT, GMT_MSG_NORMAL, " ");

	nstack = 0;

	for (opt = list, error = FALSE; !error && opt; opt = opt->next) {

		/* First check if we should skip optional arguments */

		if (strchr ("IMNRVbfnr", opt->option)) continue;
		/* if (opt->option == GMTAPI_OPT_OUTFILE) continue; */	/* We do output after the loop */

		op = decode_grd_argument (GMT, opt, &value, localhashnode);

		if (op == GRDMATH_ARG_IS_SAVE) {	/* Time to save the current stack to output and pop the stack */
			if (nstack <= 0) {
				GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: No items on stack available for output!\n");
				Return (EXIT_FAILURE);
			}

			if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "= %s", opt->arg);

			if (n_items && new_stack < 0 && constant[nstack-1]) {	/* Only a constant provided, set grid accordingly */
				if (!stack[nstack-1]) alloc_stack (GMT, &stack[nstack-1], info.G);
				alloc_mode[nstack-1] = 1;
				GMT_grd_loop (GMT, info.G, row, col, node) stack[nstack-1]->data[node] = (float)factor[nstack-1];
			}
			this_stack = nstack - 1;
			GMT_Put_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, 0, (void **)&opt->arg, (void *)stack[this_stack]);
			alloc_mode[this_stack] = 2;	/* Since it now is registered */
			if (n_items) nstack--;	/* Pop off the current stack if there is one */
			new_stack = nstack;
			continue;
		}
		
		if (op != GRDMATH_ARG_IS_FILE && !GMT_access (GMT, opt->arg, R_OK)) GMT_message (GMT, "Warning: The number or operator %s may be confused with an existing file %s!\n", opt->arg, opt->arg);

		if (op < GRDMATH_ARG_IS_OPERATOR) {	/* File name or factor */

			if (op == GRDMATH_ARG_IS_FILE && !(strncmp (opt->next->arg, "LDIST", (size_t)5) && strncmp (opt->next->arg, "PDIST", (size_t)5) && strncmp (opt->next->arg, "INSIDE", (size_t)6))) op = GRDMATH_ARG_IS_ASCIIFILE;

			if (nstack == GRDMATH_STACK_SIZE) {	/* Stack overflow */
				error = TRUE;
				continue;
			}
			n_items++;
			if (op == GRDMATH_ARG_IS_NUMBER) {
				constant[nstack] = TRUE;
				factor[nstack] = value;
				error = FALSE;
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "%g ", factor[nstack]);
				nstack++;
				continue;
			}
			else if (op <= GRDMATH_ARG_IS_PI && op >= GRDMATH_ARG_IS_NY) {
				constant[nstack] = TRUE;
				factor[nstack] = special_symbol[GRDMATH_ARG_IS_PI-op];
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "%g ", factor[nstack]);
				nstack++;
				continue;
			}

			/* Here we need a matrix */

			constant[nstack] = FALSE;

			if (op == GRDMATH_ARG_IS_X_MATRIX) {		/* Need to set up matrix of x-values */
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "X ");
				if (!stack[nstack]) alloc_stack (GMT, &stack[nstack], info.G);
				alloc_mode[nstack] = 1;
				GMT_row_padloop (GMT, info.G, row, node) {
					node = row * info.G->header->mx;
					GMT_memcpy (&stack[nstack]->data[node], info.grd_x, info.G->header->mx, float);
				}
			}
			else if (op == GRDMATH_ARG_IS_x_MATRIX) {		/* Need to set up matrix of normalized x-values */
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "Xn ");
				if (!stack[nstack]) alloc_stack (GMT, &stack[nstack], info.G);
				alloc_mode[nstack] = 1;
				GMT_row_padloop (GMT, info.G, row, node) {
					node = row * info.G->header->mx;
					GMT_memcpy (&stack[nstack]->data[node], info.grd_xn, info.G->header->mx, float);
				}
			}
			else if (op == GRDMATH_ARG_IS_Y_MATRIX) {	/* Need to set up matrix of y-values */
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "Y ");
				if (!stack[nstack]) alloc_stack (GMT, &stack[nstack], info.G);
				alloc_mode[nstack] = 1;
				GMT_grd_padloop (GMT, info.G, row, col, node) stack[nstack]->data[node] = info.grd_y[row];
			}
			else if (op == GRDMATH_ARG_IS_y_MATRIX) {	/* Need to set up matrix of normalized y-values */
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "Yn ");
				if (!stack[nstack]) alloc_stack (GMT, &stack[nstack], info.G);
				alloc_mode[nstack] = 1;
				GMT_grd_padloop (GMT, info.G, row, col, node) stack[nstack]->data[node] = info.grd_yn[row];
			}
			else if (op == GRDMATH_ARG_IS_ASCIIFILE) {
				if (info.ASCII_file) free ((void *)info.ASCII_file);
				if (!stack[nstack]) alloc_stack (GMT, &stack[nstack], info.G);
				alloc_mode[nstack] = 1;
				info.ASCII_file = strdup (opt->arg);
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "(%s) ", opt->arg);
			}
			else if (op == GRDMATH_ARG_IS_FILE) {		/* Filename given */
				if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "%s ", opt->arg);
				if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_HEADER, (void **)&(opt->arg), (void **)&stack[nstack])) Return (GMT_DATA_READ_ERROR);	/* Get header only */
				if (!subset && (stack[nstack]->header->nx != info.G->header->nx || stack[nstack]->header->ny != info.G->header->ny)) {
					GMT_report (GMT, GMT_MSG_FATAL, "grid files not of same size!\n");
					Return (EXIT_FAILURE);
				}
				else if (!Ctrl->N.active && (!subset && (fabs (stack[nstack]->header->wesn[XLO] - info.G->header->wesn[XLO]) > x_noise || fabs (stack[nstack]->header->wesn[XHI] - info.G->header->wesn[XHI]) > x_noise ||
					fabs (stack[nstack]->header->wesn[YLO] - info.G->header->wesn[YLO]) > y_noise || fabs (stack[nstack]->header->wesn[YHI] - info.G->header->wesn[YHI]) > y_noise))) {
					GMT_report (GMT, GMT_MSG_FATAL, "grid files do not cover the same area!\n");
					Return (EXIT_FAILURE);
				}
				if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(opt->arg), (void **)&stack[nstack])) Return (GMT_DATA_READ_ERROR);	/* Get header only */
				alloc_mode[nstack] = 2;
			}
			nstack++;
			continue;
		}

		/* Here we have an operator */

		if ((new_stack = nstack - consumed_operands[op] + produced_operands[op]) >= GRDMATH_STACK_SIZE) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Stack overflow (%s)\n", opt->arg);
			Return (EXIT_FAILURE);
		}

		if (nstack < consumed_operands[op]) {
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error: Operation \"%s\" requires %ld operands\n", operator[op], consumed_operands[op]);
			Return (EXIT_FAILURE);
		}

		n_items++;
		if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "%s ", operator[op]);

		for (k = produced_operands[op] - consumed_operands[op]; k > 0; k--) {
			if (stack[nstack+k-1])	continue;

			/* Must make space for more */

			alloc_stack (GMT, &stack[nstack+k-1], info.G);
			alloc_mode[nstack+k-1] = 1;
		}

		/* If operators operates on constants only we may have to make space as well */

		for (kk = 0, k = nstack - consumed_operands[op]; kk < produced_operands[op]; kk++, k++) {
			if (constant[k] && !stack[k]) {
				alloc_stack (GMT, &stack[k], info.G);
				alloc_mode[k] = 1;
			}
		}

		(*call_operator[op]) (GMT, &info, stack, constant, factor, nstack - 1);	/* Do it */

		if (info.error) Return (info.error);	/* Got an error inside the operator */

		nstack = new_stack;
		for (k = 1; k <= produced_operands[op]; k++) constant[nstack-k] = FALSE;	/* Now filled with grid */
	}
	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) GMT_message (GMT, "\n");

	if ((error = GMT_End_IO (API, GMT_IN,  0))) Return (error);	/* Disables further data input */
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data input */

	if (nstack > 0) GMT_report (GMT, GMT_MSG_FATAL, "Warning: %ld more operands left on the stack!\n", nstack);

	Return (EXIT_SUCCESS);
}
