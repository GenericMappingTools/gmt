/*--------------------------------------------------------------------
 *	$Id: gmt_plot.c,v 1.344 2011-06-09 16:07:14 remko Exp $
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
/*
 *
 *			G M T _ P L O T . C
 *
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * GMT_plot.c contains code related to plotting maps.  These functions requires
 * we pass both the GMT and PSL control strucure pointers.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5
 *
 *
 * PUBLIC Functions include:
 *
 *	GMT_draw_map_scale :	Plot map scale
 *	GMT_draw_front :	Draw a front line
 *	gmt_echo_command :	Puts the command line into the PostScript file as comments
 *	GMT_geo_line :		Plots line in lon/lat on maps, takes care of periodicity jumps
 *	GMT_geo_ellipse :	Plots ellipse in lon/lat on maps, takes care of periodicity jumps
 *	GMT_geo_polygons :	Plots polygon in lon/lat on maps, takes care of periodicity jumps
 *	GMT_geo_rectangle :	Plots rectangle in lat/lon on maps, takes care of periodicity jumps
 *	GMT_map_basemap :	Generic basemap function
 *	GMT_map_clip_off :	Deactivate map region clip path
 *	GMT_map_clip_on :	Activate map region clip path
 *	GMT_plane_perspective : Adds PS matrix to simulate perspective plotting
 *	GMT_plot_line :		Plots path (in projected coordinates), takes care of boundary crossings
 *	gmt_timestamp :		Plot UNIX time stamp with optional string
 *	GMT_vector :		Draw 2/3-D vector
 *	GMT_vertical_axis :	Draw 3-D vertical axes
 *	GMT_xy_axis :		Draw x or y axis
 *
 * INTERNAL Functions include:
 *
 *	gmt_conic_map_boundary :	Plot basemap for conic projections
 *	gmt_linear_map_boundary :	Plot basemap for Linear projections
 *	GMT_linearx_grid :		Draw linear x grid lines
 *	gmt_lineary_grid :		Draw linear y grid lines
 *	gmt_logx_grid :			Draw log10 x grid lines
 *	gmt_logy_grid :			Draw log10 y grid lines
 *	gmt_powx_grid :			Draw power x grid lines
 *	gmt_powy_grid :			Draw power y grid lines
 *	gmt_map_annotate :		Annotate basemaps
 *	gmt_map_boundary :		Draw the maps boundary
 *	gmt_map_gridcross :		Draw grid crosses on maps
 *	gmt_map_gridlines :		Draw gridlines on maps
 *	gmt_map_latline :		Draw a latitude line
 *	gmt_map_lattick :		Draw a latitude tick mark
 *	gmt_map_lonline :		Draw a longitude line
 *	gmt_map_lontick :		Draw a longitude tick mark
 *	gmt_map_symbol :		Plot map annotation
 *	gmt_map_symbol_ew :		  for east/west sides
 *	gmt_map_symbol_ns :		  for south/north sides
 *	gmt_map_tick :			Draw the ticks
 *	gmt_map_tickmarks :		Plot tickmarks on maps
 *	gmt_fancy_map_boundary :	Plot basemap for Mercator projection
 *	gmt_ellipse_map_boundary :	Plot basemap for Mollweide and Hammer-Aitoff projections
 *	gmt_oblmrc_map_boundary :	Plot basemap for Oblique Mercator projection
 *	gmt_polar_map_boundary :	Plot basemap for Polar stereographic projection
 *	gmt_rect_map_boundary :		Plot plain basemap for projections with rectangular boundaries
 *	gmt_basic_map_boundary :	Plot plain basemap for most projections
 *	gmt_wesn_map_boundary :		Plot plain basemap for projections with geographical boundaries
 *	gmt_theta_r_map_boundary :	Plot plain basemap for polar (cylindrical) projection
 */

#include "pslib.h"
#include "gmt.h"
#include "gmt_internals.h"

EXTERN_MSC double GMT_great_circle_dist_degree (struct GMT_CTRL *C, double lon1, double lat1, double lon2, double lat2);
EXTERN_MSC GMT_LONG gmt_load_custom_annot (struct GMT_CTRL *C, struct GMT_PLOT_AXIS *A, char item, double **xx, char ***labels);

#define GMT_ELLIPSE_APPROX 72

/* Local variables to this file */

GMT_LONG GMT_n_annotations[4] = {0, 0, 0, 0};
GMT_LONG GMT_alloc_annotations[4] = {0, 0, 0, 0};
double *GMT_x_annotation[4], *GMT_y_annotation[4];

/* Get bitmapped 600 dpi GMT glyph for timestamp.  The glyph is a 90 x 220 pixel 1-bit image
   and it is here represented as ceil (220 / 8) * 90 = 2520 bytes */


unsigned char GMT_glyph[2520] = {
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x7f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0x80, 0x00,
0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x3f, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f,
0xff, 0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff,
0xe0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, 0xff,
0xc0, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xfc, 0x00, 0x00, 0x01,
0xff, 0xc0, 0x00, 0x00, 0x01, 0xff, 0xe0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x00,
0x01, 0xff, 0xf0, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfe, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x00, 0x0f, 0xff, 0xff, 0xff, 0x80, 0x00, 0x03, 0xff, 0xe0, 0x00, 0x00, 0x00, 0xff, 0xf8, 0x7f,
0xff, 0xff, 0xff, 0xff, 0xff, 0x80, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff,
0xff, 0xc0, 0x00, 0x03, 0xff, 0xf0, 0x00, 0x00, 0x00, 0xff, 0xfc, 0x3f, 0xff, 0xff, 0xff, 0xff,
0xff, 0xe0, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x00, 0x03,
0xff, 0xf0, 0x00, 0x00, 0x00, 0xff, 0xfc, 0x1f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x00,
0x00, 0xff, 0xfe, 0x0f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x1f, 0xff, 0xfc, 0x3f, 0xff, 0xe0, 0x00, 0x07, 0xff, 0xf8, 0x00, 0x00, 0x00, 0xff, 0xff, 0x07,
0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0x80, 0x07,
0xff, 0xf0, 0x00, 0x07, 0xff, 0xfc, 0x00, 0x00, 0x00, 0xff, 0xff, 0x07, 0x80, 0x00, 0xff, 0xfc,
0x00, 0x0f, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x01, 0xff, 0xfc, 0x00, 0x01, 0xff, 0xf8, 0x00, 0x07,
0xff, 0xfc, 0x00, 0x00, 0x00, 0xff, 0xff, 0x80, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x07, 0xff, 0xe0, 0x00, 0x00, 0x7f, 0xf8, 0x00, 0x07, 0xff, 0xfc, 0x00, 0x00,
0x00, 0xff, 0xff, 0xc0, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x1f,
0xff, 0x00, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x0f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0xff, 0xff, 0xc0,
0x00, 0x00, 0x07, 0xff, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x7f, 0xfc, 0x00, 0x00, 0x00,
0x3f, 0xf8, 0x00, 0x0f, 0xff, 0xfe, 0x00, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x03, 0xff,
0x80, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0xff, 0xf8, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x0f,
0xff, 0xff, 0x00, 0x00, 0x00, 0xff, 0xff, 0xe0, 0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x03, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x0f, 0xff, 0xff, 0x00, 0x00,
0x00, 0xff, 0xbf, 0xf0, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x07, 0xff,
0x80, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x0f, 0xfd, 0xff, 0x80, 0x00, 0x01, 0xff, 0xbf, 0xf0,
0x00, 0x00, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0xff, 0x00, 0x00, 0x00, 0x00,
0x1f, 0xf8, 0x00, 0x0f, 0xfd, 0xff, 0x80, 0x00, 0x01, 0xff, 0x9f, 0xf8, 0x00, 0x00, 0x00, 0x7f,
0xf0, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x0f,
0xfd, 0xff, 0x80, 0x00, 0x01, 0xff, 0x9f, 0xf8, 0x00, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x1f, 0xfc, 0xff, 0xc0, 0x00,
0x01, 0xff, 0x9f, 0xf8, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x7f, 0xf0,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0xff, 0xc0, 0x00, 0x01, 0xff, 0x8f, 0xfc,
0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0x0f, 0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x1f, 0xf8, 0xff, 0xe0, 0x00, 0x01, 0xff, 0x8f, 0xfc, 0x00, 0x00, 0x00, 0x0f,
0xfc, 0x00, 0x00, 0x0f, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f,
0xf8, 0x7f, 0xe0, 0x00, 0x03, 0xff, 0x8f, 0xfc, 0x00, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x0f,
0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x7f, 0xe0, 0x00,
0x03, 0xff, 0x8f, 0xfc, 0x00, 0x00, 0x00, 0x07, 0xfe, 0x00, 0x00, 0x0f, 0x00, 0x03, 0xff, 0x80,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x7f, 0xf0, 0x00, 0x03, 0xff, 0x0f, 0xfe,
0x00, 0x00, 0x00, 0x07, 0xff, 0x00, 0x00, 0x0f, 0x00, 0x07, 0xff, 0x80, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x1f, 0xf8, 0x3f, 0xf0, 0x00, 0x03, 0xff, 0x07, 0xfe, 0x00, 0x00, 0x00, 0x07,
0xff, 0x00, 0x00, 0x0f, 0x00, 0x07, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f,
0xf8, 0x3f, 0xf8, 0x00, 0x07, 0xff, 0x07, 0xfe, 0x00, 0x00, 0x00, 0x07, 0xff, 0x00, 0x00, 0x0f,
0x00, 0x07, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x1f, 0xf8, 0x3f, 0xf8, 0x00,
0x07, 0xff, 0x07, 0xfe, 0x00, 0x00, 0x00, 0x03, 0xff, 0x00, 0x00, 0x0f, 0x00, 0x07, 0xff, 0x00,
0x00, 0x03, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x1f, 0xf8, 0x1f, 0xf8, 0x00, 0x07, 0xfe, 0x07, 0xfe,
0x00, 0x00, 0x00, 0x03, 0xff, 0x00, 0x00, 0x0f, 0x00, 0x07, 0xff, 0x00, 0x00, 0x03, 0xff, 0xff,
0xff, 0xf8, 0x00, 0x1f, 0xf8, 0x1f, 0xfc, 0x00, 0x0f, 0xfe, 0x07, 0xfe, 0x00, 0x00, 0x00, 0x03,
0xff, 0x00, 0x00, 0x0f, 0x00, 0x07, 0xff, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x1f,
0xf8, 0x1f, 0xfc, 0x00, 0x0f, 0xfe, 0x07, 0xfe, 0x00, 0x00, 0x00, 0x03, 0xff, 0x00, 0x00, 0x0f,
0x00, 0x07, 0xff, 0x00, 0x00, 0x03, 0xff, 0xff, 0xff, 0xf8, 0x00, 0x1f, 0xf8, 0x0f, 0xfc, 0x00,
0x1f, 0xfc, 0x07, 0xfe, 0x00, 0x00, 0x00, 0x03, 0xff, 0x00, 0x00, 0x0f, 0x00, 0x07, 0xff, 0x00,
0x00, 0x03, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x1f, 0xf8, 0x0f, 0xfe, 0x00, 0x1f, 0xfc, 0x07, 0xfe,
0x00, 0x00, 0x00, 0x03, 0xff, 0x00, 0x00, 0x0f, 0x00, 0x07, 0xff, 0x00, 0x00, 0x03, 0xff, 0xff,
0xff, 0xfc, 0x00, 0x1f, 0xf8, 0x0f, 0xfe, 0x00, 0x1f, 0xf8, 0x07, 0xfe, 0x00, 0x00, 0x00, 0x07,
0xff, 0x00, 0x00, 0x0f, 0x00, 0x03, 0xff, 0x80, 0x00, 0x03, 0xff, 0xff, 0xff, 0xfc, 0x00, 0x1f,
0xf8, 0x07, 0xfe, 0x00, 0x3f, 0xf8, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x07, 0xff, 0x00, 0x00, 0x0f,
0x00, 0x03, 0xff, 0x80, 0x00, 0x00, 0x03, 0xf8, 0x0f, 0xfc, 0x00, 0x1f, 0xf8, 0x07, 0xff, 0x00,
0x3f, 0xf0, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x07, 0xfe, 0x00, 0x00, 0x0f, 0x00, 0x03, 0xff, 0x80,
0x00, 0x00, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x1f, 0xf8, 0x07, 0xff, 0x00, 0x7f, 0xf0, 0x0f, 0xfc,
0x00, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x0f, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x00,
0x0f, 0xfc, 0x00, 0x1f, 0xf8, 0x03, 0xff, 0x00, 0x7f, 0xe0, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x0f,
0xfe, 0x00, 0x00, 0x0f, 0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x1f,
0xf8, 0x03, 0xff, 0x80, 0xff, 0xe0, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x1f, 0xfc, 0x03, 0xff, 0x80,
0xff, 0xc0, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x1f, 0xfc, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x3f, 0xf0,
0x00, 0x00, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x1f, 0xfc, 0x01, 0xff, 0x81, 0xff, 0xc0, 0x1f, 0xf8,
0x00, 0x00, 0x00, 0x3f, 0xf8, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00,
0x07, 0xfe, 0x00, 0x0f, 0xfc, 0x01, 0xff, 0x81, 0xff, 0x80, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x3f,
0xf0, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x00, 0x07, 0xfe, 0x00, 0x0f,
0xfc, 0x00, 0xff, 0xc3, 0xff, 0x80, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x07, 0xff, 0x00, 0x00, 0x00, 0x00, 0x07, 0xff, 0x00, 0x0f, 0xfc, 0x00, 0xff, 0xc3,
0xff, 0x00, 0x3f, 0xf0, 0x00, 0x00, 0x00, 0xff, 0xe0, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x03, 0xff,
0xc0, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x00, 0x0f, 0xfc, 0x00, 0xff, 0xc7, 0xfe, 0x00, 0x3f, 0xe0,
0x00, 0x00, 0x01, 0xff, 0xc0, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x01, 0xff, 0xe0, 0x00, 0x00, 0x00,
0x0f, 0xff, 0x00, 0x0f, 0xfc, 0x00, 0x7f, 0xcf, 0xfe, 0x00, 0x7f, 0xe0, 0x00, 0x00, 0x03, 0xff,
0x80, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x7f, 0xf8, 0x00, 0x00, 0x00, 0x0f, 0xff, 0x80, 0x0f,
0xfc, 0x00, 0x7f, 0xef, 0xfc, 0x00, 0x7f, 0xc0, 0x00, 0x00, 0x07, 0xff, 0x00, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x1f, 0xfe, 0x00, 0x00, 0x00, 0x1f, 0xff, 0x80, 0x07, 0xfc, 0x00, 0x7f, 0xff,
0xf8, 0x00, 0xff, 0xc0, 0x00, 0x00, 0x0f, 0xfe, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x0f,
0xff, 0x80, 0x00, 0x00, 0x1f, 0xff, 0x80, 0x07, 0xfc, 0x00, 0x7f, 0xff, 0xf0, 0x00, 0xff, 0x80,
0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x03, 0xff, 0xf0, 0x00, 0x00,
0x3f, 0xff, 0xc0, 0x07, 0xfc, 0x00, 0x3f, 0xff, 0xf0, 0x01, 0xff, 0x80, 0x00, 0x00, 0x3f, 0xf8,
0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0xff, 0xfe, 0x00, 0x00, 0xff, 0xff, 0xc0, 0x07,
0xfc, 0x00, 0x3f, 0xff, 0xe0, 0x01, 0xff, 0x00, 0x00, 0x00, 0x7f, 0xf0, 0x00, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xe0, 0x03, 0xff, 0xff, 0xe0, 0x07, 0xfc, 0x00, 0x3f, 0xff,
0xc0, 0x03, 0xfe, 0x00, 0x00, 0x00, 0xff, 0xc0, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x0f, 0xff, 0xff, 0xff, 0xff, 0xdf, 0xe0, 0x03, 0xfe, 0x00, 0x1f, 0xff, 0x80, 0x07, 0xfe, 0x00,
0x00, 0x01, 0xff, 0x80, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x01, 0xff, 0xff, 0xff,
0xff, 0xcf, 0xf0, 0x03, 0xfe, 0x00, 0x1f, 0xff, 0x00, 0x07, 0xfc, 0x00, 0x00, 0x03, 0xff, 0x00,
0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x7f, 0xff, 0xff, 0xff, 0xc7, 0xf0, 0x03,
0xfe, 0x00, 0x1f, 0xff, 0x00, 0x0f, 0xf8, 0x00, 0x00, 0x0f, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0xff, 0xff, 0xff, 0x83, 0xf0, 0x03, 0xfe, 0x00, 0x0f, 0xfe,
0x00, 0x1f, 0xf0, 0x00, 0x00, 0x1f, 0xf8, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x00, 0x01, 0xff, 0xff, 0xff, 0x81, 0xf8, 0x01, 0xfe, 0x00, 0x0f, 0xfc, 0x00, 0x1f, 0xe0, 0x00,
0x00, 0x3f, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff,
0xff, 0x00, 0xf8, 0x01, 0xfe, 0x00, 0x0f, 0xf8, 0x00, 0x3f, 0xe0, 0x00, 0x00, 0x7f, 0xc0, 0x00,
0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0xff, 0xfc, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x07, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0f,
};

/*	GMT_LINEAR PROJECTION MAP BOUNDARY	*/

void gmt_define_baselines (struct PSL_CTRL *P)
{
	/* Given the sizes and lengths of text, offsets, and ticks, calculate the offset from the
	 * axis to the baseline of each annotation class:
	 * PSL_A0_y:	Base of primary annotation
	 * PSL_A1_y:	Base of secondary annotation
	 * PSL_L_y:	Base of axis label
	 * PSL_H_y:	Base of plot title (set elsewhere)
	 */

	PSL_command (P, "/PSL_A0_y PSL_AO0 PSL_TL1 PSL_AO0 mul 0 gt {PSL_TL1 add} if PSL_AO0 0 gt {PSL_far {PSL_AH0} {0} ifelse add} if PSL_sign mul def\n");
	PSL_command (P, "/PSL_A1_y PSL_A0_y abs PSL_AO1 add PSL_far {PSL_AH1} {PSL_AH0} ifelse add PSL_sign mul def\n");
	PSL_command (P, "/PSL_L_y PSL_A1_y abs PSL_LO add PSL_far {PSL_LH} {PSL_AH1} ifelse add PSL_sign mul def\n");
}

void gmt_define_PS_items (struct GMT_CTRL *C, struct PSL_CTRL *P, GMT_LONG axis, GMT_LONG below, GMT_LONG ortho)
{
	/* GMT relies on the PostScript engine to calculate dimensions of
	 * text items.  Therefore, the calculation of text baseline offsets
	 * (i.e., distance from axis to base of annotations) becomes a bit
	 * tricky and must be done in PostScript language.
	 *
	 * Here we set PostScript definitions of various lengths and font
	 * sizes used in the frame annotation machinery */

	PSL_command (P, "/PSL_sign %d def\n", below ? -1 : 1); /* Annotate below/left or above/right */
	PSL_command (P, "/PSL_far %s def\n", below == (axis == GMT_X && !ortho) ? "true" : "false");		/* Current point is at the far side of the tickmark? */
	PSL_defunits (P, "PSL_TL1", C->current.setting.map_tick_length);	/* Length of tickmark (could be negative) */
	PSL_defunits (P, "PSL_AO0", C->current.setting.map_annot_offset[0]);	/* Offset between tick and primary annotation */
	PSL_defunits (P, "PSL_AO1", C->current.setting.map_annot_offset[1]);	/* Offset between tick and secondary annotation */
	PSL_defunits (P, "PSL_LO",  C->current.setting.map_label_offset);	/* Offset between annotation and label */
	PSL_defpoints (P, "PSL_AF0", C->current.setting.font_annot[0].size);	/* Primary font size */
	PSL_defpoints (P, "PSL_AF1", C->current.setting.font_annot[1].size);	/* Secondary font size */
	PSL_defpoints (P, "PSL_LF",  C->current.setting.font_label.size);	/* Label font size */
	PSL_command (P, "/PSL_AH0 0 def\n");	/* The loop over level 0 annotations will reset this to the tallest annotation */
	PSL_command (P, "/PSL_AH1 0 def\n");	/* The loop over level 1 annotations will reset this to the tallest annotation */
	PSL_setfont (P, C->current.setting.font_label.id);
	PSL_command (P, "/PSL_LH ");
	PSL_deftextdim (P, "-h", C->current.setting.font_label.size, "M");
	PSL_command (P, "def\n");
}

void gmt_linear_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	GMT_LONG form, cap = P->internal.line_cap;
	double x_length, y_length;

	x_length = C->current.proj.rect[XHI] - C->current.proj.rect[XLO];
	y_length = C->current.proj.rect[YHI] - C->current.proj.rect[YLO];

	/* Temporarily change to square cap so rectangular frames have neat corners */
	PSL_setlinecap (P, PSL_SQUARE_CAP);

	if (C->current.map.frame.side[W_SIDE]) GMT_xy_axis (C, C->current.proj.rect[XLO], C->current.proj.rect[YLO], y_length, s, n,
		&C->current.map.frame.axis[GMT_Y], TRUE,  C->current.map.frame.side[W_SIDE] & 2);	/* West or left y-axis */
	if (C->current.map.frame.side[E_SIDE]) GMT_xy_axis (C, C->current.proj.rect[XHI], C->current.proj.rect[YLO], y_length, s, n,
		&C->current.map.frame.axis[GMT_Y], FALSE, C->current.map.frame.side[E_SIDE] & 2);	/* East or right y-axis */
	if (C->current.map.frame.side[S_SIDE]) GMT_xy_axis (C, C->current.proj.rect[XLO], C->current.proj.rect[YLO], x_length, w, e,
		&C->current.map.frame.axis[GMT_X], TRUE,  C->current.map.frame.side[S_SIDE] & 2);	/* South or lower x-axis */
	if (C->current.map.frame.side[N_SIDE]) GMT_xy_axis (C, C->current.proj.rect[XLO], C->current.proj.rect[YHI], x_length, w, e,
		&C->current.map.frame.axis[GMT_X], FALSE, C->current.map.frame.side[N_SIDE] & 2);	/* North or upper x-axis */

	PSL_setlinecap (P, cap);	/* Reset back to default */
	if (!C->current.map.frame.header[0] || C->current.map.frame.plotted_header) return;	/* No header today */

	PSL_comment (P, "Placing plot title\n");

	gmt_define_PS_items (C, P, GMT_X, FALSE, FALSE);

	gmt_define_baselines (P);

	if (C->current.map.frame.side[N_SIDE] == 0)
		PSL_defunits (P, "PSL_H_y", C->current.setting.map_title_offset);	/* Just axis line is drawn, offset by map_title_offset only */
	else if (C->current.map.frame.side[N_SIDE] & 1)
		PSL_command (P, "/PSL_H_y PSL_L_y %ld add def\n", psl_iz (P, C->current.setting.map_title_offset));	/* Allow for ticks + offset */
	else
		PSL_command (P, "/PSL_H_y PSL_L_y PSL_LH add %ld add def\n", psl_iz (P, C->current.setting.map_title_offset));	/* For title adjustment */

	PSL_command (P, "%ld %ld PSL_H_y add M\n", psl_iz (P, 0.5 * x_length), psl_iz (P, y_length));
	form = GMT_setfont (C, &C->current.setting.font_title);
	PSL_plottext (P, 0.0, 0.0, -C->current.setting.font_title.size, C->current.map.frame.header, 0.0, PSL_BC, form);
	C->current.map.frame.plotted_header = TRUE;
}

GMT_LONG gmt_skip_second_annot (GMT_LONG item, double x, double x2[], GMT_LONG n, GMT_LONG primary, GMT_LONG secondary)
{
	GMT_LONG i, found;
	double small;

	if (primary == secondary) return (FALSE);	/* Not set, no need to skip */
	if (secondary != item) return (FALSE);		/* Not working on secondary annotation */
	if (!x2) return (FALSE);			/* None given */

	small = (x2[1] - x2[0]) * GMT_SMALL;
	for (i = 0, found = FALSE; !found && i < n; i++) found = (fabs (x2[i] - x) < small);
	return (found);
}

