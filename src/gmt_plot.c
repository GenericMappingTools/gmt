/*--------------------------------------------------------------------
 *	$Id: gmt_plot.c,v 1.297 2011-03-03 21:02:50 guru Exp $
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
 *
 *			G M T _ P L O T . C
 *
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * GMT_plot.c contains code related to plotting maps
 *
 * Author:	Paul Wessel
 * Date:	13-SEP-2001
 * Version:	4
 *
 *
 * PUBLIC Functions include:
 *
 *	GMT_color_image :	Calls the desired colorimage operator
 *	GMT_draw_map_scale :	Plot map scale
 *	GMT_echo_command :	Puts the command line into the PostScript file as comments
 *	GMT_plot_line :		Plots lon/lat path on maps, takes care of periodicity jumps
 *	GMT_map_basemap :	Generic basemap function
 *	GMT_map_clip_off :	Deactivate map region clip path
 *	GMT_map_clip_on :	Activate map region clip path
 *	GMT_text3D :		Draw perspective text
 *	GMT_textbox3D :		Draw perspective text box
 *	GMT_timestamp :		Plot UNIX time stamp with optional string
 *	GMT_vector :		Draw 2/3-D vector
 *	GMT_vertical_axis :	Draw 3-D vertical axes
 *	GMT_xy_axis :		Draw x or y axis
 *
 * INTERNAL Functions include:
 *
 *	GMT_basemap_3D :		Plots 3-D basemap
 *	GMT_geoplot :			As ps_plot, but using lon/lat directly
 *	GMT_fill :			Convenience function for ps_polygon with or without fill
 *	GMT_conic_map_boundary :	Plot basemap for conic projections
 *	GMT_linear_map_boundary :	Plot basemap for Linear projections
 *	GMT_linearx_grid :		Draw linear x grid lines
 *	GMT_lineary_grid :		Draw linear y grid lines
 *	GMT_logx_grid :			Draw log10 x grid lines
 *	GMT_logy_grid :			Draw log10 y grid lines
 *	GMT_powx_grid :			Draw power x grid lines
 *	GMT_powy_grid :			Draw power y grid lines
 *	GMT_prepare_label :		Gets angle and justification for frame annotations
 *	GMT_map_annotate :		Annotate basemaps
 *	GMT_map_boundary :		Draw the maps boundary
 *	GMT_map_gridcross :		Draw grid crosses on maps
 *	GMT_map_gridlines :		Draw gridlines on maps
 *	GMT_map_latline :		Draw a latitude line
 *	GMT_map_lattick :		Draw a latitude tick mark
 *	GMT_map_lonline :		Draw a longitude line
 *	GMT_map_lontick :		Draw a longitude tick mark
 *	GMT_map_symbol :		Plot map annotation
 *	GMT_map_symbol_ew :		  for east/west sides
 *	GMT_map_symbol_ns :		  for south/north sides
 *	GMT_map_tick :			Draw the ticks
 *	GMT_map_tickmarks :		Plot tickmarks on maps
 *	GMT_fancy_map_boundary :	Plot basemap for Mercator projection
 *	GMT_ellipse_map_boundary :	Plot basemap for Mollweide and Hammer-Aitoff projections
 *	GMT_oblmrc_map_boundary :	Plot basemap for Oblique Mercator projection
 *	GMT_polar_map_boundary :	Plot basemap for Polar stereographic projection
 *	GMT_rect_map_boundary :		Plot plain basemap for projections with rectangular boundaries
 *	GMT_basic_map_boundary :	Plot plain basemap for most projections
 *	GMT_wesn_map_boundary :		Plot plain basemap for projections with geographical boundaries
 *	GMT_theta_r_map_boundary :	Plot plain basemap for polar (cylindrical) projection
 *	GMT_xyz_axis3D :		Draw 3-D axis
 */

#include "gmt.h"
#include "pslib.h"

#define GMT_ELLIPSE_APPROX 72

void GMT_map_symbol (double *xx, double *yy, GMT_LONG *sides, double *line_angles, char *label, GMT_LONG nx, GMT_LONG type, GMT_LONG annotate, GMT_LONG level);
void GMT_map_symbol_ew (double lat, char *label, double west, double east, GMT_LONG annotate, GMT_LONG level);
void GMT_map_symbol_ns (double lon, char *label, double south, double north, GMT_LONG annotate, GMT_LONG level);
void GMT_get_annot_label (double val, char *label, GMT_LONG do_minutes, GMT_LONG do_seconds, GMT_LONG lonlat, GMT_LONG worldmap);
void GMT_basemap_3D (GMT_LONG mode);
void GMT_xyz_axis3D (GMT_LONG axis_no, char axis, struct GMT_PLOT_AXIS *A, GMT_LONG annotate);
GMT_LONG GMT_coordinate_array (double min, double max, struct GMT_PLOT_AXIS_ITEM *T, double **array);
GMT_LONG GMT_linear_array (double min, double max, double delta, double phase, double **array);
GMT_LONG GMT_pow_array (double min, double max, double delta, GMT_LONG x_or_y, double **array);
GMT_LONG GMT_grid_clip_path (struct GRD_HEADER *h, double **x, double **y, GMT_LONG *donut);
void GMT_wesn_map_boundary (double w, double e, double s, double n);
void GMT_rect_map_boundary (double x0, double y0, double x1, double y1);
void GMT_theta_r_map_boundary (double w, double e, double s, double n);
void GMT_map_latline (double lat, double west, double east);
void GMT_map_lonline (double lon, double south, double north);
void GMT_map_tick (double *xx, double *yy, GMT_LONG *sides, double *angles, GMT_LONG nx, GMT_LONG type, double len);
void GMT_map_lontick (double lon, double south, double north, double len);
void GMT_map_lattick (double lat, double west, double east, double len);
GMT_LONG GMT_prepare_label (double angle, GMT_LONG side, double x, double y, GMT_LONG type, double *line_angle, double *text_angle, GMT_LONG *justify);
GMT_LONG GMT_annot_too_crowded (double x, double y, GMT_LONG side);
GMT_LONG GMT_is_fancy_boundary (void);
GMT_LONG GMT_time_array (double min, double max, struct GMT_PLOT_AXIS_ITEM *T, double **array);
void GMT_timex_grid (double w, double e, double s, double n, GMT_LONG item);
void GMT_timey_grid (double w, double e, double s, double n, GMT_LONG item);
GMT_LONG GMT_skip_second_annot (GMT_LONG item, double x, double x2[], GMT_LONG n, GMT_LONG primary, GMT_LONG secondary);
double GMT_set_label_offsets (GMT_LONG axis, double val0, double val1, struct GMT_PLOT_AXIS *A, GMT_LONG below, double annot_off[], double *label_off, GMT_LONG *annot_justify, GMT_LONG *label_justify, char *format);

void GMT_map_tickitem (double w, double e, double s, double n, GMT_LONG item);
void GMT_NaN_pen_up (double x[], double y[], int pen[], GMT_LONG n);
double GMT_fancy_frame_straight_outline (double lonA, double latA, double lonB, double latB, GMT_LONG side, GMT_LONG secondary_too);
double GMT_fancy_frame_curved_outline (double lonA, double latA, double lonB, double latB, GMT_LONG side, GMT_LONG secondary_too);
void GMT_rounded_framecorners (double w, double e, double s, double n, double anglew, double anglee, GMT_LONG secondary_too);
void GMT_fancy_frame_offset (double angle, double shift[2]);
void GMT_fancy_frame_extension (double angle, double extend[2]);
void GMT_fancy_frame_straightlat_checkers (double w, double e, double s, double n, double angle_w, double angle_e, GMT_LONG secondary_too);
void GMT_fancy_frame_curvedlon_checkers (double w, double e, double s, double n, double radius_s, double radius_n, GMT_LONG secondary_too);
void GMT_fancy_frame_straightlon_checkers (double w, double e, double s, double n, double angle_s, double angle_n, GMT_LONG secondary_too);
void GMT_label_trim (char *label, GMT_LONG stage);
void GMT_draw_dir_rose (struct GMT_MAP_ROSE *mr);
void GMT_draw_mag_rose (struct GMT_MAP_ROSE *mr);
void GMT_Nstar (double x0, double y0, double r);
void GMT_contlabel_debug (struct GMT_CONTOUR *G);
void GMT_contlabel_drawlines (struct GMT_CONTOUR *G, GMT_LONG mode);
void GMT_contlabel_clippath (struct GMT_CONTOUR *G, GMT_LONG mode);
void GMT_contlabel_plotlabels (struct GMT_CONTOUR *G, GMT_LONG mode);
void GMT_textpath_init (struct GMT_PEN *LP, int Brgb[], struct GMT_PEN *BP, int Frgb[]);
void GMT_contlabel_plotboxes (struct GMT_CONTOUR *G);
void GMT_flush_symbol_piece (double *x, double *y, double z, GMT_LONG *n, struct GMT_PEN *p, struct GMT_FILL *f, GMT_LONG outline, GMT_LONG *flush);
void GMT_define_PS_items (struct GMT_PLOT_AXIS *A, GMT_LONG below, GMT_LONG annotate);
void GMT_define_baselines ();
void GMT_geoplot (double lon, double lat, int pen);
void GMT_geosegment (double lon1, double lat1, double lon2, double lat2);
GMT_LONG set_do_seconds (double inc);

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

struct GMT_PROJ {
	char *proj4name;
	GMT_LONG id;
};

#define GMT_N_PROJ4 31
struct GMT_PROJ GMT_proj4[GMT_N_PROJ4] = {
	{ "aea"      , GMT_ALBERS },
	{ "aeqd"     , GMT_AZ_EQDIST },
	{ "cyl_stere", GMT_CYL_STEREO },
	{ "cass"     , GMT_CASSINI },
	{ "cea"      , GMT_CYL_EQ },
	{ "eck4"     , GMT_ECKERT4 },
	{ "eck6"     , GMT_ECKERT6 },
	{ "eqc"      , GMT_CYL_EQDIST },
	{ "eqdc"     , GMT_ECONIC },
	{ "gnom"     , GMT_GNOMONIC },
	{ "hammer"   , GMT_HAMMER },
	{ "laea"     , GMT_LAMB_AZ_EQ },
	{ "lcc"      , GMT_LAMBERT },
	{ "merc"     , GMT_MERCATOR },
	{ "mill"     , GMT_MILLER },
	{ "moll"     , GMT_MOLLWEIDE },
	{ "nsper"    , GMT_GENPER },
	{ "omerc"    , GMT_OBLIQUE_MERC },
	{ "omercp"   , GMT_OBLIQUE_MERC_POLE },
	{ "ortho"    , GMT_ORTHO },
	{ "polar"    , GMT_POLAR },
	{ "poly"     , GMT_POLYCONIC },
	{ "robin"    , GMT_ROBINSON },
	{ "sinu"     , GMT_SINUSOIDAL },
	{ "stere"    , GMT_STEREO },
	{ "tmerc"    , GMT_TM },
	{ "utm"      , GMT_UTM },
	{ "vandg"    , GMT_VANGRINTEN },
	{ "wintri"   , GMT_WINKEL },
	{ "xy"       , GMT_LINEAR },
	{ "z"        , GMT_ZAXIS }
};

/*	GMT_LINEAR PROJECTION MAP BOUNDARY	*/

void GMT_linear_map_boundary (double w, double e, double s, double n)
{
	double x1, x2, y1, y2, x_length, y_length;

	GMT_geo_to_xy (w, s, &x1, &y1);
	GMT_geo_to_xy (e, n, &x2, &y2);
	if (x1 > x2) d_swap (x1, x2);
	if (y1 > y2) d_swap (y1, y2);
	x_length = fabs (x2 - x1);
	y_length = fabs (y2 - y1);

	if (frame_info.side[3]) GMT_xy_axis (x1, y1, y_length, s, n, &frame_info.axis[1], TRUE,  frame_info.side[3] == 2);	/* West or left y-axis */
	if (frame_info.side[1]) GMT_xy_axis (x2, y1, y_length, s, n, &frame_info.axis[1], FALSE, frame_info.side[1] == 2);	/* East or right y-axis */
	if (frame_info.side[0]) GMT_xy_axis (x1, y1, x_length, w, e, &frame_info.axis[0], TRUE,  frame_info.side[0] == 2);	/* South or lower x-axis */
	if (frame_info.side[2]) GMT_xy_axis (x1, y2, x_length, w, e, &frame_info.axis[0], FALSE, frame_info.side[2] == 2);	/* North or upper x-axis */

	if (!frame_info.header[0] || frame_info.plotted_header) return;	/* No header today */

	ps_comment ("Placing plot title");

	GMT_define_PS_items (&frame_info.axis[0], FALSE, frame_info.side[2] == 2);

	GMT_define_baselines ();

	ps_set_length ("PSL_HO", gmtdefs.header_offset);

	ps_command ("/PSL_H_y PSL_L_y PSL_LH add PSL_HO add def");	/* For title adjustment */

	if (frame_info.side[2] == 0) ps_set_length ("PSL_H_y", gmtdefs.header_offset);	/* Just axis line is drawn, offset by header_offset only */
	if (frame_info.side[2] == 1) ps_command ("/PSL_H_y PSL_L_y PSL_HO add def");	/* Allow for ticks + offset */
	ps_set_length ("PSL_x", 0.5 * x_length);
	ps_set_length ("PSL_y", y_length);
	ps_set_height ("PSL_HF", gmtdefs.header_font_size);
	ps_textdim ("PSL_dim", gmtdefs.header_font_size, gmtdefs.header_font, frame_info.header);	/* Get and set string dimensions in PostScript */
	ps_command ("PSL_x PSL_dim_w 2 div sub PSL_y PSL_H_y add M");
	ps_setfont (gmtdefs.header_font);
	ps_text (0.0, 0.0, -gmtdefs.header_font_size, frame_info.header, 0.0, 0, 0);
	frame_info.plotted_header = TRUE;
}

void GMT_xy_axis (double x0, double y0, double length, double val0, double val1, struct GMT_PLOT_AXIS *A, GMT_LONG below, GMT_LONG annotate)
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
	double *knots, *knots_p;	/* Array pointers with tick/annotation knots, the latter for primary annotations */
	double tick_len[6];		/* Ticklengths for each of the 6 axis items */
	double x, sign, len, t_use;	/* Misc. variables */
	double font_size;			/* Annotation font size (ANNOT_FONT_SIZE_PRIMARY or ANNOT_FONT_SIZE_SECONDARY) */
	struct GMT_PLOT_AXIS_ITEM *T;	/* Pointer to the current axis item */
	char string[GMT_CALSTRING_LENGTH];	/* Annotation string */
	char format[GMT_LONG_TEXT];		/* format used for non-time annotations */
	char xy[2] = {'h', 'w'};
	char cmd[BUFSIZ];
	GMT_LONG rot[2], font;
	/* Initialize parameters for this axis */

	axis = A->item[0].parent;
	if ((check_annotation = GMT_two_annot_items(axis))) {					/* Must worry about annotation overlap */
		GMT_get_primary_annot (A, &primary, &secondary);				/* Find primary and secondary axis items */
		np = GMT_coordinate_array (val0, val1, &A->item[primary], &knots_p);		/* Get all the primary tick annotation knots */
	}
	if (axis == 1) below = !below;
	len = (gmtdefs.tick_length > 0.0) ? gmtdefs.tick_length : 0.0;				/* Tick length if directed outward */
	sign = (below) ? -1.0 : 1.0;								/* since annotations go either below or above */
	tick_len[0] = tick_len[2] = sign * gmtdefs.tick_length;					/* Initialize the tick lengths */
	tick_len[1] = 3.0 * sign * gmtdefs.tick_length;
	tick_len[3] = (A->item[GMT_ANNOT_UPPER].active) ? tick_len[0] : 3.0 * sign * gmtdefs.tick_length;
	tick_len[4] = 0.5 * sign * gmtdefs.tick_length;
	tick_len[5] = 0.75 * sign * gmtdefs.tick_length;
	if (A->type != GMT_TIME) GMT_get_format (GMT_get_map_interval (axis, GMT_ANNOT_UPPER), A->unit, A->prefix, format);	/* Set the annotation format template */

	/* Ready to draw axis */

	ps_setfont (gmtdefs.annot_font[0]);
	if (axis == 0) {
		if (below) ps_comment ("Start of lower x-axis"); else ps_comment ("Start of upper x-axis");
		ps_transrotate (x0, y0, 0.0);
	}
	else if (axis == 1) {
		if (below) ps_comment ("Start of right y-axis"); else ps_comment ("Start of left y-axis");
		ps_transrotate (x0, y0, 90.0);
	}

	/* Create PostScript definitions of various lengths and font sizes */

	GMT_define_PS_items (A, below, annotate);

	ps_comment ("Axis tick marks");
	GMT_setpen (&gmtdefs.frame_pen);
	ps_segment (0.0, 0.0, length, 0.0);
	if (gmtdefs.basemap_type == GMT_IS_GRAPH) {	/* Extend axis 7.5% with an arrow */
		struct GMT_FILL arrow;
		double x2, v_w, h_w, h_l;
		GMT_init_fill (&arrow, gmtdefs.frame_pen.rgb[0], gmtdefs.frame_pen.rgb[1], gmtdefs.frame_pen.rgb[2]);
		x2 = 1.075 * length;
		v_w = rint (gmtdefs.dpi * gmtdefs.frame_pen.width / 72.0) / gmtdefs.dpi;	/* To ensure it matches the pen thickness which is integer rounded */
		h_w = 5.0 * v_w;
		h_l = 2 * h_w;
		GMT_vector (length, 0.0, x2, 0.0, 0.0, v_w, h_l, h_w, gmtdefs.vector_shape, &arrow, FALSE);
	}
	GMT_setpen (&gmtdefs.tick_pen);

	rot[0] = (A->item[GMT_ANNOT_UPPER].active && axis == 1 && gmtdefs.y_axis_type == 0) ? 1 : 0;
	rot[1] = (A->item[GMT_ANNOT_LOWER].active && axis == 1 && gmtdefs.y_axis_type == 0) ? 1 : 0;
	for (k = 0; k < GMT_GRID_UPPER; k++) {	/* For each one of the 6 axis items (gridlines are done separately) */

		T = &A->item[k];		/* Get pointer to this item */
		if (!T->active) continue;	/* Don't want this item plotted - goto next item */

		is_interval = GMT_interval_axis_item(k);			/* Interval or tick mark annotation? */
		nx = GMT_coordinate_array (val0, val1, &A->item[k], &knots);	/* Get all the annotation tick knots */

		/* First plot all the tick marks */

		do_tick = !((T->unit == 'K' || T->unit == 'k') && (T->interval > 1 && fmod (T->interval, 7.0) > 0.0));
		for (i = 0; do_tick && i < nx; i++) {
			if (knots[i] < (val0 - GMT_CONV_LIMIT) || knots[i] > (val1 + GMT_CONV_LIMIT)) continue;	/* Outside the range */
			(axis == 0) ? GMT_x_to_xx (knots[i], &x) : GMT_y_to_yy (knots[i], &x);	/* Convert to inches on the page */
			ps_segment (x, 0.0, x, tick_len[k]);
		}

		do_annot = ((k < GMT_TICK_UPPER && annotate && !project_info.degree[axis]) && !(T->unit == 'r'));	/* Cannot annotate a Gregorian week */
		if (do_annot) {	/* Then do annotations too - here just set text height/width parameters in PostScript */

			annot_pos = GMT_lower_axis_item(k);							/* 1 means lower annotation, 0 means upper (close to axis) */
			font_size = gmtdefs.annot_font_size[annot_pos];		/* Set the size of the font to use */
			font = gmtdefs.annot_font[annot_pos];				/* Set the id of the font to use */

			for (i = 0; k < 4 && i < (nx - is_interval); i++) {
				if (GMT_annot_pos (val0, val1, T, &knots[i], &t_use)) continue;				/* Outside range */
				if (GMT_skip_second_annot (k, knots[i], knots_p, np, primary, secondary)) continue;	/* Secondary annotation skipped when coinciding with primary annotation */
				GMT_get_coordinate_label (string, &GMT_plot_calclock, format, T, knots[i]);		/* Get annotation string */
				ps_textdim ("PSL_dim", font_size, font, string);				/* Get and set string dimensions in PostScript */
				sprintf (cmd, "PSL_dim_%c PSL_AH%ld gt {/PSL_AH%ld PSL_dim_%c def} if", xy[rot[annot_pos]], annot_pos, annot_pos, xy[rot[annot_pos]]);		/* Update the longest annotation */
				ps_command (cmd);
			}
		}

		if (nx) GMT_free ((void *)knots);
	}

	/* Here, PSL_AH0, PSL_AH1, and PSL_LH have been defined.  We may now set the y offsets for text baselines */

	GMT_define_baselines ();

	/* Now do annotations, if requested */

	for (k = 0; annotate && !project_info.degree[axis] && k < GMT_TICK_UPPER; k++) {	/* For each one of the 6 axis items (gridlines are done separately) */

		T = &A->item[k];					/* Get pointer to this item */
		if (!T->active) continue;				/* Don't want this item plotted - goto next item */
		if (T->unit == 'r') continue;				/* Cannot annotate a Gregorian week */

		is_interval = GMT_interval_axis_item(k);			/* Interval or tick mark annotation? */
		nx = GMT_coordinate_array (val0, val1, &A->item[k], &knots);	/* Get all the annotation tick knots */

		annot_pos = GMT_lower_axis_item(k);							/* 1 means lower annotation, 0 means upper (close to axis) */
		font_size = gmtdefs.annot_font_size[annot_pos];		/* Set the size of the font to use */
		font = gmtdefs.annot_font[annot_pos];				/* Set the id of the font to use */

		for (i = 0; k < 4 && i < (nx - is_interval); i++) {
			if (GMT_annot_pos (val0, val1, T, &knots[i], &t_use)) continue;				/* Outside range */
			if (GMT_skip_second_annot (k, knots[i], knots_p, np, primary, secondary)) continue;	/* Secondary annotation skipped when coinciding with primary annotation */
			(axis == 0) ? GMT_x_to_xx (t_use, &x) : GMT_y_to_yy (t_use, &x);	/* Get annotation position */
			ps_set_length ("PSL_x", x);
			GMT_get_coordinate_label (string, &GMT_plot_calclock, format, T, knots[i]);		/* Get annotation string */
			if (rot[annot_pos]) {	/* Rotate and adjust annotation in y direction */
				sprintf (cmd, "PSL_x PSL_A%ld_y M", annot_pos);					/* Move to new anchor point */
				ps_command (cmd);
				ps_text (0.0, 0.0, -font_size, string, -90.0, 7, 0);
			}
			else {			/* Just center horizontally */
				sprintf (cmd, "PSL_x PSL_A%ld_y M", annot_pos);					/* Move to new anchor point */
				ps_command (cmd);
				ps_text (0.0, 0.0, -font_size, string, 0.0, 2, 0);
			}
		}

		if (nx) GMT_free ((void *)knots);
	}
	if (np) GMT_free ((void *)knots_p);

	/* Finally do axis label */

	if (A->label[0] && annotate && !project_info.degree[axis]) {
		ps_set_length ("PSL_x", 0.5 * length);
		ps_textdim ("PSL_dim", gmtdefs.label_font_size, gmtdefs.label_font, A->label);	/* Get and set string dimensions in PostScript */
		ps_command ("PSL_x PSL_L_y M");				/* Move to new anchor point */
		ps_setfont (gmtdefs.label_font);
		ps_text (0.0, 0.0, -gmtdefs.label_font_size, A->label, 0.0, 2, 0);
	}
	if (axis == 0) {
		ps_rotatetrans  (-x0, -y0, 0.0);
		if (below) ps_comment ("End of lower x-axis"); else ps_comment ("End of upper x-axis");
	}
	else if (axis == 1) {
		ps_rotatetrans  (-x0, -y0, -90.0);
		if (below) ps_comment ("End of right y-axis"); else ps_comment ("End of left y-axis");
	}
}

void GMT_define_PS_items (struct GMT_PLOT_AXIS *A, GMT_LONG below, GMT_LONG annotate)
{
	/* GMT relies on the PostScript engine to calculate dimensions of
	 * text items.  Therefore, the calculation of text baseline offsets
	 * (i.e., distance from axis to base of annotations) becomes a bit
	 * tricky and must be done in PostScript language.
	 *
	 * Here we set PostScript definitions of various lengths and font
	 * sizes used in the frame annotation machinery */

	if (below) /* Axis is either lower x-axis or LEFT y-axis, with annotations "below" the axis */
		ps_command ("/PSL_sign -1 def");
	else
		ps_command ("/PSL_sign 1 def");	/* Annotate "above" axis */
	if (annotate)	/* Want annotations...  */
		ps_command ("/PSL_do_annot 1 def");
	else		/* ...or not */
		ps_command ("/PSL_do_annot 0 def");
	if (A->label[0])	/* The label is not NULL */
		ps_command ("/PSL_do_label 1 def");
	else			/* No label present */
		ps_command ("/PSL_do_label 0 def");
	if (A->item[GMT_ANNOT_LOWER].active || A->item[GMT_INTV_LOWER].active)	/* Secondary annotations requested */
		ps_command ("/PSL_do_A1 1 def");
	else	/* No secondary items */
		ps_command ("/PSL_do_A1 0 def");
	ps_set_length ("PSL_TL1", gmtdefs.tick_length);		/* Length of tickmark (could be negative) */
	ps_set_length ("PSL_AO0", gmtdefs.annot_offset[0]);	/* Offset between tick and primary annotation */
	ps_set_length ("PSL_AO1", gmtdefs.annot_offset[1]);	/* Offset between tick and secondary annotation */
	ps_set_length ("PSL_LO",  gmtdefs.label_offset);	/* Offset between annotation and label */
	ps_set_height ("PSL_AF0", gmtdefs.annot_font_size[0]);	/* Primary font size */
	ps_set_height ("PSL_AF1", gmtdefs.annot_font_size[1]);	/* Secondary font size */
	ps_set_height ("PSL_LF",  gmtdefs.label_font_size);	/* Label font size */
	ps_set_length ("PSL_AH0", 0.0);	/* The loop over level 0 annotations will reset this to the tallest annotation */
	ps_set_length ("PSL_AH1", 0.0);	/* The loop over level 1 annotations will reset this to the tallest annotation */
	ps_textdim ("PSL_dim", gmtdefs.label_font_size, gmtdefs.label_font, "M");	/* Get and set string dimensions in PostScript */
	ps_command ("PSL_dim_h PSL_dim_d sub /PSL_LH exch def"); /* XXX Might just need to be PSL_dim_h XXX */
}

void GMT_define_baselines ()
{
	/* Given the sizes and lengths of text, offsets, and ticks, calculate the offset from the
	 * axis to the baseline of each annotation class:
	 * PSL_A0_y:	Base of primary annotation
	 * PSL_A1_y:	Base of secondary annotation
	 * PSL_L_y:	Base of axis label
	 * PSL_H_y:	Base of plot title (set elsewhere)
	 */

	ps_command ("/PSL_A0_y PSL_AO0 PSL_TL1 PSL_AO0 mul 0 gt {PSL_TL1 add} if PSL_AO0 0 gt {PSL_sign 0 lt {PSL_AH0} {0} ifelse add} if PSL_sign mul def");
	ps_command ("/PSL_A1_y PSL_A0_y abs PSL_AO1 add PSL_sign 0 lt {PSL_AH1} {PSL_AH0} ifelse add PSL_sign mul def");
	ps_command ("/PSL_L_y PSL_A1_y abs PSL_LO add PSL_sign 0 lt {PSL_LH} {PSL_AH1} ifelse add PSL_sign mul def");
}

