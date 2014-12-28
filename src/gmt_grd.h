/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * grd.h contains the definition for a GMT-SYSTEM Version >= 2 grd file
 *
 * grd is stored in rows going from west (xmin) to east (xmax)
 * first row in file has yvalue = north (ymax).  
 * This is SCANLINE orientation.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#ifndef _GMT_GRID_H
#define _GMT_GRID_H

/* netcdf convention */
#define GMT_NC_CONVENTION "COARDS, CF-1.5"

enum GMT_enum_grdtype {
	/* Special cases of geographic grids with periodicity */
	GMT_GRID_CARTESIAN=0,			/* Cartesian data, no periodicity involved */
	GMT_GRID_GEOGRAPHIC_LESS360,		/* x is longitude, but range is < 360 degrees */
	GMT_GRID_GEOGRAPHIC_EXACT360_NOREPEAT,	/* x is longitude, range is 360 degrees, no repeat node */
	GMT_GRID_GEOGRAPHIC_EXACT360_REPEAT,	/* x is longitude, range is 360 degrees, gridline registered and repeat node at 360*/
	GMT_GRID_GEOGRAPHIC_MORE360		/* x is longitude, and range exceeds 360 degrees */
};

enum GMT_enum_grdlayout {	/* Grid layout for complex grids */
	GMT_GRID_IS_SERIAL = 0,		/* Grid is RRRRRR...[IIIIII...] */
	GMT_GRID_IS_INTERLEAVED = 1};	/* Grid is RIRIRIRI... - required layout for FFT */

/*
 * GMT's internal representation of grids is north-up, i.e., the index of the
 * least dimension (aka y or lat) increases to the south. NetCDF files are
 * usually written bottom-up, i.e., the index of the least dimension increases
 * from south to north (k_nc_start_south):
 *
 * k_nc_start_north:  k_nc_start_south:
 *
 * y ^ 0 1 2            -------> x
 *   | 3 4 5            | 0 1 2
 *   | 6 7 8            | 3 4 5
 *   -------> x       y V 6 7 8
 */

enum Netcdf_row_order {
	/* Order of rows in z variable */
	k_nc_start_north = -1, /* The least dimension (i.e., lat or y) decreases */
	k_nc_start_south = 1   /* The least dimension (i.e., lat or y) increases */
};

enum Netcdf_chunksize {
	k_netcdf_io_classic = 0, /* netCDF classic format */
	k_netcdf_io_chunked_auto /* netCDF 4 auto-determined optimal chunk size */
};

/* The array wesn in the header has a name that indicates the order (west, east, south, north).
 * However, to avoid using confusing indices 0-5 we define very brief constants
 * XLO, XHI, YLO, YHI, ZLO, ZHI that should be used instead: */
enum GMT_enum_wesnIDs {
	XLO = 0, /* Index for west or xmin value */
	XHI,     /* Index for east or xmax value */
	YLO,     /* Index for south or ymin value */
	YHI,     /* Index for north or ymax value */
	ZLO,     /* Index for zmin value */
	ZHI      /* Index for zmax value */
};

/* These macros should be used to convert between (column,row) and (x,y).  It will eliminate
 * one source of typos and errors, and since macros are done at compilation time there is no
 * overhead.  Note: gmt_x_to_col does not need nx but we included it for symmetry reasons.
 * gmt_y_to_row must first compute j', the number of rows in the increasing y-direction (to
 * match the sense of truncation used for x) then we revert to row number increasing down
 * by flipping: j = ny - 1 - j'.
 * Note that input col, row _may_ be negative, hence we do the cast to (int) here. */

#define gmt_x_to_col(x,x0,dx,off,nx) (irint((((x) - (x0)) / (dx)) - (off)))
#define gmt_y_to_row(y,y0,dy,off,ny) ((ny) - 1 - irint(((((y) - (y0)) / (dy)) - (off))))
#define GMT_col_to_x(C,col,x0,x1,dx,off,nx) (((int)(col) == (int)((nx)-1)) ? (x1) - (off) * (dx) : (x0) + ((col) + (off)) * (dx))
#define GMT_row_to_y(C,row,y0,y1,dy,off,ny) (((int)(row) == (int)((ny)-1)) ? (y0) + (off) * (dy) : (y1) - ((row) + (off)) * (dy))