void GMT_xy_axis (struct GMT_CTRL *C, double x0, double y0, double length, double val0, double val1, struct GMT_PLOT_AXIS *A, GMT_LONG below, GMT_LONG annotate)
{
	GMT_LONG k, i, nx, np = 0;	/* Misc. variables */
	GMT_LONG annot_pos;		/* Either 0 for upper annotation or 1 for lower annotation */
	GMT_LONG primary = 0;		/* Axis item number of annotation with largest interval/unit */
	GMT_LONG secondary = 0;		/* Axis item number of annotation with smallest interval/unit */
	GMT_LONG axis;			/* Axis id (0 = x, 1 = y) */
	GMT_LONG is_interval;		/* TRUE when the annotation is interval annotation and not tick annotation */
	GMT_LONG check_annotation;	/* TRUE is we have two levels of tick annotations that can overprint */
	GMT_LONG do_annot;		/* TRUE unless we are dealing with Gregorian weeks */
	GMT_LONG do_tick;		/* TRUE unless we are dealing with bits of weeks */
	GMT_LONG form;			/* TRUE for outline font */
	GMT_LONG ortho = FALSE;	/* TRUE if annotations are orthogonal to axes */
	GMT_LONG need_txt = FALSE;		/* TRUE if we are annotating anything on this axis */
	double *knots = NULL, *knots_p = NULL;	/* Array pointers with tick/annotation knots, the latter for primary annotations */
	double tick_len[6];		/* Ticklengths for each of the 6 axis items */
	double x, sign, len, t_use;	/* Misc. variables */
	struct GMT_FONT font;			/* Annotation font (FONT_ANNOT_PRIMARY or FONT_ANNOT_SECONDARY) */
	struct GMT_PLOT_AXIS_ITEM *T = NULL;	/* Pointer to the current axis item */
	char string[GMT_CALSTRING_LENGTH];	/* Annotation string */
	char format[GMT_TEXT_LEN256];		/* format used for non-time annotations */
	char *axis_chr[3] = {"ns", "ew", "zz"};	/* Characters corresponding to axes */
	char **label_c = NULL;
	PFD xyz_fwd = NULL;
	struct PSL_CTRL *P = C->PSL;

	/* Initialize parameters for this axis */

	axis = A->id;
	xyz_fwd = (PFD) ((axis == GMT_X) ? GMT_x_to_xx : ((axis == GMT_Y) ? GMT_y_to_yy : GMT_z_to_zz));
	if ((check_annotation = GMT_two_annot_items (C, axis))) {	/* Must worry about annotation overlap */
		GMT_get_primary_annot (C, A, &primary, &secondary);			/* Find primary and secondary axis items */
		np = GMT_coordinate_array (C, val0, val1, &A->item[primary], &knots_p, NULL);	/* Get all the primary tick annotation knots */
	}
	len = MAX (0.0, C->current.setting.map_tick_length);				/* Tick length if directed outward */
	sign = (below) ? -1.0 : 1.0;							/* since annotations go either below or above */
	tick_len[0] = tick_len[2] = sign * C->current.setting.map_tick_length;		/* Initialize the tick lengths */
	tick_len[1] = 3.0 * sign * C->current.setting.map_tick_length;
	tick_len[3] = (A->item[GMT_ANNOT_UPPER].active) ? tick_len[0] : 3.0 * sign * C->current.setting.map_tick_length;
	tick_len[4] = 0.5 * sign * C->current.setting.map_tick_length;
	tick_len[5] = 0.75 * sign * C->current.setting.map_tick_length;
	if (A->type != GMT_TIME) GMT_get_format (C, GMT_get_map_interval (C, axis, GMT_ANNOT_UPPER), A->unit, A->prefix, format);	/* Set the annotation format template */
	if (strchr (C->current.setting.map_annot_ortho, axis_chr[axis][below])) ortho = TRUE;

	/* Ready to draw axis */

	if (axis == GMT_X)
		PSL_comment (P, below ? "Start of lower x-axis\n" : "Start of upper x-axis\n");
	else if (axis == GMT_Y)
		PSL_comment (P, below ? "Start of left y-axis\n" : "Start of right y-axis\n");
	else
		PSL_comment (P, below ? "Start of front z-axis\n" : "Start of back z-axis\n");
	PSL_setorigin (P, x0, y0, 0.0, PSL_FWD);

	if (annotate && !GMT_axis_is_geo (C, axis)) {
		for (need_txt = A->label[0], k = 0; !need_txt && k < GMT_TICK_UPPER; k++) if (A->item[k].active) need_txt = TRUE;
	}
	
	if (need_txt) gmt_define_PS_items (C, P, axis, below, ortho);	/* Create PostScript definitions of various lengths and font sizes */

	PSL_comment (P, "Axis tick marks and annotations\n");
	GMT_setpen (C, &C->current.setting.map_frame_pen);
	if (axis == GMT_X)
		PSL_plotsegment (P, 0.0, 0.0, length, 0.0);
	else
		PSL_plotsegment (P, 0.0, length, 0.0, 0.0);
	
	if (C->current.setting.map_frame_type == GMT_IS_GRAPH) {	/* Extend axis 7.5% with an arrow */
		struct GMT_FILL arrow;
		double vector_width, dim[7];
		GMT_init_fill (C, &arrow, C->current.setting.map_frame_pen.rgb[0], C->current.setting.map_frame_pen.rgb[1], C->current.setting.map_frame_pen.rgb[2]);
		GMT_setfill (C, &arrow, FALSE);
		vector_width = rint (PSL_DOTS_PER_INCH * C->current.setting.map_frame_pen.width / PSL_POINTS_PER_INCH) / PSL_DOTS_PER_INCH;	/* Round off vector width same way as pen width */
		dim[2] = vector_width; dim[3] = 10.0 * vector_width; dim[4] = 5.0 * vector_width;
		dim[5] = C->current.setting.map_vector_shape; dim[6] = 0.0;
		if (axis == GMT_X) {
			dim[0] = 1.075 * length; dim[1] = 0.0;
			PSL_plotsymbol (P, length, 0.0, dim, PSL_VECTOR);
		}
		else {
			dim[0] = 0.0; dim[1] = 1.075 * length;
			PSL_plotsymbol (P, 0.0, length, dim, PSL_VECTOR);
		}
	}

	GMT_setpen (C, &C->current.setting.map_tick_pen);

	for (k = 0; k < GMT_GRID_UPPER; k++) {	/* For each one of the 6 axis items (gridlines are done separately) */

		T = &A->item[k];		/* Get pointer to this item */
		if (!T->active) continue;	/* Do not want this item plotted - go to next item */

		is_interval = GMT_interval_axis_item (k);	/* Interval or tick mark annotation? */
		nx = GMT_coordinate_array (C, val0, val1, &A->item[k], &knots, &label_c);	/* Get all the annotation tick knots */

		/* First plot all the tick marks */

		do_tick = !((T->unit == 'K' || T->unit == 'k') && T->interval > 1 && fmod (T->interval, 7.0) > 0.0);
		for (i = 0; do_tick && i < nx; i++) {
			if (knots[i] < (val0 - GMT_CONV_LIMIT) || knots[i] > (val1 + GMT_CONV_LIMIT)) continue;	/* Outside the range */
			x = (*xyz_fwd) (C, knots[i]);	/* Convert to inches on the page */
			if (axis == GMT_X)
				PSL_plotsegment (P, x, 0.0, x, tick_len[k]);
			else
				PSL_plotsegment (P, 0.0, x, tick_len[k], x);
		}

		do_annot = (nx && k < GMT_TICK_UPPER && annotate && !GMT_axis_is_geo (C, axis) && !(T->unit == 'r'));	/* Cannot annotate a Gregorian week */
		if (do_annot) {	/* Then do annotations too - here just set text height/width parameters in PostScript */

			annot_pos = GMT_lower_axis_item (k);					/* 1 means lower annotation, 0 means upper (close to axis) */
			font = C->current.setting.font_annot[annot_pos];			/* Set the font to use */
			PSL_setfont (P, font.id);

			PSL_command (P, "/PSL_AH%ld 0\n", annot_pos);
			for (i = 0; i < nx - is_interval; i++) {
				if (GMT_annot_pos (C, val0, val1, T, &knots[i], &t_use)) continue;			/* Outside range */
				if (gmt_skip_second_annot (k, knots[i], knots_p, np, primary, secondary)) continue;	/* Secondary annotation skipped when coinciding with primary annotation */
				if (label_c && label_c[i] && label_c[i][0])
					strcpy (string, label_c[i]);
				else
					GMT_get_coordinate_label (C, string, &C->current.plot.calclock, format, T, knots[i]);	/* Get annotation string */
				PSL_deftextdim (P, ortho ? "-w" : "-h", font.size, string);
				PSL_command (P, "mx\n");		/* Update the longest annotation */
			}
			PSL_command (P, "def\n");
		}

		if (nx) GMT_free (C, knots);
		if (label_c) {
			for (i = 0; i < nx; i++) if (label_c[i]) free ((void *)label_c[i]);
			GMT_free (C, label_c);
		}
	}
	
	/* Here, PSL_AH0, PSL_AH1, and PSL_LH have been defined.  We may now set the y offsets for text baselines */

	if (need_txt) gmt_define_baselines (P);

	/* Now do annotations, if requested */

	form = GMT_setfont (C, &C->current.setting.font_annot[0]);
	for (k = 0; annotate && !GMT_axis_is_geo (C, axis) && k < GMT_TICK_UPPER; k++) {	/* For each one of the 6 axis items (gridlines are done separately) */

		T = &A->item[k];					/* Get pointer to this item */
		if (!T->active) continue;				/* Do not want this item plotted - go to next item */
		if (T->unit == 'r') continue;				/* Cannot annotate a Gregorian week */

		is_interval = GMT_interval_axis_item(k);		/* Interval or tick mark annotation? */
		nx = GMT_coordinate_array (C, val0, val1, &A->item[k], &knots, &label_c);	/* Get all the annotation tick knots */
		
		annot_pos = GMT_lower_axis_item(k);				/* 1 means lower annotation, 0 means upper (close to axis) */
		font = C->current.setting.font_annot[annot_pos];		/* Set the font to use */
		GMT_setfont (C, &C->current.setting.font_annot[annot_pos]);

		for (i = 0; k < 4 && i < nx - is_interval; i++) {
			if (GMT_annot_pos (C, val0, val1, T, &knots[i], &t_use)) continue;			/* Outside range */
			if (gmt_skip_second_annot (k, knots[i], knots_p, np, primary, secondary)) continue;	/* Secondary annotation skipped when coinciding with primary annotation */
			x = (*xyz_fwd) (C, t_use);	/* Convert to inches on the page */
			if (axis == GMT_X)
				PSL_command (P, "%ld PSL_A%ld_y M\n", psl_iz (P, x), annot_pos);			/* Move to new anchor point */
			else
				PSL_command (P, "PSL_A%ld_y %ld M\n", annot_pos, psl_iz (P, x));			/* Move to new anchor point */
			if (label_c && label_c[i] && label_c[i][0])
				strcpy (string, label_c[i]);
			else
				GMT_get_coordinate_label (C, string, &C->current.plot.calclock, format, T, knots[i]);	/* Get annotation string */
			PSL_plottext (P, 0.0, 0.0, -font.size, string, (ortho == (axis == GMT_X)) ? 90.0 : 0.0, ortho ? PSL_MR : PSL_BC, form);
		}

		if (nx) GMT_free (C, knots);
		if (label_c) {
			for (i = 0; i < nx; i++) if (label_c[i]) free ((void *)label_c[i]);
			GMT_free (C, label_c);
		}
	}
	if (np) GMT_free (C, knots_p);
	
	/* Finally do axis label */

	if (A->label[0] && annotate && !GMT_axis_is_geo (C, axis)) {
		PSL_command (P, (axis == GMT_X) ? "%ld PSL_L_y M\n" : "PSL_L_y %ld M\n", psl_iz (P, 0.5 * length));		/* Move to new anchor point */
		form = GMT_setfont (C, &C->current.setting.font_label);
		PSL_plottext (P, 0.0, 0.0, -C->current.setting.font_label.size, A->label, (axis == GMT_X) ? 0.0 : 90.0, PSL_BC, form);
	}
	if (axis == GMT_X)
		PSL_comment (P, below ? "End of lower x-axis\n" : "End of upper x-axis\n");
	else if (axis == GMT_Y)
		PSL_comment (P, below ? "End of left y-axis\n" : "End of right y-axis\n");
	else
		PSL_comment (P, below ? "End of front z-axis\n" : "End of back z-axis\n");
	PSL_setorigin (P, -x0, -y0, 0.0, PSL_INV);
}

void gmt_map_latline (struct GMT_CTRL *C, struct PSL_CTRL *P, double lat, double west, double east)		/* Draws a line of constant latitude */
{
	GMT_LONG nn;
	double *llon = NULL, *llat = NULL;

	nn = GMT_latpath (C, lat, west, east, &llon, &llat);
	C->current.plot.n = GMT_geo_to_xy_line (C, llon, llat, nn);

	if (C->current.plot.n > 1) {	/* Need at least 2 points for a line */
		PSL_comment (P, "Lat = %g\n", lat);
		if (C->current.map.parallel_straight) {	/* Simplify to a 2-point straight line */
			C->current.plot.x[1] = C->current.plot.x[C->current.plot.n-1];
			C->current.plot.y[1] = C->current.plot.y[C->current.plot.n-1];
			C->current.plot.pen[1] = C->current.plot.pen[C->current.plot.n-1];
			C->current.plot.n = 2;
		}
		GMT_plot_line (C, C->current.plot.x, C->current.plot.y, C->current.plot.pen, C->current.plot.n);
	}
	GMT_free (C, llon);
	GMT_free (C, llat);
}

void gmt_map_lonline (struct GMT_CTRL *C, struct PSL_CTRL *P, double lon, double south, double north)	/* Draws a line of constant longitude */
{
	GMT_LONG nn;
	double *llon = NULL, *llat = NULL;

	nn = GMT_lonpath (C, lon, south, north, &llon, &llat);
	C->current.plot.n = GMT_geo_to_xy_line (C, llon, llat, nn);

	if (C->current.plot.n > 1) {	/* Need at least 2 points for a line */
		PSL_comment (P, "Lon = %g\n", lon);
		if (C->current.map.meridian_straight) {	/* Simplify to a 2-point straight line */
			C->current.plot.x[1] = C->current.plot.x[C->current.plot.n-1];
			C->current.plot.y[1] = C->current.plot.y[C->current.plot.n-1];
			C->current.plot.pen[1] = C->current.plot.pen[C->current.plot.n-1];
			C->current.plot.n = 2;
		}
		GMT_plot_line (C, C->current.plot.x, C->current.plot.y, C->current.plot.pen, C->current.plot.n);
	}
	GMT_free (C, llon);
	GMT_free (C, llat);
}

void GMT_linearx_grid (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, double dval)
{
	double *x = NULL, ys, yn, p_cap = 0.0, cap_start[2] = {0.0, 0.0}, cap_stop[2] = {0.0, 0.0};
	GMT_LONG i, nx, idup = FALSE, cap = FALSE;

	/* Do we have duplicate e and w boundaries ? */
	idup = (GMT_IS_AZIMUTHAL(C) && GMT_IS_ZERO(e-w-360.0));

	if (GMT_POLE_IS_POINT(C)) {	/* Might have two separate domains of gridlines */
		if (C->current.proj.projection == GMT_POLAR) {	/* Different for polar graphs since "lat" = 0 is at the center */
			ys = cap_stop[0] = cap_stop[1] = p_cap = 90.0 - C->current.setting.map_polar_cap[0];
			yn = n;
			cap_start[0] = cap_start[1] = 0.0;
		}
		else {
			p_cap = C->current.setting.map_polar_cap[0];
			ys = MAX (s, -p_cap);
			yn = MIN (n, p_cap);
			cap_start[0] = s;
			cap_stop[0]  = -p_cap ;
			cap_start[1] = p_cap;
			cap_stop[1]  = n;
		}
		cap = !GMT_IS_ZERO (C->current.setting.map_polar_cap[0] - 90.0);
	}
	else {
		ys = s;
		yn = n;
	}
	nx = GMT_linear_array (C, w, e, dval, C->current.map.frame.axis[GMT_X].phase, &x);
	for (i = 0; i < nx - idup; i++) gmt_map_lonline (C, P, x[i], ys, yn);
	if (nx) GMT_free (C, x);

	if (cap) {	/* Also draw the polar cap(s) */
		nx = 0;
		if (s < -C->current.setting.map_polar_cap[0]) {	/* Must draw some or all of the S polar cap */
			nx = GMT_linear_array (C, w, e, C->current.setting.map_polar_cap[1], C->current.map.frame.axis[GMT_X].phase, &x);
			for (i = 0; i < nx - idup; i++) gmt_map_lonline (C, P, x[i], cap_start[0], cap_stop[0]);
			gmt_map_latline (C, P, -p_cap, w, e);
		}
		if (n > C->current.setting.map_polar_cap[0]) {	/* Must draw some or all of the N polar cap */
			if (nx == 0) nx = GMT_linear_array (C, w, e, C->current.setting.map_polar_cap[1], C->current.map.frame.axis[GMT_X].phase, &x);
			for (i = 0; i < nx - idup; i++) gmt_map_lonline (C, P, x[i], cap_start[1], cap_stop[1]);
			gmt_map_latline (C, P, p_cap, w, e);
		}
		if (nx) GMT_free (C, x);
	}

}

void gmt_lineary_grid (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, double dval)
{
	double *y = NULL;
	GMT_LONG i, ny;

	if (C->current.proj.z_down) {
		ny = GMT_linear_array (C, 0.0, n-s, dval, C->current.map.frame.axis[GMT_Y].phase, &y);
		for (i = 0; i < ny; i++) y[i] = C->common.R.wesn[YHI] - y[i];	/* These are the radial values needed for positioning */
	}
	else
		ny = GMT_linear_array (C, s, n, dval, C->current.map.frame.axis[GMT_Y].phase, &y);
	for (i = 0; i < ny; i++) gmt_map_latline (C, P, y[i], w, e);
	if (ny) GMT_free (C, y);

}

void gmt_x_grid (struct GMT_CTRL *C, struct PSL_CTRL *P, double s, double n, double *x, GMT_LONG nx)
{
	GMT_LONG i;
	double x1, y1, x2, y2;

	for (i = 0; i < nx; i++) {
		GMT_geo_to_xy (C, x[i], s, &x1, &y1);
		GMT_geo_to_xy (C, x[i], n, &x2, &y2);
		PSL_plotsegment (P, x1, y1, x2, y2);
	}
}

void gmt_y_grid (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double *y, GMT_LONG ny)
{
	GMT_LONG i;
	double x1, y1, x2, y2;

	for (i = 0; i < ny; i++) {
		GMT_geo_to_xy (C, w, y[i], &x1, &y1);
		GMT_geo_to_xy (C, e, y[i], &x2, &y2);
		PSL_plotsegment (P, x1, y1, x2, y2);
	}
}

void gmt_timex_grid (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, GMT_LONG item)
{
	GMT_LONG nx;
	double *x = NULL;

	nx = GMT_time_array (C, w, e, &C->current.map.frame.axis[GMT_X].item[item], &x);
	gmt_x_grid (C, P, s, n, x, nx);
	if (nx) GMT_free (C, x);
}

void gmt_timey_grid (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, GMT_LONG item)
{
	GMT_LONG ny;
	double *y = NULL;

	ny = GMT_time_array (C, s, n, &C->current.map.frame.axis[GMT_Y].item[item], &y);
	gmt_y_grid (C, P, w, e, y, ny);
	if (ny) GMT_free (C, y);
}

void gmt_logx_grid (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, double dval)
{
	GMT_LONG nx;
	double *x = NULL;

	nx = GMT_log_array (C, w, e, dval, &x);
	gmt_x_grid (C, P, s, n, x, nx);
	if (nx) GMT_free (C, x);
}

void gmt_logy_grid (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, double dval)
{
	GMT_LONG ny;
	double *y = NULL;

	ny = GMT_log_array (C, s, n, dval, &y);
	gmt_y_grid (C, P, w, e, y, ny);
	if (ny) GMT_free (C, y);
}

void gmt_powx_grid (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, double dval)
{
	GMT_LONG nx;
	double *x = NULL;

	nx = GMT_pow_array (C, w, e, dval, 0, &x);
	gmt_x_grid (C, P, s, n, x, nx);
	if (nx) GMT_free (C, x);
}

void gmt_powy_grid (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, double dval)
{
	GMT_LONG ny;
	double *y = NULL;

	ny = GMT_pow_array (C, s, n, dval, 1, &y);
	gmt_y_grid (C, P, w, e, y, ny);
	if (ny) GMT_free (C, y);
}

/*	FANCY RECTANGULAR PROJECTION MAP BOUNDARY	*/

void gmt_fancy_frame_offset (struct GMT_CTRL *C, double angle, double shift[2])
{
	/* Given the angle of the axis, return the coordinate adjustments needed to
	 * shift in order to plot the outer 1-2 parallel frame lines (shift[0|1] */

	double s, c;
	sincos (angle, &s, &c);
	shift[0] =  C->current.setting.map_frame_width * s;
	shift[1] = -C->current.setting.map_frame_width * c;
}

void gmt_fancy_frame_straightlon_checkers (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, GMT_LONG secondary_too)
{	/* Plot checkers along straight longitude boundaries */
	GMT_LONG i, k, nx, shade, item[2] = {GMT_TICK_UPPER, GMT_TICK_LOWER};
	double dx, w1, val, v1, v2, x1, x2, y1, y2, shift_s[2], shift_n[2], scale[2];

	scale[0] = (secondary_too) ? 0.5 : 1.0;
	scale[1] = 1.5;

	GMT_geo_to_xy (C, e, s, &x1, &y1);
	GMT_geo_to_xy (C, w, s, &x2, &y2);
	gmt_fancy_frame_offset (C, d_atan2 (y2 - y1, x2 - x1), shift_s);

	GMT_geo_to_xy (C, w, n, &x1, &y1);
	GMT_geo_to_xy (C, e, n, &x2, &y2);
	gmt_fancy_frame_offset (C, d_atan2 (y2 - y1, x2 - x1), shift_n);

	for (k = 0; k < 1 + secondary_too; k++) {
		if (C->current.map.frame.axis[GMT_X].item[item[k]].active) {
			dx = GMT_get_map_interval (C, 0, item[k]);
			shade = ((GMT_LONG)floor ((w - C->current.map.frame.axis[GMT_X].phase)/ dx)) % 2;
			w1 = floor ((w - C->current.map.frame.axis[GMT_X].phase)/ dx) * dx + C->current.map.frame.axis[GMT_X].phase;
			nx = (w1 > e) ? -1 : (GMT_LONG)((e - w1) / dx + GMT_SMALL);
			for (i = 0; i <= nx; i++) {
				shade = !shade;
				val = w1 + i * dx;
				v1 = MAX (val, w);
				v2 = MIN (val + dx, e);
				if (v2 - v1 < GMT_CONV_LIMIT) continue;
				PSL_setcolor (P, shade ? C->current.setting.map_frame_pen.rgb : C->PSL->init.page_rgb, PSL_IS_STROKE);
				if (C->current.map.frame.side[S_SIDE]) {
					GMT_geo_to_xy (C, v1, s, &x1, &y1);
					GMT_geo_to_xy (C, v2, s, &x2, &y2);
					PSL_plotsegment (P, x1-0.5*scale[k]*shift_s[0], y1-0.5*scale[k]*shift_s[1], x2-0.5*scale[k]*shift_s[0], y2-0.5*scale[k]*shift_s[1]);
				}
				if (C->current.map.frame.side[N_SIDE]) {
					GMT_geo_to_xy (C, v1, n, &x1, &y1);
					GMT_geo_to_xy (C, v2, n, &x2, &y2);
					PSL_plotsegment (P, x1-0.5*scale[k]*shift_n[0], y1-0.5*scale[k]*shift_n[1], x2-0.5*scale[k]*shift_n[0], y2-0.5*scale[k]*shift_n[1]);
				}
			}
		}
	}
}

void gmt_fancy_frame_curvedlon_checkers (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, GMT_LONG secondary_too)
{	/* Plot checkers along curved longitude boundaries */
	GMT_LONG i, k, nx, shade, item[2] = {GMT_TICK_UPPER, GMT_TICK_LOWER};
	double dx, w1, v1, v2, val, x1, x2, y1, y2, az1, az2, dr, scale[2], radius_s, radius_n;

	scale[0] = (secondary_too) ? 0.5 : 1.0;
	scale[1] = 1.5;
	dr = 0.5 * C->current.setting.map_frame_width;
	GMT_geo_to_xy (C, w, s, &x1, &y1);
	GMT_geo_to_xy (C, w, n, &x2, &y2);
	radius_s = hypot (x1 - C->current.proj.c_x0, y1 - C->current.proj.c_y0);
	radius_n = hypot (x2 - C->current.proj.c_x0, y2 - C->current.proj.c_y0);

	for (k = 0; k < 1 + secondary_too; k++) {
		if (C->current.map.frame.axis[GMT_X].item[item[k]].active) {
			dx = GMT_get_map_interval (C, 0, item[k]);
			shade = ((GMT_LONG)floor ((w - C->current.map.frame.axis[GMT_X].phase) / dx)) % 2;
			w1 = floor((w - C->current.map.frame.axis[GMT_X].phase)/dx) * dx + C->current.map.frame.axis[GMT_X].phase;
			nx = (w1 > e) ? -1 : (GMT_LONG)((e-w1) / dx + GMT_SMALL);
			for (i = 0; i <= nx; i++) {
				shade = !shade;
				val = w1 + i * dx;
				v1 = MAX (val, w);
				v2 = MIN (val + dx, e);
				if (v2 - v1 < GMT_CONV_LIMIT) continue;
				PSL_setcolor (P, shade ? C->current.setting.map_frame_pen.rgb : C->PSL->init.page_rgb, PSL_IS_STROKE);
				if (C->current.map.frame.side[S_SIDE]) {
					GMT_geo_to_xy (C, v2, s, &x1, &y1);
					GMT_geo_to_xy (C, v1, s, &x2, &y2);
					az1 = d_atan2d (y1 - C->current.proj.c_y0, x1 - C->current.proj.c_x0);
					az2 = d_atan2d (y2 - C->current.proj.c_y0, x2 - C->current.proj.c_x0);
					if (C->current.proj.north_pole) {
						if (az1 < az2) az1 += 360.0;
						PSL_plotarc (P, C->current.proj.c_x0, C->current.proj.c_y0, radius_s+scale[k]*dr, az2, az1, PSL_MOVE + PSL_STROKE);
					}
					else {
						if (az2 < az1) az2 += 360.0;
						PSL_plotarc (P, C->current.proj.c_x0, C->current.proj.c_y0, radius_s-scale[k]*dr, az1, az2, PSL_MOVE + PSL_STROKE);
					}
				}
				if (C->current.map.frame.side[N_SIDE]) {
					GMT_geo_to_xy (C, v2, n, &x1, &y1);
					GMT_geo_to_xy (C, v1, n, &x2, &y2);
					az1 = d_atan2d (y1 - C->current.proj.c_y0, x1 - C->current.proj.c_x0);
					az2 = d_atan2d (y2 - C->current.proj.c_y0, x2 - C->current.proj.c_x0);
					if (C->current.proj.north_pole) {
						if (az1 < az2) az1 += 360.0;
						PSL_plotarc (P, C->current.proj.c_x0, C->current.proj.c_y0, radius_n-scale[k]*dr, az2, az1, PSL_MOVE + PSL_STROKE);
					}
					else {
						if (az2 < az1) az2 += 360.0;
						PSL_plotarc (P, C->current.proj.c_x0, C->current.proj.c_y0, radius_n+scale[k]*dr, az1, az2, PSL_MOVE + PSL_STROKE);
					}
				}
			}
		}
	}
}

void gmt_fancy_frame_straightlat_checkers (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, GMT_LONG secondary_too)
{	/* Plot checkers along straight latitude boundaries */
	GMT_LONG i, k, ny, shade, item[2] = {GMT_TICK_UPPER, GMT_TICK_LOWER};
	double dy, s1, val, v1, v2, x1, x2, y1, y2, shift_w[2], shift_e[2], scale[2];

	scale[0] = (secondary_too) ? 0.5 : 1.0;
	scale[1] = 1.5;

	GMT_geo_to_xy (C, w, s, &x1, &y1);
	GMT_geo_to_xy (C, w, n, &x2, &y2);
	gmt_fancy_frame_offset (C, d_atan2 (y2 - y1, x2 - x1), shift_w);

	GMT_geo_to_xy (C, e, s, &x1, &y1);
	GMT_geo_to_xy (C, e, n, &x2, &y2);
	gmt_fancy_frame_offset (C, d_atan2 (y2 - y1, x2 - x1), shift_e);

	/* Tick S-N axes */

	for (k = 0; k < 1 + secondary_too; k++) {
		if (C->current.map.frame.axis[GMT_Y].item[item[k]].active) {
			dy = GMT_get_map_interval (C, 1, item[k]);
			shade = ((GMT_LONG)floor ((s - C->current.map.frame.axis[GMT_Y].phase) / dy)) % 2;
			s1 = floor((s - C->current.map.frame.axis[GMT_Y].phase)/dy) * dy + C->current.map.frame.axis[GMT_Y].phase;
			ny = (s1 > n) ? -1 : (GMT_LONG)((n-s1) / dy + GMT_SMALL);
			for (i = 0; i <= ny; i++) {
				shade = !shade;
				val = s1 + i * dy;
				v1 = MAX (val, s);
				v2 = MIN (val + dy, n);
				if (v2 - v1 < GMT_CONV_LIMIT) continue;
				PSL_setcolor (P, shade ? C->current.setting.map_frame_pen.rgb : C->PSL->init.page_rgb, PSL_IS_STROKE);
				if (C->current.map.frame.side[W_SIDE]) {
					GMT_geo_to_xy (C, w, v1, &x1, &y1);
					GMT_geo_to_xy (C, w, v2, &x2, &y2);
					PSL_plotsegment (P, x1-0.5*scale[k]*shift_w[0], y1-0.5*scale[k]*shift_w[1], x2-0.5*scale[k]*shift_w[0], y2-0.5*scale[k]*shift_w[1]);
				}
				if (C->current.map.frame.side[E_SIDE]) {
					GMT_geo_to_xy (C, e, v1, &x1, &y1);
					GMT_geo_to_xy (C, e, v2, &x2, &y2);
					PSL_plotsegment (P, x1+0.5*scale[k]*shift_e[0], y1+0.5*scale[k]*shift_e[1], x2+0.5*scale[k]*shift_e[0], y2+0.5*scale[k]*shift_e[1]);
				}
			}
		}
	}
}

void gmt_fancy_frame_straight_outline (struct GMT_CTRL *C, struct PSL_CTRL *P, double lonA, double latA, double lonB, double latB, GMT_LONG side, GMT_LONG secondary_too)
{
	GMT_LONG k;
	double scale = 1.0, x[2], y[2], angle, s, c, dx, dy, Ldx, Ldy;

	if (!C->current.map.frame.side[side]) return;	/* Do not draw this frame side */

	if (secondary_too) scale = 0.5;

	GMT_geo_to_xy (C, lonA, latA, &x[0], &y[0]);
	GMT_geo_to_xy (C, lonB, latB, &x[1], &y[1]);
	angle = d_atan2 (y[1] - y[0], x[1] - x[0]);
	sincos (angle, &s, &c);
	Ldx = (C->current.setting.map_frame_type == GMT_IS_ROUNDED) ? 0.0 : C->current.setting.map_frame_width * c;
	Ldy = (C->current.setting.map_frame_type == GMT_IS_ROUNDED) ? 0.0 : C->current.setting.map_frame_width * s;
	dx =  C->current.setting.map_frame_width * s;
	dy = -C->current.setting.map_frame_width * c;
	PSL_plotsegment (P, x[0]-Ldx, y[0]-Ldy, x[1]+Ldx, y[1]+Ldy);
	for (k = 0; k < 1 + secondary_too; k++) {
		x[0] += scale*dx;
		y[0] += scale*dy;
		x[1] += scale*dx;
		y[1] += scale*dy;
		PSL_plotsegment (P, x[0]-Ldx, y[0]-Ldy, x[1]+Ldx, y[1]+Ldy);
	}
}