void GMT_linearx_grid (double w, double e, double s, double n, double dval)
{
	double *x, ys, yn, p_cap = 0.0, cap_start[2] = {0.0, 0.0}, cap_stop[2] = {0.0, 0.0};
	GMT_LONG i, nx;
	GMT_LONG cap = FALSE;

	if (GMT_POLE_IS_POINT) {	/* Might have two separate domains of gridlines */
		if (project_info.projection == GMT_POLAR) {	/* Different for polar graphs since "lat" = 0 is at the center */
			ys = cap_stop[0] = cap_stop[1] = p_cap = 90.0 - gmtdefs.polar_cap[0];
			yn = n;
			cap_start[0] = cap_start[1] = 0.0;
		}
		else {
			p_cap = gmtdefs.polar_cap[0];
			ys = MAX (s, -p_cap);
			yn = MIN (n, p_cap);
			cap_start[0] = s;
			cap_stop[0]  = -p_cap ;
			cap_start[1] = p_cap;
			cap_stop[1]  = n;
		}
		cap = !GMT_IS_ZERO (gmtdefs.polar_cap[0] - 90.0);
	}
	else {
		ys = s;
		yn = n;
	}
	nx = GMT_linear_array (w, e, dval, frame_info.axis[0].phase, &x);
	for (i = 0; i < nx; i++) GMT_map_lonline (x[i], ys, yn);
	if (nx) GMT_free ((void *)x);

	if (cap) {	/* Also draw the polar cap(s) */
		nx = 0;
		if (s < -gmtdefs.polar_cap[0]) {	/* Must draw some or all of the S polar cap */
			nx = GMT_linear_array (w, e, gmtdefs.polar_cap[1], frame_info.axis[0].phase, &x);
			for (i = 0; i < nx; i++) GMT_map_lonline (x[i], cap_start[0], cap_stop[0]);
			GMT_map_latline (-p_cap, w, e);
		}
		if (n > gmtdefs.polar_cap[0]) {	/* Must draw some or all of the N polar cap */
			if (nx == 0) nx = GMT_linear_array (w, e, gmtdefs.polar_cap[1], frame_info.axis[0].phase, &x);
			for (i = 0; i < nx; i++) GMT_map_lonline (x[i], cap_start[1], cap_stop[1]);
			GMT_map_latline (p_cap, w, e);
		}
		if (nx) GMT_free ((void *)x);
	}

}

void GMT_lineary_grid (double w, double e, double s, double n, double dval)
{
	double *y;
	GMT_LONG i, ny;

	if (project_info.z_down) {
		ny = GMT_linear_array (0.0, n-s, dval, frame_info.axis[1].phase, &y);
		for (i = 0; i < ny; i++) y[i] = project_info.n - y[i];	/* These are the radial values needed for positioning */
	}
	else
		ny = GMT_linear_array (s, n, dval, frame_info.axis[1].phase, &y);
	for (i = 0; i < ny; i++) GMT_map_latline (y[i], w, e);
	if (ny) GMT_free ((void *)y);

}

void GMT_timex_grid (double w, double e, double s, double n, GMT_LONG item)
{
	GMT_LONG i, nx;
	double *x;

	nx = GMT_time_array (w, e, &frame_info.axis[0].item[item], &x);
	for (i = 0; i < nx; i++) GMT_geosegment (x[i], s, x[i], n);
	if (nx) GMT_free ((void *)x);
}

void GMT_timey_grid (double w, double e, double s, double n, GMT_LONG item)
{
	GMT_LONG i, ny;
	double *y;

	ny = GMT_time_array (s, n, &frame_info.axis[1].item[item], &y);
	for (i = 0; i < ny; i++) GMT_geosegment (w, y[i], e, y[i]);
	if (ny) GMT_free ((void *)y);
}

void GMT_logx_grid (double w, double e, double s, double n, double dval)
{
	GMT_LONG i, nx;
	double *x;

	nx = GMT_log_array (w, e, dval, &x);
	for (i = 0; i < nx; i++) GMT_geosegment (x[i], s, x[i], n);
	if (nx) GMT_free ((void *)x);
}

void GMT_logy_grid (double w, double e, double s, double n, double dval)
{
	GMT_LONG i, ny;
	double *y;

	ny = GMT_log_array (s, n, dval, &y);
	for (i = 0; i < ny; i++) GMT_geosegment (w, y[i], e, y[i]);
	if (ny) GMT_free ((void *)y);
}

void GMT_powx_grid (double w, double e, double s, double n, double dval)
{
	GMT_LONG i, nx;
	double *x;

	nx = GMT_pow_array (w, e, dval, 0, &x);
	for (i = 0; i < nx; i++) GMT_geosegment (x[i], s, x[i], n);
	if (nx) GMT_free ((void *)x);
}

void GMT_powy_grid (double w, double e, double s, double n, double dval)
{
	GMT_LONG i, ny;
	double *y;

	ny = GMT_pow_array (s, n, dval, 1, &y);
	for (i = 0; i < ny; i++) GMT_geosegment (w, y[i], e, y[i]);
	if (ny) GMT_free ((void *)y);
}

/*	FANCY RECTANGULAR PROJECTION MAP BOUNDARY	*/

void GMT_fancy_map_boundary (double w, double e, double s, double n)
{
	double fwidth;
	GMT_LONG dual = FALSE;
	GMT_LONG fat_pen, thin_pen;

	if (gmtdefs.basemap_type == GMT_IS_PLAIN) {	/* Draw plain boundary and return */
		GMT_wesn_map_boundary (w, e, s, n);
		return;
	}

	ps_setpaint (gmtdefs.basemap_frame_rgb);

	fwidth = fabs (gmtdefs.frame_width);
	if (frame_info.axis[1].item[GMT_TICK_LOWER].active) {	/* Need two-layer frame */
		fwidth *= 0.5;
		dual = TRUE;
	}

	fat_pen = irint (fwidth * gmtdefs.dpi);
	thin_pen = irint (0.1 * fwidth * gmtdefs.dpi);

	ps_setline (thin_pen);

	(void) GMT_fancy_frame_straight_outline (w, s, e, s, 0, dual);
	(void) GMT_fancy_frame_straight_outline (e, s, e, n, 1, dual);
	(void) GMT_fancy_frame_straight_outline (e, n, w, n, 2, dual);
	(void) GMT_fancy_frame_straight_outline (w, n, w, s, 3, dual);

	GMT_rounded_framecorners (w, e, s, n, 270.0, 90.0, dual);

	/* Draw frame grid for W/E boundaries */

	ps_setline(fat_pen);

	GMT_fancy_frame_straightlat_checkers (w, e, s, n, 90.0, 90.0, dual);
	GMT_fancy_frame_straightlon_checkers (w, e, s, n, 180.0, 0.0, dual);

	ps_setline (thin_pen);
}

void GMT_fancy_frame_extension (double angle, double extend[2])
{
	/* Given the angle of the axis, return the coordinate adjustments needed to
	 * allow for a possible extension of the line (extend[0|1] */

	double s, c;
	sincosd (angle, &s, &c);
	extend[0] = (gmtdefs.basemap_type == GMT_IS_ROUNDED) ? 0.0 : gmtdefs.frame_width * c;
	extend[1] = (gmtdefs.basemap_type == GMT_IS_ROUNDED) ? 0.0 : gmtdefs.frame_width * s;
}

void GMT_fancy_frame_offset (double angle, double shift[2])
{
	/* Given the angle of the axis, return the coordinate adjustments needed to
	 * shift in order to plot the outer 1-2 parallell frame lines (shift[0|1] */

	double s, c;
	sincosd (angle, &s, &c);
	shift[0] =  gmtdefs.frame_width * s;
	shift[1] = -gmtdefs.frame_width * c;
}

void GMT_fancy_frame_straightlat_checkers (double w, double e, double s, double n, double angle_w, double angle_e, GMT_LONG secondary_too)
{
	GMT_LONG i, k, ny, item[2] = {GMT_TICK_UPPER, GMT_TICK_LOWER};
	double dy, s1, val, v1, v2, x1, x2, x3, y1, y2, y3, shift_w[2], shift_e[2], scale[2];
	GMT_LONG shade, do_it;

	scale[0] = (secondary_too) ? 0.5 : 1.0;
	scale[1] = 1.5;
	GMT_fancy_frame_offset (angle_w, shift_w);
	GMT_fancy_frame_offset (angle_e, shift_e);

	/* Tick S-N axes */

	for (k = 0; k < 1 + secondary_too; k++) {
		if (frame_info.axis[1].item[item[k]].active) {
			dy = GMT_get_map_interval (1, item[k]);
			shade = ((int)floor ((s - frame_info.axis[1].phase) / dy) + 1) % 2;
			s1 = floor((s - frame_info.axis[1].phase)/dy) * dy + frame_info.axis[1].phase;
			ny = (s1 > n) ? -1 : (int)((n-s1) / dy + GMT_SMALL);
			for (i = 0; i <= ny; i++) {
				val = s1 + i * dy;
				v1 = (val < s) ? s : val;
				GMT_geo_to_xy (w, v1, &x1, &y1);
				GMT_geo_to_xy (e, v1, &x2, &y2);
				if (shade) {
					v2 = val + dy;
					if (v2 > n) v2 = n;
					do_it = ((v2 - v1) > GMT_CONV_LIMIT);
					if (do_it && frame_info.side[3]) {
						GMT_geo_to_xy (w, v2, &x3, &y3);
						ps_segment (x1-0.5*scale[k]*shift_w[0], y1-0.5*scale[k]*shift_w[1], x3-0.5*scale[k]*shift_w[0], y3-0.5*scale[k]*shift_w[1]);
					}
					if (do_it && frame_info.side[1]) {
						GMT_geo_to_xy (e, v2, &x3, &y3);
						ps_segment (x2+0.5*scale[k]*shift_e[0], y2+0.5*scale[k]*shift_e[1], x3+0.5*scale[k]*shift_e[0], y3+0.5*scale[k]*shift_e[1]);
					}
					shade = FALSE;
				}
				else
					shade = TRUE;
			}
		}
	}
}

void GMT_fancy_frame_straightlon_checkers (double w, double e, double s, double n, double angle_s, double angle_n, GMT_LONG secondary_too)
{
	GMT_LONG i, k, nx, item[2] = {GMT_TICK_UPPER, GMT_TICK_LOWER};
	double dx, w1, val, v1, v2, x1, x2, x3, y1, y2, y3, shift_s[2], shift_n[2], scale[2];
	GMT_LONG shade, do_it;

	scale[0] = (secondary_too) ? 0.5 : 1.0;
	scale[1] = 1.5;
	GMT_fancy_frame_offset (angle_s, shift_s);
	GMT_fancy_frame_offset (angle_n, shift_n);

	for (k = 0; k < 1 + secondary_too; k++) {
		if (frame_info.axis[0].item[item[k]].active) {
			dx = GMT_get_map_interval (0, item[k]);
			shade = ((int)floor ((w - frame_info.axis[0].phase)/ dx) + 1) % 2;
			w1 = floor ((w - frame_info.axis[0].phase)/ dx) * dx + frame_info.axis[0].phase;
			nx = (w1 > e) ? -1 : (int)((e - w1) / dx + GMT_SMALL);
			for (i = 0; i <= nx; i++) {
				val = w1 + i * dx;
				v1 = (val < w) ? w : val;
				GMT_geo_to_xy (v1, s, &x1, &y1);
				GMT_geo_to_xy (v1, n, &x2, &y2);
				if (shade) {
					v2 = val + dx;
					if (v2 > e) v2 = e;
					do_it = ((v2 - v1) > GMT_CONV_LIMIT);
					if (do_it && frame_info.side[0]) {
						GMT_geo_to_xy (v2, s, &x3, &y3);
						ps_segment (x1-0.5*scale[k]*shift_s[0], y1-0.5*scale[k]*shift_s[1], x3-0.5*scale[k]*shift_s[0], y3-0.5*scale[k]*shift_s[1]);
					}
					if (do_it && frame_info.side[2]) {
						GMT_geo_to_xy (v2, n, &x3, &y3);
						ps_segment (x2-0.5*scale[k]*shift_n[0], y2-0.5*scale[k]*shift_n[1], x3-0.5*scale[k]*shift_n[0], y3-0.5*scale[k]*shift_n[1]);
					}
					shade = FALSE;
				}
				else
					shade = TRUE;
			}
		}
	}
}

void GMT_fancy_frame_curvedlon_checkers (double w, double e, double s, double n, double radius_s, double radius_n, GMT_LONG secondary_too)
{
	GMT_LONG i, k, nx, item[2] = {GMT_TICK_UPPER, GMT_TICK_LOWER};
	GMT_LONG shade, do_it;
	double dx, w1, v1, v2, val, x1, x2, y1, y2, az1, az2, dr, scale[2];

	scale[0] = (secondary_too) ? 0.5 : 1.0;
	scale[1] = 1.5;
	dr = 0.5 * gmtdefs.frame_width;

	for (k = 0; k < 1 + secondary_too; k++) {
		if (frame_info.axis[0].item[item[k]].active) {
			dx = GMT_get_map_interval (0, item[k]);
			shade = ((int)floor ((w - frame_info.axis[0].phase) / dx) + 1) % 2;
			w1 = floor((w - frame_info.axis[0].phase)/dx) * dx + frame_info.axis[0].phase;
			nx = (w1 > e) ? -1 : (int)((e-w1) / dx + GMT_SMALL);
			for (i = 0; i <= nx; i++) {
				val = w1 + i * dx;
				v1 = (val < w) ? w : val;
				if (shade) {
					v2 = val + dx;
					if (v2 > e) v2 = e;
					do_it = ((v2 - v1) > GMT_CONV_LIMIT);
					if (do_it && frame_info.side[0]) {
						GMT_geo_to_xy (v2, s, &x1, &y1);
						GMT_geo_to_xy (v1, s, &x2, &y2);
						az1 = d_atan2d (y1 - project_info.c_y0, x1 - project_info.c_x0);
						az2 = d_atan2d (y2 - project_info.c_y0, x2 - project_info.c_x0);
						if (project_info.north_pole) {
							if (az1 < az2) az1 += 360.0;
							ps_arc (project_info.c_x0, project_info.c_y0, radius_s+scale[k]*dr, az2, az1, PSL_ARC_DRAW);
						}
						else {
							if (az2 < az1) az2 += 360.0;
							ps_arc (project_info.c_x0, project_info.c_y0, radius_s-scale[k]*dr, az1, az2, PSL_ARC_DRAW);
						}
					}
					if (do_it && frame_info.side[2]) {
						GMT_geo_to_xy (v2, n, &x1, &y1);
						GMT_geo_to_xy (v1, n, &x2, &y2);
						az1 = d_atan2d (y1 - project_info.c_y0, x1 - project_info.c_x0);
						az2 = d_atan2d (y2 - project_info.c_y0, x2 - project_info.c_x0);
						if (project_info.north_pole) {
							if (az1 < az2) az1 += 360.0;
							ps_arc (project_info.c_x0, project_info.c_y0, radius_n-scale[k]*dr, az2, az1, PSL_ARC_DRAW);
						}
						else {
							if (az2 < az1) az2 += 360.0;
							ps_arc (project_info.c_x0, project_info.c_y0, radius_n+scale[k]*dr, az1, az2, PSL_ARC_DRAW);
						}
					}
					shade = FALSE;
				}
				else
					shade = TRUE;
			}
		}
	}
}

double GMT_fancy_frame_straight_outline (double lonA, double latA, double lonB, double latB, GMT_LONG side, GMT_LONG secondary_too)
{
	GMT_LONG k;
	double scale = 1.0, x[2], y[2], angle, s, c, dx, dy, Ldx, Ldy;

	if (!frame_info.side[side]) return (0.0);	/* Do not draw this frame side */

	if (secondary_too) scale = 0.5;

	GMT_geo_to_xy (lonA, latA, &x[0], &y[0]);
	GMT_geo_to_xy (lonB, latB, &x[1], &y[1]);
	angle = d_atan2 (y[1] - y[0], x[1] - x[0]);
	sincos (angle, &s, &c);
	Ldx = (gmtdefs.basemap_type == GMT_IS_ROUNDED) ? 0.0 : gmtdefs.frame_width * c;
	Ldy = (gmtdefs.basemap_type == GMT_IS_ROUNDED) ? 0.0 : gmtdefs.frame_width * s;
	dx =  gmtdefs.frame_width * s;
	dy = -gmtdefs.frame_width * c;
	ps_segment (x[0]-Ldx, y[0]-Ldy, x[1]+Ldx, y[1]+Ldy);
	for (k = 0; k < 1 + secondary_too; k++) {
		x[0] += scale*dx;
		y[0] += scale*dy;
		x[1] += scale*dx;
		y[1] += scale*dy;
		ps_segment (x[0]-Ldx, y[0]-Ldy, x[1]+Ldx, y[1]+Ldy);
	}
	return (angle);
}

double GMT_fancy_frame_curved_outline (double lonA, double latA, double lonB, double latB, GMT_LONG side, GMT_LONG secondary_too)
{
	double scale[2] = {1.0, 1.0}, escl, x1, x2, y1, y2, radius, dr, r_inc, az1, az2, da0, da, width, s;

	if (!frame_info.side[side]) return (0.0);

	if (secondary_too) scale[0] = scale[1] = 0.5;
	width = gmtdefs.frame_width;
	escl = (gmtdefs.basemap_type == GMT_IS_ROUNDED) ? 0.0 : 1.0;	/* Want rounded corners */
	GMT_geo_to_xy (lonA, latA, &x1, &y1);
	GMT_geo_to_xy (lonB, latB, &x2, &y2);
	radius = hypot (x1 - project_info.c_x0, y1 - project_info.c_y0);
	dr = 0.5 * width;
	s = ((project_info.north_pole && side == 2) || (!project_info.north_pole && side == 0)) ? -1.0 : +1.0;	/* North: needs shorter radius.  South: Needs longer radius (opposite in S hemi) */
	r_inc = s*scale[0] * width;
	if (GMT_IS_AZIMUTHAL && GMT_360_RANGE (lonA, lonB)) {	/* Full 360-degree cirle */
		ps_arc (project_info.c_x0, project_info.c_y0, radius, 0.0, 360.0, PSL_ARC_DRAW);
		ps_arc (project_info.c_x0, project_info.c_y0, radius + r_inc, 0.0, 360.0, PSL_ARC_DRAW);
		if (secondary_too)  ps_arc (project_info.c_x0, project_info.c_y0, radius + 2.0 * r_inc, 0.0, 360.0, PSL_ARC_DRAW);
	}
	else {
		az1 = d_atan2d (y1 - project_info.c_y0, x1 - project_info.c_x0);
		az2 = d_atan2d (y2 - project_info.c_y0, x2 - project_info.c_x0);
		if (!project_info.north_pole) d_swap (az1, az2);	/* In S hemisphere, must draw in opposite direction */
		while (az1 < 0.0) az1 += 360.0;	/* Wind az1 to be in the 0-360 range */
		while (az2 < az1) az2 += 360.0;	/* Likewise ensure az1 > az1 and is now in the 0-720 range */
		da0 = R2D * escl * width /radius;
		da  = R2D * escl * width / (radius + r_inc);
		ps_arc (project_info.c_x0, project_info.c_y0, radius, az1-da0, az2+da0, PSL_ARC_DRAW);
		ps_arc (project_info.c_x0, project_info.c_y0, radius + r_inc, az1-da, az2+da, PSL_ARC_DRAW);
		if (secondary_too) {
			r_inc *= 2.0;
			da = R2D * escl * width / (radius + r_inc);
			ps_arc (project_info.c_x0, project_info.c_y0, radius + r_inc, az1-da, az2+da, PSL_ARC_DRAW);
		}
	}
	return (radius);
}

void GMT_rounded_framecorners (double w, double e, double s, double n, double anglew, double anglee, GMT_LONG secondary_too)
{
	GMT_LONG k;
	double x, y, width;

	if (gmtdefs.basemap_type != GMT_IS_ROUNDED) return;	/* Only do this if rounded corners are requested */

	width = ((secondary_too) ? 0.5 : 1.0) * fabs (gmtdefs.frame_width);
	for (k = 0; k < 1 + secondary_too; k++) {
		if (frame_info.side[0] && frame_info.side[1]) {
			GMT_geo_to_xy (e, s, &x, &y);
			ps_arc (x, y, (k+1)*width, 180.0+anglee, 270.0+anglee, PSL_ARC_DRAW);
		}
		if (frame_info.side[1] && frame_info.side[2]) {
			GMT_geo_to_xy (e, n, &x, &y);
			ps_arc (x, y, (k+1)*width, 270.0+anglee, 360.0+anglee, PSL_ARC_DRAW);
		}
		if (frame_info.side[2] && frame_info.side[3]) {
			GMT_geo_to_xy (w, n, &x, &y);
			ps_arc (x, y, (k+1)*width, 180.0+anglew, 270.0+anglew, PSL_ARC_DRAW);
		}
		if (frame_info.side[3] && frame_info.side[0]) {
			GMT_geo_to_xy (w, s, &x, &y);
			ps_arc (x, y, (k+1)*width, 270.0+anglew, 360.0+anglew, PSL_ARC_DRAW);
		}
		if ((GMT_IS_AZIMUTHAL || GMT_IS_CONICAL) && frame_info.side[3] && frame_info.side[1]) {	/* Round off the pointy head? */
			if (GMT_IS_ZERO (project_info.n - 90.0)) {
				GMT_geo_to_xy (w, n, &x, &y);
				ps_arc (x, y, (k+1)*width, anglee, 180.0+anglew, PSL_ARC_DRAW);
			}
			else if (GMT_IS_ZERO (project_info.s + 90.0)) {
				GMT_geo_to_xy (w, s, &x, &y);
				ps_arc (x, y, (k+1)*width, anglew-90.0, anglee-90.0, PSL_ARC_DRAW);
			}
		}
	}
}

/*	GMT_POLAR (S or N) PROJECTION MAP BOUNDARY	*/

void GMT_polar_map_boundary (double w, double e, double s, double n)
{
	GMT_LONG thin_pen, fat_pen;
	GMT_LONG dual = FALSE;
	double anglew, anglee, fwidth, radiuss, radiusn;

	if (!project_info.region) { /* Draw rectangular boundary and return */
		GMT_rect_map_boundary (0.0, 0.0, project_info.xmax, project_info.ymax);
		return;
	}

	if (!project_info.north_pole && s <= -90.0) /* Cannot have southern boundary */
		frame_info.side[0] = FALSE;
	if (project_info.north_pole && n >= 90.0) /* Cannot have northern boundary */
		frame_info.side[2] = FALSE;
	if (GMT_360_RANGE (w,e) || GMT_IS_ZERO (e - w)) {
		frame_info.side[1] = FALSE;
		frame_info.side[3] = FALSE;
	}

	if (gmtdefs.basemap_type == GMT_IS_PLAIN) { /* Draw plain boundary and return */
		GMT_wesn_map_boundary (w, e, s, n);
		return;
	}

	/* Here draw fancy map boundary */

	fwidth = fabs (gmtdefs.frame_width);
	if (frame_info.axis[1].item[GMT_TICK_LOWER].active) {	/* Need two-layer frame */
		fwidth *= 0.5;
		dual = TRUE;
	}

	ps_setpaint (gmtdefs.basemap_frame_rgb);

	fat_pen  = irint (fwidth * gmtdefs.dpi);
	thin_pen = irint (0.1 * fwidth * gmtdefs.dpi);
	ps_setline (thin_pen);

	radiuss = GMT_fancy_frame_curved_outline (w, s, e, s, 0, dual);
	anglee  = GMT_fancy_frame_straight_outline (e, s, e, n, 1, dual);
	radiusn = GMT_fancy_frame_curved_outline (w, n, e, n, 2, dual);
	anglew  = GMT_fancy_frame_straight_outline (w, n, w, s, 3, dual);

	GMT_rounded_framecorners (w, e, s, n, anglew * R2D, anglee * R2D, dual);

	/* Tick N-S and W-E axes */

	ps_setline (fat_pen);
	GMT_fancy_frame_straightlat_checkers (w, e, s, n, 180.0 + anglew * R2D, anglee * R2D, dual);
	GMT_fancy_frame_curvedlon_checkers (w, e, s, n, radiuss, radiusn, dual);

	ps_setline (thin_pen);
}

/*	CONIC PROJECTION MAP BOUNDARY	*/

void GMT_conic_map_boundary (double w, double e, double s, double n)
{
	GMT_LONG thin_pen, fat_pen;
	GMT_LONG dual = FALSE;
	double anglew, anglee, fwidth, radiuss, radiusn;

	if (!project_info.region) { /* Draw rectangular boundary and return */
		GMT_rect_map_boundary (0.0, 0.0, project_info.xmax, project_info.ymax);
		return;
	}

	if (!project_info.north_pole && s <= -90.0) /* Cannot have southern boundary */
		frame_info.side[0] = FALSE;
	if (project_info.north_pole && n >= 90.0) /* Cannot have northern boundary */
		frame_info.side[2] = FALSE;

	if (gmtdefs.basemap_type == GMT_IS_PLAIN) { /* Draw plain boundary and return */
		GMT_wesn_map_boundary (w, e, s, n);
		return;
	}

	/* Here draw fancy map boundary */

	fwidth = fabs (gmtdefs.frame_width);
	if (frame_info.axis[1].item[GMT_TICK_LOWER].active) {	/* Need two-layer frame */
		fwidth *= 0.5;
		dual = TRUE;
	}

	ps_setpaint (gmtdefs.basemap_frame_rgb);

	fat_pen  = irint (fwidth * gmtdefs.dpi);
	thin_pen = irint (0.1 * fwidth * gmtdefs.dpi);
	ps_setline (thin_pen);

	radiuss = GMT_fancy_frame_curved_outline (w, s, e, s, 0, dual);
	anglee  = GMT_fancy_frame_straight_outline (e, s, e, n, 1, dual);
	radiusn = GMT_fancy_frame_curved_outline (w, n, e, n, 2, dual);
	anglew  = GMT_fancy_frame_straight_outline (w, n, w, s, 3, dual);

	GMT_rounded_framecorners (w, e, s, n, anglew * R2D, anglee * R2D, dual);

	/* Tick N-S and W-E axes */

	ps_setline (fat_pen);
	GMT_fancy_frame_straightlat_checkers (w, e, s, n, 180.0 + anglew * R2D, anglee * R2D, dual);
	GMT_fancy_frame_curvedlon_checkers (w, e, s, n, radiuss, radiusn, dual);

	ps_setline (thin_pen);
}

/*	OBLIQUE MERCATOR PROJECTION MAP FUNCTIONS	*/

void GMT_oblmrc_map_boundary (double w, double e, double s, double n)
{
	GMT_rect_map_boundary (0.0, 0.0, project_info.xmax, project_info.ymax);
}

/*	MOLLWEIDE and HAMMER-AITOFF EQUAL AREA PROJECTION MAP FUNCTIONS	*/

void GMT_ellipse_map_boundary (double w, double e, double s, double n)
{

	if (!project_info.region) { /* Draw rectangular boundary and return */
		GMT_rect_map_boundary (0.0, 0.0, project_info.xmax, project_info.ymax);
		return;
	}
	if (project_info.s <= -90.0) /* Cannot have southern boundary */
		frame_info.side[0] = FALSE;
	if (project_info.n >= 90.0) /* Cannot have northern boundary */
		frame_info.side[2] = FALSE;

	GMT_wesn_map_boundary (w, e, s, n);
}

void GMT_basic_map_boundary (double w, double e, double s, double n)
{
	if (!project_info.region) { /* Draw rectangular boundary and return */
		GMT_rect_map_boundary (0.0, 0.0, project_info.xmax, project_info.ymax);
		return;
	}
	GMT_wesn_map_boundary (w, e, s, n);
}

/*
 *	GENERIC MAP PLOTTING FUNCTIONS
 */

