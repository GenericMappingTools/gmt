/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2025 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * Brief synopsis: grdmath is a reverse polish calculator that operates on grid files
 * (and constants) and perform basic mathematical operations
 * on them like add, multiply, etc.
 * Some operators only work on one operand (e.g., log, exp)
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Note on KEYS: =G} is special and says the = option will write an output grid (repeatable).
 */

/* Notes: To add a new operator:
 * 1) Just add one more entry at the end of the array of operator names at top of GMT_grdmath function.
 * 2) Add one more entry at the end with the specifics in init_operators in grdmath_init function.
 * 3) Code up the operator function grd_XXXXX ()
 * 4) Add message to the usage function
 * 5) Update value of #define GRDMATH_N_OPERATORS
 * 6) Add to the rst documentation
 */

#include "gmt_dev.h"
#include "longopt/grdmath_inc.h"

#define THIS_MODULE_CLASSIC_NAME	"grdmath"
#define THIS_MODULE_MODERN_NAME	"grdmath"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Reverse Polish Notation (RPN) calculator for grids (element by element)"
#define THIS_MODULE_KEYS	"<G(,=G}"
#define THIS_MODULE_NEEDS	"r"
#define THIS_MODULE_OPTIONS "-:RVabdfghinr" GMT_OPT("F") GMT_ADD_x_OPT

/* Some local macros to simplify coding */
/*! Loop over all nodes including the pad */
#define grdmath_row_padloop(C,G,row,ij) for (row = 0, ij = 0; row < (openmp_int)G->header->my; row++)
#define grdmath_col_padloop(C,G,col,ij) for (col = 0; col < (openmp_int)G->header->mx; col++, ij++)
#define grdmath_grd_padloop(C,G,row,col,ij) grdmath_row_padloop(C,G,row,ij) grdmath_col_padloop(C,G,col,ij)
/*! Just a loop over columns */
#define grdmath_col_padloop2(C,G,col) for (col = 0; col < (openmp_int)G->header->mx; col++)

#define GRDMATH_ARG_IS_OPERATOR		 0
#define GRDMATH_ARG_IS_FILE		-1
#define GRDMATH_ARG_IS_NUMBER		-2
#define GRDMATH_ARG_IS_PI		-3
#define GRDMATH_ARG_IS_E		-4
#define GRDMATH_ARG_IS_F_EPS		-5
#define GRDMATH_ARG_IS_EULER		-6
#define GRDMATH_ARG_IS_PHI		-7
#define GRDMATH_ARG_IS_XMIN		-8
#define GRDMATH_ARG_IS_XMAX		-9
#define GRDMATH_ARG_IS_XRANGE		-10
#define GRDMATH_ARG_IS_XINC		-11
#define GRDMATH_ARG_IS_NX		-12
#define GRDMATH_ARG_IS_YMIN		-13
#define GRDMATH_ARG_IS_YMAX		-14
#define GRDMATH_ARG_IS_YRANGE		-15
#define GRDMATH_ARG_IS_YINC		-16
#define GRDMATH_ARG_IS_NY		-17
#define GRDMATH_ARG_IS_X_MATRIX		-18
#define GRDMATH_ARG_IS_x_MATRIX		-19
#define GRDMATH_ARG_IS_Y_MATRIX		-20
#define GRDMATH_ARG_IS_y_MATRIX		-21
#define GRDMATH_ARG_IS_XCOL_MATRIX	-22
#define GRDMATH_ARG_IS_YROW_MATRIX	-23
#define GRDMATH_ARG_IS_NODE_MATRIX	-24
#define GRDMATH_ARG_IS_NODEP_MATRIX	-25
#define GRDMATH_ARG_IS_ASCIIFILE	-26
#define GRDMATH_ARG_IS_SAVE		-27
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

#define GMT_OPT_OUTFILE2	'='	/* Unlike GMT_OPT_OUTFILE this one has no restriction of just one output file */

#ifdef DOUBLE_PRECISION_GRID
#define fabsf(x) fabs(x)
#endif

struct GRDMATH_CTRL {	/* All control options for this program (except common args) */
	/* active is true if the option has been activated */
	struct GRDMATH_Out {	/* = <filename> */
		bool active;
	} Out;
	struct GRDMATH_A {	/* -A<min_area>[/<min_level>/<max_level>][+ag|i|s][+r|l][+p<percent>] */
		bool active;
		struct GMT_SHORE_SELECT info;
	} A;
	struct GRDMATH_C {	/* -C[<cpt>] */
		bool active;
		char *cpt;
	} C;
	struct GRDMATH_D {	/* -D<resolution>[+f] */
		bool active;
		bool force;	/* if true, select next highest level if current set is not available */
		char set;	/* One of f, h, i, l, c, or auto */
	} D;
	struct GRDMATH_I {	/* -I (for checking only) */
		bool active;
	} I;
	struct GRDMATH_M {	/* -M */
		bool active;
	} M;
	struct GRDMATH_N {	/* -N */
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
	gmt_grdfloat *f_grd_x,  *f_grd_y;
	gmt_grdfloat *f_grd_xn, *f_grd_yn;
	double *dx, dy;		/* In flat-Earth m if -M is set */
	struct GMT_GRID *G;
	struct GMT_SHORE_SELECT *A;	/* If -A is processed */
};

struct GRDMATH_STACK {
	struct GMT_GRID *G;		/* The grid */
	bool constant;			/* true if a constant (see factor) and S == NULL */
	double factor;			/* The value if constant is true */
};

struct GRDMATH_STORE {
	char *label;	/* Name of this stored memory */
	struct GRDMATH_STACK stored;
};

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDMATH_CTRL *C = gmt_M_memory (GMT, NULL, 1, struct GRDMATH_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.info.high = GSHHS_MAX_LEVEL;	/* Include all GSHHS levels (if LDISTG is used) */
	C->D.set = 'l';				/* Low-resolution coastline data */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDMATH_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s [%s] [%s] [-D<resolution>[+f]] [%s] [-M] [-N] [-S] [%s] [%s] [%s] [%s] [%s] [%s] [%s] "
		"[%s] [%s] [%s] [%s]%s[%s] A B op C op D op ... = %s\n", name, GMT_Rgeo_OPT, GMT_A_OPT, GMT_I_OPT, GMT_V_OPT, GMT_a_OPT, GMT_bi_OPT, GMT_di_OPT,
		GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_n_OPT, GMT_r_OPT, GMT_x_OPT, GMT_PAR_OPT, GMT_OUTGRID);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");
	gmt_outgrid_syntax (API, '=', "Writes the current top of the stack to the named file and pops it off the stack. "
		"Can be used more than once");
	GMT_Usage (API, 1, "\n<operands>");
	GMT_Usage (API, -2, "A, B, etc. are grid files, constants, or symbols (see below). "
		"The stack can hold up to %d entries (given enough memory).", GRDMATH_STACK_SIZE);
	GMT_Usage (API, 1, "\n<operators>");
	GMT_Usage (API, -2, "Trigonometric operators expect radians unless noted otherwise. "
		"The operators and number of input and output arguments are:\n");
	/* Do these verbatim with no wrapping via GMT_Usage since cannot align well otherwise */
	GMT_Message (API, GMT_TIME_NONE,
		"     Name     #args  Returns\n"
		"     -----------------------\n");
	GMT_Message (API, GMT_TIME_NONE, "     ABS        1 1  ");	GMT_Usage (API, -21, "abs (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ACOS       1 1  ");	GMT_Usage (API, -21, "acos (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ACOSD      1 1  ");	GMT_Usage (API, -21, "acosd (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ACOSH      1 1  ");	GMT_Usage (API, -21, "acosh (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ACOT       1 1  ");	GMT_Usage (API, -21, "acot (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ACOTD      1 1  ");	GMT_Usage (API, -21, "acotd (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ACOTH      1 1  ");	GMT_Usage (API, -21, "acoth (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ACSC       1 1  ");	GMT_Usage (API, -21, "acsc (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ACSCD      1 1  ");	GMT_Usage (API, -21, "acscd (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ACSCH      1 1  ");	GMT_Usage (API, -21, "acsch (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ADD        2 1  ");	GMT_Usage (API, -21, "A + B");
	GMT_Message (API, GMT_TIME_NONE, "     AND        2 1  ");	GMT_Usage (API, -21, "B if A == NaN, else A");
	GMT_Message (API, GMT_TIME_NONE, "     ARC        2 1  ");	GMT_Usage (API, -21, "arc(A, B) = pi - |pi - |a-b|| for A, B in radians");
	GMT_Message (API, GMT_TIME_NONE, "     AREA       0 1  ");	GMT_Usage (API, -21, "Area of each gridnode cell (spherical calculation in km^2 if geographic)");
	GMT_Message (API, GMT_TIME_NONE, "     ASEC       1 1  ");	GMT_Usage (API, -21, "asec (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ASECD      1 1  ");	GMT_Usage (API, -21, "asecd (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ASECH      1 1  ");	GMT_Usage (API, -21, "asech (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ASIN       1 1  ");	GMT_Usage (API, -21, "asin (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ASIND      1 1  ");	GMT_Usage (API, -21, "asind (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ASINH      1 1  ");	GMT_Usage (API, -21, "asinh (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ATAN       1 1  ");	GMT_Usage (API, -21, "atan (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ATAND      1 1  ");	GMT_Usage (API, -21, "atand (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ATAN2      2 1  ");	GMT_Usage (API, -21, "atan2 (A, B)");
	GMT_Message (API, GMT_TIME_NONE, "     ATAN2D     2 1  ");	GMT_Usage (API, -21, "atan2d (A, B)");
	GMT_Message (API, GMT_TIME_NONE, "     ATANH      1 1  ");	GMT_Usage (API, -21, "atanh (A)");
	GMT_Message (API, GMT_TIME_NONE, "     BCDF       3 1  ");	GMT_Usage (API, -21, "Binomial cumulative distribution function for p = A, n = B and x = C");
	GMT_Message (API, GMT_TIME_NONE, "     BPDF       3 1  ");	GMT_Usage (API, -21, "Binomial probability density function for p = A, n = B and x = C");
	GMT_Message (API, GMT_TIME_NONE, "     BEI        1 1  ");	GMT_Usage (API, -21, "Kelvin function bei (A)");
	GMT_Message (API, GMT_TIME_NONE, "     BER        1 1  ");	GMT_Usage (API, -21, "Kelvin function ber (A)");
	GMT_Message (API, GMT_TIME_NONE, "     BITAND     2 1  ");	GMT_Usage (API, -21, "A & B (bitwise AND operator)");
	GMT_Message (API, GMT_TIME_NONE, "     BITLEFT    2 1  ");	GMT_Usage (API, -21, "A << B (bitwise left-shift operator)");
	GMT_Message (API, GMT_TIME_NONE, "     BITNOT     1 1  ");	GMT_Usage (API, -21, "~A (bitwise NOT operator, i.e., return two's complement)");
	GMT_Message (API, GMT_TIME_NONE, "     BITOR      2 1  ");	GMT_Usage (API, -21, "A | B (bitwise OR operator)");
	GMT_Message (API, GMT_TIME_NONE, "     BITRIGHT   2 1  ");	GMT_Usage (API, -21, "A >> B (bitwise right-shift operator)");
	GMT_Message (API, GMT_TIME_NONE, "     BITTEST    2 1  ");	GMT_Usage (API, -21, "1 if bit B of A is set, else 0 (bitwise TEST operator)");
	GMT_Message (API, GMT_TIME_NONE, "     BITXOR     2 1  ");	GMT_Usage (API, -21, "A ^ B (bitwise XOR operator)");
	GMT_Message (API, GMT_TIME_NONE, "     BLEND      3 1  ");	GMT_Usage (API, -21, "Blend A and B using weights in C (0-1 range) as A*C + B*(1-C)");
	GMT_Message (API, GMT_TIME_NONE, "     CAZ        2 1  ");	GMT_Usage (API, -21, "Cartesian azimuth from grid nodes to stack x,y");
	GMT_Message (API, GMT_TIME_NONE, "     CBAZ       2 1  ");	GMT_Usage (API, -21, "Cartesian back-azimuth from grid nodes to stack x,y");
	GMT_Message (API, GMT_TIME_NONE, "     CDIST      2 1  ");	GMT_Usage (API, -21, "Cartesian distance between grid nodes and stack x,y");
	GMT_Message (API, GMT_TIME_NONE, "     CDIST2     2 1  ");	GMT_Usage (API, -21, "As CDIST but only to nodes that are != 0");
	GMT_Message (API, GMT_TIME_NONE, "     CEIL       1 1  ");	GMT_Usage (API, -21, "ceil (A) (smallest integer >= A)");
	GMT_Message (API, GMT_TIME_NONE, "     CHI2CRIT   2 1  ");	GMT_Usage (API, -21, "Chi-squared distribution critical value for alpha = A and nu = B");
	GMT_Message (API, GMT_TIME_NONE, "     CHI2CDF    2 1  ");	GMT_Usage (API, -21, "Chi-squared cumulative distribution function for chi2 = A and nu = B");
	GMT_Message (API, GMT_TIME_NONE, "     CHI2PDF    2 1  ");	GMT_Usage (API, -21, "Chi-squared probability density function for chi = A and nu = B");
	GMT_Message (API, GMT_TIME_NONE, "     COMB       2 1  ");	GMT_Usage (API, -21, "Combinations n_C_r, with n = A and r = B");
	GMT_Message (API, GMT_TIME_NONE, "     CORRCOEFF  2 1  ");	GMT_Usage (API, -21, "Correlation coefficient r(A, B)");
	GMT_Message (API, GMT_TIME_NONE, "     COS        1 1  ");	GMT_Usage (API, -21, "cos (A) (A in radians)");
	GMT_Message (API, GMT_TIME_NONE, "     COSD       1 1  ");	GMT_Usage (API, -21, "cos (A) (A in degrees)");
	GMT_Message (API, GMT_TIME_NONE, "     COSH       1 1  ");	GMT_Usage (API, -21, "cosh (A)");
	GMT_Message (API, GMT_TIME_NONE, "     COT        1 1  ");	GMT_Usage (API, -21, "cot (A) (A in radians)");
	GMT_Message (API, GMT_TIME_NONE, "     COTD       1 1  ");	GMT_Usage (API, -21, "cot (A) (A in degrees)");
	GMT_Message (API, GMT_TIME_NONE, "     COTH       1 1  ");	GMT_Usage (API, -21, "coth (A)");
	GMT_Message (API, GMT_TIME_NONE, "     CSC        1 1  ");	GMT_Usage (API, -21, "csc (A) (A in radians)");
	GMT_Message (API, GMT_TIME_NONE, "     CSCD       1 1  ");	GMT_Usage (API, -21, "csc (A) (A in degrees)");
	GMT_Message (API, GMT_TIME_NONE, "     CSCH       1 1  ");	GMT_Usage (API, -21, "csch (A)");
	GMT_Message (API, GMT_TIME_NONE, "     CUMSUM     2 1  ");	GMT_Usage (API, -21, "Cumulative sum of rows (B=+/-1|3) or columns (B=+/-2|4) in A");
	GMT_Message (API, GMT_TIME_NONE, "     CURV       1 1  ");	GMT_Usage (API, -21, "Curvature of A (Laplacian)");
	GMT_Message (API, GMT_TIME_NONE, "     D2DX2      1 1  ");	GMT_Usage (API, -21, "d^2(A)/dx^2 Central 2nd derivative");
	GMT_Message (API, GMT_TIME_NONE, "     D2DY2      1 1  ");	GMT_Usage (API, -21, "d^2(A)/dy^2 Central 2nd derivative");
	GMT_Message (API, GMT_TIME_NONE, "     D2DXY      1 1  ");	GMT_Usage (API, -21, "d^2(A)/dxdy Central 2nd cross-derivative");
	GMT_Message (API, GMT_TIME_NONE, "     D2R        1 1  ");	GMT_Usage (API, -21, "Converts Degrees to Radians");
	GMT_Message (API, GMT_TIME_NONE, "     DAYNIGHT   3 1  ");	GMT_Usage (API, -21, "1 where sun at (A, B) shines and 0 elsewhere, with C transition width");
	GMT_Message (API, GMT_TIME_NONE, "     DDX        1 1  ");	GMT_Usage (API, -21, "d(A)/dx Central 1st derivative");
	GMT_Message (API, GMT_TIME_NONE, "     DDY        1 1  ");	GMT_Usage (API, -21, "d(A)/dy Central 1st derivative");
	GMT_Message (API, GMT_TIME_NONE, "     DEG2KM     1 1  ");	GMT_Usage (API, -21, "Converts Spherical Degrees to Kilometers");
	GMT_Message (API, GMT_TIME_NONE, "     DENAN      2 1  ");	GMT_Usage (API, -21, "Replace NaNs in A with values from B");
	GMT_Message (API, GMT_TIME_NONE, "     DILOG      1 1  ");	GMT_Usage (API, -21, "dilog (A)");
	GMT_Message (API, GMT_TIME_NONE, "     DIV        2 1  ");	GMT_Usage (API, -21, "A / B");
	GMT_Message (API, GMT_TIME_NONE, "     DOT        2 1  ");	GMT_Usage (API, -21, "Dot product (2-D Cartesian or 3-D geographic) of vector (A,B) with grid nodes locations");
	GMT_Message (API, GMT_TIME_NONE, "     DUP        1 2  ");	GMT_Usage (API, -21, "Places duplicate of A on the stack");
	GMT_Message (API, GMT_TIME_NONE, "     ECDF       2 1  ");	GMT_Usage (API, -21, "Exponential cumulative distribution function for x = A and lambda = B");
	GMT_Message (API, GMT_TIME_NONE, "     ECRIT      2 1  ");	GMT_Usage (API, -21, "Exponential distribution critical value for alpha = A and lambda = B");
	GMT_Message (API, GMT_TIME_NONE, "     EPDF       2 1  ");	GMT_Usage (API, -21, "Exponential probability density function for x = A and lambda = B");
	GMT_Message (API, GMT_TIME_NONE, "     ERF        1 1  ");	GMT_Usage (API, -21, "Error function erf (A)");
	GMT_Message (API, GMT_TIME_NONE, "     ERFC       1 1  ");	GMT_Usage (API, -21, "Complementary Error function erfc (A)");
	GMT_Message (API, GMT_TIME_NONE, "     EQ         2 1  ");	GMT_Usage (API, -21, "1 if A == B, else 0");
	GMT_Message (API, GMT_TIME_NONE, "     ERFINV     1 1  ");	GMT_Usage (API, -21, "Inverse error function of A");
	GMT_Message (API, GMT_TIME_NONE, "     EXCH       2 2  ");	GMT_Usage (API, -21, "Exchanges A and B on the stack");
	GMT_Message (API, GMT_TIME_NONE, "     EXP        1 1  ");	GMT_Usage (API, -21, "exp (A)");
	GMT_Message (API, GMT_TIME_NONE, "     FACT       1 1  ");	GMT_Usage (API, -21, "A! (A factorial)");
	GMT_Message (API, GMT_TIME_NONE, "     EXTREMA    1 1  ");	GMT_Usage (API, -21, "Local extrema: -1 is a (local) minimum, +1 a (local) maximum, and 0 elsewhere");
	GMT_Message (API, GMT_TIME_NONE, "     FCRIT      3 1  ");	GMT_Usage (API, -21, "F distribution critical value for alpha = A, nu1 = B, and nu2 = C");
	GMT_Message (API, GMT_TIME_NONE, "     FCDF       3 1  ");	GMT_Usage (API, -21, "F cumulative distribution function for F = A, nu1 = B, and nu2 = C");
	GMT_Message (API, GMT_TIME_NONE, "     FISHER     3 1  ");	GMT_Usage (API, -21, "Fisher probability density function at grid nodes given stack lon,lat (A, B) and kappa (C)");
	GMT_Message (API, GMT_TIME_NONE, "     FLIPLR     1 1  ");	GMT_Usage (API, -21, "Reverse order of values in each row");
	GMT_Message (API, GMT_TIME_NONE, "     FLIPUD     1 1  ");	GMT_Usage (API, -21, "Reverse order of values in each column");
	GMT_Message (API, GMT_TIME_NONE, "     FLOOR      1 1  ");	GMT_Usage (API, -21, "floor (A) (greatest integer <= A)");
	GMT_Message (API, GMT_TIME_NONE, "     FMOD       2 1  ");	GMT_Usage (API, -21, "A % B (remainder after truncated division)");
	GMT_Message (API, GMT_TIME_NONE, "     FPDF       3 1  ");	GMT_Usage (API, -21, "F probability density function for F = A, nu1 = B and nu2 = C");
	GMT_Message (API, GMT_TIME_NONE, "     GE         2 1  ");	GMT_Usage (API, -21, "1 if A >= B, else 0");
	GMT_Message (API, GMT_TIME_NONE, "     GT         2 1  ");	GMT_Usage (API, -21, "1 if A > B, else 0");
	GMT_Message (API, GMT_TIME_NONE, "     HSV2LAB    3 3  ");	GMT_Usage (API, -21, "Convert hsv to lab, with h = A, s = B and v = C");
	GMT_Message (API, GMT_TIME_NONE, "     HSV2RGB    3 3  ");	GMT_Usage (API, -21, "Convert hsv to rgb, with h = A, s = B and v = C");
	GMT_Message (API, GMT_TIME_NONE, "     HSV2XYZ    3 3  ");	GMT_Usage (API, -21, "Convert hsv to xyz, with h = A, s = B and v = C");
	GMT_Message (API, GMT_TIME_NONE, "     HYPOT      2 1  ");	GMT_Usage (API, -21, "hypot (A, B) = sqrt (A*A + B*B)");
	GMT_Message (API, GMT_TIME_NONE, "     I0         1 1  ");	GMT_Usage (API, -21, "Modified Bessel function of A (1st kind, order 0)");
	GMT_Message (API, GMT_TIME_NONE, "     I1         1 1  ");	GMT_Usage (API, -21, "Modified Bessel function of A (1st kind, order 1)");
	GMT_Message (API, GMT_TIME_NONE, "     IFELSE     3 1  ");	GMT_Usage (API, -21, "B if A != 0, else C");
	GMT_Message (API, GMT_TIME_NONE, "     IN         2 1  ");	GMT_Usage (API, -21, "Modified Bessel function of A (1st kind, order B)");
	GMT_Message (API, GMT_TIME_NONE, "     INRANGE    3 1  ");	GMT_Usage (API, -21, "1 if B <= A <= C, else 0");
	GMT_Message (API, GMT_TIME_NONE, "     INSIDE     1 1  ");	GMT_Usage (API, -21, "1 when inside or on polygon(s) in A, else 0");
	GMT_Message (API, GMT_TIME_NONE, "     INV        1 1  ");	GMT_Usage (API, -21, "1 / A");
	GMT_Message (API, GMT_TIME_NONE, "     ISFINITE   1 1  ");	GMT_Usage (API, -21, "1 if A is finite, else 0");
	GMT_Message (API, GMT_TIME_NONE, "     ISNAN      1 1  ");	GMT_Usage (API, -21, "1 if A == NaN, else 0");
	GMT_Message (API, GMT_TIME_NONE, "     J0         1 1  ");	GMT_Usage (API, -21, "Bessel function of A (1st kind, order 0)");
	GMT_Message (API, GMT_TIME_NONE, "     J1         1 1  ");	GMT_Usage (API, -21, "Bessel function of A (1st kind, order 1)");
	GMT_Message (API, GMT_TIME_NONE, "     JN         2 1  ");	GMT_Usage (API, -21, "Bessel function of A (1st kind, order B)");
	GMT_Message (API, GMT_TIME_NONE, "     K0         1 1  ");	GMT_Usage (API, -21, "Modified Kelvin function of A (2nd kind, order 0)");
	GMT_Message (API, GMT_TIME_NONE, "     K1         1 1  ");	GMT_Usage (API, -21, "Modified Bessel function of A (2nd kind, order 1)");
	GMT_Message (API, GMT_TIME_NONE, "     KEI        1 1  ");	GMT_Usage (API, -21, "Kelvin function kei (A)");
	GMT_Message (API, GMT_TIME_NONE, "     KER        1 1  ");	GMT_Usage (API, -21, "Kelvin function ker (A)");
	GMT_Message (API, GMT_TIME_NONE, "     KM2DEG     1 1  ");	GMT_Usage (API, -21, "Converts Kilometers to Spherical Degrees");
	GMT_Message (API, GMT_TIME_NONE, "     KN         2 1  ");	GMT_Usage (API, -21, "Modified Bessel function of A (2nd kind, order B)");
	GMT_Message (API, GMT_TIME_NONE, "     KURT       1 1  ");	GMT_Usage (API, -21, "Kurtosis of A");
	GMT_Message (API, GMT_TIME_NONE, "     LAB2HSV    3 3  ");	GMT_Usage (API, -21, "Convert lab to hsv, with l = A, a = B and b = C");
	GMT_Message (API, GMT_TIME_NONE, "     LAB2RGB    3 3  ");	GMT_Usage (API, -21, "Convert lab to rgb, with l = A, a = B and b = C");
	GMT_Message (API, GMT_TIME_NONE, "     LAB2XYZ    3 3  ");	GMT_Usage (API, -21, "Convert lab to xyz, with l = A, a = B and b = C");
	GMT_Message (API, GMT_TIME_NONE, "     LCDF       1 1  ");	GMT_Usage (API, -21, "Laplace cumulative distribution function for z = A");
	GMT_Message (API, GMT_TIME_NONE, "     LCRIT      1 1  ");	GMT_Usage (API, -21, "Laplace distribution critical value for alpha = A");
	GMT_Message (API, GMT_TIME_NONE, "     LDIST      1 1  ");	GMT_Usage (API, -21, "Compute minimum distance (in km if -fg) from lines in multi-segment ASCII file A");
	GMT_Message (API, GMT_TIME_NONE, "     LDISTG     0 1  ");	GMT_Usage (API, -21, "As LDIST, but operates on the GSHHG dataset (see -A, -D for options)");
	GMT_Message (API, GMT_TIME_NONE, "     LDIST2     2 1  ");	GMT_Usage (API, -21, "As LDIST, from lines in ASCII file B but only to nodes where A != 0");
	GMT_Message (API, GMT_TIME_NONE, "     LE         2 1  ");	GMT_Usage (API, -21, "1 if A <= B, else 0");
	GMT_Message (API, GMT_TIME_NONE, "     LOG        1 1  ");	GMT_Usage (API, -21, "log (A) (natural log)");
	GMT_Message (API, GMT_TIME_NONE, "     LOG10      1 1  ");	GMT_Usage (API, -21, "log10 (A) (base 10)");
	GMT_Message (API, GMT_TIME_NONE, "     LOG1P      1 1  ");	GMT_Usage (API, -21, "log (1+A) (accurate for small A)");
	GMT_Message (API, GMT_TIME_NONE, "     LOG2       1 1  ");	GMT_Usage (API, -21, "log2 (A) (base 2)");
	GMT_Message (API, GMT_TIME_NONE, "     LMSSCL     1 1  ");	GMT_Usage (API, -21, "LMS scale estimate (LMS STD) of A");
	GMT_Message (API, GMT_TIME_NONE, "     LMSSCLW    1 1  ");	GMT_Usage (API, -21, "Weighted LMS scale estimate (LMS STD) of A for weights in B");
	GMT_Message (API, GMT_TIME_NONE, "     LOWER      1 1  ");	GMT_Usage (API, -21, "The lowest (minimum) value of A");
	GMT_Message (API, GMT_TIME_NONE, "     LPDF       1 1  ");	GMT_Usage (API, -21, "Laplace probability density function for z = A");
	GMT_Message (API, GMT_TIME_NONE, "     LRAND      2 1  ");	GMT_Usage (API, -21, "Laplace random noise with mean A and std. deviation B");
	GMT_Message (API, GMT_TIME_NONE, "     LT         2 1  ");	GMT_Usage (API, -21, "1 if A < B, else 0");
	GMT_Message (API, GMT_TIME_NONE, "     MAD        1 1  ");	GMT_Usage (API, -21, "Median Absolute Deviation (L1 STD) of A");
	GMT_Message (API, GMT_TIME_NONE, "     MADW       2 1  ");	GMT_Usage (API, -21, "Weighted Median Absolute Deviation (L1 STD) of A for weights in B");
	GMT_Message (API, GMT_TIME_NONE, "     MAX        2 1  ");	GMT_Usage (API, -21, "Maximum of A and B");
	GMT_Message (API, GMT_TIME_NONE, "     MEAN       1 1  ");	GMT_Usage (API, -21, "Mean value of A");
	GMT_Message (API, GMT_TIME_NONE, "     MEANW      2 1  ");	GMT_Usage (API, -21, "Weighted mean value of A for weights in B");
	GMT_Message (API, GMT_TIME_NONE, "     MEDIAN     1 1  ");	GMT_Usage (API, -21, "Median value of A");
	GMT_Message (API, GMT_TIME_NONE, "     MEDIANW    2 1  ");	GMT_Usage (API, -21, "Weighted median value of A for weights in B");
	GMT_Message (API, GMT_TIME_NONE, "     MIN        2 1  ");	GMT_Usage (API, -21, "Minimum of A and B");
	GMT_Message (API, GMT_TIME_NONE, "     MOD        2 1  ");	GMT_Usage (API, -21, "A mod B (remainder after floored division)");
	GMT_Message (API, GMT_TIME_NONE, "     MODE       1 1  ");	GMT_Usage (API, -21, "Mode value (Least Median of Squares) of A");
	GMT_Message (API, GMT_TIME_NONE, "     MODEW      2 1  ");	GMT_Usage (API, -21, "Weighted mode value of A for weights in B");
	GMT_Message (API, GMT_TIME_NONE, "     MUL        2 1  ");	GMT_Usage (API, -21, "A * B");
	GMT_Message (API, GMT_TIME_NONE, "     NAN        2 1  ");	GMT_Usage (API, -21, "NaN if A == B, else A");
	GMT_Message (API, GMT_TIME_NONE, "     NEG        1 1  ");	GMT_Usage (API, -21, "-A");
	GMT_Message (API, GMT_TIME_NONE, "     NEQ        2 1  ");	GMT_Usage (API, -21, "1 if A != B, else 0");
	GMT_Message (API, GMT_TIME_NONE, "     NORM       1 1  ");	GMT_Usage (API, -21, "Normalize (A) so min(A) = 0 and max(A) = 1");
	GMT_Message (API, GMT_TIME_NONE, "     NOT        1 1  ");	GMT_Usage (API, -21, "NaN if A == NaN, 1 if A == 0, else 0");
	GMT_Message (API, GMT_TIME_NONE, "     NRAND      2 1  ");	GMT_Usage (API, -21, "Normal, random values with mean A and std. deviation B");
	GMT_Message (API, GMT_TIME_NONE, "     OR         2 1  ");	GMT_Usage (API, -21, "NaN if B == NaN, else A");
	GMT_Message (API, GMT_TIME_NONE, "     PCDF       2 1  ");	GMT_Usage (API, -21, "Poisson cumulative distribution function x = A and lambda = B");
	GMT_Message (API, GMT_TIME_NONE, "     PPDF       2 1  ");	GMT_Usage (API, -21, "Poisson probability density function for x = A and lambda = B");
	GMT_Message (API, GMT_TIME_NONE, "     PDIST      1 1  ");	GMT_Usage (API, -21, "Compute minimum distance (in km if -fg) from points in ASCII file A");
	GMT_Message (API, GMT_TIME_NONE, "     PDIST2     2 1  ");	GMT_Usage (API, -21, "As PDIST, from points in ASCII file B but only to nodes where A != 0");
	GMT_Message (API, GMT_TIME_NONE, "     PERM       2 1  ");	GMT_Usage (API, -21, "Permutations n_P_r, with n = A and r = B");
	GMT_Message (API, GMT_TIME_NONE, "     POP        1 0  ");	GMT_Usage (API, -21, "Delete top element from the stack");
	GMT_Message (API, GMT_TIME_NONE, "     PLM        3 1  ");	GMT_Usage (API, -21, "Associated Legendre polynomial P(A) degree B order C");
	GMT_Message (API, GMT_TIME_NONE, "     PLMg       3 1  ");	GMT_Usage (API, -21, "Normalized associated Legendre polynomial P(A) degree B order C (geophysical convention)");
	GMT_Message (API, GMT_TIME_NONE, "     POINT      1 2  ");	GMT_Usage (API, -21, "Return mean_x mean_y of points in ASCII file A");
	GMT_Message (API, GMT_TIME_NONE, "     POW        2 1  ");	GMT_Usage (API, -21, "A ^ B");
	GMT_Message (API, GMT_TIME_NONE, "     PQUANT     2 1  ");	GMT_Usage (API, -21, "The B'th Quantile (0-100%) of A");
	GMT_Message (API, GMT_TIME_NONE, "     PQUANTW    3 1  ");	GMT_Usage (API, -21, "The C'th Quantile (0-100%) of A for weights in B");
	GMT_Message (API, GMT_TIME_NONE, "     PSI        1 1  ");	GMT_Usage (API, -21, "Psi (or Digamma) of A");
	GMT_Message (API, GMT_TIME_NONE, "     PV         3 1  ");	GMT_Usage (API, -21, "Legendre function Pv(A) of degree v = real(B) + imag(C)");
	GMT_Message (API, GMT_TIME_NONE, "     QV         3 1  ");	GMT_Usage (API, -21, "Legendre function Qv(A) of degree v = real(B) + imag(C)");
	GMT_Message (API, GMT_TIME_NONE, "     R2         2 1  ");	GMT_Usage (API, -21, "R2 = A^2 + B^2");
	GMT_Message (API, GMT_TIME_NONE, "     R2D        1 1  ");	GMT_Usage (API, -21, "Convert Radians to Degrees");
	GMT_Message (API, GMT_TIME_NONE, "     RAND       2 1  ");	GMT_Usage (API, -21, "Uniform random values between A and B");
	GMT_Message (API, GMT_TIME_NONE, "     RCDF       1 1  ");	GMT_Usage (API, -21, "Rayleigh cumulative distribution function for z = A");
	GMT_Message (API, GMT_TIME_NONE, "     RCRIT      1 1  ");	GMT_Usage (API, -21, "Rayleigh distribution critical value for alpha = A");
	GMT_Message (API, GMT_TIME_NONE, "     RGB2HSV    3 3  ");	GMT_Usage (API, -21, "Convert rgb to hsv, with r = A, g = B and b = C");
	GMT_Message (API, GMT_TIME_NONE, "     RGB2LAB    3 3  ");	GMT_Usage (API, -21, "Convert rgb to lab, with r = A, g = B and b = C");
	GMT_Message (API, GMT_TIME_NONE, "     RGB2XYZ    3 3  ");	GMT_Usage (API, -21, "Convert rgb to xyz, with r = A, g = B and b = C");
	GMT_Message (API, GMT_TIME_NONE, "     RINT       1 1  ");	GMT_Usage (API, -21, "rint (A) (round to integral value nearest to A)");
	GMT_Message (API, GMT_TIME_NONE, "     RMS        1 1  ");	GMT_Usage (API, -21, "Root-mean-square of A");
	GMT_Message (API, GMT_TIME_NONE, "     RMSW       2 1  ");	GMT_Usage (API, -21, "Weighted Root-mean-square of A for weights in B");
	GMT_Message (API, GMT_TIME_NONE, "     RPDF       1 1  ");	GMT_Usage (API, -21, "Rayleigh probability density function for z = A");
	GMT_Message (API, GMT_TIME_NONE, "     ROLL       2 0  ");	GMT_Usage (API, -21, "Cyclically shifts the top A stack items by an amount B");
	GMT_Message (API, GMT_TIME_NONE, "     ROTX       2 1  ");	GMT_Usage (API, -21, "Rotate A by the (constant) shift B in x-direction");
	GMT_Message (API, GMT_TIME_NONE, "     ROTY       2 1  ");	GMT_Usage (API, -21, "Rotate A by the (constant) shift B in y-direction");
	GMT_Message (API, GMT_TIME_NONE, "     SADDLE     1 1  ");	GMT_Usage (API, -21, "Critical points: -1|+1 is a saddle with min|max in x-direction; 0 elsewhere");
	GMT_Message (API, GMT_TIME_NONE, "     SAZ        2 1  ");	GMT_Usage (API, -21, "Spherical azimuth from grid nodes to stack x,y");
	GMT_Message (API, GMT_TIME_NONE, "     SBAZ       2 1  ");	GMT_Usage (API, -21, "Spherical back-azimuth from grid nodes to stack x,y");
	GMT_Message (API, GMT_TIME_NONE, "     SEC        1 1  ");	GMT_Usage (API, -21, "sec (A) (A in radians)");
	GMT_Message (API, GMT_TIME_NONE, "     SECD       1 1  ");	GMT_Usage (API, -21, "sec (A) (A in degrees)");
	GMT_Message (API, GMT_TIME_NONE, "     SECH       1 1  ");	GMT_Usage (API, -21, "sech (A)");
	GMT_Message (API, GMT_TIME_NONE, "     SDIST      2 1  ");	GMT_Usage (API, -21, "Spherical distance (in km) between grid nodes and stack lon,lat (A, B)");
	GMT_Message (API, GMT_TIME_NONE, "     SDIST2     2 1  ");	GMT_Usage (API, -21, "As SDIST but only to nodes that are != 0");
	GMT_Message (API, GMT_TIME_NONE, "     SIGN       1 1  ");	GMT_Usage (API, -21, "sign (+1 or -1) of A");
	GMT_Message (API, GMT_TIME_NONE, "     SIN        1 1  ");	GMT_Usage (API, -21, "sin (A) (A in radians)");
	GMT_Message (API, GMT_TIME_NONE, "     SINC       1 1  ");	GMT_Usage (API, -21, "sinc (A) (sin (pi*A)/(pi*A))");
	GMT_Message (API, GMT_TIME_NONE, "     SIND       1 1  ");	GMT_Usage (API, -21, "sin (A) (A in degrees)");
	GMT_Message (API, GMT_TIME_NONE, "     SINH       1 1  ");	GMT_Usage (API, -21, "sinh (A)");
	GMT_Message (API, GMT_TIME_NONE, "     SKEW       1 1  ");	GMT_Usage (API, -21, "Skewness of A");
	GMT_Message (API, GMT_TIME_NONE, "     SQR        1 1  ");	GMT_Usage (API, -21, "A^2");
	GMT_Message (API, GMT_TIME_NONE, "     SQRT       1 1  ");	GMT_Usage (API, -21, "sqrt (A)");
	GMT_Message (API, GMT_TIME_NONE, "     STD        1 1  ");	GMT_Usage (API, -21, "Standard deviation of A");
	GMT_Message (API, GMT_TIME_NONE, "     STDW       2 1  ");	GMT_Usage (API, -21, "Weighted standard deviation of A for weights in B");
	GMT_Message (API, GMT_TIME_NONE, "     STEP       1 1  ");	GMT_Usage (API, -21, "Heaviside step function: H(A)");
	GMT_Message (API, GMT_TIME_NONE, "     STEPX      1 1  ");	GMT_Usage (API, -21, "Heaviside step function in x: H(x-A)");
	GMT_Message (API, GMT_TIME_NONE, "     STEPY      1 1  ");	GMT_Usage (API, -21, "Heaviside step function in y: H(y-A)");
	GMT_Message (API, GMT_TIME_NONE, "     SUB        2 1  ");	GMT_Usage (API, -21, "A - B");
	GMT_Message (API, GMT_TIME_NONE, "     SUM        1 1  ");	GMT_Usage (API, -21, "Sum of all values in A");
	GMT_Message (API, GMT_TIME_NONE, "     TAN        1 1  ");	GMT_Usage (API, -21, "tan (A) (A in radians)");
	GMT_Message (API, GMT_TIME_NONE, "     TAND       1 1  ");	GMT_Usage (API, -21, "tan (A) (A in degrees)");
	GMT_Message (API, GMT_TIME_NONE, "     TANH       1 1  ");	GMT_Usage (API, -21, "tanh (A)");
	GMT_Message (API, GMT_TIME_NONE, "     TAPER      2 1  ");	GMT_Usage (API, -21, "Unit weights cosine-tapered to zero within A and B of x and y grid margins");
	GMT_Message (API, GMT_TIME_NONE, "     TN         2 1  ");	GMT_Usage (API, -21, "Chebyshev polynomial Tn(-1<t<+1,n), with t = A, and n = B");
	GMT_Message (API, GMT_TIME_NONE, "     TCRIT      2 1  ");	GMT_Usage (API, -21, "Student's t-distribution critical value for alpha = A and nu = B");
	GMT_Message (API, GMT_TIME_NONE, "     TCDF       2 1  ");	GMT_Usage (API, -21, "Student's t cumulative distribution function for t = A, and nu = B");
	GMT_Message (API, GMT_TIME_NONE, "     TPDF       2 1  ");	GMT_Usage (API, -21, "Student's t probability density function for t = A and nu = B");
	GMT_Message (API, GMT_TIME_NONE, "     TRIM       3 1  ");	GMT_Usage (API, -21, "Alpha-trimming for %%-left = A, %%-right = B, and grid = C");
	GMT_Message (API, GMT_TIME_NONE, "     UPPER      1 1  ");	GMT_Usage (API, -21, "The highest (maximum) value of A");
	GMT_Message (API, GMT_TIME_NONE, "     VAR        1 1  ");	GMT_Usage (API, -21, "Variance of A");
	GMT_Message (API, GMT_TIME_NONE, "     VARW       2 1  ");	GMT_Usage (API, -21, "Weighted variance of A for weights in B");
	GMT_Message (API, GMT_TIME_NONE, "     VPDF       3 1  ");	GMT_Usage (API, -21, "Von Mises probability density function for angles = A, mu = B and kappa = C");
	GMT_Message (API, GMT_TIME_NONE, "     WCDF       3 1  ");	GMT_Usage (API, -21, "Weibull cumulative distribution function for x = A, scale = B, and shape = C");
	GMT_Message (API, GMT_TIME_NONE, "     WCRIT      3 1  ");	GMT_Usage (API, -21, "Weibull distribution critical value for alpha = A, scale = B, and shape = C");
	GMT_Message (API, GMT_TIME_NONE, "     WPDF       3 1  ");	GMT_Usage (API, -21, "Weibull probability density function for x = A, scale = B and shape = C");
	GMT_Message (API, GMT_TIME_NONE, "     WRAP       1 1  ");	GMT_Usage (API, -21, "wrap (A). (A in radians)");
	GMT_Message (API, GMT_TIME_NONE, "     XOR        2 1  ");	GMT_Usage (API, -21, "0 if A == NaN and B == NaN, NaN if B == NaN, else A");
	GMT_Message (API, GMT_TIME_NONE, "     XYZ2HSV    3 3  ");	GMT_Usage (API, -21, "Convert xyz to hsv, with x = A, y = B and z = C");
	GMT_Message (API, GMT_TIME_NONE, "     XYZ2LAB    3 3  ");	GMT_Usage (API, -21, "Convert xyz to lab, with x = A, y = B and z = C");
	GMT_Message (API, GMT_TIME_NONE, "     XYZ2RGB    3 3  ");	GMT_Usage (API, -21, "Convert xyz to rgb, with x = A, y = B and z = C");
	GMT_Message (API, GMT_TIME_NONE, "     Y0         1 1  ");	GMT_Usage (API, -21, "Bessel function of A (2nd kind, order 0)");
	GMT_Message (API, GMT_TIME_NONE, "     Y1         1 1  ");	GMT_Usage (API, -21, "Bessel function of A (2nd kind, order 1)");
	GMT_Message (API, GMT_TIME_NONE, "     YLM        2 2  ");	GMT_Usage (API, -21, "Re and Im orthonormalized spherical harmonics degree A order B");
	GMT_Message (API, GMT_TIME_NONE, "     YLMg       2 2  ");	GMT_Usage (API, -21, "Cos and Sin normalized spherical harmonics degree A order B (geophysical convention)");
	GMT_Message (API, GMT_TIME_NONE, "     YN         2 1  ");	GMT_Usage (API, -21, "Bessel function of A (2nd kind, order B)");
	GMT_Message (API, GMT_TIME_NONE, "     ZCRIT      1 1  ");	GMT_Usage (API, -21, "Normal distribution critical value for alpha = A");
	GMT_Message (API, GMT_TIME_NONE, "     ZCDF       1 1  ");	GMT_Usage (API, -21, "Normal cumulative distribution function for z = A");
	GMT_Message (API, GMT_TIME_NONE, "     ZPDF       1 1  ");	GMT_Usage (API, -21, "Normal probability density function for z = A");
	GMT_Usage (API, -2, "\nThe special constants are:\n");
	GMT_Message (API, GMT_TIME_NONE, "     PI                 = "); GMT_Usage (API, -26, "3.1415926...");
	GMT_Message (API, GMT_TIME_NONE, "     E                  = "); GMT_Usage (API, -26, "2.7182818...");
	GMT_Message (API, GMT_TIME_NONE, "     F_EPS (single eps) = "); GMT_Usage (API, -26, "1.192092896e-07");
	GMT_Message (API, GMT_TIME_NONE, "     EULER              = "); GMT_Usage (API, -26, "0.5772156...");
	GMT_Message (API, GMT_TIME_NONE, "     PHI (golden ratio) = "); GMT_Usage (API, -26, "1.6180339...");
	GMT_Message (API, GMT_TIME_NONE, "     XMIN or YMIN       = "); GMT_Usage (API, -26, "minimum value of x or y");
	GMT_Message (API, GMT_TIME_NONE, "     XMAX or YMAX       = "); GMT_Usage (API, -26, "maximum value of x or y");
	GMT_Message (API, GMT_TIME_NONE, "     XRANGE or YRANGE   = "); GMT_Usage (API, -26, "full range of x or y");
	GMT_Message (API, GMT_TIME_NONE, "     XINC or YINC       = "); GMT_Usage (API, -26, "increment in x or y");
	GMT_Message (API, GMT_TIME_NONE, "     NX or NY           = "); GMT_Usage (API, -26, "dimension of x or y");
	GMT_Usage (API, -2, "\nThe special grids are:\n");
	GMT_Message (API, GMT_TIME_NONE, "     NODE               = "); GMT_Usage (API, -26, "grid with continuous node indices (0-(NX*NY-1))");
	GMT_Message (API, GMT_TIME_NONE, "     NODEP              = "); GMT_Usage (API, -26, "grid with discontinuous node indices due to padding");
	GMT_Message (API, GMT_TIME_NONE, "     X or Y             = "); GMT_Usage (API, -26, "grid with x- or y-coordinates");
	GMT_Message (API, GMT_TIME_NONE, "     XNORM or YNORM     = "); GMT_Usage (API, -26, "grid with normalized [-1|+1] x- or y-coordinates");
	GMT_Message (API, GMT_TIME_NONE, "     XCOL               = "); GMT_Usage (API, -26, "grid with column numbers 0, 1, ..., NX-1");
	GMT_Message (API, GMT_TIME_NONE, "     YROW               = "); GMT_Usage (API, -26, "grid with row numbers 0, 1, ..., NY-1");
	GMT_Usage (API, -2, "\nUse macros for frequently used long expressions; see the grdmath documentation. "
		"Store stack to named variable via STO@<label>, recall via [RCL]@<label>, clear via CLR@<label>.");

	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:"
		"\n  (only use -R|I|r|f if no grid files are passed as arguments).\n");
	gmt_GSHHG_syntax (API->GMT, 'A');
	GMT_Usage (API, -2, "Note: -A is only relevant if using the LDISTG operator.");
	GMT_Usage (API, 1, "\n-D<resolution>[+f]");
	GMT_Usage (API, -2, "Choose one of the following resolutions to use with the LDISTG operator:");
	GMT_Usage (API, 3, "f: Full resolution (may be very slow for large regions).");
	GMT_Usage (API, 3, "h: High resolution (may be slow for large regions).");
	GMT_Usage (API, 3, "i: Intermediate resolution.");
	GMT_Usage (API, 3, "l: Low resolution [Default].");
	GMT_Usage (API, 3, "c: Crude resolution, for busy plots that need crude continent outlines only.");
	GMT_Usage (API, -2, "Append +f to use a lower resolution should the chosen one not be available [Default will abort]. ");
	GMT_Usage (API, -2, "Note: -D is only relevant if using the LDISTG operator.");
	GMT_Option (API, "I");
	GMT_Usage (API, 1, "\n-M Handle map units in derivatives.  In this case, dx,dy of grid "
		"will be converted from degrees lon,lat into meters (Flat-earth approximation). "
		"Default computes derivatives in units of data/grid_distance.");
	GMT_Usage (API, 1, "\n-N Do not perform strict domain check if several grids are involved. "
		"[Default checks that domain is within %g * [xinc or yinc] of each other].", GMT_CONV4_LIMIT);
	GMT_Option (API, "R");
	GMT_Usage (API, 1, "\n-S Reduce the entire Stack to a single layer by applying the next operator to "
		"co-registered nodes across the stack.  You must select a reducing operator, i.e., "
		"ADD, AND, MAD, LMSSCL, MAX, MEAN, MEDIAN, MIN, MODE, MUL, RMS, STD, SUB, VAR or XOR. "
		"Note: Select -S after you have placed all items of interest on the stack.");
	GMT_Option (API, "V");
	GMT_Option (API, "a,bi2,di,e,f,g,h,i");
	if (gmt_M_showusage (API)) GMT_Usage (API, -2, "Note: Only applies to the input files for operators LDIST, PDIST, POINT and INSIDE.");
	GMT_Option (API, "n,r,x,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct GRDMATH_CTRL *Ctrl, struct GMT_OPTION *options) {
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
					if (opt->next && (opt->next->option == GMT_OPT_INFILE)) {
						Ctrl->Out.active = true;
						if (opt->next->option == GMT_OPT_OUTFILE)
							opt->next->option = GMT_OPT_OUTFILE2;	/* See definition of this for reason */
					}
				}
				break;
			case '=':	/* Output files */
			case '>':	/* Output files */
				missing_equal = false;
				break;
			case '#':	/* Numbers */
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Restrict GSHHS features */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
				n_errors += gmt_set_levels (GMT, opt->arg, &Ctrl->A.info);
				break;
			case 'C':	/* Control default CPT */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				if (opt->arg[0]) Ctrl->C.cpt = opt->arg;	/* Just pass pointer if given an argument */
				break;
			case 'D':	/* Set GSHHS resolution */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->D.active);
				n_errors += gmt_get_required_char (GMT, opt->arg, opt->option, 0, &Ctrl->D.set);
				Ctrl->D.force = (opt->arg[1] == '+');
				break;
			case 'I':	/* Grid spacings */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'M':	/* Map units */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'N':	/* Relax domain check */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				n_errors += gmt_get_no_argument (GMT, opt->arg, opt->option, 0);
				break;
			case 'S':	/* Only checked later */
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
		}
	}

	if (missing_equal) {
		GMT_Report (API, GMT_MSG_ERROR, "Usage is <operations> = <outgrid>\n");
		n_errors++;
	}
	if (GMT->common.R.active[ISET] && (GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0)) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -I: Must specify positive increment(s)\n");
		n_errors++;
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL struct GMT_GRID *grdmath_alloc_stack_grid (struct GMT_CTRL *GMT, struct GMT_GRID *Template) {
	/* Allocate a new GMT_GRID structure based on dimensions etc of the Template */
	struct GMT_GRID *New = GMT_Create_Data (GMT->parent, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Template->header->wesn, Template->header->inc, \
		Template->header->registration, GMT_NOTSET, NULL);
	return (New);
}

