/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2026 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
/*!
 * \file gmt_ghost.h
 * \brief Ghost cells: the grid halo held outside the data matrix.
 *
 * Historically GMT stores an n_columns x n_rows grid inside a larger
 * mx x my matrix so that boundary condition values live in a pad around the
 * data.  That makes the interior a *strided* sub-rectangle, which is why grids
 * cannot be shared with Julia/Python/MATLAB without a copy (see issue #4358).
 *
 * Ghost cells keep the boundary values but move them out of the data matrix:
 * G->data becomes a plain contiguous n_columns * n_rows array (h->pad is all
 * zero, so mx == n_columns and gmt_M_ijp degenerates to row * n_columns + col)
 * and the halo lives in four slabs hanging off the header.
 *
 * Because gmt_M_ijp keeps working for every interior node, converting GMT is a
 * matter of finding the code that addresses nodes *outside* the interior and
 * routing it through the accessors below.  In the legacy padded layout those
 * accessors compile down to exactly the old expression, so a converted call
 * site is bit-for-bit the old call site until the layout is switched.
 *
 * Row convention reminder: row grows southward, so row = -1 is the first ghost
 * row above the north edge and belongs to pad[YHI].
 */

#ifndef GMT_GHOST_H
#define GMT_GHOST_H

struct GMT_GRID_GHOST {	/* The halo of a grid, held outside the data matrix */
	unsigned int pad[4];	/* Number of ghost layers on each side: XLO, XHI, YLO, YHI */
	unsigned int mode;	/* The BC mode that filled it [GMT_BC_IS_NOTSET until filled] */
	uint64_t mxg;		/* Width of the N and S slabs = pad[XLO] + n_columns + pad[XHI] */
	uint64_t n_alloc;	/* Number of gmt_grdfloat in the single allocation */
	gmt_grdfloat *N;	/* pad[YHI] rows of mxg, corners included */
	gmt_grdfloat *S;	/* pad[YLO] rows of mxg, corners included */
	gmt_grdfloat *W;	/* n_rows rows of pad[XLO] */
	gmt_grdfloat *E;	/* n_rows rows of pad[XHI] */
	gmt_grdfloat *alloc;	/* The one block that N, S, W and E point into */
};

/*! Address a node that lies in the halo.  Returns NULL if (row,col) is not in
 * the halo described by g, which can only happen if the caller asks for a
 * deeper halo than the grid actually has.  Kept separate from
 * gmt_grd_node_ptr so that the code that *fills* the halo can use it before
 * the halo is attached to the header. */
static inline gmt_grdfloat *gmt_ghost_slot (struct GMT_GRID_HEADER *h, struct GMT_GRID_GHOST *g, int64_t row, int64_t col) {
	if (col < -(int64_t)g->pad[XLO] || col >= (int64_t)(h->n_columns + g->pad[XHI])) return (NULL);
	if (row < 0) {	/* North of the grid; the N slab is full width so it owns the two north corners */
		if (row < -(int64_t)g->pad[YHI]) return (NULL);
		return (&g->N[(row + (int64_t)g->pad[YHI]) * (int64_t)g->mxg + col + (int64_t)g->pad[XLO]]);
	}
	if (row >= (int64_t)h->n_rows) {	/* South of the grid; likewise the S slab owns the south corners */
		if (row >= (int64_t)(h->n_rows + g->pad[YLO])) return (NULL);
		return (&g->S[(row - (int64_t)h->n_rows) * (int64_t)g->mxg + col + (int64_t)g->pad[XLO]]);
	}
	if (col < 0)	/* West of the grid, on an interior row */
		return (&g->W[row * (int64_t)g->pad[XLO] + col + (int64_t)g->pad[XLO]]);
	if (col >= (int64_t)h->n_columns)	/* East of the grid, on an interior row */
		return (&g->E[row * (int64_t)g->pad[XHI] + col - (int64_t)h->n_columns]);
	return (NULL);	/* Interior: not our business */
}

/*! True if (row,col) is a node of the grid proper rather than of the halo */
static inline bool gmt_grd_node_is_interior (struct GMT_GRID_HEADER *h, int64_t row, int64_t col) {
	return (row >= 0 && row < (int64_t)h->n_rows && col >= 0 && col < (int64_t)h->n_columns);
}