void gmt_fancy_frame_curved_outline (struct GMT_CTRL *C, struct PSL_CTRL *P, double lonA, double latA, double lonB, double latB, GMT_LONG side, GMT_LONG secondary_too)
{
	double scale[2] = {1.0, 1.0}, escl, x1, x2, y1, y2, radius, dr, r_inc, az1, az2, da0, da, width, s;

	if (!C->current.map.frame.side[side]) return;

	if (secondary_too) scale[0] = scale[1] = 0.5;
	width = C->current.setting.map_frame_width;
	escl = (C->current.setting.map_frame_type == GMT_IS_ROUNDED) ? 0.0 : 1.0;	/* Want rounded corners */
	GMT_geo_to_xy (C, lonA, latA, &x1, &y1);
	GMT_geo_to_xy (C, lonB, latB, &x2, &y2);
	radius = hypot (x1 - C->current.proj.c_x0, y1 - C->current.proj.c_y0);
	dr = 0.5 * width;
	s = ((C->current.proj.north_pole && side == 2) || (!C->current.proj.north_pole && side == 0)) ? -1.0 : +1.0;	/* North: needs shorter radius.  South: Needs longer radius (opposite in S hemi) */
	r_inc = s*scale[0] * width;
	if (GMT_IS_AZIMUTHAL(C) && GMT_360_RANGE (lonA, lonB)) {	/* Full 360-degree cirle */
		PSL_plotarc (P, C->current.proj.c_x0, C->current.proj.c_y0, radius, 0.0, 360.0, PSL_MOVE + PSL_STROKE);
		PSL_plotarc (P, C->current.proj.c_x0, C->current.proj.c_y0, radius + r_inc, 0.0, 360.0, PSL_MOVE + PSL_STROKE);
		if (secondary_too) PSL_plotarc (P, C->current.proj.c_x0, C->current.proj.c_y0, radius + 2.0 * r_inc, 0.0, 360.0, PSL_MOVE + PSL_STROKE);
	}
	else {
		az1 = d_atan2d (y1 - C->current.proj.c_y0, x1 - C->current.proj.c_x0);
		az2 = d_atan2d (y2 - C->current.proj.c_y0, x2 - C->current.proj.c_x0);
		if (!C->current.proj.north_pole) d_swap (az1, az2);	/* In S hemisphere, must draw in opposite direction */
		while (az1 < 0.0) az1 += 360.0;	/* Wind az1 to be in the 0-360 range */
		while (az2 < az1) az2 += 360.0;	/* Likewise ensure az1 > az1 and is now in the 0-720 range */
		da0 = R2D * escl * width /radius;
		da  = R2D * escl * width / (radius + r_inc);
		PSL_plotarc (P, C->current.proj.c_x0, C->current.proj.c_y0, radius, az1-da0, az2+da0, PSL_MOVE + PSL_STROKE);
		PSL_plotarc (P, C->current.proj.c_x0, C->current.proj.c_y0, radius + r_inc, az1-da, az2+da, PSL_MOVE + PSL_STROKE);
		if (secondary_too) {
			r_inc *= 2.0;
			da = R2D * escl * width / (radius + r_inc);
			PSL_plotarc (P, C->current.proj.c_x0, C->current.proj.c_y0, radius + r_inc, az1-da, az2+da, PSL_MOVE + PSL_STROKE);
		}
	}
}

void gmt_rounded_framecorners (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, GMT_LONG secondary_too)
{
	GMT_LONG k;
	double x1, y1, x2, y2, anglew, anglee, x, y, width;

	if (C->current.setting.map_frame_type != GMT_IS_ROUNDED) return;	/* Only do this if rounded corners are requested */

	GMT_geo_to_xy (C, w, n, &x1, &y1);
	GMT_geo_to_xy (C, w, s, &x2, &y2);
	anglew = d_atan2d (y2 - y1, x2 - x1);

	GMT_geo_to_xy (C, e, s, &x1, &y1);
	GMT_geo_to_xy (C, e, n, &x2, &y2);
	anglee = d_atan2d (y2 - y1, x2 - x1);

	width = ((secondary_too) ? 0.5 : 1.0) * fabs (C->current.setting.map_frame_width);
	for (k = 0; k < 1 + secondary_too; k++) {
		if (C->current.map.frame.side[S_SIDE] && C->current.map.frame.side[E_SIDE]) {
			GMT_geo_to_xy (C, e, s, &x, &y);
			PSL_plotarc (P, x, y, (k+1)*width, 180.0+anglee, 270.0+anglee, PSL_MOVE + PSL_STROKE);
		}
		if (C->current.map.frame.side[E_SIDE] && C->current.map.frame.side[N_SIDE]) {
			GMT_geo_to_xy (C, e, n, &x, &y);
			PSL_plotarc (P, x, y, (k+1)*width, 270.0+anglee, 360.0+anglee, PSL_MOVE + PSL_STROKE);
		}
		if (C->current.map.frame.side[N_SIDE] && C->current.map.frame.side[W_SIDE]) {
			GMT_geo_to_xy (C, w, n, &x, &y);
			PSL_plotarc (P, x, y, (k+1)*width, 180.0+anglew, 270.0+anglew, PSL_MOVE + PSL_STROKE);
		}
		if (C->current.map.frame.side[W_SIDE] && C->current.map.frame.side[S_SIDE]) {
			GMT_geo_to_xy (C, w, s, &x, &y);
			PSL_plotarc (P, x, y, (k+1)*width, 270.0+anglew, 360.0+anglew, PSL_MOVE + PSL_STROKE);
		}
		if ((GMT_IS_AZIMUTHAL(C) || GMT_IS_CONICAL(C)) && C->current.map.frame.side[W_SIDE] && C->current.map.frame.side[E_SIDE]) {	/* Round off the pointy head? */
			if (GMT_IS_ZERO (C->common.R.wesn[YHI] - 90.0)) {
				GMT_geo_to_xy (C, w, n, &x, &y);
				PSL_plotarc (P, x, y, (k+1)*width, anglee, 180.0+anglew, PSL_MOVE + PSL_STROKE);
			}
			else if (GMT_IS_ZERO (C->common.R.wesn[YLO] + 90.0)) {
				GMT_geo_to_xy (C, w, s, &x, &y);
				PSL_plotarc (P, x, y, (k+1)*width, anglew-90.0, anglee-90.0, PSL_MOVE + PSL_STROKE);
			}
		}
	}
}

void gmt_wesn_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	GMT_LONG i, np = 0;
	double *xx = NULL, *yy = NULL;

	GMT_setpen (C, &C->current.setting.map_frame_pen);

	if (C->current.map.frame.side[W_SIDE]) {	/* West */
		np = GMT_map_path (C, w, s, w, n, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geo_to_xy (C, xx[i], yy[i], &xx[i], &yy[i]);
		PSL_plotline (P, xx, yy, np, PSL_MOVE + PSL_STROKE);
		GMT_free (C, xx);
		GMT_free (C, yy);
	}
	if (C->current.map.frame.side[E_SIDE]) {	/* East */
		np = GMT_map_path (C, e, s, e, n, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geo_to_xy (C, xx[i], yy[i], &xx[i], &yy[i]);
		PSL_plotline (P, xx, yy, np, PSL_MOVE + PSL_STROKE);
		GMT_free (C, xx);
		GMT_free (C, yy);
	}
	if (C->current.map.frame.side[S_SIDE]) {	/* South */
		np = GMT_map_path (C, w, s, e, s, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geo_to_xy (C, xx[i], yy[i], &xx[i], &yy[i]);
		PSL_plotline (P, xx, yy, np, PSL_MOVE + PSL_STROKE);
		GMT_free (C, xx);
		GMT_free (C, yy);
	}
	if (C->current.map.frame.side[N_SIDE]) {	/* North */
		np = GMT_map_path (C, w, n, e, n, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geo_to_xy (C, xx[i], yy[i], &xx[i], &yy[i]);
		PSL_plotline (P, xx, yy, np, PSL_MOVE + PSL_STROKE);
		GMT_free (C, xx);
		GMT_free (C, yy);
	}
}

void gmt_fancy_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	double fat_pen, thin_pen;
	GMT_LONG dual = FALSE, cap = P->internal.line_cap;

	if (C->current.setting.map_frame_type == GMT_IS_PLAIN) {	/* Draw plain boundary and return */
		gmt_wesn_map_boundary (C, P, w, e, s, n);
		return;
	}

	PSL_setcolor (P, C->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);

	fat_pen = fabs (C->current.setting.map_frame_width) * C->session.u2u[GMT_INCH][GMT_PT];
	if (C->current.map.frame.axis[GMT_Y].item[GMT_TICK_LOWER].active) {	/* Need two-layer frame */
		fat_pen *= 0.5;
		dual = TRUE;
	}
	thin_pen = 0.1 * fat_pen;

	/* Draw frame checkers */
	/* This needs to be done with BUTT cap since checker segments are drawn as heavy lines */

	PSL_setlinewidth (P, fat_pen);
	PSL_setlinecap (P, PSL_BUTT_CAP);

	gmt_fancy_frame_straightlat_checkers (C, P, w, e, s, n, dual);
	gmt_fancy_frame_straightlon_checkers (C, P, w, e, s, n, dual);

	/* Draw the outline on top of the checkers */
	/* Reset line cap, etc. */

	PSL_setlinecap (P, cap);
	PSL_setcolor (P, C->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);
	PSL_setlinewidth (P, thin_pen);

	gmt_fancy_frame_straight_outline (C, P, w, s, e, s, 0, dual);
	gmt_fancy_frame_straight_outline (C, P, e, s, e, n, 1, dual);
	gmt_fancy_frame_straight_outline (C, P, e, n, w, n, 2, dual);
	gmt_fancy_frame_straight_outline (C, P, w, n, w, s, 3, dual);

	gmt_rounded_framecorners (C, P, w, e, s, n, dual);
}

void gmt_rect_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double x0, double y0, double x1, double y1)
{
	GMT_LONG cap = P->internal.line_cap;

	GMT_setpen (C, &C->current.setting.map_frame_pen);
	/* Temporarily change to square cap so rectangular frames have neat corners */
	PSL_setlinecap (P, PSL_SQUARE_CAP);

	if (C->current.map.frame.side[W_SIDE]) PSL_plotsegment (P, x0, y0, x0, y1);	/* West */
	if (C->current.map.frame.side[E_SIDE]) PSL_plotsegment (P, x1, y0, x1, y1);	/* East */
	if (C->current.map.frame.side[S_SIDE]) PSL_plotsegment (P, x0, y0, x1, y0);	/* South */
	if (C->current.map.frame.side[N_SIDE]) PSL_plotsegment (P, x0, y1, x1, y1);	/* North */
	PSL_setlinecap (P, cap);	/* Reset back to default */
}

/*	GMT_POLAR (S or N) PROJECTION MAP BOUNDARY	*/

void gmt_polar_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	GMT_LONG dual = FALSE, cap = P->internal.line_cap;
	double thin_pen, fat_pen;

	if (C->common.R.oblique) { /* Draw rectangular boundary and return */
		gmt_rect_map_boundary (C, P, 0.0, 0.0, C->current.proj.rect[XHI], C->current.proj.rect[YHI]);
		return;
	}

	if (!C->current.proj.north_pole && s <= -90.0) /* Cannot have southern boundary */
		C->current.map.frame.side[S_SIDE] = 0;
	if (C->current.proj.north_pole && n >= 90.0) /* Cannot have northern boundary */
		C->current.map.frame.side[N_SIDE] = 0;
	if (GMT_360_RANGE (w,e) || GMT_IS_ZERO (e - w))
		C->current.map.frame.side[E_SIDE] = C->current.map.frame.side[W_SIDE] = 0;

	if (C->current.setting.map_frame_type == GMT_IS_PLAIN) { /* Draw plain boundary and return */
		gmt_wesn_map_boundary (C, P, w, e, s, n);
		return;
	}

	/* Here draw fancy map boundary */

	fat_pen = fabs (C->current.setting.map_frame_width) * C->session.u2u[GMT_INCH][GMT_PT];
	if (C->current.map.frame.axis[GMT_Y].item[GMT_TICK_LOWER].active) {	/* Need two-layer frame */
		fat_pen *= 0.5;
		dual = TRUE;
	}
	thin_pen = 0.1 * fat_pen;

	/* Draw frame checkers */
	/* This needs to be done with BUTT cap since checker segments are drawn as heavy lines */

	PSL_setlinewidth (P, fat_pen);
	PSL_setlinecap (P, PSL_BUTT_CAP);

	gmt_fancy_frame_straightlat_checkers (C, P, w, e, s, n, dual);
	gmt_fancy_frame_curvedlon_checkers (C, P, w, e, s, n, dual);

	/* Draw the outline on top of the checkers */
	/* Reset line cap, etc. */

	PSL_setlinecap (P, cap);
	PSL_setcolor (P, C->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);
	PSL_setlinewidth (P, thin_pen);

	gmt_fancy_frame_curved_outline (C, P, w, s, e, s, 0, dual);
	gmt_fancy_frame_straight_outline (C, P, e, s, e, n, 1, dual);
	gmt_fancy_frame_curved_outline (C, P, w, n, e, n, 2, dual);
	gmt_fancy_frame_straight_outline (C, P, w, n, w, s, 3, dual);

	gmt_rounded_framecorners (C, P, w, e, s, n, dual);
}

/*	CONIC PROJECTION MAP BOUNDARY	*/

void gmt_conic_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	GMT_LONG dual = FALSE, cap = P->internal.line_cap;
	double thin_pen, fat_pen;

	if (C->common.R.oblique) { /* Draw rectangular boundary and return */
		gmt_rect_map_boundary (C, P, 0.0, 0.0, C->current.proj.rect[XHI], C->current.proj.rect[YHI]);
		return;
	}

	if (!C->current.proj.north_pole && s <= -90.0) /* Cannot have southern boundary */
		C->current.map.frame.side[S_SIDE] = 0;
	if (C->current.proj.north_pole && n >= 90.0) /* Cannot have northern boundary */
		C->current.map.frame.side[N_SIDE] = 0;

	if (C->current.setting.map_frame_type == GMT_IS_PLAIN) { /* Draw plain boundary and return */
		gmt_wesn_map_boundary (C, P, w, e, s, n);
		return;
	}

	/* Here draw fancy map boundary */

	fat_pen = fabs (C->current.setting.map_frame_width) * C->session.u2u[GMT_INCH][GMT_PT];
	if (C->current.map.frame.axis[GMT_Y].item[GMT_TICK_LOWER].active) {	/* Need two-layer frame */
		fat_pen *= 0.5;
		dual = TRUE;
	}
	thin_pen = 0.1 * fat_pen;

	/* Draw frame checkers */
	/* This needs to be done with BUTT cap since checker segments are drawn as heavy lines */

	PSL_setlinewidth (P, fat_pen);
	PSL_setlinecap (P, PSL_BUTT_CAP);

	gmt_fancy_frame_straightlat_checkers (C, P, w, e, s, n, dual);
	gmt_fancy_frame_curvedlon_checkers (C, P, w, e, s, n, dual);

	/* Draw the outline on top of the checkers */
	/* Reset line cap, etc. */

	PSL_setlinecap (P, cap);
	PSL_setcolor (P, C->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);
	PSL_setlinewidth (P, thin_pen);

	gmt_fancy_frame_curved_outline (C, P, w, s, e, s, 0, dual);
	gmt_fancy_frame_straight_outline (C, P, e, s, e, n, 1, dual);
	gmt_fancy_frame_curved_outline (C, P, w, n, e, n, 2, dual);
	gmt_fancy_frame_straight_outline (C, P, w, n, w, s, 3, dual);

	gmt_rounded_framecorners (C, P, w, e, s, n, dual);
}

/*	OBLIQUE MERCATOR PROJECTION MAP FUNCTIONS	*/

void gmt_oblmrc_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	gmt_rect_map_boundary (C, P, 0.0, 0.0, C->current.proj.rect[XHI], C->current.proj.rect[YHI]);
}

/*	MOLLWEIDE and HAMMER-AITOFF EQUAL AREA PROJECTION MAP FUNCTIONS	*/

void gmt_ellipse_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	if (C->common.R.oblique) { /* Draw rectangular boundary and return */
		gmt_rect_map_boundary (C, P, 0.0, 0.0, C->current.proj.rect[XHI], C->current.proj.rect[YHI]);
		return;
	}
	if (C->common.R.wesn[YLO] <= -90.0) /* Cannot have southern boundary */
		C->current.map.frame.side[S_SIDE] = 0;
	if (C->common.R.wesn[YHI] >= 90.0) /* Cannot have northern boundary */
		C->current.map.frame.side[N_SIDE] = 0;

	gmt_wesn_map_boundary (C, P, w, e, s, n);
}

void gmt_basic_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	if (C->common.R.oblique) { /* Draw rectangular boundary and return */
		gmt_rect_map_boundary (C, P, 0.0, 0.0, C->current.proj.rect[XHI], C->current.proj.rect[YHI]);
		return;
	}
	gmt_wesn_map_boundary (C, P, w, e, s, n);
}

/*
 *	GENERIC MAP PLOTTING FUNCTIONS
 */

GMT_LONG gmt_genper_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	GMT_LONG nr;

	if (C->common.R.oblique) {	/* Draw rectangular boundary and return */
		gmt_rect_map_boundary (C, P, 0.0, 0.0, C->current.proj.rect[XHI], C->current.proj.rect[YHI]);
		return 0;
	}

	GMT_setpen (C, &C->current.setting.map_frame_pen);

	nr = C->current.map.n_lon_nodes + C->current.map.n_lat_nodes;
	if (nr >= C->current.plot.n_alloc) GMT_get_plot_array (C);

	if (C->current.proj.g_debug > 1) GMT_message (C, "genper_map_boundary nr = %ld\n", nr);

	GMT_genper_map_clip_path (C, nr, C->current.plot.x, C->current.plot.y);

	PSL_plotline (P, C->current.plot.x, C->current.plot.y, nr, PSL_MOVE + PSL_STROKE);

	return 0;
}

void gmt_circle_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	if (C->common.R.oblique) { /* Draw rectangular boundary and return */
		gmt_rect_map_boundary (C, P, 0.0, 0.0, C->current.proj.rect[XHI], C->current.proj.rect[YHI]);
		return;
	}

	GMT_setpen (C, &C->current.setting.map_frame_pen);

	PSL_plotarc (P, C->current.proj.r, C->current.proj.r, C->current.proj.r, 0.0, 360.0, PSL_MOVE + PSL_STROKE);
}

void gmt_theta_r_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	GMT_LONG i, nr;
	double a, da;
	double xx[2], yy[2];

	GMT_setpen (C, &C->current.setting.map_frame_pen);

	if (C->current.proj.got_elevations) {
		if (GMT_IS_ZERO (n - 90.0)) C->current.map.frame.side[N_SIDE] = 0;	/* No donuts, please */
	}
	else {
		if (GMT_IS_ZERO (s)) C->current.map.frame.side[S_SIDE] = 0;		/* No donuts, please */
	}
	if (GMT_360_RANGE (w,e) || GMT_IS_ZERO (e - w))
		C->current.map.frame.side[E_SIDE] = C->current.map.frame.side[W_SIDE] = 0;
	nr = C->current.map.n_lon_nodes;
	while (nr > C->current.plot.n_alloc) GMT_get_plot_array (C);
	da = fabs (C->common.R.wesn[XHI] - C->common.R.wesn[XLO]) / (nr - 1);
	if (C->current.map.frame.side[N_SIDE]) {
		for (i = 0; i < nr; i++) {
			a = C->common.R.wesn[XLO] + i * da;
			GMT_geo_to_xy (C, a, C->common.R.wesn[YHI], &C->current.plot.x[i], &C->current.plot.y[i]);
		}
		PSL_plotline (P, C->current.plot.x, C->current.plot.y, nr, PSL_MOVE + PSL_STROKE);
	}
	if (C->current.map.frame.side[S_SIDE]) {
		for (i = 0; i < nr; i++) {
			a = C->common.R.wesn[XLO] + i * da;
			GMT_geo_to_xy (C, a, C->common.R.wesn[YLO], &C->current.plot.x[i], &C->current.plot.y[i]);
		}
		PSL_plotline (P, C->current.plot.x, C->current.plot.y, nr, PSL_MOVE + PSL_STROKE);
	}
	if (C->current.map.frame.side[E_SIDE]) {
		GMT_geo_to_xy (C, C->common.R.wesn[XHI], C->common.R.wesn[YLO], &xx[0], &yy[0]);
		GMT_geo_to_xy (C, C->common.R.wesn[XHI], C->common.R.wesn[YHI], &xx[1], &yy[1]);
		PSL_plotline (P, xx, yy, 2, PSL_MOVE + PSL_STROKE);
	}
	if (C->current.map.frame.side[W_SIDE]) {
		GMT_geo_to_xy (C, C->common.R.wesn[XLO], C->common.R.wesn[YLO], &xx[0], &yy[0]);
		GMT_geo_to_xy (C, C->common.R.wesn[XLO], C->common.R.wesn[YHI], &xx[1], &yy[1]);
		PSL_plotline (P, xx, yy, 2, PSL_MOVE + PSL_STROKE);
	}
}

void gmt_map_tick (struct GMT_CTRL *C, struct PSL_CTRL *P, double *xx, double *yy, GMT_LONG *sides, double *angles, GMT_LONG nx, GMT_LONG type, double len)
{
	double angle, xl, yl, c, s, tick_length;
	GMT_LONG i;

	for (i = 0; i < nx; i++) {
		if (!C->current.proj.edge[sides[i]]) continue;
		if (!C->current.map.frame.side[sides[i]]) continue;
		if (!(C->current.setting.map_annot_oblique & 1) && ((type == 0 && (sides[i] % 2)) || (type == 1 && !(sides[i] % 2)))) continue;
		angle = ((C->current.setting.map_annot_oblique & 16) ? (sides[i] - 1) * 90.0 : angles[i]);
		sincosd (angle, &s, &c);
		tick_length = len;
		if (C->current.setting.map_annot_oblique & 8) {
			if (sides[i] % 2) {
				/* if (fabs (c) > cosd (C->current.setting.map_annot_min_angle)) continue; */
				if (fabs (c) < sind (C->current.setting.map_annot_min_angle)) continue;
				tick_length /= fabs(c);
			}
			else {
				if (fabs (s) < sind (C->current.setting.map_annot_min_angle)) continue;
				tick_length /= fabs(s);
			}
		}
		xl = tick_length * c;
		yl = tick_length * s;
		PSL_plotsegment (P, xx[i], yy[i], xx[i]+xl, yy[i]+yl);
	}
}

void gmt_map_lontick (struct GMT_CTRL *C, struct PSL_CTRL *P, double lon, double south, double north, double len)
{
	GMT_LONG i, nc;
	struct GMT_XINGS *xings = NULL;

	nc = GMT_map_loncross (C, lon, south, north, &xings);
	for (i = 0; i < nc; i++) gmt_map_tick (C, P, xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, xings[i].nx, 0, len);
	if (nc) GMT_free (C, xings);
}

void gmt_map_lattick (struct GMT_CTRL *C, struct PSL_CTRL *P, double lat, double west, double east, double len)
{
	GMT_LONG i, nc;

	struct GMT_XINGS *xings = NULL;

	nc = GMT_map_latcross (C, lat, west, east, &xings);
	for (i = 0; i < nc; i++) gmt_map_tick (C, P, xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, xings[i].nx, 1, len);
	if (nc) GMT_free (C, xings);
}

GMT_LONG gmt_annot_too_crowded (struct GMT_CTRL *C, double x, double y, GMT_LONG side) {
	/* Checks if the proposed annotation is too close to a previously plotted annotation */
	GMT_LONG i;
	double d_min;

	if (C->current.setting.map_annot_min_spacing <= 0.0) return (FALSE);

	for (i = 0, d_min = DBL_MAX; i < GMT_n_annotations[side]; i++) d_min = MIN (d_min, hypot (GMT_x_annotation[side][i] - x, GMT_y_annotation[side][i] - y));
	if (d_min < C->current.setting.map_annot_min_spacing) return (TRUE);

	/* OK to plot and add to list */

	if (GMT_n_annotations[side] == GMT_alloc_annotations[side]) GMT_alloc_annotations[side] = GMT_malloc2 (C, GMT_x_annotation[side], GMT_y_annotation[side], GMT_n_annotations[side], GMT_alloc_annotations[side], double);
	GMT_x_annotation[side][GMT_n_annotations[side]] = x, GMT_y_annotation[side][GMT_n_annotations[side]] = y, GMT_n_annotations[side]++;

	return (FALSE);
}

void gmt_map_symbol (struct GMT_CTRL *C, struct PSL_CTRL *P, double *xx, double *yy, GMT_LONG *sides, double *line_angles, char *label, GMT_LONG nx, GMT_LONG type, GMT_LONG annot, GMT_LONG level, GMT_LONG form)
{
	/* type = 0 for lon and 1 for lat */

	double line_angle, text_angle, div, tick_length, o_len, len, ca, sa;
	GMT_LONG i, justify, annot_type, flip;	

	len = GMT_get_annot_offset (C, &flip, level);	/* Get annotation offset, and flip justification if "inside" */
	annot_type = 2 << type;		/* 2 = NS, 4 = EW */
	for (i = 0; i < nx; i++) {

		if (GMT_prepare_label (C, line_angles[i], sides[i], xx[i], yy[i], type, &line_angle, &text_angle, &justify)) continue;

		sincosd (line_angle, &sa, &ca);
		tick_length = C->current.setting.map_tick_length;
		o_len = len;
		if (C->current.setting.map_annot_oblique & annot_type) o_len = tick_length;
		if (C->current.setting.map_annot_oblique & 8) {
			div = ((sides[i] % 2) ? fabs (ca) : fabs (sa));
			tick_length /= div;
			o_len /= div;
		}
		xx[i] += o_len * ca;
		yy[i] += o_len * sa;
		if ((C->current.setting.map_annot_oblique & annot_type) && C->current.setting.map_annot_offset[level] > 0.0) {
			if (sides[i] % 2)
				xx[i] += (sides[i] == 1) ? C->current.setting.map_annot_offset[level] : -C->current.setting.map_annot_offset[level];
			else
				yy[i] += (sides[i] == 2) ? C->current.setting.map_annot_offset[level] : -C->current.setting.map_annot_offset[level];
		}

		if (annot) {
			if (gmt_annot_too_crowded (C, xx[i], yy[i], sides[i])) continue;
			if (C->current.proj.three_D && C->current.proj.z_project.cos_az > 0) {	/* Rotate annotation when seen "from North" */
				if (!flip) justify = GMT_flip_justify (C, justify);
				text_angle += 180.0;
			}
			else
				if (flip) justify = GMT_flip_justify (C, justify);
			PSL_plottext (P, xx[i], yy[i], C->current.setting.font_annot[level].size, label, text_angle, justify, form);
		}
	}
}

void gmt_map_symbol_ew (struct GMT_CTRL *C, struct PSL_CTRL *P, double lat, char *label, double west, double east, GMT_LONG annot, GMT_LONG level, GMT_LONG form)
{
	GMT_LONG i, nc;
	struct GMT_XINGS *xings = NULL;

	nc = GMT_map_latcross (C, lat, west, east, &xings);
	for (i = 0; i < nc; i++) gmt_map_symbol (C, P, xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, label, xings[i].nx, 1, annot, level, form);
	if (nc) GMT_free (C, xings);
}

void gmt_map_symbol_ns (struct GMT_CTRL *C, struct PSL_CTRL *P, double lon, char *label, double south, double north, GMT_LONG annot, GMT_LONG level, GMT_LONG form)
{
	GMT_LONG i, nc;
	struct GMT_XINGS *xings = NULL;

	nc = GMT_map_loncross (C, lon, south, north, &xings);
	for (i = 0; i < nc; i++) gmt_map_symbol (C, P, xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, label, xings[i].nx, 0, annot, level, form);
	if (nc) GMT_free (C, xings);
}

