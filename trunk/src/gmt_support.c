/*--------------------------------------------------------------------
 *	$Id: gmt_support.c,v 1.1.1.1 2000-12-28 01:23:45 gmt Exp $
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
 *	Contact info: www.soest.hawaii.edu/gmt
 *--------------------------------------------------------------------*/
/*
 *
 *			G M T _ S U P P O R T . C
 *
 *- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * GMT_support.c contains code used by most GMT programs
 *
 * Author:	Paul Wessel
 * Date:	13-JUL-2000
 * Version:	3.3.6
 *
 * Modules in this file:
 *
 *	GMT_akima		Akima's 1-D spline
 *	GMT_boundcond_init	Initialize struct GMT_EDGEINFO to unset flags
 *	GMT_boundcond_parse	Set struct GMT_EDGEINFO to user's requests
 *	GMT_boundcond_param_prep	Set struct GMT_EDGEINFO to what is doable
 *	GMT_boundcond_set	Set two rows of padding according to bound cond
 *	GMT_check_rgb		Check rgb for valid range
 *	GMT_comp_double_asc	Used when sorting doubles into ascending order [checks for NaN]
 *	GMT_comp_float_asc	Used when sorting floats into ascending order [checks for NaN]
 *	GMT_comp_int_asc	Used when sorting ints into ascending order
 *	GMT_contours		Subroutine for contouring
 *	GMT_cspline		Natural cubic 1-D spline solver
 *	GMT_csplint		Natural cubic 1-D spline evaluator
 *	GMT_bcr_init		Initialize structure for bicubic interpolation
 *	GMT_delaunay		Performs a Delaunay triangulation
 *	GMT_epsinfo		Fill out info need for PostScript header
 *	GMT_get_bcr_z		Get bicubic interpolated value
 *	GMT_get_bcr_nodal_values	Supports -"-
 *	GMT_get_bcr_cardinals	  	    "
 *	GMT_get_bcr_ij		  	    "
 *	GMT_get_bcr_xy		 	    "
 *	GMT_get_index		Return color table entry for given z
 *	GMT_get_format :	Find # of decimals and create format string
 *	GMT_get_rgb24		Return rgb for given z
 *	GMT_get_plot_array	Allocate memory for plotting arrays
 *	GMT_getfill		Decipher and check fill argument
 *	GMT_getinc		Decipher and check increment argument
 *	GMT_getpen		Decipher and check pen argument
 *	GMT_getrgb		Dechipher and check color argument
 *	GMT_init_fill		Initialize fill attributes
 *	GMT_init_pen		Initialize pen attributes
 *	GMT_grd_init :		Initialize grd header structure
 *	GMT_grd_shift :		Rotates grdfiles in x-direction
 *	GMT_grd_setregion :	Determines subset coordinates for grdfiles
 *	GMT_getpathname :	Prepend directory to file name
 *	GMT_hsv_to_rgb		Convert HSV to RGB
 *	GMT_illuminate		Add illumination effects to rgb
 *	GMT_intpol		1-D interpolation
 *	GMT_memory		Memory allocation/reallocation
 *	GMT_free		Memory deallocation
 *	GMT_non_zero_winding	Finds if a point is inside/outside a polygon
 *	GMT_read_cpt		Read color palette file
 *	GMT_rgb_to_hsv		Convert RGB to HSV
 *	GMT_smooth_contour	Use Akima's spline to smooth contour
 *	GMT_start_trace		Subfunction used by GMT_trace_contour
 *	GMT_trace_contour	Function that trace the contours in GMT_contours
 */
 
#include "gmt.h"
#include "gmt_boundcond.h"
#include "gmt_bcr.h"

#define I_255	(1.0 / 255.0)

int GMT_start_trace(float first, float second, int *edge, int edge_word, int edge_bit, unsigned int *bit);
int GMT_trace_contour(float *grd, struct GRD_HEADER *header, double x0, double y0, int *edge, double **x_array, double **y_array, int i, int j, int kk, int offset, int *i_off, int *j_off, int *k_off, int *p, unsigned int *bit, int *nan_flag);
int GMT_smooth_contour(double **x_in, double **y_in, int n, int sfactor, int stype);
int GMT_splice_contour(double **x, double **y, int n, double **x2, double **y2, int n2);
void GMT_setcontjump (float *z, int nz);
void GMT_rgb_to_hsv(int rgb[], double *h, double *s, double *v);
void GMT_hsv_to_rgb(int rgb[], double h, double s, double v);
void GMT_get_bcr_cardinals (double x, double y);
void GMT_get_bcr_ij (struct GRD_HEADER *grd, double xx, double yy, int *ii, int *jj, struct GMT_EDGEINFO *edgeinfo);
void GMT_get_bcr_xy(struct GRD_HEADER *grd, double xx, double yy, double *x, double *y);
void GMT_get_bcr_nodal_values(float *z, int ii, int jj);

int GMT_check_rgb (int rgb[])
{
	return (( (rgb[0] < 0 || rgb[0] > 255) || (rgb[1] < 0 || rgb[1] > 255) || (rgb[2] < 0 || rgb[2] > 255) ));
}

void GMT_init_fill (struct GMT_FILL *fill, int r, int g, int b)
{	/* Initialize FILL structure */
	int i;

	fill->use_pattern = fill->inverse = fill->colorize = FALSE;
	fill->pattern[0] = 0;
	fill->pattern_no = 0;
	fill->dpi = 0;
	for (i = 0; i < 3; i++) fill->f_rgb[i] = 0;
	for (i = 0; i < 3; i++) fill->b_rgb[i] = 255;
	fill->rgb[0] = r;	fill->rgb[1] = g; fill->rgb[2] = b;
}

int GMT_getfill (char *line, struct GMT_FILL *fill)
{
	int n, count, error = 0, slash_count(char *txt);
	int pos, i, fr, fg, fb;
	char f;
	
	/* Syntax:   -G<gray>, -G<rgb>, or -Gp|P<dpi>/<image>[:F<rgb>B<rgb>]   */
	/* Note, <rgb> can be r/g/b, gray, or - for masks */

	if (line[0] == 'p' || line[0] == 'P') {	/* Image specified */
		n = sscanf (&line[1], "%d/%s", &fill->dpi, fill->pattern);
		if (n != 2) error = 1;
		for (i = 0, pos = -1; fill->pattern[i] && pos == -1; i++) if (fill->pattern[i] == ':') pos = i;
		if (pos > -1) fill->pattern[pos] = '\0';
		fill->pattern_no = atoi (fill->pattern);
		if (fill->pattern_no == 0) fill->pattern_no = -1;
		fill->inverse = !(line[0] == 'P');
		fill->use_pattern = TRUE;

		/* See if fore- and background colors are given */

		for (i = 0, pos = -1; line[i] && pos == -1; i++) if (line[i] == ':') pos = i;
		pos++;

		if (pos > 0 && line[pos]) {	/* Gave colors */
			fill->colorize = TRUE;
			while (line[pos]) {
				f = line[pos++];
				if (line[pos] == '-') {	/* Signal for masking */
					fr = fg = fb = -1;
					n = 3;
					fill->colorize = FALSE;
				}
				else
					n = sscanf (&line[pos], "%d/%d/%d", &fr, &fg, &fb);
				if (n == 3) {
					if (f == 'f' || f == 'F') {
						fill->f_rgb[0] = fr;
						fill->f_rgb[1] = fg;
						fill->f_rgb[2] = fb;
					}
					else if (f == 'b' || f == 'B') {
						fill->b_rgb[0] = fr;
						fill->b_rgb[1] = fg;
						fill->b_rgb[2] = fb;
					}
					else
						error++;
				}
				else
					error++;

				while (line[pos] && !(line[pos] == 'F' || line[pos] == 'B')) pos++;
			}
			if (fill->f_rgb[0] >= 0) error += GMT_check_rgb (fill->f_rgb);
			if (fill->b_rgb[0] >= 0) error += GMT_check_rgb (fill->b_rgb);
		}
	}
	else {	/* Plain color or shade */
		if ((count = slash_count (line)) == 2)
			n = sscanf (line, "%d/%d/%d", &fill->rgb[0], &fill->rgb[1], &fill->rgb[2]);
		else if (count == 0) {
			n = sscanf (line, "%d", &fill->rgb[0]);
			fill->rgb[1] = fill->rgb[2] = fill->rgb[0];
		}
		else
			n = 0;
		fill->use_pattern = FALSE;
		error = (n < 1 || n > 3) ? TRUE : GMT_check_rgb (fill->rgb);
	}
	return (error);
}

int slash_count (char *txt)
{
	int i = 0, n = 0;
	while (txt[i]) if (txt[i++] == '/') n++;
	return (n);
}

int GMT_getrgb (char *line, int rgb[])
{
	int n, count, slash_count(char *txt);
	
	if ((count = slash_count (line)) == 2)
		n = sscanf (line, "%d/%d/%d", &rgb[0], &rgb[1], &rgb[2]);
	else if (count == 0) {
		n = sscanf (line, "%d", &rgb[0]);
		rgb[1] = rgb[2] = rgb[0];
	}
	else
		n = 0;
	return (n < 1 || n > 3 || GMT_check_rgb (rgb));
}

void GMT_init_pen (struct GMT_PEN *pen, double width)
{
	/* Sets default black solid pen of given width in points */

	pen->width = width;
	pen->rgb[0] = gmtdefs.basemap_frame_rgb[0];
	pen->rgb[1] = gmtdefs.basemap_frame_rgb[1];
	pen->rgb[2] = gmtdefs.basemap_frame_rgb[2];
	pen->texture[0] = 0;
	pen->offset = 0.0;
}

int GMT_getpen (char *line, struct GMT_PEN *pen)
{
	int i, n_slash, t_pos, c_pos, s_pos;
	BOOLEAN get_pen, points = FALSE;
	double val = 0.0, width, dpi_to_pt;
	
	dpi_to_pt = GMT_u2u[GMT_INCH][GMT_PT] / gmtdefs.dpi;

	GMT_init_pen (pen, GMT_PENWIDTH);	/* Default pen */

	/* Check for p which means pen thickness is given in points */
	
	points = (BOOLEAN) strchr (line, 'p');
	
	/* First check if a pen thickness has been entered */
	
	get_pen = ((line[0] == '-' && (line[1] >= '0' && line[1] <= '9')) || (line[0] >= '0' && line[0] <= '9'));
	
	/* Then look for slash which indicate start of color information */
	
	for (i = n_slash = 0, s_pos = -1; line[i]; i++) if (line[i] == '/') {
		n_slash++;
		if (s_pos < 0) s_pos = i;	/* First slash position */
	}
	
	/* Finally see if a texture is given */
	
	for (i = 0, t_pos = -1; line[i] && t_pos == -1; i++) if (line[i] == 't') t_pos = i;
	
	/* Now decode values */
	
	if (get_pen) {	/* Decode pen width */
		val = atof (line);
		pen->width = (points) ? val : (val * dpi_to_pt);
	}
	if (s_pos >= 0) {	/* Get color of pen */
		s_pos++;
		if (n_slash == 1) {	/* Gray shade is given */
			sscanf (&line[s_pos], "%d", &pen->rgb[0]);
			pen->rgb[1] = pen->rgb[2] = pen->rgb[0];
		}
		else if (n_slash == 3)	/* Color is given */
			sscanf (&line[s_pos], "%d/%d/%d", &pen->rgb[0], &pen->rgb[1], &pen->rgb[2]);
	}
	
	if (t_pos >= 0) {	/* Get texture */
		t_pos++;
		if (line[t_pos] == 'o') {	/* Default Dotted */
			width = (pen->width < SMALL) ? GMT_PENWIDTH : pen->width;
			sprintf (pen->texture, "%lg %lg\0", width, 4.0 * width);
			pen->offset = 0.0;
		}
		else if (line[t_pos] == 'a') {	/* Default Dashed */
			width = (pen->width < SMALL) ? GMT_PENWIDTH : pen->width;
			sprintf (pen->texture, "%lg %lg\0", 8.0 * width, 8.0 * width);
			pen->offset = 4.0 * width;
		}
		else {	/* Specified pattern */
			for (i = t_pos+1, c_pos = 0; line[i] && c_pos == 0; i++) if (line[i] == ':') c_pos = i;
			if (c_pos == 0) return (pen->width < 0.0 || GMT_check_rgb (pen->rgb));
			line[c_pos] = ' ';
			sscanf (&line[t_pos], "%s %lf", pen->texture, &pen->offset);
			line[c_pos] = ':';
			for (i = 0; pen->texture[i]; i++) if (pen->texture[i] == '_') pen->texture[i] = ' ';
			if (!points) {	/* Must convert current dpi units to points  */
				char tmp[32], string[BUFSIZ], *ptr;
				
				ptr = strtok (pen->texture, " ");
				memset ((void *)string, 0, (size_t)BUFSIZ);
				while (ptr) {
					sprintf (tmp, "%lg \0", (atof (ptr) * dpi_to_pt));
					strcat (string, tmp);
					ptr = strtok (CNULL, " ");
				}
				string[strlen (string) - 1] = 0;
				if (strlen (string) >= GMT_PEN_LEN) {
					fprintf (stderr, "%s: GMT Error: Pen attributes too long!\n", GMT_program);
					exit (EXIT_FAILURE);
				}
				strcpy (pen->texture, string);
				pen->offset *= dpi_to_pt;
			}
					
		}
	}
	return (pen->width < 0.0 || GMT_check_rgb (pen->rgb));
}

int GMT_getinc (char *line, double *dx, double *dy)
{
	int t_pos, i;
	char xstring[128], ystring[128];
	
	for (t_pos = -1, i = 0; line[i] && t_pos < 0; i++) if (line[i] == '/') t_pos = i;
	
	if (t_pos != -1) {
		strcpy (xstring, line);
		xstring[t_pos] = 0;
		if (t_pos > 0 && (xstring[t_pos-1] == 'm' || xstring[t_pos-1] == 'M') ) {
			xstring[t_pos-1] = 0;
			if ( (sscanf(xstring, "%lf", dx)) != 1) return(1);
			(*dx) /= 60.0;
		}
		else if (t_pos > 0 && (xstring[t_pos-1] == 'c' || xstring[t_pos-1] == 'C') ) {
			xstring[t_pos-1] = 0;
			if ( (sscanf(xstring, "%lf", dx)) != 1) return(1);
			(*dx) /= 3600.0;
		}
		else {
			if ( (sscanf(xstring, "%lf", dx)) != 1) return(1);
		}

		strcpy (ystring, &line[t_pos+1]);
		t_pos = strlen(ystring);
		if (t_pos > 0 && (ystring[t_pos-1] == 'm' || ystring[t_pos-1] == 'M') ) {
			ystring[t_pos-1] = 0;
			if ( (sscanf(ystring, "%lf", dy)) != 1) return(1);
			(*dy) /= 60.0;
		}
		else if (t_pos > 0 && (ystring[t_pos-1] == 'c' || ystring[t_pos-1] == 'C') ) {
			ystring[t_pos-1] = 0;
			if ( (sscanf(ystring, "%lf", dy)) != 1) return(1);
			(*dy) /= 3600.0;
		}
		else {
			if ( (sscanf(ystring, "%lf", dy)) != 1) return(1);
		}
	}
	else {
		strcpy (xstring, line);
		t_pos = strlen(xstring);
		if (t_pos > 0 && (xstring[t_pos-1] == 'm' || xstring[t_pos-1] == 'M') ) {
			xstring[t_pos-1] = 0;
			if ( (sscanf(xstring, "%lf", dx)) != 1) return(1);
			(*dx) /= 60.0;
		}
		else if (t_pos > 0 && (xstring[t_pos-1] == 'c' || xstring[t_pos-1] == 'C') ) {
			xstring[t_pos-1] = 0;
			if ( (sscanf(xstring, "%lf", dx)) != 1) return(1);
			(*dx) /= 3600.0;
		}
		else {
			if ( (sscanf(xstring, "%lf", dx)) != 1) return(1);
		}
		*dy = (*dx);
	}
	return(0);
}

