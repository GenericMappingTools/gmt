/*--------------------------------------------------------------------
 *	$Id: gmt_support.h,v 1.55 2011-03-03 21:02:51 guru Exp $
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

#ifndef _GMT_SUPPORT_H
#define _GMT_SUPPORT_H

EXTERN_MSC GMT_LONG GMT_err_pass (GMT_LONG err, char *file);
EXTERN_MSC void GMT_err_fail (GMT_LONG err, char *file);
EXTERN_MSC GMT_LONG GMT_parse_multisegment_header (char *header, GMT_LONG use_cpt, GMT_LONG *use_fill, struct GMT_FILL *fill, struct GMT_FILL *def_fill,  GMT_LONG *use_pen, struct GMT_PEN *pen, struct GMT_PEN *def_pen, GMT_LONG def_outline);
EXTERN_MSC GMT_LONG GMT_polygon_is_open (double x[], double y[], GMT_LONG n);
EXTERN_MSC double GMT_cartesian_dist (double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_dist_to_point (double lon, double lat, struct GMT_TABLE *T, GMT_LONG *id);
EXTERN_MSC double GMT_flatearth_dist_km (double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_flatearth_dist_meter (double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_get_annot_offset (GMT_LONG *flip, GMT_LONG level);
EXTERN_MSC double GMT_get_map_interval (GMT_LONG axis, GMT_LONG item);
EXTERN_MSC double GMT_getradius (char *line);
EXTERN_MSC double GMT_great_circle_dist_km (double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_great_circle_dist_meter (double x0, double y0, double x1, double y1);
EXTERN_MSC GMT_LONG GMT_akima (double *x, double *y, GMT_LONG nx, double *c);
EXTERN_MSC GMT_LONG GMT_annot_pos (double min, double max, struct GMT_PLOT_AXIS_ITEM *T, double coord[], double *pos);
EXTERN_MSC GMT_LONG GMT_check_rgb (int *rgb);
EXTERN_MSC int GMT_comp_double_asc (const void *p_1, const void *p_2);
EXTERN_MSC int GMT_comp_float_asc (const void *p_1, const void *p_2);
EXTERN_MSC int GMT_comp_int_asc (const void *p_1, const void *p_2);
EXTERN_MSC GMT_LONG GMT_contours (float *grd, struct GRD_HEADER *header, GMT_LONG smooth_factor, GMT_LONG int_scheme, GMT_LONG orient, GMT_LONG *edge, GMT_LONG *first, double **x, double **y);
EXTERN_MSC GMT_LONG GMT_cspline (double *x, double *y, GMT_LONG n, double *c);
EXTERN_MSC GMT_LONG GMT_delaunay (double *x_in, double *y_in, GMT_LONG n, int **link);
EXTERN_MSC GMT_LONG GMT_voronoi (double *x_in, double *y_in, GMT_LONG n, double *we, double **x_out, double **y_out);
EXTERN_MSC GMT_LONG GMT_flip_justify (GMT_LONG justify);
EXTERN_MSC GMT_LONG GMT_get_dist_scale (char c, double *d_scale, GMT_LONG *proj_type, PFD *distance_func);
EXTERN_MSC GMT_LONG GMT_get_format (double interval, char *unit, char *prefix, char *format);
EXTERN_MSC GMT_LONG GMT_get_index (double value);
EXTERN_MSC void GMT_get_rgb_lookup (GMT_LONG index, double value, int *rgb);
EXTERN_MSC GMT_LONG GMT_get_rgb_from_z (double value, int *rgb);
EXTERN_MSC GMT_LONG GMT_get_fill_from_z (double value, struct GMT_FILL *fill);
EXTERN_MSC GMT_LONG GMT_getfill (char *line, struct GMT_FILL *fill);
EXTERN_MSC GMT_LONG GMT_getinc (char *line, double *dx, double *dy);
EXTERN_MSC GMT_LONG GMT_getincn (char *line, double inc[], GMT_LONG n);
EXTERN_MSC GMT_LONG GMT_getpen (char *line, struct GMT_PEN *pen);
EXTERN_MSC GMT_LONG GMT_getrgb_index (int *rgb);
EXTERN_MSC GMT_LONG GMT_getrgb (char *line, int *rgb);
EXTERN_MSC void GMT_enforce_rgb_triplets (char *text, GMT_LONG size);
EXTERN_MSC GMT_LONG GMT_getrose (char *text, struct GMT_MAP_ROSE *mr);
EXTERN_MSC GMT_LONG GMT_getscale (char *text, struct GMT_MAP_SCALE *ms);
EXTERN_MSC GMT_LONG GMT_inonout_sphpol (double plon, double plat, const struct GMT_LINE_SEGMENT *P);
EXTERN_MSC GMT_LONG GMT_intpol (double *x, double *y, GMT_LONG n, GMT_LONG m, double *u, double *v, GMT_LONG mode);
EXTERN_MSC GMT_LONG GMT_just_decode (char *key, GMT_LONG def);
EXTERN_MSC GMT_LONG GMT_log_array (double min, double max, double delta, double **array);
EXTERN_MSC GMT_LONG GMT_minmaxinc_verify (double min, double max, double inc, double slop);
EXTERN_MSC GMT_LONG GMT_near_a_line_cartesian (double lon, double lat, struct GMT_TABLE *T, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near);
EXTERN_MSC GMT_LONG GMT_near_a_line_spherical (double lon, double lat, struct GMT_TABLE *T, GMT_LONG return_mindist, double *dist_min, double *x_near, double *y_near);
EXTERN_MSC GMT_LONG GMT_near_a_point_spherical (double x, double y, struct GMT_TABLE *T, double dist);
EXTERN_MSC GMT_LONG GMT_near_a_point_cartesian (double x, double y, struct GMT_TABLE *T, double dist);
EXTERN_MSC GMT_LONG GMT_get_arc (double x0, double y0, double r, double dir1, double dir2, double **x, double **y);
EXTERN_MSC GMT_LONG GMT_non_zero_winding (double xp, double yp, double *x, double *y, GMT_LONG n_path);
EXTERN_MSC GMT_LONG GMT_set_cpt_path (char *cpt_file, char *table);
EXTERN_MSC GMT_LONG GMT_strlcmp (char *str1, char *str2);
EXTERN_MSC GMT_LONG GMT_strtok (const char *string, const char *sep, GMT_LONG *start, char *token);
EXTERN_MSC GMT_LONG GMT_verify_expectations (GMT_LONG wanted, GMT_LONG got, char *item);
EXTERN_MSC void GMT_RI_prepare (struct GRD_HEADER *h);
EXTERN_MSC void GMT_chop (char *string);
EXTERN_MSC char *GMT_chop_ext (char *string);
EXTERN_MSC void GMT_dump_contour (double *xx, double *yy, GMT_LONG nn, double cval, GMT_LONG id, GMT_LONG interior, char *file);
EXTERN_MSC void GMT_get_plot_array (void);
EXTERN_MSC void GMT_get_primary_annot (struct GMT_PLOT_AXIS *A, GMT_LONG *primary, GMT_LONG *secondary);
EXTERN_MSC void GMT_illuminate (double intensity, int *rgb);
EXTERN_MSC void GMT_init_fill (struct GMT_FILL *fill, int r, int g, int b);
EXTERN_MSC void GMT_init_pen (struct GMT_PEN *pen, double width);
EXTERN_MSC GMT_LONG GMT_colorname2index (char *name);
EXTERN_MSC void GMT_list_custom_symbols (void);
EXTERN_MSC GMT_LONG GMT_read_cpt (char *cpt_file);
EXTERN_MSC void GMT_rotate2D (double x[], double y[], GMT_LONG n, double x0, double y0, double angle, double xp[], double yp[]);
EXTERN_MSC void GMT_smart_justify (GMT_LONG just, double angle, double dx, double dy, double *x_shift, double *y_shift);
EXTERN_MSC void GMT_str_tolower (char *string);
EXTERN_MSC void GMT_str_toupper (char *string);
EXTERN_MSC char *GMT_convertpen (struct GMT_PEN *pen, GMT_LONG *width, GMT_LONG *offset, int rgb[]);
EXTERN_MSC void GMT_fourt (float *data, GMT_LONG *nn, GMT_LONG ndim, GMT_LONG ksign, GMT_LONG iform, float *work);
EXTERN_MSC GMT_LONG GMT_get_coordinate_label (char *string, struct GMT_PLOT_CALCLOCK *P, char *format, struct GMT_PLOT_AXIS_ITEM *T, double coord);
EXTERN_MSC GMT_LONG GMT_get_proj3D (char *line, double *az, double *el);
EXTERN_MSC GMT_LONG GMT_gap_detected (void);
EXTERN_MSC GMT_LONG GMT_alloc_memory2 (void **ptr1, void **ptr2, GMT_LONG n, GMT_LONG n_alloc, size_t element_size, char *module);
EXTERN_MSC GMT_LONG GMT_alloc_memory3 (void **ptr1, void **ptr2, void **ptr3, GMT_LONG n, GMT_LONG n_alloc, size_t element_size, char *module);
EXTERN_MSC GMT_LONG GMT_alloc_memory4 (void **ptr1, void **ptr2, void **ptr3, void **ptr4, GMT_LONG n, GMT_LONG n_alloc, size_t element_size, char *module);
EXTERN_MSC void GMT_set_meminc (GMT_LONG increment);
EXTERN_MSC void GMT_reset_meminc (void);

#ifdef DEBUG
EXTERN_MSC GMT_LONG GMT_alloc_memory_func (void **ptr, GMT_LONG n, GMT_LONG n_alloc, size_t element_size, char *module, char *fname, GMT_LONG line);
EXTERN_MSC void *GMT_memory_func (void *prev_addr, GMT_LONG nelem, size_t size, char *progname, char *fname, GMT_LONG line);
EXTERN_MSC void GMT_free_func (void *addr, char *fname, GMT_LONG line);
#else
EXTERN_MSC GMT_LONG GMT_alloc_memory (void **ptr, GMT_LONG n, GMT_LONG n_alloc, size_t element_size, char *module);
EXTERN_MSC void *GMT_memory (void *prev_addr, GMT_LONG nelem, size_t size, char *progname);
EXTERN_MSC void GMT_free (void *addr);
#endif
#ifdef GMT_QSORT
/* Need to replace OS X's qsort with one that works for 64-bit data */
EXTERN_MSC void GMT_qsort(void *a, size_t n, size_t es, int (*cmp) (const void *, const void *));
#endif

