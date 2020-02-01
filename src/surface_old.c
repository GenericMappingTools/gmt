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
 * surface.c: a gridding program.
 * reads xyz triples and fits a surface to the data.
 * surface satisfies (1 - T) D4 z - T D2 z = 0,
 * where D4 is the biharmonic operator,
 * D2 is the Laplacian,
 * and T is a "tension factor" between 0 and 1.
 * End member T = 0 is the classical minimum curvature
 * surface.  T = 1 gives a harmonic surface.  Use T = 0.25
 * or so for potential data; something more for topography.
 *
 * Program includes overrelaxation for fast convergence and
 * automatic optimal grid factorization.
 *
 * See Smith & Wessel (Geophysics, 3, 293-305, 1990) for details.
 *
 * Authors:	Walter H. F. Smith and Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 * Tech Note:	1. surface uses old grid-structure (transpose of current GMT grid)
 *		   and must therefore waste time transposing grid before writing.
 *		   However, this is a tiny fraction of total run time.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"surface_old"
#define THIS_MODULE_MODERN_NAME	"surface_old"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Grid table data using adjustable tension continuous curvature splines"
#define THIS_MODULE_KEYS	"<D{,DD(,LG(=1,GG}"
#define THIS_MODULE_NEEDS	"R"
#define THIS_MODULE_OPTIONS "-:RVabdefhirs" GMT_OPT("FH")