GMT_LOCAL int grdmath_find_stored_item (struct GMT_CTRL *GMT, struct GRDMATH_STORE *recall[], int n_stored, char *label) {
	int k = 0;
	gmt_M_unused(GMT);
	while (k < n_stored && strcmp (recall[k]->label, label)) k++;
	return (k == n_stored ? GMT_NOTSET : k);
}

/* Stack collapsing operators that work on same nodes across all stack items */

GMT_LOCAL double grdmath_stack_collapse_add (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	uint64_t k;
	double sum = array[0];
	gmt_M_unused (GMT);
	for (k = 1; k < n; k++) sum += array[k];
	return sum;
}

GMT_LOCAL double grdmath_stack_collapse_and (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	uint64_t k;
	double x = array[0];
	gmt_M_unused (GMT);
	for (k = 1; k < n; k++)
		x = (gmt_M_is_dnan (x)) ? array[k] : x;
	return x;
}

GMT_LOCAL double grdmath_stack_collapse_lmsscl (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	double lmsscl;
	gmt_sort_array (GMT, array, n, GMT_DOUBLE);
	while (n > 1 && gmt_M_is_dnan (array[n-1])) n--;
	if (n) {
		unsigned int gmt_mode_selection = 0, GMT_n_multiples = 0;
		double mode;
		gmt_mode (GMT, array, n, n/2, 0, gmt_mode_selection, &GMT_n_multiples, &mode);
		gmt_getmad (GMT, array, n, mode, &lmsscl);
	}
	else
		lmsscl = GMT->session.d_NaN;
	return lmsscl;
}

GMT_LOCAL double grdmath_stack_collapse_mad (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	double mad;
	gmt_sort_array (GMT, array, n, GMT_DOUBLE);
	while (n > 1 && gmt_M_is_dnan (array[n-1])) n--;
	if (n) {
		double med = (n%2) ? array[n/2] : 0.5 * (array[(n-1)/2] + array[n/2]);
		gmt_getmad (GMT, array, n, med, &mad);
	}
	else
		mad = GMT->session.d_NaN;
	return mad;
}

GMT_LOCAL double grdmath_stack_collapse_max (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	uint64_t k;
	double max = array[0];
	gmt_M_unused (GMT);
	for (k = 1; k < n; k++)
		if (array[k] > max) max = array[k];
	return max;
}

GMT_LOCAL double grdmath_stack_collapse_mean (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	double std = 0.0;
	return gmt_mean_and_std (GMT, array, n, &std);
}

GMT_LOCAL double grdmath_stack_collapse_median (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	double med;
	gmt_sort_array (GMT, array, n, GMT_DOUBLE);
	while (n > 1 && gmt_M_is_dnan (array[n-1])) n--;
	if (n)
		med = (n%2) ? array[n/2] : 0.5 * (array[(n-1)/2] + array[n/2]);
	else
		med = GMT->session.d_NaN;
	return med;
}

GMT_LOCAL double grdmath_stack_collapse_min (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	uint64_t k;
	double min = array[0];
	gmt_M_unused (GMT);
	for (k = 1; k < n; k++)
		if (array[k] < min) min = array[k];
	return min;
}

GMT_LOCAL double grdmath_stack_collapse_mode (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	unsigned int gmt_mode_selection = 0, GMT_n_multiples = 0;
	double mode;
	gmt_mode (GMT, array, n, n/2, true, gmt_mode_selection, &GMT_n_multiples, &mode);
	return mode;
}

GMT_LOCAL double grdmath_stack_collapse_mul (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	uint64_t k;
	double prod = array[0];
	gmt_M_unused (GMT);
	for (k = 1; k < n; k++)
		prod *= array[k];
	return prod;
}

GMT_LOCAL double grdmath_stack_collapse_or (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	uint64_t k;
	double x = array[0];
	gmt_M_unused (GMT);
	for (k = 1; k < n; k++)
	 	x = (gmt_M_is_dnan (x) || gmt_M_is_dnan (array[k])) ? GMT->session.d_NaN : x;
	return x;
}

GMT_LOCAL double grdmath_stack_collapse_rms (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	uint64_t k;
	double rms = 0.0;
	gmt_M_unused (GMT);
	for (k = 0; k < n; k++)
		rms += array[k] * array[k];
	return sqrt (rms / n);
}

GMT_LOCAL double grdmath_stack_collapse_std (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	double std = 0.0;
	(void)gmt_mean_and_std (GMT, array, n, &std);
	return std;
}

GMT_LOCAL double grdmath_stack_collapse_sub (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	uint64_t k;
	double sum = array[0];
	gmt_M_unused (GMT);
	for (k = 1; k < n; k++)
		sum -= array[k];
	return sum;
}

GMT_LOCAL double grdmath_stack_collapse_var (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	double std = 0.0;
	(void)gmt_mean_and_std (GMT, array, n, &std);
	return (std * std);
}

GMT_LOCAL double grdmath_stack_collapse_xor (struct GMT_CTRL *GMT, double *array, uint64_t n) {
	uint64_t k;
	double x = array[0];
	gmt_M_unused (GMT);
	for (k = 1; k < n; k++)
	 	x = (gmt_M_is_dnan (x) && gmt_M_is_dnan (array[k])) ? 0.0 : (gmt_M_is_dnan (array[k]) ? GMT->session.d_NaN : x);
	return x;
}

/* -----------------------------------------------------------------
 *              Definitions of all operator functions
 * -----------------------------------------------------------------*/

GMT_LOCAL int grdmath_collapse_stack (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last, char *OP) {
	/* Collapse stack will apply the given operator to all items on the stack, per node.
	 * E.g., you may have 7 grids on the stack and you want to return the mean value per node
	 * for all 7 grids, to be replaced by a single grid with those means.  You would do
	 *	gmt grdmath *.grd -S MEAN = means.grd
	 * where the -S option turns on the collapsible stack operators; it turns itself off
	 * once the stack has been processed to yield a single new grid on the stack.
	 */

	uint64_t node, s;
	double *array = NULL;
	double (*func) (struct GMT_CTRL *, double *, uint64_t);	/* Pointer to function returning a double */

	/* First ensure we have a reducing operator */

	if (!strcmp (OP, "ADD"))
		func = &grdmath_stack_collapse_add;
	else if (!strcmp (OP, "AND"))
		func = &grdmath_stack_collapse_and;
	else if (!strcmp (OP, "LMSSCL"))
		func = &grdmath_stack_collapse_lmsscl;
	else if (!strcmp (OP, "MAD"))
		func = &grdmath_stack_collapse_mad;
	else if (!strcmp (OP, "MAX"))
		func = &grdmath_stack_collapse_max;
	else if (!strcmp (OP, "MEAN"))
		func = &grdmath_stack_collapse_mean;
	else if (!strcmp (OP, "MEDIAN"))
		func = &grdmath_stack_collapse_median;
	else if (!strcmp (OP, "MIN"))
		func = &grdmath_stack_collapse_min;
	else if (!strcmp (OP, "MODE"))
		func = &grdmath_stack_collapse_mode;
	else if (!strcmp (OP, "MUL"))
		func = &grdmath_stack_collapse_mul;
	else if (!strcmp (OP, "OR"))
		func = &grdmath_stack_collapse_or;
	else if (!strcmp (OP, "STD"))
		func = &grdmath_stack_collapse_std;
	else if (!strcmp (OP, "SUB"))
		func = &grdmath_stack_collapse_sub;
	else if (!strcmp (OP, "RMS"))
		func = &grdmath_stack_collapse_rms;
	else if (!strcmp (OP, "VAR"))
		func = &grdmath_stack_collapse_var;
	else if (!strcmp (OP, "XOR"))
		func = &grdmath_stack_collapse_xor;
	else {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unrecognized stack reduction operator %s - ignored\n", OP);
		return 1;
	}

	array = gmt_M_memory (GMT, NULL, last, double);
	for (node = 0; node < info->size; node++) {	/* For all nodes */
		for (s = 0; s < last; s++)	/* For all items on stack at this node */
			array[s] = (stack[s]->constant) ? stack[last]->factor : stack[s]->G->data[node];
		/* Now do reducing operation on this stack array */
		stack[0]->G->data[node] = func (GMT, array, last);
	}
	gmt_M_free (GMT, array);
	return 0;
}

/* Note: The OPERATOR: **** lines are used to extract syntax for documentation */

GMT_LOCAL void grdmath_ABS (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ABS 1 1 abs (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "ABS: Operand == 0!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)fabs (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : fabsf (stack[last]->G->data[node]);
	gmt_grd_pad_zero (GMT, stack[last]->G);	/* Reset the boundary pad, if needed */
}

GMT_LOCAL void grdmath_ACOS (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACOS 1 1 acos (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ACOS!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)d_acos (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : d_acosf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ACOSD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACOSD 1 1 acosd (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ACOSD!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)(R2D * d_acos (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : R2D * d_acosf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ACOSH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACOSH 1 1 acosh (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant && fabs (stack[last]->factor) < 1.0)
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand < 1 for ACOSH!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)acosh (stack[last]->factor);
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : acoshf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ACOT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACOT 1 1 acot (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0)
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ACOT!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)atan (1.0 / stack[last]->factor);
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : atanf (1.0f / stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ACOTD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACOTD 1 1 acotd (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0)
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ACOTD!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)(R2D * atan (1.0 / stack[last]->factor));
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : R2D * atanf (1.0f / stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ACOTH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACOTH 1 1 acoth (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant && fabs (stack[last]->factor) <= 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| <= 1 for ACOTH!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)atanh (1.0/stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : atanhf (1.0f/stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ACSC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACSC 1 1 acsc (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0)
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ACSC!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)d_asin (1.0 / stack[last]->factor);
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : d_asinf (1.0f / stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ACSCD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACSCD 1 1 acscd (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0)
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ACSCD!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)(R2D * d_asin (1.0 / stack[last]->factor));
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : R2D * d_asinf (1.0f / stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ACSCH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ACSCH 1 1 acsch (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)asinh (1.0/stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : asinhf (1.0f/stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ADD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ADD 2 1 A + B.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	double a, b;
	gmt_M_unused(GMT);

	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)(a + b);
	}
}

GMT_LOCAL void grdmath_AND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: AND 2 1 B if A == NaN, else A.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	double a, b;
	gmt_M_unused(GMT);

	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)((gmt_M_is_dnan (a)) ? b : a);
	}
}

GMT_LOCAL void grdmath_ARC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ARC 2 1 arc(A, B) = pi - |pi - |a-b|| for A, B in radians.  */
	/*
	given phase values a and b each in radians on [-pi,pi]
	return arc(a,b) on [0 pi]
	see eq 2.3.13 page 19, Mardia and Jupp [2000]
	c = pi - abs(pi-abs(a-b))
	Kurt Feigl 2014-AUG-10
	*/
{
	uint64_t node;
	unsigned int prev = last - 1;
	double a, b;
	gmt_M_unused(GMT);

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	for (node = 0; node < info->size; node++) {

		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];

		/* Both arguments must be in range [-pi,pi] radians */
		if ((a >= -M_PI) && (a <= M_PI) && (b >= -M_PI) && (b <= M_PI))
			stack[prev]->G->data[node] = (gmt_grdfloat)(M_PI-fabs(M_PI-fabs(a-b)));
		else
			stack[prev]->G->data[node] = GMT->session.f_NaN;  /* NaN output */
	}
}

GMT_LOCAL void grdmath_AREA (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: AREA 0 1 Area of each gridnode cell (spherical calculation in km^2 if geographic).  */
	gmt_M_unused(info);
	gmt_get_cellarea (GMT, stack[last]->G);
}

GMT_LOCAL void grdmath_ASEC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: ASEC 1 1 asec (A).  */
	uint64_t node;
	gmt_grdfloat a = 0.0;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ASEC!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)d_acos (1.0 / stack[last]->factor);
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : d_acosf (1.0f / stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ASECD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: ASECD 1 1 asecd (A).  */
	uint64_t node;
	gmt_grdfloat a = 0.0;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ASECD!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)(R2D * d_acos (1.0 / stack[last]->factor));
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : R2D * d_acosf (1.0f / stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ASECH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: ASECH 1 1 asech (A).  */
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0)
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand > 1 for ASECH!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)acosh (1.0/stack[last]->factor);
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : acoshf (1.0f/stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ASIN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ASIN 1 1 asin (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ASIN!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)d_asin (stack[last]->factor);
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : d_asinf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ASIND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ASIND 1 1 asind (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant && fabs (stack[last]->factor) > 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| > 1 for ASIND!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)(R2D * d_asin (stack[last]->factor));
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : R2D * d_asinf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ASINH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ASINH 1 1 asinh (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)asinh (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : asinhf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ATAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ATAN 1 1 atan (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant) a = (gmt_grdfloat)atan (stack[last]->factor);
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : atanf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ATAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ATAND 1 1 atand (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant) a = (gmt_grdfloat)(R2D * atan (stack[last]->factor));
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : R2D * atanf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ATAN2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ATAN2 2 1 atan2 (A, B).  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	double a, b;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for ATAN2!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for ATAN2!\n");
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)d_atan2 (a, b);
	}
}

GMT_LOCAL void grdmath_ATAN2D (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ATAN2D 2 1 atan2d (A, B).  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	double a, b;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for ATAN2D!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for ATAN2D!\n");
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)R2D * d_atan2 (a, b);
	}
}

GMT_LOCAL void grdmath_ATANH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ATANH 1 1 atanh (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant && fabs (stack[last]->factor) >= 1.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "|Operand| >= 1 for ATANH!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)atanh (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : atanhf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_BCDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BCDF 3 1 Binomial cumulative distribution function for p = A, n = B and x = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double p, x, n;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && stack[prev2]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument p to BPDF must be in 0 <= p <= 1!\n");
		error++;
	}
	if (stack[prev1]->constant && stack[prev1]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument n to BPDF must be a positive integer (n >= 0)!\n");
		error++;
	}
	if (stack[last]->constant  && stack[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument x to BPDF must be a positive integer (x >= 0)!\n");
		error++;
	}
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* BPDF is undefined or constant arguments */
		gmt_grdfloat value;
		p = stack[prev2]->factor;
		n = stack[prev1]->factor;	x = stack[last]->factor;
		value = (error) ? GMT->session.f_NaN : (gmt_grdfloat)gmt_binom_cdf (GMT, lrint(x), lrint(n), p);
		gmt_M_grd_loop (GMT, info->G, row, col, node) stack[prev2]->G->data[node] = value;
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		p = (stack[prev2]->constant) ? stack[prev2]->factor : (double)stack[prev2]->G->data[node];
		n = (stack[prev1]->constant) ? stack[prev1]->factor : (double)stack[prev1]->G->data[node];
		x = (stack[last]->constant)  ? stack[last]->factor  : (double)stack[last]->G->data[node];
		stack[prev2]->G->data[node] = (gmt_grdfloat)(gmt_binom_cdf (GMT, lrint(x), lrint(n), p));
	}
}

GMT_LOCAL void grdmath_BPDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BPDF 3 1 Binomial probability density function for p = A, n = B and x = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double p, q, x, n;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && stack[prev2]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument p to BPDF must be in 0 <= p <= 1!\n");
		error++;
	}
	if (stack[prev1]->constant && stack[prev1]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument n to BPDF must be a positive integer (n >= 0)!\n");
		error++;
	}
	if (stack[last]->constant  && stack[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument x to BPDF must be a positive integer (x >= 0)!\n");
		error++;
	}
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* BPDF is undefined or constant arguments */
		gmt_grdfloat value;
		p = stack[prev2]->factor;	q = 1.0 - p;
		n = stack[prev1]->factor;	x = stack[last]->factor;
		value = (error) ? GMT->session.f_NaN : (gmt_grdfloat)(gmt_combination (GMT, irint (n), irint (x)) * pow (p, x) * pow (q, n-x));
		gmt_M_grd_loop (GMT, info->G, row, col, node) stack[prev2]->G->data[node] = value;
		return;
	}
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			p = (stack[prev2]->constant) ? stack[prev2]->factor : (double)stack[prev2]->G->data[node];
			n = (stack[prev1]->constant) ? stack[prev1]->factor : (double)stack[prev1]->G->data[node];
			x = (stack[last]->constant)  ? stack[last]->factor  : (double)stack[last]->G->data[node];
			q = 1.0 - p;
			stack[prev2]->G->data[node] = (gmt_grdfloat)(gmt_combination (GMT, irint (n), irint (x)) * pow (p, x) * pow (q, n-x));
		}
	}
}

GMT_LOCAL void grdmath_BEI (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BEI 1 1 Kelvin function bei (A).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = gmt_bei (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : gmt_bei (GMT, fabs((double)stack[last]->G->data[node])));
}

GMT_LOCAL void grdmath_BER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BER 1 1 Kelvin function  ber (A).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = gmt_ber (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : gmt_ber (GMT, fabs ((double)stack[last]->G->data[node])));
}

GMT_LOCAL void grdmath_BITAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITAND 2 1 A & B (bitwise AND operator).  */
{
	uint64_t node, n_warn = 0;
	gmt_grdfloat af = 0.0f, bf = 0.0;
	unsigned int prev, a = 0, b = 0, result, result_trunc;

	prev = last - 1;
	if (stack[prev]->constant) af = (gmt_grdfloat)stack[prev]->factor;
	if (stack[last]->constant) bf = (gmt_grdfloat)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) af = stack[prev]->G->data[node];
		if (!stack[last]->constant) bf = stack[last]->G->data[node];
		if (gmt_M_is_fnan (af) || gmt_M_is_fnan (bf))	/* Any NaN in bitwise operations results in NaN output */
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;	b = (unsigned int)bf;
			result = a & b;
			result_trunc = (result & FLOAT_BIT_MASK);
			if (result != result_trunc) n_warn++;
			stack[prev]->G->data[node] = (gmt_grdfloat)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITAND resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");

}

GMT_LOCAL void grdmath_BITLEFT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITLEFT 2 1 A << B (bitwise left-shift operator).  */
{
	uint64_t node, n_warn = 0;
	unsigned int prev, a = 0, b = 0, result, result_trunc;
	int b_signed;
	bool first = true;
	gmt_grdfloat af = 0.0f, bf = 0.0;

	prev = last - 1;
	if (stack[prev]->constant) af = (gmt_grdfloat)stack[prev]->factor;
	if (stack[last]->constant) bf = (gmt_grdfloat)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) af = stack[prev]->G->data[node];
		if (!stack[last]->constant) bf = stack[last]->G->data[node];
		if (gmt_M_is_fnan (af) || gmt_M_is_fnan (bf))	/* Any NaN in bitwise operations results in NaN output */
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;	b_signed = (int)bf;
			if (b_signed < 0) {	/* Bad bitshift */
				if (first) GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bit shift must be >= 0; other values yield NaN\n");
				stack[prev]->G->data[node] = GMT->session.f_NaN;
				first = false;
			}
			else {
				b = (unsigned int)b_signed;
				result = a << b;
				result_trunc = (result & FLOAT_BIT_MASK);
				if (result != result_trunc) n_warn++;
				stack[prev]->G->data[node] = (gmt_grdfloat) result_trunc;
			}
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITLEFT resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");
}

GMT_LOCAL void grdmath_BITNOT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITNOT 1 1  ~A (bitwise NOT operator, i.e., return two's complement).  */
{
	uint64_t node, n_warn = 0;
	unsigned int a = 0, result, result_trunc;
	gmt_grdfloat af = 0.0;

	if (stack[last]->constant) af = (gmt_grdfloat)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[last]->constant) af = stack[last]->G->data[node];
		if (gmt_M_is_fnan (af))	/* Any NaN in bitwise operations results in NaN output */
			stack[last]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;
			result = ~a;
			result_trunc = (result & FLOAT_BIT_MASK);
			if (result != result_trunc) n_warn++;
			stack[last]->G->data[node] = (gmt_grdfloat)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITNOT resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");
}

GMT_LOCAL void grdmath_BITOR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITOR 2 1 A | B (bitwise OR operator).  */
{
	uint64_t node, n_warn = 0;
	unsigned int prev, a = 0, b = 0, result, result_trunc;
	gmt_grdfloat af = 0.0f, bf = 0.0;

	prev = last - 1;
	if (stack[prev]->constant) af = (gmt_grdfloat)stack[prev]->factor;
	if (stack[last]->constant) bf = (gmt_grdfloat)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) af = stack[prev]->G->data[node];
		if (!stack[last]->constant) bf = stack[last]->G->data[node];
		if (gmt_M_is_fnan (af) || gmt_M_is_fnan (bf))	/* Any NaN in bitwise operations results in NaN output */
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;	b = (unsigned int)bf;
			result = a | b;
			result_trunc = (result & FLOAT_BIT_MASK);
			if (result != result_trunc) n_warn++;
			stack[prev]->G->data[node] = (gmt_grdfloat)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITOR resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");
}

GMT_LOCAL void grdmath_BITRIGHT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITRIGHT 2 1 A >> B (bitwise right-shift operator).  */
{
	uint64_t node, n_warn = 0;
	unsigned int prev, a = 0, b = 0, result, result_trunc;
	int b_signed;
	bool first = true;
	gmt_grdfloat af = 0.0f, bf = 0.0;

	prev = last - 1;
	if (stack[prev]->constant) af = (gmt_grdfloat)stack[prev]->factor;
	if (stack[last]->constant) bf = (gmt_grdfloat)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) af = stack[prev]->G->data[node];
		if (!stack[last]->constant) bf = stack[last]->G->data[node];
		if (gmt_M_is_fnan (af) || gmt_M_is_fnan (bf))	/* Any NaN in bitwise operations results in NaN output */
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;	b_signed = (int)bf;
			if (b_signed < 0) {	/* Bad bitshift */
				if (first) GMT_Report (GMT->parent, GMT_MSG_ERROR, "Bit shift must be >= 0; other values yield NaN\n");
				stack[prev]->G->data[node] = GMT->session.f_NaN;
				first = false;
			}
			else {
				b = (unsigned int)b_signed;
				result = a >> b;
				result_trunc = (result & FLOAT_BIT_MASK);
				if (result != result_trunc) n_warn++;
				stack[prev]->G->data[node] = (gmt_grdfloat)result_trunc;
			}
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITRIGHT resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");
}

GMT_LOCAL void grdmath_BITTEST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITTEST 2 1 1 if bit B of A is set, else 0 (bitwise TEST operator).  */
{
	uint64_t node, n_warn = 0;
	unsigned int prev, a = 0, b = 0, result, result_trunc;
	int b_signed;
	bool first = true;
	gmt_grdfloat af = 0.0f, bf = 0.0;

	prev = last - 1;
	if (stack[prev]->constant) af = (gmt_grdfloat)stack[prev]->factor;
	if (stack[last]->constant) bf = (gmt_grdfloat)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) af = stack[prev]->G->data[node];
		if (!stack[last]->constant) bf = stack[last]->G->data[node];
		if (gmt_M_is_fnan (af) || gmt_M_is_fnan (bf))	/* Any NaN in bitwise operations results in NaN output */
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;	b_signed = (int)bf;
			if (b_signed < 0) {	/* Bad bit */
				if (first) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Bit position range for BITTEST is 0-31 ; other values yield NaN\n");
				stack[prev]->G->data[node] = GMT->session.f_NaN;
				first = false;
			}
			else {
				b = (unsigned int)b_signed;
				b = 1U << b;
				result = a & b;
				result_trunc = (result & FLOAT_BIT_MASK);
				if (result != result_trunc) n_warn++;
				stack[prev]->G->data[node] = (result_trunc) ? 1.0f : 0.0;
			}
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITTEST resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");
}

GMT_LOCAL void grdmath_BITXOR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BITXOR 2 1 A ^ B (bitwise XOR operator).  */
{
	uint64_t node, n_warn = 0;
	unsigned int prev, a = 0, b = 0, result, result_trunc;
	gmt_grdfloat af = 0.0f, bf = 0.0;

	prev = last - 1;
	if (stack[prev]->constant) af = (gmt_grdfloat)stack[prev]->factor;
	if (stack[last]->constant) bf = (gmt_grdfloat)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) af = stack[prev]->G->data[node];
		if (!stack[last]->constant) bf = stack[last]->G->data[node];
		if (gmt_M_is_fnan (af) || gmt_M_is_fnan (bf))	/* Any NaN in bitwise operations results in NaN output */
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (unsigned int)af;	b = (unsigned int)bf;
			result = a ^ b;
			result_trunc = (result & FLOAT_BIT_MASK);
			if (result != result_trunc) n_warn++;
			stack[prev]->G->data[node] = (gmt_grdfloat)result_trunc;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BITXOR resulted in %" PRIu64 " values truncated to fit in the 24 available bits\n");
}

GMT_LOCAL void grdmath_BLEND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: BLEND 3 1 Blend A and B using weights in C (0-1 range) as A*C+B*(1-C).  */
{
	uint64_t node, n_warn = 0;
	openmp_int row, col;
	unsigned int prev1, prev2;
	double z1, z2, w;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && stack[prev2]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for BLEND!\n");
	if (stack[prev1]->constant && stack[prev1]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for BLEND!\n");
	if (stack[last]->constant  && stack[last]->factor  == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand three == 0 for BLEND!\n");
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			z1 = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
			z2 = (stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node];
			w  = (stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node];
			stack[prev2]->G->data[node] = (gmt_grdfloat)(w * (z1 - z2) + z2);	/* This is same as w*z1 + (1-w)*z2 but one less multiply */
			if (w < 0.0 || w > 1.0) n_warn++;
		}
	}
	if (n_warn) GMT_Report (GMT->parent, GMT_MSG_WARNING, "BLEND encountered %" PRIu64 " weights that were outside the 0-1 range\n", n_warn);
}

GMT_LOCAL void grdmath_CAZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CAZ 2 1 Cartesian azimuth from grid nodes to stack x,y.  */
{
	openmp_int row, col;
	uint64_t node;
	unsigned int prev = last - 1;
	double x, y, az;
	gmt_M_unused(GMT);

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	grdmath_grd_padloop (GMT, info->G, row, col, node) {
		x = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		y = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		az = (90.0 - atan2d (y - info->d_grd_y[row], x - info->d_grd_x[col]));
		while (az < -180.0) az += 360.0;
		while (az > +180.0) az -= 360.0;
		stack[prev]->G->data[node] = (gmt_grdfloat)az;
	}
}

