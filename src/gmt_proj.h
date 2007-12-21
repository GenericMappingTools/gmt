#ifndef _GMT_PROJ_H
#define _GMT_PROJ_H

#define HALF_DBL_MAX (DBL_MAX/2.0)

/* GMT_180 is used to see if a value really is exceeding it (beyond roundoff) */
#define GMT_180	(180.0 + GMT_CONV_LIMIT)
/* GMT_WIND_LON will remove central meridian value and adjust so lon fits between -180/+180 */
#define GMT_WIND_LON(lon) {lon -= project_info.central_meridian; while (lon < -GMT_180) lon += 360.0; while (lon > +GMT_180) lon -= 360.0;}

/* Macros, structures, and functions for conversion between different kinds
 * of latitudes used in GMT
 *
 * W H F Smith, 10--13 May 1999.
 * Version:	4.1.x
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

#define GMT_latg_to_latc(lat) GMT_lat_swap_quick (lat, project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2C])
#define GMT_latg_to_lata(lat) GMT_lat_swap_quick (lat, project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_G2A])
#define GMT_latc_to_latg(lat) GMT_lat_swap_quick (lat, project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_C2G])
#define GMT_lata_to_latg(lat) GMT_lat_swap_quick (lat, project_info.GMT_lat_swap_vals.c[GMT_LATSWAP_A2G])

void GMT_lat_swap_init (void);
double	GMT_lat_swap_quick (double lat, double c[]);
double	GMT_lat_swap (double lat, int itype);
void GMT_scale_eqrad ();
void GMT_vpolar(double lon0);
void GMT_vmerc(double lon0, double slat);
void GMT_vcyleq(double lon0, double slat);
void GMT_vcyleqdist(double lon0, double slat);
void GMT_vcylstereo(double lon0, double slat);
void GMT_vmiller(double lon0);
void GMT_vstereo(double lon0, double lat0, double horizon);
void GMT_vlamb(double lon0, double lat0, double pha, double phb);
void GMT_vtm(double lon0, double lat0);
void GMT_vlambeq(double lon0, double lat0, double horizon);
void GMT_vortho(double lon0, double lat0, double horizon);
void GMT_vgenper(double lon0, double lat0, double altitude, double azimuth, double tilt, double rotation, double width, double height);
void GMT_vgnomonic(double lon0, double lat0, double horizon);
void GMT_vazeqdist(double lon0, double lat0, double horizon);
void GMT_vmollweide(double lon0, double scale);
void GMT_vhammer(double lon0, double scale);
void GMT_vwinkel(double lon0, double scale);
void GMT_veckert4(double lon0);
void GMT_veckert6(double lon0);
void GMT_vrobinson(double lon0);
void GMT_vsinusoidal(double lon0);
void GMT_vcassini(double lon0, double lat0);
void GMT_valbers(double lon0, double lat0, double ph1, double ph2);
void GMT_valbers_sph(double lon0, double lat0, double ph1, double ph2);
void GMT_veconic(double lon0, double lat0, double ph1, double ph2);
void GMT_vgrinten(double lon0, double scale);

void GMT_polar(double x, double y, double *x_i, double *y_i);		/*	Convert x/y (being theta,r) to x,y	*/
void GMT_ipolar(double *x, double *y, double x_i, double y_i);		/*	Convert (theta,r) to x,y	*/
void GMT_translin(double forw, double *inv);				/*	Forward linear	*/
void GMT_translind(double forw, double *inv);				/*	Forward linear, but using 0-360 degrees	*/
void GMT_itranslin(double *forw, double inv);				/*	Inverse linear	*/
void GMT_itranslind(double *forw, double inv);				/*	Inverse linear, but using 0-360 degrees	*/
void GMT_translog10(double forw, double *inv);				/*	Forward log10	*/
void GMT_itranslog10(double *forw, double inv);				/*	Inverse log10	*/
void GMT_transpowx(double x, double *x_in);				/*	Forward pow x	*/
void GMT_itranspowx(double *x, double x_in);				/*	Inverse pow x	*/
void GMT_transpowy(double y, double *y_in);				/*	Forward pow y 	*/
void GMT_itranspowy(double *y, double y_in);				/*	Inverse pow y 	*/
void GMT_transpowz(double z, double *z_in);				/*	Forward pow z 	*/
void GMT_itranspowz(double *z, double z_in);				/*	Inverse pow z 	*/
void GMT_albers(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Albers)	*/
void GMT_ialbers(double *lon, double *lat, double x, double y);		/*	Convert x/y (Albers) to lon/lat	*/
void GMT_econic(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Equidistant Conic)	*/
void GMT_ieconic(double *lon, double *lat, double x, double y);		/*	Convert x/y (Equidistant Conic) to lon/lat	*/
void GMT_albers_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Albers Spherical)	*/
void GMT_ialbers_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (Albers Spherical) to lon/lat	*/
void GMT_azeqdist(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Azimuthal equal-distance)	*/
void GMT_iazeqdist(double *lon, double *lat, double x, double y);	/*	Convert x/y (Azimuthal equal-distance) to lon/lat	*/
void GMT_cassini(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Cassini)	*/
void GMT_icassini(double *lon, double *lat, double x, double y);	/*	Convert x/y (Cassini) to lon/lat	*/
void GMT_cassini_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Cassini Spherical)	*/
void GMT_icassini_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (Cassini Spherical) to lon/lat	*/
void GMT_hammer(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Hammer-Aitoff)	*/
void GMT_ihammer(double *lon, double *lat, double x, double y);		/*	Convert x/y (Hammer-Aitoff) to lon/lat	*/
void GMT_grinten(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (van der Grinten)	*/
void GMT_igrinten(double *lon, double *lat, double x, double y);	/*	Convert x/y (van der Grinten) to lon/lat	*/
void GMT_merc_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Mercator Spherical)	*/
void GMT_imerc_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (Mercator Spherical) to lon/lat	*/
void GMT_plrs(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Polar)		*/
void GMT_iplrs(double *lon, double *lat, double x, double y);		/*	Convert x/y (Polar) to lon/lat		*/
void GMT_plrs_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Polar Spherical)		*/
void GMT_iplrs_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (Polar Spherical) to lon/lat		*/
void GMT_lamb(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Lambert)	*/
void GMT_ilamb(double *lon, double *lat, double x, double y);		/*	Convert x/y (Lambert) to lon/lat 	*/
void GMT_lamb_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Lambert Spherical)	*/
void GMT_ilamb_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (Lambert Spherical) to lon/lat 	*/
void GMT_oblmrc(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Oblique Mercator)	*/
void GMT_ioblmrc(double *lon, double *lat, double x, double y);		/*	Convert x/y (Oblique Mercator) to lon/lat 	*/
void GMT_genper(double lon, double lat, double *x, double *y);          /*      Convert lon/lat to x/y (ORTHO)  */
void GMT_igenper(double *lon, double *lat, double x, double y);         /*      Convert x/y (ORTHO) to lon/lat  */
void GMT_ortho(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (GMT_ORTHO)	*/
void GMT_iortho(double *lon, double *lat, double x, double y);		/*	Convert x/y (GMT_ORTHO) to lon/lat 	*/
void GMT_gnomonic(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (GMT_GNOMONIC)	*/
void GMT_ignomonic(double *lon, double *lat, double x, double y);	/*	Convert x/y (GMT_GNOMONIC) to lon/lat 	*/
void GMT_sinusoidal(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (GMT_SINUSOIDAL)	*/
void GMT_isinusoidal(double *lon, double *lat, double x, double y);	/*	Convert x/y (GMT_SINUSOIDAL) to lon/lat 	*/
void GMT_tm(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (TM)	*/
void GMT_itm(double *lon, double *lat, double x, double y);		/*	Convert x/y (TM) to lon/lat 	*/
void GMT_tm_sph(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (GMT_TM Spherical)	*/
void GMT_itm_sph(double *lon, double *lat, double x, double y);		/*	Convert x/y (GMT_TM Spherical) to lon/lat 	*/
void GMT_utm(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (UTM)	*/
void GMT_iutm(double *lon, double *lat, double x, double y);		/*	Convert x/y (UTM) to lon/lat 	*/
void GMT_utm_sph(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (UTM Spherical)	*/
void GMT_iutm_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (UTM Spherical) to lon/lat 	*/
void GMT_winkel(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Winkel)	*/
void GMT_iwinkel(double *lon, double *lat, double x, double y);		/*	Convert x/y (Winkel) to lon/lat	*/
void GMT_iwinkel_sub (double y, double *phi);				/*	Used by GMT_iwinkel */
void GMT_eckert4(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Eckert IV)	*/
void GMT_ieckert4(double *lon, double *lat, double x, double y);	/*	Convert x/y (Eckert IV) to lon/lat	*/
void GMT_eckert6(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Eckert VI)	*/
void GMT_ieckert6(double *lon, double *lat, double x, double y);	/*	Convert x/y (Eckert VI) to lon/lat	*/
void GMT_robinson(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Robinson)	*/
void GMT_irobinson(double *lon, double *lat, double x, double y);	/*	Convert x/y (Robinson) to lon/lat	*/
void GMT_stereo1(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Stereographic)	*/
void GMT_stereo2(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Stereographic, equatorial view)	*/
void GMT_istereo(double *lon, double *lat, double x, double y);		/*	Convert x/y (Stereographic) to lon/lat 	*/
void GMT_stereo1_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Stereographic Spherical)	*/
void GMT_stereo2_sph(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Stereographic Spherical, equatorial view)	*/
void GMT_istereo_sph(double *lon, double *lat, double x, double y);	/*	Convert x/y (Stereographic Spherical) to lon/lat 	*/
void GMT_lambeq(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Lambert Azimuthal Equal-Area)	*/
void GMT_ilambeq(double *lon, double *lat, double x, double y);		/*	Convert x/y (Lambert Azimuthal Equal-Area) to lon/lat 	*/
void GMT_mollweide(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Mollweide Equal-Area)	*/
void GMT_imollweide(double *lon, double *lat, double x, double y);	/*	Convert x/y (Mollweide Equal-Area) to lon/lat 	*/
void GMT_cyleq(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Cylindrical Equal-Area)	*/
void GMT_icyleq(double *lon, double *lat, double x, double y);		/*	Convert x/y (Cylindrical Equal-Area) to lon/lat 	*/
void GMT_cyleqdist(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Cylindrical Equidistant)	*/
void GMT_icyleqdist(double *lon, double *lat, double x, double y);	/*	Convert x/y (Cylindrical Equidistant) to lon/lat 	*/
void GMT_miller(double lon, double lat, double *x, double *y);		/*	Convert lon/lat to x/y (Miller Cylindrical)	*/
void GMT_imiller(double *lon, double *lat, double x, double y);		/*	Convert x/y (Miller Cylindrical) to lon/lat 	*/
void GMT_cylstereo(double lon, double lat, double *x, double *y);	/*	Convert lon/lat to x/y (Cylindrical Stereographic)	*/
void GMT_icylstereo(double *lon, double *lat, double x, double y);	/*	Convert x/y (Cylindrical Stereographic) to lon/lat 	*/
void GMT_obl (double lon, double lat, double *olon, double *olat);	/*	Convert lon/loat to oblique lon/lat		*/
void GMT_iobl (double *lon, double *lat, double olon, double olat);	/*	Convert oblique lon/lat to regular lon/lat	*/
double GMT_left_winkel(double y);	/*	For Winkel maps	*/
double GMT_right_winkel(double y);	/*	For Winkel maps	*/
double GMT_left_eckert4(double y);	/*	For Eckert IV maps	*/
double GMT_right_eckert4(double y);	/*	For Eckert IV maps	*/
double GMT_left_eckert6(double y);	/*	For Eckert VI maps	*/
double GMT_right_eckert6(double y);	/*	For Eckert VI maps	*/
double GMT_left_robinson(double y);	/*	For Robinson maps	*/
double GMT_right_robinson(double y);	/*	For Robinson maps	*/
double GMT_left_sinusoidal(double y);	/*	For sinusoidal maps	*/
double GMT_right_sinusoidal(double y);	/*	For sinusoidal maps	*/
double GMT_robinson_spline (double xp, double *x, double *y, double *c);

#endif /* _GMT_PROJ_H */