struct SURFACE_CTRL {
	struct A {	/* -A<aspect_ratio>|m */
		bool active;
		unsigned int mode;	/* 1 if we want cos(mid_latitude) */
		double value;
	} A;
	struct C {	/* -C<converge_limit> */
		bool active;
		unsigned int mode;	/* 1 if given as fraction */
		double value;
	} C;
	struct D {	/* -D<line.xyz> */
		bool active;
		char *file;	/* Name of file with breaklines */
	} D;
	struct G {	/* -G<file> */
		bool active;
		char *file;
	} G;
	struct L {	/* -Ll|u<limit> */
		bool active;
		char *low, *high;
		double min, max;
		unsigned int lmode, hmode;
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
	struct Z {	/* -Z<over_relaxation_parameter> */
		bool active;
		double value;
	} Z;
};

#define SURFACE_OUTSIDE		LONG_MAX	/* Index number indicating data is outside usable area */
#define SURFACE_CONV_LIMIT	0.0001		/* Default is 100 ppm of data range as convergence criterion */
#define SURFACE_MAX_ITERATIONS	500		/* Default iterations at final grid size */
#define SURFACE_OVERRELAXATION	1.4		/* Default over-relaxation value */
#define SURFACE_CLOSENESS_FACTOR	0.05	/* A node is considered known if the nearest data is within 0.05 of a gridspacing of the node */
#define SURFACE_IS_UNCONSTRAINED	0	/* Various constants used to flag nearest node */
#define SURFACE_DATA_IS_IN_QUAD1	1
#define SURFACE_DATA_IS_IN_QUAD2	2
#define SURFACE_DATA_IS_IN_QUAD3	3
#define SURFACE_DATA_IS_IN_QUAD4	4
#define SURFACE_IS_CONSTRAINED		5

static unsigned int p[5][4] = {	/* Indices into C->offset for each of the 4 quadrants, i.e. C->offset[kase][p[quadrant][k]]] */
	{ 0, 0, 0,  0},	/* This row is not used */
	{10, 9, 5,  1},	/* Indices for 1st quadrant */
	{ 8, 9, 6,  3},	/* Indices for 2nd quadrant */
	{ 1, 2, 6, 10},	/* Indices for 3rd quadrant */
	{ 3, 2, 5,  8}	/* Indices for 4th quadrant */
};

struct SURFACE_DATA {	/* Data point and index to node it currently constrains  */
	gmt_grdfloat x;
	gmt_grdfloat y;
	gmt_grdfloat z;
	uint64_t index;
#ifdef DEBUG	/* For debugging purposes only - it is the original input data point number before sorting */
	uint64_t number;
#endif
};

struct SURFACE_BRIGGS {		/* Coefficients in Taylor series for Laplacian(z) a la I. C. Briggs (1974)  */
	double b[6];
};

struct SURFACE_GLOBAL {		/* Things needed inside compare function must be global for now */
	int block_ny;		/* Number of nodes in y-dir for a given grid factor */
	double grid_xinc, grid_yinc;	/* size of each grid cell for a given grid factor */
	double x_min, y_min;		/* Lower left corner of grid */
} GMT_Surface_Global;

struct SURFACE_INFO {	/* Control structure for surface setup and execution */
	unsigned char *iu;		/* Pointer to grid info array */
	char mode_type[2];		/* D means include data points when iterating
					 * I means just interpolate from larger grid */
	char format[GMT_BUFSIZ];
	char *low_file, *high_file;	/* Pointers to grids with low and high limits, if selected */
	int grid, old_grid;	/* Node spacings  */
	unsigned int n_fact;		/* Number of factors in common (n_rows-1, n_columns-1) */
	unsigned int factors[32];		/* Array of common factors */
	unsigned int set_low;		/* 0 unconstrained,1 = by min data value, 2 = by user value */
	unsigned int set_high;		/* 0 unconstrained,1 = by max data value, 2 = by user value */
	size_t n_alloc;
	uint64_t npoints;			/* Number of data points */
	uint64_t ij_sw_corner, ij_se_corner,ij_nw_corner, ij_ne_corner;
	uint64_t n_empty;		/* No of unconstrained nodes at initialization  */
	int n_columns;				/* Number of nodes in x-dir. */
	int n_rows;				/* Number of nodes in y-dir. (Final grid) */
	uint64_t nxny;		/* Total number of grid nodes without boundaries  */
	int mx;
	int my;
	uint64_t mxmy;		/* Total number of grid nodes with boundaries  */
	int block_nx;		/* Number of nodes in x-dir for a given grid factor */
	int block_ny;		/* Number of nodes in y-dir for a given grid factor */
	unsigned int max_iterations;	/* Max iter per call to iterate */
	unsigned int converge_limit_mode;	/* 1 if -C set fractional convergence limit */
	uint64_t total_iterations;
	bool periodic;		/* true if geographic grid and west-east == 360 */
	int grid_east;
	int offset[25][12];	/* Indices of 12 nearby points in 25 cases of edge conditions  */
	bool constrained;		/* true if set_low or set_high is true */
	double low_limit, high_limit;	/* Constrains on range of solution */
	double grid_xinc, grid_yinc;	/* size of each grid cell for a given grid factor */
	double r_grid_xinc, r_grid_yinc;	/* Reciprocals  */
	double converge_limit;		/* Convergence limit */
	double radius;			/* Search radius for initializing grid  */
	double tension;			/* Tension parameter on the surface  */
	double boundary_tension;
	double interior_tension;
	double a0_const_1, a0_const_2;	/* Constants for off grid point equation  */
	double e_2, e_m2, one_plus_e2;
	double eps_p2, eps_m2, two_plus_ep2, two_plus_em2;
	double x_edge_const, y_edge_const;
	double l_epsilon;
	double z_mean;
	double z_scale;			/* Root mean square range of z after removing planar trend  */
	double r_z_scale;		/* reciprocal of z_scale  */
	double plane_c0, plane_c1, plane_c2;	/* Coefficients of best fitting plane to data  */
	double coeff[2][12];		/* Coefficients for 12 nearby points, constrained and unconstrained  */
	double relax_old, relax_new;	/* Coefficients for relaxation factor to speed up convergence */
	double wesn_orig[4];		/* Original -R domain as we might have shifted it due to -r */
	struct SURFACE_DATA  *data;
	struct SURFACE_BRIGGS *briggs;
	struct GMT_GRID *Grid;			/* The final grid */
	struct GMT_GRID *Low, *High;		/* arrays for minmax values, if set */
};

GMT_LOCAL void set_coefficients (struct SURFACE_INFO *C) {
	/* These are the coefficients in the finite-difference expressionss */
	double e_4, loose, a0;

	loose = 1.0 - C->interior_tension;
	C->e_2 = C->l_epsilon * C->l_epsilon;
	e_4 = C->e_2 * C->e_2;
	C->eps_p2 = C->e_2;
	C->eps_m2 = 1.0/C->e_2;
	C->one_plus_e2 = 1.0 + C->e_2;
	C->two_plus_ep2 = 2.0 + 2.0*C->eps_p2;
	C->two_plus_em2 = 2.0 + 2.0*C->eps_m2;

	C->x_edge_const = 4 * C->one_plus_e2 - 2 * (C->interior_tension / loose);
	C->e_m2 = 1.0 / C->e_2;
	C->y_edge_const = 4 * (1.0 + C->e_m2) - 2 * (C->interior_tension * C->e_m2 / loose);


	a0 = 1.0 / ( (6 * e_4 * loose + 10 * C->e_2 * loose + 8 * loose - 2 * C->one_plus_e2) + 4*C->interior_tension*C->one_plus_e2);
	C->a0_const_1 = 2 * loose * (1.0 + e_4);
	C->a0_const_2 = 2.0 - C->interior_tension + 2 * loose * C->e_2;

	C->coeff[1][4] = C->coeff[1][7] = -loose;
	C->coeff[1][0] = C->coeff[1][11] = -loose * e_4;
	C->coeff[0][4] = C->coeff[0][7] = -loose * a0;
	C->coeff[0][0] = C->coeff[0][11] = -loose * e_4 * a0;
	C->coeff[1][5] = C->coeff[1][6] = 2 * loose * C->one_plus_e2;
	C->coeff[0][5] = C->coeff[0][6] = (2 * C->coeff[1][5] + C->interior_tension) * a0;
	C->coeff[1][2] = C->coeff[1][9] = C->coeff[1][5] * C->e_2;
	C->coeff[0][2] = C->coeff[0][9] = C->coeff[0][5] * C->e_2;
	C->coeff[1][1] = C->coeff[1][3] = C->coeff[1][8] = C->coeff[1][10] = -2 * loose * C->e_2;
	C->coeff[0][1] = C->coeff[0][3] = C->coeff[0][8] = C->coeff[0][10] = C->coeff[1][1] * a0;

	C->e_2 *= 2;		/* We will need these in boundary conditions  */
	C->e_m2 *= 2;

	C->ij_sw_corner = 2 * C->my + 2;			/*  Corners of array of actual data  */
	C->ij_se_corner = C->ij_sw_corner + (C->n_columns - 1) * C->my;
	C->ij_nw_corner = C->ij_sw_corner + C->n_rows - 1;
	C->ij_ne_corner = C->ij_se_corner + C->n_rows - 1;
}

GMT_LOCAL void set_offset (struct SURFACE_INFO *C) {
	/* Because of the multigrid approach the distance from the central node to
	 * its neighbors needed in the finite difference expressions varies.  E.g.,
	 * when C->grid is 8 then the next neighbor to the right is 8 columns over.
	 * But at the boundaries the spacing is always 1.  Thus, as the current node
	 * moves over the interior of the grid the distances change as we get close
	 * to any of the 4 boundaries.  The offset array is used to determine what
	 * the offset in rows and columns are relative to the current point, given
	 * what "kase" we are examining.  kase is a combination of x and y information
	 * related to how close we are to the left/right or top/bottom boundary.
	 */
	int add_w[5], add_e[5], add_s[5], add_n[5], add_w2[5], add_e2[5], add_s2[5], add_n2[5];
	unsigned int i, j, kase;

	add_w[0] = -C->my; add_w[1] = add_w[2] = add_w[3] = add_w[4] = -C->grid_east;
	add_w2[0] = -2 * C->my;  add_w2[1] = -C->my - C->grid_east;  add_w2[2] = add_w2[3] = add_w2[4] = -2 * C->grid_east;
	add_e[4] = C->my; add_e[0] = add_e[1] = add_e[2] = add_e[3] = C->grid_east;
	add_e2[4] = 2 * C->my;  add_e2[3] = C->my + C->grid_east;  add_e2[2] = add_e2[1] = add_e2[0] = 2 * C->grid_east;

	add_n[4] = 1; add_n[3] = add_n[2] = add_n[1] = add_n[0] = C->grid;
	add_n2[4] = 2;  add_n2[3] = C->grid + 1;  add_n2[2] = add_n2[1] = add_n2[0] = 2 * C->grid;
	add_s[0] = -1; add_s[1] = add_s[2] = add_s[3] = add_s[4] = -C->grid;
	add_s2[0] = -2;  add_s2[1] = -C->grid - 1;  add_s2[2] = add_s2[3] = add_s2[4] = -2 * C->grid;

	for (i = 0, kase = 0; i < 5; i++) {
		for (j = 0; j < 5; j++, kase++) {
			C->offset[kase][0] = add_n2[j];
			C->offset[kase][1] = add_n[j] + add_w[i];
			C->offset[kase][2] = add_n[j];
			C->offset[kase][3] = add_n[j] + add_e[i];
			C->offset[kase][4] = add_w2[i];
			C->offset[kase][5] = add_w[i];
			C->offset[kase][6] = add_e[i];
			C->offset[kase][7] = add_e2[i];
			C->offset[kase][8] = add_s[j] + add_w[i];
			C->offset[kase][9] = add_s[j];
			C->offset[kase][10] = add_s[j] + add_e[i];
			C->offset[kase][11] = add_s2[j];
		}
	}
}

GMT_LOCAL void fill_in_forecast (struct SURFACE_INFO *C) {

	/* Fills in bilinear estimates into new node locations
	   after grid is divided.   These new nodes are marked as
	   unconstrained while the coarser data are considered
	   constraints in the next iteration.
	 */

	uint64_t index_0, index_1, index_2, index_3, index_new;
	int ii, jj, i, j;
	unsigned char *iu = C->iu;
	double delta_x, delta_y, a0, a1, a2, a3, old_size, a0_plus_a1_dx, a2_plus_a3_dx;
	gmt_grdfloat *u = C->Grid->data;

	old_size = 1.0 / (double)C->old_grid;

	/* First do from southwest corner */

	for (i = 0; i < (C->n_columns-1); i += C->old_grid) {

		for (j = 0; j < (C->n_rows-1); j += C->old_grid) {

			/* Get indices of bilinear square */
			index_0 = C->ij_sw_corner + i * C->my + j;
			index_1 = index_0 + C->old_grid * C->my;
			index_2 = index_1 + C->old_grid;
			index_3 = index_0 + C->old_grid;

			/* Get coefficients */
			a0 = u[index_0];
			a1 = u[index_1] - a0;
			a2 = u[index_3] - a0;
			a3 = u[index_2] - a0 - a1 - a2;

			/* Find all possible new fill ins */

			for (ii = i;  ii < (i + C->old_grid); ii += C->grid) {
				delta_x = (ii - i) * old_size;
				a0_plus_a1_dx = a0 + a1 * delta_x;
				a2_plus_a3_dx = a2 + a3 * delta_x;
				for (jj = j;  jj < (j + C->old_grid); jj += C->grid) {
					index_new = C->ij_sw_corner + ii * C->my + jj;
					if (index_new == index_0) continue;
					delta_y = (jj - j) * old_size;
					u[index_new] = (gmt_grdfloat)(a0_plus_a1_dx + delta_y * a2_plus_a3_dx);
					iu[index_new] = SURFACE_IS_UNCONSTRAINED;
				}
			}
			iu[index_0] = SURFACE_IS_CONSTRAINED;
		}
	}

	/* Now do linear guess along east edge */

	for (j = 0; j < (C->n_rows-1); j += C->old_grid) {
		index_0 = C->ij_se_corner + j;
		index_3 = index_0 + C->old_grid;
		for (jj = j;  jj < j + C->old_grid; jj += C->grid) {
			index_new = C->ij_se_corner + jj;
			delta_y = (jj - j) * old_size;
			u[index_new] = u[index_0] + (gmt_grdfloat)(delta_y * (u[index_3] - u[index_0]));
			iu[index_new] = SURFACE_IS_UNCONSTRAINED;
		}
		iu[index_0] = SURFACE_IS_CONSTRAINED;
	}
	/* Now do linear guess along north edge */
	for (i = 0; i < (C->n_columns-1); i += C->old_grid) {
		index_0 = C->ij_nw_corner + i * C->my;
		index_1 = index_0 + C->old_grid * C->my;
		for (ii = i;  ii < i + C->old_grid; ii += C->grid) {
			index_new = C->ij_nw_corner + ii * C->my;
			delta_x = (ii - i) * old_size;
			u[index_new] = u[index_0] + (gmt_grdfloat)(delta_x * (u[index_1] - u[index_0]));
			iu[index_new] = SURFACE_IS_UNCONSTRAINED;
		}
		iu[index_0] = SURFACE_IS_CONSTRAINED;
	}
	/* Now set northeast corner to fixed and we're done */
	iu[C->ij_ne_corner] = SURFACE_IS_CONSTRAINED;
}

GMT_LOCAL int compare_points (const void *point_1v, const void *point_2v) {
		/*  Routine for qsort to sort data structure for fast access to data by node location.
		    Sorts on index first, then on radius to node corresponding to index, so that index
		    goes from low to high, and so does radius.
		*/
	uint64_t block_i, block_j, index_1, index_2;
	double x0, y0, dist_1, dist_2;
	const struct SURFACE_DATA *point_1 = point_1v, *point_2 = point_2v;

	index_1 = point_1->index;
	index_2 = point_2->index;
	if (index_1 < index_2) return (-1);
	if (index_1 > index_2) return (1);
	if (index_1 == SURFACE_OUTSIDE) return (0);
	/* Points are in same grid cell, find the one who is nearest to grid point */
	block_i = point_1->index/GMT_Surface_Global.block_ny;
	block_j = point_1->index%GMT_Surface_Global.block_ny;
	x0 = GMT_Surface_Global.x_min + block_i * GMT_Surface_Global.grid_xinc;
	y0 = GMT_Surface_Global.y_min + block_j * GMT_Surface_Global.grid_yinc;
	dist_1 = (point_1->x - x0) * (point_1->x - x0) + (point_1->y - y0) * (point_1->y - y0);
	dist_2 = (point_2->x - x0) * (point_2->x - x0) + (point_2->y - y0) * (point_2->y - y0);
	if (dist_1 < dist_2) return (-1);
	if (dist_1 > dist_2) return (1);
	return (0);
}

GMT_LOCAL void smart_divide (struct SURFACE_INFO *C) {
	/* Divide grid by its largest prime factor */
	C->grid /= C->factors[C->n_fact - 1];
	C->n_fact--;
}

GMT_LOCAL void set_index (struct SURFACE_INFO *C) {
	/* recomputes data[k].index for new value of grid,
	   sorts data on index and radii, and throws away
	   data which are now outside the usable limits. */
	int i, j;
	uint64_t k, k_skipped = 0;
	struct GMT_GRID_HEADER *h = C->Grid->header;

	for (k = 0; k < C->npoints; k++) {
		i = irint (floor(((C->data[k].x-h->wesn[XLO])*C->r_grid_xinc) + 0.5));
		j = irint (floor(((C->data[k].y-h->wesn[YLO])*C->r_grid_yinc) + 0.5));
		if (i < 0 || i >= C->block_nx || j < 0 || j >= C->block_ny) {
			C->data[k].index = SURFACE_OUTSIDE;
			k_skipped++;
		}
		else
			C->data[k].index = i * C->block_ny + j;
	}

	qsort (C->data, C->npoints, sizeof (struct SURFACE_DATA), compare_points);

	C->npoints -= k_skipped;

}

GMT_LOCAL void find_nearest_point (struct SURFACE_INFO *C) {
	/* Determines the nearest data point epr bin and sets the
	 * Briggs parameters or, if really close, sets the node value */
	uint64_t ij_v2, k, last_index, iu_index, briggs_index;
	int i, j, block_i, block_j;
	double x0, y0, dx, dy, dxpdy, xys, xy1, btemp, *b = NULL;
	gmt_grdfloat z_at_node, *u = C->Grid->data;
	unsigned char *iu = C->iu;
	struct GMT_GRID_HEADER *h = C->Grid->header;

	last_index = UINTMAX_MAX;

	for (i = 0; i < C->n_columns; i += C->grid)	/* Reset grid info */
		for (j = 0; j < C->n_rows; j += C->grid)
			iu[C->ij_sw_corner + i*C->my + j] = SURFACE_IS_UNCONSTRAINED;

	briggs_index = 0;
	for (k = 0; k < C->npoints; k++) {	/* Find constraining value  */
		if (C->data[k].index != last_index) {
			block_i = (int)C->data[k].index/C->block_ny;
			block_j = (int)C->data[k].index%C->block_ny;
			last_index = C->data[k].index;
	 		iu_index = C->ij_sw_corner + (block_i * C->my + block_j) * C->grid;
	 		x0 = h->wesn[XLO] + block_i*C->grid_xinc;
	 		y0 = h->wesn[YLO] + block_j*C->grid_yinc;
	 		dx = (C->data[k].x - x0)*C->r_grid_xinc;
	 		dy = (C->data[k].y - y0)*C->r_grid_yinc;
	 		if (fabs(dx) < SURFACE_CLOSENESS_FACTOR && fabs(dy) < SURFACE_CLOSENESS_FACTOR) {	/* Close enough to assign value to node */
	 			iu[iu_index] = SURFACE_IS_CONSTRAINED;
	 			/* v3.3.4: NEW CODE
	 			 * Since point is basically moved from (dx, dy) to (0,0) we must adjust for
	 			 * the small change in the planar trend between the two locations, and then
	 			 * possibly clip the range if constraining surfaces were given.  Note that
	 			 * dx, dy is in -1/1 range normalized by (grid * x|y_inc) so to recover the
	 			 * dx,dy in final grid fractions we must scale by grid */

	 			z_at_node = C->data[k].z + (gmt_grdfloat) (C->r_z_scale * C->grid * (C->plane_c1 * dx + C->plane_c2 * dy));
	 			if (C->constrained) {	/* Must use ij_v2 since constrained grids are in standard scanline format */
					ij_v2 = gmt_M_ijp (C->Grid->header, C->n_rows - block_j * C->grid - 1, block_i * C->grid);
					if (C->set_low  && !gmt_M_is_fnan (C->Low->data[ij_v2]) && z_at_node < C->Low->data[ij_v2])
						z_at_node = C->Low->data[ij_v2];
					else if (C->set_high && !gmt_M_is_fnan (C->High->data[ij_v2]) && z_at_node > C->High->data[ij_v2])
						z_at_node = C->High->data[ij_v2];
	 			}
	 			u[iu_index] = z_at_node;
	 		}
	 		else {	/* We have a nearby data point in one of the quadrants */
	 			if (dx >= 0.0) {
	 				if (dy >= 0.0)
	 					iu[iu_index] = SURFACE_DATA_IS_IN_QUAD1;
	 				else {
						iu[iu_index] = SURFACE_DATA_IS_IN_QUAD4;
						//dy = -dy;	gmt_M_double_swap (dx, dy);
					}
	 			}
	 			else {
					//dx = -dx;
	 				if (dy >= 0.0) {
	 					iu[iu_index] = SURFACE_DATA_IS_IN_QUAD2;
						//gmt_M_double_swap (dx, dy);
					}
	 				else {
						iu[iu_index] = SURFACE_DATA_IS_IN_QUAD3;
						//dy = -dy;
					}
	 			}
				/* Evaluate the Briggs coefficients */
	 			dx = fabs (dx);
	 			dy = fabs (dy);
				dxpdy = dx + dy;
	 			xys = 1.0 + dxpdy;
	 			btemp = 2 * C->one_plus_e2 / ( dxpdy * xys );
	 			b = C->briggs[briggs_index].b;	/* Shorthand to this Briggs-array */
	 			b[0] = 1.0 - 0.5 * (dx + (dx * dx)) * btemp;
	 			b[3] = 0.5 * (C->e_2 - (dy + (dy * dy)) * btemp);
	 			xy1 = 1.0 / xys;
	 			b[1] = (C->e_2 * xys - 4 * dy) * xy1;
	 			b[2] = 2 * (dy - dx + 1.0) * xy1;
	 			b[4] = b[0] + b[1] + b[2] + b[3] + btemp;
	 			b[5] = btemp * C->data[k].z;
				b[4] = 1.0 / (C->a0_const_1 + C->a0_const_2 * b[4]);	/* Do this calculation here instead of inside iterate loop */
	 			briggs_index++;
	 		}
	 	}
	 }
}

GMT_LOCAL void set_grid_parameters (struct SURFACE_INFO *C) {
	/* Updates the grid space parameters given the new C->grid setting */
	GMT_Surface_Global.block_ny = C->block_ny = (C->n_rows - 1) / C->grid + 1;
	C->block_nx = (C->n_columns - 1) / C->grid + 1;
	GMT_Surface_Global.grid_xinc = C->grid_xinc = C->grid * C->Grid->header->inc[GMT_X];
	GMT_Surface_Global.grid_yinc = C->grid_yinc = C->grid * C->Grid->header->inc[GMT_Y];
	C->grid_east = C->grid * C->my;
	C->r_grid_xinc = 1.0 / C->grid_xinc;
	C->r_grid_yinc = 1.0 / C->grid_yinc;
}

GMT_LOCAL void initialize_grid (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/*
	 * For the initial gridsize, compute weighted averages of data inside the search radius
	 * and assign the values to u[i,j] where i,j are multiples of gridsize.
	 */
	uint64_t index_1, index_2, k, k_index;
	int irad, jrad, i, j, imin, imax, jmin, jmax, ki, kj;
	double r, rfact, sum_w, sum_zw, weight, x0, y0;
	gmt_grdfloat *u = C->Grid->data;
	struct GMT_GRID_HEADER *h = C->Grid->header;

	 irad = irint (ceil(C->radius/C->grid_xinc));
	 jrad = irint (ceil(C->radius/C->grid_yinc));
	 rfact = -4.5/(C->radius*C->radius);
	 for (i = 0; i < C->block_nx; i ++ ) {
	 	x0 = h->wesn[XLO] + i*C->grid_xinc;
	 	for (j = 0; j < C->block_ny; j ++ ) {
	 		y0 = h->wesn[YLO] + j*C->grid_yinc;
	 		imin = i - irad;
	 		if (imin < 0) imin = 0;
	 		imax = i + irad;
	 		if (imax >= C->block_nx) imax = C->block_nx - 1;
	 		jmin = j - jrad;
	 		if (jmin < 0) jmin = 0;
	 		jmax = j + jrad;
	 		if (jmax >= C->block_ny) jmax = C->block_ny - 1;
	 		index_1 = (uint64_t)imin*C->block_ny + jmin;
	 		index_2 = (uint64_t)imax*C->block_ny + jmax + 1;
	 		sum_w = sum_zw = 0.0;
	 		k = 0;
	 		while (k < C->npoints && C->data[k].index < index_1) k++;
	 		for (ki = imin; k < C->npoints && ki <= imax && C->data[k].index < index_2; ki++) {
	 			for (kj = jmin; k < C->npoints && kj <= jmax && C->data[k].index < index_2; kj++) {
	 				k_index = (uint64_t)ki*C->block_ny + kj;
	 				while (k < C->npoints && C->data[k].index < k_index) k++;
	 				while (k < C->npoints && C->data[k].index == k_index) {
	 					r = (C->data[k].x-x0)*(C->data[k].x-x0) + (C->data[k].y-y0)*(C->data[k].y-y0);
	 					weight = exp (rfact*r);
	 					sum_w += weight;
	 					sum_zw += weight*C->data[k].z;
	 					k++;
	 				}
	 			}
	 		}
	 		if (sum_w == 0.0) {
	 			sprintf (C->format, "No data inside search radius at: %s %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	 			GMT_Report (GMT->parent, GMT_MSG_ERROR, C->format, x0, y0);
	 			u[C->ij_sw_corner + (i * C->my + j) * C->grid] = (gmt_grdfloat)C->z_mean;
	 		}
	 		else {
	 			u[C->ij_sw_corner + (i*C->my+j)*C->grid] = (gmt_grdfloat)(sum_zw/sum_w);
	 		}
		}
	}
}

GMT_LOCAL int read_data_surface (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, struct GMT_OPTION *options) {
	/* Procdss input data into data structure */
	int i, j, error;
	uint64_t k, kmax = 0, kmin = 0, n_dup = 0;
	double *in, half_dx, zmin = DBL_MAX, zmax = -DBL_MAX, wesn_lim[4];
	struct GMT_GRID_HEADER *h = C->Grid->header;
	struct GMT_RECORD *In = NULL;

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Processing input table data\n");
	C->data = gmt_M_memory (GMT, NULL, C->n_alloc, struct SURFACE_DATA);

	/* Read in xyz data and computes index no and store it in a structure */

	if ((error = GMT_Set_Columns (GMT->parent, GMT_IN, 3, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
		return (error);
	}
	if (GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		return (GMT->parent->error);
	}

	k = 0;
	C->z_mean = 0.0;
	/* Initially allow points to be within 1 grid spacing of the grid */
	wesn_lim[XLO] = h->wesn[XLO] - C->grid_xinc;	wesn_lim[XHI] = h->wesn[XHI] + C->grid_xinc;
	wesn_lim[YLO] = h->wesn[YLO] - C->grid_yinc;	wesn_lim[YHI] = h->wesn[YHI] + C->grid_yinc;
	half_dx = 0.5 * C->grid_xinc;

	if (GMT_Begin_IO (GMT->parent, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
		return (GMT->parent->error);
	}
	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (GMT->parent, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
				return (GMT_RUNTIME_ERROR);
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			continue;	/* Go back and read the next record */
		}

		/* Data record to process */
		in = In->data;	/* Only need to process numerical part here */

		if (gmt_M_is_dnan (in[GMT_Z])) continue;
		if (gmt_M_y_is_outside (GMT, in[GMT_Y], wesn_lim[YLO], wesn_lim[YHI])) continue;	/* Outside y-range */
		if (gmt_x_is_outside (GMT, &in[GMT_X], wesn_lim[XLO], wesn_lim[XHI])) continue;	/* Outside x-range (or longitude) */
		if (C->periodic && (h->wesn[XHI]-in[GMT_X] < half_dx)) {	/* Push all values to the western nodes */
			in[GMT_X] -= 360.0;	/* Make this point constraining the western node value and then duplicate later */
			i = 0;
		}
		else
			i = irint (floor(((in[GMT_X]-h->wesn[XLO])*C->r_grid_xinc) + 0.5));
		if (i < 0 || i >= C->block_nx) continue;
		j = irint (floor(((in[GMT_Y]-h->wesn[YLO])*C->r_grid_yinc) + 0.5));
		if (j < 0 || j >= C->block_ny) continue;

		C->data[k].index = i * C->block_ny + j;
#ifdef DEBUG
		C->data[k].number = k + 1;
#endif
		C->data[k].x = (gmt_grdfloat)in[GMT_X];
		C->data[k].y = (gmt_grdfloat)in[GMT_Y];
		C->data[k].z = (gmt_grdfloat)in[GMT_Z];
		if (zmin > in[GMT_Z]) zmin = in[GMT_Z], kmin = k;
		if (zmax < in[GMT_Z]) zmax = in[GMT_Z], kmax = k;
		k++;
		C->z_mean += in[GMT_Z];
		if (k == C->n_alloc) {
			C->n_alloc <<= 1;
			C->data = gmt_M_memory (GMT, C->data, C->n_alloc, struct SURFACE_DATA);
		}
		if (C->periodic && i == 0) {	/* Replicate information to eastern boundary */
			i = C->block_nx - 1;
			C->data[k].index = i * C->block_ny + j;
#ifdef DEBUG
			C->data[k].number = k + 1;
#endif
			C->data[k].x = (gmt_grdfloat)(in[GMT_X] + 360.0);
			C->data[k].y = (gmt_grdfloat)in[GMT_Y];
			C->data[k].z = (gmt_grdfloat)in[GMT_Z];
			if (zmin > in[GMT_Z]) zmin = in[GMT_Z], kmin = k;
			if (zmax < in[GMT_Z]) zmax = in[GMT_Z], kmax = k;
			k++;
			C->z_mean += in[GMT_Z];
			if (k == C->n_alloc) {
				C->n_alloc <<= 1;
				C->data = gmt_M_memory (GMT, C->data, C->n_alloc, struct SURFACE_DATA);
			}
			n_dup++;
		}
	} while (true);


	if (GMT_End_IO (GMT->parent, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		return (GMT->parent->error);
	}

	C->npoints = k;

	if (C->npoints == 0) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, " No datapoints inside region, aborts\n");
		return (GMT_RUNTIME_ERROR);
	}

	C->z_mean /= k;
	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) {
		sprintf (C->format, "%s %s %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Minimum value of your dataset x,y,z at: ");
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, C->format, (double)C->data[kmin].x, (double)C->data[kmin].y, (double)C->data[kmin].z);
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Maximum value of your dataset x,y,z at: ");
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, C->format, (double)C->data[kmax].x, (double)C->data[kmax].y, (double)C->data[kmax].z);
		if (C->periodic && n_dup) GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Number of input values shared between repeating west and east column nodes: %" PRIu64 "\n", n_dup);
	}
	C->data = gmt_M_memory (GMT, C->data, C->npoints, struct SURFACE_DATA);