GMT_LOCAL void grdmath_CBAZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CBAZ 2 1 Cartesian back-azimuth from grid nodes to stack x,y.  */
{
	openmp_int row, col;
	uint64_t node;
	unsigned int prev = last - 1;
	double x, y, az;
	gmt_M_unused(GMT);

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	grdmath_grd_padloop (GMT, info->G, row, col, node) {
		x = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		y = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		az = (270.0 - atan2d (y - info->d_grd_y[row], x - info->d_grd_x[col]));
		while (az < -180.0) az += 360.0;
		while (az > +180.0) az -= 360.0;
		stack[prev]->G->data[node] = (gmt_grdfloat)az;
	}
}

GMT_LOCAL void grdmath_CDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CDIST 2 1 Cartesian distance between grid nodes and stack x,y.  */
{
	openmp_int row, col;
	uint64_t node;
	unsigned int prev = last - 1;
	double a, b;

	if (gmt_M_is_geographic (GMT, GMT_IN)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid must be Cartesian; see SDIST for geographic data.\n");
		return;
	}
	grdmath_grd_padloop (GMT, info->G, row, col, node) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)hypot (a - info->d_grd_x[col], b - info->d_grd_y[row]);
	}
}

GMT_LOCAL void grdmath_CDIST2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CDIST2 2 1 As CDIST but only to nodes that are != 0.  */
{
	openmp_int row, col;
	uint64_t node;
	unsigned int prev = last - 1;
	double a, b;

	if (gmt_M_is_geographic (GMT, GMT_IN)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid must be Cartesian; see SDIST2 for geographic data.\n");
		return;
	}
	grdmath_grd_padloop (GMT, info->G, row, col, node) {
		if (stack[prev]->G->data[node] == 0.0)
			stack[prev]->G->data[node] = GMT->session.f_NaN;
		else {
			a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
			stack[prev]->G->data[node] = (gmt_grdfloat)hypot (a - info->d_grd_x[col], b - info->d_grd_y[row]);
		}
	}
}

GMT_LOCAL void grdmath_CEIL (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CEIL 1 1 ceil (A) (smallest integer >= A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)ceil (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : ceilf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_CHI2CRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CHI2CRIT 2 1 Chi-squared distribution critical value for alpha = A and nu = B.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev = last - 1;
	double a, b;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for CHI2CRIT!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for CHI2CRIT!\n");
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)gmt_chi2crit (GMT, a, b);
	}
}

GMT_LOCAL void grdmath_CHI2CDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CHI2CDF 2 1 Chi-squared cumulative distribution function for chi2 = A and nu = B.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev = last - 1;
	double a, b, q;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for CHI2CDF!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for CHI2CDF!\n");
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
			gmt_chi2 (GMT, a, b, &q);
			stack[prev]->G->data[node] = (gmt_grdfloat)(1.0 - q);
		}
	}
}

GMT_LOCAL void grdmath_CHI2PDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CHI2PDF 2 1 Chi-squared probability density function for chi = A and nu = B.  */
{
	uint64_t node, nu;
	unsigned int prev = last - 1;
	openmp_int row, col;
	double c;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for CHI2PDF!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for CHI2PDF!\n");
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			c = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			nu = lrint ((stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node]);
			stack[prev]->G->data[node] = (gmt_grdfloat)gmt_chi2_pdf (GMT, c, nu);
		}
	}
}

GMT_LOCAL void grdmath_COMB (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: COMB 2 1 Combinations n_C_r, with n = A and r = B.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev = last - 1, error = 0;
	double a, b;

	if (stack[prev]->constant && stack[prev]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument n to COMB must be a positive integer (n >= 0)!\n");
		error++;
	}
	if (stack[last]->constant && stack[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to COMB must be a positive integer (r >= 0)!\n");
		error++;
	}
	if (error || (stack[prev]->constant && stack[last]->constant)) {	/* COMBO is undefined or we have a constant */
		gmt_grdfloat value = (error) ? GMT->session.f_NaN : (gmt_grdfloat)gmt_combination (GMT, irint(stack[prev]->factor), irint(stack[last]->factor));
		gmt_M_grd_loop (GMT, info->G, row, col, node) stack[prev]->G->data[node] = value;
		return;
	}
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
			stack[prev]->G->data[node] = (gmt_grdfloat)gmt_combination (GMT, irint(a), irint(b));
		}
	}
}

GMT_LOCAL void grdmath_CORRCOEFF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CORRCOEFF 2 1 Correlation coefficient r(A, B).  */
{
	uint64_t node;
	unsigned int prev = last - 1, pad[4];
	double coeff;

	if (stack[prev]->constant || stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Constant operands for CORRCOEFF yields NaNs\n");
		for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = GMT->session.f_NaN;
		return;
	}
	gmt_M_memcpy (pad, stack[last]->G->header->pad, 4, unsigned int);	/* Save original pad */
	gmt_grd_pad_off (GMT, stack[prev]->G);				/* Undo pad if one existed so we can sort */
	gmt_grd_pad_off (GMT, stack[last]->G);				/* Undo pad if one existed so we can sort */
	coeff = gmt_corrcoeff_f (GMT, stack[prev]->G->data, stack[last]->G->data, info->nm, 0);
	gmt_grd_pad_on (GMT, stack[prev]->G, pad);		/* Reinstate the original pad */
	gmt_grd_pad_on (GMT, stack[last]->G, pad);		/* Reinstate the original pad */
	for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = (gmt_grdfloat)coeff;
}

GMT_LOCAL void grdmath_COS (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: COS 1 1 cos (A) (A in radians).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)cos (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : cosf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_COSD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: COSD 1 1 cos (A) (A in degrees).  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = cosd (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : cosd (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_COSH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: COSH 1 1 cosh (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)cosh (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : coshf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_COT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: COT 1 1 cot (A) (A in radians).  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = 1.0 / tan (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : (1.0 / tan (stack[last]->G->data[node])));
}

GMT_LOCAL void grdmath_COTD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: COTD 1 1 cot (A) (A in degrees).  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = 1.0 / tand (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : 1.0 / tand (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_COTH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: COTH 1 1 coth (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)(1.0/tanh (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : 1.0f / tanhf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_PCDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PCDF 2 1 Poisson cumulative distribution function x = A and lambda = B.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	openmp_int row, col;
	double a, b, prob;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for PCDF!\n");
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
			gmt_poisson_cdf (GMT, a, b, &prob);
			stack[prev]->G->data[node] = (gmt_grdfloat)prob;
		}
	}
}

GMT_LOCAL void grdmath_PPDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PPDF 2 1 Poisson probability density function for x = A and lambda = B.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	openmp_int row, col;
	double a, b;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for PPDF!\n");
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
			stack[prev]->G->data[node] = (gmt_grdfloat)gmt_poissonpdf (GMT, a, b);
		}
	}
}

GMT_LOCAL void grdmath_CSC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CSC 1 1 csc (A) (A in radians).  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = 1.0 / sin (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : 1.0 / sinf (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_CSCD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CSCD 1 1 csc (A) (A in degrees).  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = 1.0 / sind (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : 1.0 / sind (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_CSCH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CSCH 1 1 csch (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)(1.0 / sinh (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : 1.0f / sinhf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_CUMSUM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CUMSUM 2 1 Cumulative sum across each row.  */
{
	bool add = false;
	uint64_t node, previous, mx, shift;
	unsigned int prev = last - 1;
	openmp_int row, col;
	int code;

	if (!stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "CUMSUM: Argument B must be a constant\n");
		return;
	}
	code = irint (stack[last]->factor);
	if (!gmt_M_is_zero (fabs (stack[last]->factor - code))) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "CUMSUM: Argument B must be an integer\n");
		return;
	}
	if (code < -4 || code > 4 || code == 0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "CUMSUM: Argument B must be either +/- 1-4\n");
		return;
	}

	mx = info->G->header->mx;
	switch (code) {
		case +3:	/* Sum rows in positive x-direction, start next row with previous sum */
			add = true;	/* Fall through on purpose */
			shift = info->G->header->pad[XLO] + info->G->header->pad[XHI] + 2;
		case +1:	/* Sum rows in positive x-direction */
			gmt_M_row_loop (GMT, info->G, row) {	/* Process sums by row in positive x-direction*/
				node = gmt_M_ijp (info->G->header, row, 1);	/* Node of 2nd col in this row */
				if (add && row) stack[prev]->G->data[node-1] += stack[prev]->G->data[node-shift];
				for (col = 1; col < (openmp_int)info->G->header->n_columns; col++, node++)
					stack[prev]->G->data[node] += stack[prev]->G->data[node-1];
			}
			break;
		case -3:	/* Sum rows in negative x-direction, start next row with previous sum */
			add = true;	/* Fall through on purpose */
			shift = 2 * mx - info->G->header->pad[XLO] - info->G->header->pad[XHI] - 2;
		case -1:	/* Sum rows in negative x-direction */
			gmt_M_row_loop (GMT, info->G, row) {	/* Process sums by row in negative x-direction*/
				node = gmt_M_ijp (info->G->header, row, info->G->header->n_columns-2);	/* Node of 2nd col from the right in this row */
				if (add && row) stack[prev]->G->data[node+1] += stack[prev]->G->data[node-shift];
				for (col = 1; col < (openmp_int)info->G->header->n_columns; col++, node--)
					stack[prev]->G->data[node] += stack[prev]->G->data[node+1];
			}
			break;
		case +4:	/* Sum columns in positive y-direction, start new column with previous sum */
			add = true;	/* Fall through on purpose */
			shift = (info->G->header->n_rows - 1) * mx + 1;
		case +2:	/* Sum columns in positive y-direction */
			gmt_M_col_loop (GMT, info->G, 0, col, node) {	/* Process sums by column in positive y-direction */
				previous = gmt_M_ijp (info->G->header, info->G->header->n_rows-1, col);	/* Last row for this column */
				if (add && col) stack[prev]->G->data[previous] += stack[prev]->G->data[previous-shift];
				for (row = 1; row < (openmp_int)info->G->header->n_rows; row++) {
					node = previous - mx;	/* current node in this column */
					stack[prev]->G->data[node] += stack[prev]->G->data[previous];
					previous = node;
				}
			}
			break;
		case -4:	/* Sum columns in negative y-direction, start new column with previous sum */
			add = true;	/* Fall through on purpose */
			shift = (info->G->header->n_rows - 1) * mx - 1;
		case -2:	/* Sum columns in negative y-direction */
			gmt_M_col_loop (GMT, info->G, 0, col, node) {	/* Process sums by column in negative y-direction */
				previous = gmt_M_ijp (info->G->header, 0, col);	/* First row for this column */
				if (add && col) stack[prev]->G->data[previous] += stack[prev]->G->data[previous+shift];
				for (row = 1; row < (openmp_int)info->G->header->n_rows; row++) {
					node = previous + mx;	/* current node in this column */
					stack[prev]->G->data[node] += stack[prev]->G->data[previous];
					previous = node;
				}
			}
			break;
	}
}

GMT_LOCAL void grdmath_CURV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: CURV 1 1 Curvature of A (Laplacian).  */
{
	uint64_t node;
	openmp_int row, col, mx;
	double cy, *cx = NULL;
	gmt_grdfloat *z = NULL;

	/* Curvature (Laplacian). */

	if (gmt_M_is_geographic (GMT, GMT_IN)) GMT_Report (GMT->parent, GMT_MSG_WARNING, "geographic grid given to a Cartesian operator [CURV]!\n");
	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand to CURV is constant!\n");
		gmt_M_memset (stack[last]->G->data, info->size, gmt_grdfloat);
		return;
	}

	/* If grid does not have BC rows/cols assigned we apply reasonable conditions:
	 * If -fg we assume geographic grid and use geographic BCs, else we use natural BCs. If the grid
	 * as a BC == GMT_BC_IS_DATA then the pad already constrains observations. */

	gmt_BC_init (GMT, stack[last]->G->header);	/* Initialize grid interpolation and boundary condition parameters */
	gmt_grd_BC_set (GMT, stack[last]->G, GMT_IN);	/* Set boundary conditions */

	/* Now, stack[last]->G->data has boundary rows/cols all set according to the boundary conditions (or actual data).
	 * We can then operate on the interior of the grid and temporarily assign values to the z grid */

	z = gmt_M_memory (GMT, NULL, info->size, gmt_grdfloat);
	cx = gmt_M_memory (GMT, NULL, info->G->header->n_rows, double);
	gmt_M_row_loop (GMT, info->G, row) cx[row] = 1.0 / (info->dx[row] * info->dx[row]);

	mx = info->G->header->mx;
	cy = 1.0 / (info->dy * info->dy);

	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
		z[node] = (gmt_grdfloat)(cx[row] * (stack[last]->G->data[node+1] - 2.0 * stack[last]->G->data[node] + stack[last]->G->data[node-1]) + \
			cy * (stack[last]->G->data[node+mx] - 2.0 * stack[last]->G->data[node] + stack[last]->G->data[node-mx]));
		}
	}

	gmt_M_memcpy (stack[last]->G->data, z, info->size, gmt_grdfloat);
	gmt_M_free (GMT, z);
	gmt_M_free (GMT, cx);
}

GMT_LOCAL void grdmath_D2DX2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: D2DX2 1 1 d^2(A)/dx^2 2nd Central derivative.  */
{
	uint64_t node, ij;
	openmp_int row, col;
	double c, left, next_left;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (stack[last]->G->header);

	/* Central 2nd difference in x */

	if (gmt_M_is_geographic (GMT, GMT_IN)) GMT_Report (GMT->parent, GMT_MSG_WARNING, "geographic grid given to a Cartesian operator [D2DX2]!\n");
	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand to D2DX2 is constant!\n");
		gmt_M_memset (stack[last]->G->data, info->size, gmt_grdfloat);
		return;
	}

	gmt_M_row_loop (GMT, info->G, row) {	/* Process d2/dx2 row by row since dx may change with row */
		c = 1.0 / (info->dx[row] * info->dx[row]);
		/* Unless pad has real data we assign outside col values via natural BCs */
		ij = gmt_M_ijp (info->G->header, row, 0);	/* First col */
		if (HH->BC[XLO] != GMT_BC_IS_DATA)
			stack[last]->G->data[ij-1] = (gmt_grdfloat)(2.0 * stack[last]->G->data[ij] - stack[last]->G->data[ij+1]);	/* Set left node via BC curv = 0 */
		next_left = stack[last]->G->data[ij-1];
		ij = gmt_M_ijp (info->G->header, row, info->G->header->n_columns-1);	/* Last col */
		if (HH->BC[XHI] != GMT_BC_IS_DATA)
			stack[last]->G->data[ij+1] = (gmt_grdfloat)(2.0 * stack[last]->G->data[ij] - stack[last]->G->data[ij-1]);	/* Set right node via BC curv = 0 */
		gmt_M_col_loop (GMT, info->G, row, col, node) {	/* Loop over cols; always save the next left before we update the array at that col */
			left = next_left;
			next_left = stack[last]->G->data[node];
			stack[last]->G->data[node] = (gmt_grdfloat)(c * (stack[last]->G->data[node+1] - 2.0 * stack[last]->G->data[node] + left));
		}
	}
	gmt_grd_pad_zero (GMT, stack[last]->G);	/* Reset the boundary pad */
}

GMT_LOCAL void grdmath_D2DY2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: D2DY2 1 1 d^2(A)/dy^2 2nd Central derivative.  */
{
	uint64_t node, ij;
	openmp_int row, col, mx;
	double c, bottom, next_bottom;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (stack[last]->G->header);

	/* Central 2nd difference in y */

	if (gmt_M_is_geographic (GMT, GMT_IN)) GMT_Report (GMT->parent, GMT_MSG_WARNING, "geographic grid given to a Cartesian operator [D2DY2]!\n");
	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand to D2DY2 is constant!\n");
		gmt_M_memset (stack[last]->G->data, info->size, gmt_grdfloat);
		return;
	}

	c = 1.0 / (info->dy * info->dy);
	mx = info->G->header->mx;
	gmt_M_col_loop (GMT, info->G, 0, col, node) {	/* Process d2/dy2 column by column */
		/* Unless pad has real data we assign outside row values via natural BCs */
		if (HH->BC[YHI] != GMT_BC_IS_DATA)
			stack[last]->G->data[node-mx] = (gmt_grdfloat)(2.0 * stack[last]->G->data[node] - stack[last]->G->data[node+mx]);	/* Set top node via BC curv = 0 */
		next_bottom = stack[last]->G->data[node-mx];
		ij = gmt_M_ijp (info->G->header, info->G->header->n_rows-1, col);	/* Last row for this column */
		if (HH->BC[YLO] != GMT_BC_IS_DATA)
			stack[last]->G->data[ij+mx] = (gmt_grdfloat)(2.0 * stack[last]->G->data[ij] - stack[last]->G->data[ij-mx]);	/* Set bottom node via BC curv = 0 */
		gmt_M_row_loop (GMT, info->G, row) { /* Cannot use node inside here and must get ij separately */
			ij = gmt_M_ijp (info->G->header, row, col);	/* current node in this column */
			bottom = next_bottom;
			next_bottom = stack[last]->G->data[ij];
			stack[last]->G->data[ij] = (gmt_grdfloat)(c * (stack[last]->G->data[ij+mx] - 2.0 * stack[last]->G->data[ij] + bottom));
		}
	}
	gmt_grd_pad_zero (GMT, stack[last]->G);	/* Reset the boundary pad */
}

GMT_LOCAL void grdmath_D2DXY (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: D2DXY 1 1 d^2(A)/dxdy 2nd Central derivative.  */
{
	uint64_t node;
	openmp_int row, col, mx;
	double *cx = NULL, cy;
	gmt_grdfloat *z = NULL;

	/* Cross derivative d2/dxy = d2/dyx  */

	if (gmt_M_is_geographic (GMT, GMT_IN)) GMT_Report (GMT->parent, GMT_MSG_WARNING, "geographic grid given to a Cartesian operator [D2DXY]!\n");
	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand to D2DXY is constant!\n");
		gmt_M_memset (stack[last]->G->data, info->size, gmt_grdfloat);
		return;
	}

	/* If grid does not have BC rows/cols assigned we apply reasonable conditions:
	 * If -fg we assume geographic grid and use geographic BCs, else we use natural BCs. If the grid
	 * as a BC == GMT_BC_IS_DATA then the pad already constrains observations. */

	gmt_BC_init (GMT, stack[last]->G->header);	/* Initialize grid interpolation and boundary condition parameters */
	gmt_grd_BC_set (GMT, stack[last]->G, GMT_IN);	/* Set boundary conditions */

	/* Now, stack[last]->G->data has boundary rows/cols all set according to the boundary conditions (or actual data).
	 * We can then operate on the interior of the grid and temporarily assign values to the z grid */

	z = gmt_M_memory (GMT, NULL, info->size, gmt_grdfloat);
	cx = gmt_M_memory (GMT, NULL, info->G->header->n_rows, double);
	gmt_M_row_loop (GMT, info->G, row) cx[row] = 0.5 / info->dx[row];

	mx = info->G->header->mx;
	cy = 0.5 / info->dy;

	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		z[node] = (gmt_grdfloat)(cx[row] * cy * (stack[last]->G->data[node-mx+1] - stack[last]->G->data[node-mx-1] + \
			stack[last]->G->data[node+mx-1] - stack[last]->G->data[node+mx+1]));
	}

	gmt_M_memcpy (stack[last]->G->data, z, info->size, gmt_grdfloat);
	gmt_M_free (GMT, z);
	gmt_M_free (GMT, cx);
}

GMT_LOCAL void grdmath_D2R (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: D2R 1 1 Converts Degrees to Radians.  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant) a = stack[last]->factor * D2R;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : (stack[last]->G->data[node] * D2R));
}

GMT_LOCAL void grdmath_DAYNIGHT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: DAYNIGHT 3 1 Return 1 where sun at (A, B) shines and 0 elsewhere, with C transition width.  */
	openmp_int row, col;
	uint64_t node, k;
	unsigned int prev1, prev2;
	double x0, y0, iw, d;

	if (gmt_M_is_geographic (GMT, GMT_IN)) {
		if (gmt_init_distaz (GMT, 'd', GMT_GREATCIRCLE, GMT_MAP_DIST) == GMT_NOT_A_VALID_TYPE) return;
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "DAYNIGHT: Grid must be geographic.\n");
		return;
	}
	prev1 = last - 1;	prev2 = last - 2;
	k = gmt_M_ijp (info->G->header, 0, 0);	/* Valid grid node index for NW node */
	if (stack[prev2]->constant)	/* Single longitude */
		x0 = stack[prev2]->factor;
	else {	/* Must check if this is a constant longitude grid or not */
		x0 = stack[prev2]->G->data[k];
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			if (!doubleAlmostEqualZero ((double)stack[prev2]->G->data[node], x0)) goto dn_error_x;
		}
		goto dn_next1;
dn_error_x:
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Operand one (longitude) must be constant for DAYNIGHT!\n");
		return;
	}
dn_next1:
	if (stack[prev1]->constant)	/* Single latitude */
		y0 = stack[prev1]->factor;
	else {	/* Must check if this is a constant latitude grid or not */
		y0 = stack[prev1]->G->data[k];
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			if (!doubleAlmostEqualZero ((double)stack[prev1]->G->data[node], y0)) goto dn_error_y;
		}
		goto dn_next2;
dn_error_y:
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Operand two (latitude) must be constant for DAYNIGHT!\n");
		return;
	}

dn_next2:

	if (stack[last]->constant && gmt_M_is_zero (stack[last]->factor)) {	/* No transition width */
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,d) shared(info,stack,prev2,GMT,x0,y0)
#endif
		for (row = 0; row < (openmp_int)info->G->header->my; row++) {
			node = row * info->G->header->mx;
			for (col = 0; col < (openmp_int)info->G->header->mx; col++, node++) {
				d = gmt_distance (GMT, x0, y0, info->d_grd_x[col], info->d_grd_y[row]);	/* Distance in degrees from (A,B) */
				stack[prev2]->G->data[node] = (d > 90.0) ? 0.0 : 1.0;
			}
		}
	}
	else if (stack[last]->constant && !gmt_M_is_zero (stack[last]->factor)) {	/* Fixed nonzero transition width */
		iw = 1.0 / stack[last]->factor;	/* To avoid division */
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,d) shared(info,stack,prev2,last,GMT,x0,y0,iw)
#endif
		for (row = 0; row < (openmp_int)info->G->header->my; row++) {
			node = row * info->G->header->mx;
			for (col = 0; col < (openmp_int)info->G->header->mx; col++, node++) {
				d = gmt_distance (GMT, x0, y0, info->d_grd_x[col], info->d_grd_y[row]);	/* Distance in degrees from (A,B) */
				stack[prev2]->G->data[node] = (gmt_grdfloat) (0.5 + atan ((90.0 - d) * iw) / M_PI);
			}
		}
	}
	else {	/* Variable width */
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,d,iw) shared(info,stack,prev2,last,GMT,x0,y0)
#endif
		for (row = 0; row < (openmp_int)info->G->header->my; row++) {
			node = row * info->G->header->mx;
			for (col = 0; col < (openmp_int)info->G->header->mx; col++, node++) {
				d = gmt_distance (GMT, x0, y0, info->d_grd_x[col], info->d_grd_y[row]);	/* Distance in degrees from (A,B) */
				iw = 1.0 / (double)stack[last]->G->data[node];	/* Allowed to have variable width */
				stack[prev2]->G->data[node] = (gmt_grdfloat) (0.5 + atan ((90.0 - d) * iw) / M_PI);
			}
		}
	}
}

GMT_LOCAL void grdmath_DDX (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DDX 1 1 d(A)/dx Central 1st derivative.  */
{
	uint64_t node, ij;
	openmp_int row, col;
	double c, left, next_left;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (stack[last]->G->header);

	/* Central 1st difference in x */

	if (gmt_M_is_geographic (GMT, GMT_IN)) GMT_Report (GMT->parent, GMT_MSG_WARNING, "geographic grid given to a Cartesian operator [DDX]!\n");
	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand to DDX is constant!\n");
		gmt_M_memset (stack[last]->G->data, info->size, gmt_grdfloat);
		return;
	}

	gmt_M_row_loop (GMT, info->G, row) {	/* Process d/dx row by row since dx may change with row */
		c = 0.5 / info->dx[row];
		/* Unless pad has real data we assign outside col values via natural BCs */
		ij = gmt_M_ijp (info->G->header, row, 0);	/* First col */
		if (HH->BC[XLO] != GMT_BC_IS_DATA)
			stack[last]->G->data[ij-1] = (gmt_grdfloat)(2.0 * stack[last]->G->data[ij] - stack[last]->G->data[ij+1]);	/* Set left node via BC curv = 0 */
		next_left = stack[last]->G->data[ij-1];
		ij = gmt_M_ijp (info->G->header, row, info->G->header->n_columns-1);	/* Last col */
		if (HH->BC[XHI] != GMT_BC_IS_DATA)
			stack[last]->G->data[ij+1] = (gmt_grdfloat)(2.0 * stack[last]->G->data[ij] - stack[last]->G->data[ij-1]);	/* Set right node via BC curv = 0 */
		gmt_M_col_loop (GMT, info->G, row, col, node) {	/* Loop over cols; always save the next left before we update the array at that col */
			left = next_left;
			next_left = stack[last]->G->data[node];
			stack[last]->G->data[node] = (gmt_grdfloat)(c * (stack[last]->G->data[node+1] - left));
		}
	}
}

GMT_LOCAL void grdmath_DDY (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DDY 1 1 d(A)/dy Central 1st derivative.  */
{
	uint64_t node, ij;
	openmp_int row, col, mx;
	double c, bottom, next_bottom;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (stack[last]->G->header);

	/* Central 1st difference in y */

	if (gmt_M_is_geographic (GMT, GMT_IN)) GMT_Report (GMT->parent, GMT_MSG_WARNING, "geographic grid given to a Cartesian operator [DDY]!\n");
	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand to DDY is constant!\n");
		gmt_M_memset (stack[last]->G->data, info->size, gmt_grdfloat);
		return;
	}

	c = -0.5 / info->dy;	/* Because the loop over j below goes from ymax to ymin we compensate with a minus sign here */
	mx = info->G->header->mx;
	gmt_M_col_loop (GMT, info->G, 0, col, node) {	/* Process d/dy column by column */
		/* Unless pad has real data we assign outside row values via natural BCs */
		if (HH->BC[YHI] != GMT_BC_IS_DATA) 	/* Set top node via BC curv = 0 */
			stack[last]->G->data[node-mx] = (gmt_grdfloat)(2.0 * stack[last]->G->data[node] - stack[last]->G->data[node+mx]);
		next_bottom = stack[last]->G->data[node-mx];
		ij = gmt_M_ijp (info->G->header, info->G->header->n_rows-1, col);	/* Last row for this column */
		if (HH->BC[YLO] != GMT_BC_IS_DATA) 	/* Set bottom node via BC curv = 0 */
			stack[last]->G->data[ij+mx] = (gmt_grdfloat)(2.0 * stack[last]->G->data[ij] - stack[last]->G->data[ij-mx]);
		gmt_M_row_loop (GMT, info->G, row) {
			ij = gmt_M_ijp (info->G->header, row, col);	/* current node in this column */
			bottom = next_bottom;
			next_bottom = stack[last]->G->data[ij];
			stack[last]->G->data[ij] = (gmt_grdfloat)(c * (stack[last]->G->data[ij+mx] - bottom));
		}
	}
	gmt_grd_pad_zero (GMT, stack[last]->G);	/* Reset the boundary pad */
}

GMT_LOCAL void grdmath_DEG2KM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DEG2KM 1 1 Converts Spherical Degrees to Kilometers.  */
{
	uint64_t node;
	double a = 0.0;

	if (gmt_M_is_geographic (GMT, GMT_IN)) {
		if (gmt_M_sph_mode (GMT) == GMT_GEODESIC) GMT_Report (GMT->parent, GMT_MSG_WARNING, "DEG2KM is only exact when PROJ_ELLIPSOID == sphere\n");
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "DEG2KM used with Cartesian data\n");
	if (stack[last]->constant) a = stack[last]->factor * GMT->current.proj.DIST_KM_PR_DEG;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : stack[last]->G->data[node] * GMT->current.proj.DIST_KM_PR_DEG);
}

GMT_LOCAL void grdmath_DENAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DENAN 2 1 Replace NaNs in A with values from B.  */
{	/* Just a more straightforward application of AND */
	grdmath_AND (GMT, info, stack, last);
}

GMT_LOCAL void grdmath_DILOG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DILOG 1 1 dilog (A).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = gmt_dilog (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : gmt_dilog (GMT, stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_DIV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DIV 2 1 A / B.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	double a, b;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Divide by zero gives NaNs\n");
	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "DIV: Operand one == 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)(a / b);
	}
}

GMT_LOCAL void grdmath_dot2d (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
{	/* Get x,y and compute 2-D unit vector then take dot products with vectors represented by grid locations */
	uint64_t node;
	unsigned int prev = last - 1;
	openmp_int row, col;
	bool update = true;
	double X[2], P[2];

	if (stack[prev]->constant && stack[last]->constant) {	/* Can compute the constant 2-D vector once */
		P[GMT_X] = stack[prev]->factor;	P[GMT_Y] = stack[last]->factor;
		gmt_normalize2v (GMT, P);	/* Normalize vector */
		update = false;
	}
	for (row = 0, node = 0; row < (openmp_int)info->G->header->my; row++) {
		for (col = 0; col < (openmp_int)info->G->header->mx; col++, node++) {	/* Visit each node */
			if (update) {	/* Must compute updated vector from grids A and B */
				P[GMT_X] = stack[prev]->G->data[node];	P[GMT_Y] = stack[last]->G->data[node];
				gmt_normalize2v (GMT, P);
			}
			X[GMT_X] = info->d_grd_x[col];	X[GMT_Y] = info->d_grd_y[row];
			gmt_normalize2v (GMT, X);	/* Normalized 2-D unit vector for this node */
			stack[prev]->G->data[node] = (gmt_grdfloat)gmt_dot2v (GMT, P, X);
		}
	}
}

GMT_LOCAL void grdmath_dot3d (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
{	/* Get lon,lat and compute 3-D unit vector then take dot products with vectors represented by grid locations */
	uint64_t node;
	unsigned int prev = last - 1;
	openmp_int row, col;
	bool update = true;
	double X[3], P[3];

	if (stack[prev]->constant && stack[last]->constant) {	/* Can compute the constant 3-D vector once */
		gmt_geo_to_cart (GMT, stack[last]->factor, stack[prev]->factor, P, true);
		update = false;
	}
	for (row = 0, node = 0; row < (openmp_int)info->G->header->my; row++) {
		for (col = 0; col < (openmp_int)info->G->header->mx; col++, node++) {	/* Visit each node */
			if (update)	/* Must compute updated vector from grids A and B */
				gmt_geo_to_cart (GMT, stack[last]->G->data[node], stack[prev]->G->data[node], P, true);
			gmt_geo_to_cart (GMT, info->d_grd_y[row], info->d_grd_x[col], X, true);	/* 3-D unit vector for this node */
			stack[prev]->G->data[node] = (gmt_grdfloat)gmt_dot3v (GMT, P, X);
		}
	}
}

GMT_LOCAL void grdmath_DOT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DOT 2 1 1 Dot product of vector (A,B) with grid nodes.  */
{
	if (gmt_M_is_geographic (GMT, GMT_IN))
		grdmath_dot3d (GMT, info, stack, last);
	else
		grdmath_dot2d (GMT, info, stack, last);
}

GMT_LOCAL void grdmath_DUP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: DUP 1 2 Places duplicate of A on the stack.  */
{
	uint64_t node;
	unsigned int next;
	gmt_M_unused(GMT);

	next = last + 1;
	stack[next]->constant = stack[last]->constant;
	stack[next]->factor = stack[last]->factor;
	if (stack[last]->constant) {	/* Time to fess up */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)stack[last]->factor;
	}

	gmt_M_memcpy (stack[next]->G->data, stack[last]->G->data, info->size, gmt_grdfloat);
}

GMT_LOCAL void grdmath_ECDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ECDF 2 1 Exponential cumulative distribution function for x = A and lambda = B.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev = last - 1;
	double a, b;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for PCDF!\n");
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = 1.0f - (gmt_grdfloat)exp (-b * a);
	}
}

GMT_LOCAL void grdmath_ECRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ECRIT 2 1 Exponential distribution critical value for alpha = A and lambda = B.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev = last - 1;
	double a, b;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for PCDF!\n");
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = -(gmt_grdfloat)(log (1.0 - a)/b);
	}
}

GMT_LOCAL void grdmath_EPDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: EPDF 2 1 Exponential probability density function for x = A and lambda = B.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev = last - 1;
	double a, b;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for PCDF!\n");
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)(b * exp (-b * a));
	}
}

GMT_LOCAL void grdmath_ERF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ERF 1 1 Error function erf (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)erf (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : erff (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ERFC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ERFC 1 1 Complementary Error function erfc (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)erfc (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : erfcf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_EQ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: EQ 2 1 1 if A == B, else 0.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	gmt_grdfloat a, b;

	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (gmt_grdfloat)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (gmt_grdfloat)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_M_is_fnan (a) || gmt_M_is_fnan (b)) ? GMT->session.f_NaN : (gmt_grdfloat)(a == b);
	}
}

GMT_LOCAL void grdmath_ERFINV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ERFINV 1 1 Inverse error function of A.  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant) {
		a = (gmt_grdfloat)gmt_erfinv (GMT, stack[last]->factor);
		for (node = 0; node < info->size; node++)
			stack[last]->G->data[node] = a;
	}
	else {
		for (node = 0; node < info->size; node++)
			stack[last]->G->data[node] = (gmt_grdfloat)gmt_erfinv (GMT, stack[last]->G->data[node]);
	}
}

GMT_LOCAL void grdmath_EXCH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: EXCH 2 2 Exchanges A and B on the stack.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	gmt_M_unused(GMT);

	for (node = 0; node < info->size; node++) {
		if (stack[prev]->constant) stack[prev]->G->data[node] = (gmt_grdfloat)stack[prev]->factor;
		if (stack[last]->constant) stack[last]->G->data[node] = (gmt_grdfloat)stack[last]->factor;
		gmt_M_grdfloat_swap (stack[last]->G->data[node], stack[prev]->G->data[node]);
	}
	gmt_M_double_swap (stack[last]->factor, stack[prev]->factor);
	gmt_M_bool_swap (stack[last]->constant, stack[prev]->constant);
}

GMT_LOCAL void grdmath_EXP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: EXP 1 1 exp (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)exp (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : expf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_FACT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FACT 1 1 A! (A factorial).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant)
		a = gmt_factorial (GMT, irint(stack[last]->factor));
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : gmt_factorial (GMT, irint((double)stack[last]->G->data[node])));
}

/* Subroutines for grdmath_EXTREMA */

GMT_LOCAL int grdmath_do_derivative (gmt_grdfloat *z, uint64_t this_node, int off, unsigned int type)
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
	if (gmt_M_is_fnan (z[prev_node])) {			/* At least one of the two neighbor points is a NaN */
		if (gmt_M_is_fnan (z[next_node])) return (nan_flag);	/* Both points are NaN, return -2 (or 0 if diagonal) */
		if (z[this_node] == z[next_node]) return (-2);	/* Flat line, no extrema possible */
		if (z[this_node] < z[next_node]) return (-1);	/* A local minimum */
		return (+1);					/* Else it must be a local maximum */
	}
	if (gmt_M_is_fnan (z[next_node])) {			/* One of the two neighbor points is a NaN */
		if (z[this_node] == z[prev_node]) return (-2);	/* Flat line, no extrema possible */
		if (z[this_node] < z[prev_node]) return (-1);	/* A local minimum */
		return (+1);					/* Else it must be a local maximum */
	}/* OK, no NaNs among the three nodes */
	if (z[this_node] == z[prev_node] && z[this_node] == z[next_node]) return (-2);	/* Flat line, no extrema possible */
	if (z[this_node] < z[prev_node] && z[this_node] < z[next_node]) return (-1);	/* A local minimum */
	if (z[this_node] > z[prev_node] && z[this_node] > z[next_node]) return (+1);	/* A local maximum */
	return (0);									/* No extrema found */
}

GMT_LOCAL void grdmath_EXTREMA (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: EXTREMA 1 1 Local extrema: -1 is a (local) minimum, +1 a (local) maximum, and 0 elsewhere.  */
{
	uint64_t node;
	openmp_int row, col;
	int dx, dy, diag, product, mx1;
	gmt_grdfloat *z = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (stack[last]->G->header);

	/* Find local extrema in grid */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand to EXTREMA is constant!\n");
		gmt_M_memset (stack[last]->G->data, info->size, gmt_grdfloat);
		return;
	}

	/* If grid does not have BC rows/cols assigned we apply reasonable conditions:
	 * If -fg we assume geographic grid and use geographic BCs, else we use natural BCs. If the grid
	 * as a BC == GMT_BC_IS_DATA then the pad already constrains observations. */

	gmt_BC_init (GMT, stack[last]->G->header);	/* Initialize grid interpolation and boundary condition parameters */
	gmt_grd_BC_set (GMT, stack[last]->G, GMT_IN);	/* Set boundary conditions */

	/* Now, stack[last]->G->data has boundary rows/cols all set according to the boundary conditions (or actual data).
	 * We can then operate on the interior of the grid and temporarily assign values to the z grid */

	z = gmt_M_memory (GMT, NULL, info->size, gmt_grdfloat);

	/* We will visit each node on the grid and determine if there are extrema.  We do this
	 * by looking at the along-x and along-y profiles separately.  If both of them shows the
	 * central node in a min or max location with respect to its two neighbors (one if at the
	 * edge of the grid or in the presence of NaNs) we must do further checking.  If both are
	 * min or max then we look at the two diagonal lines to see if they can confirm this.
	 * Here, NaNs are not held against you - it takes real values to overrule the dx,dy values.
	 *
	 * Min is given -1, Max is given +1
	 * Default is 0 which means no extrema.
	 */

	mx1 = info->G->header->mx + 1;

	gmt_M_grd_loop (GMT, info->G, row, col, node) {

		if (gmt_M_is_fnan (stack[last]->G->data[node])) continue;	/* No extrema if point is NaN */

		if ((dx = grdmath_do_derivative (stack[last]->G->data, node, 1, 0)) == -2) continue;	/* Too many NaNs or flat x-line */
		if ((dy = grdmath_do_derivative (stack[last]->G->data, node, info->G->header->mx, 0)) == -2) continue;	/* Too many NaNs or flat y-line */

		if ((product = dx * dy) == 0) continue;	/* No min or max possible */
		if (product < 0) continue;	/* Saddle point - don't need to check diagonals */

		/* Need to examine diagonal trends to verify min or max */

		if ((diag = grdmath_do_derivative (stack[last]->G->data, node, -mx1, 1)) == -2) continue;	/* Sorry, no extrema along diagonal N45E */
		if (diag != 0 && diag != dx) continue;						/* Sorry, extrema of opposite sign along diagonal N45E  */
		if ((diag = grdmath_do_derivative (stack[last]->G->data, node,  mx1, 1)) == -2) continue;	/* Sorry, no extrema along diagonal N135E */
		if (diag != 0 && diag != dx) continue;						/* Sorry, extrema of opposite sign along diagonal N135E  */

		/* OK, we have a min or max point; just use dx to check which kind */

		z[node] = (gmt_grdfloat)((dx > 0) ? +1 : -1);
	}

	gmt_M_memcpy (stack[last]->G->data, z, info->size, gmt_grdfloat);
	gmt_M_memset (HH->BC, 4, unsigned int);	/* No BC padding in this array */
	gmt_M_free (GMT, z);
}

