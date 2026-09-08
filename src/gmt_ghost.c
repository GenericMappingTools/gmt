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
/*
 * gmt_ghost.c contains the machinery for holding a grid's boundary halo
 * outside the data matrix, so that G->data can be a plain contiguous
 * n_columns * n_rows array (issue #4358).  See gmt_ghost.h for the layout and
 * for the accessors; this file only allocates, frees and converts.
 *
 * Author:	The GMT team
 * Date:	1-JAN-2026
 * Version:	6 API
 */

#include "gmt_dev.h"
#include "gmt_internals.h"

/*! Is the ghost-cell layout wanted for this session?  For now this is a plain
 * environment switch (GMT_GHOST_CELLS=1) so that the whole test suite can be
 * run in either layout from a single build.  Once the migration is complete
 * this becomes "the external handed us a pad-less matrix". */
GMT_LOCAL int gmtghost_level (void) {
	/* 0 = off (legacy pad), 1 = grids move to ghost cells once their BCs are set,
	 * 2 = as 1, and new grids are created without a pad as well */
	static int level = -1;
	char *c = NULL;
	if (level >= 0) return (level);
	c = getenv ("GMT_GHOST_CELLS");
	if (c == NULL || c[0] == '\0' || !strcmp (c, "0")) level = 0;
	else if (!strcmp (c, "2")) level = 2;
	else level = 1;
	return (level);
}

static unsigned int gmtghost_suspended = 0;

/*! Suspend and resume the move to ghost cells around a section of code that must
 * see padded grids.  Cubes are the case that needs it: gmtapi_import_cube reads
 * each layer as an ordinary grid and then copies it into the cube striding by the
 * padded layer size, so a layer that had moved to ghost cells would be copied into
 * the wrong place.  Cubes are not part of the migration yet. */
void gmtlib_ghost_suspend (bool on) {
	if (on) gmtghost_suspended++;
	else if (gmtghost_suspended) gmtghost_suspended--;
}

/*! True while inside such a section */
bool gmtlib_ghost_is_suspended (void) {
	return (gmtghost_suspended > 0);
}

bool gmtlib_ghost_wanted (struct GMT_CTRL *GMT) {
	gmt_M_unused (GMT);
	return (gmtghost_level () > 0);
}

/*! True when new grids should also be created pad-free.  Kept as a separate
 * level so the two steps can be tested apart: level 1 only changes grids that
 * arrive from outside, level 2 changes every grid GMT makes. */
bool gmtlib_ghost_no_new_pad (struct GMT_CTRL *GMT) {
	gmt_M_unused (GMT);
	return (gmtghost_level () > 1);
}

/*! Allocate the four halo slabs of a grid as one block.  The header's own pad
 * is not consulted: the caller says how deep a halo it wants. */
struct GMT_GRID_GHOST *gmtlib_ghost_alloc (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, unsigned int pad[]) {
	struct GMT_GRID_GHOST *g = NULL;
	uint64_t n_N, n_S, n_W, n_E;

	if (h == NULL) return (NULL);
	if ((g = gmt_M_memory (GMT, NULL, 1U, struct GMT_GRID_GHOST)) == NULL) return (NULL);
	gmt_M_memcpy (g->pad, pad, 4, unsigned int);
	g->mode = GMT_BC_IS_NOTSET;
	g->mxg = (uint64_t)pad[XLO] + h->n_columns + pad[XHI];
	n_N = (uint64_t)pad[YHI] * g->mxg;
	n_S = (uint64_t)pad[YLO] * g->mxg;
	n_W = (uint64_t)pad[XLO] * h->n_rows;
	n_E = (uint64_t)pad[XHI] * h->n_rows;
	g->n_alloc = n_N + n_S + n_W + n_E;
	if (g->n_alloc == 0) {	/* A halo of depth zero on every side; nothing to hold */
		gmt_M_free (GMT, g);
		return (NULL);
	}
	if ((g->alloc = gmt_M_memory (GMT, NULL, g->n_alloc, gmt_grdfloat)) == NULL) {
		gmt_M_free (GMT, g);
		return (NULL);
	}
	g->N = g->alloc;		/* Slice the one block into the four slabs */
	g->S = g->N + n_N;
	g->W = g->S + n_S;
	g->E = g->W + n_W;
	return (g);
}