void GMT_wesn_map_boundary (double w, double e, double s, double n)
{
	GMT_LONG i, np = 0;
	double *xx, *yy;

	GMT_setpen (&gmtdefs.frame_pen);

	if (frame_info.side[3]) {	/* West */
		np = GMT_map_path (w, s, w, n, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geoz_to_xy (xx[i], yy[i], project_info.z_level, &xx[i], &yy[i]);
		ps_line (xx, yy, np, 3, FALSE);
		GMT_free ((void *)xx);	GMT_free ((void *)yy);
	}
	if (frame_info.side[1]) {	/* East */
		np = GMT_map_path (e, s, e, n, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geoz_to_xy (xx[i], yy[i], project_info.z_level, &xx[i], &yy[i]);
		ps_line (xx, yy, np, 3, FALSE);
		GMT_free ((void *)xx);	GMT_free ((void *)yy);
	}
	if (frame_info.side[0]) {	/* South */
		np = GMT_map_path (w, s, e, s, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geoz_to_xy (xx[i], yy[i], project_info.z_level, &xx[i], &yy[i]);
		ps_line (xx, yy, np, 3, FALSE);
		GMT_free ((void *)xx);	GMT_free ((void *)yy);
	}
	if (frame_info.side[2]) {	/* North */
		np = GMT_map_path (w, n, e, n, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geoz_to_xy (xx[i], yy[i], project_info.z_level, &xx[i], &yy[i]);
		ps_line (xx, yy, np, 3, FALSE);
		GMT_free ((void *)xx);	GMT_free ((void *)yy);
	}
}

void GMT_rect_map_boundary (double x0, double y0, double x1, double y1)
{
	double xt[4], yt[4];

	GMT_xy_do_z_to_xy (x0, y0, project_info.z_level, &xt[0], &yt[0]);
	GMT_xy_do_z_to_xy (x1, y0, project_info.z_level, &xt[1], &yt[1]);
	GMT_xy_do_z_to_xy (x1, y1, project_info.z_level, &xt[2], &yt[2]);
	GMT_xy_do_z_to_xy (x0, y1, project_info.z_level, &xt[3], &yt[3]);

	GMT_setpen (&gmtdefs.frame_pen);

	if (frame_info.side[3]) ps_segment (xt[0], yt[0], xt[3], yt[3]);	/* West */
	if (frame_info.side[1]) ps_segment (xt[1], yt[1], xt[2], yt[2]);	/* East */
	if (frame_info.side[0]) ps_segment (xt[0], yt[0], xt[1], yt[1]);	/* South */
	if (frame_info.side[2]) ps_segment (xt[3], yt[3], xt[2], yt[2]);	/* North */
}

GMT_LONG GMT_genper_map_boundary (double w, double e, double s, double n)
{
	GMT_LONG nr;

	if (!project_info.region) {   /* Draw rectangular boundary and return */
		GMT_rect_map_boundary (0.0, 0.0, project_info.xmax, project_info.ymax);
		return 0;
	}

	GMT_setpen(&gmtdefs.frame_pen);

	nr = GMT_n_lon_nodes + GMT_n_lat_nodes;
	if (nr >= GMT_n_alloc) GMT_get_plot_array();

	if (project_info.g_debug > 1) fprintf (stderr, "genper_map_boundary nr = %ld\n", nr);

	GMT_genper_map_clip_path (nr, GMT_x_plot, GMT_y_plot);

	ps_line (GMT_x_plot, GMT_y_plot, nr, 3, FALSE);

	return 0;
}

void GMT_circle_map_boundary (double w, double e, double s, double n)
{
	GMT_LONG i, nr;
	double x0, y0, a, da, S, C;

	if (!project_info.region) { /* Draw rectangular boundary and return */
		GMT_rect_map_boundary (0.0, 0.0, project_info.xmax, project_info.ymax);
		return;
	}

	GMT_setpen (&gmtdefs.frame_pen);

	if (!project_info.three_D) {
		ps_arc (project_info.r, project_info.r, project_info.r, 0.0, 360.0, PSL_ARC_DRAW);
		return;
	}
	/* Must do 3-D squashed circle */
	nr = GMT_n_lon_nodes + GMT_n_lat_nodes;
	while (nr > GMT_n_alloc) GMT_get_plot_array ();
	da = 2.0 * M_PI / (nr - 1);
	for (i = 0; i < nr; i++) {
		a = i * da;
		sincos (a, &S, &C);
		x0 = project_info.r * C;
		y0 = project_info.r * S;
		GMT_xy_do_z_to_xy (x0, y0, project_info.z_level, &GMT_x_plot[i], &GMT_y_plot[i]);
	}
	GMT_geoz_to_xy (project_info.central_meridian, project_info.pole, project_info.z_level, &x0, &y0);
	ps_transrotate (x0, y0, 0.0);
	ps_line (GMT_x_plot, GMT_y_plot, nr, 3, FALSE);
	ps_rotatetrans (-x0, -y0, 0.0);
}

void GMT_theta_r_map_boundary (double w, double e, double s, double n)
{
	GMT_LONG i, nr;
	double a, da;
	double xx[2], yy[2];

	GMT_setpen (&gmtdefs.frame_pen);

	if (project_info.got_elevations) {
		if (GMT_IS_ZERO (n - 90.0)) frame_info.side[2] = 0;	/* No donuts, please */
	}
	else {
		if (GMT_IS_ZERO (s)) frame_info.side[0] = 0;		/* No donuts, please */
	}
	if (GMT_360_RANGE (w,e) || GMT_IS_ZERO (e - w)) {
		frame_info.side[1] = FALSE;
		frame_info.side[3] = FALSE;
	}
	nr = GMT_n_lon_nodes;
	while (nr > GMT_n_alloc) GMT_get_plot_array ();
	da = fabs (project_info.e - project_info.w) / (nr - 1);
	if (frame_info.side[2]) {
		for (i = 0; i < nr; i++) {
			a = project_info.w + i * da;
			GMT_geoz_to_xy (a, project_info.n, project_info.z_level, &GMT_x_plot[i], &GMT_y_plot[i]);
		}
		ps_line (GMT_x_plot, GMT_y_plot, nr, 3, FALSE);
	}
	if (frame_info.side[0]) {
		for (i = 0; i < nr; i++) {
			a = project_info.w + i * da;
			GMT_geoz_to_xy (a, project_info.s, project_info.z_level, &GMT_x_plot[i], &GMT_y_plot[i]);
		}
		ps_line (GMT_x_plot, GMT_y_plot, nr, 3, FALSE);
	}
	if (frame_info.side[1]) {
		GMT_geoz_to_xy (project_info.e, project_info.s, project_info.z_level, &xx[0], &yy[0]);
		GMT_geoz_to_xy (project_info.e, project_info.n, project_info.z_level, &xx[1], &yy[1]);
		ps_line (xx, yy, (GMT_LONG)2, 3, FALSE);
	}
	if (frame_info.side[3]) {
		GMT_geoz_to_xy (project_info.w, project_info.s, project_info.z_level, &xx[0], &yy[0]);
		GMT_geoz_to_xy (project_info.w, project_info.n, project_info.z_level, &xx[1], &yy[1]);
		ps_line (xx, yy, (GMT_LONG)2, 3, FALSE);
	}
}

void GMT_map_latline (double lat, double west, double east)		/* Draws a line of constant latitude */
{
	GMT_LONG nn;
	double *llon, *llat;
	char text[GMT_LONG_TEXT];

	nn = GMT_latpath (lat, west, east, &llon, &llat);
	GMT_n_plot = GMT_geo_to_xy_line (llon, llat, nn);
	if (GMT_n_plot > 1) {	/* Have at least 2 points */
		sprintf (text, "Lat = %g", lat);
		ps_comment (text);
		if (GMT_parallel_straight) {	/* Simplify to a 2-point straight line */
			GMT_x_plot[1] = GMT_x_plot[GMT_n_plot-1];
			GMT_y_plot[1] = GMT_y_plot[GMT_n_plot-1];
			GMT_pen[1] = GMT_pen[GMT_n_plot-1];
			GMT_plot_line (GMT_x_plot, GMT_y_plot, GMT_pen, 2);
		}
		else
			GMT_plot_line (GMT_x_plot, GMT_y_plot, GMT_pen, GMT_n_plot);
	}
	GMT_free ((void *)llon);
	GMT_free ((void *)llat);
}

void GMT_map_lonline (double lon, double south, double north)	/* Draws a line of constant longitude */
{
	GMT_LONG nn;
	double *llon, *llat;
	char text[GMT_LONG_TEXT];

	nn = GMT_lonpath (lon, south, north, &llon, &llat);
	GMT_n_plot = GMT_geo_to_xy_line (llon, llat, nn);
	if (GMT_n_plot > 1) {	/* Have at least 2 points */
		sprintf (text, "Lon = %g", lon);
		ps_comment (text);
		if (GMT_meridian_straight) {	/* Simplify to a 2-point straight line */
			GMT_x_plot[1] = GMT_x_plot[GMT_n_plot-1];
			GMT_y_plot[1] = GMT_y_plot[GMT_n_plot-1];
			GMT_pen[1] = GMT_pen[GMT_n_plot-1];
			GMT_plot_line (GMT_x_plot, GMT_y_plot, GMT_pen, 2);
		}
		else
			GMT_plot_line (GMT_x_plot, GMT_y_plot, GMT_pen, GMT_n_plot);
	}
	GMT_free ((void *)llon);
	GMT_free ((void *)llat);
}

void GMT_map_lontick (double lon, double south, double north, double len)
{
	GMT_LONG i, nc;
	struct GMT_XINGS *xings;

	nc = GMT_map_loncross (lon, south, north, &xings);
	for (i = 0; i < nc; i++) GMT_map_tick (xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, xings[i].nx, 0, len);
	if (nc) GMT_free ((void *)xings);
}

void GMT_map_lattick (double lat, double west, double east, double len)
{
	GMT_LONG i, nc;

	struct GMT_XINGS *xings;

	nc = GMT_map_latcross (lat, west, east, &xings);
	for (i = 0; i < nc; i++) GMT_map_tick (xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, xings[i].nx, 1, len);
	if (nc) GMT_free ((void *)xings);
}

void GMT_map_tick (double *xx, double *yy, GMT_LONG *sides, double *angles, GMT_LONG nx, GMT_LONG type, double len)
{
	double angle, xl, yl, xt, yt, c, s, tick_length;
	GMT_LONG i;

	for (i = 0; i < nx; i++) {
		if (!project_info.edge[sides[i]]) continue;
		if (!frame_info.side[sides[i]]) continue;
		if (!(gmtdefs.oblique_annotation & 1) && ((type == 0 && (sides[i] % 2)) || (type == 1 && !(sides[i] % 2)))) continue;
		angle = ((gmtdefs.oblique_annotation & 16) ? (sides[i] - 1) * 90.0 : angles[i]);
		sincosd (angle, &s, &c);
		tick_length = len;
		if (gmtdefs.oblique_annotation & 8) {
			if (sides[i] % 2) {
				/* if (fabs (c) > cosd (gmtdefs.annot_min_angle)) continue; */
				if (fabs (c) < sind (gmtdefs.annot_min_angle)) continue;
				tick_length /= fabs(c);
			}
			else {
				if (fabs (s) < sind (gmtdefs.annot_min_angle)) continue;
				tick_length /= fabs(s);
			}
		}
		xl = tick_length * c;
		yl = tick_length * s;
		GMT_xy_do_z_to_xy (xx[i], yy[i], project_info.z_level, &xt, &yt);
		ps_plot (xt, yt, PSL_PEN_MOVE);
		GMT_xy_do_z_to_xy (xx[i]+xl, yy[i]+yl, project_info.z_level, &xt, &yt);
		ps_plot (xt, yt, PSL_PEN_DRAW_AND_STROKE);
	}
}

void GMT_map_symbol_ew (double lat, char *label, double west, double east, GMT_LONG annot, GMT_LONG level)
{
	GMT_LONG i, nc;
	struct GMT_XINGS *xings;

	nc = GMT_map_latcross (lat, west, east, &xings);
	for (i = 0; i < nc; i++) GMT_map_symbol (xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, label, xings[i].nx, 1, annot, level);
	if (nc) GMT_free ((void *)xings);
}

void GMT_map_symbol_ns (double lon, char *label, double south, double north, GMT_LONG annot, GMT_LONG level)
{
	GMT_LONG i, nc;
	struct GMT_XINGS *xings;

	nc = GMT_map_loncross (lon, south, north, &xings);
	for (i = 0; i < nc; i++)  GMT_map_symbol (xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, label, xings[i].nx, 0, annot, level);
	if (nc) GMT_free ((void *)xings);
}

void GMT_map_symbol (double *xx, double *yy, GMT_LONG *sides, double *line_angles, char *label, GMT_LONG nx, GMT_LONG type, GMT_LONG annot, GMT_LONG level)
{
	/* type = 0 for lon and 1 for lat */

	double line_angle, text_angle, div, tick_length, o_len, len, ca, sa, xt1, yt1, zz;
#if 0
	double dx, dy, tick_x[2], tick_y[2];
#endif
	GMT_LONG i, justify, annot_type;
	GMT_LONG flip;
	char cmd[BUFSIZ];

	len = GMT_get_annot_offset (&flip, level);
	annot_type = 2 << type;		/* 2 = NS, 4 = EW */
	for (i = 0; i < nx; i++) {

		if (GMT_prepare_label (line_angles[i], sides[i], xx[i], yy[i], type, &line_angle, &text_angle, &justify)) continue;

		sincosd (line_angle, &sa, &ca);
		tick_length = gmtdefs.tick_length;
		o_len = len;
		if (gmtdefs.oblique_annotation & annot_type) o_len = tick_length;
		if (gmtdefs.oblique_annotation & 8) {
			div = ((sides[i] % 2) ? fabs(ca) : fabs(sa));
			tick_length /= div;
			o_len /= div;
		}
		GMT_z_to_zz (project_info.z_level, &zz);
#if 0
		dx = tick_length * ca;
		dy = tick_length * sa;
		GMT_xyz_to_xy (xx[i], yy[i], zz, &tick_x[0], &tick_y[0]);
		GMT_xyz_to_xy (xx[i]+dx, yy[i]+dy, zz, &tick_x[1], &tick_y[1]);
#endif
		xx[i] += o_len * ca;
		yy[i] += o_len * sa;
		if ((gmtdefs.oblique_annotation & annot_type) && gmtdefs.annot_offset[level] > 0.0) {
			if (sides[i] % 2)
				xx[i] += (sides[i] == 1) ? gmtdefs.annot_offset[level] : -gmtdefs.annot_offset[level];
			else
				yy[i] += (sides[i] == 2) ? gmtdefs.annot_offset[level] : -gmtdefs.annot_offset[level];
		}
		GMT_xyz_to_xy (xx[i], yy[i], zz, &xt1, &yt1);

		if (project_info.three_D) {
			GMT_LONG upside = FALSE, k;
			double xp[2], yp[2], xt2, xt3, yt2, yt3, del_y;
			double xshrink, yshrink, tilt, baseline_shift, cb, sb, a;

			upside = (z_project.quadrant == 1 || z_project.quadrant == 4);
			sincosd (text_angle, &sb, &cb);
			if (sides[i]%2 == 0 && justify%2 == 0) {
				if (upside) {
					k = (sides[i] == 0) ? 2 : 10;
					text_angle += 180.0;
				}
				else
					k = justify;
				del_y = 0.5 * gmtdefs.annot_font_size[level] * 0.732 * (k/4) * GMT_u2u[GMT_PT][GMT_INCH];
				justify = 2;
				xx[i] += del_y * ca;	yy[i] += del_y * sa;
				GMT_xyz_to_xy (xx[i], yy[i], zz, &xt1, &yt1);
			}
			else {
				del_y = -0.5 * gmtdefs.annot_font_size[level] * 0.732 * (justify/4) * GMT_u2u[GMT_PT][GMT_INCH];
				if (upside) {
					if (sides[i]%2) del_y = -del_y;
					text_angle += 180.0;
					justify = (justify == 5) ? 7 : 5;
				}
				justify -= 4;
				switch (sides[i]) {
					case 0:
						a = (justify == 1) ? line_angle + 90.0 : line_angle - 90.0;
						break;
					case 1:
						a = line_angle + 90.0;
						break;
					case 2:
						a = (justify == 1) ? line_angle + 90.0 : line_angle - 90.0;
						break;
					default:
						a = line_angle - 90.0;
						break;
				}
				sincosd (a, &sa, &ca);
				xx[i] += del_y * ca;	yy[i] += del_y * sa;
				GMT_xyz_to_xy (xx[i], yy[i], zz, &xt1, &yt1);
			}
			xp[0] = xx[i] + o_len * cb;	yp[0] = yy[i] + o_len * sb;
			xp[1] = xx[i] - o_len * sb;	yp[1] = yy[i] + o_len * cb;
			GMT_xyz_to_xy (xp[0], yp[0], zz, &xt2, &yt2);
			GMT_xyz_to_xy (xp[1], yp[1], zz, &xt3, &yt3);
			xshrink = hypot (xt2-xt1, yt2-yt1) / hypot (xp[0]-xx[i], yp[0]-yy[i]);
			yshrink = hypot (xt3-xt1, yt3-yt1) / hypot (xp[1]-xx[i], yp[1]-yy[i]);
			baseline_shift = d_atan2d (yt2 - yt1, xt2 - xt1) - d_atan2d (yp[0] - yy[i], xp[0] - xx[i]);
			tilt = 90.0 - (d_atan2d (yt3 - yt1, xt3 - xt1) - d_atan2d (yt2 - yt1, xt2 - xt1));
			tilt = tand (tilt);
			/* Temporarily modify meaning of F0 */
			sprintf (cmd, "/F0 {/%s findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont}!",
				GMT_font[gmtdefs.annot_font[level]].name, xshrink, yshrink * tilt, yshrink);
			ps_command (cmd);
			ps_setfont (0);
			text_angle += baseline_shift;
		}
		if (annot) {
			if (GMT_annot_too_crowded (xt1, yt1, sides[i])) continue;
			if (flip) justify = GMT_flip_justify (justify);
#if 0
			/* ps_line (tick_x, tick_y, 2, 3, FALSE); */
#endif
			ps_text (xt1, yt1, gmtdefs.annot_font_size[level], label, text_angle, justify, 0);
			if (project_info.three_D) ps_command ("/F0 {/Helvetica Y}!"); /* Reset F0 */
		}
	}
}

GMT_LONG GMT_annot_too_crowded (double x, double y, GMT_LONG side) {
	/* Checks if the proposed annotation is too close to a previously plotted annotation */
	GMT_LONG i;
	double d_min;

	if (gmtdefs.annot_min_spacing <= 0.0) return (FALSE);

	for (i = 0, d_min = DBL_MAX; i < GMT_n_annotations[side]; i++) d_min = MIN (d_min, hypot (GMT_x_annotation[side][i] - x, GMT_y_annotation[side][i] - y));
	if (d_min < gmtdefs.annot_min_spacing) return (TRUE);

	/* OK to plot and add to list */

	if (GMT_n_annotations[side] == GMT_alloc_annotations[side]) GMT_alloc_annotations[side] = GMT_alloc_memory2 ((void **)&GMT_x_annotation[side], (void **)&GMT_y_annotation[side], GMT_n_annotations[side], GMT_alloc_annotations[side], sizeof (double), "GMT_annot_too_crowded");
	GMT_x_annotation[side][GMT_n_annotations[side]] = x;	GMT_y_annotation[side][GMT_n_annotations[side]] = y;	GMT_n_annotations[side]++;

	return (FALSE);
}

void GMT_map_gridlines (double w, double e, double s, double n)
{
	GMT_LONG k, item[2] = {GMT_GRID_UPPER, GMT_GRID_LOWER};
	double dx, dy;
	char *comment[2] = {"Map gridlines (primary)", "Map gridlines (secondary)"};

	for (k = 0; k < 2; k++) {
		if (gmtdefs.grid_cross_size[k] > 0.0) continue;

		dx = GMT_get_map_interval (0, item[k]);
		dy = GMT_get_map_interval (1, item[k]);

		if (! (frame_info.axis[0].item[item[k]].active || frame_info.axis[1].item[item[k]].active)) continue;
		/* if (dx <= 0.0 && dy <= 0.0) continue; */

		ps_comment (comment[k]);

		GMT_setpen (&gmtdefs.grid_pen[k]);

		if (project_info.xyz_projection[0] == GMT_TIME && dx > 0.0)
			GMT_timex_grid (w, e, s, n, item[k]);
		else if (fabs(dx) > 0.0 && project_info.xyz_projection[0] == GMT_LOG10)
			GMT_logx_grid (w, e, s, n, dx);
		else if (dx > 0.0 && project_info.xyz_projection[0] == GMT_POW)
			GMT_powx_grid (w, e, s, n, dx);
		else if (dx > 0.0)	/* Draw grid lines that go E to W */
			GMT_linearx_grid (w, e, s, n, dx);

		if (project_info.xyz_projection[1] == GMT_TIME && dy > 0.0)
			GMT_timey_grid (w, e, s, n, item[k]);
		else if (fabs(dy) > 0.0 && project_info.xyz_projection[1] == GMT_LOG10)
			GMT_logy_grid (w, e, s, n, dy);
		else if (dy > 0.0 && project_info.xyz_projection[1] == GMT_POW)
			GMT_powy_grid (w, e, s, n, dy);
		else if (dy > 0.0)	/* Draw grid lines that go S to N */
			GMT_lineary_grid (w, e, s, n, dy);

		if (gmtdefs.grid_pen[k].texture) ps_setdash (CNULL, 0);
	}
}

void GMT_map_gridcross (double w, double e, double s, double n)
{
	GMT_LONG i, j, k, nx, ny, item[2] = {GMT_GRID_UPPER, GMT_GRID_LOWER};
	double x0, y0, x1, y1, xa, xb, ya, yb, xi, yj, *x, *y;
	double x_angle, y_angle, xt1, xt2, yt1, yt2, C, S, L;

	char *comment[2] = {"Map gridcrosses (primary)", "Map gridcrosses (secondary)"};

	for (k = i = 0; k < 2; k++) if (gmtdefs.grid_cross_size[k] > 0.0) i++;

	if (i == 0) return;	/* No grid ticks requested */

	GMT_map_clip_on (GMT_no_rgb, 3);

	for (k = 0; k < 2; k++) {
		if (gmtdefs.grid_cross_size[k] <= 0.0) continue;

		ps_comment (comment[k]);

		GMT_setpen (&gmtdefs.grid_pen[k]);

		nx = GMT_coordinate_array (w, e, &frame_info.axis[0].item[item[k]], &x);
		ny = GMT_coordinate_array (s, n, &frame_info.axis[1].item[item[k]], &y);

		L = 0.5 * gmtdefs.grid_cross_size[k];

		for (j = 0; j < ny; j++) {
			for (i = 0; i < nx; i++) {

				if (!GMT_map_outside (x[i], y[j])) {	/* Inside map */
					yj = y[j];
					if (GMT_POLE_IS_POINT && GMT_IS_ZERO (fabs (yj) - 90.0)) {	/* Only place one grid cross at the poles for maps where the poles are points */
						xi = project_info.central_meridian;
						i = nx;	/* This ends the loop for this particular latitude */
					}
					else
						xi = x[i];
					GMT_geo_to_xy (xi, yj, &x0, &y0);
					if (GMT_IS_MAPPING) {
						GMT_geo_to_xy (xi + GMT_dlon, yj, &x1, &y1);
						x_angle = d_atan2 (y1-y0, x1-x0);
						sincos (x_angle, &S, &C);
						xa = x0 - L * C;
						xb = x0 + L * C;
						ya = y0 - L * S;
						yb = y0 + L * S;
					}
					else {
						xa = x0 - L;	xb = x0 + L;
						ya = yb = y0;
					}

					/* Clip to map */

					if (xa < 0.0) xa = 0.0;
					if (xb < 0.0) xb = 0.0;
					if (ya < 0.0) ya = 0.0;
					if (yb < 0.0) yb = 0.0;
					if (xa > GMT_map_width) xa = GMT_map_width;
					if (xb > GMT_map_width) xb = GMT_map_width;
					if (ya > GMT_map_height) ya = GMT_map_height;
					if (yb > GMT_map_height) yb = GMT_map_height;

					/* 3-D projection */

					GMT_xy_do_z_to_xy (xa, ya, project_info.z_level, &xt1, &yt1);
					GMT_xy_do_z_to_xy (xb, yb, project_info.z_level, &xt2, &yt2);
					ps_plot (xt1, yt1, PSL_PEN_MOVE);
					ps_plot (xt2, yt2, PSL_PEN_DRAW_AND_STROKE);

					if (GMT_IS_MAPPING) {
						GMT_geo_to_xy (xi, yj - copysign (GMT_dlat, yj), &x1, &y1);
						y_angle = d_atan2 (y1-y0, x1-x0);
						sincos (y_angle, &S, &C);
						xa = x0 - L * C;
						xb = x0 + L * C;
						ya = y0 - L * S;
						yb = y0 + L * S;
					}
					else {
						xa = xb = x0;
						ya = y0 - L;	yb = y0 + L;
					}

					/* Clip to map */

					if (xa < 0.0) xa = 0.0;
					if (xb < 0.0) xb = 0.0;
					if (ya < 0.0) ya = 0.0;
					if (yb < 0.0) yb = 0.0;
					if (xa > GMT_map_width) xa = GMT_map_width;
					if (xb > GMT_map_width) xb = GMT_map_width;
					if (ya > GMT_map_height) ya = GMT_map_height;
					if (yb > GMT_map_height) yb = GMT_map_height;

					/* 3-D projection */

					GMT_xy_do_z_to_xy (xa, ya, project_info.z_level, &xt1, &yt1);
					GMT_xy_do_z_to_xy (xb, yb, project_info.z_level, &xt2, &yt2);
					ps_plot (xt1, yt1, PSL_PEN_MOVE);
					ps_plot (xt2, yt2, PSL_PEN_DRAW_AND_STROKE);
				}
			}
		}
		if (nx) GMT_free ((void *)x);
		if (ny) GMT_free ((void *)y);

		if (gmtdefs.grid_pen[k].texture) ps_setdash (CNULL, 0);

	}
	GMT_map_clip_off ();
}

void GMT_map_tickmarks (double w, double e, double s, double n)
{
	/* Tickmarks at annotation interval has already been done except when annotations were not desired */

	if (!(GMT_IS_MAPPING || project_info.projection == GMT_POLAR)) return;	/* Tickmarks already done by linear axis */

	ps_comment ("Map tickmarks");
	GMT_setpen (&gmtdefs.tick_pen);

	GMT_map_tickitem (w, e, s, n, GMT_ANNOT_UPPER);
	if (gmtdefs.basemap_type == GMT_IS_PLAIN) {	/* else we do it with checkerboard b/w patterns */
		GMT_map_tickitem (w, e, s, n, GMT_TICK_UPPER);
		GMT_map_tickitem (w, e, s, n, GMT_TICK_LOWER);
	}

	if (gmtdefs.tick_pen.texture) ps_setdash (CNULL, 0);
}

void GMT_map_tickitem (double w, double e, double s, double n, GMT_LONG item)
{
	GMT_LONG i, nx, ny;
	double dx, dy, *val, len;
	GMT_LONG do_x, do_y;

	if (! (frame_info.axis[0].item[item].active || frame_info.axis[1].item[item].active)) return;

	dx = GMT_get_map_interval (0, item);
	dy = GMT_get_map_interval (1, item);

	if (dx <= 0.0 && dy <= 0.0) return;

	do_x = ((dx > 0.0 && item == GMT_ANNOT_UPPER) || (dx > 0.0 && item == GMT_TICK_UPPER && dx != GMT_get_map_interval (0, GMT_ANNOT_UPPER)) || (dx > 0.0 && item == GMT_TICK_LOWER && dx != GMT_get_map_interval (0, GMT_ANNOT_LOWER)));
	do_y = ((dy > 0.0 && item == GMT_ANNOT_UPPER) || (dy > 0.0 && item == GMT_TICK_UPPER && dy != GMT_get_map_interval (1, GMT_ANNOT_UPPER)) || (dy > 0.0 && item == GMT_TICK_LOWER && dy != GMT_get_map_interval (1, GMT_ANNOT_LOWER)));
	len = gmtdefs.tick_length;
	if (item == GMT_TICK_UPPER) len *= 0.5;
	if (item == GMT_TICK_LOWER) len *= 0.75;

	GMT_on_border_is_outside = TRUE;	/* Temporarily, points on the border are outside */

	if (do_x) {	/* Draw grid lines that go E to W */
		nx = GMT_linear_array (w, e, dx, frame_info.axis[0].phase, &val);
		for (i = 0; i < nx; i++)  GMT_map_lontick (val[i], s, n, len);
		if (nx) GMT_free ((void *)val);
	}

	if (do_y) {	/* Draw grid lines that go S to N */
		if (project_info.z_down) {
			ny = GMT_linear_array (0.0, n-s, dy, frame_info.axis[1].phase, &val);
			for (i = 0; i < ny; i++) val[i] = project_info.n - val[i];	/* These are the radial values needed for positioning */
		}
		else
			ny = GMT_linear_array (s, n, dy, frame_info.axis[1].phase, &val);
		for (i = 0; i < ny; i++) GMT_map_lattick (val[i], w, e, len);
		if (ny) GMT_free ((void *)val);
	}

	GMT_on_border_is_outside = FALSE;	/* Reset back to default */
}

void GMT_map_annotate (double w, double e, double s, double n)
{
	double *val, dx[2], dy[2], w2, s2, x, y, del;
	GMT_LONG i, k, nx, ny, remove[2] = {0,0};
	GMT_LONG do_minutes, do_seconds, move_up, done_Greenwich, done_Dateline;
	char label[GMT_LONG_TEXT], cmd[GMT_LONG_TEXT];
	GMT_LONG full_lat_range, proj_A, proj_B, annot_0_and_360 = FALSE, dual, world_map_save, lon_wrap_save, annot;

	if (!(project_info.degree[0] || project_info.degree[1] || project_info.projection == GMT_POLAR)) return;	/* Annotations and header already done by GMT_linear_map_boundary */

	ps_setpaint (gmtdefs.basemap_frame_rgb);
	world_map_save = GMT_world_map;
	lon_wrap_save = GMT_lon_wrap;

	if (frame_info.header[0] && !frame_info.plotted_header) {	/* Make plot header for geographic maps*/
		if (project_info.three_D && GMT_IS_ZERO (project_info.z_scale)) {	/* Only do this if flat 2-D plot */

			move_up = (GMT_IS_MAPPING || frame_info.side[2] == 2);
			ps_setfont (0);
			del = ((gmtdefs.tick_length > 0.0) ? gmtdefs.tick_length : 0.0) + gmtdefs.header_offset;
			del += ((move_up) ? (gmtdefs.annot_font_size[0]) * GMT_u2u[GMT_PT][GMT_INCH] : 0.0);
			GMT_xy_do_z_to_xy (project_info.xmax * 0.5, project_info.ymax+del, project_info.z_level, &x, &y);
			sprintf (cmd, "/F0 {/%s findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont}!",
				GMT_font[gmtdefs.header_font].name, z_project.xshrink[0], z_project.yshrink[0] * z_project.tilt[0], z_project.yshrink[0]);
			ps_command (cmd);
			sprintf (cmd, "/F12 {/Symbol findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont}!",
				z_project.xshrink[0], z_project.yshrink[0] * z_project.tilt[0], z_project.yshrink[0]);
			ps_command (cmd);

			ps_text (x, y, gmtdefs.header_font_size, frame_info.header, z_project.phi[0], -2, 0);
			ps_command ("/F0 {/Helvetica Y}!");	/* Reset F0 */
			ps_command ("/F12 {/Symbol Y}!");	/* Reset F12 */
			ps_setfont (gmtdefs.header_font);
		}
		else if (!project_info.three_D) {
			ps_setfont (gmtdefs.header_font);
			if (GMT_IS_MAPPING || frame_info.side[2] == 2) {
				ps_set_length ("PSL_TL", gmtdefs.tick_length);
				ps_set_length ("PSL_AO0", gmtdefs.annot_offset[0]);
				ps_set_length ("PSL_HO", gmtdefs.header_offset);
				ps_textdim ("PSL_dim", gmtdefs.annot_font_size[0], gmtdefs.annot_font[0], "100\\312");	/* Get and set typical annotation dimensions in PostScript */
				ps_command ("PSL_dim_h PSL_dim_d sub /PSL_AF0 exch def"); /* XXX Might just need to be PSL_dim_h XXX */
			}
			else {
				ps_set_length ("PSL_TL", gmtdefs.tick_length);
				ps_set_length ("PSL_AO0", 0.0);
				ps_set_length ("PSL_HO", gmtdefs.header_offset);
			}
			ps_command ("/PSL_H_y PSL_TL PSL_AO0 add PSL_AF0 add PSL_HO add def");						/* PSL_H was not set by linear axis */
			ps_set_length ("PSL_x", project_info.xmax * 0.5);
			ps_set_length ("PSL_y", project_info.ymax);
			ps_textdim ("PSL_dim", gmtdefs.header_font_size, gmtdefs.header_font, frame_info.header);	/* Get and set string dimensions in PostScript */
			ps_command ("PSL_x PSL_dim_w 2 div sub PSL_y PSL_H_y add M");
			ps_setfont (gmtdefs.header_font);
			ps_text (0.0, 0.0, -gmtdefs.header_font_size, frame_info.header, 0.0, 0, 0);
		}
		frame_info.plotted_header = TRUE;
	}

	if (project_info.edge[0] || project_info.edge[2]) {
		dx[0] = GMT_get_map_interval (0, GMT_ANNOT_UPPER);
		dx[1] = GMT_get_map_interval (0, GMT_ANNOT_LOWER);
		/* Determine if we should annotate both 0 and 360 degrees */

		full_lat_range = (fabs (180.0 - fabs (project_info.n - project_info.s)) < GMT_SMALL);
		proj_A = (project_info.projection == GMT_MERCATOR || project_info.projection == GMT_OBLIQUE_MERC ||
			project_info.projection == GMT_WINKEL || project_info.projection == GMT_ECKERT4 || project_info.projection == GMT_ECKERT6 ||
			project_info.projection == GMT_ROBINSON || project_info.projection == GMT_CYL_EQ || project_info.projection == GMT_CYL_STEREO ||
			project_info.projection == GMT_CYL_EQDIST || project_info.projection == GMT_MILLER || project_info.projection == GMT_LINEAR);
		proj_B = (project_info.projection == GMT_HAMMER || project_info.projection == GMT_MOLLWEIDE ||
			project_info.projection == GMT_SINUSOIDAL);
		annot_0_and_360 = (world_map_save && (proj_A || (!full_lat_range && proj_B)));
	}
	else
		dx[0] = dx[1] = 0.0;
	if (project_info.edge[1] || project_info.edge[3]) {
		dy[0] = GMT_get_map_interval (1, GMT_ANNOT_UPPER);
		dy[1] = GMT_get_map_interval (1, GMT_ANNOT_LOWER);
	}
	else
		dy[0] = dy[1] = 0.0;

	if (dx[0] <= 0.0 && dy[0] <= 0.0) return;

	dual = (dx[1] > 0.0 || dy[1] > 0.0);

	ps_comment ("Map annotations");

	ps_setfont (gmtdefs.annot_font[0]);
	/* GMT_setpen (&gmtdefs.tick_pen); */

	GMT_on_border_is_outside = TRUE;	/* Temporarily, points on the border are outside */
	if (project_info.region) {
		GMT_world_map = FALSE;
		if (!(project_info.projection == GMT_GENPER || project_info.projection == GMT_GNOMONIC)) GMT_lon_wrap = FALSE;
	}

	w2 = (dx[1] > 0.0) ? floor (w / dx[1]) * dx[1] : 0.0;
	s2 = (dy[1] > 0.0) ? floor (s / dy[1]) * dy[1] : 0.0;

	if (dual) {
		remove[0] = (dx[0] < (1.0/60.0)) ? 2 : 1;
		remove[1] = (dy[0] < (1.0/60.0)) ? 2 : 1;
	}
	for (k = 0; k < 1 + dual; k++) {
		if (dx[k] > 0.0 && (project_info.degree[0] || project_info.projection == GMT_POLAR)) {	/* Annotate the S and N boundaries */
			done_Greenwich = done_Dateline = FALSE;
			do_minutes = (fabs (fmod (dx[k], 1.0)) > GMT_SMALL);
			do_seconds = set_do_seconds (dx[k]);

			nx = GMT_linear_array (w, e, dx[k], frame_info.axis[0].phase, &val);
			for (i = 0; i < nx; i++) {	/* Worry that we do not try to plot 0 and 360 OR -180 and +180 on top of each other */
				if (GMT_IS_ZERO (val[i])) done_Greenwich = TRUE;		/* OK, want to plot 0 */
				if (GMT_IS_ZERO (180.0 + val[i])) done_Dateline = TRUE;	/* OK, want to plot -180 */
				GMT_get_annot_label (val[i], label, do_minutes, do_seconds, 0, world_map_save);
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
						GMT_label_trim (label, remove[0]);
				}
				GMT_map_symbol_ns (val[i], label, s, n, annot, k);
			}
			if (nx) GMT_free ((void *)val);
		}

		if (dy[k] > 0.0 && (project_info.degree[1] || project_info.projection == GMT_POLAR)) {	/* Annotate W and E boundaries */
			GMT_LONG lonlat;
			double *tval;

			if (project_info.degree[1]) {
				do_minutes = (fabs (fmod (dy[k], 1.0)) > GMT_SMALL);
				do_seconds = set_do_seconds (dy[k]);
				lonlat = 1;
			}
			else {	/* Also, we know that gmtdefs.degree_format = -1 in this case */
				do_minutes = do_seconds = 0;
				lonlat = 2;
			}
			if (project_info.z_down) {	/* Want to annotate depth rather than radius */
				ny = GMT_linear_array (0.0, n-s, dy[k], frame_info.axis[1].phase, &tval);
				val = (double *)GMT_memory (VNULL, (size_t)ny, sizeof (double), GMT_program);
				for (i = 0; i < ny; i++) val[i] = project_info.n - tval[i];	/* These are the radial values needed for positioning */
			}
			else {				/* Annotate radius */
				ny = GMT_linear_array (s, n, dy[k], frame_info.axis[1].phase, &val);
				tval = val;	/* Same thing */
			}
			for (i = 0; i < ny; i++) {
				if ((project_info.polar || project_info.projection == GMT_VANGRINTEN) && GMT_IS_ZERO (fabs (val[i]) - 90.0)) continue;
				GMT_get_annot_label (tval[i], label, do_minutes, do_seconds, lonlat, world_map_save);
				annot = TRUE;
				if (dual && k == 0) {
					del = fmod (val[i] - s2, dy[1]);
					if (GMT_IS_ZERO (del) || GMT_IS_ZERO (del - dy[1]))
						annot = FALSE;
					else
						GMT_label_trim (label, remove[1]);
				}
				GMT_map_symbol_ew (val[i], label, w, e, annot, k);
			}
			if (ny) GMT_free ((void *)val);
			if (project_info.z_down) GMT_free ((void *)tval);
		}
	}

	if (project_info.three_D) ps_command ("/F0 {/Helvetica Y}!");	/* Reset definition of F0 */
	if (project_info.three_D) ps_command ("/F12 {/Symbol Y}!");	/* Reset definition of F12 */

	GMT_on_border_is_outside = FALSE;	/* Reset back to default */
	if (project_info.region) {
		GMT_world_map = world_map_save;
		GMT_lon_wrap = lon_wrap_save;
	}
}

void GMT_map_boundary (double w, double e, double s, double n)
{
	ps_comment ("Map boundaries");

	switch (project_info.projection) {
		case GMT_LINEAR:
			if (GMT_IS_MAPPING)	/* xy is lonlat */
				GMT_fancy_map_boundary (w, e, s, n);
			else if (project_info.three_D)
				GMT_basemap_3D (3);
			else
				GMT_linear_map_boundary (w, e, s, n);
			break;
		case GMT_POLAR:
			GMT_theta_r_map_boundary (w, e, s, n);
			break;
		case GMT_MERCATOR:
		case GMT_CYL_EQ:
		case GMT_CYL_EQDIST:
		case GMT_CYL_STEREO:
		case GMT_MILLER:
			GMT_fancy_map_boundary (w, e, s, n);
			break;
		case GMT_ALBERS:
		case GMT_ECONIC:
		case GMT_LAMBERT:
		case GMT_POLYCONIC:
			GMT_conic_map_boundary (w, e, s, n);
			break;
		case GMT_OBLIQUE_MERC:
			GMT_oblmrc_map_boundary (w, e, s, n);
			break;
		case GMT_GENPER:
			GMT_genper_map_boundary (w, e, s, n);
			break;
		case GMT_STEREO:
		case GMT_ORTHO:
		case GMT_LAMB_AZ_EQ:
		case GMT_AZ_EQDIST:
		case GMT_GNOMONIC:
			if (project_info.polar)
				GMT_polar_map_boundary (w, e, s, n);
			else
				GMT_circle_map_boundary (w, e, s, n);
			break;
		case GMT_HAMMER:
		case GMT_MOLLWEIDE:
		case GMT_SINUSOIDAL:
			GMT_ellipse_map_boundary (w, e, s, n);
			break;
		case GMT_TM:
		case GMT_UTM:
		case GMT_CASSINI:
		case GMT_WINKEL:
		case GMT_ECKERT4:
		case GMT_ECKERT6:
		case GMT_ROBINSON:
		case GMT_VANGRINTEN:
			GMT_basic_map_boundary (w, e, s, n);
			break;
	}

	if (project_info.three_D) GMT_vertical_axis (GMT_3D_mode);

	if (gmtdefs.frame_pen.texture) ps_setdash (CNULL, 0);
}


/* GMT_map_basemap will create a basemap for the given area.
 * Scaling and wesn are assumed to be passed through the project_info-structure (see GMT_project.h)
 * Tickmark info are passed through the frame_info-structure
 *
 */

void GMT_map_basemap (void) {
	GMT_LONG i;
	GMT_LONG clip_on = FALSE;
	double w, e, s, n;

	if (!frame_info.plot) return;

	ps_setpaint (gmtdefs.basemap_frame_rgb);

	w = project_info.w;	e = project_info.e;	s = project_info.s;	n = project_info.n;

	if (gmtdefs.oblique_annotation & 2) frame_info.horizontal = 2;
	if (frame_info.horizontal == 2) gmtdefs.oblique_annotation |= 2;
	if (gmtdefs.basemap_type == GMT_IS_GRAPH && GMT_IS_MAPPING) gmtdefs.basemap_type = GMT_IS_PLAIN;
	if (gmtdefs.basemap_type == GMT_IS_FANCY && !GMT_is_fancy_boundary()) gmtdefs.basemap_type = GMT_IS_PLAIN;

	ps_comment ("Start of basemap");
	ps_setdash (CNULL, 0);	/* To ensure no dashed pens are set prior */

	if (project_info.got_azimuths) l_swap (frame_info.side[1], frame_info.side[3]);	/* Temporary swap to trick justify machinery */

	if (gmtdefs.basemap_type == (int)GMT_IS_INSIDE) {
		GMT_map_clip_on (GMT_no_rgb, 3);	/* Must clip to ensure things are inside */
		clip_on = TRUE;
		gmtdefs.basemap_type = GMT_IS_PLAIN;
	}
	
	GMT_map_gridlines (w, e, s, n);
	GMT_map_gridcross (w, e, s, n);

	GMT_map_tickmarks (w, e, s, n);

	GMT_map_annotate (w, e, s, n);

	if (project_info.got_azimuths) l_swap (frame_info.side[1], frame_info.side[3]);	/* Undo swap */

	GMT_map_boundary (w, e, s, n);
	if (clip_on) GMT_map_clip_off ();

	ps_comment ("End of basemap");

	for (i = 0; i < 4; i++) {
		GMT_free (GMT_x_annotation[i]);
		GMT_free (GMT_y_annotation[i]);
	}

	ps_setpaint (gmtdefs.background_rgb);
}

void GMT_basemap_3D (GMT_LONG mode)
{
	/* Mode means: 1 = background axis, 2 = foreground axis, 3 = all */
	GMT_LONG go[4], back;
	GMT_LONG i;

	back = (mode % 2);
	for (i = 0; i < 4; i++) go[i] = (mode == 3) ? TRUE : ((back) ? z_project.draw[i] : !z_project.draw[i]);

	if (go[0] && frame_info.side[0])	/* South or lower x-axis */
		GMT_xyz_axis3D (0, 'x', &frame_info.axis[0], frame_info.side[0]-1);

	if (go[2] && frame_info.side[2])	/* North or upper x-axis */
		GMT_xyz_axis3D (2, 'x',  &frame_info.axis[0], frame_info.side[2]-1);

	if (go[3] && frame_info.side[3])	/* West or left y-axis */
		GMT_xyz_axis3D (3, 'y',  &frame_info.axis[1], frame_info.side[3]-1);

	if (go[1] && frame_info.side[1])	/* East or right y-axis */
		GMT_xyz_axis3D (1, 'y',  &frame_info.axis[1], frame_info.side[1]-1);

}

void GMT_vertical_axis (GMT_LONG mode)
{
	/* Mode means: 1 = background axis, 2 = foreground axis, 3 = all */
	GMT_LONG go[4], fore, back;
	GMT_LONG i, j;
	double xp[2], yp[2], z_annot;

	if (!frame_info.axis[2].item[GMT_ANNOT_UPPER].active) return;

	z_annot = GMT_get_map_interval (2, GMT_ANNOT_UPPER);
	fore = (mode > 1);	back = (mode % 2);
	for (i = 0; i < 4; i++) go[i] = (mode == 3) ? TRUE : ((back) ? z_project.draw[i] : !z_project.draw[i]);

	/* Vertical */

	if (fore && frame_info.side[4]) GMT_xyz_axis3D (z_project.z_axis, 'z', &frame_info.axis[2], frame_info.side[4]-1);

	if (frame_info.draw_box) {
		GMT_setpen (&gmtdefs.grid_pen[0]);
		go[0] = ( (back && z_project.quadrant == 1) || (fore && z_project.quadrant != 1) );
		go[1] = ( (back && z_project.quadrant == 4) || (fore && z_project.quadrant != 4) );
		go[2] = ( (back && z_project.quadrant == 3) || (fore && z_project.quadrant != 3) );
		go[3] = ( (back && z_project.quadrant == 2) || (fore && z_project.quadrant != 2) );
		for (i = 0; i < 4; i++) {
			if (!go[i]) continue;
			GMT_geoz_to_xy (z_project.corner_x[i], z_project.corner_y[i], project_info.z_bottom, &xp[0], &yp[0]);
			GMT_geoz_to_xy (z_project.corner_x[i], z_project.corner_y[i], project_info.z_top, &xp[1], &yp[1]);
			ps_line (xp, yp, (GMT_LONG)2, 3, FALSE);
		}
		go[0] = ( (back && (z_project.quadrant == 1 || z_project.quadrant == 4)) || (fore && (z_project.quadrant == 2 || z_project.quadrant == 3)) );
		go[1] = ( (back && (z_project.quadrant == 3 || z_project.quadrant == 4)) || (fore && (z_project.quadrant == 1 || z_project.quadrant == 2)) );
		go[2] = ( (back && (z_project.quadrant == 2 || z_project.quadrant == 3)) || (fore && (z_project.quadrant == 1 || z_project.quadrant == 4)) );
		go[3] = ( (back && (z_project.quadrant == 1 || z_project.quadrant == 2)) || (fore && (z_project.quadrant == 3 || z_project.quadrant == 4)) );
		for (i = 0; i < 4; i++) {
			if (!go[i]) continue;
			j = (i + 1) % 4;
			GMT_geoz_to_xy (z_project.corner_x[i], z_project.corner_y[i], project_info.z_top, &xp[0], &yp[0]);
			GMT_geoz_to_xy (z_project.corner_x[j], z_project.corner_y[j], project_info.z_top, &xp[1], &yp[1]);
			ps_line (xp, yp, (GMT_LONG)2, 3, FALSE);
		}
	}
	if (back && frame_info.header[0]) {
		ps_setfont (gmtdefs.header_font);
		xp[0] = 0.5 * (z_project.xmin + z_project.xmax);
		yp[0] = z_project.ymax + 0.5;
		ps_text (xp[0], yp[0], gmtdefs.header_font_size, frame_info.header, 0.0, -2, 0);
		frame_info.plotted_header = TRUE;
	}
}

void GMT_xyz_axis3D (GMT_LONG axis_no, char axis, struct GMT_PLOT_AXIS *A, GMT_LONG annotate)
{
	GMT_LONG i, j, k, id, justify, n;

	double annot_off, label_off, *knots, sign, dy, tmp, xyz[3][2], len, x0, x1, y0, y1;
	double pp[3], w[3], xp, yp, del_y, val_xyz[3], phi, val0, val1;

	PFI xyz_forward, xyz_inverse;

	char annotation[GMT_LONG_TEXT], format[GMT_LONG_TEXT], cmd[GMT_LONG_TEXT];

	id = (axis == 'x') ? 0 : ((axis == 'y') ? 1 : 2);
	j = (id == 0) ? 1 : ((id == 1) ? 0 : z_project.k);
	xyz_forward = (PFI) ((id == 0) ? GMT_x_to_xx : ((id == 1) ? GMT_y_to_yy : GMT_z_to_zz));
	xyz_inverse = (PFI) ((id == 0) ? GMT_xx_to_x : ((id == 1) ? GMT_yy_to_y : GMT_zz_to_z));
	phi = (id < 2 && axis_no > 1) ? z_project.phi[id] + 180.0 : z_project.phi[id];

	/* Get projected anchor point */

	if (id == 2) {
		GMT_geoz_to_xy (z_project.corner_x[axis_no], z_project.corner_y[axis_no], project_info.z_bottom, &x0, &y0);
		k = axis_no;
		GMT_geoz_to_xy (z_project.corner_x[axis_no], z_project.corner_y[axis_no], project_info.z_top, &x1, &y1);
		if (j == 0)
			sign = z_project.sign[z_project.z_axis];
		else
			sign = (z_project.z_axis%2) ? -z_project.sign[z_project.z_axis] : z_project.sign[z_project.z_axis];
	}
	else {
		GMT_geoz_to_xy (z_project.corner_x[axis_no], z_project.corner_y[axis_no], project_info.z_level, &x0, &y0);
		k = (axis_no + 1) % 4;
		GMT_geoz_to_xy (z_project.corner_x[k], z_project.corner_y[k], project_info.z_level, &x1, &y1);
		sign = z_project.sign[axis_no];
	}
	xyz[0][0] = project_info.w;		xyz[0][1] = project_info.e;
	xyz[1][0] = project_info.s;		xyz[1][1] = project_info.n;
	xyz[2][0] = project_info.z_bottom;	xyz[2][1] = project_info.z_top;

	ps_command ("gsave\n");
	ps_comment ("Start of xyz-axis3D");
	/* Temporarily modify meaning of F0 */
	sprintf (cmd, "/F0 {/%s findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont}!",
		GMT_font[gmtdefs.annot_font[0]].name, z_project.xshrink[id], z_project.yshrink[id] * z_project.tilt[id], z_project.yshrink[id]);
	ps_command (cmd);
	/* Temporarily redefine F12 for tilted text */
	sprintf (cmd, "/F12 {/Symbol findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont}!",
		z_project.xshrink[id], z_project.yshrink[id] * z_project.tilt[id], z_project.yshrink[id]);
	ps_command (cmd);
	ps_setfont (0);
	justify = (id == 2) ? 2 : 10;
	dy = sign * gmtdefs.tick_length;
	len = (gmtdefs.tick_length > 0.0) ? gmtdefs.tick_length : 0.0;

	val0 = xyz[id][0];
	val1 = xyz[id][1];
	if (val0 > val1) d_swap (val0, val1);

	/* Find number of decimals needed, if any */

	GMT_get_format (GMT_get_map_interval (id, GMT_ANNOT_UPPER), A->unit, A->prefix, format);

	annot_off = sign * (len + gmtdefs.annot_offset[0]);
	/* label_off = sign * (len + 2.5 * gmtdefs.annot_offset[0] + (gmtdefs.annot_font_size[0] * GMT_u2u[GMT_PT][GMT_INCH]) * GMT_font[gmtdefs.annot_font[0]].height); */
	label_off = sign * (len + gmtdefs.label_offset + gmtdefs.annot_offset[0] + gmtdefs.annot_font_size[0] * GMT_u2u[GMT_PT][GMT_INCH] * GMT_font[gmtdefs.annot_font[0]].height);

	/* Ready to draw axis */

	GMT_setpen (&gmtdefs.frame_pen);
	ps_plot (x0, y0, PSL_PEN_MOVE);
	ps_plot (x1, y1, PSL_PEN_DRAW_AND_STROKE);
	GMT_setpen (&gmtdefs.tick_pen);

	del_y = 0.5 * sign * gmtdefs.annot_font_size[0] * 0.732 * (justify/4) * GMT_u2u[GMT_PT][GMT_INCH];

	/* Do annotations with tickmarks */

	val_xyz[0] = z_project.corner_x[axis_no];
	val_xyz[1] = z_project.corner_y[axis_no];
	val_xyz[2] = project_info.z_level;
	n = GMT_coordinate_array (val0, val1, &A->item[GMT_ANNOT_UPPER], &knots);	/* Get all the annotation tick knots */
	for (i = 0; i < n; i++) {
		val_xyz[id] = knots[i];

		GMT_get_coordinate_label (annotation, &GMT_plot_calclock, format, &A->item[GMT_ANNOT_UPPER], knots[i]);

		GMT_project3D (val_xyz[0], val_xyz[1], val_xyz[2], &w[0], &w[1], &w[2]);
		pp[0] = w[0];
		pp[1] = w[1];
		pp[2] = w[2];
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);
		ps_plot (xp, yp, PSL_PEN_MOVE);
		pp[j] += dy;
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);
		ps_plot (xp, yp, PSL_PEN_DRAW_AND_STROKE);
		pp[j] += annot_off -dy + del_y;
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);
		if (annotate && (id < 2 || knots[i] != project_info.z_level)) ps_text (xp, yp, gmtdefs.annot_font_size[0], annotation, phi, 2, 0);
	}
	ps_command ("/F0 {/Helvetica Y}!");	/* Reset F0 */
	ps_command ("/F12 {/Symbol Y}!");	/* Reset F12 */

	if (n) GMT_free ((void *)knots);

	/* Now do frame tickmarks */

	dy *= 0.5;

	val_xyz[0] = z_project.corner_x[axis_no];
	val_xyz[1] = z_project.corner_y[axis_no];
	val_xyz[2] = project_info.z_level;
	n = GMT_coordinate_array (val0, val1, &A->item[GMT_TICK_UPPER], &knots);	/* Get all the frame tick knots */
	for (i = 0; i < n; i++) {
		val_xyz[id] = knots[i];
		if (A->type == GMT_POW && project_info.xyz_projection[id] == GMT_POW) {
			(*xyz_inverse) (&tmp, val_xyz[id]);
			val_xyz[id] = tmp;
		}
		GMT_project3D (val_xyz[0], val_xyz[1], val_xyz[2], &w[0], &w[1], &w[2]);

		pp[0] = w[0];
		pp[1] = w[1];
		pp[2] = w[2];
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);
		ps_plot (xp, yp, PSL_PEN_MOVE);
		pp[j] += dy;
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);
		ps_plot (xp, yp, PSL_PEN_DRAW_AND_STROKE);
	}
	if (n) GMT_free ((void *)knots);

	/* Finally do label */

	if (A->label[0] && annotate) {
		val_xyz[0] = z_project.corner_x[axis_no];
		val_xyz[1] = z_project.corner_y[axis_no];
		val_xyz[2] = project_info.z_level;
		/* Temporarily redefine /F0 for tilted text */
		sprintf (cmd, "/F0 {/%s findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont}!",
			GMT_font[gmtdefs.label_font].name, z_project.xshrink[id], z_project.yshrink[id] * z_project.tilt[id], z_project.yshrink[id]);
		ps_command (cmd);
		/* Temporarily redefine /F12 for tilted text */
		sprintf (cmd, "/F12 {/Symbol findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont}!",
			z_project.xshrink[id], z_project.yshrink[id] * z_project.tilt[id], z_project.yshrink[id]);
		ps_command (cmd);
		GMT_project3D (val_xyz[0], val_xyz[1], val_xyz[2], &w[0], &w[1], &w[2]);
		x0 = w[id];
		/* Next line was: val_xyz[id] = (val_xyz[id] == xyz[id][0]) ? xyz[id][1] : xyz[id][0]; but == is no good for floats */
		val_xyz[id] = (fabs (val_xyz[id] - xyz[id][1]) > fabs (val_xyz[id] - xyz[id][0])) ? xyz[id][1] : xyz[id][0];
		GMT_project3D (val_xyz[0], val_xyz[1], val_xyz[2], &w[0], &w[1], &w[2]);
		x1 = w[id];
		pp[0] = w[0];
		pp[1] = w[1];
		pp[2] = w[2];
		pp[id] = 0.5 * (x1 + x0);
		pp[j] += label_off + del_y;
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);

		ps_text (xp, yp, gmtdefs.label_font_size, A->label, phi, 2, 0);
		ps_command ("/F0 {/Helvetica Y}!");	/* Reset F0 */
		ps_command ("/F12 {/Symbol Y}!");	/* Reset F12 */
	}
	ps_setpaint (gmtdefs.background_rgb);
	ps_comment ("End of xyz-axis3D");
	ps_command ("grestore\n");
}

