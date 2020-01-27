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
 * grd.h contains the definition for a GMT-SYSTEM Version >= 2 grd file
 *
 * grd is stored in rows going from west (xmin) to east (xmax)
 * first row in file has yvalue = north (ymax).
 * This is SCANLINE orientation.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

/*!
 * \file gmt_grd.h
 * \brief Definition for a GMT-SYSTEM Version >= 2 grd file
 */

#ifndef GMT_GRID_H
#define GMT_GRID_H

/* netcdf convention */
#define GMT_NC_CONVENTION "CF-1.7"

enum GMT_enum_grdtype {	/* Used in gmt_grdio.c and gmt_nc.c */
	/* Special cases of geographic grids with periodicity */
	GMT_GRID_CARTESIAN=0,			/* Cartesian data, no periodicity involved */
	GMT_GRID_GEOGRAPHIC_LESS360,		/* x is longitude, but range is < 360 degrees */
	GMT_GRID_GEOGRAPHIC_EXACT360_NOREPEAT,	/* x is longitude, range is 360 degrees, no repeat node */
	GMT_GRID_GEOGRAPHIC_EXACT360_REPEAT,	/* x is longitude, range is 360 degrees, gridline registered and repeat node at 360*/
	GMT_GRID_GEOGRAPHIC_MORE360		/* x is longitude, and range exceeds 360 degrees */
};

/*! Grid layout for complex grids */
enum gmt_enum_grdlayout {	/* Needed by some modules */
	GMT_GRID_IS_SERIAL = 0,		/* Grid is RRRRRR...[IIIIII...] */
	GMT_GRID_IS_INTERLEAVED = 1};	/* Grid is RIRIRIRI... - required layout for FFT */

enum gmt_enum_grid_nans {	/* Flags to tell if a grid has NaNs */
	GMT_GRID_NOT_CHECKED = 0,
	GMT_GRID_NO_NANS = 1,
	GMT_GRID_HAS_NANS = 2};
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

/*! Order of rows in z variable */
enum Netcdf_row_order {	/* Used in gmt_grdio.c and gmt_nc.c */
	k_nc_start_north = -1, /* The least dimension (i.e., lat or y) decreases */
	k_nc_start_south = 1   /* The least dimension (i.e., lat or y) increases */
};

enum Netcdf_chunksize {	/* Used in gmt_grdio.c and gmt_nc.c */
	k_netcdf_io_classic = 0, /* netCDF classic format */
	k_netcdf_io_chunked_auto /* netCDF 4 auto-determined optimal chunk size */
};

/* The array wesn in the header has a name that indicates the order (west, east, south, north).
 * However, to avoid using confusing indices 0-5 we define very brief constants
 * XLO, XHI, YLO, YHI, ZLO, ZHI that should be used instead: */
enum gmt_enum_wesnids {
	XLO = 0, /* Index for west or xmin value */
	XHI,     /* Index for east or xmax value */
	YLO,     /* Index for south or ymin value */
	YHI,     /* Index for north or ymax value */
	ZLO,     /* Index for zmin value */
	ZHI      /* Index for zmax value */
};

/* These macros should be used to convert between (column,row) and (x,y).  It will eliminate
 * one source of typos and errors, and since macros are done at compilation time there is no
 * overhead.  Note: gmt_M_x_to_col does not need n_columns but we included it for symmetry reasons.
 * gmt_M_y_to_row must first compute j', the number of rows in the increasing y-direction (to
 * match the sense of truncation used for x) then we revert to row number increasing down
 * by flipping: j = n_rows - 1 - j'.
 * Note that input col, row _may_ be negative, hence we do the cast to (int) here. */

#define gmt_M_x_to_col(x,x0,dx,off,n_columns) (irint((((x) - (x0)) / (dx)) - (off)))
#define gmt_M_y_to_row(y,y0,dy,off,n_rows) ((n_rows) - 1 - irint(((((y) - (y0)) / (dy)) - (off))))
#define gmt_M_col_to_x(C,col,x0,x1,dx,off,n_columns) (((int)(col) == (int)((n_columns)-1)) ? (x1) - (off) * (dx) : (x0) + ((col) + (off)) * (dx))
#define gmt_M_row_to_y(C,row,y0,y1,dy,off,n_rows) (((int)(row) == (int)((n_rows)-1)) ? (y0) + (off) * (dy) : (y1) - ((row) + (off)) * (dy))

