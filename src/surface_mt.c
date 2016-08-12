/*--------------------------------------------------------------------
 *	$Id: surface.c 15861 2016-03-10 04:17:28Z pwessel $
 *
 *	Copyright (c) 1991-2016 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * surface.c: a gridding program using splines in tension.
 * Reads xyz Cartesian triples and fits a surface to the data.
 * The surface satisfies (1 - T) D4 z - T D2 z = 0,
 * where D4 is the 2-D biharmonic operator, D2 is the
 * 2-D Laplacian, and T is a "tension factor" between 0 and 1.
 * End member T = 0 is the classical minimum curvature
 * surface.  T = 1 gives a harmonic surface.  Use T = 0.25
 * or so for potential data; something more for topography.
 *
 * Program includes overrelaxation for fast convergence and
 * automatic optimal grid factorization.
 *
 * See reference Smith & Wessel (Geophysics, 3, 293-305, 1990) for details.
 *
 * Authors:	Walter H. F. Smith and Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * This 5.3 version is a complete re-write that differs from the original code:
 * 1. It uses scan-line grid structures, so we no longer need to transpose grids
 * 2. It keeps node spacing at 1, thus we no longer need complicated strides between
 *    active nodes.  That spacing is now always 1 and we expand the grid as we
 *    go to larger grids (i.e., adding more nodes).
 * 3. It relies more on functions and macros from GMT to handle rows/cols/node calculations.
 * 4. TESTING: It enables multiple threads via OpenMP [if compiled that way].
 */

#define THIS_MODULE_NAME	"surface_mt"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Grid table data using adjustable tension continuous curvature splines"
#define THIS_MODULE_KEYS	"<D{,DD(,LG(,GG}"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:RVabdfhirs" GMT_ADD_x_OPT GMT_OPT("FH")

struct SURFACE_CTRL {
	struct A {	/* -A<aspect_ratio> */
		bool active;
		double value;
	} A;
	struct C {	/* -C<converge_limit> */
		bool active;
		unsigned int mode;	/* 1 if given as fraction */
		double value;
	} C;
	struct D {	/* -D<line.xyz>[+d] */
		bool active;
		bool debug;
		char *file;	/* Name of file with breaklines */
	} D;
#ifdef DEBUG_SURF
	struct E {	/* -E[+o|p|m+1+a+s][<name>] */
		bool active;
		bool once;
		bool save;
	} E;
#endif
	struct G {	/* -G<file> */
		bool active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		bool active;
		double inc[2];
	} I;
	struct L {	/* -Ll|u<limit> */
		bool active;
		char *file[2];
		double limit[2];
		unsigned int mode[2];
	} L;
	struct N {	/* -N<max_iterations> */
		bool active;
		unsigned int value;
	} N;
	struct Q {	/* -Q */
		bool active;
	} Q;
	struct S {	/* -S<radius>[m|s] */
		bool active;
		double radius;
		char unit;
	} S;
	struct T {	/* -T<tension>[i][b] */
		bool active;
		double b_tension, i_tension;
	} T;
	struct W {	/* -W[<logfile>] */
		bool active;
		char *file;
	} W;
	struct Z {	/* -Z<over_relaxation_parameter> */
		bool active;
		double value;
	} Z;
};

#ifdef DEBUG_SURF
/* Paul Wessel testing only!
 * Compiled for debug to simulate grid updating such as under OpenMP
 * but without actually running OpenMP. The -E option supports:
 * -E[+a|o|m|p||s1][<name>]
 * +1 : Force a single iteration cycle (no multigrid)
 * +a : Determine average data constraint [default select just nearest]
 * +o : Original algorithm (update on a single grid) [Default]
 * +m : Two grids, use memcpy to move latest to old, then update on new grid
 * +p : Two grids, use pointers to alternate old and new grid.
 * +s : Save data constraints & breakline table after throw_away_unusables is done.
 * Upper case O|M|P will write normalized (nondimensional) grids [Default restores plane and scale]
 * Optionally, add the prefix name for the intermediate grids [surface].
 * If OPENMP is also selected then only mode +o|O is supported.
 */
int debug = 0;				/* No debug is the default, even when compiled this way */
bool nondimensional = false;		/* True means we save nondimensional grids */
bool save_intermediate = false;		/* True means we save intermediate grids after each iteration */
bool average = false;			/* True means we should average nearest data constraints for early grids */
char debug_prefix[32] = {"surface"};	/* Prefix for intermediate grids */
float *tmp_grid = NULL;			/* Temp grid used for writing debug grids */
#define PARALLEL_MODE			/* Use an array to relate node index to Briggs index */
#endif
#ifdef _OPENMP				/* Running the real thing with -fopenmp compilation on gcc */
#define PARALLEL_MODE			/* Use an array to relate node index to Briggs index */
#endif

/* Various constants used in surface */

#define SURFACE_OUTSIDE			LONG_MAX	/* Index number indicating data is outside usable area */
#define SURFACE_CONV_LIMIT		0.0001		/* Default is 100 ppm of data range as convergence criterion */
#define SURFACE_MAX_ITERATIONS		500		/* Default iterations at final grid size */
#define SURFACE_OVERRELAXATION		1.4		/* Default over-relaxation value */
#define SURFACE_CLOSENESS_FACTOR	0.05		/* A node is considered known if the nearest data is within 0.05 of a gridspacing of the node */
#define SURFACE_IS_UNCONSTRAINED	0		/* Various constants used to flag nearest node */
#define SURFACE_DATA_IS_IN_QUAD1	1		/* Nearnest data constraint is in quadrant 1 relative to current node */
#define SURFACE_DATA_IS_IN_QUAD2	2		/* Nearnest data constraint is in quadrant 2 relative to current node */
#define SURFACE_DATA_IS_IN_QUAD3	3		/* Nearnest data constraint is in quadrant 3 relative to current node */
#define SURFACE_DATA_IS_IN_QUAD4	4		/* Nearnest data constraint is in quadrant 4 relative to current node */
#define SURFACE_IS_CONSTRAINED		5		/* Node has already been set */
#define SURFACE_UNCONSTRAINED		0		/* Use coefficients for unconstrained node */
#define SURFACE_CONSTRAINED		1		/* Use coefficients for constrained node */
#define SURFACE_BREAKLINE		1		/* Flag for breakline constraints that should overrule data constraints */

/* Misc. macros used to get row, cols, index, node, x, y, plane trend etc. */

/* Go from row, col to grid node location, accounting for the 2 boundary rows and columns: */
#define row_col_to_node(row,col,mx) ((uint64_t)(((int64_t)(row)+(int64_t)2)*((int64_t)(mx))+(int64_t)(col)+(int64_t)2))
/* Go from row, col to index array position, no boundary rows and columns involved: */
#define row_col_to_index(row,col,n_columns) ((uint64_t)((int64_t)(row)*((int64_t)(n_columns))+(int64_t)(col)))
/* Go from data x to fractional column x: */
#define x_to_fcol(x,x0,idx) (((x) - (x0)) * (idx))
/* Go from x to grid integer column knowing it is a gridline-registered grid: */
#define x_to_col(x,x0,idx) ((int64_t)floor(x_to_fcol(x,x0,idx)+0.5))
/* Go from data y to fractional row y_up measured from south (y_up = 0) towards north (y_up = n_rows-1): */
#define y_to_frow(y,y0,idy) (((y) - (y0)) * (idy))
/* Go from y to row (row = 0 is north) knowing it is a gridline-registered grid: */
#define y_to_row(y,y0,idy,n_rows) ((n_rows) - 1 - x_to_col(y,y0,idy))
/* Go from col to x knowing it is a gridline-registered grid: */
#define col_to_x(col,x0,x1,dx,n_columns) (((int)(col) == (int)((n_columns)-1)) ? (x1) : (x0) + (col) * (dx))
/* Go from row to y knowing it is a gridline-registered grid: */
#define row_to_y(row,y0,y1,dy,n_rows) (((int)(row) == (int)((n_rows)-1)) ? (y0) : (y1) - (row) * (dy))
/* Evaluate the change in LS plane from (0,0) to (xx,y_up): */
#define evaluate_trend(C,xx,y_up) (C->plane_sx * (xx) + C->plane_sy * (y_up))
/* Evaluate the LS plane at location (xx,y_up) (this includes the intercept): */
#define evaluate_plane(C,xx,y_up) (C->plane_icept + evaluate_trend (C, xx, y_up))
/* Extract col from index: */
#define index_to_col(index,n_columns) ((index) % (n_columns))
/* Extract row from index: */
#define index_to_row(index,n_columns) ((index) / (n_columns))

/* The 4 indices refers to points A-D in Figure A-1 in the reference */
#if 0
static unsigned int p[5][4] = {	/* Indices into C->offset for each of the 4 quadrants, i.e., C->offset[p[quadrant][k]]], k = 0-3 */
	{ 0, 0, 0,  0},	/* This row is not used */
	{10, 9, 5,  1},	/* Indices for 1st quadrant */
	{ 8, 9, 6,  3},	/* Indices for 2nd quadrant */
	{ 1, 2, 6, 10},	/* Indices for 3rd quadrant */
	{ 3, 2, 5,  8}	/* Indices for 4th quadrant */
};
#endif
static unsigned int p[5][4] = {	/* Indices into C->offset for each of the 4 quadrants, i.e., C->offset[p[quadrant][k]]], k = 0-3 */
	{ 0, 0, 0,  0},	/* This row is not used */
	{ 1, 5, 9, 10},	/* Indices for 1st quadrant */
	{ 3, 6, 9,  8},	/* Indices for 2nd quadrant */
	{ 1, 2, 6, 10},	/* Indices for 3rd quadrant */
	{ 3, 2, 5,  8}	/* Indices for 4th quadrant */
};

enum surface_nodes {	/* Node location relative to current node, using compass directions */
	N2 = 0, NW, N1, NE, W2, W1, E1, E2, SW, S1, SE, S2 };

enum surface_bound { LO = 0, HI = 1 };

enum surface_limit { NONE = 0, DATA = 1, VALUE = 2, SURFACE = 3 };

enum surface_conv { BY_VALUE = 0, BY_PERCENT = 1 };

enum surface_iter { GRID_NODES = 0, GRID_DATA = 1 };

struct SURFACE_DATA {	/* Data point and index to node it currently constrains  */
	float x, y, z;
	unsigned int kind;
	uint64_t index;
#ifdef DEBUG	/* For debugging purposes only - it is the original input data point number before sorting */
	int64_t number;
#endif
};

struct SURFACE_BRIGGS {		/* Coefficients in Taylor series for Laplacian(z) a la I. C. Briggs (1974)  */
	float b[6];
};

struct SURFACE_SEARCH {		/* Things needed inside compare function will be passed to qsort_r */
	int current_nx;		/* Number of nodes in y-dir for a given grid factor */
	int current_ny;		/* Number of nodes in y-dir for a given grid factor */
	double inc[2];		/* Size of each grid cell for a given grid factor */
	double wesn[4];		/* Grid domain */
};

struct SURFACE_INFO {	/* Control structure for surface setup and execution */
	size_t n_alloc;			/* Number of data point positions allocated */
	uint64_t npoints;		/* Number of data points */
	uint64_t node_sw_corner;	/* Node index of southwest interior grid corner for current stride */
	uint64_t node_se_corner;	/* Node index of southeast interior grid corner for current stride */
	uint64_t node_nw_corner;	/* Node index of northwest interior grid corner for current stride */
	uint64_t node_ne_corner;	/* Node index of northeast interior grid corner for current stride */
	uint64_t n_empty;		/* No of unconstrained nodes at initialization  */
	uint64_t nxny;			/* Total number of grid nodes without boundaries  */
	uint64_t mxmy;			/* Total number of grid nodes with padding */
	uint64_t total_iterations;	/* Total iterations so far. */
#ifdef PARALLEL_MODE
	uint64_t *briggs_index;		/* Since we cannot access Briggs array sequentially when multiple threads */
#endif
	FILE *fp_log;			/* File pointer to log file, if -W is selected */
	struct SURFACE_DATA *data;	/* All the data constraints */
	struct SURFACE_BRIGGS *Briggs;	/* Array with Briggs 6-coefficients per nearest active data constraint */
	struct GMT_GRID *Grid;		/* The final grid */
	struct GMT_GRID *Bound[2];	/* Optional grids for lower and upper limits on the solution */
	struct SURFACE_SEARCH info;	/* Information needed by the compare function passed to qsort_r */
	unsigned int n_factors;		/* Number of factors in common for the dimensions (n_rows-1, n_columns-1) */
	unsigned int factors[32];	/* Array of these ommon factors */
	unsigned int set_limit[2];	/* For low and high: NONE = unconstrained, DATA = by min data value, VALUE = by user value, SURFACE by a grid */
	unsigned int max_iterations;	/* Max iterations per call to iterate */
	unsigned int converge_mode; 	/* BY_PERCENT if -C set fractional convergence limit [BY_VALUE] */
	unsigned int p[5][4];		/* Arrays with four nodes as function of quadrant in constrained fit */
	int current_stride;		/* Current node spacings relative to final spacing  */
	int previous_stride;		/* Previous node spacings relative to final spacing  */
	int n_columns;				/* Number of nodes in x-dir. (Final grid) */
	int n_rows;				/* Number of nodes in y-dir. (Final grid) */
	int mx;				/* Width of final grid including padding */
	int my;				/* Height of final grid including padding */
	int current_nx;			/* Number of nodes in x-dir for current stride */
	int current_ny;			/* Number of nodes in y-dir for current stride */
	int current_mx;			/* Number of current nodes in x-dir plus 4 extra columns */
	int previous_nx;		/* Number of nodes in x-dir for previous stride */
	int previous_ny;		/* Number of nodes in y-dir for previous stride */
	int previous_mx;		/* Number of current nodes in x-dir plus 4 extra columns */
	int current_mxmy;		/* Total number of grid nodes with padding */
	int offset[12];			/* Node-indices shifts of 12 nearby points relative center node */
	unsigned char *status;		/* Array with node status or quadrants */
	char mode_type[2];		/* D = include data points when iterating, I = just interpolate from larger grid */
	char format[GMT_BUFSIZ];	/* Format statement used in some messages */
	char *limit_file[2];		/* Pointers to grids with low and high limits, if selected */
	bool periodic;			/* true if geographic grid and west-east == 360 */
	bool constrained;		/* true if set_limit[LO] or set_limit[HI] is true */
	bool logging;			/* true if -W was specified */
#ifdef PARALLEL_MODE
	float *alternate_grid;		/* Used in iterate when we cannot write to the same grid across all threads */
#endif
	double limit[2];		/* Low and hight constrains on range of solution */
	double inc[2];			/* Size of each grid cell for current grid factor */
	double r_inc[2];		/* Reciprocal grid spacings  */
	double converge_limit;		/* Convergence limit */
	double radius;			/* Search radius for initializing grid  */
	double tension;			/* Tension parameter on the surface  */
	double boundary_tension;	/* Tension parameter at the boundary */
	double interior_tension;	/* Tension parameter in the interior */
	double z_mean;			/* Mean value of the data constraints z */
	double z_rms;			/* Root mean square range of z after removing planar trend  */
	double r_z_rms;			/* Reciprocal of z_rms (to avoid dividing) */
	double plane_icept;		/* Intercept of best fitting plane to data  */
	double plane_sx;		/* Slope of best fitting plane to data in x-direction */
	double plane_sy;		/* Slope of best fitting plane to data in y-direction */
	double *fraction;		/* Hold fractional increments of row and column used in fill_in_forecast */
	double coeff[2][12];		/* Coefficients for 12 nearby nodes, for constrained [0] and unconstrained [1] nodes */
	double relax_old, relax_new;	/* Coefficients for relaxation factor to speed up convergence */
	double wesn_orig[4];		/* Original -R domain as we might have shifted it due to -r */
	double alpha;			/* Aspect ratio dy/dx (1 for square pixels) */
	double a0_const_1, a0_const_2;	/* Various constants for off gridnode point equations */
	double alpha2, e_m2, one_plus_e2;
	double eps_p2, eps_m2, two_plus_ep2;
	double two_plus_em2;
#ifdef DEBUG_SURF
	bool save;
#endif
};

