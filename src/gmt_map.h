/*--------------------------------------------------------------------
 *	$Id: gmt_map.h,v 1.2 2001-02-20 17:48:32 pwessel Exp $
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

#ifndef _GMT_MAP_H
#define _GMT_MAP_H

/* Macros, structures, and functions for conversion between different kinds
 * of latitudes used in GMT
 *
 * W H F Smith, 10--13 May 1999.
 * Version:	3.3.6
 */

#define GMT_LATSWAP_G2A 0	/* input = geodetic;   output = authalic   */
#define GMT_LATSWAP_A2G 1	/* input = authalic;   output = geodetic   */
#define GMT_LATSWAP_G2C 2	/* input = geodetic;   output = conformal  */
#define GMT_LATSWAP_C2G 3	/* input = conformal;  output = geodetic   */
#define GMT_LATSWAP_G2M 4	/* input = geodetic;   output = meridional */
#define GMT_LATSWAP_M2G 5	/* input = meridional; output = geodetic   */
#define GMT_LATSWAP_G2O 6	/* input = geodetic;   output = geocentric */
#define GMT_LATSWAP_O2G 7	/* input = geocentric; output = geodetic   */
#define GMT_LATSWAP_G2P 8	/* input = geodetic;   output = parametric */
#define GMT_LATSWAP_P2G 9	/* input = parametric; output = geodetic   */
#define GMT_LATSWAP_O2P 10	/* input = geocentric; output = parametric */
#define GMT_LATSWAP_P2O 11	/* input = parametric; output = geocentric */
#define GMT_LATSWAP_N	12	/* number of defined swaps  */

struct GMT_LATSWAP_CONSTS {
	double  c[GMT_LATSWAP_N][4];	/* Coefficients in 4-term series  */
	double	ra;			/* Authalic   radius (sphere for equal-area)  */
	double	rm;			/* Meridional radius (sphere for N-S distance)  */
	BOOLEAN spherical;		/* True if no conversions need to be done.  */
} GMT_lat_swap_vals;

/* Some shorthand notation for GMT specific cases */

#define GMT_latg_to_latc(lat) GMT_lat_swap_quick (lat, GMT_lat_swap_vals.c[GMT_LATSWAP_G2C])
#define GMT_latg_to_lata(lat) GMT_lat_swap_quick (lat, GMT_lat_swap_vals.c[GMT_LATSWAP_G2A])
#define GMT_latc_to_latg(lat) GMT_lat_swap_quick (lat, GMT_lat_swap_vals.c[GMT_LATSWAP_C2G])
#define GMT_lata_to_latg(lat) GMT_lat_swap_quick (lat, GMT_lat_swap_vals.c[GMT_LATSWAP_A2G])

void GMT_lat_swap_init (void);
double	GMT_lat_swap_quick (double lat, double c[]);
double	GMT_lat_swap (double lat, int itype);

#endif /* _GMT_MAP_H */
