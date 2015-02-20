/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2015 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
/* Brief synopsis: pscontour will read a file of points in the plane, performs the
 * Delaunay triangulation, and contours these triangles.  As an option
 * the user may provide a file with indices of which vertices constitute
 * the triangles.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"pscontour"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Contour table data by direct triangulation"
#define THIS_MODULE_KEYS	"<DI,CCi,QDi,-Xo"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>BJKOPRUVXYbcdhipstxy" GMT_OPT("EMm")

struct PSCONTOUR_CTRL {
	struct GMT_CONTOUR contour;
	struct A {	/* -A[-][labelinfo] */
		bool active;
		unsigned int mode;	/* 1 turns off all labels */
		double interval;
		double single_cont;
	} A;
	struct C {	/* -C<cpt> */
		bool active;
		bool cpt;
		char *file;
		double interval;
		double single_cont;
	} C;
	struct D {	/* -D<dumpfile> */
		bool active;
		char *file;
	} D;
	struct E {	/* -E<indexfile> */
		bool active;
		char *file;
	} E;
	struct G {	/* -G[d|f|n|l|L|x|X]<params> */
		bool active;
	} G;
	struct I {	/* -I */
		bool active;
	} I;
	struct L {	/* -L<pen> */
		bool active;
		struct GMT_PEN pen;
	} L;
	struct N {	/* -N */
		bool active;
	} N;
	struct S {	/* -S[p|t] */
		bool active;
		unsigned int mode;	/* 0 skip points; 1 skip triangles */
	} S;
	struct T {	/* -T[+|-][<gap>[c|i|p]/<length>[c|i|p]][:LH] */
		bool active;
		bool label;
		bool low, high;	/* true to tick low and high locals */
		double spacing, length;
		char *txt[2];	/* Low and high label */
	} T;
	struct Q {	/* -Q<cut> */
		bool active;
		unsigned int min;
	} Q;
	struct W {	/* -W[+|-]<type><pen> */
		bool active;
		bool color_cont;
		bool color_text;
		struct GMT_PEN pen[2];
	} W;
};

#define PSCONTOUR_MIN_LENGTH 0.01	/* Contours shorter than this are skipped */
#define TICKED_SPACING	15.0		/* Spacing between ticked contour ticks (in points) */
#define TICKED_LENGTH	3.0		/* Length of ticked contour ticks (in points) */

struct SAVE {
	double *x, *y;
	double cval;
	unsigned int n;
	struct GMT_PEN pen;
	bool do_it, high;
};

/* Returns the id of the node common to the two edges */
#define get_node_index(edge_1, edge_2) (((pscontour_sum = (edge_1) + (edge_2)) == 1) ? 1 : ((pscontour_sum == 2) ? 0 : 2))
#define get_other_node(node1, node2) (((pscontour_sum = (node1 + node2)) == 3) ? 0 : ((pscontour_sum == 2) ? 1 : 2))	/* The other node needed */

struct PSCONTOUR_LINE {	/* Beginning and end of straight contour segment */
	double x0, y0;
	double x1, y1;
};

struct PSCONTOUR {
	double val;
	double angle;
	size_t n_alloc;
	unsigned int nl;
	bool do_tick;
	struct PSCONTOUR_LINE *L;
	char type;
};

struct PSCONTOUR_PT {
	double x, y;
	struct PSCONTOUR_PT *next;
};

struct PSCONTOUR_CHAIN {
	struct PSCONTOUR_PT *begin;
	struct PSCONTOUR_PT *end;
	struct PSCONTOUR_CHAIN *next;
};

void *New_pscontour_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSCONTOUR_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	GMT_contlabel_init (GMT, &C->contour, 1);
	C->A.single_cont = GMT->session.d_NaN;
	C->C.single_cont = GMT->session.d_NaN;
	C->D.file = strdup ("contour");
	C->L.pen = GMT->current.setting.map_default_pen;
	C->T.spacing = TICKED_SPACING * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 14p */
	C->T.length  = TICKED_LENGTH  * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 3p */
	C->T.txt[0] = strdup ("-");	/* Default low label */
	C->T.txt[1] = strdup ("+");	/* Default high label */
	C->W.pen[0] = C->W.pen[1] = GMT->current.setting.map_default_pen;
	C->W.pen[1].width *= 3.0;

	return (C);
}

void Free_pscontour_Ctrl (struct GMT_CTRL *GMT, struct PSCONTOUR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->C.file) free (C->C.file);	
	if (C->D.file) free (C->D.file);	
	if (C->E.file) free (C->E.file);	
	if (C->T.txt[0]) free (C->T.txt[0]);	
	if (C->T.txt[1]) free (C->T.txt[1]);	
	GMT_free (GMT, C);	
}

int get_triangle_crossings (struct GMT_CTRL *GMT, struct PSCONTOUR *P, unsigned int n_conts, double *x, double *y, double *z, int *ind, double small, double **xc, double **yc, double **zc, unsigned int **v, unsigned int **cindex)
{
	/* This routine finds all the contour crossings for this triangle.  Each contour consists of
	 * linesegments made up of two points, with coordinates xc, yc, and contour level zc.
	 */
	 
	unsigned int i, j, k, k2, i1, nx, n_ok, *vout = NULL, *cind = NULL, *ctmp = NULL;
	bool ok;
	double xx[3], yy[3], zz[3], zmin, zmax, dz, frac, *xout = NULL, *yout = NULL, *zout = NULL, *ztmp = NULL;
	size_t n_alloc;

	xx[0] = x[ind[0]];	yy[0] = y[ind[0]];	zz[0] = z[ind[0]];
	xx[1] = x[ind[1]];	yy[1] = y[ind[1]];	zz[1] = z[ind[1]];
	xx[2] = x[ind[2]];	yy[2] = y[ind[2]];	zz[2] = z[ind[2]];

	if (GMT_is_dnan (zz[0]) || GMT_is_dnan (zz[1]) || GMT_is_dnan (zz[2])) return (0);	/* Cannot have crossings if NaNs are present */
	if (zz[0] == zz[1] && zz[1] == zz[2]) return (0);	/* Cannot have crossings if all nodes are equal */

	zmin = MIN (zz[0], MIN (zz[1], zz[2]));	/* Min z vertex */
	zmax = MAX (zz[0], MAX (zz[1], zz[2]));	/* Max z vertex */

	i = 0;	j = n_conts - 1;
	while (P[i].val < zmin && i < n_conts) i++;
	while (P[j].val > zmax && j > 0) j--;

	nx = j - i + 1;	/* Total number of contours */

	if (nx <= 0) return (0);

	n_alloc = 2 * nx;
	xout = GMT_memory (GMT, NULL, n_alloc, double);
	yout = GMT_memory (GMT, NULL, n_alloc, double);
	ztmp = GMT_memory (GMT, NULL, n_alloc, double);
	zout = GMT_memory (GMT, NULL, n_alloc, double);
	vout = GMT_memory (GMT, NULL, n_alloc, unsigned int);
	ctmp = GMT_memory (GMT, NULL, nx, unsigned int);
	cind = GMT_memory (GMT, NULL, nx, unsigned int);

	/* Fill out array zout which holds the nx contour levels */

	k = k2 = 0;
	while (i <= j) {
		ztmp[k2] = ztmp[k2+1] = P[i].val;
		ctmp[k++] = i;
		k2 += 2;
		i++;
	}

	/* Loop over the contour levels and determine the line segments */

	for (k = k2 = j = n_ok = 0; k < nx; k++, k2 += 2) {
		ok = false;
		for (i = 0; i < 3; i++) if (zz[i] == ztmp[k2]) zz[i] += small;	/* Refuse to go through nodes */
		for (i = 0; i < 3; i++) {	/* Try each side in turn 0-1, 1-2, 2-0 */
			i1 = (i == 2) ? 0 : i + 1;
			if ((ztmp[k2] >= zz[i] && ztmp[k2] < zz[i1]) || (ztmp[k2] <= zz[i] && ztmp[k2] > zz[i1])) {
				dz = zz[i1] - zz[i];
				if (dz == 0.0) {	/* Contour goes along edge */
					xout[j] = xx[i];	yout[j] = yy[i];
				}
				else {
					frac = (ztmp[k2] - zz[i]) / dz;
					xout[j] = xx[i] + frac * (xx[i1] - xx[i]);
					yout[j] = yy[i] + frac * (yy[i1] - yy[i]);
				}
				zout[j] = ztmp[k2];
				vout[j++] = i;	/* Keep track of the side number */
				ok = true;	/* Wish to add this segment */
			}
		}
		if (j%2)
			j--;	/* Contour went through a single vertex only, skip this */
		else if (ok)
			cind[n_ok++] = ctmp[k];
	}

	nx = j / 2;	/* Since j might have changed */
	if (nx) {
		*xc = xout;
		*yc = yout;
		*zc = zout;
		*v = vout;
		*cindex = cind;
	} else {
		GMT_free (GMT, xout);
		GMT_free (GMT, yout);
		GMT_free (GMT, zout);
		GMT_free (GMT, vout);
		GMT_free (GMT, cind);
	}
	GMT_free (GMT, ztmp);
	GMT_free (GMT, ctmp);
	return (nx);
}