GMT_LOCAL void set_coefficients (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* These are the coefficients in the finite-difference expressions given
	 * by equations (A-4) [SURFACE_UNCONSTRAINED=0] and (A-7) [SURFACE_CONSTRAINED=1] in the reference.
	 * Note that the SURFACE_UNCONSTRAINED coefficients are normalized by a0 (20 for no tension/aspects)
	 * whereas the SURFACE_CONSTRAINED is used for a partial sum hence the normalization is done when the
	 * sum over the Briggs coefficients have been included in iterate. */
	double alpha4, loose, a0;

	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Set finite-difference coefficients [stride = %d]\n", C->current_stride);
	
	loose = 1.0 - C->interior_tension;
	C->alpha2 = C->alpha * C->alpha;
	alpha4 = C->alpha2 * C->alpha2;
	C->eps_p2 = C->alpha2;
	C->eps_m2 = 1.0 / C->alpha2;
	C->one_plus_e2 = 1.0 + C->alpha2;
	C->two_plus_ep2 = 2.0 + 2.0 * C->eps_p2;
	C->two_plus_em2 = 2.0 + 2.0 * C->eps_m2;

	C->e_m2 = 1.0 / C->alpha2;

	a0 = 1.0 / ( (6 * alpha4 * loose + 10 * C->alpha2 * loose + 8 * loose - 2 * C->one_plus_e2) + 4 * C->interior_tension * C->one_plus_e2);
	C->a0_const_1 = 2.0 * loose * (1.0 + alpha4);
	C->a0_const_2 = 2.0 - C->interior_tension + 2 * loose * C->alpha2;

	C->coeff[SURFACE_CONSTRAINED][W2]   = C->coeff[SURFACE_CONSTRAINED][E2]   = -loose;
	C->coeff[SURFACE_CONSTRAINED][N2]   = C->coeff[SURFACE_CONSTRAINED][S2]   = -loose * alpha4;
	C->coeff[SURFACE_UNCONSTRAINED][W2] = C->coeff[SURFACE_UNCONSTRAINED][E2] = -loose * a0;
	C->coeff[SURFACE_UNCONSTRAINED][N2] = C->coeff[SURFACE_UNCONSTRAINED][S2] = -loose * alpha4 * a0;
	C->coeff[SURFACE_CONSTRAINED][W1]   = C->coeff[SURFACE_CONSTRAINED][E1]   = 2 * loose * C->one_plus_e2;
	C->coeff[SURFACE_UNCONSTRAINED][W1] = C->coeff[SURFACE_UNCONSTRAINED][E1] = (2 * C->coeff[SURFACE_CONSTRAINED][W1] + C->interior_tension) * a0;
	C->coeff[SURFACE_CONSTRAINED][N1]   = C->coeff[SURFACE_CONSTRAINED][S1]   = C->coeff[SURFACE_CONSTRAINED][W1] * C->alpha2;
	C->coeff[SURFACE_UNCONSTRAINED][N1] = C->coeff[SURFACE_UNCONSTRAINED][S1] = C->coeff[SURFACE_UNCONSTRAINED][W1] * C->alpha2;
	C->coeff[SURFACE_CONSTRAINED][NW]   = C->coeff[SURFACE_CONSTRAINED][NE]   = C->coeff[SURFACE_CONSTRAINED][SW] =
		C->coeff[SURFACE_CONSTRAINED][SE] = -2 * loose * C->alpha2;
	C->coeff[SURFACE_UNCONSTRAINED][NW] = C->coeff[SURFACE_UNCONSTRAINED][NE] = C->coeff[SURFACE_UNCONSTRAINED][SW] =
		C->coeff[SURFACE_UNCONSTRAINED][SE] = C->coeff[SURFACE_CONSTRAINED][NW] * a0;

	C->alpha2 *= 2;		/* We will need these coefficients times two in the boundary conditions; do the doubling here  */
	C->e_m2   *= 2;
}

GMT_LOCAL void set_offset (struct SURFACE_INFO *C) {
	/* The offset array holds the offset in 1-D index relative
	 * to the current node.  For movement along a row this is
	 * always -2, -1, 0, +1, +2 but along a column we move in
	 * multiples of current_mx, the extended grid row width,
	 * i.e., current_mx = current_nx + 4.
	 */
 	C->offset[N2] = -2 * C->current_mx;	/* N2: 2 rows above */
 	C->offset[NW] = -C->current_mx - 1;	/* NW: 1 row above and one column left */
 	C->offset[N1] = -C->current_mx;		/* N1: 1 row above */
 	C->offset[NE] = -C->current_mx + 1;	/* NE: 1 row above and one column right */
 	C->offset[W2] = -2;			/* W2: 2 columns left */
 	C->offset[W1] = -1;			/* W1 : 1 column left */
 	C->offset[E1] = +1;			/* E1 : 1 column right */
 	C->offset[E2] = +2;			/* E2 : 2 columns right */
 	C->offset[SW] = C->current_mx - 1;	/* SW : 1 row below and one column left */
 	C->offset[S1] = C->current_mx;		/* S1 : 1 row below */
 	C->offset[SE] = C->current_mx + 1;	/* SE : 1 row below and one column right */
 	C->offset[S2] = 2 * C->current_mx;	/* S2 : 2 rows below */
}

