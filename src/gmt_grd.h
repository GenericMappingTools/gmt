/*--------------------------------------------------------------------
 *	$Id: gmt_grd.h,v 1.45 2011-03-03 21:02:50 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel and W. H. F. Smith
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
 * Date:	26-MAY-1990
 * Revised:	 6-FEB-2006
 * Version:	4.1
 */
 
#ifndef _GMT_GRD_H
#define _GMT_GRD_H

#include "netcdf.h"

/* Nodes that are unconstrained are assumed to be set to NaN */

#define GRD_COMMAND_LEN	320
#define GRD_REMARK_LEN	160
#define GRD_TITLE_LEN	 80
#define GRD_UNIT_LEN	 80
#define GRD_VARNAME_LEN	 80

struct GRD_HEADER {
/* Do not change the first three items. They are copied verbatim to the native grid header */
	int nx;				/* Number of columns */
	int ny;				/* Number of rows */
	int node_offset;		/* 0 for node grids, 1 for pixel grids */
/* This section is flexible. It is not copied to any grid header */
	GMT_LONG type;			/* Grid format */
	char name[GMT_LONG_TEXT];	/* Actual name of the file after any ?<varname> and =<stuff> has been removed */
	char varname[GRD_VARNAME_LEN];	/* NetCDF: variable name */
	int y_order;			/* NetCDF: 1 if S->N, -1 if N->S */
	int z_id;			/* NetCDF: id of z field */
	int ncid;			/* NetCDF: file ID */
	int t_index[3];			/* NetCDF: index of higher coordinates */
	int xy_dim[2];			/* NetCDF: dimension order of x and y; normally {1, 0} */
	double nan_value;		/* Missing value as stored in grid file */
	double xy_off;			/* 0.0 (node_offset == 0) or 0.5 ( == 1) */
/* The following elements should not be changed. They are copied verbatim to the native grid header */
	double x_min;			/* Minimum x coordinate */
	double x_max;			/* Maximum x coordinate */
	double y_min;			/* Minimum y coordinate */
	double y_max;			/* Maximum y coordinate */
	double z_min;			/* Minimum z value */
	double z_max;			/* Maximum z value */
	double x_inc;			/* x increment */
	double y_inc;			/* y increment */
	double z_scale_factor;		/* grd values must be multiplied by this */
	double z_add_offset;		/* After scaling, add this */
	char x_units[GRD_UNIT_LEN];	/* units in x-direction */
	char y_units[GRD_UNIT_LEN];	/* units in y-direction */
	char z_units[GRD_UNIT_LEN];	/* grid value units */
	char title[GRD_TITLE_LEN];	/* name of data set */
	char command[GRD_COMMAND_LEN];	/* name of generating command */
	char remark[GRD_REMARK_LEN];	/* comments re this data set */
};

/*-----------------------------------------------------------------------------------------
 *	Notes on node_offset:

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

/* These macros should be used to convert between (column,row) and (x,y).  It will eliminate
 * one source of typos and errors, and since macros are done at compilation time there is no
 * overhead.  Note GMT_x_to_i does not use nx but we included it for symmetry reasons.
 * GMT_y_to_j must first compute j', the number of rows in the increasing y-direction (to
 * match the sense of truncation used for x) then we revert to row number increasing down
 * by flipping: j = ny - 1 - j' */

#define GMT_x_to_i(x,x0,dx,off,nx) ((GMT_LONG)irint(((((x) - (x0)) / (dx)) - (off))))
#define GMT_y_to_j(y,y0,dy,off,ny) ((GMT_LONG)((ny) - 1 - irint(((((y) - (y0)) / (dy)) - (off)))))
#define GMT_i_to_x(i,x0,x1,dx,off,nx) (((i) == ((nx)-1)) ? (x1) - (off) * (dx) : (x0) + ((i) + (off)) * (dx))
#define GMT_j_to_y(j,y0,y1,dy,off,ny) (((j) == ((ny)-1)) ? (y0) + (off) * (dy) : (y1) - ((j) + (off)) * (dy))

/* These macros calculate the number of nodes in x or y or the increment dx, dy*/

#define GMT_get_n(min,max,inc,off) ((GMT_LONG)irint (((max) - (min)) / (inc)) + 1 - (off))
#define GMT_get_inc(min,max,n,off) (((max) - (min)) / ((n) + (off) - 1))

/* 64-bit-safe macro to return the number of points in the grid given dimensions */

#define GMT_get_nm(nx,ny) (((GMT_LONG)(nx)) * ((GMT_LONG)(ny)))

/* Calculate 1-D index a[ij] corresponding to 2-D array a[row][col], with 64-bit precision.
 * Use GMT_IJbc when array is padded by BC rows/cols, else use GMT_IJ.  In both cases
 * we pass the column dimension as padding is added by the macro. */

#define GMT_IJbc(row,col,nx,pad) (((GMT_LONG)(row)+(GMT_LONG)(pad))*((GMT_LONG)(nx) + (GMT_LONG)(2*(pad)))+(GMT_LONG)(col)+(GMT_LONG)(pad))
#define GMT_IJ(row,col,nx) (((GMT_LONG)(row))*((GMT_LONG)(nx))+(GMT_LONG)(col))

/* The GMT_y_is_outside macro returns TRUE if y is outside the given domain.
 * For GMT_x_is_outside, see the function in gmt_support.c
 */

#define GMT_y_is_outside(y,bottom,top) ((GMT_is_dnan(y) || (y) < bottom || (y) > top) ? TRUE : FALSE)
EXTERN_MSC GMT_LONG GMT_x_is_outside (double *x, double left, double right);
EXTERN_MSC void GMT_set_xy_domain (double wesn_extended[], struct GRD_HEADER *h);

#endif /* _GMT_GRD_H */
