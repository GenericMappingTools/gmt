/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
/* gmt_prototypes.h  --  All low-level GMT API function prototypes.

   Authors:	P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
   Date:	1-OCT-2009
   Version:	5 API

*/

#ifndef _GMT_PROTOTYPES_H
#define _GMT_PROTOTYPES_H

EXTERN_MSC void grd_dump (struct GMT_GRID_HEADER *header, float *grid, bool is_complex, char *txt);

/* Temporary redef of strdup to allow check of memory leaks due to usage of strdup */
#ifdef FISH_STRDUP_LEAKS
EXTERN_MSC char *GMT_strdup (struct GMT_CTRL *GMT, const char *s);
#endif

/* gmt_bcr.c: */
EXTERN_MSC double GMT_get_bcr_z (struct GMT_CTRL *GMT, struct GMT_GRID *G, double xx, double yy);		/* Compute z(x,y) from bcr structure and grid */
EXTERN_MSC int GMT_get_bcr_img (struct GMT_CTRL *GMT, struct GMT_IMAGE *G, double xx, double yy, unsigned char *z);		/* Compute z(x,y) from bcr structure and image */

/* gmt_customio.c: */

#ifdef HAVE_GDAL
/* Format # 22 */
EXTERN_MSC int GMT_gdalread (struct GMT_CTRL *GMT, char *gdal_filename, struct GDALREAD_CTRL *prhs, struct GD_CTRL *Ctrl);
EXTERN_MSC int GMT_gdalwrite (struct GMT_CTRL *GMT, char *filename, struct GDALWRITE_CTRL *prhs);
#endif

/* gmt_fft.c: */