void gmt_map_gridlines (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	GMT_LONG k, np, item[2] = {GMT_GRID_UPPER, GMT_GRID_LOWER};
	double dx, dy, *v = NULL;
	char *comment[2] = {"Map gridlines (primary)", "Map gridlines (secondary)"};

	for (k = 0; k < 2; k++) {
		if (C->current.setting.map_grid_cross_size[k] > 0.0) continue;

		dx = GMT_get_map_interval (C, 0, item[k]);
		dy = GMT_get_map_interval (C, 1, item[k]);

		if (! (C->current.map.frame.axis[GMT_X].item[item[k]].active || C->current.map.frame.axis[GMT_Y].item[item[k]].active)) continue;

		PSL_comment (P, "%s\n", comment[k]);

		GMT_setpen (C, &C->current.setting.map_grid_pen[k]);

		if (C->current.map.frame.axis[GMT_X].special == GMT_CUSTOM && (np = gmt_load_custom_annot (C, &C->current.map.frame.axis[GMT_X], 'g', &v, NULL))) {
			gmt_x_grid (C, P, s, n, v, np);
			GMT_free (C, v);
		}
		else if (C->current.proj.xyz_projection[GMT_X] == GMT_TIME && dx > 0.0)
			gmt_timex_grid (C, P, w, e, s, n, item[k]);
		else if (fabs (dx) > 0.0 && C->current.proj.xyz_projection[GMT_X] == GMT_LOG10)
			gmt_logx_grid (C, P, w, e, s, n, dx);
		else if (dx > 0.0 && C->current.proj.xyz_projection[GMT_X] == GMT_POW)
			gmt_powx_grid (C, P, w, e, s, n, dx);
		else if (dx > 0.0)	/* Draw grid lines that go S to N */
			GMT_linearx_grid (C, P, w, e, s, n, dx);

		if (C->current.map.frame.axis[GMT_Y].special == GMT_CUSTOM && (np = gmt_load_custom_annot (C, &C->current.map.frame.axis[GMT_Y], 'g', &v, NULL))) {
			gmt_y_grid (C, P, w, e, v, np);
			GMT_free (C, v);
		}
		if (C->current.proj.xyz_projection[GMT_Y] == GMT_TIME && dy > 0.0)
			gmt_timey_grid (C, P, w, e, s, n, item[k]);
		else if (fabs (dy) > 0.0 && C->current.proj.xyz_projection[GMT_Y] == GMT_LOG10)
			gmt_logy_grid (C, P, w, e, s, n, dy);
		else if (dy > 0.0 && C->current.proj.xyz_projection[GMT_Y] == GMT_POW)
			gmt_powy_grid (C, P, w, e, s, n, dy);
		else if (dy > 0.0)	/* Draw grid lines that go E to W */
			gmt_lineary_grid (C, P, w, e, s, n, dy);

		if (C->current.setting.map_grid_pen[k].style) PSL_setdash (P, CNULL, 0);
	}
}

void gmt_map_gridcross (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	GMT_LONG i, j, k, nx, ny, item[2] = {GMT_GRID_UPPER, GMT_GRID_LOWER};
	double x0, y0, x1, y1, xa, xb, ya, yb, xi, yj, *x = NULL, *y = NULL;
	double x_angle, y_angle, Ca, Sa, L;

	char *comment[2] = {"Map gridcrosses (primary)", "Map gridcrosses (secondary)"};

	for (k = i = 0; k < 2; k++) if (C->current.setting.map_grid_cross_size[k] > 0.0) i++;

	if (i == 0) return;	/* No grid ticks requested */

	GMT_map_clip_on (C, C->session.no_rgb, 3);

	for (k = 0; k < 2; k++) {
		if (C->current.setting.map_grid_cross_size[k] <= 0.0) continue;

		PSL_comment (P, "%s\n", comment[k]);

		GMT_setpen (C, &C->current.setting.map_grid_pen[k]);

		nx = GMT_coordinate_array (C, w, e, &C->current.map.frame.axis[GMT_X].item[item[k]], &x, NULL);
		ny = GMT_coordinate_array (C, s, n, &C->current.map.frame.axis[GMT_Y].item[item[k]], &y, NULL);

		L = 0.5 * C->current.setting.map_grid_cross_size[k];

		for (j = 0; j < ny; j++) {
			for (i = 0; i < nx; i++) {

				if (!GMT_map_outside (C, x[i], y[j])) {	/* Inside map */
					yj = y[j];
					if (GMT_POLE_IS_POINT(C) && GMT_IS_ZERO (fabs (yj) - 90.0)) {	/* Only place one grid cross at the poles for maps where the poles are points */
						xi = C->current.proj.central_meridian;
						i = nx;	/* This ends the loop for this particular latitude */
					}
					else
						xi = x[i];
					GMT_geo_to_xy (C, xi, yj, &x0, &y0);
					if (GMT_is_geographic (C, GMT_IN)) {
						GMT_geo_to_xy (C, xi + C->current.map.dlon, yj, &x1, &y1);
						x_angle = d_atan2 (y1-y0, x1-x0);
						sincos (x_angle, &Sa, &Ca);
						xa = x0 - L * Ca;
						xb = x0 + L * Ca;
						ya = y0 - L * Sa;
						yb = y0 + L * Sa;
					}
					else {
						xa = x0 - L, xb = x0 + L;
						ya = yb = y0;
					}

					/* Clip to map */

					if (xa < 0.0) xa = 0.0;
					if (xb < 0.0) xb = 0.0;
					if (ya < 0.0) ya = 0.0;
					if (yb < 0.0) yb = 0.0;
					if (xa > C->current.map.width) xa = C->current.map.width;
					if (xb > C->current.map.width) xb = C->current.map.width;
					if (ya > C->current.map.height) ya = C->current.map.height;
					if (yb > C->current.map.height) yb = C->current.map.height;

					PSL_plotsegment (P, xa, ya, xb, yb);

					if (GMT_is_geographic (C, GMT_IN)) {
						GMT_geo_to_xy (C, xi, yj - copysign (C->current.map.dlat, yj), &x1, &y1);
						y_angle = d_atan2 (y1-y0, x1-x0);
						sincos (y_angle, &Sa, &Ca);
						xa = x0 - L * Ca;
						xb = x0 + L * Ca;
						ya = y0 - L * Sa;
						yb = y0 + L * Sa;
					}
					else {
						xa = xb = x0;
						ya = y0 - L, yb = y0 + L;
					}

					/* Clip to map */

					if (xa < 0.0) xa = 0.0;
					if (xb < 0.0) xb = 0.0;
					if (ya < 0.0) ya = 0.0;
					if (yb < 0.0) yb = 0.0;
					if (xa > C->current.map.width) xa = C->current.map.width;
					if (xb > C->current.map.width) xb = C->current.map.width;
					if (ya > C->current.map.height) ya = C->current.map.height;
					if (yb > C->current.map.height) yb = C->current.map.height;

					PSL_plotsegment (P, xa, ya, xb, yb);
				}
			}
		}
		if (nx) GMT_free (C, x);
		if (ny) GMT_free (C, y);

		if (C->current.setting.map_grid_pen[k].style) PSL_setdash (P, CNULL, 0);

	}
	GMT_map_clip_off (C);
}

void gmt_map_tickitem (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n, GMT_LONG item)
{
	GMT_LONG i, nx, ny, do_x, do_y;
	double dx, dy, *val = NULL, len;

	if (! (C->current.map.frame.axis[GMT_X].item[item].active || C->current.map.frame.axis[GMT_Y].item[item].active)) return;

	dx = GMT_get_map_interval (C, 0, item);
	dy = GMT_get_map_interval (C, 1, item);

	if (dx <= 0.0 && dy <= 0.0) return;

	do_x = ((dx > 0.0 && item == GMT_ANNOT_UPPER) || (dx > 0.0 && item == GMT_TICK_UPPER && dx != GMT_get_map_interval (C, 0, GMT_ANNOT_UPPER)) || (dx > 0.0 && item == GMT_TICK_LOWER && dx != GMT_get_map_interval (C, 0, GMT_ANNOT_LOWER)));
	do_y = ((dy > 0.0 && item == GMT_ANNOT_UPPER) || (dy > 0.0 && item == GMT_TICK_UPPER && dy != GMT_get_map_interval (C, 1, GMT_ANNOT_UPPER)) || (dy > 0.0 && item == GMT_TICK_LOWER && dy != GMT_get_map_interval (C, 1, GMT_ANNOT_LOWER)));
	len = C->current.setting.map_tick_length;
	if (item == GMT_TICK_UPPER) len *= 0.5;
	if (item == GMT_TICK_LOWER) len *= 0.75;

	C->current.map.on_border_is_outside = TRUE;	/* Temporarily, points on the border are outside */

	if (do_x) {	/* Draw grid lines that go E to W */
		nx = GMT_linear_array (C, w, e, dx, C->current.map.frame.axis[GMT_X].phase, &val);
		for (i = 0; i < nx; i++) gmt_map_lontick (C, P, val[i], s, n, len);
		if (nx) GMT_free (C, val);
	}

	if (do_y) {	/* Draw grid lines that go S to N */
		if (C->current.proj.z_down) {
			ny = GMT_linear_array (C, 0.0, n-s, dy, C->current.map.frame.axis[GMT_Y].phase, &val);
			for (i = 0; i < ny; i++) val[i] = C->common.R.wesn[YHI] - val[i];	/* These are the radial values needed for positioning */
		}
		else
			ny = GMT_linear_array (C, s, n, dy, C->current.map.frame.axis[GMT_Y].phase, &val);
		for (i = 0; i < ny; i++) gmt_map_lattick (C, P, val[i], w, e, len);
		if (ny) GMT_free (C, val);
	}

	C->current.map.on_border_is_outside = FALSE;	/* Reset back to default */
}

void gmt_map_tickmarks (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	/* Tickmarks at annotation interval has already been done except when annotations were not desired */

	if (!(GMT_is_geographic (C, GMT_IN) || C->current.proj.projection == GMT_POLAR)) return;	/* Tickmarks already done by linear axis */

	PSL_comment (P, "Map tickmarks\n");
	GMT_setpen (C, &C->current.setting.map_tick_pen);

	gmt_map_tickitem (C, P, w, e, s, n, GMT_ANNOT_UPPER);
	if (C->current.setting.map_frame_type == GMT_IS_PLAIN) {	/* else we do it with checkerboard b/w patterns */
		gmt_map_tickitem (C, P, w, e, s, n, GMT_TICK_UPPER);
		gmt_map_tickitem (C, P, w, e, s, n, GMT_TICK_LOWER);
	}

	if (C->current.setting.map_tick_pen.style) PSL_setdash (P, CNULL, 0);
}

GMT_LONG gmt_set_do_seconds (struct GMT_CTRL *C, double inc)
{	/* Determines if seconds are to be labelled based on size of increment */
	if (C->current.plot.calclock.geo.order[2] == -1) return (FALSE);			/* Seconds not requested by format */
	if (C->current.plot.calclock.geo.n_sec_decimals > 0) return (TRUE);			/* If asked for ss.xxx annotations */
	if (fabs (60.0 * fmod (fmod (inc, 1.0) * 60.0, 1.0)) >= 1.0) return (TRUE);	/* Multiples of >= 1 sec intervals */
	return (FALSE);
}

void gmt_label_trim (char *label, GMT_LONG stage)
{
	GMT_LONG i;
	if (stage) {	/* Must remove leading stuff for 2ndary annotations */
		for (i = 0; stage && label[i]; i++) if (!isdigit((int)label[i])) stage--;
		while (label[i]) label[stage++] = label[i++];	/* Chop of beginning */
		label[stage] = '\0';
		i = strlen (label) - 1;
		if (strchr ("WESN", label[i])) label[i] = '\0';
	}
}

void gmt_map_annotate (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	GMT_LONG i, k, nx, ny, form, remove[2] = {0,0};
	GMT_LONG do_minutes, do_seconds, done_Greenwich, done_Dateline;
	GMT_LONG full_lat_range, proj_A, proj_B, annot_0_and_360 = FALSE, dual, annot, is_world_save, lon_wrap_save;
	char label[GMT_TEXT_LEN256];
	double *val = NULL, dx[2], dy[2], w2, s2, del;

	if (!(GMT_x_is_lon (C, GMT_IN) || GMT_y_is_lat (C, GMT_IN) || C->current.proj.projection == GMT_POLAR)) return;	/* Annotations and header already done by gmt_linear_map_boundary */

	is_world_save = C->current.map.is_world;
	lon_wrap_save = C->current.map.lon_wrap;

	if (C->current.map.frame.header[0] && !C->current.map.frame.plotted_header) {	/* Make plot header for geographic maps*/
		if (GMT_is_geographic (C, GMT_IN) || C->current.map.frame.side[N_SIDE] == 2) {
			PSL_setfont (P, C->current.setting.font_annot[0].id);
			PSL_command (P, "/PSL_H_y %ld ", psl_iz (P, C->current.setting.map_tick_length + C->current.setting.map_annot_offset[0] + C->current.setting.map_title_offset));
			PSL_deftextdim (P, "-h", C->current.setting.font_annot[0].size, "100\\312");
			PSL_command (P, "add def\n");
		}
		else
			PSL_defunits (P, "PSL_H_y", C->current.setting.map_title_offset + C->current.setting.map_tick_length);

		PSL_command (P, "%ld %ld PSL_H_y add M\n", psl_iz (P, C->current.proj.rect[XHI] * 0.5), psl_iz (P, C->current.proj.rect[YHI]));
		form = GMT_setfont (C, &C->current.setting.font_title);
		PSL_plottext (P, 0.0, 0.0, -C->current.setting.font_title.size, C->current.map.frame.header, 0.0, PSL_BC, form);
		C->current.map.frame.plotted_header = TRUE;
	}

	if (C->current.proj.edge[S_SIDE] || C->current.proj.edge[N_SIDE]) {
		dx[0] = GMT_get_map_interval (C, 0, GMT_ANNOT_UPPER);
		dx[1] = GMT_get_map_interval (C, 0, GMT_ANNOT_LOWER);
		/* Determine if we should annotate both 0 and 360 degrees */

		full_lat_range = (fabs (180.0 - fabs (C->common.R.wesn[YHI] - C->common.R.wesn[YLO])) < GMT_SMALL);
		proj_A = (C->current.proj.projection == GMT_MERCATOR || C->current.proj.projection == GMT_OBLIQUE_MERC ||
			C->current.proj.projection == GMT_WINKEL || C->current.proj.projection == GMT_ECKERT4 || C->current.proj.projection == GMT_ECKERT6 ||
			C->current.proj.projection == GMT_ROBINSON || C->current.proj.projection == GMT_CYL_EQ || C->current.proj.projection == GMT_CYL_STEREO ||
			C->current.proj.projection == GMT_CYL_EQDIST || C->current.proj.projection == GMT_MILLER || C->current.proj.projection == GMT_LINEAR);
		proj_B = (C->current.proj.projection == GMT_HAMMER || C->current.proj.projection == GMT_MOLLWEIDE ||
			C->current.proj.projection == GMT_SINUSOIDAL);
		annot_0_and_360 = (is_world_save && (proj_A || (!full_lat_range && proj_B)));
	}
	else
		dx[0] = dx[1] = 0.0;
	if (C->current.proj.edge[E_SIDE] || C->current.proj.edge[W_SIDE]) {
		dy[0] = GMT_get_map_interval (C, 1, GMT_ANNOT_UPPER);
		dy[1] = GMT_get_map_interval (C, 1, GMT_ANNOT_LOWER);
	}
	else
		dy[0] = dy[1] = 0.0;

	if (dx[0] <= 0.0 && dy[0] <= 0.0) return;

	dual = (dx[1] > 0.0 || dy[1] > 0.0);

	PSL_comment (P, "Map annotations\n");

	form = GMT_setfont (C, &C->current.setting.font_annot[0]);

	C->current.map.on_border_is_outside = TRUE;	/* Temporarily, points on the border are outside */
	if (!C->common.R.oblique) {
		C->current.map.is_world = FALSE;
		if (!(C->current.proj.projection == GMT_GENPER || C->current.proj.projection == GMT_GNOMONIC)) C->current.map.lon_wrap = FALSE;
	}

	w2 = (dx[1] > 0.0) ? floor (w / dx[1]) * dx[1] : 0.0;
	s2 = (dy[1] > 0.0) ? floor (s / dy[1]) * dy[1] : 0.0;

	if (dual) {
		remove[0] = (dx[0] < (1.0/60.0)) ? 2 : 1;
		remove[1] = (dy[0] < (1.0/60.0)) ? 2 : 1;
	}
	for (k = 0; k < 1 + dual; k++) {
		if (dx[k] > 0.0 && (GMT_x_is_lon (C, GMT_IN) || C->current.proj.projection == GMT_POLAR)) {	/* Annotate the S and N boundaries */
			done_Greenwich = done_Dateline = FALSE;
			do_minutes = (fabs (fmod (dx[k], 1.0)) > GMT_SMALL);
			do_seconds = gmt_set_do_seconds (C, dx[k]);

			nx = GMT_linear_array (C, w, e, dx[k], C->current.map.frame.axis[GMT_X].phase, &val);
			for (i = 0; i < nx; i++) {	/* Worry that we do not try to plot 0 and 360 OR -180 and +180 on top of each other */
				if (GMT_IS_ZERO (val[i])) done_Greenwich = TRUE;		/* OK, want to plot 0 */
				if (GMT_IS_ZERO (180.0 + val[i])) done_Dateline = TRUE;	/* OK, want to plot -180 */
				GMT_get_annot_label (C, val[i], label, do_minutes, do_seconds, 0, is_world_save);
				/* Only annotate val[i] if
				 *	(1) projection is such that 0/360 or -180/180 are in different x/y locations, OR
				 *	(2) Plot 360 if 0 hasn't been plotted, OR
				 *	(3) plot +180 if -180 hasn't been plotted
				 */

				annot = annot_0_and_360 || !((done_Greenwich && GMT_IS_ZERO (val[i] - 360.0)) || (done_Dateline && GMT_IS_ZERO (val[i] - 180.0)));
				if (dual && k == 0) {
					del = fmod (val[i] - w2, dx[1]);
					if (GMT_IS_ZERO (del) || GMT_IS_ZERO (del - dx[1]))
						annot = FALSE;
					else
						gmt_label_trim (label, remove[0]);
				}
				gmt_map_symbol_ns (C, P, val[i], label, s, n, annot, k, form);
			}
			if (nx) GMT_free (C, val);
		}

		if (dy[k] > 0.0 && (GMT_y_is_lat (C, GMT_IN) || C->current.proj.projection == GMT_POLAR)) {	/* Annotate W and E boundaries */
			GMT_LONG lonlat;
			double *tval = NULL;

			if (GMT_y_is_lat (C, GMT_IN)) {
				do_minutes = (fabs (fmod (dy[k], 1.0)) > GMT_SMALL);
				do_seconds = gmt_set_do_seconds (C, dy[k]);
				lonlat = 1;
			}
			else {	/* Also, we know that C->current.setting.format_geo_out = -1 in this case */
				do_minutes = do_seconds = 0;
				lonlat = 2;
			}
			if (C->current.proj.z_down) {	/* Want to annotate depth rather than radius */
				ny = GMT_linear_array (C, 0.0, n-s, dy[k], C->current.map.frame.axis[GMT_Y].phase, &tval);
				val = GMT_memory (C, NULL, ny, double);
				for (i = 0; i < ny; i++) val[i] = C->common.R.wesn[YHI] - tval[i];	/* These are the radial values needed for positioning */
			}
			else {				/* Annotate radius */
				ny = GMT_linear_array (C, s, n, dy[k], C->current.map.frame.axis[GMT_Y].phase, &val);
				tval = val;	/* Same thing */
			}
			for (i = 0; i < ny; i++) {
				if ((C->current.proj.polar || C->current.proj.projection == GMT_VANGRINTEN) && GMT_IS_ZERO (fabs (val[i]) - 90.0)) continue;
				GMT_get_annot_label (C, tval[i], label, do_minutes, do_seconds, lonlat, is_world_save);
				annot = TRUE;
				if (dual && k == 0) {
					del = fmod (val[i] - s2, dy[1]);
					if (GMT_IS_ZERO (del) || GMT_IS_ZERO (del - dy[1]))
						annot = FALSE;
					else
						gmt_label_trim (label, remove[1]);
				}
				gmt_map_symbol_ew (C, P, val[i], label, w, e, annot, k, form);
			}
			if (ny) GMT_free (C, val);
			if (C->current.proj.z_down) GMT_free (C, tval);
		}
	}

	C->current.map.on_border_is_outside = FALSE;	/* Reset back to default */
	C->current.map.is_world = is_world_save;
	C->current.map.lon_wrap = lon_wrap_save;
}

void gmt_map_boundary (struct GMT_CTRL *C, struct PSL_CTRL *P, double w, double e, double s, double n)
{
	PSL_comment (P, "Map boundaries\n");

	switch (C->current.proj.projection) {
		case GMT_LINEAR:
			if (GMT_is_geographic (C, GMT_IN))	/* xy is lonlat */
				gmt_fancy_map_boundary (C, P, w, e, s, n);
			else
				gmt_linear_map_boundary (C, P, w, e, s, n);
			break;
		case GMT_POLAR:
			gmt_theta_r_map_boundary (C, P, w, e, s, n);
			break;
		case GMT_MERCATOR:
		case GMT_CYL_EQ:
		case GMT_CYL_EQDIST:
		case GMT_CYL_STEREO:
		case GMT_MILLER:
			gmt_fancy_map_boundary (C, P, w, e, s, n);
			break;
		case GMT_ALBERS:
		case GMT_ECONIC:
		case GMT_LAMBERT:
		case GMT_POLYCONIC:
			gmt_conic_map_boundary (C, P, w, e, s, n);
			break;
		case GMT_OBLIQUE_MERC:
			gmt_oblmrc_map_boundary (C, P, w, e, s, n);
			break;
		case GMT_GENPER:
			gmt_genper_map_boundary (C, P, w, e, s, n);
			break;
		case GMT_STEREO:
		case GMT_ORTHO:
		case GMT_LAMB_AZ_EQ:
		case GMT_AZ_EQDIST:
		case GMT_GNOMONIC:
			if (C->current.proj.polar)
				gmt_polar_map_boundary (C, P, w, e, s, n);
			else
				gmt_circle_map_boundary (C, P, w, e, s, n);
			break;
		case GMT_HAMMER:
		case GMT_MOLLWEIDE:
		case GMT_SINUSOIDAL:
			gmt_ellipse_map_boundary (C, P, w, e, s, n);
			break;
		case GMT_TM:
		case GMT_UTM:
		case GMT_CASSINI:
		case GMT_WINKEL:
		case GMT_ECKERT4:
		case GMT_ECKERT6:
		case GMT_ROBINSON:
		case GMT_VANGRINTEN:
			gmt_basic_map_boundary (C, P, w, e, s, n);
			break;
	}
}


/* GMT_map_basemap will create a basemap for the given area.
 * Scaling and wesn are assumed to be passed through the C->current.proj-structure (see GMT_project.h)
 * Tickmark info are passed through the C->current.map.frame-structure
 *
 */

GMT_LONG gmt_is_fancy_boundary (struct GMT_CTRL *C)
{
	switch (C->current.proj.projection) {
		case GMT_LINEAR:
			return (GMT_is_geographic (C, GMT_IN));
			break;
		case GMT_MERCATOR:
		case GMT_CYL_EQ:
		case GMT_CYL_EQDIST:
		case GMT_CYL_STEREO:
		case GMT_MILLER:
			return (TRUE);
			break;
		case GMT_ALBERS:
		case GMT_ECONIC:
		case GMT_LAMBERT:
			return (!C->common.R.oblique);
			break;
		case GMT_STEREO:
		case GMT_ORTHO:
		case GMT_GENPER:
		case GMT_LAMB_AZ_EQ:
		case GMT_AZ_EQDIST:
		case GMT_GNOMONIC:
		case GMT_VANGRINTEN:
			return (C->current.proj.polar);
			break;
		case GMT_POLAR:
		case GMT_OBLIQUE_MERC:
		case GMT_HAMMER:
		case GMT_MOLLWEIDE:
		case GMT_SINUSOIDAL:
		case GMT_TM:
		case GMT_UTM:
		case GMT_CASSINI:
		case GMT_WINKEL:
		case GMT_ECKERT4:
		case GMT_ECKERT6:
		case GMT_ROBINSON:
			return (FALSE);
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "Error in gmt_is_fancy_boundary - notify developers\n");
			return (FALSE);
	}
}

void GMT_map_basemap (struct GMT_CTRL *C)
{
	GMT_LONG i, clip_on = FALSE;
	double w, e, s, n;
	struct PSL_CTRL *P = C->PSL;

	if (!C->common.B.active[0] && !C->common.B.active[1]) return;

	PSL_setcolor (P, C->current.setting.map_frame_pen.rgb, PSL_IS_STROKE);

	w = C->common.R.wesn[XLO], e = C->common.R.wesn[XHI], s = C->common.R.wesn[YLO], n = C->common.R.wesn[YHI];

	if (C->current.setting.map_annot_oblique & 2) C->current.map.frame.horizontal = 2;
	if (C->current.map.frame.horizontal == 2) C->current.setting.map_annot_oblique |= 2;
	if (C->current.setting.map_frame_type == GMT_IS_GRAPH && GMT_is_geographic (C, GMT_IN)) C->current.setting.map_frame_type = GMT_IS_PLAIN;
	if (C->current.setting.map_frame_type == GMT_IS_FANCY && !gmt_is_fancy_boundary(C)) C->current.setting.map_frame_type = GMT_IS_PLAIN;

	PSL_comment (P, "Start of basemap\n");

	PSL_setdash (P, CNULL, 0);	/* To ensure no dashed pens are set prior */

	GMT_vertical_axis (C, C->current.plot.mode_3D);

	if (C->current.proj.got_azimuths) l_swap (C->current.map.frame.side[E_SIDE], C->current.map.frame.side[W_SIDE]);	/* Temporary swap to trick justify machinery */

	if (C->current.setting.map_frame_type == GMT_IS_INSIDE) {
		GMT_map_clip_on (C, C->session.no_rgb, 3);	/* Must clip to ensure things are inside */
		clip_on = TRUE;
		C->current.setting.map_frame_type = GMT_IS_PLAIN;
	}

	gmt_map_gridlines (C, P, w, e, s, n);
	gmt_map_gridcross (C, P, w, e, s, n);

	gmt_map_tickmarks (C, P, w, e, s, n);

	gmt_map_annotate (C, P, w, e, s, n);

	if (C->current.proj.got_azimuths) l_swap (C->current.map.frame.side[E_SIDE], C->current.map.frame.side[W_SIDE]);	/* Undo swap */

	gmt_map_boundary (C, P, w, e, s, n);
	if (clip_on) GMT_map_clip_off (C);

	if (C->current.setting.map_frame_pen.style) PSL_setdash (P, CNULL, 0);

	PSL_comment (P, "End of basemap\n");

	for (i = 0; i < 4; i++) {
		GMT_free (C, GMT_x_annotation[i]);
		GMT_free (C, GMT_y_annotation[i]);
	}

	PSL_setcolor (P, C->current.setting.map_default_pen.rgb, PSL_IS_STROKE);
}

void gmt_vertical_wall (struct GMT_CTRL *C, struct PSL_CTRL *P, GMT_LONG quadrant, double *nesw)
{
	GMT_plane_perspective (C, (quadrant + 1) % 2, nesw[quadrant % 4]);
	PSL_plotbox (P, nesw[(quadrant+1)%4], C->current.proj.zmin, nesw[(quadrant+3)%4], C->current.proj.zmax);
}

