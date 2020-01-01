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
 * Brief synopsis: psmask tries to achieve masking using one of two different approaches:
 * The default way of operation is: Instead of painting tiles where there's no
 * data [or where there is data] psmask uses contouring to find the polygons
 * that contain the data [or contain the regions with no data].  For many types
 * of data coverage, this results in a manageable path instead of thousands of
 * tiles.  So, instead of painting polygons, psmask sets up actual clip paths.
 * As an option, the user may specify a rgb combination to fill the clipped areas.
 * To avoid having to deal with the problems that arise if the data distribution
 * is such that part of the map boundary should be part of the clip paths, we
 * internally enlarge the grid by one gridsize unit so no nodes along the edges
 * have data.  Before using the clippaths we move points outside the real region
 * onto the boundary.
 * Should the paths be too long for PostScript to handle, the user may override
 * the default with the -T switch.  Then, masking is achieved using tiling
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psmask"
#define THIS_MODULE_MODERN_NAME	"mask"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Clip or mask map areas with no data table coverage"
#define THIS_MODULE_KEYS	"<D{,DDD,C-(,>X},LG)@<D{,DD),C-(,LG)"
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYbdehiprstxy" GMT_OPT("Ec")

enum Mask_Modes {
	PSMASK_INSIDE  = -1,	/* Set inside nodes to NaN */
	PSMASK_DEFAULT =  0,	/* Leave nodes as they are */
	PSMASK_OUTSIDE = +1	/* Set outside nodes to NaN */
};

struct PSMASK_CTRL {
	struct C {	/* -C */
		bool active;
	} C;
	struct D {	/* -D<dumpfile> */
		bool active;
		char *file;
	} D;
	struct F {	/* -F<way> */
		bool active;
		int value;
	} F;
	struct G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct L {	/* -L<file>[+i|o] */
		bool active;
		int mode;	/* -1 = set inside node to NaN (+i), 0 as is, +1 set outside node to NaN (+o) */
		char *file;
	} L;
	struct N {	/* -N */
		bool active;
	} N;
	struct Q {	/* -Q<cut> */
		bool active;
		unsigned int min;
	} Q;
	struct S {	/* -S[-|=|+]<radius>[d|e|f|k|m|M|n|s] */
		bool active;
		int mode;	/* May be negative */
		double radius;
		char unit;
	} S;
	struct T {	/* -T */
		bool active;
	} T;
};

struct PSMASK_INFO {
	bool first_dump;
	int p[5], i_off[5], j_off[5], k_off[5];
	unsigned int bit[32], offset;
};

GMT_LOCAL void draw_clip_contours (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double *xx, double *yy, uint64_t nn, double rgb[], unsigned int id, unsigned int flag) {
	uint64_t i;
	double x, y;

	if (nn < 2 && flag < 2) return;

	for (i = 0; i < nn; i++) {
		x = xx[i];	y = yy[i];
		gmt_geo_to_xy (GMT, x, y, &xx[i], &yy[i]);
	}
	nn = gmt_compact_line (GMT, xx, yy, nn, false, 0);

	if (nn > 0) PSL_comment (PSL, "Start of clip path sub-segment %d\n", id);
	PSL_beginclipping (PSL, xx, yy, (int)nn, rgb, flag);
	if (nn > 0) PSL_comment (PSL, "End of clip path sub-segment %d\n", id);
}

