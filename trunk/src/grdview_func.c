/*--------------------------------------------------------------------
 *	$Id: grdview_func.c,v 1.16 2011-04-23 00:56:09 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
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
 * Brief synopsis: grdview will read a topofile and produce a 3-D perspective plot
 * of the surface z = f(x,y) using PostScript. The surface can
 * be represented as:
 *	1) A Mesh plot
 *	2) A shaded (or colored) surface w/wo contourlines and w/wo
 *	   illumination by artificial sun(s).
 *
 * grdview calls contours to find the line segments that make up the
 * contour lines. This allows the user to specify that the contours
 * should be smoothed before plotting. This will make the resulting
 * image smoother, especially if nx and ny are relatively small.
 * As an option, a drape grid file can be specified.  Then, the colors
 * are calculated from that file while the topo file is used for shape.
 * Alternatively, give three drape files (red, green, blue components)
 * to bypass the z -> rgb via the cpt file.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */
 
#include "pslib.h"
#include "gmt.h"

/* grdview needs to work on "tiles" which is defined as the "square" regions made
 * up by 4 nodes.  Since painting takes place within the tiles, any pixel-registered
 * grids will loose 1/2 a grid-spacing in extend in all directions.  We simplify this
 * difference by converting the headers of all pixel-registered grids to become grid-
 * line-registered grids thus: (1) shrink w/e/s/n by 1/2 grid spacing, and (2) set
 * the registration parameter to 0.
 * We can then proceed with algorithms that require gridline registration. */

/* Declarations needed for binning of smooth contours */

#define GRDVIEW_MESH		0	/* Default */
#define GRDVIEW_SURF		1
#define GRDVIEW_IMAGE		2

struct GRDVIEW_CTRL {
	struct In {
		GMT_LONG active;
		char *file;
	} In;
	struct C {	/* -C<cpt> */
		GMT_LONG active;
		char *file;
	} C;
	struct G {	/* -G<drapefile> */
		GMT_LONG active;
		GMT_LONG image;
		char *file[3];
	} G;
	struct I {	/* -G<intensfile> */
		GMT_LONG active;
		char *file;
	} I;
	struct L {	/* -L<flag> */
		GMT_LONG active;
		GMT_LONG interpolant;
		char mode[4];
		double threshold;
	} L;
	struct N {	/* -N<level>[/<color>] */
		GMT_LONG active;
		GMT_LONG facade;
		double rgb[4];
		double level;
	} N;
	struct Q {	/* -Q<type>[g] */
		GMT_LONG active, special;
		GMT_LONG outline;
		GMT_LONG mask;
		GMT_LONG monochrome;
		GMT_LONG mode;	/* GRDVIEW_MESH, GRDVIEW_SURF, GRDVIEW_IMAGE */
		GMT_LONG dpi;
		struct GMT_FILL fill;
	} Q;
	struct S {	/* -S<smooth> */
		GMT_LONG active;
		GMT_LONG value;
	} S;
	struct T {	/* -T[s][o[<pen>] */
		GMT_LONG active;
		GMT_LONG skip;
		GMT_LONG outline;
		struct GMT_PEN pen;
	} T;
	struct W {	/* -W[+]<type><pen> */
		GMT_LONG active;
		GMT_LONG contour;
		struct GMT_PEN pen[3];
	} W;
};

struct GRDVIEW_BIN {
	struct GRDVIEW_CONT *first_cont;
};

struct GRDVIEW_CONT {
	struct GRDVIEW_POINT *first_point;
	struct GRDVIEW_CONT *next_cont;
	double value;
};

struct GRDVIEW_POINT {
	double x, y;
	struct GRDVIEW_POINT *next_point;
};

struct GRDVIEW_CONT *get_cont_struct (struct GMT_CTRL *GMT, GMT_LONG bin, struct GRDVIEW_BIN *binij, double value)
{
	struct GRDVIEW_CONT *cont, *new_cont;

	if (!binij[bin].first_cont) binij[bin].first_cont = GMT_memory (GMT, NULL, 1, struct GRDVIEW_CONT);

	for (cont = binij[bin].first_cont; cont->next_cont && cont->next_cont->value <= value; cont = cont->next_cont);

	new_cont = GMT_memory (GMT, NULL, 1, struct GRDVIEW_CONT);
	if (cont->next_cont) {	/* Put it in the link */
		new_cont->next_cont = cont->next_cont;
		cont->next_cont = new_cont;
	}
	else	/* End of list */
		cont->next_cont = new_cont;
	return (new_cont);
}

struct GRDVIEW_POINT *get_point (struct GMT_CTRL *GMT, double x, double y)
{
	struct GRDVIEW_POINT *point = GMT_memory (GMT, NULL, 1, struct GRDVIEW_POINT);
	point->x = x;
	point->y = y;
	return (point);
}

void grdview_init_setup (struct GMT_CTRL *GMT, struct GMT_GRID *Topo, GMT_LONG draw_plane, double plane_level)
{
	GMT_LONG row, col, ij;
	double xtmp, ytmp, tmp;

	/* We must find the projected min/max in y-direction */

	/* Reset from whatever they were */
	GMT->current.proj.z_project.ymin = +DBL_MAX;
	GMT->current.proj.z_project.ymax = -DBL_MAX;

	GMT_grd_loop (Topo, row, col, ij) {	/* First loop over all the grid-nodes */
		if (GMT_is_fnan (Topo->data[ij])) continue;
		GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (col, Topo->header), GMT_grd_row_to_y (row, Topo->header), (double)Topo->data[ij], &xtmp, &ytmp);
		GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
		GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
	}
	if (draw_plane) {	/* The plane or facade may exceed the current min/max so loop over these as well */
		for (col = 0; col < Topo->header->nx; col++) {
			tmp = GMT_grd_col_to_x (col, Topo->header);
			GMT_geoz_to_xy (GMT, tmp, Topo->header->wesn[YLO], plane_level, &xtmp, &ytmp);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
			GMT_geoz_to_xy (GMT, tmp, Topo->header->wesn[YHI], plane_level, &xtmp, &ytmp);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
		}
		for (row = 0; row < Topo->header->ny; row++) {
			tmp = GMT_grd_row_to_y (row, Topo->header);
			GMT_geoz_to_xy (GMT, Topo->header->wesn[XLO], tmp, plane_level, &xtmp, &ytmp);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
			GMT_geoz_to_xy (GMT, Topo->header->wesn[XHI], tmp, plane_level, &xtmp, &ytmp);
			GMT->current.proj.z_project.ymin = MIN (GMT->current.proj.z_project.ymin, ytmp);
			GMT->current.proj.z_project.ymax = MAX (GMT->current.proj.z_project.ymax, ytmp);
		}
	}
}

double get_intensity (struct GMT_CTRL *GMT, struct GMT_GRID *I, GMT_LONG k)
{
	/* Returns the average intensity for this tile */
	return (0.25 * (I->data[k] + I->data[k+1] + I->data[k-I->header->mx] + I->data[k-I->header->mx+1]));
}

GMT_LONG pixel_inside (struct GMT_CTRL *GMT, GMT_LONG ip, GMT_LONG jp, GMT_LONG *ix, GMT_LONG *iy, GMT_LONG bin, GMT_LONG bin_inc[])
{	/* Returns TRUE of the ip,jp point is inside the polygon defined by the tile */
	GMT_LONG i, what;
	double x[6], y[6];

	for (i = 0; i < 4; i++) {
		x[i] = (double)ix[bin+bin_inc[i]];
		y[i] = (double)iy[bin+bin_inc[i]];
	}
	x[4] = x[0];	y[4] = y[0];
	what = GMT_non_zero_winding (GMT, (double)ip, (double)jp, x, y, 5);
	return (what);
}

GMT_LONG quick_idist (struct GMT_CTRL *GMT, GMT_LONG x1, GMT_LONG y1, GMT_LONG x2, GMT_LONG y2)
{	/* Fast integer distance calculation */
	if ((x2 -= x1) < 0) x2 = -x2;
	if ((y2 -= y1) < 0) y2 = -y2;
	return (x2 + y2 - (((x2 > y2) ? y2 : x2) >> 1));
}

GMT_LONG get_side (struct GMT_CTRL *GMT, double x, double y, double x_left, double y_bottom, double inc[], double inc2[]) {
	/* Figure out on what side this point sits on */

	double del_x, del_y;
	GMT_LONG side;

	del_x = x - x_left;
	if (del_x > inc2[GMT_X]) del_x = inc[GMT_X] - del_x;
	del_y = y - y_bottom;
	if (del_y > inc2[GMT_Y]) del_y = inc[GMT_Y] - del_y;
	if (del_x < del_y) /* Cutting N-S gridlines */
		side = ((x-x_left) > inc2[GMT_X]) ? 1 : 3;
	else /* Cutting E-W gridlines */
		side = ((y-y_bottom) > inc2[GMT_Y]) ? 2 : 0;
	return (side);
}

void copy_points_fw (struct GMT_CTRL *GMT, double x[], double y[], double z[], double v[], double xcont[], double ycont[], double zcont[], double vcont[], GMT_LONG ncont, GMT_LONG *n) {
	GMT_LONG k;
	for (k = 0; k < ncont; k++, (*n)++) {
		x[*n] = xcont[k];
		y[*n] = ycont[k];
		z[*n] = zcont[k];
		v[*n] = vcont[k];
	}
}

void copy_points_bw (struct GMT_CTRL *GMT, double x[], double y[], double z[], double v[], double xcont[], double ycont[], double zcont[], double vcont[], GMT_LONG ncont, GMT_LONG *n) {
	GMT_LONG k;
	for (k = ncont - 1; k >= 0; k--, (*n)++) {
		x[*n] = xcont[k];
		y[*n] = ycont[k];
		z[*n] = zcont[k];
		v[*n] = vcont[k];
	}
}

double get_z_ave (struct GMT_CTRL *GMT, double v[], double next_up, GMT_LONG n) {
	GMT_LONG k;
	double z_ave;

	for (k = 0, z_ave = 0.0; k < n; k++) z_ave += MIN (v[k], next_up);
	return (z_ave / n);
}

void add_node (struct GMT_CTRL *GMT, double x[], double y[], double z[], double v[], GMT_LONG *k, GMT_LONG node, double X_vert[], double Y_vert[], float topo[], float zgrd[], GMT_LONG ij) {
	/* Adds a corner node to list of points and increments counter */
	x[*k] = X_vert[node];
	y[*k] = Y_vert[node];
	z[*k] = topo[ij];
	v[*k] = zgrd[ij];
	(*k)++;
}