void GMT_vertical_axis (struct GMT_CTRL *C, GMT_LONG mode)
{
	/* Mode means: 1 = background walls and title, 2 = foreground walls and axis, 3 = all */
	GMT_LONG fore, back, old_plane, form;
	double nesw[4], old_level, xx, yy, az;
	struct PSL_CTRL *P = C->PSL;

	if (!C->current.proj.three_D || !C->current.map.frame.axis[GMT_Z].item[GMT_ANNOT_UPPER].active) return;

	nesw[0] = C->current.proj.rect[YHI], nesw[1] = C->current.proj.rect[XHI], nesw[2] = C->current.proj.rect[YLO], nesw[3] = C->current.proj.rect[XLO];

	fore = mode & 2, back = mode & 1;

	/* Since this routine messes with the perspective, we save the state first, then restore later */
	old_plane = C->current.proj.z_project.plane;
	old_level = C->current.proj.z_project.level;

	/* Vertical walls */

	if (C->current.map.frame.draw_box) {
		PSL_setfill (P, C->session.no_rgb, TRUE);
		if (fore) {
			gmt_vertical_wall (C, P, C->current.proj.z_project.quadrant + 3, nesw);
			gmt_vertical_wall (C, P, C->current.proj.z_project.quadrant    , nesw);
		}
		if (back) {
			gmt_vertical_wall (C, P, C->current.proj.z_project.quadrant + 1, nesw);
			gmt_vertical_wall (C, P, C->current.proj.z_project.quadrant + 2, nesw);
		}
	}

	/* Vertical axis */

	if (fore && C->current.map.frame.side[Z_SIDE]) {
		GMT_plane_perspective (C, -1, 0.0);
		GMT_xyz_to_xy (C, nesw[(C->current.proj.z_project.quadrant/2*2+1)%4],
			nesw[((C->current.proj.z_project.quadrant+1)/2*2)%4], C->common.R.wesn[ZLO], &xx, &yy);
		/* Restrict reduced azimuth to -45 to 45 range */
		az = C->current.proj.z_project.view_azimuth - 90.0 - floor ((C->current.proj.z_project.view_azimuth - 45.0) / 90.0) * 90.0;
		PSL_command (P, "/PSL_GPP matrix currentmatrix def [%g %g %g %g %g %g] concat\n",
			cosd(az), sind(az) * C->current.proj.z_project.sin_el, 0.0, C->current.proj.z_project.cos_el, xx * P->internal.x2ix, yy * P->internal.y2iy);
		GMT_xy_axis (C, 0.0, -C->common.R.wesn[ZLO], C->current.proj.zmax - C->current.proj.zmin, C->common.R.wesn[ZLO],
			C->common.R.wesn[ZHI], &C->current.map.frame.axis[GMT_Z], TRUE, C->current.map.frame.side[Z_SIDE] & 2);
		PSL_command (P, "PSL_GPP setmatrix\n");
	}

	/* Title */

	if (back && C->current.map.frame.header[0] && !C->current.map.frame.plotted_header) {	/* No header today */
		GMT_plane_perspective (C, -1, 0.0);
		form = GMT_setfont (C, &C->current.setting.font_title);
		PSL_plottext (P, 0.5 * (C->current.proj.z_project.xmin + C->current.proj.z_project.xmax),
			C->current.proj.z_project.ymax + C->current.setting.map_title_offset,
			C->current.setting.font_title.size, C->current.map.frame.header, 0.0, -2, form);
		C->current.map.frame.plotted_header = TRUE;
	}

	GMT_plane_perspective (C, old_plane, old_level);
}

void GMT_map_clip_on (struct GMT_CTRL *C, double rgb[], GMT_LONG flag)
{
	/* This function sets up a clip path so that only plotting
	 * inside the map area will be drawn on paper. map_setup
	 * must have been called first.  If r >= 0, the map area will
	 * first be painted in the r,g,b colors specified.  flag can
	 * be 0-3, as described in PSL_beginclipping().
	 */

	GMT_LONG np, donut;
	double *work_x = NULL, *work_y = NULL;
	struct PSL_CTRL *P = C->PSL;
	
	np = GMT_map_clip_path (C, &work_x, &work_y, &donut);

	PSL_comment (P, "Activate Map clip path\n");
	if (donut) {
		PSL_beginclipping (P, work_x, work_y, np, rgb, 1);
		PSL_beginclipping (P, &work_x[np], &work_y[np], np, rgb, 2);
	}
	else
		PSL_beginclipping (P, work_x, work_y, np, rgb, flag);

	GMT_free (C, work_x);
	GMT_free (C, work_y);
}

void GMT_map_clip_off (struct GMT_CTRL *C)
{
	/* Restores the original clipping path for the plot */

	PSL_comment (C->PSL, "Deactivate Map clip path\n");
	PSL_endclipping (C->PSL, 1);		/* Reduce polygon clipping by one level */
}

void GMT_setfill (struct GMT_CTRL *C, struct GMT_FILL *fill, GMT_LONG outline)
{
	struct PSL_CTRL *P = C->PSL;
	if (!fill) /* NO fill pointer = no fill */
		PSL_setfill (P, C->session.no_rgb, outline);
	else if (fill->use_pattern) {
		/* Fill with a pattern */
		double rgb[4] = {-3.0, -3.0, -3.0, 0.0};
		rgb[1] = (double)PSL_setpattern (P, fill->pattern_no, fill->pattern, fill->dpi, fill->f_rgb, fill->b_rgb);
		PSL_setfill (P, rgb, outline);
	}
	else	/* Fill with a color */
		PSL_setfill (P, fill->rgb, outline);
}

GMT_LONG GMT_setfont (struct GMT_CTRL *C, struct GMT_FONT *F)
{	/* Set all attributes of the selected font in the PS */
	GMT_LONG outline;
	
	PSL_setfont (C->PSL, F->id);	/* Set the current font ID */
	if (F->form & 2) {	/* Outline font requested; set pen and fill (rgb[0] == -1 means no fill) */
		GMT_setpen (C, &F->pen);		/* Stroke the text outline with this pen */
		GMT_setfill (C, &F->fill, TRUE);	/* Use this color or pattern (if any) for the text fill */
		outline = 1;	/* Indicates outline font is needed and should be stroked */
	}
	else if (F->form & 4) {	/* Want to use a pattern to fill the text but do not draw outline */
		GMT_setfill (C, &F->fill, FALSE);
		outline = 2;	/* Indicates outline font is needed for filling but will not be stroked */
	}
	else {	/* Regular, solid text fill is set via stroke color */
		PSL_setcolor (C->PSL, F->fill.rgb, PSL_IS_FONT);
		outline = 0;	/* Indicates we will fill text using "show" which takes current color (i.e., stroke color) */
	}
	return (outline);
}

void gmt_timestamp (struct GMT_CTRL *C, struct PSL_CTRL *P, double x, double y, GMT_LONG justify, char *U_label)
{
	/* x, y = location of the time stamp box
	 * justify indicates the corner of the box that (x,y) refers to, see below
	 * U_label = label to be plotted to the right of the box
	 *
	 *   9        10       11
	 *   |------------------|
	 *   5         6        7
	 *   |------------------|
	 *   1         2        3
	 */

	time_t right_now;
	char label[GMT_TEXT_LEN256], text[GMT_TEXT_LEN256];
	double dim[3] = {0.365, 0.15, 0.032};	/* Predefined dimensions in inches */
	double unset_rgb[4] = {-1.0, -1.0, -1.0, 0.0};

	/* Plot time string in format defined by format_time_logo */

	right_now = time ((time_t *)0);
	strftime (text, (size_t)sizeof(text), C->current.setting.format_time_logo, localtime (&right_now));
	sprintf (label, "  %s  ", text);

	PSL_command (P, "%% Begin GMT time-stamp\nV\n");
	PSL_setorigin (P, x, y, 0.0, PSL_FWD);
	PSL_setlinewidth (P, 0.25);
	PSL_setfont (P, C->current.setting.font_logo.id);
	PSL_defunits (P, "PSL_g_w", dim[0]);	/* Size of the black [GMT] box */
	PSL_defunits (P, "PSL_g_h", dim[1]);
	PSL_deftextdim (P, "PSL_b", 8.0, label);	/* Size of the white [timestamp] box (use only length) */

	/* When justification is not BL (justify == 1), add some PostScript code to move to the
	   location where the lower left corner of the time stamp box is to be drawn */

	switch ((justify + 3) % 4) {
		case 1:	/* Center */
			PSL_command (P, "PSL_g_w PSL_b_w add 2 div neg 0 T\n"); break;
		case 2:	/* Right justify */
			PSL_command (P, "PSL_g_w PSL_b_w add neg 0 T\n"); break;
	}
	switch (justify / 4) {
		case 1: /* Middle */
			PSL_command (P, "0 PSL_g_h 2 div neg T\n"); break;
		case 2: /* Top justify */
			PSL_command (P, "0 PSL_g_h neg T\n"); break;
	}

	/* Now draw black box with GMT logo, and white box with time stamp */

	PSL_setfill (P, C->current.setting.map_default_pen.rgb, TRUE);
	PSL_plotsymbol (P, 0.5*dim[0], 0.5*dim[1], dim, PSL_RECT);
	PSL_plotcolorimage (P, 0.0, 0.0, dim[0], dim[1], PSL_BL, GMT_glyph, 220, 90, 1);
	PSL_setfill (P, C->PSL->init.page_rgb, TRUE);
	PSL_command (P, "PSL_g_h PSL_b_w PSL_g_w 0 Sb\n");
	PSL_plottext (P, dim[0], dim[2], 8.0, label, 0.0, 1, 0);

	/* Optionally, add additional label to the right of the box */

	if (U_label && U_label[0]) {
		sprintf (label, "   %s", U_label);
		PSL_plottext (P, 0.0, 0.0, -7.0, label, 0.0, 1, 0);
	}

	PSL_command (P, "U\n%% End GMT time-stamp\n");

	/* Reset fill style to empty and no outline and reset linewidth */
	PSL_setfill (P, unset_rgb, FALSE);
	P->current.linewidth = -1.0;
}

void gmt_echo_command (struct GMT_CTRL *C, struct PSL_CTRL *P, struct GMT_OPTION *options)
{
	/* This routine will echo the command and its arguments to the
	 * PostScript output file so that the user can see what scales
	 * etc was used to produce this plot
	 */
	GMT_LONG length = 0;
	char outstring[GMT_BUFSIZ];
	struct GMT_OPTION *opt = NULL;

	GMT_memset (outstring, GMT_BUFSIZ, char);
	PSL_command (P, "\n%% PostScript produced by:\n%%%%GMT:  %s", C->init.progname);
	for (opt = options; opt; opt = opt->next) {
		if (length >= 120) {
			PSL_command (P, "%s \\\n%%%%GMT:+", outstring);
			length = 0;
			GMT_memset (outstring, GMT_BUFSIZ, char);
		}
		strcat (outstring, " ");	length++;
		if (!(opt->option == GMTAPI_OPT_INFILE || opt->option == GMTAPI_OPT_OUTFILE)) {
			outstring[length++] = '-';
			outstring[length++] = opt->option;
		}
		strcat (outstring, opt->arg);
		length += strlen (opt->arg);
	}
	PSL_command (P, "%s\n", outstring);
}

void gmt_NaN_pen_up (double x[], double y[], int pen[], GMT_LONG n)
{
	/* Ensure that if there are NaNs we set pen = PSL_MOVE */

	GMT_LONG i, n1;

	for (i = 0, n1 = n - 1; i < n; i++) {
		if (GMT_is_dnan (x[i]) || GMT_is_dnan (y[i])) {
			pen[i] = PSL_MOVE;
			if (i < n1) pen[i+1] = PSL_MOVE;	/* Since the next point must become the new anchor */
		}
	}
}

void GMT_plot_line (struct GMT_CTRL *C, double *x, double *y, int *pen, GMT_LONG n)
{
	GMT_LONG i, j, i1, way, stop, close;
	double x_cross[2], y_cross[2];
	struct PSL_CTRL *P = C->PSL;

	if (n < 2) return;

	gmt_NaN_pen_up (x, y, pen, n);	/* Ensure we dont have NaNs in the coordinates */

	i = 0;
	while (i < (n-1) && pen[i+1] == PSL_MOVE) i++;	/* Skip repeating pen == PSL_MOVE in beginning */
	if ((n-i) < 2) return;
	while (n > 1 && pen[n-1] == PSL_MOVE) n--;	/* Cut off repeating pen == PSL_MOVE at end */
	if ((n-i) < 2) return;

	for (j = i + 1; j < n && pen[j] == PSL_DRAW; j++);	/* j == n means no PSL_MOVEs present */
	close = (j == n) ? (hypot (x[n-1] - x[i], y[n-1] - y[i]) < GMT_SMALL) : FALSE;

	/* First see if we can use the PSL_plotline call directly to save points */

	for (j = i + 1, stop = FALSE; !stop && j < n; j++) stop = (pen[j] == PSL_MOVE || (*C->current.map.jump) (C, x[j-1], y[j-1], x[j], y[j]));
	if (!stop) {
		PSL_plotline (P, &x[i], &y[i], n - i, PSL_MOVE + PSL_STROKE + close * PSL_CLOSE);
		return;
	}

	/* Here we must check for jumps, pen changes etc */

	PSL_plotpoint (P, x[i], y[i], pen[i]);

	i++;
	while (i < n) {
		i1 = i - 1;
		if (pen[i] == pen[i1] && (way = (*C->current.map.jump) (C, x[i1], y[i1], x[i], y[i]))) {	/* Jumped across the map */
			(*C->current.map.get_crossings) (C, x_cross, y_cross, x[i1], y[i1], x[i], y[i]);
			if (way == -1) {	/* Add left border point */
				PSL_plotpoint (P, x_cross[0], y_cross[0], PSL_DRAW);	/* Draw to left boundary... */
				PSL_plotpoint (P, x_cross[1], y_cross[1], PSL_MOVE);	/* ...then jump to the right boundary */
			}
			else {
				PSL_plotpoint (P, x_cross[1], y_cross[1], PSL_DRAW);	/* Draw to right boundary... */
				PSL_plotpoint (P, x_cross[0], y_cross[0], PSL_MOVE);	/* ...then jump to the left boundary */
			}
			close = FALSE;
		}
		PSL_plotpoint (P, x[i], y[i], pen[i]);
		i++;
	}
	PSL_command (P, close ? "P S\n" : "S\n");
}

void GMT_draw_map_scale (struct GMT_CTRL *C, struct GMT_MAP_SCALE *ms)
{
	GMT_LONG i, j, jj, unit, form;
	GMT_LONG n_f_ticks[10] = {5, 4, 6, 4, 5, 6, 7, 4, 3, 5};
	GMT_LONG n_a_ticks[10] = {1, 2, 3, 2, 1, 3, 1, 2, 1, 1};
	double dlon, x1, x2, y1, y2, a0, tx, ty, off, f_len, a_len, x_left, x_right, bar_length, x_label, y_label;
	double base, d_base, width, half, bar_width, dx, dx_f, dx_a;
	char txt[GMT_TEXT_LEN256], *this_label = NULL;
	char *label[5] = {"km", "miles", "nautical miles", "m", "feet"}, *units[5] = {"km", "mi", "nm", "m", "ft"};
	struct PSL_CTRL *P = C->PSL;
	
	if (!ms->plot) return;

	if (!GMT_is_geographic (C, GMT_IN)) return;	/* Only for geographic projections */

	switch (ms->measure) {	/* Convert bar_length to km */
#ifdef GMT_COMPAT
		case 'm':
			GMT_report (C, GMT_MSG_COMPAT, "Warning: Distance unit m is deprecated; use M for statute miles\n");
#endif
		case 'M':	/* Statute miles instead */
			unit = 1;
			bar_length = 0.001 * METERS_IN_A_MILE * ms->length;
			break;
		case 'n':	/* Nautical miles instead */
			unit = 2;
			bar_length = 0.001 * METERS_IN_A_NAUTICAL_MILE * ms->length;
			break;
		case 'e':	/* meters instead */
			unit = 3;
			bar_length = 0.001 * ms->length;
			break;
		case 'f':	/* feet instead */
			unit = 4;
			bar_length = 0.001 * METERS_IN_A_FOOT * ms->length;
			break;
		default:	/* Default (or k) is km */
			unit = 0;
			bar_length = ms->length;
			break;
	}

	if (ms->gave_xy)	/* Also get lon/lat coordinates */
		GMT_xy_to_geo (C, &ms->lon, &ms->lat, ms->x0, ms->y0);
	else {	/* Must convert lon/lat to location on map */
		ms->lon = ms->x0;
		ms->lat = ms->y0;
		GMT_geo_to_xy (C, ms->lon, ms->lat, &ms->x0, &ms->y0);
	}

	if (C->current.proj.projection == GMT_OBLIQUE_MERC) {	/* Set latitude to the oblique latitude */
		a0 = fabs (GMT_great_circle_dist_degree (C, C->current.proj.o_pole_lon, C->current.proj.o_pole_lat, ms->scale_lon, ms->scale_lat));	/* Colatitude */
		if (a0 > 90.0) a0 = 180.0 - 90.0;	/* Flip hemisphere */
		ms->scale_lat = 90.0 - a0;
	}

	/* Get longitudinal degree length corresponding to this km length at the latitude of scale */
	dlon = 0.5 * bar_length / (C->current.proj.DIST_KM_PR_DEG * cosd (ms->scale_lat));

	GMT_geo_to_xy (C, C->current.proj.central_meridian - dlon, ms->scale_lat, &x1, &y1);
	GMT_geo_to_xy (C, C->current.proj.central_meridian + dlon, ms->scale_lat, &x2, &y2);
	width = hypot (x2 - x1, y2 - y1);
	half = 0.5 * width;
	a_len = fabs (C->current.setting.map_scale_height);
	off = a_len + 0.75 * C->current.setting.map_annot_offset[0];
	x_left  = ms->x0 - half;
	x_right = ms->x0 + half;

	if (ms->fancy) {	/* Fancy scale */
		j = irint (floor (d_log10 (C, ms->length / 0.95)));
		base = pow (10.0, (double)j);
		i = irint (ms->length / base) - 1;
		d_base = ms->length / n_a_ticks[i];
		dx_f = width / n_f_ticks[i];
		dx_a = width / n_a_ticks[i];
		bar_width = 0.5 * fabs (C->current.setting.map_scale_height);
		f_len = 0.75 * fabs (C->current.setting.map_scale_height);
		if (ms->boxdraw || ms->boxfill) {	/* Draw a rectangle beneath the scale */
			dx = fabs (0.5 * j * 0.4 * (C->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH));
			if (ms->boxdraw) GMT_setpen (C, &ms->pen);
			GMT_setfill (C, &ms->fill, ms->boxdraw);
			PSL_plotbox (P, x_left - 2.0 * C->current.setting.map_annot_offset[0] - dx,
				ms->y0 - 1.5 * a_len - C->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH - ((ms->justify == 'b') ?  fabs(C->current.setting.map_label_offset) + 0.85 * C->current.setting.font_label.size / PSL_POINTS_PER_INCH: 0.0),
				x_right + 2.0 * C->current.setting.map_annot_offset[0] + dx,
				ms->y0 + 1.5 * a_len + ((ms->justify == 't') ? fabs(C->current.setting.map_label_offset) + C->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH : 0.0));
		}
		GMT_setpen (C, &C->current.setting.map_tick_pen);
		PSL_plotsegment (P, x_left, ms->y0 - f_len, x_left, ms->y0);
		for (j = 0; j < n_f_ticks[i]; j++) {
			PSL_setfill (P, (j%2) ? C->PSL->init.page_rgb : C->current.setting.map_default_pen.rgb, TRUE);
			PSL_plotbox (P, x_left + j * dx_f, ms->y0, x_left + (j+1) * dx_f, ms->y0 - bar_width);
			PSL_plotsegment (P, x_left + (j+1) * dx_f, ms->y0 - f_len, x_left + (j+1) * dx_f, ms->y0);
		}
		ty = ms->y0 - off;
		form = GMT_setfont (C, &C->current.setting.font_annot[0]);
		for (j = 0; j <= n_a_ticks[i]; j++) {
			if (ms->unit)
				sprintf (txt, "%g %s", j * d_base, units[unit]);
			else
				sprintf (txt, "%g", j * d_base);
			tx = x_left + j * dx_a;
			PSL_plotsegment (P, tx, ms->y0 - a_len, tx, ms->y0);
			PSL_plottext (P, tx, ty, C->current.setting.font_annot[0].size, txt, 0.0, PSL_TC, form);
		}
		switch (ms->justify) {
			case 'l':	/* Left */
				x_label = x_left - f_len;
				y_label = ms->y0 - a_len;
				jj = PSL_BR;	/* XXX ARE THESE RIGHT ? XXX */
				break;
			case 'r':	/* right */
				x_label = x_right + f_len;
				y_label = ms->y0 - a_len;
				jj = PSL_BL;
				break;
			case 't':	/* top */
				x_label = ms->x0;
				y_label = ms->y0 + fabs(C->current.setting.map_label_offset);
				jj = PSL_BC;
				break;
			default:	/* bottom */
				x_label = ms->x0;
				y_label = ms->y0 - a_len - fabs(C->current.setting.map_label_offset) - 0.85 * C->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH;
				jj = PSL_TC;
				break;
		}
		if (ms->do_label) {
			this_label = (ms->label[0]) ? ms->label : label[unit];
			form = GMT_setfont (C, &C->current.setting.font_label);
			PSL_plottext (P, x_label, y_label, C->current.setting.font_label.size, this_label, 0.0, jj, form);
		}
	}
	else {	/* Simple scale */
		if (ms->boxdraw || ms->boxfill) {	/* Draw a rectangle beneath the scale */
			if (ms->boxdraw) GMT_setpen (C, &ms->pen);
			dx = fabs (0.5 * irint (floor (d_log10 (C, ms->length))) * 0.4 * (C->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH));
			GMT_setfill (C, &ms->fill, ms->boxdraw);
			PSL_plotbox (P, x_left - 2.0 * C->current.setting.map_annot_offset[0] - dx,
				ms->y0 - 1.5 * a_len - C->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH,
				x_right + 2.0 * C->current.setting.map_annot_offset[0] + dx, ms->y0 + 1.5 * a_len);
		}
		GMT_setpen (C, &C->current.setting.map_tick_pen);
		PSL_plotsegment (P, x_left, ms->y0 - C->current.setting.map_scale_height, x_left, ms->y0);
		PSL_plotsegment (P, x_left, ms->y0, x_right, ms->y0);
		PSL_plotsegment (P, x_right, ms->y0, x_right, ms->y0 - C->current.setting.map_scale_height);
		sprintf (txt, "%g %s", ms->length, label[unit]);
		form = GMT_setfont (C, &C->current.setting.font_annot[0]);
		PSL_plottext (P, ms->x0, ms->y0 - off, C->current.setting.font_annot[0].size, txt, 0.0, 10, form);
	}
}

void gmt_Nstar (struct GMT_CTRL *C, struct PSL_CTRL *P, double x0, double y0, double r)
{	/* Draw a fancy 5-pointed North star */
	GMT_LONG a;
	double r2, x[4], y[4], dir, dir2, s, c;

	r2 = r * 0.3;
	for (a = 0; a <= 360; a += 72) {	/* Azimuth of the 5 points on the star */
		/* Solid half */
		x[0] = x[3] = x0, y[0] = y[3] = y0;
		dir = 90.0 - (double)a;
		sincosd (dir, &s, &c);
		x[1] = x0 + r * c;
		y[1] = y0 + r * s;
		dir2 = dir - 36.0;
		sincosd (dir2, &s, &c);
		x[2] = x0 + r2 * c;
		y[2] = y0 + r2 * s;
		PSL_setfill (P, C->current.setting.map_default_pen.rgb, TRUE);
		PSL_plotpolygon (P, x, y, 4);
		/* Hollow half */
		x[0] = x[3] = x0, y[0] = y[3] = y0;
		sincosd (dir, &s, &c);
		x[1] = x0 + r * c;
		y[1] = y0 + r * s;
		dir2 = dir + 36.0;
		sincosd (dir2, &s, &c);
		x[2] = x0 + r2 * c;
		y[2] = y0 + r2 * s;
		PSL_setfill (P, C->PSL->init.page_rgb, TRUE);
		PSL_plotpolygon (P, x, y, 4);
	}
}

#define M_VW	0.005
#define M_HL	0.075
#define M_HW	0.015

#define DIST_TO_2ND_POINT 1.0

#define F_VW	0.01
#define F_HL	0.15
#define F_HW	0.05

void gmt_draw_mag_rose (struct GMT_CTRL *C, struct PSL_CTRL *P, struct GMT_MAP_ROSE *mr)
{	/* Magnetic compass rose */
	GMT_LONG i, k, level, just, ljust[4] = {10, 5, 2, 7}, n_tick, form;
	double ew_angle, angle, R[2], tlen[3], L, s, c, x[5], y[5], xp[5], yp[5], offset, t_angle, scale[2], base, *val = NULL, dim[7];
	char label[16];
	struct GMT_FILL f;

	/* Initialize fill structure */
	GMT_init_fill (C, &f, C->current.setting.map_default_pen.rgb[0], C->current.setting.map_default_pen.rgb[1], C->current.setting.map_default_pen.rgb[2]);

	GMT_azim_to_angle (C, mr->lon, mr->lat, DIST_TO_2ND_POINT, 90.0, &ew_angle);	/* Get angle of E-W direction at this location */

	R[0] = 0.75 * 0.5 * mr->size;
	R[1] = 0.5 * mr->size;
	tlen[0] = 0.5 * C->current.setting.map_tick_length;
	tlen[1] = C->current.setting.map_tick_length;;
	tlen[2] = 1.5 * C->current.setting.map_tick_length;
	scale[0] = 0.85;
	scale[1] = 1.0;
	C->current.plot.r_theta_annot = FALSE;	/* Just in case it was turned on in gmt_map.c */

	for (level = 0; level < 2; level++) {	/* Outer and inner angles */
		if (level == 0 && mr->kind == 1) continue;	/* Sorry, not magnetic directions */
		offset = (level == 0) ? mr->declination : 0.0;
		GMT_setpen (C, &C->current.setting.map_tick_pen);
		n_tick = GMT_linear_array (C, 0.0, 360.0, mr->g_int[level], 0.0, &val);
		for (i = 0; i < n_tick - 1; i++) {	/* Increments of fine tickmarks (-1 to avoid repeating 360) */
			angle = offset + val[i];
			k = (GMT_IS_ZERO (fmod (val[i], mr->a_int[level]))) ? 2 : ((GMT_IS_ZERO (fmod (val[i], mr->f_int[level]))) ? 1 : 0);
			sincosd (ew_angle + angle, &s, &c);
			x[0] = mr->x0 + R[level] * c, y[0] = mr->y0 + R[level] * s;
			x[1] = mr->x0 + (R[level] - scale[level]*tlen[k]) * c, y[1] = mr->y0 + (R[level] - scale[level]*tlen[k]) * s;
			PSL_plotsegment (P, x[0], y[0], x[1], y[1]);
		}
		GMT_free (C, val);

		form = GMT_setfont (C, &C->current.setting.font_annot[level]);
		n_tick = GMT_linear_array (C, 0.0, 360.0, mr->a_int[level], 0.0, &val);
		for (i = 0; i < n_tick - 1; i++) {	/* Increments of annotations (-1 to avoid repeating 360) */
			angle = 90.0 - (offset + val[i]);	/* Since val is azimuth */
			sincosd (ew_angle + angle, &s, &c);
			x[0] = mr->x0 + (R[level] + C->current.setting.map_annot_offset[level]) * c, y[0] = mr->y0 + (R[level] + C->current.setting.map_annot_offset[level]) * s;
			sprintf (label, "%ld", (GMT_LONG)irint (val[i]));
			t_angle = fmod ((double)(-val[i] - offset) + 360.0, 360.0);	/* Now in 0-360 range */
			if (t_angle > 180.0) t_angle -= 180.0;	/* Now in -180/180 range */
			if (t_angle > 90.0 || t_angle < -90.0) t_angle -= copysign (180.0, t_angle);
			just = (y[0] <= mr->y0) ? 10 : 2;
			if (level == 1 && GMT_IS_ZERO (val[i]-90.0)) t_angle = -90.0, just = 2;
			if (level == 1 && GMT_IS_ZERO (val[i]-270.0)) t_angle = 90.0, just = 2;
			PSL_plottext (P, x[0], y[0], C->current.setting.font_annot[level].size, label, t_angle, just, form);
		}
		GMT_free (C, val);
	}

	/* Draw extra tick for the 4 main compass directions */
	GMT_setpen (C, &C->current.setting.map_tick_pen);
	base = R[1] + C->current.setting.map_annot_offset[1] + C->current.setting.font_annot[1].size / PSL_POINTS_PER_INCH;
	for (i = 0, k = 1; i < 360; i += 90, k++) {	/* 90-degree increments of tickmarks */
		angle = (double)i;
		sincosd (ew_angle + angle, &s, &c);
		x[0] = mr->x0 + R[1] * c, y[0] = mr->y0 + R[1] * s;
		x[1] = mr->x0 + (R[1] + tlen[0]) * c, y[1] = mr->y0 + (R[1] + tlen[0]) * s;
		PSL_plotsegment (P, x[0], y[0], x[1], y[1]);
		if (!mr->label[k][0]) continue;	/* No label desired */
		x[0] = mr->x0 + base * c, y[0] = mr->y0 + base * s;
		x[1] = mr->x0 + (base + 2.0 * tlen[2]) * c, y[1] = mr->y0 + (base + 2.0 * tlen[2]) * s;
		PSL_plotsegment (P, x[0], y[0], x[1], y[1]);
		if (k == 4) k = 0;
		if (k == 2 && mr->label[2][0] == '*') {
			x[0] = mr->x0 + (base + 2.0*tlen[2] + C->current.setting.map_title_offset + 0.025*mr->size) * c, y[0] = mr->y0 + (base + 2.0*tlen[2] + C->current.setting.map_title_offset + 0.025*mr->size) * s;
			gmt_Nstar (C, P, x[0], y[0], 0.1*mr->size);
		}
		else {
			x[0] = mr->x0 + (base + 2.0*tlen[2] + C->current.setting.map_title_offset) * c, y[0] = mr->y0 + (base + 2.0*tlen[2] + C->current.setting.map_title_offset) * s;
			form = GMT_setfont (C, &C->current.setting.font_title);
			PSL_plottext (P, x[0], y[0], C->current.setting.font_title.size, mr->label[k], ew_angle, ljust[k], form);
			GMT_setpen (C, &C->current.setting.map_tick_pen);
		}
	}

	if (mr->kind == 2) {	/* Compass needle and label */
		sincosd (ew_angle + (90.0 - mr->declination), &s, &c);
		L = R[0] - 2.0 * tlen[2];
		x[0] = mr->x0 - L * c, y[0] = mr->y0 - L * s;
		x[1] = mr->x0 + L * c, y[1] = mr->y0 + L * s;
		dim[0] = x[1], dim[1] = y[1],
		dim[2] = M_VW * mr->size, dim[3] = M_HL * mr->size, dim[4] = M_HW * mr->size,
		dim[5] = C->current.setting.map_vector_shape, dim[6] = 0.0;
		GMT_setfill (C, &f, TRUE);
		PSL_plotsymbol (P, x[0], y[0], dim, PSL_VECTOR);
		t_angle = fmod (ew_angle + 90.0 - mr->declination + 360.0, 360.0);	/* Now in 0-360 range */
		if (fabs (t_angle) > 90.0) t_angle -= copysign (180.0, t_angle);
		sincosd (t_angle, &s, &c);
		x[0] = mr->x0 - 2.0 * M_VW * mr->size * s, y[0] = mr->y0 + 2.0 * M_VW * mr->size * c;
		if (!strcmp(mr->dlabel, "-")) GMT_get_annot_label (C, mr->declination, mr->dlabel, TRUE, FALSE, 0, C->current.map.is_world);
		form = GMT_setfont (C, &C->current.setting.font_label);
		PSL_plottext (P, x[0], y[0], C->current.setting.font_label.size, mr->dlabel, t_angle, 2, form);
	}
	else {			/* Just geographic directions and a centered arrow */
		L = mr->size - 4.0*tlen[2];
		x[0] = x[1] = x[4] = 0.0,	x[2] = -0.25 * mr->size,		x[3] = -x[2];
		y[0] = -0.5 * L,		y[1] = -y[0], y[2] = y[3] = 0.0,	y[4] = y[1] + C->current.setting.map_annot_offset[0];
		GMT_rotate2D (C, x, y, 5, mr->x0, mr->y0, ew_angle, xp, yp);	/* Coordinate transformation and placement of the 4 labels */
		dim[0] = xp[1], dim[1] = yp[1];
		dim[2] = F_VW * mr->size, dim[3] = F_HL * mr->size, dim[4] = F_HW * mr->size;
		dim[5] = C->current.setting.map_vector_shape, dim[6] = 0.0;
		GMT_setfill (C, &f, TRUE);
		PSL_plotsymbol (P, xp[0], yp[0], dim, PSL_VECTOR);
		s = 0.25 * mr->size;
		GMT_init_fill (C, &f, -1.0, -1.0, -1.0);
		GMT_setfill (C, &f, TRUE);
		PSL_plotsymbol (P, mr->x0, mr->y0, &s, PSL_CIRCLE);
		PSL_plotsegment (P, xp[2], yp[2], xp[3], yp[3]);
	}
}