void GMT_grid_clip_on (struct GRD_HEADER *h, int rgb[], GMT_LONG flag)
{
	/* This function sets up a clip path so that only plotting
	 * inside the grid domain will be drawn on paper. map_setup
	 * must have been called first.  If r >= 0, the map area will
	 * first be painted in the r,g,b colors specified.  flag can
	 * be 0-3, as described in ps_clipon().
	 */

	double *work_x, *work_y;
	GMT_LONG np;
	GMT_LONG donut;

	np = GMT_grid_clip_path (h, &work_x, &work_y, &donut);

	ps_comment ("Activate Grid clip path");
	if (donut) {
		ps_clipon (work_x, work_y, np, rgb, 1);
		ps_clipon (&work_x[np], &work_y[np], np, rgb, 2);
	}
	else
		ps_clipon (work_x, work_y, np, rgb, flag);

	GMT_free ((void *)work_x);
	GMT_free ((void *)work_y);
}

void GMT_map_clip_on (int rgb[], GMT_LONG flag)
{
	/* This function sets up a clip path so that only plotting
	 * inside the map area will be drawn on paper. map_setup
	 * must have been called first.  If r >= 0, the map area will
	 * first be painted in the r,g,b colors specified.  flag can
	 * be 0-3, as described in ps_clipon().
	 */

	double *work_x, *work_y;
	GMT_LONG np;
	GMT_LONG donut;

	np = GMT_map_clip_path (&work_x, &work_y, &donut);

	ps_comment ("Activate Map clip path");
	if (donut) {
		ps_clipon (work_x, work_y, np, rgb, 1);
		ps_clipon (&work_x[np], &work_y[np], np, rgb, 2);
	}
	else
		ps_clipon (work_x, work_y, np, rgb, flag);

	GMT_free ((void *)work_x);
	GMT_free ((void *)work_y);
}

