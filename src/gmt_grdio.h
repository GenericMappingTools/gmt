/*--------------------------------------------------------------------
 *	$Id: gmt_grdio.h,v 1.2 2001-02-20 17:48:32 pwessel Exp $
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
 *	Contact info: www.soest.hawaii.edu/gmt
 *--------------------------------------------------------------------*/

/*
 * Include file for grd i/o
 *
 * Author:	Paul Wessel
 * Date:	21-AUG-1995
 * Revised:	10-SEP-1999
 * Version:	3.3.6
 */

#ifndef GMT_GRDIO_H
#define GMT_GRDIO_H

#define N_GRD_FORMATS	12	/* Number of supported grd file formats */

EXTERN_MSC int GMT_read_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_write_grd_info (char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_read_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_write_grd (char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);

EXTERN_MSC int GMT_grd_i_format;
EXTERN_MSC int GMT_grd_o_format;
EXTERN_MSC int *GMT_grd_prep_io (struct GRD_HEADER *header, double *w, double *e, double *s, double *n, int *width, int *height, int *first_col, int *last_col, int *first_row, int *last_row);

/* These are pointers to the various functions and are set in GMT_grdio_init() */

EXTERN_MSC PFI GMT_io_readinfo[N_GRD_FORMATS];
EXTERN_MSC PFI GMT_io_writeinfo[N_GRD_FORMATS];
EXTERN_MSC PFI GMT_io_readgrd[N_GRD_FORMATS];
EXTERN_MSC PFI GMT_io_writegrd[N_GRD_FORMATS];

/* Default format # 0 */

EXTERN_MSC int GMT_cdf_read_grd_info(char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_cdf_write_grd_info(char *file, struct GRD_HEADER *header);
EXTERN_MSC int GMT_cdf_read_grd(char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex);
EXTERN_MSC int GMT_cdf_write_grd(char *file, struct GRD_HEADER *header, float *grid, double w, double e, double s, double n, int *pad, BOOLEAN complex, nc_type nc_type);

#include "gmt_customio.h"

#endif /* GMT_GRDIO_H */
