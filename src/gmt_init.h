/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
/*
 * Include file for gmt_init.c
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#ifndef GMT_INIT_H
#define GMT_INIT_H

/* Macro to do conversion to inches with PROJ_LENGTH_UNIT as default */

#define GMT_to_inch(C,value) GMT_convert_units (C, value, C->current.setting.proj_length_unit, GMT_INCH)
#define GMT_to_points(C,value) GMT_convert_units (C, value, C->current.setting.proj_length_unit, GMT_PT)

/* Extern functions */

EXTERN_MSC struct GMT_CTRL * GMT_begin (char *session, unsigned int mode);
EXTERN_MSC void GMT_end (struct GMT_CTRL *C);
EXTERN_MSC struct GMT_CTRL * GMT_begin_module (struct GMTAPI_CTRL *API, char *mod_name, struct GMT_CTRL **Ccopy);
EXTERN_MSC void GMT_end_module (struct GMT_CTRL *C, struct GMT_CTRL *Ccopy);
EXTERN_MSC int GMT_Complete_Options (struct GMT_CTRL *C, struct GMT_OPTION *options);
EXTERN_MSC int GMT_init_time_system_structure (struct GMT_CTRL *C, struct GMT_TIME_SYSTEM *time_system);
EXTERN_MSC void GMT_init_scales (struct GMT_CTRL *C, unsigned int unit, double *fwd_scale, double *inv_scale, double *inch_to_unit, double *unit_to_inch, char *unit_name);
EXTERN_MSC int GMT_set_measure_unit (struct GMT_CTRL *C, char unit);
EXTERN_MSC char * GMT_putfill (struct GMT_CTRL *C, struct GMT_FILL *F);
EXTERN_MSC char * GMT_putcolor (struct GMT_CTRL *C, double *rgb);
EXTERN_MSC char * GMT_putrgb (struct GMT_CTRL *C, double *rgb);
EXTERN_MSC char * GMT_putcmyk (struct GMT_CTRL *C, double *cmyk);
EXTERN_MSC char * GMT_puthsv (struct GMT_CTRL *C, double *hsv);
EXTERN_MSC double GMT_convert_units (struct GMT_CTRL *C, char *value, unsigned int from_default, unsigned int target_unit);
EXTERN_MSC unsigned int GMT_check_scalingopt (struct GMT_CTRL *C, char option, char unit, char *unit_name);
EXTERN_MSC int GMT_parse_common_options (struct GMT_CTRL *C, char *list, char option, char *item);
EXTERN_MSC int GMT_default_error (struct GMT_CTRL *C, char option);
EXTERN_MSC bool GMT_get_time_system (struct GMT_CTRL *C, char *name, struct GMT_TIME_SYSTEM *time_system);
EXTERN_MSC int GMT_hash_lookup (struct GMT_CTRL *C, char *key, struct GMT_HASH *hashnode, unsigned int n, unsigned int n_hash);
EXTERN_MSC void GMT_syntax (struct GMT_CTRL *C, char option);
EXTERN_MSC void GMT_cont_syntax (struct GMT_CTRL *C, unsigned int indent, unsigned int kind);
EXTERN_MSC void GMT_GSHHS_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_mapscale_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_maprose_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_fill_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_pen_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_rgb_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_inc_syntax (struct GMT_CTRL *C, char option, bool error);
EXTERN_MSC void GMT_label_syntax (struct GMT_CTRL *C, unsigned int indent, unsigned int kind);
EXTERN_MSC void GMT_dist_syntax (struct GMT_CTRL *C, char option, char *string);
EXTERN_MSC void GMT_vector_syntax (struct GMT_CTRL *C, unsigned int mode);
EXTERN_MSC void GMT_explain_options (struct GMT_CTRL *C, char *options);
EXTERN_MSC void GMT_getdefaults (struct GMT_CTRL *C, char *this_file);
EXTERN_MSC void GMT_putdefaults (struct GMT_CTRL *C, char *this_file);
EXTERN_MSC void GMT_hash_init (struct GMT_CTRL *C, struct GMT_HASH *hashnode , char **keys, unsigned int n_hash, unsigned int n_keys);
EXTERN_MSC int GMT_getdefpath (struct GMT_CTRL *C, char get, char **path);
EXTERN_MSC void GMT_extract_label (struct GMT_CTRL *C, char *line, char *label);
EXTERN_MSC void GMT_check_lattice (struct GMT_CTRL *C, double *inc, bool *pixel, bool *active);
EXTERN_MSC int GMT_check_binary_io (struct GMT_CTRL *C, unsigned int n_req);
EXTERN_MSC char * GMT_putparameter (struct GMT_CTRL *C, char *keyword);
EXTERN_MSC void GMT_set_pad (struct GMT_CTRL *C, unsigned int npad);
EXTERN_MSC int GMT_get_ellipsoid (struct GMT_CTRL *C, char *name);
EXTERN_MSC void GMT_init_vector_param (struct GMT_CTRL *C, struct GMT_SYMBOL *S);
EXTERN_MSC int GMT_parse_vector (struct GMT_CTRL *C, char *text, struct GMT_SYMBOL *S);
EXTERN_MSC bool GMT_check_region (struct GMT_CTRL *C, double wesn[]);
EXTERN_MSC void GMT_pickdefaults (struct GMT_CTRL *C, bool lines, struct GMT_OPTION *options);
EXTERN_MSC void GMT_setdefaults (struct GMT_CTRL *C, struct GMT_OPTION *options);
EXTERN_MSC int GMT_geo_C_format (struct GMT_CTRL *C);
EXTERN_MSC int GMT_loaddefaults (struct GMT_CTRL *C, char *file);
EXTERN_MSC int GMT_parse_symbol_option (struct GMT_CTRL *C, char *text, struct GMT_SYMBOL *p, unsigned int mode, bool cmd);
EXTERN_MSC int GMT_message (struct GMT_CTRL *C, char *format, ...);
EXTERN_MSC int GMT_report_func (struct GMT_CTRL *C, unsigned int level, const char *source_line, char *format, ...);

#ifdef WIN32
EXTERN_MSC void GMT_setmode (struct GMT_CTRL *C, int direction);
#endif

#ifdef MIRONE
EXTERN_MSC struct GMT_CTRL * GMT_short_begin (int argc, char **argv);
EXTERN_MSC void GMT_end_for_mex (struct GMT_CTRL *C);
#endif

/* Inline functions */

/* Wrapper around GMT_begin_module for internally registered GMT modules */
static inline struct GMT_CTRL* GMT_begin_gmt_module (struct GMTAPI_CTRL *api_ctrl, enum Gmt_module_id module_id, struct GMT_CTRL **gmt_ctrl_copy)
{
	/* Init module with NULL-name */
	struct GMT_CTRL *gmt_ctrl = GMT_begin_module (api_ctrl, NULL, gmt_ctrl_copy);
	/* Set internal module_id */
	gmt_ctrl->init.module_id = module_id;
	/* Return GMT_CTRL */
	return gmt_ctrl;
}

#endif /* GMT_INIT_H */