void paint_it_pscontour (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct GMT_PALETTE *P, double x[], double y[], int n, double z) {
	int index;
	double rgb[4];
	struct GMT_FILL *f = NULL;

	if (n < 3) return;	/* Need at least 3 points to make a polygon */

	index = GMT_get_rgb_from_z (GMT, P, z, rgb);
	if (P->skip) return;	/* Skip this z-slice */

	/* Now we must paint, with colors or patterns */

	if ((index >= 0 && (f = P->range[index].fill)) || (index < 0 && (f = P->patch[index+3].fill)))
		GMT_setfill (GMT, f, false);
	else
		PSL_setfill (PSL, rgb, -2);
	/* Contours drawn separately later if desired */
	PSL_plotpolygon (PSL, x, y, n);
}

void sort_and_plot_ticks (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct SAVE *save, size_t n, double *x, double *y, double *z, unsigned int nn, double tick_gap, double tick_length, bool tick_low, bool tick_high, bool tick_label, char *lbl[], unsigned int mode, FILE *fp)
{	/* Labeling and ticking of inner-most contours cannot happen until all contours are found and we can determine
	which are the innermost ones.
	
	   Note: mode = 1 (plot only), 2 (save labels only), 3 (both).
	*/
	unsigned int np, pol, pol2, j, kk, inside, n_ticks, form;
	int way, k;
	double add, dx, dy, x_back, y_back, x_end, y_end, sa, ca, s;
	double x_mean, y_mean, a, xmin, xmax, ymin, ymax, length;

	/* The x/y coordinates in SAVE are now all projected to map inches */

	for (pol = 0; pol < n; pol++) {	/* Mark polygons that have other polygons inside them */
		np = save[pol].n;
		for (pol2 = 0; save[pol].do_it && pol2 < n; pol2++) {
			inside = GMT_non_zero_winding (GMT, save[pol2].x[0], save[pol2].y[0], save[pol].x, save[pol].y, np);
			if (inside == 2) save[pol].do_it = false;
		}
	}

	form = GMT_setfont (GMT, &GMT->current.setting.font_annot[0]);

	/* Here, only the polygons that are innermost (containing the local max/min, will have do_it = true */

	for (pol = 0; pol < n; pol++) {
		if (!save[pol].do_it) continue;
		np = save[pol].n;

		/* Here we need to figure out if this is a local high or low. */

		/* First determine the bounding box for this contour */
		xmin = xmax = save[pol].x[0];	ymin = ymax = save[pol].y[0];
		for (j = 1; j < np; j++) {
			xmin = MIN (xmin, save[pol].x[j]);
			xmax = MAX (xmax, save[pol].x[j]);
			ymin = MIN (ymin, save[pol].y[j]);
			ymax = MAX (ymax, save[pol].y[j]);
		}
		
		/* Now try to find a data point inside this contour */
		
		for (j = 0, k = -1; k < 0 && j < nn; j++) {
			if (GMT_y_is_outside (GMT, y[j], ymin, ymax)) continue;	/* Outside y-range */
			if (GMT_y_is_outside (GMT, x[j], xmin, xmax)) continue;	/* Outside x-range (YES, use GMT_y_is_outside since projected x-coordinates)*/
			
			inside = GMT_non_zero_winding (GMT, x[j], y[j], save[pol].x, save[pol].y, np);
			if (inside == 2) k = j;	/* OK, this point is inside */
		}
		if (k < 0) continue;	/* Unable to determine */
		save[pol].high = (z[k] > save[pol].cval);

		if (save[pol].high && !tick_high) continue;	/* Do not tick highs */
		if (!save[pol].high && !tick_low) continue;	/* Do not tick lows */

		for (j = 1, s = 0.0; j < np; j++) {	/* Compute distance along the contour */
			s += hypot (save[pol].x[j]-save[pol].x[j-1], save[pol].y[j]-save[pol].y[j-1]);
		}
		if (s < PSCONTOUR_MIN_LENGTH) continue;	/* Contour is too short to be ticked or labeled */

		n_ticks = urint (floor (s / tick_gap));
		if (n_ticks == 0) continue;	/* Too short to be ticked or labeled */

		GMT_setpen (GMT, &save[pol].pen);
		way = GMT_polygon_centroid (GMT, save[pol].x, save[pol].y, np, &x_mean, &y_mean);	/* -1 is CCW, +1 is CW */
		if (tick_label) {	/* Compute mean location of closed contour ~hopefully a good point inside to place label. */
			if (mode & 1) PSL_plottext (PSL, x_mean, y_mean, GMT->current.setting.font_annot[0].size, lbl[save[pol].high], 0.0, 6, form);
			if (mode & 2) GMT_write_label_record (GMT, fp, x_mean, y_mean, 0.0, lbl[save[pol].high], mode & 4);
		}
		if (mode & 1) {	/* Tick the innermost contour */
			add = M_PI_2 * ((save[pol].high) ? -way : +way);	/* So that tick points in the right direction */
			for (j = 1; j < np; j++) {	/* Consider each segment from point j-1 to j */
				dx = save[pol].x[j] - save[pol].x[j-1];
				dy = save[pol].y[j] - save[pol].y[j-1];
				length = hypot (dx, dy);
				n_ticks = urint (ceil (length / tick_gap));	/* At least one per side */
				a = atan2 (dy, dx) + add;
				sincos (a, &sa, &ca);
				for (kk = 0; kk <= n_ticks; kk++) {
					x_back = save[pol].x[j-1] + kk * dx / (n_ticks + 1);
					y_back = save[pol].y[j-1] + kk * dy / (n_ticks + 1);
					x_end = x_back + tick_length * ca;
					y_end = y_back + tick_length * sa;
					PSL_plotsegment (PSL, x_back, y_back, x_end, y_end);
				}
			}
		}
	}
}

