/*--------------------------------------------------------------------
 *	$Id: gmt_plot.c,v 1.81 2003-04-01 20:03:27 pwessel Exp $
 *
 *	Copyright (c) 1991-2002 by P. Wessel and W. H. F. Smith
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
 *	GMT_text3d :		Draw perspective text
 *	GMT_textbox3d :		Draw perspective text box
 *	GMT_timestamp :		Plot UNIX time stamp with optional string
 *	GMT_vector3d :		Draw 3-D vector
 *	GMT_vertical_axis :	Draw 3-D vertical axes
 *	GMT_xy_axis :		Draw x or y axis
 *
 * INTERNAL Functions include:
 *	
 *	GMT_basemap_3D :		Plots 3-D basemap
 *	GMT_geoplot :			As ps_plot, but using lon/lat directly
 *	GMT_fill :			Convenience function for ps_imagefill
 *	GMT_get_angle :			Sub function to get annotation angles
 *	GMT_get_annot_label :		Construct degree/minute label
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
 *	GMT_polar_adjust :		Adjust label justification for polar projection
 */
 
#include "gmt.h"

int GMT_get_label_parameters(int side, double line_angle, int type, double *text_angle, int *justify);
int GMT_polar_adjust(int side, double angle, double x, double y);

void GMT_map_symbol(double *xx, double *yy, int *sides, double *line_angles, char *label, int nx, int type, BOOLEAN annotate);
void GMT_map_symbol_ew(double lat, char *label, double west, double east, BOOLEAN annotate);
void GMT_map_symbol_ns(double lon, char *label, double south, double north, BOOLEAN annotate);
void GMT_get_annot_label (double val, char *label, int do_minutes, int do_seconds, int lonlat, BOOLEAN worldmap);
void GMT_basemap_3D(int mode);
void GMT_xyz_axis3D(int axis_no, char axis, struct PLOT_AXIS *A, int annotate);
int GMT_coordinate_array (double min, double max, struct PLOT_AXIS_ITEM *T, double **array);
int GMT_linear_array (double min, double max, double delta, double **array);
int GMT_log_array (double min, double max, double delta, double **array);
int GMT_pow_array (double min, double max, double delta, int x_or_y, double **array);
int GMT_grid_clip_path (struct GRD_HEADER *h, double **x, double **y, BOOLEAN *donut);
void GMT_wesn_map_boundary (double w, double e, double s, double n);
void GMT_rect_map_boundary (double x0, double y0, double x1, double y1);
void GMT_theta_r_map_boundary (double w, double e, double s, double n);
void GMT_map_latline (double lat, double west, double east);
void GMT_map_lonline (double lon, double south, double north);
void GMT_map_tick (double *xx, double *yy, int *sides, double *angles, int nx, int type, double len);
void GMT_map_lontick (double lon, double south, double north, double len);
void GMT_map_lattick (double lat, double west, double east, double len);
int GMT_map_loncross (double lon, double south, double north, struct XINGS **xings);
int GMT_map_latcross (double lat, double west, double east, struct XINGS **xings);
int GMT_prepare_label (double angle, int side, double x, double y, int type, double *line_angle, double *text_angle, int *justify);
double GMT_get_annot_offset (BOOLEAN *flip);
int GMT_flip_justify (int justify);
BOOLEAN GMT_annot_too_crowded (double x, double y, int side);
BOOLEAN GMT_is_fancy_boundary (void);
void GMT_coordinate_to_x (double coord, double *x);
void GMT_coordinate_to_y (double coord, double *y);
int GMT_time_array (double min, double max, struct PLOT_AXIS_ITEM *T, double **array);
void GMT_timex_grid (double w, double e, double s, double n);
void GMT_timey_grid (double w, double e, double s, double n);
void GMT_get_time_label (char *string, struct GMT_PLOT_CALCLOCK *P, struct PLOT_AXIS_ITEM *T, GMT_dtime t);
void GMT_get_coordinate_label (char *string, struct GMT_PLOT_CALCLOCK *P, char *format, struct PLOT_AXIS_ITEM *T, double coord);
void GMT_get_primary_annot (struct PLOT_AXIS *A, int *primary, int *secondary);
BOOLEAN GMT_skip_second_annot (int item, double x, double x2[], int n, int primary, int secondary);
BOOLEAN GMT_fill_is_image (char *fill);
double GMT_set_label_offsets (int axis, double val0, double val1, struct PLOT_AXIS *A, int below, double annot_off[], double *label_off, int *annot_justify, int *label_justify, char *format);
void GMT_flush_symbol_piece (double *x, double *y, double z, int *n, struct GMT_PEN *p, struct GMT_FILL *f, BOOLEAN outline, BOOLEAN *flush);
struct CUSTOM_SYMBOL * GMT_init_custom_symbol (char *name);

void GMT_map_tickitem (double w, double e, double s, double n, int item);

/* Local variables to this file */

int GMT_n_annotations[4] = {0, 0, 0, 0};
int GMT_alloc_annotations[4] = {GMT_SMALL_CHUNK, GMT_SMALL_CHUNK, GMT_SMALL_CHUNK, GMT_SMALL_CHUNK};
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

/*	LINEAR PROJECTION MAP BOUNDARY	*/

void GMT_linear_map_boundary (double w, double e, double s, double n)
{
	double x1, x2, y1, y2, x_length, y_length;
	
	GMT_geo_to_xy (w, s, &x1, &y1);
	GMT_geo_to_xy (e, n, &x2, &y2);
	if (x1 > x2) d_swap (x1, x2);
	if (y1 > y2) d_swap (y1, y2);
	x_length = fabs (x2 - x1);
	y_length = fabs (y2 - y1);
	
	if (frame_info.side[3]) GMT_xy_axis (x1, y1, y_length, s, n, &frame_info.axis[1], TRUE,  frame_info.side[3]-1);	/* West or left y-axis */
	if (frame_info.side[1]) GMT_xy_axis (x2, y1, y_length, s, n, &frame_info.axis[1], FALSE, frame_info.side[1]-1);	/* East or right y-axis */
	if (frame_info.side[0]) GMT_xy_axis (x1, y1, x_length, w, e, &frame_info.axis[0], TRUE,  frame_info.side[0]-1);	/* South or lower x-axis */
	if (frame_info.side[2]) GMT_xy_axis (x1, y2, x_length, w, e, &frame_info.axis[0], FALSE, frame_info.side[2]-1);	/* North or upper x-axis */
	
	if (!frame_info.header[0]) return;	/* No header today */
	
	if (frame_info.side[2] == 0) ps_set_length ("PSL_H_y", 0.0);	/* PSL_H was not set by GMT_xy_axis */
	ps_set_length ("PSL_x", 0.5 * x_length);
	ps_set_length ("PSL_y", y_length);
	ps_set_height ("PSL_HF", gmtdefs.header_font_size);
	ps_textdim ("PSL_dimx", "PSL_dimy", gmtdefs.header_font_size, gmtdefs.header_font, frame_info.header, 0);			/* Get and set string dimensions in PostScript */
	ps_command ("PSL_x PSL_dimx -0.5 mul add PSL_y PSL_H_y add M");
	ps_setfont (gmtdefs.header_font);
	ps_text (0.0, 0.0, -gmtdefs.header_font_size, frame_info.header, 0.0, 0, 0);

}

void GMT_get_primary_annot (struct PLOT_AXIS *A, int *primary, int *secondary)
{	/* Return the primary and secondary annotation item numbers */
	
	int i, no[2] = {GMT_ANNOT_UPPER, GMT_ANNOT_LOWER};
	double val[2], s;
	
	for (i = 0; i < 2; i++) {
		switch (A->item[no[i]].unit) {
			case 'Y':
			case 'y':
				s = GMT_DAY2SEC_F * 365.25;
				break;
			case 'O':
			case 'o':
				s = GMT_DAY2SEC_F * 30.5;
				break;
			case 'U':
			case 'u':
				s = GMT_DAY2SEC_F * 7.0;
				break;
			case 'K':
			case 'k':
			case 'D':
			case 'd':
				s = GMT_DAY2SEC_F;
				break;
			case 'H':
			case 'h':
				s = GMT_HR2SEC_F;
				break;
			case 'M':
			case 'm':
				s = GMT_MIN2SEC_F;
				break;
			case 'C':
			case 'c':
				s = 1.0;
				break;
			default:
				break;
		}
		val[i] = A->item[no[i]].interval * s;
	}
	if (val[0] > val[1]) {
		*primary = GMT_ANNOT_UPPER;
		*secondary = GMT_ANNOT_LOWER;
	}
	else {
		*primary = GMT_ANNOT_LOWER;
		*secondary = GMT_ANNOT_UPPER;
	}
}

BOOLEAN GMT_skip_second_annot (int item, double x, double x2[], int n, int primary, int secondary)
{
	int i;
	double small;
	BOOLEAN found;
	
	if (primary == secondary) return (FALSE);	/* Not set, no need to skip */
	if (secondary != item) return (FALSE);		/* Not working on secondary annotation */
	
	small = (x2[1] - x2[0]) * GMT_CONV_LIMIT;
	for (i = 0, found = FALSE; !found && i < n; i++) found = (fabs (x2[i] - x) < small);
	return (found);
}
	
int GMT_annot_pos (double min, double max, struct PLOT_AXIS_ITEM *T, double coord[], double *pos)
{
	/* Calculates the location of the next annotation in user units.  This is
	 * trivial for tick annotations but can be tricky for interval annotations
	 * since the annotation location is not necessarily centered on the interval.
	 * For instance, if our interval is 3 months we do not want "January" centered
	 * on that quarter.  If the position is outside our range we return TRUE
	 */
	double half_width, start, stop;
	 
	if (GMT_interval_axis_item(T->id)) {
		if (GMT_uneven_interval (T->unit) && T->interval != 1.0) {	/* Must find next month to get month centered correctly */
			struct GMT_MOMENT_INTERVAL Inext;
			Inext.unit = T->unit;		/* Initialize MOMENT_INTERVAL structure members */
			Inext.step = 1;
			GMT_moment_interval (&Inext, coord[0], TRUE);	/* Get this one interval only */
			half_width = 0.5 * (Inext.dt[1] - Inext.dt[0]);	/* Half width of interval in internal representation */
			start = MAX (min, Inext.dt[0]);			/* Start of interval, but not less that start of axis */
			stop  = MIN (max, Inext.dt[1]);			/* Stop of interval,  but not beyond end of axis */
		}
		else {
			half_width = 0.5 * (coord[1] - coord[0]);	/* Half width of interval in internal representation */
			start = MAX (min, coord[0]);			/* Start of interval, but not less that start of axis */
			stop  = MIN (max, coord[1]);			/* Stop of interval,  but not beyond end of axis */
		}
		if ((stop - start) < half_width) return (TRUE);		/* Sorry, fraction not large enough to annotate */
		*pos = 0.5 * (start + stop);				/* Set half-way point */
		if (((*pos) - GMT_CONV_LIMIT) < min || ((*pos) + GMT_CONV_LIMIT) > max) return (TRUE);	/* Outside axis range */
	}
	else if (coord[0] < (min - GMT_CONV_LIMIT) || coord[0] > (max + GMT_CONV_LIMIT))		/* Outside axis range */
		return (TRUE);
	else
		*pos = coord[0];

	return (FALSE);
}