GMT_LOCAL void fill_in_forecast (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {

	/* Fills in bilinear estimates into new node locations after grid is divided.
	   These new nodes are marked as unconstrained while the coarser data are considered
	   constraints in the next iteration.  We do this in two steps:
	     a) We sweep through the grid from last to first node and move each node to its
	        new location due to increased grid dimensions.
	     b) Once nodes are in place we sweep through and apply the bilinear interpolation.
	 */

	uint64_t index_00, index_10, index_11, index_01, index_new, current_node, previous_node;
	int previous_row, previous_col, i, j, col, row, expand, first;
	unsigned char *status = C->status;
	double c, sx, sy, sxy, r_prev_size, c_plus_sy_dy, sx_plus_sxy_dy;
	float *u = C->Grid->data;
	
	/* First we expand the active grid to allow for more nodes. We do this by
	 * looping backwards from last node to first so that the locations we copy
	 * the old node values to will always have higher node number than their source.
	 * The previous grid solution has dimensions previous_nx x previous_ny while
	 * the new grid has dimensions current_nx x current_ny.  We thus loop over
	 * the old grid and place these nodes into the new grid.  */
	
	expand = C->previous_stride / C->current_stride;	/* Multiplicity of new nodes in both x and y dimensions */
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Expand grid by factor of %d when going from stride = %d to %d\n", expand, C->previous_stride, C->current_stride);
	
	for (previous_row = C->previous_ny - 1; previous_row >= 0; previous_row--) {	/* Loop backward over the previous grid rows */
		row = previous_row * expand;	/* Corresponding row in the new extended grid */
		for (previous_col = C->previous_nx - 1; previous_col >= 0; previous_col--) {	/* Loop backward over previous grid cols */
			col = previous_col * expand;	/* Corresponding col in the new extended grid */
			current_node  = row_col_to_node (row, col, C->current_mx);			/* Current node index */
			previous_node = row_col_to_node (previous_row, previous_col, C->previous_mx);	/* Previous node index */
			C->Grid->data[current_node] = C->Grid->data[previous_node];			/* Copy the value over */
		}
	}

	/* The grid has now increased in size and the previous values have been copied to their new nodes.
	 * The grid nodes in-between these new "constrained" nodes are partly filled with old values (since
	 * we just copied, not moved, the nodes) or zeros (since we expanded the grid into new unused memory).
	 * This does not matter since we will now fill in those in-between nodes with a bilinear interpolation
	 * based on the coarser (previous) nodes.
	 */
	
	/* Precalculate the fractional increments of rows and cols in-between the old constrained rows and cols.
	 * These are all fractions between 0 and 1.  E.g., if we quadruple the grid dimensions in x and y then 
	 * expand == 4 and we need 4 fractions = {0, 0.25, 0.5, 0.75}. */
	
	r_prev_size = 1.0 / (double)C->previous_stride;
	for (i = 0; i < expand; i++) C->fraction[i] = i * r_prev_size;

	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Fill in expanded grid by bilinear interpolation [stride = %d]\n", C->current_stride);

	/* Loop over 4-point "bin squares" from the first northwest bin to the last southeast bin. The bin vertices are the expanded previous nodes */

	for (previous_row = 1; previous_row < C->previous_ny; previous_row++) {	/* Starts at row 1 since it is the baseline for the bin extending up to row 0 (north) */
		row = previous_row * expand;	/* Corresponding row in the new extended grid */

		for (previous_col = 0; previous_col < (C->previous_nx-1); previous_col++) {	/* Stop 1 column short of east since east is the right boundary of last bin */
			col = previous_col * expand;	/* Corresponding col in the new extended grid */

			/* Get the indices of the bilinear square defined by nodes {00, 10, 11, 01}, with 00 referring to the current (lower left) node */
			index_00 = row_col_to_node (row, col, C->current_mx);	/* Lower left corner of square bin and our origin */
			index_01 = index_00 - expand * C->current_mx;		/* Upper left corner of square bin */
			index_10 = index_00 + expand;				/* Lower right corner of square bin */
			index_11 = index_01 + expand;				/* Upper right corner of square bin */

			/* Get bilinear coefficients for interpolation z = c + sx * delta_x + sy * delta_y + sxy * delta_x * delta_y,
			 * which we will use as z = (c + sy * delta_y) + delta_x * (sx + sxy * delta_y).
			 * Below, delta_x and delta_y are obtained via C->fraction that we pre-calculated above. */
			c = u[index_00];	sx = u[index_10] - c;
			sy = u[index_01] - c;	sxy = u[index_11] - u[index_10] - sy;

			/* Fill in all the denser nodes except the lower-left starting point */

			for (j = 0, first = 1; j < expand; j++) {	/* Set first = 1 so we skip the first column when j = 0 */
				c_plus_sy_dy = c + sy * C->fraction[j];	/* Compute terms that remain constant for this j */
				sx_plus_sxy_dy = sx + sxy * C->fraction[j];
				index_new = index_00 - j * C->current_mx + first;	/* Start node on this intermediate row */
				for (i = first;  i < expand; i++, index_new++) {	/* Sweep across this row and interpolate */
					u[index_new] = (float)(c_plus_sy_dy + C->fraction[i] * sx_plus_sxy_dy);
					status[index_new] = SURFACE_IS_UNCONSTRAINED;	/* These are considered temporary estimates */
				}
				first = 0;	/* Reset to 0 for the remainder of the j loop */
			}
			status[index_00] = SURFACE_IS_CONSTRAINED;	/* The previous node values will be kept fixed in the next iterate call */
		}
	}

	/* The loops above exclude the north and east boundaries.  First do linear interpolation along the east edge */

	for (previous_row = 1; previous_row < C->previous_ny; previous_row++) {	/* So first edge is from row = 1 up to row = 0 on eastern edge */
		row = previous_row * expand;	/* Corresponding row in the new extended grid */
		index_00 = C->node_ne_corner + row * C->current_mx;	/* Lower node */
		index_01 = index_00  - expand * C->current_mx;		/* Upper node */
		sy = u[index_01] - u[index_00];				/* Vertical gradient in u toward ymax (for increasing j) */
		index_new = index_00 - C->current_mx;			/* Since we start at j = 1 we skip up one row */
		for (j = 1; j < expand; j++, index_new -= C->current_mx) {	/* Start at 1 since we skip the constrained index_00 node */
			u[index_new] = u[index_00] + (float)(C->fraction[j] * sy);
			status[index_new] = SURFACE_IS_UNCONSTRAINED;	/* These are considered temporary estimates */
		}
		status[index_00] = SURFACE_IS_CONSTRAINED;	/* The previous node values will be kept fixed in the next iterate call */
	}
	/* Next do linear interpolation along the north edge */
	for (previous_col = 0; previous_col < (C->previous_nx-1); previous_col++) {	/* To ensure last edge ends at col = C->previous_nx-1 */
		col = previous_col * expand;	/* Corresponding col in the new extended grid */
		index_00 = C->node_nw_corner + col;	/* Left node */
		index_10 = index_00 + expand;		/* Right node */
		sx = u[index_10] - u[index_00];		/* Horizontal gradient in u toward xmax (for increasing i) */
		index_new = index_00 + 1;		/* Start at 1 since we skip the constrained index_00 node */
		for (i = 1; i < expand; i++, index_new++) {
			u[index_new] = u[index_00] + (float)(C->fraction[i] * sx);
			status[index_new] = SURFACE_IS_UNCONSTRAINED;	/* These are considered temporary estimates */
		}
		status[index_00] = SURFACE_IS_CONSTRAINED;	/* The previous node values will be kept fixed in the next iterate call */
	}
	/* Finally set the northeast corner to be considered fixed in the next iterate call and our work here is done */
	status[C->node_ne_corner] = SURFACE_IS_CONSTRAINED;
}

GMT_LOCAL int compare_points (const void *point_1v, const void *point_2v, void *arg) {
	/* Routine for qsort_r to sort data structure for fast access to data by node location.
	   Sorts on index first, then on radius to node corresponding to index, so that index
	   goes from low to high, and so does radius.  Note: These are simple Cartesian distance
	 * calculations.  The metadata needed to do the calculations are passed via *arg.
	*/
	uint64_t col, row, index_1, index_2;
	double x0, y0, dist_1, dist_2;
	const struct SURFACE_DATA *point_1 = point_1v, *point_2 = point_2v;
	struct SURFACE_SEARCH *info;
	index_1 = point_1->index;
	index_2 = point_2->index;
	if (index_1 < index_2) return (-1);
	if (index_1 > index_2) return (+1);
	if (index_1 == SURFACE_OUTSIDE) return (0);
	/* Points are in same grid cell.  First check for breakline points to sort those ahead of data points */
	if (point_1->kind == SURFACE_BREAKLINE && point_2->kind == 0) return (-1);
	if (point_2->kind == SURFACE_BREAKLINE && point_1->kind == 0) return (+1);
	/* Now find the one who is nearest to grid point */
	/* Note: index calculations do not include boundary pad */
	info = arg;	/* Get the needed metadata for distance calculations */
	row = index_to_row (index_1, info->current_nx);
	col = index_to_col (index_1, info->current_nx);
	x0 = col_to_x (col, info->wesn[XLO], info->wesn[XHI], info->inc[GMT_X], info->current_nx);
	y0 = row_to_y (row, info->wesn[YLO], info->wesn[YHI], info->inc[GMT_Y], info->current_ny);
	dist_1 = (point_1->x - x0) * (point_1->x - x0) + (point_1->y - y0) * (point_1->y - y0);
	/* Try to speed things up by first checking if point_2 x-distance from x0 alone exceeds point_1's radial distance */
	dist_2 = (point_2->x - x0) * (point_2->x - x0);	/* Just dx^2 */
	if (dist_1 < dist_2) return (-1);	/* Dont need to consider the y-distance */
	/* Did not exceed, so now we must finalize the dist_2 calculation by including the y-separation */
	dist_2 += (point_2->y - y0) * (point_2->y - y0);
	if (dist_1 < dist_2) return (-1);
	if (dist_1 > dist_2) return (+1);
	return (0);
}

GMT_LOCAL void smart_divide (struct SURFACE_INFO *C) {
	/* Divide grid by its next largest prime factor and shift that setting by one */
	C->current_stride /= C->factors[C->n_factors - 1];
	C->n_factors--;
}

GMT_LOCAL void set_index (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* Recomputes data[k].index for the new value of the stride,
	   sorts the data again on index and radii, and throws away
	   data which are now outside the usable limits.
	   Note: These indices exclude the padding. */
	int col, row;
	uint64_t k, k_skipped = 0;
	struct GMT_GRID_HEADER *h = C->Grid->header;

	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Recompute data index for next iteration [stride = %d]\n", C->current_stride);

	for (k = 0; k < C->npoints; k++) {
		col = x_to_col (C->data[k].x, h->wesn[XLO], C->r_inc[GMT_X]);
		row = y_to_row (C->data[k].y, h->wesn[YLO], C->r_inc[GMT_Y], C->current_ny);
		if (col < 0 || col >= C->current_nx || row < 0 || row >= C->current_ny) {
			C->data[k].index = SURFACE_OUTSIDE;
			k_skipped++;
		}
		else
			C->data[k].index = row_col_to_index (row, col, C->current_nx);
	}

	qsort_r (C->data, C->npoints, sizeof (struct SURFACE_DATA), compare_points, &(C->info));

	C->npoints -= k_skipped;
}

GMT_LOCAL void solve_Briggs_coefficients (struct SURFACE_INFO *C, float *b, double xx, double yy, float z) {
	/* Given the normalized offset (xx,yy) from current node (value z) we determine the
	 * Briggs coefficients b_k, k = 1,5  [Equation (A-6) in the reference] 
	 * Here, xx, yy are the fractional distances, accounting for any anisotropy.
	 * Note b[5] initially contains the sum of the 5 Briggs coefficients but
	 * we actually need to divide by it so we do that change here as well.
	 * Finally, b[4] will multiply with the off-node constraint so we do that here.
	 */
	double xx2, yy2, xx_plus_yy, xx_plus_yy_plus_one, inv_xx_plus_yy_plus_one, inv_delta, b_4;
	
	xx_plus_yy = xx + yy;
	xx_plus_yy_plus_one = 1.0 + xx_plus_yy;
	inv_xx_plus_yy_plus_one = 1.0 / xx_plus_yy_plus_one;
	xx2 = xx * xx;	yy2 = yy * yy;
	inv_delta = inv_xx_plus_yy_plus_one / xx_plus_yy;
	b[0] = (float)((xx2 + 2.0 * xx * yy + xx - yy2 - yy) * inv_delta);
	b[1] = (float)(2.0 * (yy - xx + 1.0) * inv_xx_plus_yy_plus_one);
	b[2] = (float)(2.0 * (xx - yy + 1.0) * inv_xx_plus_yy_plus_one);
	b[3] = (float)((-xx2 + 2.0 * xx * yy - xx + yy2 + yy) * inv_delta);
	b_4 = 4.0 * inv_delta;
	/* We also need to normalize by the sum of the b[k] values, so sum them here */
	b[5] = b[0] + b[1] + b[2] + b[3] + b_4;
	/* We need to sum k = 0<5 of u[k]*b[k], where u[k] are the nodes of the points A-D,
	 * but the k = 4 point (E) is our data constraint.  We multiply that in here, once,
	 * add add b[4] to the rest of the sum inside the iteration loop. */
	b[4] = (float)(b_4 * z);
	
	/* b[5] is part of a denominator so we do the division here instead of inside iterate loop */
	b[5] = (float)(1.0 / (C->a0_const_1 + C->a0_const_2 * b[5]));
}

GMT_LOCAL void find_nearest_constraint (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* Determines the nearest data point per bin and sets the
	 * Briggs parameters or, if really close, fixes the node value */
	uint64_t k, last_index, node, briggs_index;
	int row, col;
	double xx, yy, x0, y0, dx, dy;
	float z_at_node, *u = C->Grid->data;
	unsigned char *status = C->status;
	struct GMT_GRID_HEADER *h = C->Grid->header;

	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Determine nearest point and set Briggs coefficients [stride = %d]\n", C->current_stride);
	
	gmt_M_grd_loop (GMT, C->Grid, row, col, node) {	/* Reset status of all interior grid nodes */
		status[node] = SURFACE_IS_UNCONSTRAINED;
	}
	
	last_index = UINTMAX_MAX;	briggs_index = 0U;
	
	for (k = 0; k < C->npoints; k++) {	/* Find constraining value  */
		if (C->data[k].index != last_index) {	/* Moving to the next node to address its nearest data constraint */
			/* Note: Index calculations do not consider the boundary padding */
			row = (int)index_to_row (C->data[k].index, C->current_nx);
			col = (int)index_to_col (C->data[k].index, C->current_nx);
			last_index = C->data[k].index;	/* Now this is the last unique index we worked on */
	 		node = row_col_to_node (row, col, C->current_mx);
			/* Get coordinates of this node */
			x0 = col_to_x (col, h->wesn[XLO], h->wesn[XHI], C->inc[GMT_X], C->current_nx);
			y0 = row_to_y (row, h->wesn[YLO], h->wesn[YHI], C->inc[GMT_Y], C->current_ny);
			/* Get offsets dx,dy of data point location relative to this node (dy is positive up) */
			dx = x_to_fcol (C->data[k].x, x0, C->r_inc[GMT_X]);
			dy = y_to_frow (C->data[k].y, y0, C->r_inc[GMT_Y]);
	
			/* "Really close" will mean within 5% of the current grid spacing from the center node */

	 		if (fabs (dx) < SURFACE_CLOSENESS_FACTOR && fabs (dy) < SURFACE_CLOSENESS_FACTOR) {	/* Close enough to assign fixed value to node */
	 			status[node] = SURFACE_IS_CONSTRAINED;
	 			/* Since point is basically moved from (dx, dy) to (0,0) we must adjust for
	 			 * the small change in the planar trend between the two locations, and then
	 			 * possibly clip the value if constraining surfaces were given.  Note that
	 			 * dx, dy is in -1/1 range normalized by (current_x|y_inc) so to recover the
	 			 * corresponding dx,dy in units of final grid fractions we must scale both
				 * dx and dy by current_stride; this is equivalant of scaling the trend.
				 * This trend then is normalized by dividing by the z rms.*/
	 			
	 			z_at_node = C->data[k].z + (float) (C->r_z_rms * C->current_stride * evaluate_trend (C, dx, dy));
	 			if (C->constrained) {
					if (C->set_limit[LO] && !gmt_M_is_fnan (C->Bound[LO]->data[node]) && z_at_node < C->Bound[LO]->data[node])
						z_at_node = C->Bound[LO]->data[node];
					else if (C->set_limit[HI] && !gmt_M_is_fnan (C->Bound[HI]->data[node]) && z_at_node > C->Bound[HI]->data[node])
						z_at_node = C->Bound[HI]->data[node];
	 			}
	 			u[node] = z_at_node;
	 		}
	 		else {	/* We have a nearby data point in one of the quadrants */
	 			if (dy >= 0.0) {	/* Upper two quadrants */
		 			if (dx >= 0.0)
	 					status[node] = SURFACE_DATA_IS_IN_QUAD1;
	 				else
	 					status[node] = SURFACE_DATA_IS_IN_QUAD2;
					xx = fabs (dx);	yy = fabs (dy);
	 			}
	 			else {
		 			if (dx >= 0.0)
	 					status[node] = SURFACE_DATA_IS_IN_QUAD4;
	 				else
	 					status[node] = SURFACE_DATA_IS_IN_QUAD3;
	 				yy = fabs (dx);	xx = fabs (dy);	/* Must swap dx,dy for these quadrants */
				}
				/* Evaluate the Briggs coefficients */
				solve_Briggs_coefficients (C, C->Briggs[briggs_index].b, xx, yy, C->data[k].z);
#ifdef PARALLEL_MODE
				C->briggs_index[node] = briggs_index;	/* Since with OpenMP there is no longer a sequential access of Briggs coefficients in iterate */
#endif
	 			briggs_index++;
	 		}
	 	}
	 }
}

GMT_LOCAL void find_mean_constraint (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* Determines all data point per bin, obtains mean value if more than one and sets the
	 * Briggs parameters or, if really close, fixes the node value */
	uint64_t k, n, last_index, node, briggs_index;
	int row, col;
	double xx, yy, x0, y0, dx, dy, mean_x, mean_y;
	float mean_z, *u = C->Grid->data;
	unsigned char *status = C->status;
	struct GMT_GRID_HEADER *h = C->Grid->header;

	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Determine mean data constraints and set Briggs coefficients [stride = %d]\n", C->current_stride);
	
	gmt_M_grd_loop (GMT, C->Grid, row, col, node) {	/* Reset status of all interior grid nodes */
		status[node] = SURFACE_IS_UNCONSTRAINED;
	}
	
	briggs_index = 0U;	k = 0;
	while (k < C->npoints) {	/* While there are more data constraints...  */
		last_index = C->data[k].index;	/* This is the current index we are working on */
		mean_x = C->data[k].x;	mean_y = C->data[k].y;	mean_z = (float)C->data[k].z;	/* The first (and maybe only) data constraint */
		n = 1;	k++;	/* Found one point, go the next */
		while (k < C->npoints && C->data[k].index == last_index) {	/* Keep summing point values as long as we have the same index */
			mean_x += C->data[k].x;	mean_y += C->data[k].y;	mean_z += (float)C->data[k].z;
			k++;	n++;	/* Found another point, go the next */
		}
		if (n > 1) {	/* More than one constraint found, estimate mean location and value */
			mean_x /= n;	mean_y /= n;	mean_z /= n;
		}
		/* Note: Index calculations do not consider the boundary padding */
		row = (int)index_to_row (last_index, C->current_nx);
		col = (int)index_to_col (last_index, C->current_nx);
	 	node = row_col_to_node (row, col, C->current_mx);
		/* Get coordinates of this node */
		x0 = col_to_x (col, h->wesn[XLO], h->wesn[XHI], C->inc[GMT_X], C->current_nx);
		y0 = row_to_y (row, h->wesn[YLO], h->wesn[YHI], C->inc[GMT_Y], C->current_ny);
		/* Get offsets dx,dy of mean data point location relative to this node (dy is positive up) */
		dx = x_to_fcol (mean_x, x0, C->r_inc[GMT_X]);
		dy = y_to_frow (mean_y, y0, C->r_inc[GMT_Y]);
	
 		/* "Really close" will mean within 5% of the current grid spacing from the center node */

		if (fabs (dx) < SURFACE_CLOSENESS_FACTOR && fabs (dy) < SURFACE_CLOSENESS_FACTOR) {	/* Close enough to assign fixed value to node */
 			status[node] = SURFACE_IS_CONSTRAINED;
 			/* Since point is basically moved from (dx, dy) to (0,0) we must adjust for
 			 * the small change in the planar trend between the two locations, and then
 			 * possibly clip the value if constraining surfaces were given.  Note that
 			 * dx, dy is in -1/1 range normalized by (current_x|y_inc) so to recover the
 			 * corresponding dx,dy in units of final grid fractions we must scale both
			 * dx and dy by current_stride; this is equivalant of scaling the trend.
			 * This trend then is normalized by dividing by the z rms.*/
			
 			mean_z += (float) (C->r_z_rms * C->current_stride * evaluate_trend (C, dx, dy));
 			if (C->constrained) {
				if (C->set_limit[LO] && !gmt_M_is_fnan (C->Bound[LO]->data[node]) && mean_z < C->Bound[LO]->data[node])
					mean_z = C->Bound[LO]->data[node];
				else if (C->set_limit[HI] && !gmt_M_is_fnan (C->Bound[HI]->data[node]) && mean_z > C->Bound[HI]->data[node])
					mean_z = C->Bound[HI]->data[node];
 			}
 			u[node] = mean_z;	/* Set the fixed node value */
 		}
 		else {	/* We have a nearby data point in one of the quadrants */
 			if (dy >= 0.0) {	/* Upper two quadrants */
	 			if (dx >= 0.0)
 					status[node] = SURFACE_DATA_IS_IN_QUAD1;
 				else
 					status[node] = SURFACE_DATA_IS_IN_QUAD2;
				xx = fabs (dx);	yy = fabs (dy);
 			}
 			else {
	 			if (dx >= 0.0)
 					status[node] = SURFACE_DATA_IS_IN_QUAD4;
 				else
 					status[node] = SURFACE_DATA_IS_IN_QUAD3;
 				yy = fabs (dx);	xx = fabs (dy);	/* Must swap dx,dy for these quadrants */
			}
			/* Evaluate the Briggs coefficients */
			solve_Briggs_coefficients (C, C->Briggs[briggs_index].b, xx, yy, C->data[k].z);
#ifdef PARALLEL_MODE
			C->briggs_index[node] = briggs_index;	/* Since with OpenMP there is no longer a sequential access of Briggs coefficients in iterate */
#endif
 			briggs_index++;
 		}
	 }
}

GMT_LOCAL void set_grid_parameters (struct SURFACE_INFO *C) {
	/* Set the previous settings to the current settings */
	C->previous_nx = C->current_nx;
	C->previous_mx = C->current_mx;
	C->previous_ny = C->current_ny;
	/* Update the current parameters given the new C->current_stride setting */
	C->info.current_nx = C->current_nx = (C->n_columns - 1) / C->current_stride + 1;
	C->info.current_ny = C->current_ny = (C->n_rows - 1) / C->current_stride + 1;
	C->current_mx = C->current_nx + 4;
	C->current_mxmy = C->current_mx * (C->current_ny + 4);
	C->info.inc[GMT_X] = C->inc[GMT_X] = C->current_stride * C->Grid->header->inc[GMT_X];
	C->info.inc[GMT_Y] = C->inc[GMT_Y] = C->current_stride * C->Grid->header->inc[GMT_Y];
	C->r_inc[GMT_X] = 1.0 / C->inc[GMT_X];
	C->r_inc[GMT_Y] = 1.0 / C->inc[GMT_Y];
	/* Update the grid node indices of the 4 corners */
	C->node_nw_corner = 2 * C->current_mx + 2;
	C->node_sw_corner = C->node_nw_corner + (C->current_ny - 1) * C->current_mx;
	C->node_se_corner = C->node_sw_corner + C->current_nx - 1;
	C->node_ne_corner = C->node_nw_corner + C->current_nx - 1;
}

GMT_LOCAL void initialize_grid (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* For the initial gridsize, compute weighted averages of data inside the search radius
	 * and assign the values to u[col,row], where col,row are multiples of gridsize.
	 * Weights are Gaussian, i.e., this is a MA Gaussian filter operation.
	 */
	uint64_t index_1, index_2, k, k_index, node;
	int del_col, del_row, col, row, col_min, col_max, row_min, row_max, ki, kj;
	double r, rfact, sum_w, sum_zw, weight, x0, y0;
	float *u = C->Grid->data;
	struct GMT_GRID_HEADER *h = C->Grid->header;

	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Initialize grid using moving average scheme [stride = %d]\n", C->current_stride);
	
	del_col = irint (ceil (C->radius / C->inc[GMT_X]));
	del_row = irint (ceil (C->radius / C->inc[GMT_Y]));
	rfact = -4.5 / (C->radius*C->radius);
 	for (row = 0; row < C->current_ny; row++) {
		y0 = row_to_y (row, h->wesn[YLO], h->wesn[YHI], C->inc[GMT_Y], C->current_ny);
		for (col = 0; col < C->current_nx; col++) {
			/* For this node on the grid, find all data points within the radius */
			x0 = col_to_x (col, h->wesn[XLO], h->wesn[XHI], C->inc[GMT_X], C->current_nx);
	 		col_min = col - del_col;
	 		if (col_min < 0) col_min = 0;
	 		col_max = col + del_col;
	 		if (col_max >= C->current_nx) col_max = C->current_nx - 1;
	 		row_min = row - del_row;
	 		if (row_min < 0) row_min = 0;
	 		row_max = row + del_row;
	 		if (row_max >= C->current_ny) row_max = C->current_ny - 1;
			index_1 = row_col_to_index (row_min, col_min, C->current_nx);
			index_2 = row_col_to_index (row_max, col_max+1, C->current_nx);
	 		sum_w = sum_zw = 0.0;
	 		k = 0;
	 		while (k < C->npoints && C->data[k].index < index_1) k++;
			/* This double loop visits all nodes within the rectangle of dimensions (2*del_col by 2*del_row) centered on x0,y0 */
 			for (kj = row_min; k < C->npoints && kj <= row_max && C->data[k].index < index_2; kj++) {
	 			for (ki = col_min; k < C->npoints && ki <= col_max && C->data[k].index < index_2; ki++) {
					k_index = row_col_to_index (kj, ki, C->current_nx);
	 				while (k < C->npoints && C->data[k].index < k_index) k++;
	 				while (k < C->npoints && C->data[k].index == k_index) {
	 					r = (C->data[k].x-x0)*(C->data[k].x-x0) + (C->data[k].y-y0)*(C->data[k].y-y0);
						if (r > C->radius) continue;	/* Outside the circle */
	 					weight = exp (rfact * r);
	 					sum_w  += weight;
	 					sum_zw += weight * C->data[k].z;
	 					k++;
	 				}
	 			}
	 		}
			node = row_col_to_node (row, col, C->current_mx);
	 		if (sum_w == 0.0) {
	 			sprintf (C->format, "Warning: no data inside search radius at: %s %s [node set to data mean]\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	 			GMT_Report (GMT->parent, GMT_MSG_NORMAL, C->format, x0, y0);
	 			u[node] = (float)C->z_mean;
	 		}
	 		else
	 			u[node] = (float)(sum_zw/sum_w);
		}
	}
}

GMT_LOCAL int read_data_surface (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, struct GMT_OPTION *options) {
	/* Procdss input data into data structure */
	int col, row, error;
	uint64_t k = 0, kmax = 0, kmin = 0, n_dup = 0;
	double *in, half_dx, zmin = DBL_MAX, zmax = -DBL_MAX, wesn_lim[4];
	struct GMT_GRID_HEADER *h = C->Grid->header;

	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Processing input table data\n");
	C->data = gmt_M_memory (GMT, NULL, C->n_alloc, struct SURFACE_DATA);

	/* Read in xyz data and computes index no and store it in a structure */
	
	if ((error = gmt_set_cols (GMT, GMT_IN, 3)) != GMT_NOERROR)
		return (error);
	if (GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR)	/* Establishes data input */
		return (GMT->parent->error);

	C->z_mean = 0.0;
	/* Initially allow points to be within 1 grid spacing of the grid */
	wesn_lim[XLO] = h->wesn[XLO] - C->inc[GMT_X];	wesn_lim[XHI] = h->wesn[XHI] + C->inc[GMT_X];
	wesn_lim[YLO] = h->wesn[YLO] - C->inc[GMT_Y];	wesn_lim[YHI] = h->wesn[YHI] + C->inc[GMT_Y];
	half_dx = 0.5 * C->inc[GMT_X];

	if (GMT_Begin_IO (GMT->parent, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR)	/* Enables data input and sets access mode */
		return (GMT->parent->error);

	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (GMT->parent, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
				return (GMT_RUNTIME_ERROR);
			if (gmt_M_rec_is_any_header (GMT)) 	/* Skip all headers */
				continue;
			if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */
	
		if (gmt_M_is_dnan (in[GMT_Z])) continue;	/* Cannot use NaN values */
		if (gmt_M_y_is_outside (GMT, in[GMT_Y], wesn_lim[YLO], wesn_lim[YHI])) continue; /* Outside y-range (or latitude) */
		if (gmt_x_is_outside (GMT, &in[GMT_X], wesn_lim[XLO], wesn_lim[XHI]))  continue; /* Outside x-range (or longitude) */
		row = y_to_row (in[GMT_Y], h->wesn[YLO], C->r_inc[GMT_Y], C->current_ny);
		if (row < 0 || row >= C->current_ny) continue;
		if (C->periodic && ((h->wesn[XHI]-in[GMT_X]) < half_dx)) {	/* Push all values to the western nodes */
			in[GMT_X] -= 360.0;	/* Make this point constrain the western node value and then duplicate to east later */
			col = 0;
		}
		else	/* Regular point not at the periodic boundary */
			col = x_to_col (in[GMT_X], h->wesn[XLO], C->r_inc[GMT_X]);
		if (col < 0 || col >= C->current_nx) continue;

		C->data[k].index = row_col_to_index (row, col, C->current_nx);
#ifdef DEBUG
		C->data[k].number = k + 1;	/* So we can check which points are skipped later */
#endif
		C->data[k].x = (float)in[GMT_X];	C->data[k].y = (float)in[GMT_Y];	C->data[k].z = (float)in[GMT_Z];
		/* Determine the mean, min and max z-values */
		if (zmin > in[GMT_Z]) zmin = in[GMT_Z], kmin = k;
		if (zmax < in[GMT_Z]) zmax = in[GMT_Z], kmax = k;
		C->z_mean += in[GMT_Z];
		if (++k == C->n_alloc) {
			C->n_alloc <<= 1;
			C->data = gmt_M_memory (GMT, C->data, C->n_alloc, struct SURFACE_DATA);
		}
		if (C->periodic && col == 0) {	/* Now we must replicate information from the western to the eastern boundary */
			col = C->current_nx - 1;
			C->data[k].index = row_col_to_index (row, col, C->current_nx);
#ifdef DEBUG
			C->data[k].number = k + 1;
#endif
			C->data[k].x = (float)(in[GMT_X] + 360.0);
			C->data[k].y = (float)in[GMT_Y];
			C->data[k].z = (float)in[GMT_Z];
			C->z_mean += in[GMT_Z];
			if (++k == C->n_alloc) {
				C->n_alloc <<= 1;
				C->data = gmt_M_memory (GMT, C->data, C->n_alloc, struct SURFACE_DATA);
			}
			n_dup++;
		}
	} while (true);

	
	if (GMT_End_IO (GMT->parent, GMT_IN, 0) != GMT_NOERROR)	/* Disables further data input */
		return (GMT->parent->error);

	C->npoints = k;	/* Number of data points that passed being "inside" the grid region */

	if (C->npoints == 0) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "No datapoints inside region, aborting\n");
		return (GMT_RUNTIME_ERROR);
	}

	C->z_mean /= C->npoints;	/* Estimate mean data value */
	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {
		char msg[GMT_LEN256] = {""};
		sprintf (C->format, "%s %s %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		sprintf (msg, C->format, (double)C->data[kmin].x, (double)C->data[kmin].y, (double)C->data[kmin].z);
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Minimum value of your dataset x,y,z at: %s", msg);
		sprintf (msg, C->format, (double)C->data[kmax].x, (double)C->data[kmax].y, (double)C->data[kmax].z);
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Maximum value of your dataset x,y,z at: %s", msg);
		if (C->periodic && n_dup) GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Number of input values shared between repeating west and east column nodes: %" PRIu64 "\n", n_dup);
	}
	C->data = gmt_M_memory (GMT, C->data, C->npoints, struct SURFACE_DATA);

	if (C->set_limit[LO] == DATA)	/* Wanted to set lower limit based on minimum observed z value */
		C->limit[LO] = C->data[kmin].z;
	else if (C->set_limit[LO] == VALUE && C->limit[LO] > C->data[kmin].z)
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Your lower value is > than min data value.\n");
	if (C->set_limit[HI] == DATA)	/* Wanted to set upper limit based on maximum observed z value */
		C->limit[HI] = C->data[kmax].z;
	else if (C->set_limit[HI] == VALUE && C->limit[HI] < C->data[kmax].z)
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Your upper value is < than max data value.\n");
	return (0);
}

GMT_LOCAL int load_constraints (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, int transform) {
	/* Deal with the constants or grids supplied via -L.  Note: Because we remove a
	 * best-fitting plane from the data, even a simple constant constraint will become
	 * a plane and thus must be represented on a grid. */
	unsigned int end, col, row;
	uint64_t node;
	char *limit[2] = {"Lower", "Upper"};
	double y_up;
	struct GMTAPI_CTRL *API = GMT->parent;

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Load any data constraint limit grids\n");

	/* Load lower/upper limits, verify range, deplane, and rescale */

	for (end = LO; end <= HI; end++) {
		if (C->set_limit[end] == NONE) continue;	/* Nothing desired */
		if (C->set_limit[end] < SURFACE) {	/* Got a constant level for this end */
			if ((C->Bound[end] = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, C->Grid)) == NULL) return (API->error);
			for (node = 0; node < C->mxmy; node++) C->Bound[end]->data[node] = (float)C->limit[end];
		}
		else {	/* Got a grid with a surface */
			if ((C->Bound[end] = GMT_Read_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, C->limit_file[end], NULL)) == NULL) return (API->error);	/* Get header only */
			if (C->Bound[end]->header->n_columns != C->Grid->header->n_columns || C->Bound[end]->header->n_rows != C->Grid->header->n_rows) {
				GMT_Report (API, GMT_MSG_NORMAL, "%s limit file not of proper dimensions!\n", limit[end]);
				return (GMT_RUNTIME_ERROR);
			}
			if (GMT_Read_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, C->limit_file[end], C->Bound[end]) == NULL) return (API->error);
		}
		if (transform) {	/* Remove best-fitting plane and normalize the bounding values */
			for (row = 0; row < C->Grid->header->n_rows; row++) {
				y_up = (double)(C->Grid->header->n_rows - row - 1);	/* Require y_up = 0 at south and positive toward north */
				node = row_col_to_node (row, 0, C->current_mx);
				for (col = 0; col < C->Grid->header->n_columns; col++, node++) {
					if (gmt_M_is_fnan (C->Bound[end]->data[node])) continue;
					C->Bound[end]->data[node] -= (float)evaluate_plane (C, col, y_up);	/* Remove plane */
					C->Bound[end]->data[node] *= (float)C->r_z_rms;				/* Normalize residuals */
				}
			}
		}
		C->constrained = true;	/* At least one of the limits will be constrained */
	}

	return (0);
}