GMT_LOCAL uint64_t trace_clip_contours (struct GMT_CTRL *GMT, struct PSMASK_INFO *info, char *grd, unsigned int *edge, struct GMT_GRID_HEADER *h, double inc2[], double **xx, double **yy, int i, int j, int kk, uint64_t *max) {
	/* Loosely based on support_trace_contour in gmt_support.c.
	 * Differs in that grd[] is known to only have values 0 (no data) or 1 (data point).
	 */
	int k, k0, n_cuts, kk_opposite, first_k, more;
	uint64_t n = 1, edge_word, edge_bit, ij, ij0, m;
	double xk[4], yk[4], x0, y0;

	m = *max - 2;	/* Note: *max starts at 2048 or so.  Allow for maybe needing 2 more than we have */
	
	more = true;
	do {
		/* Determine lower left corner of current 4-node box */
		ij = gmt_M_ijp (h, j, i);
		x0 = gmt_M_grd_col_to_x (GMT, i, h);
		y0 = gmt_M_grd_row_to_y (GMT, j, h);
		n_cuts = 0;
		k0 = kk;

		for (k = 0; k < 4; k++) {	/* Loop over the 4 box sides */

			if (k == k0) continue;	/* Skip where we already have a cut (k == k0) */

			/* Skip edge that already have been used */

			ij0 = gmt_M_ij0 (h, j + info->j_off[k], i + info->i_off[k]);
			edge_word = ij0 / 32 + info->k_off[k] * info->offset;
			edge_bit = ij0 % 32;
			if (edge[edge_word] & info->bit[edge_bit]) continue;	/* Already used */

			/* Skip if no zero-crossing on this edge */

			if ((grd[ij+info->p[k+1]] + grd[ij+info->p[k]]) != 1) continue;

			/* Here we have a crossing, which is always half-way between the 0 and 1 node */

			if (k%2) {	/* Cutting a S-N line */
				if (k == 1) {
					xk[1] = x0 + h->inc[GMT_X];
					yk[1] = y0 + 0.5*h->inc[GMT_Y];
				}
				else {
					xk[3] = x0;
					yk[3] = y0 + 0.5*h->inc[GMT_Y];
				}
			}
			else {	/* Cutting a E-W line */
				if (k == 0) {
					xk[0] = x0 + 0.5*h->inc[GMT_X];
					yk[0] = y0;
				}
				else {
					xk[2] = x0 + 0.5*h->inc[GMT_X];
					yk[2] = y0 + h->inc[GMT_Y];
				}
			}
			kk = k;		/* Remember id of last side with cut */
			n_cuts++;	/* Update number of sides with cuts */
		}

		if (n > m) {	/* Must allocate more memory for x,y arrays */
			*max <<= 1;	/* Double the memory */
			m = *max - 2;	/* But still check for 2 less in case we add 2 points */
			*xx = gmt_M_memory (GMT, *xx, *max, double);
			*yy = gmt_M_memory (GMT, *yy, *max, double);
		}
		if (n_cuts == 0) {	/* Close interior contour and return  [Will add 1 or 2 points] */
			/* if (fmod ((*xx[0] - inc2[GMT_X]), h->inc[GMT_X]) == 0.0) */	/* On side 1 or 3 */
			if (gmt_M_is_zero (fmod ((*xx[0] - inc2[GMT_X]), h->inc[GMT_X])))	/* On side 1 or 3 */
				/* first_k = ((*xx[0] - x0) == 0.0) ? 3 : 1; */
				first_k = doubleAlmostEqualZero (*xx[0], x0) ? 3 : 1;
			else 	/* On side 0 or 2 */
				/* first_k = ((*yy[0] - y0) == 0.0) ? 0 : 2; */
				first_k = doubleAlmostEqualZero (*yy[0], y0) ? 0 : 2;
			kk_opposite = (first_k + 2) % 4;
			if (k0 != kk_opposite) {
				(*xx)[n] = x0 + 0.5*h->inc[GMT_X];
				(*yy)[n] = y0 + 0.5*h->inc[GMT_Y];
				n++;
			}
			(*xx)[n] = (*xx)[0];	/* Closed contour */
			(*yy)[n] = (*yy)[0];
			n++;
			more = false;
		}
		else if (n_cuts == 1) {	/* Draw a line to this point and keep tracing [Will add 1 or 2 points] */
			/* Add center of box if this and previous cut NOT on opposite edges */
			kk_opposite = (k0 + 2) % 4;
			if (kk != kk_opposite) {
				(*xx)[n] = x0 + 0.5*h->inc[GMT_X];
				(*yy)[n] = y0 + 0.5*h->inc[GMT_Y];
				n++;
			}
			(*xx)[n] = xk[kk];
			(*yy)[n] = yk[kk];
			n++;
		}
		else {	/* Saddle point, we decide to connect to the point nearest previous point [Will add 2 points]*/
			kk = (k0 + 1)%4;	/* Pick next edge since it is arbitrarily where we go */
			/* First add center of box */
			(*xx)[n] = x0 + 0.5*h->inc[GMT_X];
			(*yy)[n] = y0 + 0.5*h->inc[GMT_Y];
			n++;
			(*xx)[n] = xk[kk];
			(*yy)[n] = yk[kk];
			n++;
		}
		if (more) {	/* Mark this edge as used */
			ij0 = gmt_M_ij0 (h, j + info->j_off[kk], i + info->i_off[kk]);
			edge_word = ij0 / 32 + info->k_off[kk] * info->offset;
			edge_bit = ij0 % 32;
			edge[edge_word] |= info->bit[edge_bit];
		}

		/* Get next box (i,j,kk) */

		i -= (kk-2)%2;
		j -= (kk-1)%2;
		kk = (kk+2)%4;

	} while (more);
	
	return (n);	/* Return length of polygon */
}

