/*--------------------------------------------------------------------
 *      $Id$
 *
 *      Copyright (c) 1991-2018 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
 *      See LICENSE.TXT file for copying and redistribution conditions.
 *
 *      This program is free software; you can redistribute it and/or modify
 *      it under the terms of the GNU Lesser General Public License as published by
 *      the Free Software Foundation; version 3 or any later version.
 *
 *      This program is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU Lesser General Public License for more details.
 *
 *      Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/

/*
 * File with definitions that allows compatibility between GMT 5 and
 * the MB-system programs mbcontour.c, mbgrdtiff.c, and mbswath.c.
 *
 * Author:      Paul Wessel
 * Date:        11-Mar-2018
 * Version:     5 API
 */

/* Only included when compiling the MB system */
#ifndef GMT_MB_H
#define GMT_MB_H

/*  Compatibility with old lower-function/macro names use prior to GMT 5.3.0 */
#if GMT_MAJOR_VERSION == 5 && GMT_MINOR_VERSION < 3
#define gmt_M_180_range GMT_180_RANGE
#define gmt_M_free_options GMT_Free_Options
#define gmt_M_ijp GMT_IJP
#define gmt_M_ijpgi GMT_IJPGI
#define gmt_M_check_condition GMT_check_condition
#define gmt_M_get_inc GMT_get_inc
#define gmt_M_get_n GMT_get_n
#define gmt_M_grd_is_global GMT_grd_is_global
#define gmt_M_grd_same_region GMT_grd_same_region
#define gmt_M_is255 GMT_is255
#define gmt_M_is_geographic GMT_is_geographic
#define gmt_M_memcpy GMT_memcpy
#define gmt_M_rgb_copy GMT_rgb_copy
#define gmt_M_u255 GMT_u255
#define gmt_M_err_fail GMT_err_fail
#define gmt_M_free GMT_free
#define gmt_M_is_fnan GMT_is_fnan
#define gmt_M_memory GMT_memory
#define gmt_get_cpt GMT_Get_CPT
#define gmt_access GMT_access
#define gmt_begin_module GMT_begin_module
#define gmt_check_filearg GMT_check_filearg
#define gmt_default_error GMT_default_error
#define gmt_end_module GMT_end_module
#define gmt_geo_to_xy GMT_geo_to_xy
#define gmt_get_api_ptr GMT_get_API_ptr
#define gmt_get_rgb_from_z GMT_get_rgb_from_z
#define gmt_getrgb GMT_getrgb
#define gmt_grd_project GMT_grd_project
#define gmt_grd_setregion GMT_grd_setregion
#define gmt_illuminate GMT_illuminate
#define gmt_map_basemap GMT_map_basemap
#define gmt_map_clip_off GMT_map_clip_off
#define gmt_map_clip_on GMT_map_clip_on
#define gmt_map_setup GMT_map_setup
#define gmt_not_numeric GMT_not_numeric
#define gmt_plane_perspective GMT_plane_perspective
#define gmt_plotcanvas GMT_plotcanvas
#define gmt_plotend GMT_plotend
#define gmt_plotinit GMT_plotinit
#define gmt_project_init GMT_project_init
#define gmt_putrgb GMT_putrgb
#define gmt_rgb_syntax GMT_rgb_syntax
#define gmt_set_grddim GMT_set_grddim
#define gmt_show_name_and_purpose GMT_show_name_and_purpose
#endif

#endif /* GMT_MB_H */