void GMT_map_clip_off (void) {
	/* Restores the original clipping path for the plot */

	ps_comment ("Deactivate Map clip path");
	ps_clipoff ();
}

void GMT_grid_clip_off (void) {
	/* Restores the clipping path that existed prior to GMT_grid_clip_path was called */

	ps_comment ("Deactivate Grid clip path");
	ps_clipoff ();
}

void GMT_geoplot (double lon, double lat, int pen)
{
	/* Computes x/y from lon/lat, then calls plot */
	double x, y;

	GMT_geo_to_xy (lon, lat, &x, &y);
	if (project_info.three_D) {	/* Must project first */
		double xp, yp;
		GMT_xy_do_z_to_xy (x, y, project_info.z_level, &xp, &yp);
		x = xp;	y = yp;
	}
	ps_plot (x, y, pen);
}

void GMT_geosegment (double lon1, double lat1, double lon2, double lat2)
{
	/* Draws a straight line between the two points */

	GMT_geoplot (lon1, lat1, PSL_PEN_MOVE);
	GMT_geoplot (lon2, lat2, PSL_PEN_DRAW_AND_STROKE);
}

void GMT_fill (double x[], double y[], GMT_LONG n, struct GMT_FILL *fill, GMT_LONG outline)
{
	if (!fill)	/* NO fill pointer = no fill */
		ps_polygon (x, y, n, GMT_no_rgb, outline);
	else if (fill->use_pattern) {
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_polygon (x, y, n, rgb, outline);
	}
	else
		ps_polygon (x, y, n, fill->rgb, outline);
}

void GMT_timestamp (double x, double y, GMT_LONG justify, char *U_label)
{
	/* x, y = location of the time stamp box
	 * justify indicates the corner of the box that (x,y) refers to, see below
	 * U_label = label to be plotted to the right of the box
	 *
	 *   9        10       11
	 *   |------------------|                 
	 *   5  GMT | <time>    7                         
	 *   |------------------|                         
	 *   1         2        3                                 
	 */

	time_t right_now;
	char label[GMT_LONG_TEXT], text[GMT_LONG_TEXT];
	double dim[3] = {0.365, 0.15, 0.032};	/* Predefined dimensions */
	int unset_rgb[3] = {-2, -2, -2};

	/* Plot time string in format defined by UNIX_TIME_FORMAT */

	right_now = time ((time_t *)0);
	strftime (text, (size_t)sizeof(text), gmtdefs.unix_time_format, localtime(&right_now));
	sprintf (label, "  %s  ", text);

	ps_command ("% Begin GMT time-stamp\nV");
	ps_transrotate (x, y, 0.0);
	ps_setline (1);
	ps_setfont (0);
	ps_set_length ("PSL_g_w", dim[0]);	/* Size of the black [GMT] box */
	ps_set_length ("PSL_g_h", dim[1]);
	ps_textdim ("PSL_b", 8.0, 0, label);	/* Size of the white [timestamp] box (use only length) */
	
	/* When justification is not BL (justify == 1), add some PostScript code to move to the
	   location where the lower left corner of the time stamp box is to be drawn */

	switch ((justify + 3) % 4) {
		case 1:	/* Center */
			ps_command ("PSL_g_w PSL_b_w add 2 div neg 0 T"); break;
		case 2:	/* Right justify */
			ps_command ("PSL_g_w PSL_b_w add neg 0 T"); break;
	}
	switch (justify / 4) {
		case 1: /* Middle */
			ps_command ("0 PSL_g_h 2 div neg T"); break;
		case 2: /* Top justify */
			ps_command ("0 PSL_g_h neg T"); break;
	}

	/* Now draw black box with GMT logo, and white box with time stamp */

	ps_rect (0.0, 0.0, dim[0], dim[1], gmtdefs.background_rgb, TRUE);
	ps_image (0.0, 0.0, dim[0], dim[1], GMT_glyph, 220, 90, 1);
	ps_setfill (gmtdefs.foreground_rgb, TRUE);
	ps_command ("PSL_g_h PSL_b_w PSL_g_w 0 SB");
	ps_text (dim[0], dim[2], 8.0, label, 0.0, 1, 0);

	/* Optionally, add additional label to the right of the box */

	if (U_label && U_label[0]) {
		ps_setfont (1);
		sprintf (label, "   %s", U_label);
		ps_text (0.0, 0.0, -7.0, label, 0.0, 1, 0);
	}

	ps_command ("U\n% End GMT time-stamp");

	/* Reset fill style so that it will be repeated outside file stamp */
	ps_setfill (unset_rgb, -2);
}

void GMT_echo_command (int argc, char **argv)
{
	/* This routine will echo the command and its arguments to the
	 * PostScript output file so that the user can see what scales
	 * etc was used to produce this plot
	 */
	GMT_LONG i, length = 0;
	char outstring[BUFSIZ];

	ps_command ("\n%% PostScript produced by:");
	strcpy (outstring, "%%GMT:  ");
	for (i = 0; i < argc; i++) {
		strcat (outstring, argv[i]);
		strcat (outstring, " ");
		length += (strlen (argv[i]) + 1);
		if (length >= 120) {
			strcat (outstring, "\\");
			ps_command (outstring);
			length = 0;
			strcpy (outstring, "%%GMT:+ ");
		}
	}
	outstring[length+7]=0;
	if (length > 0) ps_command (outstring);
}

void GMT_plot_line (double *x, double *y, int *pen, GMT_LONG n)
{
	GMT_LONG i, j, i1;
	GMT_LONG way, stop, close;
	double x_cross[2], y_cross[2], *xx, *yy, xt1, yt1, xt2, yt2;

	if (n < 2) return;

	GMT_NaN_pen_up (x, y, pen, n);	/* Ensure we dont have NaNs in the coordinates */

	i = 0;
	while (i < (n-1) && pen[i+1] == PSL_PEN_MOVE) i++;	/* Skip repeating pen == PSL_PEN_MOVE in beginning */
	if ((n-i) < 2) return;
	while (n > 1 && pen[n-1] == PSL_PEN_MOVE) n--;		/* Cut off repeating pen == PSL_PEN_MOVE at end */
	if ((n-i) < 2) return;

	for (j = i + 1; j < n && pen[j] == PSL_PEN_DRAW; j++);	/* j == n means no PSL_PEN_MOVEs present */
	close = (j == n) ? (hypot (x[n-1] - x[i], y[n-1] - y[i]) < GMT_SMALL) : FALSE;

	/* First see if we can use the ps_line call directly to save points */

	for (j = i + 1, stop = FALSE; !stop && j < n; j++) stop = (pen[j] == PSL_PEN_MOVE || (*GMT_map_jump) (x[j-1], y[j-1], x[j], y[j]));
	if (!stop) {
		if (project_info.three_D) {	/* Must project first */
			(void)GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n-i, 0, sizeof (double), "GMT_plot_line");
			for (j = i; j < n; j++) GMT_xy_do_z_to_xy (x[j], y[j], project_info.z_level, &xx[j], &yy[j]);
			ps_line (&xx[i], &yy[i], n - i, 3, close);
			GMT_free ((void *)xx);
			GMT_free ((void *)yy);
		}
		else
			ps_line (&x[i], &y[i], n - i, 3, close);
		return;
	}

	/* Here we must check for jumps, pen changes etc */

	if (project_info.three_D) {
		GMT_xy_do_z_to_xy (x[i], y[i], project_info.z_level, &xt1, &yt1);
		ps_plot (xt1, yt1, pen[i]);
	}
	else
		ps_plot (x[i], y[i], pen[i]);

	i++;
	while (i < n) {
		i1 = i - 1;
		if (pen[i] == pen[i1] && (way = (*GMT_map_jump) (x[i1], y[i1], x[i], y[i]))) {	/* Jumped across the map */
			(*GMT_get_crossings) (x_cross, y_cross, x[i1], y[i1], x[i], y[i]);
			GMT_xy_do_z_to_xy (x_cross[0], y_cross[0], project_info.z_level, &xt1, &yt1);
			GMT_xy_do_z_to_xy (x_cross[1], y_cross[1], project_info.z_level, &xt2, &yt2);
			if (project_info.three_D) {
				GMT_xy_do_z_to_xy (xt1, yt1, project_info.z_level, &xt1, &yt1);
				GMT_xy_do_z_to_xy (xt2, yt2, project_info.z_level, &xt2, &yt2);
			}
			if (way == -1) {	/* Add left border point */
				ps_plot (xt1, yt1, PSL_PEN_DRAW);	/* Draw to left boundary... */
				ps_plot (xt2, yt2, PSL_PEN_MOVE);	/* ...then jump to the right boundary */
			}
			else {
				ps_plot (xt2, yt2, PSL_PEN_DRAW);	/* Draw to right boundary... */
				ps_plot (xt1, yt1, PSL_PEN_MOVE);	/* ...then jump to the left boundary */
			}
			close = FALSE;
		}
		if (project_info.three_D) {
			GMT_xy_do_z_to_xy (x[i], y[i], project_info.z_level, &xt1, &yt1);
			ps_plot (xt1, yt1, pen[i]);
		}
		else
			ps_plot (x[i], y[i], pen[i]);
		i++;
	}
	if (close) ps_command ("P S") ; else ps_command ("S");
}

void GMT_color_image (double x0, double y0, double x_side, double y_side, unsigned char *image, GMT_LONG nx, GMT_LONG ny, GMT_LONG depth)
{
	/* x0, y0 = Lower left corner in inches
	 * x_size, y_side = Size of cell in inches
	 * image = 1|4|8|24-bit image
	 * nx, ny = image size
	 * depth = bits per pixel (negative means we want to interpolate in PostScript) */

	if (gmtdefs.color_image == 1) /* Must call the tiles machinery */
		ps_colortiles (x0, y0, x_side, y_side, image, nx, ny);
	else
		ps_colorimage (x0, y0, x_side, y_side, image, nx, ny, depth);
}

void GMT_text3D (double x, double y, double z, double fsize, GMT_LONG fontno, char *text, double angle, GMT_LONG justify, GMT_LONG form)
{
	double xb, yb, xt, yt, xt1, xt2, xt3, yt1, yt2, yt3, del_y;
	double ca, sa, xshrink, yshrink, tilt, baseline_shift;
	char cmd[GMT_LONG_TEXT];

	ps_setfont (fontno);
	if (project_info.three_D) {
		GMT_LONG *used_fonts;
		GMT_LONG i, j, n;
		char *t = text;

		used_fonts = (GMT_LONG *) GMT_memory (VNULL, (size_t) GMT_N_FONTS, sizeof (GMT_LONG), "GMT_text3D");

		justify = GMT_abs (justify);
		del_y = 0.5 * fsize * 0.732 * (justify / 4) * GMT_u2u[GMT_PT][GMT_INCH];
		justify %= 4;
		sincosd (angle, &sa, &ca);
		x += del_y * sa;	/* Move anchor point down on baseline */
		y -= del_y * ca;
		xb = x + ca;		/* Point a distance of 1.0 along baseline */
		yb = y + sa;
		xt = x - sa;		/* Point a distance of 1.0 normal to baseline */
		yt = y + ca;
		GMT_xyz_to_xy (x, y, z, &xt1, &yt1);
		GMT_xyz_to_xy (xb, yb, z, &xt2, &yt2);
		GMT_xyz_to_xy (xt, yt, z, &xt3, &yt3);
		xshrink = hypot (xt2-xt1, yt2-yt1);	/* How lines in baseline-direction shrink */
		yshrink = hypot (xt3-xt1, yt3-yt1);	/* How lines _|_ to baseline-direction shrink */
		baseline_shift = d_atan2d (yt2 - yt1, xt2 - xt1) - d_atan2d (yb - y, xb - x);	/* Rotation of baseline */
		tilt = (xt2-xt1)*(xt3-xt1) + (yt2-yt1)*(yt3-yt1);	/* inner product */
		ca = tilt / (xshrink * yshrink);
		sa = sqrt(1. - ca*ca);
		/* Search used fonts in the string */
		i = 0;
		used_fonts[i++] = fontno;	/* current font */
		used_fonts[i++] = 12;		/* Symbol */

		while ((t = strstr (t, "@%"))) {
			t += 2;
			if (*t == '%') continue;

			n = atoi (t);
			if (n < 0 || n >= GMT_N_FONTS) continue;

			for (j = 0; j < i; j ++)
				if (n == used_fonts[j]) break;
			if (i == j)
				used_fonts[i++] = n;
		}

		/* Temporarily modify meaning of F */
		sprintf (cmd, "/YY {findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont}!",
			 xshrink, yshrink * ca, yshrink * sa);
		ps_command (cmd);

		for (j = 0; j < i; j ++) {
			n = used_fonts[j];
			sprintf (cmd, "/F%ld {/%s YY}!", n, GMT_font[n].name);
			ps_command (cmd);
		}

		ps_text (xt1, yt1, fsize, text, angle + baseline_shift, justify, form);

		/* Reset fonts */
		for (j = 0; j < i; j ++) {
			n = used_fonts[j];
			sprintf (cmd, "/F%ld {/%s Y}!", n, GMT_font[n].name);
			ps_command (cmd);
		}

		GMT_free ((void *) used_fonts);
	}
	else {
		ps_text (x, y, fsize, text, angle, justify, form);
	}
}

void GMT_textbox3D (double x, double y, double z, double size, GMT_LONG font, char *label, double angle, GMT_LONG just, GMT_LONG outline, double dx, double dy, int rgb[])
{
	double xoff, yoff;
	if (project_info.three_D) {
		GMT_LONG i, len, ndig = 0, ndash = 0, nperiod = 0;
		double xx[4], yy[4], h, w, xa, ya, cosa, sina;
		/* Need to estimate the approximate width and height of text string */
		len = strlen (label);
		for (i = 0; label[i]; i++) {	/* . - [0-9] are narrower than letters */
			if (isdigit ((int)label[i])) ndig++;
			if (strchr (label, '.')) nperiod++;
			if (strchr (label, '-')) ndash++;
		}
		len -= (ndig + nperiod + ndash);
		w = ndig * 0.78 + nperiod * 0.38 + ndash * 0.52 + len;	/* Estiamte of the number of "normal width" characters */
		h = 0.58 * GMT_font[font].height * size * GMT_u2u[GMT_PT][GMT_INCH];	/* Approximate half-height */
		w *= (0.81 * h);	/* Approximate half-width of string (inch) */
		w *= 0.95;		/* Shave off 5 % */
		/* Determine shift due to justification */
		just = GMT_abs (just);
		yoff = (((just/4) - 1) * h);
		xoff = (((just-1)%4 - 1) * w);
		/* OK, get coordinates for text box before rotation and translation */
		xx[0] = xx[3] = -w - dx - xoff;
		xx[1] = xx[2] = w + dx - xoff;
		yy[0] = yy[1] = -h - dy - yoff;
		yy[2] = yy[3] = h + dy - yoff;
		sincosd (angle, &sina, &cosa);
		for (i = 0; i < 4; i++) {	/* Rotate and translate to (x,y) */
			xa = xx[i] * cosa - yy[i] * sina;
			ya = xx[i] * sina + yy[i] * cosa;
			xx[i] = x + xa;	yy[i] = y + ya;
		}
		GMT_2Dz_to_3D (xx, yy, z, (GMT_LONG)4);
		if (rgb[0] < 0)
			ps_clipon (xx, yy, (GMT_LONG)4, rgb, 0);
		else
			ps_patch (xx, yy, (GMT_LONG)4, rgb, outline);
	}
	else
		ps_textbox (x, y, size, label, angle, just, outline, dx, dy, rgb);
}

void GMT_draw_map_scale (struct GMT_MAP_SCALE *ms)
{
	GMT_LONG i, j, jj, k, unit;
	GMT_LONG n_f_ticks[10] = {5, 4, 6, 4, 5, 6, 7, 4, 3, 5};
	GMT_LONG n_a_ticks[10] = {1, 2, 3, 2, 1, 3, 1, 2, 1, 1};
	int *rgb;
	double dlon, x1, x2, y1, y2, a, b, tx, ty, off, f_len, a_len, x_left, bar_length, x_label, y_label;
	double xx[4], yy[4], bx[4], by[4], zz, base, d_base, width, half, bar_width, dx, dx_f, dx_a;
	char txt[GMT_LONG_TEXT], *this_label;
	char *label[4] = {"km", "miles", "nautical miles", "m"}, *units[4] = {"km", "mi", "nm", "m"};

	if (!ms->plot) return;

	if (!GMT_IS_MAPPING) return;	/* Only for geographic projections */

	switch (ms->measure) {
		case 'm':	/* Statute miles instead */
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
		default:	/* Default (or k) is km */
			unit = 0;
			bar_length = ms->length;
			break;
	}

	if (ms->gave_xy)	/* Also get lon/lat coordinates */
		GMT_xy_to_geo (&ms->lon, &ms->lat, ms->x0, ms->y0);
	else {	/* Must convert lon/lat to location on map */
		ms->lon = ms->x0;
		ms->lat = ms->y0;
		GMT_geo_to_xy (ms->lon, ms->lat, &ms->x0, &ms->y0);
	}

	if (project_info.projection == GMT_OBLIQUE_MERC) {	/* Set latitude to the oblique latitude */
		a = fabs (GMT_great_circle_dist (project_info.o_pole_lon, project_info.o_pole_lat, ms->scale_lon, ms->scale_lat));	/* Colatitude */
		if (a > 90.0) a = 180.0 - 90.0;	/* Flip hemisphere */
		ms->scale_lat = 90.0 - a;
	}

	dlon = 0.5 * bar_length * 1000.0 / (project_info.M_PR_DEG * cosd (ms->scale_lat));

	GMT_geoz_to_xy (project_info.central_meridian - dlon, ms->scale_lat, project_info.z_level, &x1, &y1);
	GMT_geoz_to_xy (project_info.central_meridian + dlon, ms->scale_lat, project_info.z_level, &x2, &y2);
	width = hypot (x2 - x1, y2 - y1);
	half = 0.5 * width;
	a_len = fabs (gmtdefs.map_scale_height);
	off = a_len + 0.75 * gmtdefs.annot_offset[0];
	x_left = ms->x0 - half;
	GMT_z_to_zz (project_info.z_level, &zz);

	if (ms->fancy) {	/* Fancy scale */
		j = irint(floor (d_log10 (ms->length / 0.95)));
		base = pow (10.0, (double)j);
		i = irint (ms->length / base) - 1;
		d_base = ms->length / n_a_ticks[i];
		dx_f = width / n_f_ticks[i];
		dx_a = width / n_a_ticks[i];
		bar_width = 0.5 * fabs (gmtdefs.map_scale_height);
		f_len = 0.75 * fabs (gmtdefs.map_scale_height);
		if (ms->boxdraw || ms->boxfill) {	/* Draw a rectangle beneath the scale */
			dx = 0.5 * j * 0.4 * (gmtdefs.annot_font_size[0] / 72.0);
			if (dx < 0.0) dx = 0.0;
			xx[0] = xx[3] = x_left - 2.0 * gmtdefs.annot_offset[0] - dx;
			xx[1] = xx[2] = x_left + width + 2.0 * gmtdefs.annot_offset[0] + dx;
			yy[0] = yy[1] = ms->y0 - 1.5 * a_len - gmtdefs.annot_font_size[0] / 72.0 - ((ms->justify == 'b') ? fabs(gmtdefs.label_offset) + 0.85 * gmtdefs.label_font_size / 72.0: 0.0);
			yy[2] = yy[3] = ms->y0 + 1.5 * a_len + ((ms->justify == 't') ? fabs(gmtdefs.label_offset) + gmtdefs.annot_font_size[0] / 72.0: 0.0);
			GMT_2D_to_3D (xx, yy, project_info.z_level, (GMT_LONG)4);
			if (ms->boxdraw) GMT_setpen (&ms->pen);
			GMT_fill (xx, yy, (GMT_LONG)4, &ms->fill, ms->boxdraw);
		}
		GMT_setpen (&gmtdefs.tick_pen);
		yy[2] = yy[3] = ms->y0;
		yy[0] = yy[1] = ms->y0 - bar_width;
		GMT_xyz_to_xy (x_left, ms->y0 - f_len, zz, &a, &b);
		ps_plot (a, b, PSL_PEN_MOVE);
		GMT_xyz_to_xy (x_left, ms->y0, zz, &a, &b);
		ps_plot (a, b, PSL_PEN_DRAW_AND_STROKE);
		for (j = 0; j < n_f_ticks[i]; j++) {
			xx[0] = xx[3] = x_left + j * dx_f;
			xx[1] = xx[2] = xx[0] + dx_f;
			for (k = 0; k < 4; k++) GMT_xy_do_z_to_xy (xx[k], yy[k], project_info.z_level, &bx[k], &by[k]);
			rgb = (j%2) ? gmtdefs.foreground_rgb : gmtdefs.background_rgb;
			ps_polygon (bx, by, (GMT_LONG)4, rgb, TRUE);
			GMT_xyz_to_xy (xx[1], ms->y0 - f_len, zz, &a, &b);
			ps_plot (a, b, PSL_PEN_MOVE);
			GMT_xyz_to_xy (xx[1], ms->y0, zz, &a, &b);
			ps_plot (a, b, PSL_PEN_DRAW_AND_STROKE);
		}
		ty = ms->y0 - off;
		for (j = 0; j <= n_a_ticks[i]; j++) {
			tx = x_left + j * dx_a;
			GMT_xyz_to_xy (tx, ms->y0 - a_len, zz, &a, &b);
			ps_plot (a, b, PSL_PEN_MOVE);
			GMT_xyz_to_xy (tx, ms->y0, zz, &a, &b);
			ps_plot (a, b, PSL_PEN_DRAW_AND_STROKE);
			if (ms->unit)
				sprintf (txt, "%g %s", j * d_base, units[unit]);
			else
				sprintf (txt, "%g", j * d_base);
			GMT_text3D (tx, ty, zz, gmtdefs.annot_font_size[0], gmtdefs.annot_font[0], txt, 0.0, 10, 0);
		}
		switch (ms->justify) {
			case 'l':	/* Left */
				x_label = x_left - f_len;
				y_label = ms->y0 - a_len;
				jj = 3;
				break;
			case 'r':	/* right */
				x_label = x_left + width + f_len;
				y_label = ms->y0 - a_len;
				jj = 1;
				break;
			case 't':	/* top */
				x_label = ms->x0;
				y_label = ms->y0 + fabs(gmtdefs.label_offset);
				jj = 2;
				break;
			default:	/* bottom */
				x_label = ms->x0;
				y_label = ms->y0 - a_len - fabs(gmtdefs.label_offset) - 0.85 * gmtdefs.annot_font_size[0] / 72.0;
				jj = 10;
				break;
		}
		if (ms->do_label) {
			this_label = (ms->label[0]) ? ms->label : label[unit];
			GMT_xyz_to_xy (x_label, y_label, zz, &tx, &ty);
			GMT_text3D (x_label, y_label, zz, gmtdefs.label_font_size, gmtdefs.label_font, this_label, 0.0, jj, 0);
		}
	}
	else {	/* Simple scale */

		if (ms->boxdraw || ms->boxfill) {	/* Draw a rectangle beneath the scale */
			dx = 0.5 * irint (floor (d_log10 (ms->length))) * 0.4 * (gmtdefs.annot_font_size[0] / 72.0);
			if (dx < 0.0) dx = 0.0;
			xx[0] = xx[3] = x_left - 2.0 * gmtdefs.annot_offset[0] - dx;
			xx[1] = xx[2] = x_left + width + 2.0 * gmtdefs.annot_offset[0] + dx;
			yy[0] = yy[1] = ms->y0 - 1.5 * a_len - gmtdefs.annot_font_size[0] / 72.0;
			yy[2] = yy[3] = ms->y0 + 1.5 * a_len;
			GMT_2D_to_3D (xx, yy, project_info.z_level, (GMT_LONG)4);
			if (ms->boxdraw) GMT_setpen (&ms->pen);
			GMT_fill (xx, yy, (GMT_LONG)4, &ms->fill, ms->boxdraw);
		}
		GMT_setpen (&gmtdefs.tick_pen);
		sprintf (txt, "%g %s", ms->length, label[unit]);
		GMT_xyz_to_xy (ms->x0 - half, ms->y0 - gmtdefs.map_scale_height, zz, &a, &b);
		ps_plot (a, b, PSL_PEN_MOVE);
		GMT_xyz_to_xy (ms->x0 - half, ms->y0, zz, &a, &b);
		ps_plot (a, b, PSL_PEN_DRAW);
		GMT_xyz_to_xy (ms->x0 + half, ms->y0, zz, &a, &b);
		ps_plot (a, b, PSL_PEN_DRAW);
		GMT_xyz_to_xy (ms->x0 + half, ms->y0 - gmtdefs.map_scale_height, zz, &a, &b);
		ps_plot (a, b, PSL_PEN_DRAW_AND_STROKE);
		GMT_text3D (ms->x0, ms->y0 - off, zz, gmtdefs.annot_font_size[0], gmtdefs.annot_font[0], txt, 0.0, 10, 0);
	}
}

void GMT_draw_map_rose (struct GMT_MAP_ROSE *mr)
{
	if (!mr->plot) return;

	if (!GMT_IS_MAPPING) return;	/* Only for geographic projections */

	if (mr->gave_xy)	/* Also get lon/lat coordinates */
		GMT_xy_to_geo (&mr->lon, &mr->lat, mr->x0, mr->y0);
	else {	/* Must convert lon/lat to location on map */
		mr->lon = mr->x0;
		mr->lat = mr->y0;
		GMT_geo_to_xy (mr->lon, mr->lat, &mr->x0, &mr->y0);
	}

	/* Temporarily use miter to get sharp points to compass rose */
	if (gmtdefs.ps_line_join != 0) ps_setlinejoin (0);
	if (gmtdefs.ps_miter_limit != 0) ps_setmiterlimit (0);

	if (mr->fancy == 2)	/* Do magnetic compass rose */
		GMT_draw_mag_rose (mr);
	else
		GMT_draw_dir_rose (mr);

	/* Switch line join style back */
	if (gmtdefs.ps_line_join != 0) ps_setlinejoin (gmtdefs.ps_line_join);
	if (gmtdefs.ps_miter_limit != 0) ps_setmiterlimit (gmtdefs.ps_miter_limit);
}

/* These are used to scale the plain arrow given rose size */