void GMT_read_cpt (char *cpt_file)
{
	/* Opens and reads a color palette file in RGB or HSV of arbitrary length */
	
	int n = 0, i, j, nread, anot, n_alloc = GMT_SMALL_CHUNK, color_model;
	double r0, r1, g0, g1, b0, b1, dz;
	BOOLEAN gap, error = FALSE;
	char line[BUFSIZ], c, first_r[BUFSIZ], first_b[BUFSIZ];
	FILE *fp = NULL;
	
	if (!cpt_file)
		fp = GMT_stdin;
	else if ((fp = fopen (cpt_file, "r")) == NULL) {
		fprintf (stderr, "%s: GMT Fatal Error: Cannot open color palette table %s\n", GMT_program, cpt_file);
		exit (EXIT_FAILURE);
	}
	
	GMT_lut = (struct GMT_LUT *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (struct GMT_LUT), "GMT_read_cpt");
	
	GMT_b_and_w = GMT_gray = TRUE;
	GMT_continuous = FALSE;
	color_model = gmtdefs.color_model;
	GMT_cpt_pattern = FALSE;

	while (!error && fgets (line, BUFSIZ, fp)) {
	
		if (strstr (line, "COLOR_MODEL")) {	/* cpt file specifies color model */
			if (strstr (line, "RGB"))
				color_model = GMT_RGB;
			else if (strstr (line, "HSV"))
				color_model = GMT_HSV;
		}

		if (line[0] == '#' || line[0] == '\n') continue;	/* Comment or blank */
		
		if (line[0] == 'F' || line[0] == 'f') {	/* Foreground color */
			nread = sscanf (&line[1], "%s %lf %lf", first_r, &g0, &b0);
			if (!(nread == 1 || nread == 3)) error = TRUE;
			if (first_r[0] == 'p' || first_r[0] == 'P') {	/* Gave pattern instead */
				GMT_bfn.fill[0] = (struct GMT_FILL *) GMT_memory (VNULL, 1, sizeof (struct GMT_FILL), GMT_program);
				if (GMT_getfill (first_r, GMT_bfn.fill[0])) {
					fprintf (stderr, "%s: GMT Fatal Error: Pattern fill not understood!\n", GMT_program);
					exit (EXIT_FAILURE);
				}
				GMT_cpt_pattern = TRUE;
			}
			else {	/* Shades, RGB, or HSV */
				r0 = (first_r[0] == '-') ? -1.0 : atof (first_r);
				if (nread == 1 || color_model == GMT_RGB) {	/* Read r,g,b or gray */
					if (nread == 1)	/* Grayshades given */
						GMT_bfn.foreground_rgb[0] = GMT_bfn.foreground_rgb[1] = GMT_bfn.foreground_rgb[2] = irint (r0);
					else {
						GMT_bfn.foreground_rgb[0] = irint (r0);
						GMT_bfn.foreground_rgb[1] = irint (g0);
						GMT_bfn.foreground_rgb[2] = irint (b0);
					}
				}
				else
					GMT_hsv_to_rgb (GMT_bfn.foreground_rgb, r0, g0, b0);
				GMT_bfn.skip[0] = (GMT_bfn.foreground_rgb[0] == -1);
				if (GMT_bfn.skip[0] && nread != 1) {
					fprintf (stderr, "%s: GMT Fatal Error: Foreground to skip not in [F -] format!\n", GMT_program);
					exit (EXIT_FAILURE);
				}
			}
			continue;
		}
		else if (line[0] == 'B' || line[0] == 'b') {	/* Background color */
			nread = sscanf (&line[1], "%s %lf %lf", first_r, &g0, &b0);
			if (!(nread == 1 || nread == 3)) error = TRUE;
			if (first_r[0] == 'p' || first_r[0] == 'P') {	/* Gave pattern instead */
				GMT_bfn.fill[1] = (struct GMT_FILL *) GMT_memory (VNULL, 1, sizeof (struct GMT_FILL), GMT_program);
				if (GMT_getfill (first_r, GMT_bfn.fill[1])) {
					fprintf (stderr, "%s: GMT Fatal Error: Pattern fill not understood!\n", GMT_program);
					exit (EXIT_FAILURE);
				}
				GMT_cpt_pattern = TRUE;
			}
			else {	/* Shades, RGB, or HSV */
				r0 = (first_r[0] == '-') ? -1.0 : atof (first_r);
				if (nread == 1 || color_model == GMT_RGB) {	/* Read r,g,b  or gray*/
					if (nread == 1)	/* Grayshades given */
						GMT_bfn.background_rgb[0] = GMT_bfn.background_rgb[1] = GMT_bfn.background_rgb[2] = irint (r0);
					else {
						GMT_bfn.background_rgb[0] = irint (r0);
						GMT_bfn.background_rgb[1] = irint (g0);
						GMT_bfn.background_rgb[2] = irint (b0);
					}
				}
				else
					GMT_hsv_to_rgb (GMT_bfn.background_rgb, r0, g0, b0);
				GMT_bfn.skip[1] = (GMT_bfn.background_rgb[0] == -1);
				if (GMT_bfn.skip[1] && nread != 1) {
					fprintf (stderr, "%s: GMT Fatal Error: Background to skip not in [B -] format!\n", GMT_program);
					exit (EXIT_FAILURE);
				}
			}
			continue;
		}
		else if (line[0] == 'N' || line[0] == 'n') {	/* NaN color */
			nread = sscanf (&line[1], "%s %lf %lf", first_r, &g0, &b0);
			if (!(nread == 1 || nread == 3)) error = TRUE;
			if (first_r[0] == 'p' || first_r[0] == 'P') {	/* Gave pattern instead */
				GMT_bfn.fill[2] = (struct GMT_FILL *) GMT_memory (VNULL, 1, sizeof (struct GMT_FILL), GMT_program);
				if (GMT_getfill (first_r, GMT_bfn.fill[2])) {
					fprintf (stderr, "%s: GMT Fatal Error: Pattern fill not understood!\n", GMT_program);
					exit (EXIT_FAILURE);
				}
				GMT_cpt_pattern = TRUE;
			}
			else {	/* Shades, RGB, or HSV */
				r0 = (first_r[0] == '-') ? -1.0 : atof (first_r);
				if (nread == 1 || color_model == GMT_RGB) {	/* Read r,g,b */
					if (nread == 1)	/* Grayshades given */
						GMT_bfn.nan_rgb[0] = GMT_bfn.nan_rgb[1] = GMT_bfn.nan_rgb[2] = irint (r0);
					else {
						GMT_bfn.nan_rgb[0] = irint (r0);
						GMT_bfn.nan_rgb[1] = irint (g0);
						GMT_bfn.nan_rgb[2] = irint (b0);
					}
				}
				else
					GMT_hsv_to_rgb (GMT_bfn.nan_rgb, r0, g0, b0);
				GMT_bfn.skip[2] = (GMT_bfn.nan_rgb[0] == -1);
				if (GMT_bfn.skip[2] && nread != 1) {
					fprintf (stderr, "%s: GMT Fatal Error: NaNs to skip not in [N -] format!\n", GMT_program);
					exit (EXIT_FAILURE);
				}
			}
			continue;
		}

		nread = sscanf (line, "%lf %s %lf %s %lf %lf %lf %lf", 
			&GMT_lut[n].z_low, first_r, &g0, first_b, &GMT_lut[n].z_high, &r1, &g1, &b1);
		if (nread <= 0) continue;	/* Probably a line with spaces */
		if (!(nread == 4 || nread == 8)) error = TRUE;
		
		if (first_r[0] == 'p' || first_r[0] == 'P') {	/* Gave pattern instead */
			GMT_lut[n].fill = (struct GMT_FILL *) GMT_memory (VNULL, 1, sizeof (struct GMT_FILL), GMT_program);
			if (GMT_getfill (first_r, GMT_lut[n].fill)) {
				fprintf (stderr, "%s: GMT Fatal Error: Pattern fill not understood!\n", GMT_program);
				exit (EXIT_FAILURE);
			}
			else if (nread != 4) {
				fprintf (stderr, "%s: GMT Fatal Error: z-slice with pattern fill not in [z0 pattern z1 -] format!\n", GMT_program);
				exit (EXIT_FAILURE);
			}
			GMT_lut[n].z_high = g0;
			GMT_cpt_pattern = TRUE;
		}
		else {	/* Shades, RGB, or HSV */
		
			r0 = (first_r[0] == '-') ? -1.0 : atof (first_r);
			b0 = (first_b[0] == '-') ? -1.0 : atof (first_b);
		
			if (nread == 4 || color_model == GMT_RGB) {	/* Read r,b,g or gray */
				if (nread == 4)	{	/* Grayshades given */
					GMT_lut[n].z_high = g0;
					r1 = b0;
					GMT_lut[n].rgb_low[0] = GMT_lut[n].rgb_low[1] = GMT_lut[n].rgb_low[2] = irint (r0);
					GMT_lut[n].rgb_high[0] = GMT_lut[n].rgb_high[1] = GMT_lut[n].rgb_high[2] = irint (r1);
				}
				else {
					GMT_lut[n].rgb_low[0] = irint (r0);
					GMT_lut[n].rgb_low[1] = irint (g0);
					GMT_lut[n].rgb_low[2] = irint (b0);
					GMT_lut[n].rgb_high[0] = irint (r1);
					GMT_lut[n].rgb_high[1] = irint (g1);
					GMT_lut[n].rgb_high[2] = irint (b1);
				}
			}
			else {	/* Read hue, saturation, value */
				GMT_hsv_to_rgb (GMT_lut[n].rgb_low, r0, g0, b0);
				GMT_hsv_to_rgb (GMT_lut[n].rgb_high, r1, g1, b1);
			}
		
			dz = GMT_lut[n].z_high - GMT_lut[n].z_low;
			if (dz == 0.0) {
				fprintf (stderr, "%s: GMT Fatal Error: Z-slice with dz = 0\n", GMT_program);
				exit (EXIT_FAILURE);
			}
			GMT_lut[n].i_dz = 1.0 / dz;

			if (GMT_lut[n].rgb_low[0] != -1) {	/* Ok to check these colors */
				GMT_lut[n].skip = FALSE;
				if (!(GMT_lut[n].rgb_low[0] == GMT_lut[n].rgb_low[1] && 
					GMT_lut[n].rgb_low[0] == GMT_lut[n].rgb_low[2])) GMT_gray = FALSE;
				if (!(GMT_lut[n].rgb_high[0] == GMT_lut[n].rgb_high[1] && 
				GMT_lut[n].rgb_high[0] == GMT_lut[n].rgb_high[2])) GMT_gray = FALSE;
				if (GMT_gray && !(GMT_lut[n].rgb_low[0] == 0 || GMT_lut[n].rgb_low[0] == 255)) GMT_b_and_w = FALSE;
				if (GMT_gray && !(GMT_lut[n].rgb_high[0] == 0 || GMT_lut[n].rgb_high[0] == 255)) GMT_b_and_w = FALSE;
				for (i = 0; !GMT_continuous && i < 3; i++)
					if (GMT_lut[n].rgb_low[i] != GMT_lut[n].rgb_high[i]) GMT_continuous = TRUE;
				for (j = 0; j < 3; j++) GMT_lut[n].rgb_diff[j] = GMT_lut[n].rgb_high[j] - GMT_lut[n].rgb_low[j];	/* Used in GMT_get_rgb24 */

			}
			else {
				GMT_lut[n].skip = TRUE;	/* Dont paint this slice */
				if (GMT_lut[n].skip && nread != 4) {
					fprintf (stderr, "%s: GMT Fatal Error: z-slice to skip not in [z0 - z1 -] format!\n", GMT_program);
					exit (EXIT_FAILURE);
				}
				for (i = 0; i < 3; i++) GMT_lut[n].rgb_low[i] = GMT_lut[n].rgb_high[i] = gmtdefs.page_rgb[i];
			}
		}

		/* Determine if psscale need to label these steps */

		c = line[strlen(line)-2]; 
		if (c == 'L' || c == 'l')
			GMT_lut[n].anot = 1;
		else if (c == 'U' || c == 'u')
			GMT_lut[n].anot = 2;
		else if (c == 'B' || c == 'b')
			GMT_lut[n].anot = 3;
		n++;
		if (n == n_alloc) {
			i = n_alloc;
			n_alloc += GMT_SMALL_CHUNK;
			GMT_lut = (struct GMT_LUT *) GMT_memory ((void *)GMT_lut, (size_t)n_alloc, sizeof (struct GMT_LUT), "GMT_read_cpt");
			memset ((void *)&GMT_lut[i], 0, (size_t)(GMT_SMALL_CHUNK * sizeof (struct GMT_LUT)));	/* Initialize new structs to zero */
		}
	}
	
	if (fp != GMT_stdin) fclose (fp);

	if (error) {
		fprintf (stderr, "%s: GMT Fatal Error: Error when decoding %s - aborts!\n", GMT_program, cpt_file);
		exit (EXIT_FAILURE);
	}
	if (n == 0) {
		fprintf (stderr, "%s: GMT Fatal Error: CPT file %s has no z-slices!\n", GMT_program, cpt_file);
		exit (EXIT_FAILURE);
	}
		
	GMT_lut = (struct GMT_LUT *) GMT_memory ((void *)GMT_lut, (size_t)n, sizeof (struct GMT_LUT), "GMT_read_cpt");
	GMT_n_colors = n;
	for (i = anot = 0, gap = FALSE; i < GMT_n_colors - 1; i++) {
		if (GMT_lut[i].z_high != GMT_lut[i+1].z_low) gap = TRUE;
		anot += GMT_lut[i].anot;
	}
	anot += GMT_lut[i].anot;
	if (gap) {
		fprintf (stderr, "%s: GMT Fatal Error: Color palette table %s has gaps - aborts!\n", GMT_program, cpt_file);
		exit (EXIT_FAILURE);
	}
	if (!anot) {	/* Must set default anotation flags */
		for (i = 0; i < GMT_n_colors; i++) GMT_lut[i].anot = 1;
		GMT_lut[i-1].anot = 3;
	}
	if (!(GMT_bfn.foreground_rgb[0] == GMT_bfn.foreground_rgb[1] && GMT_bfn.foreground_rgb[0] == GMT_bfn.foreground_rgb[2])) GMT_gray = FALSE;
	if (GMT_gray && !(GMT_bfn.foreground_rgb[0] == 0 || GMT_bfn.foreground_rgb[0] == 255)) GMT_b_and_w = FALSE;
	if (!(GMT_bfn.background_rgb[0] == GMT_bfn.background_rgb[1] && GMT_bfn.background_rgb[0] == GMT_bfn.background_rgb[2])) GMT_gray = FALSE;
	if (GMT_gray && !(GMT_bfn.background_rgb[0] == 0 || GMT_bfn.background_rgb[0] == 255)) GMT_b_and_w = FALSE;
	if (!(GMT_bfn.nan_rgb[0] == GMT_bfn.nan_rgb[1] && GMT_bfn.nan_rgb[0] == GMT_bfn.nan_rgb[2])) GMT_gray = FALSE;
	if (GMT_gray && !(GMT_bfn.nan_rgb[0] == 0 || GMT_bfn.nan_rgb[0] == 255)) GMT_b_and_w = FALSE;
	if (!GMT_gray) GMT_b_and_w = FALSE;
}

void GMT_sample_cpt (double z[], int nz, BOOLEAN continuous, BOOLEAN reverse)
{
	/* Resamples the current cpt table based on new z-array.
	 * Old cpt is normalized to 0-1 range and scaled to fit new z range.
	 * New cpt may be continuous and/or reversed.
	 * We write the new cpt table to stdout */

	int i, j, k, nx, upper, lower, rgb_low[3], rgb_high[3];
	BOOLEAN even = FALSE;	/* TRUE when nz is passed as negative */
	double *x, a, b, h1, h2, h3, v1, v2, v3, s1, s2, s3, f, x_inc;
	char format[BUFSIZ];
	struct GMT_LUT *lut;

	if (nz < 0) {	/* Called from grd2cpt which want equal area colors */
		nz = -nz;
		even = TRUE;
	}
	
	lut = (struct GMT_LUT *) GMT_memory (VNULL, (size_t)GMT_n_colors, sizeof (struct GMT_LUT), GMT_program);
	
	/* First normalize old cpt file so z-range is 0-1 */

	b = 1.0 / (GMT_lut[GMT_n_colors-1].z_high - GMT_lut[0].z_low);
	a = -GMT_lut[0].z_low * b;

	for (i = 0; i < GMT_n_colors; i++) {	/* Copy/normalize cpt file and reverse if needed */
		lut[i].z_low = a + b * GMT_lut[i].z_low;
		lut[i].z_high = a + b * GMT_lut[i].z_high;
		if (reverse) {
			j = GMT_n_colors - i - 1;
			memcpy ((void *)lut[i].rgb_high, (void *)GMT_lut[j].rgb_low,  (size_t)(3 * sizeof (int)));
			memcpy ((void *)lut[i].rgb_low,  (void *)GMT_lut[j].rgb_high, (size_t)(3 * sizeof (int)));
		}
		else {
			j = i;
			memcpy ((void *)lut[i].rgb_high, (void *)GMT_lut[j].rgb_high, (size_t)(3 * sizeof (int)));
			memcpy ((void *)lut[i].rgb_low,  (void *)GMT_lut[j].rgb_low,  (size_t)(3 * sizeof (int)));
		}
	}
	
	/* Then set up normalized output locations x */

	nx = (continuous) ? nz : nz - 1;
	x = (double *) GMT_memory (VNULL, (size_t)nz, sizeof(double), GMT_program);

	if (nx == 1) {	/* Want a single color point, assume 1/2 way */
		x[0] = 0.5;
	}
	else if (even) {
		x_inc = 1.0 / (nx - 1);
		for (i = 0; i < nz; i++) x[i] = i * x_inc;	/* Normalized z values 0-1 */
	}
	else {	/* As with LUT, translate users z-range to 0-1 range */
		b = 1.0 / (z[nz-1] - z[0]);
		a = -z[0] * b;
		for (i = 0; i < nz; i++) x[i] = a + b * z[i];	/* Normalized z values 0-1 */
	}

	/* Start writing cpt file info to stdout */

	if (gmtdefs.color_model == GMT_HSV)
		fprintf (GMT_stdout, "#COLOR_MODEL = HSV\n");
	else
		fprintf (GMT_stdout, "#COLOR_MODEL = RGB\n");

	fprintf (GMT_stdout, "#\n");

	if (gmtdefs.color_model == GMT_HSV) {
		sprintf(format, "%s\t%s\t%s\t%s\t%s\t%s\t%s\t%s\n\0", gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format);
	}
	else {
		sprintf(format, "%s\t%%d\t%%d\t%%d\t%s\t%%d\t%%d\t%%d\n\0", gmtdefs.d_format, gmtdefs.d_format);
	}

	for (i = 0; i < nz-1; i++) {

		lower = i;
		upper = i + 1;
		
		/* Interpolate lower color */

		for (j = 0; j < GMT_n_colors && x[lower] >= lut[j].z_high; j++);
		if (j == GMT_n_colors) j--;

		f = 1.0 / (lut[j].z_high - lut[j].z_low);
		
		for (k = 0; k < 3; k++) rgb_low[k] = lut[j].rgb_low[k] + irint ((lut[j].rgb_high[k] - lut[j].rgb_low[k]) * f * (x[lower] - lut[j].z_low));

		if (continuous) {	/* Interpolate upper color */

			while (j < GMT_n_colors && x[upper] > lut[j].z_high) j++;

			f = 1.0 / (lut[j].z_high - lut[j].z_low);
			
			for (k = 0; k < 3; k++) rgb_high[k] = lut[j].rgb_low[k] + irint ((lut[j].rgb_high[k] - lut[j].rgb_low[k]) * f * (x[upper] - lut[j].z_low));
		}
		else	/* Just copy lower */
			for (k = 0; k < 3; k++) rgb_high[k] = rgb_low[k];

		if (gmtdefs.color_model == GMT_HSV) {
			GMT_rgb_to_hsv(rgb_low, &h1, &s1, &v1);
			GMT_rgb_to_hsv(rgb_high, &h2, &s2, &v2);
			fprintf (GMT_stdout, format, z[lower], h1, s1, v1, z[upper], h2, s2, v2);
		}
		else {
			fprintf (GMT_stdout, format, z[lower], rgb_low[0], rgb_low[1], rgb_low[2], z[upper], rgb_high[0], rgb_high[1], rgb_high[2]);
		}
	}

	/* Background, foreground, and nan colors */

	if (reverse) {	/* Flip foreground and background colors */
		memcpy ((void *)rgb_low, (void *)GMT_bfn.background_rgb, (size_t)(3 * sizeof (int)));
		memcpy ((void *)GMT_bfn.background_rgb, (void *)GMT_bfn.foreground_rgb, (size_t)(3 * sizeof (int)));
		memcpy ((void *)GMT_bfn.foreground_rgb, (void *)rgb_low, (size_t)(3 * sizeof (int)));
	}
	
	if (gmtdefs.color_model == GMT_HSV) {
		sprintf(format, "%%c\t%s\t%s\t%s\n\0", gmtdefs.d_format, gmtdefs.d_format, gmtdefs.d_format);
		GMT_rgb_to_hsv(GMT_bfn.background_rgb, &h1, &s1, &v1);
		GMT_rgb_to_hsv(GMT_bfn.foreground_rgb, &h2, &s2, &v2);
		GMT_rgb_to_hsv(GMT_bfn.nan_rgb, &h3, &s3, &v3);
		fprintf (GMT_stdout, format, 'B', h1, s1, v1);
		fprintf (GMT_stdout, format, 'F', h2, s2, v2);
		fprintf (GMT_stdout, format, 'N', h3, s3, v3);
	}
	else {
		fprintf (GMT_stdout, "B\t%d\t%d\t%d\n", GMT_bfn.background_rgb[0], GMT_bfn.background_rgb[1], GMT_bfn.background_rgb[2]);
		fprintf (GMT_stdout, "F\t%d\t%d\t%d\n", GMT_bfn.foreground_rgb[0], GMT_bfn.foreground_rgb[1], GMT_bfn.foreground_rgb[2]);
		fprintf (GMT_stdout, "N\t%d\t%d\t%d\n", GMT_bfn.nan_rgb[0], GMT_bfn.nan_rgb[1], GMT_bfn.nan_rgb[2]);
	}

	GMT_free ((void *)x);
	GMT_free ((void *)lut);
}