void GMT_get_time_label (char *string, struct GMT_PLOT_CALCLOCK *P, struct PLOT_AXIS_ITEM *T, GMT_dtime t)
{	/* Assemble the annotation label given the formatting options presented */
	struct GMT_gcal calendar;

	GMT_gcal_from_dt (t, &calendar);			/* Convert t to a complete calendar structure */

	switch (T->unit) {
		case 'Y':	/* 4-digit integer year */
			(P->date.compact) ? sprintf (string, "%d", calendar.year) : sprintf (string, "%4.4d", calendar.year);
			break;
		case 'y':	/* 2-digit integer year */
			(P->date.compact) ? sprintf (string, "%d", calendar.year % 100) : sprintf (string, "%2.2d", calendar.year % 100);
			break;
		case 'O':	/* Plot via date format */
			GMT_format_calendar (string, CNULL, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'o':	/* 2-digit month */
			(P->date.compact) ? sprintf (string, "%d", calendar.month) : sprintf (string, "%2.2d", calendar.month);
			break;
		case 'U':	/* ISO year, week, day via date format */
			GMT_format_calendar (string, CNULL, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'u':	/* 2-digit ISO week */		
			(P->date.compact) ? sprintf (string, "%d", calendar.iso_w) : sprintf (string, "%2.2d", calendar.iso_w);
			break;
		case 'K':	/*  ISO Weekday name */
			if (T->upper_case) GMT_str_toupper (GMT_time_language.day_name[calendar.iso_d%7][T->flavor]);
			sprintf (string, "%s", GMT_time_language.day_name[calendar.iso_d%7][T->flavor]);
			break;
		case 'k':	/* Day of the week 1-7 */
			sprintf (string, "%d", (calendar.day_w - gmtdefs.time_week_start + 7) % 7 + 1);
			break;
		case 'D':	/* Day, via date format */
			GMT_format_calendar (string, CNULL, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'd':	/* 2-digit day or 3-digit day of year */
			if (P->date.day_of_year)
				(P->date.compact) ? sprintf (string, "%d", calendar.day_y) : sprintf (string, "%3.3d", calendar.day_y);
			else
				(P->date.compact) ? sprintf (string, "%d", calendar.day_m) : sprintf (string, "%2.2d", calendar.day_m);
			break;
		case 'H':	/* Hours via clock format */
			GMT_format_calendar (CNULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'h':	/* 2-digit hour */
			(P->date.compact) ? sprintf (string, "%d", calendar.hour) : sprintf (string, "%2.2d", calendar.hour);
			break;
		case 'M':	/* Minutes via clock format */
			GMT_format_calendar (CNULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'm':	/* 2-digit minutes */
			(P->date.compact) ? sprintf (string, "%d", calendar.min) : sprintf (string, "%2.2d", calendar.min);
			break;
		case 'C':	/* Seconds via clock format */
			GMT_format_calendar (CNULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'c':	/* 2-digit seconds */
			(P->date.compact) ? sprintf (string, "%d", (int)calendar.sec) : sprintf (string, "%2.2d", (int)calendar.sec);
			break;
		default:
			fprintf (stderr, "ERROR: wrong unit passed to GMT_get_time_label\n");
			exit (EXIT_FAILURE);
			break;
	}
}
		
void GMT_get_coordinate_label (char *string, struct GMT_PLOT_CALCLOCK *P, char *format, struct PLOT_AXIS_ITEM *T, double coord)
{
	/* Returns the formatted annotation string for the non-geographic axes */
	
	double tmp;
	
	switch (frame_info.axis[T->parent].type) {
		case LINEAR:
			sprintf (string, format, coord);
			break;
		case LOG10:
			sprintf (string, "%d", irint (d_log10 (coord)));
			break;
		case POW:
			if (project_info.xyz_projection[T->parent] == POW)
				sprintf (string, format, coord);
			else
				sprintf (string, "10@+%d@+", irint (d_log10 (coord)));
			break;
		case TIME:
			GMT_get_time_label (string, P, T, coord);
			break;
		default:
			fprintf (stderr, "%s: GMT ERROR: Wrong type (%d) passed to GMT_get_coordinate_label!\n", GMT_program, frame_info.axis[T->parent].type);
			exit (EXIT_FAILURE);
			break;
	}
}

void GMT_coordinate_to_x (double coord, double *x)
{
	/* Convert a x user coordinate to map position in inches */

	(*GMT_x_forward) (coord, x);
	(*x) = (*x) * project_info.x_scale + project_info.x0;
}
	
void GMT_coordinate_to_y (double coord, double *y)
{
	/* Convert a GMT time representation to map position in x */

	(*GMT_y_forward) (coord, y);
	(*y) = (*y) * project_info.y_scale + project_info.y0;
}
	
void GMT_xy_axis (double x0, double y0, double length, double val0, double val1, struct PLOT_AXIS *A, int below, int annotate)
{
	int k, i, nx, np = 0;		/* Misc. variables */
	int annot_pos;			/* Either 0 for upper annotation or 1 for lower annotation */
	int primary = 0;		/* Axis item number of annotation with largest interval/unit */
	int secondary = 0;		/* Axis item number of annotation with smallest interval/unit */
	int axis;			/* Axis id (0 = x, 1 = y) */
	BOOLEAN is_interval;		/* TRUE when the annotation is interval annotation and not tick annotation */
	BOOLEAN check_annotation;	/* TRUE is we have two levels of tick annotations that can overprint */
	BOOLEAN do_annot;		/* TRUE unless we are dealing with Gregorian weeks */
	BOOLEAN do_tick;		/* TRUE unless we are dealing with bits of weeks */
	double *knots, *knots_p;	/* Array pointers with tick/annotation knots, the latter for primary annotations */
	double tick_len[5];		/* Ticklengths for each of the 5 axis items */
	double x, sign, len, t_use;	/* Misc. variables */
	double font_size;			/* Annotation font size (ANNOT_FONT_SIZE or ANNOT_FONT2_SIZE) */
	struct PLOT_AXIS_ITEM *T;	/* Pointer to the current axis item */
	char string[GMT_CALSTRING_LENGTH];	/* Annotation string */
	char format[32];		/* format used for non-time annotations */
	char xy[2] = {'y', 'x'};
	char cmd[BUFSIZ];
	int rot[2], font;
	/* Initialize parameters for this axis */
	
	axis = A->item[0].parent;
	if ((check_annotation = GMT_two_annot_items(axis))) {					/* Must worry about annotation overlap */
		GMT_get_primary_annot (A, &primary, &secondary);					/* Find primary and secondary axis items */
		np = GMT_coordinate_array (val0, val1, &A->item[primary], &knots_p);		/* Get all the primary tick annotation knots */
	}
	if (axis == 1) below = !below;
	len = (gmtdefs.tick_length > 0.0) ? gmtdefs.tick_length : 0.0;				/* Tick length if directed outward */
	sign = (below) ? -1.0 : 1.0;								/* since annotations go either below or above */
	tick_len[0] = tick_len[2] = sign * gmtdefs.tick_length;					/* Initialize the tick lengths */
	tick_len[1] = 3.0 * sign * gmtdefs.tick_length;
	tick_len[3] = (A->item[GMT_ANNOT_UPPER].active) ? tick_len[0] : 3.0 * sign * gmtdefs.tick_length;
	tick_len[4] = 0.5 * sign * gmtdefs.tick_length;
	if (A->type != TIME) GMT_get_format (GMT_get_map_interval (axis, GMT_ANNOT_UPPER), A->unit, format);	/* Set the annotation format template */
	
	/* Ready to draw axis */
	
	ps_setfont (gmtdefs.annot_font);
	if (axis == 0) {
		if (below) ps_comment ("Start of lower x-axis"); else ps_comment ("Start of upper x-axis");
		ps_transrotate (x0, y0, 0.0);
	}
	else if (axis == 1) {
		if (below) ps_comment ("Start of right y-axis"); else ps_comment ("Start of left y-axis");
		ps_transrotate (x0, y0, 90.0);
	}
	
	/* Create PostScript definitions of various lengths and font sizes */
	(below) ? ps_command ("/PSL_sign -1 def") :  ps_command ("/PSL_sign 1 def");
	ps_set_length ("PSL_TL1", gmtdefs.tick_length);
	ps_set_length ("PSL_AO0", gmtdefs.annot_offset);
	(A->item[GMT_ANNOT_LOWER].active || A->item[GMT_INTV_LOWER].active) ? ps_set_length ("PSL_AO1", gmtdefs.annot_offset2) : ps_set_length ("PSL_AO1", 0.0);
	ps_set_length ("PSL_LO", gmtdefs.label_offset);
	ps_set_length ("PSL_HO", gmtdefs.header_offset);
	ps_set_length ("PSL_AH0", 0.0);
	ps_set_length ("PSL_AH1", 0.0);
	ps_set_height ("PSL_AF0", gmtdefs.annot_font_size);
	ps_set_height ("PSL_AF1", gmtdefs.annot_font2_size);
	ps_set_height ("PSL_LF", gmtdefs.label_font_size);

	ps_comment ("Axis tick marks");
	GMT_setpen (&gmtdefs.frame_pen);
	ps_segment (0.0, 0.0, length, 0.0);
	GMT_setpen (&gmtdefs.tick_pen);

	rot[0] = (A->item[GMT_ANNOT_UPPER].active && axis == 1 && gmtdefs.y_axis_type == 0) ? 1 : 0;
	rot[1] = (A->item[GMT_ANNOT_LOWER].active && axis == 1 && gmtdefs.y_axis_type == 0) ? 1 : 0;
	for (k = 0; k < GMT_GRID_UPPER; k++) {	/* For each one of the 5 axis items (gridlines are done separately) */
		
		T = &A->item[k];		/* Get pointer to this item */
		if (!T->active) continue;	/* Don't want this item plotted - goto next item */
		
		is_interval = GMT_interval_axis_item(k);			/* Interval or tick mark annotation? */
		nx = GMT_coordinate_array (val0, val1, &A->item[k], &knots);	/* Get all the annotation tick knots */
		
		/* First plot all the tick marks */
		
		do_tick = !((T->unit == 'K' || T->unit == 'k') && (T->interval > 1 && fmod (T->interval, 7.0) > 0.0));
		for (i = 0; do_tick && i < nx; i++) {
			if (knots[i] < (val0 - GMT_CONV_LIMIT) || knots[i] > (val1 + GMT_CONV_LIMIT)) continue;	/* Outside the range */
			(axis == 0) ? GMT_coordinate_to_x (knots[i], &x) : GMT_coordinate_to_y (knots[i], &x);	/* Convert to inches on the page */
			ps_segment (x, 0.0, x, tick_len[k]);
		}
		
		do_annot = ((k < GMT_TICK_UPPER && annotate) && !(T->unit == 'R' || T->unit == 'r'));		/* Cannot annotate a Gregorian week */
		if (do_annot) {	/* Then do annotations too - here just set text height/width parameters in PostScript */
		
			annot_pos = GMT_lower_axis_item(k);							/* 1 means lower annotation, 0 means upper (close to axis) */
			font_size = (annot_pos == 1) ? gmtdefs.annot_font2_size : gmtdefs.annot_font_size;		/* Set the size of the font to use */
			font = (annot_pos == 1) ? gmtdefs.annot_font2 : gmtdefs.annot_font;			/* Set the id of the font to use */
		
			for (i = 0; k < 4 && i < (nx - is_interval); i++) {
				if (GMT_annot_pos (val0, val1, T, &knots[i], &t_use)) continue;				/* Outside range */
				if (GMT_skip_second_annot (k, knots[i], knots_p, np, primary, secondary)) continue;	/* Secondary annotation skipped when coinciding with primary annotation */
				(axis == 0) ? GMT_coordinate_to_x (t_use, &x) : GMT_coordinate_to_y (t_use, &x);	/* Get annotation position */
				GMT_get_coordinate_label (string, &GMT_plot_calclock, format, T, knots[i]);		/* Get annotation string */
				ps_textdim ("PSL_dimx", "PSL_dimy", font_size, font, string, 0);				/* Get and set string dimensions in PostScript */
				sprintf (cmd, "PSL_dim%c PSL_AH%d gt {/PSL_AH%d PSL_dim%c def} if", xy[rot[annot_pos]], annot_pos, annot_pos, xy[rot[annot_pos]]);		/* Update the longest annotation */
				ps_command (cmd);
			}
		}
			
		if (nx) GMT_free ((void *)knots);
	}
	
	if (A->label[0] && annotate)
		ps_textdim ("PSL_dimx", "PSL_LH", gmtdefs.label_font_size, gmtdefs.label_font, "M", 0);				/* Get and set string dimensions in PostScript */
	else 
		ps_command ("/PSL_LH 0 def");

	/* Here, PSL_AH0, PSL_AH1, and PSL_LH have been defined.  We may now set the y offsets */
	
	ps_command ("/PSL_A0_y PSL_AO0 PSL_TL1 PSL_AO0 mul 0 gt {PSL_TL1 add} if PSL_AO0 0 gt {PSL_sign 0 lt {PSL_AH0} {0} ifelse add} if PSL_sign mul def");
	ps_command ("/PSL_A1_y PSL_A0_y abs PSL_AO1 add PSL_sign 0 lt {PSL_AH1} {PSL_AH0} ifelse add PSL_sign mul def");
	ps_command ("/PSL_L_y PSL_A1_y abs PSL_LO add PSL_sign 0 lt {PSL_LH} {PSL_AH1} ifelse add PSL_sign mul def");
	if (axis == 0 && !below) ps_command ("/PSL_H_y PSL_L_y PSL_LH add PSL_HO add def");	/* For title adjustment */
	
	/* Now do annotations, if requested */
	
	for (k = 0; annotate && k < GMT_TICK_UPPER; k++) {	/* For each one of the 5 axis items (gridlines are done separately) */
		
		T = &A->item[k];					/* Get pointer to this item */
		if (!T->active) continue;				/* Don't want this item plotted - goto next item */
		if (T->unit == 'R' || T->unit == 'r') continue;		/* Cannot annotate a Gregorian week */
		
		is_interval = GMT_interval_axis_item(k);			/* Interval or tick mark annotation? */
		nx = GMT_coordinate_array (val0, val1, &A->item[k], &knots);	/* Get all the annotation tick knots */
		
		annot_pos = GMT_lower_axis_item(k);							/* 1 means lower annotation, 0 means upper (close to axis) */
		font_size = (annot_pos == 1) ? gmtdefs.annot_font2_size : gmtdefs.annot_font_size;		/* Set the id of the font to use */
		font = (annot_pos == 1) ? gmtdefs.annot_font2 : gmtdefs.annot_font;			/* Set the id of the font to use */
		
		for (i = 0; k < 4 && i < (nx - is_interval); i++) {
			if (GMT_annot_pos (val0, val1, T, &knots[i], &t_use)) continue;				/* Outside range */
			if (GMT_skip_second_annot (k, knots[i], knots_p, np, primary, secondary)) continue;	/* Secondary annotation skipped when coinciding with primary annotation */
			(axis == 0) ? GMT_coordinate_to_x (t_use, &x) : GMT_coordinate_to_y (t_use, &x);	/* Get annotation position */
			GMT_get_coordinate_label (string, &GMT_plot_calclock, format, T, knots[i]);		/* Get annotation string */
			ps_set_length ("PSL_x", x);
			ps_textdim ("PSL_dimx", "PSL_dimy", font_size, font, string, 0);				/* Get and set string dimensions in PostScript */
			if (rot[annot_pos]) {	/* Rotate and adjust annotation in y direction */
				sprintf (cmd, "/PSL_y_off PSL_dimy 0.5 mul neg def");
				ps_command (cmd);
				sprintf (cmd, "PSL_x PSL_A%d_y M", annot_pos);					/* Move to new anchor point */
				ps_command (cmd);
				ps_text (0.0, 0.0, -font_size, string, -90.0, 7, 0);
			}
			else {			/* Just center horizontally */
				sprintf (cmd, "PSL_x PSL_A%d_y M", annot_pos);					/* Move to new anchor point */
				ps_command (cmd);
				ps_text (0.0, 0.0, -font_size, string, 0.0, 2, 0);
			}
		}
			
		if (nx) GMT_free ((void *)knots);
	}
	if (np) GMT_free ((void *)knots_p);
	
	/* Finally do axis label */
	
	if (A->label[0] && annotate) {
		ps_set_length ("PSL_x", 0.5 * length);
		ps_textdim ("PSL_dimx", "PSL_dimy", gmtdefs.label_font_size, gmtdefs.label_font, A->label, 0);				/* Get and set string dimensions in PostScript */
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

double GMT_set_label_offsets (int axis, double val0, double val1, struct PLOT_AXIS *A, int below, double annot_off[], double *label_off, int *annot_justify, int *label_justify, char *format)
{
	/* Determines what the offsets will be for annotations and labels */
	
	int ndec;
	BOOLEAN as_is, flip, both;
	double v0, v1, tmp_offset, off, angle, sign, len;
	char text_l[256], text_u[256];
	struct PLOT_AXIS_ITEM *T;	/* Pointer to the current axis item */
	
	both = GMT_upper_and_lower_items(axis);							/* Two levels of annotations? */
	sign = ((below && axis == 0) || (!below && axis == 1)) ? -1.0 : 1.0;			/* since annotations go either below or above */
	len = (gmtdefs.tick_length > 0.0) ? gmtdefs.tick_length : 0.0;
	if (axis == 0) {
		if (A->type != TIME) GMT_get_format (GMT_get_map_interval (axis, GMT_ANNOT_UPPER), A->unit, format);	/* Set the annotation format template */
		annot_off[0] = GMT_get_annot_offset (&flip);										/* Set upper annotation offset and flip depending on annot_offset */
		annot_off[1] = annot_off[0] + (gmtdefs.annot_font_size * GMT_u2u[GMT_PT][GMT_INCH]) + 0.5 * fabs (gmtdefs.annot_offset);	/* Lower annotation offset */
		if (both)	/* Must move label farther from axis given both annotation levels */
			*label_off = sign * (((flip) ? len : fabs (annot_off[1]) + (gmtdefs.annot_font2_size * GMT_u2u[GMT_PT][GMT_INCH]) * GMT_font[gmtdefs.annot_font2].height) + 1.5 * fabs (gmtdefs.annot_offset));
		else		/* Just one level of annotation to clear */
			*label_off = sign * (((flip) ? len : fabs (annot_off[0]) + (gmtdefs.annot_font_size * GMT_u2u[GMT_PT][GMT_INCH]) * GMT_font[gmtdefs.annot_font].height) + 1.5 * fabs (gmtdefs.annot_offset));
		annot_off[0] *= sign;		/* Change sign according to which axis we are doing */
		annot_off[1] *= sign;
		annot_justify[0] = annot_justify[1] = *label_justify = (below) ? 10 : 2;				/* Justification of annotation and label strings */
		if (flip) annot_justify[0] = GMT_flip_justify (annot_justify[0]);					/* flip is TRUE so flip the justification */
		angle = 0.0;
	}
	else {
		ndec = GMT_get_format (GMT_get_map_interval (axis, GMT_ANNOT_UPPER), A->unit, format);
		as_is = (ndec == 0 && !strchr (format, 'g'));	/* Use the d_format as is */
	
		switch (project_info.xyz_projection[axis]) {
			case POW:
				if (as_is) {
					sprintf (text_l, format, fabs (val0));
					sprintf (text_u, format, fabs (val1));
				}
				else {
					sprintf (text_l, "%d", (int)floor (val0));
					sprintf (text_u, "%d", (int)ceil (val1));
				}
				break;
			case LOG10:
				v0 = d_log10 (val0);
				v1 = d_log10 (val1);
				if (A->type == 2) {	/* 10 ^ pow annotations */
					sprintf (text_l, "10%d", (int)floor (v0));
					sprintf (text_u, "10%d", (int)ceil (v1));
				}
				else {
					if (as_is) {
						sprintf (text_l, format, fabs (val0));
						sprintf (text_u, format, fabs (val1));
					}
					else if (A->type == 1) {
						sprintf (text_l, "%d", (int)floor (v0));
						sprintf (text_u, "%d", (int)ceil (v1));
					}
					else {
						sprintf (text_l, format, val0);
						sprintf (text_u, format, val1);
					}
				}
				break;
			case LINEAR:
				if (as_is) {
					sprintf (text_l, format, fabs (val0));
					sprintf (text_u, format, fabs (val1));
				}
				else {
					sprintf (text_l, "%d", (int)floor (fabs (val0)));
					sprintf (text_u, "%d", (int)ceil (fabs (val1)));
				}
				break;
			case TIME:
				T = (A->item[GMT_ANNOT_UPPER].active) ? &A->item[GMT_ANNOT_UPPER] : &A->item[GMT_INTV_UPPER];
				GMT_get_coordinate_label (text_l, &GMT_plot_calclock, format, T, val0);		/* Get time annotation string */
				GMT_get_coordinate_label (text_u, &GMT_plot_calclock, format, T, val1);		/* Get time annotation string */
				break;
		}
	
		/* Find offset based on no of digits before and after a period, if any */
	
		off = ((MAX ((int)strlen (text_l), (int)strlen (text_u)) + ndec) * 0.49 + ((ndec > 0) ? 0.3 : 0.0) + ((val0 < 0.0) ? 0.3 : 0.0))
			* gmtdefs.annot_font_size * GMT_u2u[GMT_PT][GMT_INCH];
		
		tmp_offset = GMT_get_annot_offset (&flip);
		if (A->unit && A->unit[0] && gmtdefs.y_axis_type == 0) {	/* Accommodate extra width of annotation */
			int i, u_len, n_comp, len;
			i = u_len = n_comp = 0;
			len = strlen (A->unit);
			if (A->unit[0] == '-') i++;	/* Leading - to mean no-space */
			while (i < len) {
				if (A->unit[i] == '@' &&  A->unit[i+1]) {	/* escape sequences */
					i++;
					switch (A->unit[i]) {
						case '@':	/* Print the @ sign */
							u_len++;
							break;
						case '~':	/* Toggle symbol */
						case '+':	/* Toggle superscript */
						case '-':	/* Toggle subscript */
						case '#':	/* Toggle small caps */
							break;
						case '%':	/* Set font */
							i++;
							while (A->unit[i] && A->unit[i] != '%') i++;	/* Skip font number and trailing % */
						case '!':	/* Composite character */
							n_comp++;
							break;
						default:
							break;
					}
				}
				else if (A->unit[i] == '\\' && (len - i) > 3 && isdigit (A->unit[i+1]) && isdigit (A->unit[i+2]) && isdigit (A->unit[i+3])) {	/* Octal code */
					i += 3;
					u_len++;
				}
				else if (A->unit[i] == '\\') {	/* Escaped character */
					i++;
					u_len++;
				}
				else	/* Regular char */
					u_len++;
				i++;
			}
			off += (u_len - n_comp) * 0.49 * gmtdefs.annot_font_size * GMT_u2u[GMT_PT][GMT_INCH];
		}
		*label_justify = (below) ? 2 : 10;
		if (gmtdefs.y_axis_type == 0) {	/* Horizontal annotations */
			annot_justify[0] = 7;
			annot_off[0] = sign * tmp_offset;
			if (A->item[GMT_ANNOT_LOWER].active)
				annot_off[1] = sign * (((flip) ? len : fabs (tmp_offset)) + 1.5 * fabs (gmtdefs.annot_offset));
			else
				annot_off[1] = sign * (((flip) ? len : fabs (tmp_offset + off)) + 1.5 * fabs (gmtdefs.annot_offset));
			if ((below + flip) != 1) annot_off[0] -= off;
			angle = -90.0;
		}
		else {
			annot_off[0] = sign * tmp_offset;
			annot_off[1] = sign * (((flip) ? len : fabs (tmp_offset) + (gmtdefs.annot_font_size * GMT_u2u[GMT_PT][GMT_INCH]) * GMT_font[gmtdefs.annot_font].height) + 1.5 * fabs (gmtdefs.annot_offset));
			annot_justify[0] = (below) ? 2 : 10;
			angle = 0.0;
			if (flip) annot_justify[0] = GMT_flip_justify (annot_justify[0]);
		}
		if (both)	/* Must move label farther from axis given both annotation levels */
			*label_off = sign * (((flip) ? len : fabs (annot_off[1]) + (gmtdefs.annot_font2_size * GMT_u2u[GMT_PT][GMT_INCH]) * GMT_font[gmtdefs.annot_font2].height) + 1.5 * fabs (gmtdefs.annot_offset));
		else		/* Just one level of annotation to clear */
			*label_off = sign * (((flip) ? len : fabs (annot_off[0]) + (gmtdefs.annot_font_size * GMT_u2u[GMT_PT][GMT_INCH]) * GMT_font[gmtdefs.annot_font].height) + 1.5 * fabs (gmtdefs.annot_offset));
		if (A->item[GMT_ANNOT_LOWER].active && gmtdefs.y_axis_type == 0) *label_off += off;
		annot_justify[1] = (below) ? 2 : 10;
		if (A->item[GMT_ANNOT_LOWER].active) annot_justify[1] = annot_justify[0];
	}
	return (angle);
}

int GMT_linear_array (double min, double max, double delta, double **array)
{
	double first, small, *val;
	int i, n;

	if (delta == 0.0) return (0);
	small = SMALL * delta;
	first = floor (min / delta) * delta;
	if ((min - first) > small) first += delta;
	if (first > max) return (0);

	n = irint ((max - first) / delta) + 1;
	val = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "GMT_linear_array");
	for (i = 0; i < n; i++) val[i] = first + i * delta;
	while (n && val[n-1] > max) n--;	/* In case of over-run */

	*array = val;

	return (n);
}

int GMT_log_array (double min, double max, double delta, double **array)
{
	int i, n, nticks, test, n_alloc = GMT_SMALL_CHUNK;
	double *val, v0, end_val, start_log, tvals[9];

	if (delta == 0.0) return (0);
	val = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_log_array");

	test = irint (fabs (delta)) - 1;
	if (test < 0 || test > 2) test = 0;
	if (test == 0) {
		tvals[0] = 1.0;
		nticks = 1;
	}
	if (test == 1) {
		tvals[0] = 1.0;
		tvals[1] = 2.0;
		tvals[2] = 5.0;
		nticks = 3;
	}
	else if (test == 2) {
		nticks = 9;
		for (i = 0; i < nticks; i++) tvals[i] = i + 1;
	}
	
	v0 = d_log10 (min);
	start_log = val[0] = pow (10.0, floor (v0));
	n = 0;
	i = 1;	/* Because val[0] is initially set to be a power or ten, hence * tvals[0], so next should be 1 */
	while ((v0 - d_log10 (val[n])) > SMALL) {
		if (i < nticks)
			val[n] = start_log * tvals[i];
		else {
			val[n] = (start_log *= 10.0);
			i = 0;
		}
		i++;
	}
	i--;
	end_val = max;
	
	while ((end_val - val[n]) > GMT_CONV_LIMIT) {
		i++;
		n++;
		if (n == n_alloc) {
			n_alloc += GMT_SMALL_CHUNK;
			val = (double *) GMT_memory ((void *)val, (size_t)n_alloc, sizeof (double), "GMT_log_array");
		}
			
		if (i < nticks) 
			val[n] = start_log * tvals[i];
		else {
			start_log *= 10;
			val[n] = start_log;
			i = 0;
		}
	}
	while (n && val[n] > end_val) n--;	/* In case of over-run */
	n++;
	val = (double *) GMT_memory ((void *)val, (size_t)n, sizeof (double), "GMT_log_array");

	*array = val;

	return (n);
}

int GMT_pow_array (double min, double max, double delta, int x_or_y, double **array)
{
	int annottype, n, n_alloc = GMT_SMALL_CHUNK;
	double *val, tval, v0, v1, small, start_val, end_val;
	PFI fwd, inv;
	
	if (delta == 0.0) return (0);
	val = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_pow_array");

	annottype = (frame_info.axis[x_or_y].type == 2) ? 2 : 0;
	if (x_or_y == 0) { /* x-axis */
		fwd = GMT_x_forward;
		inv = GMT_x_inverse;
	}
	else {	/* y-axis */
		fwd = GMT_y_forward;
		inv = GMT_y_inverse;
	}

	small = SMALL * delta;
	if (annottype == 2) {
		(*fwd) (min, &v0);
		(*fwd) (max, &v1);

		tval = (delta == 0.0) ? 0.0 : floor (v0 / delta) * delta;
		if (fabs (tval - v0) > small) tval += delta;
		start_val = tval;
		tval = (delta == 0.0) ? 0.0 : ceil (v1 / delta) * delta;
		if (fabs (tval - v1) > small) tval -= delta;
		end_val = tval;
	}
	else {
		tval = (delta == 0.0) ? 0.0 : floor (min / delta) * delta;
		if (fabs (tval - min) > small) tval += delta;
		start_val = tval;
		tval = (delta == 0.0) ? 0.0 : ceil (max / delta) * delta;
		if (fabs (tval - max) > small) tval -= delta;
		end_val = tval;
	}
 
	tval = start_val;
	n = 0;
	while (tval <= end_val) {
		if (annottype == 2) {
			(*inv) (&val[n], tval);
		}
		else {
			val[n] = tval;
		}
		tval += delta;
		n++;
		if (n == n_alloc) {
			n_alloc += GMT_SMALL_CHUNK;
			val = (double *) GMT_memory ((void *)val, (size_t)n_alloc, sizeof (double), "GMT_pow_array");
		}
	}
	while (n && val[n-1] > end_val) n--;	/* In case of over-run */

	val = (double *) GMT_memory ((void *)val, (size_t)n, sizeof (double), "GMT_log_array");

	*array = val;

	return (n);
}

int GMT_time_array (double min, double max, struct PLOT_AXIS_ITEM *T, double **array)
{	/* When interval is TRUE we must return interval start/stop even if outside min/max range */
	struct GMT_MOMENT_INTERVAL I;
	double *val;
	int n_alloc = GMT_SMALL_CHUNK, n = 0;
	BOOLEAN interval;

	if (T->interval == 0.0) return (0);
	val = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_time_array");
	I.unit = T->unit;
	I.step = T->interval;
	interval = (T->id == 2 || T->id == 3);	/* Only for I/i axis items */
	GMT_moment_interval (&I, min, TRUE);	/* First time we pass TRUE for initialization */
	while (I.dt[0] <= max) {		/* As long as we are not gone way past the end time */
		if (I.dt[0] >= min || interval) val[n++] = I.dt[0];		/* Was inside region */
		GMT_moment_interval (&I, 0.0, FALSE);			/* Advance to next interval */
		if (n == n_alloc) {					/* Allocate more space */
			n_alloc += GMT_SMALL_CHUNK;
			val = (double *) GMT_memory ((void *)val, (size_t)n_alloc, sizeof (double), "GMT_time_array");
		}
	}
	if (interval) val[n++] = I.dt[0];	/* Must get end of interval too */
	val = (double *) GMT_memory ((void *)val, (size_t)n, sizeof (double), "GMT_time_array");

	*array = val;

	return (n);
}

int GMT_coordinate_array (double min, double max, struct PLOT_AXIS_ITEM *T, double **array)
{
	int n;
	switch (project_info.xyz_projection[T->parent]) {
		case LINEAR:
			n = GMT_linear_array (min, max, GMT_get_map_interval (T->parent, T->id), array);
			break;
		case LOG10:
			n = GMT_log_array (min, max, GMT_get_map_interval (T->parent, T->id), array);
			break;
		case POW:
			n = GMT_pow_array (min, max, GMT_get_map_interval (T->parent, T->id), T->parent, array);
			break;
		case TIME:
			n = GMT_time_array (min, max, T, array);
			break;
		default:
			fprintf (stderr, "GMT ERROR: Invalid projection type (%d) passed to GMT_coordinate_array!\n", project_info.xyz_projection[T->parent]);
			exit (EXIT_FAILURE);
			break;
	}
	return (n);
}

void GMT_linearx_grid (double w, double e, double s, double n, double dval)
{
	double *x;
	int i, nx;

	nx = GMT_linear_array (w, e, dval, &x);
	for (i = 0; i < nx; i++) GMT_map_lonline (x[i], s, n);
	if (nx) GMT_free ((char *)x);
}

void GMT_lineary_grid (double w, double e, double s, double n, double dval)
{
	double *y;
	int i, ny;

	ny = GMT_linear_array (s, n, dval, &y);
	for (i = 0; i < ny; i++) GMT_map_latline (y[i], w, e);
	if (ny) GMT_free ((char *)y);
}

void GMT_timex_grid (double w, double e, double s, double n)
{
	int i, nx;
	double *x;
		
	nx = GMT_time_array (w, e, &frame_info.axis[0].item[5], &x);
	for (i = 0; i < nx; i++) {
		GMT_geoplot (x[i], s, 3);
		GMT_geoplot (x[i], n, 2);
	}
	if (nx) GMT_free ((char *)x);
}

void GMT_timey_grid (double w, double e, double s, double n)
{
	int i, ny;
	double *y;
		
	ny = GMT_time_array (s, n, &frame_info.axis[1].item[5], &y);
	for (i = 0; i < ny; i++) {
		GMT_geoplot (w, y[i], 3);
		GMT_geoplot (e, y[i], 2);
	}
	if (ny) GMT_free ((char *)y);
}

void GMT_logx_grid (double w, double e, double s, double n, double dval)
{
	int i, nx;
	double *x;
		
	nx = GMT_log_array (w, e, dval, &x);
	for (i = 0; i < nx; i++) {
		GMT_geoplot (x[i], s, 3);
		GMT_geoplot (x[i], n, 2);
	}
	if (nx) GMT_free ((char *)x);
}

void GMT_logy_grid (double w, double e, double s, double n, double dval)
{
	int i, ny;
	double *y;
		
	ny = GMT_log_array (s, n, dval, &y);
	for (i = 0; i < ny; i++) {
		GMT_geoplot (w, y[i], 3);
		GMT_geoplot (e, y[i], 2);
	}
	if (ny) GMT_free ((char *)y);
}

void GMT_powx_grid (double w, double e, double s, double n, double dval)
{
	int i, nx;
	double *x;
		
	nx = GMT_pow_array (w, e, dval, 0, &x);
	for (i = 0; i < nx; i++) {
		GMT_geoplot (x[i], s, 3);
		GMT_geoplot (x[i], n, 2);
	}
	if (nx) GMT_free ((char *)x);
}

void GMT_powy_grid (double w, double e, double s, double n, double dval)
{
	int i, ny;
	double *y;
		
	ny = GMT_pow_array (s, n, dval, 1, &y);
	for (i = 0; i < ny; i++) {
		GMT_geoplot (w, y[i], 3);
		GMT_geoplot (e, y[i], 2);
	}
	if (ny) GMT_free ((char *)y);
}

/*	FANCY RECTANGULAR PROJECTION MAP BOUNDARY	*/

void GMT_fancy_map_boundary (double w, double e, double s, double n)
{
	double x1, x2, x3, y1, y2, y3, s1, w1, val, v1, v2, dx, dy, sign_x, sign_y;
	int shade, i, nx, ny, fat_pen, thin_pen;
	
	if (gmtdefs.basemap_type == GMT_IS_PLAIN) {	/* Draw plain boundary and return */
		GMT_wesn_map_boundary (w, e, s, n);
		return;
	}
	
	ps_setpaint (gmtdefs.basemap_frame_rgb);

	fat_pen = irint (gmtdefs.frame_width * gmtdefs.dpi);
	thin_pen = irint (0.1 * gmtdefs.frame_width * gmtdefs.dpi);
	sign_x = (project_info.xyz_pos[0]) ? +1.0 : -1.0;
	sign_y = (project_info.xyz_pos[1]) ? +1.0 : -1.0;
	
	ps_setline (thin_pen);
	if (frame_info.side[3]) {	/* Draw western boundary */
		GMT_geo_to_xy (w, s, &x1, &y1);
		y1 -= (sign_y * gmtdefs.frame_width);
		GMT_geo_to_xy (w, n, &x2, &y2);
		y2 += (sign_y * gmtdefs.frame_width);
		ps_plot (x1, y1, 3);
		ps_plot (x1, y2, -2);
		x1 -= (sign_x * gmtdefs.frame_width);
		ps_plot (x1, y1, 3);
		ps_plot (x1, y2, -2);
	}
	if (frame_info.side[1]) {	/* Draw eastern boundary */
		GMT_geo_to_xy (e, s, &x2, &y1);
		y1 -= (sign_y * gmtdefs.frame_width);
		GMT_geo_to_xy (e, n, &x1, &y2);
		y2 += (sign_y * gmtdefs.frame_width);
		ps_plot (x2, y1, 3);
		ps_plot (x2, y2, -2);
		x2 += (sign_x * gmtdefs.frame_width);
		ps_plot (x2, y1, 3);
		ps_plot (x2, y2, -2);
	}
	if (frame_info.side[0]) {	/* Draw southern boundary */
		GMT_geo_to_xy (w, s, &x1, &y1);
		x1 -= (sign_x * gmtdefs.frame_width);
		GMT_geo_to_xy (e, s, &x2, &y2);
		x2 += (sign_x * gmtdefs.frame_width);
		ps_plot (x1, y1, 3);
		ps_plot (x2, y1, -2);
		y1 -= (sign_y * gmtdefs.frame_width);
		ps_plot (x1, y1, 3);
		ps_plot (x2, y1, -2);
	}
	if (frame_info.side[2]) {	/* Draw northern boundary */
		GMT_geo_to_xy (w, n, &x1, &y1);
		x1 -= (sign_x * gmtdefs.frame_width);
		GMT_geo_to_xy (e, n, &x2, &y2);
		x2 += (sign_x * gmtdefs.frame_width);
		ps_plot (x1, y2, 3);
		ps_plot (x2, y2, -2);
		y2 += (sign_y * gmtdefs.frame_width);
		ps_plot (x1, y2, 3);
		ps_plot (x2, y2, -2);
	}
	
	/* Draw frame grid for W/E boundaries */
	
	ps_setline(fat_pen);
	if ((dy = GMT_get_map_interval (1, GMT_TICK_UPPER)) != 0.0) {
		sign_x *= 0.5;
		shade = ((int)floor (s / dy) + 1) % 2;
		s1 = floor (s / dy) * dy;
		ny = (s1 > n) ? -1 : (int)((n - s1) / dy + SMALL);
		for (i = 0; i <= ny; i++) {
			val = s1 + i * dy;
			v1 = (val < s) ? s : val;
			GMT_geo_to_xy (w, v1, &x1, &y1);
			GMT_geo_to_xy (e, v1, &x2, &y2);
			if (shade) {
				v2 = val + dy;
				if (v2 > n) v2 = n;
				if (frame_info.side[3]) {
					GMT_geo_to_xy (w, v2, &x3, &y3);
					ps_plot (x1-sign_x*gmtdefs.frame_width, y1, 3);
					ps_plot (x3-sign_x*gmtdefs.frame_width, y3, -2);
				}
				if (frame_info.side[1]) {
					GMT_geo_to_xy (e, v2, &x3, &y3);
					ps_plot (x2+sign_x*gmtdefs.frame_width, y2, 3);
					ps_plot (x3+sign_x*gmtdefs.frame_width, y3, -2);
				}
				shade = FALSE;
			}
			else
				shade = TRUE;
		}
	}
	
	/* Draw Frame grid for N and S boundaries */
	
	if ((dx = GMT_get_map_interval (0, GMT_TICK_UPPER)) != 0.0) {
		sign_y *= 0.5;
		shade = ((int)floor (w / dx) + 1) % 2;
		w1 = floor (w / dx) * dx;
		nx = (w1 > e) ? -1 : (int)((e - w1) / dx + SMALL);
		for (i = 0; i <= nx; i++) {
			val = w1 + i * dx;
			v1 = (val < w) ? w : val;
			GMT_geo_to_xy (v1, s, &x1, &y1);
			GMT_geo_to_xy (v1, n, &x2, &y2);
			if (shade) {
				v2 = val + dx;
				if (v2 > e) v2 = e;
				if (frame_info.side[0]) {
					GMT_geo_to_xy (v2, s, &x3, &y3);
					ps_plot (x1, y1-sign_y*gmtdefs.frame_width, 3);
					ps_plot (x3, y3-sign_y*gmtdefs.frame_width, -2);
				}
				if (frame_info.side[2]) {
					GMT_geo_to_xy (v2, n, &x3, &y3);
					ps_plot (x2, y2+sign_y*gmtdefs.frame_width, 3);
					ps_plot (x3, y3+sign_y*gmtdefs.frame_width, -2);
				}
				shade = FALSE;
			}
			else
				shade = TRUE;
		}
	}
	ps_setline (thin_pen);
}

/*	POLAR (S or N) PROJECTION MAP BOUNDARY	*/

void GMT_polar_map_boundary (double w, double e, double s, double n)
{
	int i, nx, ny, shade, thin_pen, fat_pen;
	double anglew, dx2w, dy2w, anglee, dx2e, dy2e;
	double y0, x0, radiuss, radiusn, da, da0, az1, az2, psize, dx, dy;
	double x1, x2, x3, y1, y2, y3, s1, w1, val, v1, v2, dummy, r2, dr;
	
	if (!project_info.region) { /* Draw rectangular boundary and return */
		GMT_rect_map_boundary (0.0, 0.0, project_info.xmax, project_info.ymax);
		return;
	}
	
	if (!project_info.north_pole && s <= -90.0) /* Cannot have southern boundary */
		frame_info.side[0] = FALSE;
	if (project_info.north_pole && n >= 90.0) /* Cannot have northern boundary */
		frame_info.side[2] = FALSE;
	if (fabs (fabs (e-w) - 360.0) < GMT_CONV_LIMIT || fabs (e - w) < GMT_CONV_LIMIT) {
		frame_info.side[1] = FALSE;
		frame_info.side[3] = FALSE;
	}
	
	if (gmtdefs.basemap_type == GMT_IS_PLAIN) { /* Draw plain boundary and return */
		GMT_wesn_map_boundary (w, e, s, n);
		return;
	}
	
	/* Here draw fancy map boundary */
	
	ps_setpaint (gmtdefs.basemap_frame_rgb);

	fat_pen = irint (gmtdefs.frame_width * gmtdefs.dpi);
	thin_pen = irint (0.1 * gmtdefs.frame_width * gmtdefs.dpi);
	ps_setline (thin_pen);
	
	psize = gmtdefs.frame_width;
	
	/* Angle of western boundary:  */
	
	GMT_geo_to_xy (w, n, &x1, &y1);
	GMT_geo_to_xy (w, s, &x2, &y2);
	anglew = d_atan2 (y1 - y2, x1 - x2);
	dx2w = -psize * sin (anglew);
	dy2w = psize * cos (anglew);
	
	/* Angle of eastern boundary:  */
	
	GMT_geo_to_xy (e, n, &x1, &y1);
	GMT_geo_to_xy (e, s, &x2, &y2);
	anglee = d_atan2 (y1 - y2, x1 - x2);
	dx2e = -psize * cos (anglee);
	dy2e = psize * sin (anglee);
	
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &x0, &y0);
	GMT_geo_to_xy (project_info.central_meridian, s, &dummy, &y1);
	GMT_geo_to_xy (project_info.central_meridian, n, &dummy, &y2);
	radiuss = fabs(y1 - y0);
	radiusn = fabs(y2 - y0);
	dr = 0.5 * psize;
	
	if (frame_info.side[3]) {	/* Draw western boundary */
		GMT_geo_to_xy (w, n, &x1, &y1);
		GMT_geo_to_xy (w, s, &x2, &y2);
		ps_plot (x1+dy2w, y1-dx2w, 3);
		ps_plot (x2-dy2w, y2+dx2w, -2);
		x1 += dx2w;
		y1 += dy2w;
		x2 += dx2w;
		y2 += dy2w;
		ps_plot (x1+dy2w, y1-dx2w, 3);
		ps_plot (x2-dy2w, y2+dx2w, -2);
	}
	if (frame_info.side[1]) {	/* Draw eastern boundary */
		GMT_geo_to_xy (e, n, &x1, &y1);
		GMT_geo_to_xy (e, s, &x2, &y2);
		ps_plot (x1-dx2e, y1+dy2e, 3);
		ps_plot (x2+dx2e, y2-dy2e, -2);
		x1 += dy2e;
		y1 += dx2e;
		x2 += dy2e;
		y2 += dx2e;
		ps_plot (x1-dx2e, y1+dy2e, 3);
		ps_plot (x2+dx2e, y2-dy2e, -2);
	}
	if (frame_info.side[0]) {	/* Draw southern boundary */
		da0 = R2D * psize /radiuss;
		GMT_geo_to_xy (e, s, &x1, &y1);
		GMT_geo_to_xy (w, s, &x2, &y2);
		az1 = d_atan2 (y1 - y0, x1 - x0) * R2D;
		az2 = d_atan2 (y2 - y0, x2 - x0) * R2D;
		if (project_info.north_pole) {
			r2 = radiuss + psize;
			da = R2D * psize / r2;
			if (az1 <= az2) az1 += 360.0;
			ps_arc (x0, y0, radiuss, az2-da0, az1+da0, 3);
			ps_arc (x0, y0, r2, az2-da, az1+da, 3);
		}
		else {
			r2 = radiuss - psize;
			da = R2D * psize / r2;
			if (az2 <= az1) az2 += 360.0;
			ps_arc (x0, y0, radiuss, az1-da0, az2+da0, 3);
			ps_arc (x0, y0, r2, az1-da, az2+da, 3);
		}
	}
	if (frame_info.side[2]) {	/* Draw northern boundary */
		da0 = R2D * psize / radiusn;
		GMT_geo_to_xy (e, n, &x1, &y1);
		GMT_geo_to_xy (w, n, &x2, &y2);
		az1 = d_atan2 (y1 - y0, x1 - x0) * R2D;
		az2 = d_atan2 (y2 - y0, x2 - x0) * R2D;
		if (project_info.north_pole) {
			r2 = radiusn - psize;
			da = R2D * psize / r2;
			if (az1 <= az2) az1 += 360.0;
			ps_arc (x0, y0, radiusn, az2-da0, az1+da0, 3);
			ps_arc (x0, y0, r2, az2-da, az1+da, 3);
		}
		else {
			r2 = radiusn + psize;
			da = R2D * psize / r2;
			if (az2 <= az1) az2 += 360.0;
			ps_arc (x0, y0, radiusn, az1-da0, az2+da0, 3);
			ps_arc (x0, y0, r2, az1-da, az2+da, 3);
		}
	}
	
	/* Annotate S-N axes */
	
	ps_setline (fat_pen);
	if ((dy = GMT_get_map_interval (1, GMT_TICK_UPPER)) != 0.0) {
		shade = ((int)floor (s / dy) + 1) % 2;
		s1 = floor(s/dy) * dy;
		ny = (s1 > n) ? -1 : (int)((n-s1) / dy + SMALL);
		for (i = 0; i <= ny; i++) {
			val = s1 + i * dy;
			v1 = (val < s) ? s : val;
			GMT_geo_to_xy (w, v1, &x1, &y1);
			GMT_geo_to_xy (e, v1, &x2, &y2);
			if (shade) {
				v2 = val + dy;
				if (v2 > n) v2 = n;
				if (frame_info.side[3]) {
					GMT_geo_to_xy (w, v2, &x3, &y3);
					ps_plot (x1+0.5*dx2w, y1+0.5*dy2w, 3);
					ps_plot (x3+0.5*dx2w, y3+0.5*dy2w, -2);
				}
				if (frame_info.side[1]) {
					GMT_geo_to_xy (e, v2, &x3, &y3);
					ps_plot (x2+0.5*dy2e, y2+0.5*dx2e, 3);
					ps_plot (x3+0.5*dy2e, y3+0.5*dx2e, -2);
				}
				shade = FALSE;
			}
			else
				shade = TRUE;
		}
	}

	/* Annotate W-E axes */
	
	if ((dx = GMT_get_map_interval (0, GMT_TICK_UPPER)) != 0.0) {
		shade = ((int)floor (w / dx) + 1) % 2;
		w1 = floor(w/dx) * dx;
		nx = (w1 > e) ? -1 : (int)((e-w1) / dx + SMALL);
		for (i = 0; i <= nx; i++) {
			val = w1 + i * dx;
			v1 = (val < w) ? w : val;
			if (shade) {
				v2 = val + dx;
				if (v2 > e) v2 = e;
				if (frame_info.side[0]) {
					GMT_geo_to_xy (v2, s, &x1, &y1);
					GMT_geo_to_xy (v1, s, &x2, &y2);
					az1 = d_atan2 (y1 - y0, x1 - x0) * R2D;
					az2 = d_atan2 (y2 - y0, x2 - x0) * R2D;
					if (project_info.north_pole) {
						if (az1 < az2) az1 += 360.0;
						ps_arc (x0, y0, radiuss+dr, az2, az1, 3);
					}
					else {
						if (az2 < az1) az2 += 360.0;
						ps_arc (x0, y0, radiuss-dr, az1, az2, 3);
					}
				}
				if (frame_info.side[2]) {
					GMT_geo_to_xy (v2, n, &x1, &y1);
					GMT_geo_to_xy (v1, n, &x2, &y2);
					az1 = d_atan2 (y1 - y0, x1 - x0) * R2D;
					az2 = d_atan2 (y2 - y0, x2 - x0) * R2D;
					if (project_info.north_pole) {
						if (az1 < az2) az1 += 360.0;
						ps_arc (x0, y0, radiusn-dr, az2, az1, 3);
					}
					else {
						if (az2 < az1) az2 += 360.0;
						ps_arc (x0, y0, radiusn+dr, az1, az2, 3);
					}
				}
				shade = FALSE;
			}
			else
				shade = TRUE;
		}
	}
	ps_setline (thin_pen);
}

/*	CONIC PROJECTION MAP BOUNDARY	*/

void GMT_conic_map_boundary (double w, double e, double s, double n)
{
	int i, nx, ny, shade, fat_pen, thin_pen;
	double dx, dy, angle, dx2, dy2, y0, x0, radiuss, radiusn, dr, da, da0, az1, az2, psize;
	double x1, x2, x3, y1, y2, y3, s1, w1, val, v1, v2, rsize, x_inc, y_inc;
	
	if (!project_info.region) { /* Draw rectangular boundary and return */
		GMT_rect_map_boundary (0.0, 0.0, project_info.xmax, project_info.ymax);
		return;
	}
	if (gmtdefs.basemap_type == GMT_IS_PLAIN) { /* Draw plain boundary and return */
		GMT_wesn_map_boundary (w, e, s, n);
		return;
	}
	
	ps_setpaint (gmtdefs.basemap_frame_rgb);

	fat_pen = irint (gmtdefs.frame_width * gmtdefs.dpi);
	thin_pen = irint (0.1 * gmtdefs.frame_width * gmtdefs.dpi);
	ps_setline (thin_pen);
	
	psize = (project_info.north_pole) ? gmtdefs.frame_width : -gmtdefs.frame_width;
	rsize = fabs (psize);
	GMT_geo_to_xy (w, n, &x1, &y1);
	GMT_geo_to_xy (w, s, &x2, &y2);
	dx = x1 - x2;
	dy = y1 - y2;
	angle = R2D*d_atan2 (dy, dx) - 90.0;
	if (fabs(angle-180.0) < SMALL) angle = 0.0;
	dx2 = rsize * cos (angle*D2R);
	dy2 = rsize * sin (angle*D2R);
	GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &x0, &y0);
	GMT_geo_to_xy (w, project_info.pole, &x1, &y1);
	GMT_geo_to_xy (e, project_info.pole, &x2, &y2);
	dr = y1 - y0;
	if (fabs (dr) > SMALL) {
		az1 = 2.0 * d_atan2 (dr, x1 - x0);
		dr /= (1.0 - cos (az1));
		y0 += dr;
	}
	GMT_geo_to_xy (project_info.central_meridian, s, &x1, &y1);
	GMT_geo_to_xy (project_info.central_meridian, n, &x2, &y2);
	radiuss = hypot (x1 - x0, y1 - y0);
	radiusn = hypot (x2 - x0, y2 - y0);
	
	if (frame_info.side[3]) {	/* Draw western boundary */
		GMT_geo_to_xy (w, s, &x1, &y1);
		GMT_geo_to_xy (w, n, &x2, &y2);
		ps_plot (x1+dy2, y1-dx2, 3);
		ps_plot (x2-dy2, y2+dx2, -2);
		x1 -= dx2;
		y1 -= dy2;
		x2 -= dx2;
		y2 -= dy2;
		ps_plot (x1+dy2, y1-dx2, 3);
		ps_plot (x2-dy2, y2+dx2, -2);
	}
	if (frame_info.side[1]) {	/* Draw eastern boundary */
		GMT_geo_to_xy (e, s, &x1, &y1);
		GMT_geo_to_xy (e, n, &x2, &y2);
		ps_plot (x1-dy2, y1-dx2, 3);
		ps_plot (x2+dy2, y2+dx2, -2);
		x1 += dx2;
		y1 -= dy2;
		x2 += dx2;
		y2 -= dy2;
		ps_plot (x1-dy2, y1-dx2, 3);
		ps_plot (x2+dy2, y2+dx2, -2);
	}
	if (frame_info.side[0]) {	/* Draw southern boundary */
		da0 = R2D*gmtdefs.frame_width/radiuss;
		da = R2D*gmtdefs.frame_width/(radiuss+psize);
		GMT_geo_to_xy (e, s, &x1, &y1);
		GMT_geo_to_xy (w, s, &x2, &y2);
		az1 = d_atan2 (y1 - y0, x1 - x0) * R2D;
		az2 = d_atan2 (y2 - y0, x2 - x0) * R2D;
		if (project_info.north_pole) {
			if (az1 <= az2) az1 += 360.0;
			ps_arc (x0, y0, radiuss, az2-da0, az1+da0, 3);
			ps_arc (x0, y0, radiuss + psize, az2-da, az1+da, 3);
		}
		else {
			if (az2 <= az1) az2 += 360.0;
			ps_arc (x0, y0, radiuss, az1-da0, az2+da0, 3);
			ps_arc (x0, y0, radiuss + psize, az1-da, az2+da, 3);
		}
	}
	if (frame_info.side[2]) {	/* Draw northern boundary */
		da0 = R2D*gmtdefs.frame_width/radiusn;
		da = R2D*gmtdefs.frame_width/(radiusn-psize);
		GMT_geo_to_xy (e, s, &x1, &y1);
		GMT_geo_to_xy (w, s, &x2, &y2);
		az1 = d_atan2 (y1 - y0, x1 - x0) * R2D;
		az2 = d_atan2 (y2 - y0, x2 - x0) * R2D;
		if (project_info.north_pole) {
			if (az1 <= az2) az1 += 360.0;
			ps_arc (x0, y0, radiusn, az2-da0, az1+da0, 3);
			ps_arc (x0, y0, radiusn - psize, az2-da, az1+da, 3);
		}
		else {
			if (az2 <= az1) az2 += 360.0;
			ps_arc (x0, y0, radiusn, az1-da0, az2+da0, 3);
			ps_arc (x0, y0, radiusn - psize, az1-da, az2+da, 3);
		}
	}
	
	/* Frame tick S-N axes */
	
	ps_setline (fat_pen);
	if ((y_inc = GMT_get_map_interval (1, GMT_TICK_UPPER)) != 0.0) {
		shade = ((int)floor (s / y_inc) + 1) % 2;
		s1 = floor(s/y_inc) * y_inc;
		ny = (s1 > n) ? -1 : (int)((n-s1) / y_inc + SMALL);
		for (i = 0; i <= ny; i++) {
			val = s1 + i*y_inc;
			v1 = (val < s) ? s : val;
			GMT_geo_to_xy (w, v1, &x1, &y1);
			GMT_geo_to_xy (e, v1, &x2, &y2);
			if (shade) {
				v2 = val + y_inc;
				if (v2 > n) v2 = n;
				if (frame_info.side[3]) {
					GMT_geo_to_xy (w, v2, &x3, &y3);
					ps_plot (x1-0.5*dx2, y1-0.5*dy2, 3);
					ps_plot (x3-0.5*dx2, y3-0.5*dy2, -2);
				}
				if (frame_info.side[1]) {
					GMT_geo_to_xy (e, v2, &x3, &y3);
					ps_plot (x2+0.5*dx2, y2-0.5*dy2, 3);
					ps_plot (x3+0.5*dx2, y3-0.5*dy2, -2);
				}
				shade = FALSE;
			}
			else
				shade = TRUE;
		}
	}

	/* Frame tick W-E axes */
	
	if ((x_inc = GMT_get_map_interval (0, GMT_TICK_UPPER)) != 0.0) {
		shade = ((int)floor (w / x_inc) + 1) % 2;
		w1 = floor(w / x_inc) * x_inc;
		nx = (w1 > e) ? -1 : (int)((e-w1) / x_inc + SMALL);
		da = dx;
		dx = dy;
		dy = da;
		for (i = 0; i <= nx; i++) {
			val = w1 + i * x_inc;
			v1 = (val < w) ? w : val;
			if (shade) {
				v2 = val + x_inc;
				if (v2 > e) v2 = e;
				if (frame_info.side[0]) {
					GMT_geo_to_xy (v2, s, &x1, &y1);
					GMT_geo_to_xy (v1, s, &x2, &y2);
					az1 = d_atan2 (y1 - y0, x1 - x0) * R2D;
					az2 = d_atan2 (y2 - y0, x2 - x0) * R2D;
					if (project_info.north_pole) {
						if (az1 < az2) az1 += 360.0;
						ps_arc (x0, y0, radiuss+0.5*psize, az2, az1, 3);
					}
					else {
						if (az2 < az1) az2 += 360.0;
						ps_arc (x0, y0, radiuss+0.5*psize, az1, az2, 3);
					}
				}
				if (frame_info.side[2]) {
					GMT_geo_to_xy (v2, n, &x1, &y1);
					GMT_geo_to_xy (v1, n, &x2, &y2);
					az1 = d_atan2 (y1 - y0, x1 - x0) * R2D;
					az2 = d_atan2 (y2 - y0, x2 - x0) * R2D;
					if (project_info.north_pole) {
						if (az1 < az2) az1 += 360.0;
						ps_arc (x0, y0, radiusn-0.5*psize, az2, az1, 3);
					}
					else {
						if (az2 < az1) az2 += 360.0;
						ps_arc (x0, y0, radiusn-0.5*psize, az1, az2, 3);
					}
				}
				shade = FALSE;
			}
			else
				shade = TRUE;
		}
	}
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
	int i, np = 0;
	double *xx, *yy;
	
	GMT_setpen (&gmtdefs.frame_pen);
	
	if (frame_info.side[3]) {	/* West */
		np = GMT_map_path (w, s, w, n, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geoz_to_xy (xx[i], yy[i], project_info.z_level, &xx[i], &yy[i]);
		ps_line (xx, yy, np, 3, FALSE, TRUE);
		GMT_free ((void *)xx);	GMT_free ((void *)yy);
	}
	if (frame_info.side[1]) {	/* East */
		np = GMT_map_path (e, s, e, n, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geoz_to_xy (xx[i], yy[i], project_info.z_level, &xx[i], &yy[i]);
		ps_line (xx, yy, np, 3, FALSE, TRUE);
		GMT_free ((void *)xx);	GMT_free ((void *)yy);
	}
	if (frame_info.side[0]) {	/* South */
		np = GMT_map_path (w, s, e, s, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geoz_to_xy (xx[i], yy[i], project_info.z_level, &xx[i], &yy[i]);
		ps_line (xx, yy, np, 3, FALSE, TRUE);
		GMT_free ((void *)xx);	GMT_free ((void *)yy);
	}
	if (frame_info.side[2]) {	/* North */
		np = GMT_map_path (w, n, e, n, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geoz_to_xy (xx[i], yy[i], project_info.z_level, &xx[i], &yy[i]);
		ps_line (xx, yy, np, 3, FALSE, TRUE);
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
	
	if (frame_info.side[3]) {	/* West */
		ps_plot (xt[0], yt[0], 3);
		ps_plot (xt[3], yt[3], -2);
	}
	if (frame_info.side[1]) {	/* East */
		ps_plot (xt[1], yt[1], 3);
		ps_plot (xt[2], yt[2], -2);
	}
	if (frame_info.side[0]) {	/* South */
		ps_plot (xt[0], yt[0], 3);
		ps_plot (xt[1], yt[1], -2);
	}
	if (frame_info.side[2]) {	/* North */
		ps_plot (xt[3], yt[3], 3);
		ps_plot (xt[2], yt[2], -2);
	}
}
	
void GMT_circle_map_boundary (double w, double e, double s, double n)
{
	int i, nr;
	double x0, y0, a, da, S, C;
	
	if (!project_info.region) { /* Draw rectangular boundary and return */
		GMT_rect_map_boundary (0.0, 0.0, project_info.xmax, project_info.ymax);
		return;
	}
	
	GMT_setpen (&gmtdefs.frame_pen);
	
	nr = gmtdefs.n_lon_nodes + gmtdefs.n_lat_nodes;
	if (nr >= GMT_n_alloc) GMT_get_plot_array ();
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
	ps_line (GMT_x_plot, GMT_y_plot, nr, 3, FALSE, TRUE);
	ps_rotatetrans (-x0, -y0, 0.0);
}
	
void GMT_theta_r_map_boundary (double w, double e, double s, double n)
{
	int i, nr;
	double a, da;
	double xx[2], yy[2];
	
	GMT_setpen (&gmtdefs.frame_pen);
	
	if (fabs (s) < GMT_CONV_LIMIT) frame_info.side[0] = 0;	/* No donuts, please */
	if (fabs (fabs (e-w) - 360.0) < GMT_CONV_LIMIT || fabs (e - w) < GMT_CONV_LIMIT) {
		frame_info.side[1] = FALSE;
		frame_info.side[3] = FALSE;
	}
	nr = gmtdefs.n_lon_nodes;
	if (nr >= GMT_n_alloc) GMT_get_plot_array ();
	da = fabs (project_info.e - project_info.w) / (nr - 1);
	if (frame_info.side[2]) {
		for (i = 0; i < nr; i++) {
			a = project_info.w + i * da;
			GMT_geoz_to_xy (a, project_info.n, project_info.z_level, &GMT_x_plot[i], &GMT_y_plot[i]);
		}
		ps_line (GMT_x_plot, GMT_y_plot, nr, 3, FALSE, TRUE);
	}
	if (frame_info.side[0]) {
		for (i = 0; i < nr; i++) {
			a = project_info.w + i * da;
			GMT_geoz_to_xy (a, project_info.s, project_info.z_level, &GMT_x_plot[i], &GMT_y_plot[i]);
		}
		ps_line (GMT_x_plot, GMT_y_plot, nr, 3, FALSE, TRUE);
	}	
	if (frame_info.side[1]) {
		GMT_geoz_to_xy (project_info.e, project_info.s, project_info.z_level, &xx[0], &yy[0]);
		GMT_geoz_to_xy (project_info.e, project_info.n, project_info.z_level, &xx[1], &yy[1]);
		ps_line (xx, yy, 2, 3, FALSE, TRUE);
	}
	if (frame_info.side[3]) {
		GMT_geoz_to_xy (project_info.w, project_info.s, project_info.z_level, &xx[0], &yy[0]);
		GMT_geoz_to_xy (project_info.w, project_info.n, project_info.z_level, &xx[1], &yy[1]);
		ps_line (xx, yy, 2, 3, FALSE, TRUE);
	}
}
	
void GMT_map_latline (double lat, double west, double east)		/* Draws a line of constant latitude */
{
	int nn;
	double *llon, *llat;
	char text[32];
	
	nn = GMT_latpath (lat, west, east, &llon, &llat);
	
	GMT_n_plot = GMT_geo_to_xy_line (llon, llat, nn);
	sprintf (text, "Lat = %g", lat);
	ps_comment (text);
	GMT_plot_line (GMT_x_plot, GMT_y_plot, GMT_pen, GMT_n_plot);
	
	GMT_free ((void *)llon);
	GMT_free ((void *)llat);
}
	
void GMT_map_lonline (double lon, double south, double north)	/* Draws a line of constant longitude */
{
	int nn;
	double *llon, *llat;
	char text[32];
	
	nn = GMT_lonpath (lon, south, north, &llon, &llat);

	GMT_n_plot = GMT_geo_to_xy_line (llon, llat, nn);
	sprintf (text, "Lon = %g", lon);
	ps_comment (text);
	GMT_plot_line (GMT_x_plot, GMT_y_plot, GMT_pen, GMT_n_plot);
	
	GMT_free ((void *)llon);
	GMT_free ((void *)llat);
}

void GMT_map_lontick (double lon, double south, double north, double len)
{
	int i, nc;
	struct XINGS *xings;
	
	nc = GMT_map_loncross (lon, south, north, &xings);
	for (i = 0; i < nc; i++) GMT_map_tick (xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, xings[i].nx, 0, len);
	if (nc) GMT_free ((void *)xings);
}

void GMT_map_lattick (double lat, double west, double east, double len)
{
	int i, nc;
	
	struct XINGS *xings;
	
	nc = GMT_map_latcross (lat, west, east, &xings);
	for (i = 0; i < nc; i++) GMT_map_tick (xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, xings[i].nx, 1, len);
	if (nc) GMT_free ((void *)xings);
}

void GMT_map_tick (double *xx, double *yy, int *sides, double *angles, int nx, int type, double len)
{
	double angle, xl, yl, xt, yt, c, s, tick_length;
	int i;
	
	for (i = 0; i < nx; i++) {
		if (!project_info.edge[sides[i]]) continue;
		if (!frame_info.side[sides[i]]) continue;
		if (!(gmtdefs.oblique_annotation & 1) && ((type == 0 && (sides[i] % 2)) || (type == 1 && !(sides[i] % 2)))) continue;
		angle = ((gmtdefs.oblique_annotation & 16) ? (sides[i] - 1) * 90.0 : angles[i]) * D2R;
		sincos (angle, &s, &c);
		tick_length = len;
		if (gmtdefs.oblique_annotation & 8) {
			if (sides[i] % 2) {
				if (fabs (c) > cosd (gmtdefs.annot_min_angle)) continue;
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
		ps_plot (xt, yt, 3);
		GMT_xy_do_z_to_xy (xx[i]+xl, yy[i]+yl, project_info.z_level, &xt, &yt);
		ps_plot (xt, yt, -2);
	}
}

void GMT_map_symbol_ew (double lat, char *label, double west, double east, BOOLEAN annot)
{
	int i, nc;
	struct XINGS *xings;
	
	nc = GMT_map_latcross (lat, west, east, &xings);
	for (i = 0; i < nc; i++) GMT_map_symbol (xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, label, xings[i].nx, 1, annot);
	if (nc) GMT_free ((void *)xings);
}

void GMT_map_symbol_ns (double lon, char *label, double south, double north, BOOLEAN annot)
{
	int i, nc;
	struct XINGS *xings;
	
	nc = GMT_map_loncross (lon, south, north, &xings);
	for (i = 0; i < nc; i++)  GMT_map_symbol (xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, label, xings[i].nx, 0, annot);
	if (nc) GMT_free ((void *)xings);
}

void GMT_map_symbol (double *xx, double *yy, int *sides, double *line_angles, char *label, int nx, int type, BOOLEAN annot)
{
	/* type = 0 for lon and 1 for lat */
            
	double line_angle, text_angle, div, tick_length, o_len, len, dx, dy, angle, ca, sa, xt1, yt1, zz, tick_x[2], tick_y[2];
	int i, justify;
	BOOLEAN flip;
	char cmd[BUFSIZ];
	
	len = GMT_get_annot_offset (&flip);
	for (i = 0; i < nx; i++) {
	
		if (GMT_prepare_label (line_angles[i], sides[i], xx[i], yy[i], type, &line_angle, &text_angle, &justify)) continue;
		
		angle = line_angle * D2R;
		sincos (angle, &sa, &ca);
		tick_length = gmtdefs.tick_length;
		o_len = len;
		if ((type == 0 && gmtdefs.oblique_annotation & 2) || (type == 1 && gmtdefs.oblique_annotation & 4)) {
			o_len = tick_length;
		}
		if (gmtdefs.oblique_annotation & 8) {
			div = ((sides[i] % 2) ? fabs(ca) : fabs(sa));
			tick_length /= div;
			o_len /= div;
		}
		dx = tick_length * ca;
		dy = tick_length * sa;
		GMT_z_to_zz (project_info.z_level, &zz);
		GMT_xyz_to_xy (xx[i], yy[i], zz, &tick_x[0], &tick_y[0]);
		GMT_xyz_to_xy (xx[i]+dx, yy[i]+dy, zz, &tick_x[1], &tick_y[1]);
		xx[i] += o_len * ca;
		yy[i] += o_len * sa;
		if ((type == 0 && gmtdefs.oblique_annotation & 2) || (type == 1 && gmtdefs.oblique_annotation & 4)) {
			if (sides[i] % 2 && gmtdefs.annot_offset > 0.0) xx[i] += (sides[i] == 1) ? gmtdefs.annot_offset : -gmtdefs.annot_offset;
			if (!(sides[i] % 2) && gmtdefs.annot_offset > 0.0) yy[i] += (sides[i] == 2) ? gmtdefs.annot_offset : -gmtdefs.annot_offset;
		}
		GMT_xyz_to_xy (xx[i], yy[i], zz, &xt1, &yt1);
			
		if (project_info.three_D) {
			int upside = FALSE, k;
			double xp[2], yp[2], xt2, xt3, yt2, yt3, del_y;
			double xshrink, yshrink, tilt, baseline_shift, cb, sb, a;
				
			upside = (z_project.quadrant == 1 || z_project.quadrant == 4);
			sincos (text_angle * D2R, &sb, &cb);
			if (sides[i]%2 == 0 && (justify%2 == 0)) {
				if (upside) {
					k = (sides[i] == 0) ? 2 : 10;
					text_angle += 180.0;
				}
				else
					k = justify;
				del_y = 0.5 * gmtdefs.annot_font_size * 0.732 * (k/4) * GMT_u2u[GMT_PT][GMT_INCH];
				justify = 2;
				xx[i] += del_y * ca;	yy[i] += del_y * sa;
				GMT_xyz_to_xy (xx[i], yy[i], zz, &xt1, &yt1);
			}
			else {
				del_y = -0.5 * gmtdefs.annot_font_size * 0.732 * (justify/4) * GMT_u2u[GMT_PT][GMT_INCH];
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
					case 3:
						a = line_angle - 90.0;
						break;
				}
				a *= D2R;
				sincos (a, &sa, &ca);
				xx[i] += del_y * ca;	yy[i] += del_y * sa;
				GMT_xyz_to_xy (xx[i], yy[i], zz, &xt1, &yt1);
			}
			xp[0] = xx[i] + o_len * cb;	yp[0] = yy[i] + o_len * sb;
			xp[1] = xx[i] - o_len * sb;	yp[1] = yy[i] + o_len * cb;
			GMT_xyz_to_xy (xp[0], yp[0], zz, &xt2, &yt2);
			GMT_xyz_to_xy (xp[1], yp[1], zz, &xt3, &yt3);
			xshrink = hypot (xt2-xt1, yt2-yt1) / hypot (xp[0]-xx[i], yp[0]-yy[i]);
			yshrink = hypot (xt3-xt1, yt3-yt1) / hypot (xp[1]-xx[i], yp[1]-yy[i]);
			baseline_shift = d_atan2 (yt2 - yt1, xt2 - xt1) - d_atan2 (yp[0] - yy[i], xp[0] - xx[i]);
			tilt = 90.0 - R2D * (d_atan2 (yt3 - yt1, xt3 - xt1) - d_atan2 (yt2 - yt1, xt2 - xt1));
			tilt = tand (tilt);
			/* Temporarily modify meaning of F0 */
			sprintf (cmd, "/F0 {/%s findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont} bind def",
				GMT_font[gmtdefs.annot_font].name, xshrink, yshrink * tilt, yshrink);
			ps_command (cmd);
			ps_setfont (0);
			text_angle += (R2D * baseline_shift);
		}
		if (annot) {
			if (GMT_annot_too_crowded (xt1, yt1, sides[i])) continue;
			if (flip) justify = GMT_flip_justify (justify);
			/* ps_line (tick_x, tick_y, 2, 3, FALSE, TRUE); */
			ps_text (xt1, yt1, gmtdefs.annot_font_size, label, text_angle, justify, 0);
			if (project_info.three_D) ps_command ("/F0 {/Helvetica Y} bind def"); /* Reset F0 */
		}
	}
}

BOOLEAN GMT_annot_too_crowded (double x, double y, int side) {
	/* Checks if the proposed annotation is too close to a previously plotted annotation */
	int i;
	double d_min;
	
	if (gmtdefs.annot_min_spacing <= 0.0) return (FALSE);
	
	for (i = 0, d_min = DBL_MAX; i < GMT_n_annotations[side]; i++) d_min = MIN (d_min, hypot (GMT_x_annotation[side][i] - x, GMT_y_annotation[side][i] - y));
	if (d_min < gmtdefs.annot_min_spacing) return (TRUE);
	
	/* OK to plot and add to list */
	
	GMT_x_annotation[side][GMT_n_annotations[side]] = x;
	GMT_y_annotation[side][GMT_n_annotations[side]] = y;
	GMT_n_annotations[side]++;
	
	if (GMT_n_annotations[side] == GMT_alloc_annotations[side]) {
		GMT_alloc_annotations[side] += GMT_SMALL_CHUNK;
		GMT_x_annotation[side] = (double *) GMT_memory ((void *)GMT_x_annotation[side], (size_t)GMT_alloc_annotations[side], sizeof (double), "GMT_annot_too_crowded");
		GMT_y_annotation[side] = (double *) GMT_memory ((void *)GMT_y_annotation[side], (size_t)GMT_alloc_annotations[side], sizeof (double), "GMT_annot_too_crowded");
	}
	return (FALSE);
}

		
int GMT_map_latcross (double lat, double west, double east, struct XINGS **xings)
{
	int i, go = FALSE, nx, nc = 0, n_alloc = 50;
	double lon, lon_old, this_x, this_y, last_x, last_y, xlon[2], xlat[2], gap;
	double GMT_get_angle (double lon1, double lat1, double lon2, double lat2);
	struct XINGS *X;
	
	X = (struct XINGS *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (struct XINGS), "GMT_map_latcross");
		
	lon_old = west - SMALL;
	GMT_map_outside (lon_old, lat);
	GMT_geo_to_xy (lon_old, lat, &last_x, &last_y);
	for (i = 1; i <= gmtdefs.n_lon_nodes; i++) {
		lon = (i == gmtdefs.n_lon_nodes) ? east + SMALL : west + i * gmtdefs.dlon;
		GMT_map_outside (lon, lat);
		GMT_geo_to_xy (lon, lat, &this_x, &this_y);
		nx = 0;
		if ( GMT_break_through (lon_old, lat, lon, lat) ) {	/* Crossed map boundary */
			nx = GMT_map_crossing (lon_old, lat, lon, lat, xlon, xlat, X[nc].xx, X[nc].yy, X[nc].sides);
			if (nx == 1) X[nc].angle[0] = GMT_get_angle (lon_old, lat, lon, lat);
			if (nx == 2) X[nc].angle[1] = X[nc].angle[0] + 180.0;
			if (GMT_corner > 0) {
				X[nc].sides[0] = (GMT_corner%4 > 1) ? 1 : 3;
				if (project_info.got_azimuths) X[nc].sides[0] = (X[nc].sides[0] + 2) % 4;
				GMT_corner = 0;
			}
		}
		if (GMT_world_map) (*GMT_wrap_around_check) (X[nc].angle, last_x, last_y, this_x, this_y, X[nc].xx, X[nc].yy, X[nc].sides, &nx);
		if (nx == 2 && (fabs (X[nc].xx[1] - X[nc].xx[0]) - GMT_map_width) < SMALL && !GMT_world_map)
			go = FALSE;
		else if (nx == 2 && (gap = fabs (X[nc].yy[1] - X[nc].yy[0])) > SMALL && (gap - GMT_map_height) < SMALL && !GMT_world_map_tm)
			go = FALSE;
		else if (nx > 0)
			go = TRUE;
		if (go) {
			X[nc].nx = nx;
			nc++;
			if (nc == n_alloc) {
				n_alloc += 50;
				X = (struct XINGS *) GMT_memory ((void *)X, (size_t)n_alloc, sizeof (struct XINGS), "GMT_map_latcross");
			}
			go = FALSE;
		}
		lon_old = lon;
		last_x = this_x;	last_y = this_y;
	}
	
	if (nc > 0) {
		X = (struct XINGS *) GMT_memory ((void *)X, (size_t)nc, sizeof (struct XINGS), "GMT_map_latcross");
		*xings = X;
	}
	else
		GMT_free ((void *)X);
	
	return (nc);
}

int GMT_map_loncross (double lon, double south, double north, struct XINGS **xings)
{
	int go = FALSE, j, nx, nc = 0, n_alloc = 50;
	double lat, lat_old, this_x, this_y, last_x, last_y, xlon[2], xlat[2], gap;
	double GMT_get_angle (double lon1, double lat1, double lon2, double lat2);
	struct XINGS *X;
	
	X = (struct XINGS *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (struct XINGS), "GMT_map_loncross");
	
	lat_old = ((south - SMALL) >= -90.0) ? south - SMALL : south;	/* Outside */
	if ((north + SMALL) <= 90.0) north += SMALL;
	GMT_map_outside (lon, lat_old);
	GMT_geo_to_xy (lon, lat_old, &last_x, &last_y);
	for (j = 1; j <= gmtdefs.n_lat_nodes; j++) {
		lat = (j == gmtdefs.n_lat_nodes) ? north: south + j * gmtdefs.dlat;
		GMT_map_outside (lon, lat);
		GMT_geo_to_xy (lon, lat, &this_x, &this_y);
		nx = 0;
		if ( GMT_break_through (lon, lat_old, lon, lat) ) {	/* Crossed map boundary */
			nx = GMT_map_crossing (lon, lat_old, lon, lat, xlon, xlat, X[nc].xx, X[nc].yy, X[nc].sides);
			if (nx == 1) X[nc].angle[0] = GMT_get_angle (lon, lat_old, lon, lat);
			if (nx == 2) X[nc].angle[1] = X[nc].angle[0] + 180.0;
			if (GMT_corner > 0) {
				X[nc].sides[0] = (GMT_corner < 3) ? 0 : 2;
				GMT_corner = 0;
			}
		}
		if (GMT_world_map) (*GMT_wrap_around_check) (X[nc].angle, last_x, last_y, this_x, this_y, X[nc].xx, X[nc].yy, X[nc].sides, &nx);
		if (nx == 2 && (fabs (X[nc].xx[1] - X[nc].xx[0]) - GMT_map_width) < SMALL && !GMT_world_map)
			go = FALSE;
		else if (nx == 2 && (gap = fabs (X[nc].yy[1] - X[nc].yy[0])) > SMALL && (gap - GMT_map_height) < SMALL && !GMT_world_map_tm)
			go = FALSE;
		else if (nx > 0)
			go = TRUE;
		if (go) {
			X[nc].nx = nx;
			nc++;
			if (nc == n_alloc) {
				n_alloc += 50;
				X = (struct XINGS *) GMT_memory ((void *)X, (size_t)n_alloc, sizeof (struct XINGS), "GMT_map_loncross");
			}
			go = FALSE;
		}
		lat_old = lat;
		last_x = this_x;	last_y = this_y;
	}
	
	if (nc > 0) {
		X = (struct XINGS *) GMT_memory ((void *)X, (size_t)nc, sizeof (struct XINGS), "GMT_map_loncross");
		*xings = X;
	}
	else
		GMT_free ((void *)X);
	
	return (nc);
}

void GMT_map_gridlines (double w, double e, double s, double n)
{
	double dx, dy;
	
	if (gmtdefs.grid_cross_size > 0.0) return;
	
	dx = GMT_get_map_interval (0, GMT_GRID_UPPER);
	dy = GMT_get_map_interval (1, GMT_GRID_UPPER);
	
	if (dx <= 0.0 && dy <= 0.0) return;

	ps_comment ("Map gridlines");

	GMT_setpen (&gmtdefs.grid_pen);

	if (project_info.xyz_projection[0] == TIME && dx > 0.0)
		GMT_timex_grid (w, e, s, n);
	else if (dx > 0.0 && project_info.xyz_projection[0] == LOG10)
		GMT_logx_grid (w, e, s, n, dx);
	else if (dx > 0.0 && project_info.xyz_projection[0] == POW)
		GMT_powx_grid (w, e, s, n, dx);
	else if (dx > 0.0)	/* Draw grid lines that go E to W */
		GMT_linearx_grid (w, e, s, n, dx);
	
	if (project_info.xyz_projection[1] == TIME && dy > 0.0)
		GMT_timey_grid (w, e, s, n);
	else if (dy > 0.0 && project_info.xyz_projection[1] == LOG10)
		GMT_logy_grid (w, e, s, n, dy);
	else if (dy > 0.0 && project_info.xyz_projection[1] == POW)
		GMT_powy_grid (w, e, s, n, dy);
	else if (dy > 0.0)	/* Draw grid lines that go S to N */
		GMT_lineary_grid (w, e, s, n, dy);

	if (gmtdefs.grid_pen.texture) ps_setdash (CNULL, 0);
}

void GMT_map_gridcross (double w, double e, double s, double n)
{
	int i, j, nx, ny;
	double x0, y0, x1, y1, xa, xb, ya, yb, *x, *y;
	double x_angle, y_angle, xt1, xt2, yt1, yt2, C, S, L;
	
	if (gmtdefs.grid_cross_size <= 0.0) return;
	
	
	ps_comment ("Map gridcrosses");

	GMT_map_clip_on (GMT_no_rgb, 3);
	
	GMT_setpen (&gmtdefs.grid_pen);
	
	nx = GMT_coordinate_array (w, e, &frame_info.axis[0].item[5], &x);
	ny = GMT_coordinate_array (s, n, &frame_info.axis[1].item[5], &y);

	L = 0.5 * gmtdefs.grid_cross_size;
	
	for (i = 0; i < nx; i++) {
		for (j = 0; j < ny; j++) {
			
			if (!GMT_map_outside (x[i], y[j])) {	/* Inside map */
			
				GMT_geo_to_xy (x[i], y[j], &x0, &y0);
				if (MAPPING) {
					GMT_geo_to_xy (x[i] + gmtdefs.dlon, y[j], &x1, &y1);
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
				ps_plot (xt1, yt1, 3);
				ps_plot (xt2, yt2, -2);
				
				if (MAPPING) {
					GMT_geo_to_xy (x[i], y[j] - copysign (gmtdefs.dlat, y[j]), &x1, &y1);
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
				ps_plot (xt1, yt1, 3);
				ps_plot (xt2, yt2, -2);
			}
		}
	}
	if (nx) GMT_free ((void *)x);
	if (ny) GMT_free ((void *)y);

	if (gmtdefs.grid_pen.texture) ps_setdash (CNULL, 0);
	
	GMT_map_clip_off ();
}

void GMT_map_tickmarks (double w, double e, double s, double n)
{
	/* Tickmarks at annotation interval has already been done except when annotations were not desired */
	
	if (!(MAPPING || project_info.projection == POLAR)) return;	/* Tickmarks already done by linear axis */
	
	ps_comment ("Map tickmarks");
	GMT_setpen (&gmtdefs.tick_pen);

	GMT_map_tickitem (w, e, s, n, GMT_ANNOT_UPPER);
	if (gmtdefs.basemap_type == GMT_IS_PLAIN) GMT_map_tickitem (w, e, s, n, GMT_TICK_UPPER);	/* else we do it with checkerboard b/w patterns */

	if (gmtdefs.tick_pen.texture) ps_setdash (CNULL, 0);
}

void GMT_map_tickitem (double w, double e, double s, double n, int item)
{
	int i, nx, ny;
	double dx, dy, w1, s1, val, len;
	BOOLEAN do_x, do_y;
	
	dx = GMT_get_map_interval (0, item);
	dy = GMT_get_map_interval (1, item);

	if (dx <= 0.0 && dy <= 0.0) return;

	do_x = ((dx > 0.0 && item == GMT_ANNOT_UPPER) || (dx > 0.0 && item == GMT_TICK_UPPER && dx != GMT_get_map_interval (0, GMT_ANNOT_UPPER)));
	do_y = ((dy > 0.0 && item == GMT_ANNOT_UPPER) || (dy > 0.0 && item == GMT_TICK_UPPER && dy != GMT_get_map_interval (1, GMT_ANNOT_UPPER)));
	len = gmtdefs.tick_length;
	if (item == GMT_TICK_UPPER) len *= 0.5; 

	GMT_on_border_is_outside = TRUE;	/* Temporarily, points on the border are outside */
	
	if (do_x) {	/* Draw grid lines that go E to W */
		w1 = floor (w / dx) * dx;
		if (fabs (w1 - w) > SMALL) w1 += dx;
		nx = (w1 > e) ? -1 : (int)((e - w1) / dx + SMALL);
		for (i = 0; i <= nx; i++) {
			val = w1 + i * dx;
			if (val > e) val = e;
			GMT_map_lontick (val, s, n, len);
		}
	}
	
	if (do_y) {	/* Draw grid lines that go S to N */
		s1 = floor (s / dy) * dy;
		if (fabs (s1 - s) > SMALL) s1 += dy;
		ny = (s1 > n) ? -1 : (int)((n - s1) / dy + SMALL);
		for (i = 0; i <= ny; i++) {
			val = s1 + i * dy;
			if (val > n) val = n;
			GMT_map_lattick (val, w, e, len);
		}
	}
	
	GMT_on_border_is_outside = FALSE;	/* Reset back to default */
}

void GMT_map_annotate (double w, double e, double s, double n)
{
	double s1, w1, val, dx, dy, x, y, del;
	int do_minutes, do_seconds, move_up, i, nx, ny, done_zero = FALSE, annot, GMT_world_map_save;
	char label[256], cmd[256];
	PFI GMT_outside_save;
	
	if (!(MAPPING)) {
		if (project_info.projection != POLAR) return;	/* Annotations and header already done by linear_axis */
	}
	
	dx = (project_info.edge[0] || project_info.edge[2]) ? GMT_get_map_interval (0, GMT_ANNOT_UPPER) : 0.0;
	dy = (project_info.edge[1] || project_info.edge[3]) ? GMT_get_map_interval (1, GMT_ANNOT_UPPER) : 0.0;

	if (!frame_info.header[0] && dx <= 0.0 && dy <= 0.0) return;

	ps_setpaint (gmtdefs.basemap_frame_rgb);

	if (frame_info.header[0]) {	/* Make plot header for geographic maps*/
		if (project_info.three_D && fabs (project_info.z_scale) < GMT_CONV_LIMIT) {	/* Only do this if flat 2-D plot */
			
			move_up = (MAPPING || frame_info.side[2] == 2);
			ps_setfont (0);
			del = ((gmtdefs.tick_length > 0.0) ? gmtdefs.tick_length : 0.0) + gmtdefs.header_offset;
			del += ((move_up) ? (gmtdefs.annot_font_size) * GMT_u2u[GMT_PT][GMT_INCH] : 0.0);
			GMT_xy_do_z_to_xy (project_info.xmax * 0.5, project_info.ymax+del, project_info.z_level, &x, &y);
			sprintf (cmd, "/F0 {/%s findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont} bind def",
				GMT_font[gmtdefs.header_font].name, z_project.xshrink[0], z_project.yshrink[0] * z_project.tilt[0], z_project.yshrink[0]);
			ps_command (cmd);
			sprintf (cmd, "/F12 {/Symbol findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont} bind def",
				z_project.xshrink[0], z_project.yshrink[0] * z_project.tilt[0], z_project.yshrink[0]);
			ps_command (cmd);
			
			ps_text (x, y, gmtdefs.header_font_size, frame_info.header, z_project.phi[0], -2, 0);
			ps_command ("/F0 {/Helvetica Y} bind def");	/* Reset F0 */
			ps_command ("/F12 {/Symbol Y} bind def");	/* Reset F12 */
			ps_setfont (gmtdefs.header_font);
		}
		else if (!project_info.three_D) {
			ps_setfont (gmtdefs.header_font);
			if (MAPPING || frame_info.side[2] == 2) {
				ps_set_length ("PSL_TL", gmtdefs.tick_length);
				ps_set_length ("PSL_AO0", gmtdefs.annot_offset);
				ps_set_length ("PSL_HO", gmtdefs.header_offset);
				ps_textdim ("PSL_dimx", "PSL_AF0", gmtdefs.annot_font_size, gmtdefs.annot_font, "100\\312", 0);			/* Get and set typical annotation dimensions in PostScript */
			}
			else {
				ps_set_length ("PSL_TL", gmtdefs.tick_length);
				ps_set_length ("PSL_AO0", 0.0);
				ps_set_length ("PSL_HO", gmtdefs.header_offset);
			}
			ps_command ("/PSL_H_y PSL_TL PSL_AO0 add PSL_AF0 add PSL_HO add def");						/* PSL_H was not set by linear axis */
			ps_set_length ("PSL_x", project_info.xmax * 0.5);
			ps_set_length ("PSL_y", project_info.ymax);
			ps_textdim ("PSL_dimx", "PSL_dimy", gmtdefs.header_font_size, gmtdefs.header_font, frame_info.header, 0);			/* Get and set string dimensions in PostScript */
			ps_command ("PSL_x PSL_dimx -0.5 mul add PSL_y PSL_H_y add M");
			ps_setfont (gmtdefs.header_font);
			ps_text (0.0, 0.0, -gmtdefs.header_font_size, frame_info.header, 0.0, 0, 0);
		}
	}
	
	ps_comment ("Map annotations");

	ps_setfont (gmtdefs.annot_font);
	GMT_setpen (&gmtdefs.tick_pen);
	
	GMT_on_border_is_outside = TRUE;	/* Temporarily, points on the border are outside */
	GMT_world_map_save = GMT_world_map;
	if (project_info.region) {
		GMT_world_map = FALSE;
		GMT_outside_save = GMT_outside;
		GMT_outside = GMT_wesn_outside_np;
	}
	
	if (dx > 0.0) {	/* Annotate the S and N boundaries */
		BOOLEAN full_lat_range, proj_A, proj_B, annot_0_and_360;
		
		/* Determine if we should annotate both 0 and 360 degrees */
		
		full_lat_range = (fabs (180.0 - fabs (project_info.n - project_info.s)) < SMALL);
		proj_A = (project_info.projection == MERCATOR || project_info.projection == OBLIQUE_MERC ||
			project_info.projection == WINKEL || project_info.projection == ECKERT4 || project_info.projection == ECKERT6 ||
			project_info.projection == ROBINSON || project_info.projection == CYL_EQ ||
			project_info.projection == CYL_EQDIST || project_info.projection == MILLER || project_info.projection == LINEAR);
		proj_B = (project_info.projection == HAMMER || project_info.projection == MOLLWEIDE ||
			project_info.projection == SINUSOIDAL);
/*		annot_0_and_360 = (GMT_world_map_save && ((full_lat_range && proj_A) || (!full_lat_range && proj_B))); */
		annot_0_and_360 = (GMT_world_map_save && (proj_A || (!full_lat_range && proj_B)));
		
		do_minutes = (fabs (fmod (dx, 1.0)) > SMALL);
		do_seconds = (fabs (60.0 * fmod (fmod (dx, 1.0) * 60.0, 1.0)) >= 1.0);
		w1 = floor (w / dx) * dx;
		if (fabs (w1 - w) > SMALL) w1 += dx;
		nx = (w1 > e) ? -1 : (int)((e - w1) / dx + SMALL);
		for (i = 0; i <= nx; i++) {
			val = w1 + i * dx;
			if (fabs (val) < GMT_CONV_LIMIT) done_zero = TRUE;
			if (val > e) val = e;
			GMT_get_annot_label (val, label, do_minutes, do_seconds, 0, GMT_world_map_save);
			annot = annot_0_and_360 || !(done_zero && fabs (val - 360.0) < GMT_CONV_LIMIT);
			GMT_map_symbol_ns (val, label, s, n, annot);
		}
	}
	
	if (dy > 0.0) {	/* Annotate W and E boundaries */
		int lonlat;
		
		if (MAPPING) {
			do_minutes = (fabs (fmod (dy, 1.0)) > SMALL);
			do_seconds = (fabs (60.0 * fmod (fmod (dy, 1.0) * 60.0, 1.0)) >= 1.0);
			lonlat = 1;
		}
		else {	/* Also, we know that gmtdefs.degree_format = -1 in this case */
			do_minutes = do_seconds = 0;
			lonlat = 2;
			if (project_info.got_azimuths) i_swap (frame_info.side[1], frame_info.side[3]);	/* Temporary swap to trick justify machinery */
		}
		s1 = floor (s / dy) * dy;
		if (fabs (s1 - s) > SMALL) s1 += dy;
		ny = (s1 > n) ? -1: (int)((n - s1) / dy + SMALL);
		for (i = 0; i <= ny; i++) {
			/* val = s1 + i * dy; */
			val = s1 + i * dy;
			if (val > n) val = n;
			if ((project_info.polar || project_info.projection == GRINTEN) && fabs (fabs (val) - 90.0) < GMT_CONV_LIMIT) continue;
			GMT_get_annot_label (val, label, do_minutes, do_seconds, lonlat, GMT_world_map_save);
			GMT_map_symbol_ew (val, label, w, e, TRUE);
		}
		if (project_info.got_azimuths) i_swap (frame_info.side[1], frame_info.side[3]);	/* Undo the temporary swap */
	}
	
	if (project_info.three_D) ps_command ("/F0 {/Helvetica Y} bind def");	/* Reset definition of F0 */
	if (project_info.three_D) ps_command ("/F12 {/Symbol Y} bind def");	/* Reset definition of F12 */
	
	GMT_on_border_is_outside = FALSE;	/* Reset back to default */
	if (project_info.region) {
		GMT_world_map = GMT_world_map_save;
		GMT_outside = GMT_outside_save;
	}
}

void GMT_map_boundary (double w, double e, double s, double n)
{
	ps_comment ("Map boundaries");

	switch (project_info.projection) {
		case LINEAR:
			if (MAPPING)	/* xy is lonlat */
				GMT_fancy_map_boundary (w, e, s, n);
			else if (project_info.three_D)
				GMT_basemap_3D (3);
			else
				GMT_linear_map_boundary (w, e, s, n);
			break;
		case POLAR:
			GMT_theta_r_map_boundary (w, e, s, n);
			break;
		case MERCATOR:
		case CYL_EQ:
		case CYL_EQDIST:
		case MILLER:
			GMT_fancy_map_boundary (w, e, s, n);
			break;
		case ALBERS:
		case ECONIC:
		case LAMBERT:
			GMT_conic_map_boundary (w, e, s, n);
			break;
		case OBLIQUE_MERC:
			GMT_oblmrc_map_boundary (w, e, s, n);
			break;
		case STEREO:
		case ORTHO:
		case LAMB_AZ_EQ:
		case AZ_EQDIST:
		case GNOMONIC:
		case GRINTEN:
			if (project_info.polar)
				GMT_polar_map_boundary (w, e, s, n);
			else
				GMT_circle_map_boundary (w, e, s, n);
			break;
		case HAMMER:
		case MOLLWEIDE:
		case SINUSOIDAL:
			GMT_ellipse_map_boundary (w, e, s, n);
			break;
		case TM:
		case UTM:
		case CASSINI:
		case WINKEL:
		case ECKERT4:
		case ECKERT6:
		case ROBINSON:
			GMT_basic_map_boundary (w, e, s, n);
			break;
	}
	
	if (project_info.three_D) GMT_vertical_axis (GMT_3D_mode);

	if (gmtdefs.frame_pen.texture) ps_setdash (CNULL, 0);
}

BOOLEAN GMT_is_fancy_boundary (void)
{
	switch (project_info.projection) {
		case LINEAR:
			return (MAPPING);
			break;
		case MERCATOR:
		case CYL_EQ:
		case CYL_EQDIST:
		case MILLER:
			return (TRUE);
			break;
		case ALBERS:
		case ECONIC:
		case LAMBERT:
			return (project_info.region);
			break;
		case STEREO:
		case ORTHO:
		case LAMB_AZ_EQ:
		case AZ_EQDIST:
		case GNOMONIC:
		case GRINTEN:
			return (project_info.polar);
			break;
		case POLAR:
		case OBLIQUE_MERC:
		case HAMMER:
		case MOLLWEIDE:
		case SINUSOIDAL:
		case TM:
		case UTM:
		case CASSINI:
		case WINKEL:
		case ECKERT4:
		case ECKERT6:
		case ROBINSON:
			return (FALSE);
			break;
		default:
			fprintf (stderr, "%s: Error in GMT_is_fancy_boundary - notify developers\n", GMT_program);
			return (FALSE);
	}
}


/* GMT_map_basemap will create a basemap for the given area. 
 * Scaling and wesn are assumed to be passed through the project_info-structure (see GMT_project.h)
 * Tickmark info are passed through the frame_info-structure
 *
 */
 
void GMT_map_basemap (void) {
	int i;
	double w, e, s, n;

	if (!frame_info.plot) return;

	ps_setpaint (gmtdefs.basemap_frame_rgb);
	
	w = project_info.w;	e = project_info.e;	s = project_info.s;	n = project_info.n;
	
	if (gmtdefs.oblique_annotation & 2) frame_info.horizontal = 2;
	if (frame_info.horizontal == 2) gmtdefs.oblique_annotation |= 2;
	for (i = 0; i < 4; i++) {
		GMT_x_annotation[i] = (double *) GMT_memory (VNULL, (size_t)GMT_alloc_annotations[i], sizeof (double), "GMT_map_basemap");
		GMT_y_annotation[i] = (double *) GMT_memory (VNULL, (size_t)GMT_alloc_annotations[i], sizeof (double), "GMT_map_basemap");
	}
	if (gmtdefs.basemap_type == GMT_IS_FANCY && !GMT_is_fancy_boundary()) gmtdefs.basemap_type = GMT_IS_PLAIN;
	
	ps_comment ("Start of basemap");

	ps_setdash (CNULL, 0);	/* To ensure no dashed pens are set prior */
	
	GMT_map_gridlines (w, e, s, n);
	GMT_map_gridcross (w, e, s, n);
	
	GMT_map_tickmarks (w, e, s, n);
	
	GMT_map_annotate (w, e, s, n);
	
	GMT_map_boundary (w, e, s, n);
	
	ps_comment ("End of basemap");

	for (i = 0; i < 4; i++) {
		GMT_free (GMT_x_annotation[i]);
		GMT_free (GMT_y_annotation[i]);
	}
}

void GMT_basemap_3D (int mode)
{
	/* Mode means: 1 = background axis, 2 = foreground axis, 3 = all */
	BOOLEAN go[4], back;
	int i;
	
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

void GMT_vertical_axis (int mode)
{
	/* Mode means: 1 = background axis, 2 = foreground axis, 3 = all */
	BOOLEAN go[4], fore, back;
	int i, j;
	double xp[2], yp[2], z_annot;
	
	if ((z_annot = GMT_get_map_interval (2, GMT_ANNOT_UPPER)) == 0.0) return;

	fore = (mode > 1);	back = (mode % 2);
	for (i = 0; i < 4; i++) go[i] = (mode == 3) ? TRUE : ((back) ? z_project.draw[i] : !z_project.draw[i]);
	
	/* Vertical */
		
	if (fore && frame_info.side[4]) GMT_xyz_axis3D (z_project.z_axis, 'z', &frame_info.axis[2], frame_info.side[4]-1);
			
	if (frame_info.draw_box) {
		GMT_setpen (&gmtdefs.grid_pen);
		go[0] = ( (back && z_project.quadrant == 1) || (fore && z_project.quadrant != 1) );
		go[1] = ( (back && z_project.quadrant == 4) || (fore && z_project.quadrant != 4) );
		go[2] = ( (back && z_project.quadrant == 3) || (fore && z_project.quadrant != 3) );
		go[3] = ( (back && z_project.quadrant == 2) || (fore && z_project.quadrant != 2) );
		for (i = 0; i < 4; i++) {
			if (!go[i]) continue;
			GMT_geoz_to_xy (z_project.corner_x[i], z_project.corner_y[i], project_info.z_bottom, &xp[0], &yp[0]);
			GMT_geoz_to_xy (z_project.corner_x[i], z_project.corner_y[i], project_info.z_top, &xp[1], &yp[1]);
			ps_line (xp, yp, 2, 3, FALSE, TRUE);
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
			ps_line (xp, yp, 2, 3, FALSE, TRUE);
		}
	}
	if (back && frame_info.header[0]) {
		ps_setfont (gmtdefs.header_font);
		xp[0] = 0.5 * (z_project.xmin + z_project.xmax);
		yp[0] = z_project.ymax + 0.5;
		ps_text (xp[0], yp[0], gmtdefs.header_font_size, frame_info.header, 0.0, -2, 0);
	}
}

void GMT_xyz_axis3D (int axis_no, char axis, struct PLOT_AXIS *A, int annotate)
{
	int i, j, k, id, justify, n;
	
	double annot_off, label_off, *knots, sign, dy, tmp, xyz[3][2], len, x0, x1, y0, y1;
	double pp[3], w[3], xp, yp, del_y, val_xyz[3], phi, val0, val1;
	
	PFI xyz_forward, xyz_inverse;
	
	char annotation[256], format[32], cmd[256];
	
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
	sprintf (cmd, "/F0 {/%s findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont} bind def",
		GMT_font[gmtdefs.annot_font].name, z_project.xshrink[id], z_project.yshrink[id] * z_project.tilt[id], z_project.yshrink[id]);
	ps_command (cmd);
	/* Temporarily redefine F12 for tilted text */
	sprintf (cmd, "/F12 {/Symbol findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont} bind def",
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
	
	GMT_get_format (GMT_get_map_interval (id, GMT_ANNOT_UPPER), A->unit, format);

	annot_off = sign * (len + gmtdefs.annot_offset);
	label_off = sign * (len + 2.5 * gmtdefs.annot_offset + (gmtdefs.annot_font_size * GMT_u2u[GMT_PT][GMT_INCH]) * GMT_font[gmtdefs.annot_font].height);
	
	/* Ready to draw axis */
	
	GMT_setpen (&gmtdefs.frame_pen);
	ps_plot (x0, y0, 3);
	ps_plot (x1, y1, -2);
	GMT_setpen (&gmtdefs.tick_pen);
		
	del_y = 0.5 * sign * gmtdefs.annot_font_size * 0.732 * (justify/4) * GMT_u2u[GMT_PT][GMT_INCH];
	
	/* Do annotations with tickmarks */
	
	val_xyz[0] = z_project.corner_x[axis_no];
	val_xyz[1] = z_project.corner_y[axis_no];
	val_xyz[2] = project_info.z_level;
	n = GMT_coordinate_array (val0, val1, &A->item[GMT_ANNOT_UPPER], &knots);	/* Get all the annotation tick knots */
	for (i = 0; i < n; i++) {
		val_xyz[id] = knots[i];
		
		GMT_get_coordinate_label (annotation, NULL, format, &A->item[GMT_ANNOT_UPPER], knots[i]);
		
		GMT_project3D (val_xyz[0], val_xyz[1], val_xyz[2], &w[0], &w[1], &w[2]);
		pp[0] = w[0];
		pp[1] = w[1];
		pp[2] = w[2];
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);
		ps_plot (xp, yp, 3);
		pp[j] += dy;
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);
		ps_plot (xp, yp, -2);
		pp[j] += annot_off -dy + del_y;
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);
		if (annotate && (id < 2 || knots[i] != project_info.z_level)) ps_text (xp, yp, gmtdefs.annot_font_size, annotation, phi, 2, 0);
	}
	ps_command ("/F0 {/Helvetica Y} bind def");	/* Reset F0 */
	ps_command ("/F12 {/Symbol Y} bind def");	/* Reset F12 */
	
	if (n) GMT_free ((void *)knots);

	/* Now do frame tickmarks */
	
	dy *= 0.5;
	
	val_xyz[0] = z_project.corner_x[axis_no];
	val_xyz[1] = z_project.corner_y[axis_no];
	val_xyz[2] = project_info.z_level;
	n = GMT_coordinate_array (val0, val1, &A->item[GMT_TICK_UPPER], &knots);	/* Get all the frame tick knots */
	for (i = 0; i < n; i++) {
		val_xyz[id] = knots[i];
		if (A->type == POW && project_info.xyz_projection[id] == POW) {
			(*xyz_inverse) (&tmp, val_xyz[id]);
			val_xyz[id] = tmp;
		}
		GMT_project3D (val_xyz[0], val_xyz[1], val_xyz[2], &w[0], &w[1], &w[2]);
				
		pp[0] = w[0];
		pp[1] = w[1];
		pp[2] = w[2];
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);
		ps_plot (xp, yp, 3);
		pp[j] += dy;
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);
		ps_plot (xp, yp, -2);
	}
	if (n) GMT_free ((void *)knots);

	/* Finally do label */
	
	if (A->label[0] && annotate) {
		val_xyz[0] = z_project.corner_x[axis_no];
		val_xyz[1] = z_project.corner_y[axis_no];
		val_xyz[2] = project_info.z_level;
		/* Temporarily redefine /F0 for tilted text */
		sprintf (cmd, "/F0 {/%s findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont} bind def",
			GMT_font[gmtdefs.label_font].name, z_project.xshrink[id], z_project.yshrink[id] * z_project.tilt[id], z_project.yshrink[id]);
		ps_command (cmd);
		/* Temporarily redefine /F12 for tilted text */
		sprintf (cmd, "/F12 {/Symbol findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont} bind def",
			z_project.xshrink[id], z_project.yshrink[id] * z_project.tilt[id], z_project.yshrink[id]);
		ps_command (cmd);
		GMT_project3D (val_xyz[0], val_xyz[1], val_xyz[2], &w[0], &w[1], &w[2]);
		x0 = w[id];
		val_xyz[id] = (val_xyz[id] == xyz[id][0]) ? xyz[id][1] : xyz[id][0];
		GMT_project3D (val_xyz[0], val_xyz[1], val_xyz[2], &w[0], &w[1], &w[2]);
		x1 = w[id];
		pp[0] = w[0];
		pp[1] = w[1];
		pp[2] = w[2];
		pp[id] = 0.5 * (x1 + x0);
		pp[j] += label_off + del_y;
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);
	
		ps_text (xp, yp, gmtdefs.label_font_size, A->label, phi, 2, 0);
		ps_command ("/F0 {/Helvetica Y} bind def");	/* Reset F0 */
		ps_command ("/F12 {/Symbol Y} bind def");	/* Reset F12 */
	}
	ps_setpaint (gmtdefs.background_rgb);
	ps_comment ("End of xyz-axis3D");
	ps_command ("grestore\n");
}

void GMT_grid_clip_on (struct GRD_HEADER *h, int rgb[], int flag)
{
	/* This function sets up a clip path so that only plotting
	 * inside the grid domain will be drawn on paper. map_setup
	 * must have been called first.  If r >= 0, the map area will
	 * first be painted in the r,g,b colors specified.  flag can
	 * be 0-3, as described in ps_clipon().
	 */
	 
	double *work_x, *work_y;
	int np;
	BOOLEAN donut;
	
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

void GMT_map_clip_on (int rgb[], int flag)
{
	/* This function sets up a clip path so that only plotting
	 * inside the map area will be drawn on paper. map_setup
	 * must have been called first.  If r >= 0, the map area will
	 * first be painted in the r,g,b colors specified.  flag can
	 * be 0-3, as described in ps_clipon().
	 */
	 
	double *work_x, *work_y;
	int np;
	BOOLEAN donut;
	
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
	ps_plot (x, y, pen);
}

void GMT_fill (double x[], double y[], int n, struct GMT_FILL *fill, BOOLEAN outline)
{
	if (!fill)	/* NO fill pointer = no fill */
		ps_polygon (x, y, n, GMT_no_rgb, outline);
	else if (fill->use_pattern)
		ps_imagefill (x, y, n, fill->pattern_no, fill->pattern, fill->inverse, fill->dpi, outline, fill->colorize, fill->f_rgb, fill->b_rgb);
	else
		ps_polygon (x, y, n, fill->rgb, outline);
}

void GMT_timestamp (int argc, char **argv)
{
	time_t right_now;
	int i, plot_command = FALSE;
	char label[BUFSIZ], time_string[32], year[8];
	double x, y, dim[5];

	/* Plot time string in YEAR MONTH DAY HH:MM:SS format */
	
	dim[0] = 0.365; dim[1] = 1.15; dim[2] = 0.15; dim[3] = 0.075; dim[4] = 0.1;
	x = gmtdefs.unix_time_pos[0];
	y = gmtdefs.unix_time_pos[1];
	right_now = time ((time_t *)0);
	strncpy (time_string, ctime (&right_now), 32);
	time_string[24] = 0;
	sscanf (time_string, "%*s %*s %*s %*s %s", year);
	time_string[19] = 0;
	sprintf (label, "%s %s", year, &time_string[4]);
	for (i = 1; i < argc && argv[i][1] != 'J'; i++);
	ps_comment ("Begin time-stamp");
	ps_transrotate (x, y, 0.0);
	ps_setline (1);
	ps_rect (0.0, 0.0, dim[0]+dim[1], dim[2], gmtdefs.foreground_rgb, TRUE);
	ps_rect (0.0, 0.0, dim[0], dim[2], gmtdefs.background_rgb, TRUE);
	/* ps_setfont (1);
	ps_setpaint (gmtdefs.foreground_rgb); */
	ps_image (0.0, 0.0, dim[0], dim[2], GMT_glyph, 220, 90, 1);
	/* ps_text (0.5*dim[0], dim[3], 10, "GMT", 0.0, 6, 0);
	ps_setfont (0);
	ps_setpaint (gmtdefs.background_rgb); */
	ps_text (dim[0]+0.5*dim[1], dim[3], 8.0, label, 0.0, 6, 0);
	ps_setfont (1);
	label[0] = 0;
	if (gmtdefs.unix_time_label[0] == 'c' && gmtdefs.unix_time_label[1] == 0) {
		plot_command = TRUE;
		gmtdefs.unix_time_label[0] = 0;
	}
	if (plot_command) {
		strcpy (label, argv[0]);
		for (i = 1; i < argc; i++) {
			if (argv[i][0] != '-') continue;
			strcat (label, " ");
			strcat (label, argv[i]);
		}
	}
	else if (gmtdefs.unix_time_label[0])
		strcpy (label, gmtdefs.unix_time_label);
		
	if (label[0]) ps_text (dim[0]+dim[1]+dim[4], dim[3], 7.0, label, 0.0, 5, 0);
	ps_rotatetrans  (-x, -y, 0.0);
	ps_comment ("End time-stamp");
}

void GMT_echo_command (int argc, char **argv)
{
	/* This routine will echo the command and its arguments to the
	 * PostScript output file so that the user can see what scales
	 * etc was used to produce this plot
	 */
	int i, length = 0;
	char outstring[BUFSIZ];
	
	ps_comment ("PostScript produced by:");
	strcpy (outstring, "%% ");
	for (i = 0; i < argc; i++) {
		strcat (outstring, argv[i]);
		strcat (outstring, " ");
		length += (strlen (argv[i]) + 1);
		if (length >= 80) {
			ps_command (outstring);
			length = 0;
			strcpy (outstring, "%% ");
		}
	}
	if (length > 0) ps_command (outstring);
	ps_command ("");
}

void GMT_plot_line (double *x, double *y, int *pen, int n)
{
	int i, j, i1, way, stop, close;
	double x_cross[2], y_cross[2], *xx, *yy, xt1, yt1, xt2, yt2;
	
	if (n < 2) return;
	
	i = 0;
	while (i < (n-1) && pen[i+1] == 3) i++;	/* Skip repeating pen == 3 in beginning */
	if ((n-i) < 2) return;
	while (n > 1 && pen[n-1] == 3) n--;	/* Cut off repeating pen == 3 at end */
	if ((n-i) < 2) return;
	
	for (j = i + 1; j < n && pen[j] == 2; j++);	/* j == n means no moveto's present */
	close = (j == n) ? (hypot (x[n-1] - x[i], y[n-1] - y[i]) < SMALL) : FALSE;
	
	/* First see if we can use the ps_line call directly to save points */
	
	for (j = i + 1, stop = FALSE; !stop && j < n; j++) stop = (pen[j] == 3 || (*GMT_map_jump) (x[j-1], y[j-1], x[j], y[j]));
	if (!stop) {
		if (project_info.three_D) {	/* Must project first */
			xx = (double *) GMT_memory (VNULL, (size_t)(n-i), sizeof (double), "GMT_plot_line");
			yy = (double *) GMT_memory (VNULL, (size_t)(n-i), sizeof (double), "GMT_plot_line");
			for (j = i; j < n; j++) GMT_xy_do_z_to_xy (x[j], y[j], project_info.z_level, &xx[j], &yy[j]);
			ps_line (&xx[i], &yy[i], n - i, 3, close, TRUE);
			GMT_free ((void *)xx);
			GMT_free ((void *)yy);
		}
		else
			ps_line (&x[i], &y[i], n - i, 3, close, TRUE);
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
				ps_plot (xt1, yt1, 2);
				ps_plot (xt2, yt2, 3);
			}
			else {
				ps_plot (xt2, yt2, 2);
				ps_plot (xt1, yt1, 3);
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

void GMT_color_image (double x0, double y0, double x_side, double y_side, unsigned char *image, int nx, int ny, int depth)
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

void GMT_text3d (double x, double y, double z, double fsize, int fontno, char *text, double angle, int justify, int form)
{
	double xb, yb, xt, yt, xt1, xt2, xt3, yt1, yt2, yt3, del_y;
	double ca, sa, xshrink, yshrink, tilt, baseline_shift;
	char cmd[256];
	
        if (project_info.three_D) {
                ps_setfont (0);
                justify = abs (justify);
                del_y = 0.5 * fsize * 0.732 * (justify / 4) * GMT_u2u[GMT_PT][GMT_INCH];
                justify %= 4;
		sincos (angle * D2R, &sa, &ca);
                x += del_y * sa;	/* Move anchor point down on baseline */
                y -= del_y * ca;
                xb = x + ca;		/* Point a distance of 1.0 along baseline */
                yb = y + sa;
                xt = x - sa;		/* Point a distance of 1.0 normal to baseline */
                yt = y + ca;
                GMT_xyz_to_xy (x, y, z, &xt1, &yt1);
                GMT_xyz_to_xy (xb, yb, z, &xt2, &yt2);
                GMT_xyz_to_xy (xt, yt, z, &xt3, &yt3);
		xshrink = hypot (xt2-xt1, yt2-yt1) / hypot (xb-x, yb-y);	/* How lines in baseline-direction shrink */
		yshrink = hypot (xt3-xt1, yt3-yt1) / hypot (xt-x, yt-y);	/* How lines _|_ to baseline-direction shrink */
		baseline_shift = R2D * (d_atan2 (yt2 - yt1, xt2 - xt1) - d_atan2 (yb - y, xb - x));	/* Rotation of baseline */
		tilt = 90.0 - R2D * (d_atan2 (yt3 - yt1, xt3 - xt1) - d_atan2 (yt2 - yt1, xt2 - xt1));
		tilt = tand (tilt);
		/* Temporarily modify meaning of F0 */
		sprintf (cmd, "/F0 {/%s findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont} bind def",
			GMT_font[fontno].name, xshrink, yshrink * tilt, yshrink);
		ps_command (cmd);
		/* Temporarily modify meaning of F12 */
		sprintf (cmd, "/F12 {/Symbol findfont [%g 0 %g %g 0 0] makefont exch scalefont setfont} bind def",
			xshrink, yshrink * tilt, yshrink);
		ps_command (cmd);
                ps_text (xt1, yt1, fsize, text, angle + baseline_shift, justify, form);
                ps_command ("/F0 {/Helvetica Y} bind def");     /* Reset F0 */
                ps_command ("/F12 {/Symbol Y} bind def");       /* Reset F12 */
                ps_setfont (fontno);
        }
        else {
		ps_setfont (fontno);
		ps_text (x, y, fsize, text, angle, justify, form);
	}
}

void GMT_textbox3d (double x, double y, double z, int size, int font, char *label, double angle, int just, BOOLEAN outline, double dx, double dy, int rgb[])
{
        if (project_info.three_D) {
        	int i, len, ndig = 0, ndash = 0, nperiod = 0;
        	double xx[4], yy[4], h, w, xa, ya, cosa, sina;
		len = strlen (label);
		for (i = 0; label[i]; i++) {
			if (isdigit ((int)label[i])) ndig++;
			if (strchr (label, '.')) nperiod++;
			if (strchr (label, '-')) ndash++;
		}
		len -= (ndig + nperiod + ndash);
		w = ndig * 0.78 + nperiod * 0.38 + ndash * 0.52 + len;
		
		h = 0.58 * GMT_font[font].height * size * GMT_u2u[GMT_PT][GMT_INCH];
		w *= (0.81 * h);
		just = abs (just);
		y -= (((just/4) - 1) * h);
		x -= (((just-1)%4 - 1) * w);
		xx[0] = xx[3] = -w - dx;
		xx[1] = xx[2] = w + dx;
		yy[0] = yy[1] = -h - dy;
		yy[2] = yy[3] = h + dy;
		angle *= D2R;
		sincos (angle, &sina, &cosa);
		for (i = 0; i < 4; i++) {
			xa = xx[i] * cosa - yy[i] * sina;
			ya = xx[i] * sina + yy[i] * cosa;
			xx[i] = x + xa;	yy[i] = y + ya;
		}
		GMT_2D_to_3D (xx, yy, z, 4);
		if (rgb[0] < 0)
			ps_clipon (xx, yy, 4, rgb, 0);
		else
			ps_patch (xx, yy, 4, rgb, outline);
	}
	else
		ps_textbox (x, y, size, label, angle, just, outline, dx, dy, rgb);
}

void GMT_vector3d (double x0, double y0, double x1, double y1, double z0, double tailwidth, double headlength, double headwidth, double shape, int rgb[], BOOLEAN outline)
{
	if (project_info.three_D) {	/* Fill in local xx, yy cordinates for vector starting at (0,0) aligned horizontally */
		int i, n;
		double xx[10], yy[10], angle, length, s, c, L, xp, yp;
	
		angle = atan2 (y1 - y0, x1 - x0);
		length = hypot (y1 - y0, x1 - x0);
		sincos (angle, &s, &c);
		L = (1.0 - 0.5 * shape) * headlength;
		if (outline & 8) {	/* Double-headed vector */
			outline -= 8;	/* Remove the flag */
			n = 10;
			xx[0] = 0.0;
			xx[1] = xx[9] = headlength;
			xx[2] = xx[8] = L;
			xx[3] = xx[7] = length - L;
			xx[4] = xx[6] = length - headlength;
			yy[0] = yy[5] = 0.0;
			yy[1] = yy[4] = -headwidth;
			yy[6] = yy[9] = headwidth;
			yy[2] = yy[3] = -0.5 * tailwidth;
			yy[7] = yy[8] = 0.5 * tailwidth;
			xx[5] = length;
		}
		else {
			n = 7;
			xx[0] = xx[6] = 0.0;
			xx[1] = xx[5] = length - L;
			xx[2] = xx[4] = length - headlength;
			xx[3] = length;
			yy[0] = yy[1] = -0.5 * tailwidth;
			yy[5] = yy[6] = 0.5 * tailwidth;
			yy[2] = -headwidth;
			yy[4] = headwidth;
			yy[3] = 0.0;
		}
		for (i = 0; i < n; i++) {	/* Coordinate transformation */
			xp = x0 + xx[i] * c - yy[i] * s;	/* Rotate and add actual (x0, y0) origin */	
			yp = y0 + xx[i] * s + yy[i] * c;
			GMT_xyz_to_xy (xp, yp, z0, &xx[i], &yy[i]);	/* Then do 3-D projection */
		}
		ps_polygon (xx, yy, n, rgb, outline);
	}
	else
		ps_vector (x0, y0, x1, y1, tailwidth, headlength, headwidth, gmtdefs.vector_shape, rgb, outline);
}

int GMT_prepare_label (double angle, int side, double x, double y, int type, double *line_angle, double *text_angle, int *justify)
{
	BOOLEAN set_angle;
		
	if (!project_info.edge[side]) return -1;		/* Side doesn't exist */
	if (frame_info.side[side] < 2) return -1;	/* Don't want labels here */
		
	if (frame_info.check_side == TRUE) {
		if (type == 0 && side%2) return -1;
		if (type == 1 && !(side%2)) return -1;
	}
	
	/* if (gmtdefs.oblique_annotation & 2 && !(side%2)) angle = -90.0; */	/* GMT_get_label_parameters will make this 0 */
	if (gmtdefs.oblique_annotation & 16 && !(side%2)) angle = -90.0;	/* GMT_get_label_parameters will make this 0 */
	
	if (angle < 0.0) angle += 360.0;
	
	set_angle = ((project_info.region && !(AZIMUTHAL || CONICAL)) || !project_info.region);
	if (set_angle) {
		if (side == 0 && angle < 180.0) angle -= 180.0;
		if (side == 1 && (angle > 90.0 && angle < 270.0)) angle -= 180.0;
		if (side == 2 && angle > 180.0) angle -= 180.0;
		if (side == 3 && (angle < 90.0 || angle > 270.0)) angle -= 180.0;
	}
	
	if (!GMT_get_label_parameters (side, angle, type, text_angle, justify)) return -1;
	*line_angle = angle;
	if (gmtdefs.oblique_annotation & 16) *line_angle = (side - 1) * 90.0;
	
	if (!set_angle) *justify = GMT_polar_adjust (side, angle, x, y);

	return 0;
}

void GMT_get_annot_label (double val, char *label, int do_minutes, int do_seconds, int lonlat, BOOLEAN worldmap)
/* val:		Degree value of annotation */
/* label: 	String to hold the final annotation */
/* do_minutes:	TRUE if degree and minutes are desired, FALSE for just integer degrees */
/* do_seconds:	TRUE if degree, minutes, and seconds are desired */
/* lonlat:	0 = longitudes, 1 = latitudes, 2 non-geographical data passed */
/* worldmap:	T/F, whatever GMT_world_map is */
{
	int fmt, sign, d, m, s, m_sec, level, type;
	BOOLEAN zero_fix = FALSE;
	char letter = 0, format[64];
	
	if (lonlat == 0) {	/* Fix longitudes range first */
		GMT_lon_range_adjust (GMT_plot_calclock.geo.range, &val);
	}

	if (lonlat < 2) {	/* i.e., for geographical data */
		if (fabs (val - 360.0) < GMT_CONV_LIMIT && !worldmap) val = 0.0;
		if (fabs (val - 360.0) < GMT_CONV_LIMIT && worldmap && project_info.projection == OBLIQUE_MERC) val = 0.0;
	}

	fmt = gmtdefs.degree_format % 100;	/* take out the optional 100 or 1000 */
	if (GMT_plot_calclock.geo.wesn) {
		if (lonlat == 0) {
			switch (GMT_plot_calclock.geo.range) {
				case 0:
					letter = (fabs (val) < GMT_CONV_LIMIT) ? 0 : 'E';
					break;
				case 1:
					letter = (fabs (val) < GMT_CONV_LIMIT) ? 0 : 'W';
					break;
				default:
					letter = (fabs (val) < GMT_CONV_LIMIT || fabs (val - 180.0) < GMT_CONV_LIMIT) ? 0 : ((val < 0.0) ? 'W' : 'E');
					break;
			}
		}
		else 
			letter = (fabs (val) < GMT_CONV_LIMIT) ? 0 : ((val < 0.0) ? 'S' : 'N');
		val = fabs (val);
	}
	else
		letter = 0;
	if (GMT_plot_calclock.geo.no_sign) val = fabs (val);
	sign = (val < 0.0) ? -1 : 1;
	
	level = do_minutes + do_seconds;		/* 0, 1, or 2 */
	type = GMT_plot_calclock.geo.n_sec_decimals;
	
	if (fmt == -1 && lonlat) {	/* the r in r-theta */
		sprintf (format, "%s", gmtdefs.d_format);
		sprintf (label, format, val);
	}
	else if (GMT_plot_calclock.geo.decimal)
		sprintf (label, GMT_plot_calclock.geo.x_format, val, letter);
	else {
		(void) GMT_geo_to_dms (val, do_seconds, GMT_io.geo.f_sec_to_int, &d, &m, &s, &m_sec);	/* Break up into d, m, s, and remainder */
		if (d == 0 && sign == -1) {	/* Must write out -0 degrees, do so by writing -1 and change 1 to 0 */
			d = -1;
			zero_fix = TRUE;
		}
		switch (2*level+type) {
			case 0:
				sprintf (label, GMT_plot_format[level][type], d, letter);
				break;
			case 1:
				sprintf (label, GMT_plot_format[level][type], d, m_sec, letter);
				break;
			case 2:
				sprintf (label, GMT_plot_format[level][type], d, m, letter);
				break;
			case 3:
				sprintf (label, GMT_plot_format[level][type], d, m, m_sec, letter);
				break;
			case 4:
				sprintf (label, GMT_plot_format[level][type], d, m, s, letter);
				break;
			case 5:
				sprintf (label, GMT_plot_format[level][type], d, m, s, m_sec, letter);
				break;
		}
		if (zero_fix) label[1] = '0';	/* Undo the fix above */
	}
	
	return;
}

int GMT_polar_adjust (int side, double angle, double x, double y)
{
	int justify, left, right, top, bottom, low;
	double x0, y0;
	
	/* GMT_geo_to_xy (project_info.central_meridian, project_info.pole, &x0, &y0); */

	x0 = project_info.c_x0;
	y0 = project_info.c_y0;
	if (project_info.north_pole) {
		low = 0;
		left = 7;
		right = 5;
	}
	else {
		low = 2;
		left = 5;
		right = 7;
	}
	if ((y - y0 + SMALL) > 0.0) { /* i.e., y >= y0 */
		top = 2;
		bottom = 10;
	}
	else {
		top = 10;
		bottom = 2;
	}
	if (project_info.projection == POLAR && project_info.got_azimuths) i_swap (left, right);	/* Because with azimuths we get confused... */
	if (side%2) {	/* W and E border */
		if ((y - y0 + SMALL) > 0.0)
			justify = (side == 1) ? left : right;
		else
			justify = (side == 1) ? right : left;
	}
	else {
		if (frame_info.horizontal) {
			if (side == low)
				justify = (fabs (angle - 180.0) < GMT_CONV_LIMIT) ? bottom : top;
			else
				justify = (fabs (angle) < GMT_CONV_LIMIT) ? top : bottom;
		}	
		else {
			if (x >= x0)
				justify = (side == 2) ? left : right;
			else
				justify = (side == 2) ? right : left;
		}
	}
	return (justify);
}

double GMT_get_angle (double lon1, double lat1, double lon2, double lat2)
{
	double x1, y1, x2, y2, dx, dy, angle, direction;
	
	GMT_geo_to_xy (lon1, lat1, &x1, &y1);
	GMT_geo_to_xy (lon2, lat2, &x2, &y2);
	dx = x2 - x1;
	dy = y2 - y1;
	if (dy == 0.0 && dx == 0.0) {	/* Special case that only(?) occurs at N or S pole or r=0 for POLAR */
		if (fabs (fmod (lon1 - project_info.w + 360.0, 360.0)) > fabs (fmod (lon1 - project_info.e + 360.0, 360.0))) {	/* East */
			GMT_geo_to_xy (project_info.e, project_info.s, &x1, &y1);
			GMT_geo_to_xy (project_info.e, project_info.n, &x2, &y2);
			GMT_corner = 1;
		}
		else {
			GMT_geo_to_xy (project_info.w, project_info.s, &x1, &y1);
			GMT_geo_to_xy (project_info.w, project_info.n, &x2, &y2);
			GMT_corner = 3;
		}
		angle = d_atan2 (y2-y1, x2-x1) * R2D - 90.0;
		if (project_info.got_azimuths) angle += 180.0;
	}
	else
		angle = d_atan2 (dy, dx) * R2D;
	
	if (abs (GMT_x_status_old) == 2 && abs (GMT_y_status_old) == 2)	/* Last point outside */
		direction = angle + 180.0;
	else if (GMT_x_status_old == 0 && GMT_y_status_old == 0)		/* Last point inside */
		direction = angle;
	else {
		if (abs (GMT_x_status_new) == 2 && abs (GMT_y_status_new) == 2)	/* This point outside */
			direction = angle;
		else if (GMT_x_status_new == 0 && GMT_y_status_new == 0)		/* This point inside */
			direction = angle + 180.0;
		else {	/* Special case of corners and sides only */
			if (GMT_x_status_old == GMT_x_status_new)
				direction = (GMT_y_status_old == 0) ? angle : angle + 180.0;
			else if (GMT_y_status_old == GMT_y_status_new)
				direction = (GMT_x_status_old == 0) ? angle : angle + 180.0;
			else
				direction = angle;
			
		}
	}
	
	if (direction < 0.0) direction += 360.0;
	if (direction >= 360.0) direction -= 360.0;
	return (direction);
}


void GMT_draw_map_scale (struct MAP_SCALE *ms)
{
	int i, j, k, *rgb, n_a_ticks[9], n_f_ticks[9], unit;
	double dlon, x1, x2, dummy, a, b, tx, ty, off, f_len, a_len, x_left, bar_length;
	double xx[4], yy[4], bx[4], by[4], base, d_base, width, half, bar_width, dx_f, dx_a;
	char txt[256];
	char label[3][16];
	strcpy (label[0], "km");
	strcpy (label[1], "miles");
	strcpy (label[2], "nautical miles");
	
	if (!MAPPING) return;	/* Only for geographic projections */
	
	switch (ms->measure) {
		case 'm':	/* Statute miles instead */
			unit = 1;
			bar_length = 1.609344 * ms->length;
			break;
		case 'n':	/* Nautical miles instead */
			unit = 2;
			bar_length = 1.852 * ms->length;
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
	
	if (project_info.projection == OBLIQUE_MERC) {	/* Set latitude to the oblique latitude */
		a = fabs (GMT_great_circle_dist (project_info.o_pole_lon * R2D, project_info.o_pole_lat * R2D, ms->scale_lon, ms->scale_lat));	/* Colatitude */
		if (a > 90.0) a = 180.0 - 90.0;	/* Flip hemisphere */
		ms->scale_lat = 90.0 - a;
	}
	
	dlon = 0.5 * bar_length * 1000.0 / (project_info.M_PR_DEG * cosd (ms->scale_lat));
	
	GMT_geoz_to_xy (project_info.central_meridian - dlon, ms->scale_lat, project_info.z_level, &x1, &dummy);
	GMT_geoz_to_xy (project_info.central_meridian + dlon, ms->scale_lat, project_info.z_level, &x2, &dummy);
	width = x2 - x1;
	half = 0.5 * width;
	a_len = fabs (gmtdefs.map_scale_height);
	off = a_len + 0.75 * gmtdefs.annot_offset;
	
	GMT_setpen (&gmtdefs.tick_pen);
	if (ms->fancy) {	/* Fancy scale */
		n_f_ticks[8] = 3;
		n_f_ticks[1] = n_f_ticks[3] = n_f_ticks[7] = 4;
		n_f_ticks[0] = n_f_ticks[4] = 5;
		n_f_ticks[2] = n_f_ticks[5] = 6;
		n_f_ticks[6] = 7;
		n_a_ticks[4] = n_a_ticks[6] = n_a_ticks[8] = 1;
		n_a_ticks[0] = n_a_ticks[1] = n_a_ticks[3] = n_a_ticks[7] = 2;
		n_a_ticks[2] = n_a_ticks[5] = 3;
		base = pow (10.0, floor (d_log10 (ms->length)));
		i = irint (ms->length / base) - 1;
		d_base = ms->length / n_a_ticks[i];
		dx_f = width / n_f_ticks[i];
		dx_a = width / n_a_ticks[i];
		bar_width = 0.5 * fabs (gmtdefs.map_scale_height);
		f_len = 0.75 * fabs (gmtdefs.map_scale_height);
		yy[2] = yy[3] = ms->y0;
		yy[0] = yy[1] = ms->y0 - bar_width;
		x_left = ms->x0 - half;
		GMT_xyz_to_xy (x_left, ms->y0 - f_len, project_info.z_level, &a, &b);
		ps_plot (a, b, 3);
		GMT_xyz_to_xy (x_left, ms->y0, project_info.z_level, &a, &b);
		ps_plot (a, b, 2);
		for (j = 0; j < n_f_ticks[i]; j++) {
			xx[0] = xx[3] = x_left + j * dx_f;
			xx[1] = xx[2] = xx[0] + dx_f;
			for (k = 0; k < 4; k++) GMT_xyz_to_xy (xx[k], yy[k], project_info.z_level, &bx[k], &by[k]);
			rgb = (j%2) ? gmtdefs.foreground_rgb : gmtdefs.background_rgb;
			ps_polygon (bx, by, 4, rgb, TRUE);
			GMT_xyz_to_xy (xx[1], ms->y0 - f_len, project_info.z_level, &a, &b);
			ps_plot (a, b, 3);
			GMT_xyz_to_xy (xx[1], ms->y0, project_info.z_level, &a, &b);
			ps_plot (a, b, 2);
		}
		ty = ms->y0 - off;
		for (j = 0; j <= n_a_ticks[i]; j++) {
			tx = x_left + j * dx_a;
			GMT_xyz_to_xy (tx, ms->y0 - a_len, project_info.z_level, &a, &b);
			ps_plot (a, b, 3);
			GMT_xyz_to_xy (tx, ms->y0, project_info.z_level, &a, &b);
			ps_plot (a, b, 2);
			sprintf (txt, "%g", j * d_base);
			GMT_text3d (tx, ty, project_info.z_level, gmtdefs.annot_font_size, gmtdefs.annot_font, txt, 0.0, 10, 0);
		}
		GMT_xyz_to_xy (ms->x0, ms->y0 + f_len, project_info.z_level, &tx, &ty);
		GMT_text3d (tx, ty, project_info.z_level, gmtdefs.label_font_size, gmtdefs.label_font, label[unit], 0.0, 2, 0);
	}
	else {	/* Simple scale */
	
		sprintf (txt, "%g %s", ms->length, label[unit]);
		GMT_xyz_to_xy (ms->x0 - half, ms->y0 - gmtdefs.map_scale_height, project_info.z_level, &a, &b);
		ps_plot (a, b, 3);
		GMT_xyz_to_xy (ms->x0 - half, ms->y0, project_info.z_level, &a, &b);
		ps_plot (a, b, 2);
		GMT_xyz_to_xy (ms->x0 + half, ms->y0, project_info.z_level, &a, &b);
		ps_plot (a, b, 2);
		GMT_xyz_to_xy (ms->x0 + half, ms->y0 - gmtdefs.map_scale_height, project_info.z_level, &a, &b);
		ps_plot (a, b, 2);
		GMT_text3d (ms->x0, ms->y0 - off, project_info.z_level, gmtdefs.annot_font_size, gmtdefs.annot_font, txt, 0.0, 10, 0);
	}
}

int GMT_get_label_parameters (int side, double line_angle, int type, double *text_angle, int *justify)
{
	int ok;
	
	*text_angle = line_angle;
	if (*text_angle < -90.0) *text_angle += 360.0;
	if (frame_info.horizontal && !(side%2)) *text_angle += 90.0;
	if (*text_angle >= 270.0 ) *text_angle -= 360.0;
	else if (*text_angle >= 90.0) *text_angle -= 180.0;
	
	if (type == 0 && gmtdefs.oblique_annotation & 2) *text_angle = 0.0;	/* Force horizontal lon annotation */
	if (type == 1 && gmtdefs.oblique_annotation & 4) *text_angle = 0.0;	/* Force horizontal lat annotation */

	switch (side) {
		case 0:		/* S */
			if (frame_info.horizontal)
				*justify = 10;
			else
				*justify = ((*text_angle) < 0.0) ? 5 : 7;
			break;
		case 1:		/* E */
			*justify = 5;
			break;
		case 2:		/* N */
			if (frame_info.horizontal)
				*justify = 2;
			else
				*justify = ((*text_angle) < 0.0) ? 7 : 5;
			break;
		case 3:		/* W */
			*justify = 7;
			break;
	}
	
	if (frame_info.horizontal) return (TRUE);
		
	switch (side) {
		case 0:		/* S */
		case 2:		/* N */
			ok = (fabs ((*text_angle)) >= gmtdefs.annot_min_angle);
			break;
		case 1:		/* E */
		case 3:		/* W */
			ok = (fabs ((*text_angle)) <= (90.0 - gmtdefs.annot_min_angle));
			break;
	}
	return (ok);
}

char *GMT_convertpen (struct GMT_PEN *pen, int *width, int *offset, int rgb[])
{
	/* GMT_convertpen converts from internal points to current dpi unit.
	 * It allocates space and returns a pointer to the texture, if not null */

	char tmp[64], buffer[BUFSIZ], *texture = CNULL, *ptr;
	double pt_to_dpi;
	int n;

	pt_to_dpi = GMT_u2u[GMT_PT][GMT_INCH] * gmtdefs.dpi;

	*width = irint (pen->width * pt_to_dpi);

	if (pen->texture[0]) {
		texture = (char *) GMT_memory (VNULL, BUFSIZ, sizeof (char), "GMT_convertpen");
		strcpy (buffer, pen->texture);
		ptr = strtok (buffer, " ");
		while (ptr) {
			sprintf (tmp, "%d ", irint (atof (ptr) * pt_to_dpi));
			strcat (texture, tmp);
			ptr = strtok (CNULL, " ");
		}
		n = strlen (texture);
		texture[n-1] = 0;
		texture = (char *) GMT_memory ((void *)texture, n, sizeof (char), "GMT_convertpen");
		*offset = irint (pen->offset * pt_to_dpi);
	}

	memcpy ((void *)rgb, (void *)pen->rgb, (size_t)(3 * sizeof (int)));
	return (texture);
}

void GMT_setpen (struct GMT_PEN *pen)
{
	/* GMT_setpen issues PostScript code to set the specified pen.
	 * Must first convert from internal points to current dpi */

	int width, offset, rgb[3];
	char *texture = CNULL;

	texture = GMT_convertpen (pen, &width, &offset, rgb);

	ps_setline (width);

	ps_setdash (texture, offset);
	if (texture) GMT_free ((void *)texture);

	ps_setpaint (rgb);
}

int GMT_grid_clip_path (struct GRD_HEADER *h, double **x, double **y, BOOLEAN *donut)
{
	/* This function returns a clip path corresponding to the
	 * extent of the grid.
	 */

	int np, i, j;
	double *work_x, *work_y;

	*donut = FALSE;
	
	if (RECT_GRATICULE) {	/* Where wesn are straight hor/ver lines */
		np = 4;
		work_x = (double *)GMT_memory (VNULL, (size_t)np, sizeof (double), "GMT_map_clip_path");
		work_y = (double *)GMT_memory (VNULL, (size_t)np, sizeof (double), "GMT_map_clip_path");
		GMT_geo_to_xy (h->x_min, h->y_min, &work_x[0], &work_y[0]);
		GMT_geo_to_xy (h->x_max, h->y_max, &work_x[2], &work_y[2]);
		if (work_x[0] < project_info.xmin) work_x[0] = project_info.xmin;
		if (work_x[2] > project_info.xmax) work_x[2] = project_info.xmax;
		if (work_y[0] < project_info.ymin) work_y[0] = project_info.ymin;
		if (work_y[2] > project_info.ymax) work_y[2] = project_info.ymax;
		work_x[3] = work_x[0];	work_x[1] = work_x[2];
		work_y[1] = work_y[0];	work_y[3] = work_y[2];
	
	}
	else {	/* WESN are complex curved lines */

		np = 2 * (h->nx + h->ny - 2);
		work_x = (double *)GMT_memory (VNULL, (size_t)np, sizeof (double), "GMT_map_clip_path");
		work_y = (double *)GMT_memory (VNULL, (size_t)np, sizeof (double), "GMT_map_clip_path");
		for (i = j = 0; i < h->nx-1; i++, j++)	/* South */
			GMT_geo_to_xy (h->x_min + i * h->x_inc, h->y_min, &work_x[j], &work_y[j]);
		for (i = 0; i < h->ny-1; j++, i++)	/* East */
			GMT_geo_to_xy (h->x_max, h->y_min + i * h->y_inc, &work_x[j], &work_y[j]);
		for (i = 0; i < h->nx-1; i++, j++)	/* North */
			GMT_geo_to_xy (h->x_max - i * h->x_inc, h->y_max, &work_x[j], &work_y[j]);
		for (i = 0; i < h->ny-1; j++, i++)	/* West */
			GMT_geo_to_xy (h->x_min, h->y_max - i * h->y_inc, &work_x[j], &work_y[j]);
	}

	if (!(*donut)) np = GMT_compact_line (work_x, work_y, np, FALSE, (int *)0);
	if (project_info.three_D) GMT_2D_to_3D (work_x, work_y, project_info.z_level, np);
	
	*x = work_x;
	*y = work_y;

	return (np);
}

double GMT_get_annot_offset (BOOLEAN *flip)
{
	/* Return offset in inches for text annotation.  If annotation
	 * is to be placed 'inside' the map, set flip to TRUE */
	 
	double a;
	 
	a = gmtdefs.annot_offset;
	if (a >= 0.0) {	/* Outside annotation */
		if (gmtdefs.tick_length > 0.0) a += gmtdefs.tick_length;
		*flip = FALSE;
	}
	else {		/* Inside annotation */
		if (gmtdefs.tick_length < 0.0) a += gmtdefs.tick_length;
		*flip = TRUE;
	}

	return (a);
}

int GMT_flip_justify (int justify)
{
	/* Return the opposite justification */
	
	int j;
	
	switch (justify) {
		case 2:
			j = 10;
			break;
		case 5:
			j = 7;
			break;
		case 7:
			j = 5;
			break;
		case 10:
			j = 2;
			break;
		default:
			j = justify;
			fprintf (stderr, "%s: GMT_flip_justify called with incorrect argument (%d)\n", GMT_program, j);
			break;
	}
	
	return (j);
}
		
struct CUSTOM_SYMBOL * GMT_get_custom_symbol (char *name) {
	int i, found = -1;
	
	/* First see if we already have loaded this symbol */
	
	for (i = 0; found == -1 && i < GMT_n_custom_symbols; i++) if (!strcmp (name, GMT_custom_symbol[i]->name)) found = i;
	
	if (found >= 0) return (GMT_custom_symbol[found]);	/* Return a previously loaded symbol */

	/* Must load new symbol */
	
	GMT_custom_symbol = (struct CUSTOM_SYMBOL **) GMT_memory ((void *)GMT_custom_symbol, (size_t)(GMT_n_custom_symbols+1), sizeof (struct CUSTOM_SYMBOL *), GMT_program);
	GMT_custom_symbol[GMT_n_custom_symbols] = GMT_init_custom_symbol (name);
	
	return (GMT_custom_symbol[GMT_n_custom_symbols++]);
}

struct CUSTOM_SYMBOL * GMT_init_custom_symbol (char *name) {
	int nc, last, error = 0;
	BOOLEAN do_fill, do_pen, first = TRUE;
	char file[BUFSIZ], buffer[BUFSIZ], col[8][64];
	char *fill_p, *pen_p;
	FILE *fp;
	struct CUSTOM_SYMBOL *head;
	struct CUSTOM_SYMBOL_ITEM *s = NULL, *previous = NULL;
	
	sprintf (file, "%s.def", name);
	
	if (access (file, R_OK)) {	/* Not in current dir, try GMTHOME */
		sprintf (file, "%s%cshare%ccustom%c%s.def", GMTHOME, DIR_DELIM, DIR_DELIM, DIR_DELIM, name);
		if (access (file, R_OK)) {	/* Not there either - give up */
			fprintf (stderr, "GMT ERROR: %s : Could not find custom symbol %s\n", GMT_program, name);
			exit (EXIT_FAILURE);
		}
	}
	
	if ((fp = fopen (file, "r")) == NULL) {
		fprintf (stderr, "GMT ERROR: %s : Could not open file %s\n", GMT_program, file);
		exit (EXIT_FAILURE);
	}

	head = (struct CUSTOM_SYMBOL *) GMT_memory (VNULL, (size_t)1, sizeof (struct CUSTOM_SYMBOL), GMT_program);
	strcpy (head->name, name);
	while (fgets (buffer, BUFSIZ, fp)) {
		if (buffer[0] == '#' || buffer[0] == '\n') continue;
		
		nc = sscanf (buffer, "%s %s %s %s %s %s %s", col[0], col[1], col[2], col[3], col[4], col[5], col[6]);
		
		s = (struct CUSTOM_SYMBOL_ITEM *) GMT_memory (VNULL, (size_t)1, sizeof (struct CUSTOM_SYMBOL_ITEM), GMT_program);
		if (first) head->first = s;
		first = FALSE;
		
		s->x = atof (col[0]);
		s->y = atof (col[1]);

		do_fill = do_pen = FALSE;
		
		last = nc - 1;
		if (col[last][0] == '-' && col[last][1] == 'G') fill_p = &col[last][2], do_fill = TRUE, last--;
		if (col[last][0] == '-' && col[last][1] == 'W') pen_p = &col[last][2], do_pen = TRUE, last--;
		if (col[last][0] == '-' && col[last][1] == 'G') fill_p = &col[last][2], do_fill = TRUE, last--;	/* Check again for -G since perhaps -G -W was given */
		if (last < 2) error++;
		
		switch (col[last][0]) {
		
			/* M, D, and A allows for arbitrary polygons to be designed - these may be painted or filled with pattern */
			
			case 'M':		/* Set new anchor point */
				if (last != 2) error++;
				s->action = ACTION_MOVE;
				break;
		
			case 'D':		/* Draw to next point */
				if (last != 2) error++;
				s->action = ACTION_DRAW;
				break;
				
			case 'A':		/* Draw arc of a circle */
				if (last != 5) error++;
				s->p[0] = atof (col[2]);
				s->p[1] = atof (col[3]) * D2R;	/* Convert to radians here */
				s->p[2] = atof (col[4]) * D2R;
				s->action = ACTION_ARC;
				break;
			
			/* These are standard psxy-type symbols.  They can only be painted, not used with pattern fill.  Exception is circle which can take pattern */
			
			case 'C':		/* Draw complete circle (backwards compatible) */
			case 'c':		/* Draw complete circle */
				if (last != 3) error++;
				s->p[0] = atof (col[2]);
				s->action = ACTION_CIRCLE;
				break;
				
			case 'a':		/* Draw star symbol */
				if (last != 3) error++;
				s->p[0] = atof (col[2]);
				s->action = ACTION_STAR;
				break;
				
			case 'd':		/* Draw diamond symbol */
				if (last != 3) error++;
				s->p[0] = atof (col[2]);
				s->action = ACTION_DIAMOND;
				break;
				
			case 'h':		/* Draw hexagon symbol */
				if (last != 3) error++;
				s->p[0] = atof (col[2]);
				s->action = ACTION_HEXAGON;
				break;
				
			case 'i':		/* Draw inverted triangle symbol */
				if (last != 3) error++;
				s->p[0] = atof (col[2]);
				s->action = ACTION_ITRIANGLE;
				break;
				
			case 'l':		/* Draw letter/text symbol */
				if (last != 4) error++;
				s->p[0] = atof (col[2]);
				s->string = (char *)GMT_memory (VNULL, (size_t)(strlen (col[3]) + 1), sizeof (char), GMT_program);
				strcpy (s->string, col[3]);
				s->action = ACTION_TEXT;
				break;
				
			case 's':		/* Draw square symbol */
				if (last != 3) error++;
				s->p[0] = atof (col[2]);
				s->action = ACTION_SQUARE;
				break;
				
			case 't':		/* Draw triangle symbol */
				if (last != 3) error++;
				s->p[0] = atof (col[2]);
				s->action = ACTION_TRIANGLE;
				break;
				
			case 'x':		/* Draw cross symbol */
				if (last != 3) error++;
				s->p[0] = atof (col[2]);
				s->action = ACTION_CROSS;
				break;
				
			case 'r':		/* Draw rect symbol */
				if (last != 4) error++;
				s->p[0] = atof (col[2]);
				s->p[1] = atof (col[3]);
				s->action = ACTION_RECT;
				break;
				
			case 'w':		/* Draw wedge (pie) symbol */
				if (last != 5) error++;
				s->p[0] = atof (col[2]);
				s->p[1] = atof (col[3]);	/* Leave angles in degrees */
				s->p[2] = atof (col[4]);
				s->action = ACTION_PIE;
				break;
				
			case 'e':		/* Draw ellipse symbol */
				if (last != 5) error++;
				s->p[0] = atof (col[2]);	/* Leave direction in degrees */
				s->p[1] = atof (col[3]);
				s->p[2] = atof (col[4]);
				s->action = ACTION_ELLIPSE;
				break;
				
			default:
				error++;
				break;
		}
				
		if (error) {
			fprintf (stderr, "GMT ERROR: %s : Error in parsing symbol commands in file %s\n", GMT_program, file);
			fprintf (stderr, "GMT ERROR: %s : Offending line: %s\n", GMT_program, buffer);
			exit (EXIT_FAILURE);
		}

		if (do_fill) {
			s->fill = (struct GMT_FILL *) GMT_memory (VNULL, (size_t)1, sizeof (struct GMT_FILL), GMT_program);
			if (fill_p[0] == '-')	/* Do not want to fill this polygon */
				s->fill->rgb[0] = -1;
			else if (GMT_getfill (fill_p, s->fill)) {
				GMT_fill_syntax ('G');
				exit (EXIT_FAILURE);
			}
		}
		else
			s->fill = NULL;
		if (do_pen) {
			s->pen = (struct GMT_PEN *) GMT_memory (VNULL, (size_t)1, sizeof (struct GMT_PEN), GMT_program);
			if (pen_p[0] == '-')	/* Do not want to draw outline */
				s->pen->rgb[0] = -1;
			else if (GMT_getpen (pen_p, s->pen)) {
				GMT_pen_syntax ('W');
				exit (EXIT_FAILURE);
			}
		}
		else
			s->pen = NULL;
		
		if (previous) previous->next = s;
		previous = s;
	}
	fclose (fp);
	return (head);
}

BOOLEAN GMT_fill_is_image (char *fill) {
	/* Returns TRUE if the fill arguments involves an image pattern */
	return (fill[0] == 'P' || fill[0] == 'p');
}

void GMT_draw_custom_symbol (double x0, double y0, double z0, double size, struct CUSTOM_SYMBOL *symbol, struct GMT_PEN *pen, struct GMT_FILL *fill, BOOLEAN outline) {
	int n = 0, n_alloc = GMT_SMALL_CHUNK, na, i, font_no = gmtdefs.annot_font;
	BOOLEAN flush = FALSE;
	double x, y, da, sr, sa, ca, *xx, *yy, font_size;
	char cmd[64], *c;
	struct CUSTOM_SYMBOL_ITEM *s;
	struct GMT_FILL *f;
	struct GMT_PEN *p;
	
	xx = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), GMT_program);
	yy = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), GMT_program);
	
	sprintf (cmd, "Start of symbol %s", symbol->name);
	ps_comment (cmd);
	s = symbol->first;
	while (s) {
		x = x0 + s->x * size;
		y = y0 + s->y * size;
		
		switch (s->action) {
			case ACTION_MOVE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				xx[0] = x;
				yy[0] = y;
				n = 1;
				p = (s->pen)  ? s->pen  : pen;
				f = (s->fill) ? s->fill : fill;
				break;
				
			case ACTION_DRAW:
				flush = TRUE;
				if (n >= n_alloc) {
					n_alloc += GMT_SMALL_CHUNK;
					xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), GMT_program);
					yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), GMT_program);
				}
				xx[n] = x;
				yy[n] = y;
				n++;
				break;
				
			case ACTION_ARC:
				flush = TRUE;
				sr = 0.5 * s->p[0] * size;
				na = MAX (irint (fabs (s->p[2] - s->p[1]) * sr / gmtdefs.line_step), 16);
				da = (s->p[2] - s->p[1]) / na;
				for (i = 0; i <= na; i++) {
					if (n >= n_alloc) {
						n_alloc += GMT_SMALL_CHUNK;
						xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), GMT_program);
						yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), GMT_program);
					}
					sincos (s->p[1] + i * da, &sa, &ca);
					xx[n] = x + sr * ca;
					yy[n] = y + sr * sa;
					n++;
				}
				break;
				
			case ACTION_CROSS:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				if (p) GMT_setpen (p);
				(project_info.three_D) ? GMT_cross3D (x, y, z0, s->p[0] * size) : ps_cross (x, y, s->p[0] * size);
				break;

			case ACTION_CIRCLE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				if (f->use_pattern || project_info.three_D) {
					sr = 0.5 * s->p[0] * size;
					na = MAX (irint (TWO_PI * sr / gmtdefs.line_step), 16);
					da = TWO_PI / na;
					for (i = 0; i < na; i++) {
						if (n >= n_alloc) {
							n_alloc += GMT_SMALL_CHUNK;
							xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), GMT_program);
							yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), GMT_program);
						}
						sincos (i * da, &sa, &ca);
						xx[n] = x + sr * ca;
						yy[n] = y + sr * sa;
						n++;
					}
					GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				}
				else {	/* Clean circle - no image fill required */
					if (p) GMT_setpen (p);
					ps_circle (x, y, s->p[0] * size, f->rgb, outline);
				}
				break;
				
			case ACTION_SQUARE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				if (p) GMT_setpen (p);
				(project_info.three_D) ? GMT_square3D (x, y, z0, s->p[0] * size, f->rgb, outline) : ps_square (x, y, s->p[0] * size, f->rgb, outline);
				break;

			case ACTION_TRIANGLE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				if (p) GMT_setpen (p);
				(project_info.three_D) ? GMT_triangle3D (x, y, z0, s->p[0] * size, f->rgb, outline) : ps_triangle (x, y, s->p[0] * size, f->rgb, outline);
				break;

			case ACTION_DIAMOND:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				if (p) GMT_setpen (p);
				(project_info.three_D) ? GMT_diamond3D (x, y, z0, s->p[0] * size, f->rgb, outline) : ps_diamond (x, y, s->p[0] * size, f->rgb, outline);
				break;

			case ACTION_STAR:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				if (p) GMT_setpen (p);
				(project_info.three_D) ? GMT_star3D (x, y, z0, s->p[0] * size, f->rgb, outline) : ps_star (x, y, s->p[0] * size, f->rgb, outline);
				break;

			case ACTION_HEXAGON:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				if (p) GMT_setpen (p);
				(project_info.three_D) ? GMT_hexagon3D (x, y, z0, s->p[0] * size, f->rgb, outline) : ps_hexagon (x, y, s->p[0] * size, f->rgb, outline);
				break;

			case ACTION_ITRIANGLE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				if (p) GMT_setpen (p);
				(project_info.three_D) ? GMT_itriangle3D (x, y, z0, s->p[0] * size, f->rgb, outline) : ps_itriangle (x, y, s->p[0] * size, f->rgb, outline);
				break;

			case ACTION_TEXT:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				if (p) GMT_setpen (p);

				if ((c = strchr (s->string, '%'))) {	/* Gave font name or number, too */
					*c = 0;		/* Replace % with the end of string NUL indicator */
					c++;		/* Go to next character */
					if (c[0] >= '0' && c[0] <= '9')	/* Gave a font # */
						font_no = atoi (c);
					else
						font_no = GMT_font_lookup (c, GMT_font, N_FONTS);
					if (font_no >= N_FONTS) {
						fprintf (stderr, "%s: custom symbol subcommand l contains bad font (set to %s (0))\n", GMT_program, GMT_font[gmtdefs.annot_font].name);
						font_no = gmtdefs.annot_font;
					}
					ps_setfont (font_no);			/* Set the required font */
				}
				font_size = s->p[0] * size * 72.0;
				if (outline && f) {
					ps_setpaint (f->rgb);
					(project_info.three_D) ? GMT_text3d (x, y, z0, font_size, font_no, s->string, 0.0, 6, FALSE) : ps_text (x, y, font_size, s->string, 0.0, 6, FALSE);
					ps_setpaint (p->rgb);
					(project_info.three_D) ? GMT_text3d (x, y, z0, font_size, font_no, s->string, 0.0, 6, TRUE) : ps_text (x, y, font_size, s->string, 0.0, 6, TRUE);
				}
				else if (f)
					(project_info.three_D) ? GMT_text3d (x, y, z0, font_size, font_no, s->string, 0.0, 6, FALSE) : ps_text (x, y, font_size, s->string, 0.0, 6, FALSE);
				else
					(project_info.three_D) ? GMT_text3d (x, y, z0, font_size, font_no, s->string, 0.0, 6, TRUE) : ps_text (x, y, font_size, s->string, 0.0, 6, TRUE);
				break;

			case ACTION_ELLIPSE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				if (p) GMT_setpen (p);
				(project_info.three_D) ? GMT_ellipse3D (x, y, z0, s->p[0], s->p[1] * size, s->p[1] * size, f->rgb, outline) : ps_ellipse (x, y, s->p[0], s->p[1] * size, s->p[1] * size, f->rgb, outline);
				break;

			case ACTION_PIE:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				if (p) GMT_setpen (p);
				(project_info.three_D) ? GMT_pie3D (x, y, z0, s->p[0] * size, s->p[1], s->p[2], f->rgb, outline) : ps_pie (x, y, s->p[0] * size, s->p[1], s->p[2], f->rgb, outline);
				break;

			case ACTION_RECT:
				if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
				f = (s->fill) ? s->fill : fill;
				p = (s->pen)  ? s->pen  : pen;
				if (p) GMT_setpen (p);
				(project_info.three_D) ? GMT_rect3D (x, y, z0, s->p[0] * size, s->p[1] * size, f->rgb, outline) : ps_rect (x - 0.5 * s->p[0] * size, y - 0.5 * s->p[1] * size, x + 0.5 * s->p[0] * size, y + 0.5 * s->p[1] * size, f->rgb, outline);
				break;
				
			default:
				fprintf (stderr, "GMT ERROR: %s : Unrecognized symbol code (%d) passed to GMT_draw_custom_symbol\n", GMT_program, s->action);
				exit (EXIT_FAILURE);
				break;

		}
		
		s = s->next;
	}
	if (flush) GMT_flush_symbol_piece (xx, yy, z0, &n, p, f, outline, &flush);
	sprintf (cmd, "End of symbol %s\n", symbol->name);
	ps_comment (cmd);
	
	GMT_free ((void *)xx);
	GMT_free ((void *)yy);
}

