/*--------------------------------------------------------------------
 *	$Id: gmt_init.h,v 1.87 2011-03-03 21:02:50 guru Exp $
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

/*
 * Include file for gmt_init.c
 *
 * Author:	Paul Wessel
 * Date:	21-AUG-1995
 * Revised:	22-MAR-2006
 * Version:	4.1
 */

#ifndef GMT_INIT_H
#define GMT_INIT_H
EXTERN_MSC double GMT_convert_units (char *from, GMT_LONG new_format);
EXTERN_MSC GMT_LONG GMT_is_invalid_number (char *t);
EXTERN_MSC GMT_LONG GMT_begin (int argc, char **argv);
EXTERN_MSC GMT_LONG GMT_check_region (double w, double e, double s, double n);
EXTERN_MSC GMT_LONG GMT_check_scalingopt (char option, char unit, char *unit_name);
EXTERN_MSC GMT_LONG GMT_font_lookup (char *name, struct GMT_FONT *list, GMT_LONG n);
EXTERN_MSC GMT_LONG GMT_sort_options (int argc, char **argv, char *order);
EXTERN_MSC GMT_LONG GMT_parse_common_options (char *item, double *w, double *e, double *s, double *n);
EXTERN_MSC GMT_LONG GMT_get_common_args (char *item, double *w, double *e, double *s, double *n);
EXTERN_MSC GMT_LONG GMT_get_ellipsoid (char *name);
EXTERN_MSC GMT_LONG GMT_get_time_system (char *name, struct GMT_TIME_SYSTEM *time_system);
EXTERN_MSC GMT_LONG GMT_init_time_system_structure (struct GMT_TIME_SYSTEM *time_system);
EXTERN_MSC GMT_LONG GMT_get_unit (char c);
EXTERN_MSC GMT_LONG GMT_hash (char *v, GMT_LONG n_hash);
EXTERN_MSC GMT_LONG GMT_hash_lookup (char *key, struct GMT_HASH *hashnode, GMT_LONG n, GMT_LONG n_hash);
EXTERN_MSC GMT_LONG GMT_parse_J_option (char *args);
EXTERN_MSC GMT_LONG GMT_parse_R_option (char *item, double *w, double *e, double *s, double *n);
EXTERN_MSC GMT_LONG GMT_unit_lookup (GMT_LONG c);
EXTERN_MSC void GMT_begin_io ();
EXTERN_MSC void GMT_cont_syntax (GMT_LONG indent, GMT_LONG kind);
EXTERN_MSC void GMT_default_error (char option);
EXTERN_MSC void GMT_end (int argc, char **argv);
EXTERN_MSC void GMT_explain_option (char option);
EXTERN_MSC void GMT_GSHHS_syntax (char option, char *string);
EXTERN_MSC void GMT_mapscale_syntax (char option, char *string);
EXTERN_MSC void GMT_maprose_syntax (char option, char *string);
EXTERN_MSC void GMT_fill_syntax (char option, char *string);
EXTERN_MSC void GMT_getdefaults (char *this_file);
EXTERN_MSC void GMT_putdefaults (char *this_file);
EXTERN_MSC void GMT_hash_init (struct GMT_HASH *hashnode , char **keys, GMT_LONG n_hash, GMT_LONG n_keys);
EXTERN_MSC void GMT_free_hash (struct GMT_HASH *hashnode, GMT_LONG n_items);
EXTERN_MSC void GMT_inc_syntax (char option, GMT_LONG error);
EXTERN_MSC GMT_LONG GMT_init_fonts (GMT_LONG *n_fonts);
EXTERN_MSC void GMT_init_scales (GMT_LONG unit, double *fwd_scale, double *inv_scale, double *inch_to_unit, double *unit_to_inch, char *unit_name);
EXTERN_MSC void GMT_label_syntax (GMT_LONG indent, GMT_LONG kind);
EXTERN_MSC void GMT_pen_syntax (char option, char *string);
EXTERN_MSC void GMT_rgb_syntax (char option, char *string);
EXTERN_MSC void GMT_set_home (void);
EXTERN_MSC GMT_LONG GMT_set_measure_unit (char unit);
EXTERN_MSC void GMT_setdefaults (int argc, char **argv);
EXTERN_MSC void GMT_syntax (char option);
EXTERN_MSC GMT_LONG GMT_getdefpath (GMT_LONG get, char **path);
EXTERN_MSC GMT_LONG GMT_parse_symbol_option (char *text, struct GMT_SYMBOL *p, GMT_LONG mode, GMT_LONG cmd);
EXTERN_MSC void GMT_extract_label (char *line, char *label);
EXTERN_MSC GMT_LONG GMT_setparameter(char *keyword, char *value);
EXTERN_MSC void GMT_check_lattice (double *x_inc, double *y_inc, GMT_LONG *pixel, GMT_LONG *active);

#ifdef MIRONE 
EXTERN_MSC GMT_LONG GMT_short_begin (int argc, char **argv);
EXTERN_MSC void GMT_end_for_mex (int argc, char **argv);
#endif

#endif /* GMT_INIT_H */