	if (C->set_low == 1)
		C->low_limit = C->data[kmin].z;
	else if (C->set_low == 2 && C->low_limit > C->data[kmin].z) {
	/*	C->low_limit = data[kmin].z;	*/
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Your lower value is > than min data value.\n");
	}
	if (C->set_high == 1)
		C->high_limit = C->data[kmax].z;
	else if (C->set_high == 2 && C->high_limit < C->data[kmax].z) {
	/*	C->high_limit = data[kmax].z;	*/
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Your upper value is < than max data value.\n");
	}
	return (0);
}

GMT_LOCAL int load_constraints (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, int transform) {
	/* Deal with the constants or grids supplied via -L */
	unsigned int i, j;
	uint64_t ij;
	double yy;
	struct GMTAPI_CTRL *API = GMT->parent;

	/* Load lower/upper limits, verify range, deplane, and rescale */

	if (C->set_low > 0) {
		if (C->set_low < 3) {
			if ((C->Low = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, C->Grid)) == NULL) return (API->error);
			for (ij = 0; ij < C->mxmy; ij++) C->Low->data[ij] = (gmt_grdfloat)C->low_limit;
		}
		else {
			if ((C->Low = GMT_Read_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, C->low_file, NULL)) == NULL) return (API->error);	/* Get header only */
			if (C->Low->header->n_columns != C->Grid->header->n_columns || C->Low->header->n_rows != C->Grid->header->n_rows) {
				GMT_Report (API, GMT_MSG_ERROR, "Lower limit file not of proper dimension!\n");
				return (GMT_RUNTIME_ERROR);
			}
			if (GMT_Read_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, C->low_file, C->Low) == NULL) return (API->error);
		}
		if (transform) {
			for (j = 0; j < C->Grid->header->n_rows; j++) {
				yy = (double)(C->Grid->header->n_rows - j - 1);
				for (i = 0; i < C->Grid->header->n_columns; i++) {
					ij = gmt_M_ijp (C->Grid->header, j, i);
					if (gmt_M_is_fnan (C->Low->data[ij])) continue;
					C->Low->data[ij] -= (gmt_grdfloat)(C->plane_c0 + C->plane_c1 * i + C->plane_c2 * yy);
					C->Low->data[ij] *= (gmt_grdfloat)C->r_z_scale;
				}
			}
		}
		C->constrained = true;
	}
	if (C->set_high > 0) {
		if (C->set_high < 3) {
			if ((C->High = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_ALLOC, C->Grid)) == NULL) return (API->error);
			for (ij = 0; ij < C->mxmy; ij++) C->High->data[ij] = (gmt_grdfloat)C->high_limit;
		}
		else {
			if ((C->High = GMT_Read_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, C->high_file, NULL)) == NULL) return (API->error);	/* Get header only */
			if (C->High->header->n_columns != C->Grid->header->n_columns || C->High->header->n_rows != C->Grid->header->n_rows) {
				GMT_Report (API, GMT_MSG_ERROR, "Upper limit file not of proper dimension!\n");
				return (GMT_RUNTIME_ERROR);
			}
			if (GMT_Read_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, C->high_file, C->High) == NULL) return (API->error);
		}
		if (transform) {
			for (j = 0; j < C->Grid->header->n_rows; j++) {
				yy = (double)(C->n_rows - j - 1);
				for (i = 0; i < C->Grid->header->n_columns; i++) {
					ij = gmt_M_ijp (C->Grid->header, j, i);
					if (gmt_M_is_fnan (C->High->data[ij])) continue;
					C->High->data[ij] -= (gmt_grdfloat)(C->plane_c0 + C->plane_c1 * i + C->plane_c2 * yy);
					C->High->data[ij] *= (gmt_grdfloat)C->r_z_scale;
				}
			}
		}
		C->constrained = true;
	}

	return (0);
}