/* Backwards macro for MB-system support */
#define GMT_get_rgb24(z,rgb) GMT_get_rgb_from_z(z,rgb)

#ifdef DEBUG
#define MEM_TXT_LEN	64

struct MEMORY_ITEM {
	GMT_LONG size;	/* Size of memory allocated */
	GMT_LONG line;	/* Line number where things were initially allocated */
	void *ptr;	/* Memory pointer */
#ifdef NEW_DEBUG
	char *name;	/* File name */
	struct MEMORY_ITEM *l, *r;
#else
	char name[MEM_TXT_LEN];	/* File name */
#endif
};

struct MEMORY_TRACKER {
	GMT_LONG active;		/* Normally TRUE but can be changed to focus on just some allocations */
	GMT_LONG search;		/* Normally TRUE but can be changed to skip searching when we know we add a new item */
	GMT_LONG n_ptr;		/* Number of unique pointers to allocated memory */
	GMT_LONG n_allocated;	/* Number of items allocated by GMT_memory */
	GMT_LONG n_reallocated;	/* Number of items reallocated by GMT_memory */
	GMT_LONG n_freed;	/* Number of items freed by GMT_free */
	GMT_LONG current;	/* Memory allocated at current time */
	GMT_LONG maximum;	/* Highest memory count during execution */
	GMT_LONG largest;	/* Highest memory allocation to a single variable */
	GMT_LONG n_alloc;	/* Allocated size of memory pointer array */
#ifdef NEW_DEBUG
	struct MEMORY_ITEM *list_head, *list_tail;
#else
	struct MEMORY_ITEM *item;	/* Memory item array */
#endif
};

