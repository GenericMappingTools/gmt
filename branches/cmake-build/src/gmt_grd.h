/*--------------------------------------------------------------------
 *	$Id$
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

#ifndef _GMT_GRD_H
#define _GMT_GRD_H

/* Declaration modifier for netcdf DLL support
 * annoying: why can't netcdf.h do this on its own? */
#if defined WIN32 && ! defined NETCDF_STATIC
#define DLL_NETCDF
#endif

#include "netcdf.h"

/* Nodes that are unconstrained are assumed to be set to NaN */

enum GMT_enum_reg {GMT_GRIDLINE_REG = 0,
	GMT_PIXEL_REG};

/* These 4 lengths must NOT be changed as they are part of grd definition */
#define GRD_COMMAND_LEN320	320
#define GRD_REMARK_LEN160	160
#define GRD_TITLE_LEN80	 	80
#define GRD_UNIT_LEN80	 	80

#define GRD_VARNAME_LEN80	80

struct GRD_HEADER {
/* ===== Do not change the first three items. They are copied verbatim to the native grid header */
	int nx;				/* Number of columns */
	int ny;				/* Number of rows */
	int registration;		/* 0 for node grids, 1 for pixel grids */
/* This section is flexible. It is not copied to any grid header */
	GMT_LONG type;			/* Grid format */
	GMT_LONG bits;			/* Bits per data value (e.g., 32 for ints/floats; 8 for bytes) */
	GMT_LONG complex_mode;		/* 0 = normal, 1 = real part of complex grid, 2 = imag part of complex grid */
	GMT_LONG mx, my;		/* Actual dimensions of the grid in memory, allowing for the padding */
	GMT_LONG nm;			/* Number of data items in this grid (nx * ny) [padding is excluded] */
	GMT_LONG n_bands;		/* Number of bands [1]. Used with IMAGE containers and macros to get ij index from row,col, band */
	GMT_LONG size;			/* Actual number of items required to hold this grid (mx * my) */
	GMT_LONG pad[4];		/* Padding on west, east, south, north sides [2,2,2,2] */
	GMT_LONG BC[4];			/* Boundary condition applied on each side via pad [0 = not set, 1 = natural, 2 = periodic, 3 = data] */
	char name[GMT_TEXT_LEN256];	/* Actual name of the file after any ?<varname> and =<stuff> has been removed */
	char varname[GRD_VARNAME_LEN80];	/* NetCDF: variable name */
	int y_order;			/* NetCDF: 1 if S->N, -1 if N->S */
	int z_id;			/* NetCDF: id of z field */
	int ncid;			/* NetCDF: file ID */
	int t_index[3];			/* NetCDF: index of higher coordinates */
	int xy_dim[2];			/* NetCDF: dimension order of x and y; normally {1, 0} */
	double nan_value;		/* Missing value as stored in grid file */
	double xy_off;			/* 0.0 (registration == GMT_GRIDLINE_REG) or 0.5 ( == GMT_PIXEL_REG) */
	double r_inc[2];		/* Reciprocal incs, i.e. 1/inc */
	char flags[4];			/* Flags used for ESRI grids */
	char *pocket;			/* GDAL: A working variable handy to transmit info between funcs e.g. +b<band_info> to gdalread */
	double bcr_threshold;		/* sum of cardinals must >= threshold in bilinear; else NaN */
	GMT_LONG bcr_interpolant;	/* Interpolation function used (0, 1, 2, 3) */
	GMT_LONG bcr_n;			/* Width of the interpolation function */
	GMT_LONG no_BC;			/* If TRUE we skip BC stuff entirely */
	GMT_LONG nxp;			/* if X periodic, nxp > 0 is the period in pixels  */
	GMT_LONG nyp;			/* if Y periodic, nxp > 0 is the period in pixels  */
	GMT_LONG gn;			/* TRUE if top    edge will be set as N pole  */
	GMT_LONG gs;			/* TRUE if bottom edge will be set as S pole  */
	
/* ===== The following elements must not be changed. They are copied verbatim to the native grid header */
	double wesn[4];			/* Min/max x and y coordinates */
	double z_min;			/* Minimum z value */
	double z_max;			/* Maximum z value */
	double inc[2];			/* x and y increment */
	double z_scale_factor;		/* grd values must be multiplied by this */
	double z_add_offset;		/* After scaling, add this */
	char x_units[GRD_UNIT_LEN80];	/* units in x-direction */
	char y_units[GRD_UNIT_LEN80];	/* units in y-direction */
	char z_units[GRD_UNIT_LEN80];	/* grid value units */
	char title[GRD_TITLE_LEN80];	/* name of data set */
	char command[GRD_COMMAND_LEN320];	/* name of generating command */
	char remark[GRD_REMARK_LEN160];	/* comments re this data set */
};

