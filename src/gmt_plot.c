/*--------------------------------------------------------------------
 *	$Id: gmt_plot.c,v 1.23 2001-09-14 03:08:26 pwessel Exp $
 *
 *	Copyright (c) 1991-2001 by P. Wessel and W. H. F. Smith
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
 * Date:	21-JUN-2000
 * Version:	3.4
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
 *	GMT_x_axis :		Draw x axis
 *	GMT_y_axis :		Draw y axis
 *
 * INTERNAL Functions include:
 *	
 *	GMT_basemap_3D :		Plots 3-D basemap
 *	GMT_geoplot :			As ps_plot, but using lon/lat directly
 *	GMT_fill :			Convenience function for ps_imagefill
 *	GMT_get_angle :			Sub function to get annotation angles
 *	GMT_get_anot_label :		Construct degree/minute label
 *	GMT_conic_map_boundary :	Plot basemap for conic projections
 *	GMT_linear_map_boundary :	Plot basemap for Linear projections
 *	GMT_linearx_grid :		Draw linear x grid lines
 *	GMT_lineary_grid :		Draw linear y grid lines
 *	GMT_logx_grid :			Draw log10 x grid lines
 *	GMT_logy_grid :			Draw log10 y grid lines
 *	GMT_powx_grid :			Draw power x grid lines
 *	GMT_powy_grid :			Draw power y grid lines
 *	GMT_prepare_label :		Gets angle and justification for frame anotations
 *	GMT_map_anotate :		Annotate basemaps
 *	GMT_map_boundary :		Draw the maps boundary
 *	GMT_map_gridcross :		Draw grid crosses on maps
 *	GMT_map_gridlines :		Draw gridlines on maps
 *	GMT_map_latline :		Draw a latitude line
 *	GMT_map_lattick :		Draw a latitude tick marck
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

#define IS_FANCY	0
#define IS_PLAIN	1

int GMT_get_label_parameters(int side, double line_angle, int type, double *text_angle, int *justify);
int GMT_polar_adjust(int side, double angle, double x, double y);

void GMT_map_symbol(double *xx, double *yy, int *sides, double *line_angles, char *label, int nx, int type, BOOLEAN anot);
void GMT_map_symbol_ew(double lat, char *label, double west, double east, BOOLEAN anot);
void GMT_map_symbol_ns(double lon, char *label, double south, double north, BOOLEAN anot);
void GMT_get_anot_label (double val, char *label, int do_minutes, int do_seconds, int lonlat, BOOLEAN worldmap);
void GMT_basemap_3D(int mode);
void GMT_xyz_axis3D(int axis_no, char axis, struct TIME_AXIS *A, int anotate);
int GMT_coordinate_array (double min, double max, struct TIME_AXIS_ITEM *T, double **array);
int GMT_linear_array (double min, double max, double delta, double **array);
int GMT_log_array (double min, double max, double delta, double **array);
int GMT_pow_array (double min, double max, double delta, int x_or_y, double **array);
int GMT_grid_clip_path (struct GRD_HEADER *h, double **x, double **y, BOOLEAN *donut);
void GMT_wesn_map_boundary (double w, double e, double s, double n);
void GMT_rect_map_boundary (double x0, double y0, double x1, double y1);
void GMT_theta_r_map_boundary (double w, double e, double s, double n);
void GMT_map_latline (double lat, double west, double east);
void GMT_map_lonline (double lon, double south, double north);
void GMT_map_tick (double *xx, double *yy, int *sides, double *angles, int nx, int type);
void GMT_map_lontick (double lon, double south, double north);
void GMT_map_lattick (double lat, double west, double east);
int GMT_map_loncross (double lon, double south, double north, struct XINGS **xings);
int GMT_map_latcross (double lat, double west, double east, struct XINGS **xings);
int GMT_prepare_label (double angle, int side, double x, double y, int type, double *line_angle, double *text_angle, int *justify);
double GMT_get_anot_offset (BOOLEAN *flip);
int GMT_flip_justify (int justify);
BOOLEAN GMT_anot_too_crowded (double x, double y, int side);
BOOLEAN GMT_is_fancy_boundary (void);
void GMT_x_axis (double x0, double y0, double length, double val0, double val1, struct TIME_AXIS *A, int below, int anotate);
void GMT_ty_axis (double x0, double y0, double length, double val0, double val1, struct TIME_AXIS *A, int left_side, int anotate);
void GMT_coordinate_to_x (double coord, double *x);
void GMT_coordinate_to_y (double coord, double *y);
int GMT_time_array (double min, double max, struct TIME_AXIS_ITEM *T, double **array);
void GMT_timex_grid (double w, double e, double s, double n);
void GMT_timey_grid (double w, double e, double s, double n);
void GMT_get_time_label (char *string, struct GMT_PLOT_CALCLOCK *P, struct TIME_AXIS_ITEM *T, GMT_dtime t);
void GMT_get_coordinate_label (char *string, struct GMT_PLOT_CALCLOCK *P, char *format, struct TIME_AXIS_ITEM *T, double coord);
void GMT_get_primary_anot (struct TIME_AXIS *A, int *primary, int *secondary);
BOOLEAN GMT_skip_second_anot (int item, double x, double x2[], int n, int primary, int secondary);

/* Local variables to this file */

int GMT_n_anotations[4] = {0, 0, 0, 0};
int GMT_alloc_anotations[4] = {GMT_SMALL_CHUNK, GMT_SMALL_CHUNK, GMT_SMALL_CHUNK, GMT_SMALL_CHUNK};
double *GMT_x_anotation[4], *GMT_y_anotation[4];

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
	
	if (tframe_info.side[3])	{	/* West or left y-axis */
		if (project_info.xyz_projection[1] == TIME)
			GMT_ty_axis (x1, y1, y_length, s, n, &tframe_info.axis[1], TRUE, tframe_info.side[3]-1);
		else
			GMT_y_axis (x1, y1, y_length, s, n, &tframe_info.axis[1], TRUE, tframe_info.side[3]-1);
	}
	if (tframe_info.side[1])	{	/* East or right y-axis */
		if (project_info.xyz_projection[1] == TIME)
			GMT_ty_axis (x2, y1, y_length, s, n, &tframe_info.axis[1], FALSE, tframe_info.side[1]-1);
		else
			GMT_y_axis (x2, y1, y_length, s, n, &tframe_info.axis[1], FALSE, tframe_info.side[1]-1);
	}
	if (tframe_info.side[0])	/* South or lower x-axis */
		GMT_x_axis (x1, y1, x_length, w, e, &tframe_info.axis[0], TRUE, tframe_info.side[0]-1);
	if (tframe_info.side[2])	/* North or upper x-axis */
		GMT_x_axis (x1, y2, x_length, w, e, &tframe_info.axis[0], FALSE, tframe_info.side[2]-1);
}

void GMT_x_axis (double x0, double y0, double length, double val0, double val1, struct TIME_AXIS *A, int below, int anotate)
{
	int k, i, nx, np = 0, flip, justify, label_justify, anot_pos, font_size, primary = 0, secondary = 0;
	BOOLEAN is_interval, both, check_anotation;
	double t_mid, *knots, *knots_p, x, tick_len[5], sign, anot_off[2], label_off, len, t_use;
	struct TIME_AXIS_ITEM *T;
	char string[GMT_CALSTRING_LENGTH], format[32];
	
	/* Ready to draw axis */
	
	ps_setfont (gmtdefs.anot_font);
	if (below) ps_comment ("Start of lower x-axis"); else ps_comment ("Start of upper x-axis");
	ps_transrotate (x0, y0, 0.0);
	GMT_setpen (&gmtdefs.frame_pen);
	ps_plot (0.0, 0.0, 3);
	ps_plot (length, 0.0, -2);
	GMT_setpen (&gmtdefs.tick_pen);

	if (A->type != TIME) GMT_get_format (GMT_get_map_interval (0, 'a'), A->unit, format);
	both = GMT_upper_and_lower_items(0);		/* Two levels of anotations */
	check_anotation = GMT_two_anot_items(0);	/* If both are tick anotations we must sometimes skip the lower unit anotation when they overlap */
	if (check_anotation) {	/* Precalculate both sets of knots */
		GMT_get_primary_anot (A, &primary, &secondary);
		np = GMT_coordinate_array (val0, val1, &A->item[primary], &knots_p);	/* Get all the primary tick anotation knots */
	}

	len = (gmtdefs.tick_length > 0.0) ? gmtdefs.tick_length : 0.0;
	sign = (below) ? -1.0 : 1.0;
	anot_off[0] = GMT_get_anot_offset (&flip);
	anot_off[1] = anot_off[0] + (gmtdefs.anot_font_size * GMT_u2u[GMT_PT][GMT_INCH]) + 0.5 * fabs (gmtdefs.anot_offset);
	justify = label_justify = (below) ? 10 : 2;
	if (flip) justify = GMT_flip_justify (justify);
	if (both)
		label_off = sign * (((flip) ? len : anot_off[1] + (gmtdefs.anot_font2_size * GMT_u2u[GMT_PT][GMT_INCH]) * GMT_font_height[gmtdefs.anot_font2]) + 1.5 * fabs (gmtdefs.anot_offset));
	else
		label_off = sign * (((flip) ? len : anot_off[0] + (gmtdefs.anot_font_size * GMT_u2u[GMT_PT][GMT_INCH]) * GMT_font_height[gmtdefs.anot_font]) + 1.5 * fabs (gmtdefs.anot_offset));
	anot_off[0] *= sign;
	anot_off[1] *= sign;
	tick_len[0] = tick_len[2] = sign * gmtdefs.tick_length;
	tick_len[1] = 3.0 * sign * gmtdefs.tick_length;
	tick_len[3] = (A->item[GMT_ANOT_UPPER].active) ? tick_len[0] : 3.0 * sign * gmtdefs.tick_length;
	tick_len[4] = 0.5 * sign * gmtdefs.tick_length;
	
	for (k = 0; k < GMT_GRID_UPPER; k++) {	/* For each one of the 5 axis items (gridlines are done separately) */
		
		T = &A->item[k];		/* Get pointer to this item */
		if (!T->active) continue;	/* Don't want this item plotted - goto next item */
		
		is_interval = GMT_interval_axis_item(k);
		nx = GMT_coordinate_array (val0, val1, &A->item[k], &knots);	/* Get all the tick knots */
		
		/* First plot all the tick marks */
		
		for (i = 0; i < nx; i++) {
			if (knots[i] < (val0 - GMT_CONV_LIMIT) || knots[i] > (val1 + GMT_CONV_LIMIT)) continue;
			GMT_coordinate_to_x (knots[i], &x);
			ps_plot (x, 0.0, 3);
			ps_plotr (0.0, tick_len[k], -2);
		}
		
		if (k < GMT_TICK_UPPER && anotate) {	/* Then do anotations */
		
			anot_pos = GMT_lower_axis_item(k);						/* 1 means lower anotation, 0 means upper (close to axis) */
			font_size = (anot_pos == 1) ? gmtdefs.anot_font2_size : gmtdefs.anot_font_size;
		
			for (i = 0; k < 4 && i < (nx - is_interval); i++) {
				if (GMT_anot_pos (val0, val1, T, &knots[i], &t_use)) continue;				/* Outside range */
				if (GMT_skip_second_anot (k, knots[i], knots_p, np, primary, secondary)) continue;	/* Secondary anotation skipped when coinciding with primary anotation */
				GMT_coordinate_to_x (t_use, &x);							/* Get anotation position */
				GMT_get_coordinate_label (string, &GMT_plot_calclock, format, T, knots[i]);		/* Get anotation string */
				ps_text (x, anot_off[anot_pos], font_size, string, 0.0, justify, 0);			/* Plot anotation */
			}
		}
			
		if (nx) GMT_free ((void *)knots);
	}
	if (np) GMT_free ((void *)knots_p);
	
	/* Finally do label */
	
	ps_setfont (gmtdefs.label_font);
	if (A->label[0] && anotate) ps_text (0.5 * length, label_off, gmtdefs.label_font_size, A->label, 0.0, label_justify, 0);
	ps_rotatetrans  (-x0, -y0, 0.0);
	if (below) ps_comment ("End of lower x-axis"); else ps_comment ("End of upper x-axis");
}