/* In gmt_init.c we need struct MEMORY_TRACKER GMT_mem_keeper which is then
 * initialized in GMT_begin and reported upon in GMT_end
 */

EXTERN_MSC void GMT_memtrack_init (struct MEMORY_TRACKER **M);
EXTERN_MSC void GMT_memtrack_add (struct MEMORY_TRACKER *M, char *name, GMT_LONG line, void *ptr, void *prev_ptr, GMT_LONG size);
EXTERN_MSC void GMT_memtrack_sub (struct MEMORY_TRACKER *M, char *name, GMT_LONG line, void *ptr);
EXTERN_MSC void GMT_memtrack_alloc (struct MEMORY_TRACKER *M);
EXTERN_MSC void GMT_memtrack_report (struct MEMORY_TRACKER *M);
EXTERN_MSC void GMT_memtrack_on (struct MEMORY_TRACKER *M);
EXTERN_MSC void GMT_memtrack_off (struct MEMORY_TRACKER *M);
#ifdef NEW_DEBUG
EXTERN_MSC struct MEMORY_ITEM * GMT_memtrack_find (struct MEMORY_TRACKER *M, void *addr);
#else
EXTERN_MSC GMT_LONG GMT_memtrack_find (struct MEMORY_TRACKER *M, void *ptr);
#endif
#endif

#endif /* _GMT_SUPPORT_H */
