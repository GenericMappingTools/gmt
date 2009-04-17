/*--------------------------------------------------------------------
 *	$Id: gmt_support.h,v 1.44 2009-04-17 23:42:53 guru Exp $
 *
 *	Copyright (c) 1991-2009 by P. Wessel and W. H. F. Smith
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

EXTERN_MSC int GMT_err_pass (int err, char *file);
EXTERN_MSC void GMT_err_fail (int err, char *file);
EXTERN_MSC int GMT_parse_multisegment_header (char *header, BOOLEAN use_cpt, BOOLEAN *use_fill, struct GMT_FILL *fill, struct GMT_FILL *def_fill,  BOOLEAN *use_pen, struct GMT_PEN *pen, struct GMT_PEN *def_pen, int def_outline);
EXTERN_MSC BOOLEAN GMT_polygon_is_open (double x[], double y[], GMT_LONG n);
EXTERN_MSC double GMT_cartesian_dist (double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_dist_to_point (double lon, double lat, struct GMT_TABLE *T, int *id);
EXTERN_MSC double GMT_flatearth_dist_km (double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_flatearth_dist_meter (double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_get_annot_offset (BOOLEAN *flip, int level);
EXTERN_MSC double GMT_get_map_interval (int axis, int item);
EXTERN_MSC double GMT_getradius (char *line);
EXTERN_MSC double GMT_great_circle_dist_km (double x0, double y0, double x1, double y1);
EXTERN_MSC double GMT_great_circle_dist_meter (double x0, double y0, double x1, double y1);
EXTERN_MSC int GMT_akima (double *x, double *y, GMT_LONG nx, double *c);
EXTERN_MSC int GMT_annot_pos (double min, double max, struct GMT_PLOT_AXIS_ITEM *T, double coord[], double *pos);
EXTERN_MSC int GMT_check_rgb (int *rgb);
EXTERN_MSC int GMT_comp_double_asc (const void *p_1, const void *p_2);
EXTERN_MSC int GMT_comp_float_asc (const void *p_1, const void *p_2);
EXTERN_MSC int GMT_comp_int_asc (const void *p_1, const void *p_2);
EXTERN_MSC int GMT_contours (float *grd, struct GRD_HEADER *header, int smooth_factor, int int_scheme, int orient, int *side, int *edge, int first, double **x_array, double **y_array);
EXTERN_MSC int GMT_cspline (double *x, double *y, GMT_LONG n, double *c);
EXTERN_MSC int GMT_delaunay (double *x_in, double *y_in, int n, int **link);
EXTERN_MSC int GMT_voronoi (double *x_in, double *y_in, int n, double *we, double **x_out, double **y_out);
EXTERN_MSC int GMT_flip_justify (int justify);
EXTERN_MSC int GMT_get_dist_scale (char c, double *d_scale, int *proj_type, PFD *distance_func);
EXTERN_MSC int GMT_get_format (double interval, char *unit, char *prefix, char *format);
EXTERN_MSC int GMT_get_rgb_from_z (double value, int *rgb);
EXTERN_MSC int GMT_get_fill_from_z (double value, struct GMT_FILL *fill);
EXTERN_MSC int GMT_getfill (char *line, struct GMT_FILL *fill);
EXTERN_MSC int GMT_getinc (char *line, double *dx, double *dy);
EXTERN_MSC int GMT_getincn (char *line, double inc[], int n);
EXTERN_MSC int GMT_getpen (char *line, struct GMT_PEN *pen);
EXTERN_MSC int GMT_getrgb (char *line, int *rgb);
EXTERN_MSC void GMT_enforce_rgb_triplets (char *text, int size);
EXTERN_MSC int GMT_getrose (char *text, struct GMT_MAP_ROSE *mr);
EXTERN_MSC int GMT_getscale (char *text, struct GMT_MAP_SCALE *ms);
EXTERN_MSC int GMT_inonout_sphpol (double plon, double plat, const struct GMT_LINE_SEGMENT *P);
EXTERN_MSC int GMT_intpol (double *x, double *y, GMT_LONG n, GMT_LONG m, double *u, double *v, int mode);
EXTERN_MSC int GMT_just_decode (char *key, int def);
EXTERN_MSC int GMT_log_array (double min, double max, double delta, double **array);
EXTERN_MSC int GMT_minmaxinc_verify (double min, double max, double inc, double slop);
EXTERN_MSC int GMT_near_a_line_cartesian (double lon, double lat, struct GMT_TABLE *T, BOOLEAN return_mindist, double *dist_min, double *x_near, double *y_near);
EXTERN_MSC int GMT_near_a_line_spherical (double lon, double lat, struct GMT_TABLE *T, BOOLEAN return_mindist, double *dist_min, double *x_near, double *y_near);
EXTERN_MSC int GMT_near_a_point_spherical (double x, double y, struct GMT_TABLE *T, double dist);
EXTERN_MSC int GMT_near_a_point_cartesian (double x, double y, struct GMT_TABLE *T, double dist);
EXTERN_MSC int GMT_get_arc (double x0, double y0, double r, double dir1, double dir2, double **x, double **y);
EXTERN_MSC int GMT_non_zero_winding (double xp, double yp, double *x, double *y, GMT_LONG n_path);
EXTERN_MSC int GMT_set_cpt_path (char *cpt_file, char *table);
EXTERN_MSC int GMT_strlcmp (char *str1, char *str2);
EXTERN_MSC int GMT_strtok (const char *string, const char *sep, int *start, char *token);
EXTERN_MSC int GMT_verify_expectations (int wanted, int got, char *item);
EXTERN_MSC void GMT_RI_prepare (struct GRD_HEADER *h);
EXTERN_MSC void GMT_chop (char *string);
EXTERN_MSC char *GMT_chop_ext (char *string);
EXTERN_MSC void GMT_dump_contour (double *xx, double *yy, GMT_LONG nn, double cval, int id, BOOLEAN interior, char *file);
EXTERN_MSC void GMT_get_plot_array (void);
EXTERN_MSC void GMT_get_primary_annot (struct GMT_PLOT_AXIS *A, int *primary, int *secondary);
EXTERN_MSC void GMT_illuminate (double intensity, int *rgb);
EXTERN_MSC void GMT_init_fill (struct GMT_FILL *fill, int r, int g, int b);
EXTERN_MSC void GMT_init_pen (struct GMT_PEN *pen, double width);
EXTERN_MSC int GMT_colorname2index (char *name);
EXTERN_MSC void GMT_list_custom_symbols (void);
EXTERN_MSC int GMT_read_cpt (char *cpt_file);
EXTERN_MSC void GMT_rotate2D (double x[], double y[], GMT_LONG n, double x0, double y0, double angle, double xp[], double yp[]);
EXTERN_MSC void GMT_smart_justify (int just, double angle, double dx, double dy, double *x_shift, double *y_shift);
EXTERN_MSC void GMT_str_tolower (char *string);
EXTERN_MSC void GMT_str_toupper (char *string);
EXTERN_MSC char *GMT_convertpen (struct GMT_PEN *pen, int *width, int *offset, int rgb[]);
EXTERN_MSC void GMT_fourt (float *data, GMT_LONG *nn, int ndim, int ksign, int iform, float *work);
EXTERN_MSC int GMT_get_coordinate_label (char *string, struct GMT_PLOT_CALCLOCK *P, char *format, struct GMT_PLOT_AXIS_ITEM *T, double coord);
EXTERN_MSC int GMT_get_proj3D (char *line, double *az, double *el);
EXTERN_MSC BOOLEAN GMT_gap_detected (void);

#ifdef DEBUG
EXTERN_MSC void *GMT_memory_func (void *prev_addr, size_t nelem, size_t size, char *progname, char *fname, int line);
EXTERN_MSC void GMT_free_func (void *addr, char *fname, int line);
#else
EXTERN_MSC void *GMT_memory (void *prev_addr, size_t nelem, size_t size, char *progname);
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
	size_t size;	/* Size of memory allocated */
	int line;	/* Line number where things were initially allocated */
	void *ptr;		/* Memory pointer */