int GMT_get_index (double value)
{
	int index;
	int lo, hi, mid;
	
	if (GMT_is_dnan (value))	/* Set to NaN color */
		return (-1);
	if (value < GMT_lut[0].z_low)	/* Set to background color */
		return (-2);
	if (value > GMT_lut[GMT_n_colors-1].z_high)	/* Set to foreground color */
		return (-3);
	

	/* Must search for correct index */

	/* Speedup by Mika Heiskanen. This works if the colortable
	 * has been is sorted into increasing order. Careful when
	 * modifying the tests in the loop, the test and the mid+1
	 * parts are especially designed to make sure the loop
	 * converges to a single index.
	 */

        lo = 0;
        hi = GMT_n_colors - 1;
        while (lo != hi)
	{
		mid = (lo + hi) / 2;
		if (value >= GMT_lut[mid].z_high)
			lo = mid + 1;
		else
			hi = mid;
	}
        index = lo;
        if(value >= GMT_lut[index].z_low && value < GMT_lut[index].z_high) return (index);

        /* Slow search in case the table was not sorted
         * No idea whether it is possible, but it most certainly
         * does not hurt to have the code here as a backup.
         */
	
	index = 0;
	while (index < GMT_n_colors && ! (value >= GMT_lut[index].z_low && value < GMT_lut[index].z_high) ) index++;
	if (index == GMT_n_colors) index--;	/* Because we use <= for last range */
	return (index);
}

int GMT_get_rgb24 (double value, int *rgb)
{
	int index, i;
	double rel;
	
	index = GMT_get_index (value);
	
	if (index == -1) {
		memcpy ((void *)rgb, (void *)GMT_bfn.nan_rgb, 3 * sizeof (int));
		GMT_cpt_skip = GMT_bfn.skip[2];
	}
	else if (index == -2) {
		memcpy ((void *)rgb, (void *)GMT_bfn.background_rgb, 3 * sizeof (int));
		GMT_cpt_skip = GMT_bfn.skip[1];
	}
	else if (index == -3) {
		memcpy ((void *)rgb, (void *)GMT_bfn.foreground_rgb, 3 * sizeof (int));
		GMT_cpt_skip = GMT_bfn.skip[0];
	}
	else if (GMT_lut[index].skip) {		/* Set to page color for now */
		memcpy ((void *)rgb, (void *)gmtdefs.page_rgb, 3 * sizeof (int));
		GMT_cpt_skip = TRUE;
	}
	else {
		rel = (value - GMT_lut[index].z_low) * GMT_lut[index].i_dz;
		for (i = 0; i < 3; i++)
			rgb[i] = GMT_lut[index].rgb_low[i] + irint (rel * GMT_lut[index].rgb_diff[i]);
		GMT_cpt_skip = FALSE;
	}
	return (index);
}

void GMT_rgb_to_hsv (int rgb[], double *h, double *s, double *v)
{
	double xr, xg, xb, r_dist, g_dist, b_dist, max_v, min_v, diff, idiff;
	
	xr = rgb[0] * I_255;
	xg = rgb[1] * I_255;
	xb = rgb[2] * I_255;
	max_v = MAX (MAX (xr, xg), xb);
	min_v = MIN (MIN (xr, xg), xb);
	diff = max_v - min_v;
	*v = max_v;
	*s = (max_v == 0.0) ? 0.0 : diff / max_v;
	*h = 0.0;
	if ((*s) == 0.0) return;	/* Hue is undefined */
	idiff = 1.0 / diff;
	r_dist = (max_v - xr) * idiff;
	g_dist = (max_v - xg) * idiff;
	b_dist = (max_v - xb) * idiff;
	if (xr == max_v)
		*h = b_dist - g_dist;
	else if (xg == max_v)
		*h = 2.0 + r_dist - b_dist;
	else
		*h = 4.0 + g_dist - r_dist;
	(*h) *= 60.0;
	if ((*h) < 0.0) (*h) += 360.0;
}
	
void GMT_hsv_to_rgb (int rgb[], double h, double s, double v)
{
	int i;
	double f, p, q, t, rr, gg, bb;
	
	if (s == 0.0)
		rgb[0] = rgb[1] = rgb[2] = (int) floor (255.999 * v);
	else {
		while (h >= 360.0) h -= 360.0;
		h /= 60.0;
		i = (int)h;
		f = h - i;
		p = v * (1.0 - s);
		q = v * (1.0 - (s * f));
		t = v * (1.0 - (s * (1.0 - f)));
		switch (i) {
			case 0:
				rr = v;	gg = t;	bb = p;
				break;
			case 1:
				rr = q;	gg = v;	bb = p;
				break;
			case 2:
				rr = p;	gg = v;	bb = t;
				break;
			case 3:
				rr = p;	gg = q;	bb = v;
				break;
			case 4:
				rr = t;	gg = p;	bb = v;
				break;
			case 5:
				rr = v;	gg = p;	bb = q;
				break;
		}
		
		rgb[0] = (rr < 0.0) ? 0 : (int) floor (rr * 255.999);
		rgb[1] = (gg < 0.0) ? 0 : (int) floor (gg * 255.999);
		rgb[2] = (bb < 0.0) ? 0 : (int) floor (bb * 255.999);
	}
}

void GMT_illuminate (double intensity, int rgb[])
{
	double h, s, v, di;
	
	if (GMT_is_dnan (intensity)) return;
	if (intensity == 0.0) return;
	if (fabs (intensity) > 1.0) intensity = copysign (1.0, intensity);
	
	GMT_rgb_to_hsv (rgb, &h, &s, &v);
	if (intensity > 0) {
		di = 1.0 - intensity;
		if (s != 0.0) s = di * s + intensity * gmtdefs.hsv_max_saturation;
		v = di * v + intensity * gmtdefs.hsv_max_value;
	}
	else {
		di = 1.0 + intensity;
		if (s != 0.0) s = di * s - intensity * gmtdefs.hsv_min_saturation;
		v = di * v - intensity * gmtdefs.hsv_min_value;
	}
	if (v < 0.0) v = 0.0;
	if (s < 0.0) s = 0.0;
	if (v > 1.0) v = 1.0;
	if (s > 1.0) s = 1.0;
	GMT_hsv_to_rgb (rgb, h, s, v);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * AKIMA computes the coefficients for a quasi-cubic hermite spline.
 * Same algorithm as in the IMSL library.
 * Programmer:	Paul Wessel
 * Date:	16-JAN-1987
 * Ver:		v.1-pc
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

int GMT_akima (double *x, double *y, int nx, double *c)
{
	int i, no;
	double t1, t2, b, rm1, rm2, rm3, rm4;

	/* Assumes that n >= 4 and x is monotonically increaseing */

	rm3 = (y[1] - y[0])/(x[1] - x[0]);
	t1 = rm3 - (y[1] - y[2])/(x[1] - x[2]);
	rm2 = rm3 + t1;
	rm1 = rm2 + t1;
	
	/* get slopes */
	
	no = nx - 2;
	for (i = 0; i < nx; i++) {
		if (i >= no)
			rm4 = rm3 - rm2 + rm3;
		else
			rm4 = (y[i+2] - y[i+1])/(x[i+2] - x[i+1]);
		t1 = fabs(rm4 - rm3);
		t2 = fabs(rm2 - rm1);
		b = t1 + t2;
		c[3*i] = (b != 0.0) ? (t1*rm2 + t2*rm3) / b : 0.5*(rm2 + rm3);
		rm1 = rm2;
		rm2 = rm3;
		rm3 = rm4;
	}
	no = nx - 1;
	
	/* compute the coefficients for the nx-1 intervals */
	
	for (i = 0; i < no; i++) {
		t1 = 1.0 / (x[i+1] - x[i]);
		t2 = (y[i+1] - y[i])*t1;
		b = (c[3*i] + c[3*i+3] - t2 - t2)*t1;
		c[3*i+2] = b*t1;
		c[3*i+1] = -b + (t2 - c[3*i])*t1;
	}
	return (0);
}

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * GMT_cspline computes the coefficients for a natural cubic spline.
 * To evaluate, call GMT_csplint
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */
 
int GMT_cspline (double *x, double *y, int n, double *c)
{
	int i, k;
	double ip, s, dx1, i_dx2, *u;

	/* Assumes that n >= 4 and x is monotonically increaseing */

	u = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "GMT_cspline");
	c[1] = c[n] = u[1] = 0.0;
	for (i = 1; i < n-1; i++) {
		i_dx2 = 1.0 / (x[i+1] - x[i-1]);
		dx1 = x[i] - x[i-1];
		s = dx1 * i_dx2;
		ip = 1.0 / (s * c[i-1] + 2.0);
		c[i] = (s - 1.0) * ip;
		u[i] = (y[i+1] - y[i]) / (x[i+1] - x[i]) - (y[i] - y[i-1]) / dx1;
		u[i] = (6.0 * u[i] * i_dx2 - s * u[i-1]) * ip;
	}
	for (k = n-2; k >= 0; k--) c[k] = c[k] * c[k+1] + u[k];
	GMT_free ((void *)u);
	
	return (0);
}

double GMT_csplint (double *x, double *y, double *c, double xp, int klo)
{
	int khi;
	double h, ih, b, a, yp;

	khi = klo + 1;
	h = x[khi] - x[klo];
	ih = 1.0 / h;
	a = (x[khi] - xp) * ih;
	b = (xp - x[klo]) * ih;
	yp = a * y[klo] + b * y[khi] + ((a*a*a - a) * c[klo] + (b*b*b - b) * c[khi]) * (h*h) / 6.0;
	
	return (yp);
}
 
/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 * INTPOL will interpolate from the dataset <x,y> onto a new set <u,v>
 * where <x,y> and <u> is supplied by the user. <v> is returned. The 
 * parameter mode governs what interpolation scheme that will be used.
 * If u[i] is outside the range of x, then v[i] will contain NaN.
 *
 * input:  x = x-values of input data
 *	   y = y-values "    "     "
 *	   n = number of data pairs
 *	   m = number of desired interpolated values
 *	   u = x-values of these points
 *	mode = type of interpolation
 *   	  mode = 0 : Linear interpolation
 *   	  mode = 1 : Quasi-cubic hermite spline (GMT_akima)
 *   	  mode = 2 : Natural cubic spline (cubspl)
 * output: v = y-values at interpolated points
 *
 * Programmer:	Paul Wessel
 * Date:	16-JAN-1987
 * Ver:		v.2
 * - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
 */

int GMT_intpol (double *x, double *y, int n, int m, double *u, double *v, int mode)
{
	int i, j, err_flag = 0;
	BOOLEAN down = FALSE, check;
	double dx, *c, GMT_csplint (double *x, double *y, double *c, double xp, int klo);

	if (mode < 0) {	/* No need to check for sanity */
		check = FALSE;
		mode = -mode;
	}
	else
		check = TRUE;
		
	if (n < 4 || mode < 0 || mode > 3) mode = 0;

	if (check) {
		/* Check to see if x-values are monotonically increasing/decreasing */

		dx = x[1] - x[0];
		if (dx > 0.0) {
			for (i = 2; i < n && err_flag == 0; i++) {
				if ((x[i] - x[i-1]) <= 0.0) err_flag = i;
			}
		}
		else {
			down = TRUE;
			for (i = 2; i < n && err_flag == 0; i++) {
				if ((x[i] - x[i-1]) >= 0.0) err_flag = i;
			}
		}

		if (err_flag) {
			fprintf (stderr, "%s: GMT Fatal Error: x-values are not monotonically increasing/decreasing!\n", GMT_program);
			return (err_flag);
		}
	
		if (down) {	/* Must flip directions temporarily */
			for (i = 0; i < n; i++) x[i] = -x[i];
			for (i = 0; i < m; i++) u[i] = -u[i];
		}
	}

	if (mode > 0) c = (double *) GMT_memory (VNULL, (size_t)(3*n), sizeof(double), "GMT_intpol");

	if (mode == 1) 	/* Akimas spline */
		err_flag = GMT_akima (x, y, n, c);
	else if (mode == 2)	/* Natural cubic spline */
		err_flag = GMT_cspline (x, y, n, c);

	if (err_flag != 0) {
		GMT_free ((void *)c);
		return (err_flag);
	}
	
	/* Compute the interpolated values from the coefficients */
	
	j = 0;
	for (i = 0; i < m; i++) {
		if (u[i] < x[0] || u[i] > x[n-1]) {	/* Desired point outside data range */
			v[i] = GMT_d_NaN;
			continue;
		} 
		while (x[j] > u[i] && j > 0) j--;	/* In case u is not sorted */
		while (j < n && x[j] <= u[i]) j++;
		if (j == n) j--;
		if (j > 0) j--;

		switch (mode) {
			case 0:
				dx = u[i] - x[j];
				v[i] = (y[j+1]-y[j])*dx/(x[j+1]-x[j]) + y[j];
				break;
			case 1:
				dx = u[i] - x[j];
				v[i] = ((c[3*j+2]*dx + c[3*j+1])*dx + c[3*j])*dx + y[j];
				break;
			case 2:
				v[i] = GMT_csplint (x, y, c, u[i], j);
				break;
		}
	}
	if (mode > 0) GMT_free ((void *)c);

	if (down) {	/* Must reverse directions */
		for (i = 0; i < n; i++) x[i] = -x[i];
		for (i = 0; i < m; i++) u[i] = -u[i];
	}

	return (0);
}

void *GMT_memory (void *prev_addr, size_t nelem, size_t size, char *progname)
{
	void *tmp;

	if (nelem == 0) return(VNULL); /* Take care of n = 0 */
	
	if (prev_addr) {
		if ((tmp = realloc ((void *) prev_addr, (size_t)(nelem * size))) == VNULL) {
			fprintf (stderr, "GMT Fatal Error: %s could not reallocate more memory, n = %d\n", progname, nelem);
			exit (EXIT_FAILURE);
		}
	}
	else {
		if ((tmp = calloc ((size_t) nelem, (unsigned) size)) == VNULL) {
			fprintf (stderr, "GMT Fatal Error: %s could not allocate memory, n = %d\n", progname, nelem);
			exit (EXIT_FAILURE);
		}
	}
	return (tmp);
}

void GMT_free (void *addr)
{

	free (addr);
}

#define CONTOUR_IJ(i,j) ((i) + (j) * nx)