void paint_it_grdview (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct GMT_PALETTE *P, double *x, double *y, GMT_LONG n, double z, GMT_LONG intens, GMT_LONG monochrome, double intensity, GMT_LONG outline) {
	GMT_LONG index;
	double rgb[4];
	struct GMT_FILL *f = NULL;

	if (n < 3) return;	/* Need at least 3 points to make a polygon */

	index = GMT_get_rgb_from_z (GMT, P, z, rgb);
	if (P && P->skip) return;	/* Skip this z-slice */

	/* Now we must paint, with colors or patterns */

	if ((index >= 0 && (f = P->range[index].fill)) || (index < 0 && (f = P->patch[index+3].fill)))	/* Pattern */
		GMT_setfill (GMT, PSL, f, outline);
	else {	/* Solid color/gray */
		if (intens) GMT_illuminate (GMT, intensity, rgb);
		if (monochrome) rgb[0] = rgb[1] = rgb[2] = GMT_YIQ (rgb); /* GMT_YIQ transformation */
		PSL_setfill (PSL, rgb, outline);
	}
	PSL_plotpolygon (PSL, x, y, n);
}

void *New_grdview_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDVIEW_CTRL *C = GMT_memory (GMT, NULL, 1, struct GRDVIEW_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	C->T.pen = C->W.pen[0] = C->W.pen[1] = C->W.pen[2] = GMT->current.setting.map_default_pen;	/* Tile and mesh pens */
	C->W.pen[0].width *= 3.0;	/* Contour pen */
	C->W.pen[2].width *= 3.0;	/* Facade pen */
	C->Q.dpi = 100;
	GMT_init_fill (GMT, &C->Q.fill, GMT->PSL->init.page_rgb[0], GMT->PSL->init.page_rgb[1], GMT->PSL->init.page_rgb[2]);
	C->L.interpolant = BCR_BICUBIC; C->L.threshold = 1.0;
	C->S.value = 1;
	return ((void *)C);
}

void Free_grdview_Ctrl (struct GMT_CTRL *GMT, struct GRDVIEW_CTRL *C) {	/* Deallocate control structure */
	GMT_LONG i;
	if (!C) return;
	if (C->In.file) free ((void *)C->In.file);	
	if (C->C.file) free ((void *)C->C.file);	
	for (i = 0; i < 3; i++) if (C->G.file[i]) free ((void *)C->G.file[i]);	
	if (C->I.file) free ((void *)C->I.file);	
	GMT_free (GMT, C);	
}