GMT_LOCAL int write_output_surface (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, char *grdfile) {
	/* Uses v.2.0 netCDF grd format - hence need to transpose original column grid to be GMT compatible.  This will be rewritten, maybe */
	uint64_t index, k;
	int i, j, err;
	gmt_grdfloat *u = C->Grid->data, *v2 = NULL;

	if ((err = load_constraints (GMT, C, false)) != 0) return (err);	/* Reload constraints but this time do not transform data */

	strcpy (C->Grid->header->title, "Data gridded with continuous surface splines in tension");

	v2 = gmt_M_memory_aligned (GMT, NULL, C->Grid->header->size, gmt_grdfloat);
	index = C->ij_sw_corner;
	if (GMT->common.R.active[GSET]) {	/* Pixel registration request. Reset limits to the original extents */
		gmt_M_memcpy (C->Grid->header->wesn, C->wesn_orig, 4, double);
		C->Grid->header->registration = GMT->common.R.registration;
		/* Must reduce n_columns,n_rows by 1 to exclude the extra padding for pixel grids */
		C->Grid->header->n_columns--;	C->n_columns--;
		C->Grid->header->n_rows--;	C->n_rows--;
		gmt_set_grddim (GMT, C->Grid->header);	/* Reset all integer dimensions and xy_off */
	}
	for (i = 0; i < C->n_columns; i++, index += C->my) {
		for (j = 0; j < C->n_rows; j++) {
			k = gmt_M_ijp (C->Grid->header, j, i);
			v2[k] = u[index + C->n_rows - j - 1];
			if (C->set_low  && !gmt_M_is_fnan (C->Low->data[k])  && v2[k] < C->Low->data[k]) v2[k]  = C->Low->data[k];
			if (C->set_high && !gmt_M_is_fnan (C->High->data[k]) && v2[k] > C->High->data[k]) v2[k] = C->High->data[k];
		}
	}
	if (C->periodic) {	/* Ensure periodicity of E-W boundaries */
		for (j = 0; j < C->n_rows; j++) {
			k = gmt_M_ijp (C->Grid->header, j, 0);
			v2[k] = v2[k+C->n_columns-1] = (gmt_grdfloat)(0.5 * (v2[k] + v2[k+C->n_columns-1]));	/* Set these to the same as their average */
		}
	}
	gmt_M_free_aligned (GMT, C->Grid->data);	/* Free original column-oriented grid */
	C->Grid->data = v2;			/* Hook in new scanline-oriented grid */
	if (GMT_Write_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, grdfile, C->Grid) != GMT_NOERROR) {
		return (GMT->parent->error);
	}
	if ((C->set_low  > 0 && C->set_low  < 3) && GMT_Destroy_Data (GMT->parent, &C->Low) != GMT_NOERROR) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to free C->Low\n");
	}
	if ((C->set_high > 0 && C->set_high < 3) && GMT_Destroy_Data (GMT->parent, &C->High) != GMT_NOERROR) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to free C->High\n");
	}
	return (0);
}