/* These are used to scale the plain arrow given rose size */

#define ROSE_LENGTH_SCL1	(0.5 * M_SQRT2)
#define ROSE_LENGTH_SCL2	0.5
#define ROSE_WIDTH_SCL1		0.2
#define ROSE_WIDTH_SCL2		0.2
#define ROSE_WIDTH_SCL3		0.2

void gmt_draw_dir_rose (struct GMT_CTRL *C, struct PSL_CTRL *P, struct GMT_MAP_ROSE *mr)
{
	GMT_LONG i, kind, form, just[4] = {10, 5, 2, 7};
	double angle, L[4], R[4], x[8], y[8], xp[8], yp[8], tx[3], ty[3], s, c, rot[4] = {0.0, 45.0, 22.5, -22.5};
	struct GMT_FILL f;

	/* Initialize fill structure */
	GMT_init_fill (C, &f, C->current.setting.map_default_pen.rgb[0], C->current.setting.map_default_pen.rgb[1], C->current.setting.map_default_pen.rgb[2]);

	GMT_azim_to_angle (C, mr->lon, mr->lat, DIST_TO_2ND_POINT, 90.0, &angle);	/* Get angle of E-W direction at this location */

	GMT_setpen (C, &C->current.setting.map_tick_pen);

	if (mr->fancy) {	/* Fancy scale */
		mr->size *= 0.5;	/* Got diameter, use radius for calculations */
		L[0] = mr->size;
		L[1] = ROSE_LENGTH_SCL1 * mr->size;
		L[2] = L[3] = ROSE_LENGTH_SCL2 * mr->size;
		R[0] = ROSE_WIDTH_SCL1 * mr->size;
		R[1] = ROSE_WIDTH_SCL2 * mr->size;
		R[2] = R[3] = ROSE_WIDTH_SCL3 * mr->size;
		mr->kind--;	/* Turn 1-3 into 0-2 */
		if (mr->kind == 2) mr->kind = 3;	/* Trick so that we can draw 8 rather than 4 points */
		for (kind = mr->kind; kind >= 0; kind--) {
			/* Do 4 blades 90 degrees apart, aligned with main axes & relative to (0,0) */
			x[0] = L[kind], x[1] = x[7] = 0.5 * M_SQRT2 * R[kind], x[2] = x[6] = 0.0;
			y[0] = y[4] = 0.0, y[1] = y[3] = 0.5 * M_SQRT2 * R[kind], y[2] = L[kind];
			x[3] = x[5] = -x[1], x[4] = -x[0];
			y[5] = y[7] = -y[1], y[6] = -y[2];
			GMT_rotate2D (C, x, y, 8, mr->x0, mr->y0, rot[kind] + angle, xp, yp);	/* Coordinate transformation and placement of the 4 labels */
			PSL_setfill (P, C->PSL->init.page_rgb, TRUE);
			PSL_plotpolygon (P, xp, yp, 8);	/* Outline of 4-pointed star */
			tx[0] = mr->x0, ty[0] = mr->y0;
			/* Fill positive halfs of the 4-pointed blades */
			PSL_setfill (P, C->current.setting.map_default_pen.rgb, TRUE);
			tx[1] = xp[0], ty[1] = yp[0], tx[2] = xp[7], ty[2] = yp[7];
			PSL_plotpolygon (P, tx, ty, 3);	/* East */
			tx[1] = xp[1], ty[1] = yp[1], tx[2] = xp[2], ty[2] = yp[2];
			PSL_plotpolygon (P, tx, ty, 3);	/* North */
			tx[1] = xp[3], ty[1] = yp[3], tx[2] = xp[4], ty[2] = yp[4];
			PSL_plotpolygon (P, tx, ty, 3);	/* West */
			tx[1] = xp[5], ty[1] = yp[5], tx[2] = xp[6], ty[2] = yp[6];
			PSL_plotpolygon (P, tx, ty, 3);	/* South */
		}
		sincosd (angle, &s, &c);
		x[0] = x[2] = 0.0, x[1] = L[0] + C->current.setting.map_title_offset; x[3] = -x[1];
		y[1] = y[3] = 0.0, y[2] = L[0] + C->current.setting.map_title_offset; y[0] = -y[2];
		GMT_rotate2D (C, x, y, 4, mr->x0, mr->y0, angle, xp, yp);	/* Coordinate transformation and placement of the 4 labels */
		form = GMT_setfont (C, &C->current.setting.font_title);
		for (i = 0; i < 4; i++) PSL_plottext (P, xp[i], yp[i], C->current.setting.font_title.size, mr->label[i], angle, just[i], form);
	}
	else {			/* Plain North arrow w/circle */
		sincosd (angle, &s, &c);
		x[0] = x[1] = x[4] = 0.0, x[2] = -0.25 * mr->size, x[3] = -x[2];
		y[0] = -0.5 * mr->size, y[1] = -y[0], y[2] = y[3] = 0.0; y[4] = y[1] + C->current.setting.map_annot_offset[0];
		GMT_rotate2D (C, x, y, 5, mr->x0, mr->y0, angle, xp, yp);	/* Coordinate transformation and placement of the 4 labels */
		x[0] = xp[1], x[1] = yp[1];
		x[2] = F_VW * mr->size, x[3] = F_HL * mr->size, x[4] = F_HW * mr->size;
		x[5] = C->current.setting.map_vector_shape, x[6] = 0.0;
		GMT_setfill (C, &f, TRUE);
		PSL_plotsymbol (P, xp[0], yp[0], x, PSL_VECTOR);
		s = 0.25 * mr->size;
		GMT_init_fill (C, &f, -1.0, -1.0, -1.0);
		GMT_setfill (C, &f, TRUE);
		PSL_plotsymbol (P, mr->x0, mr->y0, &s, PSL_CIRCLE);
		PSL_plotsegment (P, xp[2], yp[2], xp[3], yp[3]);
		form = GMT_setfont (C, &C->current.setting.font_title);
		PSL_plottext (P, xp[4], yp[4], C->current.setting.font_title.size, mr->label[2], angle, 2, form);
	}
}

void GMT_draw_map_rose (struct GMT_CTRL *C, struct GMT_MAP_ROSE *mr)
{
	GMT_LONG tmp_join, tmp_limit;
	struct PSL_CTRL *P = C->PSL;
	if (!mr->plot) return;

	if (!GMT_is_geographic (C, GMT_IN)) return;	/* Only for geographic projections */

	if (mr->gave_xy)	/* Also get lon/lat coordinates */
		GMT_xy_to_geo (C, &mr->lon, &mr->lat, mr->x0, mr->y0);
	else {	/* Must convert lon/lat to location on map */
		mr->lon = mr->x0;
		mr->lat = mr->y0;
		GMT_geo_to_xy (C, mr->lon, mr->lat, &mr->x0, &mr->y0);
	}

	/* Temporarily use miter to get sharp points to compass rose */
	tmp_join = P->internal.line_join;	PSL_setlinejoin (P, 0);
	tmp_limit = P->internal.miter_limit;	PSL_setmiterlimit (P, 0);

	if (mr->fancy == 2)	/* Do magnetic compass rose */
		gmt_draw_mag_rose (C, P, mr);
	else
		gmt_draw_dir_rose (C, P, mr);

	/* Switch line join style back */
	PSL_setlinejoin (P, tmp_join);
	PSL_setmiterlimit (P, tmp_limit);
}

void GMT_setpen (struct GMT_CTRL *C, struct GMT_PEN *pen)
{
	/* GMT_setpen issues PostScript code to set the specified pen. */

	if (!pen) return;
	PSL_setlinewidth (C->PSL, pen->width);
	PSL_setdash (C->PSL, pen->style, pen->offset);
	PSL_setcolor (C->PSL, pen->rgb, PSL_IS_STROKE);
}

GMT_LONG gmt_custum_failed_bool_test (struct GMT_CTRL *C, struct GMT_CUSTOM_SYMBOL_ITEM *s, double size[])
{
	GMT_LONG result;
	/* Perform the boolean comparison and return FALSE if test is TRUE */
	
	switch (s->operator) {
		case '<':	/* < */
			result = (size[s->var] < s->const_val[0]);
			break;
		case 'L':	/* <= */
			result = (size[s->var] <= s->const_val[0]);
			break;
		case '=':	/* == */
			result = (size[s->var] == s->const_val[0]);
			break;
		case 'G':	/* >= */
			result = (size[s->var] >= s->const_val[0]);
			break;
		case '>':	/* > */
			result = (size[s->var] > s->const_val[0]);
			break;
		case '%':	/* % */
			result = (!GMT_IS_ZERO (fmod (size[s->var], s->const_val[0])));
			break;
		case 'I':	/* [] inclusive range */
			result = (size[s->var] >= s->const_val[0] && size[s->var] <= s->const_val[1]);
			break;
		case 'i':	/* <> exclusive range */
			result = (size[s->var] > s->const_val[0] && size[s->var] < s->const_val[1]);
			break;
		case 'l':	/* [> in/ex-clusive range */
			result = (size[s->var] >= s->const_val[0] && size[s->var] < s->const_val[1]);
			break;
		case 'r':	/* <] ex/in-clusive range */
			result = (size[s->var] > s->const_val[0] && size[s->var] <= s->const_val[1]);
			break;
		case 'E':	/* var == NaN */
			result = GMT_is_dnan (size[s->var]);
			break;
		default:
			GMT_report (C, GMT_MSG_FATAL, "Error: Unrecognized symbol macro operator (%ld = '%c') passed to GMT_draw_custom_symbol\n", s->operator, (char)s->operator);
			GMT_exit (EXIT_FAILURE);
			break;
		
	}
	if (s->negate) result = !result;	/* Negate the test since we used a ! operator , e.g., != */
	return (!result);			/* Return the opposite of the test result */
}

