/*--------------------------------------------------------------------
 *	$Id: surface_func.c,v 1.17 2011-05-16 21:23:11 guru Exp $
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
 * Version:	5 API
 * Tech Note:	1. surface uses old grid-structure (transpose of current GMT grid)
 *		   and must therefore waste time transposing grid before writing.
 *		2. No support for periodic boundary conditions.
 */

#include "gmt.h"

struct SURFACE_CTRL {
	struct A {	/* -A<aspect_ratio> */
		GMT_LONG active;
		double value;
	} A;
	struct C {	/* -C<converge_limit> */
		GMT_LONG active;
		double value;
	} C;
	struct D {	/* -D<line.xyz> */
		GMT_LONG active;
		char *file;	/* Name of file with breaklines */
	} D;
	struct G {	/* -G<file> */
		GMT_LONG active;
		char *file;
	} G;
	struct I {	/* -Idx[/dy] */
		GMT_LONG active;
		double inc[2];
	} I;
	struct L {	/* -Ll|u<limit> */
		GMT_LONG active;
		char *low, *high;
		double min, max;
		GMT_LONG lmode, hmode;
	} L;
	struct N {	/* -N<max_iterations> */
		GMT_LONG active;
		GMT_LONG value;
	} N;
	struct Q {	/* -Q */
		GMT_LONG active;
	} Q;
	struct S {	/* -S<radius>[m|c] */
		GMT_LONG active;
		double radius;
		char unit;
	} S;
	struct T {	/* -T<tension>[i][b] */
		GMT_LONG active;
		double b_tension, i_tension;
	} T;
	struct Z {	/* -Z<over_relaxation_parameter> */
		GMT_LONG active;
		double value;
	} Z;
};

#define SURFACE_OUTSIDE LONG_MAX	/* Index number indicating data is outside usable area */

struct SURFACE_DATA {	/* Data point and index to node it currently constrains  */
	float x;
	float y;
	float z;
	GMT_LONG index;
};

struct SURFACE_BRIGGS {		/* Coefficients in Taylor series for Laplacian(z) a la I. C. Briggs (1974)  */
	double b[6];
};

struct SURFACE_GLOBAL {		/* Things needed inside compare function must be global for now */
	GMT_LONG block_ny;		/* Number of nodes in y-dir for a given grid factor */
	double grid_xinc, grid_yinc;	/* size of each grid cell for a given grid factor */
	double x_min, y_min;		/* Lower left corner of grid */
} GMT_Surface_Global;

struct SURFACE_INFO {	/* Control structure for surface setup and execution */
	char *iu;			/* Pointer to grid info array */
	char mode_type[2];		/* D means include data points when iterating
					 * I means just interpolate from larger grid */
	char format[GMT_BUFSIZ];
	char *low_file, *high_file;	/* Pointers to grids with low and high limits, if selected */
	GMT_LONG grid, old_grid;	/* Node spacings  */
	GMT_LONG n_fact;		/* Number of factors in common (ny-1, nx-1) */
	GMT_LONG factors[32];		/* Array of common factors */
	GMT_LONG set_low;		/* 0 unconstrained,1 = by min data value, 2 = by user value */
	GMT_LONG set_high;		/* 0 unconstrained,1 = by max data value, 2 = by user value */
	GMT_LONG n_alloc;
	GMT_LONG npoints;		/* Number of data points */
	GMT_LONG ij_sw_corner, ij_se_corner,ij_nw_corner, ij_ne_corner;
	GMT_LONG n_empty;		/* No of unconstrained nodes at initialization  */
	GMT_LONG nx;			/* Number of nodes in x-dir. */
	GMT_LONG ny;			/* Number of nodes in y-dir. (Final grid) */
	GMT_LONG nxny;			/* Total number of grid nodes without boundaries  */
	GMT_LONG mx;
	GMT_LONG my;
	GMT_LONG mxmy;			/* Total number of grid nodes with boundaries  */
	GMT_LONG block_nx;		/* Number of nodes in x-dir for a given grid factor */
	GMT_LONG block_ny;		/* Number of nodes in y-dir for a given grid factor */
	GMT_LONG max_iterations;	/* Max iter per call to iterate */
	GMT_LONG total_iterations;
	GMT_LONG grid_east;
	GMT_LONG offset[25][12];	/* Indices of 12 nearby points in 25 cases of edge conditions  */
	GMT_LONG constrained;		/* TRUE if set_low or set_high is TRUE */
	float *lower, *upper;		/* arrays for minmax values, if set */
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
	double small;			/* Let data point coincide with node if distance < C->small */
	double coeff[2][12];		/* Coefficients for 12 nearby points, constrained and unconstrained  */
	double relax_old, relax_new;	/* Coefficients for relaxation factor to speed up convergence */
	double wesn_orig[4];		/* Original -R domain as we might have shifted it due to -r */
	struct SURFACE_DATA  *data;
	struct SURFACE_BRIGGS *briggs;
	struct GMT_GRID *Grid;			/* The final grid */
	struct GMT_GRID *Low, *High;		/* arrays for minmax values, if set */
};

struct SURFACE_SUGGESTION {	/* Used to find top ten list of faster grid dimensions  */
	GMT_LONG nx;
	GMT_LONG ny;
	double factor;	/* Speed up by a factor of factor  */
};

void set_coefficients (struct SURFACE_INFO *C)
{
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
	C->ij_se_corner = C->ij_sw_corner + (C->nx - 1) * C->my;
	C->ij_nw_corner = C->ij_sw_corner + C->ny - 1;
	C->ij_ne_corner = C->ij_se_corner + C->ny - 1;

}