void GMT_flush_symbol_piece (double *x, double *y, double z, int *n, struct GMT_PEN *p, struct GMT_FILL *f, BOOLEAN outline, BOOLEAN *flush) {
	BOOLEAN draw_outline;
	
	draw_outline = (outline && p->rgb[0] != -1) ? TRUE : FALSE;
	if (draw_outline) GMT_setpen (p);
	if (project_info.three_D) GMT_2Dz_to_3D (x, y, z, *n);
	GMT_fill (x, y, *n, f, draw_outline);
	*flush = FALSE;
	*n = 0;
}

/* Here lies all the 3-D version of psxyz symbols - used by both psxyz and the custom drawing routine GMT_draw_custom_symbol */

void GMT_cross3D (double x, double y, double z, double size)
{
	double xp[2], yp[2], plot_x, plot_y;

	size *= 0.5;	
	xp[0] = x - size;	xp[1] = x + size;
	yp[0] = y - size;	yp[1] = y + size;
	GMT_xyz_to_xy (xp[0], yp[0], z, &plot_x, &plot_y);
	ps_plot (plot_x, plot_y, 3);
	GMT_xyz_to_xy (xp[1], yp[1], z, &plot_x, &plot_y);
	ps_plot (plot_x, plot_y, 2);
	GMT_xyz_to_xy (xp[1], yp[0], z, &plot_x, &plot_y);
	ps_plot (plot_x, plot_y, 3);
	GMT_xyz_to_xy (xp[0], yp[1], z, &plot_x, &plot_y);
	ps_plot (plot_x, plot_y, 2);
}
	