GMT_LOCAL void grdmath_SADDLE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SADDLE 1 1  -1|+1 is a saddle point with min|max in x-direction; 0 elsewhere.  */
{
	uint64_t node;
	openmp_int row, col;
	int dx, dy, product;
	gmt_grdfloat *z = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (stack[last]->G->header);

	/* Find saddle points in grid */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand to SADDLE is constant!\n");
		gmt_M_memset (stack[last]->G->data, info->size, gmt_grdfloat);
		return;
	}

	/* If grid does not have BC rows/cols assigned we apply reasonable conditions:
	 * If -fg we assume geographic grid and use geographic BCs, else we use natural BCs. If the grid
	 * as a BC == GMT_BC_IS_DATA then the pad already constrains observations. */

	gmt_BC_init (GMT, stack[last]->G->header);	/* Initialize grid interpolation and boundary condition parameters */
	gmt_grd_BC_set (GMT, stack[last]->G, GMT_IN);	/* Set boundary conditions */

	/* Now, stack[last]->G->data has boundary rows/cols all set according to the boundary conditions (or actual data).
	 * We can then operate on the interior of the grid and temporarily assign values to the z grid */

	z = gmt_M_memory (GMT, NULL, info->size, gmt_grdfloat);

	/* We will visit each node on the grid and determine if there are saddles.  We do this
	 * by looking at the along-x and along-y profiles separately.  If both of them shows the
	 * central node in a min or max location with respect to its two neighbors (one if at the
	 * edge of the grid or in the presence of NaNs) we must do further checking.  If the two
	 * extrema are of different sign we call it a saddle point and we are done, else 0.
	 *
	 * Saddle points are +1 (if max in x) or -1 (if max in y)
	 * Default is 0 which means no saddle.
	 */

	gmt_M_grd_loop (GMT, info->G, row, col, node) {

		if (gmt_M_is_fnan (stack[last]->G->data[node])) continue;	/* No saddle if point is NaN */

		if ((dx = grdmath_do_derivative (stack[last]->G->data, node, 1, 0)) == -2) continue;	/* Too many NaNs or flat x-line */
		if ((dy = grdmath_do_derivative (stack[last]->G->data, node, info->G->header->mx, 0)) == -2) continue;	/* Too many NaNs or flat y-line */

		if ((product = dx * dy) == 0) continue;	/* No local min or max possible */
		if (product < 0)	/* Saddle point */
			z[node] = (gmt_grdfloat)((dx > 0) ? +1 : -1);
	}

	gmt_M_memcpy (stack[last]->G->data, z, info->size, gmt_grdfloat);
	gmt_M_memset (HH->BC, 4, unsigned int);	/* No BC padding in this array */
	gmt_M_free (GMT, z);
}

GMT_LOCAL void grdmath_FCRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FCRIT 3 1 F distribution critical value for alpha = A, nu1 = B, and nu2 = C.  */
{
	uint64_t node;
	int nu1, nu2;
	unsigned int prev1, prev2;
	openmp_int row, col;
	double alpha;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && stack[prev2]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for FCRIT!\n");
	if (stack[prev1]->constant && stack[prev1]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for FCRIT!\n");
	if (stack[last]->constant  && stack[last]->factor  == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand three == 0 for FCRIT!\n");
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			alpha = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
			nu1 = irint ((stack[prev1]->constant) ? stack[prev1]->factor : (double)stack[prev1]->G->data[node]);
			nu2 = irint ((stack[last]->constant)  ? stack[last]->factor  : (double)stack[last]->G->data[node]);
			stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_Fcrit (GMT, alpha, nu1, nu2);
		}
	}
}

GMT_LOCAL void grdmath_FCDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FCDF 3 1 F cumulative distribution function for F = A, nu1 = B, and nu2 = C.  */
{
	uint64_t node, nu1, nu2;
	unsigned int prev1, prev2;
	openmp_int row, col;
	double F;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev1]->constant && stack[prev1]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for FCDF!\n");
	if (stack[last]->constant  && stack[last]->factor  == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand three == 0 for FCDF!\n");
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			F = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
			nu1 = lrint ((stack[prev1]->constant) ? stack[prev1]->factor : (double)stack[prev1]->G->data[node]);
			nu2 = lrint ((stack[last]->constant)  ? stack[last]->factor  : (double)stack[last]->G->data[node]);
			stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_f_cdf (GMT, F, nu1, nu2);
		}
	}
}

GMT_LOCAL void grdmath_FISHER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FISHER 3 1 Fisher probability density function for lon = A, lat = B and kappa = C.  */
{
	uint64_t node;
	unsigned int prev1, prev2;
	openmp_int row, col;
	double lon, lat, kappa;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant) lon = stack[prev2]->factor;
	if (stack[prev1]->constant) lat = stack[prev1]->factor;
	if (stack[last]->constant) kappa = stack[last]->factor;
	grdmath_grd_padloop (GMT, info->G, row, col, node) {
		if (!stack[prev2]->constant) lon = stack[prev2]->G->data[node];
		if (!stack[prev1]->constant) lat = stack[prev1]->G->data[node];
		if (!stack[last]->constant) kappa = stack[last]->G->data[node];
		stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_fisher_pdf (GMT, lon, lat, info->d_grd_x[col], info->d_grd_y[row], kappa);
	}
}

GMT_LOCAL void grdmath_FLIPLR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FLIPLR 1 1 Reverse order of values in each row.  */
{
	uint64_t node;
	openmp_int mx1, row, col_l, col_r, mx_half;

	/* Reverse order of all rows */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand to FLIPLR is constant!\n");
		return;
	}

	/* This must also apply to the pads since any BCs there must be flipped as well, hence a local loop is used */

	mx_half = (openmp_int)info->G->header->mx / 2;
	mx1 = (openmp_int)info->G->header->mx - 1;
	for (node = row = 0; row < (openmp_int)info->G->header->my; row++, node += info->G->header->mx) {	/* Do this to all rows */
		for (col_l = 0, col_r = mx1; col_l < mx_half; col_l++, col_r--) gmt_M_grdfloat_swap (stack[last]->G->data[node+col_l], stack[last]->G->data[node+col_r]);
	}
}

GMT_LOCAL void grdmath_FLIPUD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FLIPUD 1 1 Reverse order of values in each column.  */
{
	openmp_int my1, mx, row_t, row_b, col, my_half;

	/* Reverse order of all columns */

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand to FLIPLR is constant!\n");
		return;
	}

	/* This must also apply to the pads since any BCs there must be flipped as well, hence a local loop is used */

	my_half = (openmp_int)info->G->header->my / 2;
	my1 = (openmp_int)info->G->header->my - 1;
	mx = (openmp_int)info->G->header->mx;
	for (col = 0; col < mx; col++) {	/* Do this to all cols */
		for (row_t = 0, row_b = my1; row_t < my_half; row_t++, row_b--) gmt_M_grdfloat_swap (stack[last]->G->data[(uint64_t)row_t*(uint64_t)mx+(uint64_t)col], stack[last]->G->data[(uint64_t)row_b*(uint64_t)mx+(uint64_t)col]);
	}
}

GMT_LOCAL void grdmath_FLOOR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FLOOR 1 1 floor (A) (greatest integer <= A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)floor (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : floorf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_FMOD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FMOD 2 1 A % B (remainder after truncated division).  */
{
	uint64_t node;
	unsigned int prev;
	double a, b;

	prev = last - 1;
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "using FMOD 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)fmod (a, b);
	}
}

GMT_LOCAL void grdmath_FPDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: FPDF 3 1 F probability density function for F = A, nu1 = B and nu2 = C.  */
{
	uint64_t node, nu1, nu2;
	unsigned int prev1, prev2;
	openmp_int row, col;
	double F;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && stack[prev2]->factor < 0.0)  GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one < 0 for FCDF!\n");
	if (stack[prev1]->constant && stack[prev1]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for FCDF!\n");
	if (stack[last]->constant  && stack[last]->factor  == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand three == 0 for FCDF!\n");
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			F = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
			nu1 = lrint ((stack[prev1]->constant) ? stack[prev1]->factor : (double)stack[prev1]->G->data[node]);
			nu2 = lrint ((stack[last]->constant)  ? stack[last]->factor  : (double)stack[last]->G->data[node]);
			stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_f_pdf (GMT, F, nu1, nu2);
		}
	}
}

GMT_LOCAL void grdmath_GE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: GE 2 1 1 if A >= B, else 0.  */
{
	uint64_t node;
	unsigned int prev;
	gmt_grdfloat a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (gmt_grdfloat)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (gmt_grdfloat)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_M_is_fnan (a) || gmt_M_is_fnan (b)) ? GMT->session.f_NaN : (gmt_grdfloat)(a >= b);
	}
}

GMT_LOCAL void grdmath_GT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: GT 2 1 1 if A > B, else 0.  */
{
	uint64_t node;
	unsigned int prev;
	gmt_grdfloat a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (gmt_grdfloat)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (gmt_grdfloat)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_M_is_fnan (a) || gmt_M_is_fnan (b)) ? GMT->session.f_NaN : (gmt_grdfloat)(a > b);
	}
}

GMT_LOCAL void grdmath_HSV2LAB (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: HSV2LAB 3 3 Convert hsv to lab, with h = A, s = B and v = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double hsv[4], rgb[4], lab[3];

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && (stack[prev2]->factor < 0.0 || stack[prev2]->factor > 360.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument h to HSV2LAB must be a 0 <= h <= 360!\n");
		error++;
	}
	if (stack[prev1]->constant && (stack[prev1]->factor < 0.0 || stack[prev1]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument s to HSV2LAB must be a 0 <= s <= 1!\n");
		error++;
	}
	if (stack[last]->constant  && (stack[last]->factor < 0.0 || stack[last]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument v to HSV2LAB must be a 0 <= v <= 1!\n");
		error++;
	}
	rgb[3] = hsv[3] = 0.0;	/* No transparency involved */
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* Constant arguments */
		hsv[0] = stack[prev2]->factor;
		hsv[1] = stack[prev1]->factor;
		hsv[2] = stack[last]->factor;
		gmt_hsv_to_rgb (rgb, hsv);	/* Must do this via RGB */
		gmt_rgb_to_lab (rgb, lab);
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			stack[prev2]->G->data[node] = (gmt_grdfloat)lab[0];
			stack[prev1]->G->data[node] = (gmt_grdfloat)lab[1];
			stack[last]->G->data[node]  = (gmt_grdfloat)lab[2];
		}
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		hsv[0] = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		hsv[1] = (stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node];
		hsv[2] = (stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node];
		gmt_hsv_to_rgb (rgb, hsv);	/* Must do this via RGB */
		gmt_rgb_to_lab (rgb, lab);
		stack[prev2]->G->data[node] = (gmt_grdfloat)lab[0];
		stack[prev1]->G->data[node] = (gmt_grdfloat)lab[1];
		stack[last]->G->data[node]  = (gmt_grdfloat)lab[2];
	}
}

GMT_LOCAL void grdmath_HSV2RGB (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: HSV2RGB 3 3 Convert hsv to rgb, with h = A, s = B and v = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double rgb[4], hsv[4];

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && (stack[prev2]->factor < 0.0 || stack[prev2]->factor > 360.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument h to HSV2RGB must be a 0 <= h <= 360!\n");
		error++;
	}
	if (stack[prev1]->constant && (stack[prev1]->factor < 0.0 || stack[prev1]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument s to HSV2RGB must be a 0 <= s <= 1!\n");
		error++;
	}
	if (stack[last]->constant  && (stack[last]->factor < 0.0 || stack[last]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument v to HSV2RGB must be a 0 <= v <= 1!\n");
		error++;
	}
	rgb[3] = hsv[3] = 0.0;	/* No transparency involved */
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* Constant arguments */
		hsv[0] = stack[prev2]->factor;
		hsv[1] = stack[prev1]->factor;
		hsv[2] = stack[last]->factor;
		gmt_hsv_to_rgb (rgb, hsv);
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_M_s255 (rgb[0]);
			stack[prev1]->G->data[node] = (gmt_grdfloat)gmt_M_s255 (rgb[1]);
			stack[last]->G->data[node]  = (gmt_grdfloat)gmt_M_s255 (rgb[2]);
		}
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		hsv[0] = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		hsv[1] = (stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node];
		hsv[2] = (stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node];
		gmt_hsv_to_rgb (rgb, hsv);
		stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_M_s255 (rgb[0]);
		stack[prev1]->G->data[node] = (gmt_grdfloat)gmt_M_s255 (rgb[1]);
		stack[last]->G->data[node]  = (gmt_grdfloat)gmt_M_s255 (rgb[2]);
	}
}

GMT_LOCAL void grdmath_HSV2XYZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: HSV2XYZ 3 3 Convert hsv to xyz, with h = A, s = B and v = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double hsv[4], rgb[4], xyz[3];

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && (stack[prev2]->factor < 0.0 || stack[prev2]->factor > 360.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument h to HSV2XYZ must be a 0 <= h <= 360!\n");
		error++;
	}
	if (stack[prev1]->constant && (stack[prev1]->factor < 0.0 || stack[prev1]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument s to HSV2XYZ must be a 0 <= s <= 1!\n");
		error++;
	}
	if (stack[last]->constant  && (stack[last]->factor < 0.0 || stack[last]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument v to HSV2XYZ must be a 0 <= v <= 1!\n");
		error++;
	}
	rgb[3] = hsv[3] = 0.0;	/* No transparency involved */
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* Constant arguments */
		hsv[0] = stack[prev2]->factor;
		hsv[1] = stack[prev1]->factor;
		hsv[2] = stack[last]->factor;
		gmt_hsv_to_rgb (rgb, hsv);	/* Must do this via RGB */
		gmt_rgb_to_xyz (rgb, xyz);
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			stack[prev2]->G->data[node] = (gmt_grdfloat)xyz[0];
			stack[prev1]->G->data[node] = (gmt_grdfloat)xyz[1];
			stack[last]->G->data[node]  = (gmt_grdfloat)xyz[2];
		}
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		hsv[0] = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		hsv[1] = (stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node];
		hsv[2] = (stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node];
		gmt_hsv_to_rgb (rgb, hsv);	/* Must do this via RGB */
		gmt_rgb_to_xyz (rgb, xyz);
		stack[prev2]->G->data[node] = (gmt_grdfloat)xyz[0];
		stack[prev1]->G->data[node] = (gmt_grdfloat)xyz[1];
		stack[last]->G->data[node]  = (gmt_grdfloat)xyz[2];
	}
}

GMT_LOCAL void grdmath_HYPOT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: HYPOT 2 1 hypot (A, B) = sqrt (A*A + B*B).  */
{
	uint64_t node;
	unsigned int prev;
	double a, b;
	gmt_M_unused(GMT);

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)hypot (a, b);
	}
}

GMT_LOCAL void grdmath_I0 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: I0 1 1 Modified Bessel function of A (1st kind, order 0).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = gmt_i0 (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : gmt_i0 (GMT, stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_I1 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: I1 1 1 Modified Bessel function of A (1st kind, order 1).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = gmt_i1 (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : gmt_i1 (GMT, stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_IFELSE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: IFELSE 3 1 B if A != 0, else C.  */
{
	uint64_t node;
	unsigned int prev1, prev2;
	gmt_grdfloat a = 0.0f, b = 0.0f, c = 0.0;
	gmt_M_unused(GMT);

	/* last is C */
	prev1 = last - 1;	/* This is B */
	prev2 = last - 2;	/* This is A */

	/* Set to B if A == 1 else set to C
	 * A, B, or C = NaN, in which case we set answer to NaN */

	if (stack[prev2]->constant) a = (gmt_grdfloat)stack[prev2]->factor;
	if (stack[prev1]->constant) b = (gmt_grdfloat)stack[prev1]->factor;
	if (stack[last]->constant)  c = (gmt_grdfloat)stack[last]->factor;

	for (node = 0; node < info->size; node++) {
		if (!stack[prev2]->constant) a = stack[prev2]->G->data[node];
		if (!stack[prev1]->constant) b = stack[prev1]->G->data[node];
		if (!stack[last]->constant)  c = stack[last]->G->data[node];

		stack[prev2]->G->data[node] = (fabsf (a) < GMT_CONV8_LIMIT) ? c : b;
	}
}

GMT_LOCAL void grdmath_IN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: IN 2 1 Modified Bessel function of A (1st kind, order B).  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	int order = 0;
	bool simple = false;
	gmt_grdfloat b = 0.0;

	if (stack[last]->constant) {
		if (stack[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order < 0 for IN!\n");
		if (fabs (rint(stack[last]->factor) - stack[last]->factor) > GMT_CONV4_LIMIT) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order not an integer for IN!\n");
		order = urint (fabs (stack[last]->factor));
		if (stack[prev]->constant) {
			b = (gmt_grdfloat)gmt_in (GMT, order, fabs (stack[prev]->factor));
			simple = true;
		}
	}
	if (simple) {
		for (node = 0; node < info->size; node++)
			stack[prev]->G->data[node] = b;
	}
	else {
		for (node = 0; node < info->size; node++) {
			if (!stack[last]->constant) order = urint (fabs (stack[last]->G->data[node]));
			stack[last]->G->data[node] = (gmt_grdfloat)gmt_in (GMT, order, fabs ((double)stack[prev]->G->data[node]));
		}
	}
}

GMT_LOCAL void grdmath_INRANGE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: INRANGE 3 1 1 if B <= A <= C, else 0.  */
{
	uint64_t node;
	unsigned int prev1, prev2;
	gmt_grdfloat a = 0.0f, b = 0.0f, c = 0.0f, inrange;

	/* last is C */
	prev1 = last - 1;	/* This is B */
	prev2 = last - 2;	/* This is A */

	/* Set to 1 where B <= A <= C, 0 elsewhere, except where
	 * A, B, or C = NaN, in which case we set answer to NaN */

	if (stack[prev2]->constant) a = (gmt_grdfloat)stack[prev2]->factor;
	if (stack[prev1]->constant) b = (gmt_grdfloat)stack[prev1]->factor;
	if (stack[last]->constant)  c = (gmt_grdfloat)stack[last]->factor;

	for (node = 0; node < info->size; node++) {
		if (!stack[prev2]->constant) a = stack[prev2]->G->data[node];
		if (!stack[prev1]->constant) b = stack[prev1]->G->data[node];
		if (!stack[last]->constant)  c = stack[last]->G->data[node];

		if (gmt_M_is_fnan (a) || gmt_M_is_fnan (b) || gmt_M_is_fnan (c)) {
			stack[prev2]->G->data[node] = GMT->session.f_NaN;
			continue;
		}

		inrange = (b <= a && a <= c) ? 1.0f : 0.0;
		stack[prev2]->G->data[node] = inrange;
	}
}

GMT_LOCAL void grdmath_INSIDE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: INSIDE 1 1 1 when inside or on polygon(s) in A, else 0.  */
{	/* Suitable for geographic (lon, lat) data and polygons */
	openmp_int row, col;
	uint64_t node, seg;
	unsigned int inside;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;

	if (GMT_Set_Columns (GMT->parent, GMT_IN, 2, GMT_COL_FIX_NO_TEXT) != GMT_NOERROR) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in operator INSIDE setting number of input columns\n");
		info->error = GMT->parent->error;
		return;
	}
	gmt_skip_xy_duplicates (GMT, true);	/* Avoid repeating x/y points in polygons */
	/* Passing GMT_VIA_MODULE_INPUT since these are command line file arguments but processed here instead of by GMT_Init_IO */
	if ((D = GMT_Read_Data (GMT->parent, GMT_IS_DATASET|GMT_VIA_MODULE_INPUT, GMT_IS_FILE, GMT_IS_POLY, GMT_READ_NORMAL|GMT_IO_RESET, NULL, info->ASCII_file, NULL)) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in operator INSIDE reading file %s!\n", info->ASCII_file);
		info->error = GMT->parent->error;
		return;
	}
	gmt_skip_xy_duplicates (GMT, false);	/* Reset */
	T = D->table[0];	/* Only one table in a single file */
	gmt_set_inside_mode (GMT, D, GMT_IOO_UNKNOWN);

#ifdef _OPENMP
#pragma omp parallel for private(row,col,seg,node,inside,S) shared(info,stack,last,GMT,T)
#endif
	for (row = 0; row < (openmp_int)info->G->header->my; row++) {
		node = row * info->G->header->mx;
		for (col = 0; col < (openmp_int)info->G->header->mx; col++, node++) {
			for (seg = inside = 0; !inside && seg < T->n_segments; seg++) {
				S = T->segment[seg];
				if (gmt_polygon_is_hole (GMT, S)) continue;	/* Holes are handled within gmt_inonout */
				inside = gmt_inonout (GMT, info->d_grd_x[col], info->d_grd_y[row], S);
			}
			stack[last]->G->data[node] = (inside > GMT_OUTSIDE) ? 1.0f : 0.0;
		}
	}

	/* Free memory used for pol */

	if (GMT_Destroy_Data (GMT->parent, &D) != GMT_NOERROR) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in operator INSIDE destroying allocated data from %s!\n", info->ASCII_file);
		info->error = GMT->parent->error;
		return;
	}
}

GMT_LOCAL void grdmath_INV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: INV 1 1 1 / A.  */
{
	uint64_t node;
	double a;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Inverse of zero gives NaNs\n");
	if (stack[last]->constant) stack[last]->factor = (stack[last]->factor == 0.0) ? GMT->session.f_NaN : 1.0 / stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		a = (stack[last]->constant) ? stack[last]->factor : 1.0 / stack[last]->G->data[node];
		stack[last]->G->data[node] = (gmt_grdfloat)a;
	}
}

GMT_LOCAL void grdmath_ISFINITE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ISFINITE 1 1 1 if A is finite, else 0.  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)isfinite (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : isfinite (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_ISNAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ISNAN 1 1 1 if A == NaN, else 0.  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)gmt_M_is_fnan (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : gmt_M_is_fnan (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_J0 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: J0 1 1 Bessel function of A (1st kind, order 0).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)j0 (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (gmt_grdfloat)j0 (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_J1 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: J1 1 1 Bessel function of A (1st kind, order 1).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)j1 (fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (gmt_grdfloat)j1 (fabsf (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_JN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: JN 2 1 Bessel function of A (1st kind, order B).  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	int order = 0;
	bool simple = false;
	gmt_grdfloat b = 0.0;

	if (stack[last]->constant) {
		if (stack[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order < 0 for JN!\n");
		if (fabs (rint(stack[last]->factor) - stack[last]->factor) > GMT_CONV4_LIMIT) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order not an integer for JN!\n");
		order = urint (fabs (stack[last]->factor));
		if (stack[prev]->constant) {
			b = (gmt_grdfloat)jn (order, fabs (stack[prev]->factor));
			simple = true;
		}
	}
	for (node = 0; node < info->size; node++) {
		if (simple)
			stack[prev]->G->data[node] = b;
		else {
			if (!stack[last]->constant) order = urint (fabsf (stack[last]->G->data[node]));
			stack[last]->G->data[node] = (gmt_grdfloat)jn (order, fabsf (stack[prev]->G->data[node]));
		}
	}
}

GMT_LOCAL void grdmath_K0 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: K0 1 1 Modified Kelvin function of A (2nd kind, order 0).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant) a = (gmt_grdfloat)gmt_k0 (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (gmt_grdfloat)gmt_k0 (GMT, stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_K1 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: K1 1 1 Modified Bessel function of A (2nd kind, order 1).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant) a = (gmt_grdfloat)gmt_k1 (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (gmt_grdfloat)gmt_k1 (GMT, stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_KEI (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: KEI 1 1 Kelvin function kei (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant) a = (gmt_grdfloat)gmt_kei (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (gmt_grdfloat)gmt_kei (GMT, fabsf (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_KER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: KER 1 1 Kelvin function ker (A).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = gmt_ker (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : gmt_ker (GMT, fabsf (stack[last]->G->data[node])));
}

GMT_LOCAL void grdmath_KM2DEG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: KM2DEG 1 1 Converts Kilometers to Spherical Degrees.  */
{
	uint64_t node;
	double a = 0.0, f = 1.0 / GMT->current.proj.DIST_KM_PR_DEG;

	if (gmt_M_is_geographic (GMT, GMT_IN)) {
		if (gmt_M_sph_mode (GMT) == GMT_GEODESIC) GMT_Report (GMT->parent, GMT_MSG_WARNING, "KM2DEG is only exact when PROJ_ELLIPSOID == sphere\n");
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "KM2DEG used with Cartesian data\n");
	if (stack[last]->constant) a = stack[last]->factor * f;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : (stack[last]->G->data[node] * f));
}

GMT_LOCAL void grdmath_KN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: KN 2 1 Modified Bessel function of A (2nd kind, order B).  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	int order = 0;
	bool simple = false;
	gmt_grdfloat b = 0.0;

	if (stack[last]->constant) {
		if (stack[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order < 0 for KN!\n");
		if (fabs (rint(stack[last]->factor) - stack[last]->factor) > GMT_CONV4_LIMIT) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order not an integer for KN!\n");
		order = urint (fabs (stack[last]->factor));
		if (stack[prev]->constant) {
			b = (gmt_grdfloat)gmt_kn (GMT, order, fabs (stack[prev]->factor));
			simple = true;
		}
	}
	if (simple) {
		for (node = 0; node < info->size; node++)
			stack[prev]->G->data[node] = b;
	}
	else {
		for (node = 0; node < info->size; node++) {
			if (!stack[last]->constant) order = urint (fabsf (stack[last]->G->data[node]));
			stack[last]->G->data[node] = (gmt_grdfloat)gmt_kn (GMT, order, fabsf (stack[prev]->G->data[node]));
		}
	}
}

GMT_LOCAL void grdmath_KURT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: KURT 1 1 Kurtosis of A.  */
{
	uint64_t node, n = 0;
	openmp_int row, col;
	double mean = 0.0, sum2 = 0.0, kurt = 0.0, delta;
	gmt_grdfloat f_kurt;

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = GMT->session.f_NaN;
		return;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		if (gmt_M_is_fnan (stack[last]->G->data[node])) continue;
		n++;
		delta = (double)stack[last]->G->data[node] - mean;
		mean += delta / n;
		sum2 += delta * ((double)stack[last]->G->data[node] - mean);
	}
	if (n > 1) {
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			if (gmt_M_is_fnan (stack[last]->G->data[node])) continue;
			delta = (double)stack[last]->G->data[node] - mean;
			kurt += pow (delta, 4.0);
		}
		sum2 /= (n - 1);
		kurt = kurt / (n * sum2 * sum2) - 3.0;
		f_kurt = (gmt_grdfloat)kurt;
	}
	else
		f_kurt = GMT->session.f_NaN;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = f_kurt;
}

/* Helper functions grdmath_ASCII_read and grdmath_ASCII_free are used in LDIST*, PDIST and *POINT */

GMT_LOCAL struct GMT_DATASET *grdmath_ASCII_read (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, int geometry, char *op)
{
	struct GMT_DATASET *D = NULL;
	int error;
	if (gmt_M_is_geographic (GMT, GMT_IN))
		error = gmt_init_distaz (GMT, 'k', gmt_M_sph_mode (GMT), GMT_MAP_DIST);
	else {
		char code = strcmp (op, "PDIST") ? 'R' : 'X';
		error = gmt_init_distaz (GMT, code, 0, GMT_MAP_DIST);	/* Cartesian distances of some flavor */
	}
	if (error == GMT_NOT_A_VALID_TYPE) return NULL;
	if (GMT_Set_Columns (GMT->parent, GMT_IN, 2, GMT_COL_FIX_NO_TEXT) != GMT_NOERROR) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in operator %s setting number of input columns\n", op);
		info->error = GMT->parent->error;
		return NULL;
	}
	/* Passing GMT_VIA_MODULE_INPUT since these are command line file arguments but processed here instead of by GMT_Init_IO */
	if ((D = GMT_Read_Data (GMT->parent, GMT_IS_DATASET|GMT_VIA_MODULE_INPUT, GMT_IS_FILE, geometry, GMT_READ_NORMAL|GMT_IO_RESET, NULL, info->ASCII_file, NULL)) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in operator %s reading file %s!\n", op, info->ASCII_file);
		info->error = GMT->parent->error;
		return NULL;
	}
	return (D);
}

GMT_LOCAL int grdmath_ASCII_free (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_DATASET **D, char *op)
{
	if (GMT_Destroy_Data (GMT->parent, D) != GMT_NOERROR) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failure in operator %s destroying allocated data from %s!\n", op, info->ASCII_file);
		info->error = GMT->parent->error;
		return 1;
	}
	return 0;
}

GMT_LOCAL void grdmath_LAB2HSV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LAB2HSV 3 3 Convert lab to hsv, with l = A, a = B and b = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double hsv[4], lab[4], rgb[4];

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && (stack[prev2]->factor < 0.0 || stack[prev2]->factor > 100.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument l to LAB2HSV must be a 0 <= l <= 100!\n");
		error++;
	}
#if 0
	if (stack[prev1]->constant && (stack[prev1]->factor < 0.0 || stack[prev1]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument s to LAB2HSV must be a 0 <= s <= 1!\n");
		error++;
	}
	if (stack[last]->constant  && (stack[last]->factor < 0.0 || stack[last]->factor < 0.0 > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument v to LAB2HSV must be a 0 <= v <= 1!\n");
		error++;
	}
#endif
	rgb[3] = hsv[3] = 0.0;	/* No transparency involved */
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* Constant arguments */
		lab[0] = stack[prev2]->factor;
		lab[1] = stack[prev1]->factor;
		lab[2] = stack[last]->factor;
		gmt_lab_to_rgb (rgb, lab);	/* Must do this via RGB */
		gmt_rgb_to_hsv (rgb, hsv);
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			stack[prev2]->G->data[node] = (gmt_grdfloat)hsv[0];
			stack[prev1]->G->data[node] = (gmt_grdfloat)hsv[1];
			stack[last]->G->data[node]  = (gmt_grdfloat)hsv[2];
		}
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		lab[0] = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		lab[1] = (stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node];
		lab[2] = (stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node];
		gmt_lab_to_rgb (rgb, lab);	/* Must do this via RGB */
		gmt_rgb_to_hsv (rgb, hsv);
		stack[prev2]->G->data[node] = (gmt_grdfloat)hsv[0];
		stack[prev1]->G->data[node] = (gmt_grdfloat)hsv[1];
		stack[last]->G->data[node]  = (gmt_grdfloat)hsv[2];
	}
}

GMT_LOCAL void grdmath_LAB2RGB (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LAB2RGB 3 3 Convert lab to rgb, with l = A, a = B and b = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double lab[3], rgb[3];

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && (stack[prev2]->factor < 0.0 || stack[prev2]->factor > 100.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument l to LAB2RGB must be a 0 <= l <= 100!\n");
		error++;
	}
#if 0
	if (stack[prev1]->constant && (stack[prev1]->factor < 0.0 || stack[prev1]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument s to LAB2RGB must be a 0 <= s <= 1!\n");
		error++;
	}
	if (stack[last]->constant  && (stack[last]->factor < 0.0 || stack[last]->factor < 0.0 > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument v to LAB2RGB must be a 0 <= v <= 1!\n");
		error++;
	}
#endif
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* Constant arguments */
		lab[0] = stack[prev2]->factor;
		lab[1] = stack[prev1]->factor;
		lab[2] = stack[last]->factor;
		gmt_lab_to_rgb (rgb, lab);
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_M_s255 (rgb[0]);
			stack[prev1]->G->data[node] = (gmt_grdfloat)gmt_M_s255 (rgb[1]);
			stack[last]->G->data[node]  = (gmt_grdfloat)gmt_M_s255 (rgb[2]);
		}
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		lab[0] = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		lab[1] = (stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node];
		lab[2] = (stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node];
		gmt_lab_to_rgb (rgb, lab);
		stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_M_s255 (rgb[0]);
		stack[prev1]->G->data[node] = (gmt_grdfloat)gmt_M_s255 (rgb[1]);
		stack[last]->G->data[node]  = (gmt_grdfloat)gmt_M_s255 (rgb[2]);
	}
}

GMT_LOCAL void grdmath_LAB2XYZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LAB2XYZ 3 3 Convert lab to xyz, with l = A, a = B and b = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double lab[3], xyz[3];

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && (stack[prev2]->factor < 0.0 || stack[prev2]->factor > 100.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument l to LAB2XYZ must be a 0 <= l <= 100!\n");
		error++;
	}
#if 0
	if (stack[prev1]->constant && (stack[prev1]->factor < 0.0 || stack[prev1]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument s to LAB2XYZ must be a 0 <= s <= 1!\n");
		error++;
	}
	if (stack[last]->constant  && (stack[last]->factor < 0.0 || stack[last]->factor < 0.0 > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument v to LAB2XYZ must be a 0 <= v <= 1!\n");
		error++;
	}
#endif
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* Constant arguments */
		lab[0] = stack[prev2]->factor;
		lab[1] = stack[prev1]->factor;
		lab[2] = stack[last]->factor;
		gmt_lab_to_xyz (xyz, lab);
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			stack[prev2]->G->data[node] = (gmt_grdfloat)xyz[0];
			stack[prev1]->G->data[node] = (gmt_grdfloat)xyz[1];
			stack[last]->G->data[node]  = (gmt_grdfloat)xyz[2];
		}
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		lab[0] = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		lab[1] = (stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node];
		lab[2] = (stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node];
		gmt_lab_to_xyz (xyz, lab);
		stack[prev2]->G->data[node] = (gmt_grdfloat)xyz[0];
		stack[prev1]->G->data[node] = (gmt_grdfloat)xyz[1];
		stack[last]->G->data[node]  = (gmt_grdfloat)xyz[2];
	}
}

GMT_LOCAL void grdmath_LCDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LCDF 1 1 Laplace cumulative distribution function for z = A.  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = 0.5 + copysign (0.5, stack[last]->factor) * (1.0 - exp (-fabs (stack[last]->factor)));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : 0.5f + copysignf (0.5f, stack[last]->G->data[node]) * (1.0 - expf (-fabsf (stack[last]->G->data[node]))));
}

GMT_LOCAL void grdmath_LCRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LCRIT 1 1 Laplace distribution critical value for alpha = A.  */
{
	uint64_t node;
	double a = 0.0, p;
	gmt_M_unused(GMT);

	if (stack[last]->constant) {
		p = (1.0 - stack[last]->factor) - 0.5;
		a = -copysign (1.0, p) * log (1.0 - 2.0 * fabs (p));
	}
	for (node = 0; node < info->size; node++) {
		if (stack[last]->constant)
			stack[last]->G->data[node] = (gmt_grdfloat)a;
		else {
			p = (1.0 - stack[last]->G->data[node]) - 0.5;
			stack[last]->G->data[node] = (gmt_grdfloat)(-copysign (1.0, p) * log (1.0 - 2.0 * fabs (p)));
		}
	}
}

GMT_LOCAL void grdmath_LDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LDIST 1 1 Compute minimum distance (in km if -fg) from lines in multi-segment ASCII file A.  */
{
	int64_t node, row, col;			/* int since VS 2013/OMP 2.0 doesn't allow unsigned index variables */
	double d;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;

	if ((D = grdmath_ASCII_read (GMT, info, GMT_IS_LINE, "LDIST")) == NULL) return;
	T = D->table[0];	/* Only one table in a single file */

#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,d) shared(info,stack,last,GMT,T)
#endif
	for (row = 0; row < (openmp_int)info->G->header->my; row++) {
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Row %d\n", row);
		for (col = 0; col < (openmp_int)info->G->header->mx; col++) {	/* Visit each node */
			(void) gmt_near_lines (GMT, info->d_grd_x[col], info->d_grd_y[row], T, 1, &d, NULL, NULL);
			node = gmt_M_ij(info->G->header,row,col);
			stack[last]->G->data[node] = (gmt_grdfloat)d;
		}
	}

	grdmath_ASCII_free (GMT, info, &D, "LDIST");	/* Free memory used for line */
}

GMT_LOCAL void grdmath_LDISTG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LDISTG 0 1 As LDIST, but operates on the GSHHG dataset (see -A, -D for options).  */
{
#ifdef _OPENMP
	uint64_t thread_num = 0;
#endif
	uint64_t node, col, seg, tbl, n_threads = 1, OFF = 0;
	int64_t row, old_row = INT64_MAX;
	int i, old_i = INT32_MAX;
	double lon, lon1, lat, x, y, hor = DBL_MAX, bin_size, slop, d;
	double max_hor = 0.0, wesn[4] = {0.0, 360.0, -90.0, 90.0};
	double *curr_dist = NULL;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;

	/* LDISTG is now set up for OpenMP parallel execution. By default all threads are used; control this via -x.
	 * We use helper array curr_dist which needs one sectiion per thread and we determine which thread we are
	 * working on in the loop.  In OpenMP is not active then number of threads is 1 and the current thread is
	 * always 0. PW Aug 4. 2018. */

	if (gmt_M_is_cartesian (GMT, GMT_IN)) /* Set -fg implicitly since not set already via input grid or -fg */
		gmt_parse_common_options (GMT, "f", 'f', "g");
	if (gmt_init_distaz (GMT, 'k', gmt_M_sph_mode (GMT), GMT_MAP_DIST) == GMT_NOT_A_VALID_TYPE) return;	/* Request distances in km */

	/* We use the global GSHHG data set to construct distances to. Although we know that the
	 * max distance to a coastline is ~2700 km, we cannot anticipate the usage of any user.
	 * If (s)he's excluding small features, then the distance will be larger. So we do not
	 * limit the region of GSHHG and ask for all. */
	if ((D = gmt_get_gshhg_lines (GMT, wesn, info->gshhg_res, info->A)) == NULL) return;

	bin_size = info->A->bin_size;		/* Current GSHHG bin size in degrees */
	slop = 2 * gmt_distance (GMT, 0.0, 0.0, bin_size, 0.0);	/* Define slop in projected units (km) for bin at Equator */
	if (last == UINT32_MAX) last = 0;	/* Was called the very first time when nstack - 1 goes crazy since it is unsigned */
#ifdef _OPENMP
	n_threads = GMT->common.x.n_threads;	/* Use the number of selected threads (see -x) */
#endif
	curr_dist = gmt_M_memory (GMT, NULL, n_threads * D->n_tables, double);	/* Need one of these arrays sections per thread */
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,d,lon,lat,i,lon1,tbl,x,y,T,seg) firstprivate(old_i,old_row,hor,max_hor,OFF,thread_num) shared(info,stack,GMT,D,bin_size,slop,last,curr_dist)
#endif
	for (row = 0; row < (int64_t)info->G->header->my; row++) {	/* Visit each row including the pad */
#ifdef _OPENMP
		thread_num = omp_get_thread_num();	/* Which thread are we at now? */
		OFF = thread_num * D->n_tables;		/* Calculate offset into curr_dist array */
#else		/* Only allow -Vi message if not threaded */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Row %d\n", row);
#endif
		node = gmt_M_ij (info->G->header, row, 0);	/* Starting node for this row */
		for (col = 0; col < (uint64_t)info->G->header->mx; col++, node++) {	/* Visit each col including the pad */
			lon = info->d_grd_x[col], lat = info->d_grd_y[row];	/* Current node coordinates */
			i = (int)floor(lon/bin_size);	/* Determine horizontal bin number given the bin size */
			/* For any new bin along a row, find the closest center of coastline bins */
			if (i != old_i || row != old_row) {
				lon1 = (i + 0.5) * bin_size;	/* Mid-bin longitude */
				for (tbl = 0, hor = DBL_MAX; tbl < D->n_tables; tbl++) {
					/* Get mid-point of table coordinates */
					x = 0.5 * (D->table[tbl]->min[GMT_X] + D->table[tbl]->max[GMT_X]);
					y = 0.5 * (D->table[tbl]->min[GMT_Y] + D->table[tbl]->max[GMT_Y]);
					curr_dist[OFF+tbl] = d = gmt_distance (GMT, lon1, lat, x, y);
					if (d < hor) hor = d;	/* Keep track of shortest distance */
				}
				/* Add 2 bin sizes to the closest distance to a bin as slop. This should always include the closest points in any bin */
				hor = hor + slop;
				old_i = i, old_row = (int)row;
				if (hor > max_hor) max_hor = hor;	/* Remember the max distance considered in this run */
			}

			/* Loop over each line segment in each bin that is closer than the horizon defined above */
			for (tbl = 0, d = DBL_MAX; tbl < D->n_tables; tbl++) {
				if (curr_dist[OFF+tbl] >= hor) continue;	/* Skip entire bins that are too far away */
				T = D->table[tbl];	/* Examine this table's segments */
				for (seg = 0; seg < T->n_segments; seg++) {
					(void) gmt_near_a_line (GMT, lon, lat, seg, T->segment[seg], true, &d, NULL, NULL);
				}
			}
			stack[last]->G->data[node] = (gmt_grdfloat)d;	/* Finally got the closest approach */
		}
	}

#ifndef _OPENMP
	/* Can only report this if not threaded */
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Max LDISTG horizon distance used: %g\n", max_hor);
#endif
	gmt_M_free (GMT, curr_dist);
	gmt_free_dataset (GMT, &D);
}

GMT_LOCAL void grdmath_LDIST2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LDIST2 2 1 As LDIST, from lines in ASCII file B but only to nodes where A != 0.  */
{
	int64_t node, row, col;			/* int since VS 2013/OMP 2.0 doesn't allow unsigned index variables */
	unsigned int prev;
	double d;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;

	if ((D = grdmath_ASCII_read (GMT, info, GMT_IS_LINE, "LDIST2")) == NULL) return;
	T = D->table[0];	/* Only one table in a single file */
	prev = last - 1;

#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,d) shared(info,stack,prev,GMT,T)
#endif
	for (row = 0; row < (openmp_int)info->G->header->my; row++) {
		node = row * info->G->header->mx;
		for (col = 0; col < (openmp_int)info->G->header->mx; col++, node++) {
			if (col == 0) GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Row %d\n", row);
			if (stack[prev]->G->data[node] == 0.0)
				stack[prev]->G->data[node] = GMT->session.f_NaN;
			else {
				(void) gmt_near_lines (GMT, info->d_grd_x[col], info->d_grd_y[row], T, 1, &d, NULL, NULL);
				stack[prev]->G->data[node] = (gmt_grdfloat)d;
			}
		}
	}

	grdmath_ASCII_free (GMT, info, &D, "LDIST2");	/* Free memory used for line */
}

GMT_LOCAL void grdmath_LE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LE 2 1 1 if A <= B, else 0.  */
{
	uint64_t node;
	unsigned int prev;
	gmt_grdfloat a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (gmt_grdfloat)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (gmt_grdfloat)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_M_is_fnan (a) || gmt_M_is_fnan (b)) ? GMT->session.f_NaN : (gmt_grdfloat)(a <= b);
	}
}

GMT_LOCAL void grdmath_LOG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LOG 1 1 log (A) (natural log).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "argument to log = 0\n");

	if (stack[last]->constant) a = (gmt_grdfloat)d_log (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : d_logf (GMT, fabsf (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_LOG10 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LOG10 1 1 log10 (A) (base 10).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "argument to log10 = 0\n");

	if (stack[last]->constant) a = (gmt_grdfloat)d_log10 (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : d_log10f (GMT, fabsf (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_LOG1P (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LOG1P 1 1 log (1+A) (accurate for small A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant && stack[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "argument to log1p < 0\n");

	if (stack[last]->constant) a = (gmt_grdfloat)d_log1p (GMT, fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : d_log1pf (GMT, fabsf (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_LOG2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: LOG2 1 1 log2 (A) (base 2).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "argument to log2 = 0\n");

	if (stack[last]->constant) a = d_log (GMT, fabs (stack[last]->factor)) * M_LN2_INV;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : d_logf (GMT, fabsf (stack[last]->G->data[node])) * M_LN2_INV);
}

GMT_LOCAL void grdmath_LMSSCL (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: LMSSCL 1 1 LMS scale estimate (LMS STD) of A.  */
	uint64_t node;
	gmt_grdfloat lmsscl_f;
	struct GMT_GRID *W = NULL;

	if (stack[last]->constant) {	/* Trivial case: lmsscale = 0 */
		gmt_M_memset (stack[last]->G->data, info->size, gmt_grdfloat);
		return;
	}

	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must use spherical weights */
		W = gmt_duplicate_grid (GMT, stack[last]->G, GMT_DUPLICATE_ALLOC);
		gmt_get_cellarea (GMT, W);
	}

	lmsscl_f = (gmt_grdfloat)gmt_grd_lmsscl (GMT, stack[last]->G, W, NULL, true);

	if (W) gmt_free_grid (GMT, &W, true);

	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = lmsscl_f;
}

GMT_LOCAL void grdmath_LMSSCLW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: LMSSCLW 1 1 Weighted LMS scale estimate (LMS STD) of A for weights in B.  */
	uint64_t node;
	unsigned int prev = last - 1;
	gmt_grdfloat lmsscl;

	if (stack[prev]->constant) {	/* Trivial case: lmsscale = 0 */
		gmt_M_memset (stack[prev]->G->data, info->size, gmt_grdfloat);
		return;
	}

	lmsscl = (gmt_grdfloat)gmt_grd_lmsscl (GMT, stack[prev]->G, stack[last]->G, NULL, true);
	for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = lmsscl;
}

GMT_LOCAL void grdmath_LOWER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: LOWER 1 1 The lowest (minimum) value of A.  */
	uint64_t node;
	openmp_int row, col;
	gmt_grdfloat low = FLT_MAX;
	gmt_M_unused(GMT);

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)stack[last]->factor;
		return;
	}

	gmt_M_grd_loop (GMT, info->G, row, col, node) {	/* First we must find the lowest value in the grid */
		if (gmt_M_is_fnan (stack[last]->G->data[node])) continue;
		if (stack[last]->G->data[node] < low) low = stack[last]->G->data[node];
	}
	/* Now copy that low value everywhere */
	if (low == FLT_MAX) low = GMT->session.f_NaN;
	for (node = 0; node < info->size; node++) if (!gmt_M_is_fnan (stack[last]->G->data[node])) stack[last]->G->data[node] = low;
}

GMT_LOCAL void grdmath_LPDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: LPDF 1 1 Laplace probability density function for z = A.  */
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = 0.5 * exp (-fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : 0.5 * expf (-fabsf (stack[last]->G->data[node])));
}

GMT_LOCAL void grdmath_LRAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: LRAND 2 1 Laplace random noise with mean A and std. deviation B.  */
	uint64_t node;
	unsigned int prev;
	double a = 0.0, b = 0.0;

	prev = last - 1;
	if (stack[prev]->constant) a = stack[prev]->factor;
	if (stack[last]->constant) b = stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) a = stack[prev]->G->data[node];
		if (!stack[last]->constant) b = stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)(a + b * gmt_lrand (GMT));
	}
}

