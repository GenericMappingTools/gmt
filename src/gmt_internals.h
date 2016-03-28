/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2016 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU Lesser General Public License as published by
 *	the Free Software Foundation; version 3 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

/*!
 * \file gmt_internals.h
 * \brief All lower-level functions and macros just needed within the dev library.
 * These are not available via gmt_dev.h.
 */

/* Authors:	P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
   Date:	1-OCT-2009
   Version:	5 API

*/

#ifndef _GMT_INTERNALS_H
#define _GMT_INTERNALS_H

#ifdef HAVE_FFTW3F
/* FFTW_planner_flags: FFTW_ESTIMATE, FFTW_MEASURE, FFTW_PATIENT, FFTW_EXHAUSTIVE */
#	include <fftw3.h>
#endif

/* LOCAL FUNCTIONS USED BY GMT_*.C ONLY - NOT PART OF GMT_DEV.H DISTRIBUTION */

/*--------------------------------------------------------------------
 *			GMT XINGS STRUCTURE DEFINITION
 *--------------------------------------------------------------------*/

struct GMT_XINGS {
        double xx[2], yy[2];    /* Cartesian coordinates of intersection with map boundary */
        double angle[2];        /* Angles of intersection */
        unsigned int sides[2];	/* Side id of intersection */
        unsigned int nx;	/* Number of intersections (1 or 2) */
};