GMT_LOCAL int write_surface (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, char *grdfile) {
	/* Write output grid to file */
	uint64_t node;
	int row, col, err, end;
	char *limit[2] = {"lower", "upper"};
	float *u = C->Grid->data;

	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Prepare final output grid [stride = %d]\n", C->current_stride);

	strcpy (C->Grid->header->title, "Data gridded with continuous surface splines in tension");

	if (GMT->common.r.active) {	/* Pixel registration request. Reset region to the original extent */
		gmt_M_memcpy (C->Grid->header->wesn, C->wesn_orig, 4, double);
		C->Grid->header->registration = GMT->common.r.registration;
		/* Must reduce both n_columns,n_rows by 1 and make the eastmost column and northernmost row part of the grid pad */
		C->Grid->header->n_columns--;	C->n_columns--;
		C->Grid->header->n_rows--;	C->n_rows--;
		C->Grid->header->pad[XHI]++;	/* Presumably increase pad from 2 to 3 */
		C->Grid->header->pad[YHI]++;	/* Presumably increase pad from 2 to 3 */
		gmt_set_grddim (GMT, C->Grid->header);	/* Reset all integer dimensions and xy_off */
	}
	if (C->constrained) {	/* Must check that we don't exceed any imposed limits.  */
		/* Reload the constraints, but this time do not transform the data */
		if ((err = load_constraints (GMT, C, false)) != 0) return (err);
		
		gmt_M_grd_loop (GMT, C->Grid, row, col, node) {	/* Make sure we clip to the specified bounds */
			if (C->set_limit[LO] && !gmt_M_is_fnan (C->Bound[LO]->data[node]) && u[node] < C->Bound[LO]->data[node]) u[node] = C->Bound[LO]->data[node];
			if (C->set_limit[HI] && !gmt_M_is_fnan (C->Bound[HI]->data[node]) && u[node] > C->Bound[HI]->data[node]) u[node] = C->Bound[HI]->data[node];
		}
		/* Free any bounding surfaces */
		for (end = LO; end <= HI; end++) {
			if ((C->set_limit[end] > NONE && C->set_limit[end] < SURFACE) && GMT_Destroy_Data (GMT->parent, &C->Bound[end]) != GMT_NOERROR) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Failed to free %s boundary\n", limit[end]);
			}
		}
	}
	if (C->periodic) {	/* Ensure exact periodicity at E-W boundaries */
		for (row = 0; row < C->current_ny; row++) {
			node = row_col_to_node (row, 0, C->current_mx);
			u[node] = u[node+C->current_nx-1] = (float)(0.5 * (u[node] + u[node+C->current_nx-1]));	/* Set these to the same as their average */
		}
	}
	/* Time to write our final grid */
	if (GMT_Write_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, grdfile, C->Grid) != GMT_NOERROR)
		return (GMT->parent->error);

	return (0);
}

GMT_LOCAL void set_BCs (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, float *u) {
	/* Fill in auxiliary boundary rows and columns; see equations (A-8,9,10) in the reference */
	uint64_t n, n_s, n_n, n_w, n_e;	/* Node variables */
	int col, row, *d_n = C->offset;	/* Relative changes in node index from present node n */
	double x_0_const = 4.0 * (1.0 - C->boundary_tension) / (2.0 - C->boundary_tension);
	double x_1_const = (3 * C->boundary_tension - 2.0) / (2.0 - C->boundary_tension);
	double y_denom = 2 * C->alpha * (1.0 - C->boundary_tension) + C->boundary_tension;
	double y_0_const = 4 * C->alpha * (1.0 - C->boundary_tension) / y_denom;
	double y_1_const = (C->boundary_tension - 2 * C->alpha * (1.0 - C->boundary_tension) ) / y_denom;
	
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Apply all boundary conditions [stride = %d]\n", C->current_stride);

	/* First set (1-T)d2[]/dn2 + Td[]/dn = 0 along edges */

	for (col = 0, n_s = C->node_sw_corner, n_n = C->node_nw_corner; col < C->current_nx; col++, n_s++, n_n++) {	/* set BC1 along south and north side */
		u[n_s+d_n[S1]] = (float)(y_0_const * u[n_s] + y_1_const * u[n_s+d_n[N1]]);	/* South: u_{0-1} = 2 * u_{00} - u_{01} */
		u[n_n+d_n[N1]] = (float)(y_0_const * u[n_n] + y_1_const * u[n_n+d_n[S1]]);	/* North: u_{01}  = 2 * u_{00} - u_{0-1} */
	}
	if (C->periodic) {	/* Set periodic boundary conditions in longitude at west and east boundaries */
		for (row = 0, n_w = C->node_nw_corner, n_e = C->node_ne_corner; row < C->current_ny; row++, n_w += C->current_mx, n_e += C->current_mx) {
			u[n_w+d_n[W1]] = u[n_e+d_n[W1]];	/* West side */
			u[n_e+d_n[E1]] = u[n_w+d_n[E1]];	/* East side */
			u[n_e] = u[n_w] = 0.5f * (u[n_e] + u[n_w]);	/* Set to average of east and west */
		}
	}
	else {	/* Regular natural BC */
		for (row = 0, n_w = C->node_nw_corner, n_e = C->node_ne_corner; row < C->current_ny; row++, n_w += C->current_mx, n_e += C->current_mx) {
			/* West: u_{-10} = 2 * u_{00} - u_{10}  */
			u[n_w+d_n[W1]] = (float)(x_1_const * u[n_w+d_n[E1]] + x_0_const * u[n_w]);
			/* East: u_{10} = 2 * u_{00} - u_{-10}  */
			u[n_e+d_n[E1]] = (float)(x_1_const * u[n_e+d_n[W1]] + x_0_const * u[n_e]);
		}
	}

	/* Now set d2[]/dxdy = 0 at each of the 4 corners */

	n = C->node_sw_corner;	/* Just use shorthand in each expression */
	u[n+d_n[SW]]  = u[n+d_n[SE]] + u[n+d_n[NW]] - u[n+d_n[NE]];
	n = C->node_nw_corner;
	u[n+d_n[NW]]  = u[n+d_n[NE]] + u[n+d_n[SW]] - u[n+d_n[SE]];
	n = C->node_se_corner;
	u[n+d_n[SE]]  = u[n+d_n[SW]] + u[n+d_n[NE]] - u[n+d_n[NW]];
	n = C->node_ne_corner;
	u[n+d_n[NE]]  = u[n+d_n[NW]] + u[n+d_n[SE]] - u[n+d_n[SW]];

	/* Now set dC/dn = 0 at each edge */

	for (col = 0, n_s = C->node_sw_corner, n_n = C->node_nw_corner; col < C->current_nx; col++, n_s++, n_n++) {	/* set BC2 along south and north side */
		/* South side */
		u[n_s+d_n[S2]] = (float)(u[n_s+d_n[N2]] + C->eps_m2*(u[n_s+d_n[NW]] + u[n_s+d_n[NE]]
			- u[n_s+d_n[SW]] - u[n_s+d_n[SE]]) + C->two_plus_em2 * (u[n_s+d_n[S1]] - u[n_s+d_n[N1]]));
		/* North side */
		u[n_n+d_n[N2]] = (float)(u[n_n+d_n[S2]] + C->eps_m2 * (u[n_n+d_n[SW]] + u[n_n+d_n[SE]]
			- u[n_n+d_n[NW]] - u[n_n+d_n[NE]]) + C->two_plus_em2 * (u[n_n+d_n[N1]] - u[n_n+d_n[S1]]));
	}

	for (row = 0, n_w = C->node_nw_corner, n_e = C->node_ne_corner; row < C->current_ny; row++, n_w += C->current_mx, n_e += C->current_mx) {	/* set BC2 along west and east side */
		if (C->periodic) {	/* Set periodic boundary conditions in longitude */
			u[n_w+d_n[W2]] = u[n_e+d_n[W2]];	/* West side */
			u[n_e+d_n[E2]] = u[n_w+d_n[E2]];	/* East side */
		}
		else {	/* Natural BCs */
			/* West side */
			u[n_w+d_n[W2]] = (float)(u[n_w+d_n[E2]] + C->eps_p2 * (u[n_w+d_n[NE]] + u[n_w+d_n[SE]]
				- u[n_w+d_n[NW]] - u[n_w+d_n[SW]]) + C->two_plus_ep2 * (u[n_w+d_n[W1]] - u[n_w+d_n[E1]]));
			/* East side */
			u[n_e+d_n[E2]] = (float)(u[n_e+d_n[W2]] + C->eps_p2 * (u[n_e+d_n[NW]] + u[n_e+d_n[SW]]
				- u[n_e+d_n[NE]] - u[n_e+d_n[SE]]) + C->two_plus_ep2 * (u[n_e+d_n[E1]] - u[n_e+d_n[W1]]));
		}
	}
}

#if _OPENMP
#define floatp_swap(px, py) {float *float_tmp; float_tmp = px, px = py, py = float_tmp;}
#elif DEBUG_SURF
#define floatp_swap(px, py) {float *float_tmp; float_tmp = px, px = py, py = float_tmp;}
#endif