GMT_LOCAL uint64_t iterate (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, int mode) {
	/* Main finite difference solver */
	uint64_t ij, briggs_index, ij_v2, iteration_count = 0, ij_sw, ij_se;
	unsigned int current_max_iterations = C->max_iterations * C->grid;
	int i, j, k, kase;
	int x_case, y_case, x_w_case, x_e_case, y_s_case, y_n_case;
	unsigned char *iu = C->iu;

	double current_limit = C->converge_limit / C->grid;
	double change, max_change = 0.0, busum, sum_ij;
	double *b = NULL;
	gmt_grdfloat *u = C->Grid->data;

	double x_0_const = 4.0 * (1.0 - C->boundary_tension) / (2.0 - C->boundary_tension);
	double x_1_const = (3 * C->boundary_tension - 2.0) / (2.0 - C->boundary_tension);
	double y_denom = 2 * C->l_epsilon * (1.0 - C->boundary_tension) + C->boundary_tension;
	double y_0_const = 4 * C->l_epsilon * (1.0 - C->boundary_tension) / y_denom;
	double y_1_const = (C->boundary_tension - 2 * C->l_epsilon * (1.0 - C->boundary_tension) ) / y_denom;

	sprintf (C->format, "%%4ld\t%%c\t%%8" PRIu64 "\t%s\t%s\t%%10" PRIu64 "\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);

	do {
		briggs_index = 0;	/* Reset the constraint table stack pointer  */
		max_change = -1.0;

		/* Fill in auxiliary boundary values (in new way) */

		/* First set (1-T)d2[]/dn2 + Td[]/dn = 0 along edges */

		for (i = 0; i < C->n_columns; i += C->grid) {
			/* set BC on south side */
			ij = C->ij_sw_corner + i * C->my;
			/* u[ij - 1] = 2 * u[ij] - u[ij + grid];  */
			u[ij - 1] = (gmt_grdfloat)(y_0_const * u[ij] + y_1_const * u[ij + C->grid]);
			/* set BC on north side */
			ij = C->ij_nw_corner + i * C->my;
			/* u[ij + 1] = 2 * u[ij] - u[ij - grid];  */
			u[ij + 1] = (gmt_grdfloat)(y_0_const * u[ij] + y_1_const * u[ij - C->grid]);

		}
		if (C->periodic) {	/* Set periodic boundary conditions in longitude at west and east boundaries */
			for (j = 0; j < C->n_rows; j += C->grid) {
				ij_sw = C->ij_sw_corner + j;
				ij_se = C->ij_se_corner + j;
				u[ij_sw+C->offset[0][5]]  = u[ij_se+C->offset[20][5]];
				u[ij_se+C->offset[20][6]] = u[ij_sw+C->offset[0][6]];
				u[ij_se] = u[ij_sw] = 0.5f * (u[ij_se] + u[ij_sw]);	/* Set to average of east and west */
			}
		}
		else {	/* Regular natural BC */
			for (j = 0; j < C->n_rows; j += C->grid) {
				/* set BC on west side */
				ij = C->ij_sw_corner + j;
				/* u[ij - my] = 2 * u[ij] - u[ij + grid_east];  */
				u[ij - C->my] = (gmt_grdfloat)(x_1_const * u[ij + C->grid_east] + x_0_const * u[ij]);
				/* set BC on east side */
				ij = C->ij_se_corner + j;
				/* u[ij + my] = 2 * u[ij] - u[ij - grid_east];  */
				u[ij + C->my] = (gmt_grdfloat)(x_1_const * u[ij - C->grid_east] + x_0_const * u[ij]);
			}
		}

		/* Now set d2[]/dxdy = 0 at each of the 4 corners */

		ij = C->ij_sw_corner;
		u[ij - C->my - 1] = u[ij + C->grid_east - 1] + u[ij - C->my + C->grid] - u[ij + C->grid_east + C->grid];

		ij = C->ij_nw_corner;
		u[ij - C->my + 1] = u[ij + C->grid_east + 1] + u[ij - C->my - C->grid] - u[ij + C->grid_east - C->grid];

		ij = C->ij_se_corner;
		u[ij + C->my - 1] = u[ij - C->grid_east - 1] + u[ij + C->my + C->grid] - u[ij - C->grid_east + C->grid];

		ij = C->ij_ne_corner;
		u[ij + C->my + 1] = u[ij - C->grid_east + 1] + u[ij + C->my - C->grid] - u[ij - C->grid_east - C->grid];

		/* Now set (1-T)dC/dn + Tdu/dn = 0 at each edge */
		/* New experiment: only dC/dn = 0  */

		x_w_case = 0;
		x_e_case = C->block_nx - 1;
		for (i = 0; i < C->n_columns; i += C->grid, x_w_case++, x_e_case--) {

			if(x_w_case < 2)
				x_case = x_w_case;
			else if(x_e_case < 2)
				x_case = 4 - x_e_case;
			else
				x_case = 2;

			/* South side */
			kase = x_case * 5;
			ij = C->ij_sw_corner + i * C->my;
			u[ij + C->offset[kase][11]] =
				(gmt_grdfloat)(u[ij + C->offset[kase][0]] + C->eps_m2*(u[ij + C->offset[kase][1]] + u[ij + C->offset[kase][3]]
					- u[ij + C->offset[kase][8]] - u[ij + C->offset[kase][10]])
					+ C->two_plus_em2 * (u[ij + C->offset[kase][9]] - u[ij + C->offset[kase][2]]) );
				/*  + tense * C->eps_m2 * (u[ij + C->offset[kase][2]] - u[ij + C->offset[kase][9]]) / (1.0 - tense);  */
			/* North side */
			kase = x_case * 5 + 4;
			ij = C->ij_nw_corner + i * C->my;
			u[ij + C->offset[kase][0]] =
				-(gmt_grdfloat)(-u[ij + C->offset[kase][11]] + C->eps_m2 * (u[ij + C->offset[kase][1]] + u[ij + C->offset[kase][3]]
					- u[ij + C->offset[kase][8]] - u[ij + C->offset[kase][10]])
					+ C->two_plus_em2 * (u[ij + C->offset[kase][9]] - u[ij + C->offset[kase][2]]) );
				/*  - tense * C->eps_m2 * (u[ij + C->offset[kase][2]] - u[ij + C->offset[kase][9]]) / (1.0 - tense);  */
		}

		y_s_case = 0;
		y_n_case = C->block_ny - 1;
		for (j = 0; j < C->n_rows; j += C->grid, y_s_case++, y_n_case--) {

			if(y_s_case < 2)
				y_case = y_s_case;
			else if(y_n_case < 2)
				y_case = 4 - y_n_case;
			else
				y_case = 2;

			if (C->periodic) {	/* Set periodic boundary conditions in longitude */
				/* West side */
				kase = y_case;
				ij_sw = C->ij_sw_corner + j;
				ij_se = C->ij_se_corner + j;
				u[ij_sw+C->offset[kase][4]] = u[ij_se+C->offset[20+kase][4]];
				/* East side */
				kase = 20 + y_case;
				u[ij_se + C->offset[kase][7]] = u[ij_sw+C->offset[y_case][7]];
			}
			else {	/* Natural BCs */
				/* West side */
				kase = y_case;
				ij = C->ij_sw_corner + j;
				u[ij+C->offset[kase][4]] =
					u[ij + C->offset[kase][7]] + (gmt_grdfloat)(C->eps_p2 * (u[ij + C->offset[kase][3]] + u[ij + C->offset[kase][10]]
					-u[ij + C->offset[kase][1]] - u[ij + C->offset[kase][8]])
					+ C->two_plus_ep2 * (u[ij + C->offset[kase][5]] - u[ij + C->offset[kase][6]]));
					/*  + tense * (u[ij + C->offset[kase][6]] - u[ij + C->offset[kase][5]]) / (1.0 - tense);  */
				/* East side */
				kase = 20 + y_case;
				ij = C->ij_se_corner + j;
				u[ij + C->offset[kase][7]] =
					- (gmt_grdfloat)(-u[ij + C->offset[kase][4]] + C->eps_p2 * (u[ij + C->offset[kase][3]] + u[ij + C->offset[kase][10]]
					- u[ij + C->offset[kase][1]] - u[ij + C->offset[kase][8]])
					+ C->two_plus_ep2 * (u[ij + C->offset[kase][5]] - u[ij + C->offset[kase][6]]) );
					/*  - tense * (u[ij + C->offset[kase][6]] - u[ij + C->offset[kase][5]]) / (1.0 - tense);  */
			}
		}



		/* That's it for the boundary points.  Now loop over all data  */

		x_w_case = 0;
		x_e_case = C->block_nx - 1;
		for (i = 0; i < C->n_columns; i += C->grid, x_w_case++, x_e_case--) {

			if (x_w_case < 2)
				x_case = x_w_case;
			else if (x_e_case < 2)
				x_case = 4 - x_e_case;
			else
				x_case = 2;

			y_s_case = 0;
			y_n_case = C->block_ny - 1;

			ij = C->ij_sw_corner + i * C->my;

			for (j = 0; j < C->n_rows; j += C->grid, ij += C->grid, y_s_case++, y_n_case--) {

				if (iu[ij] == SURFACE_IS_CONSTRAINED) continue;	/* Point is fixed, nothing to do  */

				if (y_s_case < 2)
					y_case = y_s_case;
				else if (y_n_case < 2)
					y_case = 4 - y_n_case;
				else
					y_case = 2;

				kase = x_case * 5 + y_case;
				sum_ij = 0.0;

				if (iu[ij] == SURFACE_IS_UNCONSTRAINED) {		/* Point is unconstrained  */
					for (k = 0; k < 12; k++)
						sum_ij += (u[ij + C->offset[kase][k]] * C->coeff[0][k]);
				}
				else {				/* Point is constrained  */
					b = C->briggs[briggs_index].b;	/* Shorthand to this b-array */
					for (k = 0, busum = 0.0; k < 4; k++)
						busum += b[k] * u[ij + C->offset[kase][p[iu[ij]][k]]];
					for (k = 0; k < 12; k++)
						sum_ij += (u[ij + C->offset[kase][k]] * C->coeff[1][k]);
					sum_ij = (sum_ij + C->a0_const_2 * (busum + b[5])) * b[4];
					briggs_index++;
				}

				/* New relaxation here  */
				sum_ij = u[ij] * C->relax_old + sum_ij * C->relax_new;

				if (C->constrained) {	/* Must check limits.  Note lower/upper grids are in standard scanline format and need ij_v2! */
					ij_v2 = gmt_M_ijp (C->Grid->header, C->n_rows - j - 1, i);
					if (C->set_low && !gmt_M_is_fnan (C->Low->data[ij_v2]) && sum_ij < C->Low->data[ij_v2])
						sum_ij = C->Low->data[ij_v2];
					else if (C->set_high && !gmt_M_is_fnan (C->High->data[ij_v2]) && sum_ij > C->High->data[ij_v2])
						sum_ij = C->High->data[ij_v2];
				}

				change = fabs (sum_ij - u[ij]);
				u[ij] = (gmt_grdfloat)sum_ij;
				if (change > max_change) max_change = change;
			}
		}
		iteration_count++;
		C->total_iterations++;
		max_change *= C->z_scale;	/* Put max_change into z units  */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, C->format,
			C->grid, C->mode_type[mode], iteration_count, max_change, current_limit, C->total_iterations);

	} while (max_change > current_limit && iteration_count < current_max_iterations);

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, C->format,
		C->grid, C->mode_type[mode], iteration_count, max_change, current_limit, C->total_iterations);

	return (iteration_count);
}