void GMT_get_primary_anot (struct TIME_AXIS *A, int *primary, int *secondary)
{	/* Return the primary and secondary anotation item numbers */
	
	int i, no[2] = {GMT_ANOT_UPPER, GMT_ANOT_LOWER};
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
		*primary = GMT_ANOT_UPPER;
		*secondary = GMT_ANOT_LOWER;
	}
	else {
		*primary = GMT_ANOT_LOWER;
		*secondary = GMT_ANOT_UPPER;
	}
}

BOOLEAN GMT_skip_second_anot (int item, double x, double x2[], int n, int primary, int secondary)
{
	int i;
	double small;
	BOOLEAN found;
	
	if (primary == secondary) return (FALSE);	/* Not set, no need to skip */
	if (secondary != item) return (FALSE);		/* Not working on secondary anotation */
	
	small = (x2[1] - x2[0]) * GMT_CONV_LIMIT;
	for (i = 0, found = FALSE; !found && i < n; i++) found = (fabs (x2[i] - x) < small);
	return (found);
}
	
int GMT_anot_pos (double min, double max, struct TIME_AXIS_ITEM *T, double coord[], double *pos)
{
	/* Calculates the location of the next anotation in user units.  This is
	 * trivial for tick anotations but can be tricky for interval anotations
	 * since the anotation location is not necessarily centered on the interval.
	 * For instance, if our interval is 3 months we do not want "January" centered
	 * on that quarter.  If the position is outside our range we return TRUE
	 */
	 
	if (GMT_interval_axis_item(T->id)) {
		double half_width, start, stop;
		if ((T->unit == 'o' || T->unit == 'O' || T->unit == 'k' || T->unit == 'K') && T->interval != 1.0) {	/* Must find next month to get month centered correctly */
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
		if ((stop - start) < half_width) return (TRUE);		/* Sorry, fraction not large enough to anotate */
		*pos = 0.5 * (start + stop);				/* Set half-way point */
		if (((*pos) - GMT_CONV_LIMIT) < min || ((*pos) + GMT_CONV_LIMIT) > max) return (TRUE);	/* Outside axis range */
	}
	else if (coord[0] < (min - GMT_CONV_LIMIT) || coord[0] > (max + GMT_CONV_LIMIT))		/* Outside axis range */
		return (TRUE);
	else
		*pos = coord[0];

	return (FALSE);
}

void GMT_get_time_label (char *string, struct GMT_PLOT_CALCLOCK *P, struct TIME_AXIS_ITEM *T, GMT_dtime t)
{	/* Assemble the anotation label given the formatting options presented */
	struct GMT_gcal calendar;

	GMT_gcal_from_dt (t, &calendar);			/* Convert t to a complete calendar structure */

	switch (T->unit) {
		case 'Y':	/* 4-digit integer year */
			(P->date.compact) ? sprintf (string, "%d\0", calendar.year) : sprintf (string, "%4.4d\0", calendar.year);
			break;
		case 'y':	/* 2-digit integer year */
			(P->date.compact) ? sprintf (string, "%d\0", calendar.year % 100) : sprintf (string, "%2.2d\0", calendar.year % 100);
			break;
		case 'O':	/* Plot via date format */
			GMT_format_calendar (string, CNULL, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'o':	/* 2-digit month */
			(P->date.compact) ? sprintf (string, "%d\0", calendar.month) : sprintf (string, "%2.2d\0", calendar.month);
			break;
		case 'U':	/* ISO year, week, day via date format */
			GMT_format_calendar (string, CNULL, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'u':	/* 2-digit ISO week */		
			(P->date.compact) ? sprintf (string, "%d\0", calendar.iso_w) : sprintf (string, "%2.2d\0", calendar.iso_w);
			break;
		case 'K':	/*  Weekday name */
			if (T->upper_case) GMT_str_toupper (GMT_time_language.day_name[calendar.iso_d%7][T->flavor]);
			sprintf (string, "%s\0", GMT_time_language.day_name[calendar.iso_d%7][T->flavor]);
			break;
		case 'k':	/* Day of the month */
			(P->date.compact) ? sprintf (string, "%d\0", calendar.day_m) : sprintf (string, "%2.2d\0", calendar.day_m);
			break;
		case 'D':	/* Day, via date format */
			GMT_format_calendar (string, CNULL, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'd':	/* 2-digit day or 3-digit day of year */
			if (P->date.day_of_year)
				(P->date.compact) ? sprintf (string, "%d\0", calendar.day_y) : sprintf (string, "%3.3d\0", calendar.day_y);
			else
				(P->date.compact) ? sprintf (string, "%d\0", calendar.day_m) : sprintf (string, "%2.2d\0", calendar.day_m);
			break;
		case 'H':	/* Hours via clock format */
			GMT_format_calendar (CNULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'h':	/* 2-digit hour */
			(P->date.compact) ? sprintf (string, "%d\0", calendar.hour) : sprintf (string, "%2.2d\0", calendar.hour);
			break;
		case 'M':	/* Minutes via clock format */
			GMT_format_calendar (CNULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'm':	/* 2-digit minutes */
			(P->date.compact) ? sprintf (string, "%d\0", calendar.min) : sprintf (string, "%2.2d\0", calendar.min);
			break;
		case 'C':	/* Seconds via clock format */
			GMT_format_calendar (CNULL, string, &P->date, &P->clock, T->upper_case, T->flavor, t);
			break;
		case 'c':	/* 2-digit seconds */
			(P->date.compact) ? sprintf (string, "%d\0", calendar.sec) : sprintf (string, "%2.2d\0", calendar.sec);
			break;
		default:
			fprintf (stderr, "ERROR: wrong unit passed to GMT_get_time_label\n");
			exit (EXIT_FAILURE);
			break;
	}
}
		
void GMT_get_coordinate_label (char *string, struct GMT_PLOT_CALCLOCK *P, char *format, struct TIME_AXIS_ITEM *T, double coord)
{
	/* Returns the formatted anotation string for the non-geographic axes */
	
	double tmp;
	
	switch (tframe_info.axis[T->parent].type) {
		case LINEAR:
			sprintf (string, format, coord);
			break;
		case LOG10:
			sprintf (string, "%d\0", irint (d_log10 (coord)));
			break;
		case POW:
			if (project_info.xyz_projection[T->parent] == POW) {
				if (T->parent == 0)
					(*GMT_x_inverse) (&tmp, coord);
				else
					(*GMT_y_inverse) (&tmp, coord);
				sprintf (string, format, tmp);
			}
			else
				sprintf (string, "10@+%d@+\0", irint (d_log10 (coord)));
			break;
		case TIME:
			GMT_get_time_label (string, P, T, coord);
			break;
		default:
			fprintf (stderr, "GMT ERROR: Wrong type passed to GMT_get_coordinate_label!\n", tframe_info.axis[T->parent].type);
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
	
void GMT_ty_axis (double x0, double y0, double length, double val0, double val1, struct TIME_AXIS *A, int left_side, int anotate)
{
	fprintf (stderr, "%s: GMT_ty_axis not implemented yet\n", GMT_program);
}

void GMT_y_axis (double x0, double y0, double length, double val0, double val1, struct TIME_AXIS *A, int left_side, int anotate)
{
	int i, n , anot_justify, label_justify, n_anotations = 0, n_tickmarks = 0, ndec;
	
	BOOLEAN left = FALSE, do_anot, do_tick, flip, as_is;
	
	double v0, v1, anot_off, label_off, sign, len, dy, tmp, x, off, angle, tmp_offset, *xpos;
	
	char annotation[256], text_l[256], text_u[256], format[32];
	
	double anotation_int, tickmark_int;
		
	anotation_int = GMT_get_map_interval (1, 'a');
	tickmark_int  = GMT_get_map_interval (1, 'f');
	
	ps_setfont (gmtdefs.anot_font);
	sign = (left_side) ? 1.0 : -1.0;
	len = (gmtdefs.tick_length > 0.0) ? gmtdefs.tick_length : 0.0;
	dy = sign * gmtdefs.tick_length;
	
	do_anot = (anotation_int > 0.0);
	do_tick = (tickmark_int > 0.0);
	
	ndec = GMT_get_format (anotation_int, A->unit, format);
	as_is = (ndec == 0 && !strchr (format, 'g'));	/* Use the d_format as is */
	/* Ready to draw axis */
	
	if (left_side) ps_comment ("Start of left y-axis"); else ps_comment ("Start of right y-axis");
	ps_transrotate (x0, y0, 90.0);
	GMT_setpen (&gmtdefs.frame_pen);
	ps_plot (0.0, 0.0, 3);
	ps_plot (length, 0.0, -2);
	GMT_setpen (&gmtdefs.tick_pen);
	
	switch (project_info.xyz_projection[1]) {
		case POW:
			if (as_is) {
				sprintf (text_l, format, fabs (val0));
				sprintf (text_u, format, fabs (val1));
			}
			else {
				sprintf (text_l, "%d\0", (int)floor (val0));
				sprintf (text_u, "%d\0", (int)ceil (val1));
			}
			break;
		case LOG10:
			v0 = d_log10 (val0);
			v1 = d_log10 (val1);
			if (A->type == 2) {	/* 10 ^ pow annotations */
				sprintf (text_l, "10%d\0", (int)floor (v0));
				sprintf (text_u, "10%d\0", (int)ceil (v1));
			}
			else {
				if (as_is) {
					sprintf (text_l, format, fabs (val0));
					sprintf (text_u, format, fabs (val1));
				}
				else if (A->type == 1) {
					sprintf (text_l, "%d\0", (int)floor (v0));
					sprintf (text_u, "%d\0", (int)ceil (v1));
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
				sprintf (text_l, "%d\0", (int)floor (fabs (val0)));
				sprintf (text_u, "%d\0", (int)ceil (fabs (val1)));
			}
			break;
	}
	
	/* Find offset based on no of digits before and after a period, if any */
	
	off = ((MAX ((int)strlen (text_l), (int)strlen (text_u)) + ndec) * 0.49 + ((ndec > 0) ? 0.3 : 0.0) + ((val0 < 0.0) ? 0.3 : 0.0))
		* gmtdefs.anot_font_size * GMT_u2u[GMT_PT][GMT_INCH];
	
	tmp_offset = GMT_get_anot_offset (&flip);
	if (A->unit && A->unit[0] && gmtdefs.y_axis_type == 0) {	/* Accomodate extra width of anotation */
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
		off += (u_len - n_comp) * 0.49 * gmtdefs.anot_font_size * GMT_u2u[GMT_PT][GMT_INCH];
	}
	label_justify = (left_side) ? 2 : 10;
	if (gmtdefs.y_axis_type == 0) {	/* Horizontal anotations */
		anot_justify = 7;
		anot_off = sign * tmp_offset;
		label_off = sign * (((flip) ? len : tmp_offset + off) + 1.5 * fabs (gmtdefs.anot_offset));
		if ((left_side + flip) != 1) anot_off -= off;
		angle = -90.0;
	}
	else {
		anot_off = sign * tmp_offset;
		label_off = sign * (((flip) ? len : tmp_offset + (gmtdefs.anot_font_size * GMT_u2u[GMT_PT][GMT_INCH]) * GMT_font_height[gmtdefs.anot_font]) + 1.5 * fabs (gmtdefs.anot_offset));
		anot_justify = (left_side) ? 2 : 10;
		angle = 0.0;
		if (flip) anot_justify = GMT_flip_justify (anot_justify);
	}
	
	/* Do anotations */
	
	n = GMT_coordinate_array (val0, val1, &A->item[0], &xpos);
	for (i = 0; anotate && i < n; i++) {
		GMT_coordinate_to_y (xpos[i], &x);
		GMT_get_coordinate_label (annotation, NULL, format, &A->item[0], xpos[i]);
		ps_plot (x, 0.0, 3);
		ps_plot (x, dy, -2);
		if (anotate) ps_text (x, anot_off, gmtdefs.anot_font_size, annotation, angle, anot_justify, 0);
	}
	if (n) GMT_free ((void *) xpos);

	/* Now do frame tickmarks */
	
	dy *= 0.5;
	
	n = GMT_coordinate_array (val0, val1, &A->item[4], &xpos);
	for (i = 0; i < n; i++) {
		GMT_coordinate_to_y (xpos[i], &x);
		ps_plot (x, 0.0, 3);
		ps_plot (x, dy, -2);
	}
	if (n) GMT_free ((void *) xpos);

	/* Finally do label */
	
	ps_setfont (gmtdefs.label_font);
	if (A->label && anotate) ps_text (0.5 * length, label_off, gmtdefs.label_font_size, A->label, 0.0, label_justify, 0);
	ps_rotatetrans  (-x0, -y0, -90.0);
	ps_comment ("End of y-axis");
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
	i = n = 0;
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
	
	while (val[n] <= end_val) {
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

	val = (double *) GMT_memory ((void *)val, (size_t)n, sizeof (double), "GMT_log_array");

	*array = val;

	return (n);
}

int GMT_pow_array (double min, double max, double delta, int x_or_y, double **array)
{
	int anottype, n, n_alloc = GMT_SMALL_CHUNK;
	double *val, tval, v0, v1, small, start_val, end_val;
	PFI fwd, inv;
	
	if (delta == 0.0) return (0);
	val = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_pow_array");

	anottype = (tframe_info.axis[x_or_y].type == 2) ? 2 : 0;
	if (x_or_y == 0) { /* x-axis */
		fwd = GMT_x_forward;
		inv = GMT_x_inverse;
	}
	else {	/* y-axis */
		fwd = GMT_y_forward;
		inv = GMT_y_inverse;
	}

	small = SMALL * delta;
	if (anottype == 2) {
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
		if (anottype == 2) {
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

	val = (double *) GMT_memory ((void *)val, (size_t)n, sizeof (double), "GMT_log_array");

	*array = val;

	return (n);
}

int GMT_time_array (double min, double max, struct TIME_AXIS_ITEM *T, double **array)
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

int GMT_coordinate_array (double min, double max, struct TIME_AXIS_ITEM *T, double **array)
{
	switch (project_info.xyz_projection[T->parent]) {
		case LINEAR:
			GMT_linear_array (min, max, T->interval, array);
			break;
		case LOG10:
			GMT_log_array (min, max, T->interval, array);
			break;
		case POW:
			GMT_pow_array (min, max, T->interval, T->parent, array);
			break;
		case TIME:
			GMT_time_array (min, max, T, array);
			break;
		default:
			fprintf (stderr, "GMT ERROR: Invalid projection type (%d) passed to GMT_coordinate_array!\n", project_info.xyz_projection[T->parent]);
			exit (EXIT_FAILURE);
			break;
	}
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
		
	nx = GMT_time_array (w, e, &tframe_info.axis[0].item[5], &x);
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
		
	ny = GMT_time_array (s, n, &tframe_info.axis[1].item[5], &y);
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
	
	if (gmtdefs.basemap_type == IS_PLAIN) {	/* Draw plain boundary and return */
		GMT_wesn_map_boundary (w, e, s, n);
		return;
	}
	
	ps_setpaint (gmtdefs.basemap_frame_rgb);

	fat_pen = irint (gmtdefs.frame_width * gmtdefs.dpi);
	thin_pen = irint (0.1 * gmtdefs.frame_width * gmtdefs.dpi);
	sign_x = (project_info.xyz_pos[0]) ? +1.0 : -1.0;
	sign_y = (project_info.xyz_pos[1]) ? +1.0 : -1.0;
	
	ps_setline (thin_pen);
	if (tframe_info.side[3]) {	/* Draw western boundary */
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
	if (tframe_info.side[1]) {	/* Draw eastern boundary */
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
	if (tframe_info.side[0]) {	/* Draw southern boundary */
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
	if (tframe_info.side[2]) {	/* Draw northern boundary */
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
	if ((dy = GMT_get_map_interval (1,'f')) != 0.0) {
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
				if (tframe_info.side[3]) {
					GMT_geo_to_xy (w, v2, &x3, &y3);
					ps_plot (x1-sign_x*gmtdefs.frame_width, y1, 3);
					ps_plot (x3-sign_x*gmtdefs.frame_width, y3, -2);
				}
				if (tframe_info.side[1]) {
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
	
	if ((dx = GMT_get_map_interval (0,'f')) != 0.0) {
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
				if (tframe_info.side[0]) {
					GMT_geo_to_xy (v2, s, &x3, &y3);
					ps_plot (x1, y1-sign_y*gmtdefs.frame_width, 3);
					ps_plot (x3, y3-sign_y*gmtdefs.frame_width, -2);
				}
				if (tframe_info.side[2]) {
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
		tframe_info.side[0] = FALSE;
	if (project_info.north_pole && n >= 90.0) /* Cannot have northern boundary */
		tframe_info.side[2] = FALSE;
	if (fabs (fabs (e-w) - 360.0) < GMT_CONV_LIMIT || fabs (e - w) < GMT_CONV_LIMIT) {
		tframe_info.side[1] = FALSE;
		tframe_info.side[3] = FALSE;
	}
	
	if (gmtdefs.basemap_type == IS_PLAIN) { /* Draw plain boundary and return */
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
	
	if (tframe_info.side[3]) {	/* Draw western boundary */
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
	if (tframe_info.side[1]) {	/* Draw eastern boundary */
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
	if (tframe_info.side[0]) {	/* Draw southern boundary */
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
	if (tframe_info.side[2]) {	/* Draw northern boundary */
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
	
	/* Anotate S-N axes */
	
	ps_setline (fat_pen);
	if ((dy = GMT_get_map_interval (1,'f')) != 0.0) {
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
				if (tframe_info.side[3]) {
					GMT_geo_to_xy (w, v2, &x3, &y3);
					ps_plot (x1+0.5*dx2w, y1+0.5*dy2w, 3);
					ps_plot (x3+0.5*dx2w, y3+0.5*dy2w, -2);
				}
				if (tframe_info.side[1]) {
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

	/* Anotate W-E axes */
	
	if ((dx = GMT_get_map_interval (0,'f')) != 0.0) {
		shade = ((int)floor (w / dx) + 1) % 2;
		w1 = floor(w/dx) * dx;
		nx = (w1 > e) ? -1 : (int)((e-w1) / dx + SMALL);
		for (i = 0; i <= nx; i++) {
			val = w1 + i * dx;
			v1 = (val < w) ? w : val;
			if (shade) {
				v2 = val + dx;
				if (v2 > e) v2 = e;
				if (tframe_info.side[0]) {
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
				if (tframe_info.side[2]) {
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
	if (gmtdefs.basemap_type == IS_PLAIN) { /* Draw plain boundary and return */
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
	
	if (tframe_info.side[3]) {	/* Draw western boundary */
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
	if (tframe_info.side[1]) {	/* Draw eastern boundary */
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
	if (tframe_info.side[0]) {	/* Draw southern boundary */
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
	if (tframe_info.side[2]) {	/* Draw northern boundary */
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
	
	/* Anotate S-N axes */
	
	ps_setline (fat_pen);
	if ((y_inc = GMT_get_map_interval (1,'f')) != 0.0) {
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
				if (tframe_info.side[3]) {
					GMT_geo_to_xy (w, v2, &x3, &y3);
					ps_plot (x1-0.5*dx2, y1-0.5*dy2, 3);
					ps_plot (x3-0.5*dx2, y3-0.5*dy2, -2);
				}
				if (tframe_info.side[1]) {
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

	/* Anotate W-E axes */
	
	if ((x_inc = GMT_get_map_interval (0,'f')) != 0.0) {
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
				if (tframe_info.side[0]) {
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
				if (tframe_info.side[2]) {
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
		tframe_info.side[0] = FALSE;
	if (project_info.n >= 90.0) /* Cannot have northern boundary */
		tframe_info.side[2] = FALSE;

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
	
	if (tframe_info.side[3]) {	/* West */
		np = GMT_map_path (w, s, w, n, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geoz_to_xy (xx[i], yy[i], project_info.z_level, &xx[i], &yy[i]);
		ps_line (xx, yy, np, 3, FALSE, TRUE);
		GMT_free ((void *)xx);	GMT_free ((void *)yy);
	}
	if (tframe_info.side[1]) {	/* East */
		np = GMT_map_path (e, s, e, n, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geoz_to_xy (xx[i], yy[i], project_info.z_level, &xx[i], &yy[i]);
		ps_line (xx, yy, np, 3, FALSE, TRUE);
		GMT_free ((void *)xx);	GMT_free ((void *)yy);
	}
	if (tframe_info.side[0]) {	/* South */
		np = GMT_map_path (w, s, e, s, &xx, &yy);
		for (i = 0; i < np; i++) GMT_geoz_to_xy (xx[i], yy[i], project_info.z_level, &xx[i], &yy[i]);
		ps_line (xx, yy, np, 3, FALSE, TRUE);
		GMT_free ((void *)xx);	GMT_free ((void *)yy);
	}
	if (tframe_info.side[2]) {	/* North */
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
	
	if (tframe_info.side[3]) {	/* West */
		ps_plot (xt[0], yt[0], 3);
		ps_plot (xt[3], yt[3], -2);
	}
	if (tframe_info.side[1]) {	/* East */
		ps_plot (xt[1], yt[1], 3);
		ps_plot (xt[2], yt[2], -2);
	}
	if (tframe_info.side[0]) {	/* South */
		ps_plot (xt[0], yt[0], 3);
		ps_plot (xt[1], yt[1], -2);
	}
	if (tframe_info.side[2]) {	/* North */
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
	
	if (fabs (s) < GMT_CONV_LIMIT) tframe_info.side[0] = 0;	/* No donuts, please */
	if (fabs (fabs (e-w) - 360.0) < GMT_CONV_LIMIT || fabs (e - w) < GMT_CONV_LIMIT) {
		tframe_info.side[1] = FALSE;
		tframe_info.side[3] = FALSE;
	}
	nr = gmtdefs.n_lon_nodes;
	if (nr >= GMT_n_alloc) GMT_get_plot_array ();
	da = fabs (project_info.e - project_info.w) / (nr - 1);
	if (tframe_info.side[2]) {
		for (i = 0; i < nr; i++) {
			a = project_info.w + i * da;
			GMT_geoz_to_xy (a, project_info.n, project_info.z_level, &GMT_x_plot[i], &GMT_y_plot[i]);
		}
		ps_line (GMT_x_plot, GMT_y_plot, nr, 3, FALSE, TRUE);
	}
	if (tframe_info.side[0]) {
		for (i = 0; i < nr; i++) {
			a = project_info.w + i * da;
			GMT_geoz_to_xy (a, project_info.s, project_info.z_level, &GMT_x_plot[i], &GMT_y_plot[i]);
		}
		ps_line (GMT_x_plot, GMT_y_plot, nr, 3, FALSE, TRUE);
	}	
	if (tframe_info.side[1]) {
		GMT_geoz_to_xy (project_info.e, project_info.s, project_info.z_level, &xx[0], &yy[0]);
		GMT_geoz_to_xy (project_info.e, project_info.n, project_info.z_level, &xx[1], &yy[1]);
		ps_line (xx, yy, 2, 3, FALSE, TRUE);
	}
	if (tframe_info.side[3]) {
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
	sprintf (text, "Lat = %lg\0", lat);
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
	sprintf (text, "Lon = %lg\0", lon);
	ps_comment (text);
	GMT_plot_line (GMT_x_plot, GMT_y_plot, GMT_pen, GMT_n_plot);
	
	GMT_free ((void *)llon);
	GMT_free ((void *)llat);
}

void GMT_map_lontick (double lon, double south, double north)
{
	int i, nc;
	struct XINGS *xings;
	
	nc = GMT_map_loncross (lon, south, north, &xings);
	for (i = 0; i < nc; i++) GMT_map_tick (xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, xings[i].nx, 0);
	if (nc) GMT_free ((void *)xings);
}

void GMT_map_lattick (double lat, double west, double east)
{
	int i, nc;
	
	struct XINGS *xings;
	
	nc = GMT_map_latcross (lat, west, east, &xings);
	for (i = 0; i < nc; i++) GMT_map_tick (xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, xings[i].nx, 1);
	if (nc) GMT_free ((void *)xings);
}

void GMT_map_tick (double *xx, double *yy, int *sides, double *angles, int nx, int type)
{
	double angle, xl, yl, xt, yt, c, s, tick_length;
	int i;
	
	for (i = 0; i < nx; i++) {
		if (!project_info.edge[sides[i]]) continue;
		if (!tframe_info.side[sides[i]]) continue;
		if (!(gmtdefs.oblique_anotation & 1) && ((type == 0 && (sides[i] % 2)) || (type == 1 && !(sides[i] % 2)))) continue;
		angle = ((gmtdefs.oblique_anotation & 16) ? (sides[i] - 1) * 90.0 : angles[i]) * D2R;
		sincos (angle, &s, &c);
		tick_length = gmtdefs.tick_length;
		if (gmtdefs.oblique_anotation & 8) {
			if (sides[i] % 2) {
				if (fabs (c) > cosd (gmtdefs.anot_min_angle)) continue;
				tick_length /= fabs(c);
			}
			else {
				if (fabs (s) < sind (gmtdefs.anot_min_angle)) continue;
				tick_length /= fabs(s);
			}
		}
		xl = 0.5 * tick_length * c;
		yl = 0.5 * tick_length * s;
		GMT_xy_do_z_to_xy (xx[i], yy[i], project_info.z_level, &xt, &yt);
		ps_plot (xt, yt, 3);
		GMT_xy_do_z_to_xy (xx[i]+xl, yy[i]+yl, project_info.z_level, &xt, &yt);
		ps_plot (xt, yt, -2);
	}
}

void GMT_map_symbol_ew (double lat, char *label, double west, double east, BOOLEAN anot)
{
	int i, nc;
	struct XINGS *xings;
	
	nc = GMT_map_latcross (lat, west, east, &xings);
	for (i = 0; i < nc; i++) GMT_map_symbol (xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, label, xings[i].nx, 1, anot);
	if (nc) GMT_free ((void *)xings);
}

void GMT_map_symbol_ns (double lon, char *label, double south, double north, BOOLEAN anot)
{
	int i, nc;
	struct XINGS *xings;
	
	nc = GMT_map_loncross (lon, south, north, &xings);
	for (i = 0; i < nc; i++)  GMT_map_symbol (xings[i].xx, xings[i].yy, xings[i].sides, xings[i].angle, label, xings[i].nx, 0, anot);
	if (nc) GMT_free ((void *)xings);
}

void GMT_map_symbol (double *xx, double *yy, int *sides, double *line_angles, char *label, int nx, int type, BOOLEAN anot)
{
	/* type = 0 for lon and 1 for lat */
            
	double line_angle, text_angle, div, tick_length, o_len, len, dx, dy, angle, ca, sa, xt1, yt1, zz, tick_x[2], tick_y[2];
	int i, justify;
	BOOLEAN flip;
	char cmd[BUFSIZ];
	
	len = GMT_get_anot_offset (&flip);
	for (i = 0; i < nx; i++) {
	
		if (GMT_prepare_label (line_angles[i], sides[i], xx[i], yy[i], type, &line_angle, &text_angle, &justify)) continue;
		
		angle = line_angle * D2R;
		sincos (angle, &sa, &ca);
		tick_length = gmtdefs.tick_length;
		o_len = len;
		if ((type == 0 && gmtdefs.oblique_anotation & 2) || (type == 1 && gmtdefs.oblique_anotation & 4)) {
			o_len = tick_length;
		}
		if (gmtdefs.oblique_anotation & 8) {
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
		if ((type == 0 && gmtdefs.oblique_anotation & 2) || (type == 1 && gmtdefs.oblique_anotation & 4)) {
			if (sides[i] % 2 && gmtdefs.anot_offset > 0.0) xx[i] += (sides[i] == 1) ? gmtdefs.anot_offset : -gmtdefs.anot_offset;
			if (!(sides[i] % 2) && gmtdefs.anot_offset > 0.0) yy[i] += (sides[i] == 2) ? gmtdefs.anot_offset : -gmtdefs.anot_offset;
		}
		GMT_xyz_to_xy (xx[i], yy[i], zz, &xt1, &yt1);
			
		if (project_info.three_D) {
			int upside = FALSE, k;
			double xp[2], yp[2], xt2, xt3, yt2, yt3, del_y;
			double size, xsize, ysize, xshrink, yshrink, tilt, baseline_shift, cb, sb, a;
				
			upside = (z_project.quadrant == 1 || z_project.quadrant == 4);
			sincos (text_angle * D2R, &sb, &cb);
			if (sides[i]%2 == 0 && (justify%2 == 0)) {
				if (upside) {
					k = (sides[i] == 0) ? 2 : 10;
					text_angle += 180.0;
				}
				else
					k = justify;
				del_y = 0.5 * gmtdefs.anot_font_size * 0.732 * (k/4) * GMT_u2u[GMT_PT][GMT_INCH];
				justify = 2;
				xx[i] += del_y * ca;	yy[i] += del_y * sa;
				GMT_xyz_to_xy (xx[i], yy[i], zz, &xt1, &yt1);
			}
			else {
				del_y = -0.5 * gmtdefs.anot_font_size * 0.732 * (justify/4) * GMT_u2u[GMT_PT][GMT_INCH];
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
			size = gmtdefs.anot_font_size * gmtdefs.dpi * GMT_u2u[GMT_PT][GMT_INCH];
			xsize = size * xshrink;
			ysize = size * yshrink;
			/* Temporarely modify meaning of F0 */
			sprintf (cmd, "/F0 {pop /%s findfont [%lg 0 %lg %lg 0 0] makefont setfont} bind def\0",
				GMT_font_name[gmtdefs.anot_font], xsize, ysize * tilt, ysize);
			ps_command (cmd);
			ps_setfont (0);
			text_angle += (R2D * baseline_shift);
		}
		if (anot) {
			if (GMT_anot_too_crowded (xt1, yt1, sides[i])) continue;
			if (flip) justify = GMT_flip_justify (justify);
			ps_line (tick_x, tick_y, 2, 3, FALSE, TRUE);
			ps_text (xt1, yt1, gmtdefs.anot_font_size, label, text_angle, justify, 0);
		}
	}
}

BOOLEAN GMT_anot_too_crowded (double x, double y, int side) {
	/* Checks if the proposed anotation is too close to a previously plotted anotation */
	int i;
	double d_min;
	
	if (gmtdefs.anot_min_spacing <= 0.0) return (FALSE);
	
	for (i = 0, d_min = DBL_MAX; i < GMT_n_anotations[side]; i++) d_min = MIN (d_min, hypot (GMT_x_anotation[side][i] - x, GMT_y_anotation[side][i] - y));
	if (d_min < gmtdefs.anot_min_spacing) return (TRUE);
	
	/* OK to plot and add to list */
	
	GMT_x_anotation[side][GMT_n_anotations[side]] = x;
	GMT_y_anotation[side][GMT_n_anotations[side]] = y;
	GMT_n_anotations[side]++;
	
	if (GMT_n_anotations[side] == GMT_alloc_anotations[side]) {
		GMT_alloc_anotations[side] += GMT_SMALL_CHUNK;
		GMT_x_anotation[side] = (double *) GMT_memory ((void *)GMT_x_anotation[side], (size_t)GMT_alloc_anotations[side], sizeof (double), "GMT_anot_too_crowded");
		GMT_y_anotation[side] = (double *) GMT_memory ((void *)GMT_y_anotation[side], (size_t)GMT_alloc_anotations[side], sizeof (double), "GMT_anot_too_crowded");
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
	
	dx = GMT_get_map_interval (0, 'g');;
	dy = GMT_get_map_interval (1, 'g');;
	
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
	
	nx = GMT_coordinate_array (w, e, &tframe_info.axis[0].item[5], &x);
	ny = GMT_coordinate_array (s, n, &tframe_info.axis[1].item[5], &y);

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
	int i, nx, ny;
	double dx, dy, w1, s1, val;
	
	if (!(MAPPING || project_info.projection == POLAR) || gmtdefs.basemap_type == IS_FANCY) return;		/* Tickmarks already done by linear_axis or done in fancy ways */
	
	dx = GMT_get_map_interval (0, 'f');
	dy = GMT_get_map_interval (1, 'f');

	if (dx <= 0.0 && dy <= 0.0) return;

	ps_comment ("Map tickmarks");
	GMT_setpen (&gmtdefs.tick_pen);

	GMT_on_border_is_outside = TRUE;	/* Temporarily, points on the border are outside */
	
	if (dx > 0.0 && dx != GMT_get_map_interval (0, 'a')) {	/* Draw grid lines that go E to W */
		w1 = floor (w / dx) * dx;
		if (fabs (w1 - w) > SMALL) w1 += dx;
		nx = (w1 > e) ? -1 : (int)((e - w1) / dx + SMALL);
		for (i = 0; i <= nx; i++) {
			val = w1 + i * dx;
			if (val > e) val = e;
			GMT_map_lontick (val, s, n);
		}
	}
	
	if (dy > 0.0 && dy != GMT_get_map_interval (1, 'a')) {	/* Draw grid lines that go S to N */
		s1 = floor (s / dy) * dy;
		if (fabs (s1 - s) > SMALL) s1 += dy;
		ny = (s1 > n) ? -1 : (int)((n - s1) / dy + SMALL);
		for (i = 0; i <= ny; i++) {
			val = s1 + i * dy;
			if (val > n) val = n;
			GMT_map_lattick (val, w, e);
		}
	}
	
	GMT_on_border_is_outside = FALSE;	/* Reset back to default */

	if (gmtdefs.tick_pen.texture) ps_setdash (CNULL, 0);
}

void GMT_map_anotate (double w, double e, double s, double n)
{
	double s1, w1, val, dx, dy, x, y;
	int do_minutes, do_seconds, move_up, i, nx, ny, done_zero = FALSE, anot, GMT_world_map_save;
	char label[256], cmd[256];
	
	dx = (project_info.edge[0] || project_info.edge[2]) ? GMT_get_map_interval (0, 'a') : 0.0;
	dy = (project_info.edge[1] || project_info.edge[3]) ? GMT_get_map_interval (1, 'a') : 0.0;

	if (!tframe_info.header[0] && dx <= 0.0 && dy <= 0.0) return;

	ps_setpaint (gmtdefs.basemap_frame_rgb);

	if (tframe_info.header[0]) {	/* Make plot header */
		move_up = (MAPPING || tframe_info.side[2] == 2);
		ps_setfont (gmtdefs.header_font);
		x = project_info.xmax * 0.5;
		y = project_info.ymax + ((gmtdefs.tick_length > 0.0) ? gmtdefs.tick_length : 0.0) + 2.5 * gmtdefs.anot_offset;
		y += ((move_up) ? (gmtdefs.anot_font_size + gmtdefs.label_font_size) * GMT_u2u[GMT_PT][GMT_INCH] : 0.0) + 2.5 * gmtdefs.anot_offset;
		if (project_info.three_D && fabs (project_info.z_scale) < GMT_CONV_LIMIT) {	/* Only do this if flat 2-D plot */
			double size, xsize, ysize;
			
			ps_setfont (0);
			GMT_xy_do_z_to_xy (x, y, project_info.z_level, &x, &y);
			size = gmtdefs.header_font_size * gmtdefs.dpi * GMT_u2u[GMT_PT][GMT_INCH];
			xsize = size * z_project.xshrink[0];
			ysize = size * z_project.yshrink[0];
			sprintf (cmd, "/F0 {pop /%s findfont [%lg 0 %lg %lg 0 0] makefont setfont} bind def\0",
				GMT_font_name[gmtdefs.header_font], xsize, ysize * z_project.tilt[0], ysize);
			ps_command (cmd);
			
			ps_text (x, y, gmtdefs.header_font_size, tframe_info.header, z_project.phi[0], -2, 0);
			ps_command ("/F0 {/Helvetica Y} bind def");	/* Reset definition of F0 */
			ps_setfont (gmtdefs.header_font);
		}
		else if (!project_info.three_D)
			ps_text (x, y, gmtdefs.header_font_size, tframe_info.header, 0.0, -2, 0);
	}
	
	if (!(MAPPING || project_info.projection == POLAR)) return;	/* Annotation already done by linear_axis */
	
	ps_comment ("Map anotations");

	ps_setfont (gmtdefs.anot_font);
	GMT_setpen (&gmtdefs.tick_pen);
	
	GMT_on_border_is_outside = TRUE;	/* Temporarily, points on the border are outside */
	GMT_world_map_save = GMT_world_map;
	if (project_info.region) GMT_world_map = FALSE;
	
	if (dx > 0.0) {	/* Anotate the S and N boundaries */
		BOOLEAN full_lat_range, proj_A, proj_B, anot_0_and_360;
		
		/* Determine if we should annotate both 0 and 360 degrees */
		
		full_lat_range = (fabs (180.0 - fabs (project_info.n - project_info.s)) < SMALL);
		proj_A = (project_info.projection == MERCATOR || project_info.projection == OBLIQUE_MERC ||
			project_info.projection == WINKEL || project_info.projection == ECKERT4 || project_info.projection == ECKERT6 ||
			project_info.projection == ROBINSON || project_info.projection == CYL_EQ ||
			project_info.projection == CYL_EQDIST || project_info.projection == MILLER || project_info.projection == LINEAR);
		proj_B = (project_info.projection == HAMMER || project_info.projection == MOLLWEIDE ||
			project_info.projection == SINUSOIDAL);
/*		anot_0_and_360 = (GMT_world_map_save && ((full_lat_range && proj_A) || (!full_lat_range && proj_B))); */
		anot_0_and_360 = (GMT_world_map_save && (proj_A || (!full_lat_range && proj_B)));
		
		do_minutes = (fabs (fmod (dx, 1.0)) > SMALL);
		do_seconds = (fabs (60.0 * fmod (fmod (dx, 1.0) * 60.0, 1.0)) >= 1.0);
		w1 = floor (w / dx) * dx;
		if (fabs (w1 - w) > SMALL) w1 += dx;
		nx = (w1 > e) ? -1 : (int)((e - w1) / dx + SMALL);
		for (i = 0; i <= nx; i++) {
			val = w1 + i * dx;
			if (fabs (val) < GMT_CONV_LIMIT) done_zero = TRUE;
			if (val > e) val = e;
			GMT_get_anot_label (val, label, do_minutes, do_seconds, 0, GMT_world_map_save);
			anot = anot_0_and_360 || !(done_zero && fabs (val - 360.0) < GMT_CONV_LIMIT);
			GMT_map_symbol_ns (val, label, s, n, anot);
		}
	}
	
	if (dy > 0.0) {	/* Anotate W and E boundaries */
		int lonlat;
		
		if (MAPPING) {
			do_minutes = (fabs (fmod (dy, 1.0)) > SMALL);
			do_seconds = (fabs (60.0 * fmod (fmod (dy, 1.0) * 60.0, 1.0)) >= 1.0);
			lonlat = 1;
		}
		else {	/* Also, we know that gmtdefs.degree_format = -1 in this case */
			do_minutes = do_seconds = 0;
			lonlat = 2;
			if (project_info.got_azimuths) i_swap (tframe_info.side[1], tframe_info.side[3]);	/* Temporary swap to trick justify machinery */
		}
		s1 = floor (s / dy) * dy;
		if (fabs (s1 - s) > SMALL) s1 += dy;
		ny = (s1 > n) ? -1: (int)((n - s1) / dy + SMALL);
		for (i = 0; i <= ny; i++) {
			/* val = s1 + i * dy; */
			val = s1 + i * dy;
			if (val > n) val = n;
			if ((project_info.polar || project_info.projection == GRINTEN) && fabs (fabs (val) - 90.0) < GMT_CONV_LIMIT) continue;
			GMT_get_anot_label (val, label, do_minutes, do_seconds, lonlat, GMT_world_map_save);
			GMT_map_symbol_ew (val, label, w, e, TRUE);
		}
		if (project_info.got_azimuths) i_swap (tframe_info.side[1], tframe_info.side[3]);	/* Undo the temporary swap */
	}
	
	if (project_info.three_D) ps_command ("/F0 {/Helvetica Y} bind def");	/* Reset definition of F0 */
	
	GMT_on_border_is_outside = FALSE;	/* Reset back to default */
	if (project_info.region) GMT_world_map = GMT_world_map_save;
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
			fprintf (stderr, "%s: Error in GMT_is_fancy_boundary - notify developers\n");
			return (FALSE);
	}
}


/* GMT_map_basemap will create a basemap for the given area. 
 * Scaling and wesn are assumed to be passed throught the project_info-structure (see GMT_project.h)
 * Tickmark info are passed through the tframe_info-structure
 *
 */
 
void GMT_map_basemap (void) {
	int i;
	double w, e, s, n;

	if (!tframe_info.plot) return;

	ps_setpaint (gmtdefs.basemap_frame_rgb);
	
	w = project_info.w;	e = project_info.e;	s = project_info.s;	n = project_info.n;
	
	if (gmtdefs.oblique_anotation & 2) tframe_info.horizontal = 2;
	if (tframe_info.horizontal == 2) gmtdefs.oblique_anotation |= 2;
	for (i = 0; i < 4; i++) {
		GMT_x_anotation[i] = (double *) GMT_memory (VNULL, (size_t)GMT_alloc_anotations[i], sizeof (double), "GMT_map_basemap");
		GMT_y_anotation[i] = (double *) GMT_memory (VNULL, (size_t)GMT_alloc_anotations[i], sizeof (double), "GMT_map_basemap");
	}
	if (gmtdefs.basemap_type == IS_FANCY && !GMT_is_fancy_boundary()) gmtdefs.basemap_type = IS_PLAIN;
	
	ps_comment ("Start of basemap");

	ps_setdash (CNULL, 0);	/* To ensure no dashed pens are set prior */
	
	GMT_map_gridlines (w, e, s, n);
	GMT_map_gridcross (w, e, s, n);
	
	GMT_map_tickmarks (w, e, s, n);
	
	GMT_map_anotate (w, e, s, n);
	
	GMT_map_boundary (w, e, s, n);
	
	ps_comment ("End of basemap");

	for (i = 0; i < 4; i++) {
		GMT_free (GMT_x_anotation[i]);
		GMT_free (GMT_y_anotation[i]);
	}
}

void GMT_basemap_3D (int mode)
{
	/* Mode means: 1 = background axis, 2 = foreground axis, 3 = all */
	BOOLEAN go[4], back;
	int i;
	double x_anot, x_tick, y_anot, y_tick;
	
	back = (mode % 2);
	for (i = 0; i < 4; i++) go[i] = (mode == 3) ? TRUE : ((back) ? z_project.draw[i] : !z_project.draw[i]);
	
	if (go[0] && tframe_info.side[0])	/* South or lower x-axis */
		GMT_xyz_axis3D (0, 'x', &tframe_info.axis[0], tframe_info.side[0]-1);
	
	if (go[2] && tframe_info.side[2])	/* North or upper x-axis */
		GMT_xyz_axis3D (2, 'x',  &tframe_info.axis[0], tframe_info.side[2]-1);
	
	if (go[3] && tframe_info.side[3])	/* West or left y-axis */
		GMT_xyz_axis3D (3, 'y',  &tframe_info.axis[1], tframe_info.side[3]-1);
			
	if (go[1] && tframe_info.side[1])	/* East or right y-axis */
		GMT_xyz_axis3D (1, 'y',  &tframe_info.axis[1], tframe_info.side[1]-1);
		
}

void GMT_vertical_axis (int mode)
{
	/* Mode means: 1 = background axis, 2 = foreground axis, 3 = all */
	BOOLEAN go[4], fore, back;
	int i, j;
	double xp[2], yp[2], z_anot;
	
	if ((z_anot = GMT_get_map_interval (2, 'a')) == 0.0) return;

	fore = (mode > 1);	back = (mode % 2);
	for (i = 0; i < 4; i++) go[i] = (mode == 3) ? TRUE : ((back) ? z_project.draw[i] : !z_project.draw[i]);
	
	/* Vertical */
		
	if (fore && tframe_info.side[4]) GMT_xyz_axis3D (z_project.z_axis, 'z', &tframe_info.axis[2], tframe_info.side[4]-1);
			
	if (tframe_info.draw_box) {
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
	if (back && tframe_info.header[0]) {
		ps_setfont (gmtdefs.header_font);
		xp[0] = 0.5 * (z_project.xmin + z_project.xmax);
		yp[0] = z_project.ymax + 0.5;
		ps_text (xp[0], yp[0], gmtdefs.header_font_size, tframe_info.header, 0.0, -2, 0);
	}
}

void GMT_xyz_axis3D (int axis_no, char axis, struct TIME_AXIS *A, int anotate)
{
	int i, j, i_a, i_f, k, id, justify, n_anotations = 0, n_tickmarks = 0, test;
	
	BOOLEAN do_anot, do_tick;
	
	double val, v0, v1, anot_off, label_off, start_val_a, start_val_f, end_val;
	double tvals_a[9], tvals_f[9], sign, dy, tmp, xyz[3][2], len, x0, x1, y0, y1;
	double pp[3], w[3], xp, yp, del_y, val_xyz[3], phi, size, xsize, ysize;
	double start_log_a, start_log_f, val0, val1, small, anotation_int, tickmark_int;
	
	PFI xyz_forward, xyz_inverse;
	
	char annotation[256], format[32], cmd[256];
	
	int axistype;
	
	id = (axis == 'x') ? 0 : ((axis == 'y') ? 1 : 2);
	j = (id == 0) ? 1 : ((id == 1) ? 0 : z_project.k);
	xyz_forward = (PFI) ((id == 0) ? GMT_x_to_xx : ((id == 1) ? GMT_y_to_yy : GMT_z_to_zz));
	xyz_inverse = (PFI) ((id == 0) ? GMT_xx_to_x : ((id == 1) ? GMT_yy_to_y : GMT_zz_to_z));
	phi = (id < 2 && axis_no > 1) ? z_project.phi[id] + 180.0 : z_project.phi[id];
	axistype = project_info.xyz_projection[id];
	anotation_int = GMT_get_map_interval (id, 'a');
	tickmark_int  = GMT_get_map_interval (id, 'f');
	
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
	
	size = gmtdefs.anot_font_size * gmtdefs.dpi * GMT_u2u[GMT_PT][GMT_INCH];
	xsize = size * z_project.xshrink[id];
	ysize = size * z_project.yshrink[id];
	ps_command ("gsave\n");
	ps_comment ("Start of xyz-axis3D");
	/* Temporarely modify meaning of F0 */
	sprintf (cmd, "/F0 {pop /%s findfont [%lg 0 %lg %lg 0 0] makefont setfont} bind def\0",
		GMT_font_name[gmtdefs.anot_font], xsize, ysize * z_project.tilt[id], ysize);
	ps_command (cmd);
	ps_setfont (0);
	justify = (id == 2) ? 2 : 10;
	dy = sign * gmtdefs.tick_length;
	len = (gmtdefs.tick_length > 0.0) ? gmtdefs.tick_length : 0.0;
	anotation_int = fabs (anotation_int);
	tickmark_int = fabs (tickmark_int);
	
	do_anot = (anotation_int > 0.0);
	do_tick = (tickmark_int > 0.0);
	val0 = xyz[id][0];
	val1 = xyz[id][1];
	if (val0 > val1) d_swap (val0, val1);
	
	/* Find number of decimals needed, if any */
	
	GMT_get_format (anotation_int, A->unit, format);

	anot_off = sign * (len + gmtdefs.anot_offset);
	label_off = sign * (len + 2.5 * gmtdefs.anot_offset + (gmtdefs.anot_font_size * GMT_u2u[GMT_PT][GMT_INCH]) * GMT_font_height[gmtdefs.anot_font]);
	
	/* Ready to draw axis */
	
	GMT_setpen (&gmtdefs.frame_pen);
	ps_plot (x0, y0, 3);
	ps_plot (x1, y1, -2);
	GMT_setpen (&gmtdefs.tick_pen);
	
	i_a = i_f = 0;
	
	switch (axistype) {
		case POW:	/* Anotate in pow(x) */
			(*xyz_forward) (val0, &v0);
			(*xyz_forward) (val1, &v1);
			if (A->type == 2) {
				val = (anotation_int == 0.0) ? 0.0 : floor (v0 / anotation_int) * anotation_int;
				if (fabs (val - v0) > SMALL) val += anotation_int;
				start_val_a = val;	end_val = v1;
				val = (tickmark_int == 0.0) ? 0.0 : floor (v0 / tickmark_int) * tickmark_int;
				if (fabs (val - v0) > SMALL) val += tickmark_int;
				start_val_f = val;
			}
			else {
				val = (anotation_int == 0.0) ? 0.0 : floor (val0 / anotation_int) * anotation_int;
				if (fabs (val - val0) > SMALL) val += anotation_int;
				start_val_a = val;	end_val = val1;
				val = (tickmark_int == 0.0) ? 0.0 : floor (val0 / tickmark_int) * tickmark_int;
				if (fabs (val - val0) > SMALL) val += tickmark_int;
				start_val_f = val;
			}
			break;
		case LOG10:	/* Anotate in d_log10 (x) */
			v0 = d_log10 (val0);
			v1 = d_log10 (val1);
			val = pow (10.0, floor (v0));
			test = irint (anotation_int) - 1;
			if (test < 0 || test > 2) test = 0;
			if (test == 0) {
				n_anotations = 1;
				tvals_a[0] = 10.0;
				if (fabs (val - val0) > SMALL) val *= 10.0;
			}
			else if (test == 1) {
				tvals_a[0] = 1.0;
				tvals_a[1] = 2.0;
				tvals_a[2] = 5.0;
				n_anotations = 3;
			}
			else if (test == 2) {
				n_anotations = 9;
				for (i = 0; i < n_anotations; i++) tvals_a[i] = i + 1;
			}
			test = irint (tickmark_int) - 1;
			if (test < 0 || test > 2) test = 0;
			if (test == 0) {
				n_tickmarks = 1;
				tvals_f[0] = 10.0;
			}
			else if (test == 1) {
				tvals_f[0] = 1.0;
				tvals_f[1] = 2.0;
				tvals_f[2] = 5.0;
				n_tickmarks = 3;
			}
			else if (test == 2) {
				n_tickmarks = 9;
				for (i = 0; i < n_tickmarks; i++) tvals_f[i] = i + 1;
			}
			i_a = 0;
			start_log_a = val = pow (10.0, floor (v0));
			while ((v0 - d_log10 (val)) > SMALL) {
				if (i_a < n_anotations)
					val = start_log_a * tvals_a[i_a];
				else {
					val = (start_log_a *= 10.0);
					i_a = 0;
				}
				i_a++;
			}
			i_a--;
			start_val_a = val;
			i_f = 0;
			start_log_f = val = pow (10.0, floor (v0));
			while ((v0 - d_log10 (val)) > SMALL) {
				if (i_f < n_tickmarks)
					val = start_log_f * tvals_f[i_f];
				else {
					val = (start_log_f *= 10.0);
					i_f = 0;
				}
				i_f++;
			}
			i_f--;
			start_val_f = val;
			end_val = val1;
			break;
		case LINEAR:
			v0 = val0;
			v1 = val1;
			val = (anotation_int == 0.0) ? 0.0 : floor (val0 / anotation_int) * anotation_int;
			if (fabs (val - val0) > SMALL) val += anotation_int;
			start_val_a = val;	end_val = val1;
			val = (tickmark_int == 0.0) ? 0.0 : floor (val0 / tickmark_int) * tickmark_int;
			if (fabs (val - val0) > SMALL) val += tickmark_int;
			start_val_f = val;
			break;
	}
	
	del_y = 0.5 * sign * gmtdefs.anot_font_size * 0.732 * (justify/4) * GMT_u2u[GMT_PT][GMT_INCH];
	
	/* Do anotations with tickmarks */
	
	val_xyz[0] = z_project.corner_x[axis_no];
	val_xyz[1] = z_project.corner_y[axis_no];
	val_xyz[2] = project_info.z_level;
	val = (anotation_int == 0.0) ? end_val + 1.0 : start_val_a;
	small = (axistype != LOG10) ? SMALL * anotation_int : 0.0;
	while (do_anot && val <= (end_val + small)) {
	
		i_a++;
		
		val_xyz[id] = val;
		
		switch (A->type) {
			case 0:
				sprintf (annotation, format, val_xyz[id]);
				break;
			case 1:
				sprintf (annotation, "%d\0", irint (d_log10 (val_xyz[id])));
				break;
			case 2:
				if (axistype == POW) {
					(*xyz_inverse) (&tmp, val_xyz[id]);
					val_xyz[id] = tmp;
					sprintf (annotation, format, val_xyz[id]);
				}
				else
					sprintf (annotation, "10@+%d@+\0", irint (d_log10 (val_xyz[id])));
				break;
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
		pp[j] += anot_off -dy + del_y;
		GMT_xyz_to_xy (pp[0], pp[1], pp[2], &xp, &yp);
		if (anotate) {
			if (id < 2)
				ps_text (xp, yp, gmtdefs.anot_font_size, annotation, phi, 2, 0);
			else if (val != project_info.z_level)
				ps_text (xp, yp, gmtdefs.anot_font_size, annotation, phi, 2, 0);
		}
		
		if (axistype == LOG10) {
			if (i_a < n_anotations)
				val = start_log_a * tvals_a[i_a];
			else {
				val = (start_log_a *= 10.0);
				i_a = 0;
			}
		}
		else
			val = start_val_a + i_a * anotation_int;
			
	}

	/* Now do frame tickmarks */
	
	dy *= 0.5;
	
	val_xyz[0] = z_project.corner_x[axis_no];
	val_xyz[1] = z_project.corner_y[axis_no];
	val_xyz[2] = project_info.z_level;
	val = (tickmark_int == 0.0) ? end_val + 1.0 : start_val_f;
	while (do_tick && val <= (end_val + small)) {
	
		i_f++;
		
		val_xyz[id] = val;
		if (A->type == 2 && axistype == POW) {
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
		
		if (axistype == LOG10) {
			if (i_f < n_tickmarks)
				val = start_log_f * tvals_f[i_f];
			else {
				val = start_log_f *= 10.0;
				i_f = 0;
			}
		}
		else
			val = start_val_f + i_f * tickmark_int;
	}

	/* Finally do label */
	
	if (A->label[0] && anotate) {
		val_xyz[0] = z_project.corner_x[axis_no];
		val_xyz[1] = z_project.corner_y[axis_no];
		val_xyz[2] = project_info.z_level;
		size = gmtdefs.label_font_size * gmtdefs.dpi * GMT_u2u[GMT_PT][GMT_INCH];
		xsize = size * z_project.xshrink[id];
		ysize = size * z_project.yshrink[id];
		sprintf (cmd, "/F0 {pop /%s findfont [%lg 0 %lg %lg 0 0] makefont setfont} bind def\0",
			GMT_font_name[gmtdefs.label_font], xsize, ysize * z_project.tilt[id], ysize);
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
		ps_command ("/F0 {/Helvetica Y} bind def");	/* Reset definition of F0 */
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
	sprintf (label, "%s %s\0", year, &time_string[4]);
	for (i = 1; i < argc && argv[i][1] != 'J'; i++);
	ps_comment ("Begin time-stamp");
	ps_transrotate (x, y, 0.0);
	ps_setline (1);
	ps_rect (0.0, 0.0, dim[0]+dim[1], dim[2], gmtdefs.foreground_rgb, TRUE);
	ps_rect (0.0, 0.0, dim[0], dim[2], gmtdefs.background_rgb, TRUE);
	ps_setfont (1);
	ps_setpaint (gmtdefs.foreground_rgb);
	ps_text (0.5*dim[0], dim[3], 10, "GMT", 0.0, 6, 0);
	ps_setfont (0);
	ps_setpaint (gmtdefs.background_rgb);
	ps_text (dim[0]+0.5*dim[1], dim[3], 8, label, 0.0, 6, 0);
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
		
	if (label[0]) ps_text (dim[0]+dim[1]+dim[4], dim[3], 7, label, 0.0, 5, 0);
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

void GMT_color_image (double x0, double y0, double x_side, double y_side, unsigned char *image, int nx, int ny)
              		/* Lower left corner in inches */
                      	/* Size of cell in inches */
                     	/* color image  */
            {		/* image size */
	
	/* Call the appropriate image filler (see pslib) */
	
	switch (gmtdefs.color_image) {
		case 0:
			ps_colorimage (x0, y0, x_side, y_side, image, nx, ny);
			break;
		case 1:
			ps_colortiles (x0, y0, x_side, y_side, image, nx, ny);
			break;
	}
}

void GMT_text3d (double x, double y, double z, int fsize, int fontno, char *text, double angle, int justify, int form)
{
	double xb, yb, xt, yt, xt1, xt2, xt3, yt1, yt2, yt3, del_y;
	double ca, sa, xshrink, yshrink, tilt, baseline_shift, size, xsize, ysize;
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
		size = fsize * gmtdefs.dpi * GMT_u2u[GMT_PT][GMT_INCH];
		xsize = size * xshrink;
		ysize = size * yshrink;
		/* Temporarely modify meaning of F0 */
		sprintf (cmd, "/F0 {pop /%s findfont [%lg 0 %lg %lg 0 0] makefont setfont} bind def\0",
			GMT_font_name[fontno], xsize, ysize * tilt, ysize);
		ps_command (cmd);

                ps_text (xt1, yt1, fsize, text, angle + baseline_shift, justify, form);
                ps_command ("/F0 {/Helvetica Y} bind def");       /* Reset definition of F0 */
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
		
		h = 0.58 * GMT_font_height[font] * size * GMT_u2u[GMT_PT][GMT_INCH];
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
		d_swap (z, project_info.z_level); 
		GMT_2D_to_3D (xx, yy, 4);
		d_swap (z, project_info.z_level);
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
	int i;
	double xx[7], yy[7], dx, dy, angle, length, s, c;
	
	if (project_info.three_D) {
		angle = atan2 (y1 - y0, x1 - x0);
		length = hypot (y1 - y0, x1 - x0);
		sincos (angle, &s, &c);
		xx[3] = x0 + length * c;
		yy[3] = y0 + length * s;
		dx = 0.5 * tailwidth * s;
		dy = 0.5 * tailwidth * c;
		xx[0] = x0 + dx;	xx[6] = x0 - dx;
		yy[0] = y0 - dy;	yy[6] = y0 + dy;
		dx = (length - (1.0 - 0.5 * shape) * headlength) * c;
		dy = (length - (1.0 - 0.5 * shape) * headlength) * s;
		xx[1] = xx[0] + dx;	xx[5] = xx[6] + dx;
		yy[1] = yy[0] + dy;	yy[5] = yy[6] + dy;
		x0 += (length - headlength) * c;
		y0 += (length - headlength) * s;
		dx = headwidth * s;
		dy = headwidth * c;
		xx[2] = x0 + dx;	xx[4] = x0 - dx;
		yy[2] = y0 - dy;	yy[4] = y0 + dy;
		for (i = 0; i < 7; i++) {
			GMT_xyz_to_xy (xx[i], yy[i], z0, &x0, &y0);
			xx[i] = x0;
			yy[i] = y0;
		}
		ps_polygon (xx, yy, 7, rgb, outline);
	}
	else
		ps_vector (x0, y0, x1, y1, tailwidth, headlength, headwidth, gmtdefs.vector_shape, rgb, outline);
}

int GMT_prepare_label (double angle, int side, double x, double y, int type, double *line_angle, double *text_angle, int *justify)
{
	BOOLEAN set_angle;
		
	if (!project_info.edge[side]) return -1;		/* Side doesnt exist */
	if (tframe_info.side[side] < 2) return -1;	/* Dont want labels here */
		
	if (tframe_info.check_side == TRUE) {
		if (type == 0 && side%2) return -1;
		if (type == 1 && !(side%2)) return -1;
	}
	
	/* if (gmtdefs.oblique_anotation & 2 && !(side%2)) angle = -90.0; */	/* GMT_get_label_parameters will make this 0 */
	if (gmtdefs.oblique_anotation & 16 && !(side%2)) angle = -90.0;	/* GMT_get_label_parameters will make this 0 */
	
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
	if (gmtdefs.oblique_anotation & 16) *line_angle = (side - 1) * 90.0;
	
	if (!set_angle) *justify = GMT_polar_adjust (side, angle, x, y);

	return 0;
}

void old_GMT_get_anot_label (double val, char *label, int do_minutes, int do_seconds, int lonlat, BOOLEAN worldmap)
/* val:		Degree value of anotation */
/* label: 	String to hold the final anotation */
/* do_minutes:	TRUE if degree and minutes are desired, FALSE for just integer degrees */
/* do_seconds:	TRUE if degree, minutes, and seconds are desired */
/* lonlat:	0 = longitudes, 1 = latitudes, 2 non-geographical data passed */
/* worldmap:	T/F, whatever GMT_world_map is */
{
	int ival, minutes, seconds, sign, which, fmt;
	BOOLEAN zero_fix = FALSE, dec_minutes = FALSE, no_degree;
	char letter = 0, format[64];
	
	which = (gmtdefs.degree_format >= 100);	/* 0 is small [Default], 1 is large */
	no_degree = (gmtdefs.degree_format >= 1000);	/* No, we dont want the degree symbol at all */
	fmt = gmtdefs.degree_format % 100;	/* take out the optional 100 or 1000 */
	
	if (lonlat == 0 && fmt != -1) {	/* Fix longitudes to 0.0 <= lon <= 360 first */
		while (val > 360.0) val -= 360.0;
		while (val < 0.0) val += 360.0;
	}

	if (lonlat < 2) {	/* i.e., for geographical data */
		if (fabs (val - 360.0) < GMT_CONV_LIMIT && !worldmap) val = 0.0;
		if (fabs (val - 360.0) < GMT_CONV_LIMIT && worldmap && project_info.projection == OBLIQUE_MERC) val = 0.0;
	}

	switch (fmt) {
		case 8:
			dec_minutes = TRUE;
		case 4:
		case 0:	/* Use 0 to 360 for longitudes and -90 to +90 for latitudes */
			break;
		case 9:
			dec_minutes = TRUE;
		case 5:
		case 1:	/* Use -180 to +180 for longitudes and -90 to +90 for latitudes */
			if (lonlat == 0 && val > 180.0) val -= 360.0;
			break;
		case 10:
			dec_minutes = TRUE;
		case 6:
		case 2:	/* Use unsigned 0 to 180 for longitudes and 0 to 90 for latitudes */
			if (lonlat == 0 && val > 180.0) val -= 360.0;
			val = fabs (val);
			break;
		case 11:
			dec_minutes = TRUE;
		case 7:
		case 3:	/* Use 0 to 180E and 0 to 180W for longitudes and 90S to 90N for latitudes */
			if (lonlat == 0) {
				if (val > 180.0) val -= 360.0;
				letter = (fabs (val) < GMT_CONV_LIMIT || fabs (val - 180.0) < GMT_CONV_LIMIT) ? 0 : ((val < 0.0) ? 'W' : 'E');
			}
			else
				letter = (fabs (val) < GMT_CONV_LIMIT) ? 0 : ((val < 0.0) ? 'S' : 'N');
			val = fabs (val);
			break;
		case 14:	/* 12-15 are for planetary uses where west longitudes are used */
			dec_minutes = TRUE;
		case 12:
		case 13:	/* Use -360 to 0 for longitudes and -90 to +90 for latitudes */
			if (lonlat == 0 && val > 0.0) val -= 360.0;
			break;
		case 17:
			dec_minutes = TRUE;
		case 15:
		case 16:	/* Use -360 to 0 for longitudes and -90 to +90 for latitudes */
			if (lonlat == 0) {
				if (val > 0.0) val -= 360.0;
				letter = (fabs (val) < GMT_CONV_LIMIT) ? 0 : 'W';
			}
			else
				letter = (fabs (val) < GMT_CONV_LIMIT) ? 0 : ((val < 0.0) ? 'S' : 'N');
			val = fabs (val);
			break;
	}
	
	if (fmt == -1) {	/* theta-r */
		if (lonlat || no_degree) {
			sprintf (format, "%s\0", gmtdefs.d_format);
			sprintf (label, format, val);
		}
		else {
			sprintf (format, "%s%s\0", gmtdefs.d_format, GMT_degree_symbol[gmtdefs.char_encoding][which]);
			sprintf (label, format, val);
		}
		return;
	}
	
	if ((fmt >= 4 && fmt <= 6) || fmt == 13) { /* Pure decimal degrees */
		if (no_degree)
			sprintf (format, "%s\0", gmtdefs.d_format);
		else
			sprintf (format, "%s%s\0", gmtdefs.d_format, GMT_degree_symbol[gmtdefs.char_encoding][which]);
		sprintf (label, format, val);
		return;
	}
	if (fmt == 7 || fmt == 16) { /* Pure decimal degrees with trailing letter */
		if (no_degree)
			sprintf (format, "%s%c\0", gmtdefs.d_format, letter);
		else
			sprintf (format, "%s%s%c\0", gmtdefs.d_format, GMT_degree_symbol[gmtdefs.char_encoding][which], letter);
		sprintf (label, format, val);
		return;
	}
	
	sign = (val < 0.0) ? -1 : 1;
	val = fabs (val);
	ival = (int)val;	/* Truncate to integer in the direction toward 0 */
	minutes = seconds = 0;
	if ((val - (double) ival) > SMALL) {
		minutes = (int)floor (((val - ival) * 60.0) + SMALL);
		if (minutes == 60) {
			minutes = 0;
			ival = irint (val);
		}
		seconds = irint (((val - ival - minutes / 60.0) * 3600.0));
		if (seconds == 60) {
			seconds = 0;
			minutes++;
			if (minutes == 60) {
				minutes = 0;
				ival = irint (val);
			}
		}
	}
	
	if (dec_minutes) do_minutes = do_seconds = TRUE;
	if (do_minutes) {
		if (ival == 0 && sign == -1) {	/* Must write out -0 degrees, do so by writing -1 and change 1 to 0 */
			ival = 1;
			zero_fix = TRUE;
		}
		if (do_seconds) {
			char minsec[128];
			if (dec_minutes)
				sprintf (minsec, gmtdefs.d_format, minutes + (seconds / 60.0));
			else
				sprintf (minsec, "%.2d\\251 %.2d\\042\0", minutes, seconds);
			if (letter) {
				if (no_degree)
					sprintf (label, "%d %s%c\0", sign * ival, minsec, letter);
				else
					sprintf (label, "%d%s %s%c\0", sign * ival, GMT_degree_symbol[gmtdefs.char_encoding][which], minsec, letter);
			}
			else {
				if (no_degree)
					sprintf (label, "%d %s\0", sign * ival, minsec);
				else
					sprintf (label, "%d%s %s\0", sign * ival, GMT_degree_symbol[gmtdefs.char_encoding][which], minsec);
			}
		}
		else {
			if (letter) {
				if (no_degree)
					sprintf (label, "%d %.2d\\251%c\0", sign * ival, minutes, letter);
				else
					sprintf (label, "%d%s %.2d\\251%c\0", sign * ival, GMT_degree_symbol[gmtdefs.char_encoding][which], minutes, letter);
			}
			else {
				if (no_degree)
					sprintf (label, "%d %.2d\\251\0", sign * ival, minutes);
				else
					sprintf (label, "%d%s %.2d\\251\0", sign * ival, GMT_degree_symbol[gmtdefs.char_encoding][which], minutes);
			}
		}
		if (zero_fix) label[1] = '0';	/* Undo the fix above */
	}
	else {
		if (letter) {
			if (no_degree)
				sprintf (label, "%d%c\0", sign * ival, letter);
			else
				sprintf (label, "%d%s%c\0", sign * ival, GMT_degree_symbol[gmtdefs.char_encoding][which], letter);
		}
		else {
			if (no_degree)
				sprintf (label, "%d\0", sign * ival);
			else
				sprintf (label, "%d%s\0", sign * ival, GMT_degree_symbol[gmtdefs.char_encoding][which]);
		}
	}
	return;
}

void GMT_get_anot_label (double val, char *label, int do_minutes, int do_seconds, int lonlat, BOOLEAN worldmap)
/* val:		Degree value of anotation */
/* label: 	String to hold the final anotation */
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
		sprintf (format, "%s\0", gmtdefs.d_format);
		sprintf (label, format, val);
	}
	else if (GMT_plot_calclock.geo.decimal)
		sprintf (label, GMT_plot_calclock.geo.x_format, val, letter);
	else {
		GMT_geo_to_dms (val, do_seconds, GMT_io.geo.f_sec_to_int, &d, &m, &s, &m_sec);	/* Break up into d, m, s, and remainder */
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
		if (tframe_info.horizontal) {
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


void GMT_draw_map_scale (double x0, double y0, double lat, double length, char measure, BOOLEAN gave_xy, BOOLEAN fancy)
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
	
	switch (measure) {
		case 'm':
			unit = 1;
			bar_length = 1.609344 * length;
			break;
		case 'n':
			unit = 2;
			bar_length = 1.852 * length;
			break;
		default:
			unit = 0;
			bar_length = length;
			break;
	}

	if (!gave_xy) {
		GMT_geo_to_xy (x0, y0, &a, &b);
		x0 = a;
		y0 = b;
	}
	
	dlon = 0.5 * bar_length * 1000.0 / (project_info.M_PR_DEG * cosd (lat));
	
	GMT_geoz_to_xy (project_info.central_meridian - dlon, lat, project_info.z_level, &x1, &dummy);
	GMT_geoz_to_xy (project_info.central_meridian + dlon, lat, project_info.z_level, &x2, &dummy);
	width = x2 - x1;
	half = 0.5 * width;
	a_len = fabs (gmtdefs.map_scale_height);
	off = a_len + 0.75 * gmtdefs.anot_offset;
	
	GMT_setpen (&gmtdefs.tick_pen);
	if (fancy) {	/* Fancy scale */
		n_f_ticks[8] = 3;
		n_f_ticks[1] = n_f_ticks[3] = n_f_ticks[7] = 4;
		n_f_ticks[0] = n_f_ticks[4] = 5;
		n_f_ticks[2] = n_f_ticks[5] = 6;
		n_f_ticks[6] = 7;
		n_a_ticks[4] = n_a_ticks[6] = n_a_ticks[8] = 1;
		n_a_ticks[0] = n_a_ticks[1] = n_a_ticks[3] = n_a_ticks[7] = 2;
		n_a_ticks[2] = n_a_ticks[5] = 3;
		base = pow (10.0, floor (d_log10 (length)));
		i = irint (length / base) - 1;
		d_base = length / n_a_ticks[i];
		dx_f = width / n_f_ticks[i];
		dx_a = width / n_a_ticks[i];
		bar_width = 0.5 * fabs (gmtdefs.map_scale_height);
		f_len = 0.75 * fabs (gmtdefs.map_scale_height);
		yy[2] = yy[3] = y0;
		yy[0] = yy[1] = y0 - bar_width;
		x_left = x0 - half;
		GMT_xyz_to_xy (x_left, y0 - f_len, project_info.z_level, &a, &b);
		ps_plot (a, b, 3);
		GMT_xyz_to_xy (x_left, y0, project_info.z_level, &a, &b);
		ps_plot (a, b, 2);
		for (j = 0; j < n_f_ticks[i]; j++) {
			xx[0] = xx[3] = x_left + j * dx_f;
			xx[1] = xx[2] = xx[0] + dx_f;
			for (k = 0; k < 4; k++) GMT_xyz_to_xy (xx[k], yy[k], project_info.z_level, &bx[k], &by[k]);
			rgb = (j%2) ? gmtdefs.foreground_rgb : gmtdefs.background_rgb;
			ps_polygon (bx, by, 4, rgb, TRUE);
			GMT_xyz_to_xy (xx[1], y0 - f_len, project_info.z_level, &a, &b);
			ps_plot (a, b, 3);
			GMT_xyz_to_xy (xx[1], y0, project_info.z_level, &a, &b);
			ps_plot (a, b, 2);
		}
		ty = y0 - off;
		for (j = 0; j <= n_a_ticks[i]; j++) {
			tx = x_left + j * dx_a;
			GMT_xyz_to_xy (tx, y0 - a_len, project_info.z_level, &a, &b);
			ps_plot (a, b, 3);
			GMT_xyz_to_xy (tx, y0, project_info.z_level, &a, &b);
			ps_plot (a, b, 2);
			sprintf (txt, "%lg\0", j * d_base);
			GMT_text3d (tx, ty, project_info.z_level, gmtdefs.anot_font_size, gmtdefs.anot_font, txt, 0.0, 10, 0);
		}
		GMT_xyz_to_xy (x0, y0 + f_len, project_info.z_level, &tx, &ty);
		GMT_text3d (tx, ty, project_info.z_level, gmtdefs.label_font_size, gmtdefs.label_font, label[unit], 0.0, 2, 0);
	}
	else {	/* Simple scale */
	
		sprintf (txt, "%lg %s\0", length, label[unit]);
		GMT_xyz_to_xy (x0 - half, y0 - gmtdefs.map_scale_height, project_info.z_level, &a, &b);
		ps_plot (a, b, 3);
		GMT_xyz_to_xy (x0 - half, y0, project_info.z_level, &a, &b);
		ps_plot (a, b, 2);
		GMT_xyz_to_xy (x0 + half, y0, project_info.z_level, &a, &b);
		ps_plot (a, b, 2);
		GMT_xyz_to_xy (x0 + half, y0 - gmtdefs.map_scale_height, project_info.z_level, &a, &b);
		ps_plot (a, b, 2);
		GMT_text3d (x0, y0 - off, project_info.z_level, gmtdefs.anot_font_size, gmtdefs.anot_font, txt, 0.0, 10, 0);
	}
}

int GMT_get_label_parameters (int side, double line_angle, int type, double *text_angle, int *justify)
{
	int ok;
	
	*text_angle = line_angle;
	if (*text_angle < -90.0) *text_angle += 360.0;
	if (tframe_info.horizontal && !(side%2)) *text_angle += 90.0;
	if (*text_angle >= 270.0 ) *text_angle -= 360.0;
	else if (*text_angle >= 90.0) *text_angle -= 180.0;
	
	if (type == 0 && gmtdefs.oblique_anotation & 2) *text_angle = 0.0;	/* Force horizontal lon anotation */
	if (type == 1 && gmtdefs.oblique_anotation & 4) *text_angle = 0.0;	/* Force horizontal lat anotation */

	switch (side) {
		case 0:		/* S */
			if (tframe_info.horizontal)
				*justify = 10;
			else
				*justify = ((*text_angle) < 0.0) ? 5 : 7;
			break;
		case 1:		/* E */
			*justify = 5;
			break;
		case 2:		/* N */
			if (tframe_info.horizontal)
				*justify = 2;
			else
				*justify = ((*text_angle) < 0.0) ? 7 : 5;
			break;
		case 3:		/* W */
			*justify = 7;
			break;
	}
	
	if (tframe_info.horizontal) return (TRUE);
		
	switch (side) {
		case 0:		/* S */
		case 2:		/* N */
			ok = (fabs ((*text_angle)) >= gmtdefs.anot_min_angle);
			break;
		case 1:		/* E */
		case 3:		/* W */
			ok = (fabs ((*text_angle)) <= (90.0 - gmtdefs.anot_min_angle));
			break;
	}
	return (ok);
}

char *GMT_convertpen (struct GMT_PEN *pen, int *width, int *offset, int rgb[])
{
	/* GMT_convertpen converts from internal points to current dpi unit.
	 * It allocates space and returs a pointer to the texture, if not null */

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
			sprintf (tmp, "%d \0", irint (atof (ptr) * pt_to_dpi));
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
	if (project_info.three_D) GMT_2D_to_3D (work_x, work_y, np);
	
	*x = work_x;
	*y = work_y;

	return (np);
}

double GMT_get_anot_offset (BOOLEAN *flip)
{
	/* Return offset in inches for text anotation.  If anotation
	 * is to be placed 'inside' the map, set flip to TRUE */
	 
	double a;
	 
	a = gmtdefs.anot_offset;
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
