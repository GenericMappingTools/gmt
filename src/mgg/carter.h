/*--------------------------------------------------------------------
 *	$Id: carter.h,v 1.12 2011-03-03 21:02:51 guru Exp $
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

/* carter.h contains definitions, global variables, and function prototypes
 * for the Carter water depth correction system employed by GMT.
 */

/* Various size parameters */

#define N_CARTER_BINS 64800		/* Number of 1x1 degree bins */
#define N_CARTER_ZONES 85		/* Number of Carter zones */
#define N_CARTER_OFFSETS 86		/* Number of Carter offsets */
#define N_CARTER_CORRECTIONS 5812	/* Number of Carter corrections */

/* Function ANSI-C prototypes */

int carter_setup (void);
int carter_get_bin (int lat, int lon, int *bin);
int carter_get_zone (int bin, int *zone);
int carter_depth_from_twt (int zone, short int twt_in_msec, short int *depth_in_corr_m);
int carter_twt_from_depth (int zone, short int depth_in_corr_m, short int *twt_in_msec);


/* Global variables */

short int carter_zone[N_CARTER_BINS];
short int carter_offset[N_CARTER_OFFSETS];
short int carter_correction[N_CARTER_CORRECTIONS];

int carter_not_initialized = 1;