/*! The follow macros simplify using the 4 above macros when all info is in the struct header h. */

#define gmt_M_grd_col_to_x(C,col,h) gmt_M_col_to_x(C,col,h->wesn[XLO],h->wesn[XHI],h->inc[GMT_X],h->xy_off,h->n_columns)
#define gmt_M_grd_row_to_y(C,row,h) gmt_M_row_to_y(C,row,h->wesn[YLO],h->wesn[YHI],h->inc[GMT_Y],h->xy_off,h->n_rows)
#define gmt_M_grd_x_to_col(C,x,h) gmt_M_x_to_col(x,h->wesn[XLO],h->inc[GMT_X],h->xy_off,h->n_columns)
#define gmt_M_grd_y_to_row(C,y,h) gmt_M_y_to_row(y,h->wesn[YLO],h->inc[GMT_Y],h->xy_off,h->n_rows)

/*! These macros calculate the number of nodes in x or y for the increment dx, dy */

#define gmt_M_get_n(C,min,max,inc,off) (urint ((((max) - (min)) / (inc)) + 1 - (off)) )
#define gmt_M_get_inc(C,min,max,n,off) (((max) - (min)) / ((n) + (off) - 1))

/*! The follow macros simplify using the 2 above macros when all info is in the struct header */

#define gmt_M_grd_get_nx(C,h) (gmt_M_get_n(C,h->wesn[XLO],h->wesn[XHI],h->inc[GMT_X],h->registration))
#define gmt_M_grd_get_ny(C,h) (gmt_M_get_n(C,h->wesn[YLO],h->wesn[YHI],h->inc[GMT_Y],h->registration))

/*! The follow macros gets the full length or rows and columns when padding is considered (i.e., mx and my) */

#define gmt_M_grd_get_nxpad(h,pad) ((h->n_columns) + pad[XLO] + pad[XHI])
#define gmt_M_grd_get_nypad(h,pad) ((h->n_rows) + pad[YLO] + pad[YHI])

/*! 64-bit-safe macros to return the number of points in the grid given its dimensions */

#define gmt_M_get_nm(C,n_columns,n_rows) (((uint64_t)(n_columns)) * ((uint64_t)(n_rows)))
#define gmt_M_grd_get_nm(h) (((uint64_t)(h->n_columns)) * ((uint64_t)(h->n_rows)))

/*! gmt_M_grd_setpad copies the given pad into the header */

#define gmt_M_grd_setpad(C,h,newpad) memcpy ((h)->pad, newpad, 4*sizeof(unsigned int))

/* Calculate 1-D index a[ij] corresponding to 2-D array a[row][col], with 64-bit precision.
 * Use gmt_M_ijp when array is padded by BC rows/cols, else use gmt_M_ij0.  In all cases
 * we pass the interior row,col as padding is added by the macro. Note that row,col may
 * be negative as we seek to address nodes within the padding itself.  Hence calculations
 * use int64_t for signed integers, but cast final index to uint64_t.  Finally, there is
 * gmt_M_ijpgi which is gmt_M_ijp for when there are more than 1 band (it uses h->n_bands). */

/*! IJP macro using h and the pad info */
#define gmt_M_ijp(h,row,col) ((uint64_t)(((int64_t)(row)+(int64_t)h->pad[YHI])*((int64_t)h->mx)+(int64_t)(col)+(int64_t)h->pad[XLO]))
/*! IJ0 macro using h but ignores the pad info */
#define gmt_M_ij0(h,row,col) ((uint64_t)(((int64_t)(row))*((int64_t)h->n_columns)+(int64_t)(col)))
/*! IJ macro using h but treats the entire grid with pad as no-pad grid, i.e. using mx as width */
#define gmt_M_ij(h,row,col) ((uint64_t)(((int64_t)(row))*((int64_t)h->mx)+(int64_t)(col)))
/*! IJPGI macro using h and the pad info that works for either grids (n_bands = 1) or images (n_bands = 1,3,4) */
#define gmt_M_ijpgi(h,row,col) ((uint64_t)(((int64_t)(row)+(int64_t)h->pad[YHI])*((int64_t)h->mx*(int64_t)h->n_bands)+(int64_t)(col)+(int64_t)h->pad[XLO]*(int64_t)h->n_bands))