int GMT_contours (float *grd, struct GRD_HEADER *header, int smooth_factor, int int_scheme, int *side, int *edge, int first, double **x_array, double **y_array)
{
	/* The routine finds the zero-contour in the grd dataset.  it assumes that
	 * no node has a value exactly == 0.0.  If more than max points are found
	 * GMT_trace_contour will try to allocate more memory in blocks of GMT_CHUNK points
	 */
	 
	static int i0, j0;
	int i, j, ij, n = 0, n2, n_edges, edge_word, edge_bit, nx, ny, nans = 0;
	BOOLEAN go_on = TRUE;
	float z[2];
	double x0, y0, r, west, east, south, north, dx, dy, xinc2, yinc2, *x, *y, *x2, *y2;
	int p[5], i_off[5], j_off[5], k_off[5], offset;
	unsigned int bit[32];
	 
	nx = header->nx;	ny = header->ny;
	west = header->x_min;	east = header->x_max;
	south = header->y_min;	north = header->y_max;
	dx = header->x_inc;	dy = header->y_inc;
	xinc2 = (header->node_offset) ? 0.5 * dx : 0.0;
	yinc2 = (header->node_offset) ? 0.5 * dy : 0.0;
	x = *x_array;	y = *y_array;
	
	n_edges = ny * (int) ceil (nx / 16.0);
	offset = n_edges / 2;
	 
	/* Reset edge-flags to zero, if necessary */
	if (first) {	/* Set i0,j0 for southern boundary */
		memset ((void *)edge, 0, (size_t)(n_edges * sizeof (int)));
		i0 = 0;
		j0 = ny - 1;
	}
	p[0] = p[4] = 0;	p[1] = 1;	p[2] = 1 - nx;	p[3] = -nx;
	i_off[0] = i_off[2] = i_off[3] = i_off[4] = 0;	i_off[1] =  1;
	j_off[0] = j_off[1] = j_off[3] = j_off[4] = 0;	j_off[2] = -1;
	k_off[0] = k_off[2] = k_off[4] = 0;	k_off[1] = k_off[3] = 1;
	for (i = 1, bit[0] = 1; i < 32; i++) bit[i] = bit[i-1] << 1;

	switch (*side) {
		case 0:	/* Southern boundary */
			for (i = i0, j = j0, ij = (ny - 1) * nx + i0; go_on && i < nx-1; i++, ij++) {
				edge_word = ij / 32;
				edge_bit = ij % 32;
				z[0] = grd[ij+1];
				z[1] = grd[ij];
				if (GMT_z_periodic) GMT_setcontjump (z, 2);
				if (GMT_start_trace (z[0], z[1], edge, edge_word, edge_bit, bit)) { /* Start tracing countour */
					r = z[0] - z[1];
					x0 = west + (i - z[1]/r)*dx + xinc2;
					y0 = south + yinc2;
					edge[edge_word] |= bit[edge_bit];
					n = GMT_trace_contour (grd, header, x0, y0, edge, &x, &y, i, j, 0, offset, i_off, j_off, k_off, p, bit, &nans);
					n = GMT_smooth_contour (&x, &y, n, smooth_factor, int_scheme);
					go_on = FALSE;
					i0 = i + 1;	j0 = j;
				}
			}
			if (n == 0) {
				i0 = nx - 2;
				j0 = ny - 1;
				(*side)++;
			}
			break;
			
		case 1:		/* Eastern boundary */
		
			for (j = j0, ij = nx * (j0 + 1) - 1, i = i0; go_on && j > 0; j--, ij -= nx) {
				edge_word = ij / 32 + offset;
				edge_bit = ij % 32;
				z[0] = grd[ij-nx];
				z[1] = grd[ij];
				if (GMT_z_periodic) GMT_setcontjump (z, 2);
				if (GMT_start_trace (z[0], z[1], edge, edge_word, edge_bit, bit)) { /* Start tracing countour */
					r = z[1] - z[0];
					x0 = east - xinc2;
					y0 = north - (j - z[1]/r) * dy - yinc2;
					edge[edge_word] |= bit[edge_bit];
					n = GMT_trace_contour (grd, header, x0, y0, edge, &x, &y, i, j, 1, offset, i_off, j_off, k_off, p, bit, &nans);
					n = GMT_smooth_contour (&x, &y, n, smooth_factor, int_scheme);
					go_on = FALSE;
					i0 = i;	j0 = j - 1;
				}
			}
			if (n == 0) {
				i0 = nx - 2;
				j0 = 1;
				(*side)++;
			}
			break;

		case 2:		/* Northern boundary */	
		
			for (i = i0, j = j0, ij = i0; go_on && i >= 0; i--, ij--) {
				edge_word = ij / 32;
				edge_bit = ij % 32;
				z[0] = grd[ij];
				z[1] = grd[ij+1];
				if (GMT_z_periodic) GMT_setcontjump (z, 2);
				if (GMT_start_trace (z[0], z[1], edge, edge_word, edge_bit, bit)) { /* Start tracing countour */
					r = z[1] - z[0];
					x0 = west + (i - z[0]/r)*dx + xinc2;
					y0 = north - yinc2;
					edge[edge_word] |= bit[edge_bit];
					n = GMT_trace_contour (grd, header, x0, y0, edge, &x, &y, i, j, 2, offset, i_off, j_off, k_off, p, bit, &nans);
					n = GMT_smooth_contour (&x, &y, n, smooth_factor, int_scheme);
					go_on = FALSE;
					i0 = i - 1;	j0 = j;
				}
			}
			if (n == 0) {
				i0 = 0;
				j0 = 1;
				(*side)++;
			}
			break;
		
		case 3:		/* Western boundary */
		
			for (j = j0, ij = j * nx, i = i0; go_on && j < ny; j++, ij += nx) {
				edge_word = ij / 32 + offset;
				edge_bit = ij % 32;
				z[0] = grd[ij];
				z[1] = grd[ij-nx];
				if (GMT_z_periodic) GMT_setcontjump (z, 2);
				if (GMT_start_trace (z[0], z[1], edge, edge_word, edge_bit, bit)) { /* Start tracing countour */
					r = z[0] - z[1];
					x0 = west + xinc2;
					y0 = north - (j - z[0] / r) * dy - yinc2;
					edge[edge_word] |= bit[edge_bit];
					n = GMT_trace_contour (grd, header, x0, y0, edge, &x, &y, i, j, 3, offset, i_off, j_off, k_off, p, bit, &nans);
					n = GMT_smooth_contour (&x, &y, n, smooth_factor, int_scheme);
					go_on = FALSE;
					i0 = i;	j0 = j + 1;
				}
			}
			if (n == 0) {
				i0 = 1;
				j0 = 1;
				(*side)++;
			}
			break;

		case 4:	/* Then loop over interior boxes */
		
			for (j = j0; go_on && j < ny; j++) {
				ij = i0 + j * nx;
				for (i = i0; go_on && i < nx-1; i++, ij++) {
					edge_word = ij / 32 + offset;
					z[0] = grd[ij];
					z[1] = grd[ij-nx];
					edge_bit = ij % 32;
					if (GMT_z_periodic) GMT_setcontjump (z, 2);
					if (GMT_start_trace (z[0], z[1], edge, edge_word, edge_bit, bit)) { /* Start tracing countour */
						r = z[0] - z[1];
						x0 = west + i*dx + xinc2;
						y0 = north - (j - z[0] / r) * dy - yinc2;
						/* edge[edge_word] |= bit[edge_bit]; */
						nans = 0;
						n = GMT_trace_contour (grd, header, x0, y0, edge, &x, &y, i, j, 3, offset, i_off, j_off, k_off, p, bit, &nans);
						if (nans) {	/* Must trace in other direction, then splice */
							n2 = GMT_trace_contour (grd, header, x0, y0, edge, &x2, &y2, i-1, j, 1, offset, i_off, j_off, k_off, p, bit, &nans);
							n = GMT_splice_contour (&x, &y, n, &x2, &y2, n2);
							GMT_free ((void *)x2);
							GMT_free ((void *)y2);
							
							n = GMT_smooth_contour (&x, &y, n, smooth_factor, int_scheme);
						}
						i0 = i + 1;
						go_on = FALSE;
						j0 = j;
					}
				}
				if (go_on) i0 = 1;
			}
			if (n == 0) (*side)++;
			break;
		default:
			break;
	}
	*x_array = x;	*y_array = y;
	return (n);
}

int GMT_start_trace (float first, float second, int *edge, int edge_word, int edge_bit, unsigned int *bit)
{
	/* First make sure we havent been here before */

	if ( (edge[edge_word] & bit[edge_bit]) ) return (FALSE);

	/* Then make sure both points are real numbers */

	if (GMT_is_fnan (first) ) return (FALSE);
	if (GMT_is_fnan (second) ) return (FALSE);

	/* Finally return TRUE if values of opposite sign */

	return ( first * second < 0.0);
}

int GMT_splice_contour (double **x, double **y, int n, double **x2, double **y2, int n2)
{	/* Basically does a "tail -r" on the array x,y and append arrays x2/y2 */

	int i, j, m;
	double *xtmp, *ytmp, *xa, *ya, *xb, *yb;
	
	xa = *x;	ya = *y;
	xb = *x2;	yb = *y2;
	
	m = n + n2 - 1;	/* Total length since one point is shared */
	
	/* First copy and reverse first contour piece */
	
	xtmp = (double *) GMT_memory (VNULL, (size_t)n , sizeof (double), "GMT_splice_contour");
	ytmp = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "GMT_splice_contour");
	
	memcpy ((void *)xtmp, (void *)xa, (size_t)(n * sizeof (double)));
	memcpy ((void *)ytmp, (void *)ya, (size_t)(n * sizeof (double)));
	
	xa = (double *) GMT_memory ((void *)xa, (size_t)m, sizeof (double), "GMT_splice_contour");
	ya = (double *) GMT_memory ((void *)ya, (size_t)m, sizeof (double), "GMT_splice_contour");
	
	for (i = 0; i < n; i++) xa[i] = xtmp[n-i-1];
	for (i = 0; i < n; i++) ya[i] = ytmp[n-i-1];
	
	/* Then append second piece */
	
	for (j = 1, i = n; j < n2; j++, i++) xa[i] = xb[j];
	for (j = 1, i = n; j < n2; j++, i++) ya[i] = yb[j];

	GMT_free ((void *)xtmp);
	GMT_free ((void *)ytmp);
	
	*x = xa;
	*y = ya;
	
	return (m);
}
	
int GMT_trace_contour (float *grd, struct GRD_HEADER *header, double x0, double y0, int *edge, double **x_array, double **y_array, int i, int j, int kk, int offset, int *i_off, int *j_off, int *k_off, int *p, unsigned int *bit, int *nan_flag)
{
	int side = 0, n = 1, k, k0, ij, ij0, n_cuts, k_index[2], kk_opposite, more;
	int edge_word, edge_bit, m, n_nan, nx, ny, n_alloc, ij_in;
	float z[5];
	double xk[4], yk[4], r, dr[2];
	double west, north, dx, dy, xinc2, yinc2, *xx, *yy;
	
	west = header->x_min;	north = header->y_max;
	dx = header->x_inc;	dy = header->y_inc;
	nx = header->nx;	ny = header->ny;
	xinc2 = (header->node_offset) ? 0.5 * dx : 0.0;
	yinc2 = (header->node_offset) ? 0.5 * dy : 0.0;
	
	n_alloc = GMT_CHUNK;
	m = n_alloc - 2;

	xx = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_trace_contour");
	yy = (double *) GMT_memory (VNULL, (size_t)n_alloc, sizeof (double), "GMT_trace_contour");
	
	xx[0] = x0;	yy[0] = y0;
	ij_in = j * nx + i - 1;
	
	more = TRUE;
	do {
		ij = i + j * nx;
		x0 = west + i * dx + xinc2;
		y0 = north - j * dy - yinc2;
		n_cuts = 0;
		k0 = kk;
		for (k = 0; k < 5; k++) z[k] = grd[ij+p[k]];	/* Copy the 4 corners */
		if (GMT_z_periodic) GMT_setcontjump (z, 5);

		for (k = n_nan = 0; k < 4; k++) {	/* Loop over box sides */

			/* Skip where we already have a cut (k == k0) */
			
			if (k == k0) continue;
			
			/* Skip if NAN encountered */
			
			if (GMT_is_fnan (z[k+1]) || GMT_is_fnan (z[k])) {
				n_nan++;
				continue;
			}
			
			/* Skip edge already has been used */
			
			ij0 = CONTOUR_IJ (i + i_off[k], j + j_off[k]);
			edge_word = ij0 / 32 + k_off[k] * offset;
			edge_bit = ij0 % 32;
			if (edge[edge_word] & bit[edge_bit]) continue;
			
			/* Skip if no zero-crossing on this edge */
			
			if (z[k+1] * z[k] > 0.0) continue;
			
			/* Here we have a crossing */
			
			r = z[k+1] - z[k];
			
			if (k%2) {	/* Cutting a S-N line */
				if (k == 1) {
					xk[1] = x0 + dx;
					yk[1] = y0 - z[k]*dy/r;
				}
				else {
					xk[3] = x0;
					yk[3] = y0 + (1.0 + z[k]/r)*dy;
				}
			}
			else {	/* Cutting a E-W line */
				if (k == 0) {
					xk[0] = x0 - z[k]*dx/r;
					yk[0] = y0;
				}
				else {
					xk[2] = x0 + (1.0 + z[k]/r)*dx;
					yk[2] = y0 + dy;
				}
			}
			kk = k;
			n_cuts++;
		}
		
		if (n > m) {	/* Must try to allocate more memory */
			n_alloc += GMT_CHUNK;
			m += GMT_CHUNK;
			xx = (double *) GMT_memory ((void *)xx, (size_t)n_alloc, sizeof (double), "GMT_trace_contour");
			yy = (double *) GMT_memory ((void *)yy, (size_t)n_alloc, sizeof (double), "GMT_trace_contour");
		}
		if (n_cuts == 0) {	/* Close interior contour and return */
		/*	if (n_nan == 0 && (n > 0 && fabs (xx[n-1] - xx[0]) <= dx && fabs (yy[n-1] - yy[0]) <= dy)) { */
			if (ij == ij_in) {	/* Close iterior contour */
				xx[n] = xx[0];
				yy[n] = yy[0];
				n++;
			}
			more = FALSE;
			*nan_flag = n_nan;
		}
		else if (n_cuts == 1) {	/* Draw a line to this point and keep tracing */
			xx[n] = xk[kk];
			yy[n] = yk[kk];
			n++;
		}
		else {	/* Saddle point, we decide to connect to the point nearest previous point */
			kk_opposite = (k0 + 2) % 4;	/* But it cannot be on opposite side */
			for (k = side = 0; k < 4; k++) {
				if (k == k0 || k == kk_opposite) continue;
				dr[side] = (xx[n-1]-xk[k])*(xx[n-1]-xk[k]) + (yy[n-1]-yk[k])*(yy[n-1]-yk[k]);
				k_index[side++] = k;
			}
			kk = (dr[0] < dr[1]) ? k_index[0] : k_index[1];
			xx[n] = xk[kk];
			yy[n] = yk[kk];
			n++;
		}
		if (more) {	/* Mark this edge as used */
			ij0 = CONTOUR_IJ (i + i_off[kk], j + j_off[kk]);
			edge_word = ij0 / 32 + k_off[kk] * offset;
			edge_bit = ij0 % 32;
			edge[edge_word] |= bit[edge_bit];	
		}
		
		if ((kk == 0 && j == ny - 1) || (kk == 1 && i == nx - 2) ||
			(kk == 2 && j == 1) || (kk == 3 && i == 0))	/* Going out of grid */
			more = FALSE;
			
		/* Get next box (i,j,kk) */
		
		i -= (kk-2)%2;
		j -= (kk-1)%2;
		kk = (kk+2)%4;
		
	} while (more);
	
	xx = (double *) GMT_memory ((void *)xx, (size_t)n, sizeof (double), "GMT_trace_contour");
	yy = (double *) GMT_memory ((void *)yy, (size_t)n, sizeof (double), "GMT_trace_contour");
	
	*x_array = xx;	*y_array = yy;
	return (n);
}

int GMT_smooth_contour (double **x_in, double **y_in, int n, int sfactor, int stype)
                        	/* Input (x,y) points */
      		/* Number of input points */
            	/* n_out = sfactor * n -1 */
           {	/* Interpolation scheme used (0 = linear, 1 = Akima, 2 = Cubic spline */
	int i, j, k, n_out;
	double ds, t_next, *x, *y;
	double *t_in, *t_out, *x_tmp, *y_tmp, x0, x1, y0, y1;
	char *flag;
	
	if (sfactor == 0 || n < 4) return (n);	/* Need at least 4 points to call Akima */
	
	x = *x_in;	y = *y_in;
	
	n_out = sfactor * n - 1;	/* Number of new points */
	
	t_in = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "GMT_smooth_contour");
	t_out = (double *) GMT_memory (VNULL, (size_t)(n_out + n), sizeof (double), "GMT_smooth_contour");
	x_tmp = (double *) GMT_memory (VNULL, (size_t)(n_out + n), sizeof (double), "GMT_smooth_contour");
	y_tmp = (double *) GMT_memory (VNULL, (size_t)(n_out + n), sizeof (double), "GMT_smooth_contour");
	flag = (char *)GMT_memory (VNULL, (size_t)(n_out + n), sizeof (char), "GMT_smooth_contour");
	
	/* Create dummy distance values for input points, and throw out duplicate points while at it */
	
	t_in[0] = 0.0;
	for (i = j = 1; i < n; i++)	{
		ds = hypot ((x[i]-x[i-1]), (y[i]-y[i-1]));
		if (ds > 0.0) {
			t_in[j] = t_in[j-1] + ds;
			x[j] = x[i];
			y[j] = y[i];
			j++;
		}
	}
	
	n = j;	/* May have lost some duplicates */
	if (sfactor == 0 || n < 4) return (n);	/* Need at least 4 points to call Akima */
	
	/* Create equidistance output points */
	
	ds = t_in[n-1] / (n_out-1);
	t_next = ds;
	t_out[0] = 0.0;
	flag[0] = TRUE;
	for (i = j = 1; i < n_out; i++) {
		if (j < n && t_in[j] < t_next) {
			t_out[i] = t_in[j];
			flag[i] = TRUE;
			j++;
			n_out++;
		}
		else {
			t_out[i] = t_next;
			t_next += ds;
		}
	}
	t_out[n_out-1] = t_in[n-1];
	if (t_out[n_out-1] == t_out[n_out-2]) n_out--;
	flag[n_out-1] = TRUE;
	
	GMT_intpol (t_in, x, n, n_out, t_out, x_tmp, stype);
	GMT_intpol (t_in, y, n, n_out, t_out, y_tmp, stype);
	
	/* Make sure interpolated function is bounded on each segment interval */
	
	i = 0;
	while (i < (n_out - 1)) {
		j = i + 1;
		while (j < n_out && !flag[j]) j++;
		x0 = x_tmp[i];	x1 = x_tmp[j];
		if (x0 > x1) d_swap (x0, x1);
		y0 = y_tmp[i];	y1 = y_tmp[j];
		if (y0 > y1) d_swap (y0, y1);
		for (k = i + 1; k < j; k++) {
			if (x_tmp[k] < x0)
				x_tmp[k] = x0 + 1.0e-10;
			else if (x_tmp[k] > x1)
				x_tmp[k] = x1 - 1.0e-10;
			if (y_tmp[k] < y0)
				y_tmp[k] = y0 + 1.0e-10;
			else if (y_tmp[k] > y1)
				y_tmp[k] = y1 - 1.0e-10;
		}
		i = j;
	}
	
	GMT_free ((void *)x);
	GMT_free ((void *)y);
	
	*x_in = x_tmp;
	*y_in = y_tmp;
	
	GMT_free ((void *) t_in);
	GMT_free ((void *) t_out);
	GMT_free ((void *) flag);
	
	return (n_out);
}

void GMT_get_plot_array (void) {      /* Allocate more space for plot arrays */

	GMT_n_alloc += GMT_CHUNK;
	GMT_x_plot = (double *) GMT_memory ((void *)GMT_x_plot, (size_t)GMT_n_alloc, sizeof (double), "gmt");
	GMT_y_plot = (double *) GMT_memory ((void *)GMT_y_plot, (size_t)GMT_n_alloc, sizeof (double), "gmt");
	GMT_pen = (int *) GMT_memory ((void *)GMT_pen, (size_t)GMT_n_alloc, sizeof (int), "gmt");
}

int GMT_comp_double_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	int bad_1, bad_2;
	double *point_1, *point_2;
	
	point_1 = (double *)p_1;
	point_2 = (double *)p_2;
	bad_1 = GMT_is_dnan ((*point_1));
	bad_2 = GMT_is_dnan ((*point_2));
	
	if (bad_1 && bad_2) return (0);
	if (bad_1) return (1);
	if (bad_2) return (-1);
	
	if ( (*point_1) < (*point_2) )
		return (-1);
	else if ( (*point_1) > (*point_2) )
		return (1);
	else
		return (0);
}

int GMT_comp_float_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	int bad_1, bad_2;
	float	*point_1, *point_2;

	point_1 = (float *)p_1;
	point_2 = (float *)p_2;
	bad_1 = GMT_is_fnan ((*point_1));
	bad_2 = GMT_is_fnan ((*point_2));
	
	if (bad_1 && bad_2) return (0);
	if (bad_1) return (1);
	if (bad_2) return (-1);
	
	if ( (*point_1) < (*point_2) )
		return (-1);
	else if ( (*point_1) > (*point_2) )
		return (1);
	else
		return (0);
}

int GMT_comp_int_asc (const void *p_1, const void *p_2)
{
	/* Returns -1 if point_1 is < that point_2,
	   +1 if point_2 > point_1, and 0 if they are equal
	*/
	int *point_1, *point_2;

	point_1 = (int *)p_1;
	point_2 = (int *)p_2;
	if ( (*point_1) < (*point_2) )
		return (-1);
	else if ( (*point_1) > (*point_2) )
		return (1);
	else
		return (0);
}

#define PADDING 72	/* Amount of padding room for anotations in points */

struct EPS *GMT_epsinfo (char *program)
{
	/* Supply info about the EPS file that will be created */
	 
	int fno[5], id, i, n, n_fonts, last, move_up = FALSE, old_x0, old_y0, old_x1, old_y1;
	int tick_space, frame_space, u_dx, u_dy;
	double dy, x0, y0, orig_x0 = 0.0, orig_y0 = 0.0;
	char title[BUFSIZ];
	FILE *fp;
	struct passwd *pw;
	struct EPS *new;

	new = (struct EPS *) GMT_memory (VNULL, (size_t)1, sizeof (struct EPS), "GMT_epsinfo");
	 
	 /* First crudely estimate the boundingbox coordinates */

