/*--------------------------------------------------------------------
 *	$Id: gmt_customio.h,v 1.10 2005-01-02 05:13:19 pwessel Exp $
 *
 *	Copyright (c) 1991-2004 by P. Wessel and W. H. F. Smith
 *	See COPYING file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
 
/*
 * Include file for gmt_customio functions.
 *
 * Author:	Paul Wessel
 * Date:	06-DEC-2001
 * Version:	4
 *
 */

#ifndef GMT_CUSTOMIO_H
#define GMT_CUSTOMIO_H

/* List groups of 5 integer functions for each custom i/o grd format */

/* Format # 1 */
EXTERN_MSC int GMT_bin_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_bin_update_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_bin_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_bin_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_bin_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

/* Format # 2 */
EXTERN_MSC int GMT_short_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_short_update_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_short_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_short_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_short_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

/* Format # 3 */
EXTERN_MSC int GMT_ras_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_ras_update_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_ras_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_ras_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_ras_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

/* Format # 4 */
EXTERN_MSC int GMT_uchar_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_uchar_update_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_uchar_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_uchar_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_uchar_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

/* Format # 5 */
EXTERN_MSC int GMT_bit_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_bit_update_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_bit_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_bit_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_bit_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

/* Format # 6 */
EXTERN_MSC int GMT_srf_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_srf_update_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_srf_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_srf_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_srf_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

/* Format # 13 */
EXTERN_MSC int GMT_int_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_int_update_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_int_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_int_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_int_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

#define HEADER_SIZE	892
#define GMT_N_NATIVE_FORMATS	6

#define GMT_NATIVE_CHAR		0
#define GMT_NATIVE_UCHAR	1
#define GMT_NATIVE_SHORT	2
#define GMT_NATIVE_INT		3
#define GMT_NATIVE_FLOAT	4
#define GMT_NATIVE_DOUBLE	5

EXTERN_MSC size_t GMT_native_write_one (FILE *fp, float z, int type);
EXTERN_MSC float GMT_native_decode (void *vptr, int k, int type);
EXTERN_MSC double GMT_native_encode (float z, int type);

/* NOAA NGDC MGG format dealt with via includes: */
#include "gmt_mgg_header2.h"

#endif /* GMT_CUSTOMIO_H */