int GMT_pscontour_usage (struct GMTAPI_CTRL *API, int level)
{
	struct GMT_PEN P;

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: pscontour <table> -C[+]<cont_int>|<cpt> %s\n", GMT_J_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s [-A[-|[+]<annot_int>][<labelinfo>]\n\t[%s] [-D<template>] ", GMT_Rgeoz_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "[-E<indextable>] [%s] [-I] [%s] [-K] [-L<pen>] [-N]\n", GMT_CONTG, GMT_Jz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-O] [-P] [-Q<cut>] [-S[p|t]] [%s]\n", GMT_CONTT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-W[+]<pen>] [%s] [%s]\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\n", GMT_Y_OPT, GMT_b_OPT, GMT_d_OPT, GMT_c_OPT, GMT_h_OPT,
		GMT_i_OPT, GMT_p_OPT, GMT_t_OPT, GMT_s_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-C Contours to be drawn can be specified in one of three ways:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   1. Fixed contour interval, or a single contour if prepended with a + sign.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   2. File with contour levels in col 1 and C(ont) or A(nnot) in col 2\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      [and optionally an individual annotation angle in col 3].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   3. Name of a cpt-file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -T is used, only contours with upper case C or A is ticked\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     [cpt-file contours are set to C unless the CPT flags are set;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use -A to force all to become A].\n");
	GMT_Option (API, "J-Z,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Annotation label information. [Default is no annoted contours].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give annotation interval OR - to disable all contour annotations\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   implied by the informatino provided in -C.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively prepend + to annotation interval to plot that as a single contour.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <labelinfo> controls the specifics of the labels.  Choose from:\n");
	GMT_label_syntax (API->GMT, 5, 0);
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Dump contours as data line segments; no plotting takes place.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append filename template which may contain C-format specifiers.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no filename template is given we write all lines to stdout.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If filename has no specifiers then we write all lines to a single file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If a float format (e.g., %%6.2f) is found we substitute the contour z-value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If an integer format (e.g., %%06d) is found we substitute a running segment count.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If an char format (%%c) is found we substitute C or O for closed and open contours.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The 1-3 specifiers may be combined and appear in any order to produce the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the desired number of output files (e.g., just %%c gives two files, just %%f would.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   separate segments into one file per contour level, and %%d would write all segments.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to individual files; see manual page for more examples.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E File with triplets of point indices for each triangle\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default performs the Delaunay triangulation on xyz-data].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Control placement of labels along contours.  Choose among five algorithms:\n");
	GMT_cont_syntax (API->GMT, 3, 0);
	GMT_Message (API, GMT_TIME_NONE, "\t-I Color triangles using the cpt file.\n");
	GMT_Option (API, "K");
	GMT_pen_syntax (API->GMT, 'L', "Draws the triangular mesh with the specified pen.");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do NOT clip contours/image at the border [Default clips].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Do not draw closed contours with less than <cut> points [Draw all contours].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S (or -Sp) Skip xyz points outside region [Default keeps all].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -St to instead skip triangles whose 3 vertices are outside.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Will embellish innermost, closed contours with ticks pointing in\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the downward direction.  User may specify to tick only highs\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (-T+) or lows (-T-) [-T implies both extrema].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append spacing/ticklength (with units) to change defaults [%gp/%gp].\n", TICKED_SPACING, TICKED_LENGTH);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append :LH to plot L and H in the center of closed contours\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   for local Lows and Highs (e.g, give :-+ to plot - and + signs).\n");
	GMT_Option (API, "U,V");
	GMT_pen_syntax (API->GMT, 'W', "Set pen attributes. Append a<pen> for annotated or c<pen> for regular contours [Default].");
	GMT_Message (API, GMT_TIME_NONE, "\t   The default settings are\n");
	P = API->GMT->current.setting.map_default_pen;
	GMT_Message (API, GMT_TIME_NONE, "\t   Contour pen: %s.\n", GMT_putpen (API->GMT, P));
	P.width *= 3.0;
	GMT_Message (API, GMT_TIME_NONE, "\t   Annotate pen: %s.\n", GMT_putpen (API->GMT, P));
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend + to draw colored contours based on the cpt file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Prepend - to color contours and annotations based on the cpt file.\n");
	GMT_Option (API, "X,bi3,bo,c,d,h,i,p,s,t,:,.");
	
	return (EXIT_FAILURE);
}

