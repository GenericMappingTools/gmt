/*--------------------------------------------------------------------
 *	$Id: gmt_plot.h,v 1.1 2006-03-22 05:20:11 pwessel Exp $
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

#ifndef _GMT_PLOT_H
#define _GMT_PLOT_H

EXTERN_MSC void GMT_circle3D (double x, double y, double z, double size, int rgb[], int outline);
EXTERN_MSC void GMT_color_image (double x0, double y0, double x_side, double y_side, unsigned char *image, int nx, int ny, int depth);
EXTERN_MSC void GMT_cross3D (double x, double y, double z, double size);
EXTERN_MSC void GMT_diamond3D (double x, double y, double z, double size, int rgb[], int outline);
EXTERN_MSC void GMT_draw_map_rose (struct MAP_ROSE *mr);
EXTERN_MSC void GMT_draw_map_scale (struct MAP_SCALE *ms);
EXTERN_MSC void GMT_echo_command (int argc, char **argv);
EXTERN_MSC void GMT_ellipse3D (double x, double y, double z, double direction, double major, double minor, int rgb[], int outline);
EXTERN_MSC void GMT_fill (double x[], double y[], int n, struct GMT_FILL *fill, BOOLEAN outline);
EXTERN_MSC void GMT_fill_polygon (double *lon, double *lat, int n, struct GMT_FILL *F, BOOLEAN outline);
EXTERN_MSC void GMT_fill_syntax (char option);
EXTERN_MSC void GMT_geoplot (double lon, double lat, int pen);
EXTERN_MSC void GMT_grid_clip_off (void);
EXTERN_MSC void GMT_grid_clip_on (struct GRD_HEADER *h, int rgb[], int flag);
EXTERN_MSC void GMT_hexagon3D (double x, double y, double z, double size, int rgb[], int outline);
EXTERN_MSC void GMT_itriangle3D (double x, double y, double z, double size, int rgb[], int outline);
EXTERN_MSC void GMT_linearx_grid (double w, double e, double s, double n, double dval);
EXTERN_MSC void GMT_map_basemap (void);
EXTERN_MSC void GMT_map_clip_off (void);
EXTERN_MSC void GMT_map_clip_on (int rgb[], int flag);
EXTERN_MSC void GMT_octagon3D (double x, double y, double z, double size, int rgb[], int outline);
EXTERN_MSC void GMT_pentagon3D (double x, double y, double z, double size, int rgb[], int outline);
EXTERN_MSC void GMT_pie3D (double x, double y, double z, double size, double dir1, double dir2, int rgb[], int outline);
EXTERN_MSC void GMT_plot_line (double *x, double *y, int *pen, int n);
EXTERN_MSC void GMT_rect3D (double x, double y, double z, double xsize, double ysize, int rgb[], int outline);
EXTERN_MSC void GMT_setpen (struct GMT_PEN *pen);
EXTERN_MSC void GMT_square3D (double x, double y, double z, double size, int rgb[], int outline);
EXTERN_MSC void GMT_star3D (double x, double y, double z, double size, int rgb[], int outline);
EXTERN_MSC void GMT_text3D (double x, double y, double z, double fsize, int fontno, char *text, double angle, int justify, int form);
EXTERN_MSC void GMT_textbox3D (double x, double y, double z, double size, int font, char *label, double angle, int just, BOOLEAN outline, double dx, double dy, int rgb[]);
EXTERN_MSC void GMT_timestamp (int argc, char **argv);
EXTERN_MSC void GMT_triangle3D (double x, double y, double z, double size, int rgb[], int outline);
EXTERN_MSC void GMT_vector3D (double x0, double y0, double x1, double y1, double z0, double tailwidth, double headlength, double headwidth, double shape, int rgb[], BOOLEAN outline);
EXTERN_MSC void GMT_vertical_axis (int mode);
EXTERN_MSC void GMT_xy_axis (double x0, double y0, double length, double val0, double val1, struct PLOT_AXIS *A, int below, int annotate);
EXTERN_MSC struct EPS *GMT_epsinfo (char *program);
EXTERN_MSC int *GMT_split_line (double **xx, double **yy, int *nn, BOOLEAN add_crossings);

#endif /* _GMT_PLOT_H */