	if (gmtdefs.overlay && (fp = fopen (".GMT_bb_info", "r")) != NULL) {	/* Must get previous boundingbox values */
		fscanf (fp, "%d %lf %lf %d %d %d %d\n", &(new->portrait), &orig_x0, &orig_y0, &old_x0, &old_y0, &old_x1, &old_y1);
		x0 = orig_x0;
		y0 = orig_y0;
		fclose (fp);
		if (gmtdefs.page_orientation & 8) {	/* Absolute */
			x0 = gmtdefs.x_origin;
			y0 = gmtdefs.y_origin;
		}
		else {					/* Relative */
			x0 += gmtdefs.x_origin;
			y0 += gmtdefs.y_origin;
		}
	}
	else {	/* New plot, initialize stuff */
		old_x0 = old_y0 = old_x1 = old_y1 = 0;
		x0 = gmtdefs.x_origin;	/* Always absolute the first time */
		y0 = gmtdefs.y_origin;
		new->portrait = (gmtdefs.page_orientation & 1);
	}

	/* Estinates the bounding box for this overlay */

	new->x0 = irint (GMT_u2u[GMT_INCH][GMT_PT] * x0);
	new->y0 = irint (GMT_u2u[GMT_INCH][GMT_PT] * y0);
	new->x1 = new->x0 + irint (GMT_u2u[GMT_INCH][GMT_PT] * (z_project.xmax - z_project.xmin));
	new->y1 = new->y0 + irint (GMT_u2u[GMT_INCH][GMT_PT] * (z_project.ymax - z_project.ymin));
	 
	tick_space = (gmtdefs.tick_length > 0.0) ? irint (GMT_u2u[GMT_INCH][GMT_PT] * gmtdefs.tick_length) : 0;
	frame_space = irint (GMT_u2u[GMT_INCH][GMT_PT] * gmtdefs.frame_width);
	if (frame_info.header[0]) {	/* Make space for header text */
		move_up = (MAPPING || frame_info.side[2] == 2);
		dy = ((move_up) ? (gmtdefs.anot_font_size + gmtdefs.label_font_size) * GMT_u2u[GMT_PT][GMT_INCH] : 0.0) + 2.5 * gmtdefs.anot_offset;
		new->y1 += tick_space + irint (GMT_u2u[GMT_INCH][GMT_PT] * dy);
	}
	
	/* Find the max extent and use it if the overlay exceeds what we already have */

	/* Extend box in all directions depending on what kind of frame annotations we have */

	u_dx = (gmtdefs.unix_time && gmtdefs.unix_time_pos[0] < 0.0) ? -irint (GMT_u2u[GMT_INCH][GMT_PT] * gmtdefs.unix_time_pos[0]) : 0;
	u_dy = (gmtdefs.unix_time && gmtdefs.unix_time_pos[1] < 0.0) ? -irint (GMT_u2u[GMT_INCH][GMT_PT] * gmtdefs.unix_time_pos[1]) : 0;
	if (frame_info.plot && !project_info.three_D) {
		if (frame_info.side[3]) new->x0 -= MAX (u_dx, ((frame_info.side[3] == 2) ? PADDING : tick_space)); else new->x0 -= MAX (u_dx, frame_space);
		if (frame_info.side[0]) new->y0 -= MAX (u_dy, ((frame_info.side[0] == 2) ? PADDING : tick_space)); else new->y0 -= MAX (u_dy, frame_space);
		if (frame_info.side[1]) new->x1 += (frame_info.side[1] == 2) ? PADDING : tick_space; else new->x1 += frame_space;
		if (frame_info.side[2]) new->y1 += (frame_info.header[0] || frame_info.side[2] == 2) ? PADDING : tick_space; else new->y1 += frame_space;
	}
	else if (project_info.three_D) {
		new->x0 -= MAX (u_dx, PADDING/2);
		new->y0 -= MAX (u_dy, PADDING/2);
		new->x1 += PADDING/2;
		new->y1 += PADDING/2;
	}
	else if (gmtdefs.unix_time) {
		new->x0 -= u_dx;
		new->y0 -= u_dy;
	}

	/* Get the high water mark in all directions */

	if (gmtdefs.overlay) {
		new->x0 = MIN (old_x0, new->x0);
		new->y0 = MIN (old_y0, new->y0);
	}
	new->x1 = MAX (old_x1, new->x1);
	new->y1 = MAX (old_y1, new->y1);

	if (gmtdefs.page_orientation & 8) {	/* Undo Absolute */
		x0 = orig_x0;
		y0 = orig_y0;
	}

	/* Update the bb file or tell use */

	if (gmtdefs.last_page) {	/* Clobber the .GMT_bb_info file and add label padding */
		(void) remove (".GMT_bb_info");	/* Don't really care if it is successful or not */

	}
	else if ((fp = fopen (".GMT_bb_info", "w")) != NULL) {	/* Update the .GMT_bb_info file */
		fprintf (fp, "%d %lg %lg %d %d %d %d\n", new->portrait, x0, y0, new->x0, new->y0, new->x1, new->y1);
		fclose (fp);
	}

	/* Get font names used */
	
	id = 0;
	if (gmtdefs.unix_time) {
		fno[0] = 0;
		fno[1] = 1;
		id = 2;
	}
	
	if (frame_info.header[0]) fno[id++] = gmtdefs.header_font;
	
	if (frame_info.label[0][0] || frame_info.label[1][0] || frame_info.label[2][0]) fno[id++] = gmtdefs.label_font;
	
	fno[id++] = gmtdefs.anot_font;
	
	qsort ((void *)fno, (size_t)id, sizeof (int), GMT_comp_int_asc);
	
	last = -1;
	for (i = n_fonts = 0; i < id; i++) {
		if (fno[i] != last) {
			new->fontno[n_fonts] = fno[i];
			new->font[n_fonts++] = GMT_font_name[fno[i]];
			last = fno[i];
		}
	}
	new->font[n_fonts] = CNULL;
	
	/* Get user name and date */

	if ((pw = getpwuid (getuid ())) != NULL) {

		n = strlen (pw->pw_name) + 1;
		new->name = (char *) GMT_memory (VNULL, (size_t)n, sizeof (char), "GMT_epsinfo");
		strcpy (new->name, pw->pw_name);
	}
	else
	{
		new->name = (char *) GMT_memory (VNULL, (size_t)8, sizeof (char), "GMT_epsinfo");
		strcpy (new->name, "unknown");
	}	
	sprintf (title, "GMT v%s Document from %s\0", GMT_VERSION, program);
	new->title = (char *) GMT_memory (VNULL, (size_t)(strlen (title) + 1), sizeof (char), "GMT_epsinfo");
	strcpy (new->title, title);
	
	return (new);
}

int GMT_get_format (double interval, char *unit, char *format)
{
	int i, j, ndec = 0;
	char text[128];
	
	if (strchr (gmtdefs.d_format, 'g')) {	/* General format requested */

		/* Find number of decimals needed in the format statement */
	
		sprintf (text, "%.12lg\0", interval);
		for (i = 0; text[i] && text[i] != '.'; i++);
		if (text[i]) {	/* Found a decimal point */
			for (j = i + 1; text[j]; j++);
			ndec = j - i - 1;
		}
	}
	
	if (unit && unit[0]) {	/* Must append the unit string */
		if (!strchr (unit, '%'))	/* No percent signs */
			strncpy (text, unit, 80);
		else {
			for (i = j = 0; i < (int)strlen (unit); i++) {
				text[j++] = unit[i];
				if (unit[i] == '%') text[j++] = unit[i];
			}
			text[j] = 0;
		}
		if (text[0] == '-') {	/* No space between anotation and unit */
			if (ndec > 0)
				sprintf (format, "%%.%dlf%s\0", ndec, &text[1]);
			else
				sprintf (format, "%s%s\0", gmtdefs.d_format, &text[1]);
		}
		else {			/* 1 space between anotation and unit */
			if (ndec > 0)
				sprintf (format, "%%.%dlf %s\0", ndec, text);
			else
				sprintf (format, "%s %s\0", gmtdefs.d_format, text);
		}
		if (ndec == 0) ndec = 1;	/* To avoid resetting format later */
	}
	else if (ndec > 0)
		sprintf (format, "%%.%dlf\0", ndec);
	else
		strcpy (format, gmtdefs.d_format);

	return (ndec);
}

int	GMT_non_zero_winding(double xp, double yp, double *x, double *y, int n_path)
{
	/* Routine returns (2) if (xp,yp) is inside the
	   polygon x[n_path], y[n_path], (0) if outside,
	   and (1) if exactly on the path edge.
	   Uses non-zero winding rule in Adobe PostScript
	   Language reference manual, section 4.6 on Painting.
	   Path should have been closed first, so that
	   x[n_path-1] = x[0], and y[n_path-1] = y[0].

	   This is version 2, trying to kill a bug
	   in above routine:  If point is on X edge,
	   fails to discover that it is on edge.

	   We are imagining a ray extending "up" from the
	   point (xp,yp); the ray has equation x = xp, y >= yp.
	   Starting with crossing_count = 0, we add 1 each time
	   the path crosses the ray in the +x direction, and
	   subtract 1 each time the ray crosses in the -x direction.
	   After traversing the entire polygon, if (crossing_count)
	   then point is inside.  We also watch for edge conditions.

	   If two or more points on the path have x[i] == xp, then
	   we have an ambiguous case, and we have to find the points
	   in the path before and after this section, and check them.
	   */
	
	int	i, j, k, jend, crossing_count, above;
	double	y_sect;

	above = FALSE;
	crossing_count = 0;

	/* First make sure first point in path is not a special case:  */
	j = jend = n_path - 1;
	if (x[j] == xp) {
		/* Trouble already.  We might get lucky:  */
		if (y[j] == yp) return(1);

		/* Go backward down the polygon until x[i] != xp:  */
		if (y[j] > yp) above = TRUE;
		i = j - 1;
		while (x[i] == xp && i > 0) {
			if (y[i] == yp) return (1);
			if (!(above) && y[i] > yp) above = TRUE;
			i--;
		}

		/* Now if i == 0 polygon is degenerate line x=xp;
		   since we know xp,yp is inside bounding box,
		   it must be on edge:  */
		if (i == 0) return(1);

		/* Now we want to mark this as the end, for later:  */
		jend = i;

		/* Now if (j-i)>1 there are some segments the point could be exactly on:  */
		for (k = i+1; k < j; k++) {
			if ( (y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp) ) return (1);
		}


		/* Now we have arrived where i is > 0 and < n_path-1, and x[i] != xp.
			We have been using j = n_path-1.  Now we need to move j forward 
			from the origin:  */
		j = 1;
		while (x[j] == xp) {
			if (y[j] == yp) return (1);
			if (!(above) && y[j] > yp) above = TRUE;
			j++;
		}

		/* Now at the worst, j == jstop, and we have a polygon with only 1 vertex
			not at x = xp.  But now it doesn't matter, that would end us at
			the main while below.  Again, if j>=2 there are some segments to check:  */
		for (k = 0; k < j-1; k++) {
			if ( (y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp) ) return (1);
		}


		/* Finally, we have found an i and j with points != xp.  If (above) we may have crossed the ray:  */
		if (above && x[i] < xp && x[j] > xp) 
			crossing_count++;
		else if (above && x[i] > xp && x[j] < xp) 
			crossing_count--;

		/* End nightmare scenario for x[0] == xp.  */
	}

	else {
		/* Get here when x[0] != xp:  */
		i = 0;
		j = 1;
		while (x[j] == xp && j < jend) {
			if (y[j] == yp) return (1);
			if (!(above) && y[j] > yp) above = TRUE;
			j++;
		}
		/* Again, if j==jend, (i.e., 0) then we have a polygon with only 1 vertex
			not on xp and we will branch out below.  */

		/* if ((j-i)>2) the point could be on intermediate segments:  */
		for (k = i+1; k < j-1; k++) {
			if ( (y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp) ) return (1);
		}

		/* Now we have x[i] != xp and x[j] != xp.
			If (above) and x[i] and x[j] on opposite sides, we are certain to have crossed the ray.
			If not (above) and (j-i)>1, then we have not crossed it.
			If not (above) and j-i == 1, then we have to check the intersection point.  */

		if (x[i] < xp && x[j] > xp) {
			if (above) 
				crossing_count++;
			else if ( (j-i) == 1) {
				y_sect = y[i] + (y[j] - y[i]) * ( (xp - x[i]) / (x[j] - x[i]) );
				if (y_sect == yp) return (1);
				if (y_sect > yp) crossing_count++;
			}
		}
		if (x[i] > xp && x[j] < xp) {
			if (above) 
				crossing_count--;
			else if ( (j-i) == 1) {
				y_sect = y[i] + (y[j] - y[i]) * ( (xp - x[i]) / (x[j] - x[i]) );
				if (y_sect == yp) return (1);
				if (y_sect > yp) crossing_count--;
			}
		}
					
		/* End easier case for x[0] != xp  */
	}

	/* Now MAIN WHILE LOOP begins:
		Set i = j, and search for a new j, and do as before.  */

	i = j;
	while (i < jend) {
		above = FALSE;
		j = i+1;
		while (x[j] == xp) {
			if (y[j] == yp) return (1);
			if (!(above) && y[j] > yp) above = TRUE;
			j++;
		}
		/* if ((j-i)>2) the point could be on intermediate segments:  */
		for (k = i+1; k < j-1; k++) {
			if ( (y[k] <= yp && y[k+1] >= yp) || (y[k] >= yp && y[k+1] <= yp) ) return (1);
		}

		/* Now we have x[i] != xp and x[j] != xp.
			If (above) and x[i] and x[j] on opposite sides, we are certain to have crossed the ray.
			If not (above) and (j-i)>1, then we have not crossed it.
			If not (above) and j-i == 1, then we have to check the intersection point.  */

		if (x[i] < xp && x[j] > xp) {
			if (above) 
				crossing_count++;
			else if ( (j-i) == 1) {
				y_sect = y[i] + (y[j] - y[i]) * ( (xp - x[i]) / (x[j] - x[i]) );
				if (y_sect == yp) return (1);
				if (y_sect > yp) crossing_count++;
			}
		}
		if (x[i] > xp && x[j] < xp) {
			if (above) 
				crossing_count--;
			else if ( (j-i) == 1) {
				y_sect = y[i] + (y[j] - y[i]) * ( (xp - x[i]) / (x[j] - x[i]) );
				if (y_sect == yp) return (1);
				if (y_sect > yp) crossing_count--;
			}
		}

		/* That's it for this piece.  Advance i:  */

		i = j;
	}

	/* End of MAIN WHILE.  Get here when we have gone all around without landing on edge.  */

	if (crossing_count)
		return(2);
	else
		return(0);
}

/* GMT can either compile with its standard Delaunay triangulation routine
 * based on the work by Dave Watson, OR you may link with the triangle.o
 * module from Jonathan Shewchuck, Berkely U.  By default, the former is
 * chosen unless the compiler directive -DTRIANGLE_D is passed.  The latter
 * is much faster and will hopefullty become the standard once we sort out
 * copyright issues etc.
 */

#ifdef TRIANGLE_D

/*
 * New GMT_delaunay interface routine that calls the triangulate function
 * developed by Jonathan Richard Shewchuk, Berkeley University.
 * Suggested by alert GMT user Alain Coat.  You need to get triangle.c and
 * triangle.h from www.cs.cmu.edu/
 */

#define REAL double

#include "triangle.h"

int GMT_delaunay (double *x_in, double *y_in, int n, int **link)
{
	/* GMT interface to the triangle package; see above for references.
	 * All that is done is reformatting of parameters and calling the
	 * main triangulate routine.  Thanx to Alain Coat for the tip.
	 */

	int i, j;
	struct triangulateio In, Out, vorOut;

	/* Set everything to 0 and NULL */

	memset ((void *)&In,	 0, sizeof (struct triangulateio));
	memset ((void *)&Out,	 0, sizeof (struct triangulateio));
	memset ((void *)&vorOut, 0, sizeof (struct triangulateio));

	/* Allocate memory for input points */

	In.numberofpoints = n;
	In.pointlist = (double *) GMT_memory ((void *)NULL, (size_t)(2 * n), sizeof (double), "GMT_delaunay");

	/* Copy x,y points to In structure array */

	for (i = j = 0; i < n; i++) {
		In.pointlist[j++] = x_in[i];
		In.pointlist[j++] = y_in[i];
	}

	/* Call Jonathan Shewchuk's triangulate algorithm */

	triangulate ("zIQB", &In, &Out, &vorOut);

	*link = Out.trianglelist;	/* List of node numbers to return via link */

	if (Out.pointlist) GMT_free ((void *)Out.pointlist);

	return (Out.numberoftriangles);
}

#else
	
/*
 * GMT_delaunay performs a Delaunay triangulation on the input data
 * and returns a list of indeces of the points for each triangle
 * found.  Algorithm translated from
 * Watson, D. F., ACORD: Automatic contouring of raw data,
 *   Computers & Geosciences, 8, 97-101, 1982.
 */