GMT_LOCAL void grdmath_LT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: LT 2 1 1 if A < B, else 0.  */
	uint64_t node;
	unsigned int prev;
	gmt_grdfloat a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (gmt_grdfloat)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (gmt_grdfloat)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_M_is_fnan (a) || gmt_M_is_fnan (b)) ? GMT->session.f_NaN : (gmt_grdfloat)(a < b);
	}
}

GMT_LOCAL void grdmath_MAD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: MAD 1 1 Median Absolute Deviation (L1 STD) of A.  */
	uint64_t node;
	gmt_grdfloat mad_f;
	struct GMT_GRID *W = NULL;

	if (stack[last]->constant) {	/* Trivial case: mad = 0 */
		gmt_M_memset (stack[last]->G->data, info->size, gmt_grdfloat);
		return;
	}

	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must use spherical weights */
		W = gmt_duplicate_grid (GMT, stack[last]->G, GMT_DUPLICATE_ALLOC);
		gmt_get_cellarea (GMT, W);
	}

	mad_f = (gmt_grdfloat)gmt_grd_mad (GMT, stack[last]->G, W, NULL, true);

	if (W) gmt_free_grid (GMT, &W, true);

	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = mad_f;
}

GMT_LOCAL void grdmath_MADW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: MADW 2 1 Weighted Median Absolute Deviation (L1 STD) of A for weights in B.  */
	uint64_t node;
	unsigned int prev = last - 1;
	gmt_grdfloat wmad;

	if (stack[prev]->constant) {	/* Trivial case if data are constant: mad = 0 */
		gmt_M_memset (stack[last]->G->data, info->size, gmt_grdfloat);
		return;
	}

	wmad = (gmt_grdfloat)gmt_grd_mad (GMT, stack[prev]->G, stack[last]->G, NULL, true);
	for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = wmad;
}

GMT_LOCAL void grdmath_MAX (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: MAX 2 1 Maximum of A and B.  */
	uint64_t node;
	unsigned int prev;
	gmt_grdfloat a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (gmt_grdfloat)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (gmt_grdfloat)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_M_is_fnan (a) || gmt_M_is_fnan (b)) ? GMT->session.f_NaN : MAX (a, b);
	}
}

GMT_LOCAL void grdmath_MEAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: MEAN 1 1 Mean value of A.  */
	uint64_t node;
	gmt_grdfloat zm;
	struct GMT_GRID *W = NULL;
	gmt_M_unused(GMT);

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)stack[last]->factor;
		return;
	}

	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must use spherical weights */
		W = gmt_duplicate_grid (GMT, stack[last]->G, GMT_DUPLICATE_ALLOC);
		gmt_get_cellarea (GMT, W);
	}

	zm = (gmt_grdfloat)gmt_grd_mean (GMT, stack[last]->G, W);	/* Compute the [weighted] mean */

	if (W) gmt_free_grid (GMT, &W, true);

	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = zm;
}

GMT_LOCAL void grdmath_MEANW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: MEANW 2 1 Weighted mean value of A for weights in B.  */
	uint64_t node;
	unsigned int prev = last - 1;
	gmt_grdfloat zm;
	gmt_M_unused(GMT);

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for MEANW!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for MEANW!\n");

	if (stack[prev]->constant) {	/* Trivial case if data are constant */
		for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = (gmt_grdfloat)stack[prev]->factor;
		return;
	}

	zm = (gmt_grdfloat)gmt_grd_mean (GMT, stack[prev]->G, stack[last]->G);
	for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = zm;
}

GMT_LOCAL void grdmath_MEDIAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: MEDIAN 1 1 Median value of A.  */
	uint64_t node;
	gmt_grdfloat med;
	struct GMT_GRID *W = NULL;

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)stack[last]->factor;
		return;
	}
	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must use spherical weights */
		W = gmt_duplicate_grid (GMT, stack[last]->G, GMT_DUPLICATE_ALLOC);
		gmt_get_cellarea (GMT, W);
	}

	med = (gmt_grdfloat) gmt_grd_median (GMT, stack[last]->G, W, true);

	if (W) gmt_free_grid (GMT, &W, true);

	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = med;
}

GMT_LOCAL void grdmath_MEDIANW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: MEDIANW 2 1 Weighted median value of A for weights in B.  */
	uint64_t node;
	unsigned int prev = last - 1;
	gmt_grdfloat wmed;

	if (stack[prev]->constant) {	/* Trivial case if data are constant */
		for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = (gmt_grdfloat)stack[prev]->factor;
		return;
	}

	wmed = (gmt_grdfloat)gmt_grd_median (GMT, stack[prev]->G, stack[last]->G, true);
	for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = wmed;
}

GMT_LOCAL void grdmath_MIN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: MIN 2 1 Minimum of A and B.  */
	uint64_t node;
	unsigned int prev;
	gmt_grdfloat a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (gmt_grdfloat)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (gmt_grdfloat)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_M_is_fnan (a) || gmt_M_is_fnan (b)) ? GMT->session.f_NaN : MIN (a, b);
	}
}

GMT_LOCAL void grdmath_MOD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: MOD 2 1 A mod B (remainder after floored division).  */
	uint64_t node;
	unsigned int prev;
	double a, b;

	prev = last - 1;
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "using MOD 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (gmt_grdfloat)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (gmt_grdfloat)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)MOD (a, b);
	}
}

GMT_LOCAL void grdmath_MODE (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: MODE 1 1 Mode value (Least Median of Squares) of A.  */
	uint64_t node;
	gmt_grdfloat mode = 0.0;
	struct GMT_GRID *W = NULL;

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)stack[last]->factor;
		return;
	}
	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must use spherical weights */
		W = gmt_duplicate_grid (GMT, stack[last]->G, GMT_DUPLICATE_ALLOC);
		gmt_get_cellarea (GMT, W);
	}

	mode = (gmt_grdfloat)gmt_grd_mode (GMT, stack[last]->G, W, true);

	if (W) gmt_free_grid (GMT, &W, true);

	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = mode;
}

GMT_LOCAL void grdmath_MODEW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: MODEW 2 1 Weighted mode value of A for weights in B.  */
	uint64_t node;
	unsigned int prev = last - 1;
	gmt_grdfloat wmode;

	if (stack[prev]->constant) {	/* Trivial case if data are constant */
		for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = (gmt_grdfloat)stack[prev]->factor;
		return;
	}

	wmode = (gmt_grdfloat)gmt_grd_mode (GMT, stack[prev]->G, stack[last]->G, true);
	for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = wmode;
}

GMT_LOCAL void grdmath_MUL (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: MUL 2 1 A * B.  */
	uint64_t node;
	unsigned int prev;
	double a, b;

	prev = last - 1;
	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "MUL: Operand one == 0!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "MUL: Operand two == 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)(a * b);
	}
}

GMT_LOCAL void grdmath_NAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: NAN 2 1 NaN if A == B, else A.  */
	uint64_t node;
	unsigned int prev;
	gmt_grdfloat a = 0.0f, b = 0.0;

	prev = last - 1;
	if (stack[prev]->constant) a = (gmt_grdfloat)stack[prev]->factor;
	if (stack[last]->constant) b = (gmt_grdfloat)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) a = stack[prev]->G->data[node];
		if (!stack[last]->constant) b = stack[last]->G->data[node];
		stack[prev]->G->data[node] = (a == b) ? GMT->session.f_NaN : a;
	}
}

GMT_LOCAL void grdmath_NEG (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: NEG 1 1 -A.  */
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "NEG: Operand == 0!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)-stack[last]->factor;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : -stack[last]->G->data[node];
}

GMT_LOCAL void grdmath_NEQ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: NEQ 2 1 1 if A != B, else 0.  */
	uint64_t node;
	unsigned int prev;
	gmt_grdfloat a, b;
	gmt_M_unused(GMT);

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (gmt_grdfloat)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (gmt_grdfloat)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)(a != b);
	}
}

GMT_LOCAL void grdmath_NORM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: NORM 1 1 Normalize (A) so min(A) = 0 and max(A) = 1.  */
	uint64_t node, n = 0;
	openmp_int row, col;
	gmt_grdfloat z, zmin = FLT_MAX, zmax = -FLT_MAX;
	double a;

	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "NORM of a constant gives NaN!\n");
		a = GMT->session.d_NaN;
	}
	else {
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			z = stack[last]->G->data[node];
			if (gmt_M_is_fnan (z)) continue;
			if (z < zmin) zmin = z;
			if (z > zmax) zmax = z;
			n++;
		}
		a = (n == 0 || zmax == zmin) ? GMT->session.f_NaN : (1.0 / (zmax - zmin));	/* Normalization scale */
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : a * (stack[last]->G->data[node] - zmin));
}

GMT_LOCAL void grdmath_NOT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: NOT 1 1 NaN if A == NaN, 1 if A == 0, else 0.  */
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "NOT: Operand == 0!\n");
	if (stack[last]->constant) a = (fabs (stack[last]->factor) > GMT_CONV8_LIMIT) ? 0.0 : 1.0;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : ((fabsf (stack[last]->G->data[node]) > GMT_CONV8_LIMIT) ? 0.0f : 1.0f);
}

GMT_LOCAL void grdmath_NRAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: NRAND 2 1 Normal, random values with mean A and std. deviation B.  */
	uint64_t node;
	unsigned int prev;
	double a = 0.0, b = 0.0;

	prev = last - 1;
	if (stack[prev]->constant) a = stack[prev]->factor;
	if (stack[last]->constant) b = stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) a = stack[prev]->G->data[node];
		if (!stack[last]->constant) b = stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)(a + b * gmt_nrand (GMT));
	}
}

GMT_LOCAL void grdmath_OR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: OR 2 1 NaN if B == NaN, else A.  */
	uint64_t node;
	unsigned int prev;
	gmt_grdfloat a, b;

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? (gmt_grdfloat)stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? (gmt_grdfloat)stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_M_is_fnan (a) || gmt_M_is_fnan (b)) ? GMT->session.f_NaN : a;
	}
}

GMT_LOCAL void grdmath_PDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: PDIST 1 1 Compute minimum distance (in km if -fg) from points in ASCII file A.  */
	int64_t node, row, col;			/* int since VS 2013/OMP 2.0 doesn't allow unsigned index variables */
	uint64_t dummy[2];
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;

	if ((D = grdmath_ASCII_read (GMT, info, GMT_IS_POINT, "PDIST")) == NULL) return;

	T = D->table[0];	/* Only one table in a single file */

#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,dummy) shared(info,stack,last,GMT,T)
#endif
	for (row = 0; row < (openmp_int)info->G->header->my; row++) {
		node = row * info->G->header->mx;
		for (col = 0; col < (openmp_int)info->G->header->mx; col++, node++) {
			stack[last]->G->data[node] = (gmt_grdfloat)gmt_mindist_to_point (GMT, info->d_grd_x[col], info->d_grd_y[row], T, dummy);
		}
	}
	grdmath_ASCII_free (GMT, info, &D, "PDIST");	/* Free memory used for points */
}

GMT_LOCAL void grdmath_PDIST2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: PDIST2 2 1 As PDIST, from points in ASCII file B but only to nodes where A != 0.  */
	uint64_t node, dummy[2];	
	openmp_int row, col;
	unsigned int prev;
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;

	if ((D = grdmath_ASCII_read (GMT, info, GMT_IS_POINT, "PDIST")) == NULL) return;

	T = D->table[0];	/* Only one table in a single file */
	prev = last - 1;

#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,dummy) shared(info,stack,prev,GMT,T)
#endif
	for (row = 0; row < (openmp_int)info->G->header->my; row++) {
		node = row * info->G->header->mx;
		for (col = 0; col < (openmp_int)info->G->header->mx; col++, node++) {
			if (stack[prev]->G->data[node] == 0.0)
				stack[prev]->G->data[node] = GMT->session.f_NaN;
			else
				stack[prev]->G->data[node] = (gmt_grdfloat)gmt_mindist_to_point (GMT, info->d_grd_x[col], info->d_grd_y[row], T, dummy);
		}
	}

	grdmath_ASCII_free (GMT, info, &D, "PDIST2");	/* Free memory used for points */
}

GMT_LOCAL void grdmath_PERM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: PERM 2 1 Permutations n_P_r, with n = A and r = B.  */
	uint64_t node;
	openmp_int row, col;
	unsigned int prev = last - 1, error = 0;
	double a, b;

	if (stack[prev]->constant && stack[prev]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument n to PERM must be a positive integer (n >= 0)!\n");
		error++;
	}
	if (stack[last]->constant && stack[last]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to PERM must be a positive integer (r >= 0)!\n");
		error++;
	}
	if (error || (stack[prev]->constant && stack[last]->constant)) {	/* PERM is undefined */
		gmt_grdfloat value = (error) ? GMT->session.f_NaN : (gmt_grdfloat)gmt_permutation (GMT, irint(stack[prev]->factor), irint(stack[last]->factor));
		gmt_M_grd_loop (GMT, info->G, row, col, node) stack[prev]->G->data[node] = value;
		return;
	}
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
			stack[prev]->G->data[node] = (gmt_grdfloat)gmt_permutation (GMT, irint(a), irint(b));
		}
	}
}

GMT_LOCAL void grdmath_POP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: POP 1 0 Delete top element from the stack.  */
{
	gmt_M_unused(GMT); gmt_M_unused(info); gmt_M_unused(stack); gmt_M_unused(last);
	/* Dummy routine that does nothing but consume the top element of stack */
}

GMT_LOCAL void grdmath_PLM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PLM 3 1 Associated Legendre polynomial P(A) degree B order C.  */
{
	int64_t node;	/* Because of Win OpenMP */
	unsigned int prev = last - 1, first = last - 2;
	int L, M;
	double a = 0.0;
	/* last holds the order M , prev holds the degree L, first holds the argument x = cos(colat) */

	if (!(stack[prev]->constant && stack[last]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "L and M must be constants in PLM (no calculations performed)\n");
		return;
	}

	L = irint (stack[prev]->factor);
	M = irint (stack[last]->factor);

	if (stack[first]->constant) {
		a = gmt_plm (GMT, L, M, stack[first]->factor);
		for (node = 0; node < (int64_t)info->size; node++) stack[first]->G->data[node] = (gmt_grdfloat)a;
	}
	else {
#ifdef _OPENMP
#pragma omp parallel for private(node) shared(info,stack,first,GMT,L,M)
#endif
		for (node = 0; node < (int64_t)info->size; node++)
			stack[first]->G->data[node] = (gmt_grdfloat)gmt_plm (GMT, L, M, stack[first]->G->data[node]);
	}
}


GMT_LOCAL void grdmath_PLMg (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PLMg 3 1 Normalized associated Legendre polynomial P(A) degree B order C (geophysical convention).  */
{
	int64_t node;	/* Because of Win OpenMP */
	unsigned int prev = last - 1, first = last - 2;
	int L, M;
	double a = 0.0;
	/* last holds the order M, prev holds the degree L, first holds the argument x = cos(colat) */

	if (!(stack[prev]->constant && stack[last]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "L and M must be constants in PLMg (no calculations performed)\n");
		return;
	}

	L = irint (stack[prev]->factor);
	M = irint (stack[last]->factor);

	if (stack[first]->constant) {
		a = gmt_plm_bar (GMT, L, M, stack[first]->factor, false);
		for (node = 0; node < (int64_t)info->size; node++) stack[first]->G->data[node] = (gmt_grdfloat)a;
	}
	else {
#ifdef _OPENMP
#pragma omp parallel for private(node) shared(info,stack,first,GMT,L,M)
#endif
		for (node = 0; node < (int64_t)info->size; node++)
			stack[first]->G->data[node] = (gmt_grdfloat)gmt_plm_bar (GMT, L, M, stack[first]->G->data[node], false);
	}
}

GMT_LOCAL void grdmath_POINT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: POINT 1 2 Return mean_x mean_y of points in ASCII file A.  */
{
	uint64_t node, n = 0;
	unsigned int next = last + 1;
	double *x = NULL, *y = NULL, pos[2];
	struct GMT_DATATABLE *T = NULL;
	struct GMT_DATASET *D = NULL;
	int geo = gmt_M_is_geographic (GMT, GMT_IN) ? 1 : 0;

	/* Read a table and compute mean location */
	if ((D = grdmath_ASCII_read (GMT, info, GMT_IS_POINT, "POINT")) == NULL) return;
	T = D->table[0];	/* Only one table in a single file */
	if (T->n_records == 1) {	/* Got a single point record; no need to average etc */
		pos[GMT_X] = T->segment[0]->data[GMT_X][0];
		pos[GMT_Y] = T->segment[0]->data[GMT_Y][0];
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "[Single point reported as %g %g]\n", pos[GMT_X], pos[GMT_Y]);
	}
	else {	/* Must compute mean point */
		if (T->n_segments > 1) {	/* Must build single x,y arrays for gmt_mean_point */
			uint64_t seg;
			size_t n_alloc = 0;
			gmt_M_malloc2 (GMT, x, y, T->n_records, &n_alloc, double);		/* Allocate one long array for each */
			for (seg = 0; seg < T->n_segments; seg++) {
				gmt_M_memcpy (&x[n], T->segment[seg]->data[GMT_X], T->segment[seg]->n_rows, double);
				gmt_M_memcpy (&y[n], T->segment[seg]->data[GMT_Y], T->segment[seg]->n_rows, double);
				n += T->segment[seg]->n_rows;
			}
		}
		else {	/* Just a single segment, use pointers */
			x = T->segment[0]->data[GMT_X];
			y = T->segment[0]->data[GMT_Y];
			n = T->segment[0]->n_rows;
		}
		gmt_mean_point (GMT, x, y, n, geo, pos);	/* Get mean location */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "[Mean point computed as %g %g]\n", pos[GMT_X], pos[GMT_Y]);
	}
	/* Place mean x and y on the stack */
	stack[last]->constant = true;
	stack[last]->factor = pos[GMT_X];
	/* The last stack needs to be filled */
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)stack[last]->factor;
	stack[next]->constant = true;
	stack[next]->factor = pos[GMT_Y];
	/* The next stack needs to be filled */
	for (node = 0; node < info->size; node++) stack[next]->G->data[node] = (gmt_grdfloat)stack[next]->factor;
	if (T->n_segments > 1) {	/* Free what we allocated */
		gmt_M_free (GMT, x);
		gmt_M_free (GMT, y);
	}
	grdmath_ASCII_free (GMT, info, &D, "POINT");	/* Free memory used for points */
}

GMT_LOCAL void grdmath_POW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: POW 2 1 A ^ B.  */
{
	uint64_t node;
	unsigned int prev;
	double a, b;

	prev = last - 1;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "POW: Operand one == 0!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "POW: Operand two == 0!\n");
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)pow (a, b);
	}
}

GMT_LOCAL gmt_grdfloat grdmath_wquant_sub (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *G, struct GMT_GRID *W, double q, bool use_grid, double weight) {
	uint64_t node, n = 0;
	openmp_int row, col;
	gmt_grdfloat p;
	double w = 1.0;
	struct GMT_OBSERVATION *pair = gmt_M_memory (GMT, NULL, info->nm, struct GMT_OBSERVATION);
	/* 1. Create array of value,weight pairs, skipping NaNs */
	if (!use_grid) w = weight;
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		if (gmt_M_is_fnan (G->data[node])) continue;
		if (use_grid) {
			if (gmt_M_is_dnan (W->data[node]))
				continue;
			else
				w = W->data[node];
		}
		pair[n].value    = G->data[node];
		pair[n++].weight = (gmt_grdfloat)w;
	}
	/* 2. Find the weighted quantile */
	p = (gmt_grdfloat)gmt_quantile_weighted (GMT, pair, n, 0.01*q);
	gmt_M_free (GMT, pair);
	return p;
}

GMT_LOCAL void grdmath_PQUANT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PQUANT 2 1 The B'th Quantile (0-100%) of A.  */
{
	uint64_t node;
	unsigned int prev, pad[4];
	gmt_grdfloat p;

	prev  = last - 1;	/* last holds the selected quantile (0-100), prev the data % */
	if (!stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "PQUANT must be given a constant quantile (no calculations performed)\n");
		return;
	}
	if (stack[last]->factor < 0.0 || stack[last]->factor > 100.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "PQUANT must be given a constant quantile between 0-100%% (no calculations performed)\n");
		return;
	}
	if (stack[prev]->constant) {	/* Trivial case */
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "PQUANT of a constant is set to NaN\n");
		p = GMT->session.f_NaN;
	}
	else if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must use spherical weights */
		struct GMT_GRID *W = gmt_duplicate_grid (GMT, stack[prev]->G, GMT_DUPLICATE_ALLOC);
		gmt_get_cellarea (GMT, W);
		p = grdmath_wquant_sub (GMT, info, stack[prev]->G, W, stack[last]->factor, true, 0.0);
		gmt_free_grid (GMT, &W, true);
	}
	else {
		gmt_M_memcpy (pad, stack[prev]->G->header->pad, 4U, unsigned int);	/* Save original pad */
		gmt_grd_pad_off (GMT, stack[prev]->G);				/* Undo pad if one existed so we can sort */
		gmt_sort_array (GMT, stack[prev]->G->data, info->nm, GMT_FLOAT);
		p = (gmt_grdfloat) gmt_quantile_f (GMT, stack[prev]->G->data, stack[last]->factor, info->nm);
		gmt_M_memset (stack[prev]->G->data, info->size, gmt_grdfloat);	/* Wipes everything */
		gmt_grd_pad_on (GMT, stack[prev]->G, pad);		/* Reinstate the original pad */
	}

	for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = p;
}

GMT_LOCAL void grdmath_PQUANTW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PQUANTW 3 1 The C'th Quantile (0-100%) of A for weights in B.  */
{
	uint64_t node;
	unsigned int prev = last - 1, prev2 = last - 2;
	gmt_grdfloat p;

	if (!stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "PQUANTW must be given a constant quantile (no calculations performed)\n");
		return;
	}
	if (stack[last]->factor < 0.0 || stack[last]->factor > 100.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "PQUANTW must be given a constant quantile between 0-100%% (no calculations performed)\n");
		return;
	}
	if (stack[prev2]->constant) {	/* Trivial case */
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "PQUANTW of a constant is set to NaN\n");
		p = GMT->session.f_NaN;
	}
	else
		p = grdmath_wquant_sub (GMT, info, stack[prev2]->G, stack[prev]->G, stack[last]->factor, !stack[prev]->constant, stack[prev]->factor);
	for (node = 0; node < info->size; node++) stack[prev2]->G->data[node] = p;
}

GMT_LOCAL void grdmath_PSI (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PSI 1 1 Psi (or Digamma) of A.  */
{
	int64_t node;
	gmt_grdfloat a = 0.0;
	double x[2];

	x[1] = 0.0;	/* No imaginary part */
	if (stack[last]->constant) {
		x[0] = stack[last]->factor;
		a = (gmt_grdfloat)gmt_psi (GMT, x, NULL);
		for (node = 0; node < (int64_t)info->size; node++)
			stack[last]->G->data[node] = a;
	}
	else {
#ifdef _OPENMP
#pragma omp parallel for private(node) firstprivate(x) shared(info,stack,last,GMT)
#endif
		for (node = 0; node < (int64_t)info->size; node++) {
			x[0] = stack[last]->G->data[node];
			stack[last]->G->data[node] = (gmt_grdfloat)gmt_psi (GMT, x, NULL);
		}
	}
}

GMT_LOCAL void grdmath_PVQV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last, unsigned int kind)
{
	bool calc;
	unsigned int prev = last - 1, first = last - 2, n;
	uint64_t node;
	gmt_grdfloat a = 0.0;
	double x = 0.0, nu[2], pq[4];
	static char *name[2] = {"PV", "QV"};
	/* last holds the imaginary order vi, prev holds the real order vr, first holds the argument x = cos(colat) */

	calc = !(stack[prev]->constant && stack[last]->constant && stack[first]->constant);	/* Only constant if all args are constant */
	if (!calc) {	/* All constants */
		nu[0] = stack[prev]->factor;
		nu[1] = stack[last]->factor;
		if ((stack[first]->factor < -1.0 || stack[first]->factor > 1.0)) GMT_Report (GMT->parent, GMT_MSG_WARNING, "argument to %s outside domain!\n", name[kind]);
		gmt_PvQv (GMT, stack[first]->factor, nu, pq, &n);
		a = (gmt_grdfloat)pq[2*kind];
		for (node = 0; node < info->size; node++)
			stack[first]->G->data[node] = a;
	}
	else {	/* Must evaluate GMT_PvQv repeatedly */
		kind *= 2;
		for (node = 0; node < info->size; node++) {
			nu[0] = (stack[prev]->constant)  ? stack[prev]->factor  : stack[prev]->G->data[node];
			nu[1] = (stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node];
			x     = (stack[first]->constant) ? stack[first]->factor : stack[first]->G->data[node];
			gmt_PvQv (GMT, x, nu, pq, &n);
			stack[first]->G->data[node] = (gmt_grdfloat)pq[kind];
		}
	}
}

GMT_LOCAL void grdmath_PV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: PV 3 1 Legendre function Pv(A) of degree v = real(B) + imag(C).  */
{
	grdmath_PVQV (GMT, info, stack, last, 0);
}

GMT_LOCAL void grdmath_QV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: QV 3 1 Legendre function Qv(A) of degree v = real(B) + imag(C).  */
{
	grdmath_PVQV (GMT, info, stack, last, 1);
}

GMT_LOCAL void grdmath_R2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: R2 2 1 R2 = A^2 + B^2.  */
{
	uint64_t node;
	unsigned int prev;
	double a = 0.0, b = 0.0;

	prev = last - 1;
	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "R2: Operand one == 0!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_DEBUG, "R2: Operand two == 0!\n");
	if (stack[prev]->constant) a = stack[prev]->factor * stack[prev]->factor;
	if (stack[last]->constant) b = stack[last]->factor * stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) a = stack[prev]->G->data[node] * stack[prev]->G->data[node];
		if (!stack[last]->constant) b = stack[last]->G->data[node] * stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)(a + b);
	}
}

GMT_LOCAL void grdmath_R2D (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: R2D 1 1 Convert Radians to Degrees.  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (stack[last]->constant) a = R2D * stack[last]->factor;
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : R2D * stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_RAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
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
		stack[prev]->G->data[node] = (gmt_grdfloat)(a + gmt_rand (GMT) * (b - a));
	}
}

GMT_LOCAL void grdmath_RCDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: RCDF 1 1 Rayleigh cumulative distribution function for z = A.  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = 1.0 - exp (-0.5*stack[last]->factor*stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : 1.0 - expf (-0.5f*stack[last]->G->data[node]*stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_RCRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: RCRIT 1 1 Rayleigh distribution critical value for alpha = A.  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = M_SQRT2 * sqrt (-log (1.0 - stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : M_SQRT2 * sqrtf (-logf (1.0f - stack[last]->G->data[node])));
}

GMT_LOCAL void grdmath_RGB2HSV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: RGB2HSV 3 3 Convert rgb to hsv, with r = A, g = B and b = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double rgb[4], hsv[4];

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && (stack[prev2]->factor < 0.0 || stack[prev2]->factor > 255.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to RGB2HSV must be a 0 <= r <= 255!\n");
		error++;
	}
	if (stack[prev1]->constant && (stack[prev1]->factor < 0.0 || stack[prev1]->factor > 255.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument g to RGB2HSV must be a 0 <= g <= 255!\n");
		error++;
	}
	if (stack[last]->constant  && (stack[last]->factor < 0.0 || stack[last]->factor > 255.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument b to RGB2HSV must be a 0 <= b <= 255!\n");
		error++;
	}
	rgb[3] = hsv[3] = 0.0;	/* No transparency involved */
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* Constant arguments */
		rgb[0] = gmt_M_is255 (stack[prev2]->factor);
		rgb[1] = gmt_M_is255 (stack[prev1]->factor);
		rgb[2] = gmt_M_is255 (stack[last]->factor);
		gmt_rgb_to_hsv (rgb, hsv);
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			stack[prev2]->G->data[node] = (gmt_grdfloat)hsv[0];
			stack[prev1]->G->data[node] = (gmt_grdfloat)hsv[1];
			stack[last]->G->data[node]  = (gmt_grdfloat)hsv[2];
		}
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		rgb[0] = gmt_M_is255 ((stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node]);
		rgb[1] = gmt_M_is255 ((stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node]);
		rgb[2] = gmt_M_is255 ((stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node]);
		gmt_rgb_to_hsv (rgb, hsv);
		stack[prev2]->G->data[node] = (gmt_grdfloat)hsv[0];
		stack[prev1]->G->data[node] = (gmt_grdfloat)hsv[1];
		stack[last]->G->data[node]  = (gmt_grdfloat)hsv[2];
	}
}

GMT_LOCAL void grdmath_RGB2LAB (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: RGB2LAB 3 3 Convert rgb to lab, with r = A, g = B and b = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double rgb[3], lab[3];

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && (stack[prev2]->factor < 0.0 || stack[prev2]->factor > 255.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to RGB2LAB must be a 0 <= r <= 255!\n");
		error++;
	}
	if (stack[prev1]->constant && (stack[prev1]->factor < 0.0 || stack[prev1]->factor > 255.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument g to RGB2LAB must be a 0 <= g <= 255!\n");
		error++;
	}
	if (stack[last]->constant  && (stack[last]->factor < 0.0 || stack[last]->factor > 255.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument b to RGB2LAB must be a 0 <= b <= 255!\n");
		error++;
	}
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* Constant arguments */
		rgb[0] = gmt_M_is255 (stack[prev2]->factor);
		rgb[1] = gmt_M_is255 (stack[prev1]->factor);
		rgb[2] = gmt_M_is255 (stack[last]->factor);
		gmt_rgb_to_lab (rgb, lab);
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			stack[prev2]->G->data[node] = (gmt_grdfloat)lab[0];
			stack[prev1]->G->data[node] = (gmt_grdfloat)lab[1];
			stack[last]->G->data[node]  = (gmt_grdfloat)lab[2];
		}
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		rgb[0] = gmt_M_is255 ((stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node]);
		rgb[1] = gmt_M_is255 ((stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node]);
		rgb[2] = gmt_M_is255 ((stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node]);
		gmt_rgb_to_lab (rgb, lab);
		stack[prev2]->G->data[node] = (gmt_grdfloat)lab[0];
		stack[prev1]->G->data[node] = (gmt_grdfloat)lab[1];
		stack[last]->G->data[node]  = (gmt_grdfloat)lab[2];
	}
}

GMT_LOCAL void grdmath_RGB2XYZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: RGB2XYZ 3 3 Convert rgb to xyz, with r = A, g = B and b = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double rgb[3], xyz[3];

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev2]->constant && (stack[prev2]->factor < 0.0 || stack[prev2]->factor > 255.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument r to RGB2XYZ must be a 0 <= r <= 255!\n");
		error++;
	}
	if (stack[prev1]->constant && (stack[prev1]->factor < 0.0 || stack[prev1]->factor > 255.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument g to RGB2XYZ must be a 0 <= g <= 255!\n");
		error++;
	}
	if (stack[last]->constant  && (stack[last]->factor < 0.0 || stack[last]->factor > 255.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument b to RGB2XYZ must be a 0 <= b <= 255!\n");
		error++;
	}
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* Constant arguments */
		rgb[0] = gmt_M_is255 (stack[prev2]->factor);
		rgb[1] = gmt_M_is255 (stack[prev1]->factor);
		rgb[2] = gmt_M_is255 (stack[last]->factor);
		gmt_rgb_to_xyz (rgb, xyz);
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			stack[prev2]->G->data[node] = (gmt_grdfloat)xyz[0];
			stack[prev1]->G->data[node] = (gmt_grdfloat)xyz[1];
			stack[last]->G->data[node]  = (gmt_grdfloat)xyz[2];
		}
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		rgb[0] = gmt_M_is255 ((stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node]);
		rgb[1] = gmt_M_is255 ((stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node]);
		rgb[2] = gmt_M_is255 ((stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node]);
		gmt_rgb_to_xyz (rgb, xyz);
		stack[prev2]->G->data[node] = (gmt_grdfloat)xyz[0];
		stack[prev1]->G->data[node] = (gmt_grdfloat)xyz[1];
		stack[last]->G->data[node]  = (gmt_grdfloat)xyz[2];
	}
}