GMT_LOCAL uint64_t iterate (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, int mode) {
	/* Main finite difference solver */
	uint64_t node, briggs_index, iteration_count = 0;
	unsigned int set, quadrant, current_max_iterations = C->max_iterations * C->current_stride;
	int col, row, k, *d_node = C->offset;	/* Relative changes in node index from present node */
	unsigned char *status = C->status;	/* Quadrant or status information for each node */
	char *mode_name[2] = {"node", "data"};
	bool finished;
	double current_limit = C->converge_limit / C->current_stride;
	double u_change, max_u_change, max_z_change, sum_bk_uk, u_00;
	float *b = NULL;
#ifdef _OPENMP	/* We will alternate between treating the two grids as old (reading from) and new (writing to) */
	bool dump;
	float *u_new = C->Grid->data, *u_old = C->alternate_grid;
#else		/* Here they are the same single grid */
	//bool dump;
	float *u_new = C->Grid->data, *u_old = C->Grid->data;
#endif
#ifdef DEBUG_SURF
	struct GMT_GRID *G = NULL;
	double y_up;
	char file[GMT_LEN256] = {""};

	if (debug) {
		G = GMT_Create_Data (GMT->parent, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, C->Grid->header->wesn, C->inc, GMT_GRID_NODE_REG, GMT_NOTSET, NULL);
		G->header->n_columns = C->current_nx;	G->header->n_rows = C->current_ny;
		if (debug == 2)	/* Use two grids and copy new to old before each iteration */
			u_old = C->alternate_grid;	/* u_old is always the temp grid which we copy to before iteration */
		if (debug == 3)	/* Use two grids and alternate pointer */
			u_old = C->alternate_grid;	/* So upon first swap u_new will be the tmp grid and u_old will be G->data */
	}
#endif

	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Starting iterations, mode = %s Max iterations = %d [stride = %d]\n", mode_name[mode], current_max_iterations, C->current_stride);

	sprintf (C->format, "%%4ld\t%%c\t%%8" PRIu64 "\t%s\t%s\t%%10" PRIu64 "\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	if (C->logging) fprintf (C->fp_log, "%c Grid size = %d Mode = %c Convergence limit = %g -Z%d\n",
		GMT->current.setting.io_seg_marker[GMT_OUT], C->current_stride, C->mode_type[mode], current_limit, C->current_stride);

	/* We need to do an even number of iterations so that the final result for this iteration resides in C->Grid->data */
	do {
#ifdef _OPENMP
		floatp_swap (u_old, u_new);	/* Swap the two grid pointers. First time u_old will point to previous (existing) solution  */
#endif
#ifdef DEBUG_SURF
#ifndef _OPENMP	/* debug modes 2 and 3 cannot be used under OpenMP */
		if (debug == 2) gmt_M_memcpy (u_old, u_new, C->current_mxmy, float);	/* Copy previous solution to u_old */
		if (debug == 3) floatp_swap (u_old, u_new);	/* Swap the two grid pointers. First time u_old will point to previous (existing) solution  */
#endif
		if (save_intermediate) {	/* Dump this iteration grid */
			sprintf (file, "%s_%7.7d_%s_%d.nc", debug_prefix, (int)iteration_count, mode_name[mode], C->current_stride);
			gmt_M_memcpy (tmp_grid, u_old, C->current_mxmy, float);
			G->data = tmp_grid;	/* This is the latest solution */
			if (!nondimensional) {	/* Must scale back up and add plane */
				for (row = 0; row < C->current_ny; row++) {
					y_up = (double)(C->Grid->header->n_rows - row - 1);	/* # of rows from south (where y_up = 0) to this node */
					node = row_col_to_node (row, 0, C->current_mx);	/* Node index at left end of interior row */
					for (col = 0; col < C->current_nx; col++, node++)	/* March across this row */
					 	G->data[node] = (float)((G->data[node] * C->z_rms) + (evaluate_plane (C, col*C->current_stride, y_up*C->current_stride)));
				}
			}
			if (GMT_Write_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, file, G) != GMT_NOERROR)
				fprintf (stderr, "Writing %s failed\n", file);
			G->data = NULL;	/* Since it is not ours */
		}
#endif

		set_BCs (GMT, C, u_old);	/* Set the boundary rows and columns */
		
#ifndef PARALLEL_MODE
		briggs_index = 0;	/* Reset the Briggs constraint table index  */
#endif
#ifndef _OPENMP
		max_u_change = -1.0;	/* Ensure max_u_change is < 0 for starters */
#endif

		/* Now loop over all interior data nodes */
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Iteration %d\n", iteration_count);

#ifdef _OPENMP
#pragma omp parallel for private(row,node,col,u_00,set,k,b,quadrant,sum_bk_uk,briggs_index,u_change,dump) shared(C,status,u_new,u_old,d_node) reduction(max:max_u_change)
#endif
		for (row = 0; row < C->current_ny; row++) {	/* Loop over rows */
			node = C->node_nw_corner + row * C->current_mx;	/* Node at left side of this row */
			for (col = 0; col < C->current_nx; col++, node++) {	/* Loop over all columns */
				if (status[node] == SURFACE_IS_CONSTRAINED) {	/* Data constraint fell exactly on the node, keep it as is */
#ifdef PARALLEL_MODE
					u_new[node] = u_old[node];		/* Must copy over to other grid when OpenMP is enabled */
#endif
					continue;
				}
				
				/* Here we must estimate a solution via equations (A-4) [SURFACE_UNCONSTRAINED] or (A-7) [SURFACE_CONSTRAINED] */
				//dump = (row == 19 && col == 32 && (iteration_count > 0 && iteration_count < 20));
				u_00 = 0.0;	/* Start with zero, build updated solution for central node */
				set = (status[node] == SURFACE_IS_UNCONSTRAINED) ? SURFACE_UNCONSTRAINED : SURFACE_CONSTRAINED;	/* Index to C->coeff set to use */
				for (k = 0; k < 12; k++) {	/* This is either equation (A-4) or the corresponding part of (A-7), depending on the value of set */
					u_00 += (u_old[node+d_node[k]] * C->coeff[set][k]);
					//if (dump) fprintf (stderr, "I%d k = %d u_ij = %g C = %g sum = %g\n", (int)iteration_count, k, u_old[node+d_node[k]], C->coeff[set][k], u_00);
				}
				if (set == SURFACE_CONSTRAINED) {	/* Solution is (A-7) and modifications depend on which quadrant the point lies in */
#ifdef PARALLEL_MODE
					briggs_index = C->briggs_index[node];	/* Get Briggs array index from lookup table */
#endif
					b = C->Briggs[briggs_index].b;		/* Shorthand to this node's Briggs b-array */
					quadrant = status[node];		/* Which quadrant did the point fall in? */
					for (k = 0, sum_bk_uk = 0.0; k < 4; k++) {	/* Sum over b[k]*u[k] for nodes A-D in Fig A-1 */
						sum_bk_uk += b[k] * u_old[node+d_node[C->p[quadrant][k]]];
						//if (dump) fprintf (stderr, "I%d k = %d b[k] = %g u[k] = %g sum_bk_uk = %g\n", (int)iteration_count, k, b[k], u_old[node+d_node[C->p[quadrant][k]]], sum_bk_uk);
					}
					u_00 = (u_00 + C->a0_const_2 * (sum_bk_uk + b[4])) * b[5];	/* Add point E in Fig A-1 to sum_bk_uk and normalize */
					//if (dump) fprintf (stderr, "I%d b[4] = %g b[5] = %g u_00 = %g\n\n", (int)iteration_count, b[4], b[5], u_00);
#ifndef PARALLEL_MODE
					briggs_index++;	/* Got to next sequential Briggs array index */
#endif
				}
				/* We now apply the over-relaxation: */
				u_00 = u_old[node] * C->relax_old + u_00 * C->relax_new;

				if (C->constrained) {	/* Must check that we don't exceed any imposed limits.  */
					if (C->set_limit[LO] && !gmt_M_is_fnan (C->Bound[LO]->data[node]) && u_00 < C->Bound[LO]->data[node])
						u_00 = C->Bound[LO]->data[node];
					else if (C->set_limit[HI] && !gmt_M_is_fnan (C->Bound[HI]->data[node]) && u_00 > C->Bound[HI]->data[node])
						u_00 = C->Bound[HI]->data[node];
				}
				u_change = fabs (u_00 - u_old[node]);		/* Change in node value between iterations */
				u_new[node] = (float)u_00;			/* Our updated estimate at this node */
				if (u_change > max_u_change) max_u_change = u_change;	/* Keep track of max u_change across all nodes */
#ifdef DEBUG_SURF
				//if (iteration_count == 7) fprintf (stderr, "u_new[%d] = u_new[%d,%d] = %g\n", node, row,col, u_new[node]);
#endif
			}	/* End of loop over columns */
		}	/* End of loop over rows [and possibly threads via OpenMP] */
	
		iteration_count++;	C->total_iterations++;	/* Update iteration counts for this stride and for total */
		max_z_change = max_u_change * C->z_rms;		/* Scale max_u_change back into original z units -> max_z_change */
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, C->format,
			C->current_stride, C->mode_type[mode], iteration_count, max_z_change, current_limit, C->total_iterations);
		if (C->logging) fprintf (C->fp_log, "%d\t%c\t%" PRIu64 "\t%.8g\t%.8g\t%" PRIu64 "\n", C->current_stride, C->mode_type[mode], iteration_count, max_z_change, current_limit, C->total_iterations);
#ifdef PARALLEL_MODE	/* Must impose the condition that # of iteration is even so that old_u (i.e. C->Grid->data) holds the final solution */
		finished = ((max_z_change <= current_limit || iteration_count >= current_max_iterations) && (iteration_count%2 == 0));
#else		/* Does not matter here since u_old == u_new anyway */
		finished = (max_z_change <= current_limit || iteration_count >= current_max_iterations);
#endif
#ifdef DEBUG_SURF
		if (save_intermediate) {	/* Dump this iteration grid */
			sprintf (file, "%s_%7.7d_%s_%d.nc", debug_prefix, (int)iteration_count, mode_name[mode], C->current_stride);
			gmt_M_memcpy (tmp_grid, u_new, C->current_mxmy, float);
			G->data = tmp_grid;	/* This is the latest solution */
			if (!nondimensional) {	/* Must scale back up and add plane */
				for (row = 0; row < C->current_ny; row++) {
					y_up = (double)(C->Grid->header->n_rows - row - 1);	/* # of rows from south (where y_up = 0) to this node */
					node = row_col_to_node (row, 0, C->current_mx);	/* Node index at left end of interior row */
					for (col = 0; col < C->current_nx; col++, node++)	/* March across this row */
					 	G->data[node] = (float)((G->data[node] * C->z_rms) + (evaluate_plane (C, col*C->current_stride, y_up*C->current_stride)));
				}
			}
			if (GMT_Write_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_GRID_ALL, NULL, file, G) != GMT_NOERROR)
				fprintf (stderr, "Writing %s failed\n", file);
			G->data = NULL;	/* Since it is not ours */
		}
#endif
		
	} while (!finished);

	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, C->format,
		C->current_stride, C->mode_type[mode], iteration_count, max_z_change, current_limit, C->total_iterations);

#ifdef DEBUG_SURF
	if (debug) {	/* Free this iteration grid */
		GMT_Destroy_Data (GMT->parent, &G);
	}	
#endif
	return (iteration_count);
}