GMT_LONG GMT_grdview_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;
	struct GMT_PEN P;

	GMT_message (GMT, "grdview %s [API] - Plot topofiles in 3-D\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: grdview <topofile> %s [-B<tickinfo>] [-C<cpt_file>]\n", GMT_J_OPT);
	GMT_message (GMT, "\t[-G<drapefile> | -G<grd_r>,<grd_g>,<grd_b>] [-I<intensfile>] [%s] [-K] [-L[<flags>]]\n", GMT_Jz_OPT);
	GMT_message (GMT, "\t[-N<level>[/<color>]] [-O] [-P] [-Q<type>[g]] [%s]\n", GMT_Rgeoz_OPT);
	GMT_message (GMT, "\t[-S<smooth>] [-T[s][o[<pen>]]] [%s] [%s] [-W<type><pen>]\n\t[%s] [%s]\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\n", GMT_c_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<topofile> is data set to be plotted.\n");
	GMT_explain_options (GMT, "jZ");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_explain_options (GMT, "b");
	GMT_message (GMT, "\t-C Color palette file\n");
	GMT_message (GMT, "\t-G Use <drapefile> rather than <topofile> for color-coding.\n");
	GMT_message (GMT, "\t   Use <topofile> as the relief and drape the image on top.\n");
	GMT_message (GMT, "\t   Note that -Jz and -N always refers to the <topofile>.\n");
	GMT_message (GMT, "\t   Alternatively, give three grid files with the red, green, and blue components in 0-255 range.\n");
	GMT_message (GMT, "\t   If so, you must also choose -Qi.\n");
	GMT_message (GMT, "\t-I Gives name of intensity file and selects illumination.\n");
	GMT_message (GMT, "\t-L Sets boundary conditions when resampling the grid.  <flags> can be either\n");
	GMT_message (GMT, "\t   g for geographic boundary conditions or one or both of\n");
	GMT_message (GMT, "\t   x for periodic boundary conditions on x.\n");
	GMT_message (GMT, "\t   y for periodic boundary conditions on y.\n");
	GMT_message (GMT, "\t   If no <flags> are set, use bilinear rather than bicubic [Default] resampling.\n");
	GMT_explain_options (GMT, "ZK");
	GMT_message (GMT, "\t-N Draw a horizontal plane at z = level.  Append color [/<color>] to paint\n");
	GMT_message (GMT, "\t   the facade between the plane and the data perimeter.\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-Q Sets plot request. Choose one of the following:\n");
	GMT_message (GMT, "\t   -Qm for Mesh plot [Default].  Append /<color> for mesh paint [%s]\n", GMT_putcolor (GMT, GMT->PSL->init.page_rgb));
	GMT_message (GMT, "\t   -Qs[m] for colored or shaded Surface.  Append m to draw meshlines on the surface.\n");
	GMT_message (GMT, "\t   -Qi for scanline converting polygons to rasterimage.  Append effective dpi [100].\n");
	GMT_message (GMT, "\t   -Qc. As -Qi but use PS Level 3 colormasking for nodes with z = NaN.  Append effective dpi [100].\n");
	GMT_message (GMT, "\t   To force a monochrome image using the GMT_YIQ transformation, append g.\n");
	GMT_explain_options (GMT, "R");
	GMT_message (GMT, "\t-S Smooth contours first (see grdview for <smooth> value info).\n");
	GMT_pen_syntax (GMT, 'T', "Image the data without interpolation by painting polygonal tiles.\n\t   Append s to skip tiles for nodes with z = NaN [Default paints all tiles].\n\t   Append o[<pen>] to draw tile outline [Default uses no outline]");
	GMT_message (GMT, "\t   Cannot be used with -Jz|Z as it produces a flat image.\n");
	GMT_explain_options (GMT, "UV");
	GMT_pen_syntax (GMT, 'W', "Sets pen attributes for various features in form <type><pen>");
	GMT_message (GMT, "\t   <type> can be c for contours, m for mesh, and f for facade.\n");
	P = GMT->current.setting.map_default_pen;
	GMT_message (GMT, "\t   m sets attributes for mesh lines [%s].\n", GMT_putpen (GMT, P));
	GMT_message (GMT, "\t     Requires -Qm or -Qsm to take effect.\n");
	P.width *= 3.0;
	GMT_message (GMT, "\t   c draw scontours on top of surface or mesh [Default is no contours].\n");
	GMT_message (GMT, "\t     Optionally append pen attributes [%s].\n", GMT_putpen (GMT, P));
	GMT_message (GMT, "\t   f sets attributes for facade outline [%s].\n", GMT_putpen (GMT, P));
	GMT_message (GMT, "\t     Requires -N to take effect.\n");
	GMT_explain_options (GMT, "Xcpt.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_grdview_parse (struct GMTAPI_CTRL *C, struct GRDVIEW_CTRL *Ctrl, struct GMT_EDGEINFO *edgeinfo, struct GMT_OPTION *options)
{
	/* This parses the options provided to grdview and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, n_files = 0, q_set = 0, n_commas, j, k, n, id, n_drape;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	GMT_boundcond_init (GMT, edgeinfo);

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {
			/* Common parameters */
			case '<':	/* Input file (only one is accepted) */
				Ctrl->In.active = TRUE;
				if (n_files++ == 0) Ctrl->In.file = strdup (opt->arg);
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Cpt file */
				Ctrl->C.active = TRUE;
				Ctrl->C.file = strdup (opt->arg);
				break;
			case 'G':	/* One or Three grid files */
				Ctrl->G.active = TRUE;
				for (k = 0, n_commas = 0; opt->arg[k]; k++) if (opt->arg[k] == ',') n_commas++;
				if (n_commas == 2) {	/* Three r,g,b grids for draping */
					char A[GMT_TEXT_LEN256], B[GMT_TEXT_LEN256], C[GMT_TEXT_LEN256];
					sscanf (opt->arg, "%[^,],%[^,],%s", A, B, C);
					Ctrl->G.file[0] = strdup (A);
					Ctrl->G.file[1] = strdup (B);
					Ctrl->G.file[2] = strdup (C);
					Ctrl->G.image = TRUE;
				}
				else if (n_commas == 0) {
					Ctrl->G.file[0] = strdup (opt->arg);
				}
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error option -G: Usage is -G<z.grd> | -G<r.grd>,<g.grd>,<b.grd>\n");
					n_errors++;
				}
				break;
			case 'I':	/* Intensity grid */
				Ctrl->I.active = TRUE;
				Ctrl->I.file = strdup (opt->arg);
				break;
			case 'L':	/* BC and interpolation mode */
				if (opt->arg[0]) {
					Ctrl->L.active = TRUE;
					strncpy (Ctrl->L.mode, opt->arg, (size_t)4);
				}
				else
					Ctrl->L.interpolant = BCR_BILINEAR;
				break;
			case 'N':	/* Facade */
				if (opt->arg[0]) {
					char colors[GMT_TEXT_LEN64];
					Ctrl->N.active = TRUE;
					n = sscanf (opt->arg, "%lf/%s", &Ctrl->N.level, colors);
					if (n == 2) {
						n_errors += GMT_check_condition (GMT, GMT_getrgb (GMT, colors, Ctrl->N.rgb), "Syntax error option -N: Usage is -N<level>[/<color>]\n");
						Ctrl->N.facade = TRUE;
					}
				}
				else {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error option -N: Usage is -N<level>[/<color>]\n");
					n_errors++;
				}
				break;
			case 'Q':	/* Plot mode */
				Ctrl->Q.active = TRUE;
				q_set++;
				switch (opt->arg[0]) {
					case 'm':	/* Mesh plot */
						Ctrl->Q.mode = GRDVIEW_MESH;
						n_errors += GMT_check_condition (GMT, opt->arg[1] == '/' && GMT_getfill (GMT, &opt->arg[2], &Ctrl->Q.fill), "Syntax error -Qm option: To give mesh color, use -Qm/<color>\n");
						break;
					case 's':	/* Color without contours */
						Ctrl->Q.mode = GRDVIEW_SURF;
						if (opt->arg[1] == 'm') Ctrl->Q.outline = TRUE;
						break;
					case 't':	/* Image without color interpolation */
						Ctrl->Q.special = TRUE;
					case 'i':	/* Image with clipmask */
						Ctrl->Q.mode = GRDVIEW_IMAGE;
						if (opt->arg[1] && isdigit ((int)opt->arg[1])) Ctrl->Q.dpi = atoi (&opt->arg[1]);
						break;
					case 'c':	/* Image with colormask */
						if (opt->arg[1] && isdigit ((int)opt->arg[1])) Ctrl->Q.dpi = atoi (&opt->arg[1]);
						Ctrl->Q.mode = GRDVIEW_IMAGE;
						Ctrl->Q.mask = TRUE;
						break;
					default:
						GMT_report (GMT, GMT_MSG_FATAL, "Syntax error option -Q: Unrecognized qualifier (%c)\n", opt->arg[0]);
						n_errors++;
						break;
				}
				Ctrl->Q.monochrome = (opt->arg[strlen(opt->arg)-1] == 'g');
				break;
			case 'S':	/* Smoothing of contours */
				Ctrl->S.active = TRUE;
				Ctrl->S.value = atoi (opt->arg);
				break;
			case 'T':	/* Tile plot */
				Ctrl->T.active = TRUE;
				k = 0;
				if (opt->arg[0] == 's') Ctrl->T.skip = TRUE, k = 1;
				if (opt->arg[k] == 'o') {	/* Want tile outline also */
					Ctrl->T.outline = TRUE;
					k++;
					if (opt->arg[k] && GMT_getpen (GMT, &opt->arg[k], &Ctrl->T.pen)) {
						GMT_pen_syntax (GMT, 'T', " ");
						n_errors++;
					}
				}
				break;
			case 'W':	/* Contour, mesh, or facade pens */
				Ctrl->W.active = TRUE;
				j = (opt->arg[0] == 'm' || opt->arg[0] == 'c' || opt->arg[0] == 'f');
				id = 0;
				if (j == 1) {	/* First check that the m or c is not part of a color name instead */
					char txt_a[GMT_TEXT_LEN256];
					n = j+1;
					while (opt->arg[n] && opt->arg[n] != ',' && opt->arg[n] != '/') n++;	/* Wind until end or , or / */
					strncpy (txt_a, opt->arg, (size_t)n);	txt_a[n] = '\0';
					if (GMT_colorname2index (GMT, txt_a) >= 0)	/* Found a colorname: reset j to 0 */
						j = id = 0;
					else
						id = (opt->arg[0] == 'f') ? 2 : ((opt->arg[0] == 'm') ? 1 : 0);
				}
				if (GMT_getpen (GMT, &opt->arg[j], &Ctrl->W.pen[id])) {
					GMT_pen_syntax (GMT, 'W', " ");
					n_errors++;
				}
				if (j == 0)	/* Copy pen when using just -W */
					Ctrl->W.pen[2] = Ctrl->W.pen[1] = Ctrl->W.pen[0];
				if (id == 0) Ctrl->W.contour = TRUE;	/* Switch contouring ON for -Wc or -W */
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += GMT_check_condition (GMT, !strcmp (Ctrl->In.file, "="), "Error: Piping of topofile not supported!\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");

	/* Gave more than one -Q setting */
	n_errors += GMT_check_condition (GMT, q_set > 1, "Error: -Qm, -Qs, -Qc, and -Qi are mutually exclusive options\n");
	/* Gave both -Q and -T */
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && Ctrl->Q.active, "Error: -Q and -T are mutually exclusive options\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->In.file, "Syntax error: Must specify input file\n");
	n_drape = Ctrl->G.image ? 3 : 1;
	if (Ctrl->G.active) {
		GMT_LONG i;
		for (i = 0; i < n_drape; i++) {
			n_errors += GMT_check_condition (GMT, !Ctrl->G.file[i][0], "Syntax error -G option: Must specify drape file\n");
		}
		n_errors += GMT_check_condition (GMT, n_drape == 3 && Ctrl->Q.mode != GRDVIEW_IMAGE, "R/G/B drape requires -Qi option\n");
	}
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->I.file, "Syntax error -I option: Must specify intensity file\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->Q.mode == GRDVIEW_SURF || Ctrl->Q.mode == GRDVIEW_IMAGE || Ctrl->W.contour) && !Ctrl->C.file && !Ctrl->G.image, "Syntax error: Must specify color palette table\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.mode == GRDVIEW_IMAGE && Ctrl->Q.dpi <= 0, "Syntax error -Qi option: Must specify positive dpi\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && GMT->current.proj.JZ_set, "Syntax error -T option: Cannot specify -JZ|z\n");
	n_errors += GMT_check_condition (GMT, Ctrl->S.value < 0, "Syntax error -S option: smooth value must be positive\n");
	if (Ctrl->L.active && GMT_boundcond_parse (GMT, edgeinfo, Ctrl->L.mode)) n_errors++;

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_grdview_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_grdview (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG get_contours, bad, good, error = FALSE, pen_set, row, col;
	GMT_LONG first, begin, saddle, drape_resample = FALSE;
	GMT_LONG j, n_edges, max, i_bin, j_bin, i_bin_old, j_bin_old, t_reg, d_reg[3], i_reg = 0;
	GMT_LONG sw, se, nw, ne, n4, nk, c, i_start, i_stop, j_start, j_stop, i_inc, j_inc, ii, jj;
	GMT_LONG bin, i, ij, k, k1, n, n_drape = 0, n_out, bin_inc[4], ij_inc[4];
	GMT_LONG PS_colormask_off = 0, way, *edge = NULL;

	double cval, x_left, x_right, y_top, y_bottom, small = GMT_SMALL, z_ave;
	double inc2[2], take_out, wesn[4], z_val, x_pixel_size, y_pixel_size;
	double this_intensity = 0.0, next_up = 0.0, xmesh[4], ymesh[4], rgb[4];
	double *x_imask = NULL, *y_imask = NULL, x_inc[4], y_inc[4], *x = NULL, *y = NULL;
	double *z = NULL, *v = NULL, *xx = NULL, *yy = NULL, *xval = NULL, *yval = NULL;

	struct GRDVIEW_CONT *start_cont = NULL, *this_cont = NULL, *last_cont = NULL;
	struct GRDVIEW_POINT *this_point = NULL, *last_point = NULL;
	struct GMT_GRID *Drape[3] = {NULL, NULL, NULL}, *Topo = NULL, *Intens = NULL, *Z = NULL;
	struct GMT_EDGEINFO edgeinfo;
	struct GMT_BCR t_bcr, i_bcr;
	struct GRDVIEW_BIN *binij = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GRDVIEW_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;	/* General GMT interal parameters */
	struct PSL_CTRL *PSL = NULL;	/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_grdview_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_grdview_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_grdview", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJR", "BKOPUXxYycpt>" GMT_OPT("E"), options))) Return (error);
	Ctrl = (struct GRDVIEW_CTRL *) New_grdview_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_grdview_parse (API, Ctrl, &edgeinfo, options))) Return (error);
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the grdview main code ----------------------------*/

	GMT->current.plot.mode_3D = 1;	/* Only do background axis first; do foreground at end */
	
	if ((error = GMT_Begin_IO (API, 0, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */

	if (Ctrl->C.active) {
		if (GMT_Get_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->C.file, (void **)&P)) Return (GMT_DATA_READ_ERROR);
		if (P->is_bw) Ctrl->Q.monochrome = TRUE;
		if (P->categorical && Ctrl->W.active) {
			GMT_report (GMT, GMT_MSG_FATAL, "Warning: Categorical data (as implied by CPT file) do not have contours.  Check plot.\n");
		}
	}
	get_contours = (Ctrl->Q.mode == GRDVIEW_MESH && Ctrl->W.contour) || (Ctrl->Q.mode == GRDVIEW_SURF && P->n_colors > 1);

	n_drape = (Ctrl->G.image) ? 3 : 1;

	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->In.file), (void **)&Topo)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
	if (Ctrl->I.active && GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->I.file), (void **)&Intens)) Return (GMT_DATA_READ_ERROR);	/* Get header only */

	if (Ctrl->G.active) {
		for (i = 0; i < n_drape; i++) if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, NULL, GMT_GRID_HEADER, (void **)&(Ctrl->G.file[i]), (void **)&Drape[i])) Return (GMT_DATA_READ_ERROR);	/* Get header only */
	}

	/* Determine what wesn to pass to map_setup */

	if (!GMT->common.R.active) GMT_memcpy (GMT->common.R.wesn, Topo->header->wesn, 4, double);	/* No -R, use grid region */
	GMT_memcpy (wesn, GMT->common.R.wesn, 4, double);
	
	if (GMT->common.R.wesn[ZLO] == 0.0 && GMT->common.R.wesn[ZHI] == 0.0) {
		GMT->common.R.wesn[ZLO] = Topo->header->z_min;
		GMT->common.R.wesn[ZHI] = Topo->header->z_max;
		if (Ctrl->N.active && Ctrl->N.level < GMT->common.R.wesn[ZLO]) GMT->common.R.wesn[ZLO] = Ctrl->N.level;
		if (Ctrl->N.active && Ctrl->N.level > GMT->common.R.wesn[ZHI]) GMT->common.R.wesn[ZHI] = Ctrl->N.level;
	}

	if (GMT_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_RUNTIME_ERROR);

	/* Determine the wesn to be used to read the grid file */

	if (GMT_grd_setregion (GMT, Topo->header, wesn, BCR_BILINEAR)) {
		/* No grid to plot; just do empty map and bail */
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
		GMT_plotinit (API, PSL, options);
		GMT_plane_perspective (GMT, PSL, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT->current.plot.mode_3D |= 2;	/* Ensure that foreground axis is drawn */
		GMT_map_basemap (GMT, PSL);
		GMT_plane_perspective (GMT, PSL, -1, 0.0);
		GMT_plotend (GMT, PSL);
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Topo);
		if (Ctrl->I.active) GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Intens);
		if (Ctrl->C.active) GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&P);
		Return (GMT_OK);
	}

	/* Read data */

	GMT_report (GMT, GMT_MSG_NORMAL, "Processing shape file\n");

	if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->In.file), (void **)&Topo)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
	t_reg = GMT_change_grdreg (GMT, Topo->header, GMT_GRIDLINE_REG);	/* Ensure gridline registration */

	if (Ctrl->G.active) {	/* Draping wanted */
		for (i = 0; i < n_drape; i++) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Processing drape file %s\n", Ctrl->G.file[i]);

			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->G.file[i]), (void **)&Drape[i])) Return (GMT_DATA_READ_ERROR);	/* Get header only */
			if (Drape[i]->header->nx != Topo->header->nx || Drape[i]->header->ny != Topo->header->ny) drape_resample = TRUE;
			d_reg[i] = GMT_change_grdreg (GMT, Drape[i]->header, GMT_GRIDLINE_REG);	/* Ensure gridline registration */
		}
		Z = Drape[0];
	}
	else
		Z = Topo;

	GMT_boundcond_param_prep (GMT, Topo->header, &edgeinfo);

	xval = GMT_memory (GMT, NULL, Topo->header->nx, double);
	yval = GMT_memory (GMT, NULL, Topo->header->ny, double);

	for (i = 0; i < Topo->header->nx; i++) xval[i] = GMT_grd_col_to_x (i, Topo->header);
	for (j = 0; j < Topo->header->ny; j++) yval[j] = GMT_grd_row_to_y (j, Topo->header);

	if (!GMT->current.proj.xyz_pos[2]) d_swap (GMT->common.R.wesn[ZLO], GMT->common.R.wesn[ZHI]);	/* Negative z-scale, must flip */

	ij_inc[0] = 0;		ij_inc[1] = 1;	ij_inc[2] = 1 - Z->header->mx;	ij_inc[3] = -Z->header->mx;
	nw = GMT_IJP (Topo->header, 0, 0);
	ne = GMT_IJP (Topo->header, 0, Topo->header->nx - 1);
	sw = GMT_IJP (Topo->header, Topo->header->ny - 1, 0);
	se = GMT_IJP (Topo->header, Topo->header->ny - 1, Topo->header->nx - 1);

	grdview_init_setup (GMT, Topo, Ctrl->N.active, Ctrl->N.level);	/* Find projected min/max in y-direction */

	i_start = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 2) ? 0 : Z->header->nx - 2;
	i_stop  = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 2) ? Z->header->nx - 1 : -1;
	i_inc   = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 2) ? 1 : -1;
	j_start = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 4) ? Z->header->ny - 1 : 1;
	j_stop  = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 4) ? 0 : Z->header->ny;
	j_inc   = (GMT->current.proj.z_project.quadrant == 1 || GMT->current.proj.z_project.quadrant == 4) ? -1 : 1;
	bin_inc[0] = 0;		bin_inc[1] = 1;	bin_inc[2] = 1 - Z->header->nx;	bin_inc[3] = -Z->header->nx;
	x_inc[0] = x_inc[3] = 0.0;	x_inc[1] = x_inc[2] = Z->header->inc[GMT_X];
	y_inc[0] = y_inc[1] = 0.0;	y_inc[2] = y_inc[3] = Z->header->inc[GMT_Y];

	if (get_contours) {	/* Need to find contours */
		GMT_report (GMT, GMT_MSG_NORMAL, "Find contours\n");
		n_edges = Z->header->ny * (GMT_LONG )ceil (Z->header->nx / 16.0);
		edge = GMT_memory (GMT, NULL, n_edges, GMT_LONG);
		binij = GMT_memory (GMT, NULL, Topo->header->nm, struct GRDVIEW_BIN);
		small = GMT_SMALL * (Z->header->z_max - Z->header->z_min);
		GMT_report (GMT, GMT_MSG_NORMAL, "Trace and bin contours...\n");
		first = TRUE;
		for (c = 0; c < P->n_colors+1; c++) {	/* For each color change */

			/* Reset markers and set up new zero-contour*/

			cval = (c == P->n_colors) ? P->range[c-1].z_high : P->range[c].z_low;

			if (cval < Z->header->z_min || cval > Z->header->z_max) continue;

			GMT_report (GMT, GMT_MSG_NORMAL, "Now tracing contour interval %8g\r", cval);
			take_out = (first) ? cval : cval - P->range[c-1].z_low;
			first = FALSE;
			GMT_grd_loop (Topo, row, col, ij) {
				if (!GMT_is_fnan (Z->data[ij])) Z->data[ij] -= (float)take_out;
				if (Z->data[ij] == 0.0) Z->data[ij] += (float)small;
			}

			begin = TRUE;
			while ((n = GMT_contours (GMT, Z, Ctrl->S.value, GMT->current.setting.interpolant, 0, edge, &begin, &x, &y)) > 0) {

				i_bin_old = j_bin_old = -1;
				for (i = 1; i < n; i++) {
					/* Compute the lower-left bin i,j of the tile cut by the start of the contour (first 2 points) */
					i_bin = (GMT_LONG)floor (((0.5 * (x[i-1] + x[i]) - Z->header->wesn[XLO]) / Z->header->inc[GMT_X]));
					j_bin = (GMT_LONG)floor (((Z->header->wesn[YHI] - 0.5 * (y[i-1] + y[i])) / Z->header->inc[GMT_Y])) + 1;
					if (i_bin != i_bin_old || j_bin != j_bin_old) {	/* Entering new bin */
						bin = j_bin * Z->header->nx + i_bin;
						this_cont = get_cont_struct (GMT, bin, binij, cval);
						this_cont->value = cval;
						this_cont->first_point = get_point (GMT, x[i-1], y[i-1]);
						this_point = this_cont->first_point;
						i_bin_old = i_bin;
						j_bin_old = j_bin;
					}
					this_point->next_point = get_point (GMT, x[i], y[i]);
					this_point = this_point->next_point;
				}
				GMT_free (GMT, x);
				GMT_free (GMT, y);
			}
		}

		/* Remove temporary variables */

		GMT_free (GMT, edge);

		/* Go back to beginning and reread since grd has been destroyed */

		if (Ctrl->G.active) {
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Drape[0]);
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_ALL, (void **)&(Ctrl->G.file[0]), (void **)&Drape[0])) Return (GMT_DATA_READ_ERROR);	/* Get header only */
			Z = Drape[0];
		}
		else {
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Topo);
			if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_ALL, (void **)&(Ctrl->In.file), (void **)&Topo)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
			Z = Topo;
		}
		GMT_change_grdreg (GMT, Z->header, GMT_GRIDLINE_REG);	/* Ensure gridline registration, again */
	}

	if (Ctrl->I.active) {	/* Illumination wanted */

		GMT_report (GMT, GMT_MSG_NORMAL, "Processing illumination file\n");

		if (GMT_Get_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, wesn, GMT_GRID_DATA, (void **)&(Ctrl->I.file), (void **)&Intens)) Return (GMT_DATA_READ_ERROR);	/* Get header only */
		if (Intens->header->nx != Topo->header->nx || Intens->header->ny != Topo->header->ny) {
			GMT_report (GMT, GMT_MSG_FATAL, "Intensity file has improper dimensions!\n");
			Return (EXIT_FAILURE);
		}
		i_reg = GMT_change_grdreg (GMT, Intens->header, GMT_GRIDLINE_REG);	/* Ensure gridline registration */
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	/* Initialize bcr stuff */
	GMT_bcr_init (GMT, Topo->header, Ctrl->L.interpolant, Ctrl->L.threshold, &t_bcr);

	/* Set boundary conditions  */

	GMT_boundcond_grid_set (GMT, Topo, &edgeinfo);

	inc2[GMT_X] = 0.5 * Z->header->inc[GMT_X];	inc2[GMT_Y] = 0.5 * Z->header->inc[GMT_Y];

	for (j = 0; j < Topo->header->ny - 1; j++) {	/* Nodes part of tiles completely outside -R is set to NaN and thus not considered below */
		ij = GMT_IJP (Topo->header, j, 0);
		for (i = 0; i < Topo->header->nx - 1; i++, ij++) {
			for (jj = n_out = 0; jj < 2; jj++) {	/* Loop over the 4 nodes making up one tile */
				for (ii = 0; ii < 2; ii++) if (GMT_map_outside (GMT, xval[i+ii], yval[j+jj])) n_out++;
			}
			if (n_out == 4) Topo->data[ij] = Topo->data[ij+1] = Topo->data[ij+Topo->header->mx] = Topo->data[ij+Topo->header->mx+1] = GMT->session.f_NaN;	/* Entire tile is outside */
		}
	}

	max = 2 * (MAX (1,Ctrl->S.value) * (((Z->header->nx > Z->header->ny) ? Z->header->nx : Z->header->ny) + 2)) + 1;
	x = GMT_memory (GMT, NULL, max, double);
	y = GMT_memory (GMT, NULL, max, double);
	z = GMT_memory (GMT, NULL, max, double);
	v = GMT_memory (GMT, NULL, max, double);

	GMT_report (GMT, GMT_MSG_NORMAL, "Start creating PostScript plot\n");

	GMT_plotinit (API, PSL, options);

	PSL_setformat (PSL, 3);

	GMT_plane_perspective (GMT, PSL, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	if (GMT->current.proj.three_D) GMT_map_basemap (GMT, PSL); /* Plot basemap first if 3-D */
	if (GMT->current.proj.z_pars[0] == 0.0) GMT_map_clip_on (GMT, PSL, GMT->session.no_rgb, 3);
	GMT_plane_perspective (GMT, PSL, -1, 0.0);

	xx = GMT_memory (GMT, NULL, max, double);
	yy = GMT_memory (GMT, NULL, max, double);
	if (Ctrl->N.active) {
		PSL_comment (PSL, "Plot the plane at desired level\n");
		GMT_setpen (GMT, PSL, &Ctrl->W.pen[2]);
		if (!GMT->current.proj.z_project.draw[0]) {	/* Southern side */
			if (GMT->common.R.oblique) {
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[0], GMT->current.proj.z_project.corner_y[0], Ctrl->N.level, &xx[0], &yy[0]);
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[1], GMT->current.proj.z_project.corner_y[1], Ctrl->N.level, &xx[1], &yy[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
			else {
				for (i = 0; i < Z->header->nx; i++) GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (i, Z->header), Z->header->wesn[YLO], Ctrl->N.level, &xx[i], &yy[i]);
				PSL_plotline (PSL, xx, yy, Z->header->nx, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
		}
		if (!GMT->current.proj.z_project.draw[2]) {	/* Northern side */
			if (GMT->common.R.oblique) {
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[3], GMT->current.proj.z_project.corner_y[3], Ctrl->N.level, &xx[0], &yy[0]);
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[2], GMT->current.proj.z_project.corner_y[2], Ctrl->N.level, &xx[1], &yy[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
			else {
				for (i = 0; i < Z->header->nx; i++) GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (i, Z->header), Z->header->wesn[YHI], Ctrl->N.level, &xx[i], &yy[i]);
				PSL_plotline (PSL, xx, yy, Z->header->nx, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
		}
		if (!GMT->current.proj.z_project.draw[3]) {	/* Western side */
			if (GMT->common.R.oblique) {
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[0], GMT->current.proj.z_project.corner_y[0], Ctrl->N.level, &xx[0], &yy[0]);
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[3], GMT->current.proj.z_project.corner_y[3], Ctrl->N.level, &xx[1], &yy[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
			else {
				for (j = 0; j < Z->header->ny; j++) GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], GMT_grd_row_to_y (j, Z->header), Ctrl->N.level, &xx[j], &yy[j]);
				PSL_plotline (PSL, xx, yy, Z->header->ny, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
		}
		if (!GMT->current.proj.z_project.draw[1]) {	/* Eastern side */
			if (GMT->common.R.oblique) {
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[1], GMT->current.proj.z_project.corner_y[1], Ctrl->N.level, &xx[0], &yy[0]);
				GMT_geoz_to_xy (GMT, GMT->current.proj.z_project.corner_x[2], GMT->current.proj.z_project.corner_y[2], Ctrl->N.level, &xx[1], &yy[1]);
				PSL_plotline (PSL, xx, yy, 2, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
			else {
				for (j = 0; j < Z->header->ny; j++) GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], GMT_grd_row_to_y (j, Z->header), Ctrl->N.level, &xx[j], &yy[j]);
				PSL_plotline (PSL, xx, yy, Z->header->ny, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
			}
		}

		if (!GMT->common.R.oblique) {
			GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], Z->header->wesn[YLO], Ctrl->N.level, &xx[0], &yy[0]);
			GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], Z->header->wesn[YLO], Ctrl->N.level, &xx[1], &yy[1]);
			GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], Z->header->wesn[YHI], Ctrl->N.level, &xx[2], &yy[2]);
			GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], Z->header->wesn[YHI], Ctrl->N.level, &xx[3], &yy[3]);
			if (!GMT_is_fnan (Topo->data[nw])) {
				GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], Z->header->wesn[YHI], (double)(Topo->data[nw]), &x_left, &y_top);
				PSL_plotsegment (PSL, x_left, y_top, xx[3], yy[3]);
			}
			if (!GMT_is_fnan (Topo->data[ne])) {
				GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], Z->header->wesn[YHI], (double)(Topo->data[ne]), &x_right, &y_top);
				PSL_plotsegment (PSL, x_right, y_top, xx[2], yy[2]);
			}
			if (!GMT_is_fnan (Topo->data[se])) {
				GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], Z->header->wesn[YLO], (double)(Topo->data[se]), &x_right, &y_bottom);
				PSL_plotsegment (PSL, x_right, y_bottom, xx[1], yy[1]);
			}
			if (!GMT_is_fnan (Topo->data[sw])) {
				GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], Z->header->wesn[YLO], (double)(Topo->data[sw]), &x_left, &y_bottom);
				PSL_plotsegment (PSL, x_left, y_bottom, xx[0], yy[0]);
			}
		}
	}

	if (Ctrl->T.active) {	/* Plot image as polygonal pieces. Here, -JZ is not set */
		double *xx = NULL, *yy = NULL;
		struct GMT_FILL fill;
		struct GMT_LINE_SEGMENT S;
		GMT_init_fill (GMT, &fill, -1.0, -1.0, -1.0);	/* Initialize fill structure */

		GMT_report (GMT, GMT_MSG_NORMAL, "Tiling without interpolation\n");

		if (Ctrl->T.outline) GMT_setpen (GMT, PSL, &Ctrl->T.pen);
		S.coord = GMT_memory (GMT, NULL, 2, double *);
		GMT_grd_loop (Z, row, col, k) {	/* Compute rgb for each pixel */
			if (GMT_is_fnan (Topo->data[k]) && Ctrl->T.skip) continue;
			if (Ctrl->I.active && Ctrl->T.skip && GMT_is_fnan (Intens->data[k])) continue;
			GMT_get_rgb_from_z (GMT, P, Topo->data[k], fill.rgb);
			if (Ctrl->I.active) GMT_illuminate (GMT, Intens->data[k], fill.rgb);
			n = GMT_graticule_path (GMT, &xx, &yy, 1, xval[col] - inc2[GMT_X], xval[col] + inc2[GMT_X], yval[row] - inc2[GMT_Y], yval[row] + inc2[GMT_Y]);
			GMT_setfill (GMT, PSL, &fill, Ctrl->T.outline);
			S.coord[GMT_X] = xx;	S.coord[GMT_Y] = yy;	S.n_rows = n;
			GMT_geo_polygons (GMT, PSL, &S);
			GMT_free (GMT, xx);
			GMT_free (GMT, yy);
		}
		GMT_free (GMT, S.coord);
		GMT_free (GMT, xval);
		GMT_free (GMT, yval);
	}
	
	else if (Ctrl->Q.mode == GRDVIEW_IMAGE) {	/* Plot image */
		GMT_LONG nx_i, ny_i, kk, ip, jp, min_i, max_i, min_j, max_j, dist, node;
		GMT_LONG done, nm_i, layers, last_i, last_j, p, d_node;
		GMT_LONG *top_jp = NULL, *bottom_jp = NULL, *ix = NULL, *iy = NULL;
		double xp, yp, sum_w, w, sum_i, x_width, y_width, value;
		double sum_r, sum_g, sum_b, intval = 0.0, *y_drape = NULL, *x_drape = NULL;
		float *int_drape = NULL;
		unsigned char *bitimage_24 = NULL, *bitimage_8 = NULL;

		if (Ctrl->C.active && P->has_pattern) GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Patterns in cpt file will not work with -Qi\n");
		GMT_report (GMT, GMT_MSG_NORMAL, "Get and store projected vertices\n");

		PSL_comment (PSL, "Plot 3-D surface using scanline conversion of polygons to raster image\n");

		x_width = GMT->current.proj.z_project.xmax - GMT->current.proj.z_project.xmin;	/* Size of image in inches */
		y_width = GMT->current.proj.z_project.ymax - GMT->current.proj.z_project.ymin;
		nx_i = irint (x_width * Ctrl->Q.dpi);	/* Size of image in pixels */
		ny_i = irint (y_width * Ctrl->Q.dpi);
		last_i = nx_i - 1;	last_j = ny_i - 1;

		if (drape_resample) {
			GMT_report (GMT, GMT_MSG_NORMAL, "Resampling illumination grid to drape grid resolution\n");
			GMT_bcr_init (GMT, Intens->header, Ctrl->L.interpolant, Ctrl->L.threshold, &i_bcr);
			ix = GMT_memory (GMT, NULL, Z->header->nm, GMT_LONG);
			iy = GMT_memory (GMT, NULL, Z->header->nm, GMT_LONG);
			x_drape = GMT_memory (GMT, NULL, Z->header->nx, double);
			y_drape = GMT_memory (GMT, NULL, Z->header->ny, double);
			if (Ctrl->I.active) int_drape = GMT_memory (GMT, NULL, Z->header->mx*Z->header->my, float);
			for (col = 0; col < Z->header->nx; col++) x_drape[col] = GMT_grd_col_to_x (col, Z->header);
			for (row = 0; row < Z->header->ny; row++) y_drape[row] = GMT_grd_row_to_y (row, Z->header);
			bin = 0;
			GMT_grd_loop (Z, row, col, ij) {	/* Get projected coordinates converted to pixel locations */
				value = GMT_get_bcr_z (GMT, Topo, x_drape[col], y_drape[row], &t_bcr);
				if (GMT_is_fnan (value))	/* Outside -R or NaNs not used */
					ix[bin] = iy[bin] = -1;
				else {
					GMT_geoz_to_xy (GMT, x_drape[col], y_drape[row], value, &xp, &yp);
					/* Make sure ix,iy fall in the range (0,nx_i-1), (0,ny_i-1) */
					ix[bin] = MAX(0, MIN((GMT_LONG)floor((xp - GMT->current.proj.z_project.xmin) * Ctrl->Q.dpi), last_i));
					iy[bin] = MAX(0, MIN((GMT_LONG)floor((yp - GMT->current.proj.z_project.ymin) * Ctrl->Q.dpi), last_j));
				}
				if (Ctrl->I.active) int_drape[ij] = (float)GMT_get_bcr_z (GMT, Intens, x_drape[col], y_drape[row], &i_bcr);
				bin++;
			}
			GMT_free (GMT, x_drape);
			GMT_free (GMT, y_drape);
			if (Ctrl->I.active) {	/* Reset intensity grid so that we have no boundary row/cols */
				GMT_free (GMT, Intens->data);
				Intens->data = int_drape;
			}
		}
		else {
			ix = GMT_memory (GMT, NULL, Topo->header->nm, GMT_LONG);
			iy = GMT_memory (GMT, NULL, Topo->header->nm, GMT_LONG);
			bin = 0;
			GMT_grd_loop (Z, row, col, ij) {	/* Get projected coordinates converted to pixel locations */
				if (GMT_is_fnan (Topo->data[ij]))	/* Outside -R or NaNs not used */
					ix[bin] = iy[bin] = -1;
				else {
					GMT_geoz_to_xy (GMT, xval[col], yval[row], (double)Topo->data[ij], &xp, &yp);
					/* Make sure ix,iy fall in the range (0,nx_i-1), (0,ny_i-1) */
					ix[bin] = MAX(0, MIN((GMT_LONG)floor((xp - GMT->current.proj.z_project.xmin) * Ctrl->Q.dpi), last_i));
					iy[bin] = MAX(0, MIN((GMT_LONG)floor((yp - GMT->current.proj.z_project.ymin) * Ctrl->Q.dpi), last_j));
				}
				bin++;
			}
		}
		GMT_free (GMT, xval);
		GMT_free (GMT, yval);

		/* Allocate image array and set background to PAGE_COLOR */

		if (Ctrl->Q.monochrome) {
			char gray;

			nm_i = nx_i * ny_i;
			layers = 1;
			bitimage_8 = GMT_memory (GMT, NULL, nm_i, unsigned char);
			gray = GMT_u255 (GMT_YIQ (GMT->current.setting.ps_page_rgb));
			memset ((void *)bitimage_8, gray, nm_i * sizeof (unsigned char));
		}
		else {
			nm_i = nx_i * ny_i * 3;
			layers = 3;
			if (Ctrl->Q.mask) PS_colormask_off = 3;
			bitimage_24 = GMT_memory (GMT, NULL, nm_i + PS_colormask_off, unsigned char);
			if (Ctrl->C.active && Ctrl->Q.mask)
				GMT_rgb_copy (rgb, P->patch[GMT_NAN].rgb);
			else
				GMT_rgb_copy (rgb, GMT->current.setting.ps_page_rgb);
			kk = 0;
			while (kk < (nm_i + PS_colormask_off)) {
				for (k = 0; k < 3; k++) bitimage_24[kk++] = GMT_u255 (rgb[k]);
			}
		}

		if (!Ctrl->Q.mask) {	/* Set up arrays for staircase clippath and initialize them */
			top_jp = GMT_memory (GMT, NULL, nx_i, GMT_LONG);
			bottom_jp = GMT_memory (GMT, NULL, nx_i, GMT_LONG);
			for (ip = 0; ip < nx_i; ip++) bottom_jp[ip] = ny_i;
		}

		/* Plot from back to front */

		GMT_memset (rgb, 4, double);
		GMT_report (GMT, GMT_MSG_NORMAL, "Start rasterization\n");
		for (j = j_start; j != j_stop; j += j_inc) {

			GMT_report (GMT, GMT_MSG_NORMAL, "Scan line conversion at j-line %.6ld\r", j);

			for (i = i_start; i != i_stop; i += i_inc) {
				bin = GMT_IJ0 (Z->header, j, i);
				ij = GMT_IJP (Z->header, j, i);
				for (k = bad = 0; !bad && k < 4; k++) bad = (ix[bin+bin_inc[k]] < 0 || iy[bin+bin_inc[k]] < 0);
				if (bad) continue;

				min_i = max_i = ix[bin];
				min_j = max_j = iy[bin];
				for (k = 1; k < 4; k++) {
					p = bin+bin_inc[k];
					if (ix[p] < min_i) min_i = ix[p];
					if (ix[p] > max_i) max_i = ix[p];
					if (iy[p] < min_j) min_j = iy[p];
					if (iy[p] > max_j) max_j = iy[p];
				}
				for (jp = min_j; jp <= max_j; jp++) {	/* Loop over all the pixels that will make up this tile */
					if (jp < 0 || jp >= ny_i) continue;
					for (ip = min_i; ip <= max_i; ip++) {
						if (ip < 0 || ip >= nx_i) continue;
						if (!pixel_inside (GMT, ip, jp, ix, iy, bin, bin_inc)) continue;
						/* These pixels are part of the current tile */
						if (!Ctrl->Q.mask) {	/* Update clip mask */
							if (jp > top_jp[ip]) top_jp[ip] = jp; 
							if (jp < bottom_jp[ip]) bottom_jp[ip] = jp;
						}

						sum_r = sum_g = sum_b = sum_w = sum_i = 0.0;
						done = FALSE;
						for (k = good = 0; !done && k < 4; k++) {	/* Loop over the 4 corners of the present tile */
							node = bin + bin_inc[k];
							d_node = ij + ij_inc[k];
							if (Ctrl->G.image) {	/* Have 3 grids with R,G,B values */
								for (kk = 0; kk < 3; kk++) {
									rgb[kk] = GMT_is255 (Drape[kk]->data[d_node]);
									if (rgb[kk] < 0.0) rgb[kk] = 0; else if (rgb[kk] > 1.0) rgb[kk] = 1.0;
								}
								if (Ctrl->C.active && GMT_same_rgb (rgb, P->patch[GMT_NAN].rgb)) continue;	/* Skip NaN colors */
							}
							else {		/* Use lookup to get color */
								GMT_get_rgb_from_z (GMT, P, Z->data[d_node], rgb);
								if (GMT_is_fnan (Z->data[d_node])) continue;	/* Skip NaNs in the z-data*/
							}
							if (Ctrl->I.active && GMT_is_fnan (Intens->data[d_node])) continue;	/* Skip NaNs in the intensity data*/
							/* We don't want to blend in the (typically) gray NaN colors with the others. */
							
							good++;
							dist = quick_idist (GMT, ip, jp, ix[node], iy[node]);
							if (dist == 0) {	/* Only need this corner value */
								done = TRUE;
								if (Ctrl->I.active) intval = Intens->data[d_node];
							}
							else {	/* Crude weighted average based on 1/distance to the nearest node */
								w = 1.0 / (double)dist;
								sum_r += rgb[0] * w;
								sum_g += rgb[1] * w;
								sum_b += rgb[2] * w;
								if (Ctrl->I.active) sum_i += Intens->data[d_node] * w;
								sum_w += w;
							}
						}
						if (!done && good) {	/* Must get weighted value when more than one non-nan value was found */
							sum_w = 1.0 / sum_w;
							rgb[0] = sum_r * sum_w;
							rgb[1] = sum_g * sum_w;
							rgb[2] = sum_b * sum_w;
							if (Ctrl->I.active) intval = sum_i * sum_w;
						}
						if (Ctrl->Q.special) GMT_get_rgb_from_z (GMT, P, Z->data[ij], rgb);
						if (Ctrl->I.active && good) GMT_illuminate (GMT, intval, rgb);
						kk = layers * ((ny_i-jp-1) * nx_i + ip) + PS_colormask_off;
						if (Ctrl->Q.monochrome) /* GMT_YIQ transformation */
							bitimage_8[kk] = (unsigned char) GMT_YIQ (rgb);
						else {
							for (k = 0; k < 3; k++) bitimage_24[kk++] = GMT_u255 (rgb[k]);
						}
					}
				}
			}
		}

		if (!Ctrl->Q.mask) {	/* Must implement the clip path for the image */

			x_pixel_size = x_width / (double)nx_i;
			y_pixel_size = y_width / (double)ny_i;
			n4 = 4 * nx_i;
			x_imask = GMT_memory (GMT, NULL, n4, double);
			y_imask = GMT_memory (GMT, NULL, n4, double);
			nk = n4 - 1;

			for (ip = k = 0; ip < nx_i; ip++, k+= 2) {
				k1 = k + 1;
				x_imask[k]  = x_imask[nk-k]  = GMT->current.proj.z_project.xmin + ip * x_pixel_size;
				x_imask[k1] = x_imask[nk-k1] = x_imask[k] + x_pixel_size;
				if (top_jp[ip] < bottom_jp[ip]) {	/* No pixels set in this column */
					y_imask[k] = y_imask[k1] = y_imask[nk-k] = y_imask[nk-k1] = GMT->current.proj.z_project.ymin;
				}
				else {	/* Set top of upper pixel and bottom of lower pixel */
					y_imask[k] = y_imask[k1] = GMT->current.proj.z_project.ymin + (top_jp[ip] + 1) * y_pixel_size;
					y_imask[nk-k] = y_imask[nk-k1] = GMT->current.proj.z_project.ymin + bottom_jp[ip] * y_pixel_size;
				}
			}
			PSL_beginclipping (PSL, x_imask, y_imask, 4 * nx_i, GMT->session.no_rgb, 3);
			GMT_free (GMT, x_imask);
			GMT_free (GMT, y_imask);
		}

		GMT_report (GMT, GMT_MSG_NORMAL, "Creating PostScript image ");
		if (Ctrl->Q.monochrome) {
			if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) GMT_message (GMT, "[B/W image]\n");
			PSL_plotcolorimage (PSL, GMT->current.proj.z_project.xmin, GMT->current.proj.z_project.ymin, x_width, y_width, PSL_BL, bitimage_8, nx_i, ny_i, 8);
			GMT_free (GMT, bitimage_8);
		}
		else {
			if (GMT->current.setting.verbose >= GMT_MSG_NORMAL) GMT_message (GMT, "[color image]\n");
			PSL_plotcolorimage (PSL, GMT->current.proj.z_project.xmin, GMT->current.proj.z_project.ymin, x_width, y_width, PSL_BL, bitimage_24, Ctrl->Q.mask ? -nx_i : nx_i, ny_i, 24);
			GMT_free (GMT, bitimage_24);
		}

		if (!Ctrl->Q.mask){
			PSL_endclipping (PSL, 1);	/* Undo mask clipping */
			GMT_free (GMT, top_jp);
			GMT_free (GMT, bottom_jp);
		}

		GMT_free (GMT, ix);
		GMT_free (GMT, iy);
	}

	else if (Ctrl->Q.mode == GRDVIEW_MESH) {	/* Plot mesh */
		PSL_comment (PSL, "Start of mesh plot\n");
		GMT_setpen (GMT, PSL, &Ctrl->W.pen[1]);
		if (Ctrl->Q.monochrome) Ctrl->Q.fill.rgb[0] = Ctrl->Q.fill.rgb[1] = Ctrl->Q.fill.rgb[2] = GMT_YIQ (Ctrl->Q.fill.rgb);	/* Do GMT_YIQ transformation */
		for (j = j_start; j != j_stop; j += j_inc) {
			y_bottom = yval[j];
			y_top = y_bottom + GMT_abs (j_inc) * Z->header->inc[GMT_Y];
			for (i = i_start; i != i_stop; i += i_inc) {
				bin = GMT_IJ0 (Z->header, j, i);
				ij = GMT_IJP (Topo->header, j, i);
				for (k = bad = 0; !bad && k < 4; k++) bad += GMT_is_fnan (Topo->data[ij+ij_inc[k]]);
				if (bad) continue;
				x_left = xval[i];
				x_right = x_left + GMT_abs (i_inc) * Z->header->inc[GMT_X];
				GMT_geoz_to_xy (GMT, x_left, y_bottom, (double)(Topo->data[ij+ij_inc[0]]), &xx[0], &yy[0]);
				GMT_geoz_to_xy (GMT, x_right, y_bottom, (double)(Topo->data[ij+ij_inc[1]]), &xx[1], &yy[1]);
				GMT_geoz_to_xy (GMT, x_right, y_top, (double)(Topo->data[ij+ij_inc[2]]), &xx[2], &yy[2]);
				GMT_geoz_to_xy (GMT, x_left, y_top, (double)(Topo->data[ij+ij_inc[3]]), &xx[3], &yy[3]);
				GMT_setfill (GMT, PSL, &Ctrl->Q.fill, TRUE);
				PSL_plotpolygon (PSL, xx, yy, 4);
				if (Ctrl->W.contour) {
					pen_set = FALSE;
					if (binij[bin].first_cont == NULL) continue;
					for (this_cont = binij[bin].first_cont->next_cont; this_cont; this_cont = this_cont->next_cont) {
						for (k = 0, this_point = this_cont->first_point; this_point; this_point = this_point->next_point) {
							z_val = (Ctrl->G.active) ? GMT_get_bcr_z (GMT, Topo, (double)this_point->x, (double)this_point->y, &t_bcr) : this_cont->value;
							if (GMT_is_dnan (z_val)) continue;
							GMT_geoz_to_xy (GMT, (double)this_point->x, (double)this_point->y, z_val, &xx[k], &yy[k]);
							k++;
						}
						if (!pen_set) {
							GMT_setpen (GMT, PSL, &Ctrl->W.pen[0]);
							pen_set = TRUE;
						}
						PSL_plotline (PSL, xx, yy, k, PSL_MOVE + PSL_STROKE);
					}
					if (pen_set) GMT_setpen (GMT, PSL, &Ctrl->W.pen[1]);
				}
			}
		}
		GMT_free (GMT, xval);
		GMT_free (GMT, yval);
	}
	
	else if (Ctrl->Q.mode == GRDVIEW_SURF) {	/* Plot surface using closed polygons */
		GMT_LONG start_side, entry_side, exit_side, next_side, low, ncont, nw_se_diagonal, check;
		GMT_LONG corner[2], bad_side[2][2], p, p1, p2, saddle_sign;
		double *xcont = NULL, *ycont = NULL, *zcont = NULL, *vcont = NULL, X_vert[4], Y_vert[4], saddle_small;

		xcont = GMT_memory (GMT, NULL, max, double);
		ycont = GMT_memory (GMT, NULL, max, double);
		zcont = GMT_memory (GMT, NULL, max, double);
		vcont = GMT_memory (GMT, NULL, max, double);

		PSL_comment (PSL, "Start of filled surface\n");
		if (Ctrl->Q.outline) GMT_setpen (GMT, PSL, &Ctrl->W.pen[1]);

		for (j = j_start; j != j_stop; j += j_inc) {
			y_bottom = yval[j];
			y_top = y_bottom + Z->header->inc[GMT_Y];
			for (i = i_start; i != i_stop; i += i_inc) {
				ij = GMT_IJP (Topo->header, j, i);
				bin = GMT_IJ0 (Topo->header, j, i);
				x_left = xval[i];
				x_right = x_left + Z->header->inc[GMT_X];
				for (k = bad = 0; !bad && k < 4; k++) bad += GMT_is_fnan (Topo->data[ij+ij_inc[k]]);
				if (bad) {
					if (P->patch[GMT_NAN].skip || GMT->current.proj.three_D) continue;

					X_vert[0] = X_vert[3] = x_left;	X_vert[1] = X_vert[2] = x_right;
					Y_vert[0] = Y_vert[1] = y_bottom;	Y_vert[2] = Y_vert[3] = y_top;
					for (k = 0; k < 4; k++) GMT_geoz_to_xy (GMT, X_vert[k], Y_vert[k], 0.0, &xmesh[k], &ymesh[k]);
					paint_it_grdview (GMT, PSL, P, xmesh, ymesh, 4, GMT->session.d_NaN, FALSE, Ctrl->Q.monochrome, 0.0, Ctrl->Q.outline);
					continue;
				}

				if (Ctrl->I.active) {
					this_intensity = get_intensity (GMT, Intens, ij);
					if (GMT_is_dnan (this_intensity)) continue;
				}
	
				/* Get mesh polygon */

				X_vert[0] = X_vert[3] = x_left;		X_vert[1] = X_vert[2] = x_right;
				Y_vert[0] = Y_vert[1] = y_bottom;	Y_vert[2] = Y_vert[3] = y_top;

				if (get_contours && binij[bin].first_cont) {	/* Contours go thru here */

					/* Determine if this bin will give us saddle trouble */

					start_cont = this_cont = binij[bin].first_cont->next_cont;
					saddle = FALSE;
					while (!saddle && this_cont->next_cont) {
						if (this_cont->next_cont->value == this_cont->value)
							saddle = TRUE;
						else
							this_cont = this_cont->next_cont;
					}
					if (saddle) {	/* Must deal with this separately */

						this_point = this_cont->first_point;
						entry_side = get_side (GMT, this_point->x, this_point->y, x_left, y_bottom, Z->header->inc, inc2);
						while (this_point->next_point) this_point = this_point->next_point;	/* Go to end */
						exit_side  = get_side (GMT, this_point->x, this_point->y, x_left, y_bottom, Z->header->inc, inc2);


						if (MIN (Z->data[ij+ij_inc[1]], Z->data[ij+ij_inc[3]]) > MAX (Z->data[ij], Z->data[ij+ij_inc[2]])) {
							saddle_sign = +1;
							check = TRUE;
						}
						else if (MAX (Z->data[ij+ij_inc[1]], Z->data[ij+ij_inc[3]]) < MIN (Z->data[ij], Z->data[ij+ij_inc[2]])) {
							saddle_sign = -1;
							check = TRUE;
						}
						else if (MIN (Z->data[ij], Z->data[ij+ij_inc[2]]) > MAX (Z->data[ij+ij_inc[1]], Z->data[ij+ij_inc[3]])) {
							saddle_sign = +1;
							check = FALSE;
						}
						else {
							saddle_sign = -1;
							check = FALSE;
						}
						nw_se_diagonal = ((entry_side + exit_side) == 3);
						if (nw_se_diagonal != check) saddle_sign = -saddle_sign;
						if (nw_se_diagonal) {	/* Diagonal goes NW - SE */
							corner[0] = 0;	bad_side[0][0] = 1;	bad_side[0][1] = 2;
							corner[1] = 2;	bad_side[1][0] = 0;	bad_side[1][1] = 3;
						}
						else {	/* Diagonal goes NE -SW */
							corner[0] = 1;	bad_side[0][0] = 2;	bad_side[0][1] = 3;
							corner[1] = 3;	bad_side[1][0] = 0;	bad_side[1][1] = 1;
						}
						saddle_small = saddle_sign * small;

						for (p = 0; p < 2; p++) {	/* For each triangular half */

							/* Set this points as the start anchor */

							low = corner[p];
							n = 0;
							add_node (GMT, x, y, z, v, &n, low, X_vert, Y_vert, Topo->data, Z->data, ij+ij_inc[low]);
							start_side = next_side = low;
							way = 0;

							for (this_cont = start_cont; this_cont; this_cont = this_cont->next_cont) {

								/* First get all the x/y pairs for this contour */

								for (k = 0, this_point = this_cont->first_point; this_point; this_point = this_point->next_point) {
									xcont[k] = this_point->x;
									ycont[k] = this_point->y;
									zcont[k] = (Ctrl->G.active) ? GMT_get_bcr_z (GMT, Topo, xcont[k], ycont[k], &t_bcr) : this_cont->value;
									if (GMT_is_dnan (zcont[k])) continue;
									vcont[k] = this_cont->value;
									k++;
								}
								ncont = k;

								entry_side = get_side (GMT, xcont[0], ycont[0], x_left, y_bottom, Z->header->inc, inc2);
								exit_side  = get_side (GMT, xcont[ncont-1], ycont[ncont-1], x_left, y_bottom, Z->header->inc, inc2);

								if (entry_side == bad_side[p][0] || entry_side == bad_side[p][1]) continue;
								if (exit_side == bad_side[p][0] || exit_side == bad_side[p][1]) continue;

								/* OK, got the correct contour */

								next_up = (this_cont->next_cont) ? this_cont->next_cont->value : DBL_MAX;

								exit_side  = get_side (GMT, xcont[ncont-1], ycont[ncont-1], x_left, y_bottom, Z->header->inc, inc2);

								if (way == 0 || next_side == entry_side) {	/* Just hook up */
									copy_points_fw (GMT, x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
									next_side = exit_side;
								}
								else if (next_side == exit_side) {	/* Just hook up but reverse */
									copy_points_bw (GMT, x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
									next_side = entry_side;
								}
								/* Compute the xy from the xyz triplets */

								for (k = 0; k < n; k++) GMT_geoz_to_xy (GMT, x[k], y[k], z[k], &xx[k], &yy[k]);
								z_ave = (P->is_continuous) ? get_z_ave (GMT, v, next_up, n) : this_cont->value;

								/* Now paint the polygon piece */

								paint_it_grdview (GMT, PSL, P, xx, yy, n, z_ave-saddle_small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, FALSE);

								/* Reset the anchor points to previous contour */

								n = 0;
								copy_points_fw (GMT, x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
								next_side = exit_side;
								start_side = entry_side;
								way = (Z->data[bin+bin_inc[low]] < this_cont->value) ? -1 : 1;
							}

							/* Final contour needs to add diagonal */

							if (corner[p] == 0 || corner[p] == 2) {
								p1 = (next_side < 2) ? 1 : 3;
								p2 = (next_side < 2) ? 3 : 1;
							}
							else {
								p1 = (next_side % 3) ? 2 : 0;
								p2 = (next_side % 3) ? 0 : 2;
							}
							add_node (GMT, x, y, z, v, &n, p1, X_vert, Y_vert, Topo->data, Z->data, ij+ij_inc[p1]);
							add_node (GMT, x, y, z, v, &n, p2, X_vert, Y_vert, Topo->data, Z->data, ij+ij_inc[p2]);

							/* Compute the xy from the xyz triplets */

							for (k = 0; k < n; k++) GMT_geoz_to_xy (GMT, x[k], y[k], z[k], &xx[k], &yy[k]);

							z_ave = (P->is_continuous) ? get_z_ave (GMT, v, next_up, n) : v[0];

							/* Now paint the polygon piece */

							paint_it_grdview (GMT, PSL, P, xx, yy, n, z_ave+saddle_small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, FALSE);

						} /* End triangular piece */

					} /* End Saddle section */
					else {
						/* Ok, here we do not have to worry about saddles */

						/* Find lowest corner (id = low) */

						for (k = 1, low = 0; k < 4; k++) if (Z->data[ij+ij_inc[k]] < Z->data[ij+ij_inc[low]]) low = k;

						/* Set this points as the start anchor */

						n = 0;
						add_node (GMT, x, y, z, v, &n, low, X_vert, Y_vert, Topo->data, Z->data, ij+ij_inc[low]);
						start_side = next_side = low;
						way = 1;

						this_cont = start_cont;
						while (this_cont) {

							next_up = (this_cont->next_cont) ? this_cont->next_cont->value : DBL_MAX;
							/* First get all the x/y pairs for this contour */

							for (k = 0, this_point = this_cont->first_point; this_point; this_point = this_point->next_point) {
								xcont[k] = this_point->x;
								ycont[k] = this_point->y;
								zcont[k] = (Ctrl->G.active) ? GMT_get_bcr_z (GMT, Topo, xcont[k], ycont[k], &t_bcr) : this_cont->value;
								if (GMT_is_dnan (zcont[k])) continue;
								vcont[k] = this_cont->value;
								k++;
							}
							ncont = k;

							entry_side = get_side (GMT, xcont[0], ycont[0], x_left, y_bottom, Z->header->inc, inc2);
							exit_side  = get_side (GMT, xcont[ncont-1], ycont[ncont-1], x_left, y_bottom, Z->header->inc, inc2);

							while (!(next_side == entry_side || next_side == exit_side)) {	/* Must add intervening corner */
								if (way == 1) next_side = (next_side + 1) % 4;
								add_node (GMT, x, y, z, v, &n, next_side, X_vert, Y_vert, Topo->data, Z->data, ij+ij_inc[next_side]);
								if (way == -1) next_side = (next_side - 1 + 4) % 4;
							}
							if (next_side == entry_side) {	/* Just hook up */
								copy_points_fw (GMT, x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
								next_side = exit_side;
							}
							else if (next_side == exit_side) {	/* Just hook up but reverse */
								copy_points_bw (GMT, x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
								next_side = entry_side;
							}
							/* Now we must complete the polygon if necessary */

							while (!(start_side == next_side)) {	/* Must add intervening corner */
								if (way == 1) next_side = (next_side + 1) % 4;
								add_node (GMT, x, y, z, v, &n, next_side, X_vert, Y_vert, Topo->data, Z->data, ij+ij_inc[next_side]);
								if (way == -1) next_side = (next_side - 1 + 4) % 4;
							}

							/* Compute the xy from the xyz triplets */

							for (k = 0; k < n; k++) GMT_geoz_to_xy (GMT, x[k], y[k], z[k], &xx[k], &yy[k]);
							z_ave = (P->is_continuous) ? get_z_ave (GMT, v, next_up, n) : this_cont->value;

							/* Now paint the polygon piece */

							paint_it_grdview (GMT, PSL, P, xx, yy, n, z_ave-small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, FALSE);

							/* Reset the anchor points to previous contour */

							n = 0;
							copy_points_fw (GMT, x, y, z, v, xcont, ycont, zcont, vcont, ncont, &n);
							next_side = exit_side;
							start_side = entry_side;
							way = (Z->data[ij+ij_inc[start_side]] < this_cont->value) ? -1 : 1;

							this_cont = this_cont->next_cont;	/* Goto next contour */
 						}

						/* Final contour needs to compete with corners only */

						while (!(start_side == next_side)) {	/* Must add intervening corner */
							if (way == 1) next_side = (next_side +1) % 4;
							add_node (GMT, x, y, z, v, &n, next_side, X_vert, Y_vert, Topo->data, Z->data, ij+ij_inc[next_side]);
							if (way == -1) next_side = (next_side - 1 + 4) % 4;
						}

						/* Compute the xy from the xyz triplets */

						for (k = 0; k < n; k++) GMT_geoz_to_xy (GMT, x[k], y[k], z[k], &xx[k], &yy[k]);

						z_ave = (P->is_continuous) ? get_z_ave (GMT, v, next_up, n) : v[0];

						/* Now paint the polygon piece */

						paint_it_grdview (GMT, PSL, P, xx, yy, n, z_ave+small, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, FALSE);

					} /* End non-saddle case */

					/* Draw contour lines if desired */

					pen_set = FALSE;
					for (this_cont = start_cont; Ctrl->W.contour && this_cont; this_cont = this_cont->next_cont) {
						for (k = 0, this_point = this_cont->first_point; this_point; this_point = this_point->next_point) {
							z_val = (Ctrl->G.active) ? GMT_get_bcr_z (GMT, Topo, (double)this_point->x, (double)this_point->y, &t_bcr) : this_cont->value;
							if (GMT_is_dnan (z_val)) continue;

							GMT_geoz_to_xy (GMT, (double)this_point->x, (double)this_point->y, z_val, &xx[k], &yy[k]);
							k++;
						}
						if (!pen_set) {
							GMT_setpen (GMT, PSL, &Ctrl->W.pen[0]);
							pen_set = TRUE;
						}
						PSL_plotline (PSL, xx, yy, k, PSL_MOVE + PSL_STROKE);
					}
					if (pen_set) GMT_setpen (GMT, PSL, &Ctrl->W.pen[1]);
					if (Ctrl->Q.outline) {
						for (k = 0; k < 4; k++) GMT_geoz_to_xy (GMT, X_vert[k], Y_vert[k], (double)(Topo->data[ij+ij_inc[k]]), &xmesh[k], &ymesh[k]);
						PSL_setfill (PSL, GMT->session.no_rgb, TRUE);
						PSL_plotpolygon (PSL, xmesh, ymesh, 4);
					}
				}
				else {	/* No Contours */

					if (P->is_continuous) {	/* Take the color corresponding to the average value of the four corners */
						for (n = 0, z_ave = 0.0; n < 4; n++) z_ave += Z->data[ij+ij_inc[n]];
						z_ave *= 0.25;
					}
					else	/* Take the value of any corner */
						z_ave = Z->data[ij];

					/* Now paint the polygon piece */

					for (k = 0; k < 4; k++) GMT_geoz_to_xy (GMT, X_vert[k], Y_vert[k], (double)(Topo->data[ij+ij_inc[k]]), &xmesh[k], &ymesh[k]);
					paint_it_grdview (GMT, PSL, P, xmesh, ymesh, 4, z_ave, Ctrl->I.active, Ctrl->Q.monochrome, this_intensity, Ctrl->Q.outline);
				}
			}
		}
		GMT_free (GMT, xval);
		GMT_free (GMT, yval);
		GMT_free (GMT, xcont);
		GMT_free (GMT, ycont);
		GMT_free (GMT, zcont);
		GMT_free (GMT, vcont);
	}

	if (Ctrl->W.pen[1].style || Ctrl->W.pen[0].style) PSL_setdash (PSL, CNULL, 0);

	if (GMT->current.proj.z_pars[0] == 0.0) GMT_map_clip_off (GMT, PSL);

	if (Ctrl->N.facade) {	/* Cover the two front sides */
		PSL_comment (PSL, "Painting the frontal facade\n");
		GMT_setpen (GMT, PSL, &Ctrl->W.pen[2]);
		PSL_setfill (PSL, Ctrl->N.rgb, TRUE);
		if (!GMT->current.proj.z_project.draw[0])	{	/* Southern side */
			for (i = n = 0, ij = sw; i < Z->header->nx; i++, ij++) {
				if (GMT_is_fnan (Topo->data[ij])) continue;
				GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (i, Z->header), Z->header->wesn[YLO], (double)(Topo->data[ij]), &xx[n], &yy[n]);
				n++;
			}
			for (i = Z->header->nx - 1; i >= 0; i--, n++) GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (i, Z->header), Z->header->wesn[YLO], Ctrl->N.level, &xx[n], &yy[n]);
			PSL_plotpolygon (PSL, xx, yy, n);
		}
		if (!GMT->current.proj.z_project.draw[1]) {	/*	Eastern side */
			for (j = n = 0, ij = ne; j < Z->header->ny; j++, ij += Topo->header->mx) {
				if (GMT_is_fnan (Topo->data[ij])) continue;
				GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], GMT_grd_row_to_y (j, Z->header), (double)(Topo->data[ij]), &xx[n], &yy[n]);
				n++;
			}
			for (j = Z->header->ny - 1; j >= 0; j--, n++) GMT_geoz_to_xy (GMT, Z->header->wesn[XHI], GMT_grd_row_to_y (j, Z->header), Ctrl->N.level, &xx[n], &yy[n]);
			PSL_plotpolygon (PSL, xx, yy, n);
		}
		if (!GMT->current.proj.z_project.draw[2])	{	/* Northern side */
			for (i = n = 0, ij = nw; i < Z->header->nx; i++, ij++) {
				if (GMT_is_fnan (Topo->data[ij])) continue;
				GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (i, Z->header), Z->header->wesn[YHI], (double)(Topo->data[ij]), &xx[n], &yy[n]);
				n++;
			}
			for (i = Z->header->nx - 1; i >= 0; i--, n++) GMT_geoz_to_xy (GMT, GMT_grd_col_to_x (i, Z->header), Z->header->wesn[YHI], Ctrl->N.level, &xx[n], &yy[n]);
			PSL_plotpolygon (PSL, xx, yy, n);
		}
		if (!GMT->current.proj.z_project.draw[3]) {	/*	Western side */
			for (j = n = 0, ij = nw; j < Z->header->ny; j++, ij += Topo->header->mx) {
				if (GMT_is_fnan (Topo->data[ij])) continue;
				GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], GMT_grd_row_to_y (j, Z->header), (double)(Topo->data[ij]), &xx[n], &yy[n]);
				n++;
			}
			for (j = Z->header->ny - 1; j >= 0; j--, n++) GMT_geoz_to_xy (GMT, Z->header->wesn[XLO], GMT_grd_row_to_y (j, Z->header), Ctrl->N.level, &xx[n], &yy[n]);
			PSL_plotpolygon (PSL, xx, yy, n);
		}
	}

	if (GMT->current.proj.three_D)
		GMT_vertical_axis (GMT, PSL, 2);	/* Draw foreground axis */
	else
		GMT_map_basemap (GMT, PSL);	/* Plot basemap last if not 3-D */

	GMT_plotend (GMT, PSL);

	/* Free memory */

	if (get_contours) {
		for (ij = 0; ij < Topo->header->nm; ij++) {
			if (!binij[ij].first_cont) continue;
			last_cont = binij[ij].first_cont;
			for (this_cont = binij[ij].first_cont->next_cont; this_cont; this_cont = this_cont->next_cont) {
				if (this_cont->first_point) {
					last_point = this_cont->first_point;
					for (this_point = this_cont->first_point->next_point; this_point; this_point = this_point->next_point) {
						GMT_free (GMT, last_point);
						last_point = this_point;
					}
					GMT_free (GMT, last_point);
				}
				GMT_free (GMT, last_cont);
				last_cont = this_cont;
			}
			GMT_free (GMT, last_cont);
		}
		GMT_free (GMT, binij);
	}

	GMT_change_grdreg (GMT, Topo->header, t_reg);	/* Reset registration, if required */
	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Topo);
	if (Ctrl->I.active) {
		GMT_change_grdreg (GMT, Intens->header, i_reg);	/* Reset registration, if required */
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Intens);
	}
	if (Ctrl->C.active) GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&P);
	GMT_free (GMT, xx);
	GMT_free (GMT, yy);
	GMT_free (GMT, x);
	GMT_free (GMT, y);
	GMT_free (GMT, z);
	GMT_free (GMT, v);
	if (Ctrl->G.active) for (i = 0; i < n_drape; i++) {
		GMT_change_grdreg (GMT, Drape[i]->header, d_reg[i]);	/* Reset registration, if required */
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Drape[i]);
	}

	GMT_report (GMT, GMT_MSG_NORMAL, "Done!\n");

	Return (EXIT_SUCCESS);
}