GMT_LOCAL uint64_t clip_contours (struct GMT_CTRL *GMT, struct PSMASK_INFO *info, char *grd, struct GMT_GRID_HEADER *h, double inc2[], unsigned int *edge, unsigned int first, double **x, double **y, uint64_t *max) {
	/* The routine finds the zero-contour in the grd dataset.  it assumes that
	 * no node has a value exactly == 0.0.  If more than max points are found
	 * trace_clip_contours will try to allocate more memory in blocks of GMT_CHUNK points.
	 * Note: info->offset is added to edge_word when looking at vertical edges.
	 */
	 
	unsigned int n_edges, edge_bit, i, j;
	static unsigned int i0, j0, side;
	uint64_t edge_word, ij, n = 0;
	bool go_on = true;
	 
	 
	n_edges = h->n_rows * (urint (ceil (h->n_columns / 16.0)));

	if (first) {	/* Reset edge-flags to zero, if necessary */
		int signed_nx = h->n_columns;	/* Needed to assign p[3] below */
		info->offset = n_edges / 2;
	 	i0 = 0;	/* Begin with upper left bin which is i = 0 and j = 1 */
	 	j0 = 1;
		side = 4;	/* Vertical interior gridlines */
		info->p[0] = info->p[4] = 0;	info->p[1] = 1;	info->p[2] = 1 - signed_nx;	info->p[3] = -signed_nx;
		info->i_off[0] = info->i_off[2] = info->i_off[3] = info->i_off[4] = 0;	info->i_off[1] =  1;
		info->j_off[0] = info->j_off[1] = info->j_off[3] = info->j_off[4] = 0;	info->j_off[2] = -1;
		info->k_off[0] = info->k_off[2] = info->k_off[4] = 0;	info->k_off[1] = info->k_off[3] = 1;
		for (i = 1, info->bit[0] = 1; i < 32; i++) info->bit[i] = info->bit[i-1] << 1;
	 }

	/* Loop over interior boxes */

	if (side == 4) {
		for (j = j0; go_on && j < h->n_rows; j++) {
			ij = gmt_M_ijp (h, j, i0);
			for (i = i0; go_on && i < h->n_columns-1; i++, ij++) {	/* n_columns-1 since the last bin starts at n_columns-2 and ends at n_columns-1 */
				edge_word = ij / 32 + info->offset;
				edge_bit = (unsigned int)(ij % 32ULL);
				if (!(edge[edge_word] & info->bit[edge_bit]) && ((grd[ij]+grd[ij-h->n_columns]) == 1)) { /* Start tracing contour */
					*x[0] = gmt_M_grd_col_to_x (GMT, i, h);
					*y[0] = gmt_M_grd_row_to_y (GMT, j, h) + 0.5 * h->inc[GMT_Y];
					edge[edge_word] |= info->bit[edge_bit];
					n = trace_clip_contours (GMT, info, grd, edge, h, inc2, x, y, i, j, 3, max);
					go_on = false;
					i0 = i + 1;
					j0 = j;	/* Return to finish this row later */
				}
			}
			if (go_on) i0 = 0;	/* Go to start of next row unless we found something */
		}
		if (n == 0) {
			side = 5;	/* Horizontal interior gridlines */
			i0 = 0;
			j0 = 1;
		}
	}
	if (n == 0 && side == 5) {	/* Look at horizontal interior gridlines */
		for (j = j0; go_on && j < h->n_rows; j++) {
			ij = gmt_M_ijp (h, j, i0);
			for (i = i0; go_on && i < h->n_columns-1; i++, ij++) {
				edge_word = ij / 32;
				edge_bit = (unsigned int)(ij % 32ULL);
				if (!(edge[edge_word] & info->bit[edge_bit]) && ((grd[ij]+grd[ij+1]) == 1)) { /* Start tracing contour */
					*x[0] = gmt_M_grd_col_to_x (GMT, i, h) + 0.5 * h->inc[GMT_X];
					*y[0] = gmt_M_grd_row_to_y (GMT, j, h);
					edge[edge_word] |= info->bit[edge_bit];
					n = trace_clip_contours (GMT, info, grd, edge, h, inc2, x, y, i, j, 2, max);
					go_on = false;
					i0 = i + 1;
					j0 = j;	/* Return to finish this row later */
				}
				if (go_on) i0 = 1;
			}
		}
	}	
	if (n>2) {	/* Properly connect last and first point given the half-pixel steps */
		if (n >= (*max)) {	/* Awkward, must allocate memory for one more point */
			(*max)++;
			*x = gmt_M_memory (GMT, *x, *max, double);
			*y = gmt_M_memory (GMT, *y, *max, double);
		}
		(*x)[n-1] = (*x)[n-2];	/* Change next to last point's x-coordinate to not cut over by 0.5*dx */
		(*x)[n] = (*x)[0];	/* Because of that, close the polygon explicitly by adding duplicate point */
		(*y)[n] = (*y)[0];
		n++;
	}
	return (n);
}

GMT_LOCAL void orient_contours (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, char *grd, double *x, double *y, uint64_t n, int orient) {
	/* Determine handedness of the contour and if opposite of orient reverse the contour */
	int side[2], z_dir, k, k2;
	bool reverse;
	uint64_t i, j, ij_ul, ij_ur, ij_ll, ij_lr;
	double fx[2], fy[2], dx, dy;
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (h);

	if (orient == 0) return;	/* Nothing to be done when no orientation specified */
	if (n < 2) return;		/* Cannot work on a single point */

	for (k = 0; k < 2; k++) {	/* Calculate fractional node numbers from left/top */
		fx[k] = (x[k] - h->wesn[XLO]) * HH->r_inc[GMT_X] - h->xy_off;
		fy[k] = (h->wesn[YHI] - y[k]) * HH->r_inc[GMT_Y] - h->xy_off;
	}

	/* Get(i,j) of the lower left node in the rectangle containing this contour segment.
	   We use the average x and y coordinate for this to avoid any round-off involved in
	   working on a single coordinate. The average coordinate should always be inside the
	   rectangle and hence the floor/ceil operators will yield the LL node. */

	i = lrint (floor (0.5 * (fx[0] + fx[1])));
	j = lrint (ceil  (0.5 * (fy[0] + fy[1])));
	ij_ll = gmt_M_ijp (h, j, i);     /* lower left corner  */
	ij_lr = gmt_M_ijp (h, j, i+1);   /* lower right corner */
	ij_ul = gmt_M_ijp (h, j-1, i);   /* upper left corner  */
	ij_ur = gmt_M_ijp (h, j-1, i+1); /* upper right corner */

	for (k = 0; k < 2; k++) {	/* Determine which edge the contour points lie on (0-3) */
		/* We KNOW that for each k, either x[k] or y[k] lies EXACTLY on a gridline.  This is used
		 * to deal with the inevitable round-off that places points slightly off the gridline.  We
		 * pick the coordinate closest to the gridline as the one that should be exactly on the gridline */

		k2 = 1 - k;	/* The other point */
		dx = fmod (fx[k], 1.0);
		if (dx > 0.5) dx = 1.0 - dx;	/* Fraction to closest vertical gridline */
		dy = fmod (fy[k], 1.0);
		if (dy > 0.5) dy = 1.0 - dy;	/* Fraction to closest horizontal gridline */
		if (dx < dy)		/* Point is on a vertical grid line (left [3] or right [1]) */
			side[k] = (fx[k] < fx[k2]) ? 3 : 1;	/* Simply check order of fx to determine which it is */
		else						/* Point must be on horizontal grid line (top [2] or bottom [0]) */
			side[k] = (fy[k] > fy[k2]) ? 0 : 2;	/* Same for fy */
	}

	switch (side[0]) {	/* Entry side: check heights of corner points.*/
	                        /* if point to the right of the line is higher z_dir = +1 else -1 */
		case 0:	/* Bottom: check heights of lower left and lower right nodes */
			z_dir = (grd[ij_lr] > grd[ij_ll]) ? +1 : -1;
			break;
		case 1:	/* Right */
			z_dir = (grd[ij_ur] > grd[ij_lr]) ? +1 : -1;
			break;
		case 2:	/* Top */
			z_dir = (grd[ij_ul] > grd[ij_ur]) ? +1 : -1;
			break;
		default:/* Left */
			z_dir = (grd[ij_ll] > grd[ij_ul]) ? +1 : -1;
			break;
	}
	reverse = (z_dir != orient);

	if (reverse) {	/* Must reverse order of contour */
		GMT_Report (GMT->parent, GMT_MSG_LONG_VERBOSE, "Change orientation of closed polygon\n");
		for (i = 0, j = n-1; i < n/2; i++, j--) {
			gmt_M_double_swap (x[i], x[j]);
			gmt_M_double_swap (y[i], y[j]);
		}
	}
}