void GMT_square3D (double x, double y, double z, double size, int rgb[], int outline)
{
	int i;
	double xp[4], yp[4], plot_x[4], plot_y[4];
	
	size *= 0.3535533906;
	xp[0] = xp[3] = x - size;	xp[1] = xp[2] = x + size;
	yp[0] = yp[1] = y - size;	yp[2] = yp[3] = y + size;
	for (i = 0; i < 4; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &plot_x[i], &plot_y[i]);
	ps_patch (plot_x, plot_y, 4, rgb, outline);
}

void GMT_rect3D (double x, double y, double z, double xsize, double ysize, int rgb[], int outline)
{
	int i;
	double xp[4], yp[4], plot_x[4], plot_y[4];
	
	xp[0] = xp[3] = x - xsize;	xp[1] = xp[2] = x + xsize;
	yp[0] = yp[1] = y - ysize;	yp[2] = yp[3] = y + ysize;
	for (i = 0; i < 4; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &plot_x[i], &plot_y[i]);
	ps_patch (plot_x, plot_y, 4, rgb, outline);
}

void GMT_circle3D (double x, double y, double z, double size, int rgb[], int outline)
{
	/* Must plot a squashed circle */
	int i;
	double xx, yy, a, da, s, c, plot_x[51], plot_y[51];
	
	da = TWO_PI / 50.0;
	size *= 0.5;
	for (i = 0; i <= 50; i++) {
		a = i * da;
		sincos (a, &s, &c);
		xx = x + size * c;
		yy = y + size * s;
		GMT_xyz_to_xy (xx, yy, z, &plot_x[i], &plot_y[i]);
	}
	ps_polygon (plot_x, plot_y, 51, rgb, outline);
}