/*! Release a grid's halo, if it has one */
void gmtlib_ghost_free (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h) {
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_GRID_GHOST *g = NULL;

	if (h == NULL || h->hidden == NULL) return;
	HH = gmt_get_H_hidden (h);
	if ((g = HH->ghost) == NULL) return;
	HH->ghost = NULL;
	gmt_M_free (GMT, g->alloc);
	gmt_M_free (GMT, g);
}

/*! Give "to" a copy of "from"'s halo.  A duplicated grid must carry its halo
 * with it: recomputing the boundary conditions would be wrong whenever the halo
 * holds real data from a parent grid (GMT_BC_IS_DATA), which is precisely the
 * accuracy the issue does not want to lose. */
int gmtlib_ghost_duplicate (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *to, struct GMT_GRID_HEADER *from) {
	struct GMT_GRID_HEADER_HIDDEN *Hto = NULL, *Hfrom = NULL;
	struct GMT_GRID_GHOST *g = NULL, *gnew = NULL;

	if (to == NULL || from == NULL) return (GMT_NOERROR);
	Hfrom = gmt_get_H_hidden (from);
	if ((g = Hfrom->ghost) == NULL) return (GMT_NOERROR);	/* Nothing to copy */
	Hto = gmt_get_H_hidden (to);
	gmtlib_ghost_free (GMT, to);	/* In case it already had one */
	if ((gnew = gmtlib_ghost_alloc (GMT, to, g->pad)) == NULL) return (GMT_MEMORY_ERROR);
	if (gnew->n_alloc != g->n_alloc) {	/* Different shape; the halo does not transfer */
		gmt_M_free (GMT, gnew->alloc);
		gmt_M_free (GMT, gnew);
		return (GMT_NOERROR);
	}
	gmt_M_memcpy (gnew->alloc, g->alloc, g->n_alloc, gmt_grdfloat);
	gnew->mode = g->mode;
	Hto->ghost = gnew;
	return (GMT_NOERROR);
}

/*! Move a padded grid to the ghost-cell layout: copy the pad into the halo
 * slabs, then compact the data matrix so that G->data is contiguous and
 * h->pad is zero.  The values are copied, not recomputed, so the halo holds
 * exactly what the pad held - that is what makes the two layouts comparable
 * bit for bit. */
int gmtlib_ghost_from_pad (struct GMT_CTRL *GMT, struct GMT_GRID *G) {
	struct GMT_GRID_HEADER *h = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_GRID_GHOST *g = NULL;
	unsigned int pad[4];
	int64_t row, col, col_lo, col_hi;
	gmt_grdfloat *slot = NULL;

	if (G == NULL || G->data == NULL || (h = G->header) == NULL) return (GMT_NOERROR);
	HH = gmt_get_H_hidden (h);
	if (HH->ghost) return (GMT_NOERROR);	/* Already in the ghost layout */
	if (HH->no_ghost) return (GMT_NOERROR);	/* A module has asked for a padded matrix and we must not take it away again */
	if (gmtghost_suspended) return (GMT_NOERROR);	/* Inside a section that needs padded grids */
	if (h->complex_mode & GMT_GRID_IS_COMPLEX_MASK) return (GMT_NOERROR);	/* Complex grids keep their pad for now */
	gmt_M_memcpy (pad, h->pad, 4, unsigned int);
	if ((pad[XLO] + pad[XHI] + pad[YLO] + pad[YHI]) == 0) return (GMT_NOERROR);	/* No halo to preserve */
	/* Only a genuine boundary halo is moved.  A few modules use the pad as working
	 * space rather than as boundary conditions - grdpaste positions two grids inside
	 * one array by giving each a pad the height of the other, and the FFT modules pad
	 * out to a convenient transform size - and those arrays must be left alone.  A BC
	 * pad is always small and symmetric, so that is the test. */
	if (pad[XLO] != pad[XHI] || pad[YLO] != pad[YHI] || pad[XLO] > 2 || pad[YLO] > 2) {
		if (getenv ("GMT_GHOST_DEBUG"))
			fprintf (stderr, "gmtlib_ghost_from_pad: pad %u/%u/%u/%u is working space, not a halo; left alone\n",
				pad[XLO], pad[XHI], pad[YLO], pad[YHI]);
		return (GMT_NOERROR);
	}
	if ((g = gmtlib_ghost_alloc (GMT, h, pad)) == NULL) return (GMT_MEMORY_ERROR);

	col_lo = -(int64_t)pad[XLO];
	col_hi = (int64_t)(h->n_columns + pad[XHI]);
	/* Read through gmt_M_ijp while the grid is still padded and HH->ghost is still NULL */
	for (row = -(int64_t)pad[YHI]; row < 0; row++)	/* North slab, corners included */
		for (col = col_lo; col < col_hi; col++)
			if ((slot = gmt_ghost_slot (h, g, row, col))) *slot = G->data[gmt_M_ijp (h, row, col)];
	for (row = (int64_t)h->n_rows; row < (int64_t)(h->n_rows + pad[YLO]); row++)	/* South slab, corners included */
		for (col = col_lo; col < col_hi; col++)
			if ((slot = gmt_ghost_slot (h, g, row, col))) *slot = G->data[gmt_M_ijp (h, row, col)];
	for (row = 0; row < (int64_t)h->n_rows; row++) {	/* West and east sides on the interior rows */
		for (col = col_lo; col < 0; col++)
			if ((slot = gmt_ghost_slot (h, g, row, col))) *slot = G->data[gmt_M_ijp (h, row, col)];
		for (col = (int64_t)h->n_columns; col < col_hi; col++)
			if ((slot = gmt_ghost_slot (h, g, row, col))) *slot = G->data[gmt_M_ijp (h, row, col)];
	}
	g->mode = HH->BC[XLO];	/* Remember what kind of halo this is */

	gmt_grd_pad_off (GMT, G);	/* Compacts the data and zeroes h->pad, so mx becomes n_columns */
	HH->ghost = g;			/* Only now does the accessor switch to the ghost path */
	if (getenv ("GMT_GHOST_DEBUG"))
		fprintf (stderr, "gmtlib_ghost_from_pad: %s [%u x %u] moved to ghost cells, halo %u/%u/%u/%u\n",
			HH->name[0] ? HH->name : "<memory>", h->n_columns, h->n_rows, pad[XLO], pad[XHI], pad[YLO], pad[YHI]);
	return (GMT_NOERROR);
}