#ifdef NEW_DEBUG
	char *name;	/* File name */
	struct MEMORY_ITEM *l, *r;
#else
	char name[MEM_TXT_LEN];	/* File name */
#endif
};

struct MEMORY_TRACKER {
	BOOLEAN active;	/* Normally TRUE but can be changed to focus on just some allocations */
	BOOLEAN search;	/* Normally TRUE but can be changed to skip searching when we know we add a new item */
	GMT_LONG n_ptr;	/* Number of unique pointers to allocated memory */
	GMT_LONG n_allocated;	/* Number of items allocated by GMT_memory */
	GMT_LONG n_reallocated;	/* Number of items reallocated by GMT_memory */
	GMT_LONG n_freed;	/* Number of items freed by GMT_free */
	size_t current;	/* Memory allocated at current time */
	size_t maximum;	/* Highest memory count during execution */
	size_t largest;	/* Highest memory allocation to a single variable */
	size_t n_alloc;			/* Allocated size of memory pointer array */
#ifdef NEW_DEBUG
	struct MEMORY_ITEM *list_head, *list_tail;
#else
	struct MEMORY_ITEM *item;			/* Memory item array */
#endif
};

/* In gmt_init.c we need struct MEMORY_TRACKER GMT_mem_keeper which is then
 * initialized in GMT_begin and reported upon in GMT_end
 */

EXTERN_MSC void GMT_memtrack_init (struct MEMORY_TRACKER **M);
EXTERN_MSC void GMT_memtrack_add (struct MEMORY_TRACKER *M, char *name, int line, void *ptr, void *prev_ptr, size_t size);
EXTERN_MSC void GMT_memtrack_sub (struct MEMORY_TRACKER *M, char *name, int line, void *ptr);
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