EXTERN_MSC int gmtlib_regexp_match (struct GMT_CTRL *GMT, const char *subject, const char *pattern, bool caseless);
EXTERN_MSC int gmtlib_geo_C_format (struct GMT_CTRL *GMT);
EXTERN_MSC void gmtlib_init_cpt (struct GMT_CTRL *GMT, struct GMT_PALETTE *P);
EXTERN_MSC struct GMT_PALETTE * gmtlib_duplicate_palette (struct GMT_CTRL *GMT, struct GMT_PALETTE *P_from, unsigned int mode);
EXTERN_MSC unsigned int gmtlib_unit_lookup (struct GMT_CTRL *GMT, int c, unsigned int unit);
EXTERN_MSC void gmtlib_get_annot_label (struct GMT_CTRL *GMT, double val, char *label, bool do_minutes, bool do_seconds, bool do_hemi, unsigned int lonlat, bool worldmap);
EXTERN_MSC unsigned int gmtlib_coordinate_array (struct GMT_CTRL *GMT, double min, double max, struct GMT_PLOT_AXIS_ITEM *T, double **array, char ***labels);
EXTERN_MSC unsigned int gmtlib_linear_array (struct GMT_CTRL *GMT, double min, double max, double delta, double phase, double **array);
EXTERN_MSC unsigned int gmtlib_pow_array (struct GMT_CTRL *GMT, double min, double max, double delta, unsigned int x_or_y_or_z, double **array);
EXTERN_MSC int gmtlib_prepare_label (struct GMT_CTRL *GMT, double angle, unsigned int side, double x, double y, unsigned int type, double *line_angle, double *text_angle, unsigned int *justify);
EXTERN_MSC unsigned int gmtlib_time_array (struct GMT_CTRL *GMT, double min, double max, struct GMT_PLOT_AXIS_ITEM *T, double **array);
EXTERN_MSC void gmtlib_get_lon_minmax (struct GMT_CTRL *GMT, double *lon, uint64_t n, double *min, double *max);
EXTERN_MSC struct GMT_OGR * gmtlib_duplicate_ogr (struct GMT_CTRL *GMT, struct GMT_OGR *G);
EXTERN_MSC void gmtlib_free_ogr (struct GMT_CTRL *GMT, struct GMT_OGR **G, unsigned int mode);
EXTERN_MSC int gmtlib_ogr_get_geometry (char *item);
EXTERN_MSC int gmtlib_ogr_get_type (char *item);
EXTERN_MSC void gmtlib_plot_C_format (struct GMT_CTRL *GMT);
EXTERN_MSC void gmtlib_clock_C_format (struct GMT_CTRL *GMT, char *form, struct GMT_CLOCK_IO *S, unsigned int mode);
EXTERN_MSC void gmtlib_date_C_format (struct GMT_CTRL *GMT, char *form, struct GMT_DATE_IO *S, unsigned int mode);
EXTERN_MSC void * gmtio_ascii_textinput (struct GMT_CTRL *GMT, FILE *fp, uint64_t *ncol, int *status);
EXTERN_MSC double gmtlib_get_map_interval (struct GMT_CTRL *GMT, struct GMT_PLOT_AXIS_ITEM *T);
EXTERN_MSC unsigned int gmtlib_log_array (struct GMT_CTRL *GMT, double min, double max, double delta, double **array);
EXTERN_MSC int gmtlib_nc_get_att_text (struct GMT_CTRL *GMT, int ncid, int varid, char *name, char *text, size_t textlen);
EXTERN_MSC int gmtlib_akima (struct GMT_CTRL *GMT, double *x, double *y, uint64_t nx, double *c);
EXTERN_MSC int gmtlib_cspline (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, double *c);
EXTERN_MSC bool gmtlib_annot_pos (struct GMT_CTRL *GMT, double min, double max, struct GMT_PLOT_AXIS_ITEM *T, double coord[], double *pos);
EXTERN_MSC float gmtlib_decode (struct GMT_CTRL *GMT, void *vptr, uint64_t k, unsigned int type);
EXTERN_MSC void gmtlib_encode (struct GMT_CTRL *GMT, void *vptr, uint64_t k, float z, unsigned int type);
EXTERN_MSC struct GMT_CUSTOM_SYMBOL * gmtlib_get_custom_symbol (struct GMT_CTRL *GMT, char *name);
EXTERN_MSC void gmtlib_free_custom_symbols (struct GMT_CTRL *GMT);
EXTERN_MSC bool gmtlib_geo_to_dms (double val, int n_items, double fact, int *d, int *m,  int *s,  int *ix);
EXTERN_MSC double gmtlib_get_annot_offset (struct GMT_CTRL *GMT, bool *flip, unsigned int level);
EXTERN_MSC int gmtlib_get_coordinate_label (struct GMT_CTRL *GMT, char *string, struct GMT_PLOT_CALCLOCK *P, char *format, struct GMT_PLOT_AXIS_ITEM *T, double coord);
EXTERN_MSC void gmtlib_get_time_label (struct GMT_CTRL *GMT, char *string, struct GMT_PLOT_CALCLOCK *P, struct GMT_PLOT_AXIS_ITEM *T, double t);
EXTERN_MSC int gmtlib_getrgb_index (struct GMT_CTRL *GMT, double *rgb);
EXTERN_MSC char * gmtlib_getuserpath (struct GMT_CTRL *GMT, const char *stem, char *path);	/* Look for user file */
EXTERN_MSC size_t gmt_grd_data_size (struct GMT_CTRL *GMT, unsigned int format, float *nan_value);
EXTERN_MSC void gmtlib_init_ellipsoid (struct GMT_CTRL *GMT);
EXTERN_MSC void gmtlib_init_geodesic (struct GMT_CTRL *GMT);
EXTERN_MSC void gmtlib_io_init (struct GMT_CTRL *GMT);			/* Initialize pointers */
EXTERN_MSC uint64_t gmtlib_latpath (struct GMT_CTRL *GMT, double lat, double lon1, double lon2, double **x, double **y);
EXTERN_MSC uint64_t gmtlib_lonpath (struct GMT_CTRL *GMT, double lon, double lat1, double lat2, double **x, double **y);
EXTERN_MSC uint64_t gmtlib_map_path (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2, double **x, double **y);
EXTERN_MSC unsigned int gmtlib_map_latcross (struct GMT_CTRL *GMT, double lat, double west, double east, struct GMT_XINGS **xings);
EXTERN_MSC unsigned int gmtlib_map_loncross (struct GMT_CTRL *GMT, double lon, double south, double north, struct GMT_XINGS **xings);
EXTERN_MSC void gmtlib_rotate2D (struct GMT_CTRL *GMT, double x[], double y[], uint64_t n, double x0, double y0, double angle, double xp[], double yp[]);
EXTERN_MSC void gmtlib_set_bin_io (struct GMT_CTRL *GMT);
EXTERN_MSC uint64_t * gmtlib_split_line (struct GMT_CTRL *GMT, double **xx, double **yy, uint64_t *nn, bool add_crossings);
EXTERN_MSC int gmtlib_verify_time_step (struct GMT_CTRL *GMT, int step, char unit);	/* Check that time step and unit for time axis are OK  */
EXTERN_MSC int gmtlib_y2_to_y4_yearfix (struct GMT_CTRL *GMT, unsigned int y2);	/* Convert a 2-digit year to a 4-digit year */
EXTERN_MSC bool gmtlib_g_ymd_is_bad (int y, int m, int d);	/* Check range of month and day for Gregorian YMD calendar values  */
EXTERN_MSC bool gmtlib_iso_ywd_is_bad (int y, int w, int d);	/* Check range of week and day for ISO W calendar.  */
EXTERN_MSC int gmtlib_genper_map_clip_path (struct GMT_CTRL *GMT, uint64_t np, double *work_x, double *work_y);
EXTERN_MSC double gmtlib_half_map_width (struct GMT_CTRL *GMT, double y);
EXTERN_MSC void gmtlib_moment_interval (struct GMT_CTRL *GMT, struct GMT_MOMENT_INTERVAL *p, double dt_in, bool init); /* step a time axis by time units */
EXTERN_MSC int64_t gmtlib_rd_from_iywd (struct GMT_CTRL *GMT, int iy, int iw, int id);
EXTERN_MSC void gmtlib_scale_eqrad (struct GMT_CTRL *GMT);
EXTERN_MSC void gmtlib_enforce_rgb_triplets (struct GMT_CTRL *GMT, char *text, unsigned int size);
EXTERN_MSC struct GMT_TEXTSET * gmtlib_create_textset (struct GMT_CTRL *GMT, uint64_t n_tables, uint64_t n_segments, uint64_t n_rows, bool alloc_only);
EXTERN_MSC struct GMT_PALETTE * gmtlib_create_palette (struct GMT_CTRL *GMT, uint64_t n_colors);
EXTERN_MSC struct GMT_TEXTTABLE * gmtlib_read_texttable (struct GMT_CTRL *GMT, void *source, unsigned int source_type);
EXTERN_MSC int gmtlib_write_textset (struct GMT_CTRL *GMT, void *dest, unsigned int dest_type, struct GMT_TEXTSET *D, int table);
EXTERN_MSC struct GMT_TEXTSET * gmtlib_alloc_textset (struct GMT_CTRL *GMT, struct GMT_TEXTSET *Din, unsigned int mode);
EXTERN_MSC bool gmtlib_init_complex (struct GMT_GRID_HEADER *h, unsigned int complex_mode, uint64_t *imag_offset);
EXTERN_MSC struct GMT_MATRIX * gmtlib_duplicate_matrix (struct GMT_CTRL *GMT, struct GMT_MATRIX *M_in, bool duplicate_data);
EXTERN_MSC void gmtlib_init_rot_matrix (double R[3][3], double E[]);
EXTERN_MSC void gmtlib_load_rot_matrix (double w, double R[3][3], double E[]);
EXTERN_MSC int gmtlib_io_banner (struct GMT_CTRL *GMT, unsigned int direction);
EXTERN_MSC double gmtlib_great_circle_dist_degree (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2);
EXTERN_MSC int gmtlib_gmonth_length (int year, int month);
EXTERN_MSC void gmtlib_gcal_from_dt (struct GMT_CTRL *GMT, double t, struct GMT_GCAL *cal);	/* Break internal time into calendar and clock struct info  */
EXTERN_MSC int gmtlib_great_circle_intersection (struct GMT_CTRL *GMT, double A[], double B[], double C[], double X[], double *CX_dist);
EXTERN_MSC void gmtlib_get_point_from_r_az (struct GMT_CTRL *GMT, double lon0, double lat0, double r, double azim, double *lon1, double *lat1);
EXTERN_MSC bool gmtlib_check_url_name (char *fname);
EXTERN_MSC int64_t gmtlib_splitinteger (double value, int epsilon, double *doublepart);
EXTERN_MSC bool gmtlib_is_gleap (int gyear);
EXTERN_MSC void gmtlib_str_tolower (char *string);
EXTERN_MSC char *gmtlib_file_unitscale (char *name);
EXTERN_MSC void gmtlib_set_oblique_pole_and_origin (struct GMT_CTRL *GMT, double plon, double plat, double olon, double olat);
EXTERN_MSC int gmtlib_get_grdtype (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h);
EXTERN_MSC void gmtlib_grd_real_interleave (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, float *data);
EXTERN_MSC int gmtinit_backwards_SQ_parsing (struct GMT_CTRL *GMT, char option, char *item);
EXTERN_MSC int gmtlib_process_binary_input (struct GMT_CTRL *GMT, uint64_t n_read);
EXTERN_MSC uint64_t gmtlib_bin_colselect (struct GMT_CTRL *GMT);
EXTERN_MSC bool gmtlib_gap_detected (struct GMT_CTRL *GMT);
EXTERN_MSC int gmtlib_set_gap (struct GMT_CTRL *GMT);
EXTERN_MSC struct GMT_PS * gmtlib_create_ps (struct GMT_CTRL *GMT, uint64_t length);
EXTERN_MSC void gmtlib_inplace_transpose (float *A, unsigned int n_rows, unsigned int n_cols);
EXTERN_MSC void gmtlib_grd_flip_vertical (void *gridp, const unsigned n_cols, const unsigned n_rows, const unsigned n_stride, size_t cell_size);
EXTERN_MSC void gmtlib_free_dataset_ptr (struct GMT_CTRL *GMT, struct GMT_DATASET *data);
EXTERN_MSC void gmtlib_free_textset_ptr (struct GMT_CTRL *GMT, struct GMT_TEXTSET *data);
EXTERN_MSC void gmtlib_free_cpt_ptr (struct GMT_CTRL *GMT, struct GMT_PALETTE *P);
EXTERN_MSC void gmtlib_free_ps_ptr (struct GMT_CTRL *GMT, struct GMT_PS *P);
EXTERN_MSC unsigned int gmtlib_free_matrix_ptr (struct GMT_CTRL *GMT, struct GMT_MATRIX *M, bool free_matrix);
EXTERN_MSC unsigned int gmtlib_free_vector_ptr (struct GMT_CTRL *GMT, struct GMT_VECTOR *V, bool free_vector);
EXTERN_MSC void gmtlib_free_image_ptr (struct GMT_CTRL *GMT, struct GMT_IMAGE *I, bool free_image);
EXTERN_MSC struct GMT_IMAGE *gmtlib_duplicate_image (struct GMT_CTRL *GMT, struct GMT_IMAGE *I, unsigned int mode);