/*-----------------------------------------------------------------------------------------
 *	Notes on registration:

	Assume x_min = y_min = 0 and x_max = y_max = 10 and x_inc = y_inc = 1.
	For a normal node grid we have:
		(1) nx = (x_max - x_min) / x_inc + 1 = 11
		    ny = (y_max - y_min) / y_inc + 1 = 11
		(2) node # 0 is at (x,y) = (x_min, y_max) = (0,10) and represents the surface
		    value in a box with dimensions (1,1) centered on the node.
	For a pixel grid we have:
		(1) nx = (x_max - x_min) / x_inc = 10
		    ny = (y_max - y_min) / y_inc = 10
		(2) node # 0 is at (x,y) = (x_min + 0.5*x_inc, y_max - 0.5*y_inc) = (0.5, 9.5)
		    and represents the surface value in a box with dimensions (1,1)
		    centered on the node.
-------------------------------------------------------------------------------------------*/

/* The array wesn in the header has a name that indicates the order (west, east, south, north).
 * However, to avoid using confusing indices 0-3 we define very brief constants XLO, XHI, YLO, YHI
 * that should be used instead: */
enum GMT_enum_wesnIDs {XLO = 0,	/* Index for west or xmin value */
	XHI,			/* Index for east or xmax value */
	YLO,			/* Index for south or ymin value */
	YHI,			/* Index for north or ymax value */
	ZLO, 			/* Index for zmin value */
	ZHI};			/* Index for zmax value */

/* These macros should be used to convert between (column,row) and (x,y).  It will eliminate
 * one source of typos and errors, and since macros are done at compilation time there is no
 * overhead.  Note: gmt_x_to_col does not need nx but we included it for symmetry reasons.
 * gmt_y_to_row must first compute j', the number of rows in the increasing y-direction (to
 * match the sense of truncation used for x) then we revert to row number increasing down
 * by flipping: j = ny - 1 - j'. */

#define gmt_x_to_col(x,x0,dx,off,nx) ((GMT_LONG)irint(((((x) - (x0)) / (dx)) - (off))))
#define gmt_y_to_row(y,y0,dy,off,ny) ((GMT_LONG)((ny) - 1 - irint(((((y) - (y0)) / (dy)) - (off)))))
#define GMT_col_to_x(C,col,x0,x1,dx,off,nx) (((col) == ((nx)-1)) ? (x1) - (off) * (dx) : (x0) + ((col) + (off)) * (dx))
#define GMT_row_to_y(C,row,y0,y1,dy,off,ny) (((row) == ((ny)-1)) ? (y0) + (off) * (dy) : (y1) - ((row) + (off)) * (dy))

/* The follow macros simplify using the 4 above macros when all info is in the struct header h. */

#define GMT_grd_col_to_x(C,col,h) GMT_col_to_x(C,col,h->wesn[XLO],h->wesn[XHI],h->inc[GMT_X],h->xy_off,h->nx)
#define GMT_grd_row_to_y(C,row,h) GMT_row_to_y(C,row,h->wesn[YLO],h->wesn[YHI],h->inc[GMT_Y],h->xy_off,h->ny)
#define GMT_grd_x_to_col(C,x,h) gmt_x_to_col(x,h->wesn[XLO],h->inc[GMT_X],h->xy_off,h->nx)
#define GMT_grd_y_to_row(C,y,h) gmt_y_to_row(y,h->wesn[YLO],h->inc[GMT_Y],h->xy_off,h->ny)

/* These macros calculate the number of nodes in x or y for the increment dx, dy */

#define GMT_get_n(C,min,max,inc,off) ((GMT_LONG)irint (((max) - (min)) / (inc)) + 1 - (off))
#define GMT_get_inc(C,min,max,n,off) (((max) - (min)) / ((n) + (off) - 1))

/* The follow macros simplify using the 2 above macros when all info is in the struct header */

#define GMT_grd_get_nx(C,h) GMT_get_n(C,h->wesn[XLO],h->wesn[XHI],h->inc[GMT_X],h->registration)
#define GMT_grd_get_ny(C,h) GMT_get_n(C,h->wesn[YLO],h->wesn[YHI],h->inc[GMT_Y],h->registration)

/* The follow macros gets the full length or rows and columns when padding is considered (i.e., mx and my) */

#define gmt_grd_get_nxpad(h,pad) ((GMT_LONG)(h->nx) + pad[XLO] + pad[XHI])
#define gmt_grd_get_nypad(h,pad) ((GMT_LONG)(h->ny) + pad[YLO] + pad[YHI])

/* 64-bit-safe macros to return the number of points in the grid given its dimensions */

#define GMT_get_nm(C,nx,ny) (((GMT_LONG)(nx)) * ((GMT_LONG)(ny)))
#define gmt_grd_get_nm(h) (((GMT_LONG)(h->nx)) * ((GMT_LONG)(h->ny)))