#define F_VW	0.01
#define F_HL	0.15
#define F_HW	0.05

#define ROSE_LENGTH_SCL1	(0.5 * M_SQRT2)
#define ROSE_LENGTH_SCL2	0.5
#define ROSE_WIDTH_SCL1		0.2
#define ROSE_WIDTH_SCL2		0.2
#define ROSE_WIDTH_SCL3		0.2

#define DIST_TO_2ND_POINT 1.0

void GMT_draw_dir_rose (struct GMT_MAP_ROSE *mr)
{
	GMT_LONG i, kind, just[4] = {10, 5, 2, 7};
	double angle, L[4], R[4], x[8], y[8], xp[8], yp[8], tx[3], ty[3], s, c, rot[4] = {0.0, 45.0, 22.5, -22.5};
	struct GMT_FILL f;

	GMT_init_fill (&f, gmtdefs.background_rgb[0], gmtdefs.background_rgb[1], gmtdefs.background_rgb[2]);       /* Initialize fill structure */

	GMT_azim_to_angle (mr->lon, mr->lat, DIST_TO_2ND_POINT, 90.0, &angle);	/* Get angle of E-W direction at this location */

	GMT_setpen (&gmtdefs.tick_pen);

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
			x[0] = L[kind];	x[1] = x[7] = 0.5 * M_SQRT2 * R[kind];	x[2] = x[6] = 0.0;
			y[0] = y[4] = 0.0;	y[1] = y[3] = 0.5 * M_SQRT2 * R[kind];	y[2] = L[kind];
			x[3] = x[5] = -x[1];	x[4] = -x[0];
			y[5] = y[7] = -y[1];	y[6] = -y[2];
			GMT_rotate2D (x, y, (GMT_LONG)8, mr->x0, mr->y0, rot[kind] + angle, xp, yp);	/* Coordinate transformation and placement of the 4 labels */
			GMT_2D_to_3D (xp, yp, project_info.z_level, (GMT_LONG)8);				/* Then do 3-D projection */
			ps_polygon (xp, yp, (GMT_LONG)8, gmtdefs.foreground_rgb, TRUE);	/* Outline of 4-pointed star */
			GMT_xyz_to_xy (mr->x0, mr->y0, project_info.z_level, &tx[0], &ty[0]);
			/* Fill positive halfs of the 4-pointed blades */
			tx[1] = xp[0];	ty[1] = yp[0];	tx[2] = xp[7];	ty[2] = yp[7];
			ps_patch (tx, ty, (GMT_LONG)3, gmtdefs.background_rgb, TRUE);	/* East */
			tx[1] = xp[1];	ty[1] = yp[1];	tx[2] = xp[2];	ty[2] = yp[2];
			ps_patch (tx, ty, (GMT_LONG)3, gmtdefs.background_rgb, TRUE);	/* North */
			tx[1] = xp[3];	ty[1] = yp[3];	tx[2] = xp[4];	ty[2] = yp[4];
			ps_patch (tx, ty, (GMT_LONG)3, gmtdefs.background_rgb, TRUE);	/* West */
			tx[1] = xp[5];	ty[1] = yp[5];	tx[2] = xp[6];	ty[2] = yp[6];
			ps_patch (tx, ty, (GMT_LONG)3, gmtdefs.background_rgb, TRUE);	/* South */
		}
		sincosd (angle, &s, &c);
		x[0] = x[2] = 0.0;	x[1] = L[0] + gmtdefs.header_offset; x[3] = -x[1];
		y[1] = y[3] = 0.0;	y[2] = L[0] + gmtdefs.header_offset; y[0] = -y[2];
		GMT_rotate2D (x, y, (GMT_LONG)4, mr->x0, mr->y0, angle, xp, yp);	/* Coordinate transformation and placement of the 4 labels */
		for (i = 0; i < 4; i++) GMT_text3D (xp[i], yp[i], project_info.z_level, gmtdefs.header_font_size, gmtdefs.header_font, mr->label[i], angle, just[i], 0);
	}
	else {			/* Plain North arrow w/circle */
		sincosd (angle, &s, &c);
		x[0] = x[1] = x[4] = 0.0;	x[2] = -0.25 * mr->size;	x[3] = -x[2];
		y[0] = -0.5 * mr->size;	y[1] = -y[0];	 y[2] = y[3] = 0.0; y[4] = y[1] + gmtdefs.annot_offset[0];
		GMT_rotate2D (x, y, (GMT_LONG)5, mr->x0, mr->y0, angle, xp, yp);	/* Coordinate transformation and placement of the 4 labels */
		GMT_vector (xp[0], yp[0], xp[1], yp[1], project_info.z_level, F_VW * mr->size, F_HL * mr->size, F_HW * mr->size, gmtdefs.vector_shape, &f, TRUE);
		s = 0.25 * mr->size;
		GMT_init_fill (&f, -1, -1, -1);
		GMT_circle (mr->x0, mr->y0, project_info.z_level, &s, &f, TRUE);
		GMT_text3D (xp[4], yp[4], project_info.z_level, gmtdefs.header_font_size, gmtdefs.header_font, mr->label[2], angle, 2, 0);
		GMT_2D_to_3D (xp, yp, project_info.z_level, (GMT_LONG)4);
		ps_segment (xp[2], yp[2], xp[3], yp[3]);
	}
}

#define M_VW	0.005
#define M_HL	0.075
#define M_HW	0.015

void GMT_draw_mag_rose (struct GMT_MAP_ROSE *mr)
{	/* Magnetic compass rose */
	GMT_LONG i, k, level, just, ljust[4] = {10, 5, 2, 7}, n_tick;
	double ew_angle, angle, R[2], tlen[3], L, s, c, x[5], y[5], xp[5], yp[5], offset, t_angle, scale[2], base, *val;
	char label[16];
	struct GMT_FILL f;

	GMT_init_fill (&f, gmtdefs.background_rgb[0], gmtdefs.background_rgb[1], gmtdefs.background_rgb[2]);       /* Initialize fill structure */

	GMT_azim_to_angle (mr->lon, mr->lat, DIST_TO_2ND_POINT, 90.0, &ew_angle);	/* Get angle of E-W direction at this location */

	R[0] = 0.75 * 0.5 * mr->size;
	R[1] = 0.5 * mr->size;
	tlen[0] = 0.5 * gmtdefs.tick_length;
	tlen[1] = gmtdefs.tick_length;;
	tlen[2] = 1.5 * gmtdefs.tick_length;
	scale[0] = 0.85;
	scale[1] = 1.0;

	for (level = 0; level < 2; level++) {	/* Outer and inner angles */
		if (level == 0 && mr->kind == 1) continue;	/* Sorry, not magnetic directions */
		offset = (level == 0) ? mr->declination : 0.0;
		GMT_setpen (&gmtdefs.tick_pen);
		n_tick = GMT_linear_array (0.0, 360.0, mr->g_int[level], 0.0, &val);
		for (i = 0; i < n_tick - 1; i++) {	/* Increments of fine tickmarks (-1 to avoid repeating 360) */
			angle = offset + val[i];
			k = (GMT_IS_ZERO (fmod (val[i], mr->a_int[level]))) ? 2 : ((GMT_IS_ZERO (fmod (val[i], mr->f_int[level]))) ? 1 : 0);
			sincosd (ew_angle + angle, &s, &c);
			x[0] = mr->x0 + R[level] * c;	y[0] = mr->y0 + R[level] * s;
			x[1] = mr->x0 + (R[level] - scale[level]*tlen[k]) * c;	y[1] = mr->y0 + (R[level] - scale[level]*tlen[k]) * s;
			GMT_2D_to_3D (x, y, project_info.z_level, (GMT_LONG)2);
			ps_segment (x[0], y[0], x[1], y[1]);
		}
		GMT_free ((void *)val);

		ps_setpaint (gmtdefs.background_rgb);
		n_tick = GMT_linear_array (0.0, 360.0, mr->a_int[level], 0.0, &val);
		for (i = 0; i < n_tick - 1; i++) {	/* Increments of annotations (-1 to avoid repeating 360) */
			angle = 90.0 - (offset + val[i]);	/* Since val is azimuth */
			sincosd (ew_angle + angle, &s, &c);
			x[0] = mr->x0 + (R[level] + gmtdefs.annot_offset[level]) * c;	y[0] = mr->y0 + (R[level] + gmtdefs.annot_offset[level]) * s;
			sprintf (label, "%d", (int)irint (val[i]));
			t_angle = fmod ((double)(-val[i] - offset) + 360.0, 360.0);	/* Now in 0-360 range */
			if (t_angle > 180.0) t_angle -= 180.0;	/* Now in -180/180 range */
			if (t_angle > 90.0 || t_angle < -90.0) t_angle -= copysign (180.0, t_angle);
			just = (y[0] <= mr->y0) ? 10 : 2;
			if (level == 1 && GMT_IS_ZERO (val[i]-90.0))  t_angle = -90.0, just = 2;
			if (level == 1 && GMT_IS_ZERO (val[i]-270.0)) t_angle =  90.0, just = 2;
			GMT_text3D (x[0], y[0], project_info.z_level, gmtdefs.annot_font_size[level], gmtdefs.annot_font[level], label, t_angle, just, 0);
		}
		GMT_free ((void *)val);
	}
	/* Draw extra tick for the 4 main compass directions */
	GMT_setpen (&gmtdefs.tick_pen);
	base = R[1] + gmtdefs.annot_offset[1] + gmtdefs.annot_font_size[1] / 72.0;
	for (i = 0, k = 1; i < 360; i += 90, k++) {	/* 90-degree increments of tickmarks */
		angle = (double)i;
		sincosd (ew_angle + angle, &s, &c);
		x[0] = mr->x0 + R[1] * c;	y[0] = mr->y0 + R[1] * s;
		x[1] = mr->x0 + (R[1] + tlen[0]) * c;	y[1] = mr->y0 + (R[1] + tlen[0]) * s;
		GMT_2D_to_3D (x, y, project_info.z_level, (GMT_LONG)2);
		ps_segment (x[0], y[0], x[1], y[1]);
		if (!mr->label[k][0]) continue;	/* No label desired */
		x[0] = mr->x0 + base * c;	y[0] = mr->y0 + base * s;
		x[1] = mr->x0 + (base + 2.0 * tlen[2]) * c;	y[1] = mr->y0 + (base + 2.0 * tlen[2]) * s;
		GMT_2D_to_3D (x, y, project_info.z_level, (GMT_LONG)2);
		ps_segment (x[0], y[0], x[1], y[1]);
		if (k == 4) k = 0;
		if (k == 2 && mr->label[2][0] == '*') {
			x[0] = mr->x0 + (base + 2.0*tlen[2] + gmtdefs.header_offset+ 0.025*mr->size) * c;	y[0] = mr->y0 + (base + 2.0*tlen[2] + gmtdefs.header_offset + 0.025*mr->size) * s;
			GMT_Nstar (x[0], y[0], 0.1*mr->size);
		}
		else {
			ps_setpaint (gmtdefs.background_rgb);
			x[0] = mr->x0 + (base + 2.0*tlen[2] + gmtdefs.header_offset) * c;	y[0] = mr->y0 + (base + 2.0*tlen[2] + gmtdefs.header_offset) * s;
			GMT_text3D (x[0], y[0], project_info.z_level, gmtdefs.header_font_size, gmtdefs.header_font, mr->label[k], ew_angle, ljust[k], 0);
			GMT_setpen (&gmtdefs.tick_pen);
		}
	}

	if (mr->kind == 2) {	/* Compass needle and label */
		sincosd (ew_angle + (90.0 - mr->declination), &s, &c);
		L = R[0] - 2.0 * tlen[2];
		x[0] = mr->x0 - L * c;	y[0] = mr->y0 - L * s;
		x[1] = mr->x0 + L * c;	y[1] = mr->y0 + L * s;
		GMT_vector (x[0], y[0], x[1], y[1], project_info.z_level, M_VW * mr->size, M_HL * mr->size, M_HW * mr->size, gmtdefs.vector_shape, &f, TRUE);
		t_angle = fmod (ew_angle + 90.0 - mr->declination + 360.0, 360.0);	/* Now in 0-360 range */
		if (fabs (t_angle) > 90.0) t_angle -= copysign (180.0, t_angle);
		sincosd (t_angle, &s, &c);
		x[0] = mr->x0 - 2.0 * M_VW * mr->size * s;	y[0] = mr->y0 + 2.0 * M_VW * mr->size * c;
		ps_setpaint (gmtdefs.background_rgb);
		if (!strcmp(mr->dlabel, "-")) GMT_get_annot_label (mr->declination, mr->dlabel, TRUE, FALSE, 0, GMT_world_map);
		GMT_text3D (x[0], y[0], project_info.z_level, gmtdefs.label_font_size, gmtdefs.label_font, mr->dlabel, t_angle, 2, 0);
	}
	else {			/* Just geographic directions and a centered arrow */
		L = mr->size - 4.0*tlen[2];
		x[0] = x[1] = x[4] = 0.0;	x[2] = -0.25 * mr->size;	x[3] = -x[2];
		y[0] = -0.5 * L;		y[1] = -y[0];	 y[2] = y[3] = 0.0; y[4] = y[1] + gmtdefs.annot_offset[0];
		GMT_rotate2D (x, y, (GMT_LONG)5, mr->x0, mr->y0, ew_angle, xp, yp);	/* Coordinate transformation and placement of the 4 labels */
		GMT_vector (xp[0], yp[0], xp[1], yp[1], project_info.z_level, F_VW * mr->size, F_HL * mr->size, F_HW * mr->size, gmtdefs.vector_shape, &f, TRUE);
		s = 0.25 * mr->size;
		GMT_init_fill (&f, -1, -1, -1);
		GMT_circle (mr->x0, mr->y0, project_info.z_level, &s, &f, TRUE);
		GMT_2D_to_3D (xp, yp, project_info.z_level, (GMT_LONG)4);
		ps_segment (xp[2], yp[2], xp[3], yp[3]);
	}
}

void GMT_Nstar (double x0, double y0, double r)
{	/* Draw a fancy 5-pointed North star */
	GMT_LONG a;
	double r2, x[4], y[4], dir, dir2, s, c;

	r2 = r * 0.3;
	for (a = 0; a <= 360; a += 72) {	/* Azimuth of the 5 points on the star */
		/* Solid half */
		x[0] = x[3] = x0;	y[0] = y[3] = y0;
		dir = 90.0 - (double)a;
		sincosd (dir, &s, &c);
		x[1] = x0 + r * c;
		y[1] = y0 + r * s;
		dir2 = dir - 36.0;
		sincosd (dir2, &s, &c);
		x[2] = x0 + r2 * c;
		y[2] = y0 + r2 * s;
		GMT_2D_to_3D (x, y, project_info.z_level, (GMT_LONG)4);
		ps_patch (x, y, (GMT_LONG)4, gmtdefs.background_rgb, TRUE);
		/* Hollow half */
		x[0] = x[3] = x0;	y[0] = y[3] = y0;
		sincosd (dir, &s, &c);
		x[1] = x0 + r * c;
		y[1] = y0 + r * s;
		dir2 = dir + 36.0;
		sincosd (dir2, &s, &c);
		x[2] = x0 + r2 * c;
		y[2] = y0 + r2 * s;
		GMT_2D_to_3D (x, y, project_info.z_level, (GMT_LONG)4);
		ps_patch (x, y, (GMT_LONG)4, gmtdefs.foreground_rgb, TRUE);
	}
}

void GMT_setpen (struct GMT_PEN *pen)
{
	/* GMT_setpen issues PostScript code to set the specified pen.
	 * Must first convert from internal points to current dpi */

	GMT_LONG width, offset;
	int rgb[3];
	char *texture = CNULL;

	texture = GMT_convertpen (pen, &width, &offset, rgb);

	ps_setline (width);

	ps_setdash (texture, offset);
	GMT_free ((void *)texture);

	ps_setpaint (rgb);
}

void GMT_draw_custom_symbol (double x0, double y0, double z0, double size, struct GMT_CUSTOM_SYMBOL *symbol, struct GMT_PEN *pen, struct GMT_FILL *fill, GMT_LONG outline) {
	GMT_LONG n = 0, n_alloc = 0, na, i, font_no = gmtdefs.annot_font[0];
	GMT_LONG flush = FALSE, this_outline = FALSE;
	double x, y, *xx, *yy, *xp, *yp, font_size, dim[3];
	char cmd[GMT_TEXT_LEN], *c;
	struct GMT_CUSTOM_SYMBOL_ITEM *s;
	struct GMT_FILL *f = VNULL;
	struct GMT_PEN *p = VNULL;

	GMT_set_meminc (GMT_SMALL_CHUNK);
	sprintf (cmd, "Start of symbol %s", symbol->name);
	ps_comment (cmd);
	s = symbol->first;
	while (s) {
		x = x0 + s->x * size;
		y = y0 + s->y * size;
		dim[0] = s->p[0] * size;

		switch (s->action) {
			case GMT_ACTION_MOVE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				n = 0;
				if (n >= n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n, n_alloc, sizeof (double), GMT_program);
				xx[n] = x;	yy[n] = y;	n++;
				p = (s->pen)  ? s->pen  : pen;
				f = (s->fill) ? s->fill : fill;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				break;

			case GMT_ACTION_DRAW:
				flush = TRUE;
				if (n >= n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n, n_alloc, sizeof (double), GMT_program);
				xx[n] = x;	yy[n] = y;	n++;
				break;

			case GMT_ACTION_ARC:
				flush = TRUE;
				na = GMT_get_arc (x, y, 0.5 * s->p[0] * size, s->p[1], s->p[2], &xp, &yp);
				for (i = 0; i < na; i++) {
					if (n >= n_alloc) n_alloc = GMT_alloc_memory2 ((void **)&xx, (void **)&yy, n, n_alloc, sizeof (double), GMT_program);
					xx[n] = xp[i];	yy[n] = yp[i];	n++;
				}
				GMT_free ((void *)xp);	GMT_free ((void *)yp);
				break;

			case GMT_ACTION_CROSS:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				GMT_cross (x, y, z0, dim, NULL, FALSE);
				break;

			case GMT_ACTION_CIRCLE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				GMT_circle (x, y, z0, dim, f, this_outline);
				break;

			case GMT_ACTION_SQUARE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				GMT_square (x, y, z0, dim, f, this_outline);
				break;

			case GMT_ACTION_TRIANGLE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				GMT_triangle (x, y, z0, dim, f, this_outline);
				break;

			case GMT_ACTION_DIAMOND:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				GMT_diamond (x, y, z0, dim, f, this_outline);
				break;

			case GMT_ACTION_STAR:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				GMT_star (x, y, z0, dim, f, this_outline);
				break;

			case GMT_ACTION_HEXAGON:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				GMT_hexagon (x, y, z0, dim, f, this_outline);
				break;

			case GMT_ACTION_ITRIANGLE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				GMT_itriangle (x, y, z0, dim, f, this_outline);
				break;

			case GMT_ACTION_TEXT:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);

				if ((c = strchr (s->string, '%'))) {	/* Gave font name or number, too */
					*c = 0;		/* Replace % with the end of string NUL indicator */
					c++;		/* Go to next character */
					font_no = GMT_font_lookup (c, GMT_font, GMT_N_FONTS);
					if (font_no >= GMT_N_FONTS) {
						fprintf (stderr, "%s: custom symbol subcommand l contains bad font (set to %s (0))\n", GMT_program, GMT_font[gmtdefs.annot_font[0]].name);
						font_no = gmtdefs.annot_font[0];
					}
					ps_setfont (font_no);			/* Set the required font */
				}
				font_size = s->p[0] * size * 72.0;
				if (this_outline && f) {
					ps_setpaint (f->rgb);
					(project_info.three_D) ? GMT_text3D (x, y, z0, font_size, font_no, s->string, 0.0, 6, FALSE) : ps_text (x, y, font_size, s->string, 0.0, 6, FALSE);
					ps_setpaint (p->rgb);
					(project_info.three_D) ? GMT_text3D (x, y, z0, font_size, font_no, s->string, 0.0, 6, TRUE) : ps_text (x, y, font_size, s->string, 0.0, 6, TRUE);
				}
				else if (f)
					(project_info.three_D) ? GMT_text3D (x, y, z0, font_size, font_no, s->string, 0.0, 6, FALSE) : ps_text (x, y, font_size, s->string, 0.0, 6, FALSE);
				else
					(project_info.three_D) ? GMT_text3D (x, y, z0, font_size, font_no, s->string, 0.0, 6, TRUE) : ps_text (x, y, font_size, s->string, 0.0, 6, TRUE);
				break;

			case GMT_ACTION_PENTAGON:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				GMT_pentagon (x, y, z0, dim, f, this_outline);
				break;

			case GMT_ACTION_OCTAGON:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				GMT_octagon (x, y, z0, dim, f, this_outline);
				break;

			case GMT_ACTION_ELLIPSE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				dim[0] = s->p[0];	dim[1] = s->p[1] * size;	dim[2] = s->p[2] * size;
				GMT_ellipse (x, y, z0, dim, f, this_outline);
				break;

			case GMT_ACTION_PIE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				dim[1] = s->p[1];	dim[2] = s->p[2];
				GMT_pie (x, y, z0, dim, f, this_outline);
				break;

			case GMT_ACTION_RECT:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				this_outline = (p && p->rgb[0] == -1) ? FALSE : outline;
				if (this_outline) GMT_setpen (p);
				dim[1] = s->p[1] * size;
				GMT_rect (x - 0.5 * dim[0], y - 0.5 * dim[1], z0, dim, f, this_outline);
				break;

			default:
				fprintf (stderr, "GMT ERROR: %s : Unrecognized symbol code (%ld) passed to GMT_draw_custom_symbol\n", GMT_program, s->action);
				GMT_exit (EXIT_FAILURE);
				break;

		}

		s = s->next;
	}
	if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, this_outline, &flush);
	sprintf (cmd, "End of symbol %s\n", symbol->name);
	ps_comment (cmd);
	GMT_reset_meminc ();

	if (n_alloc) {
		GMT_free ((void *)xx);
		GMT_free ((void *)yy);
	}
}

void GMT_flush_symbol_piece (double *x, double *y, double z, GMT_LONG *n, struct GMT_PEN *p, struct GMT_FILL *f, GMT_LONG outline, GMT_LONG *flush) {
	GMT_LONG draw_outline;

	draw_outline = (outline && p->rgb[0] != -1) ? TRUE : FALSE;
	if (draw_outline) GMT_setpen (p);
	if (project_info.three_D) GMT_2Dz_to_3D (x, y, z, (GMT_LONG)*n);
	GMT_fill (x, y, (GMT_LONG)*n, f, draw_outline);
	*flush = FALSE;
	*n = 0;
}

/* Here lies general 2/3-D symbols that can be filled or painted */

/* In order to simplify interface to symbol plotting we will use this prototype:
 *
 * void GMT_symbol (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
 *
 * where size is now a pointer - that way those who need more than one size cane be handled
 * Some symbosl do not use fill etc - for those they are not accessed
 */

/* These symbols all take the same arguments:
 * SQUARE, CIRCLE, TRIANGLE, INVTRIANGLE, DIAMOND, HEXAGON, PENTAGON, OCTAGON, STAR */

