/*--------------------------------------------------------------------
 *	$Id: gmt_customio.h,v 1.3 2001-03-01 22:08:26 pwessel Exp $
 *
 *	Copyright (c) 1991-2001 by P. Wessel and W. H. F. Smith
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
 * Date:	10-AUG-1995
 * Version:	3.4
 *
 */

#ifndef GMT_CUSTOMIO_H
#define GMT_CUSTOMIO_H

/* List groups of 4 integer functions for each custom i/o grd format */

/* Format # 1 */
EXTERN_MSC int GMT_bin_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_bin_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_bin_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_bin_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

/* Format # 2 */
EXTERN_MSC int GMT_short_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_short_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_short_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_short_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

/* Format # 3 */
EXTERN_MSC int GMT_ras_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_ras_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_ras_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_ras_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

/* Format # 4 */
EXTERN_MSC int GMT_uchar_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_uchar_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_uchar_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_uchar_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

/* Format # 5 */
EXTERN_MSC int GMT_bit_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_bit_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_bit_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_bit_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

/* Format # 6 */
EXTERN_MSC int GMT_srf_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_srf_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_srf_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_srf_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

#endif /* GMT_CUSTOMIO_H */