/* GMT_grd_setpad copies the given pad into the header */

#define GMT_grd_setpad(C,h,newpad) memcpy ((h)->pad, newpad, 4*sizeof(GMT_LONG))

/* gmt_grd_get_size computes grid size including the padding, and doubles it if complex values */

#define gmt_grd_get_size(h) (((h->complex_mode > 0) + 1) * h->mx * h->my)

/* Calculate 1-D index a[ij] corresponding to 2-D array a[row][col], with 64-bit precision.
 * Use GMT_IJP when array is padded by BC rows/cols, else use GMT_IJ0.  In both cases
 * we pass the column dimension as padding is added by the macro. */

/* New IJP macro using h and the pad info */
#define GMT_IJP(h,row,col) (((GMT_LONG)(row)+(GMT_LONG)h->pad[YHI])*((GMT_LONG)h->mx)+(GMT_LONG)(col)+(GMT_LONG)h->pad[XLO])
/* New IJPR|C macros using h and the pad info to get the real or imag component of a complex array*/
#define GMT_IJPR(h,row,col) (2*(((GMT_LONG)(row)+(GMT_LONG)h->pad[YHI])*((GMT_LONG)h->mx)+(GMT_LONG)(col)+(GMT_LONG)h->pad[XLO]))
#define GMT_IJPC(h,row,col) (GMT_IJPR(h,row,col)+1)
/* New IJ0 macro using h but ignores the pad info */
#define GMT_IJ0(h,row,col) (((GMT_LONG)(row))*((GMT_LONG)h->nx)+(GMT_LONG)(col))
/* New IJPGI macro using h and the pad info that works for either grids (n_bands = 1) or images (n_bands = 1,3,4) */
#define GMT_IJPGI(h,row,col) (((GMT_LONG)(row)+(GMT_LONG)h->pad[YHI])*((GMT_LONG)h->mx*(GMT_LONG)h->n_bands)+(GMT_LONG)(col)+(GMT_LONG)h->pad[XLO]*(GMT_LONG)h->n_bands)

/* Obtain row and col from index */
#define GMT_col(h,ij) ((ij) % h->mx - h->pad[XLO])
#define GMT_row(h,ij) ((ij) / h->mx - h->pad[YHI])

/* To set up a standard double for-loop over rows and columns to visit all nodes in a padded array by computing the node index, use GMT_grd_loop */
/* Note: All arguments must be actual variables and not expressions */

#define GMT_row_loop(C,G,row) for (row = 0; row < G->header->ny; row++)
#define GMT_col_loop(C,G,row,col,ij) for (col = 0, ij = GMT_IJP (G->header, row, 0); col < G->header->nx; col++, ij++)
#define GMT_grd_loop(C,G,row,col,ij) GMT_row_loop(C,G,row) GMT_col_loop(C,G,row,col,ij)
/* Just a loop over columns */
#define GMT_col_loop2(C,G,col) for (col = 0; col < G->header->nx; col++)
/* Loop over all nodes including the pad */
#define GMT_row_padloop(C,G,row,ij) for (row = ij = 0; row < G->header->my; row++)
#define GMT_col_padloop(C,G,col,ij) for (col = 0; col < G->header->mx; col++, ij++)
#define GMT_grd_padloop(C,G,row,col,ij) GMT_row_padloop(C,G,row,ij) GMT_col_padloop(C,G,col,ij)

/* The usage could be:
	GMT_grd_loop (GMT, Grid, row, col, node) fprintf (stderr, "Value at row = %ld and col = %ld is %g\n", row, col, Grid->data[node]);
*/
/* The GMT_y_is_outside macro returns TRUE if y is outside the given domain.
 * For GMT_x_is_outside, see the function in gmt_support.c since we must also deal with longitude periodicity.
 */

/* GMT_is_subset is TRUE if wesn is set and wesn cuts through the grid region */
#define GMT_is_subset(C,h,R) (R[XHI] > R[XLO] && R[YHI] > R[YLO] && (R[XLO] > h->wesn[XLO] || R[XHI] < h->wesn[XHI] || R[YLO] > h->wesn[YLO] || R[YHI] < h->wesn[YHI]))
/* GMT_grd_same_region is TRUE if two grids have the exact same regions */
#define GMT_grd_same_region(C,G1,G2) (G1->header->wesn[XLO] == G2->header->wesn[XLO] && G1->header->wesn[XHI] == G2->header->wesn[XHI] && G1->header->wesn[YLO] == G2->header->wesn[YLO] && G1->header->wesn[YHI] == G2->header->wesn[YHI])
/* GMT_y_is_outside is TRUE if y is outside the given range */
#define GMT_y_is_outside(C,y,bottom,top) ((GMT_is_dnan(y) || (y) < bottom || (y) > top) ? TRUE : FALSE)

#endif /* _GMT_GRD_H */