GMT_LOCAL void grdmath_RINT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: RINT 1 1 rint (A) (round to integral value nearest to A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)rint (stack[last]->factor);
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : rintf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_RMS (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: RMS 1 1 Root-mean-square of A.  */
{
	uint64_t node;
	gmt_grdfloat rms = 0.0;
	struct GMT_GRID *W = NULL;
	gmt_M_unused(GMT);

	if (stack[last]->constant) {	/* Trivial case */
		rms = (gmt_grdfloat)stack[last]->factor;
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = rms;
		return;
	}

	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must use spherical weights */
		W = gmt_duplicate_grid (GMT, stack[last]->G, GMT_DUPLICATE_ALLOC);
		gmt_get_cellarea (GMT, W);
	}

	rms = (gmt_grdfloat)gmt_grd_std (GMT, stack[last]->G, W);

	if (W) gmt_free_grid (GMT, &W, true);

	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = rms;
}

GMT_LOCAL void grdmath_RMSW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: RMSW 2 1 Weighted Root-mean-square of A for weights in B.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	gmt_grdfloat rms;
	gmt_M_unused(GMT);

	if (stack[prev]->constant) {	/* Trivial case */
		rms = (gmt_grdfloat)stack[prev]->factor;
		for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = rms;
		return;
	}

	rms = (gmt_grdfloat)gmt_grd_rms (GMT, stack[prev]->G, stack[last]->G);

	for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = rms;
}

GMT_LOCAL void grdmath_RPDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: RPDF 1 1 Rayleigh probability density function for z = A.  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = stack[last]->factor * exp (-0.5 * stack[last]->factor * stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : stack[last]->G->data[node] * expf (-0.5f * stack[last]->G->data[node] * stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_assign_grdstack (struct GRDMATH_STACK *Sto, struct GRDMATH_STACK *Sfrom)
{	/* Copy contents of Sfrom to Sto */
	Sto->G          = Sfrom->G;
	Sto->constant   = Sfrom->constant;
	Sto->factor     = Sfrom->factor;
}

GMT_LOCAL void grdmath_ROLL (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ROLL 2 0 Cyclicly shifts the top A stack items by an amount B.  */
{
	unsigned int prev, top, bottom, k, kk, n_items;
	int n_shift;
	struct GRDMATH_STACK Stmp;
	gmt_M_unused(GMT); gmt_M_unused(info);
	assert (last > 2);	/* Must have at least 3 items on the stack: A single item plus the two roll arguments */
	prev = last - 1;	/* This gives the number of stack items to include in the cycle */
	if (!(stack[last]->constant && stack[prev]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Length and shift must be constants in ROLL!\n");
		return;
	}
	n_items = urint (stack[prev]->factor);
	n_shift = irint (stack[last]->factor);
	if (n_items > prev) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Items on stack is fewer than required by ROLL!\n");
		return;
	}
	top = prev - 1;
	bottom = prev - n_items;
	for (k = 0; k < (unsigned int)abs (n_shift); k++) {	/* Do the cyclical shift */
		if (n_shift > 0) {	/* Positive roll */
			grdmath_assign_grdstack (&Stmp, stack[top]);	/* Keep copy of top item */
			for (kk = 1; kk < n_items; kk++)	/* Move all others up one step */
				grdmath_assign_grdstack (stack[top-kk+1], stack[top-kk]);
			grdmath_assign_grdstack (stack[bottom], &Stmp);	/* Place copy on bottom */
		}
		else if (n_shift < 0) {	/* Negative roll */
			grdmath_assign_grdstack (&Stmp, stack[bottom]);	/* Keep copy of bottom item */
			for (kk = 1; kk < n_items; kk++)	/* Move all others down one step */
				grdmath_assign_grdstack (stack[bottom+kk-1], stack[bottom+kk]);
			grdmath_assign_grdstack (stack[top], &Stmp);	/* Place copy on top */
		}
	}
	return;
}

GMT_LOCAL void grdmath_ROTX (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: ROTX 2 1 Rotate A by the (constant) shift B in x-direction.  */
	uint64_t node;
	unsigned int prev = last - 1;
	openmp_int col, row, *new_col = NULL, n_columns;
	int colx, shift;
	gmt_grdfloat *z = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (info->G->header);

	/* Shift grid A by the x-shift B.  B must be a constant */

	if (!stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "DX shift (B) must be a constant in ROTX (no calculations performed)\n");
		return;
	}
	shift = irint (stack[last]->factor * HH->r_inc[GMT_X]);	/* Shift of nodes */

	if (stack[prev]->constant || !shift) return;	/* Trivial since A is a constant or shift is zero */
	if (shift < 0) shift += info->G->header->n_columns;	/* Same thing */
	n_columns = (openmp_int)info->G->header->n_columns;
	/* Set up permutation vector */

	new_col = gmt_M_memory (GMT, NULL, n_columns, openmp_int);
	z = gmt_M_memory (GMT, NULL, n_columns, gmt_grdfloat);
	for (col = colx = 0; col < (openmp_int)info->G->header->n_columns; col++, colx++) new_col[colx] = (colx + shift) % info->G->header->n_columns;	/* Move by shift but rotate around */
	gmt_M_row_loop (GMT, info->G, row) {	/* For each row */
		gmt_M_col_loop (GMT, info->G, row, col, node) z[new_col[col]] = stack[prev]->G->data[node];	/* Copy one row of data to z with shift */
		node = gmt_M_ijp (info->G->header, row, 0);		/* First col */
		gmt_M_memcpy (&stack[prev]->G->data[node], z, n_columns, gmt_grdfloat);	/* Replace this row */
	}
	gmt_M_free (GMT, z);
	gmt_M_free (GMT, new_col);
}

GMT_LOCAL void grdmath_ROTY (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: ROTY 2 1 Rotate A by the (constant) shift B in y-direction.  */
	unsigned int prev = last - 1;
	openmp_int row, col, *new_row = NULL;
	int rowx, shift;
	gmt_grdfloat *z = NULL;

	/* Shift grid A by the y-shift B.  B must be a constant */

	if (!stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "DY shift (B) must be a constant in ROTY (no calculations performed)\n");
		return;
	}
	shift = irint (stack[last]->factor / info->G->header->inc[GMT_Y]);	/* Shift of nodes */

	if (stack[prev]->constant || !shift) return;	/* Trivial since A is a constant or shift is zero */
	if (shift < 0) shift += info->G->header->n_rows;	/* Same thing */
	/* Set up permutation vector */

	new_row = gmt_M_memory (GMT, NULL, info->G->header->n_rows, openmp_int);
	z = gmt_M_memory (GMT, NULL, info->G->header->n_rows, gmt_grdfloat);
	for (row = rowx = 0; row < (openmp_int)info->G->header->n_rows; row++, rowx++) new_row[rowx] = (rowx + info->G->header->n_rows - shift) % info->G->header->n_rows;	/* Move by shift but rotate around */
	for (col = 0; col < (openmp_int)info->G->header->n_columns; col++) {	/* For each column */
		for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) z[new_row[row]] = stack[prev]->G->data[gmt_M_ijp(info->G->header, row, col)];	/* Copy one column of data to z with shift */
		for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) stack[prev]->G->data[gmt_M_ijp(info->G->header, row, col)] = z[row];	/* Replace this column */
	}
	gmt_M_free (GMT, z);
	gmt_M_free (GMT, new_row);
}

GMT_LOCAL void grdmath_SDIST (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: SDIST 2 1 Spherical distance (in km) between grid nodes and stack lon,lat (A, B).  */
	int error = GMT_NOERROR;
	uint64_t node;	
	openmp_int row, col;
	unsigned int prev = last - 1;
	double x0, y0;

	if (gmt_M_is_geographic (GMT, GMT_IN))
		error = gmt_init_distaz (GMT, 'k', gmt_M_sph_mode (GMT), GMT_MAP_DIST);
	else {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid must be geographic; see CDIST for Cartesian data.\n");
		return;
	}
	if (error == GMT_NOT_A_VALID_TYPE) return;

#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,x0,y0) shared(info,stack,prev,last,GMT)
#endif
	for (row = 0; row < (openmp_int)info->G->header->my; row++) {
		node = row * info->G->header->mx;
		for (col = 0; col < (openmp_int)info->G->header->mx; col++, node++) {
			x0 = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			y0 = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
			stack[prev]->G->data[node] = (gmt_grdfloat) gmt_distance (GMT, x0, y0, info->d_grd_x[col], info->d_grd_y[row]);
		}
	}
}

GMT_LOCAL void grdmath_SDIST2 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last) {
/*OPERATOR: SDIST2 2 1 As SDIST but only to nodes that are != 0.  */
	int error = GMT_NOERROR;
	uint64_t node;	
	openmp_int row, col;
	unsigned int prev = last - 1;
	double x0, y0;

	if (gmt_M_is_geographic (GMT, GMT_IN))
		error = gmt_init_distaz (GMT, 'k', gmt_M_sph_mode (GMT), GMT_MAP_DIST);
	else {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Grid must be geographic; see CDIST2 for Cartesian data.\n");
		return;
	}
	if (error == GMT_NOT_A_VALID_TYPE) return;

#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,x0,y0) shared(info,stack,prev,last,GMT)
#endif
	for (row = 0; row < (openmp_int)info->G->header->my; row++) {
		node = row * info->G->header->mx;
		for (col = 0; col < (openmp_int)info->G->header->mx; col++, node++) {
			if (stack[prev]->G->data[node] == 0.0)
				stack[prev]->G->data[node] = GMT->session.f_NaN;
			else {
				x0 = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
				y0 = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
				stack[prev]->G->data[node] = (gmt_grdfloat) gmt_distance (GMT, x0, y0, info->d_grd_x[col], info->d_grd_y[row]);
			}
		}
	}
}

GMT_LOCAL void grdmath_AZ_sub (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last, bool reverse) {
	uint64_t node;	
	openmp_int row, col;
	unsigned int prev = last - 1;
	double x0 = 0.0, y0 = 0.0, az;

	gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_ANGLE);
	if (gmt_init_distaz (GMT, 'd', gmt_M_sph_mode (GMT), GMT_MAP_DIST) == GMT_NOT_A_VALID_TYPE) return;
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,x0,y0,az) shared(info,stack,prev,last,GMT,reverse)
#endif
	for (row = 0; row < (openmp_int)info->G->header->my; row++) {
		node = row * info->G->header->mx;
		for (col = 0; col < (openmp_int)info->G->header->mx; col++, node++) {
			x0 = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			y0 = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
			az = gmt_az_backaz (GMT, info->d_grd_x[col], info->d_grd_y[row], x0, y0, reverse);
			while (az < -180.0) az += 360.0;
			while (az > +180.0) az -= 360.0;
			stack[prev]->G->data[node] = (gmt_grdfloat)az;
		}
	}
}

GMT_LOCAL void grdmath_SAZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SAZ 2 1 Spherical azimuth from grid nodes to stack x,y.  */
/* Azimuth from grid ones to stack point */
{
	grdmath_AZ_sub (GMT, info, stack, last, false);
}

GMT_LOCAL void grdmath_SBAZ (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SBAZ 2 1 Spherical back-azimuth from grid nodes to stack x,y.  */
/* Azimuth from stack point to grid ones (back azimuth) */
{
	grdmath_AZ_sub (GMT, info, stack, last, true);
}

GMT_LOCAL void grdmath_SEC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SEC 1 1 sec (A) (A in radians).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)(1.0 / cos (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (1.0f / cosf (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_SECD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SECD 1 1 sec (A) (A in degrees).  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = 1.0 / cosd (stack[last]->factor);
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : 1.0 / cosd (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_SECH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SECH 1 1 sech (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)(1.0/cosh (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : 1.0f/coshf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_SIGN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SIGN 1 1 sign (+1 or -1) of A.  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant && stack[last]->factor == 0.0)
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "SIGN: Operand == 0!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)copysign (1.0, stack[last]->factor);
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : copysignf (1.0f, stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_SIN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SIN 1 1 sin (A) (A in radians).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)sin (stack[last]->factor);
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (stack[last]->constant) ? a : sinf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_SINC (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SINC 1 1 sinc (A) (sin (pi*A)/(pi*A)).  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = gmt_sinc (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++)
		stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : gmt_sinc (GMT, stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_SIND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SIND 1 1 sin (A) (A in degrees).  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = sind (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : sind (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_SINH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SINH 1 1 sinh (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)sinh (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : sinhf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_SKEW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SKEW 1 1 Skewness of A.  */
{
	uint64_t node, n = 0;
	openmp_int row, col;
	double mean = 0.0, sum2 = 0.0, skew = 0.0, delta;
	gmt_grdfloat f_skew;

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = GMT->session.f_NaN;
		return;
	}

	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		if (gmt_M_is_fnan (stack[last]->G->data[node])) continue;
		n++;
		delta = stack[last]->G->data[node] - mean;
		mean += delta / n;
		sum2 += delta * (stack[last]->G->data[node] - mean);
	}
	if (n > 1) {
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			if (gmt_M_is_fnan (stack[last]->G->data[node])) continue;
			delta = stack[last]->G->data[node] - mean;
			skew += pow (delta, 3.0);
		}
		sum2 /= (n - 1);
		skew /= n * pow (sum2, 1.5);
		f_skew = (gmt_grdfloat)skew;
	}
	else
		f_skew = GMT->session.f_NaN;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = f_skew;
}

GMT_LOCAL void grdmath_SQR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SQR 1 1 A^2.  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = stack[last]->factor * stack[last]->factor;
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : stack[last]->G->data[node] * stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_SQRT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SQRT 1 1 sqrt (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;

	if (stack[last]->constant && stack[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one < 0!\n");
	if (stack[last]->constant) a = (gmt_grdfloat)sqrt (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : sqrtf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_STD (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: STD 1 1 Standard deviation of A.  */
{
	uint64_t node;
	gmt_grdfloat std;
	struct GMT_GRID *W = NULL;
	gmt_M_unused(GMT);

	if (stack[last]->constant) {	/* Trivial case: std of a constant grid is zero */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = 0.0;
		return;
	}

	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must use spherical weights */
		W = gmt_duplicate_grid (GMT, stack[last]->G, GMT_DUPLICATE_ALLOC);
		gmt_get_cellarea (GMT, W);
	}

	std = (gmt_grdfloat)gmt_grd_std (GMT, stack[last]->G, W);

	if (W) gmt_free_grid (GMT, &W, true);

	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = std;
}

GMT_LOCAL void grdmath_STDW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: STDW 2 1 Weighted standard deviation of A for weights in B.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	gmt_grdfloat std;
	gmt_M_unused(GMT);

	if (stack[prev]->constant)	/* Trivial case: std of constant grid is zero */
		std = 0.0;
	else
		std = (gmt_grdfloat)gmt_grd_std (GMT, stack[prev]->G, stack[last]->G);

	for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = std;
}

GMT_LOCAL void grdmath_STEP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: STEP 1 1 Heaviside step function: H(A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[last]->constant) a = stack[last]->G->data[node];
		if (a == 0.0f)
			stack[last]->G->data[node] = 0.5;
		else
			stack[last]->G->data[node] = (a < 0.0) ? 0.0 : 1.0;
	}
}

GMT_LOCAL void grdmath_STEPX (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: STEPX 1 1 Heaviside step function in x: H(x-A).  */
{
	uint64_t node;
	openmp_int row, col;
	double a;
	gmt_M_unused(GMT);

	grdmath_grd_padloop (GMT, info->G, row, col, node) {
		a = info->d_grd_x[col] - ((stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node]);
		if (a == 0.0)
			stack[last]->G->data[node] = 0.5;
		else
			stack[last]->G->data[node] = (a < 0.0) ? 0.0 : 1.0;
	}
}

GMT_LOCAL void grdmath_STEPY (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: STEPY 1 1 Heaviside step function in y: H(y-A).  */
{
	uint64_t node;
	openmp_int row, col;
	double a;
	gmt_M_unused(GMT);

	grdmath_grd_padloop (GMT, info->G, row, col, node) {
		a = info->d_grd_y[row] - ((stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node]);
		if (a == 0.0)
			stack[last]->G->data[node] = 0.5;
		else
			stack[last]->G->data[node] = (a < 0.0) ? 0.0 : 1.0;
	}
}

GMT_LOCAL void grdmath_SUB (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SUB 2 1 A - B.  */
{
	uint64_t node;
	unsigned int prev;
	double a, b;
	gmt_M_unused(GMT);

	prev = last - 1;
	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		b = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_grdfloat)(a - b);
	}
}

GMT_LOCAL void grdmath_SUM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: SUM 1 1 Sum of all values in A.  */
{
	uint64_t node, n_used = 0;
	double sum = 0.0;
	if (stack[last]->constant)
		sum = stack[last]->factor * stack[last]->G->header->nm;
	else {
		openmp_int row, col;
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			if (gmt_M_is_fnan (stack[last]->G->data[node])) continue;
			sum += stack[last]->G->data[node];
			n_used++;
		}
		if (n_used == 0) sum = GMT->session.d_NaN;
	}
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)sum;
}

GMT_LOCAL void grdmath_TAN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TAN 1 1 tan (A) (A in radians).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)tan (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : tanf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_TAND (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TAND 1 1 tan (A) (A in degrees).  */
{
	uint64_t node;
	double a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = tand (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : tand (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_TANH (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TANH 1 1 tanh (A).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)tanh (stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : tanhf (stack[last]->G->data[node]);
}

GMT_LOCAL void grdmath_TAPER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TAPER 2 1 Unit weights cosine-tapered to zero within A and B of x and y grid margins.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	openmp_int row, col;
	double strip, scale, start, stop, from_start, from_stop, w_y, *w_x = NULL;

	if (!(stack[last]->constant && stack[prev]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "TAPER: Arguments A and B must both be constants\n");
		return;
	}
	if (stack[last]->factor < 0.0 || stack[prev]->factor < 0.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "TAPER: Arguments A and B must both be >= 0\n");
		return;
	}

	/* First compute and store x taper weights: Ramp 0 to 1 for left margin, constant 1, then ramp 1 to 0 for right margin */
	w_x = gmt_M_memory (GMT, NULL, info->G->header->mx, double);
	if (stack[prev]->factor == 0.0) {	/* No taper in x so set weights to 1 */
		grdmath_col_padloop2 (GMT, info->G, col) w_x[col] = 1.0;
	}
	else {
		strip = stack[prev]->factor;
		scale = M_PI / strip;
		start = strip + info->G->header->wesn[XLO];
		stop  = strip - info->G->header->wesn[XHI];
		grdmath_col_padloop2 (GMT, info->G, col) {
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

	grdmath_row_padloop (GMT, info->G, row, node) {
		from_start = start - info->d_grd_y[row];
		if (stack[last]->factor == 0.0) w_y = 1.0;	/* No taper in y-range */
		else if (from_start > 0.0) w_y = 0.5 * (1.0 + cos (from_start * scale));
		else if ((from_stop = stop + info->d_grd_y[row]) > 0.0) w_y = 0.5 * (1.0 + cos (from_stop * scale));
		else w_y = 1.0;	/* Inside non-tapered y-range */
		grdmath_col_padloop (GMT, info->G, col, node) {
			stack[prev]->G->data[node] = (gmt_grdfloat)(w_y * w_x[col]);
		}
	}
	gmt_M_free (GMT, w_x);
}

GMT_LOCAL void grdmath_TN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TN 2 1 Chebyshev polynomial Tn(-1<t<+1,n), with t = A, and n = B.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	int n;
	double a = 0.0, t;

	for (node = 0; node < info->size; node++) {
		a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
		n = irint ((stack[last]->constant) ? stack[last]->factor : (double)stack[last]->G->data[node]);
		gmt_chebyshev (GMT, a, n, &t);
		stack[prev]->G->data[node] = (gmt_grdfloat)t;
	}
}

GMT_LOCAL void grdmath_TCRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TCRIT 2 1 Student's t-distribution critical value for alpha = A and nu = B.  */
{
	uint64_t node;
	int b;
	unsigned int prev = last - 1;
	openmp_int row, col;
	double a;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for TCRIT!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for TCRIT!\n");
	if (stack[prev]->constant && stack[last]->constant) {	/* Compute once then copy */
		gmt_grdfloat tcrit;
		a = stack[prev]->factor;
		b = irint (stack[last]->factor);
		tcrit = (gmt_grdfloat)gmt_tcrit (GMT, a, (double)b);
		for (node = 0; node < info->size; node++)
			stack[prev]->G->data[node] = tcrit;
	}
	else {
#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,a,b) shared(info,stack,prev,last,GMT)
#endif
		for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
			for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
				a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
				b = irint ((stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node]);
				stack[prev]->G->data[node] = (gmt_grdfloat)gmt_tcrit (GMT, a, (double)b);
			}
		}
	}
}

GMT_LOCAL void grdmath_TCDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TCDF 2 1 Student's t cumulative distribution function for t = A, and nu = B.  */
{
	uint64_t node, b;
	unsigned int prev = last - 1;
	openmp_int row, col;
	double a;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for TCDF!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for TCDF!\n");
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			b = lrint ((stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node]);
			stack[prev]->G->data[node] = (gmt_grdfloat)gmt_t_cdf (GMT, a, b);
		}
	}
}

GMT_LOCAL void grdmath_TPDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TPDF 2 1 Student's t probability density function for t = A and nu = B.  */
{
	uint64_t node, b;
	unsigned int prev = last - 1;
	openmp_int row, col;
	double a;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand one == 0 for TCDF!\n");
	if (stack[last]->constant && stack[last]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two == 0 for TCDF!\n");
	for (row = 0; row < (openmp_int)info->G->header->n_rows; row++) {
		for (col = 0, node = gmt_M_ijp (info->G->header, row, 0); col < (openmp_int)info->G->header->n_columns; col++, node++) {
			a = (stack[prev]->constant) ? stack[prev]->factor : stack[prev]->G->data[node];
			b = lrint ((stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node]);
			stack[prev]->G->data[node] = (gmt_grdfloat)gmt_t_pdf (GMT, a, b);
		}
	}
}

GMT_LOCAL void grdmath_TRIM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: TRIM 3 1 Alpha-trimming for %%-left = A, %%-right = B, and grid = C.  */
{
	/* Determine cumulative distribution and find left and right tail z cutoffs,
	 * then set grid values in the tails to NaN */
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2;
	gmt_grdfloat global_zmin, global_zmax, *tmp_grid = NULL;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[last]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "3rd operand for TRIM must be a grid!\n");
		return;
	}
	if (!stack[prev1]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "1st operand for TRIM must be constants!\n");
		return;
	}
	if (stack[prev1]->factor <= 0.0 || stack[prev1]->factor > 100.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Right alpha for TRIM must be in 0-100%% range!\n");
		return;
	}
	if (!stack[prev2]->constant) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "2nd operand for TRIM must be constants!\n");
		return;
	}
	if (stack[prev2]->factor <= 0.0 || stack[prev2]->factor > 100.0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Left alpha for TRIM must be in 0-100%% range!\n");
		return;
	}
	if (stack[prev1]->factor <= stack[prev2]->factor) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Right alpha for TRIM must exceed left alpha!\n");
		return;
	}
	tmp_grid = gmt_M_memory_aligned (GMT, NULL, stack[last]->G->header->size, gmt_grdfloat);
	gmt_M_memcpy (tmp_grid, stack[last]->G->data, stack[last]->G->header->size, gmt_grdfloat);
#ifdef DOUBLE_PRECISION_GRID
	gmt_sort_array (GMT, tmp_grid, stack[last]->G->header->size, GMT_DOUBLE);	/* Sort so we can find quantiles */
#else
	gmt_sort_array (GMT, tmp_grid, stack[last]->G->header->size, GMT_FLOAT);	/* Sort so we can find quantiles */
#endif
	global_zmin = (gmt_grdfloat)gmt_quantile_f (GMT, tmp_grid, stack[prev2]->factor, stack[last]->G->header->size);	/* "Left" quantile */
	global_zmax = (gmt_grdfloat)gmt_quantile_f (GMT, tmp_grid, stack[prev1]->factor, stack[last]->G->header->size);	/* "Right" quantile */
	gmt_M_free (GMT, tmp_grid);
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		stack[prev2]->G->data[node] = (stack[last]->G->data[node] < global_zmin || stack[last]->G->data[node] > global_zmax) ? GMT->session.f_NaN : stack[last]->G->data[node];
	}
}

GMT_LOCAL void grdmath_UPPER (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: UPPER 1 1 The highest (maximum) value of A.  */
{
	uint64_t node;
	openmp_int row, col;
	gmt_grdfloat high = -FLT_MAX;
	gmt_M_unused(GMT);

	if (stack[last]->constant) {	/* Trivial case */
		for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)stack[last]->factor;
		return;
	}

	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		if (gmt_M_is_fnan (stack[last]->G->data[node])) continue;
		if (stack[last]->G->data[node] > high) high = stack[last]->G->data[node];
	}
	if (high == -FLT_MAX) high = GMT->session.f_NaN;
	for (node = 0; node < info->size; node++) if (!gmt_M_is_fnan (stack[last]->G->data[node])) stack[last]->G->data[node] = high;
}

GMT_LOCAL gmt_grdfloat grdmath_wvar_sub (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GMT_GRID *G, struct GMT_GRID *W, bool use_grid, double weight) {
	/* Use West (1979) algorithm to compute mean and corrected sum of squares.
	 * https://en.wikipedia.org/wiki/Algorithms_for_calculating_variance */
	uint64_t node, n = 0;
	openmp_int row, col;
	double temp, mean = 0.0, sumw = 0.0, delta, R, M2 = 0.0, w = 1.0;
	if (!use_grid) w = weight;
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		if (gmt_M_is_fnan (G->data[node])) continue;
		if (use_grid) {
			if (gmt_M_is_dnan (W->data[node]))
				continue;
			else
				w = W->data[node];
		}
		temp  = w + sumw;
		delta = G->data[node] - mean;
		R = delta * w / temp;
		mean += R;
		M2 += sumw * delta * R;
		sumw = temp;
		n++;
	}
	return (n <= 1 || sumw == 0.0) ? GMT->session.f_NaN : (gmt_grdfloat) ((n * M2) / (sumw * (n - 1.0)));
}

GMT_LOCAL void grdmath_VAR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: VAR 1 1 Variance of A.  */
{
	uint64_t node;
	gmt_grdfloat var;
	gmt_M_unused(GMT);

	if (stack[last]->constant)	/* Trivial case: variance is undefined */
		var = GMT->session.f_NaN;
	else if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Must use spherical weights */
		struct GMT_GRID *W = gmt_duplicate_grid (GMT, stack[last]->G, GMT_DUPLICATE_ALLOC);
		gmt_get_cellarea (GMT, W);
		var = grdmath_wvar_sub (GMT, info, stack[last]->G, W, true, 0.0);
		gmt_free_grid (GMT, &W, true);
	}
	else {	/* Use Welford (1962) algorithm to compute mean and corrected sum of squares */
		uint64_t n = 0;
		openmp_int row, col;
		double mean = 0.0, sum2 = 0.0, delta;
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			if (gmt_M_is_fnan (stack[last]->G->data[node])) continue;
			n++;
			delta = stack[last]->G->data[node] - mean;
			mean += delta / n;
			sum2 += delta * (stack[last]->G->data[node] - mean);
		}
		var = (n > 1) ? (gmt_grdfloat)(sum2 / (n - 1)) : GMT->session.f_NaN;
	}
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = var;
}

GMT_LOCAL void grdmath_VARW (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: VARW 2 1 Weighted variance of A for weights in B.  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	gmt_grdfloat var;
	gmt_M_unused(GMT);

	if (stack[prev]->constant)	/* Trivial case: variance is undefined  */
		var = GMT->session.f_NaN;
	else
		var = grdmath_wvar_sub (GMT, info, stack[prev]->G, stack[last]->G, !stack[last]->constant, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[prev]->G->data[node] = var;
}

GMT_LOCAL void grdmath_VPDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: VPDF 3 1 Von Mises probability density function for angles = A, mu = B and kappa = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2;
	double x, mu, kappa;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev1]->constant) mu = stack[prev1]->factor;
	if (stack[last]->constant) kappa = stack[prev1]->factor;
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		x = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		if (!stack[prev1]->constant) mu = (double)stack[prev1]->G->data[node];
		if (!stack[last]->constant) kappa = (double)stack[last]->G->data[node];
		stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_vonmises_pdf (GMT, x, mu, kappa);
	}
}

GMT_LOCAL void grdmath_WCDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: WCDF 3 1 Weibull cumulative distribution function for x = A, scale = B, and shape = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2;
	double x, a, b;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev1]->constant && stack[prev1]->factor <= 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two <= 0 for WCDF!\n");
	if (stack[last]->constant  && stack[last]->factor  <= 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand three <= 0 for WCDF!\n");
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		x = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		a = lrint ((stack[prev1]->constant) ? stack[prev1]->factor : (double)stack[prev1]->G->data[node]);
		b = lrint ((stack[last]->constant)  ? stack[last]->factor  : (double)stack[last]->G->data[node]);
		stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_weibull_cdf (GMT, x, a, b);
	}
}

GMT_LOCAL void grdmath_WCRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: WCRIT 3 1 Weibull distribution critical value for alpha = A, scale = B, and shape = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2;
	double alpha, a, b;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev1]->constant && stack[prev1]->factor <= 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two <= 0 for WCRIT!\n");
	if (stack[last]->constant  && stack[last]->factor  <= 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand three <= 0 for WCRIT!\n");
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		alpha = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		a = lrint ((stack[prev1]->constant) ? stack[prev1]->factor : (double)stack[prev1]->G->data[node]);
		b = lrint ((stack[last]->constant)  ? stack[last]->factor  : (double)stack[last]->G->data[node]);
		stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_weibull_crit (GMT, alpha, a, b);
	}
}

GMT_LOCAL void grdmath_WPDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: WPDF 3 1 Weibull probability density function for x = A, scale = B and shape = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2;
	double x, a, b;

	prev1 = last - 1;
	prev2 = last - 2;
	if (stack[prev1]->constant && stack[prev1]->factor <= 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand two <= 0 for WPDF!\n");
	if (stack[last]->constant  && stack[last]->factor  <= 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Operand three <= 0 for WPDF!\n");
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		x = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		a = lrint ((stack[prev1]->constant) ? stack[prev1]->factor : (double)stack[prev1]->G->data[node]);
		b = lrint ((stack[last]->constant)  ? stack[last]->factor  : (double)stack[last]->G->data[node]);
		stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_weibull_pdf (GMT, x, a, b);
	}
}