/*! Obtain row and col from index */
#define gmt_M_col(h,ij) (((ij) % h->mx) - h->pad[XLO])
#define gmt_M_row(h,ij) (((ij) / h->mx) - h->pad[YHI])

/* To set up a standard double for-loop over rows and columns to visit all nodes in a padded array by computing the node index, use gmt_M_grd_loop */
/* Note: All arguments must be actual variables and not expressions.
 * Note: that input col, row _may_ be signed, hence we do the cast to (int) here. */

#define gmt_M_row_loop(C,G,row) for (row = 0; (int)row < (int)G->header->n_rows; row++)
#define gmt_M_col_loop(C,G,row,col,ij) for (col = 0, ij = gmt_M_ijp (G->header, row, 0); (int)col < (int)G->header->n_columns; col++, ij++)
#define gmt_M_grd_loop(C,G,row,col,ij) gmt_M_row_loop(C,G,row) gmt_M_col_loop(C,G,row,col,ij)
/*! Just a loop over columns */
#define gmt_M_col_loop2(C,G,col) for (col = 0; (int)col < (int)G->header->n_columns; col++)

/* The usage could be:
	gmt_M_grd_loop (GMT, Grid, row, col, node) fprintf (stderr, "Value at row = %d and col = %d is %g\n", row, col, Grid->data[node]);
*/
/* The gmt_M_y_is_outside macro returns true if y is outside the given domain.
 * For gmt_x_is_outside, see the function in gmt_support.c since we must also deal with longitude periodicity.
 */

/*! gmt_M_is_subset is true if wesn is set and wesn cuts through the grid region */
#define gmt_M_is_subset(C,h,R) (R[XHI] > R[XLO] && R[YHI] > R[YLO] && (R[XLO] > h->wesn[XLO] || R[XHI] < h->wesn[XHI] || R[YLO] > h->wesn[YLO] || R[YHI] < h->wesn[YHI]))
/*! gmt_M_grd_same_region is true if two grids have the exact same regions */
#define gmt_M_grd_same_region(C,G1,G2) (G1->header->wesn[XLO] == G2->header->wesn[XLO] && G1->header->wesn[XHI] == G2->header->wesn[XHI] && G1->header->wesn[YLO] == G2->header->wesn[YLO] && G1->header->wesn[YHI] == G2->header->wesn[YHI])
/*! gmt_M_grd_same_inc is true if two grids have the exact same grid increments */
#define gmt_M_grd_same_inc(C,G1,G2) (G1->header->inc[GMT_X] == G2->header->inc[GMT_X] && G1->header->inc[GMT_Y] == G2->header->inc[GMT_Y])
/*! gmt_M_grd_equal_inc is true if a grid has approximately the same x and y grid increments (within 1e-6) */
#define gmt_M_grd_equal_xy_inc(C,G) (fabs (1.0 - G->header->inc[GMT_X] / G->header->inc[GMT_Y]) < 1.0e-6)
/*! GMT_grd_same_dim is true if two grids have the exact same dimensions and registrations */
#define gmt_M_grd_same_shape(C,G1,G2) (G1->header->n_columns == G2->header->n_columns && G1->header->n_rows == G2->header->n_rows && G1->header->registration == G2->header->registration)
/*! gmt_M_y_is_outside is true if y is outside the given range */
#define gmt_M_y_is_outside(C,y,bottom,top) ((gmt_M_is_dnan(y) || (y) < bottom || (y) > top) ? true : false)

/*! gmt_M_grd_duplicate_column is true for geographical global grid where first and last data columns are identical */
#define gmt_M_grd_duplicate_column(C,h,way) (C->current.io.col_type[way][GMT_X] == GMT_IS_LON && gmt_M_360_range (h->wesn[XHI], h->wesn[XLO]) && h->registration == GMT_GRID_NODE_REG)

#endif /* GMT_GRID_H */