void set_offset (struct SURFACE_INFO *C)
{
	GMT_LONG add_w[5], add_e[5], add_s[5], add_n[5], add_w2[5], add_e2[5], add_s2[5], add_n2[5];
	GMT_LONG i, j, kase;

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

void fill_in_forecast (struct SURFACE_INFO *C) {

	/* Fills in bilinear estimates into new node locations
	   after grid is divided.   
	 */

	GMT_LONG i, j, ii, jj, index_0, index_1, index_2, index_3, index_new;
	char *iu = C->iu;
	double delta_x, delta_y, a0, a1, a2, a3, old_size;
	float *u = C->Grid->data;
	
	old_size = 1.0 / (double)C->old_grid;

	/* first do from southwest corner */

	for (i = 0; i < C->nx-1; i += C->old_grid) {

		for (j = 0; j < C->ny-1; j += C->old_grid) {

			/* get indices of bilinear square */
			index_0 = C->ij_sw_corner + i * C->my + j;
			index_1 = index_0 + C->old_grid * C->my;
			index_2 = index_1 + C->old_grid;
			index_3 = index_0 + C->old_grid;

			/* get coefficients */
			a0 = u[index_0];
			a1 = u[index_1] - a0;
			a2 = u[index_3] - a0;
			a3 = u[index_2] - a0 - a1 - a2;

			/* find all possible new fill ins */

			for (ii = i;  ii < i + C->old_grid; ii += C->grid) {
				delta_x = (ii - i) * old_size;
				for (jj = j;  jj < j + C->old_grid; jj += C->grid) {
					index_new = C->ij_sw_corner + ii * C->my + jj;
					if (index_new == index_0) continue;
					delta_y = (jj - j) * old_size;
					u[index_new] = (float)(a0 + a1 * delta_x + delta_y * ( a2 + a3 * delta_x));
					iu[index_new] = 0;
				}
			}
			iu[index_0] = 5;
		}
	}

	/* now do linear guess along east edge */

	for (j = 0; j < (C->ny-1); j += C->old_grid) {
		index_0 = C->ij_se_corner + j;
		index_3 = index_0 + C->old_grid;
		for (jj = j;  jj < j + C->old_grid; jj += C->grid) {
			index_new = C->ij_se_corner + jj;
			delta_y = (jj - j) * old_size;
			u[index_new] = u[index_0] + (float)(delta_y * (u[index_3] - u[index_0]));
			iu[index_new] = 0;
		}
		iu[index_0] = 5;
	}
	/* now do linear guess along north edge */
	for (i = 0; i < (C->nx-1); i += C->old_grid) {
		index_0 = C->ij_nw_corner + i * C->my;
		index_1 = index_0 + C->old_grid * C->my;
		for (ii = i;  ii < i + C->old_grid; ii += C->grid) {
			index_new = C->ij_nw_corner + ii * C->my;
			delta_x = (ii - i) * old_size;
			u[index_new] = u[index_0] + (float)(delta_x * (u[index_1] - u[index_0]));
			iu[index_new] = 0;
		}
		iu[index_0] = 5;
	}
	/* now set northeast corner to fixed and we're done */
	iu[C->ij_ne_corner] = 5;
}

int compare_points (const void *point_1v, const void *point_2v)
{
		/*  Routine for qsort to sort data structure for fast access to data by node location.
		    Sorts on index first, then on radius to node corresponding to index, so that index
		    goes from low to high, and so does radius.
		*/
	GMT_LONG block_i, block_j, index_1, index_2;
	double x0, y0, dist_1, dist_2;
	struct SURFACE_DATA *point_1, *point_2;
	
	point_1 = (struct SURFACE_DATA *)point_1v;
	point_2 = (struct SURFACE_DATA *)point_2v;
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

void smart_divide (struct SURFACE_INFO *C) {
		/* Divide grid by its largest prime factor */
	C->grid /= C->factors[C->n_fact - 1];
	C->n_fact--;
}

void set_index (struct SURFACE_INFO *C) {
	/* recomputes data[k].index for new value of grid,
	   sorts data on index and radii, and throws away
	   data which are now outside the usable limits. */
	GMT_LONG i, j, k, k_skipped = 0;
	struct GRD_HEADER *h = C->Grid->header;

	for (k = 0; k < C->npoints; k++) {
		i = (GMT_LONG)floor(((C->data[k].x-h->wesn[XLO])*C->r_grid_xinc) + 0.5);
		j = (GMT_LONG)floor(((C->data[k].y-h->wesn[YLO])*C->r_grid_yinc) + 0.5);
		if (i < 0 || i >= C->block_nx || j < 0 || j >= C->block_ny) {
			C->data[k].index = SURFACE_OUTSIDE;
			k_skipped++;
		}
		else
			C->data[k].index = i * C->block_ny + j;
	}

	qsort ((void *)C->data, (size_t)C->npoints, sizeof (struct SURFACE_DATA), compare_points);

	C->npoints -= k_skipped;

}

void find_nearest_point (struct SURFACE_INFO *C) {
	GMT_LONG i, j, ij_v2, k, last_index, block_i, block_j, iu_index, briggs_index;
	double x0, y0, dx, dy, xys, xy1, btemp, b0, b1, b2, b3, b4, b5;
	float z_at_node, *u = C->Grid->data;
	char *iu = C->iu;
	struct GRD_HEADER *h = C->Grid->header;

	last_index = -1;
	C->small = 0.05 * ((C->grid_xinc < C->grid_yinc) ? C->grid_xinc : C->grid_yinc);

	for (i = 0; i < C->nx; i += C->grid)	/* Reset grid info */
		for (j = 0; j < C->ny; j += C->grid)
			iu[C->ij_sw_corner + i*C->my + j] = 0;

	briggs_index = 0;
	for (k = 0; k < C->npoints; k++) {	/* Find constraining value  */
		if (C->data[k].index != last_index) {
			block_i = C->data[k].index/C->block_ny;
			block_j = C->data[k].index%C->block_ny;
			last_index = C->data[k].index;
	 		iu_index = C->ij_sw_corner + (block_i * C->my + block_j) * C->grid;
	 		x0 = h->wesn[XLO] + block_i*C->grid_xinc;
	 		y0 = h->wesn[YLO] + block_j*C->grid_yinc;
	 		dx = (C->data[k].x - x0)*C->r_grid_xinc;
	 		dy = (C->data[k].y - y0)*C->r_grid_yinc;
	 		if (fabs(dx) < C->small && fabs(dy) < C->small) {	/* Close enough to assign value to node */
	 			iu[iu_index] = 5;
	 			/* v3.3.4: NEW CODE
	 			 * Since point is basically moved from (dx, dy) to (0,0) we must adjust for
	 			 * the C->small change in the planar trend between the two locations, and then
	 			 * possibly clip the range if constraining surfaces were given.  Note that
	 			 * dx, dy is in -1/1 range normalized by (grid * x|y_inc) so to recover the
	 			 * dx,dy in final grid fractions we must scale by grid */
	 			 
	 			z_at_node = C->data[k].z + (float) (C->r_z_scale * C->grid * (C->plane_c1 * dx + C->plane_c2 * dy));
	 			if (C->constrained) {	/* Must use ij_v2 since constrained grids are in standard scanline format */
					ij_v2 = GMT_IJP (C->Grid->header, C->ny - block_j * C->grid - 1, block_i * C->grid);
					if (C->set_low  && !GMT_is_fnan (C->Low->data[ij_v2]) && z_at_node < C->Low->data[ij_v2])
						z_at_node = C->Low->data[ij_v2];
					else if (C->set_high && !GMT_is_fnan (C->High->data[ij_v2]) && z_at_node > C->High->data[ij_v2])
						z_at_node = C->High->data[ij_v2];
	 			}
	 			u[iu_index] = z_at_node;
	 		}
	 		else {
	 			if (dx >= 0.0) {
	 				if (dy >= 0.0)
	 					iu[iu_index] = 1;
	 				else
	 					iu[iu_index] = 4;
	 			}
	 			else {
	 				if (dy >= 0.0)
	 					iu[iu_index] = 2;
	 				else
	 					iu[iu_index] = 3;
	 			}
	 			dx = fabs(dx);
	 			dy = fabs(dy);
	 			btemp = 2 * C->one_plus_e2 / ( (dx + dy) * (1.0 + dx + dy) );
	 			b0 = 1.0 - 0.5 * (dx + (dx * dx)) * btemp;
	 			b3 = 0.5 * (C->e_2 - (dy + (dy * dy)) * btemp);
	 			xys = 1.0 + dx + dy;
	 			xy1 = 1.0 / xys;
	 			b1 = (C->e_2 * xys - 4 * dy) * xy1;
	 			b2 = 2 * (dy - dx + 1.0) * xy1;
	 			b4 = b0 + b1 + b2 + b3 + btemp;
	 			b5 = btemp * C->data[k].z;
	 			C->briggs[briggs_index].b[0] = b0;
	 			C->briggs[briggs_index].b[1] = b1;
	 			C->briggs[briggs_index].b[2] = b2;
	 			C->briggs[briggs_index].b[3] = b3;
	 			C->briggs[briggs_index].b[4] = b4;
	 			C->briggs[briggs_index].b[5] = b5;
	 			briggs_index++;
	 		}
	 	}
	 }
}

void set_grid_parameters (struct SURFACE_INFO *C)
{
	GMT_Surface_Global.block_ny = C->block_ny = (C->ny - 1) / C->grid + 1;
	C->block_nx = (C->nx - 1) / C->grid + 1;
	GMT_Surface_Global.grid_xinc = C->grid_xinc = C->grid * C->Grid->header->inc[GMT_X];
	GMT_Surface_Global.grid_yinc = C->grid_yinc = C->grid * C->Grid->header->inc[GMT_Y];
	C->grid_east = C->grid * C->my;
	C->r_grid_xinc = 1.0 / C->grid_xinc;
	C->r_grid_yinc = 1.0 / C->grid_yinc;
}

void initialize_grid (struct GMT_CTRL *GMT, struct SURFACE_INFO *C)
{	/*
	 * For the initial gridsize, compute weighted averages of data inside the search radius
	 * and assign the values to u[i,j] where i,j are multiples of gridsize.
	 */
	GMT_LONG irad, jrad, i, j, imin, imax, jmin, jmax, index_1, index_2, k, ki, kj, k_index;
	double r, rfact, sum_w, sum_zw, weight, x0, y0;
	float *u = C->Grid->data;
	struct GRD_HEADER *h = C->Grid->header;

	 irad = (GMT_LONG)ceil(C->radius/C->grid_xinc);
	 jrad = (GMT_LONG)ceil(C->radius/C->grid_yinc);
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
	 		index_1 = imin*C->block_ny + jmin;
	 		index_2 = imax*C->block_ny + jmax + 1;
	 		sum_w = sum_zw = 0.0;
	 		k = 0;
	 		while (k < C->npoints && C->data[k].index < index_1) k++;
	 		for (ki = imin; k < C->npoints && ki <= imax && C->data[k].index < index_2; ki++) {
	 			for (kj = jmin; k < C->npoints && kj <= jmax && C->data[k].index < index_2; kj++) {
	 				k_index = ki*C->block_ny + kj;
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
	 			sprintf (C->format, "Warning: no data inside search radius at: %s %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	 			GMT_report (GMT, GMT_MSG_FATAL, C->format, x0, y0);
	 			u[C->ij_sw_corner + (i * C->my + j) * C->grid] = (float)C->z_mean;
	 		}
	 		else {
	 			u[C->ij_sw_corner + (i*C->my+j)*C->grid] = (float)(sum_zw/sum_w);
	 		}
		}
	}
}

#if 0
void new_initialize_grid (struct SURFACE_INFO *C)
{	/*
	 * For the initial gridsize, load constrained nodes with weighted avg of their data;
	 * and then do something with the unconstrained ones.
	 */
	GMT_LONG k, k_index, u_index, block_i, block_j;
	char *iu = C->iu;
	double sum_w, sum_zw, weight, x0, y0, dx, dy, dx_scale, dy_scale;
	float *u = C->Grid->data;
	struct GRD_HEADER *h = C->Grid->header;

	dx_scale = 4.0 / C->grid_xinc;
	dy_scale = 4.0 / C->grid_yinc;
	C->n_empty = C->block_ny * C->block_nx;
	k = 0;
	while (k < C->npoints) {
		block_i = C->data[k].index / C->block_ny;
		block_j = C->data[k].index % C->block_ny;
		x0 = h->wesn[XLO] + block_i*C->grid_xinc;
		y0 = h->wesn[YLO] + block_j*C->grid_yinc;
		u_index = C->ij_sw_corner + (block_i*C->my + block_j) * C->grid;
		k_index = C->data[k].index;

		dy = (C->data[k].y - y0) * dy_scale;
		dx = (C->data[k].x - x0) * dx_scale;
		sum_w = 1.0 / (1.0 + dx*dx + dy*dy);
		sum_zw = C->data[k].z * sum_w;
		k++;

		while (k < C->npoints && C->data[k].index == k_index) {

			dy = (C->data[k].y - y0) * dy_scale;
			dx = (C->data[k].x - x0) * dx_scale;
			weight = 1.0 / (1.0 + dx*dx + dy*dy);
			sum_zw += C->data[k].z * weight;
			sum_w += weight;
			sum_zw += weight*C->data[k].z;
			k++;
	 	}
	 	u[u_index] = (float)(sum_zw/sum_w);
	 	iu[u_index] = 5;
	 	C->n_empty--;
	 }
}
#endif

GMT_LONG read_data_surface (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, struct GMT_OPTION *options)
{
	GMT_LONG i, j, k, kmax = 0, kmin = 0, n_fields, error;
	double *in, zmin = DBL_MAX, zmax = -DBL_MAX, wesn_lim[4];
	struct GRD_HEADER *h = C->Grid->header;

	C->data = GMT_memory (GMT, NULL, C->n_alloc, struct SURFACE_DATA);

	/* Read in xyz data and computes index no and store it in a structure */
	
	if ((error = GMT_set_cols (GMT, GMT_IN, 3))) return (error);
	if ((error = GMT_Init_IO (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) return (error);	/* Establishes data input */

	k = 0;
	C->z_mean = 0.0;
	/* Initially allow points to be within 1 grid spacing of the grid */
	wesn_lim[XLO] = h->wesn[XLO] - C->grid_xinc;	wesn_lim[XHI] = h->wesn[XHI] + C->grid_xinc;
	wesn_lim[YLO] = h->wesn[YLO] - C->grid_yinc;	wesn_lim[YHI] = h->wesn[YHI] + C->grid_yinc;

	if ((error = GMT_Begin_IO (GMT->parent, GMT_IS_DATASET, GMT_IN, GMT_BY_REC))) return (error);	/* Enables data input and sets access mode */
	while ((n_fields = GMT_Get_Record (GMT->parent, GMT_READ_DOUBLE, (void **)&in)) != EOF) {	/* Keep returning records until we reach EOF */

		if (GMT_REC_IS_ERROR (GMT)) return (GMT_RUNTIME_ERROR);

		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Skip table and segment headers */

		if (GMT_is_dnan (in[GMT_Z])) continue;
		if (GMT_y_is_outside (GMT, in[GMT_Y], wesn_lim[YLO], wesn_lim[YHI])) continue;	/* Outside y-range */
		if (GMT_x_is_outside (GMT, &in[GMT_X], wesn_lim[XLO], wesn_lim[XHI])) continue;	/* Outside x-range (or longitude) */

		i = (GMT_LONG)floor(((in[GMT_X]-h->wesn[XLO])*C->r_grid_xinc) + 0.5);
		if (i < 0 || i >= C->block_nx) continue;
		j = (GMT_LONG)floor(((in[GMT_Y]-h->wesn[YLO])*C->r_grid_yinc) + 0.5);
		if (j < 0 || j >= C->block_ny) continue;

		C->data[k].index = i * C->block_ny + j;
		C->data[k].x = (float)in[GMT_X];
		C->data[k].y = (float)in[GMT_Y];
		C->data[k].z = (float)in[GMT_Z];
		if (zmin > in[GMT_Z]) zmin = in[GMT_Z], kmin = k;
		if (zmax < in[GMT_Z]) zmax = in[GMT_Z], kmax = k;
		k++;
		C->z_mean += in[GMT_Z];
		if (k == (GMT_LONG)C->n_alloc) {
			C->n_alloc <<= 1;
			C->data = GMT_memory (GMT, C->data, C->n_alloc, struct SURFACE_DATA);
		}
	}
	if ((error = GMT_End_IO (GMT->parent, GMT_IN, 0))) return (error);	/* Disables further data input */

	C->npoints = k;

	if (C->npoints == 0) {
		GMT_report (GMT, GMT_MSG_FATAL, " No datapoints inside region, aborts\n");
		return (EXIT_FAILURE);
	}

	C->z_mean /= k;
	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) {
		sprintf (C->format, "%s %s %s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		GMT_report (GMT, GMT_MSG_NORMAL, "Minimum value of your dataset x,y,z at: ");
		GMT_report (GMT, GMT_MSG_NORMAL, C->format, (double)C->data[kmin].x, (double)C->data[kmin].y, (double)C->data[kmin].z);
		GMT_report (GMT, GMT_MSG_NORMAL, "Maximum value of your dataset x,y,z at: ");
		GMT_report (GMT, GMT_MSG_NORMAL, C->format, (double)C->data[kmax].x, (double)C->data[kmax].y, (double)C->data[kmax].z);
	}
	C->data = GMT_memory (GMT, C->data, C->npoints, struct SURFACE_DATA);

	if (C->set_low == 1)
		C->low_limit = C->data[kmin].z;
	else if (C->set_low == 2 && C->low_limit > C->data[kmin].z) {
	/*	C->low_limit = data[kmin].z;	*/
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: Your lower value is > than min data value.\n");
	}
	if (C->set_high == 1)
		C->high_limit = C->data[kmax].z;
	else if (C->set_high == 2 && C->high_limit < C->data[kmax].z) {
	/*	C->high_limit = data[kmax].z;	*/
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: Your upper value is < than max data value.\n");
	}
	return (0);
}

GMT_LONG load_constraints (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, GMT_LONG transform)
{
	GMT_LONG i, j, ij, error;
	double yy;

	/* Load lower/upper limits, verify range, deplane, and rescale */

	if ((C->set_low == 3 || C->set_high == 3) && (error = GMT_Begin_IO (GMT->parent, GMT_IS_GRID, GMT_IN, GMT_BY_SET))) return (error);	/* Enables grid input and sets access mode */
	
	if (C->set_low > 0) {
		if (C->set_low < 3) {
			C->Low = GMT_create_grid (GMT);
			C->Low->data = GMT_memory (GMT, NULL, C->mxmy, float);
			for (i = 0; i < C->mxmy; i++) C->Low->data[i] = (float)C->low_limit;
		}
		else {
			if (GMT_Get_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(C->low_file), (void **)&C->Low)) return (GMT_DATA_READ_ERROR);	/* Get header only */
			if (C->Low->header->nx != C->nx || C->Low->header->ny != C->ny) {
				GMT_report (GMT, GMT_MSG_FATAL, "Lower limit file not of proper dimension!\n");
				return (EXIT_FAILURE);
			}
			if (GMT_Get_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, (void **)&(C->low_file), (void **)&C->Low)) return (GMT_DATA_READ_ERROR);
		}
		if (transform) {
			for (j = 0; j < C->ny; j++) {
				yy = (double)(C->ny - j - 1);
				for (i = 0; i < C->nx; i++) {
					ij = GMT_IJP (C->Grid->header, j, i);
					if (GMT_is_fnan (C->Low->data[ij])) continue;
					C->Low->data[ij] -= (float)(C->plane_c0 + C->plane_c1 * i + C->plane_c2 * yy);
					C->Low->data[ij] *= (float)C->r_z_scale;
				}
			}
		}
		C->constrained = TRUE;
	}
	if (C->set_high > 0) {
		if (C->set_high < 3) {
			C->High = GMT_create_grid (GMT);
			C->High->data = GMT_memory (GMT, NULL, C->mxmy, float);
			for (i = 0; i < C->mxmy; i++) C->High->data[i] = (float)C->high_limit;
		}
		else {
			if (GMT_Get_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(C->low_file), (void **)&C->High)) return (GMT_DATA_READ_ERROR);	/* Get header only */
			if (C->High->header->nx != C->nx || C->High->header->ny != C->ny) {
				GMT_report (GMT, GMT_MSG_FATAL, "Upper limit file not of proper dimension!\n");
				return (EXIT_FAILURE);
			}
			if (GMT_Get_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_DATA, (void **)&(C->low_file), (void **)&C->High)) return (GMT_DATA_READ_ERROR);
		}
		if (transform) {
			for (j = 0; j < C->ny; j++) {
				yy = (double)(C->ny - j - 1);
				for (i = 0; i < C->nx; i++) {
					ij = GMT_IJP (C->Grid->header, j, i);
					if (GMT_is_fnan (C->High->data[ij])) continue;
					C->High->data[ij] -= (float)(C->plane_c0 + C->plane_c1 * i + C->plane_c2 * yy);
					C->High->data[ij] *= (float)C->r_z_scale;
				}
			}
		}
		C->constrained = TRUE;
	}
	if ((C->set_low == 3 || C->set_high == 3) && (error = GMT_End_IO (GMT->parent, GMT_IN, 0))) return (error);	/* Disables further data input */

	return (0);
}

