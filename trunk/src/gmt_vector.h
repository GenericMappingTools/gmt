/*--------------------------------------------------------------------
 *	$Id: gmt_vector.h,v 1.18 2011-03-03 21:02:51 guru Exp $
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

#ifndef _GMT_VECTOR_H
#define _GMT_VECTOR_H

EXTERN_MSC double GMT_dot3v (double *a, double *b);
EXTERN_MSC double GMT_mag3v (double *a);
EXTERN_MSC GMT_LONG GMT_chol_dcmp (double *a, double *d, double *cond, GMT_LONG nr, GMT_LONG n);
EXTERN_MSC GMT_LONG GMT_fix_up_path (double **a_lon, double **a_lat, GMT_LONG n, double step, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_jacobi (double *a, GMT_LONG *n, GMT_LONG *m, double *d, double *v, double *b, double *z, GMT_LONG *nrots);
EXTERN_MSC void GMT_gauss (double *a, double *vec, GMT_LONG n, GMT_LONG nstore, double test, GMT_LONG *ierror, GMT_LONG itriag);
EXTERN_MSC void GMT_cart_to_geo (double *lat, double *lon, double *a, GMT_LONG degrees);
EXTERN_MSC void GMT_chol_recover (double *a, double *d, GMT_LONG nr, GMT_LONG n, GMT_LONG nerr, GMT_LONG donly);
EXTERN_MSC void GMT_chol_solv (double *a, double *x, double *y, GMT_LONG nr, GMT_LONG n);
EXTERN_MSC void GMT_cross3v (double *a, double *b, double *c);
EXTERN_MSC void GMT_geo_to_cart (double lat, double lon, double *a, GMT_LONG degrees);
EXTERN_MSC void GMT_normalize3v (double *a);

#endif /* _GMT_VECTOR_H */