int GMT_pscontour_parse (struct GMT_CTRL *GMT, struct PSCONTOUR_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pscontour and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, id;
	int k, j, n;
	char txt_a[GMT_LEN64] = {""}, txt_b[GMT_LEN64] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Annotation control */
				Ctrl->A.active = true;
				if (GMT_contlabel_specs (GMT, opt->arg, &Ctrl->contour)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -A option: Expected\n\t-A[-|<aint>][+a<angle>|n|p[u|d]][+c<dx>[/<dy>]][+d][+e][+f<font>][+g<fill>][+j<just>][+l<label>][+n|N<dx>[/<dy>]][+o][+p<pen>][+r<min_rc>][+t[<file>]][+u<unit>][+v][+w<width>][+=<prefix>]\n");
					n_errors ++;
				}
				else if (opt->arg[0] == '-')
					Ctrl->A.mode = 1;	/* Turn off all labels */
				else if (opt->arg[0] == '+') {
					Ctrl->A.single_cont = atof (&opt->arg[1]);
					Ctrl->contour.annot = true;
				}
				else {
					Ctrl->A.interval = atof (opt->arg);
					Ctrl->contour.annot = true;
				}
				break;
			case 'C':	/* CPT table */
				Ctrl->C.active = true;
				if (GMT_File_Is_Memory (opt->arg)) {	/* Passed a memory reference from a module */
					Ctrl->C.interval = 1.0;
					Ctrl->C.cpt = true;
					if (Ctrl->C.file) free (Ctrl->C.file);
					Ctrl->C.file = strdup (opt->arg);
				}
				else if (!GMT_access (GMT, opt->arg, R_OK)) {	/* Gave a readable file */
					Ctrl->C.interval = 1.0;
					Ctrl->C.cpt = (!strncmp (&opt->arg[strlen(opt->arg)-4], ".cpt", 4U)) ? true : false;
					if (Ctrl->C.file) free (Ctrl->C.file);
					Ctrl->C.file = strdup (opt->arg);
				}
				else if (opt->arg[0] == '+')
					Ctrl->C.single_cont = atof (&opt->arg[1]);
				else
					Ctrl->C.interval = atof (opt->arg);
				break;
			case 'D':	/* Dump contours */
				Ctrl->D.active = true;
				free (Ctrl->D.file);
				Ctrl->D.file = strdup (opt->arg);
				break;
			case 'E':	/* Triplet file */
				Ctrl->E.file = strdup (opt->arg);
				Ctrl->E.active = true;
				break;
			case 'G':	/* contour annotation settings */
				Ctrl->G.active = true;
				n_errors += GMT_contlabel_info (GMT, 'G', opt->arg, &Ctrl->contour);
				break;
			case 'I':	/* Image triangles */
				Ctrl->I.active = true;
				break;
			case 'L':	/* Draw triangular mesh lines */
				Ctrl->L.active = true;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->L.pen)) {
					GMT_pen_syntax (GMT, 'L', " ");
					n_errors++;
				}
				break;
			case 'N':	/* Do not clip at boundary */
				Ctrl->N.active = true;
				break;
			case 'Q':	/* Skip small closed contours */
				if (!GMT_access (GMT, opt->arg, F_OK) && GMT_compat_check (GMT, 4)) {	/* Must be the now old -Q<indexfile> option, set to -E */
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -Q<indexfile> is deprecated; use -E instead.\n");
					Ctrl->E.file = strdup (opt->arg);
					Ctrl->E.active = true;
					break;
				}
				Ctrl->Q.active = true;
				n = atoi (opt->arg);
				n_errors += GMT_check_condition (GMT, n < 0, "Syntax error -Q option: Value must be >= 0\n");
				Ctrl->Q.min = n;
				break;
			case 'S':	/* Skip points outside border */
				Ctrl->S.active = true;
				if (opt->arg[0] == 'p') Ctrl->S.mode = 0;
				else if (opt->arg[0] == 't') Ctrl->S.mode = 1;
				break;
			case 'T':	/* Embellish innermost closed contours */
				if (!GMT_access (GMT, opt->arg, F_OK) && GMT_compat_check (GMT, 4)) {	/* Must be the old -T<indexfile> option, set to -E */
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: Option -T<indexfile> is deprecated; use -E instead.\n");
					Ctrl->E.file = strdup (opt->arg);
					Ctrl->E.active = true;
					break;
				}
				Ctrl->T.active = Ctrl->T.high = Ctrl->T.low = true;	/* Default if just -T is given */
				if (opt->arg[0]) {	/* But here we gave more options */
					if (opt->arg[0] == '+')			/* Only tick local highs */
						Ctrl->T.low = false, j = 1;
					else if (opt->arg[0] == '-')		/* Only tick local lows */
						Ctrl->T.high = false, j = 1;
					else
						j = 0;
					if (strchr (&opt->arg[j], '/')) {	/* Gave gap/length */
						n = sscanf (&opt->arg[j], "%[^/]/%[^:]", txt_a, txt_b);
						if (n == 2) {
							Ctrl->T.spacing = GMT_to_inch (GMT, txt_a);
							Ctrl->T.length = GMT_to_inch (GMT, txt_b);
						}
					}
					n = 0;
					for (j = 0; opt->arg[j] && opt->arg[j] != ':'; j++);
					if (opt->arg[j] == ':') Ctrl->T.label = true, j++;
					if (opt->arg[j]) {	/* Override high/low markers */
						if (strlen (&(opt->arg[j])) == 2) {	/* Standard :LH syntax */
							txt_a[0] = opt->arg[j++];	txt_a[1] = '\0';
							txt_b[0] = opt->arg[j++];	txt_b[1] = '\0';
							n = 2;
						}
						else if (strchr (&(opt->arg[j]), ',')) {	/* Found :<labellow>,<labelhigh> */
							n = sscanf (&(opt->arg[j]), "%[^,],%s", txt_a, txt_b);
						}
						else {
							GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -T option: Give low and high labels either as -:LH or -:<low>,<high>.\n");
							Ctrl->T.label = false;
							n_errors++;
						}
						if (Ctrl->T.label) {	/* Replace defaults */
							free (Ctrl->T.txt[0]);
							free (Ctrl->T.txt[1]);
							Ctrl->T.txt[0] = strdup (txt_a);
							Ctrl->T.txt[1] = strdup (txt_b);
						}
					}
					n_errors += GMT_check_condition (GMT, n == 1 || Ctrl->T.spacing <= 0.0 || Ctrl->T.length == 0.0, "Syntax error -T option: Expected\n\t-T[+|-][<tick_gap>[%s]/<tick_length>[%s]][:LH], <tick_gap> must be > 0\n", GMT_DIM_UNITS_DISPLAY, GMT_DIM_UNITS_DISPLAY);
				}
				break;
			case 'W':	/* Sets pen attributes */
				Ctrl->W.active = true;
				k = 0;
				if (opt->arg[k] == '+') Ctrl->W.color_cont = true, k++;
				if (opt->arg[k] == '-') Ctrl->W.color_cont = Ctrl->W.color_text = true, k++;
				j = (opt->arg[k] == 'a' || opt->arg[k] == 'c') ? k+1 : k;
				if (j == k) {	/* Set both */
					if (GMT_getpen (GMT, &opt->arg[j], &Ctrl->W.pen[0])) {
						GMT_pen_syntax (GMT, 'W', " ");
						n_errors++;
					}
					else Ctrl->W.pen[1] = Ctrl->W.pen[0];
				}
				else {	/* Gave a or c.  Because the user may say -Wcyan we must prevent this from being seen as -Wc and color yan! */
					/* Get the argument following a or c and up to first comma, slash (or to the end) */
					n = k+1;
					while (!(opt->arg[n] == ',' || opt->arg[n] == '/' || opt->arg[n] == '\0')) n++;
					strncpy (txt_a, &opt->arg[k], (size_t)(n-k));	txt_a[n-k] = '\0';
					if (GMT_colorname2index (GMT, txt_a) >= 0) j = k;	/* Found a colorname; wind j back by 1 */
					id = (opt->arg[k] == 'a') ? 1 : 0;
					if (GMT_getpen (GMT, &opt->arg[j], &Ctrl->W.pen[id])) {
						GMT_pen_syntax (GMT, 'W', " ");
						n_errors++;
					}
					if (j == k) Ctrl->W.pen[1] = Ctrl->W.pen[0];	/* Must copy since it was not -Wc nor -Wa after all */
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->A.interval > 0.0 && (!Ctrl->C.file && Ctrl->C.interval == 0.0)) Ctrl->C.interval = Ctrl->A.interval;

	/* Check that the options selected are mutually consistent */

	n_errors += GMT_check_condition (GMT, !GMT->common.J.active && !Ctrl->D.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active && !Ctrl->D.active, "Syntax error: Must specify a region with the -R option\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->C.file && Ctrl->C.interval <= 0.0 && 
			GMT_is_dnan (Ctrl->C.single_cont) && GMT_is_dnan (Ctrl->A.single_cont), 
			"Syntax error -C option: Must specify contour interval, file name with levels, or cpt-file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->D.active && !Ctrl->E.active && !(Ctrl->W.active || Ctrl->I.active), "Syntax error: Must specify one of -W or -I\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && (Ctrl->I.active || Ctrl->L.active || Ctrl->N.active || Ctrl->G.active || Ctrl->W.active), "Syntax error: Cannot use -G, -I, -L, -N, -W with -D\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->C.file, "Syntax error -I option: Must specify a color palette table via -C\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && !Ctrl->E.file, "Syntax error -E option: Must specify an index file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && Ctrl->E.file && GMT_access (GMT, Ctrl->E.file, F_OK), "Syntax error -E option: Cannot find file %s\n", Ctrl->E.file);
	n_errors += GMT_check_condition (GMT, Ctrl->W.color_cont && !Ctrl->C.cpt, "Syntax error -W option: + or - only valid if -C sets a cpt file\n");
	n_errors += GMT_check_binary_io (GMT, 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_pscontour_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_pscontour (void *V_API, int mode, void *args)
{
	int add, error = 0;
	bool two_only = false, make_plot, skip = false, convert, get_contours, is_closed, skip_points, skip_triangles;
	
	unsigned int pscontour_sum, m, n, nx, k2, k3, node1, node2, c, cont_counts[2] = {0, 0};
	unsigned int label_mode = 0, last_entry, last_exit, fmt[3] = {0, 0, 0}, n_skipped, n_out;
	unsigned int i, low, high, n_contours = 0, n_tables = 0, tbl_scl = 0, io_mode = 0, tbl, id, *vert = NULL, *cind = NULL;
	
	size_t n_alloc, n_save = 0, n_save_alloc = 0, *n_seg_alloc = NULL, c_alloc = 0;
	
	uint64_t k, np, ij, *n_seg = NULL;
	
	int *ind = NULL;	/* Must remain int due to triangle */
	
	double xx[3], yy[3], zz[3], xout[5], yout[5], xyz[2][3], rgb[4], z_range, small;
	double *xc = NULL, *yc = NULL, *zc = NULL, *x = NULL, *y = NULL, *z = NULL;
	double current_contour = -DBL_MAX, *in = NULL, *xp = NULL, *yp = NULL;

	char cont_label[GMT_LEN256] = {""}, format[GMT_LEN256] = {""};
	char *tri_algorithm[2] = {"Watson", "Shewchuk"};

	struct PSCONTOUR *cont = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_PALETTE *P = NULL;
	struct SAVE *save = NULL;
	struct PSCONTOUR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_pscontour_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_pscontour_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_pscontour_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_pscontour_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pscontour_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the pscontour main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	if ((error = GMT_set_cols (GMT, GMT_IN, 3)) != GMT_OK) {
		Return (error);
	}
	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	if (Ctrl->C.cpt) {	/* Presumably got a cpt-file; read it here so we can crash if no-such-file before we process input data */
		if ((P = GMT_Read_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (Ctrl->I.active && P->is_continuous) {
			GMT_Report (API, GMT_MSG_NORMAL, "-I option requires constant color between contours!\n");
			Return (GMT_OK);
		}
		if (P->categorical) {
			GMT_Report (API, GMT_MSG_NORMAL, "Warning: Categorical data (as implied by CPT file) do not have contours.  Check plot.\n");
		}
	}
	make_plot = !Ctrl->D.active;	/* Turn off plotting if -D was used */
	convert = (make_plot || (GMT->common.R.active && GMT->common.J.active));
	get_contours = (Ctrl->D.active || Ctrl->W.active);

	if (GMT->common.J.active && GMT_err_fail (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	n_alloc = GMT_INITIAL_MEM_ROW_ALLOC;
	x = GMT_memory (GMT, NULL, n_alloc, double);
	y = GMT_memory (GMT, NULL, n_alloc, double);
	z = GMT_memory (GMT, NULL, n_alloc, double);

	xyz[0][GMT_Z] = DBL_MAX;	xyz[1][GMT_Z] = -DBL_MAX;
	n = 0;
	skip_points = (Ctrl->S.active && Ctrl->S.mode == 0);
	skip_triangles = (Ctrl->S.active && Ctrl->S.mode == 1);

	do {	/* Keep returning records until we reach EOF */
		if ((in = GMT_Get_Record (API, GMT_READ_DOUBLE, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all table and segment headers */
				continue;
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record to process */
		
		if (skip_points) {	/* Must check if points are inside plot region */
			GMT_map_outside (GMT, in[GMT_X], in[GMT_Y]);
			skip = (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1);
		}

		if (!(skip || GMT_is_dnan (in[GMT_Z]))) {	/* Unless outside or z = NaN */

			x[n] = in[GMT_X];
			y[n] = in[GMT_Y];
			z[n] = in[GMT_Z];
			if (z[n] < xyz[0][GMT_Z]) {	/* New minimum */
				xyz[0][GMT_X] = x[n];
				xyz[0][GMT_Y] = y[n];
				xyz[0][GMT_Z] = z[n];
			}
			if (z[n] > xyz[1][GMT_Z]) {	/* New maximum */
				xyz[1][GMT_X] = x[n];
				xyz[1][GMT_Y] = y[n];
				xyz[1][GMT_Z] = z[n];
			}
			n++;

			if (n == n_alloc) {
				n_alloc <<= 1;
				x = GMT_memory (GMT, x, n_alloc, double);
				y = GMT_memory (GMT, y, n_alloc, double);
				z = GMT_memory (GMT, z, n_alloc, double);
			}
			if (n == INT_MAX) {
				GMT_Report (API, GMT_MSG_NORMAL, "Error: Cannot triangulate more than %d points\n", INT_MAX);
				GMT_free (GMT, x);
				GMT_free (GMT, y);
				GMT_free (GMT, z);
				Return (EXIT_FAILURE);
			}	
		}
	} while (true);

	x = GMT_memory (GMT, x, n, double);
	y = GMT_memory (GMT, y, n, double);
	z = GMT_memory (GMT, z, n, double);

	if (make_plot && GMT_contlabel_prep (GMT, &Ctrl->contour, xyz)) Return (EXIT_FAILURE);	/* Prep for crossing lines, if any */

	/* Map transform */

	if (convert) for (i = 0; i < n; i++) GMT_geo_to_xy (GMT, x[i], y[i], &x[i], &y[i]);

	if (Ctrl->E.active) {	/* Read precalculated triangulation indices */
		uint64_t seg, row, col;
		double d_n = (double)n - 0.5;	/* So we can use > in test near line 806 */
		struct GMT_DATASET *Tin = NULL;
		struct GMT_DATATABLE *T = NULL;

		if ((Tin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->E.file, NULL)) == NULL) {
			Return (API->error);
		}

 		if (Tin->n_columns < 3) {	/* Trouble */
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -E: %s does not have at least 3 columns with indices\n", Ctrl->E.file);
			if (GMT_Destroy_Data (API, &Tin) != GMT_OK) {
				Return (API->error);
			}
			Return (EXIT_FAILURE);
		}
		T = Tin->table[0];	/* Since we only have one table here */
		np = T->n_records;
		ind = GMT_memory (GMT, NULL, 3 * np, int);	/* Allocate the integer index array */
		for (seg = ij = n_skipped = 0; seg < T->n_segments; seg++) {
			for (row = 0; row < T->segment[seg]->n_rows; row++) {
				if (T->segment[seg]->coord[0][row] > d_n || T->segment[seg]->coord[1][row] > d_n || T->segment[seg]->coord[2][row] > d_n)
					n_skipped++;	/* Outside point range */
				else {
					for (col = 0; col < 3; col++) ind[ij++] = irint (T->segment[seg]->coord[col][row]);
				}
			}
		}
		np = ij / 3;	/* The actual number of vertices passing the test */
		if (GMT_Destroy_Data (API, &Tin) != GMT_OK) {
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_VERBOSE, "Read %d indices triplets from %s.\n", np, Ctrl->E.file);
		if (n_skipped) GMT_Report (API, GMT_MSG_VERBOSE, "Found %d indices triplets exceeding range of known vertices - skipped.\n", n_skipped);
	}
	else {	/* Do our own Delaunay triangulation */
		np = GMT_delaunay (GMT, x, y, n, &ind);
		GMT_Report (API, GMT_MSG_VERBOSE, "Obtained %d indices triplets via Delauney triangulation [%s].\n", np, tri_algorithm[GMT->current.setting.triangulate]);
	}

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	/* Determine if some triangles are outside the region and should be removed entirely */
	
	if (skip_triangles) {	/* Must check if triangles are outside plot region */
		for (k = i = n_skipped = 0; i < np; i++) {	/* For all triangles */
			k2 = (unsigned int)k;
			for (k3 = n_out = 0; k3 < 3; k3++, k++) {
				if (GMT_cart_outside (GMT, x[ind[k]], y[ind[k]])) n_out++;	/* Count how many vertices are outside */
			}
			if (n_out == 3) {
				ind[k2] = -1;	/* Flag so no longer to be used */
				n_skipped++;
			}
		}
		if (n_skipped) GMT_Report (API, GMT_MSG_VERBOSE, "Skipped %u triangles whose verticies are all outside the domain.\n", n_skipped);
	}
	
	if (Ctrl->C.cpt) {	/* We already read the cpt-file */
		/* Set up which contours to draw based on the CPT slices and their attributes */
		cont = GMT_memory (GMT, NULL, P->n_colors + 1, struct PSCONTOUR);
		for (i = c = 0; i < P->n_colors; i++) {
			if (P->range[i].skip) continue;
			cont[c].val = P->range[i].z_low;
			if (Ctrl->A.mode)
				cont[c].type = 'C';
			else if (P->range[i].annot)
				cont[c].type = 'A';
			else
				cont[c].type = (Ctrl->contour.annot) ? 'A' : 'C';
			cont[c].type = (P->range[i].annot && !Ctrl->A.mode) ? 'A' : 'C';
			cont[c].angle = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
			cont[c].do_tick = Ctrl->T.active;
			GMT_Report (API, GMT_MSG_DEBUG, "Contour slice %d: Value = %g type = %c angle = %g\n", c, cont[c].val, cont[c].type, cont[c].angle);
			c++;
		}
		cont[c].val = P->range[P->n_colors-1].z_high;
		if (Ctrl->A.mode)
			cont[c].type = 'C';
		else if (P->range[P->n_colors-1].annot & 2)
			cont[c].type = 'A';
		else
			cont[c].type = (Ctrl->contour.annot) ? 'A' : 'C';
		cont[c].angle = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
		cont[c].do_tick = Ctrl->T.active;
		GMT_Report (API, GMT_MSG_DEBUG, "Contour slice %d: Value = %g type = %c angle = %g\n", c, cont[c].val, cont[c].type, cont[c].angle);
		n_contours = c + 1;
	}
	else if (Ctrl->C.file) {	/* read contour info from file with cval C|A [angle] records */
		char *record = NULL;
		int got, in_ID, NL;
		double tmp;

		if ((in_ID = GMT_Register_IO (API, GMT_IS_TEXTSET, GMT_IS_FILE, GMT_IS_NONE, GMT_IN, NULL, Ctrl->C.file)) == GMT_NOTSET) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error registering contour info file %s\n", Ctrl->C.file);
			Return (EXIT_FAILURE);
		}
		if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables text input and sets access mode */
			Return (API->error);
		}
		c = 0;
		do {	/* Keep returning records until we reach EOF */
			if ((record = GMT_Get_Record (API, GMT_READ_TEXT, &NL)) == NULL) {	/* Read next record, get NULL if special case */
				if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (GMT_REC_IS_ANY_HEADER (GMT)) 	/* Skip all table and segment headers */
					continue;
				if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
					break;
			}
			if (GMT_is_a_blank_line (record)) continue;	/* Nothing in this record */

			/* Data record to process */

			if (c == c_alloc) cont = GMT_malloc (GMT, cont, c, &c_alloc, struct PSCONTOUR);
			GMT_memset (&cont[c], 1, struct PSCONTOUR);
			got = sscanf (record, "%lf %c %lf", &cont[c].val, &cont[c].type, &tmp);
			if (cont[c].type == '\0') cont[c].type = 'C';
			cont[c].do_tick = (Ctrl->T.active && ((cont[c].type == 'C') || (cont[c].type == 'A'))) ? true : false;
			cont[c].angle = (got == 3) ? tmp : GMT->session.d_NaN;
			if (got == 3) Ctrl->contour.angle_type = 2;	/* Must set this directly if angles are provided */
			cont[c].do_tick = Ctrl->T.active;
			c++;
		} while (true);
		
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
		n_contours = c;
	}
	else if (!GMT_is_dnan (Ctrl->C.single_cont) || !GMT_is_dnan (Ctrl->A.single_cont)) {	/* Plot one or two contours only */
		n_contours = 0;
		cont = GMT_malloc (GMT, cont, 2, &c_alloc, struct PSCONTOUR);
		if (!GMT_is_dnan (Ctrl->C.single_cont)) {
			cont[n_contours].type = 'C';
			cont[n_contours++].val = Ctrl->C.single_cont;
		}
		if (!GMT_is_dnan (Ctrl->A.single_cont)) {
			cont[n_contours].type = 'A';
			cont[n_contours].val = Ctrl->A.single_cont;
			cont[n_contours].do_tick = Ctrl->T.active;
			cont[n_contours].angle = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
			n_contours++;
		}
	}
	else {	/* Set up contour intervals automatically from Ctrl->C.interval and Ctrl->A.interval */
		int ic;
		double min, max, aval;
		min = floor (xyz[0][GMT_Z] / Ctrl->C.interval) * Ctrl->C.interval; if (min < xyz[0][GMT_Z]) min += Ctrl->C.interval;
		max = ceil (xyz[1][GMT_Z] / Ctrl->C.interval) * Ctrl->C.interval; if (max > xyz[1][GMT_Z]) max -= Ctrl->C.interval;
		if (Ctrl->contour.annot) {	/* Want annotated contours */
			/* Determine the first annotated contour level */
			aval = floor (xyz[0][GMT_Z] / Ctrl->A.interval) * Ctrl->A.interval;
			if (aval < xyz[0][GMT_Z]) aval += Ctrl->A.interval;
		}
		else	/* No annotations, set aval outside range */
			aval = xyz[1][GMT_Z] + 1.0;
		for (ic = irint (min/Ctrl->C.interval), c = 0; ic <= irint (max/Ctrl->C.interval); ic++, c++) {
			if (c == c_alloc) cont = GMT_malloc (GMT, cont, c, &c_alloc, struct PSCONTOUR);
			GMT_memset (&cont[c], 1, struct PSCONTOUR);
			cont[c].val = ic * Ctrl->C.interval;
			if (Ctrl->contour.annot && (cont[c].val - aval) > GMT_CONV4_LIMIT) aval += Ctrl->A.interval;
			cont[c].type = (fabs (cont[c].val - aval) < GMT_CONV4_LIMIT) ? 'A' : 'C';
			cont[c].angle = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
			cont[c].do_tick = Ctrl->T.active;
		}
		n_contours = c;
	}
	c_alloc = n_contours;
	cont = GMT_malloc (GMT, cont, 0, &c_alloc, struct PSCONTOUR);

	if (Ctrl->D.active) {
		uint64_t dim[4] = {0, 0, 0, 3};
		if (!Ctrl->D.file[0] || !strchr (Ctrl->D.file, '%')) {	/* No file given or filename without C-format specifiers means a single output file */
			io_mode = GMT_WRITE_SET;
			n_tables = 1;
		}
		else {	/* Must determine the kind of output organization */
			i = 0;
			while (Ctrl->D.file[i]) {
				if (Ctrl->D.file[i++] == '%') {	/* Start of format */
					while (Ctrl->D.file[i] && !strchr ("cdf", Ctrl->D.file[i])) i++;	/* Scan past any format modifiers, like in %7.4f */
					if (Ctrl->D.file[i] == 'c') fmt[0] = i;
					if (Ctrl->D.file[i] == 'd') fmt[1] = i;
					if (Ctrl->D.file[i] == 'f') fmt[2] = i;
					i++;
				}
			}
			n_tables = 1;
			if (fmt[2]) {	/* Want files with the contour level in the name */
				if (fmt[1])	/* f+d[+c]: Write segment files named after contour level and running numbers, with or without C|O modifier */
					io_mode = GMT_WRITE_SEGMENT;
				else {	/* f[+c]: Write one table with all segments for each contour level, possibly one each for C|O */	
					io_mode = GMT_WRITE_TABLE;
					tbl_scl = (fmt[0]) ? 2 : 1;
					n_tables = n_contours * tbl_scl;
				}
			}
			else if (fmt[1])	/* d[+c]: Want individual files with running numbers only, with or without C|O modifier  */
				io_mode = GMT_WRITE_SEGMENT;
			else if (fmt[0]) {	/* c: Want two files: one for open and one for closed contours */
				io_mode = GMT_WRITE_TABLE;
				n_tables = 2;
				two_only = true;
			}
		}
		GMT_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
		dim[GMT_TBL] = n_tables;
		if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, Ctrl->D.file)) == NULL) Return (API->error);	/* An empty dataset */
		n_seg_alloc = GMT_memory (GMT, NULL, n_tables, size_t);
		n_seg = GMT_memory (GMT, NULL, n_tables, uint64_t);
		if ((error = GMT_set_cols (GMT, GMT_OUT, 3))) Return (error);
	}
	
	if (make_plot) {
		if (Ctrl->contour.delay) GMT->current.ps.nclip = (Ctrl->N.active) ? +1 : +2;	/* Signal that this program initiates clipping that will outlive this process */
		PSL = GMT_plotinit (GMT, options);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT_plotcanvas (GMT);	/* Fill canvas if requested */
		if (Ctrl->contour.delay) GMT_map_basemap (GMT);	/* If delayed clipping the basemap must be done before clipping */
		if (!Ctrl->N.active) GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);
		Ctrl->contour.line_pen = Ctrl->W.pen[0];
	}

	if (Ctrl->L.active) {	/* Draw triangular mesh */

		GMT_setpen (GMT, &Ctrl->L.pen);

		for (k = i = 0; i < np; i++) {	/* For all triangles */
			if (ind[k] < 0) { k += 3; continue;}	/* Skip triangles that are fully outside */

			xx[0] = x[ind[k]];	yy[0] = y[ind[k++]];
			xx[1] = x[ind[k]];	yy[1] = y[ind[k++]];
			xx[2] = x[ind[k]];	yy[2] = y[ind[k++]];

			PSL_plotline (PSL, xx, yy, 3, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
		}
	}

	/* Get PSCONTOUR structs */

	if (get_contours) {
		for (i = 0; i < n_contours; i++) {
			cont[i].n_alloc = GMT_SMALL_CHUNK;
			cont[i].L = GMT_memory (GMT, NULL, GMT_SMALL_CHUNK, struct PSCONTOUR_LINE);
		}
	}

	z_range = xyz[1][GMT_Z] - xyz[0][GMT_Z];
	small = MIN (Ctrl->C.interval, z_range) * 1.0e-6;	/* Our float noise threshold */

	for (ij = i = 0; i < np; i++, ij += 3) {	/* For all triangles */

		if (ind[ij] < 0) continue;	/* Skip triangles that are fully outside */

		k = ij;
		xx[0] = x[ind[k]];	yy[0] = y[ind[k]];	zz[0] = z[ind[k++]];
		xx[1] = x[ind[k]];	yy[1] = y[ind[k]];	zz[1] = z[ind[k++]];
		xx[2] = x[ind[k]];	yy[2] = y[ind[k]];	zz[2] = z[ind[k]];

		nx = get_triangle_crossings (GMT, cont, n_contours, x, y, z, &ind[ij], small, &xc, &yc, &zc, &vert, &cind);

		if (Ctrl->I.active) {	/* Must color the triangle slices according to cpt file */

			if (nx == 0) {	/* No contours go through - easy, but must check for NaNs */
				int kzz;
				double zzz;
				for (k = kzz = 0, zzz = 0.0; k < 3; k++) {
					if (GMT_is_dnan (zz[k])) continue;
					zzz += zz[k];
					kzz++;
				}
				if (kzz) paint_it_pscontour (GMT, PSL, P, xx, yy, 3, zzz / kzz);
			}
			else {	/* Must paint all those slices separately */

				/* Find vertices with the lowest and highest values */

				for (k = 1, low = high = 0; k < 3; k++) {
					if (zz[k] < zz[low])   low = (unsigned int)k;
					if (zz[k] > zz[high]) high = (unsigned int)k;
				}

				/* Paint the piece delimited by the low node and the first contour */

				xout[0] = xx[low];	yout[0] = yy[low];
				node1 = get_node_index (vert[0], vert[1]);	/* Find single vertex opposing this contour segment */
				if (node1 == low) {	/* Contour and low node make up a triangle */
					xout[1] = xc[0];	yout[1] = yc[0];
					xout[2] = xc[1];	yout[2] = yc[1];
					m = 3;
				}
				else {	/* Need the other two vertices to form a 4-sided polygon */
					node2 = get_other_node (node1, low);	/* The other node needed */
					xout[1] = xx[node2];	yout[1] = yy[node2];
					if (low == vert[0] || node2 == vert[1]) {	/* Add segment in opposite order */
						xout[2] = xc[1];	yout[2] = yc[1];
						xout[3] = xc[0];	yout[3] = yc[0];
					}
					else  {	/* Add in regular order */
						xout[2] = xc[0];	yout[2] = yc[0];
						xout[3] = xc[1];	yout[3] = yc[1];
					}
					m = 4;
				}
				paint_it_pscontour (GMT, PSL, P, xout, yout, m, 0.5 * (zz[low] + zc[1]));	/* z is contour value */

				/* Then loop over contours and paint the part between contours */

				for (k = 1, k2 = 2, k3 = 3; k < nx; k++, k2 += 2, k3 += 2) {
					xout[0] = xc[k2-2];	yout[0] = yc[k2-2];
					xout[1] = xc[k3-2];	yout[1] = yc[k3-2];
					m = 2;
					last_entry = vert[k2-2];
					last_exit  = vert[k3-2];
					if (last_exit == vert[k2]) {
						xout[m] = xc[k2];	yout[m] = yc[k2];	m++;
						xout[m] = xc[k3];	yout[m] = yc[k3];	m++;
						if (vert[k3] != last_entry) {	/* Need to add an intervening corner */
							node1 = get_node_index (last_entry, vert[k3]);	/* Find corner id */
							xout[m] = xx[node1];	yout[m] = yy[node1];	m++;
						}
					}
					else if (last_exit == vert[k3]) {
						xout[m] = xc[k3];	yout[m] = yc[k3];	m++;
						xout[m] = xc[k2];	yout[m] = yc[k2];	m++;
						if (vert[k2] != last_entry) {	/* Need to add an intervening corner */
							node1 = get_node_index (last_entry, vert[k2]);	/* Find corner id */
							xout[m] = xx[node1];	yout[m] = yy[node1];	m++;
						}
					}
					else if (last_entry == vert[k2]) {
						node1 = get_node_index (last_exit, vert[k3]);	/* Find corner id */
						xout[m] = xx[node1];	yout[m] = yy[node1];	m++;
						xout[m] = xc[k3];	yout[m] = yc[k3];	m++;
						xout[m] = xc[k2];	yout[m] = yc[k2];	m++;
					}
					else {
						node1 = get_node_index (last_exit, vert[k2]);	/* Find corner id */
						xout[m] = xx[node1];	yout[m] = yy[node1];	m++;
						xout[m] = xc[k2];	yout[m] = yc[k2];	m++;
						xout[m] = xc[k3];	yout[m] = yc[k3];	m++;
					}
					paint_it_pscontour (GMT, PSL, P, xout, yout, m, 0.5 * (zc[k2]+zc[k2-2]));
				}

				/* Add the last piece between last contour and high node */

				k2 -= 2;	k3 -= 2;
				xout[0] = xx[high];	yout[0] = yy[high];
				node1 = get_node_index (vert[k2], vert[k3]);	/* Find corner id */
				if (node1 == high) {	/* Cut off a triangular piece */
					xout[1] = xc[k2];	yout[1] = yc[k2];
					xout[2] = xc[k3];	yout[2] = yc[k3];
					m = 3;
				}
				else {	/* Need a 4-sided polygon */
					node2 = get_other_node (node1, high);	/* The other node needed */
					xout[1] = xx[node2];	yout[1] = yy[node2];
					if (high == vert[0] || node2 == vert[1]) {	/* On same side, start here */
						xout[2] = xc[k3];	yout[2] = yc[k3];
						xout[3] = xc[k2];	yout[3] = yc[k2];
					}
					else  {	/* On same side, start here */
						xout[2] = xc[k2];	yout[2] = yc[k2];
						xout[3] = xc[k3];	yout[3] = yc[k3];
					}
					m = 4;
				}
				paint_it_pscontour (GMT, PSL, P, xout, yout, m, 0.5 * (zz[high] + zc[k2]));	/* z is contour value */
			}
		}

		if (get_contours && nx > 0) {	/* Save contour line segments L for later */
			for (k = k2 = 0; k < nx; k++) {
				c = cind[k];
				m = cont[c].nl;
				cont[c].L[m].x0 = xc[k2];
				cont[c].L[m].y0 = yc[k2++];
				cont[c].L[m].x1 = xc[k2];
				cont[c].L[m].y1 = yc[k2++];
				m++;
				if (m >= cont[c].n_alloc) {
					cont[c].n_alloc <<= 1;
					cont[c].L = GMT_memory (GMT, cont[c].L, cont[c].n_alloc, struct PSCONTOUR_LINE);
				}
				cont[c].nl = m;
			}
		}

		if (nx > 0) {
			GMT_free (GMT, xc);
			GMT_free (GMT, yc);
			GMT_free (GMT, zc);
			GMT_free (GMT, vert);
			GMT_free (GMT, cind);
		}
	}
	
	/* Draw or dump contours */

	if (get_contours) {
		bool use_it;
		struct PSCONTOUR_CHAIN *head_c = NULL, *last_c = NULL, *this_c = NULL;
		struct PSCONTOUR_PT *p = NULL, *q = NULL;
				
		if (Ctrl->contour.half_width == 5) Ctrl->contour.half_width = 0;	/* Since contours are straight line segments we override the default */

		for (c = 0; c < n_contours; c++) {	/* For all selected contour levels */

			if (cont[c].nl == 0) {	/* No contours at this level */
				GMT_free (GMT, cont[c].L);
				continue;
			}

			GMT_Report (API, GMT_MSG_VERBOSE, "Tracing the %g contour\n", cont[c].val);

			id = (cont[c].type == 'A' || cont[c].type == 'a') ? 1 : 0;

			Ctrl->contour.line_pen = Ctrl->W.pen[id];	/* Load current pen into contour structure */
			if (Ctrl->W.color_cont) {	/* Override pen color according to cpt file */
				GMT_get_rgb_from_z (GMT, P, cont[c].val, rgb);
				GMT_rgb_copy (&Ctrl->contour.line_pen.rgb, rgb);
			}
			if (Ctrl->W.color_text && Ctrl->contour.curved_text) {	/* Override label color according to cpt file */
				GMT_rgb_copy (&Ctrl->contour.font_label.fill.rgb, rgb);
			}
			
			head_c = last_c = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_CHAIN);

			while (cont[c].nl) {	/* Still more line segments at this contour level */
				/* Must hook all the segments into continuous contours. Start with first segment L */
				this_c = last_c->next = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_CHAIN);
				k = 0;
				this_c->begin = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_PT);
				this_c->end = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_PT);
				this_c->begin->x = cont[c].L[k].x0;
				this_c->begin->y = cont[c].L[k].y0;
				this_c->end->x = cont[c].L[k].x1;
				this_c->end->y = cont[c].L[k].y1;
				this_c->begin->next = this_c->end;
				cont[c].nl--;	/* Used one segment now */
				cont[c].L[k] = cont[c].L[cont[c].nl];
				while (k < cont[c].nl) {	/* As long as there are more */
					add = 0;
					if (fabs(cont[c].L[k].x0 - this_c->begin->x) < GMT_CONV4_LIMIT && fabs(cont[c].L[k].y0 - this_c->begin->y) < GMT_CONV4_LIMIT) {	/* L matches previous */
						p = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_PT);
						p->x = cont[c].L[k].x1;
						p->y = cont[c].L[k].y1;
						p->next = this_c->begin;
						add = -1;
					}
					else if (fabs(cont[c].L[k].x1 - this_c->begin->x) < GMT_CONV4_LIMIT && fabs(cont[c].L[k].y1 - this_c->begin->y) < GMT_CONV4_LIMIT) {	/* L matches previous */
						p = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_PT);
						p->x = cont[c].L[k].x0;
						p->y = cont[c].L[k].y0;
						p->next = this_c->begin;
						add = -1;
					}
					else if (fabs(cont[c].L[k].x0 - this_c->end->x) < GMT_CONV4_LIMIT && fabs(cont[c].L[k].y0 - this_c->end->y) < GMT_CONV4_LIMIT) {	/* L matches previous */
						p = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_PT);
						p->x = cont[c].L[k].x1;
						p->y = cont[c].L[k].y1;
						this_c->end->next = p;
						add = 1;
					}
					else if (fabs(cont[c].L[k].x1 - this_c->end->x) < GMT_CONV4_LIMIT && fabs(cont[c].L[k].y1 - this_c->end->y) < GMT_CONV4_LIMIT) {	/* L matches previous */
						p = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_PT);
						p->x = cont[c].L[k].x0;
						p->y = cont[c].L[k].y0;
						this_c->end->next = p;
						add = 1;
					}
					if (add) {	/* Got one, check if we must reverse it */
						if (add == -1)
							this_c->begin = p;
						else if (add == 1)
							this_c->end = p;
						cont[c].nl--;
						cont[c].L[k] = cont[c].L[cont[c].nl];
						k = 0;
					}
					else	/* No match, go to next */
						k++;
				}
				last_c = this_c;
			}
			GMT_free (GMT, cont[c].L);	/* Done with temporary contour segment array for this level */

			/* Now, turn this linked list of segment pointers into x,y arrays */
			
			this_c = head_c->next;
			while (this_c) {
				p = this_c->begin;
				m = 1;
				while ((p = p->next)) m++;	/* Find total number of points in this linked list */
				use_it = (m >= Ctrl->Q.min);	/* True if we will use this line */

				if (use_it) {	/* Need arrays to hold the coordinates */
					xp = GMT_memory (GMT, NULL, m, double);
					yp = GMT_memory (GMT, NULL, m, double);
					m = 0;
				}
				p = this_c->begin;	/* Start over and maybe get the points this time */
				while ((q = p)) {	/* More points to add */
					if (use_it) {xp[m] = p->x; yp[m++] = p->y;}
					p = p->next;
					GMT_free (GMT, q);	/* Free linked list as we go along */
				}
				last_c = this_c;
				this_c = this_c->next;
				GMT_free (GMT, last_c);	/* Free last item */
				if (!use_it) continue;	/* No, go to next */
				
				is_closed = !GMT_polygon_is_open (GMT, xp, yp, m);

				if (current_contour != cont[c].val) {
					if (make_plot) {
						if (Ctrl->W.color_cont) {	/* Override pen color according to cpt file */
							GMT_get_rgb_from_z (GMT, P, cont[c].val, rgb);
							PSL_setcolor (PSL, rgb, PSL_IS_STROKE);
							GMT_rgb_copy (&Ctrl->contour.line_pen.rgb, rgb);
						}
						if (Ctrl->W.color_text && Ctrl->contour.curved_text) {	/* Override label color according to cpt file */
							GMT_rgb_copy (&Ctrl->contour.font_label.fill.rgb, rgb);
						}
					}
					current_contour = cont[c].val;
				}

				if (make_plot && (cont[c].type == 'A' || cont[c].type == 'a')) {	/* Annotated contours */
					GMT_get_format (GMT, cont[c].val, Ctrl->contour.unit, NULL, format);
					sprintf (cont_label, format, cont[c].val);
				}
				if (Ctrl->D.active) {
					size_t count;
					unsigned int closed;
					if (convert) {	/* Must first apply inverse map transform */
						double *xtmp = NULL, *ytmp = NULL;
						xtmp = GMT_memory (GMT, NULL, m, double);
						ytmp = GMT_memory (GMT, NULL, m, double);
						for (count = 0; count < m; count++) GMT_xy_to_geo (GMT, &xtmp[count], &ytmp[count], xp[count], yp[count]);
						S = GMT_prepare_contour (GMT, xtmp, ytmp, m, cont[c].val);
						GMT_free (GMT, xtmp);
						GMT_free (GMT, ytmp);
					}
					else
						S = GMT_prepare_contour (GMT, xp, yp, m, cont[c].val);
					/* Select which table this segment should be added to */
					closed = (is_closed) ? 1 : 0;
					tbl = (io_mode == GMT_WRITE_TABLE) ? ((two_only) ? closed : tbl_scl * c) : 0;
					if (n_seg[tbl] == n_seg_alloc[tbl]) {
						size_t n_old_alloc = n_seg_alloc[tbl];
						D->table[tbl]->segment = GMT_memory (GMT, D->table[tbl]->segment, (n_seg_alloc[tbl] += GMT_SMALL_CHUNK), struct GMT_DATASEGMENT *);
						GMT_memset (&(D->table[tbl]->segment[n_old_alloc]), n_seg_alloc[tbl] - n_old_alloc, struct GMT_DATASEGMENT *);	/* Set to NULL */
					}
					D->table[tbl]->segment[n_seg[tbl]++] = S;
					D->table[tbl]->n_segments++;	D->n_segments++;
					D->table[tbl]->n_records += m;	D->n_records += m;
					/* Generate a file name and increment cont_counts, if relevant */
					if (io_mode == GMT_WRITE_TABLE && !D->table[tbl]->file[GMT_OUT])
						D->table[tbl]->file[GMT_OUT] = GMT_make_filename (GMT, Ctrl->D.file, fmt, cont[c].val, is_closed, cont_counts);
					else if (io_mode == GMT_WRITE_SEGMENT)
						S->file[GMT_OUT] = GMT_make_filename (GMT, Ctrl->D.file, fmt, cont[c].val, is_closed, cont_counts);
				}

				if (make_plot) {
					if (cont[c].do_tick && is_closed) {	/* Must store the entire contour for later processing */
						if (n_save == n_save_alloc) save = GMT_malloc (GMT, save, n_save, &n_save_alloc, struct SAVE);
						n_alloc = 0;
						GMT_memset (&save[n_save], 1, struct SAVE);
						GMT_malloc2 (GMT, save[n_save].x, save[n_save].y, m, &n_alloc, double);
						GMT_memcpy (save[n_save].x, xp, m, double);
						GMT_memcpy (save[n_save].y, yp, m, double);
						save[n_save].n = m;
						GMT_memcpy (&save[n_save].pen, &Ctrl->W.pen[id], 1, struct GMT_PEN);
						save[n_save].do_it = true;
						save[n_save].cval = cont[c].val;
						n_save++;
					}
					GMT_hold_contour (GMT, &xp, &yp, m, cont[c].val, cont_label, cont[c].type, cont[c].angle, is_closed, true, &Ctrl->contour);
				}

				GMT_free (GMT, xp);
				GMT_free (GMT, yp);
			}
			GMT_free (GMT, head_c);
		}
		if (make_plot) label_mode |= 1;		/* Would want to plot ticks and labels if -T is set */
		if (Ctrl->contour.save_labels) {	/* Want to save the contour label locations (lon, lat, angle, label) */
			label_mode |= 2;
			if (Ctrl->contour.save_labels == 2) label_mode |= 4;
			if ((error = GMT_contlabel_save_begin (GMT, &Ctrl->contour))) Return (error);
		}
		if (Ctrl->T.active && n_save) {	/* Finally sort and plot ticked innermost contoursand plot/save L|H labels */
			size_t kk;
			save = GMT_malloc (GMT, save, 0, &n_save, struct SAVE);

			sort_and_plot_ticks (GMT, PSL, save, n_save, x, y, z, n, Ctrl->T.spacing, Ctrl->T.length, Ctrl->T.low, Ctrl->T.high, Ctrl->T.label, Ctrl->T.txt, label_mode, Ctrl->contour.fp);
			for (kk = 0; kk < n_save; kk++) {
				GMT_free (GMT, save[kk].x);
				GMT_free (GMT, save[kk].y);
			}
			GMT_free (GMT, save);
		}
		if (make_plot) {
			GMT_contlabel_plot (GMT, &Ctrl->contour);
			GMT_contlabel_free (GMT, &Ctrl->contour);
		}
		if (Ctrl->contour.save_labels) {	/* Close file with the contour label locations (lon, lat, angle, label) */
			if ((error = GMT_contlabel_save_end (GMT, &Ctrl->contour))) Return (error);
		}
	}

	if (Ctrl->D.active) {	/* Write the contour line output file(s) */
		for (tbl = 0; tbl < D->n_tables; tbl++) D->table[tbl]->segment = GMT_memory (GMT, D->table[tbl]->segment, n_seg[tbl], struct GMT_DATASEGMENT *);
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, io_mode, NULL, Ctrl->D.file, D) != GMT_OK) {
			Return (API->error);
		}
		GMT_free (GMT, n_seg_alloc);
		GMT_free (GMT, n_seg);
	}

	if (make_plot) {
		if (!(Ctrl->N.active || Ctrl->contour.delay)) GMT_map_clip_off (GMT);
		if (!Ctrl->contour.delay) GMT_map_basemap (GMT);	/* If delayed clipping the basemap is done before plotting, else here */
		GMT_plane_perspective (GMT, -1, 0.0);
		GMT_plotend (GMT);
	}

	GMT_free (GMT, x);
	GMT_free (GMT, y);
	GMT_free (GMT, z);
	GMT_free (GMT, cont);
	if (Ctrl->E.active)
		GMT_free (GMT, ind);	/* Allocated above by GMT_memory */
	else
		GMT_delaunay_free (GMT, &ind);

	Return (GMT_OK);
}