void GMT_ellipse3D (double x, double y, double z, double direction, double major, double minor, int rgb[], int outline)
{
	/* Must plot a squashed circle */
	int i;
	double dx, dy, a, da, s, c, sin_direction, cos_direction, x_prime, y_prime, plot_x[51], plot_y[51];
	
	sincos (direction * D2R, &sin_direction, &cos_direction);
	da = TWO_PI / 50.0;
	for (i = 0; i <= 50; i++) {
		a = i * da;
		sincos (a, &s, &c);
		dx = major * c;
		dy = minor * s;
		x_prime = x + dx * cos_direction - dy * sin_direction;
		y_prime = y + dx * sin_direction + dy * cos_direction;
		GMT_xyz_to_xy (x_prime, y_prime, z, &plot_x[i], &plot_y[i]);
	}
	ps_polygon (plot_x, plot_y, 51, rgb, outline);
}

void GMT_pie3D (double x, double y, double z, double size, double dir1, double dir2, int rgb[], int outline)
{
	/* Must plot a squashed pie wedge */
	int i, j, n;
	double arc, xx, yy, a, da, s, c, plot_x[52], plot_y[52];
	
	arc = (dir2 - dir1);
	while (arc > TWO_PI) arc -= TWO_PI;
	da = TWO_PI / 50.0;	/* Standard step length for full circle */
	n = irint (arc / da);	/* But we are not doing a full circle so use less than 50 points */
	da = arc / n;		/* Step length for this pie wedge */
	size *= 0.5;
	GMT_xyz_to_xy (x, y, z, &plot_x[0], &plot_y[0]);	/* Start from center */
	for (i = 0, j = 1; i <= n; i++, j++) {
		a = dir1 + i * da;
		sincos (a, &s, &c);
		xx = x + size * c;
		yy = y + size * s;
		GMT_xyz_to_xy (xx, yy, z, &plot_x[j], &plot_y[j]);
	}
	ps_polygon (plot_x, plot_y, j, rgb, outline);
}

