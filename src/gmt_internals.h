/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/

/*!
 * \file gmt_internals.h
 * \brief All lower-level functions and macros just needed within the dev library.
 * These are not available via gmt_dev.h.
 */

/* Authors:	P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
   Date:	1-OCT-2009
   Version:	6 API

*/

#ifndef GMT_INTERNALS_H
#define GMT_INTERNALS_H

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

EXTERN_MSC char *opt (struct GMTAPI_CTRL *API,char code);

EXTERN_MSC bool gmtinit_B_is_frame (struct GMT_CTRL *GMT, char *in);
EXTERN_MSC int64_t gmt_parse_index_range (struct GMT_CTRL *GMT, char *p, int64_t *start, int64_t *stop);
EXTERN_MSC void gmtlib_handle_escape_text (char *text, char key, int way);
EXTERN_MSC int gmtlib_ascii_output_trailing_text (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *ptr, char *txt);
EXTERN_MSC void gmtlib_reparse_i_option (struct GMT_CTRL *GMT, uint64_t n_columns);
EXTERN_MSC void gmtlib_reparse_o_option (struct GMT_CTRL *GMT, uint64_t n_columns);
EXTERN_MSC void gmtlib_get_cpt_level (struct GMTAPI_CTRL *API, int *fig, int *subplot, char *panel, int *inset);
EXTERN_MSC unsigned int gmtlib_char_count (char *txt, char c);
EXTERN_MSC void gmt_check_modern_oneliner (struct GMTAPI_CTRL *API, const char *module, struct GMT_OPTION *head);
EXTERN_MSC void gmtlib_set_KOP_strings (struct GMTAPI_CTRL *API);
EXTERN_MSC bool gmtlib_is_modern_name (struct GMTAPI_CTRL *API, const char *module);
EXTERN_MSC const char *gmtlib_get_active_name (struct GMTAPI_CTRL *API, const char *module);
EXTERN_MSC double gmtlib_cartesian_dist (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1);
EXTERN_MSC double gmtlib_cartesian_dist_proj (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2);
EXTERN_MSC uint64_t gmtlib_read_list (struct GMT_CTRL *GMT, char *file, char ***list);
EXTERN_MSC void gmtlib_finalize_dataset (struct GMT_CTRL *GMT, struct GMT_DATASET *D);
EXTERN_MSC bool gmtlib_maybe_abstime (struct GMT_CTRL *GMT, char *txt, bool *no_T);
EXTERN_MSC void gmtlib_update_outcol_type (struct GMT_CTRL *GMT, uint64_t n);
EXTERN_MSC void gmtlib_reset_input (struct GMT_CTRL *GMT);
EXTERN_MSC bool gmtlib_file_is_srtmrequest (struct GMTAPI_CTRL *API, const char *file, unsigned int *res, bool *ocean);
EXTERN_MSC bool gmtlib_file_is_srtmlist (struct GMTAPI_CTRL *API, const char *file);
EXTERN_MSC char *gmtlib_get_srtmlist  (struct GMTAPI_CTRL *API, double wesn[], unsigned int res, bool ocean);
EXTERN_MSC struct GMT_GRID * gmtlib_assemble_srtm (struct GMTAPI_CTRL *API, double *region, char *file);
EXTERN_MSC bool gmtlib_fig_is_ps (struct GMT_CTRL *GMT);
EXTERN_MSC void gmtlib_refpoint_to_panel_xy (struct GMT_CTRL *GMT, int refpoint, struct GMT_SUBPLOT *P, double *x, double *y);
EXTERN_MSC int gmtlib_read_figures (struct GMT_CTRL *API, unsigned int mode, struct GMT_FIGURE **figs);
EXTERN_MSC bool gmtlib_file_is_downloadable (struct GMT_CTRL *GMT, const char *file, unsigned int *kind);
EXTERN_MSC unsigned int gmtlib_get_pos_of_filename (const char *url);
EXTERN_MSC bool gmtlib_is_color (struct GMT_CTRL *GMT, char *word);
EXTERN_MSC char * gmtlib_putfill (struct GMT_CTRL *GMT, struct GMT_FILL *F);
EXTERN_MSC char * gmtlib_putcmyk (struct GMT_CTRL *GMT, double *cmyk);
EXTERN_MSC char * gmtlib_puthsv (struct GMT_CTRL *GMT, double *hsv);
EXTERN_MSC enum gmt_enum_units gmtlib_get_unit_number (struct GMT_CTRL *GMT, char unit);
EXTERN_MSC void gmtlib_explain_options (struct GMT_CTRL *GMT, char *options);
EXTERN_MSC char * gmtlib_putparameter (struct GMT_CTRL *GMT, const char *keyword);
EXTERN_MSC unsigned int gmtlib_setparameter (struct GMT_CTRL *GMT, const char *keyword, char *value, bool core);
EXTERN_MSC int gmtlib_report_func (struct GMT_CTRL *GMT, unsigned int level, const char *source_line, const char *format, ...);
EXTERN_MSC int gmtlib_get_num_processors ();
EXTERN_MSC int gmtlib_bcr_get_img (struct GMT_CTRL *GMT, struct GMT_IMAGE *G, double xx, double yy, unsigned char *z);		/* Compute z(x,y) from bcr structure and image */
EXTERN_MSC void gmtlib_suggest_fft_dim (struct GMT_CTRL *GMT, unsigned int nx, unsigned int ny, struct GMT_FFT_SUGGESTION *fft_sug, bool do_print);
EXTERN_MSC int gmtlib_read_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header);
EXTERN_MSC int gmtlib_write_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header);
EXTERN_MSC int gmtlib_read_grd (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double *wesn, unsigned int *pad, int complex_mode);
EXTERN_MSC int gmtlib_write_grd (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header, gmt_grdfloat *grid, double *wesn, unsigned int *pad, int complex_mode);
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
EXTERN_MSC gmt_grdfloat gmtlib_decode (struct GMT_CTRL *GMT, void *vptr, uint64_t k, unsigned int type);
EXTERN_MSC void gmtlib_encode (struct GMT_CTRL *GMT, void *vptr, uint64_t k, gmt_grdfloat z, unsigned int type);
EXTERN_MSC struct GMT_CUSTOM_SYMBOL * gmtlib_get_custom_symbol (struct GMT_CTRL *GMT, char *name);
EXTERN_MSC void gmtlib_free_custom_symbols (struct GMT_CTRL *GMT);
EXTERN_MSC bool gmtlib_geo_to_dms (double val, int n_items, double fact, int *d, int *m,  int *s,  int *ix);
EXTERN_MSC int gmtlib_get_coordinate_label (struct GMT_CTRL *GMT, char *string, struct GMT_PLOT_CALCLOCK *P, char *format, struct GMT_PLOT_AXIS_ITEM *T, double coord);
EXTERN_MSC void gmtlib_get_time_label (struct GMT_CTRL *GMT, char *string, struct GMT_PLOT_CALCLOCK *P, struct GMT_PLOT_AXIS_ITEM *T, double t);
EXTERN_MSC int gmtlib_getrgb_index (struct GMT_CTRL *GMT, double *rgb);
EXTERN_MSC char * gmtlib_getuserpath (struct GMT_CTRL *GMT, const char *stem, char *path);	/* Look for user file */
EXTERN_MSC size_t gmt_grd_data_size (struct GMT_CTRL *GMT, unsigned int format, gmt_grdfloat *nan_value);
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
EXTERN_MSC void gmtlib_moment_interval (struct GMT_CTRL *GMT, struct GMT_MOMENT_INTERVAL *p, double dt_in, bool init); /* step a time axis by time units */
EXTERN_MSC int64_t gmtlib_rd_from_iywd (struct GMT_CTRL *GMT, int iy, int iw, int id);
EXTERN_MSC void gmtlib_scale_eqrad (struct GMT_CTRL *GMT);
EXTERN_MSC void gmtlib_enforce_rgb_triplets (struct GMT_CTRL *GMT, char *text, unsigned int size);
EXTERN_MSC struct GMT_PALETTE * gmtlib_create_palette (struct GMT_CTRL *GMT, uint64_t n_colors);
EXTERN_MSC bool gmtlib_init_complex (struct GMT_GRID_HEADER *h, unsigned int complex_mode, uint64_t *imag_offset);
EXTERN_MSC struct GMT_MATRIX * gmtlib_duplicate_matrix (struct GMT_CTRL *GMT, struct GMT_MATRIX *M_in, unsigned int mode);
EXTERN_MSC struct GMT_VECTOR * gmtlib_duplicate_vector (struct GMT_CTRL *GMT, struct GMT_VECTOR *V_in, unsigned int mode);
EXTERN_MSC int gmtlib_alloc_vectors (struct GMT_CTRL *GMT, struct GMT_VECTOR *V, uint64_t n_alloc);
EXTERN_MSC void gmtlib_init_rot_matrix (double R[3][3], double E[]);
EXTERN_MSC void gmtlib_load_rot_matrix (double w, double R[3][3], double E[]);
EXTERN_MSC int gmtlib_io_banner (struct GMT_CTRL *GMT, unsigned int direction);
EXTERN_MSC double gmtlib_great_circle_dist_degree (struct GMT_CTRL *GMT, double lon1, double lat1, double lon2, double lat2);
EXTERN_MSC int gmtlib_gmonth_length (int year, int month);
EXTERN_MSC int gmtlib_great_circle_intersection (struct GMT_CTRL *GMT, double A[], double B[], double C[], double X[], double *CX_dist);
EXTERN_MSC int gmtlib_small_circle_intersection (struct GMT_CTRL *GMT, double Ax, double Ay, double Ar, double Bx, double By, double Br, double lon[], double lat[]);
EXTERN_MSC void gmtlib_get_point_from_r_az (struct GMT_CTRL *GMT, double lon0, double lat0, double r, double azim, double *lon1, double *lat1);
EXTERN_MSC bool gmtlib_check_url_name (char *fname);
EXTERN_MSC int64_t gmtlib_splitinteger (double value, int epsilon, double *doublepart);
EXTERN_MSC bool gmtlib_is_gleap (int gyear);
EXTERN_MSC char *gmtlib_file_unitscale (char *name);
EXTERN_MSC void gmtlib_set_oblique_pole_and_origin (struct GMT_CTRL *GMT, double plon, double plat, double olon, double olat);
EXTERN_MSC int gmtlib_get_grdtype (struct GMT_CTRL *GMT, unsigned int direction, struct GMT_GRID_HEADER *h);
EXTERN_MSC void gmtlib_grd_real_interleave (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, gmt_grdfloat *data);
EXTERN_MSC void gmtlib_grd_get_units (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
EXTERN_MSC void gmtlib_grd_set_units (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header);
EXTERN_MSC int gmtinit_backwards_SQ_parsing (struct GMT_CTRL *GMT, char option, char *item);
EXTERN_MSC int gmtlib_process_binary_input (struct GMT_CTRL *GMT, uint64_t n_read);
EXTERN_MSC uint64_t gmtlib_bin_colselect (struct GMT_CTRL *GMT);
EXTERN_MSC bool gmtlib_gap_detected (struct GMT_CTRL *GMT);
EXTERN_MSC int gmtlib_set_gap (struct GMT_CTRL *GMT);
EXTERN_MSC struct GMT_POSTSCRIPT * gmtlib_create_ps (struct GMT_CTRL *GMT, uint64_t length);
EXTERN_MSC struct GMT_POSTSCRIPT * gmtlib_duplicate_ps (struct GMT_CTRL *GMT, struct GMT_POSTSCRIPT *P_from, unsigned int mode);
EXTERN_MSC void gmtlib_free_ps (struct GMT_CTRL *GMT, struct GMT_POSTSCRIPT **P);
EXTERN_MSC struct GMT_POSTSCRIPT * gmtlib_read_ps (struct GMT_CTRL *GMT, void *source, unsigned int source_type, unsigned int mode);
EXTERN_MSC int gmtlib_write_ps (struct GMT_CTRL *GMT, void *dest, unsigned int dest_type, unsigned int mode, struct GMT_POSTSCRIPT *P);
EXTERN_MSC void gmtlib_copy_ps (struct GMT_CTRL *GMT, struct GMT_POSTSCRIPT *P_copy, struct GMT_POSTSCRIPT *P_obj);
EXTERN_MSC void gmtlib_inplace_transpose (gmt_grdfloat *A, unsigned int n_rows, unsigned int n_cols);
EXTERN_MSC void gmtlib_free_dataset_ptr (struct GMT_CTRL *GMT, struct GMT_DATASET *data);
EXTERN_MSC void gmtlib_free_cpt_ptr (struct GMT_CTRL *GMT, struct GMT_PALETTE *P);
EXTERN_MSC void gmtlib_free_ps_ptr (struct GMT_CTRL *GMT, struct GMT_POSTSCRIPT *P);
EXTERN_MSC unsigned int gmtlib_free_matrix_ptr (struct GMT_CTRL *GMT, struct GMT_MATRIX *M, bool free_matrix);
EXTERN_MSC unsigned int gmtlib_free_vector_ptr (struct GMT_CTRL *GMT, struct GMT_VECTOR *V, bool free_vector);
EXTERN_MSC void gmtlib_union_transpose (struct GMT_CTRL *GMT, union GMT_UNIVECTOR *m, const uint64_t n_rows, const uint64_t n_columns, unsigned int type);
EXTERN_MSC void gmtlib_free_image_ptr (struct GMT_CTRL *GMT, struct GMT_IMAGE *I, bool free_image);
EXTERN_MSC struct GMT_IMAGE *gmtlib_duplicate_image (struct GMT_CTRL *GMT, struct GMT_IMAGE *I, unsigned int mode);
EXTERN_MSC int gmtlib_scanf_geodim (struct GMT_CTRL *GMT, char *s, double *val);
EXTERN_MSC unsigned int gmtlib_conv_text2datarec (struct GMT_CTRL *GMT, char *record, unsigned int ncols, double *out);
EXTERN_MSC void gmtlib_free_list (struct GMT_CTRL *GMT, char **list, uint64_t n);
EXTERN_MSC p_to_io_func gmtlib_get_io_ptr (struct GMT_CTRL *GMT, int direction, enum GMT_swap_direction swap, char type);
EXTERN_MSC void gmtlib_io_binary_header (struct GMT_CTRL *GMT, FILE *fp, unsigned int dir);
EXTERN_MSC void gmtlib_write_tableheader (struct GMT_CTRL *GMT, FILE *fp, char *txt);
EXTERN_MSC void gmtlib_write_textrecord (struct GMT_CTRL *GMT, FILE *fp, char *txt);
EXTERN_MSC struct GMT_DATASET * gmtlib_create_dataset (struct GMT_CTRL *GMT, uint64_t n_tables, uint64_t n_segments, uint64_t n_rows, uint64_t n_columns, unsigned int geometry, unsigned int mode, bool alloc_only);
EXTERN_MSC struct GMT_DATATABLE * gmtlib_read_table (struct GMT_CTRL *GMT, void *source, unsigned int source_type, bool greenwich, unsigned int *geometry, unsigned int *datatype, bool use_GMT_io);
EXTERN_MSC int gmtlib_write_dataset (struct GMT_CTRL *GMT, void *dest, unsigned int dest_type, struct GMT_DATASET *D, bool use_GMT_io, int table);
EXTERN_MSC void gmtlib_free_palette (struct GMT_CTRL *GMT, struct GMT_PALETTE **P);
EXTERN_MSC int gmtlib_append_ogr_item (struct GMT_CTRL *GMT, char *name, enum GMT_enum_type type, struct GMT_OGR *S);
EXTERN_MSC void gmtlib_write_ogr_header (FILE *fp, struct GMT_OGR *G);
EXTERN_MSC struct GMT_IMAGE * gmtlib_create_image (struct GMT_CTRL *GMT);
EXTERN_MSC void gmtlib_free_image (struct GMT_CTRL *GMT, struct GMT_IMAGE **I, bool free_image);
EXTERN_MSC struct GMT_MATRIX * gmtlib_create_matrix (struct GMT_CTRL *GMT, uint64_t n_layers, unsigned int direction, int flag);
EXTERN_MSC void gmtlib_free_matrix (struct GMT_CTRL *GMT, struct GMT_MATRIX **M, bool free_matrix);
EXTERN_MSC int gmtlib_determine_pole (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t n);
EXTERN_MSC void gmtlib_write_newheaders (struct GMT_CTRL *GMT, FILE *fp, uint64_t n_columns);
EXTERN_MSC char **gmtlib_get_dir_list (struct GMT_CTRL *GMT, char *path, char *ext);
EXTERN_MSC void gmtlib_free_dir_list (struct GMT_CTRL *GMT, char ***list);
EXTERN_MSC void gmtlib_assign_segment (struct GMT_CTRL *GMT, unsigned int direction, struct GMT_DATASEGMENT *S, uint64_t n_rows, uint64_t n_columns);
EXTERN_MSC double *gmtlib_assign_vector (struct GMT_CTRL *GMT, uint64_t n_rows, uint64_t col);
EXTERN_MSC void gmtlib_free_tmp_arrays (struct GMT_CTRL *GMT);
EXTERN_MSC unsigned int gmtlib_split_line_at_dateline (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S, struct GMT_DATASEGMENT ***Lout);
EXTERN_MSC int gmtlib_detrend (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, double increment, double *intercept, double *slope, int mode);
EXTERN_MSC double gmtlib_conv_distance (struct GMT_CTRL *GMT, double value, char in_unit, char out_unit);
EXTERN_MSC int gmtlib_image_BC_set (struct GMT_CTRL *GMT, struct GMT_IMAGE *I);
EXTERN_MSC int gmtlib_write_cpt (struct GMT_CTRL *GMT, void *dest, unsigned int dest_type, unsigned int cpt_flags, struct GMT_PALETTE *P);
EXTERN_MSC void gmtlib_copy_palette (struct GMT_CTRL *GMT, struct GMT_PALETTE *P_to, struct GMT_PALETTE *P_from);
EXTERN_MSC void gmtlib_decorate_init (struct GMT_CTRL *GMT, struct GMT_DECORATE *G, unsigned int mode);
EXTERN_MSC int gmtlib_decorate_specs (struct GMT_CTRL *GMT, char *txt, struct GMT_DECORATE *D);
EXTERN_MSC int gmtlib_decorate_info (struct GMT_CTRL *GMT, char flag, char *txt, struct GMT_DECORATE *D);
EXTERN_MSC unsigned int gmtlib_get_arc (struct GMT_CTRL *GMT, double x0, double y0, double r, double dir1, double dir2, double **x, double **y);
EXTERN_MSC struct GMT_PALETTE * gmtlib_read_cpt (struct GMT_CTRL *GMT, void *source, unsigned int source_type, unsigned int cpt_flags);
EXTERN_MSC char * gmtlib_trim_segheader (struct GMT_CTRL *GMT, char *line);
EXTERN_MSC int gmtlib_alloc_univector (struct GMT_CTRL *GMT, union GMT_UNIVECTOR *u, unsigned int type, uint64_t n_rows);
EXTERN_MSC unsigned int gmtlib_get_arc (struct GMT_CTRL *GMT, double x0, double y0, double r, double dir1, double dir2, double **x, double **y);
EXTERN_MSC unsigned int gmtlib_expand_headerpad (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, double *new_wesn, unsigned int *orig_pad, double *orig_wesn);
EXTERN_MSC void gmtlib_contract_headerpad (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, unsigned int *orig_pad, double *orig_wesn);
EXTERN_MSC void gmtlib_contract_pad (struct GMT_CTRL *GMT, void *object, int family, unsigned int *orig_pad, double *orig_wesn);
EXTERN_MSC uint64_t gmtlib_glob_list (struct GMT_CTRL *GMT, const char *pattern, char ***list);
EXTERN_MSC void gmtlib_change_dataset (struct GMT_CTRL *GMT, struct GMT_DATASET *D);
EXTERN_MSC void gmtlib_ellipsoid_name_convert (char *inname, char outname[]);

#ifdef HAVE_GDAL
EXTERN_MSC int gmtlib_read_image (struct GMT_CTRL *GMT, char *file, struct GMT_IMAGE *I, double *wesn,
			unsigned int *pad, unsigned int complex_mode);		/* Function to read true images via GDAL */
int gmtlib_read_image_info (struct GMT_CTRL *GMT, char *file, bool must_be_image, struct GMT_IMAGE *I);
#endif

/* LOCAL MACROS USED BY GMT_*.C ONLY - NOT PART OF GMT_DEV.H DISTRIBUTION */

/* Definition for an error trap */
#ifdef DEBUG
#define gmt_M_err_trap(func_call) if ((err = (func_call)) != GMT_NOERROR) {GMT_Report(GMT->parent,GMT_MSG_ERROR,"gmt_M_err_trap: %d\n", err);return(err);}
#else
#define gmt_M_err_trap(func_call) if ((err = (func_call)) != GMT_NOERROR) return(err)
#endif

/* Determine if current binary table has header */
#define gmt_M_binary_header(GMT,dir) (GMT->common.b.active[dir] && GMT->current.setting.io_header[dir] && GMT->current.setting.io_n_header_items)

/*! Checking of h,m,s */

#define gmt_M_hms_is_bad(h,m,s) ((h) < 0 || (h) > 23 || (m) < 0 || (m) > 59 || (s) < 0.0 || (s) >= 61.0)

/*! Force component to be in 0 <= s <= 255 range */
#define gmt_M_0_255_truncate(s) ((s < 0) ? 0 : ((s > 255) ? 255 : s))	/* Truncate to allowable 0-255 range */

#define gmt_M_axis_is_geo(C,axis) (C->current.io.col_type[GMT_IN][axis] & GMT_IS_GEO)
#define gmt_M_axis_is_time(C,axis) (C->current.io.col_type[GMT_IN][axis] & GMT_IS_ABSTIME)

#define gmt_M_is_perspective(C) (C->current.proj.projection == GMT_ORTHO || C->current.proj.projection == GMT_GENPER)
#define gmt_M_pole_is_point(C) ((C->current.proj.projection == GMT_OBLIQUE_MERC || C->current.proj.projection == GMT_OBLIQUE_MERC_POLE) || (C->current.proj.projection >= GMT_LAMBERT && C->current.proj.projection <= GMT_VANGRINTEN && C->current.proj.projection != GMT_POLAR))
#define gmt_M_is_grdmapproject(C) (!strncmp (C->init.module_name, "grdproject", 10U) || !strncmp (C->init.module_name, "mapproject", 10U))

#endif /* GMT_INTERNALS_H */