int GMT_delaunay (double *x_in, double *y_in, int n, int **link)
              	/* Input point x coordinates */
              	/* Input point y coordinates */
      		/* Number of input poins */
            	/* pointer to List of point ids per triangle.  Vertices for triangle no i
		   is in link[i*3], link[i*3+1], link[i*3+2] */
{
	int ix[3], iy[3];
	int i, j, ij, nuc, jt, km, id, isp, l1, l2, k, k1, jz, i2, kmt, kt, done, size;
	int *index, *istack, *x_tmp, *y_tmp;
	double det[2][3], *x_circum, *y_circum, *r2_circum, *x, *y;
	double xmin, xmax, ymin, ymax, datax, dx, dy, dsq, dd;
	
	size = 10 * n + 1;
	n += 3;
	
	index = (int *) GMT_memory (VNULL, (size_t)(3 * size), sizeof (int), "GMT_delaunay");
	istack = (int *) GMT_memory (VNULL, (size_t)size, sizeof (int), "GMT_delaunay");
	x_tmp = (int *) GMT_memory (VNULL, (size_t)size, sizeof (int), "GMT_delaunay");
	y_tmp = (int *) GMT_memory (VNULL, (size_t)size, sizeof (int), "GMT_delaunay");
	x_circum = (double *) GMT_memory (VNULL, (size_t)size, sizeof (double), "GMT_delaunay");
	y_circum = (double *) GMT_memory (VNULL, (size_t)size, sizeof (double), "GMT_delaunay");
	r2_circum = (double *) GMT_memory (VNULL, (size_t)size, sizeof (double), "GMT_delaunay");
	x = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "GMT_delaunay");
	y = (double *) GMT_memory (VNULL, (size_t)n, sizeof (double), "GMT_delaunay");
	
	x[0] = x[1] = -1.0;	x[2] = 5.0;
	y[0] = y[2] = -1.0;	y[1] = 5.0;
	x_circum[0] = y_circum[0] = 2.0;	r2_circum[0] = 18.0;
	
	ix[0] = ix[1] = 0;	ix[2] = 1;
	iy[0] = 1;	iy[1] = iy[2] = 2;
	
	for (i = 0; i < 3; i++) index[i] = i;
	for (i = 0; i < size; i++) istack[i] = i;
	
	xmin = ymin = 1.0e100;	xmax = ymax = -1.0e100;
	
	for (i = 3, j = 0; i < n; i++, j++) {	/* Copy data and get extremas */
		x[i] = x_in[j];
		y[i] = y_in[j];
		if (x[i] > xmax) xmax = x[i];
		if (x[i] < xmin) xmin = x[i];
		if (y[i] > ymax) ymax = y[i];
		if (y[i] < ymin) ymin = y[i];
	}
	
	/* Normalize data */
	
	datax = 1.0 / MAX (xmax - xmin, ymax - ymin);
	
	for (i = 3; i < n; i++) {
		x[i] = (x[i] - xmin) * datax;
		y[i] = (y[i] - ymin) * datax;
	}
	
	isp = id = 1;
	for (nuc = 3; nuc < n; nuc++) {
	
		km = 0;
		
		for (jt = 0; jt < isp; jt++) {	/* loop through established 3-tuples */
		
			ij = 3 * jt;
			
			/* Test if new data is within the jt circumcircle */
			
			dx = x[nuc] - x_circum[jt];
			if ((dsq = r2_circum[jt] - dx * dx) < 0.0) continue;
			dy = y[nuc] - y_circum[jt];
			if ((dsq -= dy * dy) < 0.0) continue;

			/* Delete this 3-tuple but save its edges */
			
			id--;
			istack[id] = jt;
			
			/* Add edges to x/y_tmp but delete if already listed */
			
			for (i = 0; i < 3; i++) {
				l1 = ix[i];
				l2 = iy[i];
				if (km > 0) {
					kmt = km;
					for (j = 0, done = FALSE; !done && j < kmt; j++) {
						if (index[ij+l1] != x_tmp[j]) continue;
						if (index[ij+l2] != y_tmp[j]) continue;
						km--;
						if (j >= km) {
							done = TRUE;
							continue;
						}
						for (k = j; k < km; k++) {
							k1 = k + 1;
							x_tmp[k] = x_tmp[k1];
							y_tmp[k] = y_tmp[k1];
							done = TRUE;
						}
					}
				}
				else
					done = FALSE;
				if (!done) {
					x_tmp[km] = index[ij+l1];
					y_tmp[km] = index[ij+l2];
					km++;
				}
			}
		}
			
		/* Form new 3-tuples */
			
		for (i = 0; i < km; i++) {
			kt = istack[id];
			ij = 3 * kt;
			id++;
				
			/* Calculate the circumcircle center and radius
			 * squared of points ktetr[i,*] and place in tetr[kt,*] */
				
			for (jz = 0; jz < 2; jz++) {
				i2 = (jz == 0) ? x_tmp[i] : y_tmp[i];
				det[jz][0] = x[i2] - x[nuc];
				det[jz][1] = y[i2] - y[nuc];
				det[jz][2] = 0.5 * (det[jz][0] * (x[i2] + x[nuc]) + det[jz][1] * (y[i2] + y[nuc]));
			}
			dd = 1.0 / (det[0][0] * det[1][1] - det[0][1] * det[1][0]);
			x_circum[kt] = (det[0][2] * det[1][1] - det[1][2] * det[0][1]) * dd;
			y_circum[kt] = (det[0][0] * det[1][2] - det[1][0] * det[0][2]) * dd;
			dx = x[nuc] - x_circum[kt];
			dy = y[nuc] - y_circum[kt];
			r2_circum[kt] = dx * dx + dy * dy;
			index[ij++] = x_tmp[i];
			index[ij++] = y_tmp[i];
			index[ij] = nuc;
		}
		isp += 2;
	}
	
	for (jt = i = 0; jt < isp; jt++) {
		ij = 3 * jt;
		if (index[ij] < 3 || r2_circum[jt] > 1.0) continue;
		index[i++] = index[ij++] - 3;
		index[i++] = index[ij++] - 3;
		index[i++] = index[ij++] - 3;
	}
	
	index = (int *) GMT_memory ((void *)index, (size_t)i, sizeof (int), "GMT_delaunay");
	*link = index;
	
	GMT_free ((void *)istack);
	GMT_free ((void *)x_tmp);
	GMT_free ((void *)y_tmp);
	GMT_free ((void *)x_circum);
	GMT_free ((void *)y_circum);
	GMT_free ((void *)r2_circum);
	GMT_free ((void *)x);
	GMT_free ((void *)y);
	
	return (i/3);
}

#endif

/*
 * GMT_grd_init initializes a grd header to default values and copies the
 * command line to the header variable command
 */
 
void GMT_grd_init (struct GRD_HEADER *header, int argc, char **argv, BOOLEAN update)
{	/* TRUE if we only want to update command line */
	int i, len;
	
	/* Always update command line history */
	
	memset ((void *)header->command, 0, (size_t)GRD_COMMAND_LEN);
	if (argc > 0) {
		strcpy (header->command, argv[0]);
		len = strlen (header->command);
		for (i = 1; len < GRD_COMMAND_LEN && i < argc; i++) {
			len += strlen (argv[i]) + 1;
			if (len > GRD_COMMAND_LEN) continue;
			strcat (header->command, " ");
			strcat (header->command, argv[i]);
		}
		header->command[len] = 0;
	}

	if (update) return;	/* Leave other variables unchanged */
	
	/* Here we initialize the variables to default settings */
	
	header->x_min = header->x_max = 0.0;
	header->y_min = header->y_max = 0.0;
	header->z_min = header->z_max = 0.0;
	header->x_inc = header->y_inc	= 0.0;
	header->z_scale_factor		= 1.0;
	header->z_add_offset		= 0.0;
	header->nx = header->ny		= 0;
	header->node_offset		= FALSE;
	
	memset ((void *)header->x_units, 0, (size_t)GRD_UNIT_LEN);
	memset ((void *)header->y_units, 0, (size_t)GRD_UNIT_LEN);
	memset ((void *)header->z_units, 0, (size_t)GRD_UNIT_LEN);
	strcpy (header->x_units, "user_x_unit");
	strcpy (header->y_units, "user_y_unit");
	strcpy (header->z_units, "user_z_unit");
	memset ((void *)header->title, 0, (size_t)GRD_TITLE_LEN);
	memset ((void *)header->remark, 0, (size_t)GRD_REMARK_LEN);
}

void GMT_grd_shift (struct GRD_HEADER *header, float *grd, double shift)
                          
            
             		/* Rotate geographical, global grid in e-w direction */
{
	/* This function will shift a grid by shift degrees */
	
	int i, j, k, ij, nc, n_shift, width;
	float *tmp;
	
	tmp = (float *) GMT_memory (VNULL, (size_t)header->nx, sizeof (float), "GMT_grd_shift");
	
	n_shift = irint (shift / header->x_inc);
	width = (header->node_offset) ? header->nx : header->nx - 1;
	nc = header->nx * sizeof (float);
	
	for (j = ij = 0; j < header->ny; j++, ij += header->nx) {
		for (i = 0; i < header->nx; i++) {
			k = (i - n_shift) % width;
			if (k < 0) k += header->nx;
			tmp[k] = grd[ij+i];
		}
		if (!header->node_offset) tmp[width] = tmp[0];
		memcpy ((void *)&grd[ij], (void *)tmp, (size_t)nc);
	}
	
	/* Shift boundaries */
	
	header->x_min += shift;
	header->x_max += shift;
	if (header->x_max < 0.0) {
		header->x_min += 360.0;
		header->x_max += 360.0;
	}
	else if (header->x_max > 360.0) {
		header->x_min -= 360.0;
		header->x_max -= 360.0;
	}
	
	GMT_free ((void *) tmp);
}

/* GMT_grd_setregion determines what wesn should be passed to grd_read.  It
  does so by using project_info.w,e,s,n which have been set correctly
  by map_setup. */

void GMT_grd_setregion (struct GRD_HEADER *h, double *xmin, double *xmax, double *ymin, double *ymax)
{
	int shifted;
	
	/* Round off to nearest multiple of the grid spacing.  This should
	   only affect the numbers when oblique projections or -R...r has been used */

	*xmin = floor (project_info.w / h->x_inc) * h->x_inc;
	*xmax = ceil (project_info.e / h->x_inc) * h->x_inc;
	if (fabs (h->x_max - h->x_min - 360.0) > SMALL) {	/* Not a periodic grid */
		shifted = 0;
		if ((h->x_min < 0.0 && h->x_max <= 0.0) && (project_info.w >= 0.0 && project_info.e > 0.0)) {
			h->x_min += 360.0;
			h->x_max += 360.0;
			shifted = 1;
		}
		else if ((h->x_min >= 0.0 && h->x_max > 0.0) && (project_info.w < 0.0 && project_info.e < 0.0)) {
			h->x_min -= 360.0;
			h->x_max -= 360.0;
			shifted = -1;
		}
		if (*xmin < h->x_min) *xmin = h->x_min;
		if (*xmax > h->x_max) *xmax = h->x_max;
		if (shifted) {
			h->x_min -= (shifted * 360.0);
			h->x_max -= (shifted * 360.0);
		}
	}
	else	{ /* Should be 360 */
		if (*xmin < project_info.w) *xmin = project_info.w;
		if (*xmax > project_info.e) *xmax = project_info.e;
	}
	*ymin = floor (project_info.s / h->y_inc) * h->y_inc;
	if (*ymin < h->y_min) *ymin = h->y_min;
	*ymax = ceil (project_info.n / h->y_inc) * h->y_inc;
	if (*ymax > h->y_max) *ymax = h->y_max;
	
	if ((*xmax) < (*xmin) || (*ymax) < (*ymin)) {	/* Sheisse! */
		fprintf (stderr, "%s: Error setting grid region in GMT_grd_setregion.\n", GMT_program);
		fprintf (stderr, "%s: This is possibly a bug - notify the GMT developers.\n", GMT_program);
	}
}

/* code for bicubic rectangle and bilinear interpolation of grd files
 *
 * Author:	Walter H F Smith
 * Date:	23 September 1993
*/


void	GMT_bcr_init(struct GRD_HEADER *grd, int *pad, int bilinear)
                       
   	      	/* padding on grid, as given to read_grd2 */
   	         	/* T/F we only want bilinear  */
{
	/* Initialize i,j so that they cannot look like they have been used:  */
	bcr.i = -10;
	bcr.j = -10;

	/* Initialize bilinear:  */
	bcr.bilinear = bilinear;

	/* Initialize ioff, joff, mx, my according to grd and pad:  */
	bcr.ioff = pad[0];
	bcr.joff = pad[3];
	bcr.mx = grd->nx + pad[0] + pad[1];
	bcr.my = grd->ny + pad[2] + pad[3];

	/* Initialize rx_inc, ry_inc, and offset:  */
	bcr.rx_inc = 1.0 / grd->x_inc;
	bcr.ry_inc = 1.0 / grd->y_inc;
	bcr.offset = (grd->node_offset) ? 0.5 : 0.0;

	/* Initialize ij_move:  */
	bcr.ij_move[0] = 0;
	bcr.ij_move[1] = 1;
	bcr.ij_move[2] = -bcr.mx;
	bcr.ij_move[3] = 1 - bcr.mx;
}

void	GMT_get_bcr_cardinals (double x, double y)
{
	/* Given x,y compute the cardinal functions.  Note x,y should be in
	 * normalized range, usually [0,1) but sometimes a little outside this.  */

	double	xcf[2][2], ycf[2][2], tsq, tm1, tm1sq, dx, dy;
	int	vertex, verx, very, value, valx, valy;

	if (bcr.bilinear) {
		dx = 1.0 - x;
		dy = 1.0 - y;
		bcr.bl_basis[0] = dx * dy;
		bcr.bl_basis[1] = x * dy;
		bcr.bl_basis[2] = y * dx;
		bcr.bl_basis[3] = x * y;

		return;
	}

	/* Get here when we need to do bicubic functions:  */
	tsq = x*x;
	tm1 = x - 1.0;
	tm1sq = tm1 * tm1;
	xcf[0][0] = (2.0*x + 1.0) * tm1sq;
	xcf[0][1] = x * tm1sq;
	xcf[1][0] = tsq * (3.0 - 2.0*x);
	xcf[1][1] = tsq * tm1;

	tsq = y*y;
	tm1 = y - 1.0;
	tm1sq = tm1 * tm1;
	ycf[0][0] = (2.0*y + 1.0) * tm1sq;
	ycf[0][1] = y * tm1sq;
	ycf[1][0] = tsq * (3.0 - 2.0*y);
	ycf[1][1] = tsq * tm1;

	for (vertex = 0; vertex < 4; vertex++) {
		verx = vertex%2;
		very = vertex/2;
		for (value = 0; value < 4; value++) {
			valx = value%2;
			valy = value/2;

			bcr.bcr_basis[vertex][value] = xcf[verx][valx] * ycf[very][valy];
		}
	}
}

void GMT_get_bcr_ij (struct GRD_HEADER *grd, double xx, double yy, int *ii, int *jj, struct GMT_EDGEINFO *edgeinfo)
{
	/* Given xx, yy in user's grdfile x and y units (not normalized),
	   set ii,jj to the point to be used for the bqr origin. 

	   This function should NOT be called unless xx,yy are within the
	   valid range of the grid. 

		Changed by WHFS 6 May 1998 for GMT 3.1 with two rows of BC's
		implemented:  It used to say: 

	   This 
	   should have jj in the range 1 grd->ny-1 and ii in the range 0 to
	   grd->nx-2, so that the north and east edges will not have the
	   origin on them.

		But now if x is periodic we allow ii to include -1 and nx-1,
		and if y is periodic or polar we allow similar things.  So
		we have to pass in the edgeinfo struct, which was not
		an argument in previous versions.

		I think this will correctly make interpolations match
		boundary conditions.

 */

	int	i, j;

	i = (int)floor((xx-grd->x_min)*bcr.rx_inc - bcr.offset);
/*	if (i < 0) i = 0;  CHANGED:  */
	if (i < 0 && edgeinfo->nxp <= 0) i = 0;
/*	if (i > grd->nx-2) i = grd->nx-2;  CHANGED:  */
	if (i > grd->nx-2  && edgeinfo->nxp <= 0) i = grd->nx-2;
	j = (int)ceil ((grd->y_max-yy)*bcr.ry_inc - bcr.offset);
/*	if (j < 1) j = 1;  CHANGED:  */
	if (j < 1 && !(edgeinfo->nyp > 0 || edgeinfo->gn) ) j = 1;
/*	if (j > grd->ny-1) j = grd->ny-1;  CHANGED:  */
	if (j > grd->ny-1 && !(edgeinfo->nyp > 0 || edgeinfo->gs) ) j = grd->ny-1;
	
	*ii = i;
	*jj = j;
}
	
void	GMT_get_bcr_xy(struct GRD_HEADER *grd, double xx, double yy, double *x, double *y)
{
	/* Given xx, yy in user's grdfile x and y units (not normalized),
	   use the bcr.i and bcr.j to find x,y (normalized) which are the
	   coordinates of xx,yy in the bcr rectangle.  */

	double	xorigin, yorigin;

	xorigin = (bcr.i + bcr.offset)*grd->x_inc + grd->x_min;
	yorigin = grd->y_max - (bcr.j + bcr.offset)*grd->y_inc;

	*x = (xx - xorigin) * bcr.rx_inc;
	*y = (yy - yorigin) * bcr.ry_inc;
}

void	GMT_get_bcr_nodal_values(float *z, int ii, int jj)
{
	/* ii, jj is the point we want to use now, which is different from
	   the bcr.i, bcr.j point we used last time this function was called.
	   If (nan_condition == FALSE) && abs(ii-bcr.i) < 2 && abs(jj-bcr.j)
	   < 2 then we can reuse some previous results.  

	Changed 22 May 98 by WHFS to load vertex values even in case of NaN,
	so that test if (we are within SMALL of node) can return node value. 
	This will make things a  little slower, because now we have to load
	all the values, whether or not they are NaN, whereas before we bailed 
	out at the first NaN we encountered. */

	int	i, valstop, vertex, ij, ij_origin, k0, k1, k2, k3, dontneed[4];

	int	nnans;		/* WHFS 22 May 98  */

	/* whattodo[vertex] says which vertices are previously known.  */
	for (i = 0; i < 4; i++) dontneed[i] = FALSE;

	valstop = (bcr.bilinear) ? 1 : 4;

	if (!(bcr.nan_condition) && (abs(ii-bcr.i) < 2 && abs(jj-bcr.j) < 2) ) {
		/* There was no nan-condition last time and we can use some
			previously computed results.  */
		switch (ii-bcr.i) {
			case 1:
				/* We have moved to the right ...  */
				switch (jj-bcr.j) {
					case -1:
						/* ... and up.  New 0 == old 3  */
						dontneed[0] = TRUE;
						for (i = 0; i < valstop; i++)
							bcr.nodal_value[0][i] = bcr.nodal_value[3][i];
						break;
					case 0:
						/* ... horizontally.  New 0 == old 1; New 2 == old 3  */
						dontneed[0] = dontneed[2] = TRUE;
						for (i = 0; i < valstop; i++) {
							bcr.nodal_value[0][i] = bcr.nodal_value[1][i];
							bcr.nodal_value[2][i] = bcr.nodal_value[3][i];
						}
						break;
					case 1:
						/* ... and down.  New 2 == old 1  */
						dontneed[2] = TRUE;
						for (i = 0; i < valstop; i++)
							bcr.nodal_value[2][i] = bcr.nodal_value[1][i];
						break;
				}
				break;
			case 0:
				/* We have moved only ...  */
				switch (jj-bcr.j) {
					case -1:
						/* ... up.  New 0 == old 2; New 1 == old 3  */
						dontneed[0] = dontneed[1] = TRUE;
						for (i = 0; i < valstop; i++) {
							bcr.nodal_value[0][i] = bcr.nodal_value[2][i];
							bcr.nodal_value[1][i] = bcr.nodal_value[3][i];
						}
						break;
					case 0:
						/* ... not at all.  This shouldn't happen  */
						return;
					case 1:
						/* ... down.  New 2 == old 0; New 3 == old 1  */
						dontneed[2] = dontneed[3] = TRUE;
						for (i = 0; i < valstop; i++) {
							bcr.nodal_value[2][i] = bcr.nodal_value[0][i];
							bcr.nodal_value[3][i] = bcr.nodal_value[1][i];
						}
						break;
				}
				break;
			case -1:
				/* We have moved to the left ...  */
				switch (jj-bcr.j) {
					case -1:
						/* ... and up.  New 1 == old 2  */
						dontneed[1] = TRUE;
						for (i = 0; i < valstop; i++)
							bcr.nodal_value[1][i] = bcr.nodal_value[2][i];
						break;
					case 0:
						/* ... horizontally.  New 1 == old 0; New 3 == old 2  */
						dontneed[1] = dontneed[3] = TRUE;
						for (i = 0; i < valstop; i++) {
							bcr.nodal_value[1][i] = bcr.nodal_value[0][i];
							bcr.nodal_value[3][i] = bcr.nodal_value[2][i];
						}
						break;
					case 1:
						/* ... and down.  New 3 == old 0  */
						dontneed[3] = TRUE;
						for (i = 0; i < valstop; i++)
							bcr.nodal_value[3][i] = bcr.nodal_value[0][i];
						break;
				}
				break;
		}
	}
		
	/* When we get here, we are ready to look for new values (and possibly derivatives)  */

	ij_origin = (jj + bcr.joff) * bcr.mx + (ii + bcr.ioff);
	bcr.i = ii;
	bcr.j = jj;

	nnans = 0;	/* WHFS 22 May 98  */

	for (vertex = 0; vertex < 4; vertex++) {

		if (dontneed[vertex]) continue;

		ij = ij_origin + bcr.ij_move[vertex];
		if (GMT_is_fnan (z[ij])) {
			bcr.nodal_value[vertex][0] = (GMT_d_NaN);
			nnans++;
		}
		else {
			bcr.nodal_value[vertex][0] = (double)z[ij];
		}

		if (bcr.bilinear) continue;

		/* Get dz/dx:  */
		if (GMT_is_fnan (z[ij+1]) || GMT_is_fnan (z[ij-1]) ){
			bcr.nodal_value[vertex][1] = (GMT_d_NaN);
			nnans++;
		}
		else {
			bcr.nodal_value[vertex][1] = 0.5 * (z[ij+1] - z[ij-1]);
		}

		/* Get dz/dy:  */
		if (GMT_is_fnan (z[ij+bcr.mx]) || GMT_is_fnan (z[ij-bcr.mx]) ){
			bcr.nodal_value[vertex][2] = (GMT_d_NaN);
			nnans++;
		}
		else {
			bcr.nodal_value[vertex][2] = 0.5 * (z[ij-bcr.mx] - z[ij+bcr.mx]);
		}

		/* Get d2z/dxdy:  */
		k0 = ij + bcr.mx - 1;
		k1 = k0 + 2;
		k2 = ij - bcr.mx - 1;
		k3 = k2 + 2;
		if (GMT_is_fnan (z[k0]) || GMT_is_fnan (z[k1]) || GMT_is_fnan (z[k2]) || GMT_is_fnan (z[k3]) ) {
			bcr.nodal_value[vertex][3] = (GMT_d_NaN);
			nnans++;
		}
		else {
			bcr.nodal_value[vertex][3] = 0.25 * ( (z[k3] - z[k2]) - (z[k1] - z[k0]) );
		}
	}

	bcr.nan_condition = (nnans > 0);

	return;
}