void GMT_triangle3D (double x, double y, double z, double size, int rgb[], int outline)
{
	int i;
	double xp[3], yp[3], plot_x[3], plot_y[3];
	
	xp[0] = x - 0.433012701892*size;	yp[0] = yp[1] = y - 0.25  * size;
	xp[1] = x + 0.433012701892*size;	xp[2] = x; 	yp[2] = y + 0.5 * size;
	for (i = 0; i < 3; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &plot_x[i], &plot_y[i]);
	ps_patch (plot_x, plot_y, 3, rgb, outline);
}

void GMT_itriangle3D (double x, double y, double z, double size, int rgb[], int outline)
{
	int i;
	double xp[3], yp[3], plot_x[3], plot_y[3];
	
	xp[0] = x - 0.433012701892*size;	yp[0] = yp[1] = y + 0.25 * size;
	xp[1] = x + 0.433012701892*size;	xp[2] = x; 	yp[2] = y - 0.5 * size;
	for (i = 0; i < 3; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &plot_x[i], &plot_y[i]);
	ps_patch (plot_x, plot_y, 3, rgb, outline);
}

void GMT_diamond3D (double x, double y, double z, double size, int rgb[], int outline)
{
	int i;
	double xp[4], yp[4], plot_x[4], plot_y[4];

	size *= 0.5;
	xp[0] = xp[2] = x;	xp[1] = x - size;	xp[3] = x + size;
	yp[0] = y - size;	yp[1] = yp[3] = y;	yp[2] = y + size;
	for (i = 0; i < 4; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &plot_x[i], &plot_y[i]);
	ps_patch (plot_x, plot_y, 4, rgb, outline);
}