/*! Address any node in the interior or the halo.  Returns NULL if (row,col) is
 * outside both.
 *
 * A trap worth knowing about: GMT's loop counters are openmp_int, which is
 * *unsigned* everywhere except MSVC, so writing gmt_grd_get_node (h, d, row,
 * col - 1) at col 0 hands this function 4294967295 rather than -1.  Cast the
 * arguments at every call site.  Building with GMT_GHOST_STRICT turns such a
 * value into an abort instead of a wild pointer. */
static inline gmt_grdfloat *gmt_grd_node_ptr (struct GMT_GRID_HEADER *h, gmt_grdfloat *data, int64_t row, int64_t col) {
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (h);

#ifdef GMT_GHOST_STRICT
	assert (row >= -8 && col >= -8 && row < (int64_t)h->n_rows + 8 && col < (int64_t)h->n_columns + 8);
#endif
	if (HH->ghost == NULL)	/* Legacy layout: the halo is inside the matrix, so one expression covers everything */
		return (&data[gmt_M_ijp (h, row, col)]);
	if (gmt_grd_node_is_interior (h, row, col))	/* h->pad is zero here so this is row * n_columns + col */
		return (&data[gmt_M_ijp (h, row, col)]);
	return (gmt_ghost_slot (h, HH->ghost, row, col));
}

/*! Read one node, interior or halo */
static inline gmt_grdfloat gmt_grd_get_node (struct GMT_GRID_HEADER *h, gmt_grdfloat *data, int64_t row, int64_t col) {
	gmt_grdfloat *p = gmt_grd_node_ptr (h, data, row, col);
	return ((p) ? *p : (gmt_grdfloat)0.0);
}

/*! Write one node, interior or halo */
static inline void gmt_grd_put_node (struct GMT_GRID_HEADER *h, gmt_grdfloat *data, int64_t row, int64_t col, gmt_grdfloat z) {
	gmt_grdfloat *p = gmt_grd_node_ptr (h, data, row, col);
	if (p) *p = z;
}

/*! Like gmt_grd_node_ptr, but also rejects a node that falls outside the
 * allocated array in the legacy layout, so a caller can treat "not available"
 * uniformly by skipping NULL.  In the ghost layout the halo has a real edge, so
 * a node past it is simply absent; in the padded layout the same node would
 * silently wrap onto the next row's pad, which is why this check exists. */
static inline gmt_grdfloat *gmt_grd_node_ptr_checked (struct GMT_GRID_HEADER *h, gmt_grdfloat *data, int64_t row, int64_t col) {
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (h);
	uint64_t node;

	if (HH->ghost == NULL) {
		node = gmt_M_ijp (h, row, col);	/* Unsigned, so a negative index wraps and fails the test below */
		return ((node < h->size) ? &data[node] : NULL);
	}
	if (gmt_grd_node_is_interior (h, row, col)) return (&data[gmt_M_ijp (h, row, col)]);
	return (gmt_ghost_slot (h, HH->ghost, row, col));
}

/*! Get n consecutive values along a row starting at (row,col).  Returns a pointer
 * straight into data when the run is contiguous there - which is always true in
 * the legacy layout and true for interior runs in the ghost layout - and only
 * copies into the caller's buffer when the run straddles a halo. */
static inline const gmt_grdfloat *gmt_grd_get_window (struct GMT_GRID_HEADER *h, gmt_grdfloat *data, int64_t row, int64_t col, unsigned int n, gmt_grdfloat *buf) {
	struct GMT_GRID_HEADER_HIDDEN *HH = gmt_get_H_hidden (h);
	unsigned int k;

	if (HH->ghost == NULL)	/* Legacy layout: the whole run, halo included, is contiguous */
		return (&data[gmt_M_ijp (h, row, col)]);
	if (row >= 0 && row < (int64_t)h->n_rows && col >= 0 && (col + (int64_t)n) <= (int64_t)h->n_columns)
		return (&data[gmt_M_ijp (h, row, col)]);	/* Entirely interior, still contiguous */
	for (k = 0; k < n; k++) buf[k] = gmt_grd_get_node (h, data, row, col + (int64_t)k);
	return (buf);
}

#endif /* GMT_GHOST_H */