/*! The inverse of gmtlib_ghost_from_pad, used to check the conversion and to
 * hand a grid to code that has not been converted yet. */
int gmtlib_ghost_to_pad (struct GMT_CTRL *GMT, struct GMT_GRID *G) {
	struct GMT_GRID_HEADER *h = NULL;
	struct GMT_GRID_HEADER_HIDDEN *HH = NULL;
	struct GMT_GRID_GHOST *g = NULL;
	int64_t row, col, col_lo, col_hi;
	gmt_grdfloat *slot = NULL;

	if (G == NULL || G->data == NULL || (h = G->header) == NULL) return (GMT_NOERROR);
	HH = gmt_get_H_hidden (h);
	if ((g = HH->ghost) == NULL) return (GMT_NOERROR);	/* Not in the ghost layout */

	HH->ghost = NULL;	/* Detach first: gmt_grd_pad_on works on the data matrix alone */
	gmt_grd_pad_on (GMT, G, g->pad);	/* Re-expands the matrix and sets h->pad */

	col_lo = -(int64_t)g->pad[XLO];
	col_hi = (int64_t)(h->n_columns + g->pad[XHI]);
	for (row = -(int64_t)g->pad[YHI]; row < 0; row++)
		for (col = col_lo; col < col_hi; col++)
			if ((slot = gmt_ghost_slot (h, g, row, col))) G->data[gmt_M_ijp (h, row, col)] = *slot;
	for (row = (int64_t)h->n_rows; row < (int64_t)(h->n_rows + g->pad[YLO]); row++)
		for (col = col_lo; col < col_hi; col++)
			if ((slot = gmt_ghost_slot (h, g, row, col))) G->data[gmt_M_ijp (h, row, col)] = *slot;
	for (row = 0; row < (int64_t)h->n_rows; row++) {
		for (col = col_lo; col < 0; col++)
			if ((slot = gmt_ghost_slot (h, g, row, col))) G->data[gmt_M_ijp (h, row, col)] = *slot;
		for (col = (int64_t)h->n_columns; col < col_hi; col++)
			if ((slot = gmt_ghost_slot (h, g, row, col))) G->data[gmt_M_ijp (h, row, col)] = *slot;
	}
	gmt_M_free (GMT, g->alloc);
	gmt_M_free (GMT, g);
	return (GMT_NOERROR);
}