double	GMT_get_bcr_z(struct GRD_HEADER *grd, double xx, double yy, float *data,  struct GMT_EDGEINFO *edgeinfo)
{
	/* Given xx, yy in user's grdfile x and y units (not normalized),
	   this routine returns the desired interpolated value (bilinear
	   or bicubic) at xx, yy.  */

	int	i, j, vertex, value;
	double	x, y, retval;
	
	if (xx < grd->x_min || xx > grd->x_max) return(GMT_d_NaN);
	if (yy < grd->y_min || yy > grd->y_max) return(GMT_d_NaN);

	GMT_get_bcr_ij(grd, xx, yy, &i, &j, edgeinfo);

	if (i != bcr.i || j != bcr.j)
		GMT_get_bcr_nodal_values(data, i, j);

	GMT_get_bcr_xy(grd, xx, yy, &x, &y);

	/* See if we can copy a node value.  This saves the user
		from getting a NaN if the node value is fine but
		there is a NaN in the neighborhood.  Check if
		x or y is almost equal to 0 or 1, and if so,
		return a node value.  */

	if ( (fabs(x)) <= SMALL) {
		if ( (fabs(y)) <= SMALL)
			return(bcr.nodal_value[0][0]);
		if ( (fabs(1.0 - y)) <= SMALL)
			return(bcr.nodal_value[2][0]);
	}
	if ( (fabs(1.0 - x)) <= SMALL) {
		if ( (fabs(y)) <= SMALL)
			return(bcr.nodal_value[1][0]);
		if ( (fabs(1.0 - y)) <= SMALL)
			return(bcr.nodal_value[3][0]);
	}

	if (bcr.nan_condition) return(GMT_d_NaN);

	GMT_get_bcr_cardinals(x, y);

	retval = 0.0;
	if (bcr.bilinear) {
		for (vertex = 0; vertex < 4; vertex++) {
			retval += (bcr.nodal_value[vertex][0] * bcr.bl_basis[vertex]);
		}
		return(retval);
	}
	for (vertex = 0; vertex < 4; vertex++) {
		for (value = 0; value < 4; value++) {
			retval += (bcr.nodal_value[vertex][value]*bcr.bcr_basis[vertex][value]);
		}
	}
	return(retval);
}

/*
 * This section holds functions used for setting boundary  conditions in 
 * processing grd file data. 
 *
 * This is a new feature of GMT v3.1.   My first draft of this (April 1998)
 * set only one row of padding.  Additional thought showed that the bilinear
 * invocation of bcr routines would not work properly on periodic end conditions
 * in this case.  So second draft (May 5-6, 1998) will set two rows of padding,
 * and I will also have to edit bcr so that it works for this case.  The GMT
 * bcr routines are currently used in grdsample, grdtrack, and grdview.
 *
 * I anticipate that later (GMT v4 ?) this code could (?) be modified to also
 * handle the boundary conditions needed by surface. 
 *
 * The default boundary condition is derived from application of Green's
 * theorem to the conditions for minimizing curvature:
 * laplacian (f) = 0 on edges, and d[laplacian(f)]/dn = 0 on edges, where
 * n is normal to an edge.  We also set d2f/dxdy = 0 at corners.
 *
 * The new routines here allow the user to choose other specifications:
 *
 * Either
 * 	one or both of
 * 	data are periodic in (x_max - x_min)
 * 	data are periodic in (y_max - y_min)
 *
 * Or
 *	data are a geographical grid.
 *
 * Periodicities assume that the min,max are compatible with the inc;
 * that is, (x_max - x_min)modulo(x_inc) ~= 0.0 within precision tolerances,
 * and similarly for y.  It is assumed that this is OK and that gmt_grd_RI_verify
 * was called during read grd and found to be OK.
 *
 * In the geographical case, if x_max - x_min < 360 we will use the default
 * boundary conditions, but if x_max - x_min >= 360 the 360 periodicity in x 
 * will be used to set the x boundaries, and so we require 360modulo(x_inc)
 * == 0.0 within precision tolerance.  If y_max != 90 the north edge will 
 * default, and similarly for y_min != -90.  If a y edge is at a pole and
 * x_max - x_min >= 360 then the geographical y uses a 180 degree phase
 * shift in the values, so we require 180modulo(x_inc) == 0.
 * Note that a grid-registered file will require that the entire row of
 * values representing a pole must all be equal, else d/dx != 0 which
 * is wrong.  So compatibility error-checking is built in.
 *
 * Note that a periodicity or polar boundary eliminates the need for
 * d2/dxdy = 0 at a corner.  There are no "corners" in those cases.
 *
 * Author:	W H F Smith
 * Date:	17 April 1998
 * Revised:	5  May 1998
 *
 */

void GMT_boundcond_init (struct GMT_EDGEINFO *edgeinfo)
{
	edgeinfo->nxp = 0;
	edgeinfo->nyp = 0;
	edgeinfo->gn = FALSE;
	edgeinfo->gs = FALSE;
	return;
}


int GMT_boundcond_parse (struct GMT_EDGEINFO *edgeinfo, char *edgestring)
{
	/* Parse string beginning at argv[i][2] and load user's
		requests in edgeinfo->  Return success or failure.
		Requires that edgeinfo previously initialized to
		zero/FALSE stuff.  Expects g or (x and or y) is
		all that is in string.  */

	int	i, ier;

	i = 0;
	ier = FALSE;
	while (!ier && edgestring[i]) {
		switch (edgestring[i]) {
			case 'g':
			case 'G':
				edgeinfo->gn = TRUE;
				edgeinfo->gs = TRUE;
				break;
			case 'x':
			case 'X':
				edgeinfo->nxp = -1;
				break;
			case 'y':
			case 'Y':
				edgeinfo->nyp = -1;
				break;
			default:
				ier = TRUE;
				break;
		
		}
		i++;
	}

	if (ier) return (-1);

	if (edgeinfo->gn && (edgeinfo->nxp == -1 || edgeinfo->nxp == -1) ) {
		(void) fprintf (stderr, "WARNING:  GMT boundary condition g overrides x or y\n");
	}

	return (0);
}



int GMT_boundcond_param_prep (struct GRD_HEADER *h, struct GMT_EDGEINFO *edgeinfo)
{
	/* Called when edgeinfo holds user's choices.  Sets
		edgeinfo according to choices and h.  */

	double	xtest;

	if (edgeinfo->gn) {
		/* User has requested geographical conditions.  */
		if ( (h->x_max - h->x_min) < (360.0 - SMALL * h->x_inc) ) {
			(void) fprintf (stderr, 
				"GMT Warning:  x range too small; g boundary condition ignored.\n");
			edgeinfo->nxp = edgeinfo->nyp = 0;
			edgeinfo->gn  = edgeinfo->gs = FALSE;
			return (0);
		}
		xtest = fmod (180.0, h->x_inc) / h->x_inc;
		/* xtest should be within SMALL of zero or of one.  */
		if ( xtest > SMALL && xtest < (1.0 - SMALL) ) {
			/* Error.  We need it to divide into 180 so we can phase-shift at poles.  */
			(void) fprintf (stderr, 
				"GMT Warning:  x_inc does not divide 180; g boundary condition ignored.\n");
			edgeinfo->nxp = edgeinfo->nyp = 0;
			edgeinfo->gn  = edgeinfo->gs = FALSE;
			return (0);
		}
		edgeinfo->nxp = irint(360.0/h->x_inc);
		edgeinfo->nyp = 0;
		edgeinfo->gn = ( (fabs(h->y_max - 90.0) ) < (SMALL * h->y_inc) );
		edgeinfo->gs = ( (fabs(h->y_min + 90.0) ) < (SMALL * h->y_inc) );
		return (0);
	}
	if (edgeinfo->nxp != 0) edgeinfo->nxp = (h->node_offset) ? h->nx : h->nx - 1;
	if (edgeinfo->nyp != 0) edgeinfo->nyp = (h->node_offset) ? h->ny : h->ny - 1;
	return (0);
}