GMT_LOCAL void check_errors (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* Compute misfits at original data locations,  This is only done at the
	 * final grid resolution, hence current_stride == 1. */

	uint64_t node, k;
	int row, col, *d_node = C->offset;
	unsigned char *status = C->status;

	double x0, y0, dx, dy, mean_error = 0.0, mean_squared_error = 0.0, z_est, z_err, curvature, c;
	double du_dx, du_dy, d2u_dx2, d2u_dxdy, d2u_dy2, d3u_dx3, d3u_dx2dy, d3u_dxdy2, d3u_dy3;

	float *u = C->Grid->data;
	struct GMT_GRID_HEADER *h = C->Grid->header;
	
	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Compute rms misfit and curvature.\n");

	set_BCs (GMT, C, u);	/* First update the boundary values */

	/* Estimate solution at all data constraints using 3rd order Taylor expansion from nearest node */

	for (k = 0; k < C->npoints; k++) {
		row = (int)index_to_row (C->data[k].index, C->n_columns);
		col = (int)index_to_col (C->data[k].index, C->n_columns);
		node = row_col_to_node (row, col, C->mx);
	 	if (status[node] == SURFACE_IS_CONSTRAINED) continue;	/* Since misfit by definition is zero so no point adding it */
		/* Get coordinates of this node */
		x0 = col_to_x (col, h->wesn[XLO], h->wesn[XHI], h->inc[GMT_X], h->n_columns);
		y0 = row_to_y (row, h->wesn[YLO], h->wesn[YHI], h->inc[GMT_Y], h->n_rows);
		/* Get dx,dy of data point away from this node */
		dx = x_to_fcol (C->data[k].x, x0, h->r_inc[GMT_X]);
		dy = y_to_frow (C->data[k].y, y0, h->r_inc[GMT_Y]);
 
	 	du_dx = 0.5 * (u[node+d_node[E1]] - u[node+d_node[W1]]);
	 	du_dy = 0.5 * (u[node+d_node[N1]] - u[node+d_node[S1]]);
	 	d2u_dx2 = u[node+d_node[E1]] + u[node+d_node[W1]] - 2 * u[node];
	 	d2u_dy2 = u[node+d_node[N1]] + u[node+d_node[S1]] - 2 * u[node];
	 	d2u_dxdy = 0.25 * (u[node+d_node[NE]] - u[node+d_node[NW]]
	 		- u[node+d_node[SE]] + u[node+d_node[SW]]);
	 	d3u_dx3 = 0.5 * (u[node+d_node[E2]] - 2 * u[node+d_node[E1]]
	 		+ 2 * u[node+d_node[W1]] - u[node+d_node[W2]]);
	 	d3u_dy3 = 0.5 * (u[node+d_node[N2]] - 2 * u[node+d_node[N1]]
	 		+ 2 * u[node+d_node[S1]] - u[node+d_node[S2]]);
	 	d3u_dx2dy = 0.5 * ((u[node+d_node[NE]] + u[node+d_node[NW]] - 2 * u[node+d_node[N1]])
	 		- (u[node+d_node[SE]] + u[node+d_node[SW]] - 2 * u[node+d_node[S1]]));
	 	d3u_dxdy2 = 0.5 * ((u[node+d_node[NE]] + u[node+d_node[SE]] - 2 * u[node+d_node[E1]])
	 		- (u[node+d_node[NW]] + u[node+d_node[SW]] - 2 * u[node+d_node[W1]]));

	 	/* Compute the 3rd order Taylor approximation from current node */
	 
	 	z_est = u[node] + dx * (du_dx +  dx * ((0.5 * d2u_dx2) + dx * (d3u_dx3 / 6.0)))
			+ dy * (du_dy +  dy * ((0.5 * d2u_dy2) + dy * (d3u_dy3 / 6.0)))
	 		+ dx * dy * (d2u_dxdy) + (0.5 * dx * d3u_dx2dy) + (0.5 * dy * d3u_dxdy2);
	 
	 	z_err = z_est - C->data[k].z;	/* Misfit between surface estimate and observation */
	 	mean_error += z_err;
	 	mean_squared_error += (z_err * z_err);
	 }
	 mean_error /= C->npoints;
	 mean_squared_error = sqrt (mean_squared_error / C->npoints);
	 
	/* Compute the total curvature of the grid */
	
	curvature = 0.0;
	gmt_M_grd_loop (GMT, C->Grid, row, col, node) {
 		c = u[node+d_node[E1]] + u[node+d_node[W1]] + u[node+d_node[N1]] + u[node+d_node[S1]] - 4.0 * u[node+d_node[E1]];
		curvature += (c * c);
	}

	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Fit info: N data points  N nodes\tmean error\trms error\tcurvature\n");
	sprintf (C->format,"\t%%8ld\t%%8ld\t%s\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, C->format, C->npoints, C->nxny, mean_error, mean_squared_error, curvature);
}

GMT_LOCAL void remove_planar_trend (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* Fit LS plane and remove trend from our (x,y,z) input data; we add trend to grid before output.
	 * Note: Here, x and y are first converted to fractional grid spacings from 0 to {n_columns,n_rows}-1.
	 * Hence the same scheme is used by replace_planar trend (i.e., just use row,col as coordinates).
	 * Note: The plane is fit to the original data z-values before normalizing by rms. */
	uint64_t k;
	double a, b, c, d, xx, y_up, zz, sx, sy, sz, sxx, sxy, sxz, syy, syz;
	struct GMT_GRID_HEADER *h = C->Grid->header;

	sx = sy = sz = sxx = sxy = sxz = syy = syz = 0.0;

	for (k = 0; k < C->npoints; k++) {	/* Sum up normal equation terms */
		xx = x_to_fcol (C->data[k].x, h->wesn[XLO], h->r_inc[GMT_X]);	/* Distance from west to this point */
		y_up = y_to_frow (C->data[k].y, h->wesn[YLO], h->r_inc[GMT_Y]);	/* Distance from south to this point */
		zz = C->data[k].z;
		sx += xx;		sy += y_up;		sz += zz;		sxx += (xx * xx);
		sxy += (xx * y_up);	sxz += (xx * zz);	syy += (y_up * y_up);	syz += (y_up * zz);
	}

	d = C->npoints*sxx*syy + 2*sx*sy*sxy - C->npoints*sxy*sxy - sx*sx*syy - sy*sy*sxx;

	if (d == 0.0) {	/* When denominator is zero we have a horizontal plane */
		C->plane_icept = C->plane_sx = C->plane_sy = 0.0;
		return;
	}

	a = sz*sxx*syy + sx*sxy*syz + sy*sxy*sxz - sz*sxy*sxy - sx*sxz*syy - sy*syz*sxx;
	b = C->npoints*sxz*syy + sz*sy*sxy + sy*sx*syz - C->npoints*sxy*syz - sz*sx*syy - sy*sy*sxz;
	c = C->npoints*sxx*syz + sx*sy*sxz + sz*sx*sxy - C->npoints*sxy*sxz - sx*sx*syz - sz*sy*sxx;

	C->plane_icept = a / d;
	C->plane_sx    = b / d;
	C->plane_sy    = c / d;
	if (C->periodic) C->plane_sx = 0.0;	/* Cannot have x-trend for periodic geographic data */

	for (k = 0; k < C->npoints; k++) {	/* Now remove this plane from the data constraints */
		xx = x_to_fcol (C->data[k].x, h->wesn[XLO], h->r_inc[GMT_X]);
		y_up = y_to_frow (C->data[k].y, h->wesn[YLO], h->r_inc[GMT_Y]);
		C->data[k].z -= (float)evaluate_plane(C, xx, y_up);
	}

	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Plane fit z = %g + (%g * col) + (%g * row)\n", C->plane_icept, C->plane_sx, C->plane_sy);
}

GMT_LOCAL void restore_planar_trend (struct SURFACE_INFO *C) {
	/* Scale grid back up by the data rms and restore the least-square plane.
	 * Note: In determining the plane and in evaluating it, remember that the
	 * x and y coordinates needed are not data coordinates but fractional col
	 * and row distances from an origin at the lower left (southwest corner).
	 * This means the y-values are positive up and increase in the opposite
	 * direction than how rows increase. Hence the use of y_up below. */
	unsigned int row, col;
	uint64_t node;
	float *u = C->Grid->data;
	double y_up;	/* Measure y up from south in fractional rows */

	for (row = 0; row < C->Grid->header->n_rows; row++) {
		y_up = (double)(C->Grid->header->n_rows - row - 1);	/* # of rows from south (where y_up = 0) to this node */
		node = row_col_to_node (row, 0, C->current_mx);	/* Node index at left end of interior row */
		for (col = 0; col < C->Grid->header->n_columns; col++, node++)	/* March across this row */
		 	u[node] = (float)((u[node] * C->z_rms) + (evaluate_plane (C, col, y_up)));
	}
}

GMT_LOCAL void throw_away_unusables (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* We eliminate data which will become unusable on the final iteration, when current_stride = 1.
	   It assumes current_stride = 1 and that set_grid_parameters has been called.
	   We sort, mark redundant data as SURFACE_OUTSIDE, and sort again, chopping off the excess.
	*/

	uint64_t last_index, n_outside, k,  n_ignored;
#ifdef DEBUG
	unsigned int last_kind = 0;
	char *point_type[2] = {"original", "breakline"};
#endif

	GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Eliminate data points that are not nearest a node.\n");

	/* Sort the data  */

	qsort_r (C->data, C->npoints, sizeof (struct SURFACE_DATA), compare_points, &(C->info));

	/* If more than one datum is indexed to the same node, only the first should be kept.
	   Mark the additional ones as SURFACE_OUTSIDE.
	*/
	last_index = UINTMAX_MAX;	n_outside = n_ignored = 0;
	for (k = 0; k < C->npoints; k++) {
		if (C->data[k].index == last_index) {	/* Same node but further away than our guy */
			C->data[k].index = SURFACE_OUTSIDE;
#ifdef DEBUG
			if (C->data[k].kind != last_kind)
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "The %s point %" PRId64 " has been replaced by a %s point.\n", point_type[C->data[k].kind], labs(C->data[k].number), point_type[last_kind]);
			else {
				GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "The %s point %" PRId64 " will be ignored.\n", point_type[C->data[k].kind], labs(C->data[k].number));
				n_ignored++;
			}
			//fprintf (stderr, "This points (x,y) vs previous (x,y): (%g, %g) vs {%g, %g}\n", C->data[k].x, C->data[k].y, C->data[k-1].x, C->data[k-1].y);
#endif
			n_outside++;
		}
		else {	/* New index, just update last_index */
			last_index = C->data[k].index;
		}
		last_kind = C->data[k].kind;
	}
	
	if (n_outside) {	/* Sort again; this time the SURFACE_OUTSIDE points will be sorted to end of the array */
		qsort_r (C->data, C->npoints, sizeof (struct SURFACE_DATA), compare_points, &(C->info));
		C->npoints -= n_outside;	/* Effectively chopping off the eliminated points */
		C->data = gmt_M_memory (GMT, C->data, C->npoints, struct SURFACE_DATA);	/* Adjust memory accordingly */
		if (n_ignored) {
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "%" PRIu64 " unusable points were supplied; these will be ignored.\n", n_ignored);
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "You should have pre-processed the data with block-mean, -median, or -mode.\n");
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Check that previous processing steps write results with enough decimals.\n");
			GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Possibly some data were half-way between nodes and subject to IEEE 754 rounding.\n");
		}
		if (C->save) {	/* Debug saving the final data constraints */
			char kind[2] = {'D', 'B'};
			FILE *fp = fopen ("surface.data", "w");
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Writing final usable data constraints to file surface.data.\n");
			fprintf (fp, "# x\ty\tz\tID\tindex\tkind\n");
			for (k = 0; k < C->npoints; k++)
				fprintf (fp, "%g\t%g\t%g\t%" PRId64 "\t%" PRIu64 "\t%c\n", C->data[k].x, C->data[k].y, C->data[k].z, C->data[k].number, C->data[k].index, kind[C->data[k].kind]);
			fclose (fp);
		}
	}
}

GMT_LOCAL int rescale_z_values (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* Find and normalize data by their rms value */
	uint64_t k;
	double ssz = 0.0;

	for (k = 0; k < C->npoints; k++) ssz += (C->data[k].z * C->data[k].z);
	C->z_rms = sqrt (ssz / C->npoints);
	GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Normalize detrended data constraints by z rms = %g\n", C->z_rms);

	if (C->z_rms < GMT_CONV8_LIMIT) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Input data lie exactly on a plane.\n");
		C->r_z_rms = C->z_rms = 1.0;
		return (1);	/* Flag to tell the main to just write out the plane */
	}
	else
		C->r_z_rms = 1.0 / C->z_rms;

	for (k = 0; k < C->npoints; k++) C->data[k].z *= (float)C->r_z_rms;

	if (C->converge_limit == 0.0 || C->converge_mode == BY_PERCENT) {	/* Set default values for convergence criteria */
		unsigned int ppm;
		double limit = (C->converge_mode == BY_PERCENT) ? C->converge_limit : SURFACE_CONV_LIMIT;
		ppm = urint (limit / 1.0e-6);
		C->converge_limit = limit * C->z_rms; /* i.e., 100 ppm of L2 scale */
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Select default convergence limit of %g (%u ppm of L2 scale)\n", C->converge_limit, ppm);
	}
	return (0);
}

GMT_LOCAL void suggest_sizes (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int factors[], unsigned int n_columns, unsigned int n_rows, bool pixel) {
	/* Calls gmt_optimal_dim_for_surface to determine if there are
	 * better choices for n_columns, n_rows that might speed up calculations
	 * by having many more common factors.
	 *
	 * W. H. F. Smith, 26 Feb 1992.  */

	unsigned int k;
	unsigned int n_sug = 0;	/* Number of suggestions found */
	struct GMT_SURFACE_SUGGESTION *sug = NULL;

	n_sug = gmt_optimal_dim_for_surface (GMT, factors, n_columns, n_rows, &sug);

	if (n_sug) {	/* We did find some suggestions, report them (up to the first 10 suggestions) */
		char region[GMT_LEN128] = {""}, buffer[GMT_LEN128] = {""};
		unsigned int m, save_range = GMT->current.io.geo.range;
		double w, e, s, n;
		GMT->current.io.geo.range = GMT_IS_GIVEN_RANGE;		/* Override this setting explicitly */
		for (k = 0; k < n_sug && k < 10; k++) {
			m = sug[k].n_columns - (G->header->n_columns - 1);	/* Additional nodes needed in x to give more factors */
			w = G->header->wesn[XLO] - (m/2)*G->header->inc[GMT_X];	/* Potential revised w/e extent */
			e = G->header->wesn[XHI] + (m/2)*G->header->inc[GMT_X];
			if (m%2) e += G->header->inc[GMT_X];
			m = sug[k].n_rows - (G->header->n_rows - 1);	/* Additional nodes needed in y to give more factors */
			s = G->header->wesn[YLO] - (m/2)*G->header->inc[GMT_Y];	/* Potential revised s/n extent */
			n = G->header->wesn[YHI] + (m/2)*G->header->inc[GMT_Y];
			if (m%2) n += G->header->inc[GMT_Y];
			if (pixel) {	/* Since we already added 1/2 pixel we need to undo that here so the report matches original phase */
				w -= G->header->inc[GMT_X] / 2.0;	e -= G->header->inc[GMT_X] / 2.0;
				s -= G->header->inc[GMT_Y] / 2.0;	n -= G->header->inc[GMT_Y] / 2.0;
			}
			gmt_ascii_format_col (GMT, buffer, w, GMT_OUT, GMT_X);
			sprintf (region, "-R%s/", buffer);
			gmt_ascii_format_col (GMT, buffer, e, GMT_OUT, GMT_X);
			strcat (region, buffer);	strcat (region, "/");
			gmt_ascii_format_col (GMT, buffer, s, GMT_OUT, GMT_Y);
			strcat (region, buffer);	strcat (region, "/");
			gmt_ascii_format_col (GMT, buffer, n, GMT_OUT, GMT_Y);
			strcat (region, buffer);
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Hint: Choosing %s [n_columns = %d, n_rows = %d] might cut run time by a factor of %.8g\n",
				region, sug[k].n_columns, sug[k].n_rows, sug[k].factor);
		}
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Hint: After completion you can recover the desired region via gmt grdcut\n");
		gmt_M_free (GMT, sug);
		GMT->current.io.geo.range = save_range;
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Cannot suggest any n_columns,n_rows better than your current -R -I settings.\n");
	return;
}

GMT_LOCAL void init_surface_parameters (struct SURFACE_INFO *C, struct SURFACE_CTRL *Ctrl) {
	/* Place program options into the surface struct.  This was done this way
	 * since surface.c relied heavily on global variables which are a no-no
	 * in GMT5.  The simplest solution was to collect all those variables into
	 * a single structure and pass a pointer to that structure to functions.
	 */
	if (Ctrl->S.active) {	/* Gave a serach radius; adjust if minutes or seconds where specified */
		if (Ctrl->S.unit == 'm') Ctrl->S.radius /= 60.0;
		if (Ctrl->S.unit == 's') Ctrl->S.radius /= 3600.0;
	}
	C->radius		= Ctrl->S.radius;
	C->relax_new		= Ctrl->Z.value;
	C->relax_old		= 1.0 - C->relax_new;
	C->max_iterations	= Ctrl->N.value;
	C->radius		= Ctrl->S.radius;
	C->limit_file[LO]	= Ctrl->L.file[LO];
	C->limit_file[HI]	= Ctrl->L.file[HI];
	C->set_limit[LO]	= Ctrl->L.mode[LO];
	C->set_limit[HI]	= Ctrl->L.mode[HI];
	C->limit[LO]		= Ctrl->L.limit[LO];
	C->limit[HI]		= Ctrl->L.limit[HI];
	C->boundary_tension	= Ctrl->T.b_tension;
	C->interior_tension	= Ctrl->T.i_tension;
	C->alpha		= Ctrl->A.value;
	C->converge_limit	= Ctrl->C.value;
	C->converge_mode	= Ctrl->C.mode;
	C->n_alloc		= GMT_INITIAL_MEM_ROW_ALLOC;
	C->z_rms		= 1.0;
	C->r_z_rms		= 1.0;
	C->mode_type[0]		= 'I';	/* I means exclude data points when iterating */
	C->mode_type[1]		= 'D';	/* D means include data points when iterating */
	C->n_columns			= C->Grid->header->n_columns;
	C->n_rows			= C->Grid->header->n_rows;
	C->nxny			= C->Grid->header->nm;
	C->mx			= C->Grid->header->mx;
	C->my			= C->Grid->header->my;
	C->mxmy			= C->Grid->header->size;
	gmt_M_memcpy (C->p, p, 20, unsigned int);
	
	gmt_M_memcpy (C->info.wesn, C->Grid->header->wesn, 4, double);
#ifdef DEBUG_SURF
	C->save = Ctrl->E.save;
#endif
}

double find_closest_point (double *x, double *y, double *z, uint64_t k, double x0, double y0, double half_dx, double half_dy, double *xx, double *yy, double *zz) {
	/* Find the point (xx,yy) on the line from (x[k-1],y[k-1]) to (x[k], y[k]) that is closest to (x0,y0).  If (xx,yy)
	 * is outside the end of the line segment or outside the bin then we return r = DBL_MAX */
	double dx, dy, a, r= DBL_MAX;	/* Initialize distance from (x0,y0) to nearest point (xx,yy) measured orthogonally onto break line */
	uint64_t km1 = k - 1;
	dx = x[k] - x[km1];	dy = y[k] - y[km1];
	if (gmt_M_is_zero (dx)) {	/* Break line is vertical */
		if ((y[k] <= y0 && y[km1] > y0) || (y[km1] <= y0 && y[k] > y0)) {	/* Nearest point is in same bin */
			*xx = x[k];	*yy = y0;
			r = fabs (*xx - x0);
			*zz = z[km1] + (z[k] - z[km1]) * (*yy - y[km1]) / dy;
		}
	}
	else if (gmt_M_is_zero (dy)) {	/* Break line is horizontal */
		if ((x[k] <= x0 && x[km1] > x0) || (x[km1] <= x0 && x[k] > x0)) {	/* Nearest point in same bin */
			*xx = x0;	*yy = y[k];
			r = fabs (*yy - y0);
			*zz = z[km1] + (z[k] - z[km1]) * (*xx - x[km1]) / dx;
		}
	}
	else {	/* General case.  Nearest orthogonal point may or may not be in bin, in which case r > r_prev */
		a = dy / dx;	/* Slope of line */
		*xx = (y0 - y[km1] + a * x[km1] + x0 / a) / (a + 1.0/a);
		*yy = a * (*xx - x[k]) + y[k];
		if ((x[k] <= *xx && x[km1] > *xx) || (x[km1] <= *xx && x[k] > *xx)) {	/* Orthonormal point found between the end points of line */
			if (fabs (*xx-x0) < half_dx && fabs (*yy-y0) < half_dy) {	/* Yes, within this bin */
				r = hypot (*xx - x0, *yy - y0);
				*zz = z[km1] + (z[k] - z[km1]) * (*xx - x[km1]) / dx;
			}
		}
	}
	return r;
}