GMT_LOCAL void grdmath_WRAP (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: WRAP 1 1 wrap (A). (A in radians). */
/*
wrap a value in radians onto [-pi,pi]

r=2.0*PI*(x/pi/2.0 - rintf(x/PI/2.0))

Kurt Feigl 2014-AUG-10

http://www.gnu.org/software/libc/manual/html_node/Rounding-Functions.html

Function: float rintf (float x) These functions round x to an integer value according to the current
rounding mode. See Floating Point Parameters, for information about the various rounding modes. The
default rounding mode is to round to the nearest integer; some machines support other modes, but
round-to-nearest is always used unless you explicitly select another. If x was not initially an
integer, these functions raise the inexact exception.

Function: float nearbyintf (float x) These functions return the same value as the rint functions,
but do not raise the inexact exception if x is not an integer.

Function: float roundf (float x) These functions are similar to rint, but they round halfway cases
away from zero instead of to the nearest integer (or other current rounding mode).
*/
{
	uint64_t node;
	double a;
	gmt_M_unused(GMT);

	for (node = 0; node < info->size; node++) {
		/* Argument must be finite  */

		a = (stack[last]->constant) ? stack[last]->factor : stack[last]->G->data[node];

		stack[last]->G->data[node] = (gmt_grdfloat)(TWO_PI*(a/TWO_PI - rint(a/TWO_PI)));
	}
}

GMT_LOCAL void grdmath_XOR (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: XOR 2 1 0 if A == NaN and B == NaN, NaN if B == NaN, else A.  */
{
	uint64_t node;
	unsigned int prev;
	gmt_grdfloat a = 0.0f, b = 0.0;

	prev = last - 1;
	if (stack[prev]->constant) a = (gmt_grdfloat)stack[prev]->factor;
	if (stack[last]->constant) b = (gmt_grdfloat)stack[last]->factor;
	for (node = 0; node < info->size; node++) {
		if (!stack[prev]->constant) a = stack[prev]->G->data[node];
		if (!stack[last]->constant) b = stack[last]->G->data[node];
		stack[prev]->G->data[node] = (gmt_M_is_fnan (a) && gmt_M_is_fnan (b)) ? 0.0f : (gmt_M_is_fnan (b) ? GMT->session.f_NaN : a);
	}
}

GMT_LOCAL void grdmath_XYZ2HSV (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: XYZ2HSV 3 3 Convert xyz to hsv, with x = A, y = B and z = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double rgb[4], hsv[4], xyz[3];
	gmt_M_unused (GMT);

	prev1 = last - 1;
	prev2 = last - 2;
#if 0
	if (stack[prev2]->constant && (stack[prev2]->factor < 0.0 || stack[prev2]->factor > 100.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument l to XYZ2HSV must be a 0 <= l <= 100!\n");
		error++;
	}
	if (stack[prev1]->constant && (stack[prev1]->factor < 0.0 || stack[prev1]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument s to XYZ2HSV must be a 0 <= s <= 1!\n");
		error++;
	}
	if (stack[last]->constant  && (stack[last]->factor < 0.0 || stack[last]->factor < 0.0 > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument v to XYZ2HSV must be a 0 <= v <= 1!\n");
		error++;
	}
#endif
	rgb[3] = hsv[3] = 0.0;	/* No transparency involved */
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* Constant arguments */
		xyz[0] = stack[prev2]->factor;
		xyz[1] = stack[prev1]->factor;
		xyz[2] = stack[last]->factor;
		gmt_xyz_to_rgb (rgb, xyz);
		gmt_rgb_to_hsv (rgb, hsv);
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			stack[prev2]->G->data[node] = (gmt_grdfloat)hsv[0];
			stack[prev1]->G->data[node] = (gmt_grdfloat)hsv[1];
			stack[last]->G->data[node]  = (gmt_grdfloat)hsv[2];
		}
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		xyz[0] = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		xyz[1] = (stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node];
		xyz[2] = (stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node];
		gmt_xyz_to_rgb (rgb, xyz);
		gmt_rgb_to_hsv (rgb, hsv);
		stack[prev2]->G->data[node] = (gmt_grdfloat)hsv[0];
		stack[prev1]->G->data[node] = (gmt_grdfloat)hsv[1];
		stack[last]->G->data[node]  = (gmt_grdfloat)hsv[2];
	}
}

GMT_LOCAL void grdmath_XYZ2LAB (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: XYZ2LAB 3 3 Convert xyz to lab, with x = A, y = B and z = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double lab[3], xyz[3];
	gmt_M_unused (GMT);

	prev1 = last - 1;
	prev2 = last - 2;
#if 0
	if (stack[prev2]->constant && (stack[prev2]->factor < 0.0 || stack[prev2]->factor > 100.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument l to XYZ2LAB must be a 0 <= l <= 100!\n");
		error++;
	}
	if (stack[prev1]->constant && (stack[prev1]->factor < 0.0 || stack[prev1]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument s to XYZ2LAB must be a 0 <= s <= 1!\n");
		error++;
	}
	if (stack[last]->constant  && (stack[last]->factor < 0.0 || stack[last]->factor < 0.0 > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument v to XYZ2LAB must be a 0 <= v <= 1!\n");
		error++;
	}
#endif
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* Constant arguments */
		xyz[0] = stack[prev2]->factor;
		xyz[1] = stack[prev1]->factor;
		xyz[2] = stack[last]->factor;
		gmt_xyz_to_lab (xyz, lab);
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			stack[prev2]->G->data[node] = (gmt_grdfloat)lab[0];
			stack[prev1]->G->data[node] = (gmt_grdfloat)lab[1];
			stack[last]->G->data[node]  = (gmt_grdfloat)lab[2];
		}
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		xyz[0] = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		xyz[1] = (stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node];
		xyz[2] = (stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node];
		gmt_xyz_to_lab (xyz, lab);
		stack[prev2]->G->data[node] = (gmt_grdfloat)lab[0];
		stack[prev1]->G->data[node] = (gmt_grdfloat)lab[1];
		stack[last]->G->data[node]  = (gmt_grdfloat)lab[2];
	}
}

GMT_LOCAL void grdmath_XYZ2RGB (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: XYZ2RGB 3 3 Convert xyz to rgb, with x = A, y = B and z = C.  */
{
	uint64_t node;
	openmp_int row, col;
	unsigned int prev1, prev2, error = 0;
	double rgb[3], xyz[3];
	gmt_M_unused (GMT);

	prev1 = last - 1;
	prev2 = last - 2;
#if 0
	if (stack[prev2]->constant && (stack[prev2]->factor < 0.0 || stack[prev2]->factor > 100.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument l to XYZ2RGB must be a 0 <= l <= 100!\n");
		error++;
	}
	if (stack[prev1]->constant && (stack[prev1]->factor < 0.0 || stack[prev1]->factor > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument s to XYZ2RGB must be a 0 <= s <= 1!\n");
		error++;
	}
	if (stack[last]->constant  && (stack[last]->factor < 0.0 || stack[last]->factor < 0.0 > 1.0)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Argument v to XYZ2RGB must be a 0 <= v <= 1!\n");
		error++;
	}
#endif
	if (error || (stack[prev2]->constant && stack[prev1]->constant && stack[last]->constant)) {	/* Constant arguments */
		xyz[0] = stack[prev2]->factor;
		xyz[1] = stack[prev1]->factor;
		xyz[2] = stack[last]->factor;
		gmt_xyz_to_rgb (rgb, xyz);
		gmt_M_grd_loop (GMT, info->G, row, col, node) {
			stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_M_s255 (rgb[0]);
			stack[prev1]->G->data[node] = (gmt_grdfloat)gmt_M_s255 (rgb[1]);
			stack[last]->G->data[node]  = (gmt_grdfloat)gmt_M_s255 (rgb[2]);
		}
		return;
	}
	gmt_M_grd_loop (GMT, info->G, row, col, node) {
		xyz[0] = (stack[prev2]->constant) ? stack[prev2]->factor : stack[prev2]->G->data[node];
		xyz[1] = (stack[prev1]->constant) ? stack[prev1]->factor : stack[prev1]->G->data[node];
		xyz[2] = (stack[last]->constant)  ? stack[last]->factor  : stack[last]->G->data[node];
		gmt_xyz_to_rgb (rgb, xyz);
		stack[prev2]->G->data[node] = (gmt_grdfloat)gmt_M_s255 (rgb[0]);
		stack[prev1]->G->data[node] = (gmt_grdfloat)gmt_M_s255 (rgb[1]);
		stack[last]->G->data[node]  = (gmt_grdfloat)gmt_M_s255 (rgb[2]);
	}
}

GMT_LOCAL void grdmath_Y0 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: Y0 1 1 Bessel function of A (2nd kind, order 0).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)y0 (fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (gmt_grdfloat)y0 ((double)fabsf (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_Y1 (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: Y1 1 1 Bessel function of A (2nd kind, order 1).  */
{
	uint64_t node;
	gmt_grdfloat a = 0.0;
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = (gmt_grdfloat)y1 (fabs (stack[last]->factor));
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (stack[last]->constant) ? a : (gmt_grdfloat)y1 ((double)fabsf (stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_YLM_sub (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last, bool ortho)
{
	/* Returns geophysical normalization, unless M < 0, then orthonormalized form */
	int64_t node;
	openmp_int row, col;
	unsigned int prev = last - 1;
	int L, M;
	double x, z, P, C, S;

	if (!(stack[prev]->constant && stack[last]->constant)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "L and M must be constants in YLM[g] (no calculations performed)\n");
		return;
	}

	L = irint (stack[prev]->factor);
	M = irint (stack[last]->factor);
	z = abs (M) * D2R;	/* abs() just in case routine is called with -M to add (-1)^M */

#ifdef _OPENMP
#pragma omp parallel for private(row,col,node,x,P,S,C) shared(info,stack,prev,GMT,L,M,ortho,z)
#endif
	for (row = 0; row < (openmp_int)info->G->header->my; row++) {	/* For each latitude */
		node = row * info->G->header->mx;

		x = sind (info->d_grd_y[row]);	/* Plm takes cos(colatitude) = sin(latitude) */
		P = gmt_plm_bar (GMT, L, M, x, ortho);
		if (M == 0) {
			grdmath_col_padloop (GMT, info->G, col, node) {
				stack[prev]->G->data[node] = (gmt_grdfloat)P;
				stack[last]->G->data[node] = 0.0;
			}
		}
		else {
			grdmath_col_padloop (GMT, info->G, col, node) {
				sincos (z * info->d_grd_x[col], &S, &C);
				stack[prev]->G->data[node] = (gmt_grdfloat)(P * C);
				stack[last]->G->data[node] = (gmt_grdfloat)(P * S);
			}
		}
	}
}

GMT_LOCAL void grdmath_YLM (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: YLM 2 2 Re and Im orthonormalized spherical harmonics degree A order B.  */
{
	grdmath_YLM_sub (GMT, info, stack, last, true);
}

GMT_LOCAL void grdmath_YLMg (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: YLMg 2 2 Cos and Sin normalized spherical harmonics degree A order B (geophysical convention).  */
{
	grdmath_YLM_sub (GMT, info, stack, last, false);
}

GMT_LOCAL void grdmath_YN (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: YN 2 1 Bessel function of A (2nd kind, order B).  */
{
	uint64_t node;
	unsigned int prev = last - 1;
	int order = 0;
	bool simple = false;
	gmt_grdfloat b = 0.0;

	if (stack[prev]->constant && stack[prev]->factor == 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "argument = 0 for YN!\n");
	if (stack[last]->constant) {
		if (stack[last]->factor < 0.0) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order < 0 for YN!\n");
		if ((rint(stack[last]->factor) != stack[last]->factor)) GMT_Report (GMT->parent, GMT_MSG_WARNING, "order not an integer for YN!\n");
		order = urint (fabs (stack[last]->factor));
		if (stack[prev]->constant) {
			b = (gmt_grdfloat)yn (order, fabs (stack[prev]->factor));
			simple = true;
		}
	}
	for (node = 0; node < info->size; node++) {
		if (simple)
			stack[prev]->G->data[node] = b;
		else {
			if (!stack[last]->constant) order = urint (fabsf (stack[last]->G->data[node]));
			stack[last]->G->data[node] = (gmt_grdfloat)yn (order, (double)fabsf (stack[prev]->G->data[node]));
		}
	}
}

GMT_LOCAL void grdmath_ZCRIT (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ZCRIT 1 1 Normal distribution critical value for alpha = A.  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = gmt_zcrit (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : gmt_zcrit (GMT, stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_ZCDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ZCDF 1 1 Normal cumulative distribution function for z = A.  */
{
	uint64_t node;
	double a = 0.0;

	if (stack[last]->constant) a = gmt_zdist (GMT, stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : gmt_zdist (GMT, stack[last]->G->data[node]));
}

GMT_LOCAL void grdmath_ZPDF (struct GMT_CTRL *GMT, struct GRDMATH_INFO *info, struct GRDMATH_STACK *stack[], unsigned int last)
/*OPERATOR: ZPDF 1 1 Normal probability density function for z = A.  */
{
	uint64_t node;
	double a = 0.0, f = 1.0 / sqrt (TWO_PI);
	gmt_M_unused(GMT);

	if (stack[last]->constant) a = f * exp (-0.5 * stack[last]->factor * stack[last]->factor);
	for (node = 0; node < info->size; node++) stack[last]->G->data[node] = (gmt_grdfloat)((stack[last]->constant) ? a : f * expf (-0.5f * stack[last]->G->data[node] * stack[last]->G->data[node]));
}

/* ---------------------- end operator functions --------------------- */

#define GRDMATH_N_OPERATORS 235

static void grdmath_init (void (*ops[]) (struct GMT_CTRL *, struct GRDMATH_INFO *, struct GRDMATH_STACK **, unsigned int), unsigned int n_args[], unsigned int n_out[])
{
	/* Operator function	# of operands	# of outputs */

	ops[0] = grdmath_ABS;	n_args[0] = 1;	n_out[0] = 1;
	ops[1] = grdmath_ACOS;	n_args[1] = 1;	n_out[1] = 1;
	ops[2] = grdmath_ACOSH;	n_args[2] = 1;	n_out[2] = 1;
	ops[3] = grdmath_ACOT;	n_args[3] = 1;	n_out[3] = 1;
	ops[4] = grdmath_ACOTH;	n_args[4] = 1;	n_out[4] = 1;
	ops[5] = grdmath_ACSC;	n_args[5] = 1;	n_out[5] = 1;
	ops[6] = grdmath_ACSCH;	n_args[6] = 1;	n_out[6] = 1;
	ops[7] = grdmath_ADD;	n_args[7] = 2;	n_out[7] = 1;
	ops[8] = grdmath_AND;	n_args[8] = 2;	n_out[8] = 1;
	ops[9] = grdmath_ARC;	n_args[9] = 2;	n_out[9] = 1;
	ops[10] = grdmath_AREA;	n_args[10] = 0;	n_out[10] = 1;
	ops[11] = grdmath_ASEC;	n_args[11] = 1;	n_out[11] = 1;
	ops[12] = grdmath_ASECH;	n_args[12] = 1;	n_out[12] = 1;
	ops[13] = grdmath_ASIN;	n_args[13] = 1;	n_out[13] = 1;
	ops[14] = grdmath_ASINH;	n_args[14] = 1;	n_out[14] = 1;
	ops[15] = grdmath_ATAN;	n_args[15] = 1;	n_out[15] = 1;
	ops[16] = grdmath_ATAN2;	n_args[16] = 2;	n_out[16] = 1;
	ops[17] = grdmath_ATANH;	n_args[17] = 1;	n_out[17] = 1;
	ops[18] = grdmath_BCDF;	n_args[18] = 3;	n_out[18] = 1;
	ops[19] = grdmath_BPDF;	n_args[19] = 3;	n_out[19] = 1;
	ops[20] = grdmath_BEI;	n_args[20] = 1;	n_out[20] = 1;
	ops[21] = grdmath_BER;	n_args[21] = 1;	n_out[21] = 1;
	ops[22] = grdmath_BITAND;	n_args[22] = 2;	n_out[22] = 1;
	ops[23] = grdmath_BITLEFT;	n_args[23] = 2;	n_out[23] = 1;
	ops[24] = grdmath_BITNOT;	n_args[24] = 1;	n_out[24] = 1;
	ops[25] = grdmath_BITOR;	n_args[25] = 2;	n_out[25] = 1;
	ops[26] = grdmath_BITRIGHT;	n_args[26] = 2;	n_out[26] = 1;
	ops[27] = grdmath_BITTEST;	n_args[27] = 2;	n_out[27] = 1;
	ops[28] = grdmath_BITXOR;	n_args[28] = 2;	n_out[28] = 1;
	ops[29] = grdmath_CAZ;	n_args[29] = 2;	n_out[29] = 1;
	ops[30] = grdmath_CBAZ;	n_args[30] = 2;	n_out[30] = 1;
	ops[31] = grdmath_CDIST;	n_args[31] = 2;	n_out[31] = 1;
	ops[32] = grdmath_CDIST2;	n_args[32] = 2;	n_out[32] = 1;
	ops[33] = grdmath_CEIL;	n_args[33] = 1;	n_out[33] = 1;
	ops[34] = grdmath_CHI2CRIT;	n_args[34] = 2;	n_out[34] = 1;
	ops[35] = grdmath_CHI2CDF;	n_args[35] = 2;	n_out[35] = 1;
	ops[36] = grdmath_CHI2PDF;	n_args[36] = 2;	n_out[36] = 1;
	ops[37] = grdmath_COMB;	n_args[37] = 2;	n_out[37] = 1;
	ops[38] = grdmath_CORRCOEFF;	n_args[38] = 2;	n_out[38] = 1;
	ops[39] = grdmath_COS;	n_args[39] = 1;	n_out[39] = 1;
	ops[40] = grdmath_COSD;	n_args[40] = 1;	n_out[40] = 1;
	ops[41] = grdmath_COSH;	n_args[41] = 1;	n_out[41] = 1;
	ops[42] = grdmath_COT;	n_args[42] = 1;	n_out[42] = 1;
	ops[43] = grdmath_COTD;	n_args[43] = 1;	n_out[43] = 1;
	ops[44] = grdmath_COTH;	n_args[44] = 1;	n_out[44] = 1;
	ops[45] = grdmath_PCDF;	n_args[45] = 2;	n_out[45] = 1;
	ops[46] = grdmath_PPDF;	n_args[46] = 2;	n_out[46] = 1;
	ops[47] = grdmath_CSC;	n_args[47] = 1;	n_out[47] = 1;
	ops[48] = grdmath_CSCD;	n_args[48] = 1;	n_out[48] = 1;
	ops[49] = grdmath_CSCH;	n_args[49] = 1;	n_out[49] = 1;
	ops[50] = grdmath_CURV;	n_args[50] = 1;	n_out[50] = 1;
	ops[51] = grdmath_D2DX2;	n_args[51] = 1;	n_out[51] = 1;
	ops[52] = grdmath_D2DY2;	n_args[52] = 1;	n_out[52] = 1;
	ops[53] = grdmath_D2DXY;	n_args[53] = 1;	n_out[53] = 1;
	ops[54] = grdmath_D2R;	n_args[54] = 1;	n_out[54] = 1;
	ops[55] = grdmath_DDX;	n_args[55] = 1;	n_out[55] = 1;
	ops[56] = grdmath_DDY;	n_args[56] = 1;	n_out[56] = 1;
	ops[57] = grdmath_DEG2KM;	n_args[57] = 1;	n_out[57] = 1;
	ops[58] = grdmath_DENAN;	n_args[58] = 2;	n_out[58] = 1;
	ops[59] = grdmath_DILOG;	n_args[59] = 1;	n_out[59] = 1;
	ops[60] = grdmath_DIV;	n_args[60] = 2;	n_out[60] = 1;
	ops[61] = grdmath_DUP;	n_args[61] = 1;	n_out[61] = 2;
	ops[62] = grdmath_ECDF;	n_args[62] = 2;	n_out[62] = 1;
	ops[63] = grdmath_ECRIT;	n_args[63] = 2;	n_out[63] = 1;
	ops[64] = grdmath_EPDF;	n_args[64] = 2;	n_out[64] = 1;
	ops[65] = grdmath_ERF;	n_args[65] = 1;	n_out[65] = 1;
	ops[66] = grdmath_ERFC;	n_args[66] = 1;	n_out[66] = 1;
	ops[67] = grdmath_EQ;	n_args[67] = 2;	n_out[67] = 1;
	ops[68] = grdmath_ERFINV;	n_args[68] = 1;	n_out[68] = 1;
	ops[69] = grdmath_EXCH;	n_args[69] = 2;	n_out[69] = 2;
	ops[70] = grdmath_EXP;	n_args[70] = 1;	n_out[70] = 1;
	ops[71] = grdmath_FACT;	n_args[71] = 1;	n_out[71] = 1;
	ops[72] = grdmath_EXTREMA;	n_args[72] = 1;	n_out[72] = 1;
	ops[73] = grdmath_FCRIT;	n_args[73] = 3;	n_out[73] = 1;
	ops[74] = grdmath_FCDF;	n_args[74] = 3;	n_out[74] = 1;
	ops[75] = grdmath_FLIPLR;	n_args[75] = 1;	n_out[75] = 1;
	ops[76] = grdmath_FLIPUD;	n_args[76] = 1;	n_out[76] = 1;
	ops[77] = grdmath_FLOOR;	n_args[77] = 1;	n_out[77] = 1;
	ops[78] = grdmath_FMOD;	n_args[78] = 2;	n_out[78] = 1;
	ops[79] = grdmath_FPDF;	n_args[79] = 3;	n_out[79] = 1;
	ops[80] = grdmath_GE;	n_args[80] = 2;	n_out[80] = 1;
	ops[81] = grdmath_GT;	n_args[81] = 2;	n_out[81] = 1;
	ops[82] = grdmath_HYPOT;	n_args[82] = 2;	n_out[82] = 1;
	ops[83] = grdmath_I0;	n_args[83] = 1;	n_out[83] = 1;
	ops[84] = grdmath_I1;	n_args[84] = 1;	n_out[84] = 1;
	ops[85] = grdmath_IFELSE;	n_args[85] = 3;	n_out[85] = 1;
	ops[86] = grdmath_IN;	n_args[86] = 2;	n_out[86] = 1;
	ops[87] = grdmath_INRANGE;	n_args[87] = 3;	n_out[87] = 1;
	ops[88] = grdmath_INSIDE;	n_args[88] = 1;	n_out[88] = 1;
	ops[89] = grdmath_INV;	n_args[89] = 1;	n_out[89] = 1;
	ops[90] = grdmath_ISFINITE;	n_args[90] = 1;	n_out[90] = 1;
	ops[91] = grdmath_ISNAN;	n_args[91] = 1;	n_out[91] = 1;
	ops[92] = grdmath_J0;	n_args[92] = 1;	n_out[92] = 1;
	ops[93] = grdmath_J1;	n_args[93] = 1;	n_out[93] = 1;
	ops[94] = grdmath_JN;	n_args[94] = 2;	n_out[94] = 1;
	ops[95] = grdmath_K0;	n_args[95] = 1;	n_out[95] = 1;
	ops[96] = grdmath_K1;	n_args[96] = 1;	n_out[96] = 1;
	ops[97] = grdmath_KEI;	n_args[97] = 1;	n_out[97] = 1;
	ops[98] = grdmath_KER;	n_args[98] = 1;	n_out[98] = 1;
	ops[99] = grdmath_KM2DEG;	n_args[99] = 1;	n_out[99] = 1;
	ops[100] = grdmath_KN;	n_args[100] = 2;	n_out[100] = 1;
	ops[101] = grdmath_KURT;	n_args[101] = 1;	n_out[101] = 1;
	ops[102] = grdmath_LCDF;	n_args[102] = 1;	n_out[102] = 1;
	ops[103] = grdmath_LCRIT;	n_args[103] = 1;	n_out[103] = 1;
	ops[104] = grdmath_LDIST;	n_args[104] = 1;	n_out[104] = 1;
	ops[105] = grdmath_LDISTG;	n_args[105] = 0;	n_out[105] = 1;
	ops[106] = grdmath_LDIST2;	n_args[106] = 2;	n_out[106] = 1;
	ops[107] = grdmath_LE;	n_args[107] = 2;	n_out[107] = 1;
	ops[108] = grdmath_LOG;	n_args[108] = 1;	n_out[108] = 1;
	ops[109] = grdmath_LOG10;	n_args[109] = 1;	n_out[109] = 1;
	ops[110] = grdmath_LOG1P;	n_args[110] = 1;	n_out[110] = 1;
	ops[111] = grdmath_LOG2;	n_args[111] = 1;	n_out[111] = 1;
	ops[112] = grdmath_LMSSCL;	n_args[112] = 1;	n_out[112] = 1;
	ops[113] = grdmath_LMSSCLW;	n_args[113] = 1;	n_out[113] = 1;
	ops[114] = grdmath_LOWER;	n_args[114] = 1;	n_out[114] = 1;
	ops[115] = grdmath_LPDF;	n_args[115] = 1;	n_out[115] = 1;
	ops[116] = grdmath_LRAND;	n_args[116] = 2;	n_out[116] = 1;
	ops[117] = grdmath_LT;	n_args[117] = 2;	n_out[117] = 1;
	ops[118] = grdmath_MAD;	n_args[118] = 1;	n_out[118] = 1;
	ops[119] = grdmath_MADW;	n_args[119] = 2;	n_out[119] = 1;
	ops[120] = grdmath_MAX;	n_args[120] = 2;	n_out[120] = 1;
	ops[121] = grdmath_MEAN;	n_args[121] = 1;	n_out[121] = 1;
	ops[122] = grdmath_MEANW;	n_args[122] = 2;	n_out[122] = 1;
	ops[123] = grdmath_MEDIAN;	n_args[123] = 1;	n_out[123] = 1;
	ops[124] = grdmath_MEDIANW;	n_args[124] = 2;	n_out[124] = 1;
	ops[125] = grdmath_MIN;	n_args[125] = 2;	n_out[125] = 1;
	ops[126] = grdmath_MOD;	n_args[126] = 2;	n_out[126] = 1;
	ops[127] = grdmath_MODE;	n_args[127] = 1;	n_out[127] = 1;
	ops[128] = grdmath_MODEW;	n_args[128] = 2;	n_out[128] = 1;
	ops[129] = grdmath_MUL;	n_args[129] = 2;	n_out[129] = 1;
	ops[130] = grdmath_NAN;	n_args[130] = 2;	n_out[130] = 1;
	ops[131] = grdmath_NEG;	n_args[131] = 1;	n_out[131] = 1;
	ops[132] = grdmath_NEQ;	n_args[132] = 2;	n_out[132] = 1;
	ops[133] = grdmath_NORM;	n_args[133] = 1;	n_out[133] = 1;
	ops[134] = grdmath_NOT;	n_args[134] = 1;	n_out[134] = 1;
	ops[135] = grdmath_NRAND;	n_args[135] = 2;	n_out[135] = 1;
	ops[136] = grdmath_OR;	n_args[136] = 2;	n_out[136] = 1;
	ops[137] = grdmath_PDIST;	n_args[137] = 1;	n_out[137] = 1;
	ops[138] = grdmath_PDIST2;	n_args[138] = 2;	n_out[138] = 1;
	ops[139] = grdmath_PERM;	n_args[139] = 2;	n_out[139] = 1;
	ops[140] = grdmath_POP;	n_args[140] = 1;	n_out[140] = 0;
	ops[141] = grdmath_PLM;	n_args[141] = 3;	n_out[141] = 1;
	ops[142] = grdmath_PLMg;	n_args[142] = 3;	n_out[142] = 1;
	ops[143] = grdmath_POINT;	n_args[143] = 1;	n_out[143] = 2;
	ops[144] = grdmath_POW;	n_args[144] = 2;	n_out[144] = 1;
	ops[145] = grdmath_PQUANT;	n_args[145] = 2;	n_out[145] = 1;
	ops[146] = grdmath_PQUANTW;	n_args[146] = 3;	n_out[146] = 1;
	ops[147] = grdmath_PSI;	n_args[147] = 1;	n_out[147] = 1;
	ops[148] = grdmath_PV;	n_args[148] = 3;	n_out[148] = 1;
	ops[149] = grdmath_QV;	n_args[149] = 3;	n_out[149] = 1;
	ops[150] = grdmath_R2;	n_args[150] = 2;	n_out[150] = 1;
	ops[151] = grdmath_R2D;	n_args[151] = 1;	n_out[151] = 1;
	ops[152] = grdmath_RAND;	n_args[152] = 2;	n_out[152] = 1;
	ops[153] = grdmath_RCDF;	n_args[153] = 1;	n_out[153] = 1;
	ops[154] = grdmath_RCRIT;	n_args[154] = 1;	n_out[154] = 1;
	ops[155] = grdmath_RINT;	n_args[155] = 1;	n_out[155] = 1;
	ops[156] = grdmath_RMS;	n_args[156] = 1;	n_out[156] = 1;
	ops[157] = grdmath_RMSW;	n_args[157] = 2;	n_out[157] = 1;
	ops[158] = grdmath_RPDF;	n_args[158] = 1;	n_out[158] = 1;
	ops[159] = grdmath_ROLL;	n_args[159] = 2;	n_out[159] = 0;
	ops[160] = grdmath_ROTX;	n_args[160] = 2;	n_out[160] = 1;
	ops[161] = grdmath_ROTY;	n_args[161] = 2;	n_out[161] = 1;
	ops[162] = grdmath_SDIST;	n_args[162] = 2;	n_out[162] = 1;
	ops[163] = grdmath_SDIST2;	n_args[163] = 2;	n_out[163] = 1;
	ops[164] = grdmath_SAZ;	n_args[164] = 2;	n_out[164] = 1;
	ops[165] = grdmath_SBAZ;	n_args[165] = 2;	n_out[165] = 1;
	ops[166] = grdmath_SEC;	n_args[166] = 1;	n_out[166] = 1;
	ops[167] = grdmath_SECD;	n_args[167] = 1;	n_out[167] = 1;
	ops[168] = grdmath_SECH;	n_args[168] = 1;	n_out[168] = 1;
	ops[169] = grdmath_SIGN;	n_args[169] = 1;	n_out[169] = 1;
	ops[170] = grdmath_SIN;	n_args[170] = 1;	n_out[170] = 1;
	ops[171] = grdmath_SINC;	n_args[171] = 1;	n_out[171] = 1;
	ops[172] = grdmath_SIND;	n_args[172] = 1;	n_out[172] = 1;
	ops[173] = grdmath_SINH;	n_args[173] = 1;	n_out[173] = 1;
	ops[174] = grdmath_SKEW;	n_args[174] = 1;	n_out[174] = 1;
	ops[175] = grdmath_SQR;	n_args[175] = 1;	n_out[175] = 1;
	ops[176] = grdmath_SQRT;	n_args[176] = 1;	n_out[176] = 1;
	ops[177] = grdmath_STD;	n_args[177] = 1;	n_out[177] = 1;
	ops[178] = grdmath_STDW;	n_args[178] = 2;	n_out[178] = 1;
	ops[179] = grdmath_STEP;	n_args[179] = 1;	n_out[179] = 1;
	ops[180] = grdmath_STEPX;	n_args[180] = 1;	n_out[180] = 1;
	ops[181] = grdmath_STEPY;	n_args[181] = 1;	n_out[181] = 1;
	ops[182] = grdmath_SUB;	n_args[182] = 2;	n_out[182] = 1;
	ops[183] = grdmath_SUM;	n_args[183] = 1;	n_out[183] = 1;
	ops[184] = grdmath_TAN;	n_args[184] = 1;	n_out[184] = 1;
	ops[185] = grdmath_TAND;	n_args[185] = 1;	n_out[185] = 1;
	ops[186] = grdmath_TANH;	n_args[186] = 1;	n_out[186] = 1;
	ops[187] = grdmath_TAPER;	n_args[187] = 2;	n_out[187] = 1;
	ops[188] = grdmath_TN;	n_args[188] = 2;	n_out[188] = 1;
	ops[189] = grdmath_TCRIT;	n_args[189] = 2;	n_out[189] = 1;
	ops[190] = grdmath_TCDF;	n_args[190] = 2;	n_out[190] = 1;
	ops[191] = grdmath_TPDF;	n_args[191] = 2;	n_out[191] = 1;
	ops[192] = grdmath_TRIM;	n_args[192] = 3;	n_out[192] = 1;
	ops[193] = grdmath_UPPER;	n_args[193] = 1;	n_out[193] = 1;
	ops[194] = grdmath_VAR;	n_args[194] = 1;	n_out[194] = 1;
	ops[195] = grdmath_VARW;	n_args[195] = 2;	n_out[195] = 1;
	ops[196] = grdmath_WCDF;	n_args[196] = 3;	n_out[196] = 1;
	ops[197] = grdmath_WCRIT;	n_args[197] = 3;	n_out[197] = 1;
	ops[198] = grdmath_WPDF;	n_args[198] = 3;	n_out[198] = 1;
	ops[199] = grdmath_WRAP;	n_args[199] = 1;	n_out[199] = 1;
	ops[200] = grdmath_XOR;	n_args[200] = 2;	n_out[200] = 1;
	ops[201] = grdmath_Y0;	n_args[201] = 1;	n_out[201] = 1;
	ops[202] = grdmath_Y1;	n_args[202] = 1;	n_out[202] = 1;
	ops[203] = grdmath_YLM;	n_args[203] = 2;	n_out[203] = 2;
	ops[204] = grdmath_YLMg;	n_args[204] = 2;	n_out[204] = 2;
	ops[205] = grdmath_YN;	n_args[205] = 2;	n_out[205] = 1;
	ops[206] = grdmath_ZCRIT;	n_args[206] = 1;	n_out[206] = 1;
	ops[207] = grdmath_ZCDF;	n_args[207] = 1;	n_out[207] = 1;
	ops[208] = grdmath_ZPDF;	n_args[208] = 1;	n_out[208] = 1;
	ops[209] = grdmath_HSV2LAB;	n_args[209] = 3;	n_out[209] = 3;
	ops[210] = grdmath_HSV2RGB;	n_args[210] = 3;	n_out[210] = 3;
	ops[211] = grdmath_HSV2XYZ;	n_args[211] = 3;	n_out[211] = 3;
	ops[212] = grdmath_LAB2HSV;	n_args[212] = 3;	n_out[212] = 3;
	ops[213] = grdmath_LAB2RGB;	n_args[213] = 3;	n_out[213] = 3;
	ops[214] = grdmath_LAB2XYZ;	n_args[214] = 3;	n_out[214] = 3;
	ops[215] = grdmath_RGB2HSV;	n_args[215] = 3;	n_out[215] = 3;
	ops[216] = grdmath_RGB2LAB;	n_args[216] = 3;	n_out[216] = 3;
	ops[217] = grdmath_RGB2XYZ;	n_args[217] = 3;	n_out[217] = 3;
	ops[218] = grdmath_XYZ2HSV;	n_args[218] = 3;	n_out[218] = 3;
	ops[219] = grdmath_XYZ2LAB;	n_args[219] = 3;	n_out[219] = 3;
	ops[220] = grdmath_XYZ2RGB;	n_args[220] = 3;	n_out[220] = 3;
	ops[221] = grdmath_DOT;	n_args[221] = 2;	n_out[221] = 1;
	ops[222] = grdmath_BLEND;	n_args[222] = 3;	n_out[222] = 1;
	ops[223] = grdmath_DAYNIGHT;	n_args[223] = 3;	n_out[223] = 1;
	ops[224] = grdmath_VPDF;	n_args[224] = 3;	n_out[224] = 1;
	ops[225] = grdmath_FISHER;	n_args[225] = 3;	n_out[225] = 1;
	ops[226] = grdmath_CUMSUM;	n_args[226] = 2;	n_out[226] = 1;
	ops[227] = grdmath_ASIND;	n_args[227] = 1;	n_out[227] = 1;
	ops[228] = grdmath_ACOSD;	n_args[228] = 1;	n_out[228] = 1;
	ops[229] = grdmath_ACOTD;	n_args[229] = 1;	n_out[229] = 1;
	ops[230] = grdmath_ACSCD;	n_args[230] = 1;	n_out[230] = 1;
	ops[231] = grdmath_ASECD;	n_args[231] = 1;	n_out[231] = 1;
	ops[232] = grdmath_ATAND;	n_args[232] = 1;	n_out[232] = 1;
	ops[233] = grdmath_ATAN2D;	n_args[233] = 2;	n_out[233] = 1;
	ops[234] = grdmath_SADDLE;	n_args[234] = 1;	n_out[234] = 1;
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return1(code) {GMT_Destroy_Options (API, &list); Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}
#define Return(code) {GMT_Destroy_Options (API, &list); Free_Ctrl (GMT, Ctrl); grdmath_free (GMT, stack, recall, &info); gmt_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LOCAL void grdmath_backwards_fixing (struct GMT_CTRL *GMT, char **arg) {
	/* Handle backwards compatible operator names */
	char *t = NULL, old[GMT_LEN16] = {""};
	if (!gmt_M_compat_check (GMT, 6)) return;	/* No checking so we may fail later */
	if (!strcmp (*arg, "CHIDIST"))      {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("CHI2CDF");  }
	else if (!strcmp (*arg, "CHICRIT")) {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("CHI2CRIT"); }
	else if (!strcmp (*arg, "CPOISS"))  {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("PCDF");     }
	else if (!strcmp (*arg, "FDIST"))   {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("FCDF");     }
	else if (!strcmp (*arg, "MED"))     {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("MEDIAN");   }
	else if (!strcmp (*arg, "TDIST"))   {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("TCDF");     }
	else if (!strcmp (*arg, "Xn"))      {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("XNORM");    }
	else if (!strcmp (*arg, "Yn"))      {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("YNORM");    }
	else if (!strcmp (*arg, "ZDIST"))   {strncpy (old, *arg, GMT_LEN16-1); gmt_M_str_free (*arg); *arg = t = strdup ("ZCDF");     }

	if (t)
		GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Operator %s is deprecated; use %s instead.\n", old, t);
}

GMT_LOCAL int grdmath_decode_argument (struct GMT_CTRL *GMT, struct GMT_OPTION *opt, double *value, struct GMT_HASH *H) {
	int i, expect, check = GMT_IS_NAN;
	bool possible_number = false;
	double tmp = 0.0;

	if (opt == NULL || opt->arg == NULL || opt->arg[0] == '\0') return GRDMATH_ARG_IS_BAD;

	grdmath_backwards_fixing (GMT, &(opt->arg));	/* Possibly exchange obsolete operator name for new one unless compatibility is off */

	if (opt->option == GMT_OPT_OUTFILE2) return GRDMATH_ARG_IS_SAVE;	/* Time to save stack; arg is filename */

	if (gmt_M_file_is_memory (opt->arg)) return GRDMATH_ARG_IS_FILE;	/* Deal with memory references first */
	if (gmt_M_file_is_remote (opt->arg)) return GRDMATH_ARG_IS_FILE;	/* Deal with cache or dataset references as well */

	/* Check if argument is operator */

	if ((i = gmt_hash_lookup (GMT, opt->arg, H, GRDMATH_N_OPERATORS, GRDMATH_N_OPERATORS)) >= GRDMATH_ARG_IS_OPERATOR) return (i);

	/* Next look for symbols with special meaning */

	if (!strncmp (opt->arg, GRDMATH_STORE_CMD, strlen(GRDMATH_STORE_CMD))) return GRDMATH_ARG_IS_STORE;	/* store into mem location @<label> */
	if (!strncmp (opt->arg, GRDMATH_CLEAR_CMD, strlen(GRDMATH_CLEAR_CMD))) return GRDMATH_ARG_IS_CLEAR;	/* clear mem location @<label> */
	if (!strncmp (opt->arg, GRDMATH_RECALL_CMD, strlen(GRDMATH_RECALL_CMD))) return GRDMATH_ARG_IS_RECALL;	/* load from mem location @<label> */
	if (!(strcmp (opt->arg, "PI") && strcmp (opt->arg, "pi"))) return GRDMATH_ARG_IS_PI;
	if (!(strcmp (opt->arg, "E") && strcmp (opt->arg, "e"))) return GRDMATH_ARG_IS_E;
	if (!(strcmp (opt->arg, "F_EPS") && strcmp (opt->arg, "EPS"))) return GRDMATH_ARG_IS_F_EPS;
	if (!strcmp (opt->arg, "EULER"))  return GRDMATH_ARG_IS_EULER;
	if (!strcmp (opt->arg, "PHI"))    return GRDMATH_ARG_IS_PHI;
	if (!strcmp (opt->arg, "XMIN"))   return GRDMATH_ARG_IS_XMIN;
	if (!strcmp (opt->arg, "XMAX"))   return GRDMATH_ARG_IS_XMAX;
	if (!strcmp (opt->arg, "XRANGE")) return GRDMATH_ARG_IS_XRANGE;
	if (!strcmp (opt->arg, "XINC"))   return GRDMATH_ARG_IS_XINC;
	if (!strcmp (opt->arg, "NX"))     return GRDMATH_ARG_IS_NX;
	if (!strcmp (opt->arg, "YMIN"))   return GRDMATH_ARG_IS_YMIN;
	if (!strcmp (opt->arg, "YMAX"))   return GRDMATH_ARG_IS_YMAX;
	if (!strcmp (opt->arg, "YRANGE")) return GRDMATH_ARG_IS_YRANGE;
	if (!strcmp (opt->arg, "YINC"))   return GRDMATH_ARG_IS_YINC;
	if (!strcmp (opt->arg, "NY"))     return GRDMATH_ARG_IS_NY;
	if (!strcmp (opt->arg, "X"))      return GRDMATH_ARG_IS_X_MATRIX;
	if (!strcmp (opt->arg, "XNORM"))  return GRDMATH_ARG_IS_x_MATRIX;
	if (!strcmp (opt->arg, "Y"))      return GRDMATH_ARG_IS_Y_MATRIX;
	if (!strcmp (opt->arg, "YNORM"))  return GRDMATH_ARG_IS_y_MATRIX;
	if (!strcmp (opt->arg, "XCOL"))   return GRDMATH_ARG_IS_XCOL_MATRIX;
	if (!strcmp (opt->arg, "YROW"))   return GRDMATH_ARG_IS_YROW_MATRIX;
	if (!strcmp (opt->arg, "NODE"))   return GRDMATH_ARG_IS_NODE_MATRIX;
	if (!strcmp (opt->arg, "NODEP"))  return GRDMATH_ARG_IS_NODEP_MATRIX;
	if (!strcmp (opt->arg, "NaN")) {*value = GMT->session.d_NaN; return GRDMATH_ARG_IS_NUMBER;}

	/* Preliminary test-conversion to a number */

	if (!gmt_not_numeric (GMT, opt->arg)) {	/* Only check if we are not sure this is NOT a number */
		expect = (strchr (opt->arg, 'T')) ? GMT_IS_ABSTIME : GMT_IS_UNKNOWN;	/* Watch out for dateTclock-strings */
		check = gmt_scanf (GMT, opt->arg, expect, &tmp);
		possible_number = true;
	}

	/* Determine if argument is file. But first strip off suffix */

	if (!gmt_access (GMT, opt->arg, F_OK)) {	/* Yes it is */
		if (check != GMT_IS_NAN && possible_number) GMT_Report (GMT->parent, GMT_MSG_WARNING, "Your argument %s is both a file and a number.  File is selected\n", opt->arg);
		return GRDMATH_ARG_IS_FILE;
	}

	if (check != GMT_IS_NAN) {	/* OK it is a number */
		*value = tmp;
		return GRDMATH_ARG_IS_NUMBER;
	}

	if (opt->arg[0] == '-') {	/* Probably a bad commandline option */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option %s not recognized\n", opt->arg);
		return GRDMATH_ARG_IS_BAD;
	}

	GMT_Report (GMT->parent, GMT_MSG_ERROR, "%s is not a number, operator or file name\n", opt->arg);
	return GRDMATH_ARG_IS_BAD;
}

GMT_LOCAL char *grdmath_setlabel (struct GMT_CTRL *GMT, char *arg) {
	char *label = strchr (arg, '@') + 1;	/* Label that follows @ */
	if (!label || label[0] == '\0') {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "No label appended to STO|RCL|CLR operator!\n");
		return (NULL);
	}
	return (label);
}

GMT_LOCAL void grdmath_free (struct GMT_CTRL *GMT, struct GRDMATH_STACK *stack[], struct GRDMATH_STORE *recall[], struct GRDMATH_INFO *info) {
	/* Free allocated memory via GMT_Destroy_Data. */
	unsigned int k;
	int error = 0;

	/* Free anything on the stack */
	for (k = 0; k < GRDMATH_STACK_SIZE; k++) {
		if (stack[k]->G) {
			if ((error = GMT_Destroy_Data (GMT->parent, &stack[k]->G)) == GMT_NOERROR)
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Freed stack item %d\n", k);
			else
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to free stack item %d\n", k);
		}
		gmt_M_free (GMT, stack[k]);
	}

	/* Free anything used for store/recall */
	for (k = 0; k < GRDMATH_STORE_SIZE; k++) {
		if (recall[k] == NULL) continue;
		if (!recall[k]->stored.constant) {
			if ((error = GMT_Destroy_Data (GMT->parent, &recall[k]->stored.G)) != GMT_NOERROR)
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to free recall item %d\n", k);
		}
		gmt_M_free (GMT, recall[k]);
	}

	/* Free the info grid structure */
	if (GMT_Destroy_Data (GMT->parent, &info->G) != GMT_NOERROR)
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to free info.G\n");

	/* Free misc. arrays for actual and normalized coordinates */
	gmt_M_free (GMT, info->d_grd_x);
	gmt_M_free (GMT, info->d_grd_y);
	gmt_M_free (GMT, info->d_grd_xn);
	gmt_M_free (GMT, info->d_grd_yn);
	gmt_M_free (GMT, info->f_grd_x);
	gmt_M_free (GMT, info->f_grd_y);
	gmt_M_free (GMT, info->f_grd_xn);
	gmt_M_free (GMT, info->f_grd_yn);
	gmt_M_free (GMT, info->dx);
	gmt_M_str_free (info->ASCII_file);
	info->A = NULL;
	gmt_free_grid (GMT, &(info->G), false);
}

GMT_LOCAL void grdmath_expand_recall_cmd (struct GMT_OPTION *list) {
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

EXTERN_MSC int GMT_grdmath (void *V_API, int mode, void *args) {
	int k, op = 0, new_stack = GMT_NOTSET, rowx, colx, status, start, error = 0;
	unsigned int kk, nstack = 0, n_stored = 0, n_items = 0, this_stack, pos;
	unsigned int consumed_operands[GRDMATH_N_OPERATORS], produced_operands[GRDMATH_N_OPERATORS];
	bool subset;
	char *in_file = NULL, *label = NULL;

	uint64_t node;
	openmp_int row, col;

	struct GRDMATH_STACK *stack[GRDMATH_STACK_SIZE];
	struct GRDMATH_STORE *recall[GRDMATH_STORE_SIZE];
	struct GMT_GRID *G_in = NULL;

	double value, x_noise, y_noise, off, scale;
	double wesn[4], special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_NY+1];

	/* Declare operator array */

	static char *operator[GRDMATH_N_OPERATORS + 1] = {
		"ABS",	/* id = 0 */
		"ACOS",	/* id = 1 */
		"ACOSH",	/* id = 2 */
		"ACOT",	/* id = 3 */
		"ACOTH",	/* id = 4 */
		"ACSC",	/* id = 5 */
		"ACSCH",	/* id = 6 */
		"ADD",	/* id = 7 */
		"AND",	/* id = 8 */
		"ARC",	/* id = 9 */
		"AREA",	/* id = 10 */
		"ASEC",	/* id = 11 */
		"ASECH",	/* id = 12 */
		"ASIN",	/* id = 13 */
		"ASINH",	/* id = 14 */
		"ATAN",	/* id = 15 */
		"ATAN2",	/* id = 16 */
		"ATANH",	/* id = 17 */
		"BCDF",	/* id = 18 */
		"BPDF",	/* id = 19 */
		"BEI",	/* id = 20 */
		"BER",	/* id = 21 */
		"BITAND",	/* id = 22 */
		"BITLEFT",	/* id = 23 */
		"BITNOT",	/* id = 24 */
		"BITOR",	/* id = 25 */
		"BITRIGHT",	/* id = 26 */
		"BITTEST",	/* id = 27 */
		"BITXOR",	/* id = 28 */
		"CAZ",	/* id = 29 */
		"CBAZ",	/* id = 30 */
		"CDIST",	/* id = 31 */
		"CDIST2",	/* id = 32 */
		"CEIL",	/* id = 33 */
		"CHI2CRIT",	/* id = 34 */
		"CHI2CDF",	/* id = 35 */
		"CHI2PDF",	/* id = 36 */
		"COMB",	/* id = 37 */
		"CORRCOEFF",	/* id = 38 */
		"COS",	/* id = 39 */
		"COSD",	/* id = 40 */
		"COSH",	/* id = 41 */
		"COT",	/* id = 42 */
		"COTD",	/* id = 43 */
		"COTH",	/* id = 44 */
		"PCDF",	/* id = 45 */
		"PPDF",	/* id = 46 */
		"CSC",	/* id = 47 */
		"CSCD",	/* id = 48 */
		"CSCH",	/* id = 49 */
		"CURV",	/* id = 50 */
		"D2DX2",	/* id = 51 */
		"D2DY2",	/* id = 52 */
		"D2DXY",	/* id = 53 */
		"D2R",	/* id = 54 */
		"DDX",	/* id = 55 */
		"DDY",	/* id = 56 */
		"DEG2KM",	/* id = 57 */
		"DENAN",	/* id = 58 */
		"DILOG",	/* id = 59 */
		"DIV",	/* id = 60 */
		"DUP",	/* id = 61 */
		"ECDF",	/* id = 62 */
		"ECRIT",	/* id = 63 */
		"EPDF",	/* id = 64 */
		"ERF",	/* id = 65 */
		"ERFC",	/* id = 66 */
		"EQ",	/* id = 67 */
		"ERFINV",	/* id = 68 */
		"EXCH",	/* id = 69 */
		"EXP",	/* id = 70 */
		"FACT",	/* id = 71 */
		"EXTREMA",	/* id = 72 */
		"FCRIT",	/* id = 73 */
		"FCDF",	/* id = 74 */
		"FLIPLR",	/* id = 75 */
		"FLIPUD",	/* id = 76 */
		"FLOOR",	/* id = 77 */
		"FMOD",	/* id = 78 */
		"FPDF",	/* id = 79 */
		"GE",	/* id = 80 */
		"GT",	/* id = 81 */
		"HYPOT",	/* id = 82 */
		"I0",	/* id = 83 */
		"I1",	/* id = 84 */
		"IFELSE",	/* id = 85 */
		"IN",	/* id = 86 */
		"INRANGE",	/* id = 87 */
		"INSIDE",	/* id = 88 */
		"INV",	/* id = 89 */
		"ISFINITE",	/* id = 90 */
		"ISNAN",	/* id = 91 */
		"J0",	/* id = 92 */
		"J1",	/* id = 93 */
		"JN",	/* id = 94 */
		"K0",	/* id = 95 */
		"K1",	/* id = 96 */
		"KEI",	/* id = 97 */
		"KER",	/* id = 98 */
		"KM2DEG",	/* id = 99 */
		"KN",	/* id = 100 */
		"KURT",	/* id = 101 */
		"LCDF",	/* id = 102 */
		"LCRIT",	/* id = 103 */
		"LDIST",	/* id = 104 */
		"LDISTG",	/* id = 105 */
		"LDIST2",	/* id = 106 */
		"LE",	/* id = 107 */
		"LOG",	/* id = 108 */
		"LOG10",	/* id = 109 */
		"LOG1P",	/* id = 110 */
		"LOG2",	/* id = 111 */
		"LMSSCL",	/* id = 112 */
		"LMSSCLW",	/* id = 113 */
		"LOWER",	/* id = 114 */
		"LPDF",	/* id = 115 */
		"LRAND",	/* id = 116 */
		"LT",	/* id = 117 */
		"MAD",	/* id = 118 */
		"MADW",	/* id = 119 */
		"MAX",	/* id = 120 */
		"MEAN",	/* id = 121 */
		"MEANW",	/* id = 122 */
		"MEDIAN",	/* id = 123 */
		"MEDIANW",	/* id = 124 */
		"MIN",	/* id = 125 */
		"MOD",	/* id = 126 */
		"MODE",	/* id = 127 */
		"MODEW",	/* id = 128 */
		"MUL",	/* id = 129 */
		"NAN",	/* id = 130 */
		"NEG",	/* id = 131 */
		"NEQ",	/* id = 132 */
		"NORM",	/* id = 133 */
		"NOT",	/* id = 134 */
		"NRAND",	/* id = 135 */
		"OR",	/* id = 136 */
		"PDIST",	/* id = 137 */
		"PDIST2",	/* id = 138 */
		"PERM",	/* id = 139 */
		"POP",	/* id = 140 */
		"PLM",	/* id = 141 */
		"PLMg",	/* id = 142 */
		"POINT",	/* id = 143 */
		"POW",	/* id = 144 */
		"PQUANT",	/* id = 145 */
		"PQUANTW",	/* id = 146 */
		"PSI",	/* id = 147 */
		"PV",	/* id = 148 */
		"QV",	/* id = 149 */
		"R2",	/* id = 150 */
		"R2D",	/* id = 151 */
		"RAND",	/* id = 152 */
		"RCDF",	/* id = 153 */
		"RCRIT",	/* id = 154 */
		"RINT",	/* id = 155 */
		"RMS",	/* id = 156 */
		"RMSW",	/* id = 157 */
		"RPDF",	/* id = 158 */
		"ROLL",	/* id = 159 */
		"ROTX",	/* id = 160 */
		"ROTY",	/* id = 161 */
		"SDIST",	/* id = 162 */
		"SDIST2",	/* id = 163 */
		"SAZ",	/* id = 164 */
		"SBAZ",	/* id = 165 */
		"SEC",	/* id = 166 */
		"SECD",	/* id = 167 */
		"SECH",	/* id = 168 */
		"SIGN",	/* id = 169 */
		"SIN",	/* id = 170 */
		"SINC",	/* id = 171 */
		"SIND",	/* id = 172 */
		"SINH",	/* id = 173 */
		"SKEW",	/* id = 174 */
		"SQR",	/* id = 175 */
		"SQRT",	/* id = 176 */
		"STD",	/* id = 177 */
		"STDW",	/* id = 178 */
		"STEP",	/* id = 179 */
		"STEPX",	/* id = 180 */
		"STEPY",	/* id = 181 */
		"SUB",	/* id = 182 */
		"SUM",	/* id = 183 */
		"TAN",	/* id = 184 */
		"TAND",	/* id = 185 */
		"TANH",	/* id = 186 */
		"TAPER",	/* id = 187 */
		"TN",	/* id = 188 */
		"TCRIT",	/* id = 189 */
		"TCDF",	/* id = 190 */
		"TPDF",	/* id = 191 */
		"TRIM",	/* id = 192 */
		"UPPER",	/* id = 193 */
		"VAR",	/* id = 194 */
		"VARW",	/* id = 195 */
		"WCDF",	/* id = 196 */
		"WCRIT",	/* id = 197 */
		"WPDF",	/* id = 198 */
		"WRAP",	/* id = 199 */
		"XOR",	/* id = 200 */
		"Y0",	/* id = 201 */
		"Y1",	/* id = 202 */
		"YLM",	/* id = 203 */
		"YLMg",	/* id = 204 */
		"YN",	/* id = 205 */
		"ZCRIT",	/* id = 206 */
		"ZCDF",	/* id = 207 */
		"ZPDF",	/* id = 208 */
		"HSV2LAB",	/* id = 209 */
		"HSV2RGB",	/* id = 210 */
		"HSV2XYZ",	/* id = 211 */
		"LAB2HSV",	/* id = 212 */
		"LAB2RGB",	/* id = 213 */
		"LAB2XYZ",	/* id = 214 */
		"RGB2HSV",	/* id = 215 */
		"RGB2LAB",	/* id = 216 */
		"RGB2XYZ",	/* id = 217 */
		"XYZ2HSV",	/* id = 218 */
		"XYZ2LAB",	/* id = 219 */
		"XYZ2RGB",	/* id = 220 */
		"DOT",	/* id = 221 */
		"BLEND",	/* id = 222 */
		"DAYNIGHT",	/* id = 223 */
		"VPDF",	/* id = 224 */
		"FISHER",	/* id = 225 */
		"CUMSUM",	/* id = 226 */
		"ASIND",	/* id = 227 */
		"ACOSD",	/* id = 228 */
		"ACOTD",	/* id = 229 */
		"ACSCD",	/* id = 230 */
		"ASECD",	/* id = 231 */
		"ATAND",	/* id = 232 */
		"ATAN2D",	/* id = 233 */
		"SADDLE",	/* id = 234 */
		"" /* last element is intentionally left blank */
	};

	void (*call_operator[GRDMATH_N_OPERATORS]) (struct GMT_CTRL *, struct GRDMATH_INFO *, struct GRDMATH_STACK **, unsigned int);

	struct GMT_HASH localhashnode[GRDMATH_N_OPERATORS];
	struct GRDMATH_INFO info;
	struct GRDMATH_CTRL *Ctrl = NULL;
	struct GMT_OPTION *opt = NULL, *list = NULL, *next = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */
	grdmath_expand_recall_cmd (options);	/* Avoid any conflicts with [RCL]@item and remote cache files */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if ((list = gmt_substitute_macros (GMT, options, "grdmath.macros")) == NULL) Return1 (GMT_RUNTIME_ERROR);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return1 (API->error);
	if ((error = parse (GMT, Ctrl, options)) != 0) Return1 (error);

	/*---------------------------- This is the grdmath main code ----------------------------*/

	gmt_grd_set_datapadding (GMT, true);	/* Turn on gridpadding when reading a subset */

	gmt_enable_threads (GMT);	/* Set number of active threads, if supported */
	GMT_Report (API, GMT_MSG_INFORMATION, "Perform reverse Polish notation calculations on grids\n");
	gmt_M_memset (&info, 1, struct GRDMATH_INFO);		/* Initialize here to not crash when Return gets called */
	gmt_M_memset (recall, GRDMATH_STORE_SIZE, struct GRDMATH_STORE *);
	gmt_M_memset (localhashnode, GRDMATH_N_OPERATORS, struct GMT_HASH);
	for (k = 0; k < GRDMATH_STACK_SIZE; k++) stack[k] = gmt_M_memory (GMT, NULL, 1, struct GRDMATH_STACK);
	gmt_set_pad (GMT, 2U);	/* Ensure space for BCs in case an API passed pad == 0 */

	/* The list is now the active options list. */
	/* Internally replace the = file sequence with an output option ->file*/

	for (opt = list; opt; opt = opt->next) {
		if (opt->option == GMT_OPT_INFILE && !strcmp (opt->arg, "=")) {	/* Found the output sequence */
			if (opt->next) {
				opt->next->option = GMT_OPT_OUTFILE2;
				/* Bypass the current opt in the linked list */
				opt->next->previous = opt->previous;
				opt->previous->next = opt->next;
				GMT_Delete_Option (API, opt, &list);
				opt = list;	/* GO back to start to avoid bad pointer */
			}
			else {	/* Standard output */
				GMT_Report (API, GMT_MSG_ERROR, "No output file specified via = file mechanism\n");
				Return (GMT_RUNTIME_ERROR);
			}
		}
	}

	if (gmt_hash_init (GMT, localhashnode, operator, GRDMATH_N_OPERATORS, GRDMATH_N_OPERATORS)) {
		Return (GMT_DIM_TOO_SMALL);
	}

	gmt_M_memset (wesn, 4, double);

	/* Read the first input file we encounter so we may determine dimensions and allocate space */

	for (opt = list; !G_in && opt; opt = opt->next) {	/* Look for a grid file, if given */
		if (!(opt->option == GMT_OPT_INFILE))	continue;	/* Skip command line options and output file */
		/* Skip table files given as argument to the LDIST, PDIST, POINT, INSIDE operators */
		next = opt->next;
		while (next && next->option != GMT_OPT_INFILE) next = next->next;	/* Skip any options splitting the operand OPERATOR sequence */
		if (next && !(strncmp (next->arg, "LDIST", 5U) && strncmp (next->arg, "PDIST", 5U) && strncmp (next->arg, "POINT", 5U) && strncmp (next->arg, "INSIDE", 6U))) continue;
		/* Filenames,  operators, some numbers and = will all have been flagged as input files by the parser */
		status = grdmath_decode_argument (GMT, opt, &value, localhashnode);		/* Determine what this is */
		if (status == GRDMATH_ARG_IS_BAD) Return (GMT_RUNTIME_ERROR);		/* Horrible */
		if (status != GRDMATH_ARG_IS_FILE) continue;				/* Skip operators and numbers */
		in_file = opt->arg;
		/* Read but request IO reset since the file (which may be a memory reference) will be read again later */
		/* Passing GMT_VIA_MODULE_INPUT since these are command line file arguments but processed here instead of by GMT_Init_IO */
		if ((G_in = GMT_Read_Data (API, GMT_IS_GRID|GMT_VIA_MODULE_INPUT, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY | GMT_IO_RESET, NULL, in_file, NULL)) == NULL) {	/* Get header only */
			Return (API->error);
		}
	}

	if (gmt_add_R_if_modern_and_true (GMT, THIS_MODULE_NEEDS, G_in == NULL))
		Return (API->error);

	subset = GMT->common.R.active[RSET];

	if (G_in) {	/* We read a gridfile header above, now update columns */
		if (GMT->common.R.active[RSET] && GMT->common.R.active[ISET]) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot use -I together with -R<gridfile>\n");
			Return (GMT_RUNTIME_ERROR);
		}
		else if  (GMT->common.R.active[GSET]) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot use -r when grid files are specified\n");
			Return (GMT_RUNTIME_ERROR);
		}
		if (subset) {	/* Gave -R and files: Read the subset to set the header properly */
			gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);
			/* Passing GMT_VIA_MODULE_INPUT since these are command line file arguments but processed here instead of by GMT_Init_IO */
			if (GMT_Read_Data (API, GMT_IS_GRID|GMT_VIA_MODULE_INPUT, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, in_file, G_in) == NULL) {	/* Get subset only */
				Return (API->error);
			}
		}
		if ((info.G = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_NONE, G_in)) == NULL) Return (API->error);
		GMT_Report (API, GMT_MSG_DEBUG, "Freeing G_in after duplication\n");
		if (GMT_Destroy_Data (API, &G_in) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_DEBUG, "Failed to free G_in after duplication\n");
			Return (API->error);
		}
	}
	else if (GMT->common.R.active[RSET] && GMT->common.R.active[ISET]) {	/* Must create from -R -I [-r] */
		/* Completely determine the header for the new grid; croak if there are issues.  No memory is allocated here. */
		if ((info.G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, NULL, NULL, \
			GMT_GRID_DEFAULT_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
		GMT->current.io.inc_code[GMT_X]	= GMT->current.io.inc_code[GMT_Y] = 0;	/* Must reset this since later we don't use GMT->common.R.inc but G->header->inc */
	}
	else {
		GMT_Report (API, GMT_MSG_ERROR, "Expression must contain at least one grid file or -R, -I\n");
		Return (GMT_RUNTIME_ERROR);
	}
	info.nm = info.G->header->nm;	info.size = info.G->header->size;

	/* Get x and y vectors (these extend onto the pad) */

	info.d_grd_x  = gmt_M_memory (GMT, NULL, info.G->header->mx, double);
	info.d_grd_y  = gmt_M_memory (GMT, NULL, info.G->header->my, double);
	info.d_grd_xn = gmt_M_memory (GMT, NULL, info.G->header->mx, double);
	info.d_grd_yn = gmt_M_memory (GMT, NULL, info.G->header->my, double);
	info.f_grd_x  = gmt_M_memory (GMT, NULL, info.G->header->mx, gmt_grdfloat);
	info.f_grd_y  = gmt_M_memory (GMT, NULL, info.G->header->my, gmt_grdfloat);
	info.f_grd_xn = gmt_M_memory (GMT, NULL, info.G->header->mx, gmt_grdfloat);
	info.f_grd_yn = gmt_M_memory (GMT, NULL, info.G->header->my, gmt_grdfloat);
	for (k = 0, start = info.G->header->pad[XLO], colx = -start; k < (int)info.G->header->mx; colx++, k++) info.d_grd_x[k] = gmt_M_grd_col_to_x (GMT, colx, info.G->header);
	for (k = 0, start = info.G->header->pad[YHI], rowx = -start; k < (int)info.G->header->my; rowx++, k++) info.d_grd_y[k] = gmt_M_grd_row_to_y (GMT, rowx, info.G->header);
	if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Make sure latitudes remain in range; if not apply geographic BC */
		for (kk = 0; kk < info.G->header->pad[YHI]; kk++)
			if (info.d_grd_y[kk] > 90.0) info.d_grd_y[kk] = 2.0 * 90.0 - info.d_grd_y[kk];
		for (kk = 0, k = info.G->header->my - info.G->header->pad[YLO]; kk < info.G->header->pad[YLO]; kk++, k++)
			if (info.d_grd_y[k] < -90.0) info.d_grd_y[k] = -2.0 * 90.0 - info.d_grd_y[k];
	}
	for (k = 0; k < (int)info.G->header->mx; k++) info.f_grd_x[k] = (gmt_grdfloat)info.d_grd_x[k];
	for (k = 0; k < (int)info.G->header->my; k++) info.f_grd_y[k] = (gmt_grdfloat)info.d_grd_y[k];
	off = 0.5 * (info.G->header->wesn[XHI] + info.G->header->wesn[XLO]);
	scale = 2.0 / (info.G->header->wesn[XHI] - info.G->header->wesn[XLO]);
	for (kk = 0; kk < info.G->header->mx; kk++) {
		info.d_grd_xn[kk] = (info.d_grd_x[kk] - off) * scale;
		info.f_grd_xn[kk] = (gmt_grdfloat)info.d_grd_xn[kk];
	}
	off = 0.5 * (info.G->header->wesn[YHI] + info.G->header->wesn[YLO]);
	scale = 2.0 / (info.G->header->wesn[YHI] - info.G->header->wesn[YLO]);
	for (kk = 0; kk < info.G->header->my; kk++) {
		info.d_grd_yn[kk] = (info.d_grd_y[kk] - off) * scale;
		info.f_grd_yn[kk] = (gmt_grdfloat)info.d_grd_yn[kk];
	}
	x_noise = GMT_CONV4_LIMIT * info.G->header->inc[GMT_X];	y_noise = GMT_CONV4_LIMIT * info.G->header->inc[GMT_Y];
	info.dx = gmt_M_memory (GMT, NULL, info.G->header->my, double);
	if (Ctrl->D.force) Ctrl->D.set = gmt_shore_adjust_res (GMT, Ctrl->D.set, true);
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

	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_PI]    = M_PI;
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_E]     = M_E;
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_EULER] = M_EULER;
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_PHI]   = M_PHI;
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_F_EPS] = FLT_EPSILON;
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_XMIN]  = info.G->header->wesn[XLO];
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_XMAX]  = info.G->header->wesn[XHI];
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_XINC]  = info.G->header->inc[GMT_X];
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_NX]    = info.G->header->n_columns;
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_YMIN]  = info.G->header->wesn[YLO];
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_YMAX]  = info.G->header->wesn[YHI];
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_YINC]  = info.G->header->inc[GMT_Y];
	special_symbol[GRDMATH_ARG_IS_PI-GRDMATH_ARG_IS_NY]    = info.G->header->n_rows;

	GMT_Report (API, GMT_MSG_INFORMATION, "");

	nstack = 0;

	for (opt = list, error = false; !error && opt; opt = opt->next) {

		/* First check if we should skip optional arguments */

		if (strchr ("ACDIMNRVbfnr-" GMT_OPT("F") GMT_ADD_x_OPT, opt->option)) continue;
		if (opt->option == 'S' && nstack > 1) {	/* Turn on reducing stack behavior */
			opt = opt->next;	/* Skip to actual operator */
			if (grdmath_collapse_stack (GMT, &info, stack, nstack, opt->arg)) continue;	/* Failed, just ignore */
			nstack = 1;	/* Collapsed back to a single item on stack */
			continue;
		}

		op = grdmath_decode_argument (GMT, opt, &value, localhashnode);
		if (op == GRDMATH_ARG_IS_BAD) Return (GMT_RUNTIME_ERROR);		/* Horrible way to go... */

		if (op == GRDMATH_ARG_IS_SAVE) {	/* Time to save the current stack to output and pop the stack */
			struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
			if (nstack <= 0) {
				GMT_Report (API, GMT_MSG_ERROR, "No items on stack are available for output!\n");
				Return (GMT_RUNTIME_ERROR);
			}

			if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
				if (opt->next) GMT_Message (API, GMT_TIME_NONE, "= %s", opt->arg);
				else GMT_Message (API, GMT_TIME_NONE, "= %s\n", opt->arg);
			}

			if (n_items && (new_stack < 0 || stack[nstack-1]->constant)) {	/* Only a constant provided, set grid accordingly */
				if (!stack[nstack-1]->G)
					stack[nstack-1]->G = grdmath_alloc_stack_grid (GMT, info.G);
				if (stack[nstack-1]->constant) {
					gmt_M_grd_loop (GMT, info.G, row, col, node) stack[nstack-1]->G->data[node] = (gmt_grdfloat)stack[nstack-1]->factor;
				}
			}
			this_stack = nstack - 1;
			gmt_grd_init (GMT, stack[this_stack]->G->header, options, true);	/* Update command history only */

			gmt_set_pad (GMT, API->pad);	/* Reset to session default pad before output */

			HH = gmt_get_H_hidden (stack[this_stack]->G->header);
			if (Ctrl->C.active) {	/* Keep or replace the grid's default CPT */
				if (Ctrl->C.cpt) {	/* Setting another default CPT */
					if (HH->cpt) gmt_M_str_free (HH->cpt);	/* Must wipe any CPT inherited from input grid */
					HH->cpt = strdup (Ctrl->C.cpt);
					API->meta.ignore_remote_cpt = true;	/* Since we just specified something else */
				}
				else	/* Want to keep what we have or assign the remote CPT if this is a remote grid */
					API->meta.ignore_remote_cpt = false;	/* Just in case it had been set elsewhere */
			}
			else {	/* Default is to get rid of the grid's default CPT since we cannot know if it is valid anymore */
				gmt_M_str_free (HH->cpt);	/* Must wipe any CPT inherited from input grid */
				API->meta.ignore_remote_cpt = true;	/* Since we cannot keep track of what grdmath did to this grid */
			}

			if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, stack[this_stack]->G)) Return (API->error);
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, opt->arg, stack[this_stack]->G) != GMT_NOERROR) {
				Return (API->error);
			}
			gmt_set_pad (GMT, 2U);			/* Ensure space for BCs in case an API passed pad == 0 */
			if (n_items) nstack--;	/* Pop off the current stack if there is one */
			new_stack = nstack;
			continue;
		}

		if (op != GRDMATH_ARG_IS_FILE && !gmt_access (GMT, opt->arg, R_OK)) GMT_Message (API, GMT_TIME_NONE, "The number or operator %s may be confused with an existing file named %s!  The file will be ignored.\n", opt->arg, opt->arg);

		if (op < GRDMATH_ARG_IS_OPERATOR) {	/* File name or factor */
			next = opt->next;
			while (next && next->option != GMT_OPT_INFILE) next = next->next;	/* Skip any options splitting the operand OPERATOR sequence */
			if (next && op == GRDMATH_ARG_IS_FILE && !(strncmp (next->arg, "LDIST", 5U) && strncmp (next->arg, "PDIST", 5U) && strncmp (next->arg, "POINT", 5U) && strncmp (next->arg, "INSIDE", 6U))) op = GRDMATH_ARG_IS_ASCIIFILE;

			if (nstack == GRDMATH_STACK_SIZE) {	/* Stack overflow */
				error = true;
				continue;
			}
			n_items++;
			if (op == GRDMATH_ARG_IS_NUMBER) {
				stack[nstack]->constant = true;
				stack[nstack]->factor = value;
				error = false;
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "%g ", stack[nstack]->factor);
				nstack++;
				continue;
			}
			else if (op <= GRDMATH_ARG_IS_PI && op >= GRDMATH_ARG_IS_NY) {
				stack[nstack]->constant = true;
				stack[nstack]->factor = special_symbol[GRDMATH_ARG_IS_PI-op];
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "%g ", stack[nstack]->factor);
				nstack++;
				continue;
			}
			else if (op == GRDMATH_ARG_IS_STORE) {
				/* Duplicate stack into stored memory location associated with specified label */
				int last = nstack - 1;
				bool added_new = false;
				if (nstack == 0) {
					GMT_Report (API, GMT_MSG_ERROR, "No items on stack to put into stored memory!\n");
					Return (GMT_RUNTIME_ERROR);
				}
				if ((label = grdmath_setlabel (GMT, opt->arg)) == NULL) Return (GMT_RUNTIME_ERROR);
				if ((k = grdmath_find_stored_item (GMT, recall, n_stored, label)) != GMT_NOTSET) {
					GMT_Report (API, GMT_MSG_DEBUG, "Stored memory cell %d named %s is overwritten with new information\n", k, label);
					if (!stack[last]->constant) {	/* Must copy over the grid - and allocate if not yet done */
						if (recall[k]->stored.G == NULL) recall[k]->stored.G = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, stack[last]->G);
						gmt_M_memcpy (recall[k]->stored.G->data, stack[last]->G->data, info.size, gmt_grdfloat);
					}
				}
				else {	/* Need new named storage place */
					k = n_stored;
					recall[k] = gmt_M_memory (GMT, NULL, 1, struct GRDMATH_STORE);
					recall[k]->label = strdup (label);
					if (!stack[last]->constant) recall[k]->stored.G = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_DATA, stack[last]->G);
					added_new = true;
					GMT_Report (API, GMT_MSG_DEBUG, "Stored memory cell %d named %s is created with new information\n", k, label);
				}
				recall[k]->stored.constant = stack[last]->constant;
				recall[k]->stored.factor = stack[last]->factor;
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "[--> %s] ", recall[n_stored]->label);
				if (added_new) n_stored++;	/* We added a new item */
				continue;	/* Just go back and process next item */
			}
			else if (op == GRDMATH_ARG_IS_RECALL) {
				/* Add to stack from stored memory location */
				if ((label = grdmath_setlabel (GMT, opt->arg)) == NULL) Return (GMT_RUNTIME_ERROR);
				if ((k = grdmath_find_stored_item (GMT, recall, n_stored, label)) == GMT_NOTSET) {
					GMT_Report (API, GMT_MSG_ERROR, "No stored memory item with label %s exists!\n", label);
					Return (GMT_RUNTIME_ERROR);
				}
				if (recall[k]->stored.constant) {	/* Place a stored constant on the stack */
					stack[nstack]->constant = true;
					stack[nstack]->factor = recall[k]->stored.factor;
				}
				else {	/* Place the stored grid on the stack */
					stack[nstack]->constant = false;
					if (!stack[nstack]->G)
						stack[nstack]->G = grdmath_alloc_stack_grid (GMT, info.G);
					gmt_M_memcpy (stack[nstack]->G->data, recall[k]->stored.G->data, info.size, gmt_grdfloat);
				}
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "@%s ", recall[k]->label);
				nstack++;
				continue;
			}
			else if (op == GRDMATH_ARG_IS_CLEAR) {
				/* Free stored memory location */
				if ((label = grdmath_setlabel (GMT, opt->arg)) == NULL) Return (GMT_RUNTIME_ERROR);
				if ((k = grdmath_find_stored_item (GMT, recall, n_stored, label)) == GMT_NOTSET) {
					GMT_Report (API, GMT_MSG_ERROR, "No stored memory item with label %s exists!\n", label);
					Return (GMT_RUNTIME_ERROR);
				}
				if (recall[k]->stored.G && GMT_Destroy_Data (API, &recall[k]->stored.G) != GMT_NOERROR) {
					GMT_Report (API, GMT_MSG_ERROR, "Failed to free recall item %d\n", k);
				}

				gmt_M_str_free (recall[k]->label);
				gmt_M_free (GMT, recall[k]);
				while (n_stored && k == (int)(n_stored-1) && !recall[k]) k--, n_stored--;	/* Chop off trailing NULL cases */
				continue;
			}

			/* Here we need a matrix */

			stack[nstack]->constant= false;

			if (op == GRDMATH_ARG_IS_X_MATRIX) {		/* Need to set up matrix of x-values */
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "X ");
				if (!stack[nstack]->G) stack[nstack]->G = grdmath_alloc_stack_grid (GMT, info.G);
				grdmath_row_padloop (GMT, info.G, row, node) {
					node = row * info.G->header->mx;
					gmt_M_memcpy (&stack[nstack]->G->data[node], info.f_grd_x, info.G->header->mx, gmt_grdfloat);
				}
			}
			else if (op == GRDMATH_ARG_IS_x_MATRIX) {		/* Need to set up matrix of normalized x-values */
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "XNORM ");
				if (!stack[nstack]->G) stack[nstack]->G = grdmath_alloc_stack_grid (GMT, info.G);
				grdmath_row_padloop (GMT, info.G, row, node) {
					node = row * info.G->header->mx;
					gmt_M_memcpy (&stack[nstack]->G->data[node], info.f_grd_xn, info.G->header->mx, gmt_grdfloat);
				}
			}
			else if (op == GRDMATH_ARG_IS_XCOL_MATRIX) {		/* Need to set up matrix of column numbers */
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "XCOL ");
				if (!stack[nstack]->G) stack[nstack]->G = grdmath_alloc_stack_grid (GMT, info.G);
				grdmath_grd_padloop (GMT, info.G, row, col, node) stack[nstack]->G->data[node] = (gmt_grdfloat)(col - stack[nstack]->G->header->pad[XLO]);
			}
			else if (op == GRDMATH_ARG_IS_Y_MATRIX) {	/* Need to set up matrix of y-values */
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "Y ");
				if (!stack[nstack]->G) stack[nstack]->G = grdmath_alloc_stack_grid (GMT, info.G);
				grdmath_grd_padloop (GMT, info.G, row, col, node) stack[nstack]->G->data[node] = info.f_grd_y[row];
			}
			else if (op == GRDMATH_ARG_IS_y_MATRIX) {	/* Need to set up matrix of normalized y-values */
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "YNORM ");
				if (!stack[nstack]->G) stack[nstack]->G = grdmath_alloc_stack_grid (GMT, info.G);
				grdmath_grd_padloop (GMT, info.G, row, col, node) stack[nstack]->G->data[node] = info.f_grd_yn[row];
			}
			else if (op == GRDMATH_ARG_IS_YROW_MATRIX) {		/* Need to set up matrix of row numbers */
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "YROW ");
				if (!stack[nstack]->G) stack[nstack]->G = grdmath_alloc_stack_grid (GMT, info.G);
				grdmath_grd_padloop (GMT, info.G, row, col, node) stack[nstack]->G->data[node] = (gmt_grdfloat)(row - stack[nstack]->G->header->pad[YHI]);
			}
			else if (op == GRDMATH_ARG_IS_NODE_MATRIX) {		/* Need to set up matrix of continuous node numbers (pad not considered) */
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "NODE ");
				if (!stack[nstack]->G) stack[nstack]->G = grdmath_alloc_stack_grid (GMT, info.G);
				gmt_M_grd_loop (GMT, info.G, row, col, node) stack[nstack]->G->data[node] = (gmt_grdfloat)gmt_M_ij0(stack[nstack]->G->header,row,col);
			}
			else if (op == GRDMATH_ARG_IS_NODEP_MATRIX) {		/* Need to set up matrix of node numbers (in presence of pad) */
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "NODEP ");
				if (!stack[nstack]->G) stack[nstack]->G = grdmath_alloc_stack_grid (GMT, info.G);
				gmt_M_grd_loop (GMT, info.G, row, col, node) stack[nstack]->G->data[node] = (gmt_grdfloat)gmt_M_ijp(stack[nstack]->G->header,row,col);
			}
			else if (op == GRDMATH_ARG_IS_ASCIIFILE) {
				gmt_M_str_free (info.ASCII_file);
				if (!stack[nstack]->G) stack[nstack]->G = grdmath_alloc_stack_grid (GMT, info.G);
				info.ASCII_file = strdup (opt->arg);
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "(%s) ", opt->arg);
			}
			else if (op == GRDMATH_ARG_IS_FILE) {		/* Filename given */
				if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "%s ", opt->arg);
				/* Passing GMT_VIA_MODULE_INPUT since these are command line file arguments but processed here instead of by GMT_Init_IO */
				if ((stack[nstack]->G = GMT_Read_Data (API, GMT_IS_GRID|GMT_VIA_MODULE_INPUT, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, opt->arg, NULL)) == NULL) {	/* Get header only */
					Return (API->error);
				}
				if (!subset && !gmt_M_grd_same_shape (GMT, stack[nstack]->G, info.G)) {
					GMT_Report (API, GMT_MSG_ERROR, "grid files not of same size!\n");
					Return (GMT_RUNTIME_ERROR);
				}
				else if (!Ctrl->N.active && (!subset && (fabs (stack[nstack]->G->header->wesn[XLO] - info.G->header->wesn[XLO]) > x_noise || fabs (stack[nstack]->G->header->wesn[XHI] - info.G->header->wesn[XHI]) > x_noise ||
					fabs (stack[nstack]->G->header->wesn[YLO] - info.G->header->wesn[YLO]) > y_noise || fabs (stack[nstack]->G->header->wesn[YHI] - info.G->header->wesn[YHI]) > y_noise))) {
					GMT_Report (API, GMT_MSG_ERROR, "grid files do not cover the same area!\n");
					Return (GMT_RUNTIME_ERROR);
				}
				/* Passing GMT_VIA_MODULE_INPUT since these are command line file arguments but processed here instead of by GMT_Init_IO */
				if (GMT_Read_Data (API, GMT_IS_GRID|GMT_VIA_MODULE_INPUT, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, opt->arg, stack[nstack]->G) == NULL) {	/* Get data */
					Return (API->error);
				}
			}
			nstack++;
			continue;
		}

		/* Here we have an operator */

		if ((new_stack = nstack - consumed_operands[op] + produced_operands[op]) >= GRDMATH_STACK_SIZE) {
			GMT_Report (API, GMT_MSG_ERROR, "Stack overflow (%s)\n", opt->arg);
			Return (GMT_RUNTIME_ERROR);
		}

		if (nstack < consumed_operands[op]) {
			GMT_Report (API, GMT_MSG_ERROR, "Operation \"%s\" requires %d operands\n", operator[op], consumed_operands[op]);
			Return (GMT_RUNTIME_ERROR);
		}

		n_items++;
		if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "%s ", operator[op]);

		for (k = produced_operands[op] - consumed_operands[op]; k > 0; k--) {
			if (stack[nstack+k-1]->G) continue;

			/* Must make space for more */
			stack[nstack+k-1]->G = grdmath_alloc_stack_grid (GMT, info.G);
		}

		/* If operators operates on constants only we may have to make space as well */

		for (kk = 0, k = nstack - consumed_operands[op]; kk < produced_operands[op]; kk++, k++) {
			if (stack[k]->constant && !stack[k]->G)
				stack[k]->G = grdmath_alloc_stack_grid (GMT, info.G);
		}

		gmt_set_column_type (GMT, GMT_OUT, GMT_Z, GMT_IS_FLOAT);

		pos = (consumed_operands[op]) ? nstack - 1 : nstack;
		(*call_operator[op]) (GMT, &info, stack, pos);	/* Do it */

		if (info.error) Return (info.error);	/* Got an error inside the operator */

		nstack = new_stack;
		for (kk = 1; kk <= produced_operands[op]; kk++) stack[nstack-kk]->constant = false;	/* Now filled with grid */
	}

	/* Clean-up time */

	for (kk = 0; kk < n_stored; kk++) {	/* Free up stored STO/RCL memory */
		if (recall[kk]->stored.G && GMT_Destroy_Data (API, &recall[kk]->stored.G) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_ERROR, "Failed to free recall item %d\n", kk);
		}
		gmt_M_str_free (recall[kk]->label);
		gmt_M_free (GMT, recall[kk]);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) GMT_Message (API, GMT_TIME_NONE, "\n");

	if (nstack > 0) GMT_Report (API, GMT_MSG_WARNING, "%d more operands left on the stack!\n", nstack);

	Return (GMT_NOERROR);
}