/* LOCAL MACROS USED BY GMT_*.C ONLY - NOT PART OF GMT_DEV.H DISTRIBUTION */

/* Definition for an error trap */
#ifdef DEBUG
#define GMT_err_trap(func_call) if ((err = (func_call)) != GMT_NOERROR) {GMT_Report(GMT->parent,GMT_MSG_NORMAL,"GMT_err_trap: %d\n", err);return(err);}
#else
#define GMT_err_trap(func_call) if ((err = (func_call)) != GMT_NOERROR) return(err)
#endif

/*! gmt_grd_get_size computes grid size including the padding, and doubles it if complex values */

#define gmt_grd_get_size(h) ((((h->complex_mode & GMT_GRID_IS_COMPLEX_MASK) > 0) + 1ULL) * h->mx * h->my)

/* Determine if current binary table has header */
#define GMT_binary_header(GMT,dir) (GMT->common.b.active[dir] && GMT->current.setting.io_header[dir] && GMT->current.setting.io_n_header_items)

/*! Checking of h,m,s */

#define GMT_hms_is_bad(h,m,s) ((h) < 0 || (h) > 23 || (m) < 0 || (m) > 59 || (s) < 0.0 || (s) >= 61.0)

/*! Force component to be in 0 <= s <= 255 range */
#define GMT_0_255_truncate(s) ((s < 0) ? 0 : ((s > 255) ? 255 : s))	/* Truncate to allowable 0-255 range */

#define GMT_axis_is_geo(C,axis) (C->current.io.col_type[GMT_IN][axis] & GMT_IS_GEO)

#define GMT_IS_PERSPECTIVE(C) (C->current.proj.projection == GMT_ORTHO || C->current.proj.projection == GMT_GENPER)
#define GMT_POLE_IS_POINT(C) ((C->current.proj.projection == GMT_OBLIQUE_MERC || C->current.proj.projection == GMT_OBLIQUE_MERC_POLE) || (C->current.proj.projection >= GMT_LAMBERT && C->current.proj.projection <= GMT_VANGRINTEN))
#define GMT_is_grdmapproject(C) (!strncmp (C->init.module_name, "grdproject", 10U) || !strncmp (C->init.module_name, "mapproject", 10U))

#define GMT_uneven_interval(unit) ((unit == 'o' || unit == 'O' || unit == 'k' || unit == 'K' || unit == 'R' || unit == 'r' || unit == 'D' || unit == 'd') ? true : false)	/* true for uneven units */

#endif /* _GMT_INTERNALS_H */