GMT_LOCAL void shrink_clip_contours (double *x, double *y, uint64_t np, double w, double e, double s, double n) {
	/* Moves outside points to boundary.  Array length is not changed. */
	uint64_t i;

	for (i = 0; i < np; i++) {
		if (x[i] < w)
			x[i] = w;
		else if (x[i] > e)
			x[i] = e;
		if (y[i] < s)
			y[i] = s;
		else if (y[i] > n)
			y[i] = n;
	}
}

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSMASK_CTRL *C = NULL;
	
	C = gmt_M_memory (GMT, NULL, 1, struct PSMASK_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	gmt_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);
		
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSMASK_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->D.file);	
	gmt_M_str_free (C->L.file);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <table> %s %s\n", name, GMT_I_OPT, GMT_J_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [%s] [-C] [-D<template>] [-F[l|r]] [-G<fill>]\n\t%s[-L<grid>[+i|o]] [-N] %s%s[-Q<min>] [-S%s] [-T]\n", GMT_Rgeoz_OPT, GMT_B_OPT, API->K_OPT, API->O_OPT, API->P_OPT, GMT_RADIUS_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] %s[%s] [%s]\n\t[%s] [%s]\n", GMT_Y_OPT, GMT_b_OPT, API->c_OPT, GMT_d_OPT, GMT_e_OPT, GMT_h_OPT, GMT_i_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n", GMT_p_OPT, GMT_r_OPT, GMT_s_OPT, GMT_t_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "I,J-Z,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<,B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Terminate existing clip-path.  No other options required.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Dump clip polygons as data polygons; no plotting takes place.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append filename template which may contain a C-format specifier.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no filename template is given we write all polygons to stdout.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If filename has no specifiers then we write all polygons to a single file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If an integer format (e.g., %%06d) is found we substitute a running segment count\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and write all polygons to individual files; see manual page for more examples.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Cannot be used with -T; see -Q to eliminate small polygons.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Force clip contours to be oriented so that the higher z-values\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   are to the left (-Fl [Default]) or right (-Fr) as we move along\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the contour lines [Default is not oriented].\n");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Select fill color/pattern [Default is no fill].");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Save internal on/off node grid to <grid> for testing [no grid saved].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +i to change inside nodes to NaNs or +o to change outside nodes to NaNs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Invert the sense of the clipping [or tiling].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Do not dump contours with less than <cut> points [Dump all contours].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Ignored unless -D is set.\n");
	gmt_dist_syntax (API->GMT, 'S', "Set search radius to identify inside points.");
	GMT_Message (API, GMT_TIME_NONE, "\t   This means nodes inside circles of <radius> centered on the input\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   data points are considered to be reliable estimates of the surface.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is -S0, i.e., only the nearest node is considered reliable.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Paint tiles [Default will trace data outline].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If set you must also specify a color/fill with -G.\n");
	GMT_Option (API, "U,V,X,bi2,bo,c,d,e,h,i,p,r,s,t,:,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSMASK_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psmask and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int n_plus = -1, k;
	unsigned int n_errors = 0;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;
	char *c = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':
				Ctrl->C.active = true;
				break;
			case 'D':	/* Dump the polygons to files */
				Ctrl->D.active = true;

				if (gmt_M_compat_check (GMT, 4)) {
					for (n_plus = -1, k = 0; opt->arg[k]; k++) {
						if (opt->arg[k] == '+' && opt->arg[k+1] == 'n') {
							GMT_Report (API, GMT_MSG_COMPAT, "Option -D..+n<min> is deprecated; use -Q instead.\n");
							Ctrl->Q.min = atoi (&opt->arg[k + 2]);
							Ctrl->Q.active = true;
							n_plus = k;
							break;
						}
					}

					if (n_plus >= 0) opt->arg[n_plus] = '\0';	/* If extra option rip it before check if there is a prefix */
				}
				if (opt->arg[0]) Ctrl->D.file = strdup (opt->arg);
				if (n_plus >= 0) opt->arg[n_plus] = '+';	/* Restore it */
				break;
			case 'F':	/* Orient the clip contours */
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case '\0': case 'l':
						Ctrl->F.value = -1;
						break;
					case 'r':
						Ctrl->F.value = +1;
						break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -F[l|r]\n");
						break;
				}
				break;
			case 'G':
				Ctrl->G.active = true;
				if (gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " ");
					n_errors++;
				}
				break;
			case 'I':
				n_errors += gmt_parse_inc_option (GMT, 'I', opt->arg);
				break;
			case 'L':
				Ctrl->L.active = true;
				k = 0;
				if (opt->arg[0] == '-')	/* Old style leading -<name> */
					Ctrl->L.mode = PSMASK_INSIDE, k = 1;
				else if (opt->arg[0] == '+')	/* Old style leading +<name> */
					Ctrl->L.mode = PSMASK_OUTSIDE, k = 1;
				else if ((c = strstr (opt->arg, "+i"))) {	/* Appended modifier +i */
					c[0] = '\0';	/* Temporarily chop off modifier */
					Ctrl->L.mode = PSMASK_INSIDE;
				}
				else if ((c = strstr (opt->arg, "+o"))) {	/* Appended modifier +o */
					c[0] = '\0';	/* Temporarily chop off modifier */
					Ctrl->L.mode = PSMASK_OUTSIDE;
				}
				if (opt->arg[k]) Ctrl->L.file = strdup (&opt->arg[k]);
				if (c) c[0] = '+';	/* Restore modifier */
				break;
			case 'N':
				Ctrl->N.active = true;
				break;
			case 'Q':
				Ctrl->Q.active = true;
				Ctrl->Q.min = atoi (opt->arg);
				break;
			case 'S':	/* Radius of influence */
				Ctrl->S.active = true;
				Ctrl->S.mode = gmt_get_distance (GMT, opt->arg, &(Ctrl->S.radius), &(Ctrl->S.unit));
				break;
			case 'T':
				Ctrl->T.active = true;
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->C.active) {
		n_errors += gmt_M_check_condition (GMT, GMT->common.B.active[0] || GMT->common.B.active[1], "Syntax error: Cannot specify -B option in -C mode\n");
	}
	else {
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Syntax error: Must specify -R option\n");
		n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active && !Ctrl->D.active, "Syntax error: Must specify a map projection with the -J option\n");
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[ISET], "Syntax error: Must specify -I option\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && !gmt_M_is_rect_graticule(GMT), "Syntax error -T option: Only available with Linear, Mercator, or basic cylindrical projections\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->T.active && !(Ctrl->G.fill.rgb[0] >= 0 || Ctrl->G.fill.use_pattern), "Syntax error -T option: Must also specify a tile fill with -G\n");
		n_errors += gmt_M_check_condition (GMT, GMT->common.R.active[ISET] && (GMT->common.R.inc[GMT_X] <= 0.0 || GMT->common.R.inc[GMT_Y] <= 0.0), "Syntax error -I option: Must specify positive increments\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -1, "Syntax error -S: Unrecognized unit\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -2, "Syntax error -S: Unable to decode radius\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.mode == -3, "Syntax error -S: Radius is negative\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->T.active, "Syntax error: -D cannot be used with -T\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && Ctrl->L.file == NULL, "Syntax error: -L requires an output gridfile name\n");
		n_errors += gmt_check_binary_io (GMT, 2);
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_mask (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_NORMAL, "Shared GMT module not found: mask\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psmask (V_API, mode, args);
}

int GMT_psmask (void *V_API, int mode, void *args) {
	unsigned int section, k, row, col, n_edges, *d_col = NULL, d_row = 0;
	unsigned int io_mode = GMT_WRITE_SET, max_d_col = 0, ii, jj, i_start, j_start, first = 1;
	unsigned int fmt[3] = {0, 0, 0}, cont_counts[2] = {0, 0}, *edge = NULL;
	int error = 0;
	bool node_only, make_plot, closed;

	uint64_t ij, n_seg = 0, n_read, n;

	size_t n_seg_alloc = 0;

	char *grd = NULL;

	double *in = NULL, distance, x0, y0;
	double inc2[2], *x = NULL, *y = NULL, *grd_x0 = NULL, *grd_y0 = NULL;

	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_GRID *Grid = NULL;
	struct GMT_RECORD *In = NULL;
	struct PSMASK_INFO info;
	struct PSMASK_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psmask main code ----------------------------*/

	make_plot = !Ctrl->D.active;	/* Turn off plotting if -D was used */

	/* Now test also if we must provide a default -J */
	if (Ctrl->D.active && GMT->current.proj.projection < 0) {	/* Is this the right way of testing it? */
		gmt_parse_common_options (GMT, "J", 'J', "x1d");	/* Fake linear projection */
	}

	if (!Ctrl->C.active && make_plot && gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_PROJECTION_ERROR);

	if (Ctrl->D.active) {	/* Want to dump the x-y contour lines of the mask */
		uint64_t dim[GMT_DIM_SIZE] = {1, 0, 0, 2};
		if (Ctrl->D.file && strchr (Ctrl->D.file, '%')) {	/* File given and filename has C-format specifier */
			/* Must determine the kind of output organization */
			k = 0;
			while (Ctrl->D.file[k]) {
				if (Ctrl->D.file[k++] == '%') {	/* Start of format */
					while (Ctrl->D.file[k] && !strchr ("d", Ctrl->D.file[k])) k++;	/* Scan past any format modifiers, like in %04d */
					if (Ctrl->D.file[k] == 'd') fmt[1] = k;
					k++;
				}
			}
			if (fmt[1]) io_mode = GMT_WRITE_SEGMENT;	/* d: Want individual files with running numbers */
		}
		if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);	/* An empty table */
		if ((error = GMT_Set_Columns (API, GMT_OUT, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) Return (error);
	}
	
	if (make_plot) {
		if (Ctrl->C.active)
			GMT->current.ps.nclip = -1;	/* Signal that this program terminates clipping that initiated prior to this process */
		else if (!Ctrl->T.active)
			GMT->current.ps.nclip = +1;	/* Signal that this program initiates clipping that will outlive this process */
		if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	}


	if (Ctrl->C.active) {	/* Just undo previous polygon clip-path */
		PSL_endclipping (PSL, 1);
		gmt_map_basemap (GMT);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Clipping off!\n");
	}
	else {	/* Start new clip_path */
		gmt_M_memset (inc2, 2, double);
		gmt_M_memset (&info, 1, struct PSMASK_INFO);
		info.first_dump = true;

		if ((Grid = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, NULL, NULL, \
			GMT_GRID_DEFAULT_REG, 1, NULL)) == NULL) Return (API->error);	/* Specifically only need 1 row/col padding */
		
		inc2[GMT_X] = 0.5 * Grid->header->inc[GMT_X];
		inc2[GMT_Y] = 0.5 * Grid->header->inc[GMT_Y];
		
		if (make_plot) {
			gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
			gmt_plotcanvas (GMT);	/* Fill canvas if requested */
		}

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Allocate memory, read and process data file\n");

		/* Enlarge region by 1 row/column */

		Grid->header->wesn[XLO] -= Grid->header->inc[GMT_X];	Grid->header->wesn[XHI] += Grid->header->inc[GMT_X];	Grid->header->wesn[YLO] -= Grid->header->inc[GMT_Y];	Grid->header->wesn[YHI] += Grid->header->inc[GMT_Y];
		gmt_set_pad (GMT, 0U);		/* Change default pad to 0 only */
		gmt_M_grd_setpad (GMT, Grid->header, GMT->current.io.pad);	/* Change header pad to 0 */
		gmt_set_grddim (GMT, Grid->header);	/* Recompute dimensions of array */
		/* We allocate a 1-byte array separately instead of the 4-byte gmt_grdfloat array that GMT_Create_Data would have given us */
		grd = gmt_M_memory (GMT, NULL, Grid->header->size, char);	/* Only need char array to store 0 and 1 */

		if (Ctrl->S.active) {	/* Need distance calculations in correct units, and the d_row/d_col machinery */
			gmt_init_distaz (GMT, Ctrl->S.unit, Ctrl->S.mode, GMT_MAP_DIST);
			d_col = gmt_prep_nodesearch (GMT, Grid, Ctrl->S.radius, Ctrl->S.mode, &d_row, &max_d_col);
		}
		grd_x0 = gmt_grd_coord (GMT, Grid->header, GMT_X);
		grd_y0 = gmt_grd_coord (GMT, Grid->header, GMT_Y);

		/* Add GMT_CONV8_LIMIT to ensure that special case radius = inc --> lrint(0.5) actually rounds to 1 */
		
		node_only = (max_d_col == 0 && d_row == 0);
		if (node_only && Ctrl->S.radius > 0.0) {
			GMT_Report (API, GMT_MSG_VERBOSE, "Your search radius is too small to have any effect and is ignored.\n");
		}
		
		if ((error = GMT_Set_Columns (API, GMT_IN, 2, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			Return (error);
		}
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
			Return (API->error);
		}

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input table data\n");
		n_read = 0;
		do {	/* Keep returning records until we reach EOF */
			n_read++;
			if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (gmt_M_rec_is_any_header (GMT)) 	/* Skip all table and segment headers */
					continue;
				if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
					break;
			}
			in = In->data;	/* Only need to process numerical part here */

			if (gmt_M_y_is_outside (GMT, in[GMT_Y], Grid->header->wesn[YLO], Grid->header->wesn[YHI])) continue;	/* Outside y-range */
			if (gmt_x_is_outside (GMT, &in[GMT_X], Grid->header->wesn[XLO], Grid->header->wesn[XHI])) continue;	/* Outside x-range (or longitude) */

			/* Data record to process */

			/* Determine the node closest to the data point */

			if (gmt_row_col_out_of_bounds (GMT, in, Grid->header, &row, &col)) continue;			/* Outside grid node range */

			if (node_only) {
				ij = gmt_M_ijp (Grid->header, row, col);
				grd[ij] = 1;
			}
			else {	/* Set coordinate of this node */
				if (gmt_M_is_geographic (GMT, GMT_IN)) {	/* Make special checks for N and S poles */
					if (gmt_M_is_Npole (in[GMT_Y])) {	/* N pole */
						if (Ctrl->S.radius == 0.0) {	/* Only set the N pole row */
							gmt_M_col_loop (GMT, Grid, 0, col, ij)	/* Set this entire N row */
								grd[ij] = 1;
							continue;
						}
						for (row = 0; row < Grid->header->n_rows && (distance = gmt_distance (GMT, 0.0, 90.0, grd_x0[0], grd_y0[row])) <= Ctrl->S.radius; row++) {
							gmt_M_col_loop (GMT, Grid, row, col, ij)	/* Set this entire row */
								grd[ij] = 1;
						}
						continue;
					}
					else if (gmt_M_is_Spole (in[GMT_Y])) {	/* S pole */
						if (Ctrl->S.radius == 0.0) {	/* Only set the S pole row */
							gmt_M_col_loop (GMT, Grid, Grid->header->n_rows - 1, col, ij)	/* Set this entire S row */
								grd[ij] = 1;
							continue;
						}
						for (row = Grid->header->n_rows; row > 0 && (distance = gmt_distance (GMT, 0.0, -90.0, grd_x0[0], grd_y0[row-1])) <= Ctrl->S.radius; row--) {
							gmt_M_col_loop (GMT, Grid, row-1, col, ij)	/* Set this entire row */
								grd[ij] = 1;
						}
						continue;
					}
				}

				x0 = gmt_M_grd_col_to_x (GMT, col, Grid->header);
				y0 = gmt_M_grd_row_to_y (GMT, row, Grid->header);

				/* Set this and all nodes within radius distance to 1 */

				j_start = (row > d_row) ? row - d_row : 0;
				for (jj = j_start; jj <= row + d_row; jj++) {
					if (jj >= Grid->header->n_rows) continue;
					i_start = (col > d_col[jj]) ? col - d_col[jj] : 0;
					for (ii = i_start; ii <= col + d_col[jj]; ii++) {
						if (ii >= Grid->header->n_columns) continue;
						distance = gmt_distance (GMT, x0, y0, grd_x0[ii], grd_y0[jj]);
						if (distance > Ctrl->S.radius) continue;
						ij = gmt_M_ijp (Grid->header, jj, ii);
						grd[ij] = 1;
					}
				}
			}
		} while (true);
		
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}

		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Read %" PRIu64 " data points\n", n_read);

		if (Ctrl->N.active) for (ij = 0; ij < Grid->header->size; ij++) grd[ij] = 1 - grd[ij];	/* Reverse sense of test */

		/* Force perimeter nodes to be false; thus all contours will be closed */

		for (col = 0, ij = (Grid->header->n_rows-1) * (uint64_t)Grid->header->n_columns; col < Grid->header->n_columns; col++) grd[col] = grd[col+ij] = 0;
		for (row = 0; row < Grid->header->n_rows; row++) grd[row*Grid->header->n_columns] = grd[(row+1)*Grid->header->n_columns-1] = 0;

		if (Ctrl->L.active) {	/* Save a copy of the grid to file */
			struct GMT_GRID *G = NULL;
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Saving internal inside|outside grid to file %s\n", Ctrl->L.file);
			if ((G = GMT_Create_Data (API, GMT_IS_GRID, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Grid->header->wesn, Grid->header->inc, \
				Grid->header->registration, 0, NULL)) == NULL) Return (API->error);
			for (ij = 0; ij < Grid->header->size; ij++) {	/* Copy over the 0/1 grid */
				switch (Ctrl->L.mode) {
					case 0:	/* As is */
						G->data[ij] = (gmt_grdfloat)grd[ij];
						break;
					case +1:	/* Replace zeros by NaNs */
						G->data[ij] = (grd[ij] == 0) ? GMT->session.f_NaN : (gmt_grdfloat)grd[ij];
						break;
					case -1:	/* Replace ones by NaNs */
						G->data[ij] = (grd[ij] == 1) ? GMT->session.f_NaN : (gmt_grdfloat)grd[ij];
						break;
				}
			}
			if (GMT_Write_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->L.file, G) != GMT_NOERROR) {
				Return (API->error);
			}
			if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
				Return (API->error);
			}
		}
		if (!Ctrl->T.active) {	/* Must trace the outline of ON/OFF values in grd */
			uint64_t max_alloc_points = GMT_CHUNK;
			struct GMT_DATATABLE_HIDDEN *TH = NULL;
			struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
			/* Arrays holding the contour xy values; preallocate space */
			x = gmt_M_memory (GMT, NULL, max_alloc_points, double);
			y = gmt_M_memory (GMT, NULL, max_alloc_points, double);

			n_edges = Grid->header->n_rows * (urint (ceil (Grid->header->n_columns / 16.0)));
			edge = gmt_M_memory (GMT, NULL, n_edges, unsigned int);

			if (make_plot) gmt_map_basemap (GMT);

			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Tracing the clip path\n");

			section = 0;
			first = 1;
			while ((n = clip_contours (GMT, &info, grd, Grid->header, inc2, edge, first, &x, &y, &max_alloc_points)) > 0) {
				closed = false;
				shrink_clip_contours (x, y, n, Grid->header->wesn[XLO], Grid->header->wesn[XHI], Grid->header->wesn[YLO], Grid->header->wesn[YHI]);
				if (Ctrl->F.active) orient_contours (GMT, Grid->header, grd, x, y, n, Ctrl->F.value);
				if (Ctrl->D.active && n > (uint64_t)Ctrl->Q.min) {	/* Save the contour as output data */
					S = gmt_prepare_contour (GMT, x, y, n, GMT->session.d_NaN);
					/* Select which table this segment should be added to */
					if (n_seg == n_seg_alloc) {
						size_t n_old_alloc = n_seg_alloc;
						D->table[0]->segment = gmt_M_memory (GMT, D->table[0]->segment, (n_seg_alloc += GMT_SMALL_CHUNK), struct GMT_DATASEGMENT *);
						gmt_M_memset (&(D->table[0]->segment[n_old_alloc]), n_seg_alloc - n_old_alloc, struct GMT_DATASEGMENT *);	/* Set to NULL */
					}
					D->table[0]->segment[n_seg++] = S;
					D->table[0]->n_segments++;	D->n_segments++;
					D->table[0]->n_records += n;	D->n_records += n;
					/* Generate a file name and increment cont_counts, if relevant */
					TH = gmt_get_DT_hidden (D->table[0]);
					if (io_mode == GMT_WRITE_SET && !TH->file[GMT_OUT])
						TH->file[GMT_OUT] = gmt_make_filename (GMT, Ctrl->D.file, fmt, GMT->session.d_NaN, closed, cont_counts);
					else if (io_mode == GMT_WRITE_SEGMENT) {
						SH = gmt_get_DS_hidden (S);
						SH->file[GMT_OUT] = gmt_make_filename (GMT, Ctrl->D.file, fmt, GMT->session.d_NaN, closed, cont_counts);
					}
				}
				if (make_plot) draw_clip_contours (GMT, PSL, x, y, n, Ctrl->G.fill.rgb, section, first);
				first = 0;
				section++;
			}

			if (make_plot) draw_clip_contours (GMT, PSL, x, y, 0, Ctrl->G.fill.rgb, section, 2);	/* Activate clip-path */

			gmt_M_free (GMT, edge);
			gmt_M_free (GMT, x);
			gmt_M_free (GMT, y);
			if (Ctrl->D.active) {	/* Write the clip polygon file(s) */
				if (n_seg > 1) gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Set -mo if > 1 segment */
				D->table[0]->segment = gmt_M_memory (GMT, D->table[0]->segment, n_seg, struct GMT_DATASEGMENT *);
				if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, io_mode, NULL, Ctrl->D.file, D) != GMT_NOERROR) {
					Return (API->error);
				}
			}
		}
		else {	/* Just paint tiles */
			uint64_t start, n_use, np, plot_n;
			double y_bot, y_top, *xx = NULL, *yy = NULL, *xp = NULL, *yp = NULL;
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Tiling...\n");

			for (row = 0; row < Grid->header->n_rows; row++) {
				y_bot = grd_y0[row] - inc2[GMT_Y];
				y_top = grd_y0[row] + inc2[GMT_Y];
				ij = gmt_M_ijp (Grid->header, row, 0);
				for (col = 0; col < Grid->header->n_columns; col++, ij++) {
					if (grd[ij] == 0) continue;

					np = gmt_graticule_path (GMT, &xx, &yy, 1, true, grd_x0[col] - inc2[GMT_X], grd_x0[col] + inc2[GMT_X], y_bot, y_top);
					plot_n = gmt_clip_to_map (GMT, xx, yy, np, &xp, &yp);
					gmt_M_free (GMT, xx);
					gmt_M_free (GMT, yy);
					if (plot_n == 0) continue;	/* Outside */
					
					gmt_setfill (GMT, &Ctrl->G.fill, false);
					if ((*GMT->current.map.will_it_wrap) (GMT, xp, yp, plot_n, &start)) {	/* Polygon wraps */

						/* First truncate against left border */

						GMT->current.plot.n = gmt_map_truncate (GMT, xp, yp, plot_n, start, -1);
						n_use = gmt_compact_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n, false, 0);
						PSL_plotpolygon (PSL, GMT->current.plot.x, GMT->current.plot.y, (int)n_use);

						/* Then truncate against right border */

						GMT->current.plot.n = gmt_map_truncate (GMT, xp, yp, plot_n, start, +1);
						n_use = gmt_compact_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n, false, 0);
						PSL_plotpolygon (PSL, GMT->current.plot.x, GMT->current.plot.y, (int)n_use);
					}
					else
						PSL_plotpolygon (PSL, xp, yp, (int)plot_n);
					gmt_M_free (GMT, xp);
					gmt_M_free (GMT, yp);
				}
			}
			gmt_map_basemap (GMT);
		}

		if (make_plot) gmt_plane_perspective (GMT, -1, 0.0);

		gmt_M_free (GMT, grd);
		if (GMT_Destroy_Data (API, &Grid) != GMT_NOERROR) {
			GMT_Report (API, GMT_MSG_NORMAL, "Failed to free Grid\n");
		}
		if (Ctrl->S.active) gmt_M_free (GMT, d_col);
		gmt_M_free (GMT, grd_x0);
		gmt_M_free (GMT, grd_y0);
		if (make_plot && !Ctrl->T.active) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Clipping on!\n");
	}

	gmt_set_pad (GMT, API->pad);		/* Reset default pad */
	if (make_plot) gmt_plotend (GMT);

	Return (GMT_NOERROR);
}