GMT_LOCAL void interpolate_add_breakline (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, struct GMT_DATATABLE *T, char *file) {
	int srow, scol;
	uint64_t new_n = 0, n_int = 0, nb = 0;
	uint64_t k = 0, n, kmax = 0, kmin = 0, row, seg, node_this, node_prev;
	size_t n_alloc, n_alloc_b;
	double dx, dy, dz, r, r_this, r_min, x0_prev, y0_prev, x0_this, y0_this;
	double xx, yy, zz, half_dx, half_dy, zmin = DBL_MAX, zmax = -DBL_MAX;
	double *xline = NULL, *yline = NULL, *zline = NULL;
	double *x = NULL, *y = NULL, *z = NULL, *xb = NULL, *yb = NULL, *zb = NULL;
	char fname1[GMT_LEN256] = {""}, fname2[GMT_LEN256] = {""};
	FILE *fp1 = NULL, *fp2 = NULL;

	if (file) {
		sprintf (fname1, "%s.int",   file);
		sprintf (fname2, "%s.final", file);
		fp1 = fopen (fname1, "w");
		fp2 = fopen (fname2, "w");
	}
	/* Add constraints from breaklines */
	/* Reduce breaklines to the nearest point per node of cells crossed */

	n_alloc = n_alloc_b = GMT_INITIAL_MEM_ROW_ALLOC;
	xb = gmt_M_memory (GMT, NULL, n_alloc_b, double);
	yb = gmt_M_memory (GMT, NULL, n_alloc_b, double);
	zb = gmt_M_memory (GMT, NULL, n_alloc_b, double);

	x = gmt_M_memory (GMT, NULL, n_alloc, double);
	y = gmt_M_memory (GMT, NULL, n_alloc, double);
	z = gmt_M_memory (GMT, NULL, n_alloc, double);

	half_dx = 0.5 * C->inc[GMT_X];	half_dy = 0.5 * C->inc[GMT_Y];
	for (seg = 0; seg < T->n_segments; seg++) {
		xline = T->segment[seg]->data[GMT_X];
		yline = T->segment[seg]->data[GMT_Y];
		zline = T->segment[seg]->data[GMT_Z];
		/* 1. Interpolate the breakline to ensure there are points in every bin that it crosses */
		if (file) fprintf (fp1, "> Segment %d\n", (int)seg);
		for (row = k = 0, new_n = 1; row < T->segment[seg]->n_rows - 1; row++) {
			dx = xline[row+1] - xline[row];
			dy = yline[row+1] - yline[row];
			dz = zline[row+1] - zline[row];
			/* Given point spacing and grid spacing, how many points to interpolate? */
			n_int = lrint (hypot (dx, dy) * MAX (C->r_inc[GMT_X], C->r_inc[GMT_Y])) + 1;
			new_n += n_int;
			if (n_alloc <= new_n) {
				n_alloc += MAX (GMT_CHUNK, n_int);
				x = gmt_M_memory (GMT, x, n_alloc, double);
				y = gmt_M_memory (GMT, y, n_alloc, double);
				z = gmt_M_memory (GMT, z, n_alloc, double);
			}

			dx /= n_int;	dy /= n_int;	dz /= n_int;
			for (n = 0; n < n_int; k++, n++) {
				x[k] = xline[row] + n * dx;
				y[k] = yline[row] + n * dy;
				z[k] = zline[row] + n * dz;
				if (file) fprintf (fp1, "%g\t%g\t%g\n", x[k], y[k], z[k]);
			}
		}
		x[k] = xline[row];	y[k] = yline[row];	z[k] = zline[row];
		if (file) fprintf (fp1, "%g\t%g\t%g\n", x[k], y[k], z[k]);
	
		/* 2. Go along the (x,y,z), k = 1:new_n line and find the closest point to each bin node */
		if (file) fprintf (fp2, "> Segment %d\n", (int)seg);
		scol = x_to_col (x[0], C->Grid->header->wesn[XLO], C->r_inc[GMT_X]);
		srow = y_to_row (y[0], C->Grid->header->wesn[YLO], C->r_inc[GMT_Y], C->current_ny);
		node_this = row_col_to_node (srow, scol, C->current_mx);				/* The bin we are in */
		x0_this = col_to_x (scol, C->Grid->header->wesn[XLO], C->Grid->header->wesn[XHI], C->inc[GMT_X], C->current_nx);	/* Node center point */
		y0_this = row_to_y (srow, C->Grid->header->wesn[YLO], C->Grid->header->wesn[YHI], C->inc[GMT_Y], C->current_ny);
		r_min = hypot (x[0] - x0_this, y[0] - y0_this);	/* Distance from node center to start of breakline */
		xb[nb] = x[0];	yb[nb] = y[0];	zb[nb] = z[0];	/* Add this as our "nearest" breakline point (so far) for this bin */
		//fprintf (stderr, "p2 k = 0 nb = %d x = %g y = %g r = %g\n", (int)nb, xb[nb], yb[nb], r_min);
		for (k = 1; k < new_n; k++) {
			//fprintf (stderr, "-------------------------------------------------\n");
			/* Reset what is the previous point */
			node_prev = node_this;
			x0_prev = x0_this;	y0_prev = y0_this;
			scol = x_to_col (x[k], C->Grid->header->wesn[XLO], C->r_inc[GMT_X]);
			srow = y_to_row (y[k], C->Grid->header->wesn[YLO], C->r_inc[GMT_Y], C->current_ny);
			x0_this = col_to_x (scol, C->Grid->header->wesn[XLO], C->Grid->header->wesn[XHI], C->inc[GMT_X], C->current_nx);	/* Node center point */
			y0_this = row_to_y (srow, C->Grid->header->wesn[YLO], C->Grid->header->wesn[YHI], C->inc[GMT_Y], C->current_ny);
			node_this = row_col_to_node (srow, scol, C->current_mx);
			r_this = hypot (x[k] - x0_this, y[k] - y0_this);
			if (node_this == node_prev) {	/* Both points in same bin, see if 2nd point is closer */
				if (r_this < r_min) {	/* This point is closer than previous point */
					xb[nb] = x[k];	yb[nb] = y[k];	zb[nb] = z[k];
					r_min = r_this;
					//fprintf (stderr, "p1 k = %d nb = %d x = %g y = %g r = %g\n", (int)k, (int)nb, xb[nb], yb[nb], r_min);
				}
			}
			/* Find point on line closest to prev bin center */
			r = find_closest_point (x, y, z, k, x0_prev, y0_prev, half_dx, half_dy, &xx, &yy, &zz);
			if (r < r_min) {	/* Yes, closer than previous point */
				xb[nb] = xx;	yb[nb] = yy;	zb[nb] = zz;
				r_min = r;
				//fprintf (stderr, "x1 k = %d nb = %d x = %g y = %g r = %g\n", (int)k, (int)nb, xb[nb], yb[nb], r_min);
			}
			if (node_this != node_prev) {	/* Find point on line closest to this bin center */
				if (file) fprintf (fp2, "%g\t%g\t%g\n", xb[nb], yb[nb], zb[nb]);
				nb++;	/* OK, moving on from this bin */
				if (nb > n_alloc_b) {
					n_alloc_b += GMT_CHUNK;
					xb = gmt_M_memory (GMT, xb, n_alloc_b, double);
					yb = gmt_M_memory (GMT, yb, n_alloc_b, double);
					zb = gmt_M_memory (GMT, zb, n_alloc_b, double);
				}
				xb[nb] = x[k];	yb[nb] = y[k];	zb[nb] = z[k];
				r_min = r_this;
				//fprintf (stderr, "p2 k = %d nb = %d x = %g y = %g r = %g\n", (int)k, (int)nb, xb[nb], yb[nb], r_min);
				r = find_closest_point (x, y, z, k, x0_this, y0_this, half_dx, half_dy, &xx, &yy, &zz);
				if (r < r_min) {	/* Yes, closer than previous point */
					xb[nb] = xx;	yb[nb] = yy;	zb[nb] = zz;
					r_min = r;
					//fprintf (stderr, "x2 k = %d nb = %d x = %g y = %g r = %g\n", (int)k, (int)nb, xb[nb], yb[nb], r_min);
				}
			}
		}
		if (file) fprintf (fp2, "%g\t%g\t%g\n", xb[nb], yb[nb], zb[nb]);
		nb++;
	}
	if (file) {
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Reinterpolated breakline saved to file %s\n", fname1);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Final breakline constraints saved to file %s\n", fname2);
		fclose (fp1);
		fclose (fp2);
	}
	
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Found %d breakline points, reinterpolated to %d points, reduced to %d points\n", (int)T->n_records, (int)new_n, (int)nb);

	/* Now append the interpolated breakline to the C data structure */

	k = C->npoints;
	C->data = gmt_M_memory (GMT, C->data, k+nb, struct SURFACE_DATA);
	C->z_mean *= k;		/* It was already computed, reset it to the sum so we can add more and recalculate the mean */
	if (C->set_limit[LO] == DATA)	/* Lower limit should equal minimum data found.  Start with what we have so far and change if we find lower values */
		zmin = C->limit[LO];
	if (C->set_limit[HI] == DATA)	/* Upper limit should equal maximum data found.  Start with what we have so far and change if we find higher values */
		zmax = C->limit[HI];

	for (n = 0; n < nb; n++) {

		if (gmt_M_is_dnan (zb[n])) continue;

		scol = x_to_col (xb[n], C->Grid->header->wesn[XLO], C->r_inc[GMT_X]);
		if (scol < 0 || scol >= C->current_nx) continue;
		srow = y_to_row (yb[n], C->Grid->header->wesn[YLO], C->r_inc[GMT_Y], C->current_ny);
		if (srow < 0 || srow >= C->current_ny) continue;

		C->data[k].index = row_col_to_index (srow, scol, C->current_nx);
#ifdef DEBUG
		C->data[k].number = -(n + 1);
		// printf ("%g\t%g\t%g\n", x[n], y[n], z[n]);
#endif
		C->data[k].x = (float)xb[n];
		C->data[k].y = (float)yb[n];
		C->data[k].z = (float)zb[n];
		C->data[k].kind = SURFACE_BREAKLINE;	/* Mark as breakline constraint */
		if (zmin > zb[n]) zmin = z[n], kmin = k;
		if (zmax < zb[n]) zmax = z[n], kmax = k;
		k++;
		C->z_mean += zb[n];
	}

	if (k != (C->npoints + nb))		/* We had some NaNs */
		C->data = gmt_M_memory (GMT, C->data, k, struct SURFACE_DATA);

	C->npoints = k;
	C->z_mean /= k;

	if (C->set_limit[LO] == DATA)	/* Update our lower data-driven limit to the new minimum found */
		C->limit[LO] = C->data[kmin].z;
	if (C->set_limit[HI] == DATA)	/* Update our upper data-driven limit to the new maximum found */
		C->limit[HI] = C->data[kmax].z;

	gmt_M_free (GMT, x);
	gmt_M_free (GMT, y);
	gmt_M_free (GMT, z);
	gmt_M_free (GMT, xb);
	gmt_M_free (GMT, yb);
	gmt_M_free (GMT, zb);
}

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SURFACE_CTRL *C;
	
	C = gmt_M_memory (GMT, NULL, 1, struct SURFACE_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->N.value = SURFACE_MAX_ITERATIONS;
	C->A.value = 1.0;	/* Real xinc == yinc in terms of distances */
	C->W.file = strdup ("surface_log.txt");
	C->Z.value = SURFACE_OVERRELAXATION;
		
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct SURFACE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);	
	if (C->D.file) gmt_M_str_free (C->D.file);	
	if (C->L.file[LO]) gmt_M_str_free (C->L.file[LO]);	
	if (C->L.file[HI]) gmt_M_str_free (C->L.file[HI]);	
	if (C->W.file) gmt_M_str_free (C->W.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	unsigned int ppm;
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: surface [<table>] -G<outgrid> %s\n", GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-A<aspect_ratio>] [-C<convergence_limit>]\n", GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-D<breakline>] [-Ll<limit>] [-Lu<limit>] [-N<n_iterations>] [-Q] [-S<search_radius>[m|s]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T[i|b]<tension>] [%s] [-W[<logfile>]] [-Z<over_relaxation_parameter>]\n\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]%s[%s]\n\n",
		GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_s_OPT, GMT_x_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);
	ppm = urint (SURFACE_CONV_LIMIT / 1e-6);	/* Default convergence criteria */
	
	GMT_Message (API, GMT_TIME_NONE, "\t-G sets output grid file name.\n");
	GMT_Option (API, "I,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Set aspect-ratio> [Default = 1 gives an isotropic solution],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i.e., xinc and yinc assumed to give derivatives of equal weight; if not, specify\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <aspect_ratio> such that yinc = xinc / <aspect_ratio>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   e.g., if gridding lon,lat use <aspect_ratio> = cosine(middle of lat range).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Set final convergence limit; iteration stops when max |change| < <convergence_limit>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default will choose %g of the rms of your z data after removing L2 plane (%u ppm precision).\n", SURFACE_CONV_LIMIT, ppm);
	GMT_Message (API, GMT_TIME_NONE, "\t   Enter your own convergence limit in the same units as your z data.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Use xyz data in the <breakline> file as a 'soft breakline'.\n");
#ifdef DEBUG_SURF
	/* Paul Wessel testing only! */
	GMT_Message (API, GMT_TIME_NONE, "\t-E Special debug switches.  Append any combination of:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +1 : Force a single iteration cycle (no multigrid)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +a : Determine average data constraint [default select just nearest]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +o : Original algorithm (update on a single grid) [Default]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +m : Two grids, use memcpy to move latest to old, then update on new grid\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +p : Two grids, use pointers to alternate old and new grid\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +s : Save data constraints & breakline table after throw_away_unusables is done\n");
#endif	
	GMT_Message (API, GMT_TIME_NONE, "\t-L Constrain the range of output values:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ll<limit> specifies lower limit; forces solution to be >= <limit>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Lu<limit> specifies upper limit; forces solution to be <= <limit>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <limit> can be any number, or the letter d for min (or max) input data value,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or the filename of a grid with bounding values.  [Default solution is unconstrained].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Example: -Ll0 enforces a non-negative solution.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set max <n_iterations> in the final cycle; default = %d.\n", SURFACE_MAX_ITERATIONS);
	GMT_Message (API, GMT_TIME_NONE, "\t-S Set <search_radius> to initialize grid; default = 0 will skip this step.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This step is slow and not needed unless grid dimensions are pathological;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i.e., have few or no common factors.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append m or s to give <search_radius> in minutes or seconds.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Add Tension to the gridding equation; use a value between 0 and 1.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default = 0 gives minimum curvature (smoothest; bicubic) solution.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   1 gives a harmonic spline solution (local max/min occur only at data points).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Typically, 0.25 or more is good for potential field (smooth) data;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   0.5-0.75 or so for topography.  We encourage you to experiment.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend b to set tension in boundary conditions only;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend i to set tension in interior equations only;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   No appended letter sets tension for both to same value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Query for grid sizes that might run faster than your selected -R -I.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Write convergence information to log file [surface_log.txt]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Set <over_relaxation parameter>.  Default = %g.  Use a value\n", SURFACE_OVERRELAXATION);
	GMT_Message (API, GMT_TIME_NONE, "\t   between 1 and 2.  Larger number accelerates convergence but can be unstable.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use 1 if you want to be sure to have (slow) stable convergence.\n");
	GMT_Option (API, "a,bi3,di,f,h,i,r,s,x,:,.");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   Note: Geographic data with 360-degree range use periodic boundary condition in longitude.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t(For additional details, see Smith & Wessel, Geophysics, 55, 293-305, 1990.)\n");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SURFACE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* Parse the options provided and set parameters in CTRL structure.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, end;
	char modifier, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

#ifdef DEBUG_SURF
			case 'E':
				Ctrl->E.active = true;
				debug = 1; k = 0;
				while (opt->arg[k] == '+') {
					switch (opt->arg[k+1]) {
						case '1': Ctrl->E.once = true; break;
						case 'a': average = true; break;
						case 'o': debug = 1; break;
						case 'O': debug = 1; nondimensional = true; break;
						case 'p': debug = 3; break;
						case 'P': debug = 3; nondimensional = true; break;
						case 'm': debug = 2; break;
						case 'M': debug = 2; nondimensional = true; break;
						case 's': Ctrl->E.save = true; break;
					}
					k += 2;
				}
				if (opt->arg[k]) {
					strcpy (debug_prefix, &opt->arg[k]);
					save_intermediate = true;
					fprintf (stderr, "Save intermediate grids with prefix %s_\n", debug_prefix);
				}
				switch (debug) {
					case 1:
						fprintf (stderr, "Debug using original single grid iteration.\n");
						break;
					case 2:
						fprintf (stderr, "Debug using two grids, copying starting grid.\n");
						break;
					case 3:
						fprintf (stderr, "Debug using two grids, swapping pointers.\n");
						break;
				}
				break;