void gmt_flush_symbol_piece (struct GMT_CTRL *C, struct PSL_CTRL *P, double *x, double *y, GMT_LONG *n, struct GMT_PEN *p, struct GMT_FILL *f, GMT_LONG outline, GMT_LONG *flush)
{
	GMT_LONG draw_outline;

	draw_outline = (outline && p->rgb[0] != -1) ? TRUE : FALSE;
	if (draw_outline) GMT_setpen (C, p);
	if (draw_outline == 2) {	/* Stroke path only */
		PSL_plotline (P, x, y, *n, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
	}
	else {	/* Fill polygon and possibly stroke outline */
		GMT_setfill (C, f, draw_outline);
		PSL_plotpolygon (P, x, y, *n);
	}
	*flush = FALSE;
	*n = 0;
}

void GMT_draw_custom_symbol (struct GMT_CTRL *C, double x0, double y0, double size[], struct GMT_CUSTOM_SYMBOL *symbol, struct GMT_PEN *pen, struct GMT_FILL *fill, GMT_LONG outline)
{
	GMT_LONG n = 0, n_alloc = 0, na, i, flush = FALSE, this_outline = FALSE;
	GMT_LONG level = 0, found_elseif = FALSE, skip[11];
	double x, y, *xx = NULL, *yy = NULL, *xp = NULL, *yp = NULL, dim[3];
	char *c = NULL;
	struct GMT_CUSTOM_SYMBOL_ITEM *s = NULL;
	struct GMT_FILL *f = NULL, *current_fill = fill;
	struct GMT_PEN *p = NULL, *current_pen = pen;
	struct GMT_FONT font = C->current.setting.font_annot[0];
	struct PSL_CTRL *P = C->PSL;

#ifdef PS_MACRO
	/* PS_MACRO stuff is on hold, awaiting more testing */
	if (symbol->PS) {	/* Special PostScript-only symbol */
		if (symbol->PS == 1) {	/* First time we must dump the PS code definition */
			PSL_comment (P, "Start of symbol %s\n", symbol->name);
			PSL_command (P, "%s", symbol->PS_macro);
			PSL_comment (P, "End of symbol %s\n", symbol->name);
			symbol->PS = 2;	/* Flag to say we have dumped the PS code */
		}
		PSL_command (P, "V ");
		PSL_setorigin (P, x0, y0, 0.0, PSL_FWD);
		for (i = symbol->n_required; i >= 0; i--) PSL_command (P, "%g ", size[i]);
		PSL_command (P, "Sk_%s U\n", symbol->name);
		return;
	}
#endif
	/* Regular macro symbol */

	/* We encapsulate symbol with gsave and translate origin to (x0, y0) first */
	PSL_command (P, "V ");
	PSL_setorigin (P, x0, y0, 0.0, PSL_FWD);
	GMT_set_meminc (C, GMT_SMALL_CHUNK);
	s = symbol->first;
	while (s) {
		if (s->conditional > 1) {	/* Process if/elseif/else and } by updating level and skip array, then go to next item */
			if (s->conditional == 2) {	/* Beginning of if branch. If we are inside an earlier branch whose test false then all is FALSE */
				skip[level+1] = (level > 0 && skip[level]) ? TRUE : gmt_custum_failed_bool_test (C, s, size), level++;
				found_elseif = !skip[level];
			}
			if (level == 10) {
				GMT_report (C, GMT_MSG_FATAL, "Error: Symbol macro (%s) logical nesting too deep [> 10]\n", symbol->name);
				GMT_exit (EXIT_FAILURE);
			}
			if (s->conditional == 4) level--, found_elseif = FALSE;	/* Simply reduce indent */
			if (s->conditional == 6) {	/* else branch */
				skip[level] = (found_elseif) ? TRUE : !skip[level];	/* Reverse test-result to apply to else branch */
				found_elseif = FALSE;
			}
			if (s->conditional == 8) {	/* Skip if prior if/elseif was TRUE, otherwise evaluate */
				skip[level] = (skip[level]) ? gmt_custum_failed_bool_test (C, s, size) : TRUE;
				if (!skip[level]) found_elseif = TRUE;	/* Needed since a final else branch will need to know if any of the if/elseifs kicked in */
			}
			s = s->next;
			continue;
		}
		if (level && skip[level]) {	/* We are inside an if-block but the block test was FALSE, so we skip */
			s = s->next;
			continue;
		}
		/* Finally, check for 1-line if tests */
		if (s->conditional == 1 && gmt_custum_failed_bool_test (C, s, size)) {	/* Done here, move to next item */
			s = s->next;
			continue;
		}

		/* Scale coordinates and size parameters by the scale in size[0] */
		
		x = s->x * size[0];
		y = s->y * size[0];
		dim[0] = s->p[0] * size[0];
		dim[1] = s->p[1] * size[0];
		dim[2] = s->p[2] * size[0];

		switch (s->action) {
			case GMT_SYMBOL_MOVE:	/* Flush existing polygon and start a new path */
				if (flush) gmt_flush_symbol_piece (C, P, xx, yy, &n, p, f, this_outline, &flush);
				n = 0;
				if (n >= n_alloc) n_alloc = GMT_malloc2 (C, xx, yy, n, n_alloc, double);
				xx[n] = x, yy[n] = y, n++;
				p = (s->pen) ? s->pen : current_pen;
				f = (s->fill) ? s->fill : current_fill;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				break;

			case GMT_SYMBOL_STROKE:	/* To force the drawing of a line (outline == 2), not a closed polygon */
				if (flush) gmt_flush_symbol_piece (C, P, xx, yy, &n, p, f, 2, &flush);
				n = 0;
				break;

			case GMT_SYMBOL_DRAW:	/* Append another point to the path */
				flush = TRUE;
				if (n >= n_alloc) n_alloc = GMT_malloc2 (C, xx, yy, n, n_alloc, double);
				xx[n] = x, yy[n] = y, n++;
				break;

			case GMT_SYMBOL_ARC:	/* Append a circular arc to the path */
				flush = TRUE;
				na = GMT_get_arc (C, x, y, 0.5 * s->p[0] * size[0], s->p[1], s->p[2], &xp, &yp);
				for (i = 0; i < na; i++) {
					if (n >= n_alloc) n_alloc = GMT_malloc2 (C, xx, yy, n, n_alloc, double);
					xx[n] = xp[i], yy[n] = yp[i], n++;
				}
				GMT_free (C, xp);
				GMT_free (C, yp);
				break;

			case GMT_SYMBOL_ROTATE:		/* Rotate the symbol coordinate system by a fixed amount */
				if (flush) gmt_flush_symbol_piece (C, P, xx, yy, &n, p, f, this_outline, &flush);
				PSL_setorigin (P, 0.0, 0.0, s->p[0], PSL_FWD);
				break;

			case GMT_SYMBOL_VARROTATE:	/* Rotate the symbol coordinate system by a variable amount */
				if (flush) gmt_flush_symbol_piece (C, P, xx, yy, &n, p, f, this_outline, &flush);
				PSL_setorigin (P, 0.0, 0.0, size[s->var], PSL_FWD);
				break;

			case GMT_SYMBOL_TEXTURE:	/* Change the current pen/fill settings */
				if (s->fill) current_fill = s->fill;
				if (s->pen) current_pen = s->pen;
				break;

#ifdef GMT_COMPAT
			case (GMT_LONG)'C':
				GMT_report (C, GMT_MSG_COMPAT, "Warning: Circle macro symbol C is deprecated; use c instead\n");
				s->action = GMT_SYMBOL_CIRCLE;	/* Backwards compatibility, circles are now 'c' */
#endif
			case GMT_SYMBOL_CROSS:
			case GMT_SYMBOL_CIRCLE:
			case GMT_SYMBOL_SQUARE:
			case GMT_SYMBOL_TRIANGLE:
			case GMT_SYMBOL_DIAMOND:
			case GMT_SYMBOL_STAR:
			case GMT_SYMBOL_HEXAGON:
			case GMT_SYMBOL_OCTAGON:
			case GMT_SYMBOL_PENTAGON:
			case GMT_SYMBOL_INVTRIANGLE:
			case GMT_SYMBOL_RECT:
			case GMT_SYMBOL_XDASH:
			case GMT_SYMBOL_YDASH:
				if (flush) gmt_flush_symbol_piece (C, P, xx, yy, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : current_fill;
				p = (s->pen)  ? s->pen  : current_pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (C, p);
				GMT_setfill (C, f, this_outline);
				PSL_plotsymbol (P, x, y, dim, s->action);
				break;

			case GMT_SYMBOL_ELLIPSE:
			case GMT_SYMBOL_ROTRECT:
				if (flush) gmt_flush_symbol_piece (C, P, xx, yy, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : current_fill;
				p = (s->pen)  ? s->pen  : current_pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (C, p);
				GMT_setfill (C, f, this_outline);
				dim[0] = s->p[0];
				PSL_plotsymbol (P, x, y, dim, PSL_ELLIPSE);
				break;

			case GMT_SYMBOL_WEDGE:
				if (flush) gmt_flush_symbol_piece (C, P, xx, yy, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : current_fill;
				p = (s->pen)  ? s->pen  : current_pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (C, p);
				GMT_setfill (C, f, this_outline);
				dim[1] = s->p[1], dim[2] = s->p[2];
				PSL_plotsymbol (P, x, y, dim, PSL_WEDGE);
				break;

			case GMT_SYMBOL_TEXT:
				if (flush) gmt_flush_symbol_piece (C, P, xx, yy, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : current_fill;
				p = (s->pen)  ? s->pen  : current_pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (C, p);

				if ((c = strchr (s->string, '%'))) {	/* Gave font name or number, too */
					*c = 0;		/* Replace % with the end of string NUL indicator */
					c++;		/* Go to next character */
					if (GMT_getfont (C, c, &font)) GMT_report (C, GMT_MSG_FATAL, "Custom symbol subcommand l contains bad font (set to %s)\n", GMT_putfont (C, C->current.setting.font_annot[0]));
					(void) GMT_setfont (C, &font);
				}
				font.size = s->p[0] * size[0] * PSL_POINTS_PER_INCH;
				if (f && this_outline)
					GMT_setfill (C, f, this_outline);
				else if (f)
					PSL_setcolor (P, f->rgb, PSL_IS_FILL);
				else
					PSL_setfill (P, C->session.no_rgb, this_outline);
				PSL_plottext (P, x, y, font.size, s->string, 0.0, PSL_MC, this_outline);
				break;

			default:
				GMT_report (C, GMT_MSG_FATAL, "Error: Unrecognized symbol code (%ld = '%c') passed to GMT_draw_custom_symbol\n", s->action, (char)s->action);
				GMT_exit (EXIT_FAILURE);
				break;
		}

		s = s->next;
	}
	if (flush) gmt_flush_symbol_piece (C, P, xx, yy, &n, p, f, this_outline, &flush);
	PSL_command (P, "U\n");
	PSL_comment (P, "End of symbol %s\n", symbol->name);
	GMT_reset_meminc (C);

	GMT_free (C, xx);
	GMT_free (C, yy);
}

/* Plotting functions related to contours */

GMT_LONG GMT_contlabel_save (struct GMT_CTRL *C, struct GMT_CONTOUR *G)
{
	GMT_LONG i, k, error, kind, object_ID;
	char word[GMT_TEXT_LEN64], record[GMT_BUFSIZ], *name = strdup (G->label_file);
	char *xname[2] = {"x", "lon"}, *yname[2] = {"y", "lat"};
	double geo[2], angle;
	struct GMT_CONTOUR_LINE *L = NULL;

	/* Save the lon, lat, angle, text for each annotation to specified file*/

	if (GMT_Register_IO (C->parent, GMT_IS_TEXTSET, GMT_IS_FILE, GMT_IS_TEXT, GMT_OUT, (void **)&name, NULL, NULL, &object_ID)) return (EXIT_FAILURE);
	if ((error = GMT_set_cols (C, GMT_OUT, 1))) return (error);
	if ((error = GMT_Begin_IO (C->parent, GMT_IS_TEXTSET, GMT_OUT, GMT_BY_REC))) return (error);	/* Enables data output and sets access mode */
	free ((void *)name);
	kind = GMT_is_geographic (C, GMT_IN);
	if (G->save_labels == 2)
		sprintf (record, "# %s%s%s%sangle%slabel", xname[kind], C->current.setting.io_col_separator, yname[kind],
			C->current.setting.io_col_separator, C->current.setting.io_col_separator);
	else 
		sprintf (record, "# %s%s%s%slabel", xname[kind], C->current.setting.io_col_separator, yname[kind], C->current.setting.io_col_separator);
	GMT_Put_Record (C->parent, GMT_WRITE_TEXT, (void *)record);	/* Write this to output */
	for (i = 0; i < G->n_segments; i++) {
		L = G->segment[i];	/* Pointer to current segment */
		if (!L->annot || L->n_labels == 0) continue;
		for (k = 0; k < L->n_labels; k++) {
			record[0] = 0;	/* Start with blank record */
			GMT_xy_to_geo (C, &geo[GMT_X], &geo[GMT_Y], L->L[k].x, L->L[k].y);
			GMT_ascii_format_col (C, word, geo[GMT_X], GMT_X);
			strcat (record, word);
			strcat (record, C->current.setting.io_col_separator);
			GMT_ascii_format_col (C, word, geo[GMT_Y], GMT_Y);
			strcat (record, word);
			strcat (record, C->current.setting.io_col_separator);
			if (G->save_labels == 2) {
				angle = fmod (2.0 * (L->L[k].angle + 360.0), 360.0) / 2.0;	/* Get text line in 0-180 range */
				GMT_ascii_format_col (C, word, angle, GMT_Z);
				strcat (record, word);
				strcat (record, C->current.setting.io_col_separator);
			}
			strcat (record, L->L[k].label);
			GMT_Put_Record (C->parent, GMT_WRITE_TEXT, (void *)record);	/* Write this to output */
		}
	}
	if ((error = GMT_End_IO (C->parent, GMT_OUT, 0))) return (error);	/* Disables further data output */
	return (GMT_NOERROR);
}

void gmt_contlabel_debug (struct GMT_CTRL *C, struct PSL_CTRL *P, struct GMT_CONTOUR *G)
{
	GMT_LONG i, j;
	int *pen = NULL;
	double size[1] = {0.025};

	/* If called we simply draw the helper lines or points to assist in debug */

	GMT_setpen (C, &C->current.setting.map_default_pen);
	if (G->fixed) {	/* Place a small open circle at each fixed point */
		PSL_setfill (P, C->session.no_rgb, PSL_OUTLINE);
		for (i = 0; i < G->f_n; i++) PSL_plotsymbol (P, G->f_xy[0][i], G->f_xy[1][i], size, PSL_CIRCLE);
	}
	else if (G->crossing) {	/* Draw a thin line */
		for (j = 0; j < G->xp->n_segments; j++) {
			pen = GMT_memory (C, NULL, G->xp->segment[j]->n_rows, int);
			for (i = 1, pen[0] = PSL_MOVE; i < G->xp->segment[j]->n_rows; i++) pen[i] = PSL_DRAW;
			GMT_plot_line (C, G->xp->segment[j]->coord[GMT_X], G->xp->segment[j]->coord[GMT_Y], pen, G->xp->segment[j]->n_rows);
			GMT_free (C, pen);
		}
	}
}

void gmt_contlabel_drawlines (struct GMT_CTRL *C, struct PSL_CTRL *P, struct GMT_CONTOUR *G, GMT_LONG mode)
{
	GMT_LONG i, k;
	int *pen = NULL;
	struct GMT_CONTOUR_LINE *L = NULL;
	for (i = 0; i < G->n_segments; i++) {
		L = G->segment[i];	/* Pointer to current segment */
		if (L->annot && mode == 1) continue; /* Annotated lines done with curved text routine */
		GMT_setpen (C, &L->pen);
		pen = GMT_memory (C, NULL, L->n, int);
		for (k = 1, pen[0] = PSL_MOVE; k < L->n; k++) pen[k] = PSL_DRAW;
		PSL_comment (P, "%s: %s\n", G->line_name, L->name);
		GMT_plot_line (C, L->x, L->y, pen, L->n);
		GMT_free (C, pen);
	}
}

void gmt_contlabel_plotlabels (struct GMT_CTRL *C, struct PSL_CTRL *P, struct GMT_CONTOUR *G, GMT_LONG mode)
{	/* mode = 1 when clipping is in effect */
	GMT_LONG i, k, m, first_i, last_i, just, form, *node = NULL;
	double *angle = NULL, *xt = NULL, *yt = NULL;
	char **txt = NULL;
	struct GMT_CONTOUR_LINE *L = NULL;

	if (G->box & 8) {	/* Repeat call for Transparent text box (already set by clip) */
		form = 8;
		if (G->box & 1) form |= 256;		/* Transparent box with outline */
		if (G->box & 4) form |= 16;		/* Rounded box with outline */
		if (G->curved_text)
			PSL_plottextpath (P, NULL, NULL, 0, NULL, 0.0, NULL, 0, NULL, 0, NULL, form);
		else
			PSL_plottextclip (P, NULL, NULL, 0, 0.0, NULL, NULL, 0, NULL, form | 1);
		return;
	}

	if (G->number_placement && G->n_cont == 1)		/* Special 1-label justification check */
		just = G->end_just[(G->number_placement+1)/2];	/* Gives index 0 or 1 */
	else
		just = G->just;

	for (i = last_i = m = 0, first_i = -1; i < G->n_segments; i++) {	/* Find first and last set of labels */
		L = G->segment[i];	/* Pointer to current segment */
		if (L->n_labels) {	/* This segment has labels */
			if (first_i == -1) first_i = i;	/* OK, this is the first */
			last_i = i;			/* When done, this will hold the last i */
			m += L->n_labels;		/* Total number of labels */
		}
	}

	if (m == 0) return;	/* There are no labels */

	if (G->curved_text) {	/* Curved labels in 2D with transparent or opaque textbox: use PSL_plottextpath */
		for (i = 0; i < G->n_segments; i++) {
			L = G->segment[i];	/* Pointer to current segment */
			if (!L->annot || L->n_labels == 0) continue;
			angle = GMT_memory (C, NULL, L->n_labels, double);
			txt = GMT_memory (C, NULL, L->n_labels, char *);
			node = GMT_memory (C, NULL, L->n_labels, GMT_LONG);
			for (k = 0; k < L->n_labels; k++) {
				angle[k] = L->L[k].angle;
				txt[k] = L->L[k].label;
				node[k] = L->L[k].node;
			}

			form = mode;		/* 1 means clip labelboxes, 0 means place text */
			if (i == first_i) form |= 32;		/* First of possibly several calls to PSL_plottextpath */
			if (i == last_i)  form |= 64;		/* Final call to PSL_plottextpath */
			if (!G->transparent) form |= 128;	/* Want the box filled */
			if (G->box & 1) form |= 256;		/* Want box outline */
			GMT_textpath_init (C, &L->pen, G->rgb, &G->pen, L->rgb);
			PSL_plottextpath (P, L->x, L->y, L->n, node, G->font_label.size, txt, L->n_labels, angle, just, G->clearance, form);
			GMT_free (C, angle);
			GMT_free (C, node);
			GMT_free (C, txt);
		}
	}
	else {	/* 2-D Straight transparent or opaque text labels: repeat call to PSL_plottextclip */
		form = 1;
		if (G->box & 4) form |= 16;		/* Want round box shape */
		if (!G->transparent) form |= 128;	/* Want the box filled */
		if (G->box & 1) form |= 256;		/* Want box outline */

		if (mode == 0) {	/* Opaque so PSL_plottextclip is called for 1st time here */
			/* Allocate temp space for everything that must be passed to PSL_plottextclip */
			(void)GMT_malloc3 (C, angle, xt, yt, m, 0, double);
			txt = GMT_memory (C, NULL, m, char *);
			for (i = m = 0; i < G->n_segments; i++) {
				L = G->segment[i];	/* Pointer to current segment */
				for (k = 0; k < L->n_labels; k++, m++) {
					angle[m] = L->L[k].angle;
					txt[m] = L->L[k].label;
					xt[m] = L->L[k].x;
					yt[m] = L->L[k].y;
				}
			}
			/* Note this uses the last segments pen/fontrgb on behalf of all */
			GMT_textpath_init (C, &L->pen, G->rgb, &G->pen, L->rgb);
			PSL_plottextclip (P, xt, yt, m, G->font_label.size, txt, angle, just, G->clearance, form);	/* This turns clipping ON */
			GMT_free (C, angle);
			GMT_free (C, xt);
			GMT_free (C, yt);
			GMT_free (C, txt);
		}
		else {	/* 2nd time called, just pass form with the 3rd bit set */
			PSL_plottextclip (P, NULL, NULL, 0, 0.0, NULL, NULL, 0, NULL, form | 8);	/* Now place the text using PSL variables already declared */
		}
	}
}

void gmt_contlabel_clippath (struct GMT_CTRL *C, struct PSL_CTRL *P, struct GMT_CONTOUR *G, GMT_LONG mode)
{
	GMT_LONG i, k, m, nseg, just, form;
	double *angle = NULL, *xt = NULL, *yt = NULL;
	char **txt = NULL;
	struct GMT_CONTOUR_LINE *L = NULL;

	if (mode == 0) {	/* Turn OFF Clipping and bail */
		PSL_comment (P, "Turn label clipping off:\n");
		PSL_plottextclip (P, NULL, NULL, 0, 0.0, NULL, NULL, 0, NULL, 2);	/* This turns clipping OFF if it was ON in the first place */
		return;
	}

	for (i = m = nseg = 0; i < G->n_segments; i++) {	/* Get total number of segments with labels */
		L = G->segment[i];		/* Pointer to current segment */
		if (L->n_labels) {
			nseg++;
			m += L->n_labels;
		}
	}

	if (m == 0) return;	/* Nothing to do */

	/* Turn ON clipping */
	if (G->curved_text) {		/* Do it via the labeling PSL function */
		gmt_contlabel_plotlabels (C, P, G, 1);
		if (nseg == 1) G->box |= 8;	/* Special message to just repeate the labelline call */
	}
	else {				/* Save PS memory by doing it this way instead via PSL_plottextclip */
		if (G->number_placement && G->n_cont == 1)		/* Special 1-label justification check */
			just = G->end_just[(G->number_placement+1)/2];	/* Gives index 0 or 1 */
		else
			just = G->just;
		/* Allocate temp space for everything that must be passed to PSL_plottextclip */
		(void)GMT_malloc3 (C, angle, xt, yt, m, 0, double);
		txt = GMT_memory (C, NULL, m, char *);
		for (i = m = 0; i < G->n_segments; i++) {
			L = G->segment[i];	/* Pointer to current segment */
			for (k = 0; k < L->n_labels; k++, m++) {
				angle[m] = L->L[k].angle;
				txt[m] = L->L[k].label;
				xt[m] = L->L[k].x;
				yt[m] = L->L[k].y;
			}
		}
		/* Note this uses the last segments pen/fontrgb on behalf of all */
		GMT_textpath_init (C, &L->pen, G->rgb, &G->pen, L->rgb);
		form = (G->box & 4) ? 16 : 0;
		PSL_plottextclip (P, xt, yt, m, G->font_label.size, txt, angle, just, G->clearance, form);	/* This turns clipping ON */
		G->box |= 8;	/* Special message to just repeate the PSL call as variables have been defined */
		GMT_free (C, angle);
		GMT_free (C, xt);
		GMT_free (C, yt);
		GMT_free (C, txt);
	}
}

void GMT_textpath_init (struct GMT_CTRL *C, struct GMT_PEN *LP, double Brgb[], struct GMT_PEN *BP, double Frgb[])
{
	PSL_defpen (C->PSL, "PSL_setlinepen", LP->width, LP->style, LP->offset, LP->rgb);
	PSL_defpen (C->PSL, "PSL_setboxpen", LP->width, LP->style, LP->offset, LP->rgb);

	PSL_defcolor (C->PSL, "PSL_setboxrgb", Brgb);
	PSL_defcolor (C->PSL, "PSL_settxtrgb", Frgb);
}

void GMT_contlabel_plot (struct GMT_CTRL *C, struct GMT_CONTOUR *G)
{
	GMT_LONG i, no_labels;
	struct PSL_CTRL *P = C->PSL;

	if (!G->n_segments) return;	/* Northing to do here */

	if (G->debug) gmt_contlabel_debug (C, P, G);		/* Debugging lines and points */

	/* See if there are labels at all */
	for (i = 0, no_labels = TRUE; i < G->n_segments && no_labels; i++) if (G->segment[i]->n_labels) no_labels = FALSE;

	if (no_labels) {	/* No labels, just draw lines */
		gmt_contlabel_drawlines (C, P, G, 0);
		return;
	}

	GMT_setfont (C, &G->font_label);
	
	if (G->transparent) {		/* Transparent boxes */
		gmt_contlabel_clippath (C, P, G, 1);		/* Lays down clippath based on ALL labels */
		gmt_contlabel_drawlines (C, P, G, 0);		/* Safe to draw continuous lines everywhere - they will be clipped at labels */
		if (G->delay) return;						/* Leave clipping on and do not plot text yet - delayed until psclip -Cc|s */
		gmt_contlabel_clippath (C, P, G, 0);		/* Turn off label clipping */
		gmt_contlabel_plotlabels (C, P, G, 0);		/* Now plot labels where they go directly */
	}
	else {	/* Opaque text boxes */
		gmt_contlabel_drawlines (C, P, G, 0);
		gmt_contlabel_plotlabels (C, P, G, 0);
	}
}

char *GMT_export2proj4 (struct GMT_CTRL *C, char *pStrOut) {
	char szProj4[512];
	double scale_factor, false_easting = 0.0, false_northing = 0.0, a, b, f;

	scale_factor = C->current.setting.proj_scale_factor;
	szProj4[0] = 0;

	switch (C->current.proj.projection) {
	/* Cylindrical projections */
	case GMT_UTM:
		sprintf (szProj4, "+proj=utm +zone=%d", (int)C->current.proj.pars[0]);
		if (C->current.proj.utm_hemisphere < 0) sprintf (szProj4, " +south");
		break;
	case GMT_MERCATOR:
		sprintf (szProj4, "+proj=merc +lon_0=%.16g +k=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[0], scale_factor, false_easting, false_northing);
		break;
	case GMT_CYL_EQ:
		sprintf (szProj4, "+proj=cea +lon_0=%.16g +lat_ts=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[1], C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_CYL_EQDIST:
		sprintf (szProj4, "+proj=eqc +lat_ts=%.16g +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[1], 0.0, C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_CYL_STEREO:
		break;
	case GMT_MILLER:
		sprintf (szProj4, "+proj=mill +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g +R_A", C->current.proj.pars[1], C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_TM:
		sprintf (szProj4, "+proj=tmerc +lat_0=%.16g +lon_0=%.16g +k=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[1], C->current.proj.pars[0], scale_factor, false_easting, false_northing);
		break;
	case GMT_CASSINI:
		sprintf (szProj4, "+proj=cass +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[1], C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_OBLIQUE_MERC:
		sprintf (szProj4, "+unavailable");
		/*sprintf (szProj4, "+proj=omerc +lat_0=%.16g +lonc=%.16g +alpha=%.16g +k=%.16g +x_0=%.16g +y_0=%.16g",
		0.0,0.0,0.0,0.0,0.0,0.0 );*/
		break;
	case GMT_OBLIQUE_MERC_POLE:
		sprintf (szProj4, "+unavailable");
		break;

	/* Conic projections */
	case GMT_ALBERS:
		sprintf (szProj4, "+proj=aea +lat_1=%.16g +lat_2=%.16g +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[2], C->current.proj.pars[3], C->current.proj.pars[1], C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_ECONIC:
		sprintf (szProj4, "+proj=eqdc +lat_1=%.16g +lat_2=%.16g +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[2], C->current.proj.pars[3], C->current.proj.pars[1], C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_LAMBERT:
		sprintf (szProj4, "+proj=lcc +lat_1=%.16g +lat_2=%.16g +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[2], C->current.proj.pars[3], C->current.proj.pars[1], C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_POLYCONIC:
		sprintf (szProj4, "+proj=poly +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[1], C->current.proj.pars[0], false_easting, false_northing);
		break;

	/* Azimuthal projections */
	case GMT_STEREO:
		sprintf (szProj4, "+proj=stere +lat_0=%.16g +lon_0=%.16g +k=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[1], C->current.proj.pars[0], scale_factor, false_easting, false_northing);
		break;
	case GMT_LAMB_AZ_EQ:
		sprintf (szProj4, "+proj=laea +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[1], C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_ORTHO:
		sprintf (szProj4, "+unavailable");
		break;
	case GMT_AZ_EQDIST:
		sprintf (szProj4, "+proj=aeqd +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[1], C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_GNOMONIC:
		sprintf (szProj4, "+proj=gnom +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[1], C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_GENPER:
		sprintf (szProj4, "+unavailable");
		break;
	case GMT_POLAR:
		sprintf (szProj4, "+unavailable");
		break;

	/* Misc projections */
	case GMT_MOLLWEIDE:
		sprintf (szProj4, "+proj=moll +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_HAMMER:
		sprintf (szProj4, "+unavailable");
		break;
	case GMT_SINUSOIDAL:
		sprintf (szProj4, "+proj=sinu +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_VANGRINTEN:
		sprintf (szProj4, "+proj=vandg +lon_0=%.16g +x_0=%.16g +y_0=%.16g +R_A", C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_ROBINSON:
		sprintf (szProj4, "+proj=robin +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_ECKERT4:
		sprintf (szProj4, "+proj=eck4 +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_ECKERT6:
		sprintf (szProj4, "+proj=eck6 +lon_0=%.16g +x_0=%.16g +y_0=%.16g", C->current.proj.pars[0], false_easting, false_northing);
		break;
	case GMT_WINKEL:
		 printf (szProj4, "+unavailable");
		break;
	default:
		if (GMT_is_geographic (C, GMT_IN))
			sprintf (szProj4, "+proj=latlong");
		else
			sprintf (szProj4, "+xy");	/* Probably useless as a info, but put there something */
	}

	a = C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].eq_radius;
	f = C->current.setting.ref_ellipsoid[C->current.setting.proj_ellipsoid].flattening;
	b = a * (1 - f);
	sprintf (szProj4+strlen(szProj4), " +a=%.3f +b=%.6f", a, b);

	pStrOut = strdup(szProj4);
	return (pStrOut);
}

void GMT_plotinit (struct GMT_CTRL *C, struct GMT_OPTION *options)
{
	/* Shuffles parameters and calls PSL_beginplot, issues PS comments regarding the GMT options
	 * and places a time stamp, if selected */

	GMT_LONG k, id, fno[PSL_MAX_EPS_FONTS], n_fonts, last;
	char title[GMT_BUFSIZ];
	char *mode[2] = {"w","a"};
	FILE *fp = NULL;	/* Default which means stdout in PSL */
	struct GMT_OPTION *Out = NULL;
	struct PSL_CTRL *P = C->PSL;
	
	if (!P) {
		GMT_report (C, GMT_MSG_FATAL, "PSL pointer not initialized!\n");
		GMT_exit (GMT_RUNTIME_ERROR);
	}

	P->internal.verbose = C->current.setting.verbose;		/* Inherit verbosity level from GMT */
#ifdef GMT_COMPAT
	if (C->current.setting.ps_copies > 1) P->init.copies = C->current.setting.ps_copies;
#endif
	PSL_setdefaults (P, C->current.setting.ps_magnify, C->current.setting.ps_page_rgb);
	if (P->init.encoding) free ((void *)P->init.encoding);
	P->init.encoding = strdup (C->current.setting.ps_encoding.name);

	if (!GMT_Find_Option (C->parent, '>', options, &Out)) {	/* Want to use a specific output file */
		k = (Out->arg[0] == '>') ? 1 : 0;	/* Are we appending (k = 1) or starting a new file (k = 0) */
		if (C->common.O.active && k == 0) {
			GMT_report (C, GMT_MSG_NORMAL, "Warning: -O given but append-mode not selected for file %s\n", &(Out->arg[k]));
		}
		if ((fp = PSL_fopen (&(Out->arg[k]), mode[k])) == NULL) {	/* Must open inside PSL DLL */
			GMT_report (C, GMT_MSG_FATAL, "Cannot open %s with mode %s\n", &(Out->arg[k]), mode[k]);
			GMT_exit (GMT_RUNTIME_ERROR);
		}
	}

	/* Initialize the plot header and settings */

	if (C->common.P.active) C->current.setting.ps_orientation = TRUE;

	/* Default for overlay plots is no shifting */

	if (!C->common.X.active && C->common.O.active) C->current.setting.map_origin[GMT_X] = 0.0;
	if (!C->common.Y.active && C->common.O.active) C->current.setting.map_origin[GMT_Y] = 0.0;

	/* Adjust offset when centering plot on center of page (PS does the rest) */

	if (C->current.ps.origin[GMT_X] == 'c') C->current.setting.map_origin[GMT_X] -= 0.5 * C->current.map.width;
	if (C->current.ps.origin[GMT_Y] == 'c') C->current.setting.map_origin[GMT_Y] -= 0.5 * C->current.map.height;

	/* Get font names used */

	id = 0;
	if (C->common.U.active) fno[id++] = C->current.setting.font_logo.id;	/* Add GMT logo font */
	/* Add title font if a title was used */
	if (C->current.map.frame.header[0]) fno[id++] = C->current.setting.font_title.id;
	/* Add the label font if labels were used */
	if (C->current.map.frame.axis[GMT_X].label[0] || C->current.map.frame.axis[GMT_Y].label[0] || C->current.map.frame.axis[GMT_Z].label[0]) fno[id++] = C->current.setting.font_label.id;
	/* Always add annotation fonts */
	fno[id++] = C->current.setting.font_annot[0].id;
	fno[id++] = C->current.setting.font_annot[1].id;

	GMT_sort_array (C, (void *)fno, id, GMT_LONG_TYPE);

	last = -1;
	for (k = n_fonts = 0; k < id; k++) {
		if (fno[k] != last) last = fno[n_fonts++] = fno[k]; /* To avoid duplicates */
	}
	for (k = n_fonts; k < PSL_MAX_EPS_FONTS; k++) fno[k] = -1;	/* Terminate */

	/* Get title */

	sprintf (title, "GMT v%s Document from %s", GMT_VERSION, C->init.module_name);
	
	PSL_beginplot (P, fp, C->current.setting.ps_orientation, C->common.O.active, C->current.setting.ps_color_mode, C->current.ps.origin, C->current.setting.map_origin, C->current.setting.ps_page_size, title, fno);

	/* Issue the comments that allow us to trace down what command created this layer */

	gmt_echo_command (C, P, options);

	/* Create %%PROJ tag that ps2raster can use to prepare a ESRI world file */

	for (k = 0, id = -1; id == -1 && k < GMT_N_PROJ4; k++) if (C->current.proj.proj4[k].id == C->current.proj.projection) id = k;
	if (id >= 0) {			/* Valid projection for creating world file info */
		double Cartesian_m[4];	/* WESN equivalents in projected meters */
		char *pstr = NULL, proj4name[16];
		Cartesian_m[0] = (C->current.proj.rect[YLO] - C->current.proj.origin[GMT_Y]) * C->current.proj.i_scale[GMT_Y];
		Cartesian_m[1] = (C->current.proj.rect[XHI] - C->current.proj.origin[GMT_X]) * C->current.proj.i_scale[GMT_X];
		Cartesian_m[2] = (C->current.proj.rect[YHI] - C->current.proj.origin[GMT_Y]) * C->current.proj.i_scale[GMT_Y];
		Cartesian_m[3] = (C->current.proj.rect[XLO] - C->current.proj.origin[GMT_X]) * C->current.proj.i_scale[GMT_X];
		/* It woul be simpler if we had a cleaner way of telling when data is lon-lat */
		if (C->current.proj.projection == GMT_LINEAR && GMT_is_geographic (C, GMT_IN))
			strcpy(proj4name, "latlong");
		else
			strcpy(proj4name, C->current.proj.proj4[id].name);

		PSL_command (P, "%%%%PROJ: %s %.8f %.8f %.8f %.8f %.3f %.3f %.3f %.3f %s\n", proj4name,
			C->common.R.wesn[XLO], C->common.R.wesn[XHI], C->common.R.wesn[YLO], C->common.R.wesn[YHI],
			Cartesian_m[3], Cartesian_m[1], Cartesian_m[0], Cartesian_m[2], GMT_export2proj4 (C, pstr));
		free((void *)pstr);
	}

	/* Set layer transparency, if requested. Note that /SetTransparency actually sets the opacity, which is (1 - transparency) */
	if (C->common.t.active) {
		PSL_command (P,  "[ /ca %g /CA %g /BM /%s /SetTransparency pdfmark\n", 1.0 - 0.01 * C->common.t.value, 1.0 - 0.01 * C->common.t.value, C->current.setting.ps_transpmode);
	}

	/* If requested, place the timestamp */

	if (C->current.ps.map_logo_label[0] == 'c' && C->current.ps.map_logo_label[1] == 0) {
		char txt[4] = {' ', '-', 'X', 0};
		struct GMT_OPTION *opt;
		/* -Uc was given as shorthand for "plot current command line" */
		strcpy (C->current.ps.map_logo_label, C->init.module_name);
		for (opt = options; opt; opt = opt->next) {
			if (opt->option == GMTAPI_OPT_INFILE || opt->option == GMTAPI_OPT_OUTFILE) continue;	/* Skip file names */
			txt[2] = opt->option;
			strcat (C->current.ps.map_logo_label, txt);
			strcat (C->current.ps.map_logo_label, opt->arg);
		}
	}
	if (C->current.setting.map_logo) gmt_timestamp (C, P, C->current.setting.map_logo_pos[GMT_X], C->current.setting.map_logo_pos[GMT_Y], C->current.setting.map_logo_justify, C->current.ps.map_logo_label);
	PSL_settransparencymode (P, C->current.setting.ps_transpmode);	/* Set PDF transparency mode, if used */
	/* Enforce chosen line parameters */
	k = C->PSL->internal.line_cap;	C->PSL->internal.line_cap = -1; PSL_setlinecap (P, k);
	k = C->PSL->internal.line_join;	C->PSL->internal.line_join = -1; PSL_setlinejoin (P, k);
	k = C->PSL->internal.miter_limit;	C->PSL->internal.miter_limit = -1; PSL_setmiterlimit (P, k);
}

void GMT_plotcanvas (struct GMT_CTRL *C)
{
	if (C->current.map.frame.paint) {	/* Paint the inside of the map with specified fill */
		double *x = NULL, *y = NULL;
		GMT_LONG np, donut;
		np = GMT_map_clip_path (C, &x, &y, &donut);
		GMT_setfill (C, &C->current.map.frame.fill, FALSE);
		PSL_plotpolygon (C->PSL, x, y, (1 + donut) * np);
		GMT_free (C, x);
		GMT_free (C, y);
	}
}

GMT_LONG GMT_plotend (struct GMT_CTRL *C) {
	struct PSL_CTRL *P = C->PSL;
	if (C->common.t.active) PSL_command (P, "[ /ca 1 /CA 1 /BM /Normal /SetTransparency pdfmark\n"); /* Reset transparency to fully opague, if required */

	/* Check expected change of clip level to achieved one. Update overall clip level. Check for pending clips. */

	if (C->current.ps.nclip != P->current.nclip)
		GMT_report (C, GMT_MSG_FATAL, "Module was expected to change clip level by %ld, but clip level changed by %ld\n", C->current.ps.nclip, P->current.nclip);

	if (GMT_abs (C->current.ps.nclip) == PSL_ALL_CLIP)	/* Special case where we reset all polygon clip levels */
		C->current.ps.clip_level = 0;
	else
		C->current.ps.clip_level += C->current.ps.nclip;

	if (!C->common.K.active) {
		if (C->current.ps.clip_level > 0) GMT_report (C, GMT_MSG_FATAL, "Warning: %ld external clip operations were not terminated!\n", C->current.ps.clip_level);
		if (C->current.ps.clip_level < 0) GMT_report (C, GMT_MSG_FATAL, "Warning: %ld extra terminations of external clip operations!\n", -C->current.ps.clip_level);
		C->current.ps.clip_level = 0;	/* Reset to zero, so it will no longer show up in .gmtcommands */
	}

	PSL_endplot (P, !C->common.K.active);
	return (0);
}

void GMT_geo_line (struct GMT_CTRL *C, double *lon, double *lat, GMT_LONG n)
{
	/* When geographic lines are plotted, they may cross the boundaries, may need to be clipped,
	 * or may appear again on the other side of the map. This is all taken care of in this
	 * routine.
	 */
	if ((C->current.plot.n = GMT_geo_to_xy_line (C, lon, lat, n)) == 0) return;	/* Nothing further to do */
	GMT_plot_line (C, C->current.plot.x, C->current.plot.y, C->current.plot.pen, C->current.plot.n);	/* Separately plot the outline */
}

void gmt_geo_polygon (struct GMT_CTRL *C, double *lon, double *lat, GMT_LONG n)
{
	/* When geographic data are plotted, polygons that cross the west map boundary will
	 * sometimes appear on the area bounded by the east map boundary - they "wrap around".
	 * This usually means we have a global map with (east-west) = 360.
	 * This function solves this by determining the polygon outline three times:
	 * First time: Truncate polygon between left and right border
	 * Second time: Find where the polygon jumps and set all the points between jumps to
	 *	       the point on the west boundary at the same latitude.
	 * Third time: Find where the polygon jumps and set all the points between jumps to
	 *	       the point on the east boundary at the same latitude.
	 * In reality it depends on the nature of the first jump in which order we do the
	 * west and east truncation above.
	 * If the polygon is clipped or wraps around at a periodic boundary then we must
	 * be careful how we draw the outline (if selected).  This function only lays down
	 * the paths; filling/outline is controlled by higher powers (GMT_geo_polygons).
	 */

#define JUMP_L 0
#define JUMP_R 1

	GMT_LONG jump, i, k, first, jump_dir = JUMP_L;
	double *xp = NULL, *yp = NULL;
	PFD x_on_border[2] = {NULL, NULL};
	struct PSL_CTRL *P = C->PSL;

	if (GMT_eq (P->current.rgb[PSL_IS_FILL][0], -1.0)) {
		/* Just draw optional outline, no fill, nor pattern */
	}
	else if (GMT_IS_AZIMUTHAL (C) || !C->current.map.is_world) {
		/* Because points way outside the map might get close to the antipode we must
		 * clip the polygon first.  The new radial clip handles this by excluding points
		 * beyond the horizon and adding arcs along the boundary between exit points
		 */

		if ((C->current.plot.n = GMT_clip_to_map (C, lon, lat, n, &xp, &yp)) == 0) return;		/* All points are outside region */
		PSL_plotpolygon (P, xp, yp, C->current.plot.n);	/* Fill Cartesian polygon and possibly draw outline */
		/* Free the memory we are done with */
		GMT_free (C, xp);
		GMT_free (C, yp);
	}
	else {
		/* Here we come for all non-azimuthal projections */

		if ((C->current.plot.n = GMT_geo_to_xy_line (C, lon, lat, n)) == 0) return;		/* Convert to (x,y,pen) - return if nothing to do */

		if (!GMT_is_geographic (C, GMT_IN)) {		/* Not geographic data so there are no periodic boundaries to worry about */
			PSL_plotpolygon (P, C->current.plot.x, C->current.plot.y, C->current.plot.n);
			return;
		}

		/* Check if there are any boundary jumps in the data as evidenced by pen up [PSL_MOVE] */

		for (first = 1, jump = FALSE; first < n && !jump; first++) jump = (C->current.plot.pen[first] != PSL_DRAW);
		if (!jump) {	/* We happened to avoid the periodic boundary - just paint and return */
			PSL_plotpolygon (P, C->current.plot.x, C->current.plot.y, C->current.plot.n);
			return;
		}

		/* Polygon wraps and we will plot it up to three times by truncating the part that would wrap the wrong way.
		 * Here we cannot use the clipped/wrapped polygon to draw outline - that is done at the end, separately */

		/* Temporary array to hold the modified x values */

		xp = GMT_memory (C, NULL, C->current.plot.n, double);

		x_on_border[JUMP_R] = GMT_left_boundary;	/* Pointers to functions that supply the x-coordinate of boundary for given y */
		x_on_border[JUMP_L] = GMT_right_boundary;

		/* Do the main truncation of bulk of polygon */

		for (i = 0, jump = FALSE; i < C->current.plot.n; i++) {
			if (C->current.plot.pen[i] == PSL_MOVE && i) {
				jump = !jump;
				jump_dir = (C->current.plot.x[i] > C->current.map.half_width) ? JUMP_R : JUMP_L;
			}
			xp[i] = (jump) ? (*x_on_border[jump_dir]) (C, C->current.plot.y[i]) : C->current.plot.x[i];
		}
		PSL_plotpolygon (P, xp, C->current.plot.y, C->current.plot.n);	/* Paint the truncated polygon */

		/* Then do the Left truncation since some wrapped pieces might not have been plotted (k > 0 means we found a piece) */

		jump_dir = (C->current.plot.x[first] > C->current.map.half_width) ? JUMP_L : JUMP_R;	/* Opposite */
		for (i = k = 0, jump = TRUE; i < C->current.plot.n; i++) {
			if (C->current.plot.pen[i] == PSL_MOVE && i) {
				jump = !jump;
				jump_dir = (C->current.plot.x[i] > C->current.map.half_width) ? JUMP_R : JUMP_L;
			}
			xp[i] = (jump || jump_dir == JUMP_R) ? (*x_on_border[JUMP_R]) (C, C->current.plot.y[i]) : C->current.plot.x[i], k++;
		}
		if (k) PSL_plotpolygon (P, xp, C->current.plot.y, C->current.plot.n);	/* Paint the truncated polygon */

		/* Then do the R truncation since some wrapped pieces might not have been plotted (k > 0 means we found a piece) */

		jump_dir = (C->current.plot.x[first] > C->current.map.half_width) ? JUMP_R : JUMP_L;	/* Opposite */
		for (i = k = 0, jump = TRUE; i < C->current.plot.n; i++) {
			if (C->current.plot.pen[i] == PSL_MOVE && i) {
				jump = !jump;
				jump_dir = (C->current.plot.x[i] > C->current.map.half_width) ? JUMP_R : JUMP_L;
			}
			xp[i] = (jump || jump_dir == JUMP_L) ? (*x_on_border[JUMP_L]) (C, C->current.plot.y[i]) : C->current.plot.x[i], k++;
		}
		if (k) PSL_plotpolygon (P, xp, C->current.plot.y, C->current.plot.n);	/* Paint the truncated polygon */

		/* Free the memory we are done with */
		GMT_free (C, xp);
	}
}

void gmt_geo_polygon_segment (struct GMT_CTRL *C, struct PSL_CTRL *P, struct GMT_LINE_SEGMENT *S, GMT_LONG add_pole)
{
	/* Handles the laying down of polygons suitable for filling only; outlines are done separately later.
	 * Polar caps need special treatment in that we must add a detour to the pole.
	 * That detour will not be drawn, only used for fill. */
	
	GMT_LONG n = S->n_rows;
	double *plon = S->coord[GMT_X], *plat = S->coord[GMT_Y];
	
	if (add_pole) {	/* Must detour to the N or S pole, then resample the path */
		n += 2;	/* Add new first and last point to connect to the pole */
		plon = GMT_memory (C, NULL, n, double);
		plat = GMT_memory (C, NULL, n, double);
		plat[0] = plat[n-1] = S->pole * 90.0;
		plon[0] = S->coord[GMT_X][0];
		plon[n-1] = S->coord[GMT_X][S->n_rows-1];
		GMT_memcpy (&plon[1], S->coord[GMT_X], S->n_rows, double);
		GMT_memcpy (&plat[1], S->coord[GMT_Y], S->n_rows, double);
		if (C->current.map.path_mode == GMT_RESAMPLE_PATH) n = GMT_fix_up_path (C, &plon, &plat, n, 0.0, 0);
	}
	gmt_geo_polygon (C, plon, plat, n);	/* Plot filled polygon [no outline] */
	if (add_pole) {		/* Delete what we made */
		GMT_free (C, plon);
		GMT_free (C, plat);
	}
}

void GMT_geo_polygons (struct GMT_CTRL *C, struct GMT_LINE_SEGMENT *S)
{	/* Deal with plotting of one or more polygons that may wrap across the map.
 	 * Multi-polygons occur if composed of a perimeter and one or more holes.
 	 * This is marked by S->next being set to point to the next hole.
	 * Also, if the perimeter is a polar cap we must add a helping line that
	 * connects to the pole but this line should not be drawn.  This is why
	 * we must lay down path twice (first for fill; then for line) since the
	 * two paths are not the same.  If no fill is requested then we just draw lines.
	 */
	struct GMT_LINE_SEGMENT *S2 = NULL;
	GMT_LONG add_pole, outline = 0, separate;
	char *type[2] = {"Perimeter", "Polar cap perimeter"};
	char *use[2] = {"fill only", "fill and outline"};
	struct PSL_CTRL *P = C->PSL;
	
	/* CASE 1: NO FILL REQUESTED -- JUST DRAW OUTLINE */
	
	if (GMT_eq (P->current.rgb[PSL_IS_FILL][0], -1.0)) {
		PSL_comment (P, "Perimeter polygon for outline only\n");
		GMT_geo_line (C, S->coord[GMT_X], S->coord[GMT_Y], S->n_rows);	/* Draw the outline only */
		for (S2 = S->next; S2; S2 = S2->next) {
			PSL_comment (P, "Hole polygon for outline only\n");
			GMT_geo_line (C, S2->coord[GMT_X], S2->coord[GMT_Y], S2->n_rows);
		}
		return;	/* Done with the simple task of drawing lines */
	}
		
	/* CASE 2: FILL REQUESTED -- WITH OR WITHOUT OUTLINE */
	
	add_pole = GMT_abs (S->pole);		/* 1 (TRUE) if a polar cap */
	separate = ((add_pole || S->next) && P->current.outline);	/* Multi-polygon (or polar cap) fill with outline handled by doing fill and outline separately */
	if (separate) {				/* Do fill and outline separately */
		outline = P->current.outline;	/* Keep a copy of what we wanted */
		P->current.outline = FALSE;	/* Turns off outline for now (if set) */
		PSL_command (P, "O0\n");	/* Temporarily switch off outline in the PS */
	}
	
	/* Here we must lay down the perimeter and then the holes.  */
	
	PSL_comment (P, "Temporarily set FO to P for complex polygon building\n");
	PSL_command (P, "/FO {P}!\n");		/* Temporarily replace FO so we can build a complex path of closed polygons using {P} */
	PSL_comment (P, "%s polygon for %s\n", type[add_pole], use[P->current.outline]);
	gmt_geo_polygon_segment (C, P, S, add_pole);	/* First lay down perimeter */
	for (S2 = S->next; S2; S2 = S2->next) {	/* Process all holes [none processed if there aren't any holes] */
		PSL_comment (P, "Hole polygon for %s\n", use[P->current.outline]);
		gmt_geo_polygon_segment (C, P, S2, FALSE);	/* Add this hole to the path */
	}
	PSL_comment (P, "Reset FO and fill the path\n");
	PSL_command (P, "/FO {fs os}!\nFO\n");	/* Reset FO to its original settings, then force the fill */
	if (separate) {	/* Must draw outline separately */
		PSL_command (P, "O1\n");	/* Switch on outline again */
		PSL_comment (P, "%s polygon for outline only\n", type[add_pole]);
		P->current.outline = outline;	/* Reset outline to what it was originally */
		GMT_geo_line (C, S->coord[GMT_X], S->coord[GMT_Y], S->n_rows);
		for (S2 = S->next; S2; S2 = S2->next) {
			PSL_comment (P, "Hole polygon for outline only\n");
			GMT_geo_line (C, S2->coord[GMT_X], S2->coord[GMT_Y], S2->n_rows);
		}
	}
}

void GMT_geo_ellipse (struct GMT_CTRL *C, double lon, double lat, double major, double minor, double azimuth)
{
	/* GMT_geo_ellipse takes the location, axes (in km), and azimuth of an ellipse
	   and draws the ellipse using the chosen map projection */

	GMT_LONG i;
	double delta_azimuth, sin_azimuth, cos_azimuth, sinp, cosp, x, y, x_prime, y_prime, rho, c;
	double sin_c, cos_c, center, *px = NULL, *py = NULL;
	struct GMT_LINE_SEGMENT *S = GMT_memory (C, NULL, 1, struct GMT_LINE_SEGMENT);

	GMT_alloc_segment (C, S, GMT_ELLIPSE_APPROX+1, 2, TRUE);
	px = S->coord[GMT_X];	py = S->coord[GMT_Y];

	delta_azimuth = 2.0 * M_PI / GMT_ELLIPSE_APPROX;
	major *= 500.0, minor *= 500.0;	/* Convert to meters of semi-major and semi-minor axes */
	sincosd (90.0 - azimuth, &sin_azimuth, &cos_azimuth);
	sincosd (lat, &sinp, &cosp);	/* Set up azimuthal equidistant projection */

	center = (C->current.proj.central_meridian < C->common.R.wesn[XLO] || C->current.proj.central_meridian > C->common.R.wesn[XHI]) ? 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]) : C->current.proj.central_meridian;

	/* Approximate ellipse by a GMT_ELLIPSE_APPROX-sided polygon */

	for (i = 0; i < GMT_ELLIPSE_APPROX; i++) {

		sincos (i * delta_azimuth, &y, &x);
		x *= major;
		y *= minor;

		/* Get rotated coordinates in m */

		x_prime = x * cos_azimuth - y * sin_azimuth;
		y_prime = x * sin_azimuth + y * cos_azimuth;

		/* Convert m back to lon lat */

		rho = hypot (x_prime, y_prime);

		c = rho / C->current.proj.EQ_RAD;
		sincos (c, &sin_c, &cos_c);
		py[i] = d_asind (cos_c * sinp + (y_prime * sin_c * cosp / rho));
		if ((lat - 90.0) > -GMT_CONV_LIMIT)	/* origin in Northern hemisphere */
			px[i] = lon + d_atan2d (x_prime, -y_prime);
		else if ((lat + 90.0) < GMT_CONV_LIMIT)	/* origin in Southern hemisphere */
			px[i] = lon + d_atan2d (x_prime, y_prime);
		else
			px[i] = lon + d_atan2d (x_prime * sin_c, (rho * cosp * cos_c - y_prime * sinp * sin_c));
		while ((px[i] - center) < -180.0) px[i] += 360.0;
		while ((px[i] - center) > +180.0) px[i] -= 360.0;
	}
	/* Explicitly close the polygon */
	px[GMT_ELLIPSE_APPROX] = px[0], py[GMT_ELLIPSE_APPROX] = py[0];
	GMT_geo_polygons (C, S);

	GMT_free_segment (C, S);
}

void GMT_geo_rectangle (struct GMT_CTRL *C, double lon, double lat, double width, double height, double azimuth)
{
	/* GMT_geo_rectangle takes the location, axes (in km), and azimuth of a rectangle
	   and draws the rectangle using the chosen map projection */

	double sin_azimuth, cos_azimuth, sinp, cosp, x, y, x_prime, y_prime, rho, c, dim[3];
	double sin_c, cos_c, center, lon_w, lat_w, lon_h, lat_h, xp, yp, xw, yw, xh, yh, tmp;
	struct PSL_CTRL *P = C->PSL;

	GMT_azim_to_angle (C, lon, lat, 0.1, azimuth, &tmp);
	azimuth = tmp;
	GMT_geo_to_xy (C, lon, lat, &xp, &yp);		/* Center of rectangle */

	width *= 500.0, height *= 500.0;	/* Convert to meters and get half the size */
	dim[0] = azimuth;
	sincosd (azimuth, &sin_azimuth, &cos_azimuth);
	sincosd (lat, &sinp, &cosp);		/* Set up azimuthal equidistant projection */

	center = (C->current.proj.central_meridian < C->common.R.wesn[XLO] || C->current.proj.central_meridian > C->common.R.wesn[XHI]) ? 0.5 * (C->common.R.wesn[XLO] + C->common.R.wesn[XHI]) : C->current.proj.central_meridian;

	/* Get first point width away from center */
	sincos (0.0, &y, &x);
	x *= width;
	y *= height;
	/* Get rotated coordinates in m */
	x_prime = x * cos_azimuth - y * sin_azimuth;
	y_prime = x * sin_azimuth + y * cos_azimuth;
	/* Convert m back to lon lat */
	rho = hypot (x_prime, y_prime);
	c = rho / C->current.proj.EQ_RAD;
	sincos (c, &sin_c, &cos_c);
	lat_w = d_asind (cos_c * sinp + (y_prime * sin_c * cosp / rho));
	if ((lat - 90.0) > -GMT_CONV_LIMIT)	/* origin in Northern hemisphere */
		lon_w = lon + d_atan2d (x_prime, -y_prime);
	else if ((lat + 90.0) < GMT_CONV_LIMIT)	/* origin in Southern hemisphere */
		lon_w = lon + d_atan2d (x_prime, y_prime);
	else
		lon_w = lon + d_atan2d (x_prime * sin_c, (rho * cosp * cos_c - y_prime * sinp * sin_c));
	while ((lon_w - center) < -180.0) lon_w += 360.0;
	while ((lon_w - center) > +180.0) lon_w -= 360.0;
	GMT_geo_to_xy (C, lon_w, lat_w, &xw, &yw);	/* Get projected x,y coordinates */
	dim[1] = 2.0 * hypot (xp - xw, yp - yw);	/* Estimate of rectangle width in plot units (inch) */
	/* Get 2nd point height away from center */
	sincos (M_PI_2, &y, &x);
	x *= width;
	y *= height;
	/* Get rotated coordinates in m */
	x_prime = x * cos_azimuth - y * sin_azimuth;
	y_prime = x * sin_azimuth + y * cos_azimuth;
	/* Convert m back to lon lat */
	rho = hypot (x_prime, y_prime);
	c = rho / C->current.proj.EQ_RAD;
	sincos (c, &sin_c, &cos_c);
	lat_h = d_asind (cos_c * sinp + (y_prime * sin_c * cosp / rho));
	if ((lat - 90.0) > -GMT_CONV_LIMIT)	/* origin in Northern hemisphere */
		lon_h = lon + d_atan2d (x_prime, -y_prime);
	else if ((lat + 90.0) < GMT_CONV_LIMIT)	/* origin in Southern hemisphere */
		lon_h = lon + d_atan2d (x_prime, y_prime);
	else
		lon_h = lon + d_atan2d (x_prime * sin_c, (rho * cosp * cos_c - y_prime * sinp * sin_c));
	while ((lon_h - center) < -180.0) lon_h += 360.0;
	while ((lon_h - center) > +180.0) lon_h -= 360.0;
	GMT_geo_to_xy (C, lon_h, lat_h, &xh, &yh);
	dim[2] = 2.0 * hypot (xp - xh, yp - yh);	/* Estimate of rectangle width in plot units (inch) */
	PSL_plotsymbol (P, xp, yp, dim, PSL_ROTRECT);
}

void GMT_draw_front (struct GMT_CTRL *C, double x[], double y[], GMT_LONG n, struct GMT_FRONTLINE *f)
{
	GMT_LONG i, ngap, skip;
	double *s = NULL, xx[4], yy[4], dist = 0.0, w, frac, dx, dy, angle, dir1, dir2;
	double gap, x0, y0, xp, yp, len2, len3, cosa, sina, sa, ca, offx, offy, dim[3];
	struct PSL_CTRL *P = C->PSL;

	if (n < 2) return;

	s = GMT_memory (C, NULL, n, double);
	for (i = 1, s[0] = 0.0; i < n; i++) {
		/* Watch out for longitude wraps */
		dx = x[i] - x[i-1];
		w = GMT_half_map_width (C, y[i]);
		if (C->current.map.is_world && dx > w) dx = copysign (2 * w - fabs (dx), -dx);
		s[i] = s[i-1] + hypot (dx, y[i] - y[i-1]);
	}

	if (f->f_gap > 0.0) {
		ngap = irint (s[n-1] / f->f_gap);
		gap = s[n-1] / (double)ngap;
		dist = f->f_off;
		ngap++;
	}
	else {
		ngap = (GMT_LONG) fabs (f->f_gap);
		if (ngap == 0) {
			GMT_report (C, GMT_MSG_FATAL, "Warning: Number of front ticks reset from 0 to 1 (check your arguments)\n");
			ngap = 1;
		}
		gap = s[n-1] / (ngap - 1);
		if (ngap == 1) dist = 0.5 * s[n-1];
	}

	len2 = 0.5 * f->f_len;
	len3 = 0.866025404 * f->f_len;
	if (f->f_sense == GMT_FRONT_CENTERED) len3 = len2;

	i = 0;
	while (i < n) {
		while ((s[i] - dist) > -GMT_SMALL) {	/* Time for tick */
			if (i > 0) {
				dx = x[i] - x[i-1];
				dy = y[i] - y[i-1];
			}
			else {
				dx = x[1] - x[0];
				dy = y[1] - y[0];
			}
			if (fabs (dist - s[i]) < GMT_SMALL) {
				x0 = x[i];
				y0 = y[i];
			}
			else {
				frac = (s[i] - dist) / (s[i] - s[i-1]);
				x0 = x[i] - dx * frac;
				y0 = y[i] - dy * frac;
			}
			angle = d_atan2 (dy, dx);
			skip = (C->current.map.is_world && fabs (dx) > GMT_half_map_width (C, y[i]));	/* Don't do ticks on jumps */
			if (skip) {
				dist += gap;
				i++;
				continue;
			}

			switch (f->f_symbol) {
				case GMT_FRONT_TRIANGLE:	/* Triangle */
					switch (f->f_sense) {
						case GMT_FRONT_CENTERED:
							sincos (angle, &sina, &cosa);
							xx[0] = x0 + len2 * cosa;
							yy[0] = y0 + len2 * sina;
							xx[1] = x0 - len3 * sina;
							yy[1] = y0 + len3 * cosa;
							xx[2] = x0 - len2 * cosa;
							yy[2] = y0 - len2 * sina;
							xx[3] = x0 + len3 * sina;
							yy[3] = y0 - len3 * cosa;
							PSL_plotpolygon (P, xx, yy, 4);
							break;
						case GMT_FRONT_RIGHT:
							angle += M_PI;
						case GMT_FRONT_LEFT:
							sincos (angle, &sina, &cosa);
							xx[0] = x0 + len2 * cosa;
							yy[0] = y0 + len2 * sina;
							xx[1] = x0 - len3 * sina;
							yy[1] = y0 + len3 * cosa;
							xx[2] = x0 - len2 * cosa;
							yy[2] = y0 - len2 * sina;
							PSL_plotpolygon (P, xx, yy, 3);
							break;
					}
					break;

				case GMT_FRONT_CIRCLE:	/* Circles */
					switch (f->f_sense) {
						case GMT_FRONT_CENTERED:
							PSL_plotsymbol (P, x0, y0, &(f->f_len), PSL_CIRCLE);
							break;
						case GMT_FRONT_RIGHT:
							angle += M_PI;
						case GMT_FRONT_LEFT:
							dir1 = R2D * angle;
							dir2 = dir1 + 180.0;
							if (dir1 > dir2) dir1 -= 360.0;
							dim[0] = f->f_len, dim[1] = dir1, dim[2] = dir2;
							PSL_plotsymbol (P, x0, y0, dim, PSL_WEDGE);
							break;
					}
					break;

				case GMT_FRONT_BOX:	/* Squares */
					switch (f->f_sense) {
						case GMT_FRONT_CENTERED:	/* Full square centered on line */
							sincos (angle, &sina, &cosa);
							xx[0] = x0 + len2 * (cosa + sina);	/* LR */
							yy[0] = y0 + len2 * (sina - cosa);
							xx[1] = x0 + len2 * (cosa - sina);	/* UR */
							yy[1] = y0 + len2 * (sina + cosa);
							xx[2] = x0 + len2 * (-cosa - sina);	/* UL */
							yy[2] = y0 + len2 * (-sina + cosa);
							xx[3] = x0 + len2 * (-cosa + sina);	/* LL */
							yy[3] = y0 + len2 * (-sina - cosa);
							break;
						case GMT_FRONT_RIGHT:
							angle += M_PI;
						case GMT_FRONT_LEFT:
							/* Half square on the chosen side */
							sincos (angle, &sina, &cosa);
							xx[0] = x0 + len2 * (cosa);	/* LR */
							yy[0] = y0 + len2 * (sina);
							xx[1] = x0 + len2 * (cosa - sina);	/* UR */
							yy[1] = y0 + len2 * (sina + cosa);
							xx[2] = x0 + len2 * (-cosa - sina);	/* UL */
							yy[2] = y0 + len2 * (-sina + cosa);
							xx[3] = x0 + len2 * (-cosa);	/* LL */
							yy[3] = y0 + len2 * (-sina);
							break;
					}
					PSL_plotpolygon (P, xx, yy, 4);
					break;

				case GMT_FRONT_SLIP: /* draw strike-slip arrows */
					sincos (angle, &sina, &cosa);
					offx = C->current.setting.map_annot_offset[0] * sina; /* get offsets from front line */
					offy = C->current.setting.map_annot_offset[0] * cosa;
					/* sense == GMT_FRONT_LEFT == left-lateral, R_RIGHT = right lateral */
					/* arrow "above" line */
					sincos (angle + (f->f_sense * 30.0 * D2R), &sa, &ca);
					xp = x0 + f->f_sense * offx;
					yp = y0 - f->f_sense * offy;
					xx[0] = xp - len2 * cosa;
					yy[0] = yp - len2 * sina;
					xx[1] = xp + len2 * cosa;
					yy[1] = yp + len2 * sina;
					xx[2] = xx[1] - len2 * ca;
					yy[2] = yy[1] - len2 * sa;
					PSL_plotline (P, xx, yy, 3, PSL_MOVE + PSL_STROKE);

					/* arrow "below" line */
					sincos (angle - (f->f_sense * 150.0 *D2R), &sa, &ca);
					xp = x0 - f->f_sense * offx;
					yp = y0 + f->f_sense * offy;
					xx[0] = xp + len2 * cosa;
					yy[0] = yp + len2 * sina;
					xx[1] = xp - len2 * cosa;
					yy[1] = yp - len2 * sina;
					xx[2] = xx[1] - len2 * ca;
					yy[2] = yy[1] - len2 * sa;
					PSL_plotline (P, xx, yy, 3, PSL_MOVE + PSL_STROKE);
					break;

				case GMT_FRONT_FAULT:	/* Normal fault ticks */
					xx[0] = xx[1] = x0, yy[0] = yy[1] = y0;
					if (f->f_sense == GMT_FRONT_CENTERED) {
						angle -= M_PI_2;
						sincos (angle, &sina, &cosa);
						xx[0] += len2 * cosa;
						yy[0] += len2 * sina;
						xx[1] -= len2 * cosa;
						yy[1] -= len2 * sina;
					}
					else {
						angle += (f->f_sense * M_PI_2);
						sincos (angle, &sina, &cosa);
						xx[1] += len2 * cosa;
						yy[1] += len2 * sina;
					}
					PSL_plotline (P, xx, yy, 2, PSL_MOVE + PSL_STROKE);
					break;
			}
			dist += gap;
		}
		i++;
	}
	GMT_free (C, s);
}

void GMT_plane_perspective (struct GMT_CTRL *C, GMT_LONG plane, double level)
{
	/* This routine write the PostScript code to change any following matter printed in the plane
	 * of the paper into a perspective view of that plane based on the C->current.proj.z_project
	 * parameters (azimuth and elevation).
	 * The plane is portrayed as a plane of constant X, Y, or Z.
	 * Input arguments:
	 * C	: The GMT struct
	 * P	: The PSL struct
	 * plane: The perspective plane if a constant X, Y, or Z (GMT_X = 0, GMT_Y = 1, GMT_Z = 2)
	 *        To indicate that the z-level is not in projected but "world" coordinates, add GMT_ZW = 3
	 *        To reset to normal printing, use -1.
	 * level: Level of X, Y, or Z in projected coordinates (inch).
	 */
	double a, b, c, d, e, f;
	struct PSL_CTRL *P = C->PSL;

	/* Only do this in 3D mode */
	if (!C->current.proj.three_D) return;

	/* Nothing changed since last call, hence ignore */
	if (plane == C->current.proj.z_project.plane && GMT_eq(level,C->current.proj.z_project.level)) return;

	/* Store value of level (store plane at end) */
	C->current.proj.z_project.level = level;

	/* Concat contains the proper derivatives of these functions:
	x_out = - x * C->current.proj.z_project.cos_az + y * C->current.proj.z_project.sin_az + C->current.proj.z_project.x_off;
	y_out = - (x * C->current.proj.z_project.sin_az + y * C->current.proj.z_project.cos_az) *
		C->current.proj.z_project.sin_el + z * C->current.proj.z_project.cos_el + C->current.proj.z_project.y_off;
	*/

	a = b = c = d = e = f = 0.0;
	if (plane < 0)			/* Reset to original matrix */
		PSL_command (P, "PSL_GPP setmatrix\n");
	else {	/* New perspective plane: compute all derivatives and use full matrix */
		if (plane >= GMT_ZW) level = GMT_z_to_zz (C, level);	/* First convert world z coordinate to projected z coordinate */
		switch (plane % 3) {
			case GMT_X:	/* Constant x, Convert y,z to x',y' */
				a = C->current.proj.z_project.sin_az;
				b = -C->current.proj.z_project.cos_az * C->current.proj.z_project.sin_el;
				c = 0.0;
				d = C->current.proj.z_project.cos_el;
				e = C->current.proj.z_project.x_off - level * C->current.proj.z_project.cos_az;
				f = C->current.proj.z_project.y_off - level * C->current.proj.z_project.sin_az * C->current.proj.z_project.sin_el;
				break;
			case GMT_Y:	/* Constant y. Convert x,z to x',y' */
				a = -C->current.proj.z_project.cos_az;
				b = -C->current.proj.z_project.sin_az * C->current.proj.z_project.sin_el;
				c = 0.0;
				d = C->current.proj.z_project.cos_el;
				e = C->current.proj.z_project.x_off + level * C->current.proj.z_project.sin_az;
				f = C->current.proj.z_project.y_off - level * C->current.proj.z_project.cos_az * C->current.proj.z_project.sin_el;
				break;
			case GMT_Z:	/* Constant z. Convert x,y to x',y' */
				a = -C->current.proj.z_project.cos_az;
				b = -C->current.proj.z_project.sin_az * C->current.proj.z_project.sin_el;
				c = C->current.proj.z_project.sin_az;
				d = -C->current.proj.z_project.cos_az * C->current.proj.z_project.sin_el;
				e = C->current.proj.z_project.x_off;
				f = C->current.proj.z_project.y_off + level * C->current.proj.z_project.cos_el;
				break;
		}

		/* First restore the old matrix or save the old one when that was not done before */
		PSL_command (P, "%s [%g %g %g %g %g %g] concat\n",
			(C->current.proj.z_project.plane >= 0) ? "PSL_GPP setmatrix" : "/PSL_GPP matrix currentmatrix def",
			a, b, c, d, e * P->internal.x2ix, f * P->internal.y2iy);
	}

	/* Store value of plane */
	C->current.proj.z_project.plane = plane;
}