GMT_LONG write_output_surface (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, char *grdfile)
{	/* Uses v.2.0 netCDF grd format - hence need to transpose original grid to be GMT compatible.  This will be rewritten, eventually */
	GMT_LONG index, i, j, k, err;
	float *u = C->Grid->data, *v2 = NULL;

	if ((err = load_constraints (GMT, C, FALSE))) return (err);	/* Reload constraints but this time do not transform data */
		
	strcpy (C->Grid->header->title, "GMT_surface");

	v2 = GMT_memory (GMT, NULL, C->Grid->header->size, float);
	index = C->ij_sw_corner;
	if (GMT->common.r.active) {	/* Pixel registration request. Reset limits to the original extents */
		GMT_memcpy (C->Grid->header->wesn, C->wesn_orig, 4, double);
		C->Grid->header->registration = GMT_PIXEL_REG;
		/* Must reduce nx,ny by 1 to exclude the extra padding for pixel grids */
		C->Grid->header->nx--;	C->nx--;
		C->Grid->header->ny--;	C->ny--;
	}
	for (i = 0; i < C->nx; i++, index += C->my) {
		for (j = 0; j < C->ny; j++) {
			k = GMT_IJP (C->Grid->header, j, i);
			v2[k] = u[index + C->ny - j - 1];
			if (C->set_low  && !GMT_is_fnan (C->Low->data[k]) && v2[k] < C->Low->data[k]) v2[k] = C->Low->data[k];
			if (C->set_high && !GMT_is_fnan (C->High->data[k]) && v2[k] > C->High->data[k]) v2[k] = C->High->data[k];
		}
	}
	GMT_free (GMT, C->Grid->data);
	C->Grid->data = v2;
	if (GMT_Put_Data (GMT->parent, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_ALL, (void **)&grdfile, (void *)(C->Grid))) return (GMT_DATA_WRITE_ERROR);
	if (C->set_low) GMT_free (GMT, C->lower);
	if (C->set_high) GMT_free (GMT, C->upper);
	return (0);
}