#endif
			case 'A':
				Ctrl->A.active = true;
				Ctrl->A.value = atof (opt->arg);
				break;
			case 'C':
				Ctrl->C.active = true;
				Ctrl->C.value = atof (opt->arg);
				if (strchr (opt->arg, '%')) {	/* Gave convergence in percent */
					Ctrl->C.mode = BY_PERCENT;
					Ctrl->C.value *= 0.01;
				}
				break;
			case 'D':
				if ((c = strstr (opt->arg, "+d"))) {
					c[0] = '\0';	/* Temporarily chop off +d part */
					Ctrl->D.debug = true;
				}
				if ((Ctrl->D.active = gmt_check_filearg (GMT, 'D', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0)
					Ctrl->D.file = strdup (opt->arg);
				else
					n_errors++;
				if (Ctrl->D.debug) c[0] = '+';	/* Restore original string */
				break;
			case 'G':
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				Ctrl->I.active = true;
				if (gmt_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					gmt_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':	/* Set limits */
				Ctrl->L.active = true;
				switch (opt->arg[0]) {
					case 'l': case 'u':	/* Lower or upper limits  */
						end = (opt->arg[0] == 'l') ? LO : HI;	/* Which one it is */
						n_errors += gmt_M_check_condition (GMT, opt->arg[1] == 0, "Syntax error -L%c option: No argument given\n", opt->arg[0]);
						Ctrl->L.file[end] = strdup (&opt->arg[1]);
						if (!gmt_access (GMT, Ctrl->L.file[end], F_OK))	/* File exists */
							Ctrl->L.mode[end] = SURFACE;
						else if (Ctrl->L.file[end][0] == 'd')		/* Use data minimum */
							Ctrl->L.mode[end] = DATA;
						else {
							Ctrl->L.mode[end] = VALUE;		/* Use given value */
							Ctrl->L.limit[end] = atof (&opt->arg[1]);
						}
						break;
					default:
						n_errors++;
						break;
				}
				break;
			case 'N':
				Ctrl->N.active = true;
				Ctrl->N.value = atoi (opt->arg);
				break;
			case 'Q':
				Ctrl->Q.active = true;
				break;
			case 'S':
				Ctrl->S.active = true;
				Ctrl->S.radius = atof (opt->arg);
				Ctrl->S.unit = opt->arg[strlen(opt->arg)-1];
				if (Ctrl->S.unit == 'c' && gmt_M_compat_check (GMT, 4)) {
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Unit c for seconds is deprecated; use s instead.\n");
					Ctrl->S.unit = 's';
				}
				if (!strchr ("sm ", Ctrl->S.unit)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -S option: Unrecognized unit %c\n", Ctrl->S.unit);
					n_errors++;
				}
				break;
			case 'T':
				Ctrl->T.active = true;
				k = 0;
				if (gmt_M_compat_check (GMT, 4)) {	/* GMT4 syntax allowed for upper case */
					modifier = opt->arg[strlen(opt->arg)-1];
					if (modifier == 'B') modifier = 'b';
					else if (modifier == 'I') modifier = 'i';
					if (!(modifier == 'b' || modifier == 'i'))
						modifier = opt->arg[0], k = 1;
				}
				else {
					modifier = opt->arg[0];
					k = 1;
				}
				if (modifier == 'b') {
					Ctrl->T.b_tension = atof (&opt->arg[k]);
				}
				else if (modifier == 'i') {
					Ctrl->T.i_tension = atof (&opt->arg[k]);
				}
				else if (modifier == '.' || (modifier >= '0' && modifier <= '9')) {
					Ctrl->T.i_tension = Ctrl->T.b_tension = atof (opt->arg);
				}
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Unrecognized modifier %c\n", modifier);
					n_errors++;
				}
				break;
			case 'W':
				Ctrl->W.active = true;
				if (opt->arg[0]) {	/* Specified named log file */
					gmt_M_str_free (Ctrl->W.file);
					Ctrl->W.file = strdup (opt->arg);
				}
				break;
			case 'Z':
				Ctrl->Z.active = true;
				Ctrl->Z.value = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	gmt_check_lattice (GMT, Ctrl->I.inc, NULL, &Ctrl->I.active);

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.value < 1, "Syntax error -N option: Max iterations must be nonzero\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.value < 0.0 || Ctrl->Z.value > 2.0, "Syntax error -Z option: Relaxation value must be 1 <= z <= 2\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Syntax error option -G: Must specify output grid file\n");
	n_errors += gmt_check_binary_io (GMT, 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_surface_mt (void *V_API, int mode, void *args) {
	int error = 0, key, one = 1, end;
	char *limit[2] = {"lower", "upper"};
	double wesn[4];
	
	struct SURFACE_INFO C;
	struct SURFACE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = gmt_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);
	
	/*---------------------------- This is the surface main code ----------------------------*/

	gmt_enable_threads (GMT);	/* Set number of active threads, if supported */
	/* Some initializations and defaults setting */
	gmt_M_memset (&C, 1, struct SURFACE_INFO);

	gmt_M_memcpy (C.wesn_orig, GMT->common.R.wesn, 4, double);	/* Save original region in case user specified -r */
	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);		/* Save specified region */
	C.periodic = (gmt_M_is_geographic (GMT, GMT_IN) && gmt_M_360_range (wesn[XLO], wesn[XHI]));
	if (GMT->common.r.active) {		/* Pixel registration request. Use the trick of offsetting area by x_inc(y_inc) / 2 */
		/* Note that the grid remains node-registered and only gets tagged as pixel-registered upon writing the final grid to file */
		wesn[XLO] += Ctrl->I.inc[GMT_X] / 2.0;	wesn[XHI] += Ctrl->I.inc[GMT_X] / 2.0;
		wesn[YLO] += Ctrl->I.inc[GMT_Y] / 2.0;	wesn[YHI] += Ctrl->I.inc[GMT_Y] / 2.0;
		/* n_columns,n_rows remain the same for now but nodes are in "pixel" position.  We reset to original wesn and reduce n_columns,n_rows by 1 when we write result */
	}
	
	if ((C.Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_HEADER_ONLY, NULL, wesn, Ctrl->I.inc, \
		GMT_GRID_NODE_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);
	
	if (C.Grid->header->n_columns < 4 || C.Grid->header->n_rows < 4) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: Grid must have at least 4 nodes in each direction (you have %d by %d) - abort.\n", C.Grid->header->n_columns, C.Grid->header->n_rows);
		Return (GMT_RUNTIME_ERROR);
	}

	init_surface_parameters (&C, Ctrl);	/* Pass parameters from parsing control to surface information structure C */

	/* Determine the initial and intermediate grid dimensions */
	C.current_stride = gmt_gcd_euclid (C.n_columns-1, C.n_rows-1);

	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE) || Ctrl->Q.active) {
		sprintf (C.format, "Grid domain: W: %s E: %s S: %s N: %s n_columns: %%d n_rows: %%d [", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		(GMT->common.r.active) ? strcat (C.format, "pixel registration]\n") : strcat (C.format, "gridline registration]\n");
		GMT_Report (API, GMT_MSG_VERBOSE, C.format, C.wesn_orig[XLO], C.wesn_orig[XHI], C.wesn_orig[YLO], C.wesn_orig[YHI], C.n_columns-one, C.n_rows-one);
	}
	if (C.current_stride == 1) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Your grid dimensions are mutually prime.  Convergence is very unlikely.\n");
	if ((C.current_stride == 1 && gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) || Ctrl->Q.active) suggest_sizes (GMT, C.Grid, C.factors, C.n_columns-1, C.n_rows-1, GMT->common.r.active);
	if (Ctrl->Q.active) Return (GMT_NOERROR);

	/* Set current_stride = 1, read data, setting indices.  Then throw
	   away data that can't be used in the end game, limiting the
	   size of data arrays and Briggs->b[6] structure/array.  */

	C.current_stride = 1;
	set_grid_parameters (&C);
	if (read_data_surface (GMT, &C, options))
		Return (GMT_RUNTIME_ERROR);
	if (Ctrl->D.active) {	/* Append breakline dataset */
		struct GMT_DATASET *Lin = NULL;
		char *file = (Ctrl->D.debug) ? Ctrl->D.file : NULL;
		if ((Lin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_READ_NORMAL, NULL, Ctrl->D.file, NULL)) == NULL)
			Return (API->error);
		if (Lin->n_columns < 2) {
			GMT_Report (API, GMT_MSG_NORMAL, "Input file %s has %d column(s) but at least 2 are needed\n", Ctrl->D.file, (int)Lin->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		interpolate_add_breakline (GMT, &C, Lin->table[0], file);	/* Pass the single table since we read a single file */
	}
	
	throw_away_unusables (GMT, &C);		/* Eliminate data points that will not serve as constraints */
	remove_planar_trend (GMT, &C);		/* Fit best-fitting plane and remove it from the data; plane will be restored at the end */
	key = rescale_z_values (GMT, &C);	/* Divide residual data by their rms value */
	
	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, C.Grid) != GMT_NOERROR) Return (API->error);
	if (key == 1) {	/* Data lie exactly on a plane; write a grid with the plan and exit */
		gmt_M_free (GMT, C.data);	/* The data set is no longer needed */
		if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, NULL, NULL,
			0, 0, C.Grid) == NULL) Return (API->error);	/* Get a grid of zeros... */
		restore_planar_trend (&C);	/* ...and restore the plane we found */
		if ((error = write_surface (GMT, &C, Ctrl->G.file)) != 0)	/* Write this grid */
			Return (error);
		Return (GMT_NOERROR);	/* Clean up and return */
	}
	
	load_constraints (GMT, &C, true);	/* Set lower and upper constraint grids, if requested */

	/* Set up factors and reset current_stride to its initial (and largest) value  */

	C.current_stride = gmt_gcd_euclid (C.n_columns-1, C.n_rows-1);
	C.n_factors = gmt_get_prime_factors (GMT, C.current_stride, C.factors);
#ifdef DEBUG
	if (Ctrl->E.once) C.current_stride = 1;	/* Force final iteration stage immediately */
#endif
	set_grid_parameters (&C);
	while (C.current_nx < 4 || C.current_ny < 4) {	/* Must have at least a grid of 4x4 */
		smart_divide (&C);
		set_grid_parameters (&C);
	}
	set_offset (&C);	/* Initialize the node-jumps across rows for this grid size */
	set_index (GMT, &C);	/* Determine the nearest data constraint for this grid size */

	if (Ctrl->W.active) {	/* Want to log convergence information to file */
		if ((C.fp_log = gmt_fopen (GMT, Ctrl->W.file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: Unable to create log file %s.\n", Ctrl->W.file);
			Return (GMT_ERROR_ON_FOPEN);
		}
		C.logging = true;
		fprintf (C.fp_log, "#grid\tmode\tgrid_iteration\tchange\tlimit\ttotal_iteration\n");
	}
	
	/* Now the data are ready to go for the first iteration.  */

	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) {	/* Report on memory usage for this run */
		size_t mem_use, mem_total;
		mem_use = mem_total = C.npoints * sizeof (struct SURFACE_DATA);
		GMT_Report (API, GMT_MSG_VERBOSE, "------------------------------------------\n");
		GMT_Report (API, GMT_MSG_VERBOSE, "%-31s: %9s\n", "Memory for data array", gmt_memory_use (mem_use));
		mem_use = sizeof (struct GMT_GRID) + C.Grid->header->size * sizeof (float);	mem_total += mem_use;
		GMT_Report (API, GMT_MSG_VERBOSE, "%-31s: %9s\n", "Memory for final grid", gmt_memory_use (mem_use));
		for (end = LO; end <= HI; end++) if (C.set_limit[end]) {	/* Will need to keep a lower|upper surface constrain grid */
			mem_total += mem_use;
			GMT_Report (API, GMT_MSG_VERBOSE, "%-31s: %9s\n", "Memory for constraint grid", gmt_memory_use (mem_use));
		}
#ifdef PARALLEL_MODE
		mem_total += mem_use;
		GMT_Report (API, GMT_MSG_VERBOSE, "%-31s: %9s\n", "Memory for alternate grid", gmt_memory_use (mem_use));
		mem_use = C.Grid->header->size * sizeof (uint64_t);	mem_total += mem_use;
		GMT_Report (API, GMT_MSG_VERBOSE, "%-31s: %9s\n", "Memory for Briggs indices", gmt_memory_use (mem_use));
#endif
		mem_use = C.npoints * sizeof (struct SURFACE_BRIGGS) ;	mem_total += mem_use;
		GMT_Report (API, GMT_MSG_VERBOSE, "%-31s: %9s\n", "Memory for Briggs coefficients", gmt_memory_use (mem_use));
		mem_use = C.Grid->header->size;	mem_total += mem_use;
		GMT_Report (API, GMT_MSG_VERBOSE, "%-31s: %9s\n", "Memory for node status", gmt_memory_use (mem_use));
		GMT_Report (API, GMT_MSG_VERBOSE, "------------------------------------------\n");
		GMT_Report (API, GMT_MSG_VERBOSE, "%-31s: %9s\n", "Total memory use", gmt_memory_use (mem_total));
		GMT_Report (API, GMT_MSG_VERBOSE, "==========================================\n");
	}

	/* Allocate the memory needed to perform the gridding  */

	C.Briggs   = gmt_M_memory (GMT, NULL, C.npoints, struct SURFACE_BRIGGS);
	C.status   = gmt_M_memory (GMT, NULL, C.mxmy, char);
	C.fraction = gmt_M_memory (GMT, NULL, C.current_stride, double);
	if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_GRID_DATA_ONLY, NULL, NULL, NULL, 0, 0, C.Grid) == NULL)
		Return (API->error);
#ifdef PARALLEL_MODE
	/* To avoid race conditions we must alternate by updating one grid with values from another */
	C.alternate_grid = gmt_M_memory_aligned (GMT, NULL, C.Grid->header->size, float);
	/* Because threads run simultaneously we cannot access the Briggs array sequentially and must use a helper index array */
	C.briggs_index   = gmt_M_memory (GMT, NULL, C.mxmy, uint64_t);
#endif
	if (C.radius > 0) initialize_grid (GMT, &C); /* Fill in nodes with a weighted average in a search radius  */
#ifdef DEBUG_SURF
	if (debug)
		tmp_grid = gmt_M_memory_aligned (GMT, NULL, C.Grid->header->size, float);
#endif
	GMT_Report (API, GMT_MSG_VERBOSE, "Grid\tMode\tIteration\tMax Change\tConv Limit\tTotal Iterations\n");

	set_coefficients (GMT, &C);	/* Initialize the coefficients needed in the finite-difference expressions */

	/* Here is the main multigrid loop, were we first grid using a coarse grid and the
	 * progressively refine the grid until we reach the final configuration. */
	
	C.previous_stride = C.current_stride;
#ifdef DEBUG_SURF
	if (average)
		find_mean_constraint (GMT, &C);		/* Assign mean data value to nodes and evaluate Briggs coefficients */
	else
#endif
	find_nearest_constraint (GMT, &C);		/* Assign nearest data value to nodes and evaluate Briggs coefficients */
	iterate (GMT, &C, GRID_DATA);			/* Grid the data using the data constraints */
	 
	while (C.current_stride > 1) {	/* More intermediate grids remain, go to next */
		smart_divide (&C);			/* Set the new current_stride */
		set_grid_parameters (&C);		/* Update node book-keeping constants */
		set_offset (&C);			/* Reset the node-jumps across rows for this grid size */
		set_index (GMT, &C);			/* Recompute the index values for the nearest data points */
		fill_in_forecast (GMT, &C);		/* Expand the grid and fill it via bilinear interpolation */
		iterate (GMT, &C, GRID_NODES);		/* Grid again but only to improve on the bilinear guesses */
#ifdef DEBUG_SURF
		if (average)
			find_mean_constraint (GMT, &C);	/* Assign mean data value to nodes and evaluate Briggs coefficients */
		else
#endif
		find_nearest_constraint (GMT, &C);	/* Assign nearest data value to nodes and evaluate Briggs coefficients */
		iterate (GMT, &C, GRID_DATA);		/* Grid the data but now use the data constraints */
		C.previous_stride = C.current_stride;	/* Remember previous stride before we smart-divide again */
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE)) check_errors (GMT, &C);	/* Report on mean misfit and curvature */

	restore_planar_trend (&C);	/* Restore the least-square plane we removed earlier */

	if (Ctrl->W.active)	/* Close the log file */
		gmt_fclose (GMT, C.fp_log);

	/* Write the output grid */
	if ((error = write_surface (GMT, &C, Ctrl->G.file)) != 0)
		Return (error);

	/* Clean up after ourselves */
	
#ifdef PARALLEL_MODE
	gmt_M_free_aligned (GMT, C.alternate_grid);
	gmt_M_free (GMT, C.briggs_index);
#endif
#ifdef DEBUG_SURF
	if (debug) gmt_M_free (GMT, tmp_grid);
#endif
	gmt_M_free (GMT, C.data);
	gmt_M_free (GMT, C.Briggs);
	gmt_M_free (GMT, C.status);
	gmt_M_free (GMT, C.fraction);
	for (end = LO; end <= HI; end++) if (C.set_limit[end]) {	/* Free lower|upper surface constrain grids */
		if (GMT_Destroy_Data (API, &C.Bound[end]) != GMT_NOERROR)
			GMT_Report (API, GMT_MSG_NORMAL, "Failed to free grid with %s bounds\n", limit[end]);
	}

	Return (GMT_NOERROR);
}
