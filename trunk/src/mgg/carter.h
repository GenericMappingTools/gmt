/*--------------------------------------------------------------------
 *	$Id: carter.h,v 1.6 2008-03-22 11:55:36 guru Exp $
 *
 *	Copyright (c) 1991-2008 by P. Wessel and W. H. F. Smith
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

/* carter.h contains definitions, global variables, and function prototypes
 * for the Carter water depth correction system employed by GMT.
 */

/* Various size parameters */

#define N_CARTER_BINS 64800		/* Number of 1x1 degree bins */
#define N_CARTER_ZONES 85		/* Number of Carter zones */
#define N_CARTER_OFFSETS 86		/* Number of Carter offsets */
#define N_CARTER_CORRECTIONS 5812	/* Number of Carter corrections */

/* Function ANSI-C prototypes */

GMT_LONG carter_setup (void);
GMT_LONG carter_get_bin (GMT_LONG lat, GMT_LONG lon, GMT_LONG *bin);
GMT_LONG carter_get_zone (GMT_LONG bin, GMT_LONG *zone);
GMT_LONG carter_depth_from_twt (GMT_LONG zone, short int twt_in_msec, short int *depth_in_corr_m);
GMT_LONG carter_twt_from_depth (GMT_LONG zone, short int depth_in_corr_m, short int *twt_in_msec);


/* Global variables */

short int carter_zone[N_CARTER_BINS];
short int carter_offset[N_CARTER_OFFSETS];
short int carter_correction[N_CARTER_CORRECTIONS];

GMT_LONG carter_not_initialized = 1;
