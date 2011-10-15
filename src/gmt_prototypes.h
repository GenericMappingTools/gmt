/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
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
/* gmt_prototypes.h  --  All low-level GMT API function prototypes.

   Authors:	P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
   Date:	1-OCT-2009
   Version:	5 API

*/

#ifndef _GMT_PROTOTYPES_H
#define _GMT_PROTOTYPES_H

/* gmt_bcr.c: */
EXTERN_MSC double GMT_get_bcr_z (struct GMT_CTRL *C, struct GMT_GRID *G, double xx, double yy);		/* Compute z(x,y) from bcr structure and grid */
EXTERN_MSC GMT_LONG GMT_get_bcr_img (struct GMT_CTRL *C, struct GMT_IMAGE *G, double xx, double yy, unsigned char *z);		/* Compute z(x,y) from bcr structure and image */

/* gmt_customio.c: */

#ifdef USE_GDAL
/* Format # 22 */
EXTERN_MSC int GMT_gdalread (struct GMT_CTRL *C, char *gdal_filename, struct GDALREAD_CTRL *prhs, struct GD_CTRL *Ctrl);
EXTERN_MSC int GMT_gdalwrite (struct GMT_CTRL *C, char *filename, struct GDALWRITE_CTRL *prhs);
#endif

/* gmt_fft.c: */