GMT_LOCAL void check_errors (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* Compute misfits at data locations */

	int i, j, move_over[12];
	uint64_t ij, k;
	unsigned char *iu = C->iu;	/* move_over = C->offset[kase][12], but grid = 1 so move_over is easy  */

	double	x0, y0, dx, dy, mean_error, mean_squared_error, z_est, z_err, curvature, c;
	double	du_dx, du_dy, d2u_dx2, d2u_dxdy, d2u_dy2, d3u_dx3, d3u_dx2dy, d3u_dxdy2, d3u_dy3;

	double	x_0_const = 4.0 * (1.0 - C->boundary_tension) / (2.0 - C->boundary_tension);
	double	x_1_const = (3 * C->boundary_tension - 2.0) / (2.0 - C->boundary_tension);
	double	y_denom = 2 * C->l_epsilon * (1.0 - C->boundary_tension) + C->boundary_tension;
	double	y_0_const = 4 * C->l_epsilon * (1.0 - C->boundary_tension) / y_denom;
	double	y_1_const = (C->boundary_tension - 2 * C->l_epsilon * (1.0 - C->boundary_tension) ) / y_denom;
	gmt_grdfloat *u = C->Grid->data;
	struct GMT_GRID_HEADER *h = C->Grid->header;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (h);

	move_over[0] = 2;
	move_over[1] = 1 - C->my;
	move_over[2] = 1;
	move_over[3] = 1 + C->my;
	move_over[4] = -2 * C->my;
	move_over[5] = -C->my;
	move_over[6] = C->my;
	move_over[7] = 2 * C->my;
	move_over[8] = -1 - C->my;
	move_over[9] = -1;
	move_over[10] = -1 + C->my;
	move_over[11] = -2;

	mean_error = mean_squared_error = 0.0;

	/* First update the boundary values  */

	for (i = 0; i < C->n_columns; i ++) {
		ij = C->ij_sw_corner + i * C->my;
		u[ij - 1] = (gmt_grdfloat)(y_0_const * u[ij] + y_1_const * u[ij + 1]);
		ij = C->ij_nw_corner + i * C->my;
		u[ij + 1] = (gmt_grdfloat)(y_0_const * u[ij] + y_1_const * u[ij - 1]);
	}

	for (j = 0; j < C->n_rows; j ++) {
		ij = C->ij_sw_corner + j;
		u[ij - C->my] = (gmt_grdfloat)(x_1_const * u[ij + C->my] + x_0_const * u[ij]);
		ij = C->ij_se_corner + j;
		u[ij + C->my] = (gmt_grdfloat)(x_1_const * u[ij - C->my] + x_0_const * u[ij]);
	}

	ij = C->ij_sw_corner;
	u[ij - C->my - 1] = u[ij + C->my - 1] + u[ij - C->my + 1] - u[ij + C->my + 1];
	ij = C->ij_nw_corner;
	u[ij - C->my + 1] = u[ij + C->my + 1] + u[ij - C->my - 1] - u[ij + C->my - 1];
	ij = C->ij_se_corner;
	u[ij + C->my - 1] = u[ij - C->my - 1] + u[ij + C->my + 1] - u[ij - C->my + 1];
	ij = C->ij_ne_corner;
	u[ij + C->my + 1] = u[ij - C->my + 1] + u[ij + C->my - 1] - u[ij - C->my - 1];

	for (i = 0; i < C->n_columns; i ++) {

		ij = C->ij_sw_corner + i * C->my;
		u[ij + move_over[11]] =
			(gmt_grdfloat)(u[ij + move_over[0]] + C->eps_m2*(u[ij + move_over[1]] + u[ij + move_over[3]]
				- u[ij + move_over[8]] - u[ij + move_over[10]])
				+ C->two_plus_em2 * (u[ij + move_over[9]] - u[ij + move_over[2]]) );

		ij = C->ij_nw_corner + i * C->my;
		u[ij + move_over[0]] =
			-(gmt_grdfloat)(-u[ij + move_over[11]] + C->eps_m2 * (u[ij + move_over[1]] + u[ij + move_over[3]]
				- u[ij + move_over[8]] - u[ij + move_over[10]])
				+ C->two_plus_em2 * (u[ij + move_over[9]] - u[ij + move_over[2]]) );
	}

	for (j = 0; j < C->n_rows; j ++) {

		ij = C->ij_sw_corner + j;
		u[ij+move_over[4]] =
			u[ij + move_over[7]] + (gmt_grdfloat)(C->eps_p2 * (u[ij + move_over[3]] + u[ij + move_over[10]]
			-u[ij + move_over[1]] - u[ij + move_over[8]])
			+ C->two_plus_ep2 * (u[ij + move_over[5]] - u[ij + move_over[6]]));

		ij = C->ij_se_corner + j;
		u[ij + move_over[7]] =
			- (gmt_grdfloat)(-u[ij + move_over[4]] + C->eps_p2 * (u[ij + move_over[3]] + u[ij + move_over[10]]
			- u[ij + move_over[1]] - u[ij + move_over[8]])
			+ C->two_plus_ep2 * (u[ij + move_over[5]] - u[ij + move_over[6]]) );
	}

	/* That resets the boundary values.  Now we can test all data.
		Note that this loop checks all values, even though only nearest were used.  */

	for (k = 0; k < C->npoints; k++) {
		i = (int)C->data[k].index / C->n_rows;
		j = (int)C->data[k].index % C->n_rows;
	 	ij = C->ij_sw_corner + i * C->my + j;
	 	if (iu[ij] == SURFACE_IS_CONSTRAINED) continue;
	 	x0 = h->wesn[XLO] + i*h->inc[GMT_X];
	 	y0 = h->wesn[YLO] + j*h->inc[GMT_Y];
	 	dx = (C->data[k].x - x0)*HH->r_inc[GMT_X];
	 	dy = (C->data[k].y - y0)*HH->r_inc[GMT_Y];

	 	du_dx = 0.5 * (u[ij + move_over[6]] - u[ij + move_over[5]]);
	 	du_dy = 0.5 * (u[ij + move_over[2]] - u[ij + move_over[9]]);
	 	d2u_dx2 = u[ij + move_over[6]] + u[ij + move_over[5]] - 2 * u[ij];
	 	d2u_dy2 = u[ij + move_over[2]] + u[ij + move_over[9]] - 2 * u[ij];
	 	d2u_dxdy = 0.25 * (u[ij + move_over[3]] - u[ij + move_over[1]]
	 			- u[ij + move_over[10]] + u[ij + move_over[8]]);
	 	d3u_dx3 = 0.5 * ( u[ij + move_over[7]] - 2 * u[ij + move_over[6]]
	 				+ 2 * u[ij + move_over[5]] - u[ij + move_over[4]]);
	 	d3u_dy3 = 0.5 * ( u[ij + move_over[0]] - 2 * u[ij + move_over[2]]
	 				+ 2 * u[ij + move_over[9]] - u[ij + move_over[11]]);
	 	d3u_dx2dy = 0.5 * ( ( u[ij + move_over[3]] + u[ij + move_over[1]] - 2 * u[ij + move_over[2]] )
	 				- ( u[ij + move_over[10]] + u[ij + move_over[8]] - 2 * u[ij + move_over[9]] ) );
	 	d3u_dxdy2 = 0.5 * ( ( u[ij + move_over[3]] + u[ij + move_over[10]] - 2 * u[ij + move_over[6]] )
	 				- ( u[ij + move_over[1]] + u[ij + move_over[8]] - 2 * u[ij + move_over[5]] ) );

	 	/* 3rd order Taylor approx */

	 	z_est = u[ij] + dx * (du_dx +  dx * ( (0.5 * d2u_dx2) + dx * (d3u_dx3 / 6.0) ) )
				+ dy * (du_dy +  dy * ( (0.5 * d2u_dy2) + dy * (d3u_dy3 / 6.0) ) )
	 			+ dx * dy * (d2u_dxdy) + (0.5 * dx * d3u_dx2dy) + (0.5 * dy * d3u_dxdy2);

	 	z_err = z_est - C->data[k].z;
	 	mean_error += z_err;
	 	mean_squared_error += (z_err * z_err);
	 }
	 mean_error /= C->npoints;
	 mean_squared_error = sqrt (mean_squared_error / C->npoints);

	/* Compute the total curvature of the grid */

	 curvature = 0.0;
	 for (i = 0; i < C->n_columns; i++) {
	 	for (j = 0; j < C->n_rows; j++) {
	 		ij = C->ij_sw_corner + i * C->my + j;
	 		c = u[ij + move_over[6]] + u[ij + move_over[5]]
	 			+ u[ij + move_over[2]] + u[ij + move_over[9]] - 4.0 * u[ij + move_over[6]];
			curvature += (c * c);
		}
	}

	 GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Fit info: N data points  N nodes\tmean error\trms error\tcurvature\n");
	 sprintf (C->format,"\t%%8ld\t%%8ld\t%s\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	 GMT_Report (GMT->parent, GMT_MSG_INFORMATION, C->format, C->npoints, C->nxny, mean_error, mean_squared_error, curvature);
 }

GMT_LOCAL void remove_planar_trend (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* Fit LS plane and remove from data; we restore before output */
	uint64_t i;
	double a, b, c, d, xx, yy, zz;
	double sx, sy, sz, sxx, sxy, sxz, syy, syz;
	struct GMT_GRID_HEADER *h = C->Grid->header;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (h);

	sx = sy = sz = sxx = sxy = sxz = syy = syz = 0.0;

	for (i = 0; i < C->npoints; i++) {

		xx = (C->data[i].x - h->wesn[XLO]) * HH->r_inc[GMT_X];
		yy = (C->data[i].y - h->wesn[YLO]) * HH->r_inc[GMT_Y];
		zz = C->data[i].z;

		sx += xx;
		sy += yy;
		sz += zz;
		sxx += (xx * xx);
		sxy += (xx * yy);
		sxz += (xx * zz);
		syy += (yy * yy);
		syz += (yy * zz);
	}

	d = C->npoints*sxx*syy + 2*sx*sy*sxy - C->npoints*sxy*sxy - sx*sx*syy - sy*sy*sxx;

	if (d == 0.0) {
		C->plane_c0 = C->plane_c1 = C->plane_c2 = 0.0;
		return;
	}

	a = sz*sxx*syy + sx*sxy*syz + sy*sxy*sxz - sz*sxy*sxy - sx*sxz*syy - sy*syz*sxx;
	b = C->npoints*sxz*syy + sz*sy*sxy + sy*sx*syz - C->npoints*sxy*syz - sz*sx*syy - sy*sy*sxz;
	c = C->npoints*sxx*syz + sx*sy*sxz + sz*sx*sxy - C->npoints*sxy*sxz - sx*sx*syz - sz*sy*sxx;

	C->plane_c0 = a / d;
	C->plane_c1 = b / d;
	C->plane_c2 = c / d;
	if (C->periodic) C->plane_c1 = 0.0;	/* Cannot have x-trend for periodic geographic data */

	for (i = 0; i < C->npoints; i++) {
		xx = (C->data[i].x - h->wesn[XLO]) * HH->r_inc[GMT_X];
		yy = (C->data[i].y - h->wesn[YLO]) * HH->r_inc[GMT_Y];
		C->data[i].z -= (gmt_grdfloat)(C->plane_c0 + C->plane_c1 * xx + C->plane_c2 * yy);
	}

	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "LS plane determined: z = %g + (%g * col) + (%g * row)\n", C->plane_c0, C->plane_c1, C->plane_c2);
}

GMT_LOCAL void replace_planar_trend (struct SURFACE_INFO *C) {
	/* Restore the LS plan we removed */
	int i, j;
	uint64_t ij;
	gmt_grdfloat *u = C->Grid->data;

	 for (i = 0; i < C->n_columns; i++) {
	 	for (j = 0; j < C->n_rows; j++) {
	 		ij = C->ij_sw_corner + i * C->my + j;
	 		u[ij] = (gmt_grdfloat)((u[ij] * C->z_scale) + (C->plane_c0 + C->plane_c1 * i + C->plane_c2 * j));
		}
	}
}

GMT_LOCAL void throw_away_unusables (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* This is a new routine to eliminate data which will become
		unusable on the final iteration, when grid = 1.
		It assumes grid = 1 and set_grid_parameters has been
		called.  We sort, mark redundant data as SURFACE_OUTSIDE, and
		sort again, chopping off the excess.

		Experimental modification 5 Dec 1988 by Smith, as part
		of a new implementation using core memory for b[6]
		coefficients, eliminating calls to temp file.
	*/

	uint64_t last_index, n_outside, k;

	/* Sort the data  */

	qsort (C->data, C->npoints, sizeof (struct SURFACE_DATA), compare_points);

	/* If more than one datum is indexed to same node, only the first should be kept.
		Mark the additional ones as SURFACE_OUTSIDE
	*/
	last_index = UINTMAX_MAX;
	n_outside = 0;
	for (k = 0; k < C->npoints; k++) {
		if (C->data[k].index == last_index) {
			C->data[k].index = SURFACE_OUTSIDE;
#ifdef DEBUG
			GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Original point %" PRIu64 " will be ignored.\n", C->data[k].number);
#endif
			n_outside++;
		}
		else
			last_index = C->data[k].index;
	}

	if (n_outside) {	/* Sort again; this time the SURFACE_OUTSIDE points will be thrown away  */
		qsort (C->data, C->npoints, sizeof (struct SURFACE_DATA), compare_points);
		C->npoints -= n_outside;
		C->data = gmt_M_memory (GMT, C->data, C->npoints, struct SURFACE_DATA);
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "%" PRIu64 " unusable points were supplied; these will be ignored.\n", n_outside);
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "You should have pre-processed the data with block-mean, -median, or -mode.\n");
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Check that previous processing steps write results with enough decimals.\n");
	}
}

GMT_LOCAL int rescale_z_values (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {
	/* Find and normalize data by its rms */
	uint64_t i;
	double ssz = 0.0;

	for (i = 0; i < C->npoints; i++) ssz += (C->data[i].z * C->data[i].z);

	/* Set z_scale = rms(z) */

	C->z_scale = sqrt (ssz / C->npoints);

	if (C->z_scale < GMT_CONV8_LIMIT) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Input data lie exactly on a plane.\n");
		C->r_z_scale = C->z_scale = 1.0;
		return (1);	/* Flag to tell the main to just write out the plane */
	}
	else
		C->r_z_scale = 1.0 / C->z_scale;

	for (i = 0; i < C->npoints; i++) C->data[i].z *= (gmt_grdfloat)C->r_z_scale;

	if (C->converge_limit == 0.0 || C->converge_limit_mode == 1) {	/* Set default values */
		unsigned int ppm;
		double limit = (C->converge_limit_mode == 1) ? C->converge_limit : SURFACE_CONV_LIMIT;
		ppm = urint (limit / 1.0e-6);
		C->converge_limit = limit * C->z_scale; /* i.e., 100 ppm of L2 scale */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Select default convergence limit of %g (%u ppm of L2 scale)\n", C->converge_limit, ppm);
	}
	return (0);
}

GMT_LOCAL void suggest_sizes (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int factors[], unsigned int n_columns, unsigned int n_rows, bool pixel) {
	/* Calls gmt_optimal_dim_for_surface to determine if there are
	 * better choices for n_columns, n_rows that might speed up calculations
	 * by having many more common factors.
	 *
	 * W. H. F. Smith, 26 Feb 1992.  */

	unsigned int i;
	unsigned int n_sug = 0;	/* N of suggestions found  */
	struct GMT_SURFACE_SUGGESTION *sug = NULL;

	n_sug = gmt_optimal_dim_for_surface (GMT, factors, n_columns, n_rows, &sug);

	if (n_sug) {
		char region[GMT_LEN128] = {""}, buffer[GMT_LEN128] = {""};
		unsigned int m, save_range = GMT->current.io.geo.range;
		double w, e, s, n;
		GMT->current.io.geo.range = GMT_IS_GIVEN_RANGE;		/* Override this setting explicitly */
		for (i = 0; i < n_sug && i < 10; i++) {
			m = sug[i].n_columns - (G->header->n_columns - 1);	/* Additional nodes in x */
			w = G->header->wesn[XLO] - (m/2)*G->header->inc[GMT_X];
			e = G->header->wesn[XHI] + (m/2)*G->header->inc[GMT_X];
			if (m%2) e += G->header->inc[GMT_X];
			m = sug[i].n_rows - (G->header->n_rows - 1);	/* Additional nodes in y */
			s = G->header->wesn[YLO] - (m/2)*G->header->inc[GMT_Y];
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
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Hint: Choosing %s [n_columns = %d, n_rows = %d] might cut run time by a factor of %.8g\n",
				region, sug[i].n_columns, sug[i].n_rows, sug[i].factor);
		}
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Hint: After completion you can recover the desired region via gmt grdcut\n");
		gmt_M_free (GMT, sug);
		GMT->current.io.geo.range = save_range;
	}
	else
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot suggest any n_columns,n_rows better than your -R -I define.\n");
	return;
}