/* The follow macros simplify using the 4 above macros when all info is in the struct header h. */

#define GMT_grd_col_to_x(C,col,h) GMT_col_to_x(C,col,h->wesn[XLO],h->wesn[XHI],h->inc[GMT_X],h->xy_off,h->nx)
#define GMT_grd_row_to_y(C,row,h) GMT_row_to_y(C,row,h->wesn[YLO],h->wesn[YHI],h->inc[GMT_Y],h->xy_off,h->ny)
#define GMT_grd_x_to_col(C,x,h) gmt_x_to_col(x,h->wesn[XLO],h->inc[GMT_X],h->xy_off,h->nx)
#define GMT_grd_y_to_row(C,y,h) gmt_y_to_row(y,h->wesn[YLO],h->inc[GMT_Y],h->xy_off,h->ny)

/* These macros calculate the number of nodes in x or y for the increment dx, dy */

#define GMT_get_n(C,min,max,inc,off) (urint ((((max) - (min)) / (inc)) + 1 - (off)) )
#define GMT_get_inc(C,min,max,n,off) (((max) - (min)) / ((n) + (off) - 1))

/* The follow macros simplify using the 2 above macros when all info is in the struct header */

#define GMT_grd_get_nx(C,h) (GMT_get_n(C,h->wesn[XLO],h->wesn[XHI],h->inc[GMT_X],h->registration))
#define GMT_grd_get_ny(C,h) (GMT_get_n(C,h->wesn[YLO],h->wesn[YHI],h->inc[GMT_Y],h->registration))

/* The follow macros gets the full length or rows and columns when padding is considered (i.e., mx and my) */

#define gmt_grd_get_nxpad(h,pad) ((h->nx) + pad[XLO] + pad[XHI])
#define gmt_grd_get_nypad(h,pad) ((h->ny) + pad[YLO] + pad[YHI])

/* 64-bit-safe macros to return the number of points in the grid given its dimensions */

#define GMT_get_nm(C,nx,ny) (((uint64_t)(nx)) * ((uint64_t)(ny)))
#define gmt_grd_get_nm(h) (((uint64_t)(h->nx)) * ((uint64_t)(h->ny)))

/* GMT_grd_setpad copies the given pad into the header */

#define GMT_grd_setpad(C,h,newpad) memcpy ((h)->pad, newpad, 4*sizeof(unsigned int))

/* gmt_grd_get_size computes grid size including the padding, and doubles it if complex values */

#define gmt_grd_get_size(h) ((((h->complex_mode & GMT_GRID_IS_COMPLEX_MASK) > 0) + 1ULL) * h->mx * h->my)

/* Calculate 1-D index a[ij] corresponding to 2-D array a[row][col], with 64-bit precision.
 * Use GMT_IJP when array is padded by BC rows/cols, else use GMT_IJ0.  In all cases
 * we pass the interior row,col as padding is added by the macro. Note that row,col may
 * be negative as we seek to address nodes within the padding itself.  Hence calculations
 * use int64_t for signed integers, but cast final index to uint64_t.  Finally, there is
 * GMT_IJPGI which is GMT_IJP for when there are more than 1 band (it uses h->n_bands). */

/* IJP macro using h and the pad info */
#define GMT_IJP(h,row,col) ((uint64_t)(((int64_t)(row)+(int64_t)h->pad[YHI])*((int64_t)h->mx)+(int64_t)(col)+(int64_t)h->pad[XLO]))
/* IJ0 macro using h but ignores the pad info */
#define GMT_IJ0(h,row,col) ((uint64_t)(((int64_t)(row))*((int64_t)h->nx)+(int64_t)(col)))
/* IJ macro using h but treats the entire grid with pad as no-pad grid, i.e. using mx as width */
#define GMT_IJ(h,row,col) ((uint64_t)(((int64_t)(row))*((int64_t)h->mx)+(int64_t)(col)))
/* IJPGI macro using h and the pad info that works for either grids (n_bands = 1) or images (n_bands = 1,3,4) */
#define GMT_IJPGI(h,row,col) ((uint64_t)(((int64_t)(row)+(int64_t)h->pad[YHI])*((int64_t)h->mx*(int64_t)h->n_bands)+(int64_t)(col)+(int64_t)h->pad[XLO]*(int64_t)h->n_bands))

