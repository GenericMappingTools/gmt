/*--------------------------------------------------------------------
 *	$Id: gmt_internals.h,v 1.2 2011-03-15 02:06:36 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
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
/* gmt_internals.h  --  All lower-level functions needed within library.

   Authors:	P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
   Date:	1-OCT-2009
   Version:	5 API

*/

#ifndef _GMT_INTERNALS_H
#define _GMT_INTERNALS_H

EXTERN_MSC struct GMT_OGR * GMT_duplicate_ogr (struct GMT_CTRL *C, struct GMT_OGR *G);
EXTERN_MSC void GMT_free_ogr (struct GMT_CTRL *C, struct GMT_OGR **G, GMT_LONG mode);
EXTERN_MSC GMT_LONG ogr_get_geometry (char *item);
EXTERN_MSC GMT_LONG ogr_get_type (char *item);
EXTERN_MSC void GMT_plot_C_format (struct GMT_CTRL *C);
EXTERN_MSC void gmt_clock_C_format (struct GMT_CTRL *C, char *form, struct GMT_CLOCK_IO *S, GMT_LONG mode);
EXTERN_MSC void gmt_date_C_format (struct GMT_CTRL *C, char *form, struct GMT_DATE_IO *S, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_ascii_textinput (struct GMT_CTRL *C, FILE *fp, GMT_LONG *ncol, void **ptr);
EXTERN_MSC double GMT_get_map_interval (struct GMT_CTRL *C, GMT_LONG axis, GMT_LONG item);
EXTERN_MSC GMT_LONG GMT_log_array (struct GMT_CTRL *C, double min, double max, double delta, double **array);
EXTERN_MSC GMT_LONG GMT_nc_get_att_text (struct GMT_CTRL *C, int ncid, int varid, char *name, char *text, size_t textlen);
EXTERN_MSC GMT_LONG GMT_akima (struct GMT_CTRL *C, double *x, double *y, GMT_LONG nx, double *c);
EXTERN_MSC GMT_LONG GMT_cspline (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, double *c);
EXTERN_MSC GMT_LONG GMT_annot_pos (struct GMT_CTRL *C, double min, double max, struct GMT_PLOT_AXIS_ITEM *T, double coord[], double *pos);
EXTERN_MSC int GMT_comp_int_asc (const void *p_1, const void *p_2);
EXTERN_MSC float GMT_decode (struct GMT_CTRL *C, void *vptr, GMT_LONG k, GMT_LONG type);
EXTERN_MSC void GMT_encode (struct GMT_CTRL *C, void *vptr, GMT_LONG k, float z, GMT_LONG type);
EXTERN_MSC struct EPS * GMT_epsinfo (struct GMT_CTRL *C);
EXTERN_MSC GMT_LONG GMT_flip_justify (struct GMT_CTRL *C, GMT_LONG justify);
EXTERN_MSC struct GMT_CUSTOM_SYMBOL * GMT_get_custom_symbol (struct GMT_CTRL *C, char *name);
EXTERN_MSC void GMT_free_custom_symbols (struct GMT_CTRL *C);
EXTERN_MSC GMT_LONG GMT_geo_to_dms (struct GMT_CTRL *C, double val, GMT_LONG n_items, double fact, GMT_LONG *d, GMT_LONG *m,  GMT_LONG *s,  GMT_LONG *ix);
EXTERN_MSC double GMT_get_annot_offset (struct GMT_CTRL *C, GMT_LONG *flip, GMT_LONG level);
EXTERN_MSC GMT_LONG GMT_get_coordinate_label (struct GMT_CTRL *C, char *string, struct GMT_PLOT_CALCLOCK *P, char *format, struct GMT_PLOT_AXIS_ITEM *T, double coord);
EXTERN_MSC void GMT_get_primary_annot (struct GMT_CTRL *C, struct GMT_PLOT_AXIS *A, GMT_LONG *primary, GMT_LONG *secondary);
EXTERN_MSC void GMT_get_time_label (struct GMT_CTRL *C, char *string, struct GMT_PLOT_CALCLOCK *P, struct GMT_PLOT_AXIS_ITEM *T, double t);
EXTERN_MSC GMT_LONG GMT_getrgb_index (struct GMT_CTRL *C, double *rgb);
EXTERN_MSC char * GMT_getuserpath (struct GMT_CTRL *C, const char *stem, char *path);	/* Look for user file */
EXTERN_MSC GMT_LONG GMT_grd_data_size (struct GMT_CTRL *C, GMT_LONG format, double *nan_value);
EXTERN_MSC GMT_LONG GMT_hms_is_bad (struct GMT_CTRL *C, GMT_LONG h, GMT_LONG m, double s);	/* Check range of hours, min, and secs */
EXTERN_MSC void GMT_init_ellipsoid (struct GMT_CTRL *C);
EXTERN_MSC void GMT_io_init (struct GMT_CTRL *C);			/* Initialize pointers */
EXTERN_MSC GMT_LONG GMT_latpath (struct GMT_CTRL *C, double lat, double lon1, double lon2, double **x, double **y);
EXTERN_MSC GMT_LONG GMT_lonpath (struct GMT_CTRL *C, double lon, double lat1, double lat2, double **x, double **y);
EXTERN_MSC double GMT_left_boundary (struct GMT_CTRL *C, double y);
EXTERN_MSC double GMT_right_boundary (struct GMT_CTRL *C, double y);
EXTERN_MSC GMT_LONG GMT_map_latcross (struct GMT_CTRL *C, double lat, double west, double east, struct GMT_XINGS **xings);
EXTERN_MSC GMT_LONG GMT_map_loncross (struct GMT_CTRL *C, double lon, double south, double north, struct GMT_XINGS **xings);
EXTERN_MSC void GMT_rotate2D (struct GMT_CTRL *C, double x[], double y[], GMT_LONG n, double x0, double y0, double angle, double xp[], double yp[]);
EXTERN_MSC void GMT_set_bin_input (struct GMT_CTRL *C);
EXTERN_MSC GMT_LONG * GMT_split_line (struct GMT_CTRL *C, double **xx, double **yy, GMT_LONG *nn, GMT_LONG add_crossings);
EXTERN_MSC GMT_LONG GMT_strlcmp (char *str1, char *str2);
EXTERN_MSC GMT_LONG GMT_strrcmp (char *str1, char *str2);
EXTERN_MSC GMT_LONG GMT_unit_lookup (struct GMT_CTRL *C, GMT_LONG c, GMT_LONG default_unit);
EXTERN_MSC GMT_LONG GMT_verify_time_step (struct GMT_CTRL *C, GMT_LONG step, char unit);	/* Check that time step and unit for time axis are OK  */
EXTERN_MSC double GMT_xx_to_x (struct GMT_CTRL *C, double xx);
EXTERN_MSC double GMT_yy_to_y (struct GMT_CTRL *C, double yy);
EXTERN_MSC double GMT_zz_to_z (struct GMT_CTRL *C, double zz);
EXTERN_MSC GMT_LONG GMT_y2_to_y4_yearfix (struct GMT_CTRL *C, GMT_LONG y2);	/* Convert a 2-digit year to a 4-digit year */
EXTERN_MSC GMT_LONG GMT_g_ymd_is_bad (struct GMT_CTRL *C, GMT_LONG y, GMT_LONG m, GMT_LONG d);	/* Check range of month and day for Gregorian YMD calendar values  */
EXTERN_MSC GMT_LONG GMT_iso_ywd_is_bad (struct GMT_CTRL *C, GMT_LONG y, GMT_LONG w, GMT_LONG d);	/* Check range of week and day for ISO W calendar.  */
EXTERN_MSC GMT_LONG GMT_genper_map_clip_path (struct GMT_CTRL *C, GMT_LONG np, double *work_x, double *work_y);
EXTERN_MSC double GMT_half_map_width (struct GMT_CTRL *C, double y);
EXTERN_MSC GMT_LONG GMT_map_path (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2, double **x, double **y);
EXTERN_MSC void GMT_moment_interval (struct GMT_CTRL *C, struct GMT_MOMENT_INTERVAL *p, double dt_in, GMT_LONG init); /* step a time axis by time units */
EXTERN_MSC GMT_LONG GMT_rd_from_iywd (struct GMT_CTRL *C, GMT_LONG iy, GMT_LONG iw, GMT_LONG id);
EXTERN_MSC GMT_LONG GMT_grd_format_decoder (struct GMT_CTRL *C, const char *code);
EXTERN_MSC GMT_LONG GMT_grd_prep_io (struct GMT_CTRL *C, struct GRD_HEADER *header, double wesn[], GMT_LONG *width, GMT_LONG *height, GMT_LONG *first_col, GMT_LONG *last_col, GMT_LONG *first_row, GMT_LONG *last_row, GMT_LONG **index);
EXTERN_MSC void GMT_scale_eqrad (struct GMT_CTRL *C);
EXTERN_MSC void GMT_enforce_rgb_triplets (struct GMT_CTRL *C, char *text, GMT_LONG size);
GMT_LONG GMT_get_fill_from_z (struct GMT_CTRL *C, struct GMT_PALETTE *P, double value, struct GMT_FILL *fill);
GMT_LONG GMT_update_grd_info (struct GMT_CTRL *C, char *file, struct GRD_HEADER *header);
EXTERN_MSC struct GMT_TEXTSET *GMT_create_textset (struct GMT_CTRL *C, GMT_LONG n_tables, GMT_LONG n_segments, GMT_LONG n_rows);
EXTERN_MSC struct GMT_PALETTE *GMT_create_palette (struct GMT_CTRL *C, GMT_LONG n_colors);
EXTERN_MSC GMT_LONG GMT_read_texttable (struct GMT_CTRL *C, void *source, GMT_LONG source_type, struct GMT_TEXT_TABLE **table);
EXTERN_MSC GMT_LONG GMT_write_texttable (struct GMT_CTRL *C, void *dest, GMT_LONG dest_type, struct GMT_TEXT_TABLE *table, GMT_LONG io_mode);
EXTERN_MSC GMT_LONG GMT_write_textset (struct GMT_CTRL *C, void *dest, GMT_LONG dest_type, struct GMT_TEXTSET *D, GMT_LONG table);
EXTERN_MSC void GMT_alloc_textset (struct GMT_CTRL *C, struct GMT_TEXTSET *Din, struct GMT_TEXTSET **Dout, GMT_LONG mode);

EXTERN_MSC int GMT_comp_double_asc (const void *p_1, const void *p_2);
EXTERN_MSC GMT_LONG gmonth_length (struct GMT_CTRL *C, GMT_LONG year, GMT_LONG month);
EXTERN_MSC void gcal_from_dt (struct GMT_CTRL *C, double t, struct GMT_gcal *cal);	/* Break internal time into calendar and clock struct info  */
EXTERN_MSC GMT_LONG GMT_great_circle_intersection (struct GMT_CTRL *T, double A[], double B[], double C[], double X[], double *CX_dist);
EXTERN_MSC double GMT_great_circle_dist_degree (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2);
EXTERN_MSC void GMT_get_point_from_r_az (struct GMT_CTRL *C, double lon0, double lat0, double r, double azim, double *lon1, double *lat1);
EXTERN_MSC GMT_LONG gmt_parse_b_option (struct GMT_CTRL *C, char *text);
EXTERN_MSC GMT_LONG GMT_fix_up_path_cartesian (struct GMT_CTRL *C, double **a_x, double **a_y, GMT_LONG n, double step, GMT_LONG mode);

/* Functions declared in gmt_proj.c */

EXTERN_MSC void GMT_vpolar (struct GMT_CTRL *C, double lon0);
EXTERN_MSC void GMT_vmerc (struct GMT_CTRL *C, double lon0, double slat);
EXTERN_MSC void GMT_vcyleq (struct GMT_CTRL *C, double lon0, double slat);
EXTERN_MSC void GMT_vcyleqdist (struct GMT_CTRL *C, double lon0, double slat);
EXTERN_MSC void GMT_vcylstereo (struct GMT_CTRL *C, double lon0, double slat);
EXTERN_MSC void GMT_vmiller (struct GMT_CTRL *C, double lon0);
EXTERN_MSC void GMT_vstereo (struct GMT_CTRL *C, double lon0, double lat0, double horizon);
EXTERN_MSC void GMT_vlamb (struct GMT_CTRL *C, double lon0, double lat0, double pha, double phb);
EXTERN_MSC void GMT_vtm (struct GMT_CTRL *C, double lon0, double lat0);
EXTERN_MSC void GMT_vlambeq (struct GMT_CTRL *C, double lon0, double lat0, double horizon);
EXTERN_MSC void GMT_vortho (struct GMT_CTRL *C, double lon0, double lat0, double horizon);
EXTERN_MSC void GMT_vgenper (struct GMT_CTRL *C, double lon0, double lat0, double altitude, double azimuth, double tilt, double rotation, double width, double height);
EXTERN_MSC void GMT_vgnomonic (struct GMT_CTRL *C, double lon0, double lat0, double horizon);
EXTERN_MSC void GMT_vazeqdist (struct GMT_CTRL *C, double lon0, double lat0, double horizon);
EXTERN_MSC void GMT_vmollweide (struct GMT_CTRL *C, double lon0, double scale);
EXTERN_MSC void GMT_vhammer (struct GMT_CTRL *C, double lon0, double scale);
EXTERN_MSC void GMT_vwinkel (struct GMT_CTRL *C, double lon0, double scale);
EXTERN_MSC void GMT_veckert4 (struct GMT_CTRL *C, double lon0);
EXTERN_MSC void GMT_veckert6 (struct GMT_CTRL *C, double lon0);
EXTERN_MSC void GMT_vrobinson (struct GMT_CTRL *C, double lon0);
EXTERN_MSC void GMT_vsinusoidal (struct GMT_CTRL *C, double lon0);
EXTERN_MSC void GMT_vcassini (struct GMT_CTRL *C, double lon0, double lat0);
EXTERN_MSC void GMT_valbers (struct GMT_CTRL *C, double lon0, double lat0, double ph1, double ph2);
EXTERN_MSC void GMT_valbers_sph (struct GMT_CTRL *C, double lon0, double lat0, double ph1, double ph2);
EXTERN_MSC void GMT_veconic (struct GMT_CTRL *C, double lon0, double lat0, double ph1, double ph2);
EXTERN_MSC void GMT_vpolyconic (struct GMT_CTRL *C, double lon0, double lat0);
EXTERN_MSC void GMT_vgrinten (struct GMT_CTRL *C, double lon0, double scale);
EXTERN_MSC void GMT_polar (struct GMT_CTRL *C, double x, double y, double *x_i, double *y_i);		/* Convert x/y (being theta,r) to x,y	*/
EXTERN_MSC void GMT_ipolar (struct GMT_CTRL *C, double *x, double *y, double x_i, double y_i);		/* Convert (theta,r) to x,y	*/
EXTERN_MSC void GMT_translin (struct GMT_CTRL *C, double forw, double *inv);				/* Forward linear	*/
EXTERN_MSC void GMT_translind (struct GMT_CTRL *C, double forw, double *inv);				/* Forward linear, but using 0-360 degrees	*/
EXTERN_MSC void GMT_itranslin (struct GMT_CTRL *C, double *forw, double inv);				/* Inverse linear	*/
EXTERN_MSC void GMT_itranslind (struct GMT_CTRL *C, double *forw, double inv);				/* Inverse linear, but using 0-360 degrees	*/
EXTERN_MSC void GMT_translog10 (struct GMT_CTRL *C, double forw, double *inv);				/* Forward log10	*/
EXTERN_MSC void GMT_itranslog10 (struct GMT_CTRL *C, double *forw, double inv);			/* Inverse log10	*/
EXTERN_MSC void GMT_transpowx (struct GMT_CTRL *C, double x, double *x_in);				/* Forward pow x	*/
EXTERN_MSC void GMT_itranspowx (struct GMT_CTRL *C, double *x, double x_in);				/* Inverse pow x	*/
EXTERN_MSC void GMT_transpowy (struct GMT_CTRL *C, double y, double *y_in);				/* Forward pow y 	*/
EXTERN_MSC void GMT_itranspowy (struct GMT_CTRL *C, double *y, double y_in);				/* Inverse pow y 	*/
EXTERN_MSC void GMT_transpowz (struct GMT_CTRL *C, double z, double *z_in);				/* Forward pow z 	*/
EXTERN_MSC void GMT_itranspowz (struct GMT_CTRL *C, double *z, double z_in);				/* Inverse pow z 	*/
EXTERN_MSC void GMT_albers (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Albers)	*/
EXTERN_MSC void GMT_ialbers (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Albers) to lon/lat	*/
EXTERN_MSC void GMT_econic (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Equidistant Conic)	*/
EXTERN_MSC void GMT_ieconic (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Equidistant Conic) to lon/lat	*/
EXTERN_MSC void GMT_polyconic (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Polyconic)	*/
EXTERN_MSC void GMT_ipolyconic (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Polyconic) to lon/lat	*/
EXTERN_MSC void GMT_albers_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Albers Spherical)	*/
EXTERN_MSC void GMT_ialbers_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Albers Spherical) to lon/lat	*/
EXTERN_MSC void GMT_azeqdist (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Azimuthal equal-distance)	*/
EXTERN_MSC void GMT_iazeqdist (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Azimuthal equal-distance) to lon/lat	*/
EXTERN_MSC void GMT_cassini (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Cassini)	*/
EXTERN_MSC void GMT_icassini (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Cassini) to lon/lat	*/
EXTERN_MSC void GMT_cassini_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Cassini Spherical)	*/
EXTERN_MSC void GMT_icassini_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Cassini Spherical) to lon/lat	*/
EXTERN_MSC void GMT_hammer (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Hammer-Aitoff)	*/
EXTERN_MSC void GMT_ihammer (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Hammer-Aitoff) to lon/lat	*/
EXTERN_MSC void GMT_grinten (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (van der Grinten)	*/
EXTERN_MSC void GMT_igrinten (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (van der Grinten) to lon/lat	*/
EXTERN_MSC void GMT_merc_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Mercator Spherical)	*/
EXTERN_MSC void GMT_imerc_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Mercator Spherical) to lon/lat	*/
EXTERN_MSC void GMT_plrs (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Polar)		*/
EXTERN_MSC void GMT_iplrs (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);		/* Convert x/y (Polar) to lon/lat		*/
EXTERN_MSC void GMT_plrs_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Polar Spherical)		*/
EXTERN_MSC void GMT_iplrs_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Polar Spherical) to lon/lat		*/
EXTERN_MSC void GMT_lamb (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Lambert)	*/
EXTERN_MSC void GMT_ilamb (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);		/* Convert x/y (Lambert) to lon/lat 	*/
EXTERN_MSC void GMT_lamb_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Lambert Spherical)	*/
EXTERN_MSC void GMT_ilamb_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Lambert Spherical) to lon/lat 	*/
EXTERN_MSC void GMT_oblmrc (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Oblique Mercator)	*/
EXTERN_MSC void GMT_ioblmrc (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Oblique Mercator) to lon/lat 	*/
EXTERN_MSC void GMT_genper (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (ORTHO)  */
EXTERN_MSC void GMT_igenper (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (ORTHO) to lon/lat  */
EXTERN_MSC void GMT_ortho (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (GMT_ORTHO)	*/
EXTERN_MSC void GMT_iortho (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);		/* Convert x/y (GMT_ORTHO) to lon/lat 	*/
EXTERN_MSC void GMT_gnomonic (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (GMT_GNOMONIC)	*/
EXTERN_MSC void GMT_ignomonic (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (GMT_GNOMONIC) to lon/lat 	*/
EXTERN_MSC void GMT_sinusoidal (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (GMT_SINUSOIDAL)	*/
EXTERN_MSC void GMT_isinusoidal (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (GMT_SINUSOIDAL) to lon/lat 	*/
EXTERN_MSC void GMT_tm (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (TM)	*/
EXTERN_MSC void GMT_itm (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);		/* Convert x/y (TM) to lon/lat 	*/
EXTERN_MSC void GMT_tm_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (GMT_TM Spherical)	*/
EXTERN_MSC void GMT_itm_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (GMT_TM Spherical) to lon/lat 	*/
EXTERN_MSC void GMT_utm (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (UTM)	*/
EXTERN_MSC void GMT_iutm (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);		/* Convert x/y (UTM) to lon/lat 	*/
EXTERN_MSC void GMT_utm_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (UTM Spherical)	*/
EXTERN_MSC void GMT_iutm_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (UTM Spherical) to lon/lat 	*/
EXTERN_MSC void GMT_winkel (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Winkel)	*/
EXTERN_MSC void GMT_iwinkel (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Winkel) to lon/lat	*/
EXTERN_MSC void GMT_eckert4 (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Eckert IV)	*/
EXTERN_MSC void GMT_ieckert4 (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Eckert IV) to lon/lat	*/
EXTERN_MSC void GMT_eckert6 (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Eckert VI)	*/
EXTERN_MSC void GMT_ieckert6 (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Eckert VI) to lon/lat	*/
EXTERN_MSC void GMT_robinson (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Robinson)	*/
EXTERN_MSC void GMT_irobinson (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Robinson) to lon/lat	*/
EXTERN_MSC void GMT_stereo1 (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Stereographic)	*/
EXTERN_MSC void GMT_stereo2 (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Stereographic, equatorial view)	*/
EXTERN_MSC void GMT_istereo (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Stereographic) to lon/lat 	*/
EXTERN_MSC void GMT_stereo1_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Stereographic Spherical)	*/
EXTERN_MSC void GMT_stereo2_sph (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Stereographic Spherical, equatorial view)	*/
EXTERN_MSC void GMT_istereo_sph (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Stereographic Spherical) to lon/lat 	*/
EXTERN_MSC void GMT_lambeq (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Lambert Azimuthal Equal-Area)	*/
EXTERN_MSC void GMT_ilambeq (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Lambert Azimuthal Equal-Area) to lon/lat 	*/
EXTERN_MSC void GMT_mollweide (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Mollweide Equal-Area)	*/
EXTERN_MSC void GMT_imollweide (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Mollweide Equal-Area) to lon/lat 	*/
EXTERN_MSC void GMT_cyleq (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Cylindrical Equal-Area)	*/
EXTERN_MSC void GMT_icyleq (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);		/* Convert x/y (Cylindrical Equal-Area) to lon/lat 	*/
EXTERN_MSC void GMT_cyleqdist (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Cylindrical Equidistant)	*/
EXTERN_MSC void GMT_icyleqdist (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Cylindrical Equidistant) to lon/lat 	*/
EXTERN_MSC void GMT_miller (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);		/* Convert lon/lat to x/y (Miller Cylindrical)	*/
EXTERN_MSC void GMT_imiller (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Miller Cylindrical) to lon/lat 	*/
EXTERN_MSC void GMT_cylstereo (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);	/* Convert lon/lat to x/y (Cylindrical Stereographic)	*/
EXTERN_MSC void GMT_icylstereo (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);	/* Convert x/y (Cylindrical Stereographic) to lon/lat 	*/
EXTERN_MSC void GMT_obl (struct GMT_CTRL *C, double lon, double lat, double *olon, double *olat);	/* Convert lon/loat to oblique lon/lat		*/
EXTERN_MSC void GMT_iobl (struct GMT_CTRL *C, double *lon, double *lat, double olon, double olat);	/* Convert oblique lon/lat to regular lon/lat	*/
EXTERN_MSC double GMT_left_winkel (struct GMT_CTRL *C, double y);	/* For Winkel maps	*/
EXTERN_MSC double GMT_right_winkel (struct GMT_CTRL *C, double y);	/* For Winkel maps	*/
EXTERN_MSC double GMT_left_eckert4 (struct GMT_CTRL *C, double y);	/* For Eckert IV maps	*/
EXTERN_MSC double GMT_right_eckert4 (struct GMT_CTRL *C, double y);	/* For Eckert IV maps	*/
EXTERN_MSC double GMT_left_eckert6 (struct GMT_CTRL *C, double y);	/* For Eckert VI maps	*/
EXTERN_MSC double GMT_right_eckert6 (struct GMT_CTRL *C, double y);	/* For Eckert VI maps	*/
EXTERN_MSC double GMT_left_robinson (struct GMT_CTRL *C, double y);	/* For Robinson maps	*/
EXTERN_MSC double GMT_right_robinson (struct GMT_CTRL *C, double y);	/* For Robinson maps	*/
EXTERN_MSC double GMT_left_sinusoidal (struct GMT_CTRL *C, double y);	/* For sinusoidal maps	*/
EXTERN_MSC double GMT_right_sinusoidal (struct GMT_CTRL *C, double y);	/* For sinusoidal maps	*/
EXTERN_MSC double GMT_left_polyconic (struct GMT_CTRL *C, double y);	/* For polyconic maps	*/
EXTERN_MSC double GMT_right_polyconic (struct GMT_CTRL *C, double y);	/* For polyconic maps	*/
EXTERN_MSC double GMT_robinson_spline (struct GMT_CTRL *C, double xp, double *x, double *y, double *c);

#endif /* _GMT_INTERNALS_H */