EXTERN_MSC GMT_LONG GMT_fft_1d (struct GMT_CTRL *C, float *data, GMT_LONG n, GMT_LONG direction, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_fft_2d (struct GMT_CTRL *C, float *data, GMT_LONG nx, GMT_LONG ny, GMT_LONG direction, GMT_LONG mode);

/* gmt_grdio.c: */

EXTERN_MSC GMT_LONG GMT_create_grid (struct GMT_CTRL *C, struct GMT_GRID **Gout);
EXTERN_MSC struct GMT_GRID *GMT_duplicate_grid (struct GMT_CTRL *C, struct GMT_GRID *G, GMT_LONG alloc_data);
EXTERN_MSC struct GRD_HEADER *GMT_duplicate_gridheader (struct GMT_CTRL *C, struct GRD_HEADER *h);
EXTERN_MSC void GMT_grd_init (struct GMT_CTRL *C, struct GRD_HEADER *header, struct GMT_OPTION *options, GMT_LONG update);
EXTERN_MSC void GMT_decode_grd_h_info (struct GMT_CTRL *C, char *input, struct GRD_HEADER *h);
EXTERN_MSC void GMT_close_grd (struct GMT_CTRL *C, struct GMT_GRDFILE *G);
EXTERN_MSC void GMT_free_grid (struct GMT_CTRL *C, struct GMT_GRID **G, GMT_LONG free_grid);
EXTERN_MSC void GMT_set_grddim (struct GMT_CTRL *C, struct GRD_HEADER *h);
EXTERN_MSC void GMT_grd_pad_on (struct GMT_CTRL *C, struct GMT_GRID *G, GMT_LONG *pad);
EXTERN_MSC void GMT_grd_pad_off (struct GMT_CTRL *C, struct GMT_GRID *G);
EXTERN_MSC void GMT_grd_pad_zero (struct GMT_CTRL *C, struct GMT_GRID *G);
EXTERN_MSC void GMT_grd_do_scaling (struct GMT_CTRL *C, float *grid, GMT_LONG nm, double scale, double offset);
EXTERN_MSC void GMT_grd_zminmax (struct GMT_CTRL *C, struct GMT_GRID *G);
EXTERN_MSC GMT_LONG GMT_read_grd_info (struct GMT_CTRL *C, char *file, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_write_grd_info (struct GMT_CTRL *C, char *file, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_read_grd (struct GMT_CTRL *C, char *file, struct GRD_HEADER *header, float *grid, double *wesn, GMT_LONG *pad, GMT_LONG complex);
EXTERN_MSC GMT_LONG GMT_write_grd (struct GMT_CTRL *C, char *file, struct GRD_HEADER *header, float *grid, double *wesn, GMT_LONG *pad, GMT_LONG complex);
EXTERN_MSC GMT_LONG GMT_adjust_loose_wesn (struct GMT_CTRL *C, double wesn[], struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_grd_setregion (struct GMT_CTRL *C, struct GRD_HEADER *h, double *wesn, GMT_LONG interpolant);
EXTERN_MSC GMT_LONG GMT_grd_RI_verify (struct GMT_CTRL *C, struct GRD_HEADER *h, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_open_grd (struct GMT_CTRL *C, char *file, struct GMT_GRDFILE *G, char mode);
EXTERN_MSC GMT_LONG GMT_read_grd_row (struct GMT_CTRL *C, struct GMT_GRDFILE *G, GMT_LONG row_no, float *row);
EXTERN_MSC GMT_LONG GMT_write_grd_row (struct GMT_CTRL *C, struct GMT_GRDFILE *G, float *row);
EXTERN_MSC GMT_LONG GMT_read_img (struct GMT_CTRL *C, char *imgfile, struct GMT_GRID *G, double *wesn, double scale, GMT_LONG mode, double lat, GMT_LONG init);
EXTERN_MSC GMT_LONG GMT_conv_intext2dbl (struct GMT_CTRL *C, char *record, GMT_LONG ncols);
EXTERN_MSC GMT_LONG GMT_grd_pad_status (struct GMT_CTRL *C, struct GRD_HEADER *header, GMT_LONG *pad);
EXTERN_MSC GMT_LONG GMT_set_outgrid (struct GMT_CTRL *C, struct GMT_GRID *G, struct GMT_GRID **Out);
EXTERN_MSC GMT_LONG GMT_init_newgrid (struct GMT_CTRL *C, struct GMT_GRID *G, double wesn[], double inc[], GMT_LONG node_offset);
EXTERN_MSC GMT_LONG GMT_change_grdreg (struct GMT_CTRL *C, struct GRD_HEADER *h, GMT_LONG registration);
#ifdef USE_GDAL
EXTERN_MSC GMT_LONG GMT_read_image (struct GMT_CTRL *C, char *file, struct GMT_IMAGE *I, double *wesn, 
			GMT_LONG *pad, GMT_LONG complex);		/* Function to read true images via GDAL */
GMT_LONG GMT_read_image_info (struct GMT_CTRL *C, char *file, struct GMT_IMAGE *I);
#endif

#ifdef _PSLIB_H
/* gmt_plot.c prototypes only included if pslib has been included */

EXTERN_MSC char * GMT_export2proj4 (struct GMT_CTRL *C);
EXTERN_MSC void GMT_textpath_init (struct GMT_CTRL *C, struct GMT_PEN *LP, double Brgb[], struct GMT_PEN *BP, double Frgb[]);
EXTERN_MSC void GMT_draw_map_rose (struct GMT_CTRL *C, struct GMT_MAP_ROSE *mr);
EXTERN_MSC void GMT_draw_map_scale (struct GMT_CTRL *C, struct GMT_MAP_SCALE *ms);
EXTERN_MSC void GMT_geo_line (struct GMT_CTRL *C, double *lon, double *lat, GMT_LONG n);
EXTERN_MSC void GMT_geo_polygons (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S);
EXTERN_MSC void GMT_geo_ellipse (struct GMT_CTRL *C, double lon, double lat, double major, double minor, double azimuth);
EXTERN_MSC void GMT_geo_rectangle (struct GMT_CTRL *C, double lon, double lat, double width, double height, double azimuth);
EXTERN_MSC void GMT_draw_front (struct GMT_CTRL *C, double x[], double y[], GMT_LONG n, struct GMT_FRONTLINE *f);
EXTERN_MSC void GMT_map_basemap (struct GMT_CTRL *C);
EXTERN_MSC void GMT_map_clip_off (struct GMT_CTRL *C);
EXTERN_MSC void GMT_map_clip_on (struct GMT_CTRL *C, double rgb[], GMT_LONG flag);
EXTERN_MSC void GMT_plot_line (struct GMT_CTRL *C, double *x, double *y, GMT_LONG *pen, GMT_LONG n);
EXTERN_MSC void GMT_setpen (struct GMT_CTRL *C, struct GMT_PEN *pen);
EXTERN_MSC void GMT_setfill (struct GMT_CTRL *C, struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_vertical_axis (struct GMT_CTRL *C, GMT_LONG mode);
EXTERN_MSC void GMT_xy_axis (struct GMT_CTRL *C, double x0, double y0, double length, double val0, double val1, struct GMT_PLOT_AXIS *A, GMT_LONG below, GMT_LONG annotate);
EXTERN_MSC void GMT_draw_custom_symbol (struct GMT_CTRL *C, double x0, double y0, double size[], struct GMT_CUSTOM_SYMBOL *symbol, struct GMT_PEN *pen, struct GMT_FILL *fill, GMT_LONG outline);
EXTERN_MSC void GMT_contlabel_plot (struct GMT_CTRL *C, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_plane_perspective (struct GMT_CTRL *C, GMT_LONG plane, double level);
EXTERN_MSC void GMT_plotcanvas (struct GMT_CTRL *C);
EXTERN_MSC GMT_LONG GMT_contlabel_save (struct GMT_CTRL *C, struct GMT_CONTOUR *G);
EXTERN_MSC GMT_LONG GMT_setfont (struct GMT_CTRL *C, struct GMT_FONT *F);
EXTERN_MSC GMT_LONG GMT_plotend (struct GMT_CTRL *C);
EXTERN_MSC void GMT_plotinit (struct GMT_CTRL *C, struct GMT_OPTION *options);
EXTERN_MSC GMT_LONG GMT_PSL_Create_Session (struct GMT_CTRL *C, struct PSL_CTRL **P);
EXTERN_MSC GMT_LONG GMT_PSL_Destroy_Session (struct GMT_CTRL *C, struct PSL_CTRL *P);
#endif

/* gmt_io.c: */

EXTERN_MSC void GMT_io_binary_header (struct GMT_CTRL *C, FILE *fp, GMT_LONG dir);
EXTERN_MSC GMT_LONG GMT_z_input (struct GMT_CTRL *C, FILE *fp, GMT_LONG *n, double **data);
EXTERN_MSC GMT_LONG GMT_z_output (struct GMT_CTRL *C, FILE *fp, GMT_LONG n, void *data);
EXTERN_MSC GMT_LONG GMT_get_io_type (struct GMT_CTRL *C, char type);
EXTERN_MSC PFL GMT_get_io_ptr (struct GMT_CTRL *C, GMT_LONG direction, GMT_LONG swap, char type);
EXTERN_MSC struct GMT_QUAD * GMT_quad_init (struct GMT_CTRL *C, GMT_LONG n_items);
EXTERN_MSC void GMT_quad_reset (struct GMT_CTRL *C, struct GMT_QUAD *Q, GMT_LONG n_items);
EXTERN_MSC void GMT_quad_add (struct GMT_CTRL *C, struct GMT_QUAD *Q, double x);
EXTERN_MSC GMT_LONG GMT_quad_finalize (struct GMT_CTRL *C, struct GMT_QUAD *Q);
EXTERN_MSC char *GMT_fgets (struct GMT_CTRL *C, char *line, GMT_LONG buf, FILE *fp);
EXTERN_MSC int GMT_fclose (struct GMT_CTRL *C, FILE *stream);
EXTERN_MSC int GMT_access (struct GMT_CTRL *C, const char *filename, int mode);		/* access wrapper */
EXTERN_MSC FILE * GMT_fopen (struct GMT_CTRL *C, const char *filename, const char *mode);
EXTERN_MSC char * GMT_getdatapath (struct GMT_CTRL *C, const char *stem, char *path);	/* Look for data file */
EXTERN_MSC char * GMT_getsharepath (struct GMT_CTRL *C, const char *subdir, const char *stem, const char *suffix, char *path);	/* Look for shared file */
EXTERN_MSC void GMT_write_tableheader (struct GMT_CTRL *C, FILE *fp, char *txt);
EXTERN_MSC void GMT_write_segmentheader (struct GMT_CTRL *C, FILE *fp, GMT_LONG n);		/* Write segment header back out */
EXTERN_MSC void GMT_write_textrecord (struct GMT_CTRL *C, FILE *fp, char *txt);
EXTERN_MSC void GMT_ascii_format_col (struct GMT_CTRL *C, char *text, double x, GMT_LONG col);
EXTERN_MSC void GMT_lon_range_adjust (GMT_LONG range, double *lon);		/* Adjust the longitude given the desired range */
EXTERN_MSC void GMT_add_to_record (struct GMT_CTRL *C, char *record, double val, GMT_LONG col, GMT_LONG sep);
EXTERN_MSC GMT_LONG GMT_scanf (struct GMT_CTRL *C, char *p, GMT_LONG expectation, double *val);	/* Convert strings to double, handling special formats [Data records only ] */
EXTERN_MSC GMT_LONG GMT_scanf_arg (struct GMT_CTRL *C, char *p, GMT_LONG expectation, double *val);	/* Convert strings to double, handling special formats [ command line only ] */
EXTERN_MSC GMT_LONG GMT_ascii_output_col (struct GMT_CTRL *C, FILE *fp, double x, GMT_LONG col);
EXTERN_MSC GMT_LONG GMT_not_numeric (struct GMT_CTRL *C, char *text);				/* Rules out _some_ text as possible numerics */
EXTERN_MSC GMT_LONG GMT_parse_segment_item (struct GMT_CTRL *C, char *in_string, char *pattern, char *out_string);
EXTERN_MSC GMT_LONG GMT_set_cols (struct GMT_CTRL *C, GMT_LONG direction, GMT_LONG expected);
EXTERN_MSC GMT_LONG GMT_create_dataset (struct GMT_CTRL *C, GMT_LONG n_tables, GMT_LONG n_segments, GMT_LONG n_columns, GMT_LONG n_rows, struct GMT_DATASET **Dout);
EXTERN_MSC GMT_LONG GMT_create_table (struct GMT_CTRL *C, GMT_LONG n_segments, GMT_LONG n_columns, GMT_LONG n_rows, struct GMT_TABLE **Tout);
EXTERN_MSC void GMT_free_textset (struct GMT_CTRL *C, struct GMT_TEXTSET **data);
EXTERN_MSC void GMT_duplicate_textset (struct GMT_CTRL *C, struct GMT_TEXTSET *Din, struct GMT_TEXTSET **Dout, GMT_LONG mode);
EXTERN_MSC void GMT_adjust_dataset (struct GMT_CTRL *C, struct GMT_DATASET *D, GMT_LONG n_columns);
EXTERN_MSC void GMT_alloc_dataset (struct GMT_CTRL *C, struct GMT_DATASET *Din, GMT_LONG n_columns, GMT_LONG n_rows, GMT_LONG mode, struct GMT_DATASET **Dout);
EXTERN_MSC void GMT_duplicate_dataset (struct GMT_CTRL *C, struct GMT_DATASET *Din, GMT_LONG n_columns, GMT_LONG mode, struct GMT_DATASET **Dout);
EXTERN_MSC GMT_LONG GMT_read_table (struct GMT_CTRL *C, void *source, GMT_LONG source_type, struct GMT_TABLE **T, GMT_LONG greenwich, GMT_LONG poly, GMT_LONG use_GMT_io);
EXTERN_MSC GMT_LONG GMT_write_dataset (struct GMT_CTRL *C, void *dest, GMT_LONG dest_type, struct GMT_DATASET *D, GMT_LONG use_GMT_io, GMT_LONG table);
EXTERN_MSC GMT_LONG GMT_write_table (struct GMT_CTRL *C, void *dest, GMT_LONG dest_type, struct GMT_TABLE *T, GMT_LONG use_GMT_io, GMT_LONG io_mode);
EXTERN_MSC GMT_LONG GMT_alloc_segment (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S, GMT_LONG n_rows, GMT_LONG n_columns, GMT_LONG first);
EXTERN_MSC void GMT_free_segment (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *segment);
EXTERN_MSC void GMT_free_table (struct GMT_CTRL *C, struct GMT_TABLE *table);
EXTERN_MSC void GMT_free_dataset (struct GMT_CTRL *C, struct GMT_DATASET **data);
EXTERN_MSC GMT_LONG GMT_free_palette (struct GMT_CTRL *C, struct GMT_PALETTE **P);
EXTERN_MSC struct GMT_IMAGE *GMT_create_image (struct GMT_CTRL *C);
EXTERN_MSC void GMT_free_image (struct GMT_CTRL *C, struct GMT_IMAGE **I, GMT_LONG free_image);
EXTERN_MSC GMT_LONG GMT_create_matrix (struct GMT_CTRL *C, struct GMT_MATRIX **M_out);
EXTERN_MSC void GMT_free_matrix (struct GMT_CTRL *C, struct GMT_MATRIX **M, GMT_LONG free_matrix);
EXTERN_MSC GMT_LONG GMT_create_vector (struct GMT_CTRL *C, GMT_LONG n_columns, struct GMT_VECTOR **Vout);
EXTERN_MSC void GMT_free_vector (struct GMT_CTRL *C, struct GMT_VECTOR **V, GMT_LONG free_vector);
EXTERN_MSC GMT_LONG GMT_validate_aspatial (struct GMT_CTRL *C, struct GMT_OGR *G);
EXTERN_MSC GMT_LONG GMT_load_aspatial_values (struct GMT_CTRL *C, struct GMT_OGR *G);
EXTERN_MSC GMT_LONG GMT_load_aspatial_string (struct GMT_CTRL *C, struct GMT_OGR *G, GMT_LONG col, char out[GMT_BUFSIZ]);
EXTERN_MSC double GMT_get_aspatial_value (struct GMT_CTRL *C, GMT_LONG col, struct GMT_LINE_SEGMENT *S);
EXTERN_MSC void GMT_set_seg_minmax (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S);
EXTERN_MSC void GMT_set_seg_polar (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S);
EXTERN_MSC void GMT_skip_xy_duplicates (struct GMT_CTRL *C, GMT_LONG mode);
EXTERN_MSC void GMT_duplicate_ogr_seg (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S_to, struct GMT_LINE_SEGMENT *S_from);
EXTERN_MSC GMT_LONG GMT_append_ogr_item (struct GMT_CTRL *C, char *name, GMT_LONG type, struct GMT_OGR *S);
EXTERN_MSC void GMT_write_ogr_header (FILE *fp, struct GMT_OGR *G);
EXTERN_MSC GMT_LONG GMT_trim_segheader (struct GMT_CTRL *C, char *line);

/* gmt_proj.c: */

EXTERN_MSC double GMT_lat_swap_quick (struct GMT_CTRL *C, double lat, double c[]);
EXTERN_MSC double GMT_lat_swap (struct GMT_CTRL *C, double lat, GMT_LONG itype);
EXTERN_MSC void GMT_lat_swap_init (struct GMT_CTRL *C);

/* gmt_support.c: */

EXTERN_MSC GMT_LONG GMT_split_line_at_dateline (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S, struct GMT_LINE_SEGMENT ***Lout);
EXTERN_MSC GMT_LONG GMT_split_poly_at_dateline (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S, struct GMT_LINE_SEGMENT ***Lout);
EXTERN_MSC GMT_LONG GMT_x_is_outside (struct GMT_CTRL *C, double *x, double left, double right);
EXTERN_MSC void GMT_set_xy_domain (struct GMT_CTRL *C, double wesn_extended[], struct GRD_HEADER *h);
EXTERN_MSC GMT_LONG GMT_BC_init (struct GMT_CTRL *C, struct GRD_HEADER *h);
EXTERN_MSC GMT_LONG GMT_grd_BC_set (struct GMT_CTRL *C, struct GMT_GRID *G);
EXTERN_MSC GMT_LONG GMT_image_BC_set (struct GMT_CTRL *C, struct GMT_IMAGE *I);
EXTERN_MSC GMT_LONG GMT_y_out_of_bounds (struct GMT_CTRL *C, GMT_LONG *j, struct GRD_HEADER *h, GMT_LONG *wrap_180);
EXTERN_MSC GMT_LONG GMT_x_out_of_bounds (struct GMT_CTRL *C, GMT_LONG *i, struct GRD_HEADER *h, GMT_LONG wrap_180);
EXTERN_MSC GMT_LONG GMT_list_cpt (struct GMT_CTRL *C, char option);
EXTERN_MSC void GMT_sample_cpt (struct GMT_CTRL *C, struct GMT_PALETTE *Pin, double z[], GMT_LONG nz, GMT_LONG continuous, GMT_LONG reverse, GMT_LONG log_mode, GMT_LONG no_inter, struct GMT_PALETTE **Pout);
EXTERN_MSC GMT_LONG GMT_write_cpt (struct GMT_CTRL *C, void *dest, GMT_LONG dest_type, GMT_LONG cpt_flags, struct GMT_PALETTE *P);
EXTERN_MSC void GMT_cpt_transparency (struct GMT_CTRL *C, struct GMT_PALETTE *P, double transparency, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_copy_palette (struct GMT_CTRL *C, struct GMT_PALETTE *P_to, struct GMT_PALETTE *P_from);
EXTERN_MSC GMT_LONG GMT_contlabel_info (struct GMT_CTRL *C, char flag, char *txt, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_contlabel_init (struct GMT_CTRL *C, struct GMT_CONTOUR *G, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_contlabel_specs (struct GMT_CTRL *C, char *txt, struct GMT_CONTOUR *G);
EXTERN_MSC GMT_LONG GMT_contlabel_prep (struct GMT_CTRL *C, struct GMT_CONTOUR *G, double xyz[2][3]);
EXTERN_MSC void GMT_contlabel_free (struct GMT_CTRL *C, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_hold_contour (struct GMT_CTRL *C, double **xx, double **yy, GMT_LONG nn, double zval, char *label, char ctype, double cangle, GMT_LONG closed, struct GMT_CONTOUR *G);
EXTERN_MSC void GMT_x_free (struct GMT_CTRL *C, struct GMT_XOVER *X);
EXTERN_MSC GMT_LONG GMT_init_track (struct GMT_CTRL *C, double y[], GMT_LONG n, struct GMT_XSEGMENT **S);
EXTERN_MSC GMT_LONG GMT_crossover (struct GMT_CTRL *C, double xa[], double ya[], GMT_LONG sa[], struct GMT_XSEGMENT A[], GMT_LONG na, double xb[], double yb[], GMT_LONG sb[], struct GMT_XSEGMENT B[], GMT_LONG nb, GMT_LONG internal, struct GMT_XOVER *X);
EXTERN_MSC GMT_LONG GMT_malloc_func (struct GMT_CTRL *C, void **ptr, GMT_LONG n, GMT_LONG n_alloc, size_t element_size, char *fname, GMT_LONG line);
EXTERN_MSC char *GMT_make_filename (struct GMT_CTRL *C, char *template, GMT_LONG fmt[], double z, GMT_LONG closed, GMT_LONG count[]);
EXTERN_MSC void GMT_str_setcase (struct GMT_CTRL *C, char *value, GMT_LONG mode);
EXTERN_MSC char *GMT_putusername (struct GMT_CTRL *C);
EXTERN_MSC GMT_LONG * GMT_prep_nodesearch (struct GMT_CTRL *GMT, struct GMT_GRID *G, double radius, GMT_LONG mode, GMT_LONG *d_row, GMT_LONG *actual_max_d_col);
EXTERN_MSC GMT_LONG GMT_detrend (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, double increment, double *intercept, double *slope, GMT_LONG mode);

/* gmt_calclock.c: */

EXTERN_MSC double GMT_rdc2dt (struct GMT_CTRL *C, GMT_LONG rd, double secs);
EXTERN_MSC void GMT_dt2rdc (struct GMT_CTRL *C, double t, GMT_LONG *rd, double *s);
EXTERN_MSC GMT_LONG GMT_rd_from_gymd (struct GMT_CTRL *C, GMT_LONG gy, GMT_LONG gm, GMT_LONG gd);
EXTERN_MSC void GMT_format_calendar (struct GMT_CTRL *C, char *date, char *clock, struct GMT_DATE_IO *D, struct GMT_CLOCK_IO *W, GMT_LONG upper, GMT_LONG kind, double dt);
EXTERN_MSC void GMT_gcal_from_rd (struct GMT_CTRL *C, GMT_LONG date, struct GMT_gcal *gcal);

/* gmt_init.c: */

EXTERN_MSC struct GMT_CTRL * GMT_begin (char *session, GMT_LONG mode);
EXTERN_MSC void GMT_end (struct GMT_CTRL *C);
EXTERN_MSC struct GMT_CTRL * GMT_begin_module (struct GMTAPI_CTRL *API, char *mod_name, struct GMT_CTRL **Ccopy);
EXTERN_MSC void GMT_end_module (struct GMT_CTRL *C, struct GMT_CTRL *Ccopy);
EXTERN_MSC GMT_LONG GMT_Complete_Options (struct GMT_CTRL *C, struct GMT_OPTION *options);
EXTERN_MSC GMT_LONG GMT_init_time_system_structure (struct GMT_CTRL *C, struct GMT_TIME_SYSTEM *time_system);
EXTERN_MSC void GMT_init_scales (struct GMT_CTRL *C, GMT_LONG unit, double *fwd_scale, double *inv_scale, double *inch_to_unit, double *unit_to_inch, char *unit_name);
EXTERN_MSC GMT_LONG GMT_set_measure_unit (struct GMT_CTRL *C, char unit);
EXTERN_MSC char *GMT_putfill (struct GMT_CTRL *C, struct GMT_FILL *F);
EXTERN_MSC char *GMT_putcolor (struct GMT_CTRL *C, double *rgb);
EXTERN_MSC char *GMT_putrgb (struct GMT_CTRL *C, double *rgb);
EXTERN_MSC char *GMT_putcmyk (struct GMT_CTRL *C, double *cmyk);
EXTERN_MSC char *GMT_puthsv (struct GMT_CTRL *C, double *hsv);
EXTERN_MSC double GMT_convert_units (struct GMT_CTRL *C, char *value, GMT_LONG from_default, GMT_LONG target_unit);
EXTERN_MSC GMT_LONG GMT_check_scalingopt (struct GMT_CTRL *C, char option, char unit, char *unit_name);
EXTERN_MSC GMT_LONG GMT_parse_common_options (struct GMT_CTRL *C, char *list, char option, char *item);
EXTERN_MSC GMT_LONG GMT_default_error (struct GMT_CTRL *C, char option);
EXTERN_MSC GMT_LONG GMT_get_time_system (struct GMT_CTRL *C, char *name, struct GMT_TIME_SYSTEM *time_system);
EXTERN_MSC GMT_LONG GMT_hash_lookup (struct GMT_CTRL *C, char *key, struct GMT_HASH *hashnode, GMT_LONG n, GMT_LONG n_hash);
EXTERN_MSC void GMT_syntax (struct GMT_CTRL *C, char option);
EXTERN_MSC void GMT_cont_syntax (struct GMT_CTRL *C, GMT_LONG indent, GMT_LONG kind);
EXTERN_MSC void GMT_GSHHS_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_mapscale_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_maprose_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_fill_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_pen_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_rgb_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_inc_syntax (struct GMT_CTRL *C, char option, GMT_LONG error);
EXTERN_MSC void GMT_label_syntax (struct GMT_CTRL *C, GMT_LONG indent, GMT_LONG kind);
EXTERN_MSC void GMT_dist_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_explain_options (struct GMT_CTRL *C, char *options);
EXTERN_MSC void GMT_getdefaults (struct GMT_CTRL *C, char *this_file);
EXTERN_MSC void GMT_putdefaults (struct GMT_CTRL *C, char *this_file);
EXTERN_MSC void GMT_hash_init (struct GMT_CTRL *C, struct GMT_HASH *hashnode , char **keys, GMT_LONG n_hash, GMT_LONG n_keys);
EXTERN_MSC GMT_LONG GMT_getdefpath (struct GMT_CTRL *C, char get, char **path);
EXTERN_MSC void GMT_extract_label (struct GMT_CTRL *C, char *line, char *label);
EXTERN_MSC void GMT_check_lattice (struct GMT_CTRL *C, double *inc, GMT_LONG *pixel, GMT_LONG *active);
EXTERN_MSC GMT_LONG GMT_check_binary_io (struct GMT_CTRL *C, GMT_LONG n_req);
EXTERN_MSC GMT_LONG GMT_setparameter (struct GMT_CTRL *C, char *keyword, char *value);
EXTERN_MSC char *GMT_putparameter (struct GMT_CTRL *C, char *keyword);
EXTERN_MSC void GMT_set_pad (struct GMT_CTRL *C, GMT_LONG npad);
EXTERN_MSC GMT_LONG GMT_get_ellipsoid (struct GMT_CTRL *C, char *name);

EXTERN_MSC void GMT_pickdefaults (struct GMT_CTRL *C, GMT_LONG lines, struct GMT_OPTION *options);
EXTERN_MSC void GMT_setdefaults (struct GMT_CTRL *C, struct GMT_OPTION *options);
EXTERN_MSC GMT_LONG GMT_geo_C_format (struct GMT_CTRL *C);
EXTERN_MSC GMT_LONG GMT_loaddefaults (struct GMT_CTRL *C, char *file);
EXTERN_MSC GMT_LONG GMT_equal_double (double A, double B, int maxUlps);
EXTERN_MSC GMT_LONG GMT_equal_float  (float A, float B, int maxUlps);

EXTERN_MSC int GMT_message (struct GMT_CTRL *C, char *format, ...);
EXTERN_MSC int GMT_report (struct GMT_CTRL *C, GMT_LONG level, char *format, ...);

#ifdef MIRONE 
EXTERN_MSC struct GMT_CTRL * GMT_short_begin (int argc, char **argv);
EXTERN_MSC void GMT_end_for_mex (struct GMT_CTRL *C);
#endif

/* gmt_map.c: */

EXTERN_MSC void GMT_auto_frame_interval (struct GMT_CTRL *C, GMT_LONG axis, GMT_LONG item);
EXTERN_MSC double GMT_az_backaz (struct GMT_CTRL *C, double lonE, double latE, double lonS, double latS, GMT_LONG baz);
EXTERN_MSC double GMT_distance (struct GMT_CTRL *C, double lonS, double latS, double lonE, double latE);
EXTERN_MSC GMT_LONG GMT_clip_to_map (struct GMT_CTRL *C, double *lon, double *lat, GMT_LONG np, double **x, double **y);
EXTERN_MSC GMT_LONG GMT_compact_line (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG pen_flag, int *pen);
EXTERN_MSC GMT_LONG GMT_geo_to_xy_line (struct GMT_CTRL *C, double *lon, double *lat, GMT_LONG n);
EXTERN_MSC GMT_LONG GMT_graticule_path (struct GMT_CTRL *C, double **x, double **y, GMT_LONG dir, double w, double e, double s, double n);
EXTERN_MSC GMT_LONG GMT_grd_project (struct GMT_CTRL *C, struct GMT_GRID *I, struct GMT_GRID *O, GMT_LONG inverse);
EXTERN_MSC GMT_LONG GMT_img_project (struct GMT_CTRL *C, struct GMT_IMAGE *I, struct GMT_IMAGE *O, GMT_LONG inverse);
EXTERN_MSC GMT_LONG GMT_map_clip_path (struct GMT_CTRL *C, double **x, double **y, GMT_LONG *donut);
EXTERN_MSC GMT_LONG GMT_map_outside (struct GMT_CTRL *C, double lon, double lat);
EXTERN_MSC void GMT_azim_to_angle (struct GMT_CTRL *C, double lon, double lat, double c, double azim, double *angle);
EXTERN_MSC GMT_LONG GMT_geo_to_xy (struct GMT_CTRL *C, double lon, double lat, double *x, double *y);
EXTERN_MSC void GMT_geoz_to_xy (struct GMT_CTRL *C, double x, double y, double z, double *x_out, double *y_out);
EXTERN_MSC GMT_LONG GMT_project_init (struct GMT_CTRL *C, struct GRD_HEADER *header, double *inc, GMT_LONG nx, GMT_LONG ny, GMT_LONG dpi, GMT_LONG offset);
EXTERN_MSC GMT_LONG GMT_map_setup (struct GMT_CTRL *C, double wesn[]);
EXTERN_MSC double GMT_x_to_xx (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_y_to_yy (struct GMT_CTRL *C, double y);
EXTERN_MSC double GMT_z_to_zz (struct GMT_CTRL *C, double z);
EXTERN_MSC void GMT_xy_to_geo (struct GMT_CTRL *C, double *lon, double *lat, double x, double y);
EXTERN_MSC void GMT_xyz_to_xy (struct GMT_CTRL *C, double x, double y, double z, double *x_out, double *y_out);
EXTERN_MSC void GMT_xyz_to_xy_n (struct GMT_CTRL *C, double *x, double *y, double z, GMT_LONG n);
EXTERN_MSC GMT_LONG GMT_dist_array (struct GMT_CTRL *C, double x[], double y[], GMT_LONG n, double scale, GMT_LONG dist_flag, double **dist);
EXTERN_MSC GMT_LONG GMT_map_truncate (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG start, GMT_LONG side);

/* gmt_shore.c: */

EXTERN_MSC void GMT_set_levels (struct GMT_CTRL *C, char *info, struct GMT_SHORE_SELECT *I);
EXTERN_MSC GMT_LONG GMT_get_shore_bin (struct GMT_CTRL *C, GMT_LONG b, struct GMT_SHORE *c);
EXTERN_MSC GMT_LONG GMT_get_br_bin (struct GMT_CTRL *C, GMT_LONG b, struct GMT_BR *c, GMT_LONG *level, GMT_LONG n_levels);
EXTERN_MSC void GMT_free_shore_polygons (struct GMT_CTRL *C, struct GMT_GSHHS_POL *p, GMT_LONG n);
EXTERN_MSC void GMT_free_shore (struct GMT_CTRL *C, struct GMT_SHORE *c);
EXTERN_MSC void GMT_free_br (struct GMT_CTRL *C, struct GMT_BR *c);
EXTERN_MSC void GMT_shore_cleanup (struct GMT_CTRL *C, struct GMT_SHORE *c);
EXTERN_MSC void GMT_br_cleanup (struct GMT_CTRL *C, struct GMT_BR *c);
EXTERN_MSC GMT_LONG GMT_init_shore (struct GMT_CTRL *C, char res, struct GMT_SHORE *c, double wesn[], struct GMT_SHORE_SELECT *I);
EXTERN_MSC GMT_LONG GMT_init_br (struct GMT_CTRL *C, char which, char res, struct GMT_BR *c, double wesn[]);
EXTERN_MSC GMT_LONG GMT_assemble_shore (struct GMT_CTRL *C, struct GMT_SHORE *c, GMT_LONG dir, GMT_LONG assemble, double west, double east, struct GMT_GSHHS_POL **pol);
EXTERN_MSC GMT_LONG GMT_assemble_br (struct GMT_CTRL *C, struct GMT_BR *c, GMT_LONG shift, double edge, struct GMT_GSHHS_POL **pol);
EXTERN_MSC GMT_LONG GMT_prep_shore_polygons (struct GMT_CTRL *C, struct GMT_GSHHS_POL **p, GMT_LONG np, GMT_LONG sample, double step, GMT_LONG anti_bin);
EXTERN_MSC GMT_LONG GMT_set_resolution (struct GMT_CTRL *C, char *res, char opt);
EXTERN_MSC char GMT_shore_adjust_res (struct GMT_CTRL *C, char res);

/* gmt_stat.c: */

EXTERN_MSC double GMT_bei (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_ber (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_kei (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_ker (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_plm (struct GMT_CTRL *C, GMT_LONG l, GMT_LONG m, double x);
EXTERN_MSC double GMT_plm_bar (struct GMT_CTRL *C, GMT_LONG l, GMT_LONG m, double x, GMT_LONG ortho);
EXTERN_MSC double GMT_factorial (struct GMT_CTRL *C, GMT_LONG n);
EXTERN_MSC double GMT_i0 (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_i1 (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_in (struct GMT_CTRL *C, GMT_LONG n, double x);
EXTERN_MSC double GMT_k0 (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_k1 (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_kn (struct GMT_CTRL *C, GMT_LONG n, double x);
EXTERN_MSC double GMT_dilog (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_sinc (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_erfinv (struct GMT_CTRL *C, double x);
EXTERN_MSC double GMT_rand (struct GMT_CTRL *C);
EXTERN_MSC double GMT_nrand (struct GMT_CTRL *C);
EXTERN_MSC double GMT_lrand (struct GMT_CTRL *C);
EXTERN_MSC GMT_LONG GMT_chebyshev (struct GMT_CTRL *C, double x, GMT_LONG n, double *t);
EXTERN_MSC double GMT_corrcoeff (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG mode);
EXTERN_MSC double GMT_corrcoeff_f (struct GMT_CTRL *C, float *x, float *y, GMT_LONG n, GMT_LONG mode);
EXTERN_MSC double GMT_Fcrit (struct GMT_CTRL *C, double alpha, double nu1, double nu2);
EXTERN_MSC double GMT_chi2crit (struct GMT_CTRL *C, double alpha, double nu);
EXTERN_MSC double GMT_extreme (struct GMT_CTRL *C, double *x, GMT_LONG n, double x_default, GMT_LONG kind, GMT_LONG way);
EXTERN_MSC double GMT_tcrit (struct GMT_CTRL *C, double alpha, double nu);
EXTERN_MSC double GMT_zcrit (struct GMT_CTRL *C, double alpha);
EXTERN_MSC double GMT_zdist (struct GMT_CTRL *C, double x);
EXTERN_MSC GMT_LONG GMT_f_q (struct GMT_CTRL *C, double chisq1, GMT_LONG nu1, double chisq2, GMT_LONG nu2, double *prob);
EXTERN_MSC GMT_LONG GMT_median (struct GMT_CTRL *C, double *x, GMT_LONG n, double xmin, double xmax, double m_initial, double *med);
EXTERN_MSC GMT_LONG GMT_mode (struct GMT_CTRL *C, double *x, GMT_LONG n, GMT_LONG j, GMT_LONG sort, GMT_LONG mode_selection, GMT_LONG *n_multiples, double *mode_est);
EXTERN_MSC GMT_LONG GMT_mode_f (struct GMT_CTRL *C, float *x, GMT_LONG n, GMT_LONG j, GMT_LONG sort, GMT_LONG mode_selection, GMT_LONG *n_multiples, double *mode_est);
EXTERN_MSC double GMT_mean (struct GMT_CTRL *C, double *x, GMT_LONG n);
EXTERN_MSC double GMT_std (struct GMT_CTRL *C, double *x, GMT_LONG n);
EXTERN_MSC double GMT_median_weighted (struct GMT_CTRL *C, fpair *data, GMT_LONG n, double quantile);
EXTERN_MSC double GMT_mode_weighted (struct GMT_CTRL *C, fpair *data, GMT_LONG n);
EXTERN_MSC GMT_LONG GMT_sig_f (struct GMT_CTRL *C, double chi1, GMT_LONG n1, double chi2, GMT_LONG n2, double level, double *prob);
EXTERN_MSC GMT_LONG GMT_student_t_a (struct GMT_CTRL *C, double t, GMT_LONG n, double *prob);
EXTERN_MSC void GMT_chi2 (struct GMT_CTRL *C, double chi2, double nu, double *prob);
EXTERN_MSC void GMT_cumpoisson (struct GMT_CTRL *C, double k, double mu, double *prob);
EXTERN_MSC void GMT_getmad (struct GMT_CTRL *C, double *x, GMT_LONG n, double location, double *scale);
EXTERN_MSC void GMT_getmad_f (struct GMT_CTRL *C, float *x, GMT_LONG n, double location, double *scale);
EXTERN_MSC double GMT_psi (struct GMT_CTRL *C, double z[], double p[]);
EXTERN_MSC void GMT_PvQv (struct GMT_CTRL *C, double x, double v_ri[], double pq[], GMT_LONG *iter);
EXTERN_MSC double GMT_quantile (struct GMT_CTRL *C, double *x, double q, GMT_LONG n);
EXTERN_MSC double GMT_quantile_f (struct GMT_CTRL *C, float *x, double q, GMT_LONG n);
EXTERN_MSC GMT_LONG GMT_chol_dcmp (struct GMT_CTRL *C, double *a, double *d, double *cond, GMT_LONG nr, GMT_LONG n);
EXTERN_MSC void GMT_chol_recover (struct GMT_CTRL *C, double *a, double *d, GMT_LONG nr, GMT_LONG n, GMT_LONG nerr, GMT_LONG donly);
EXTERN_MSC void GMT_chol_solv (struct GMT_CTRL *C, double *a, double *x, double *y, GMT_LONG nr, GMT_LONG n);
EXTERN_MSC void GMT_set_tbl_minmax (struct GMT_CTRL *C, struct GMT_TABLE *T);

/* gmt_support.c: */

EXTERN_MSC GMT_LONG GMT_crossing_dateline (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S);
EXTERN_MSC GMT_LONG GMT_near_lines (struct GMT_CTRL *C, double lon, double lat, struct GMT_TABLE *T, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near);
EXTERN_MSC GMT_LONG GMT_near_a_line (struct GMT_CTRL *C, double lon, double lat, GMT_LONG seg, struct GMT_LINE_SEGMENT *S, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near);
EXTERN_MSC GMT_LONG GMT_near_a_point (struct GMT_CTRL *C, double x, double y, struct GMT_TABLE *T, double dist);
EXTERN_MSC void GMT_sort_array (struct GMT_CTRL *C, void *base, GMT_LONG n, GMT_LONG type);
EXTERN_MSC GMT_LONG GMT_err_func (struct GMT_CTRL *C, GMT_LONG err, GMT_LONG fail, char *file, char *fname, int line);
EXTERN_MSC GMT_LONG GMT_parse_segment_header (struct GMT_CTRL *C, char *header, struct GMT_PALETTE *P, GMT_LONG *use_fill, struct GMT_FILL *fill, struct GMT_FILL def_fill,  GMT_LONG *use_pen, struct GMT_PEN *pen, struct GMT_PEN def_pen, GMT_LONG def_outline, struct GMT_OGR_SEG *G);
EXTERN_MSC GMT_LONG GMT_polygon_is_open (struct GMT_CTRL *C, double x[], double y[], GMT_LONG n);
EXTERN_MSC double GMT_polygon_area (struct GMT_CTRL *C, double x[], double y[], GMT_LONG n);
EXTERN_MSC GMT_LONG GMT_polygon_centroid (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, double *Cx, double *Cy);
EXTERN_MSC GMT_LONG GMT_get_distance (struct GMT_CTRL *C, char *line, double *dist, char *unit);
EXTERN_MSC GMT_LONG GMT_contours (struct GMT_CTRL *C, struct GMT_GRID *Grid, GMT_LONG smooth_factor, GMT_LONG int_scheme, GMT_LONG orient, GMT_LONG *edge, GMT_LONG *first, double **x, double **y);
EXTERN_MSC GMT_LONG GMT_init_distaz (struct GMT_CTRL *C, char c, GMT_LONG mode, GMT_LONG type);
EXTERN_MSC GMT_LONG GMT_get_format (struct GMT_CTRL *C, double interval, char *unit, char *prefix, char *format);
EXTERN_MSC GMT_LONG GMT_get_index (struct GMT_CTRL *C, struct GMT_PALETTE *P, double value);
EXTERN_MSC GMT_LONG GMT_get_rgb_from_z (struct GMT_CTRL *C, struct GMT_PALETTE *P, double value, double *rgb);
EXTERN_MSC GMT_LONG GMT_getfill (struct GMT_CTRL *C, char *line, struct GMT_FILL *fill);
EXTERN_MSC GMT_LONG GMT_getinc (struct GMT_CTRL *C, char *line, double inc[]);
EXTERN_MSC GMT_LONG GMT_getincn (struct GMT_CTRL *C, char *line, double inc[], GMT_LONG n);
EXTERN_MSC GMT_LONG GMT_getfont (struct GMT_CTRL *C, char *line, struct GMT_FONT *F);
EXTERN_MSC GMT_LONG GMT_getpen (struct GMT_CTRL *C, char *line, struct GMT_PEN *pen);
EXTERN_MSC GMT_LONG GMT_getrgb (struct GMT_CTRL *C, char *line, double *rgb);
EXTERN_MSC GMT_LONG GMT_getrose (struct GMT_CTRL *C, char *text, struct GMT_MAP_ROSE *mr);
EXTERN_MSC GMT_LONG GMT_getscale (struct GMT_CTRL *C, char *text, struct GMT_MAP_SCALE *ms);
EXTERN_MSC char *GMT_putfont (struct GMT_CTRL *C, struct GMT_FONT F);
EXTERN_MSC char *GMT_putpen (struct GMT_CTRL *C, struct GMT_PEN pen);
EXTERN_MSC GMT_LONG GMT_inonout (struct GMT_CTRL *C, double x, double y, const struct GMT_LINE_SEGMENT *S);
EXTERN_MSC GMT_LONG GMT_inonout_sphpol (struct GMT_CTRL *C, double plon, double plat, const struct GMT_LINE_SEGMENT *P);
EXTERN_MSC GMT_LONG GMT_intpol (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, GMT_LONG m, double *u, double *v, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_just_decode (struct GMT_CTRL *C, char *key, GMT_LONG def);
EXTERN_MSC GMT_LONG GMT_minmaxinc_verify (struct GMT_CTRL *C, double min, double max, double inc, double slop);
EXTERN_MSC GMT_LONG GMT_get_arc (struct GMT_CTRL *C, double x0, double y0, double r, double dir1, double dir2, double **x, double **y);
EXTERN_MSC GMT_LONG GMT_non_zero_winding (struct GMT_CTRL *C, double xp, double yp, double *x, double *y, GMT_LONG n_path);
EXTERN_MSC GMT_LONG GMT_strtok (struct GMT_CTRL *C, const char *string, const char *sep, GMT_LONG *start, char *token);
EXTERN_MSC GMT_LONG GMT_getmodopt (struct GMT_CTRL *C, const char *string, const char *sep, GMT_LONG *pos, char *token);
EXTERN_MSC GMT_LONG GMT_verify_expectations (struct GMT_CTRL *C, GMT_LONG wanted, GMT_LONG got, char *item);
EXTERN_MSC void GMT_RI_prepare (struct GMT_CTRL *C, struct GRD_HEADER *h);
EXTERN_MSC void GMT_chop (struct GMT_CTRL *C, char *string);
EXTERN_MSC char *GMT_chop_ext (struct GMT_CTRL *C, char *string);
EXTERN_MSC struct GMT_LINE_SEGMENT * GMT_dump_contour (struct GMT_CTRL *C, double *x, double *y, GMT_LONG n, double z);
EXTERN_MSC void GMT_get_plot_array (struct GMT_CTRL *C);
EXTERN_MSC void GMT_illuminate (struct GMT_CTRL *C, double intensity, double *rgb);
EXTERN_MSC void GMT_init_fill (struct GMT_CTRL *C, struct GMT_FILL *fill, double r, double g, double b);
EXTERN_MSC void GMT_init_pen (struct GMT_CTRL *C, struct GMT_PEN *pen, double width);
EXTERN_MSC GMT_LONG GMT_colorname2index (struct GMT_CTRL *C, char *name);
EXTERN_MSC void GMT_list_custom_symbols (struct GMT_CTRL *C);
EXTERN_MSC GMT_LONG GMT_read_cpt (struct GMT_CTRL *C, void *source, GMT_LONG source_type, GMT_LONG cpt_flags, struct GMT_PALETTE **P);
EXTERN_MSC void GMT_smart_justify (struct GMT_CTRL *C, GMT_LONG just, double angle, double dx, double dy, double *x_shift, double *y_shift);
EXTERN_MSC void GMT_fourt (struct GMT_CTRL *C, float *data, GMT_LONG *nn, GMT_LONG ndim, GMT_LONG ksign, GMT_LONG iform, float *work);
EXTERN_MSC void GMT_set_meminc (struct GMT_CTRL *C, GMT_LONG increment);
EXTERN_MSC void GMT_reset_meminc (struct GMT_CTRL *C);
EXTERN_MSC void * GMT_memory_func (struct GMT_CTRL *C, void *prev_addr, GMT_LONG nelem, size_t size, char *fname, GMT_LONG line);
EXTERN_MSC void GMT_free_func (struct GMT_CTRL *C, void **addr, char *fname, GMT_LONG line);
EXTERN_MSC GMT_LONG GMT_resample_data (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, double along_ds, GMT_LONG mode, GMT_LONG ex_cols, GMT_LONG smode, struct GMT_DATASET **Dout);
EXTERN_MSC GMT_LONG GMT_crosstracks (struct GMT_CTRL *GMT, struct GMT_DATASET *Din, double cross_length, double across_ds, GMT_LONG n_cols, struct GMT_DATASET **Dout);

/* gmt_regexp.c */

EXTERN_MSC GMT_LONG gmt_regexp_match (struct GMT_CTRL *C, const char *subject, const char *pattern, GMT_LONG caseless);

/* gmt_vector.c: */

EXTERN_MSC void GMT_cart_to_geo (struct GMT_CTRL *C, double *lat, double *lon, double *a, GMT_LONG degrees);
EXTERN_MSC void GMT_geo_to_cart (struct GMT_CTRL *C, double lat, double lon, double *a, GMT_LONG degrees);
EXTERN_MSC void GMT_add3v (struct GMT_CTRL *C, double *a, double *b, double *c);
EXTERN_MSC void GMT_sub3v (struct GMT_CTRL *C, double *a, double *b, double *c);
EXTERN_MSC double GMT_dot3v (struct GMT_CTRL *C, double *a, double *b);
EXTERN_MSC double GMT_dot2v (struct GMT_CTRL *C, double *a, double *b);
EXTERN_MSC double GMT_mag3v (struct GMT_CTRL *C, double *a);
EXTERN_MSC void GMT_cross3v (struct GMT_CTRL *C, double *a, double *b, double *c);
EXTERN_MSC void GMT_normalize3v (struct GMT_CTRL *C, double *a);
EXTERN_MSC void GMT_normalize2v (struct GMT_CTRL *C, double *a);
EXTERN_MSC GMT_LONG GMT_fix_up_path (struct GMT_CTRL *C, double **a_lon, double **a_lat, GMT_LONG n, double step, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_jacobi (struct GMT_CTRL *C, double *a, GMT_LONG *n, GMT_LONG *m, double *d, double *v, double *b, double *z, GMT_LONG *nrots);
EXTERN_MSC void GMT_gauss (struct GMT_CTRL *C, double *a, double *vec, GMT_LONG n, GMT_LONG nstore, double test, GMT_LONG *ierror, GMT_LONG itriag);
EXTERN_MSC GMT_LONG GMT_gaussjordan (struct GMT_CTRL *C, double *a, GMT_LONG n, GMT_LONG ndim, double *b, GMT_LONG m, GMT_LONG mdim);
EXTERN_MSC GMT_LONG GMT_svdcmp (struct GMT_CTRL *C, double *a, GMT_LONG m, GMT_LONG n, double *w, double *v);
EXTERN_MSC GMT_LONG GMT_solve_svd (struct GMT_CTRL *C, double *u, GMT_LONG m, GMT_LONG n, double *v, double *w, double *b, GMT_LONG k, double *x, double cutoff);
EXTERN_MSC void GMT_polar_to_cart (struct GMT_CTRL *C, double r, double theta, double *a, GMT_LONG degrees);
EXTERN_MSC void GMT_cart_to_polar (struct GMT_CTRL *C, double *r, double *theta, double *a, GMT_LONG degrees);

/* gmt_version.c: */

EXTERN_MSC char *GMT_version ();

/* grdfilter_func */
EXTERN_MSC double GMT_great_circle_dist_meter (struct GMT_CTRL *C, double x0, double y0, double x1, double y1);

/* grdblend_func */
EXTERN_MSC GMT_LONG GMT_check_region (struct GMT_CTRL *C, double wesn[]);
EXTERN_MSC GMT_LONG GMT_update_grd_info (struct GMT_CTRL *C, char *file, struct GRD_HEADER *header);
EXTERN_MSC GMT_LONG GMT_grd_get_format (struct GMT_CTRL *C, char *file, struct GRD_HEADER *header, GMT_LONG magic);

/* grdedit_func */
EXTERN_MSC GMT_LONG GMT_grd_is_global (struct GMT_CTRL *C, struct GRD_HEADER *h);
EXTERN_MSC void GMT_grd_shift (struct GMT_CTRL *C, struct GMT_GRID *Grid, double shift);

/* grdfft_func */
EXTERN_MSC GMT_LONG GMT_get_prime_factors (struct GMT_CTRL *C, GMT_LONG n, GMT_LONG *f);

/* grdmath_func */
EXTERN_MSC double GMT_mindist_to_point (struct GMT_CTRL *C, double lon, double lat, struct GMT_TABLE *T, GMT_LONG *id);

/* mapproject_func */
EXTERN_MSC GMT_LONG GMT_UTMzone_to_wesn (struct GMT_CTRL *C, GMT_LONG zone_x, GMT_LONG zone_y, GMT_LONG hemi, double wesn[]);
EXTERN_MSC void GMT_ECEF_forward (struct GMT_CTRL *C, double in[], double out[]);
EXTERN_MSC void GMT_ECEF_inverse (struct GMT_CTRL *C, double in[], double out[]);
EXTERN_MSC void GMT_ECEF_init (struct GMT_CTRL *C, struct GMT_DATUM *D);
EXTERN_MSC void GMT_datum_init (struct GMT_CTRL *C, struct GMT_DATUM *from, struct GMT_DATUM *to, GMT_LONG heights);
EXTERN_MSC GMT_LONG GMT_set_datum (struct GMT_CTRL *C, char *text, struct GMT_DATUM *D);
EXTERN_MSC void GMT_conv_datum (struct GMT_CTRL *C, double in[], double out[]);

/* pscontour_func */
EXTERN_MSC GMT_LONG GMT_delaunay (struct GMT_CTRL *C, double *x_in, double *y_in, GMT_LONG n, int **link);

/* psxy_func */
EXTERN_MSC GMT_LONG GMT_parse_symbol_option (struct GMT_CTRL *C, char *text, struct GMT_SYMBOL *p, GMT_LONG mode, GMT_LONG cmd);

/* psxyz_func */
EXTERN_MSC GMT_LONG GMT_parse_symbol_option (struct GMT_CTRL *C, char *text, struct GMT_SYMBOL *p, GMT_LONG mode, GMT_LONG cmd);

/* triangulate_func */
EXTERN_MSC GMT_LONG GMT_voronoi (struct GMT_CTRL *C, double *x_in, double *y_in, GMT_LONG n, double *we, double **x_out, double **y_out);

/* grd2xyz_func & xyz2grd_func */
EXTERN_MSC GMT_LONG GMT_parse_z_io (struct GMT_CTRL *C, char *txt, struct GMT_PARSE_Z_IO *z);
EXTERN_MSC GMT_LONG GMT_init_z_io (struct GMT_CTRL *C, char format[], GMT_LONG repeat[], GMT_LONG swab, GMT_LONG skip, char type, struct GMT_Z_IO *r);
EXTERN_MSC GMT_LONG GMT_set_z_io (struct GMT_CTRL *C, struct GMT_Z_IO *r, struct GMT_GRID *G);
EXTERN_MSC void GMT_check_z_io (struct GMT_CTRL *C, struct GMT_Z_IO *r, struct GMT_GRID *G);

/* From gmtapi_parse.c */
/* This macro is called via each modules Return macro so API and options are set */
#define GMT_Free_Options(mode) {if (mode >= 0 && GMT_Destroy_Options (API, &options)) exit (EXIT_FAILURE);}
EXTERN_MSC struct GMT_OPTION * GMT_Prep_Options (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args);

#endif /* _GMT_PROTOTYPES_H */