GMT_LOCAL void load_parameters_surface (struct SURFACE_INFO *C, struct SURFACE_CTRL *Ctrl) {
	/* Place program options into the surface struct.  This was done this way
	 * since surface.c relied heavily on global variables which are a no-no
	 * in GMT5.  The simplest solution was to collect all those variables into
	 * a single structure and pass a pointer to that structure to functions.
	 */
	if (Ctrl->S.active) {
		if (Ctrl->S.unit == 'm') Ctrl->S.radius /= 60.0;
		if (Ctrl->S.unit == 's') Ctrl->S.radius /= 3600.0;
	}
	C->radius = Ctrl->S.radius;
	C->relax_new = Ctrl->Z.value;
	C->max_iterations = Ctrl->N.value;
	C->radius = Ctrl->S.radius;
	C->low_file = Ctrl->L.low;
	C->high_file = Ctrl->L.high;
	C->set_low = Ctrl->L.lmode;
	C->low_limit = Ctrl->L.min;
	C->set_high = Ctrl->L.hmode;
	C->high_limit = Ctrl->L.max;
	C->boundary_tension = Ctrl->T.b_tension;
	C->interior_tension = Ctrl->T.i_tension;
	C->l_epsilon = Ctrl->A.value;
	C->converge_limit = Ctrl->C.value;
	C->converge_limit_mode = Ctrl->C.mode;
}