/* Obtain row and col from index */
#define GMT_col(h,ij) (((ij) % h->mx) - h->pad[XLO])
#define GMT_row(h,ij) (((ij) / h->mx) - h->pad[YHI])

/* To set up a standard double for-loop over rows and columns to visit all nodes in a padded array by computing the node index, use GMT_grd_loop */
/* Note: All arguments must be actual variables and not expressions.
 * Note: that input col, row _may_ be signed, hence we do the cast to (int) here. */

#define GMT_row_loop(C,G,row) for (row = 0; (int)row < (int)G->header->ny; row++)
#define GMT_col_loop(C,G,row,col,ij) for (col = 0, ij = GMT_IJP (G->header, row, 0); (int)col < (int)G->header->nx; col++, ij++)
#define GMT_grd_loop(C,G,row,col,ij) GMT_row_loop(C,G,row) GMT_col_loop(C,G,row,col,ij)
/* Just a loop over columns */
#define GMT_col_loop2(C,G,col) for (col = 0; (int)col < (int)G->header->nx; col++)
/* Loop over all nodes including the pad */
#define GMT_row_padloop(C,G,row,ij) for (row = 0, ij = 0; (int)row < (int)G->header->my; row++)
#define GMT_col_padloop(C,G,col,ij) for (col = 0; (int)col < (int)G->header->mx; col++, ij++)
#define GMT_grd_padloop(C,G,row,col,ij) GMT_row_padloop(C,G,row,ij) GMT_col_padloop(C,G,col,ij)
/* Just a loop over columns */
#define GMT_col_padloop2(C,G,col) for (col = 0; (int)col < (int)G->header->mx; col++)

/* The usage could be:
	GMT_grd_loop (GMT, Grid, row, col, node) fprintf (stderr, "Value at row = %d and col = %d is %g\n", row, col, Grid->data[node]);
*/
/* The GMT_y_is_outside macro returns true if y is outside the given domain.
 * For GMT_x_is_outside, see the function in gmt_support.c since we must also deal with longitude periodicity.
 */

/* GMT_is_subset is true if wesn is set and wesn cuts through the grid region */
#define GMT_is_subset(C,h,R) (R[XHI] > R[XLO] && R[YHI] > R[YLO] && (R[XLO] > h->wesn[XLO] || R[XHI] < h->wesn[XHI] || R[YLO] > h->wesn[YLO] || R[YHI] < h->wesn[YHI]))
/* GMT_grd_same_region is true if two grids have the exact same regions */
#define GMT_grd_same_region(C,G1,G2) (G1->header->wesn[XLO] == G2->header->wesn[XLO] && G1->header->wesn[XHI] == G2->header->wesn[XHI] && G1->header->wesn[YLO] == G2->header->wesn[YLO] && G1->header->wesn[YHI] == G2->header->wesn[YHI])
/* GMT_grd_same_inc is true if two grids have the exact same grid increments */
#define GMT_grd_same_inc(C,G1,G2) (G1->header->inc[GMT_X] == G2->header->inc[GMT_X] && G1->header->inc[GMT_Y] == G2->header->inc[GMT_Y])
/* GMT_grd_same_dim is true if two grids have the exact same dimensions and registrations */
#define GMT_grd_same_shape(C,G1,G2) (G1->header->nx == G2->header->nx && G1->header->ny == G2->header->ny && G1->header->registration == G2->header->registration)
/* GMT_y_is_outside is true if y is outside the given range */
#define GMT_y_is_outside(C,y,bottom,top) ((GMT_is_dnan(y) || (y) < bottom || (y) > top) ? true : false)
/* GMT_grd_is_global is true for a geographic grid with exactly 360-degree range (with or without repeating column) */
#define GMT_grd_is_global(C,h) (h->grdtype == GMT_GRID_GEOGRAPHIC_EXACT360_NOREPEAT || h->grdtype == GMT_GRID_GEOGRAPHIC_EXACT360_REPEAT)

/* GMT_grd_duplicate_column is true for geographical global grid where first and last data columns are identical */
#define GMT_grd_duplicate_column(C,h,way) (C->current.io.col_type[way][GMT_X] == GMT_IS_LON && GMT_360_RANGE (h->wesn[XHI], h->wesn[XLO]) && h->registration == GMT_GRID_NODE_REG)

#endif /* _GMT_GRID_H */