GMT_LONG iterate (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, GMT_LONG mode)
{
	GMT_LONG i, j, k, ij, kase, briggs_index, ij_v2;
	GMT_LONG x_case, y_case, x_w_case, x_e_case, y_s_case, y_n_case;
	GMT_LONG iteration_count = 0;
	char *iu = C->iu;

	double current_limit = C->converge_limit / C->grid;
	double change, max_change = 0.0, busum, sum_ij;
	double b0, b1, b2, b3, b4, b5;
	float *u = C->Grid->data;

	double x_0_const = 4.0 * (1.0 - C->boundary_tension) / (2.0 - C->boundary_tension);
	double x_1_const = (3 * C->boundary_tension - 2.0) / (2.0 - C->boundary_tension);
	double y_denom = 2 * C->l_epsilon * (1.0 - C->boundary_tension) + C->boundary_tension;
	double y_0_const = 4 * C->l_epsilon * (1.0 - C->boundary_tension) / y_denom;
	double y_1_const = (C->boundary_tension - 2 * C->l_epsilon * (1.0 - C->boundary_tension) ) / y_denom;

	sprintf (C->format,"%%4ld\t%%c\t%%8ld\t%s\t%s\t%%10ld\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);

	do {
		briggs_index = 0;	/* Reset the constraint table stack pointer  */

		max_change = -1.0;

		/* Fill in auxiliary boundary values (in new way) */

		/* First set d2[]/dn2 = 0 along edges */
		/* New experiment : (1-T)d2[]/dn2 + Td[]/dn = 0  */

		for (i = 0; i < C->nx; i += C->grid) {
			/* set d2[]/dy2 = 0 on south side */
			ij = C->ij_sw_corner + i * C->my;
			/* u[ij - 1] = 2 * u[ij] - u[ij + grid];  */
			u[ij - 1] = (float)(y_0_const * u[ij] + y_1_const * u[ij + C->grid]);
			/* set d2[]/dy2 = 0 on north side */
			ij = C->ij_nw_corner + i * C->my;
			/* u[ij + 1] = 2 * u[ij] - u[ij - grid];  */
			u[ij + 1] = (float)(y_0_const * u[ij] + y_1_const * u[ij - C->grid]);

		}

		for (j = 0; j < C->ny; j += C->grid) {
			/* set d2[]/dx2 = 0 on west side */
			ij = C->ij_sw_corner + j;
			/* u[ij - my] = 2 * u[ij] - u[ij + grid_east];  */
			u[ij - C->my] = (float)(x_1_const * u[ij + C->grid_east] + x_0_const * u[ij]);
			/* set d2[]/dx2 = 0 on east side */
			ij = C->ij_se_corner + j;
			/* u[ij + my] = 2 * u[ij] - u[ij - grid_east];  */
			u[ij + C->my] = (float)(x_1_const * u[ij - C->grid_east] + x_0_const * u[ij]);
		}

		/* Now set d2[]/dxdy = 0 at each corner */

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
		for (i = 0; i < C->nx; i += C->grid, x_w_case++, x_e_case--) {

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
				(float)(u[ij + C->offset[kase][0]] + C->eps_m2*(u[ij + C->offset[kase][1]] + u[ij + C->offset[kase][3]]
					- u[ij + C->offset[kase][8]] - u[ij + C->offset[kase][10]])
					+ C->two_plus_em2 * (u[ij + C->offset[kase][9]] - u[ij + C->offset[kase][2]]) );
				/*  + tense * C->eps_m2 * (u[ij + C->offset[kase][2]] - u[ij + C->offset[kase][9]]) / (1.0 - tense);  */
			/* North side */
			kase = x_case * 5 + 4;
			ij = C->ij_nw_corner + i * C->my;
			u[ij + C->offset[kase][0]] = 
				-(float)(-u[ij + C->offset[kase][11]] + C->eps_m2 * (u[ij + C->offset[kase][1]] + u[ij + C->offset[kase][3]]
					- u[ij + C->offset[kase][8]] - u[ij + C->offset[kase][10]])
					+ C->two_plus_em2 * (u[ij + C->offset[kase][9]] - u[ij + C->offset[kase][2]]) );
				/*  - tense * C->eps_m2 * (u[ij + C->offset[kase][2]] - u[ij + C->offset[kase][9]]) / (1.0 - tense);  */
		}

		y_s_case = 0;
		y_n_case = C->block_ny - 1;
		for (j = 0; j < C->ny; j += C->grid, y_s_case++, y_n_case--) {

			if(y_s_case < 2)
				y_case = y_s_case;
			else if(y_n_case < 2)
				y_case = 4 - y_n_case;
			else
				y_case = 2;

			/* West side */
			kase = y_case;
			ij = C->ij_sw_corner + j;
			u[ij+C->offset[kase][4]] = 
				u[ij + C->offset[kase][7]] + (float)(C->eps_p2 * (u[ij + C->offset[kase][3]] + u[ij + C->offset[kase][10]]
				-u[ij + C->offset[kase][1]] - u[ij + C->offset[kase][8]])
				+ C->two_plus_ep2 * (u[ij + C->offset[kase][5]] - u[ij + C->offset[kase][6]]));
				/*  + tense * (u[ij + C->offset[kase][6]] - u[ij + C->offset[kase][5]]) / (1.0 - tense);  */
			/* East side */
			kase = 20 + y_case;
			ij = C->ij_se_corner + j;
			u[ij + C->offset[kase][7]] = 
				- (float)(-u[ij + C->offset[kase][4]] + C->eps_p2 * (u[ij + C->offset[kase][3]] + u[ij + C->offset[kase][10]]
				- u[ij + C->offset[kase][1]] - u[ij + C->offset[kase][8]])
				+ C->two_plus_ep2 * (u[ij + C->offset[kase][5]] - u[ij + C->offset[kase][6]]) );
				/*  - tense * (u[ij + C->offset[kase][6]] - u[ij + C->offset[kase][5]]) / (1.0 - tense);  */
		}



		/* That's it for the boundary points.  Now loop over all data  */

		x_w_case = 0;
		x_e_case = C->block_nx - 1;
		for (i = 0; i < C->nx; i += C->grid, x_w_case++, x_e_case--) {

			if(x_w_case < 2)
				x_case = x_w_case;
			else if(x_e_case < 2)
				x_case = 4 - x_e_case;
			else
				x_case = 2;

			y_s_case = 0;
			y_n_case = C->block_ny - 1;

			ij = C->ij_sw_corner + i * C->my;

			for (j = 0; j < C->ny; j += C->grid, ij += C->grid, y_s_case++, y_n_case--) {

				if (iu[ij] == 5) continue;	/* Point is fixed  */

				if(y_s_case < 2)
					y_case = y_s_case;
				else if(y_n_case < 2)
					y_case = 4 - y_n_case;
				else
					y_case = 2;

				kase = x_case * 5 + y_case;
				sum_ij = 0.0;

				if (iu[ij] == 0) {		/* Point is unconstrained  */
					for (k = 0; k < 12; k++) {
						sum_ij += (u[ij + C->offset[kase][k]] * C->coeff[0][k]);
					}
				}
				else {				/* Point is constrained  */

					b0 = C->briggs[briggs_index].b[0];
					b1 = C->briggs[briggs_index].b[1];
					b2 = C->briggs[briggs_index].b[2];
					b3 = C->briggs[briggs_index].b[3];
					b4 = C->briggs[briggs_index].b[4];
					b5 = C->briggs[briggs_index].b[5];
					briggs_index++;
					if (iu[ij] < 3) {
						if (iu[ij] == 1) {	/* Point is in quadrant 1  */
							busum = b0 * u[ij + C->offset[kase][10]]
								+ b1 * u[ij + C->offset[kase][9]]
								+ b2 * u[ij + C->offset[kase][5]]
								+ b3 * u[ij + C->offset[kase][1]];
						}
						else {			/* Point is in quadrant 2  */
							busum = b0 * u[ij + C->offset[kase][8]]
								+ b1 * u[ij + C->offset[kase][9]]
								+ b2 * u[ij + C->offset[kase][6]]
								+ b3 * u[ij + C->offset[kase][3]];
						}
					}
					else {
						if (iu[ij] == 3) {	/* Point is in quadrant 3  */
							busum = b0 * u[ij + C->offset[kase][1]]
								+ b1 * u[ij + C->offset[kase][2]]
								+ b2 * u[ij + C->offset[kase][6]]
								+ b3 * u[ij + C->offset[kase][10]];
						}
						else {		/* Point is in quadrant 4  */
							busum = b0 * u[ij + C->offset[kase][3]]
								+ b1 * u[ij + C->offset[kase][2]]
								+ b2 * u[ij + C->offset[kase][5]]
								+ b3 * u[ij + C->offset[kase][8]];
						}
					}
					for (k = 0; k < 12; k++) {
						sum_ij += (u[ij + C->offset[kase][k]] * C->coeff[1][k]);
					}
					sum_ij = (sum_ij + C->a0_const_2 * (busum + b5))
						/ (C->a0_const_1 + C->a0_const_2 * b4);
				}

				/* New relaxation here  */
				sum_ij = u[ij] * C->relax_old + sum_ij * C->relax_new;

				if (C->constrained) {	/* Must check limits.  Note lower/upper is in standard scanline format and need ij_v2! */
					ij_v2 = GMT_IJP (C->Grid->header, C->ny - j - 1, i);
					if (C->set_low && !GMT_is_fnan (C->Low->data[ij_v2]) && sum_ij < C->Low->data[ij_v2])
						sum_ij = C->Low->data[ij_v2];
					else if (C->set_high && !GMT_is_fnan (C->High->data[ij_v2]) && sum_ij > C->High->data[ij_v2])
						sum_ij = C->High->data[ij_v2];
				}

				change = fabs(sum_ij - u[ij]);
				u[ij] = (float)sum_ij;
				if (change > max_change) max_change = change;
			}
		}
		iteration_count++;
		C->total_iterations++;
		max_change *= C->z_scale;	/* Put max_change into z units  */
		GMT_report (GMT, GMT_MSG_VERBOSE, C->format,
			C->grid, C->mode_type[mode], iteration_count, max_change, current_limit, C->total_iterations);

	} while (max_change > current_limit && iteration_count < C->max_iterations);

	GMT_report (GMT, GMT_MSG_NORMAL, C->format,
		C->grid, C->mode_type[mode], iteration_count, max_change, current_limit, C->total_iterations);

	return (iteration_count);
}

void check_errors (struct GMT_CTRL *GMT, struct SURFACE_INFO *C) {

	GMT_LONG i, j, k, ij, move_over[12];
	char *iu = C->iu;	/* move_over = C->offset[kase][12], but grid = 1 so move_over is easy  */

	double	x0, y0, dx, dy, mean_error, mean_squared_error, z_est, z_err, curvature, c;
	double	du_dx, du_dy, d2u_dx2, d2u_dxdy, d2u_dy2, d3u_dx3, d3u_dx2dy, d3u_dxdy2, d3u_dy3;

	double	x_0_const = 4.0 * (1.0 - C->boundary_tension) / (2.0 - C->boundary_tension);
	double	x_1_const = (3 * C->boundary_tension - 2.0) / (2.0 - C->boundary_tension);
	double	y_denom = 2 * C->l_epsilon * (1.0 - C->boundary_tension) + C->boundary_tension;
	double	y_0_const = 4 * C->l_epsilon * (1.0 - C->boundary_tension) / y_denom;
	double	y_1_const = (C->boundary_tension - 2 * C->l_epsilon * (1.0 - C->boundary_tension) ) / y_denom;
	float *u = C->Grid->data;
	struct GRD_HEADER *h = C->Grid->header;
	
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

	for (i = 0; i < C->nx; i ++) {
		ij = C->ij_sw_corner + i * C->my;
		u[ij - 1] = (float)(y_0_const * u[ij] + y_1_const * u[ij + 1]);
		ij = C->ij_nw_corner + i * C->my;
		u[ij + 1] = (float)(y_0_const * u[ij] + y_1_const * u[ij - 1]);
	}

	for (j = 0; j < C->ny; j ++) {
		ij = C->ij_sw_corner + j;
		u[ij - C->my] = (float)(x_1_const * u[ij + C->my] + x_0_const * u[ij]);
		ij = C->ij_se_corner + j;
		u[ij + C->my] = (float)(x_1_const * u[ij - C->my] + x_0_const * u[ij]);
	}

	ij = C->ij_sw_corner;
	u[ij - C->my - 1] = u[ij + C->my - 1] + u[ij - C->my + 1] - u[ij + C->my + 1];
	ij = C->ij_nw_corner;
	u[ij - C->my + 1] = u[ij + C->my + 1] + u[ij - C->my - 1] - u[ij + C->my - 1];
	ij = C->ij_se_corner;
	u[ij + C->my - 1] = u[ij - C->my - 1] + u[ij + C->my + 1] - u[ij - C->my + 1];
	ij = C->ij_ne_corner;
	u[ij + C->my + 1] = u[ij - C->my + 1] + u[ij + C->my - 1] - u[ij - C->my - 1];

	for (i = 0; i < C->nx; i ++) {

		ij = C->ij_sw_corner + i * C->my;
		u[ij + move_over[11]] = 
			(float)(u[ij + move_over[0]] + C->eps_m2*(u[ij + move_over[1]] + u[ij + move_over[3]]
				- u[ij + move_over[8]] - u[ij + move_over[10]])
				+ C->two_plus_em2 * (u[ij + move_over[9]] - u[ij + move_over[2]]) );

		ij = C->ij_nw_corner + i * C->my;
		u[ij + move_over[0]] = 
			-(float)(-u[ij + move_over[11]] + C->eps_m2 * (u[ij + move_over[1]] + u[ij + move_over[3]]
				- u[ij + move_over[8]] - u[ij + move_over[10]])
				+ C->two_plus_em2 * (u[ij + move_over[9]] - u[ij + move_over[2]]) );
	}

	for (j = 0; j < C->ny; j ++) {

		ij = C->ij_sw_corner + j;
		u[ij+move_over[4]] = 
			u[ij + move_over[7]] + (float)(C->eps_p2 * (u[ij + move_over[3]] + u[ij + move_over[10]]
			-u[ij + move_over[1]] - u[ij + move_over[8]])
			+ C->two_plus_ep2 * (u[ij + move_over[5]] - u[ij + move_over[6]]));

		ij = C->ij_se_corner + j;
		u[ij + move_over[7]] = 
			- (float)(-u[ij + move_over[4]] + C->eps_p2 * (u[ij + move_over[3]] + u[ij + move_over[10]]
			- u[ij + move_over[1]] - u[ij + move_over[8]])
			+ C->two_plus_ep2 * (u[ij + move_over[5]] - u[ij + move_over[6]]) );
	}

	/* That resets the boundary values.  Now we can test all data.  
		Note that this loop checks all values, even though only nearest were used.  */

	for (k = 0; k < C->npoints; k++) {
		i = C->data[k].index / C->ny;
		j = C->data[k].index % C->ny;
	 	ij = C->ij_sw_corner + i * C->my + j;
	 	if ( iu[ij] == 5 ) continue;
	 	x0 = h->wesn[XLO] + i*h->inc[GMT_X];
	 	y0 = h->wesn[YLO] + j*h->inc[GMT_Y];
	 	dx = (C->data[k].x - x0)*h->r_inc[GMT_X];
	 	dy = (C->data[k].y - y0)*h->r_inc[GMT_Y];
 
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
	 
	 curvature = 0.0;
	 
	 for (i = 0; i < C->nx; i++) {
	 	for (j = 0; j < C->ny; j++) {
	 		ij = C->ij_sw_corner + i * C->my + j;
	 		c = u[ij + move_over[6]] + u[ij + move_over[5]]
	 			+ u[ij + move_over[2]] + u[ij + move_over[9]] - 4.0 * u[ij + move_over[6]];
			curvature += (c * c);
		}
	}

	 GMT_report (GMT, GMT_MSG_NORMAL, "Fit info: N data points  N nodes\tmean error\trms error\tcurvature\n");
	 sprintf (C->format,"\t%%8ld\t%%8ld\t%s\t%s\t%s\n", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
	 GMT_report (GMT, GMT_MSG_NORMAL, C->format, C->npoints, C->nxny, mean_error, mean_squared_error, curvature);
 }

void remove_planar_trend (struct SURFACE_INFO *C)
{
	GMT_LONG i;
	double a, b, c, d, xx, yy, zz;
	double sx, sy, sz, sxx, sxy, sxz, syy, syz;
	struct GRD_HEADER *h = C->Grid->header;

	sx = sy = sz = sxx = sxy = sxz = syy = syz = 0.0;

	for (i = 0; i < C->npoints; i++) {

		xx = (C->data[i].x - h->wesn[XLO]) * h->r_inc[GMT_X];
		yy = (C->data[i].y - h->wesn[YLO]) * h->r_inc[GMT_Y];
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

	for (i = 0; i < C->npoints; i++) {
		xx = (C->data[i].x - h->wesn[XLO]) * h->r_inc[GMT_X];
		yy = (C->data[i].y - h->wesn[YLO]) * h->r_inc[GMT_Y];
		C->data[i].z -= (float)(C->plane_c0 + C->plane_c1 * xx + C->plane_c2 * yy);
	}
}

void replace_planar_trend (struct SURFACE_INFO *C)
{
	GMT_LONG i, j, ij;
	float *u = C->Grid->data;

	 for (i = 0; i < C->nx; i++) {
	 	for (j = 0; j < C->ny; j++) {
	 		ij = C->ij_sw_corner + i * C->my + j;
	 		u[ij] = (float)((u[ij] * C->z_scale) + (C->plane_c0 + C->plane_c1 * i + C->plane_c2 * j));
		}
	}
}

void throw_away_unusables (struct GMT_CTRL *GMT, struct SURFACE_INFO *C)
{
	/* This is a new routine to eliminate data which will become
		unusable on the final iteration, when grid = 1.
		It assumes grid = 1 and set_grid_parameters has been
		called.  We sort, mark redundant data as SURFACE_OUTSIDE, and
		sort again, chopping off the excess.

		Experimental modification 5 Dec 1988 by Smith, as part
		of a new implementation using core memory for b[6]
		coefficients, eliminating calls to temp file.
	*/

	GMT_LONG last_index, n_outside, k;

	/* Sort the data  */

	qsort ((void *)C->data, (size_t)C->npoints, sizeof (struct SURFACE_DATA), compare_points);

	/* If more than one datum is indexed to same node, only the first should be kept.
		Mark the additional ones as SURFACE_OUTSIDE
	*/
	last_index = -1;
	n_outside = 0;
	for (k = 0; k < C->npoints; k++) {
		if (C->data[k].index == last_index) {
			C->data[k].index = SURFACE_OUTSIDE;
			n_outside++;
		}
		else
			last_index = C->data[k].index;
	}
	
	if (n_outside) {	/* Sort again; this time the SURFACE_OUTSIDE points will be thrown away  */
		qsort ((void *)C->data, (size_t)C->npoints, sizeof (struct SURFACE_DATA), compare_points);
		C->npoints -= n_outside;
		C->data = GMT_memory (GMT, C->data, C->npoints, struct SURFACE_DATA);
		GMT_report (GMT, GMT_MSG_NORMAL, "%ld unusable points were supplied; these will be ignored.\n", n_outside);
		GMT_report (GMT, GMT_MSG_NORMAL, "You should have pre-processed the data with block-mean, -median, or -mode.\n");
	}
}

GMT_LONG rescale_z_values (struct GMT_CTRL *GMT, struct SURFACE_INFO *C)
{
	GMT_LONG i;
	double ssz = 0.0;

	for (i = 0; i < C->npoints; i++) ssz += (C->data[i].z * C->data[i].z);

	/* Set z_scale = rms(z) */

	C->z_scale = sqrt (ssz / C->npoints);

	if (C->z_scale < GMT_CONV_LIMIT) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: Input data lie exactly on a plane.\n");
		C->r_z_scale = C->z_scale = 1.0;
		return (1);	/* Flag to tell the main to just write out the plane */
	}
	else
		C->r_z_scale = 1.0 / C->z_scale;

	for (i = 0; i < C->npoints; i++) C->data[i].z *= (float)C->r_z_scale;

	if (C->converge_limit == 0.0) C->converge_limit = 0.001 * C->z_scale; /* i.e., 1 ppt of L2 scale */
	return (0);
}
/* gcd_euclid.c  Greatest common divisor routine  */

GMT_LONG gcd_euclid (GMT_LONG a, GMT_LONG b)
{
	/* Returns the greatest common divisor of u and v by Euclid's method.
	 * I have experimented also with Stein's method, which involves only
	 * subtraction and left/right shifting; Euclid is faster, both for
	 * integers of size 0 - 1024 and also for random integers of a size
	 * which fits in a long integer.  Stein's algorithm might be better
	 * when the integers are HUGE, but for our purposes, Euclid is fine.
	 *
	 * Walter H. F. Smith, 25 Feb 1992, after D. E. Knuth, vol. II  */

	GMT_LONG u, v, r;

	u = MAX (GMT_abs(a), GMT_abs(b));
	v = MIN (GMT_abs(a), GMT_abs(b));

	while (v > 0) {
		r = u % v;	/* Knuth notes that u < 2v 40% of the time;  */
		u = v;		/* thus we could have tried a subtraction  */
		v = r;		/* followed by an if test to do r = u%v  */
	}
	return (u);
}

double guess_surface_time (struct GMT_CTRL *GMT, GMT_LONG factors[], GMT_LONG nx, GMT_LONG ny)
{
	/* Routine to guess a number proportional to the operations
	 * required by surface working on a user-desired grid of
	 * size nx by ny, where nx = (x_max - x_min)/dx, and same for
	 * ny.  (That is, one less than actually used in routine.)
	 *
	 * This is based on the following untested conjecture:
	 * 	The operations are proportional to T = nxg*nyg*L,
	 *	where L is a measure of the distance that data
	 *	constraints must propagate, and nxg, nyg are the
	 * 	current size of the grid.
	 *	For nx,ny relatively prime, we will go through only
	 * 	one grid cycle, L = max(nx,ny), and T = nx*ny*L.
	 *	But for nx,ny whose greatest common divisor is a highly
	 * 	composite number, we will have L equal to the division
	 * 	step made at each new grid cycle, and nxg,nyg will
	 * 	also be smaller than nx,ny.  Thus we can hope to find
	 *	some nx,ny for which the total value of T is C->small.
	 *
	 * The above is pure speculation and has not been derived
	 * empirically.  In actual practice, the distribution of the
	 * data, both spatially and in terms of their values, will
	 * have a strong effect on convergence.
	 *
	 * W. H. F. Smith, 26 Feb 1992.  */

	GMT_LONG gcd;		/* Current value of the gcd  */
	GMT_LONG nxg, nyg;	/* Current value of the grid dimensions  */
	GMT_LONG nfactors = 0;	/* Number of prime factors of current gcd  */
	GMT_LONG factor;	/* Currently used factor  */
	/* Doubles are used below, even though the values will be integers,
		because the multiplications might reach sizes of O(n**3)  */
	double t_sum;		/* Sum of values of T at each grid cycle  */
	double length;		/* Current propagation distance.  */

	gcd = gcd_euclid ((GMT_LONG)nx, (GMT_LONG)ny);
	if (gcd > 1) {
		nfactors = GMT_get_prime_factors (GMT, gcd, factors);
		nxg = (GMT_LONG)(nx/gcd);
		nyg = (GMT_LONG)(ny/gcd);
		if (nxg < 3 || nyg < 3) {
			factor = factors[nfactors - 1];
			nfactors--;
			gcd /= factor;
			nxg *= factor;
			nyg *= factor;
		}
	}
	else {
		nxg = nx;
		nyg = ny;
	}
	length = (double)MAX(nxg, nyg);
	t_sum = nxg * (nyg * length);	/* Make it double at each multiply  */

	/* Are there more grid cycles ?  */
	while (gcd > 1) {
		factor = factors[nfactors - 1];
		nfactors--;
		gcd /= factor;
		nxg *= factor;
		nyg *= factor;
		length = (double)factor;
		t_sum += nxg * (nyg * length);
	}
	return (t_sum);
}

void suggest_sizes_for_surface (struct GMT_CTRL *GMT, GMT_LONG factors[], GMT_LONG nx, GMT_LONG ny)
{
	/* Calls guess_surface_time for a variety of trial grid
	 * sizes, where the trials are highly composite numbers
	 * with lots of factors of 2, 3, and 5.  The sizes are
	 * within the range (nx,ny) - (2*nx, 2*ny).  Prints to
	 * GMT->session.std[GMT_ERR] the values which are an improvement over the
	 * user's original nx,ny.
	 * Should be called with nx=(x_max-x_min)/dx, and ditto
	 * for ny; that is, one smaller than the lattice used
	 * in surface.c
	 *
	 * W. H. F. Smith, 26 Feb 1992.  */

	double users_time;	/* Time for user's nx, ny  */
	double current_time;	/* Time for current nxg, nyg  */
	GMT_LONG i;
	GMT_LONG nxg, nyg;	/* Guessed by this routine  */
	GMT_LONG nx2, ny2, nx3, ny3, nx5, ny5;	/* For powers  */
	GMT_LONG xstop, ystop;	/* Set to 2*nx, 2*ny  */
	GMT_LONG n_sug = 0;	/* N of suggestions found  */
	int compare_sugs (const void *point_1, const void *point_2);	/* Sort suggestions decreasing  */
	struct SURFACE_SUGGESTION *sug = NULL;

	users_time = guess_surface_time (GMT, factors, nx, ny);
	xstop = 2*nx;
	ystop = 2*ny;

	for (nx2 = 2; nx2 <= xstop; nx2 *= 2) {
	  for (nx3 = 1; nx3 <= xstop; nx3 *= 3) {
	    for (nx5 = 1; nx5 <= xstop; nx5 *= 5) {
		nxg = nx2 * nx3 * nx5;
		if (nxg < nx || nxg > xstop) continue;

		for (ny2 = 2; ny2 <= ystop; ny2 *= 2) {
		  for (ny3 = 1; ny3 <= ystop; ny3 *= 3) {
		    for (ny5 = 1; ny5 <= ystop; ny5 *= 5) {
			nyg = ny2 * ny3 * ny5;
			if (nyg < ny || nyg > ystop) continue;

			current_time = guess_surface_time (GMT, factors, nxg, nyg);
			if (current_time < users_time) {
				n_sug++;
				sug = GMT_memory (GMT, sug, n_sug, struct SURFACE_SUGGESTION);
				sug[n_sug-1].nx = nxg;
				sug[n_sug-1].ny = nyg;
				sug[n_sug-1].factor = users_time/current_time;
			}

		    }
		  }
		}
	    }
	  }
	}

	if (n_sug) {
		qsort ((void *)sug, (size_t)n_sug, sizeof(struct SURFACE_SUGGESTION), compare_sugs);
		for (i = 0; i < n_sug && i < 10; i++) {
			GMT_report (GMT, GMT_MSG_FATAL, "Hint: Choosing nx = %ld, ny = %ld might cut run time by a factor of %.8g\n",
				sug[i].nx, sug[i].ny, sug[i].factor);
		}
		GMT_free (GMT, sug);
	}
	else
		GMT_report (GMT, GMT_MSG_FATAL, "Cannot suggest any nx,ny better than your -R -I define.\n");
	return;
}

int compare_sugs (const void *point_1, const void *point_2)
{
	/* Sorts sugs into DESCENDING order!  */
	if (((struct SURFACE_SUGGESTION *)point_1)->factor < ((struct SURFACE_SUGGESTION *)point_2)->factor) return (1);
	if (((struct SURFACE_SUGGESTION *)point_1)->factor > ((struct SURFACE_SUGGESTION *)point_2)->factor) return(-1);
	return (0);
}

void load_parameters_surface (struct SURFACE_INFO *C, struct SURFACE_CTRL *Ctrl)
{
	if (Ctrl->S.active) {
		if (Ctrl->S.unit == 'm') Ctrl->S.radius /= 60.0;
		if (Ctrl->S.unit == 's') Ctrl->S.radius /= 3600.0;
	}
	C->radius = Ctrl->S.radius;
	GMT_memcpy (C->Grid->header->inc, Ctrl->I.inc, 2, double);
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
}

void interp_breakline (struct GMT_CTRL *GMT, struct SURFACE_INFO *C, struct GMT_TABLE *xyzline) {

	GMT_LONG n_tot = 0, this_ini = 0, this_end = 0, n_int = 0, i, j, k = 0, n, n_alloc, kmax = 0, kmin = 0;
	double *x = NULL, *y = NULL, *z = NULL, dx, dy, dz, r_dx, r_dy, zmin = DBL_MAX, zmax = -DBL_MAX;

	n_alloc = GMT_CHUNK;
	x = GMT_memory (GMT, NULL, n_alloc, double);
	y = GMT_memory (GMT, NULL, n_alloc, double);
	z = GMT_memory (GMT, NULL, n_alloc, double);

	r_dx = 1.0 / C->grid_xinc; 
	r_dy = 1.0 / C->grid_yinc; 
	for (i = 0; i < xyzline->n_segments; i++) {
		for (j = 0; j < xyzline->segment[i]->n_rows - 1; j++) {
			dx = xyzline->segment[i]->coord[GMT_X][j+1] - xyzline->segment[i]->coord[GMT_X][j];
			dy = xyzline->segment[i]->coord[GMT_Y][j+1] - xyzline->segment[i]->coord[GMT_Y][j];
			dz = xyzline->segment[i]->coord[GMT_Z][j+1] - xyzline->segment[i]->coord[GMT_Z][j];
			n_int = irint (MAX (fabs(dx) * r_dx, fabs(dy) * r_dy ) ) + 1;
			this_end += n_int;

			if (n_alloc >= this_end) {
				n_alloc += MAX (GMT_CHUNK, n_int);
				x = GMT_memory (GMT, x, n_alloc, double);
				y = GMT_memory (GMT, x, n_alloc, double);
				z = GMT_memory (GMT, x, n_alloc, double);
			}

			dx /= (floor((double)n_int) - 1);
			dy /= (floor((double)n_int) - 1);
			dz /= (floor((double)n_int) - 1);
			for (k = this_ini, n = 0; k < this_end - 1; k++, n++) {
				x[k] = xyzline->segment[i]->coord[GMT_X][j] + n * dx;
				y[k] = xyzline->segment[i]->coord[GMT_Y][j] + n * dy;
				z[k] = xyzline->segment[i]->coord[GMT_Z][j] + n * dz;
			}
			x[this_end - 1] = xyzline->segment[i]->coord[GMT_X][j+1];
			y[this_end - 1] = xyzline->segment[i]->coord[GMT_Y][j+1];
			z[this_end - 1] = xyzline->segment[i]->coord[GMT_Z][j+1];

			this_ini += n_int;
		}

		n_tot += this_end;
	}

	/* Now add the interpolated breakline to the C structure */

	k = C->npoints;
	C->data = GMT_memory (GMT, C->data, k+n_tot, struct SURFACE_DATA);
	C->z_mean *= k;		/* It was already computed, reset it to sum */
	if (C->set_low == 1)
		zmin = C->low_limit;
	if (C->set_high == 1)
		zmax = C->high_limit;

	for (n = 0; n < n_tot; n++) {

		if (GMT_is_dnan (z[n])) continue;

		i = (GMT_LONG)floor (((x[n] - C->Grid->header->wesn[XLO]) * C->r_grid_xinc) + 0.5);
		if (i < 0 || i >= C->block_nx) continue;
		j = (GMT_LONG)floor (((y[n] - C->Grid->header->wesn[YLO]) * C->r_grid_yinc) + 0.5);
		if (j < 0 || j >= C->block_ny) continue;

		C->data[k].index = i * C->block_ny + j;
		C->data[k].x = (float)x[n];
		C->data[k].y = (float)y[n];
		C->data[k].z = (float)z[n];
		if (zmin > z[n]) zmin = z[n], kmin = k;
		if (zmax < z[n]) zmax = z[n], kmax = k;
		k++;
		C->z_mean += z[n];
	}

	if (k != (C->npoints + n_tot))		/* We had some NaNs */
		C->data = GMT_memory (GMT, C->data, k, struct SURFACE_DATA);

	C->npoints = k;
	C->z_mean /= k;

	if (C->set_low == 1)
		C->low_limit = C->data[kmin].z;
	if (C->set_high == 1)
		C->high_limit = C->data[kmax].z;

	GMT_free (GMT, x);
	GMT_free (GMT, y);
	GMT_free (GMT, z);
}

void *New_surface_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SURFACE_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct SURFACE_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->N.value = 250;
	C->A.value = 1.0;
	C->Z.value = 1.4;
		
	return ((void *)C);
}

void Free_surface_Ctrl (struct GMT_CTRL *GMT, struct SURFACE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->G.file) free ((void *)C->G.file);	
	if (C->D.file) free ((void *)C->D.file);	
	if (C->L.low)  free ((void *)C->L.low);	
	if (C->L.high) free ((void *)C->L.high);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_surface_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "surface %s [API] - Adjustable tension continuous curvature surface gridding\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: surface [xyz-file] -G<output_grdfile_name> %s\n", GMT_I_OPT);
	GMT_message (GMT, "\t%s [-A<aspect_ratio>] [-C<convergence_limit>] [-D<breakline>]\n", GMT_Rgeo_OPT);
	GMT_message (GMT, "\t[-Ll<limit>] [-Lu<limit>] [-N<n_iterations>] ] [-S<search_radius>[m|s]] [-T<tension>[i][b]]\n");
	GMT_message (GMT, "\t[-Q] [%s] [-Z<over_relaxation_parameter>] [%s] [%s] [%s] [%s] [%s] [%s]\n\n",
		GMT_V_OPT, GMT_bi_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_r_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\tsurface will read from standard input or a single <xyz-file>.\n\n");
	GMT_message (GMT, "\tRequired arguments to surface:\n");
	GMT_message (GMT, "\t-G sets output grid file name.\n");
	GMT_inc_syntax (GMT, 'I', 0);
	GMT_message (GMT, "\t\tNote that only gridline registration can be used.\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Set aspect-ratio> [Default = 1 gives an isotropic solution],\n");
	GMT_message (GMT, "\t   i.e. xinc and yinc assumed to give derivatives of equal weight; if not, specify\n");
	GMT_message (GMT, "\t   <aspect_ratio> such that yinc = xinc / <aspect_ratio>.\n");
	GMT_message (GMT, "\t   e.g. if gridding lon,lat use <aspect_ratio> = cosine(middle of lat range).\n");
	GMT_message (GMT, "\t-C Set convergence limit; iteration stops when max abs change is less than <c.l.>\n");
	GMT_message (GMT, "\t   Default will choose 0.001 of the range of your z data (1 ppt precision).\n");
	GMT_message (GMT, "\t   Enter your own convergence limit in same units as z data.\n");
	GMT_message (GMT, "\t-D Uses xyz data (can be multiseg) in the <breakline> file as a 'soft breakline'.\n");
	GMT_message (GMT, "\t-L Constrain the range of output values:\n");
	GMT_message (GMT, "\t   -Ll<limit> specifies lower limit; forces solution to be >= <limit>.\n");
	GMT_message (GMT, "\t   -Lu<limit> specifies upper limit; forces solution to be <= <limit>.\n");
	GMT_message (GMT, "\t   <limit> can be any number, or the letter d for min (or max) input data value,\n");
	GMT_message (GMT, "\t   or the filename of a grid with bounding values.  [Default solution unconstrained].\n");
	GMT_message (GMT, "\t   Example: -Ll0 gives a non-negative solution.\n");
	GMT_message (GMT, "\t-N Sets max <n_iterations> in each cycle; default = 250.\n");
	GMT_message (GMT, "\t-S Sets <search_radius> to initialize grid; default = 0 will skip this step.\n");
	GMT_message (GMT, "\t   This step is slow and not needed unless grid dimensions are pathological;\n");
	GMT_message (GMT, "\t   i.e., have few or no common factors.\n");
	GMT_message (GMT, "\t   Append m or s to give <search_radius> in minutes or seconds.\n");
	GMT_message (GMT, "\t-T Adds Tension to the gridding equation; use a value between 0 and 1.\n");
	GMT_message (GMT, "\t   default = 0 gives minimum curvature (smoothest; bicubic) solution.\n");
	GMT_message (GMT, "\t   1 gives a harmonic spline solution (local max/min occur only at data points).\n");
	GMT_message (GMT, "\t   typically 0.25 or more is good for potential field (smooth) data;\n");
	GMT_message (GMT, "\t   0.75 or so for topography.  Experiment.\n");
	GMT_message (GMT, "\t   Append B or b to set tension in boundary conditions only;\n");
	GMT_message (GMT, "\t   Append I or i to set tension in interior equations only;\n");
	GMT_message (GMT, "\t   No appended letter sets tension for both to same value.\n");
	GMT_message (GMT, "\t-Q Query for grid sizes that might run faster than your -R -I give.\n");
	GMT_explain_options (GMT, "V");
	GMT_message (GMT, "\t-Z Sets <over_relaxation parameter>.  Default = 1.4\n");
	GMT_message (GMT, "\t   Use a value between 1 and 2.  Larger number accelerates convergence but can be unstable.\n");
	GMT_message (GMT, "\t   Use 1 if you want to be sure to have (slow) stable convergence.\n\n");
	GMT_explain_options (GMT, "C3fhiF:.");
	GMT_message (GMT, "\t(For additional details, see Smith & Wessel, Geophysics, 55, 293-305, 1990.)\n");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_surface_parse (struct GMTAPI_CTRL *C, struct SURFACE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to surface and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0;
	char modifier;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = TRUE;
				Ctrl->A.value = atof (opt->arg);
				break;
			case 'C':
				Ctrl->C.active = TRUE;
				Ctrl->C.value = atof (opt->arg);
				break;
			case 'D':
				Ctrl->D.active = TRUE;
				Ctrl->D.file = strdup (opt->arg);
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'I':
				Ctrl->I.active = TRUE;
				if (GMT_getinc (GMT, opt->arg, Ctrl->I.inc)) {
					GMT_inc_syntax (GMT, 'I', 1);
					n_errors++;
				}
				break;
			case 'L':	/* Set limits */
				Ctrl->L.active = TRUE;
				switch (opt->arg[0]) {
					case 'l':	/* Lower limit  */
						n_errors += GMT_check_condition (GMT, opt->arg[1] == 0, "Syntax error -Ll option: No argument given\n");
						Ctrl->L.low = strdup (&opt->arg[1]);
						if (!GMT_access (GMT, Ctrl->L.low, R_OK))	/* file exists */
							Ctrl->L.lmode = 3;
						else if (Ctrl->L.low[0] == 'd')		/* Use data minimum */
							Ctrl->L.lmode = 1;
						else {
							Ctrl->L.lmode = 2;		/* Use given value */
							Ctrl->L.min = atof (&opt->arg[1]);
						}
						break;
					case 'u':	/* Upper limit  */
						n_errors += GMT_check_condition (GMT, opt->arg[1] == 0, "Syntax error -Lu option: No argument given\n");
						Ctrl->L.high = strdup (&opt->arg[1]);
						if (!GMT_access (GMT, Ctrl->L.high, R_OK))	/* file exists */
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
				Ctrl->N.active = TRUE;
				Ctrl->N.value = atoi (opt->arg);
				break;
			case 'Q':
				Ctrl->Q.active = TRUE;
				break;
			case 'S':
				Ctrl->S.active = TRUE;
				Ctrl->S.radius = atof (opt->arg);
				Ctrl->S.unit = opt->arg[strlen(opt->arg)-1];
#ifdef GMT_COMPAT
				if (Ctrl->S.unit == 'c') {
					GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Unit c is deprecated; use s instead.\n");
					Ctrl->S.unit = 's';
				}
#endif
				if (!strchr ("sm ", Ctrl->S.unit)) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -S option: Unrecognized unit %c\n", Ctrl->S.unit);
					n_errors++;
				}
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				modifier = opt->arg[strlen(opt->arg)-1];
				if (modifier == 'b' || modifier == 'B') {
					Ctrl->T.b_tension = atof (opt->arg);
				}
				else if (modifier == 'i' || modifier == 'I') {
					Ctrl->T.i_tension = atof (opt->arg);
				}
				else if (modifier >= '0' && modifier <= '9') {
					Ctrl->T.i_tension = Ctrl->T.b_tension = atof (opt->arg);
				}
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -T option: Unrecognized modifier %c\n", modifier);
					n_errors++;
				}
				break;
			case 'Z':
				Ctrl->Z.active = TRUE;
				Ctrl->Z.value = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	GMT_check_lattice (GMT, Ctrl->I.inc, NULL, &Ctrl->I.active);

	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && Ctrl->D.file && GMT_access (GMT, Ctrl->D.file, R_OK), "Syntax error -D: Cannot read file %s!\n", Ctrl->D.file);
	n_errors += GMT_check_condition (GMT, Ctrl->I.inc[GMT_X] <= 0.0 || Ctrl->I.inc[GMT_Y] <= 0.0, "Syntax error -I option: Must specify positive increment(s)\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.value < 1, "Syntax error -N option: Max iterations must be nonzero\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Z.value < 1.0 || Ctrl->Z.value > 2.0, "Syntax error -Z option: Relaxation value must be 1 <= z <= 2\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->G.file, "Syntax error option -G: Must specify output file\n");
	n_errors += GMT_check_binary_io (GMT, 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_surface_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_surface (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG error = FALSE, key, one = 1, greenwich = FALSE;
	
	struct GMT_TABLE *xyzline = NULL;
	struct GMT_DATASET *Lin = NULL;
	struct SURFACE_INFO C;
	struct SURFACE_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_surface_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_surface_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_surface", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VRbf:", "hirs" GMT_OPT("FH"), options))) Return (error);
	Ctrl = (struct SURFACE_CTRL *) New_surface_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_surface_parse (API, Ctrl, options))) Return (error);
	
	/*---------------------------- This is the surface main code ----------------------------*/

	GMT_memset (&C, 1, struct SURFACE_INFO);
	GMT_memset (&GMT_Surface_Global, 1, struct SURFACE_GLOBAL);
	C.n_alloc = GMT_CHUNK;
	C.z_scale = C.r_z_scale = 1.0;
	C.mode_type[0] = 'I';
	C.mode_type[1] = 'D';	/* D means include data points when iterating */

	C.Grid = GMT_create_grid (GMT);
	GMT_grd_init (GMT, C.Grid->header, options, FALSE);
	GMT_memcpy (C.Grid->header->wesn, GMT->common.R.wesn, 4, double);

	load_parameters_surface (&C, Ctrl);	/* Pass parameters from parsing control to surface INFO structure */

	GMT_memcpy (C.wesn_orig, C.Grid->header->wesn, 4, double);	/* Save original region in case of -r */
	if (GMT->common.r.active) {		/* Pixel registration request. Use the trick of offset area by x_inc(y_inc) / 2 */
		C.Grid->header->wesn[XLO] += Ctrl->I.inc[GMT_X] / 2.0;	C.Grid->header->wesn[XHI] += Ctrl->I.inc[GMT_X] / 2.0;
		C.Grid->header->wesn[YLO] += Ctrl->I.inc[GMT_Y] / 2.0;	C.Grid->header->wesn[YHI] += Ctrl->I.inc[GMT_Y] / 2.0;
		one++;	/* Just so we can report correct nx,ny for the grid; internally it is the same until output */
		/* nx,ny remains the same for now but nodes are in "pixel" position.  Must reduce nx,ny by 1 when we write result */
	}
	
	GMT_RI_prepare (GMT, C.Grid->header);	/* Ensure -R -I consistency and set nx, ny */
	GMT_err_fail (GMT, GMT_grd_RI_verify (GMT, C.Grid->header, 1), Ctrl->G.file);
	GMT_set_grddim (GMT, C.Grid->header);

	if (C.Grid->header->nx < 4 || C.Grid->header->ny < 4) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: Grid must have at least 4 nodes in each direction (you have %d by %d) - abort.\n", C.Grid->header->nx, C.Grid->header->ny);
		Return (EXIT_FAILURE);
	}

	C.relax_old = 1.0 - C.relax_new;

	C.nx = (GMT_LONG)C.Grid->header->nx;
	C.ny = (GMT_LONG)C.Grid->header->ny;
	C.nxny = C.Grid->header->nm;

	C.mx = C.nx + 4;
	C.my = C.ny + 4;
	C.mxmy = C.Grid->header->size;
	GMT_Surface_Global.x_min = C.Grid->header->wesn[XLO];
	GMT_Surface_Global.y_min = C.Grid->header->wesn[YLO];

	/* New stuff here for v4.3: Check out the grid dimensions */
	C.grid = gcd_euclid (C.nx-1, C.ny-1);

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL) || Ctrl->Q.active) {
		sprintf (C.format, "Grid domain: W: %s E: %s S: %s N: %s nx: %%ld ny: %%ld [", GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out, GMT->current.setting.format_float_out);
		(GMT->common.r.active) ? strcat (C.format, "pixel registration]\n") : strcat (C.format, "gridline registration]\n");
		GMT_report (GMT, GMT_MSG_NORMAL, C.format, C.wesn_orig[XLO], C.wesn_orig[XHI], C.wesn_orig[YLO], C.wesn_orig[YHI], C.nx-one, C.ny-one);
	}
	if (C.grid == 1) GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Your grid dimensions are mutually prime.\n");
	if ((C.grid == 1 && GMT_is_verbose (GMT, GMT_MSG_NORMAL)) || Ctrl->Q.active) suggest_sizes_for_surface (GMT, C.factors, C.nx-1, C.ny-1);
	if (Ctrl->Q.active) Return (EXIT_SUCCESS);

	/* New idea: set grid = 1, read data, setting index.  Then throw
		away data that can't be used in end game, constraining
		size of briggs->b[6] structure.  */

	C.grid = 1;
	set_grid_parameters (&C);
	if (read_data_surface (GMT, &C, options)) Return (EXIT_FAILURE);
	if (Ctrl->D.active) {
		greenwich = (C.Grid->header->wesn[XLO] < 0.0 && C.Grid->header->wesn[XHI] > 0.0);

		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, NULL, 0, (void **)&Ctrl->D.file, (void **)&Lin))
			Return ((error = GMT_DATA_READ_ERROR));
		xyzline = Lin->table[0];			/* Can only be one table since we read a single file */

		interp_breakline (GMT, &C, xyzline);
	}
	throw_away_unusables (GMT, &C);
	remove_planar_trend (&C);
	key = rescale_z_values (GMT, &C);
	
	if ((error = GMT_Begin_IO (API, GMT_IS_GRID, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */

	if (key == 1) {	/* Data lies exactly on a plane; just return the plane grid */
		GMT_free (GMT, C.data);
		C.Grid->data = GMT_memory (GMT, NULL, C.mxmy, float);
		C.ij_sw_corner = 2 * C.my + 2;			/*  Corners of array of actual data  */
		replace_planar_trend (&C);
		if ((error = write_output_surface (GMT, &C, Ctrl->G.file))) Return (error);
		Return (EXIT_SUCCESS);
	}
	
	load_constraints (GMT, &C, TRUE);

	/* Set up factors and reset grid to first value  */

	C.grid = gcd_euclid (C.nx-1, C.ny-1);
	C.n_fact = GMT_get_prime_factors (GMT, (GMT_LONG)C.grid, C.factors);
	set_grid_parameters (&C);
	while (C.block_nx < 4 || C.block_ny < 4) {
		smart_divide (&C);
		set_grid_parameters (&C);
	}
	set_offset (&C);
	set_index (&C);
	/* Now the data are ready to go for the first iteration.  */

	/* Allocate more space  */

	C.briggs = GMT_memory (GMT, NULL, C.npoints, struct SURFACE_BRIGGS);
	C.iu = GMT_memory (GMT, NULL, C.mxmy, char);
	C.Grid->data = GMT_memory (GMT, NULL, C.mxmy, float);

	if (C.radius > 0) initialize_grid (GMT, &C); /* Fill in nodes with a weighted avg in a search radius  */

	GMT_report (GMT, GMT_MSG_NORMAL, "Grid\tMode\tIteration\tMax Change\tConv Limit\tTotal Iterations\n");

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

	if (GMT_is_verbose (GMT, GMT_MSG_NORMAL)) check_errors (GMT, &C);

	replace_planar_trend (&C);

	GMT_free (GMT, C.data);
	GMT_free (GMT, C.briggs);
	GMT_free (GMT, C.iu);
	if (C.set_low) GMT_free (GMT, C.lower);
	if (C.set_high) GMT_free (GMT, C.upper);

	if ((error = write_output_surface (GMT, &C, Ctrl->G.file))) Return (error);
	if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */

	Return (GMT_OK);
}