EXTERN_MSC void GMT_fft_initialization (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_fft_cleanup (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_fft_set_k (struct GMT_CTRL *GMT, unsigned int mode, struct GMT_FFT_WAVENUMBER *K);
EXTERN_MSC int GMT_fft_set_wave (struct GMT_CTRL *GMT, unsigned int mode, struct GMT_FFT_WAVENUMBER *K);
EXTERN_MSC double GMT_fft_get_wave (uint64_t k, struct GMT_FFT_WAVENUMBER *K);
EXTERN_MSC double GMT_fft_any_wave (uint64_t k, unsigned int mode, struct GMT_FFT_WAVENUMBER *K);
EXTERN_MSC void GMT_suggest_fft_dim (struct GMT_CTRL *GMT, unsigned int nx, unsigned int ny, struct GMT_FFT_SUGGESTION *fft_sug, bool do_print);

/* gmt_grdio.c: */

EXTERN_MSC void GMT_grd_detrend (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, unsigned int mode, double *a);
EXTERN_MSC void GMT_grd_minmax (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, double xyz[2][3]);
EXTERN_MSC struct GMT_GRID * GMT_create_grid (struct GMT_CTRL *GMT);
EXTERN_MSC struct GMT_GRID * GMT_duplicate_grid (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int mode);
EXTERN_MSC struct GMT_GRID_HEADER * GMT_duplicate_gridheader (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h);
EXTERN_MSC void GMT_grd_init (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, struct GMT_OPTION *options, bool update);
EXTERN_MSC void GMT_decode_grd_h_info (struct GMT_CTRL *GMT, char *input, struct GMT_GRID_HEADER *h);
EXTERN_MSC void GMT_free_grid (struct GMT_CTRL *GMT, struct GMT_GRID **G, bool free_grid);
EXTERN_MSC void GMT_set_grdinc (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h);
EXTERN_MSC void GMT_set_grddim (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h);
EXTERN_MSC void GMT_grd_pad_on (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int *pad);
EXTERN_MSC void GMT_grd_pad_off (struct GMT_CTRL *GMT, struct GMT_GRID *G);
EXTERN_MSC void GMT_grd_pad_zero (struct GMT_CTRL *GMT, struct GMT_GRID *G);
EXTERN_MSC void GMT_grd_zminmax (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, float *z);
EXTERN_MSC int GMT_read_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header);
EXTERN_MSC int GMT_write_grd_info (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header);
EXTERN_MSC int GMT_read_grd (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header, float *grid, double *wesn, unsigned int *pad, int complex_mode);
EXTERN_MSC int GMT_write_grd (struct GMT_CTRL *GMT, char *file, struct GMT_GRID_HEADER *header, float *grid, double *wesn, unsigned int *pad, int complex_mode);
EXTERN_MSC int GMT_adjust_loose_wesn (struct GMT_CTRL *GMT, double wesn[], struct GMT_GRID_HEADER *header);
EXTERN_MSC int GMT_grd_setregion (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, double *wesn, unsigned int interpolant);
EXTERN_MSC int GMT_grd_RI_verify (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, unsigned int mode);
EXTERN_MSC int GMT_read_img (struct GMT_CTRL *GMT, char *imgfile, struct GMT_GRID *G, double *wesn, double scale, unsigned int mode, double lat, bool init);
EXTERN_MSC int GMT_conv_intext2dbl (struct GMT_CTRL *GMT, char *record, unsigned int ncols);
EXTERN_MSC bool GMT_grd_pad_status (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, unsigned int *pad);
EXTERN_MSC int GMT_set_outgrid (struct GMT_CTRL *GMT, char *file, struct GMT_GRID *G, struct GMT_GRID **Out);
EXTERN_MSC int GMT_change_grdreg (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, unsigned int registration);
EXTERN_MSC void GMT_grd_shift (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, double shift);
EXTERN_MSC void GMT_grd_set_ij_inc (struct GMT_CTRL *GMT, unsigned int nx, int *ij_inc);
EXTERN_MSC double * GMT_grd_coord (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h, int dir);
#ifdef HAVE_GDAL
EXTERN_MSC int GMT_read_image (struct GMT_CTRL *GMT, char *file, struct GMT_IMAGE *I, double *wesn, 
			unsigned int *pad, unsigned int complex_mode);		/* Function to read true images via GDAL */
int GMT_read_image_info (struct GMT_CTRL *GMT, char *file, struct GMT_IMAGE *I);
#endif

#ifdef _PSLIB_H
/* gmt_plot.c prototypes only included if pslib has been included */

EXTERN_MSC char * GMT_export2proj4 (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_textpath_init (struct GMT_CTRL *GMT, struct GMT_PEN *LP, double Brgb[], struct GMT_PEN *BP, double Frgb[]);
EXTERN_MSC void GMT_draw_map_rose (struct GMT_CTRL *GMT, struct GMT_MAP_ROSE *mr);
EXTERN_MSC int GMT_draw_map_scale (struct GMT_CTRL *GMT, struct GMT_MAP_SCALE *ms);
EXTERN_MSC void GMT_draw_map_insert (struct GMT_CTRL *GMT, struct GMT_MAP_INSERT *B);
EXTERN_MSC void GMT_geo_line (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t n);
EXTERN_MSC void GMT_geo_polygons (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S);
EXTERN_MSC void GMT_geo_ellipse (struct GMT_CTRL *GMT, double lon, double lat, double major, double minor, double azimuth);
EXTERN_MSC void GMT_geo_rectangle (struct GMT_CTRL *GMT, double lon, double lat, double width, double height, double azimuth);
EXTERN_MSC unsigned int GMT_geo_vector (struct GMT_CTRL *GMT, double lon0, double lat0, double azimuth, double length, struct GMT_PEN *pen, struct GMT_SYMBOL *S);
EXTERN_MSC void GMT_draw_front (struct GMT_CTRL *GMT, double x[], double y[], uint64_t n, struct GMT_FRONTLINE *f);
EXTERN_MSC void GMT_map_basemap (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_map_clip_off (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_map_clip_on (struct GMT_CTRL *GMT, double rgb[], unsigned int flag);
EXTERN_MSC void GMT_plot_line (struct GMT_CTRL *GMT, double *x, double *y, unsigned int *pen, uint64_t n);
EXTERN_MSC void GMT_setpen (struct GMT_CTRL *GMT, struct GMT_PEN *pen);
EXTERN_MSC void GMT_setfill (struct GMT_CTRL *GMT, struct GMT_FILL *fill, int outline);
EXTERN_MSC void GMT_vertical_axis (struct GMT_CTRL *GMT, unsigned int mode);
EXTERN_MSC void GMT_xy_axis (struct GMT_CTRL *GMT, double x0, double y0, double length, double val0, double val1, struct GMT_PLOT_AXIS *A, bool below, bool annotate);
EXTERN_MSC int GMT_draw_custom_symbol (struct GMT_CTRL *GMT, double x0, double y0, double size[], struct GMT_CUSTOM_SYMBOL *symbol, struct GMT_PEN *pen, struct GMT_FILL *fill, unsigned int outline);
EXTERN_MSC void GMT_contlabel_plot (struct GMT_CTRL *GMT, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_plane_perspective (struct GMT_CTRL *GMT, int plane, double level);
EXTERN_MSC void GMT_plotcanvas (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_write_label_record (struct GMT_CTRL *GMT, FILE *fp, double x, double y, double angle, char *label, bool save_angle);
EXTERN_MSC int GMT_contlabel_save_begin (struct GMT_CTRL *GMT, struct GMT_CONTOUR *G);
EXTERN_MSC int GMT_contlabel_save_end (struct GMT_CTRL *GMT, struct GMT_CONTOUR *G);
EXTERN_MSC unsigned int GMT_setfont (struct GMT_CTRL *GMT, struct GMT_FONT *F);
EXTERN_MSC void GMT_plotend (struct GMT_CTRL *GMT);
EXTERN_MSC struct PSL_CTRL * GMT_plotinit (struct GMT_CTRL *GMT, struct GMT_OPTION *options);
#endif

/* gmt_io.c: */

EXTERN_MSC void GMT_eliminate_lon_jumps (struct GMT_CTRL *GMT, double *lon, uint64_t n_rows);
EXTERN_MSC void GMT_set_geographic (struct GMT_CTRL *GMT, unsigned int dir);
EXTERN_MSC void GMT_set_cartesian (struct GMT_CTRL *GMT, unsigned int dir);
EXTERN_MSC void GMT_set_xycolnames (struct GMT_CTRL *GMT, char *string);
EXTERN_MSC p_to_io_func GMT_get_io_ptr (struct GMT_CTRL *GMT, int direction, enum GMT_swap_direction swap, char type);
EXTERN_MSC bool GMT_is_ascii_record (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_set_segmentheader (struct GMT_CTRL *GMT, int direction, bool true_false);
EXTERN_MSC void GMT_set_tableheader (struct GMT_CTRL *GMT, int direction, bool true_false);
EXTERN_MSC void GMT_io_binary_header (struct GMT_CTRL *GMT, FILE *fp, unsigned int dir);
EXTERN_MSC void * GMT_z_input (struct GMT_CTRL *GMT, FILE *fp, uint64_t *n, int *status);
EXTERN_MSC int GMT_z_output (struct GMT_CTRL *GMT, FILE *fp, uint64_t n, double *data);
EXTERN_MSC int GMT_get_io_type (struct GMT_CTRL *GMT, char type);
EXTERN_MSC struct GMT_QUAD * GMT_quad_init (struct GMT_CTRL *GMT, uint64_t n_items);
EXTERN_MSC void GMT_quad_reset (struct GMT_CTRL *GMT, struct GMT_QUAD *Q, uint64_t n_items);
EXTERN_MSC void GMT_quad_add (struct GMT_CTRL *GMT, struct GMT_QUAD *Q, double x);
EXTERN_MSC unsigned int GMT_quad_finalize (struct GMT_CTRL *GMT, struct GMT_QUAD *Q);
EXTERN_MSC char * GMT_fgets (struct GMT_CTRL *GMT, char *str, int size, FILE *stream);
EXTERN_MSC int GMT_fclose (struct GMT_CTRL *GMT, FILE *stream);
EXTERN_MSC int GMT_access (struct GMT_CTRL *GMT, const char *filename, int mode);		/* access wrapper */
EXTERN_MSC FILE * GMT_fopen (struct GMT_CTRL *GMT, const char *filename, const char *mode);
EXTERN_MSC char * GMT_getdatapath (struct GMT_CTRL *GMT, const char *stem, char *path, int mode);	/* Look for data file */
EXTERN_MSC char * GMT_getsharepath (struct GMT_CTRL *GMT, const char *subdir, const char *stem, const char *suffix, char *path, int mode);	/* Look for shared file */
EXTERN_MSC void GMT_write_tableheader (struct GMT_CTRL *GMT, FILE *fp, char *txt);
EXTERN_MSC void GMT_write_segmentheader (struct GMT_CTRL *GMT, FILE *fp, uint64_t n_cols);		/* Write segment header back out */
EXTERN_MSC void GMT_write_textrecord (struct GMT_CTRL *GMT, FILE *fp, char *txt);
EXTERN_MSC void GMT_ascii_format_col (struct GMT_CTRL *GMT, char *text, double x, unsigned int direction, uint64_t col);
EXTERN_MSC void GMT_lon_range_adjust (unsigned int range, double *lon);		/* Adjust the longitude given the desired range */
EXTERN_MSC void GMT_add_to_record (struct GMT_CTRL *GMT, char *record, double val, uint64_t col, unsigned int sep);
EXTERN_MSC int GMT_scanf (struct GMT_CTRL *GMT, char *p, unsigned int expectation, double *val);	/* Convert strings to double, handling special formats [Data records only ] */
EXTERN_MSC int GMT_scanf_arg (struct GMT_CTRL *GMT, char *p, unsigned int expectation, double *val);	/* Convert strings to double, handling special formats [ command line only ] */
EXTERN_MSC int GMT_ascii_output_col (struct GMT_CTRL *GMT, FILE *fp, double x, uint64_t col);
EXTERN_MSC bool GMT_not_numeric (struct GMT_CTRL *GMT, char *text);				/* Rules out _some_ text as possible numerics */
EXTERN_MSC bool GMT_parse_segment_item (struct GMT_CTRL *GMT, char *in_string, char *pattern, char *out_string);
EXTERN_MSC int GMT_set_cols (struct GMT_CTRL *GMT, unsigned int direction, uint64_t expected);
EXTERN_MSC uint64_t GMT_get_cols (struct GMT_CTRL *GMT, unsigned int direction);
EXTERN_MSC struct GMT_DATASET * GMT_create_dataset (struct GMT_CTRL *GMT, uint64_t n_tables, uint64_t n_segments, uint64_t n_rows, uint64_t n_columns, unsigned int geometry, bool alloc_only);
EXTERN_MSC struct GMT_DATATABLE * GMT_create_table (struct GMT_CTRL *GMT, uint64_t n_segments, uint64_t n_rows, uint64_t n_columns, bool alloc_only);
EXTERN_MSC void GMT_free_textset (struct GMT_CTRL *GMT, struct GMT_TEXTSET **data);
EXTERN_MSC struct GMT_TEXTSET * GMT_duplicate_textset (struct GMT_CTRL *GMT, struct GMT_TEXTSET *Din, unsigned int mode);
EXTERN_MSC void GMT_adjust_dataset (struct GMT_CTRL *GMT, struct GMT_DATASET *D, uint64_t n_columns);
EXTERN_MSC struct GMT_DATASET * GMT_alloc_dataset (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, uint64_t n_rows, uint64_t n_columns, unsigned int mode);
EXTERN_MSC struct GMT_DATASET * GMT_duplicate_dataset (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, unsigned int mode, unsigned int *geometry);
EXTERN_MSC struct GMT_DATATABLE * GMT_read_table (struct GMT_CTRL *GMT, void *source, unsigned int source_type, bool greenwich, unsigned int *geometry, bool use_GMT_io);
EXTERN_MSC int GMT_write_dataset (struct GMT_CTRL *GMT, void *dest, unsigned int dest_type, struct GMT_DATASET *D, bool use_GMT_io, int table);
EXTERN_MSC int GMT_write_table (struct GMT_CTRL *GMT, void *dest, unsigned int dest_type, struct GMT_DATATABLE *T, bool use_GMT_io, unsigned int io_mode);
EXTERN_MSC int GMT_alloc_segment (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S, uint64_t n_rows, uint64_t n_columns, bool first);
EXTERN_MSC void GMT_free_segment (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT **segment, enum GMT_enum_alloc alloc_mode);
EXTERN_MSC void GMT_free_table (struct GMT_CTRL *GMT, struct GMT_DATATABLE *table, enum GMT_enum_alloc alloc_mode);
EXTERN_MSC void GMT_free_dataset (struct GMT_CTRL *GMT, struct GMT_DATASET **data);
EXTERN_MSC void GMT_free_palette (struct GMT_CTRL *GMT, struct GMT_PALETTE **P);
#ifdef HAVE_GDAL
EXTERN_MSC struct GMT_IMAGE * GMT_create_image (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_free_image (struct GMT_CTRL *GMT, struct GMT_IMAGE **I, bool free_image);
#endif
EXTERN_MSC struct GMT_MATRIX * GMT_create_matrix (struct GMT_CTRL *GMT, uint64_t n_layers);
EXTERN_MSC void GMT_free_matrix (struct GMT_CTRL *GMT, struct GMT_MATRIX **M, bool free_matrix);
EXTERN_MSC struct GMT_VECTOR * GMT_create_vector (struct GMT_CTRL *GMT, uint64_t n_columns);
EXTERN_MSC void GMT_free_vector (struct GMT_CTRL *GMT, struct GMT_VECTOR **V, bool free_vector);
EXTERN_MSC int GMT_load_aspatial_string (struct GMT_CTRL *GMT, struct GMT_OGR *G, uint64_t col, char out[GMT_BUFSIZ]);
EXTERN_MSC double GMT_get_aspatial_value (struct GMT_CTRL *GMT, uint64_t col, struct GMT_DATASEGMENT *S);
EXTERN_MSC void GMT_set_seg_minmax (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S);
EXTERN_MSC void GMT_set_seg_polar (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S);
EXTERN_MSC void GMT_skip_xy_duplicates (struct GMT_CTRL *GMT, bool mode);
EXTERN_MSC void GMT_duplicate_ogr_seg (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S_to, struct GMT_DATASEGMENT *S_from);
EXTERN_MSC struct GMT_DATASEGMENT * GMT_duplicate_segment (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *Sin);
EXTERN_MSC void GMT_write_ogr_header (FILE *fp, struct GMT_OGR *G);
EXTERN_MSC int GMT_append_ogr_item (struct GMT_CTRL *GMT, char *name, unsigned int type, struct GMT_OGR *S);
EXTERN_MSC char * GMT_trim_segheader (struct GMT_CTRL *GMT, char *line);
EXTERN_MSC int GMT_alloc_univector (struct GMT_CTRL *GMT, union GMT_UNIVECTOR *u, unsigned int type, uint64_t n_rows);
EXTERN_MSC bool gmt_byteswap_file (struct GMT_CTRL *GMT,
		FILE *outfp, FILE *infp, const SwapWidth nbytes,
		const uint64_t offset, const uint64_t length);
EXTERN_MSC int GMT_parse_segment_header (struct GMT_CTRL *GMT, char *header, struct GMT_PALETTE *P, bool *use_fill, struct GMT_FILL *fill, struct GMT_FILL def_fill, bool *use_pen, struct GMT_PEN *pen, struct GMT_PEN def_pen, unsigned int def_outline, struct GMT_OGR_SEG *G);
EXTERN_MSC int GMT_parse_z_io (struct GMT_CTRL *GMT, char *txt, struct GMT_PARSE_Z_IO *z);
EXTERN_MSC int GMT_init_z_io (struct GMT_CTRL *GMT, char format[], bool repeat[], enum GMT_swap_direction swab, off_t skip, char type, struct GMT_Z_IO *r);
EXTERN_MSC int GMT_set_z_io (struct GMT_CTRL *GMT, struct GMT_Z_IO *r, struct GMT_GRID *G);
EXTERN_MSC void GMT_check_z_io (struct GMT_CTRL *GMT, struct GMT_Z_IO *r, struct GMT_GRID *G);
EXTERN_MSC void GMT_init_io_columns (struct GMT_CTRL *GMT, unsigned int dir);
EXTERN_MSC void GMT_write_newheaders (struct GMT_CTRL *GMT, FILE *fp, uint64_t n_columns);
EXTERN_MSC char **GMT_get_dir_list (struct GMT_CTRL *GMT, char *path, char *ext);
EXTERN_MSC void GMT_free_dir_list (struct GMT_CTRL *GMT, char ***list);
EXTERN_MSC void GMT_assign_segment (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S, uint64_t n_rows, uint64_t n_columns);
EXTERN_MSC double *GMT_assign_vector (struct GMT_CTRL *GMT, uint64_t n_rows, uint64_t col);

/* gmt_memory.c: */

EXTERN_MSC void GMT_prep_tmp_arrays (struct GMT_CTRL *GMT, size_t row, size_t n_cols);
EXTERN_MSC void GMT_free_tmp_arrays (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_set_meminc (struct GMT_CTRL *GMT, size_t increment);
EXTERN_MSC void GMT_reset_meminc (struct GMT_CTRL *GMT);
EXTERN_MSC void * GMT_memory_func (struct GMT_CTRL *GMT, void *prev_addr, size_t nelem, size_t size, bool align, const char *where);
EXTERN_MSC void GMT_free_func (struct GMT_CTRL *GMT, void *addr, bool align, const char *where);

/* gmt_support.c: */

EXTERN_MSC void GMT_flip_angle_d (struct GMT_CTRL *GMT, double *angle);
EXTERN_MSC void GMT_flip_angle_f (struct GMT_CTRL *GMT, float *angle);
EXTERN_MSC struct GMT_DATATABLE *GMT_make_profile (struct GMT_CTRL *GMT, char option, char *args, bool resample, bool project, bool get_distances, double step, enum GMT_enum_track mode, double xyz[2][3]);
EXTERN_MSC unsigned int GMT_split_line_at_dateline (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S, struct GMT_DATASEGMENT ***Lout);
EXTERN_MSC unsigned int GMT_split_poly_at_dateline (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S, struct GMT_DATASEGMENT ***Lout);
EXTERN_MSC bool GMT_x_is_outside (struct GMT_CTRL *GMT, double *x, double left, double right);
EXTERN_MSC void GMT_set_xy_domain (struct GMT_CTRL *GMT, double wesn_extended[], struct GMT_GRID_HEADER *h);
EXTERN_MSC int GMT_BC_init (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h);
EXTERN_MSC int GMT_grd_BC_set (struct GMT_CTRL *GMT, struct GMT_GRID *G, unsigned int direction);
EXTERN_MSC void GMT_inplace_transpose (float *A, unsigned int n_rows, unsigned int n_cols);
EXTERN_MSC struct GMT_PALETTE * GMT_truncate_cpt (struct GMT_CTRL *GMT, struct GMT_PALETTE *P, double z_low, double z_high);
EXTERN_MSC void GMT_free_int_selection (struct GMT_CTRL *GMT, struct GMT_INT_SELECTION **S);
EXTERN_MSC struct GMT_INT_SELECTION * GMT_set_int_selection (struct GMT_CTRL *GMT, char *item);
EXTERN_MSC bool GMT_get_int_selection (struct GMT_CTRL *GMT, struct GMT_INT_SELECTION *S, uint64_t this);
EXTERN_MSC void GMT_free_text_selection (struct GMT_CTRL *GMT, struct GMT_TEXT_SELECTION **S);
EXTERN_MSC bool GMT_get_text_selection (struct GMT_CTRL *GMT, struct GMT_TEXT_SELECTION *S, struct GMT_DATASEGMENT *T, bool last_match);
EXTERN_MSC struct GMT_TEXT_SELECTION * GMT_set_text_selection (struct GMT_CTRL *GMT, char *arg);

#ifdef HAVE_GDAL
EXTERN_MSC int GMT_image_BC_set (struct GMT_CTRL *GMT, struct GMT_IMAGE *I);
#endif
EXTERN_MSC bool GMT_y_out_of_bounds (struct GMT_CTRL *GMT, int *j, struct GMT_GRID_HEADER *h, bool *wrap_180);
EXTERN_MSC bool GMT_x_out_of_bounds (struct GMT_CTRL *GMT, int *i, struct GMT_GRID_HEADER *h, bool wrap_180);
EXTERN_MSC bool GMT_row_col_out_of_bounds (struct GMT_CTRL *GMT, double *in, struct GMT_GRID_HEADER *h, unsigned int *row, unsigned int *col);
EXTERN_MSC int GMT_list_cpt (struct GMT_CTRL *GMT, char option);
EXTERN_MSC struct GMT_PALETTE * GMT_sample_cpt (struct GMT_CTRL *GMT, struct GMT_PALETTE *Pin, double z[], int nz, bool continuous, bool reverse, bool log_mode, bool no_inter);
EXTERN_MSC int GMT_write_cpt (struct GMT_CTRL *GMT, void *dest, unsigned int dest_type, unsigned int cpt_flags, struct GMT_PALETTE *P);
EXTERN_MSC void GMT_cpt_transparency (struct GMT_CTRL *GMT, struct GMT_PALETTE *P, double transparency, unsigned int mode);
EXTERN_MSC void GMT_copy_palette (struct GMT_CTRL *GMT, struct GMT_PALETTE *P_to, struct GMT_PALETTE *P_from);
EXTERN_MSC int GMT_contlabel_info (struct GMT_CTRL *GMT, char flag, char *txt, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_contlabel_init (struct GMT_CTRL *GMT, struct GMT_CONTOUR *G, unsigned int mode);
EXTERN_MSC int GMT_contlabel_specs (struct GMT_CTRL *GMT, char *txt, struct GMT_CONTOUR *G);
EXTERN_MSC int GMT_contlabel_prep (struct GMT_CTRL *GMT, struct GMT_CONTOUR *G, double xyz[2][3]);
EXTERN_MSC void GMT_contlabel_free (struct GMT_CTRL *GMT, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_hold_contour (struct GMT_CTRL *GMT, double **xx, double **yy, uint64_t nn, double zval, char *label, char ctype, double cangle, bool closed, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_x_free (struct GMT_CTRL *GMT, struct GMT_XOVER *X);
EXTERN_MSC int GMT_init_track (struct GMT_CTRL *GMT, double y[], uint64_t n, struct GMT_XSEGMENT **S);
EXTERN_MSC uint64_t GMT_crossover (struct GMT_CTRL *GMT, double xa[], double ya[], uint64_t sa[], struct GMT_XSEGMENT A[], uint64_t na, double xb[], double yb[], uint64_t sb[], struct GMT_XSEGMENT B[], uint64_t nb, bool internal, bool geo, struct GMT_XOVER *X);
EXTERN_MSC void * GMT_malloc_func (struct GMT_CTRL *GMT, void *ptr, size_t n, size_t *n_alloc, size_t element_size, const char *where);
EXTERN_MSC char * GMT_make_filename (struct GMT_CTRL *GMT, char *template, unsigned int fmt[], double z, bool closed, unsigned int count[]);
EXTERN_MSC void GMT_str_setcase (struct GMT_CTRL *GMT, char *value, int mode);
EXTERN_MSC char * GMT_putusername (struct GMT_CTRL *GMT);
EXTERN_MSC unsigned int * GMT_prep_nodesearch (struct GMT_CTRL *GMT, struct GMT_GRID *G, double radius, unsigned int mode, unsigned int *d_row, unsigned int *actual_max_d_col);
EXTERN_MSC int GMT_detrend (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, double increment, double *intercept, double *slope, int mode);
EXTERN_MSC struct GMT_PALETTE * GMT_Get_CPT (struct GMT_CTRL *GMT, char *file, enum GMT_enum_cpt mode, double zmin, double zmax);
EXTERN_MSC unsigned int GMT_gcd_euclid (unsigned int a, unsigned int b);
EXTERN_MSC unsigned int GMT_optimal_dim_for_surface (struct GMT_CTRL *GMT, unsigned int factors[], unsigned int nx, unsigned int ny, struct GMT_SURFACE_SUGGESTION **S);
EXTERN_MSC int GMT_best_dim_choice (struct GMT_CTRL *GMT, unsigned int mode, unsigned int in_dim[], unsigned int out_dim[]);

/* gmt_calclock.c: */

EXTERN_MSC double GMT_rdc2dt (struct GMT_CTRL *GMT, int64_t rd, double secs);
EXTERN_MSC void GMT_dt2rdc (struct GMT_CTRL *GMT, double t, int64_t *rd, double *s);
EXTERN_MSC int64_t GMT_rd_from_gymd (struct GMT_CTRL *GMT, int gy, int gm, int gd);
EXTERN_MSC void GMT_format_calendar (struct GMT_CTRL *GMT, char *date, char *clock, struct GMT_DATE_IO *D, struct GMT_CLOCK_IO *W, bool upper, unsigned int kind, double dt);
EXTERN_MSC void GMT_gcal_from_rd (struct GMT_CTRL *GMT, int64_t rd, struct GMT_gcal *gcal);

/* gmt_map.c: */

EXTERN_MSC bool GMT_cart_outside (struct GMT_CTRL *GMT, double x, double y);
EXTERN_MSC void GMT_auto_frame_interval (struct GMT_CTRL *GMT, unsigned int axis, unsigned int item);
EXTERN_MSC double GMT_az_backaz (struct GMT_CTRL *GMT, double lonE, double latE, double lonS, double latS, bool baz);
EXTERN_MSC double GMT_distance (struct GMT_CTRL *GMT, double lonS, double latS, double lonE, double latE);
EXTERN_MSC double GMT_azim_to_angle (struct GMT_CTRL *GMT, double lon, double lat, double c, double azim);
EXTERN_MSC uint64_t GMT_clip_to_map (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t np, double **x, double **y);
EXTERN_MSC uint64_t GMT_compact_line (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, int pen_flag, int *pen);
EXTERN_MSC uint64_t GMT_geo_to_xy_line (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t n);
EXTERN_MSC uint64_t GMT_graticule_path (struct GMT_CTRL *GMT, double **x, double **y, int dir, double w, double e, double s, double n);
EXTERN_MSC int GMT_grd_project (struct GMT_CTRL *GMT, struct GMT_GRID *I, struct GMT_GRID *O, bool inverse);
EXTERN_MSC int GMT_img_project (struct GMT_CTRL *GMT, struct GMT_IMAGE *I, struct GMT_IMAGE *O, bool inverse);
EXTERN_MSC uint64_t GMT_map_clip_path (struct GMT_CTRL *GMT, double **x, double **y, bool *donut);
EXTERN_MSC bool GMT_map_outside (struct GMT_CTRL *GMT, double lon, double lat);
EXTERN_MSC bool GMT_geo_to_xy (struct GMT_CTRL *GMT, double lon, double lat, double *x, double *y);
EXTERN_MSC void GMT_geoz_to_xy (struct GMT_CTRL *GMT, double x, double y, double z, double *x_out, double *y_out);
EXTERN_MSC int GMT_project_init (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *header, double *inc, unsigned int nx, unsigned int ny, unsigned int dpi, unsigned int offset);
EXTERN_MSC int GMT_map_setup (struct GMT_CTRL *GMT, double wesn[]);
EXTERN_MSC double GMT_x_to_xx (struct GMT_CTRL *GMT, double x);
EXTERN_MSC double GMT_y_to_yy (struct GMT_CTRL *GMT, double y);
EXTERN_MSC double GMT_z_to_zz (struct GMT_CTRL *GMT, double z);
EXTERN_MSC void GMT_xy_to_geo (struct GMT_CTRL *GMT, double *lon, double *lat, double x, double y);
EXTERN_MSC void GMT_xyz_to_xy (struct GMT_CTRL *GMT, double x, double y, double z, double *x_out, double *y_out);
EXTERN_MSC void GMT_xyz_to_xy_n (struct GMT_CTRL *GMT, double *x, double *y, double z, uint64_t n);
EXTERN_MSC double * GMT_dist_array (struct GMT_CTRL *GMT, double x[], double y[], uint64_t n, bool cumulative);
EXTERN_MSC double * GMT_dist_array_2 (struct GMT_CTRL *GMT, double x[], double y[], uint64_t n, double scale, int dist_flag);
EXTERN_MSC uint64_t GMT_map_truncate (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, uint64_t start, int side);
EXTERN_MSC unsigned int GMT_init_distaz (struct GMT_CTRL *GMT, char c, unsigned int mode, unsigned int type);
EXTERN_MSC bool GMT_near_lines (struct GMT_CTRL *GMT, double lon, double lat, struct GMT_DATATABLE *T, unsigned int return_mindist, double *dist_min, double *x_near, double *y_near);
EXTERN_MSC bool GMT_near_a_line (struct GMT_CTRL *GMT, double lon, double lat, uint64_t seg, struct GMT_DATASEGMENT *S, unsigned int return_mindist, double *dist_min, double *x_near, double *y_near);
EXTERN_MSC bool GMT_near_a_point (struct GMT_CTRL *GMT, double x, double y, struct GMT_DATATABLE *T, double dist);
EXTERN_MSC double GMT_great_circle_dist_meter (struct GMT_CTRL *GMT, double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_lat_swap_quick (struct GMT_CTRL *GMT, double lat, double c[]);
EXTERN_MSC double GMT_lat_swap (struct GMT_CTRL *GMT, double lat, unsigned int itype);
EXTERN_MSC double GMT_mindist_to_point (struct GMT_CTRL *GMT, double lon, double lat, struct GMT_DATATABLE *T, uint64_t *id);
EXTERN_MSC bool GMT_UTMzone_to_wesn (struct GMT_CTRL *GMT, unsigned int zone_x, char zone_y, int hemi, double wesn[]);
EXTERN_MSC void GMT_ECEF_forward (struct GMT_CTRL *GMT, double in[], double out[]);
EXTERN_MSC void GMT_ECEF_inverse (struct GMT_CTRL *GMT, double in[], double out[]);
EXTERN_MSC void GMT_ECEF_init (struct GMT_CTRL *GMT, struct GMT_DATUM *D);
EXTERN_MSC void GMT_datum_init (struct GMT_CTRL *GMT, struct GMT_DATUM *from, struct GMT_DATUM *to, bool heights);
EXTERN_MSC int GMT_set_datum (struct GMT_CTRL *GMT, char *text, struct GMT_DATUM *D);
EXTERN_MSC void GMT_conv_datum (struct GMT_CTRL *GMT, double in[], double out[]);
EXTERN_MSC uint64_t GMT_wesn_clip (struct GMT_CTRL *GMT, double *lon, double *lat, uint64_t n_orig, double **x, double **y, uint64_t *total_nx);
EXTERN_MSC void GMT_set_spherical (struct GMT_CTRL *GMT, bool notify);

/* gmt_shore.c: */

EXTERN_MSC int GMT_set_levels (struct GMT_CTRL *GMT, char *info, struct GMT_SHORE_SELECT *I);
EXTERN_MSC int GMT_get_shore_bin (struct GMT_CTRL *GMT, unsigned int b, struct GMT_SHORE *c);
EXTERN_MSC int GMT_get_br_bin (struct GMT_CTRL *GMT, unsigned int b, struct GMT_BR *c, unsigned int *level, unsigned int n_levels);
EXTERN_MSC void GMT_free_shore_polygons (struct GMT_CTRL *GMT, struct GMT_GSHHS_POL *p, unsigned int n);
EXTERN_MSC void GMT_free_shore (struct GMT_CTRL *GMT, struct GMT_SHORE *c);
EXTERN_MSC void GMT_free_br (struct GMT_CTRL *GMT, struct GMT_BR *c);
EXTERN_MSC void GMT_shore_cleanup (struct GMT_CTRL *GMT, struct GMT_SHORE *c);
EXTERN_MSC void GMT_br_cleanup (struct GMT_CTRL *GMT, struct GMT_BR *c);
EXTERN_MSC int GMT_init_shore (struct GMT_CTRL *GMT, char res, struct GMT_SHORE *c, double wesn[], struct GMT_SHORE_SELECT *I);
EXTERN_MSC int GMT_init_br (struct GMT_CTRL *GMT, char which, char res, struct GMT_BR *c, double wesn[]);
EXTERN_MSC int GMT_assemble_shore (struct GMT_CTRL *GMT, struct GMT_SHORE *c, int dir, bool assemble, double west, double east, struct GMT_GSHHS_POL **pol);
EXTERN_MSC int GMT_assemble_br (struct GMT_CTRL *GMT, struct GMT_BR *c, bool shift, double edge, struct GMT_GSHHS_POL **pol);
EXTERN_MSC int GMT_prep_shore_polygons (struct GMT_CTRL *GMT, struct GMT_GSHHS_POL **p, unsigned int np, bool sample, double step, int anti_bin);
EXTERN_MSC int GMT_set_resolution (struct GMT_CTRL *GMT, char *res, char opt);
EXTERN_MSC char GMT_shore_adjust_res (struct GMT_CTRL *GMT, char res);

/* gmt_vector.c: */

EXTERN_MSC int GMT_chol_dcmp (struct GMT_CTRL *GMT, double *a, double *d, double *cond, int nr, int n);
EXTERN_MSC void GMT_chol_recover (struct GMT_CTRL *GMT, double *a, double *d, int nr, int n, int nerr, bool donly);
EXTERN_MSC void GMT_chol_solv (struct GMT_CTRL *GMT, double *a, double *x, double *y, int nr, int n);
EXTERN_MSC void GMT_set_tbl_minmax (struct GMT_CTRL *GMT, struct GMT_DATATABLE *T);
EXTERN_MSC void GMT_matrix_vect_mult (struct GMT_CTRL *GMT, unsigned int dim, double a[3][3], double b[3], double c[3]);
EXTERN_MSC void GMT_make_rot_matrix (struct GMT_CTRL *GMT, double lonp, double latp, double w, double R[3][3]);
EXTERN_MSC void GMT_make_rot_matrix2 (struct GMT_CTRL *GMT, double E[3], double w, double R[3][3]);

/* gmt_support.c: */

EXTERN_MSC void GMT_sort_array (struct GMT_CTRL *GMT, void *base, uint64_t n, unsigned int type);
EXTERN_MSC bool GMT_polygon_is_open (struct GMT_CTRL *GMT, double x[], double y[], uint64_t n);
EXTERN_MSC double GMT_polygon_area (struct GMT_CTRL *GMT, double x[], double y[], uint64_t n);
EXTERN_MSC int GMT_polygon_centroid (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, double *Cx, double *Cy);
EXTERN_MSC int GMT_get_distance (struct GMT_CTRL *GMT, char *line, double *dist, char *unit);
EXTERN_MSC uint64_t GMT_contours (struct GMT_CTRL *GMT, struct GMT_GRID *Grid, unsigned int smooth_factor, unsigned int int_scheme, int orient, unsigned int *edge, bool *first, double **x, double **y);
EXTERN_MSC int GMT_get_format (struct GMT_CTRL *GMT, double interval, char *unit, char *prefix, char *format);
EXTERN_MSC int GMT_get_index (struct GMT_CTRL *GMT, struct GMT_PALETTE *P, double value);
EXTERN_MSC int GMT_get_rgb_from_z (struct GMT_CTRL *GMT, struct GMT_PALETTE *P, double value, double *rgb);
EXTERN_MSC bool GMT_getfill (struct GMT_CTRL *GMT, char *line, struct GMT_FILL *fill);
EXTERN_MSC bool GMT_getinc (struct GMT_CTRL *GMT, char *line, double inc[]);
EXTERN_MSC int GMT_getincn (struct GMT_CTRL *GMT, char *line, double inc[], unsigned int n);
EXTERN_MSC int GMT_getfont (struct GMT_CTRL *GMT, char *line, struct GMT_FONT *F);
EXTERN_MSC bool GMT_getpen (struct GMT_CTRL *GMT, char *line, struct GMT_PEN *pen);
EXTERN_MSC bool GMT_getrgb (struct GMT_CTRL *GMT, char *line, double *rgb);
EXTERN_MSC int GMT_getrose (struct GMT_CTRL *GMT, char option, char *text, struct GMT_MAP_ROSE *mr);
EXTERN_MSC int GMT_getscale (struct GMT_CTRL *GMT, char option, char *text, struct GMT_MAP_SCALE *ms);
EXTERN_MSC int GMT_getinsert (struct GMT_CTRL *GMT, char option, char *text, struct GMT_MAP_INSERT *B);
EXTERN_MSC char * GMT_putfont (struct GMT_CTRL *GMT, struct GMT_FONT F);
EXTERN_MSC char * GMT_putpen (struct GMT_CTRL *GMT, struct GMT_PEN pen);
EXTERN_MSC unsigned int GMT_inonout (struct GMT_CTRL *GMT, double x, double y, const struct GMT_DATASEGMENT *S);
EXTERN_MSC unsigned int GMT_inonout_sphpol (struct GMT_CTRL *GMT, double plon, double plat, const struct GMT_DATASEGMENT *P);
EXTERN_MSC int GMT_intpol (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, uint64_t m, double *u, double *v, int mode);
EXTERN_MSC int GMT_just_decode (struct GMT_CTRL *GMT, char *key, unsigned int def);
EXTERN_MSC unsigned int GMT_minmaxinc_verify (struct GMT_CTRL *GMT, double min, double max, double inc, double slop);
EXTERN_MSC unsigned int GMT_get_arc (struct GMT_CTRL *GMT, double x0, double y0, double r, double dir1, double dir2, double **x, double **y);
EXTERN_MSC unsigned int GMT_non_zero_winding (struct GMT_CTRL *GMT, double xp, double yp, double *x, double *y, uint64_t n_path);
EXTERN_MSC unsigned int GMT_getmodopt (struct GMT_CTRL *GMT, const char *string, const char *sep, unsigned int *pos, char *token);
EXTERN_MSC unsigned int GMT_verify_expectations (struct GMT_CTRL *GMT, unsigned int wanted, unsigned int got, char *item);
EXTERN_MSC void GMT_RI_prepare (struct GMT_CTRL *GMT, struct GMT_GRID_HEADER *h);
EXTERN_MSC struct GMT_DATASEGMENT * GMT_prepare_contour (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, double z);
EXTERN_MSC void GMT_get_plot_array (struct GMT_CTRL *GMT);
EXTERN_MSC void GMT_illuminate (struct GMT_CTRL *GMT, double intensity, double *rgb);
EXTERN_MSC void GMT_init_fill (struct GMT_CTRL *GMT, struct GMT_FILL *fill, double r, double g, double b);
EXTERN_MSC void GMT_init_pen (struct GMT_CTRL *GMT, struct GMT_PEN *pen, double width);
EXTERN_MSC int GMT_colorname2index (struct GMT_CTRL *GMT, char *name);
EXTERN_MSC void GMT_list_custom_symbols (struct GMT_CTRL *GMT);
EXTERN_MSC struct GMT_PALETTE * GMT_read_cpt (struct GMT_CTRL *GMT, void *source, unsigned int source_type, unsigned int cpt_flags);
EXTERN_MSC void GMT_smart_justify (struct GMT_CTRL *GMT, int just, double angle, double dx, double dy, double *x_shift, double *y_shift, unsigned int mode);
EXTERN_MSC struct GMT_DATASET * GMT_resample_data (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, double along_ds, unsigned int mode, unsigned int ex_cols, enum GMT_enum_track smode);
EXTERN_MSC struct GMT_DATASET * GMT_crosstracks (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, double cross_length, double across_ds, uint64_t n_cols, bool alternate);
EXTERN_MSC uint64_t GMT_resample_path (struct GMT_CTRL *GMT, double **x, double **y, uint64_t n_in, double step_out, enum GMT_enum_track mode);
EXTERN_MSC bool GMT_crossing_dateline (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S);
EXTERN_MSC int GMT_err_func (struct GMT_CTRL *GMT, int err, bool fail, char *file, const char *where);
EXTERN_MSC int64_t GMT_delaunay (struct GMT_CTRL *GMT, double *x_in, double *y_in, uint64_t n, int **link);
EXTERN_MSC void GMT_delaunay_free (struct GMT_CTRL *GMT, int **link);
EXTERN_MSC unsigned int GMT_get_prime_factors (struct GMT_CTRL *GMT, uint64_t n, unsigned int *f);
EXTERN_MSC int64_t GMT_voronoi (struct GMT_CTRL *GMT, double *x_in, double *y_in, uint64_t n, double *we, double **x_out, double **y_out);

/* gmt_regexp.c */

EXTERN_MSC int gmt_regexp_match (struct GMT_CTRL *GMT, const char *subject, const char *pattern, bool caseless);

/* gmt_vector.c: */

EXTERN_MSC void GMT_cart_to_geo (struct GMT_CTRL *GMT, double *lat, double *lon, double *a, bool degrees);
EXTERN_MSC void GMT_geo_to_cart (struct GMT_CTRL *GMT, double lat, double lon, double *a, bool degrees);
EXTERN_MSC void GMT_add3v (struct GMT_CTRL *GMT, double *a, double *b, double *c);
EXTERN_MSC void GMT_sub3v (struct GMT_CTRL *GMT, double *a, double *b, double *c);
EXTERN_MSC double GMT_dot3v (struct GMT_CTRL *GMT, double *a, double *b);
EXTERN_MSC double GMT_dot2v (struct GMT_CTRL *GMT, double *a, double *b);
EXTERN_MSC double GMT_mag3v (struct GMT_CTRL *GMT, double *a);
EXTERN_MSC void GMT_cross3v (struct GMT_CTRL *GMT, double *a, double *b, double *c);
EXTERN_MSC void GMT_normalize3v (struct GMT_CTRL *GMT, double *a);
EXTERN_MSC void GMT_normalize2v (struct GMT_CTRL *GMT, double *a);
EXTERN_MSC uint64_t GMT_fix_up_path (struct GMT_CTRL *GMT, double **a_lon, double **a_lat, uint64_t n, double step, unsigned int mode);
EXTERN_MSC int GMT_jacobi (struct GMT_CTRL *GMT, double *a, unsigned int n, unsigned int m, double *d, double *v, double *b, double *z, unsigned int *nrots);
EXTERN_MSC int GMT_gauss (struct GMT_CTRL *GMT, double *a, double *vec, unsigned int n, unsigned int nstore, bool itriag);
EXTERN_MSC int GMT_gaussjordan (struct GMT_CTRL *GMT, double *a, unsigned int n, unsigned int ndim, double *b, unsigned int m, unsigned int mdim);
EXTERN_MSC int GMT_svdcmp (struct GMT_CTRL *GMT, double *a, unsigned int m, unsigned int n, double *w, double *v);
EXTERN_MSC int GMT_solve_svd (struct GMT_CTRL *GMT, double *u, unsigned int m, unsigned int n, double *v, double *w, double *b, unsigned int k, double *x, double *cutoff, unsigned int mode);
EXTERN_MSC void GMT_polar_to_cart (struct GMT_CTRL *GMT, double r, double theta, double *a, bool degrees);
EXTERN_MSC void GMT_cart_to_polar (struct GMT_CTRL *GMT, double *r, double *theta, double *a, bool degrees);

/* From gmt_parse.c */
/* This macro is called via each modules Return macro so API and options are set */
#define GMT_Free_Options(mode) {if (mode >= 0 && GMT_Destroy_Options (API, &options) != GMT_OK) exit (EXIT_FAILURE);}

/* From gmt_api.c */
EXTERN_MSC struct GMTAPI_CTRL * GMT_get_API_ptr (struct GMTAPI_CTRL *ptr);
EXTERN_MSC void GMT_show_name_and_purpose (void *API, const char *name, const char *component, const char *purpose);

#endif /* _GMT_PROTOTYPES_H */