int GMT_boundcond_set (struct GRD_HEADER *h, struct GMT_EDGEINFO *edgeinfo, int *pad, float *a)
{
	/* Set two rows of padding (pad[] can be larger) around data according
		to desired boundary condition info in edgeinfo.  
		Returns -1 on problem, 0 on success.
		If either x or y is periodic, the padding is entirely set.
		However, if neither is true (this rules out geographical also)
		then all but three corner-most points in each corner are set.

		As written, not ready to use with "surface" for GMT v4, because
		assumes left/right is +/- 1 and down/up is +/- mx.  In "surface"
		the amount to move depends on the current mesh size, a parameter
		not used here. 

		This is the revised, two-rows version (WHFS 6 May 1998). 
	*/

	int	bok;	/* Counter used to test that things are OK  */
	int	mx;	/* Width of padded array; width as malloc'ed  */
	int	mxnyp;	/* distance to periodic constraint in j direction  */
	int	i, jmx;	/* Current i, j * mx  */
	int	nxp2;	/* 1/2 the xg period (180 degrees) in cells  */
	int	i180;	/* index to 180 degree phase shift  */
	int	iw, iwo1, iwo2, iwi1, ie, ieo1, ieo2, iei1;  /* see below  */
	int	jn, jno1, jno2, jni1, js, jso1, jso2, jsi1;  /* see below  */
	int	jno1k, jno2k, jso1k, jso2k, iwo1k, iwo2k, ieo1k, ieo2k;
	int	j1p, j2p;	/* j_o1 and j_o2 pole constraint rows  */




	/* Check pad  */
	bok = 0;
	for (i = 0; i < 4; i++) {
		if (pad[i] < 2) bok++;
	}
	if (bok > 0) {
		fprintf (stderr, "GMT BUG:  bad pad for GMT_boundcond_set.\n");
		return (-1);
	}

	/* Check minimum size:  */
	if (h->nx < 2 || h->ny < 2) {
		fprintf (stderr, "GMT ERROR:  GMT_boundcond_set requires nx,ny at least 2.\n");
		return (-1);
	}


	/* Initialize stuff:  */

	mx = h->nx + pad[0] + pad[1];
	nxp2 = edgeinfo->nxp / 2;	/* Used for 180 phase shift at poles  */

	iw = pad[0];		/* i for west-most data column */
	iwo1 = iw - 1;		/* 1st column outside west  */
	iwo2 = iwo1 - 1;	/* 2nd column outside west  */
	iwi1 = iw + 1;		/* 1st column  inside west  */

	ie = pad[0] + h->nx - 1;	/* i for east-most data column */
	ieo1 = ie + 1;		/* 1st column outside east  */
	ieo2 = ieo1 + 1;	/* 2nd column outside east  */
	iei1 = ie - 1;		/* 1st column  inside east  */

	jn = mx * pad[3];	/* j*mx for north-most data row  */
	jno1 = jn - mx;		/* 1st row outside north  */
	jno2 = jno1 - mx;	/* 2nd row outside north  */
	jni1 = jn + mx;		/* 1st row  inside north  */

	js = mx * (pad[3] + h->ny - 1);	/* j*mx for south-most data row  */
	jso1 = js + mx;		/* 1st row outside south  */
	jso2 = jso1 + mx;	/* 2nd row outside south  */
	jsi1 = js - mx;		/* 1st row  inside south  */

	mxnyp = mx * edgeinfo->nyp;

	jno1k = jno1 + mxnyp;	/* data rows periodic to boundary rows  */
	jno2k = jno2 + mxnyp;
	jso1k = jso1 - mxnyp;
	jso2k = jso2 - mxnyp;

	iwo1k = iwo1 + edgeinfo->nxp;	/* data cols periodic to bndry cols  */
	iwo2k = iwo2 + edgeinfo->nxp;
	ieo1k = ieo1 - edgeinfo->nxp;
	ieo2k = ieo2 - edgeinfo->nxp;


	/* Check poles for grid case.  It would be nice to have done this
		in GMT_boundcond_param_prep() but at that point the data
		array isn't passed into that routine, and may not have been
		read yet.  Also, as coded here, this bombs with error if
		the pole data is wrong.  But there could be an option to
		to change the condition to Natural in that case, with warning.  */

	if (h->node_offset == 0) {
		if (edgeinfo->gn) {
			bok = 0;
			for (i = iw+1; i <= ie; i++) {
				if (a[jn + i] != a[jn + iw]) 	/* This maybe fails if they are NaNs ? */
					bok++;
			}
			if (bok > 0) {
				fprintf (stderr, "GMT ERROR:  Inconsistent grid values at North pole.\n");
				return (-1);
			}
		}

		if (edgeinfo->gs) {
			bok = 0;
			for (i = iw+1; i <= ie; i++) {
				if (a[js + i] != a[js + iw]) 	/* This maybe fails if they are NaNs ? */
					bok++;
			}
			if (bok > 0) {
				fprintf (stderr, "GMT ERROR:  Inconsistent grid values at South pole.\n");
				return (-1);
			}
		}
	}



	/* Start with the case that x is not periodic, because in that
		case we also know that y cannot be polar.  */

	if (edgeinfo->nxp <= 0) {

		/* x is not periodic  */

		if (edgeinfo->nyp > 0) {	

			/* y is periodic  */

			for (i = iw; i <= ie; i++) {
				a[jno1 + i] = a[jno1k + i];
				a[jno2 + i] = a[jno2k + i];
				a[jso1 + i] = a[jso1k + i];
				a[jso2 + i] = a[jso2k + i];
			}

			/* periodic Y rows copied.  Now do X naturals.
				This is easy since y's are done; no corner problems.
				Begin with Laplacian = 0, and include 1st outside rows
				in loop, since y's already loaded to 2nd outside.  */

			for (jmx = jno1; jmx <= jso1; jmx += mx) {
				a[jmx + iwo1] = (float)(4.0 * a[jmx + iw])
					- (a[jmx + iw + mx] + a[jmx + iw - mx] + a[jmx + iwi1]);
				a[jmx + ieo1] = (float)(4.0 * a[jmx + ie])
					- (a[jmx + ie + mx] + a[jmx + ie - mx] + a[jmx + iei1]);
			}

			/* Copy that result to 2nd outside row using periodicity.  */
			a[jno2 + iwo1] = a[jno2k + iwo1];
			a[jso2 + iwo1] = a[jso2k + iwo1];
			a[jno2 + ieo1] = a[jno2k + ieo1];
			a[jso2 + ieo1] = a[jso2k + ieo1];

			/* Now set d[laplacian]/dx = 0 on 2nd outside column.  Include
				1st outside rows in loop.  */
			for (jmx = jno1; jmx <= jso1; jmx += mx) {
				a[jmx + iwo2] = (a[jmx + iw - mx] + a[jmx + iw + mx] + a[jmx + iwi1])
					- (a[jmx + iwo1 - mx] + a[jmx + iwo1 + mx])
					+ (float)(5.0 * (a[jmx + iwo1] - a[jmx + iw]));

				a[jmx + ieo2] = (a[jmx + ie - mx] + a[jmx + ie + mx] + a[jmx + iei1])
					- (a[jmx + ieo1 - mx] + a[jmx + ieo1 + mx])
					+ (float)(5.0 * (a[jmx + ieo1] - a[jmx + ie]));
			}

			/* Now copy that result also, for complete periodicity's sake  */
			a[jno2 + iwo2] = a[jno2k + iwo2];
			a[jso2 + iwo2] = a[jso2k + iwo2];
			a[jno2 + ieo2] = a[jno2k + ieo2];
			a[jso2 + ieo2] = a[jso2k + ieo2];

			/* DONE with X not periodic, Y periodic case.  Fully loaded.  */

			return (0);
		}
		else {
			/* Here begins the X not periodic, Y not periodic case  */

			/* First, set corner points.  Need not merely Laplacian(f) = 0
				but explicitly that d2f/dx2 = 0 and d2f/dy2 = 0.
				Also set d2f/dxdy = 0.  Then can set remaining points.  */

	/* d2/dx2 */	a[jn + iwo1]   = (float)(2.0 * a[jn + iw] - a[jn + iwi1]);
	/* d2/dy2 */	a[jno1 + iw]   = (float)(2.0 * a[jn + iw] - a[jni1 + iw]);
	/* d2/dxdy */	a[jno1 + iwo1] = a[jno1 + iwi1] - a[jni1 + iwi1]
						+ a[jni1 + iwo1];


	/* d2/dx2 */	a[jn + ieo1]   = (float)(2.0 * a[jn + ie] - a[jn + iei1]);
	/* d2/dy2 */	a[jno1 + ie]   = (float)(2.0 * a[jn + ie] - a[jni1 + ie]);
	/* d2/dxdy */	a[jno1 + ieo1] = a[jno1 + iei1] - a[jni1 + iei1]
						+ a[jni1 + ieo1];

	/* d2/dx2 */	a[js + iwo1]   = (float)(2.0 * a[js + iw] - a[js + iwi1]);
	/* d2/dy2 */	a[jso1 + iw]   = (float)(2.0 * a[js + iw] - a[jsi1 + iw]);
	/* d2/dxdy */	a[jso1 + iwo1] = a[jso1 + iwi1] - a[jsi1 + iwi1]
						+ a[jsi1 + iwo1];

	/* d2/dx2 */	a[js + ieo1]   = (float)(2.0 * a[js + ie] - a[js + iei1]);
	/* d2/dy2 */	a[jso1 + ie]   = (float)(2.0 * a[js + ie] - a[jsi1 + ie]);
	/* d2/dxdy */	a[jso1 + ieo1] = a[jso1 + iei1] - a[jsi1 + iei1]
						+ a[jsi1 + ieo1];

			/* Now set Laplacian = 0 on interior edge points,
				skipping corners:  */
			for (i = iwi1; i <= iei1; i++) {
				a[jno1 + i] = (float)(4.0 * a[jn + i]) 
					- (a[jn + i - 1] + a[jn + i + 1] 
						+ a[jni1 + i]);

				a[jso1 + i] = (float)(4.0 * a[js + i]) 
					- (a[js + i - 1] + a[js + i + 1] 
						+ a[jsi1 + i]);
			}
			for (jmx = jni1; jmx <= jsi1; jmx += mx) {
				a[iwo1 + jmx] = (float)(4.0 * a[iw + jmx])
					- (a[iw + jmx + mx] + a[iw + jmx - mx]
						+ a[iwi1 + jmx]);
				a[ieo1 + jmx] = (float)(4.0 * a[ie + jmx])
					- (a[ie + jmx + mx] + a[ie + jmx - mx]
						+ a[iei1 + jmx]);
			}

			/* Now set d[Laplacian]/dn = 0 on all edge pts, including
				corners, since the points needed in this are now set.  */
			for (i = iw; i <= ie; i++) {
				a[jno2 + i] = a[jni1 + i]
					+ (float)(5.0 * (a[jno1 + i] - a[jn + i]))
					+ (a[jn + i - 1] - a[jno1 + i - 1])
					+ (a[jn + i + 1] - a[jno1 + i + 1]);
				a[jso2 + i] = a[jsi1 + i]
					+ (float)(5.0 * (a[jso1 + i] - a[js + i]))
					+ (a[js + i - 1] - a[jso1 + i - 1])
					+ (a[js + i + 1] - a[jso1 + i + 1]);
			}
			for (jmx = jn; jmx <= js; jmx += mx) {
				a[iwo2 + jmx] = a[iwi1 + jmx]
					+ (float)(5.0 * (a[iwo1 + jmx] - a[iw + jmx]))
					+ (a[iw + jmx - mx] - a[iwo1 + jmx - mx])
					+ (a[iw + jmx + mx] - a[iwo1 + jmx + mx]);
				a[ieo2 + jmx] = a[iei1 + jmx]
					+ (float)(5.0 * (a[ieo1 + jmx] - a[ie + jmx]))
					+ (a[ie + jmx - mx] - a[ieo1 + jmx - mx])
					+ (a[ie + jmx + mx] - a[ieo1 + jmx + mx]);
			}
			/* DONE with X not periodic, Y not periodic case.  
				Loaded all but three cornermost points at each corner.  */

			return (0);
		}
		/* DONE with all X not periodic cases  */
	}
	else {
		/* X is periodic.  Load x cols first, then do Y cases.  */

		for (jmx = jn; jmx <= js; jmx += mx) {
			a[iwo1 + jmx] = a[iwo1k + jmx];
			a[iwo2 + jmx] = a[iwo2k + jmx];
			a[ieo1 + jmx] = a[ieo1k + jmx];
			a[ieo2 + jmx] = a[ieo2k + jmx];
		}

		if (edgeinfo->nyp > 0) {
			/* Y is periodic.  copy all, including boundary cols:  */
			for (i = iwo2; i <= ieo2; i++) {
				a[jno1 + i] = a[jno1k + i];
				a[jno2 + i] = a[jno2k + i];
				a[jso1 + i] = a[jso1k + i];
				a[jso2 + i] = a[jso2k + i];
			}
			/* DONE with X and Y both periodic.  Fully loaded.  */

			return (0);
		}

		/* Do north (top) boundary:  */

		if (edgeinfo->gn) {
			/* Y is at north pole.  Phase-shift all, incl. bndry cols. */
			if (h->node_offset) {
				j1p = jn;	/* constraint for jno1  */
				j2p = jni1;	/* constraint for jno2  */
			}
			else {
				j1p = jni1;	/* constraint for jno1  */
				j2p = jni1 + mx;	/* constraint for jno2  */
			}
			for (i = iwo2; i <= ieo2; i++) {
				i180 = pad[0] + ((i + nxp2)%edgeinfo->nxp);
				a[jno1 + i] = a[j1p + i180];
				a[jno2 + i] = a[j2p + i180];
			}
		}
		else {
			/* Y needs natural conditions.  x bndry cols periodic.
				First do Laplacian.  Start/end loop 1 col outside,
				then use periodicity to set 2nd col outside.  */

			for (i = iwo1; i <= ieo1; i++) {
				a[jno1 + i] = (float)(4.0 * a[jn + i])
					- (a[jn + i - 1] + a[jn + i + 1] + a[jni1 + i]);
			}
			a[jno1 + iwo2] = a[jno1 + iwo2 + edgeinfo->nxp];
			a[jno1 + ieo2] = a[jno1 + ieo2 - edgeinfo->nxp];
			

			/* Now set d[Laplacian]/dn = 0, start/end loop 1 col out,
				use peroidicity to set 2nd out col after loop.  */
			
			for (i = iwo1; i <= ieo1; i++) {
				a[jno2 + i] = a[jni1 + i]
					+ (float)(5.0 * (a[jno1 + i] - a[jn + i]))
					+ (a[jn + i - 1] - a[jno1 + i - 1])
					+ (a[jn + i + 1] - a[jno1 + i + 1]);
			}
			a[jno2 + iwo2] = a[jno2 + iwo2 + edgeinfo->nxp];
			a[jno2 + ieo2] = a[jno2 + ieo2 - edgeinfo->nxp];

			/* End of X is periodic, north (top) is Natural.  */

		}

		/* Done with north (top) BC in X is periodic case.  Do south (bottom)  */

		if (edgeinfo->gs) {
			/* Y is at south pole.  Phase-shift all, incl. bndry cols. */
			if (h->node_offset) {
				j1p = js;	/* constraint for jso1  */
				j2p = jsi1;	/* constraint for jso2  */
			}
			else {
				j1p = jsi1;	/* constraint for jso1  */
				j2p = jsi1 - mx;	/* constraint for jso2  */
			}
			for (i = iwo2; i <= ieo2; i++) {
				i180 = pad[0] + ((i + nxp2)%edgeinfo->nxp);
				a[jso1 + i] = a[j1p + i180];
				a[jso2 + i] = a[j2p + i180];
			}
		}
		else {
			/* Y needs natural conditions.  x bndry cols periodic.
				First do Laplacian.  Start/end loop 1 col outside,
				then use periodicity to set 2nd col outside.  */

			for (i = iwo1; i <= ieo1; i++) {
				a[jso1 + i] = (float)(4.0 * a[js + i])
					- (a[js + i - 1] + a[js + i + 1] + a[jsi1 + i]);
			}
			a[jso1 + iwo2] = a[jso1 + iwo2 + edgeinfo->nxp];
			a[jso1 + ieo2] = a[jso1 + ieo2 - edgeinfo->nxp];
			

			/* Now set d[Laplacian]/dn = 0, start/end loop 1 col out,
				use peroidicity to set 2nd out col after loop.  */
			
			for (i = iwo1; i <= ieo1; i++) {
				a[jso2 + i] = a[jsi1 + i]
					+ (float)(5.0 * (a[jso1 + i] - a[js + i]))
					+ (a[js + i - 1] - a[jso1 + i - 1])
					+ (a[js + i + 1] - a[jso1 + i + 1]);
			}
			a[jso2 + iwo2] = a[jso2 + iwo2 + edgeinfo->nxp];
			a[jso2 + ieo2] = a[jso2 + ieo2 - edgeinfo->nxp];

			/* End of X is periodic, south (bottom) is Natural.  */

		}

		/* Done with X is periodic cases.  */

		return (0);
	}
}

BOOLEAN GMT_y_out_of_bounds (int *j, struct GRD_HEADER *h, struct GMT_EDGEINFO *edgeinfo, BOOLEAN *wrap_180) {
	/* Adjusts the j (y-index) value if we are dealing with some sort of periodic boundary
	* condition.  If a north or south pole condition we must "go over the pole" and access
	* the longitude 180 degrees away - this is achieved by passing the wrap_180 flag; the
	* shifting of longitude is then deferred to GMT_x_out_of_bounds.
	* If no periodicities are present then nothing happens here.  If we end up being
	* out of bounds we return TRUE (and main program can take action like continue);
	* otherwise we return FALSE.
	*/
	
	if ((*j) < 0) {	/* Depending on BC's we wrap around or we are above the top of the domain */
		if (edgeinfo->gn) {	/* N Polar condition - adjust j and set wrap flag */
			(*j) = abs (*j) - h->node_offset;
			(*wrap_180) = TRUE;	/* Go "over the pole" */
		}
		else if (edgeinfo->nyp) {	/* Periodic in y */
			(*j) += edgeinfo->nyp;
			(*wrap_180) = FALSE;
		}
		else
			return (TRUE);	/* We are outside the range */
	}
	else if ((*j) >= h->ny) {	/* Depending on BC's we wrap around or we are below the bottom of the domain */
		if (edgeinfo->gs) {	/* S Polar condition - adjust j and set wrap flag */
			(*j) += h->node_offset - 2;
			(*wrap_180) = TRUE;	/* Go "over the pole" */
		}
		else if (edgeinfo->nyp) {	/* Periodic in y */
			(*j) -= edgeinfo->nyp;
			(*wrap_180) = FALSE;
		}
		else
			return (TRUE);	/* We are outside the range */
	}
	else
		(*wrap_180) = FALSE;
	
	return (FALSE);	/* OK, we are inside grid now for sure */
}

BOOLEAN GMT_x_out_of_bounds (int *i, struct GRD_HEADER *h, struct GMT_EDGEINFO *edgeinfo, BOOLEAN wrap_180) {
	/* Adjusts the i (x-index) value if we are dealing with some sort of periodic boundary
	* condition.  If a north or south pole condition we must "go over the pole" and access
	* the longitude 180 degrees away - this is achieved by examining the wrap_180 flag and take action.
	* If no periodicities are present and we end up being out of bounds we return TRUE (and
	* main program can take action like continue); otherwise we return FALSE.
	*/

	/* Depending on BC's we wrap around or leave as is. */
	
	if ((*i) < 0) {	/* Potentially outside to the left of the domain */
		if (edgeinfo->nxp)	/* Periodic in x - always inside grid */
			(*i) += edgeinfo->nxp;
		else	/* Sorry, you're outside */
			return (TRUE);
	}
	else if ((*i) >= h->nx) {	/* Potentially outside to the right of the domain */
		if (edgeinfo->nxp)	/* Periodic in x -always inside grid */
			(*i) -= edgeinfo->nxp;
		else	/* Sorry, you're outside */
			return (TRUE);
	}

	if (wrap_180) (*i) = ((*i) + (edgeinfo->nxp / 2)) % edgeinfo->nxp;	/* Must move 180 degrees */
	
	return (FALSE);	/* OK, we are inside grid now for sure */
}

void GMT_setcontjump (float *z, int nz)
{
/* This routine will check if there is a 360 jump problem
 * among these coordinates and adjust them accordingly so
 * that subsequent testing can determine if a zero contour
 * goes through these edges.  E.g., values like 359, 1
 * should become -1, 1 after this function */

	int i;
	BOOLEAN jump = FALSE;
	double dz;

	for (i = 1; !jump && i < nz; i++) {
		dz = z[i] - z[0];
		if (fabs (dz) > 180.0) jump = TRUE;
	}

	if (!jump) return;

	z[0] = (float)fmod (z[0], 360.0);
	if (z[0] > 180.0) z[0] -= 360.0;
	for (i = 1; i < nz; i++) {
		if (z[i] > 180.0) z[i] -= 360.0;
		dz = z[i] - z[0];
		if (fabs (dz) > 180.0) z[i] -= (float)copysign (360.0, dz);
		z[i] = (float)fmod (z[i], 360.0);
	}
}

BOOLEAN GMT_getpathname (char *name, char *path) {
	/* Prepends the appropriate directory to the file name
	 * and returns TRUE if file is readable. */
	 
	BOOLEAN found;
	char dir[BUFSIZ];
	FILE *fp;

	/* First check the $GMTHOME/share directory */
	
	sprintf (path, "%s%cshare%c%s\0", GMTHOME, DIR_DELIM, DIR_DELIM, name);
	if (!access (path, R_OK)) return (TRUE);	/* File exists and is readable, return with name */
	
	/* File was not readable.  Now check if it exists */
	
	if (!access (path, F_OK))  { /* Kicks in if file is there, meaning it has the wrong permissions */
		fprintf (stderr, "%s: Error: GMT does not have permission to open %s!\n", GMT_program, path);
		exit (EXIT_FAILURE);
	}

	/* File is not there.  Thus, we check if a coastline.conf file exists
	 * It is not an error if we cannot find the named file, only if it is found
	 * but cannot be read due to permission problems */
	
	sprintf (dir, "%s%cshare%ccoastline.conf\0", GMTHOME, DIR_DELIM, DIR_DELIM);
	if (!access (dir, F_OK))  { /* File exists... */
		if (access (dir, R_OK)) {	/* ...but cannot be read */
			fprintf (stderr, "%s: Error: GMT does not have permission to open %s!\n", GMT_program, dir);
			exit (EXIT_FAILURE);
		}
	}
	else {	/* There is no coastline.conf file to use; we're out of luck */
		fprintf (stderr, "%s: Error: No configuration file %s available!\n", GMT_program, dir);
		exit (EXIT_FAILURE);
	}
	
	/* We get here if coastline.conf exists - search among its directories for the named file */
	
	if ((fp = fopen (dir, "r")) == NULL) {	/* This shouldn't be necessary, but cannot hurt */
		fprintf (stderr, "%s: Error: Cannot open configuration file %s\n", GMT_program, dir);
		exit (EXIT_FAILURE);
	}
		
	found = FALSE;
	while (!found && fgets (dir, BUFSIZ, fp)) {	/* Loop over all input lines until found or done */
		if (dir[0] == '#' || dir[0] == '\n') continue;	/* Comment or blank */
		
		dir[strlen(dir)-1] = '\0';			/* Chop off linefeed */
		sprintf (path, "%s%c%s\0", dir, DIR_DELIM, name);
		if (!access (path, F_OK)) {	/* TRUE if file exists */
			if (!access (path, R_OK)) {	/* TRUE if file is readable */
				found = TRUE;
			}
			else {
				fprintf (stderr, "%s: Error: GMT does not have permission to open %s!\n", GMT_program, path);
				exit (EXIT_FAILURE);
			}
		}
	}

	fclose (fp);
	
	return (found);
}

int GMT_getscale (char *text, double *x0, double *y0, double *scale_lat, double *length, char *measure, BOOLEAN *fancy, BOOLEAN *gave_xy)
{
	/* Pass text as &argv[i][2] */
	
	int j = 0, error = 0, k;
	char txt_a[32], txt_b[32], txt_c[32];
	
	*fancy = *gave_xy = FALSE;
	*measure = '\0';
	*length = 0.0;
	
	if (text[j] == 'f') *fancy = TRUE, j++;
	if (text[j] == 'x') *gave_xy = TRUE, j++;
	k = sscanf (&text[j], "%[^/]/%[^/]/%[^/]/%lf", txt_a, txt_b, txt_c, length);
	if ((*gave_xy)) {	/* Convert user's x/y to inches */
		*x0 = GMT_convert_units (txt_a, GMT_INCH);
		*y0 = GMT_convert_units (txt_b, GMT_INCH);
	}
	else {	/* Read geographical coordinates */
		*x0 = GMT_ddmmss_to_degree (txt_a);
		*y0 = GMT_ddmmss_to_degree (txt_b);
		if (fabs (*y0) > 90.0) {
			fprintf (stderr, "%s: GMT SYNTAX ERROR -L option:  Position latitude is out of range\n", GMT_program);
			error++;
		}
		if (fabs (*x0) > 360.0) {
			fprintf (stderr, "%s: GMT SYNTAX ERROR -L option:  Position longitude is out of range\n", GMT_program);
			error++;
		}
	}
	*scale_lat = GMT_ddmmss_to_degree (txt_c);
	*measure = text[strlen(text)-1];
	if (k != 4) {
		fprintf (stderr, "%s: GMT SYNTAX ERROR -L option:  Correct syntax\n", GMT_program);
		fprintf (stderr, "	-L[f][x]<x0>/<y0>/<lat>/<length>[m|n|k], append m, n, or k for miles, nautical miles, or km [Default]\n");
		error++;
	}
	if ((*length) <= 0.0) {
		fprintf (stderr, "%s: GMT SYNTAX ERROR -L option:  Length must be positive\n", GMT_program);
		error++;
	}
	if (fabs (*scale_lat) > 90.0) {
		fprintf (stderr, "%s: GMT SYNTAX ERROR -L option:  Defining latitude is out of range\n", GMT_program);
		error++;
	}
	if (isalpha ((int)(*measure)) && ! ((*measure) == 'm' || (*measure) == 'n' || (*measure) == 'k')) {
		fprintf (stderr, "%s: GMT SYNTAX ERROR -L option:  Valid units are m, n, or k\n", GMT_program);
		error++;
	}
	
	return (error);
}

int GMT_minmaxinc_verify (double min, double max, double inc, double slop)
{
	double checkval, range;
	
	/* Check for how compatible inc is with the range max - min.
	   We will tolerate a fractional sloppiness <= slop.  The
	   return values are:
	   0 : Everything is ok
	   1 : range is not a whole multiple of inc (within assiged slop)
	   2 : inc is <= 0
	   3 : the range (max - min) is <= 0
	*/
	   
	if (inc <= 0.0) return (3); 
	   
	if ((range = (max - min)) <= 0.0) return (2);
	
	checkval = (fmod (max - min, inc)) / inc;
	if (checkval > slop && checkval < (1.0 - slop)) return 1;
	return 0;
}