GMT_LOCAL void interp_breakline (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, struct GMT_DATATABLE *xyzline) {
	/* Add constraints from breaklines */

	uint64_t n_tot = 0, this_ini = 0, this_end = 0, n_int = 0;
	uint64_t k = 0, n, kmax = 0, kmin = 0, row, seg;
	int srow, scol;
	size_t n_alloc;
	double *x = NULL, *y = NULL, *z = NULL, dx, dy, dz, r_dx, r_dy, zmin = DBL_MAX, zmax = -DBL_MAX;

	n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;
	x = gmt_M_memory (GMT, NULL, n_alloc, double);
	y = gmt_M_memory (GMT, NULL, n_alloc, double);
	z = gmt_M_memory (GMT, NULL, n_alloc, double);

	r_dx = 1.0 / C->grid_xinc;
	r_dy = 1.0 / C->grid_yinc;
	for (seg = 0; seg < xyzline->n_segments; seg++) {
		for (row = 0; row < xyzline->segment[seg]->n_rows - 1; row++) {
			dx = xyzline->segment[seg]->data[GMT_X][row+1] - xyzline->segment[seg]->data[GMT_X][row];
			dy = xyzline->segment[seg]->data[GMT_Y][row+1] - xyzline->segment[seg]->data[GMT_Y][row];
			dz = xyzline->segment[seg]->data[GMT_Z][row+1] - xyzline->segment[seg]->data[GMT_Z][row];
			n_int = lrint (MAX (fabs(dx) * r_dx, fabs(dy) * r_dy ) ) + 1;
			this_end += n_int;

			if (n_alloc >= this_end) {
				n_alloc += MAX (GMT_CHUNK, n_int);
				x = gmt_M_memory (GMT, x, n_alloc, double);
				y = gmt_M_memory (GMT, y, n_alloc, double);
				z = gmt_M_memory (GMT, z, n_alloc, double);
			}

			dx /= (floor((double)n_int) - 1);
			dy /= (floor((double)n_int) - 1);
			dz /= (floor((double)n_int) - 1);
			for (k = this_ini, n = 0; k < this_end - 1; k++, n++) {
				x[k] = xyzline->segment[seg]->data[GMT_X][row] + n * dx;
				y[k] = xyzline->segment[seg]->data[GMT_Y][row] + n * dy;
				z[k] = xyzline->segment[seg]->data[GMT_Z][row] + n * dz;
			}
			x[this_end - 1] = xyzline->segment[seg]->data[GMT_X][row+1];
			y[this_end - 1] = xyzline->segment[seg]->data[GMT_Y][row+1];
			z[this_end - 1] = xyzline->segment[seg]->data[GMT_Z][row+1];

			this_ini += n_int;
		}

		n_tot += this_end;
	}

	/* Now add the interpolated breakline to the C structure */

	k = C->npoints;
	C->data = gmt_M_memory (GMT, C->data, k+n_tot, struct SURFACE_DATA);
	C->z_mean *= k;		/* It was already computed, reset it to sum */
	if (C->set_low == 1)
		zmin = C->low_limit;
	if (C->set_high == 1)
		zmax = C->high_limit;

	for (n = 0; n < n_tot; n++) {

		if (gmt_M_is_dnan (z[n])) continue;

		scol = irint (floor (((x[n] - C->Grid->header->wesn[XLO]) * C->r_grid_xinc) + 0.5));
		if (scol < 0 || scol >= C->block_nx) continue;
		srow = irint (floor (((y[n] - C->Grid->header->wesn[YLO]) * C->r_grid_yinc) + 0.5));
		if (srow < 0 || srow >= C->block_ny) continue;

		C->data[k].index = scol * C->block_ny + srow;
#ifdef DEBUG
		C->data[k].number = k + 1;
#endif
		C->data[k].x = (gmt_grdfloat)x[n];
		C->data[k].y = (gmt_grdfloat)y[n];
		C->data[k].z = (gmt_grdfloat)z[n];
		if (zmin > z[n]) zmin = z[n], kmin = k;
		if (zmax < z[n]) zmax = z[n], kmax = k;
		k++;
		C->z_mean += z[n];
	}

	if (k != (C->npoints + n_tot))		/* We had some NaNs */
		C->data = gmt_M_memory (GMT, C->data, k, struct SURFACE_DATA);

	C->npoints = k;
	C->z_mean /= k;

	if (C->set_low == 1)
		C->low_limit = C->data[kmin].z;
	if (C->set_high == 1)
		C->high_limit = C->data[kmax].z;

	gmt_M_free (GMT, x);
	gmt_M_free (GMT, y);
	gmt_M_free (GMT, z);
}

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SURFACE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct SURFACE_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
	C->N.value = SURFACE_MAX_ITERATIONS;
	C->A.value = 1.0;
	C->Z.value = SURFACE_OVERRELAXATION;

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct SURFACE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->D.file);
	gmt_M_str_free (C->L.low);
	gmt_M_str_free (C->L.high);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	unsigned int ppm;
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -G<outgrid> %s\n", name, GMT_I_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-A<aspect_ratio>|m] [-C<convergence_limit>[%%]]\n", GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-D<breakline>] [-Ll<limit>] [-Lu<limit>] [-N<n_iterations>] [-S<search_radius>[m|s]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T[i|b]<tension>] [-Q] [%s] [-Z<over_relaxation_parameter>]\n\t[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_bi_OPT, GMT_di_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_s_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);
	ppm = urint (SURFACE_CONV_LIMIT / 1e-6);

	GMT_Message (API, GMT_TIME_NONE, "\t-G sets output grid file name.\n");
	GMT_Option (API, "I,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Set aspect-ratio> [Default = 1 gives an isotropic solution],\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i.e., xinc and yinc assumed to give derivatives of equal weight; if not, specify\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <aspect_ratio> such that yinc = xinc / <aspect_ratio>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If gridding lon,lat use -Am to set <aspect_ratio> = cosine(middle of lat range).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Set final convergence limit; iteration stops when max |change| < <convergence_limit>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default will choose %g of the rms of your z data after removing L2 plane (%u ppm precision).\n", SURFACE_CONV_LIMIT, ppm);
	GMT_Message (API, GMT_TIME_NONE, "\t   Enter your own convergence limit in same units as your z data.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, append %% to set convergence limit in percentage of rms deviation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Use xyz data in the <breakline> file as a 'soft breakline'.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Constrain the range of output values:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Ll<limit> specifies lower limit; forces solution to be >= <limit>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Lu<limit> specifies upper limit; forces solution to be <= <limit>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <limit> can be any number, or the letter d for min (or max) input data value,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or the filename of a grid with bounding values.  [Default solution unconstrained].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Example: -Ll0 gives a non-negative solution.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set max <n_iterations> in the final cycle; default = %d.\n", SURFACE_MAX_ITERATIONS);
	GMT_Message (API, GMT_TIME_NONE, "\t-S Set <search_radius> to initialize grid; default = 0 will skip this step.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This step is slow and not needed unless grid dimensions are pathological;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i.e., have few or no common factors.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append m or s to give <search_radius> in minutes or seconds.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Add Tension to the gridding equation; use a value between 0 and 1.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   default = 0 gives minimum curvature (smoothest; bicubic) solution.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   1 gives a harmonic spline solution (local max/min occur only at data points).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   typically 0.25 or more is good for potential field (smooth) data;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   0.75 or so for topography.  Experiment.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend b to set tension in boundary conditions only;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend i to set tension in interior equations only;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   No appended letter sets tension for both to same value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Query for grid sizes that might run faster than your -R -I give.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Set <over_relaxation parameter>.  Default = %g.  Use a value\n", SURFACE_OVERRELAXATION);
	GMT_Message (API, GMT_TIME_NONE, "\t   between 1 and 2.  Larger number accelerates convergence but can be unstable.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use 1 if you want to be sure to have (slow) stable convergence.\n");
	GMT_Option (API, "a,bi3,di,e,f,h,i,r,s,:,.");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: Geographic data with 360-degree range use periodic boundary condition in longitude.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t(For additional details, see Smith & Wessel, Geophysics, 55, 293-305, 1990.)\n");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SURFACE_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to surface and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k;
	char modifier;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				if (opt->arg[0] == 'm')
					Ctrl->A.mode = 1;
				else
					Ctrl->A.value = atof (opt->arg);
				break;
			case 'C':
				Ctrl->C.active = true;
				Ctrl->C.value = atof (opt->arg);
				if (strchr (opt->arg, '%')) {	/* Gave convergence in percent */
					Ctrl->C.mode = 1;
					Ctrl->C.value *= 0.01;
				}
				break;
			case 'D':
				if ((Ctrl->D.active = gmt_check_filearg (GMT, 'D', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0)
					Ctrl->D.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'G':
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_OUT, GMT_IS_GRID)) != 0)
					Ctrl->G.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'I':
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'L':	/* Set limits */
				Ctrl->L.active = true;
				switch (opt->arg[0]) {
					case 'l':	/* Lower limit  */
						n_errors += gmt_M_check_condition (GMT, opt->arg[1] == 0, "Option -Ll: No argument given\n");
						Ctrl->L.low = strdup (&opt->arg[1]);
						if (!gmt_access (GMT, Ctrl->L.low, F_OK))	/* file exists */
							Ctrl->L.lmode = 3;
						else if (Ctrl->L.low[0] == 'd')		/* Use data minimum */
							Ctrl->L.lmode = 1;
						else {
							Ctrl->L.lmode = 2;		/* Use given value */
							Ctrl->L.min = atof (&opt->arg[1]);
						}
						break;
					case 'u':	/* Upper limit  */
						n_errors += gmt_M_check_condition (GMT, opt->arg[1] == 0, "Option -Lu: No argument given\n");
						Ctrl->L.high = strdup (&opt->arg[1]);
						if (!gmt_access (GMT, Ctrl->L.high, F_OK))	/* file exists */
							Ctrl->L.hmode = 3;
						else if (Ctrl->L.high[0] == 'd')	/* Use data maximum */
							Ctrl->L.hmode = 1;
						else {
							Ctrl->L.hmode = 2;		/* Use given value */
							Ctrl->L.max = atof (&opt->arg[1]);
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
					GMT_Report (API, GMT_MSG_COMPAT, "Unit c is deprecated; use s instead.\n");
					Ctrl->S.unit = 's';
				}
				if (!strchr ("sm ", Ctrl->S.unit)) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -S: Unrecognized unit %c\n", Ctrl->S.unit);
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
					GMT_Report (API, GMT_MSG_ERROR, "Option -T: Unrecognized modifier %c\n", modifier);
					n_errors++;
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

	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0, "Option -I: Must specify positive increment(s)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.value < 1, "Option -N: Max iterations must be nonzero\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.value < 1.0 || Ctrl->Z.value > 2.0, "Option -Z: Relaxation value must be 1 <= z <= 2\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->G.file, "Option -G: Must specify output file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.mode && gmt_M_is_cartesian (GMT, GMT_IN), "Option -G: Must specify output file\n");
	n_errors += gmt_check_binary_io (GMT, 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_surface_old (void *V_API, int mode, void *args) {
	int error = 0, key, one = 1;
	double wesn[4];

	struct GMT_DATATABLE *xyzline = NULL;
	struct GMT_DATASET *Lin = NULL;
	struct SURFACE_INFO C;
	struct SURFACE_CTRL *Ctrl = NULL;
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

	/*---------------------------- This is the surface main code ----------------------------*/

	gmt_M_memset (&C, 1, struct SURFACE_INFO);
	gmt_M_memset (&GMT_Surface_Global, 1, struct SURFACE_GLOBAL);
	C.n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;
	C.z_scale = C.r_z_scale = 1.0;
	C.mode_type[0] = 'I';
	C.mode_type[1] = 'D';	/* D means include data points when iterating */

	gmt_M_memcpy (C.wesn_orig, GMT->common.R.wesn, 4, double);	/* Save original region in case of -r */
	gmt_M_memcpy (wesn, GMT->common.R.wesn, 4, double);		/* Specified region */
	C.periodic = (gmt_M_is_geographic (GMT, GMT_IN) && gmt_M_360_range (wesn[XLO], wesn[XHI]));
	if (C.periodic && gmt_M_180_range (wesn[YLO], wesn[YHI])) {
		/* Trying to grid global geographic data - this is not something surface can do */
		GMT_Report (API, GMT_MSG_ERROR, "You are attempting to grid a global geographic data set, but surface cannot handle poles.\n");
		GMT_Report (API, GMT_MSG_ERROR, "It will do its best but it remains a Cartesian calculation which affects nodes near the poles.\n");
		GMT_Report (API, GMT_MSG_ERROR, "Because the grid is flaggged as geographic, the (repeated) pole values will be averaged upon writing to file.\n");
		GMT_Report (API, GMT_MSG_ERROR, "This may introduce a jump at either pole which will distort the grid near the poles.\n");
		GMT_Report (API, GMT_MSG_ERROR, "Consider spherical gridding instead with greenspline or sphinterpolate.\n");
	}
	if (GMT->common.R.active[GSET]) {		/* Pixel registration request. Use the trick of offsetting area by x_inc(y_inc) / 2 */
		/* Note that the grid remains node-registered and only gets tagged as pixel-registered upon writing the final grid to file */
		wesn[XLO] += GMT->common.R.inc[GMT_X] / 2.0;	wesn[XHI] += GMT->common.R.inc[GMT_X] / 2.0;
		wesn[YLO] += GMT->common.R.inc[GMT_Y] / 2.0;	wesn[YHI] += GMT->common.R.inc[GMT_Y] / 2.0;
		/* n_columns,n_rows remain the same for now but nodes are in "pixel" position.  Must reset to original wesn and reduce n_columns,n_rows by 1 when we write result */
	}
	if (Ctrl->A.mode) Ctrl->A.value = cosd (0.5 * (wesn[YLO] + wesn[YHI]));	/* Set cos of middle latitude as aspect ratio */

	if ((C.Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, wesn, NULL, \
		GMT_GRID_NODE_REG, GMT_NOTSET, NULL)) == NULL) Return (API->error);

	if (C.Grid->header->n_columns < 4 || C.Grid->header->n_rows < 4) {
		GMT_Report (API, GMT_MSG_ERROR, "Grid must have at least 4 nodes in each direction (you have %d by %d) - abort.\n", C.Grid->header->n_columns, C.Grid->header->n_rows);
		Return (GMT_RUNTIME_ERROR);
	}

	load_parameters_surface (&C, Ctrl);	/* Pass parameters from parsing control to surface INFO structure */

	C.relax_old = 1.0 - C.relax_new;

	C.n_columns = C.Grid->header->n_columns;
	C.n_rows = C.Grid->header->n_rows;
	C.nxny = C.Grid->header->nm;

	C.mx = C.Grid->header->mx;
	C.my = C.Grid->header->my;
	C.mxmy = C.Grid->header->size;
	GMT_Surface_Global.x_min = C.Grid->header->wesn[XLO];
	GMT_Surface_Global.y_min = C.Grid->header->wesn[YLO];

	/* New stuff here for v4.3: Check out the grid dimensions */
	C.grid = gmt_gcd_euclid (C.n_columns-1, C.n_rows-1);

	if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION) || Ctrl->Q.active) {
		sprintf (C.format, "Grid domain: W: %s E: %s S: %s N: %s n_columns: %%d n_rows: %%d [", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		(GMT->common.R.active[GSET]) ? strcat (C.format, "pixel registration]\n") : strcat (C.format, "gridline registration]\n");
		GMT_Report (API, GMT_MSG_ERROR, C.format, C.wesn_orig[XLO], C.wesn_orig[XHI], C.wesn_orig[YLO], C.wesn_orig[YHI], C.n_columns-one, C.n_rows-one);
	}
	if (C.grid == 1) GMT_Report (API, GMT_MSG_WARNING, "Your grid dimensions are mutually prime.  Convergence is very unlikely.\n");
	if ((C.grid == 1 && gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION)) || Ctrl->Q.active) suggest_sizes (GMT, C.Grid, C.factors, C.n_columns-1, C.n_rows-1, GMT->common.R.active[GSET]);
	if (Ctrl->Q.active) Return (GMT_NOERROR);

	/* New idea: set grid = 1, read data, setting index.  Then throw
		away data that can't be used in end game, constraining
		size of briggs->b[6] structure.  */

	C.grid = 1;
	set_grid_parameters (&C);
	if (read_data_surface (GMT, &C, options)) Return (GMT_RUNTIME_ERROR);
	if (Ctrl->D.active) {	/* Consider breakline dataset */
		if ((Lin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_READ_NORMAL, NULL, Ctrl->D.file, NULL)) == NULL)
			Return (API->error);
		if (Lin->n_columns < 2) {
			GMT_Report (API, GMT_MSG_ERROR, "Input file %s has %d column(s) but at least 2 are needed\n", Ctrl->D.file, (int)Lin->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}
		xyzline = Lin->table[0];			/* Can only be one table since we read a single file */

		interp_breakline (GMT, &C, xyzline);
	}
	throw_away_unusables (GMT, &C);
	remove_planar_trend (GMT, &C);
	key = rescale_z_values (GMT, &C);

	if (GMT_Set_Comment (API, GMT_IS_GRID, GMT_COMMENT_IS_OPTION | GMT_COMMENT_IS_COMMAND, options, C.Grid) != GMT_NOERROR) Return (API->error);
	if (key == 1) {	/* Data lies exactly on a plane; just return the plane grid */
		gmt_M_free (GMT, C.data);
		if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL,
			0, 0, C.Grid) == NULL) Return (API->error);	/* Don't bother with padding since no BCs will be applied */
		C.ij_sw_corner = 2 * C.my + 2;			/*  Corners of array of actual data  */
		replace_planar_trend (&C);
		if ((error = write_output_surface (GMT, &C, Ctrl->G.file)) != 0) Return (error);
		Return (GMT_NOERROR);
	}

	load_constraints (GMT, &C, true);

	/* Set up factors and reset grid to first value  */

	C.grid = gmt_gcd_euclid (C.n_columns-1, C.n_rows-1);
	C.n_fact = gmt_get_prime_factors (GMT, C.grid, C.factors);
	set_grid_parameters (&C);
	while (C.block_nx < 4 || C.block_ny < 4) {
		smart_divide (&C);
		set_grid_parameters (&C);
	}
	set_offset (&C);
	set_index (&C);

	/* Now the data are ready to go for the first iteration.  */

	/* Allocate more space  */

	C.briggs = gmt_M_memory (GMT, NULL, C.npoints, struct SURFACE_BRIGGS);
	C.iu = gmt_M_memory (GMT, NULL, C.mxmy, char);
	if (GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_DATA_ONLY, NULL, NULL, NULL, 0, 0, C.Grid) == NULL) Return (API->error);

	if (C.radius > 0) initialize_grid (GMT, &C); /* Fill in nodes with a weighted avg in a search radius  */

	GMT_Report (API, GMT_MSG_INFORMATION, "Grid\tMode\tIteration\tMax Change\tConv Limit\tTotal Iterations\n");

	set_coefficients (&C);

	C.old_grid = C.grid;
	find_nearest_point (&C);
	iterate (GMT, &C, 1);

	while (C.grid > 1) {
		smart_divide (&C);
		set_grid_parameters (&C);
		set_offset (&C);
		set_index (&C);
		fill_in_forecast (&C);
		iterate (GMT, &C, 0);
		C.old_grid = C.grid;
		find_nearest_point (&C);
		iterate (GMT, &C, 1);
	}

	if (gmt_M_is_verbose (GMT, GMT_MSG_WARNING)) check_errors (GMT, &C);

	replace_planar_trend (&C);

	gmt_M_free (GMT, C.data);
	gmt_M_free (GMT, C.briggs);
	gmt_M_free (GMT, C.iu);
	if ((C.set_low  > 0 && C.set_low < 3) && GMT_Destroy_Data (API, &C.Low) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to free C.Low\n");
	}
	if ((C.set_high > 0 && C.set_high < 3) && GMT_Destroy_Data (API, &C.High) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to free C.High\n");
	}

	if ((error = write_output_surface (GMT, &C, Ctrl->G.file)) != 0) Return (error);

	Return (GMT_NOERROR);
}
