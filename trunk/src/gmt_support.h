/*--------------------------------------------------------------------
 *	$Id: gmt_support.h,v 1.8 2006-04-13 06:20:35 pwessel Exp $
 *
 *	Copyright (c) 1991-2006 by P. Wessel and W. H. F. Smith
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

#ifndef _GMT_SUPPORT_H
#define _GMT_SUPPORT_H

EXTERN_MSC BOOLEAN GMT_polygon_is_open (double x[], double y[], int n);
EXTERN_MSC double GMT_cartesian_dist (double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_dist_to_point (double lon, double lat, struct GMT_TABLE *T, int *id);
EXTERN_MSC double GMT_flatearth_dist_km (double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_flatearth_dist_meter (double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_get_annot_offset (BOOLEAN *flip, int level);
EXTERN_MSC double GMT_get_map_interval (int axis, int item);
EXTERN_MSC double GMT_getradius (char *line);
EXTERN_MSC double GMT_great_circle_dist_km (double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_great_circle_dist_meter (double x0, double y0, double x1, double y1);
EXTERN_MSC int GMT_akima (double *x, double *y, int nx, double *c);
EXTERN_MSC int GMT_annot_pos (double min, double max, struct GMT_PLOT_AXIS_ITEM *T, double coord[], double *pos);
EXTERN_MSC int GMT_check_rgb (int *rgb);
EXTERN_MSC int GMT_comp_double_asc (const void *p_1, const void *p_2);
EXTERN_MSC int GMT_comp_float_asc (const void *p_1, const void *p_2);
EXTERN_MSC int GMT_comp_int_asc (const void *p_1, const void *p_2);
EXTERN_MSC int GMT_contours (float *grd, struct GRD_HEADER *header, int smooth_factor, int int_scheme, int *side, int *edge, int first, double **x_array, double **y_array);
EXTERN_MSC int GMT_cspline (double *x, double *y, int n, double *c);
EXTERN_MSC int GMT_delaunay (double *x_in, double *y_in, int n, int **link);
EXTERN_MSC int GMT_flip_justify (int justify);
EXTERN_MSC int GMT_get_dist_scale (char c, double *d_scale, int *proj_type, PFD *distance_func);
EXTERN_MSC int GMT_get_format (double interval, char *unit, char *prefix, char *format);
EXTERN_MSC int GMT_get_rgb24 (double value, int *rgb);
EXTERN_MSC int GMT_getfill (char *line, struct GMT_FILL *fill);
EXTERN_MSC int GMT_getinc (char *line, double *dx, double *dy);
EXTERN_MSC int GMT_getincn (char *line, double inc[], int n);
EXTERN_MSC int GMT_getpen (char *line, struct GMT_PEN *pen);
EXTERN_MSC int GMT_getrgb (char *line, int *rgb);
EXTERN_MSC int GMT_getrose (char *text, struct GMT_MAP_ROSE *mr);
EXTERN_MSC int GMT_getscale (char *text, struct GMT_MAP_SCALE *ms);
EXTERN_MSC int GMT_grd_setregion (struct GRD_HEADER *h, double *xmin, double *xmax, double *ymin, double *ymax);
EXTERN_MSC int GMT_inonout_sphpol (double plon, double plat, const struct GMT_LINE_SEGMENT *P);
EXTERN_MSC int GMT_intpol (double *x, double *y, int n, int m, double *u, double *v, int mode);
EXTERN_MSC int GMT_just_decode (char *key, int i, int j);
EXTERN_MSC int GMT_log_array (double min, double max, double delta, double **array);
EXTERN_MSC int GMT_minmaxinc_verify (double min, double max, double inc, double slop);
EXTERN_MSC int GMT_near_a_line_cartesian (double lon, double lat, struct GMT_TABLE *T, BOOLEAN return_mindist, double *dist_min, double *x_near, double *y_near);
EXTERN_MSC int GMT_near_a_line_spherical (double lon, double lat, struct GMT_TABLE *T, BOOLEAN return_mindist, double *dist_min, double *x_near, double *y_near);
EXTERN_MSC int GMT_near_a_point (double x, double y, struct GMT_TABLE *T, double dist);
EXTERN_MSC int GMT_near_a_point_cart (double x, double y, struct GMT_TABLE *T, double dist);
EXTERN_MSC int GMT_non_zero_winding (double xp, double yp, double *x, double *y, int n_path);
EXTERN_MSC int GMT_set_cpt_path (char *cpt_file, char *table);
EXTERN_MSC int GMT_strtok (const char *string, const char *sep, int *start, char *token);
EXTERN_MSC int GMT_verify_expectations (int wanted, int got, char *item);
EXTERN_MSC void GMT_RI_prepare (struct GRD_HEADER *h);
EXTERN_MSC void GMT_chop (char *string);
EXTERN_MSC void GMT_dump_contour (double *xx, double *yy, int nn, double cval, int id, BOOLEAN interior, char *file);
EXTERN_MSC void GMT_free (void *addr);
EXTERN_MSC void GMT_get_plot_array (void);
EXTERN_MSC void GMT_get_primary_annot (struct GMT_PLOT_AXIS *A, int *primary, int *secondary);
EXTERN_MSC void GMT_grd_init (struct GRD_HEADER *header, int argc, char **argv, BOOLEAN update);
EXTERN_MSC void GMT_grd_shift (struct GRD_HEADER *header, float *grd, double shift);
EXTERN_MSC void GMT_illuminate (double intensity, int *rgb);
EXTERN_MSC void GMT_init_fill (struct GMT_FILL *fill, int r, int g, int b);
EXTERN_MSC void GMT_init_pen (struct GMT_PEN *pen, double width);
EXTERN_MSC void GMT_list_custom_symbols (void);
EXTERN_MSC void GMT_read_cpt (char *cpt_file);
EXTERN_MSC void GMT_rotate2D (double x[], double y[], int n, double x0, double y0, double angle, double xp[], double yp[]);
EXTERN_MSC void GMT_smart_justify (int just, double angle, double dx, double dy, double *x_shift, double *y_shift);
EXTERN_MSC void GMT_str_tolower (char *string);
EXTERN_MSC void GMT_str_toupper (char *string);
EXTERN_MSC void *GMT_memory (void *prev_addr, size_t nelem, size_t size, char *progname);
EXTERN_MSC char *GMT_convertpen (struct GMT_PEN *pen, int *width, int *offset, int rgb[]);
EXTERN_MSC void GMT_fourt (float *data, int *nn, int ndim, int ksign, int iform, float *work);

#endif /* _GMT_SUPPORT_H */