void GMT_hexagon3D (double x, double y, double z, double size, int rgb[], int outline)
{
	int i;
	double xp[6], yp[6], plot_x[6], plot_y[6], sx, sy;
	
	size *= 0.5;
	sx = 0.5 * size;	sy = 0.8660254038 * size;
	xp[0] = x + size;	yp[0] = y;
	xp[1] = x + sx;		yp[1] = y + sy;
	xp[2] = x - sx;		yp[2] = yp[1];
	xp[3] = x - size;	yp[3] = y;
	xp[4] = xp[2];		yp[4] = y - sy;
	xp[5] = xp[1];		yp[5] = yp[4];

	for (i = 0; i < 6; i++) GMT_xyz_to_xy (xp[i], yp[i], z, &plot_x[i], &plot_y[i]);

	ps_patch (plot_x, plot_y, 6, rgb, outline);
}

void GMT_star3D (double x, double y, double z, double size, int rgb[], int outline)
{
	int i, k;
	double xx, yy, plot_x[10], plot_y[10], a, s2;

	size *= 0.5;
	s2 = 0.38196601125 * size;
	for (i = k = 0; i < 5; i++) {
		a = -54.0 + i * 72.0;
		xx = x + size * cosd (a);
		yy = y + size * sind (a);
		GMT_xyz_to_xy (xx, yy, z, &plot_x[k], &plot_y[k]);
		k++;
		a += 36.0;
		xx = x + s2 * cosd (a);
		yy = y + s2 * sind (a);
		GMT_xyz_to_xy (xx, yy, z, &plot_x[k], &plot_y[k]);
		k++;
	}

	ps_patch (plot_x, plot_y, 10, rgb, outline);
}

