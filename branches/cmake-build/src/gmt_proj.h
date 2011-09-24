#ifndef _GMT_PROJ_H
#define _GMT_PROJ_H

#define HALF_DBL_MAX (DBL_MAX/2.0)

/* GMT_180 is used to see if a value really is exceeding it (beyond roundoff) */
#define GMT_180	(180.0 + GMT_CONV_LIMIT)
/* GMT_WIND_LON will remove central meridian value and adjust so lon fits between -180/+180 */
#define GMT_WIND_LON(C,lon) {lon -= C->current.proj.central_meridian; while (lon < -GMT_180) lon += 360.0; while (lon > +GMT_180) lon -= 360.0;}

/* Macros, structures, and functions for conversion between different kinds
 * of latitudes used in GMT
 *
 * W H F Smith, 1 JAN 2010
 * Version:	5 API
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

/* Some shorthand notation for GMT specific cases */

#define GMT_latg_to_latc(C,lat) GMT_lat_swap_quick (C, lat, C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2C])
#define GMT_latg_to_lata(C,lat) GMT_lat_swap_quick (C, lat, C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_G2A])
#define GMT_latc_to_latg(C,lat) GMT_lat_swap_quick (C, lat, C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_C2G])
#define GMT_lata_to_latg(C,lat) GMT_lat_swap_quick (C, lat, C->current.proj.GMT_lat_swap_vals.c[GMT_LATSWAP_A2G])

#endif /* _GMT_PROJ_H */