void GMT_square (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the square symbol */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i;
		double xp[4], yp[4], S;
		S = size[0] * 0.3535533906;
		xp[0] = xp[3] = x - S;	xp[1] = xp[2] = x + S;
		yp[0] = yp[1] = y - S;	yp[2] = yp[3] = y + S;
		for (i = 0; i < 4; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		GMT_fill (xp, yp, (GMT_LONG)4, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_square (x, y, size[0], rgb, outline);
	}
	else
		ps_square (x, y, size[0], fill->rgb, outline);
}

void GMT_circle (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the circle symbol */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i;
		double da, s, c, S, xp[GMT_ELLIPSE_APPROX], yp[GMT_ELLIPSE_APPROX];

		da = TWO_PI / (GMT_ELLIPSE_APPROX - 1);
		S = size[0] * 0.5;
		for (i = 0; i < GMT_ELLIPSE_APPROX; i++) {
			sincos (i * da, &s, &c);
			xp[i] = x + S * c;
			yp[i] = y + S * s;
			GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		}
		GMT_fill (xp, yp, (GMT_LONG)GMT_ELLIPSE_APPROX, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_circle (x, y, size[0], rgb, outline);
	}
	else
		ps_circle (x, y, size[0], fill->rgb, outline);
}

void GMT_triangle (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the triangle symbol */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i;
		double xp[3], yp[3];

		xp[0] = x - 0.433012701892*size[0];	yp[0] = yp[1] = y - 0.25  * size[0];
		xp[1] = x + 0.433012701892*size[0];	xp[2] = x; 	yp[2] = y + 0.5 * size[0];
		for (i = 0; i < 3; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		GMT_fill (xp, yp, (GMT_LONG)3, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_triangle (x, y, size[0], rgb, outline);
	}
	else
		ps_triangle (x, y, size[0], fill->rgb, outline);
}

void GMT_itriangle (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the inverted triangle symbol */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i;
		double xp[3], yp[3];

		xp[0] = x - 0.433012701892*size[0];	yp[0] = yp[1] = y + 0.25 * size[0];
		xp[1] = x + 0.433012701892*size[0];	xp[2] = x; 	yp[2] = y - 0.5 * size[0];
		for (i = 0; i < 3; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		GMT_fill (xp, yp, (GMT_LONG)3, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_itriangle (x, y, size[0], rgb, outline);
	}
	else
		ps_itriangle (x, y, size[0], fill->rgb, outline);
}

void GMT_diamond (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the diamond symbol */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i;
		double xp[4], yp[4], S;

		S = size[0] * 0.5;
		xp[0] = xp[2] = x;	xp[1] = x - S;	xp[3] = x + S;
		yp[0] = y - S;	yp[1] = yp[3] = y;	yp[2] = y + S;
		for (i = 0; i < 4; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		GMT_fill (xp, yp, (GMT_LONG)4, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_diamond (x, y, size[0], rgb, outline);
	}
	else
		ps_diamond (x, y, size[0], fill->rgb, outline);
}

void GMT_hexagon (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the hexagon symbol */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i;
		double xp[6], yp[6], S, sx, sy;

		S = size[0] * 0.5;
		sx = 0.5 * S;	sy = 0.8660254038 * S;
		xp[0] = x + S;		yp[0] = y;
		xp[1] = x + sx;		yp[1] = y + sy;
		xp[2] = x - sx;		yp[2] = yp[1];
		xp[3] = x - S;		yp[3] = y;
		xp[4] = xp[2];		yp[4] = y - sy;
		xp[5] = xp[1];		yp[5] = yp[4];
		for (i = 0; i < 6; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		GMT_fill (xp, yp, (GMT_LONG)6, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_hexagon (x, y, size[0], rgb, outline);
	}
	else
		ps_hexagon (x, y, size[0], fill->rgb, outline);
}

void GMT_pentagon (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the pentagon symbol */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i;
		double s, c, S, xp[5], yp[5];

		S = size[0] * 0.5;
		for (i = 0; i < 5; i++) {
			sincosd (90.0 + i * 72.0, &s, &c);
			xp[i] = x + S * c;
			yp[i] = y + S * s;
			GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		}
		GMT_fill (xp, yp, (GMT_LONG)5, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_pentagon (x, y, size[0], rgb, outline);
	}
	else
		ps_pentagon (x, y, size[0], fill->rgb, outline);
}

void GMT_octagon (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the octagon symbol */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i;
		double s, c, S, xp[8], yp[8];

		S = size[0] * 0.5;
		for (i = 0; i < 8; i++) {
			sincosd (22.5 + i * 45.0, &s, &c);
			xp[i] = x + S * c;
			yp[i] = y + S * s;
			GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		}
		GMT_fill (xp, yp, (GMT_LONG)8, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_octagon (x, y, size[0], rgb, outline);
	}
	else
		ps_octagon (x, y, size[0], fill->rgb, outline);
}

void GMT_star (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the star symbol */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i, k;
		double xp[10], yp[10], S, s, c, s2;

		S = size[0] * 0.5;
		s2 = 0.38196601125 * S;
		for (i = k = 0; i < 5; i++) {
			sincosd (-54.0 + i * 72.0, &s, &c);
			xp[k] = x + S * c;
			yp[k] = y + S * s;
			GMT_xyz_to_xy (xp[k], yp[k], z, &xp[k], &yp[k]);
			k++;
			sincosd (-18.0 + i * 72.0, &s, &c);
			xp[k] = x + s2 * c;
			yp[k] = y + s2 * s;
			GMT_xyz_to_xy (xp[k], yp[k], z, &xp[k], &yp[k]);
			k++;
		}
		GMT_fill (xp, yp, (GMT_LONG)10, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_star (x, y, size[0], rgb, outline);
	}
	else
		ps_star (x, y, size[0], fill->rgb, outline);
}

void GMT_plus (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the plus symbol (fill, outline not accessed) */

	if (project_info.three_D) {
		GMT_LONG i;
		double S, xp[4], yp[4];

		S = size[0] * 0.5;
		xp[0] = xp[1] = x;	xp[2] = x - S;	xp[3] = x + S;
		yp[2] = yp[3] = y;	yp[0] = y - S;	yp[1] = y + S;
		for (i = 0; i < 4; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		ps_segment (xp[0], yp[0], xp[1], yp[1]);
		ps_segment (xp[2], yp[2], xp[3], yp[3]);
	}
	else
		ps_plus (x, y, size[0]);
}

void GMT_cross (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the cross symbol (fill, outline not accessed) */

	if (project_info.three_D) {
		GMT_LONG i;
		double S, xp[4], yp[4];

		S = size[0] * 0.353553391;
		xp[0] = xp[2] = x - S;	xp[1] = xp[3] = x + S;
		yp[0] = yp[3] = y - S;	yp[1] = yp[2] = y + S;
		for (i = 0; i < 4; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		ps_segment (xp[0], yp[0], xp[1], yp[1]);
		ps_segment (xp[2], yp[2], xp[3], yp[3]);
	}
	else
		ps_cross (x, y, size[0]);
}

void GMT_rect (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the rect symbol [x,y is lower left corner] */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i;
		double xp[4], yp[4];

		xp[0] = xp[3] = x;	xp[1] = xp[2] = x + size[0];
		yp[0] = yp[1] = y;	yp[2] = yp[3] = y + size[1];
		for (i = 0; i < 4; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		GMT_fill (xp, yp, (GMT_LONG)4, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_rect (x, y, x + size[0], y + size[1], rgb, outline);
	}
	else
		ps_rect (x, y, x + size[0], y + size[1], fill->rgb, outline);
}

void GMT_ellipse (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the ellipse symbol */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i;
		double dx, dy, da, s, c, sin_direction, cos_direction, xp[GMT_ELLIPSE_APPROX], yp[GMT_ELLIPSE_APPROX];

		sincosd (size[0], &sin_direction, &cos_direction);
		da = TWO_PI / (GMT_ELLIPSE_APPROX - 1);
		for (i = 0; i < GMT_ELLIPSE_APPROX; i++) {
			sincos (i * da, &s, &c);
			dx = 0.5 * size[1] * c;
			dy = 0.5 * size[2] * s;
			xp[i] = x + dx * cos_direction - dy * sin_direction;
			yp[i] = y + dx * sin_direction + dy * cos_direction;
			GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		}
		GMT_fill (xp, yp, (GMT_LONG)GMT_ELLIPSE_APPROX, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_ellipse (x, y, size[0], size[1], size[2], rgb, outline);
	}
	else
		ps_ellipse (x, y, size[0], size[1], size[2], fill->rgb, outline);
}

void GMT_pie (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the pie symbol */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i, j, n;
		double *xp, *yp, *dx, *dy;

		n = GMT_get_arc (x, y, size[0], size[1], size[2], &dx, &dy);
		(void)GMT_alloc_memory2 ((void **)&xp, (void **)&yp, n+1, 0, sizeof (double), GMT_program);

		xp[0] = x;	yp[0] = y;	/* Start from center */
		GMT_xyz_to_xy (xp[0], yp[0], z, &xp[0], &yp[0]);
		for (j = 0, i = 1; j < n; j++, i++) {
			xp[i] = dx[j];	yp[i] = dy[j];
			GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		}
		GMT_fill (xp, yp, (GMT_LONG)i, fill, outline);
		GMT_free ((void *)xp);
		GMT_free ((void *)yp);
		GMT_free ((void *)dx);
		GMT_free ((void *)dy);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_pie (x, y, size[0], size[1], size[2], rgb, outline);
	}
	else
		ps_pie (x, y, size[0], size[1], size[2], fill->rgb, outline);
}

void GMT_matharc (double x, double y, double z, double size[], double shape, struct GMT_PEN *pen, GMT_LONG status)
{
	/* Plots the math arc symbol */

	if (project_info.three_D) {	/* Must do arc [no arrow heads so far] */
		GMT_LONG j, n;
		double *xp = NULL, *yp = NULL, dx, dy;

		n = GMT_get_arc (x, y, size[0], size[1], size[2], &xp, &yp);

		for (j = 0; j < n; j++) {
			dx = xp[j];	dy = yp[j];
			GMT_xyz_to_xy (dx, dy, z, &xp[j], &yp[j]);
		}
		ps_line (xp, yp, (GMT_LONG)n, 3, FALSE);
		GMT_free ((void *)xp);
		GMT_free ((void *)yp);
	}
	else {
		ps_matharc (x, y, size[0], size[1], size[2], shape, status);
	}
}

void GMT_rotrect (double x, double y, double z, double size[], struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the rotated rectangle symbol */

	if (project_info.three_D) {	/* Must do polygon */
		GMT_LONG i;
		double W, H, xp[4], yp[4], x_prime, y_prime, s, c;

		W = size[1] * 0.5;	H = size[2] * 0.5;
		sincosd (size[0], &s, &c);
		xp[0] = xp[3] = -W;	xp[1] = xp[2] = W;
		yp[0] = yp[1] = -H;	yp[2] = yp[3] = H;
		for (i = 0; i < 4; i++) {
			x_prime = x + xp[i] * c - yp[i] * s;
			y_prime = y + xp[i] * s + yp[i] * c;
			xp[i] = x_prime;	yp[i] = y_prime;
			GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);
		}
		GMT_fill (xp, yp, (GMT_LONG)4, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_rotaterect (x, y, size[0], size[1], size[2], rgb, outline);
	}
	else
		ps_rotaterect (x, y, size[0], size[1], size[2], fill->rgb, outline);
}

/* Vector takes different kinds of arguments */

void GMT_vector (double x0, double y0, double x1, double y1, double z, double tailwidth, double headlength, double headwidth, double shape, struct GMT_FILL *fill, GMT_LONG outline)
{
	/* Plots the vector symbol */

	if (project_info.three_D) {	/* Fill in local xp, yp coordinates for vector starting at (0,0) aligned horizontally */
		GMT_LONG i, n;
		double xp[10], yp[10], angle, length, s, c, L, x, y;

		angle = atan2 (y1 - y0, x1 - x0);
		length = hypot (y1 - y0, x1 - x0);
		sincos (angle, &s, &c);
		L = (1.0 - 0.5 * shape) * headlength;
		if (outline & 8) {	/* Double-headed vector */
			outline -= 8;	/* Remove the flag */
			n = 10;
			xp[0] = 0.0;
			xp[1] = xp[9] = headlength;
			xp[2] = xp[8] = L;
			xp[3] = xp[7] = length - L;
			xp[4] = xp[6] = length - headlength;
			yp[0] = yp[5] = 0.0;
			yp[1] = yp[4] = -headwidth;
			yp[6] = yp[9] = headwidth;
			yp[2] = yp[3] = -0.5 * tailwidth;
			yp[7] = yp[8] = 0.5 * tailwidth;
			xp[5] = length;
		}
		else {
			n = 7;
			xp[0] = xp[6] = 0.0;
			xp[1] = xp[5] = length - L;
			xp[2] = xp[4] = length - headlength;
			xp[3] = length;
			yp[0] = yp[1] = -0.5 * tailwidth;
			yp[5] = yp[6] = 0.5 * tailwidth;
			yp[2] = -headwidth;
			yp[4] = headwidth;
			yp[3] = 0.0;
		}
		for (i = 0; i < n; i++) {	/* Coordinate transformation */
			x = x0 + xp[i] * c - yp[i] * s;	/* Rotate and add actual (x0, y0) origin */
			y = y0 + xp[i] * s + yp[i] * c;
			xp[i] = x;	yp[i] = y;
			GMT_xyz_to_xy (xp[i], yp[i], z, &xp[i], &yp[i]);	/* Then do 3-D projection */
		}
		GMT_fill (xp, yp, (GMT_LONG)n, fill, outline);
	}
	else if (fill && fill->use_pattern) {	/* Setup pattern first */
		int rgb[3] = {-3, -3, -3};
		rgb[1] = (int)ps_pattern (fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->f_rgb, fill->b_rgb);
		ps_vector (x0, y0, x1, y1, tailwidth, headlength, headwidth, shape, rgb, outline);
	}
	else
		ps_vector (x0, y0, x1, y1, tailwidth, headlength, headwidth, shape, fill->rgb, outline);
}

/* Plotting functions related to contours */

void GMT_contlabel_debug (struct GMT_CONTOUR *G)
{
	GMT_LONG i, j;
	int *pen;
	struct GMT_PEN P;

	/* If called we simply draw the helper lines or points to assist in debug */

	GMT_init_pen (&P, GMT_PENWIDTH);
	GMT_setpen (&P);
	if (G->fixed) {	/* Place a small open circle at each fixed point */
		for (i = 0; i < G->f_n; i++) ps_circle (G->f_xy[0][i], G->f_xy[1][i], 0.025, GMT_no_rgb, 1);
	}
	else if (G->crossing) {	/* Draw a thin line */
		for (j = 0; j < G->xp->n_segments; j++) {
			pen = (int *) GMT_memory (VNULL, (size_t)G->xp->segment[j]->n_rows, sizeof (int), GMT_program);
			for (i = 1, pen[0] = 3; i < G->xp->segment[j]->n_rows; i++) pen[i] = 2;
			GMT_plot_line (G->xp->segment[j]->coord[GMT_X], G->xp->segment[j]->coord[GMT_Y], pen, G->xp->segment[j]->n_rows);
			GMT_free ((void *)pen);
		}
	}
}

void GMT_contlabel_drawlines (struct GMT_CONTOUR *G, GMT_LONG mode)
{
	GMT_LONG i, k;
	int *pen;
	struct GMT_CONTOUR_LINE *C;
	char buffer[BUFSIZ];
	for (i = 0; i < G->n_segments; i++) {
		C = G->segment[i];	/* Pointer to current segment */
		if (C->annot && mode == 1) continue; /* Annotated lines done with curved text routine */
		GMT_setpen (&C->pen);
		pen = (int *) GMT_memory (VNULL, (size_t)C->n, sizeof (int), GMT_program);
		for (k = 1, pen[0] = 3; k < C->n; k++) pen[k] = 2;
		sprintf (buffer, "%s: %s", G->line_name, C->name);
		ps_comment (buffer);
		GMT_plot_line (C->x, C->y, pen, (GMT_LONG)C->n);
		GMT_free ((void *)pen);
	}
}

void GMT_contlabel_clippath (struct GMT_CONTOUR *G, GMT_LONG mode)
{
	GMT_LONG i, k, m, nseg, just, form;
	double *angle, *xt, *yt;
	char **txt;
	struct GMT_CONTOUR_LINE *C = VNULL;

	if (mode == 0) {	/* Turn OFF Clipping and bail */
		ps_comment ("Turn label clipping off:");
		ps_textclip (NULL, NULL, (GMT_LONG)0, NULL, NULL, 0.0, NULL, 0, 2);	/* This turns clipping OFF if it was ON in the first place */
		return;
	}

	for (i = m = nseg = 0; i < G->n_segments; i++) {	/* Get total number of segments with labels */
		C = G->segment[i];		/* Pointer to current segment */
		if (C->n_labels) {
			nseg++;
			m += C->n_labels;
		}
	}

	if (m == 0) return;	/* Nothing to do */

	/* Turn ON clipping */
	if (G->curved_text) {		/* Do it via the labeling PSL function */
		GMT_contlabel_plotlabels (G, 1);
		if (nseg == 1) G->box |= 8;	/* Special message to just repeate the labelline call */
	}
	else {				/* Same PS memory by doing it this way instead via ps_textclip */
		if (G->number_placement && G->n_cont == 1)		/* Special 1-label justification check */
			just = G->end_just[(G->number_placement+1)/2];	/* Gives index 0 or 1 */
		else
			just = G->just;
		/* Allocate temp space for everything that must be passed to ps_textclip */
		(void)GMT_alloc_memory3 ((void **)&angle, (void **)&xt, (void **)&yt, m, 0, sizeof (double), GMT_program);
		txt = (char **) GMT_memory (VNULL, (size_t)m, sizeof (char *), GMT_program);
		for (i = m = 0; i < G->n_segments; i++) {
			C = G->segment[i];	/* Pointer to current segment */
			for (k = 0; k < C->n_labels; k++, m++) {
				angle[m] = C->L[k].angle;
				txt[m] = C->L[k].label;
				xt[m] = C->L[k].x;
				yt[m] = C->L[k].y;
			}
		}
		if (project_info.three_D) {	/* Must place text items with GMT_text3D */
			GMT_2D_to_3D (xt, yt, G->z_level, (GMT_LONG)m);
		}
		/* Note this uses the last segments pen/fontrgb on behalf of all */
		GMT_textpath_init (&C->pen, G->rgb, &G->pen, C->font_rgb);
		form = (G->box & 4) ? 16 : 0;
		ps_textclip (xt, yt, (GMT_LONG)m, angle, txt, G->label_font_size, G->clearance, just, form);	/* This turns clipping ON */
		G->box |= 8;	/* Special message to just repeate the PSL call as variables have been defined */
		GMT_free ((void *)angle);
		GMT_free ((void *)xt);
		GMT_free ((void *)yt);
		GMT_free ((void *)txt);
	}
}

void GMT_contlabel_plotlabels (struct GMT_CONTOUR *G, GMT_LONG mode)
{	/* mode = 1 when clipping is in effect */
	GMT_LONG i, k, m, first_i, last_i, *node;
	GMT_LONG just, form;
	double *angle, *xt, *yt;
	char **txt;
	struct GMT_CONTOUR_LINE *C = VNULL;

	if (G->box & 8) {	/* Repeat call for Transparent text box (already set by clip) */
		form = 8;
		if (G->box & 1) form |= 256;		/* Transparent box with outline */
		if (G->box & 4) form |= 16;		/* Rounded box with outline */
		if (G->curved_text)
			ps_textpath (NULL, NULL, (GMT_LONG)0, NULL, NULL, NULL, (GMT_LONG)0, 0.0, NULL, 0, form);
		else
			ps_textclip (NULL, NULL, (GMT_LONG)0, NULL, NULL, 0.0, NULL, 0, (form | 1));
		return;
	}

	if (G->number_placement && G->n_cont == 1)		/* Special 1-label justification check */
		just = G->end_just[(G->number_placement+1)/2];	/* Gives index 0 or 1 */
	else
		just = G->just;

	for (i = last_i = m = 0, first_i = -1; i < G->n_segments; i++) {	/* Find first and last set of labels */
		C = G->segment[i];	/* Pointer to current segment */
		if (C->n_labels) {	/* This segment has labels */
			if (first_i == -1) first_i = i;	/* OK, this is the first */
			last_i = i;			/* When done, this will hold the last i */
			m += C->n_labels;		/* Total number of labels */
		}
	}

	if (m == 0) return;	/* There are no labels */

	if (project_info.three_D) {	/* 3-D or opaque straight text: text will be placed with GMT_text3D */
		for (i = 0; i < G->n_segments; i++) {
			C = G->segment[i];	/* Pointer to current segment */
			for (k = 0; k < C->n_labels; k++) {
				GMT_text3D (C->L[k].x, C->L[k].y, project_info.z_level, G->label_font_size, gmtdefs.annot_font[0], C->L[k].label, C->L[k].angle, just, 0);
			}
		}
	}
	else if (G->curved_text) {	/* Curved labels in 2D with transparent or opaque textbox: use ps_textpath */
		for (i = 0; i < G->n_segments; i++) {
			C = G->segment[i];	/* Pointer to current segment */
			if (!C->annot || C->n_labels == 0) continue;
			angle = (double *) GMT_memory (VNULL, (size_t)C->n_labels, sizeof (double), GMT_program);
			txt = (char **) GMT_memory (VNULL, (size_t)C->n_labels, sizeof (char *), GMT_program);
			node = (GMT_LONG *) GMT_memory (VNULL, (size_t)C->n_labels, sizeof (GMT_LONG), GMT_program);
			for (k = 0; k < C->n_labels; k++) {
				angle[k] = C->L[k].angle;
				txt[k] = C->L[k].label;
				node[k] = C->L[k].node;
			}

			form = mode;		/* 1 means clip labelboxes, 0 means place text */
			if (i == first_i) form |= 32;		/* First of possibly several calls to ps_textpath */
			if (i == last_i)  form |= 64;		/* Final call to ps_textpath */
			if (!G->transparent) form |= 128;	/* Want the box filled */
			if (G->box & 1) form |= 256;		/* Want box outline */
			GMT_textpath_init (&C->pen, G->rgb, &G->pen, C->font_rgb);
			ps_textpath (C->x, C->y, C->n, node, angle, txt, C->n_labels, G->label_font_size, G->clearance, just, form);
			GMT_free ((void *)angle);
			GMT_free ((void *)node);
			GMT_free ((void *)txt);
		}
	}
	else {	/* 2-D Straight transparent or opaque text labels: repeat call to ps_textclip */
		form = 1;
		if (G->box & 4) form |= 16;		/* Want round box shape */
		if (!G->transparent) form |= 128;	/* Want the box filled */
		if (G->box & 1) form |= 256;		/* Want box outline */

		if (mode == 0) {	/* Opaque so ps_textclip is called for 1st time here */
			/* Allocate temp space for everything that must be passed to ps_textclip */
			(void)GMT_alloc_memory3 ((void **)&angle, (void **)&xt, (void **)&yt, m, 0, sizeof (double), GMT_program);
			txt = (char **) GMT_memory (VNULL, (size_t)m, sizeof (char *), GMT_program);
			for (i = m = 0; i < G->n_segments; i++) {
				C = G->segment[i];	/* Pointer to current segment */
				for (k = 0; k < C->n_labels; k++, m++) {
					angle[m] = C->L[k].angle;
					txt[m] = C->L[k].label;
					xt[m] = C->L[k].x;
					yt[m] = C->L[k].y;
				}
			}
			if (project_info.three_D) {	/* Must place text items with GMT_text3D */
				GMT_2D_to_3D (xt, yt, G->z_level, m);
			}
			/* Note this uses the last segments pen/fontrgb on behalf of all */
			GMT_textpath_init (&C->pen, G->rgb, &G->pen, C->font_rgb);
			ps_textclip (xt, yt, m, angle, txt, G->label_font_size, G->clearance, just, form);	/* This turns clipping ON */
			GMT_free ((void *)angle);
			GMT_free ((void *)xt);
			GMT_free ((void *)yt);
			GMT_free ((void *)txt);
		}
		else {	/* 2nd time called, just pass form */
			form |= 8;
			ps_textclip (NULL, NULL, (GMT_LONG)0, NULL, NULL, 0.0, NULL, 0, form);	/* Now place the text using PSL variables already declared */
		}
	}
}

void GMT_textpath_init (struct GMT_PEN *LP, int Brgb[], struct GMT_PEN *BP, int Frgb[])
{
	char *texture;
	GMT_LONG width, offset;
	int rgb[3];

	texture = GMT_convertpen (LP, &width, &offset, rgb);
	ps_define_pen ("PSL_setlinepen", width, texture, offset, rgb);
	if (texture) GMT_free ((void *)texture);
	texture = GMT_convertpen (BP, &width, &offset, rgb);
	ps_define_pen ("PSL_setboxpen", width, texture, offset, rgb);
	if (texture) GMT_free ((void *)texture);
	ps_define_rgb ("PSL_setboxrgb", Brgb);
	ps_define_rgb ("PSL_settxtrgb", Frgb);
}

void GMT_contlabel_plotboxes (struct GMT_CONTOUR *G)
{
	GMT_LONG i, k, just;
	GMT_LONG outline;
	double x, y;
	struct GMT_CONTOUR_LINE *C;

	if (G->transparent) return;	/* Transparent boxes */

	outline = (G->box & 4) + (G->box & 1);		/* This will give outline as (4|0) + (1|0) = {0, 1, 4, or 5} */
	if (G->number_placement && G->n_cont == 1)		/* Special 1-label justification check */
		just = G->end_just[(G->number_placement+1)/2];	/* Gives index 0 or 1 */
	else
		just = G->just;
	for (i = 0; i < G->n_segments; i++) {
		C = G->segment[i];	/* Pointer to current segment */
		if (!C->annot || C->n_labels == 0) continue;
		GMT_setpen (&C->pen);
		for (k = 0; k < C->n_labels; k++) {
			x = C->L[k].x;	/* Make a copy since the justify operation may change x,y */
			y = C->L[k].y;
			/* GMT_smart_justify (G->just, C->L[k].angle, G->clearance[0], G->clearance[1], &x, &y); */	/* This may change x,y */
			GMT_textbox3D (x, y, project_info.z_level, G->label_font_size, gmtdefs.annot_font[0], C->L[k].label, C->L[k].angle, just, outline, G->clearance[0], G->clearance[1], G->rgb);
		}
	}
}

void GMT_contlabel_plot (struct GMT_CONTOUR *G)
{
	ps_setfont (G->label_font);
	ps_setpaint (G->font_rgb);
	if (G->debug) GMT_contlabel_debug (G);		/* Debugging lines and points */
	if (G->transparent) {		/* Transparent boxes */
		GMT_contlabel_clippath (G, 1);		/* Lays down clippath based on ALL labels */
		GMT_contlabel_drawlines (G, 0);		/* Safe to draw continuous lines everywhere - they will be clipped at labels */
		GMT_contlabel_clippath (G, 0);		/* Turn off label clipping so no need for GMT_map_clip_off */
		GMT_contlabel_plotlabels (G, 0);	/* Now plot labels where they go directly */
	}
	else {	/* Opaque text boxes */
		GMT_contlabel_drawlines (G, 0);
		if (project_info.three_D) GMT_contlabel_plotboxes (G);
		GMT_contlabel_plotlabels (G, 0);
	}
}

GMT_LONG GMT_plotinit (int argc, char *argv[])
{
	/* Shuffles parameters and calls ps_plotinit, issues PS comments regarding the GMT options
	 * and places a time stamp, if selected */

	GMT_LONG PS_bit_settings = 0, k, id;
	struct EPS *eps;
	char cmd[BUFSIZ];
	
	/* Load all the bits required by ps_plotinit */

	if (GMT_ps.portrait)    PS_bit_settings |= 1;
	if (GMT_ps.verbose)     PS_bit_settings |= 2;
	if (GMT_ps.heximage)    PS_bit_settings |= 4;
	if (GMT_ps.absolute)    PS_bit_settings |= 8;
	if (GMT_ps.colormode)   PS_bit_settings |= (GMT_ps.colormode   <<  9);
	if (GMT_ps.compress)    PS_bit_settings |= (GMT_ps.compress    << 12);
	if (GMT_ps.line_cap)    PS_bit_settings |= (GMT_ps.line_cap    << 14);
	if (GMT_ps.line_join)   PS_bit_settings |= (GMT_ps.line_join   << 16);
	if (GMT_ps.miter_limit) PS_bit_settings |= (GMT_ps.miter_limit << 18);
	if (GMT_ps.comments)    PS_bit_settings |= (1 << 30);

	/* Initialize the plot header and settings */

	eps = GMT_epsinfo (GMT_program);
	ps_plotinit_hires (CNULL, GMT_ps.overlay, PS_bit_settings, GMT_ps.x_origin, GMT_ps.y_origin,
		GMT_ps.x_scale, GMT_ps.y_scale, GMT_ps.n_copies, GMT_ps.dpi, GMT_INCH,
		GMT_ps.paper_width, GMT_ps.page_rgb, GMT_ps.encoding_name, eps);

	/* Issue the comments that allow us to trace down what command created this layer */

	GMT_echo_command (argc, argv);

	/* Create %%PROJ tag that ps2raster can use to prepare a ESRI world file */
	
	for (k = 0, id = -1; id == -1 && k < GMT_N_PROJ4; k++) if (GMT_proj4[k].id == project_info.projection) id = k;
	if (id >= 0) {			/* Valid projection for creating world file info */
		double Cartesian_m[4];	/* WESN equivalents in projected meters */
		char *pstr = NULL, proj4name[16];
		Cartesian_m[0] = (project_info.ymin - project_info.y0) * project_info.i_y_scale;
		Cartesian_m[1] = (project_info.xmax - project_info.x0) * project_info.i_x_scale;
		Cartesian_m[2] = (project_info.ymax - project_info.y0) * project_info.i_y_scale;
		Cartesian_m[3] = (project_info.xmin - project_info.x0) * project_info.i_x_scale;
		/* It woul be simpler if we had a cleaner way of telling when data is lon-lat */
		if (project_info.projection == GMT_LINEAR && GMT_IS_MAPPING)
			strcpy(proj4name, "latlong");
		else
			strcpy(proj4name, GMT_proj4[id].proj4name);

		sprintf (cmd, "%%%%PROJ: %s %.8f %.8f %.8f %.8f %.3f %.3f %.3f %.3f %s", proj4name,
			project_info.w, project_info.e, project_info.s, project_info.n,
			Cartesian_m[3], Cartesian_m[1], Cartesian_m[0], Cartesian_m[2], GMT_export2proj4(pstr));
		ps_command (cmd);
		free((void *)pstr);
	}

	/* Set transparency, if requested. Note that /SetTransparency actually sets the opacity, which is (1 - transparency) */
	if (gmtdefs.transparency[0] | gmtdefs.transparency[1]) {
		sprintf (cmd, "[ /ca %g /CA %g /BM /Normal /SetTransparency pdfmark\n", 1.0 - 0.01 * gmtdefs.transparency[0], 1.0 - 0.01 * gmtdefs.transparency[1]);
		ps_command (cmd);
	}

	/* If requested, place the timestamp */

	if (GMT_ps.unix_time_label[0] == 'c' && GMT_ps.unix_time_label[1] == 0) {
		GMT_LONG i;
		/* -Uc was given as shorthand for "plot current command line" */
		strcpy (GMT_ps.unix_time_label, argv[0]);
		for (i = 1; i < argc; i++) {
			if (argv[i][0] != '-') continue;	/* Skip file names */
			strcat (GMT_ps.unix_time_label, " ");
			strcat (GMT_ps.unix_time_label, argv[i]);
		}
	}
	if (GMT_ps.unix_time) GMT_timestamp (GMT_ps.unix_time_pos[0], GMT_ps.unix_time_pos[1], GMT_ps.unix_time_just, GMT_ps.unix_time_label);
	if (eps->name) free ((void *)eps->name);
	if (eps->title) free ((void *)eps->title);
	if (eps) GMT_free ((void *)eps);
	return (0);
}

GMT_LONG GMT_plotend (void) {
	if (gmtdefs.transparency[0] | gmtdefs.transparency[1]) ps_command ("[ /ca 1 /CA 1 /BM /Normal /SetTransparency pdfmark\n"); /* Reset transparency to fully opague, if required */
	ps_plotend (GMT_ps.last_page);
	return (0);
}

#define PADDING 72.0	/* Amount of padding room for annotations in points (1 inch) */

struct EPS *GMT_epsinfo (char *program)
{
	/* Supply info about the EPS file that will be created */

	GMT_LONG fno[6], id, i, n_fonts, last, move_up = FALSE, not_used = 0;
	double old_x0, old_y0, old_x1, old_y1;
	double tick_space, frame_space, u_dx, u_dy;
	double dy, x0, y0, orig_x0 = 0.0, orig_y0 = 0.0;
	char info[BUFSIZ];
	FILE *fp;
	struct passwd *pw;
	struct EPS *new;

	new = (struct EPS *) GMT_memory (VNULL, (size_t)1, sizeof (struct EPS), "GMT_epsinfo");

	/* Set the name of .gmt_bb_info file */
	if (GMT_TMPDIR)
		sprintf (info, "%s%c.gmt_bb_info", GMT_TMPDIR, DIR_DELIM);
	else
		sprintf (info, ".gmt_bb_info");

	/* First crudely estimate the boundingbox coordinates */

	if (GMT_ps.overlay && (fp = fopen (info, "r")) != NULL) {	/* Must get previous boundingbox values */
		not_used = fscanf (fp, "%d %d %lf %lf %lf %lf %lf %lf\n", &(new->portrait), &(new->clip_level), &orig_x0, &orig_y0, &old_x0, &old_y0, &old_x1, &old_y1);
		fclose (fp);
		x0 = orig_x0;
		y0 = orig_y0;
		if (GMT_ps.absolute) {	/* Absolute */
			x0 = GMT_ps.x_origin;
			y0 = GMT_ps.y_origin;
		}
		else {			/* Relative */
			x0 += GMT_ps.x_origin;
			y0 += GMT_ps.y_origin;
		}
	}
	else {	/* New plot, initialize stuff */
		old_x0 = old_y0 = old_x1 = old_y1 = 0.0;
		x0 = GMT_ps.x_origin;	/* Always absolute the first time */
		y0 = GMT_ps.y_origin;
		new->portrait = (int)GMT_ps.portrait;
		new->clip_level = 0;
	}

	/* Lower or increase clip level based on GMT_ps.clip (-1, 0 or +1) */
	new->clip_level += (int)GMT_ps.clip;

	/* Estimates the bounding box for this overlay */

	new->x0 = GMT_u2u[GMT_INCH][GMT_PT] * x0;
	new->y0 = GMT_u2u[GMT_INCH][GMT_PT] * y0;
	new->x1 = new->x0 + GMT_u2u[GMT_INCH][GMT_PT] * (z_project.xmax - z_project.xmin);
	new->y1 = new->y0 + GMT_u2u[GMT_INCH][GMT_PT] * (z_project.ymax - z_project.ymin);

	tick_space = (gmtdefs.tick_length > 0.0) ? GMT_u2u[GMT_INCH][GMT_PT] * gmtdefs.tick_length : 0.0;
	frame_space = GMT_u2u[GMT_INCH][GMT_PT] * gmtdefs.frame_width;
	if (frame_info.header[0]) {	/* Make space for header text */
		move_up = (GMT_IS_MAPPING || frame_info.side[2] == 2);
		dy = ((move_up) ? (gmtdefs.annot_font_size[0] + gmtdefs.label_font_size) * GMT_u2u[GMT_PT][GMT_INCH] : 0.0) + 2.5 * gmtdefs.annot_offset[0];
		new->y1 += tick_space + GMT_u2u[GMT_INCH][GMT_PT] * dy;
	}

	/* Find the max extent and use it if the overlay exceeds what we already have */

	/* Extend box in all directions depending on what kind of frame annotations we have */

	u_dx = (GMT_ps.unix_time && GMT_ps.unix_time_pos[0] < 0.0) ? -GMT_u2u[GMT_INCH][GMT_PT] * GMT_ps.unix_time_pos[0] : 0.0;
	u_dy = (GMT_ps.unix_time && GMT_ps.unix_time_pos[1] < 0.0) ? -GMT_u2u[GMT_INCH][GMT_PT] * GMT_ps.unix_time_pos[1] : 0.0;
	if (frame_info.plot && !project_info.three_D) {
		if (frame_info.side[3]) new->x0 -= MAX (u_dx, ((frame_info.side[3] == 2) ? PADDING : tick_space)); else new->x0 -= MAX (u_dx, frame_space);
		if (frame_info.side[0]) new->y0 -= MAX (u_dy, ((frame_info.side[0] == 2) ? PADDING : tick_space)); else new->y0 -= MAX (u_dy, frame_space);
		if (frame_info.side[1]) new->x1 += (frame_info.side[1] == 2) ? PADDING : tick_space; else new->x1 += frame_space;
		if (frame_info.side[2]) new->y1 += (frame_info.header[0] || frame_info.side[2] == 2) ? PADDING : tick_space; else new->y1 += frame_space;
	}
	else if (project_info.three_D) {
		new->x0 -= MAX (u_dx, PADDING/2.0);
		new->y0 -= MAX (u_dy, PADDING/2.0);
		new->x1 += PADDING/2.0;
		new->y1 += PADDING/2.0;
	}
	else if (GMT_ps.unix_time) {
		new->x0 -= u_dx;
		new->y0 -= u_dy;
	}

	/* Get the high water mark in all directions */

	if (GMT_ps.overlay) {
		new->x0 = MIN (old_x0, new->x0);
		new->y0 = MIN (old_y0, new->y0);
	}
	new->x1 = MAX (old_x1, new->x1);
	new->y1 = MAX (old_y1, new->y1);

	if (GMT_ps.absolute) {	/* Undo Absolute */
		x0 = orig_x0;
		y0 = orig_y0;
	}

	/* Update the bb file or tell use */

	if (GMT_ps.last_page) {	/* Clobber the .gmt_bb_info file and add label padding */
		(void) remove (info);	/* Don't really care if it is successful or not */
		if (new->clip_level > 0) fprintf (stderr, "%s: Warning: %d (?) external clip operations were not terminated!\n", GMT_program, new->clip_level);
		if (new->clip_level < 0) fprintf (stderr, "%s: Warning: %d extra terminations of external clip operations!\n", GMT_program, -new->clip_level);
	}
	else if ((fp = fopen (info, "w")) != NULL) {	/* Update the .gmt_bb_info file */
		fprintf (fp, "%d %d %g %g %g %g %g %g\n", new->portrait, new->clip_level, x0, y0, new->x0, new->y0, new->x1, new->y1);
		fclose (fp);
	}

	/* Get font names used */

	id = 0;
	if (GMT_ps.unix_time) {
		fno[0] = 0;
		fno[1] = 1;
		id = 2;
	}

	if (frame_info.header[0]) fno[id++] = gmtdefs.header_font;

	if (frame_info.axis[0].label[0] || frame_info.axis[1].label[0] || frame_info.axis[2].label[0]) fno[id++] = gmtdefs.label_font;

	fno[id++] = gmtdefs.annot_font[0];
	fno[id++] = gmtdefs.annot_font[1];

	qsort ((void *)fno, (size_t)id, sizeof (GMT_LONG), GMT_comp_int_asc);

	last = -1;
	for (i = n_fonts = 0; i < id; i++) {
		if (fno[i] != last) {	/* To avoid duplicates */
			new->fontno[n_fonts++] = (int)fno[i];
			last = fno[i];
		}
	}
	if (n_fonts < 6) new->fontno[n_fonts] = -1;	/* Terminate */

	/* Get user name and date */

	if ((pw = getpwuid (getuid ())) != NULL)
		new->name = strdup (pw->pw_name);
	else
		new->name = strdup ("unknown");
	sprintf (info, "GMT v%s Document from %s", GMT_VERSION, program);
	new->title = strdup (info);

	return (new);
}

#define JUMP_L 0
#define JUMP_R 1

void GMT_fill_polygon (double *lon, double *lat, double z, GMT_LONG n, struct GMT_FILL *F, GMT_LONG outline)
{
	/* When geographic data are plotted, polygons that cross the west map boundary will
	 * sometimes appear on the area bounded by the east map boundary - they "wrap around".
	 * This usually means we have a global map with (east-west) = 360.
	 * This function solves this by plotting the polygon three times:
	 * First time: Truncate polygon between left and right border
	 * Second time: Find where the polygon jumps and set all the points between jumps to
	 *	       the point on the west boundary at the same latitude.
	 * Third time: Find where the polygon jumps and set all the points between jumps to
	 *	       the point on the east boundary at the same latitude.
	 * In reality it depends on the nature of the first jump in which order we do the
	 * west and east truncation above.
	 * If the polygon is clipped or wraps around at a periodic boundary then we must
	 * be careful how we draw the outline (if selected).  It is only when there is no
	 * clipping/wrapping that we can call GMT_fill with outline set to the input argument;
	 * Otherwise we must first fill the polygon without an outline and then separately
	 * plot the outline as a path.
	 */

	GMT_LONG jump;
	GMT_LONG i, k, first, n_new, n_orig;
	GMT_LONG jump_dir = JUMP_L;
	double *x, *xp, *yp;
	PFD x_on_border[2];

	if (GMT_is_nofill (F)) {	/* Just draw outline, no fill, nor pattern */
		if (!outline) return;			/* No fill, no pattern, no outline. Hence nothing to do */
		if ((n_new = GMT_geo_to_xy_line (lon, lat, n)) == 0) return;	/* Nothing further to do */
		GMT_plot_line (GMT_x_plot, GMT_y_plot, GMT_pen, n_new);		/* Separately plot the outline */
		return;
	}
	
	if (GMT_IS_AZIMUTHAL || !GMT_world_map) {
		/* Because points way outside the map might get close to the antipode we must
		 * clip the polygon first.  The new radial clip handles this by excluding points
		 * beyond the horizon and adding arcs along the boundary between exit points
		 */
		if ((n_new = GMT_clip_to_map (lon, lat, n, &xp, &yp)) == 0) return;		/* All points are outside region */
		if (project_info.three_D) GMT_2Dz_to_3D (xp, yp, z, n_new);			/* Project onto 2-D plane first */
		if (n_new == n)	{	/* No clipping took place, OK to fill and outline polygon in one go */
			GMT_fill (xp, yp, n_new, F, outline);					/* Fill cartesian polygon and possibly draw outline */
			outline = FALSE;	/* Meaning we just draw the outline (if it was TRUE) */
		}
		else	/* Must fill with outline = FALSE then draw the outline separately (if outline = TRUE) */
			GMT_fill (xp, yp, n_new, F, FALSE);					/* Fill cartesian polygon but do not draw outline */
		/* Free the memory we are done with */
		GMT_free ((void *)xp);
		GMT_free ((void *)yp);
			
		if (!outline || (n_new = GMT_geo_to_xy_line (lon, lat, n)) == 0) return;	/* Nothing further to do */
		GMT_plot_line (GMT_x_plot, GMT_y_plot, GMT_pen, n_new);				/* Separately plot the outline */
		return;
	}

	/* Here we come for all non-azimuthal projections */

	n_orig = n;	/* Number of points in lon, lat array (needed at end to draw line) */
	if ((n = GMT_geo_to_xy_line (lon, lat, n)) == 0) return;		/* Convert to (x,y,pen) - return if nothing to do */

	if (!GMT_IS_MAPPING) {		/* Not geographic data so there are no periodic boundaries to worry about */
		if (project_info.three_D) GMT_2Dz_to_3D (GMT_x_plot, GMT_y_plot, z, n);			/* Project onto plane */
		GMT_fill (GMT_x_plot, GMT_y_plot, n, F, outline);
		return;
	}

	/* Check if there are any boundary jumps in the data as evidenced by pen up [3] */

	for (first = 1, jump = FALSE; first < n && !jump; first++) jump = (GMT_pen[first] != PSL_PEN_DRAW);
	if (!jump) {	/* We happened to avoid the periodic boundary - just paint and return */
		if (project_info.three_D) GMT_2Dz_to_3D (GMT_x_plot, GMT_y_plot, z, n);			/* Project onto plane */
		GMT_fill (GMT_x_plot, GMT_y_plot, n, F, outline);
		return;
	}

	/* Polygon wraps and we will plot it up to three times by truncating the part that would wrap the wrong way.
	 * Here we cannot use the clipped/wrapped polygon to draw outline - that is done at the end, separately */

	/* Temporary array to hold the modified x values */

	x = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), GMT_program);
	yp = (project_info.three_D) ? (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), GMT_program) : GMT_y_plot;

	x_on_border[JUMP_R] = GMT_left_boundary;	/* Pointers to functions that supply the x-coordinate of boundary for given y */
	x_on_border[JUMP_L] = GMT_right_boundary;

	/* Do the main truncation of bulk of polygon */

	for (i = 0, jump = FALSE; i < n; i++) {
		if (GMT_pen[i] == PSL_PEN_MOVE && i) {
			jump = !jump;
			jump_dir = (GMT_x_plot[i] > GMT_half_map_size) ? JUMP_R : JUMP_L;
		}
		x[i] = (jump) ? (*x_on_border[jump_dir]) (GMT_y_plot[i]) : GMT_x_plot[i];
	}
	if (project_info.three_D) {
		memcpy ((void *)yp, (void *)GMT_y_plot, (size_t)(n * sizeof (double)));
		GMT_2Dz_to_3D (x, yp, z, n);		/* Project onto plane */
	}
	GMT_fill (x, yp, n, F, FALSE);	/* Paint the truncated polygon */

	/* Then do the Left truncation since some wrapped pieces might not have been plotted (k > 0 means we found a piece) */

	jump_dir = (GMT_x_plot[first] > GMT_half_map_size) ? JUMP_L : JUMP_R;	/* Opposite */
	for (i = k = 0, jump = TRUE; i < n; i++) {
		if (GMT_pen[i] == PSL_PEN_MOVE && i) {
			jump = !jump;
			jump_dir = (GMT_x_plot[i] > GMT_half_map_size) ? JUMP_R : JUMP_L;
		}
		x[i] = (jump || jump_dir == JUMP_R) ? (*x_on_border[JUMP_R]) (GMT_y_plot[i]) : GMT_x_plot[i], k++;
	}
	if (project_info.three_D) {
		memcpy ((void *)yp, (void *)GMT_y_plot, (size_t)(n * sizeof (double)));
		GMT_2Dz_to_3D (x, yp, z, n);		/* Project onto plane */
	}
	if (k) GMT_fill (x, yp, n, F, FALSE);	/* Paint the truncated polygon */

	/* Then do the R truncation since some wrapped pieces might not have been plotted (k > 0 means we found a piece) */

	jump_dir = (GMT_x_plot[first] > GMT_half_map_size) ? JUMP_R : JUMP_L;	/* Opposite */
	for (i = k = 0, jump = TRUE; i < n; i++) {
		if (GMT_pen[i] == PSL_PEN_MOVE && i) {
			jump = !jump;
			jump_dir = (GMT_x_plot[i] > GMT_half_map_size) ? JUMP_R : JUMP_L;
		}
		x[i] = (jump || jump_dir == JUMP_L) ? (*x_on_border[JUMP_L]) (GMT_y_plot[i]) : GMT_x_plot[i], k++;
	}
	if (project_info.three_D) {
		memcpy ((void *)yp, (void *)GMT_y_plot, (size_t)(n * sizeof (double)));
		GMT_2Dz_to_3D (x, yp, z, n);		/* Project onto plane */
	}
	if (k) GMT_fill (x, yp, n, F, FALSE);	/* Paint the truncated polygon */

	GMT_free ((void *)x);
	if (project_info.three_D) GMT_free ((void *)yp);
	
	/* If outline is TRUE then we must now draw the outline */
	if (!outline || (n_new = GMT_geo_to_xy_line (lon, lat, n_orig)) == 0) return;	/* Nothing further to do */
	GMT_plot_line (GMT_x_plot, GMT_y_plot, GMT_pen, n_new);				/* Separately plot the outline */
}

void GMT_plot_ellipse (double lon, double lat, double z, double major, double minor, double azimuth, struct GMT_FILL fill, GMT_LONG outline)
{
	/* GMT_plot_ellipse takes the location, axes (in km), and azimuth of an ellipse
	   and draws the ellipse using the chosen map projection */

	GMT_LONG i;
	double delta_azimuth, sin_azimuth, cos_azimuth, sinp, cosp, angle, x, y, x_prime, y_prime, rho, c;
	double sin_c, cos_c, center, *px, *py;

	(void)GMT_alloc_memory2 ((void **)&px, (void **)&py, GMT_ELLIPSE_APPROX+1, 0, sizeof (double), GMT_program);

	delta_azimuth = 2.0 * M_PI / GMT_ELLIPSE_APPROX;
	major *= 500.0;	minor *= 500.0;	/* Convert to meters and semi-axes */
	sincosd (90.0 - azimuth, &sin_azimuth, &cos_azimuth);
	sincosd (lat, &sinp, &cosp);	/* Set up azimuthal equidistant projection */

	center = (project_info.central_meridian < project_info.w || project_info.central_meridian > project_info.e) ? 0.5 * (project_info.w + project_info.e) :  project_info.central_meridian;

	/* Approximate ellipse by a GMT_ELLIPSE_APPROX-sided polygon */

	for (i = 0; i < GMT_ELLIPSE_APPROX; i++) {

		angle = i * delta_azimuth;

		sincos (angle, &y, &x);
		x *= major;
		y *= minor;

		/* Get rotated coordinates in m */

		x_prime = x * cos_azimuth - y * sin_azimuth;
		y_prime = x * sin_azimuth + y * cos_azimuth;

		/* Convert m back to lon lat */

		rho = hypot (x_prime, y_prime);

		c = rho / project_info.EQ_RAD;
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
	px[GMT_ELLIPSE_APPROX] = px[0];	py[GMT_ELLIPSE_APPROX] = py[0];
	GMT_fill_polygon (px, py, z, (GMT_LONG)(GMT_ELLIPSE_APPROX+1), &fill, outline);

	GMT_free ((void *)px);
	GMT_free ((void *)py);
}

void GMT_plot_rectangle (double lon, double lat, double z, double width, double height, double azimuth, struct GMT_FILL fill, GMT_LONG outline)
{
	/* GMT_plot_rectangle takes the location, axes (in km), and azimuth of a rectangle
	   and draws the rectangle using the chosen map projection */

	double sin_azimuth, cos_azimuth, sinp, cosp, x, y, x_prime, y_prime, rho, c, dim[3];
	double sin_c, cos_c, center, lon_w, lat_w, lon_h, lat_h, xp, yp, xw, yw, xh, yh, tmp;

	GMT_azim_to_angle (lon, lat, 0.1, azimuth, &tmp);
	azimuth = tmp;
	GMT_geo_to_xy (lon, lat, &xp, &yp);		/* Center of rectangle */

	width *= 0.5;		height *= 0.5;		/* Get half-dimensions */
	width *= 1000.0;	height *= 1000.0;	/* Convert to meters */
	dim[0] = azimuth;
	sincosd (azimuth, &sin_azimuth, &cos_azimuth);
	sincosd (lat, &sinp, &cosp);		/* Set up azimuthal equidistant projection */

	center = (project_info.central_meridian < project_info.w || project_info.central_meridian > project_info.e) ? 0.5 * (project_info.w + project_info.e) :  project_info.central_meridian;

	/* Get first point width away from center */
	sincos (0.0, &y, &x);
	x *= width;
	y *= height;
	/* Get rotated coordinates in m */
	x_prime = x * cos_azimuth - y * sin_azimuth;
	y_prime = x * sin_azimuth + y * cos_azimuth;
	/* Convert m back to lon lat */
	rho = hypot (x_prime, y_prime);
	c = rho / project_info.EQ_RAD;
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
	GMT_geo_to_xy (lon_w, lat_w, &xw, &yw);	/* Get projected x,y coordinates */
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
	c = rho / project_info.EQ_RAD;
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
	GMT_geo_to_xy (lon_h, lat_h, &xh, &yh);
	dim[2] = 2.0 * hypot (xp - xh, yp - yh);	/* Estimate of rectangle width in plot units (inch) */
	GMT_rotrect (xp, yp, z, dim, &fill, outline);
}

void GMT_draw_fence (double x[], double y[], double z, GMT_LONG n, struct GMT_FRONTLINE *f, struct GMT_FILL *g, GMT_LONG outline)
{
	GMT_LONG i, ngap;
	GMT_LONG skip;
	double *s, xx[4], yy[4], dist = 0.0, w, frac, dx, dy, angle, dir1, dir2;
	double gap, x0, y0, xp, yp, len2, len3, cosa, sina, sa, ca, offx, offy, dim[3];

	if (n < 2) return;

	s = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), GMT_program);
	for (i = 1, s[0] = 0.0; i < n; i++) {
		/* Watch out for longitude wraps */
		dx = x[i] - x[i-1];
		w = GMT_half_map_width (y[i]);
		if (GMT_world_map && dx > w) dx = copysign (2 * w - fabs (dx), -dx);
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
			fprintf (stderr, "%s: Warning: Number of front ticks reset from 0 to 1 (check your arguments)\n", GMT_program);
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
			angle = d_atan2 (dy, dx);	/* In radians */
			skip = (GMT_world_map && fabs (dx) > GMT_half_map_width (y[i]));	/* Don't do ticks on jumps */
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
							if (project_info.three_D) GMT_2D_to_3D (xx, yy, z, (GMT_LONG)4);
							ps_patch (xx, yy, (GMT_LONG)4, g->rgb, outline);
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
							if (project_info.three_D) GMT_2D_to_3D (xx, yy, z, (GMT_LONG)3);
							ps_patch (xx, yy, (GMT_LONG)3, g->rgb, outline);
							break;
					}
					break;

				case GMT_FRONT_CIRCLE:	/* Circles */
					switch (f->f_sense) {
						case GMT_FRONT_CENTERED:
							GMT_circle (x0, y0, 0.0, &(f->f_len), g, outline);
							break;
						case GMT_FRONT_RIGHT:
							angle += M_PI;
						case GMT_FRONT_LEFT:
							dir1 = R2D * angle;
							dir2 = dir1 + 180.0;
							if (dir1 > dir2) dir1 -= 360.0;
							dim[0] = f->f_len;	dim[1] = dir1;	dim[2] = dir2;
							GMT_pie (x0, y0, 0.0, dim, g, outline);
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
					if (project_info.three_D) GMT_2D_to_3D (xx, yy, z, (GMT_LONG)4);
					ps_patch (xx, yy, (GMT_LONG)4, g->rgb, outline);
					break;

				case GMT_FRONT_SLIP: /* draw strike-slip arrows */
					sincos (angle, &sina, &cosa);
				        offx = gmtdefs.annot_offset[0] * sina; /* get offsets from front line */
					offy = gmtdefs.annot_offset[0] * cosa;
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
 					if (project_info.three_D) GMT_2D_to_3D (xx, yy, z, (GMT_LONG)3);
					ps_line (xx, yy, (GMT_LONG)3, 3, FALSE);

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
					if (project_info.three_D) GMT_2D_to_3D (xx, yy, z, (GMT_LONG)3);
					ps_line (xx, yy, (GMT_LONG)3, 3, FALSE);
					break;

				case GMT_FRONT_FAULT:	/* Normal fault ticks */
					xx[0] = xx[1] = x0;	yy[0] = yy[1] = y0;
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
					if (project_info.three_D) GMT_2D_to_3D (xx, yy, z, (GMT_LONG)2);
					ps_line (xx, yy, (GMT_LONG)2, 3, FALSE);
					break;
			}
			dist += gap;
		}
		i++;
	}
	GMT_free ((void *)s);
}

GMT_LONG set_do_seconds (double inc)
{	/* Determines if seconds are to be labelled based on size of increment */
	if (GMT_plot_calclock.geo.order[2] == -1) return (FALSE);			/* Seconds not requested by format */
	if (GMT_plot_calclock.geo.n_sec_decimals > 0) return (TRUE);			/* If asked for ss.xxx annotations */
	if (fabs (60.0 * fmod (fmod (inc, 1.0) * 60.0, 1.0)) >= 1.0) return (TRUE);	/* Multiples of >= 1 sec intervals */
	return (FALSE);
}

char *GMT_export2proj4(char *pStrOut) {
	char	szProj4[512];
	double	scale_factor, false_easting = 0, false_northing = 0, a, b, f;

	scale_factor = gmtdefs.map_scale_factor;
	szProj4[0] = 0;

	/* Cylindrical projections */
	if (project_info.projection == GMT_UTM) {
		sprintf( szProj4, "+proj=utm +zone=%d", (int)project_info.pars[0]);
		if (project_info.utm_hemisphere < 0) sprintf( szProj4, " +south");
	}
	else if (project_info.projection == GMT_MERCATOR) {
		sprintf( szProj4, "+proj=merc +lon_0=%.16g +k=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[0], scale_factor, false_easting, false_northing);
	}
	else if (project_info.projection == GMT_CYL_EQ) {
		sprintf( szProj4, "+proj=cea +lon_0=%.16g +lat_ts=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[1], project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_CYL_EQDIST) {
                 sprintf( szProj4, "+proj=eqc +lat_ts=%.16g +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[1], 0.0, project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_CYL_STEREO) {
	}
	else if (project_info.projection == GMT_MILLER) {
		sprintf( szProj4, "+proj=mill +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g +R_A",
			project_info.pars[1], project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_TM) {
		sprintf( szProj4, "+proj=tmerc +lat_0=%.16g +lon_0=%.16g +k=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[1], project_info.pars[0], scale_factor, false_easting, false_northing);
	}
	else if (project_info.projection == GMT_CASSINI) {
		sprintf( szProj4, "+proj=cass +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[1], project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_OBLIQUE_MERC) {
		sprintf( szProj4, "+unavailable");
		/*sprintf( szProj4, "+proj=omerc +lat_0=%.16g +lonc=%.16g +alpha=%.16g +k=%.16g +x_0=%.16g +y_0=%.16g",
		0.0,0.0,0.0,0.0,0.0,0.0 );*/
	}
	else if (project_info.projection == GMT_OBLIQUE_MERC_POLE) {
		sprintf( szProj4, "+unavailable");
	}

	/* Conic projections */
	else if (project_info.projection == GMT_ALBERS) {
		sprintf( szProj4, "+proj=aea +lat_1=%.16g +lat_2=%.16g +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[2], project_info.pars[3], project_info.pars[1], project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_ECONIC) {
		sprintf( szProj4, "+proj=eqdc +lat_1=%.16g +lat_2=%.16g +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[2], project_info.pars[3], project_info.pars[1], project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_LAMBERT) {
		sprintf( szProj4, "+proj=lcc +lat_1=%.16g +lat_2=%.16g +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[2], project_info.pars[3], project_info.pars[1], project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_POLYCONIC) {
		sprintf( szProj4, "+proj=poly +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[1], project_info.pars[0], false_easting, false_northing);
	}

	/* Azimuthal projections */
	else if (project_info.projection == GMT_STEREO) {
		sprintf( szProj4, "+proj=stere +lat_0=%.16g +lon_0=%.16g +k=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[1], project_info.pars[0], scale_factor, false_easting, false_northing);
	}
	else if (project_info.projection == GMT_LAMB_AZ_EQ) {
		sprintf( szProj4, "+proj=laea +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[1], project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_ORTHO) {
                 sprintf( szProj4, "+unavailable");
	}
	else if (project_info.projection == GMT_AZ_EQDIST) {
		sprintf( szProj4, "+proj=aeqd +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[1], project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_GNOMONIC) {
		sprintf( szProj4, "+proj=gnom +lat_0=%.16g +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[1], project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_GENPER) {
                 sprintf( szProj4, "+unavailable");
	}
	else if (project_info.projection == GMT_POLAR) {
                 sprintf( szProj4, "+unavailable");
	}

	/* Misc projections */
	else if (project_info.projection == GMT_MOLLWEIDE) {
                 sprintf( szProj4, "+proj=moll +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_HAMMER) {
                 sprintf( szProj4, "+unavailable");
	}
	else if (project_info.projection == GMT_SINUSOIDAL) {
                 sprintf( szProj4, "+proj=sinu +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_VANGRINTEN) {
                 sprintf( szProj4, "+proj=vandg +lon_0=%.16g +x_0=%.16g +y_0=%.16g +R_A",
			project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_ROBINSON) {
                 sprintf( szProj4, "+proj=robin +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_ECKERT4) {
                 sprintf( szProj4, "+proj=eck4 +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_ECKERT6) {
                 sprintf( szProj4, "+proj=eck6 +lon_0=%.16g +x_0=%.16g +y_0=%.16g",
			project_info.pars[0], false_easting, false_northing);
	}
	else if (project_info.projection == GMT_WINKEL) {
                 sprintf( szProj4, "+unavailable");
	}
	else if (project_info.projection == GMT_LINEAR && GMT_IS_MAPPING) {
                 sprintf( szProj4, "+proj=latlong");
	}
	else 
                 sprintf( szProj4, "+xy");	/* Probably useless as a info, but put there something */

	a = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].eq_radius;
	f = gmtdefs.ref_ellipsoid[gmtdefs.ellipsoid].flattening;
	b = a * (1 - f);
        sprintf( szProj4+strlen(szProj4), " +a=%.3f +b=%.6f", a, b);

	pStrOut = strdup(szProj4);
	return(pStrOut);
}
