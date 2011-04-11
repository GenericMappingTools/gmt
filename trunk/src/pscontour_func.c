/*--------------------------------------------------------------------
 *	$Id: pscontour_func.c,v 1.9 2011-04-11 21:15:31 remko Exp $
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
/* Brief synopsis: pscontour will read a file of points in the plane, performs the
 * Delaunay triangulation, and contours these triangles.  As an option
 * the user may provide a file with indices of which vertices constitute
 * the triangles.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#include "pslib.h"
#include "gmt.h"

struct PSCONTOUR_CTRL {
	struct GMT_CONTOUR contour;
	struct A {	/* -A[-|<aint>][+a<angle>][+c<dx>[/<dy>]][+f<font>][+g<fill>][+j<just>][+l<label>][+o|O|t][+s<size>][+p<pen>][+u<unit>] */
		GMT_LONG active;
		GMT_LONG mode;	/* 1 turns off all labels */
		double interval;
	} A;
	struct C {	/* -C<cpt> */
		GMT_LONG active;
		GMT_LONG cpt;
		char *file;
		double interval;
	} C;
	struct D {	/* -D<dumpfile> */
		GMT_LONG active;
		char *file;
	} D;
	struct G {	/* -G[d|f|n|l|L|x|X]<params> */
		GMT_LONG active;
	} G;
	struct I {	/* -I */
		GMT_LONG active;
	} I;
	struct L {	/* -L<pen> */
		GMT_LONG active;
		struct GMT_PEN pen;
	} L;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct T {	/* -T[+|-][<gap>[c|i|p]/<length>[c|i|p]][:LH] */
		GMT_LONG active;
		GMT_LONG label;
		GMT_LONG low, high;	/* TRUE to tick low and high locals */
		double spacing, length;
		char *txt[2];	/* Low and high label */
	} T;
	struct Q {	/* -Q<indexfile> */
		GMT_LONG active;
		char *file;
	} Q;
	struct W {	/* -W[+|-]<type><pen> */
		GMT_LONG active;
		GMT_LONG color_cont;
		GMT_LONG color_text;
		struct GMT_PEN pen[2];
	} W;
};

#define GRDCONTOUR_MIN_LENGTH 0.01	/* Contours shorter than this are skipped */
#define TICKED_SPACING	15.0		/* Spacing between ticked contour ticks (in points) */
#define TICKED_LENGTH	3.0		/* Length of ticked contour ticks (in points) */

struct SAVE {
	double *x, *y;
	double cval;
	GMT_LONG n;
	struct GMT_PEN pen;
	GMT_LONG do_it, high;
};

#ifdef TRIANGLE_D
#define ALGORITHM "Shewchuk"
#else
#define ALGORITHM "Watson"
#endif

/* Returns the id of the node common to the two edges */
#define get_node_index(edge_1, edge_2) (((PSCONTOUR_SUM = (edge_1) + (edge_2)) == 1) ? 1 : ((PSCONTOUR_SUM == 2) ? 0 : 2))
#define get_other_node(node1, node2) (((PSCONTOUR_SUM = (node1 + node2)) == 3) ? 0 : ((PSCONTOUR_SUM == 2) ? 1 : 2))	/* The other node needed */

struct PSCONTOUR_LINE {	/* Beginning and end of straight contour segment */
	double x0, y0;
	double x1, y1;
};

struct PSCONTOUR {
	GMT_LONG n_alloc, nl;
	double val;
	double angle;
	char type, do_tick;
	struct PSCONTOUR_LINE *L;
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

EXTERN_MSC GMT_LONG GMT_delaunay (struct GMT_CTRL *C, double *x_in, double *y_in, GMT_LONG n, int **link);

void *New_pscontour_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSCONTOUR_CTRL *C;
	
	C = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_CTRL);
	
	/* Initialize values whose defaults are not 0/FALSE/NULL */
	GMT_contlabel_init (GMT, &C->contour, 1);
	C->D.file = strdup ("contour");
	C->L.pen = GMT->current.setting.map_default_pen;
	C->T.spacing = TICKED_SPACING * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 14p */
	C->T.length  = TICKED_LENGTH  * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 3p */
	C->T.txt[0] = strdup ("-");	/* Default low label */
	C->T.txt[1] = strdup ("+");	/* Default high label */
	C->W.pen[0] = C->W.pen[1] = GMT->current.setting.map_default_pen;
	C->W.pen[1].width *= 3.0;

	return ((void *)C);
}

void Free_pscontour_Ctrl (struct GMT_CTRL *GMT, struct PSCONTOUR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->C.file) free ((void *)C->C.file);	
	if (C->D.file) free ((void *)C->D.file);	
	if (C->Q.file) free ((void *)C->Q.file);	
	if (C->T.txt[0]) free ((void *)C->T.txt[0]);	
	if (C->T.txt[1]) free ((void *)C->T.txt[1]);	
	GMT_free (GMT, C);	
}

GMT_LONG get_triangle_crossings (struct GMT_CTRL *GMT, struct PSCONTOUR *P, GMT_LONG n_conts, double *x, double *y, double *z, int *ind, double small, double **xc, double **yc, double **zc, GMT_LONG **v, GMT_LONG **cindex)
{
	/* This routine finds all the contour crossings for this triangle.  Each contour consists of
	 * linesegments made up of two points, with coordinates xc, yc, and contour level zc.
	 */
	 
	GMT_LONG i, j, k, k2, i1, nx, n_alloc, ok, n_ok, *vout = NULL, *cind = NULL, *ctmp = NULL;
	double xx[3], yy[3], zz[3], zmin, zmax, dz, frac, *xout = NULL, *yout = NULL, *zout = NULL, *ztmp = NULL;

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
	vout = GMT_memory (GMT, NULL, n_alloc, GMT_LONG);
	ctmp = GMT_memory (GMT, NULL, nx, GMT_LONG);
	cind = GMT_memory (GMT, NULL, nx, GMT_LONG);

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
		ok = FALSE;
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
				ok = TRUE;	/* Wish to add this segment */
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

void paint_it_pscontour (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct GMT_PALETTE *P, double x[], double y[], GMT_LONG n, double z) {
	GMT_LONG index;
	double rgb[4];
	struct GMT_FILL *f = NULL;

	if (n < 3) return;	/* Need at least 3 points to make a polygon */

	index = GMT_get_rgb_from_z (GMT, P, z, rgb);
	if (P->skip) return;	/* Skip this z-slice */

	/* Now we must paint, with colors or patterns */

	if ((index >= 0 && (f = P->range[index].fill)) || (index < 0 && (f = P->patch[index+3].fill)))
		GMT_setfill (GMT, PSL, f, FALSE);
	else
		PSL_setfill (PSL, rgb, -2);
	/* Contours drawn separately later if desired */
	PSL_plotpolygon (PSL, x, y, n);
}

void sort_and_plot_ticks (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct SAVE *save, GMT_LONG n, double *x, double *y, double *z, GMT_LONG nn, double tick_gap, double tick_length, GMT_LONG tick_low, GMT_LONG tick_high, GMT_LONG tick_label, char *lbl[])
{	/* Labeling and ticking of inner-most contours cannot happen until all contours are found and we can determine
	which are the innermost ones. */
	GMT_LONG np, i, j, k, inside, n_ticks, way, form;
	double add, dx, dy, x_back, y_back, x_end, y_end, sa, ca, s;
	double x_mean, y_mean, a, xmin, xmax, ymin, ymax, length;

	/* The x/y coordinates in SAVE are now all projected to map inches */

	for (i = 0; i < n; i++) {	/* Mark polygons that have other polygons inside them */
		np = save[i].n;
		for (j = 0; save[i].do_it && j < n; j++) {
			inside = GMT_non_zero_winding (GMT, save[j].x[0], save[j].y[0], save[i].x, save[i].y, np);
			if (inside == 2) save[i].do_it = FALSE;
		}
	}

	form = GMT_setfont (GMT, PSL, &GMT->current.setting.font_annot[0]);

	/* Here, only the polygons that are innermost (containing the local max/min, will have do_it = TRUE */

	for (i = 0; i < n; i++) {
		if (!save[i].do_it) continue;
		np = save[i].n;

		/* Here we need to figure out if this is a local high or low. */

		/* First determine the bounding box for this contour */
		xmin = xmax = save[i].x[0];	ymin = ymax = save[i].y[0];
		for (j = 1; j < np; j++) {
			xmin = MIN (xmin, save[i].x[j]);
			xmax = MAX (xmax, save[i].x[j]);
			ymin = MIN (ymin, save[i].y[j]);
			ymax = MAX (ymax, save[i].y[j]);
		}
		
		/* Now try to find a data point inside this contour */
		
		for (j = 0, k = -1; k < 0 && j < nn; j++) {
			if (GMT_y_is_outside (y[j], ymin, ymax)) continue;	/* Outside y-range */
			if (GMT_y_is_outside (x[j], xmin, xmax)) continue;	/* Outside x-range (YES, use GMT_y_is_outside since projected x-coordinates)*/
			
			inside = GMT_non_zero_winding (GMT, x[j], y[j], save[i].x, save[i].y, np);
			if (inside == 2) k = j;	/* OK, this point is inside */
		}
		if (k < 0) continue;	/* Unable to determine */
		save[i].high = (z[k] > save[i].cval);

		if (save[i].high && !tick_high) continue;	/* Do not tick highs */
		if (!save[i].high && !tick_low) continue;	/* Do not tick lows */

		for (j = 1, s = 0.0; j < np; j++) {	/* Compute distance along the contour */
			s += hypot (save[i].x[j]-save[i].x[j-1], save[i].y[j]-save[i].y[j-1]);
		}
		if (s < GRDCONTOUR_MIN_LENGTH) continue;	/* Contour is too short to be ticked or labeled */

		n_ticks = (GMT_LONG)(s / tick_gap);
		if (n_ticks == 0) continue;	/* Too short to be ticked or labeled */

		GMT_setpen (GMT, PSL, &save[i].pen);
		way = GMT_polygon_centroid (GMT, save[i].x, save[i].y, np, &x_mean, &y_mean);	/* -1 is CCW, +1 is CW */
		if (tick_label)	/* Compute mean location of closed contour ~hopefully a good point inside to place label. */
			PSL_plottext (PSL, x_mean, y_mean, GMT->current.setting.font_annot[0].size, lbl[save[i].high], 0.0, 6, form);
		add = M_PI_2 * ((save[i].high) ? -way : +way);	/* So that tick points in the right direction */
		for (j = 1; j < np; j++) {	/* Consider each segment from point j-1 to j */
			dx = save[i].x[j] - save[i].x[j-1];
			dy = save[i].y[j] - save[i].y[j-1];
			length = hypot (dx, dy);
			n_ticks = (GMT_LONG)ceil (length / tick_gap);	/* At least one per side */
			a = atan2 (dy, dx) + add;
			sincos (a, &sa, &ca);
			for (k = 0; k <= n_ticks; k++) {
				x_back = save[i].x[j-1] + k * dx / (n_ticks + 1);
				y_back = save[i].y[j-1] + k * dy / (n_ticks + 1);
				x_end = x_back + tick_length * ca;
				y_end = y_back + tick_length * sa;
				PSL_plotsegment (PSL, x_back, y_back, x_end, y_end);
			}
		}
	}
}

GMT_LONG GMT_pscontour_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;
	struct GMT_PEN P;

	GMT_message (GMT, "pscontour %s [API] - Contour xyz-data by triangulation [%s]\n\n", GMT_VERSION, ALGORITHM);
	GMT_message (GMT, "usage: pscontour <xyzfile> -C<cpt_file> %s %s\n", GMT_J_OPT, GMT_Rgeoz_OPT);
	GMT_message (GMT, "\t[-A[-|<annot_int>][<labelinfo>] [%s] [-D<dumpfile>]\n", GMT_B_OPT);
	GMT_message (GMT, "\t[%s] [-I] [%s] [-K] [-L<pen>] [-N] [-O]\n", GMT_CONTG, GMT_Jz_OPT);
	GMT_message (GMT, "\t[-P] [-Q<indexfile>] [-S] [%s]\n", GMT_CONTT);
	GMT_message (GMT, "\t[%s] [-W[+]<pen>] [%s] [%s] [%s]\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_b_OPT, GMT_c_OPT, GMT_h_OPT, GMT_h_OPT, GMT_p_OPT, GMT_t_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t-C Color palette table\n");
	GMT_explain_options (GMT, "jZR");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Annotation label information.\n");
	GMT_message (GMT, "\t   Give A- to disable all contour annotations implied in -C\n");
	GMT_message (GMT, "\t   <labelinfo> controls the specifics of the labels.  Append what you need:\n");
	GMT_label_syntax (GMT, 5, 0);
	GMT_explain_options (GMT, "b");
	GMT_message (GMT, "\t-D Dump contours as data line segments; no plotting takes place.\n");
	GMT_message (GMT, "\t   Append filename template which may contain C-format specifiers.\n");
	GMT_message (GMT, "\t   If no filename template is given we write all lines to stdout.\n");
	GMT_message (GMT, "\t   If filename has no specifiers then we write all lines to a single file.\n");
	GMT_message (GMT, "\t   If a float format (e.g., %%6.2f) is found we substitute the contour z-value.\n");
	GMT_message (GMT, "\t   If an integer format (e.g., %%6.6d) is found we substitute a running segment count.\n");
	GMT_message (GMT, "\t   If an char format (%%c) is found we substitute C or O for closed and open contours.\n");
	GMT_message (GMT, "\t   The 1-3 specifiers may be combined and appear in any order to produce the\n");
	GMT_message (GMT, "\t   the desired number of output files (e.g., just %%c gives two files, just %%f would.\n");
	GMT_message (GMT, "\t   separate segments into one file per contour level, and %%d would write all segments.\n");
	GMT_message (GMT, "\t   to individual files; see manual page for more examples.\n");
	GMT_message (GMT, "\t-G Controls placement of labels along contours.  Choose among five algorithms:\n");
	GMT_cont_syntax (GMT, 3, 0);
	GMT_message (GMT, "\t-I Color triangles using the cpt file\n");
	GMT_explain_options (GMT, "K");
	GMT_pen_syntax (GMT, 'L', "Draws the triangular mesh with the specified pen.");
	GMT_message (GMT, "\t-N Do NOT clip contours/image at the border [Default clips]\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-Q File with triplets of point indices for each triangle\n");
	GMT_message (GMT, "\t   [Default performs the Delaunay triangulation on xyz-data]\n");
	GMT_message (GMT, "\t-S Skip xyz points outside region [Default keeps all]\n");
	GMT_message (GMT, "\t-T Will embellish innermost, closed contours with ticks pointing in\n");
	GMT_message (GMT, "\t   the downward direction.  User may specify to tick only highs\n");
	GMT_message (GMT, "\t   (-T+) or lows (-T-) [-T implies both extrema].\n");
	GMT_message (GMT, "\t   Append spacing/ticklength (with units) to change defaults [%gp/%gp].\n", TICKED_SPACING, TICKED_LENGTH);
	GMT_message (GMT, "\t   Append :LH to plot L and H in the center of closed contours\n");
	GMT_message (GMT, "\t   for local Lows and Highs (e.g, give :-+ to plot - and + signs).\n");
	GMT_explain_options (GMT, "UV");
	GMT_pen_syntax (GMT, 'W', "Sets pen attributes. Append a<pen> for annotated or c<pen> for regular contours [Default]");
	GMT_message (GMT, "\t   The default settings are\n");
	P = GMT->current.setting.map_default_pen;
	GMT_message (GMT, "\t   Contour pen:  %s\n", GMT_putpen (GMT, P));
	P.width *= 3.0;
	GMT_message (GMT, "\t   Annotate pen: %s\n", GMT_putpen (GMT, P));
	GMT_message (GMT, "\t   Prepend + to draw colored contours based on the cpt file\n");
	GMT_message (GMT, "\t   Prepend - to color contours and annotations based on the cpt file\n");
	GMT_explain_options (GMT, "XC3D0chipt:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_pscontour_parse (struct GMTAPI_CTRL *C, struct PSCONTOUR_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pscontour and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG n_errors = 0, k, j, n, id;
	char txt_a[GMT_TEXT_LEN], txt_b[GMT_TEXT_LEN];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'A':
				/* Format: -A[-|<aint>][+a<angle>][+c<dx>[/<dy>]][+f<font>][+g<fill>][+j<just>][+l<label>][+o|O|t][+s<size>][+p<pen>][+u<unit>] */

				Ctrl->A.active = TRUE;
				if (GMT_contlabel_specs (GMT, opt->arg, &Ctrl->contour)) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -A option.  Correct syntax:\n\t-A[-][<aint>][+a<angle>][+c<dx>[/<dy>]][+f<font>][+g[<fill>]][+j<just>][+o][+p[<pen>]][+s<size>][+u<unit>][+v]\n");
					n_errors ++;
				}
				else if (opt->arg[0] == '-')
					Ctrl->A.mode = 1;	/* Turn off all labels */
				else {
					Ctrl->A.interval = atof (opt->arg);
					Ctrl->contour.annot = TRUE;
				}
				break;
			case 'C':	/* CPT table */
				Ctrl->C.active = TRUE;
				if (!GMT_access (GMT, opt->arg, R_OK)) {	/* Gave a readable file */
					Ctrl->C.interval = 1.0;
					Ctrl->C.cpt = (!strncmp (&opt->arg[strlen(opt->arg)-4], ".cpt", (size_t)4)) ? TRUE : FALSE;
					Ctrl->C.file = strdup (opt->arg);
				}
				else
					Ctrl->C.interval = atof (opt->arg);
				break;
			case 'D':	/* Dump contours */
				Ctrl->D.active = TRUE;
				free ((void *)Ctrl->D.file);
				Ctrl->D.file = strdup (opt->arg);
				break;
			case 'G':	/* contour annotation settings */
				Ctrl->G.active = TRUE;
				n_errors += GMT_contlabel_info (GMT, 'G', opt->arg, &Ctrl->contour);
				break;
			case 'I':	/* Image triangles */
				Ctrl->I.active = TRUE;
				break;
			case 'L':	/* Draw triangular mesh lines */
				Ctrl->L.active = TRUE;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->L.pen)) {
					GMT_pen_syntax (GMT, 'L', " ");
					n_errors++;
				}
				break;
			case 'N':	/* Do not clip at boundary */
				Ctrl->N.active = TRUE;
				break;
			case 'Q':	/* Triplet file */
				Ctrl->Q.file = strdup (opt->arg);
				Ctrl->Q.active = TRUE;
				break;
			case 'S':	/* Skip points outside border */
				Ctrl->S.active = TRUE;
				break;
			case 'T':	/* Embellish innermost closed contours */
#ifdef GMT_COMPAT
				if (!GMT_access (GMT, opt->arg, F_OK)) {	/* Must be the old -T<indexfile> option, set to -Q */
					GMT_report (GMT, GMT_MSG_COMPAT, "Warning: Option -T is deprecated; use -Q instead.\n");
					Ctrl->Q.file = strdup (opt->arg);
					Ctrl->Q.active = TRUE;
					break;
				}
#endif
				Ctrl->T.active = Ctrl->T.high = Ctrl->T.low = TRUE;	/* Default if just -T is given */
				if (opt->arg[0]) {	/* But here we gave more options */
					if (opt->arg[0] == '+')			/* Only tick local highs */
						Ctrl->T.low = FALSE, j = 1;
					else if (opt->arg[0] == '-')		/* Only tick local lows */
						Ctrl->T.high = FALSE, j = 1;
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
					if (opt->arg[j] == ':') Ctrl->T.label = TRUE, j++;
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
							GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -T option.  Give low and high labels either as -:LH or -:<low>,<high>.\n");
							Ctrl->T.label = FALSE;
							n_errors++;
						}
						if (Ctrl->T.label) {	/* Replace defaults */
							free ((void *)Ctrl->T.txt[0]);
							free ((void *)Ctrl->T.txt[1]);
							Ctrl->T.txt[0] = strdup (txt_a);
							Ctrl->T.txt[1] = strdup (txt_b);
						}
					}
					n_errors += GMT_check_condition (GMT, n == 1 || Ctrl->T.spacing <= 0.0 || Ctrl->T.length == 0.0, "Syntax error -T option.  Correct syntax:\n\t-T[+|-][<tick_gap>[%s]/<tick_length>[%s]][:LH], <tick_gap> must be > 0\n", GMT_DIM_UNITS_DISPLAY, GMT_DIM_UNITS_DISPLAY);
				}
				break;
			case 'W':	/* Sets pen attributes */
				Ctrl->W.active = TRUE;
				k = 0;
				if (opt->arg[k] == '+') Ctrl->W.color_cont = TRUE, k++;
				if (opt->arg[k] == '-') Ctrl->W.color_cont = Ctrl->W.color_text = TRUE, k++;
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

	n_errors += GMT_check_condition (GMT, !GMT->common.J.active && !Ctrl->D.active, "Syntax error:  Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->C.file && Ctrl->C.interval <= 0.0, "Syntax error -C option:  Must specify contour interval, file name with levels, or cpt-file\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->Q.active && !(Ctrl->W.active || Ctrl->I.active), "Syntax error:  Must specify one of -W or -I\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.active && (Ctrl->I.active || Ctrl->L.active || Ctrl->N.active || Ctrl->G.active || Ctrl->W.active), "Syntax error:  Cannot use -G, -I, -L, -N, -W with -D\n");
	n_errors += GMT_check_condition (GMT, Ctrl->I.active && !Ctrl->C.file, "Syntax error -I option:  Must specify a color palette table via -C\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && !Ctrl->Q.file, "Syntax error -Q option:  Must specify an index file\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && Ctrl->Q.file && GMT_access (GMT, Ctrl->Q.file, F_OK), "Syntax error -Q option:  Cannot find file %s\n", Ctrl->Q.file);
	n_errors += GMT_check_condition (GMT, Ctrl->W.color_cont && !Ctrl->C.cpt, "Syntax error -W option:  + or - only valid if -C sets a cpt file\n");
	n_errors += GMT_check_binary_io (GMT, 3);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_pscontour_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_pscontour (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{
	GMT_LONG nx, k2, k3, node1, node2, c, PSCONTOUR_SUM, cont_counts[2] = {0, 0};
	GMT_LONG n_alloc, n_fields, n_contours = 0, io_mode = 0, id;
	GMT_LONG add, last_entry, last_exit, make_plot, n_save = 0, n_save_alloc = 0;
	GMT_LONG ij, n, np, k, i, low, high, *vert = NULL, *cind = NULL;
	GMT_LONG error = FALSE, skip = FALSE, closed, *n_seg_alloc = NULL, *n_seg = NULL;
	GMT_LONG tbl_scl = 0, two_only = FALSE, fmt[3] = {0, 0, 0}, n_tables = 0, tbl;
	
	int *ind = NULL;
	
	double xx[3], yy[3], zz[3], xout[5], yout[5], xyz[2][3], rgb[4], z_range, small;
	double *xc = NULL, *yc = NULL, *zc = NULL, *x = NULL, *y = NULL, *z = NULL;
	double current_contour = -DBL_MAX, *in = NULL, *xp = NULL, *yp = NULL;

	char cont_label[GMT_LONG_TEXT], format[GMT_LONG_TEXT];

	struct PSCONTOUR *cont = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_LINE_SEGMENT *S = NULL;
	struct GMT_PALETTE *P = NULL;
	struct SAVE *save = NULL;
	struct PSCONTOUR_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_pscontour_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_pscontour_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_pscontour", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJRb:", "BKOPUXxYychipst>" GMT_OPT("EMm"), options))) Return (error);
	Ctrl = (struct PSCONTOUR_CTRL *)New_pscontour_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pscontour_parse (API, Ctrl, options))) Return (error);
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the pscontour main code ----------------------------*/

	if ((error = GMT_set_cols (GMT, GMT_IN, 3))) Return (error);
	if ((error = GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Register data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_REC))) Return (error);	/* Enables data input and sets access mode */

	if (Ctrl->C.cpt) {	/* Presumably got a cpt-file; read it here so we can crash if no-such-file before we process input data */
		if (GMT_Get_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->C.file, (void **)&P)) Return (GMT_DATA_READ_ERROR);
		if (Ctrl->I.active && P->is_continuous) {
			GMT_report (GMT, GMT_MSG_FATAL, "-I option requires constant color between contours!\n");
			Return (GMT_OK);
		}
		if (P->categorical) {
			GMT_report (GMT, GMT_MSG_FATAL, "Warning:  Categorical data (as implied by CPT file) do not have contours.  Check plot.\n");
		}
	}
	make_plot = !Ctrl->D.active;	/* Turn off plotting if -D was used */

	if (make_plot && GMT_err_fail (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	n_alloc = GMT_CHUNK;
	x = GMT_memory (GMT, NULL, n_alloc, double);
	y = GMT_memory (GMT, NULL, n_alloc, double);
	z = GMT_memory (GMT, NULL, n_alloc, double);

	xyz[0][GMT_Z] = DBL_MAX;	xyz[1][GMT_Z] = -DBL_MAX;
	n = 0;

	while ((n_fields = GMT_Get_Record (API, GMT_READ_DOUBLE, (void **)&in)) != EOF) {	/* Keep returning records until we have no more files */

		if (GMT_REC_IS_ERROR (GMT)) Return (EXIT_FAILURE);

		if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Skip table and segment headers */
		
		if (Ctrl->S.active) {	/* Must check if points are inside plot region */
			GMT_map_outside (GMT, in[GMT_X], in[GMT_Y]);
			skip = (GMT_abs (GMT->current.map.this_x_status) > 1 || GMT_abs (GMT->current.map.this_y_status) > 1);
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
				GMT_report (GMT, GMT_MSG_FATAL, "Error: Cannot triangulate more than %d points\n", INT_MAX);
				GMT_free (GMT, x);
				GMT_free (GMT, y);
				GMT_free (GMT, z);
				Return (EXIT_FAILURE);
			}	
		}
	}

	x = GMT_memory (GMT, x, n, double);
	y = GMT_memory (GMT, y, n, double);
	z = GMT_memory (GMT, z, n, double);

	if (make_plot && GMT_contlabel_prep (GMT, &Ctrl->contour, xyz)) Return (EXIT_FAILURE);	/* Prep for crossing lines, if any */

	/* Map transform */

	for (i = 0; i < n; i++) GMT_geo_to_xy (GMT, x[i], y[i], &x[i], &y[i]);

	if (Ctrl->Q.active) {	/* Read precalculated triangulation indices */
		GMT_LONG seg, row, col;
		struct GMT_DATASET *Tin = NULL;
		struct GMT_TABLE *T = NULL;

		if (GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->Q.file, (void **)&Tin)) Return ((error = GMT_DATA_READ_ERROR));

 		if (Tin->n_columns < 3) {	/* Trouble */
			GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -Q:  %s does not have at least 3 columns with indices\n", Ctrl->Q.file);
			GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Tin);
			Return (EXIT_FAILURE);
		}
		T = Tin->table[0];	/* Since we only have one table here */
		np = T->n_records;
		ind = GMT_memory (GMT, NULL, 3 * np, int);	/* Allocate the integer index array */
		for (seg = ij = 0; seg < T->n_segments; seg++) {
			for (row = 0; row < T->segment[seg]->n_rows; row++, ij++) {
				for (col = 0; col < 3; col++) ind[ij++] = irint (T->segment[seg]->coord[col][row]);
			}
		}
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&Tin);
		GMT_report (GMT, GMT_MSG_NORMAL, "Read %ld indices triplets from %s.\n", np, Ctrl->Q.file);
	}
	else {	/* Do our own Delaunay triangulation */
		np = GMT_delaunay (GMT, x, y, (int)n, &ind);
		GMT_report (GMT, GMT_MSG_NORMAL, "Obtained %ld indices triplets via Delauney triangulation [%s].\n", np, ALGORITHM);
	}

	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

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
			cont[c].do_tick = (char)Ctrl->T.active;
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
		cont[c].do_tick = (char)Ctrl->T.active;
		n_contours = c + 1;
	}
	else if (Ctrl->C.file) {	/* read contour info from file with cval C|A [angle] records */
		char record[BUFSIZ];
		GMT_LONG got, out_ID, c_alloc = 0;
		double tmp;

		if (GMT_Register_IO (API, GMT_IS_TEXTSET, GMT_IS_FILE, GMT_IS_TEXT, GMT_IN, (void **)&Ctrl->C.file, NULL, NULL, &out_ID)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error registering contour info file %s\n", Ctrl->C.file);
			Return (EXIT_FAILURE);
		}
		c = 0;
		while (GMT_Get_Record (API, GMT_READ_TEXT, (void **)&record) != EOF) {
			if (GMT_REC_IS_ANY_HEADER (GMT)) continue;	/* Skip table and segment headers */
			if (c == c_alloc) n_alloc = GMT_malloc (GMT, cont, c, c_alloc, struct PSCONTOUR);
			got = sscanf (record, "%lf %c %lf", &cont[c].val, &cont[c].type, &tmp);
			if (cont[c].type == '\0') cont[c].type = 'C';
			cont[c].do_tick = (Ctrl->T.active && ((cont[c].type == 'C') || (cont[c].type == 'A'))) ? 1 : 0;
			cont[c].angle = (got == 3) ? tmp : GMT->session.d_NaN;
			if (got == 3) Ctrl->contour.angle_type = 2;	/* Must set this directly if angles are provided */
			cont[c].do_tick = (char)Ctrl->T.active;
			c++;
		}
		n_contours = c;
	}
	else {	/* Set up contour intervals automatically from Ctrl->C.interval and Ctrl->A.interval */
		GMT_LONG c_alloc = 0, ic;
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
			if (c == c_alloc) n_alloc = GMT_malloc (GMT, cont, c, c_alloc, struct PSCONTOUR);
			cont[c].val = ic * Ctrl->C.interval;
			if (Ctrl->contour.annot && (cont[c].val - aval) > GMT_SMALL) aval += Ctrl->A.interval;
			cont[c].type = (fabs (cont[c].val - aval) < GMT_SMALL) ? 'A' : 'C';
			cont[c].angle = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
			cont[c].do_tick = (char)Ctrl->T.active;
		}
		n_contours = c;
	}
	GMT_malloc (GMT, cont, 0, n_contours, struct PSCONTOUR);

	if (Ctrl->D.active) {
		if (!Ctrl->D.file[0] || !strchr (Ctrl->D.file, '%'))	/* No file given or filename without C-format specifiers means a single output file */
			io_mode = GMT_WRITE_DATASET;
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
					io_mode = GMT_WRITE_SEGMENTS;
				else {	/* f[+c]: Write one table with all segments for each contour level, possibly one each for C|O */	
					io_mode = GMT_WRITE_TABLES;
					tbl_scl = (fmt[0]) ? 2 : 1;
					n_tables = n_contours * tbl_scl;
				}
			}
			else if (fmt[1])	/* d[+c]: Want individual files with running numbers only, with or without C|O modifier  */
				io_mode = GMT_WRITE_SEGMENTS;
			else if (fmt[0]) {	/* c: Want two files: one for open and one for closed contours */
				io_mode = GMT_WRITE_TABLES;
				n_tables = 2;
				two_only = TRUE;
			}
		}
		GMT->current.io.multi_segments[GMT_OUT] = TRUE;		/* Turn on -mo explicitly */
		D = GMT_create_dataset (GMT, n_tables, 0, 3, 0);	/* An empty table */
		n_seg_alloc = GMT_memory (GMT, NULL, n_tables, GMT_LONG);
		n_seg = GMT_memory (GMT, NULL, n_tables, GMT_LONG);
		if ((error = GMT_set_cols (GMT, GMT_OUT, 3))) Return (error);
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_OUT, GMT_BY_SET))) Return (error);	/* Enables data output and sets access mode */
	}
	
	if (make_plot) {
		/* if (Ctrl->contour.delay) GMT->current.ps.clip = +1; */	/* Signal that this program initiates clipping that wil outlive this process */
		GMT_plotinit (API, PSL, options);
		GMT_plane_perspective (GMT, PSL, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
        	if (!(Ctrl->N.active  || Ctrl->contour.delay)) GMT_map_clip_on (GMT, PSL, GMT->session.no_rgb, 3);
		Ctrl->contour.line_pen = Ctrl->W.pen[0];
	}

	if (Ctrl->L.active) {	/* Draw triangular mesh */

		GMT_setpen (GMT, PSL, &Ctrl->L.pen);

		for (i = k = 0; i < np; i++) {	/* For all triangles */

			xx[0] = x[ind[k]];	yy[0] = y[ind[k++]];
			xx[1] = x[ind[k]];	yy[1] = y[ind[k++]];
			xx[2] = x[ind[k]];	yy[2] = y[ind[k++]];

			PSL_plotline (PSL, xx, yy, 3, PSL_MOVE + PSL_STROKE + PSL_CLOSE);
		}
	}

	/* Get PSCONTOUR structs */

	if (Ctrl->W.active) {
		for (i = 0; i < n_contours; i++) {
			cont[i].n_alloc = GMT_SMALL_CHUNK;
			cont[i].L = GMT_memory (GMT, NULL, GMT_SMALL_CHUNK, struct PSCONTOUR_LINE);
		}
	}

	z_range = xyz[1][GMT_Z] - xyz[0][GMT_Z];
	small = MIN (Ctrl->C.interval, z_range) * 1.0e-6;	/* Our float noise threshold */

	for (i = ij = 0; i < np; i++, ij += 3) {	/* For all triangles */

		k = ij;
		xx[0] = x[ind[k]];	yy[0] = y[ind[k]];	zz[0] = z[ind[k++]];
		xx[1] = x[ind[k]];	yy[1] = y[ind[k]];	zz[1] = z[ind[k++]];
		xx[2] = x[ind[k]];	yy[2] = y[ind[k]];	zz[2] = z[ind[k]];

		nx = get_triangle_crossings (GMT, cont, n_contours, x, y, z, &ind[ij], small, &xc, &yc, &zc, &vert, &cind);

		if (Ctrl->I.active) {	/* Must color the triangle slices according to cpt file */

			if (nx == 0) {	/* No contours go through - easy, but must check for NaNs */
				GMT_LONG kzz;
				double zzz;
				for (k = kzz = 0, zzz = 0.0; k < 3; k++) {
					if (GMT_is_dnan (zz[k])) continue;
					zzz += zz[k];
					kzz++;
				}
				if (kzz) paint_it_pscontour (GMT, PSL, P, xx, yy, (GMT_LONG)3, zzz / kzz);
			}
			else {	/* Must paint all those slices separately */

				/* Find vertices with the lowest and highest values */

				for (k = 1, low = high = 0; k < 3; k++) {
					if (zz[k] < zz[low])   low = k;
					if (zz[k] > zz[high]) high = k;
				}

				/* Paint the piece delimited by the low node and the first contour */

				xout[0] = xx[low];	yout[0] = yy[low];
				node1 = get_node_index (vert[0], vert[1]);	/* Find single vertex opposing this contour segment */
				if (node1 == low) {	/* Contour and low node make up a triangle */
					xout[1] = xc[0];	yout[1] = yc[0];
					xout[2] = xc[1];	yout[2] = yc[1];
					n = 3;
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
					n = 4;
				}
				paint_it_pscontour (GMT, PSL, P, xout, yout, n, 0.5 * (zz[low] + zc[1]));	/* z is contour value */

				/* Then loop over contours and paint the part between contours */

				for (k = 1, k2 = 2, k3 = 3; k < nx; k++, k2 += 2, k3 += 2) {
					xout[0] = xc[k2-2];	yout[0] = yc[k2-2];
					xout[1] = xc[k3-2];	yout[1] = yc[k3-2];
					n = 2;
					last_entry = vert[k2-2];
					last_exit  = vert[k3-2];
					if (last_exit == vert[k2]) {
						xout[n] = xc[k2];	yout[n] = yc[k2];	n++;
						xout[n] = xc[k3];	yout[n] = yc[k3];	n++;
						if (vert[k3] != last_entry) {	/* Need to add an intervening corner */
							node1 = get_node_index (last_entry, vert[k3]);	/* Find corner id */
							xout[n] = xx[node1];	yout[n] = yy[node1];	n++;
						}
					}
					else if (last_exit == vert[k3]) {
						xout[n] = xc[k3];	yout[n] = yc[k3];	n++;
						xout[n] = xc[k2];	yout[n] = yc[k2];	n++;
						if (vert[k2] != last_entry) {	/* Need to add an intervening corner */
							node1 = get_node_index (last_entry, vert[k2]);	/* Find corner id */
							xout[n] = xx[node1];	yout[n] = yy[node1];	n++;
						}
					}
					else if (last_entry == vert[k2]) {
						node1 = get_node_index (last_exit, vert[k3]);	/* Find corner id */
						xout[n] = xx[node1];	yout[n] = yy[node1];	n++;
						xout[n] = xc[k3];	yout[n] = yc[k3];	n++;
						xout[n] = xc[k2];	yout[n] = yc[k2];	n++;
					}
					else {
						node1 = get_node_index (last_exit, vert[k2]);	/* Find corner id */
						xout[n] = xx[node1];	yout[n] = yy[node1];	n++;
						xout[n] = xc[k2];	yout[n] = yc[k2];	n++;
						xout[n] = xc[k3];	yout[n] = yc[k3];	n++;
					}
					paint_it_pscontour (GMT, PSL, P, xout, yout, n, 0.5 * (zc[k2]+zc[k2-2]));
				}

				/* Add the last piece between last contour and high node */

				k2 -= 2;	k3 -= 2;
				xout[0] = xx[high];	yout[0] = yy[high];
				node1 = get_node_index (vert[k2], vert[k3]);	/* Find corner id */
				if (node1 == high) {	/* Cut off a triangular piece */
					xout[1] = xc[k2];	yout[1] = yc[k2];
					xout[2] = xc[k3];	yout[2] = yc[k3];
					n = 3;
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
					n = 4;
				}
				paint_it_pscontour (GMT, PSL, P, xout, yout, n, 0.5 * (zz[high] + zc[k2]));	/* z is contour value */
			}
		}

		if (Ctrl->W.active && nx > 0) {	/* Save contour lines for later */
			for (k = k2 = 0; k < nx; k++) {
				c = cind[k];
				n = cont[c].nl;
				cont[c].L[n].x0 = xc[k2];
				cont[c].L[n].y0 = yc[k2++];
				cont[c].L[n].x1 = xc[k2];
				cont[c].L[n].y1 = yc[k2++];
				n++;
				if (n >= cont[c].n_alloc) {
					cont[c].n_alloc <<= 1;
					cont[c].L = GMT_memory (GMT, cont[c].L, cont[c].n_alloc, struct PSCONTOUR_LINE);
				}
				cont[c].nl = (int)n;
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
	
	/* Draw contours */

	if (Ctrl->W.active) {

		struct PSCONTOUR_CHAIN *head_c = NULL, *last_c = NULL, *this_c = NULL;
		struct PSCONTOUR_PT *p = NULL, *q = NULL;
				
		if (Ctrl->contour.half_width == 5) Ctrl->contour.half_width = 0;	/* Since contours are straight line segments we override the default */

		for (c = 0; c < n_contours; c++) {

			if (cont[c].nl == 0) {
				GMT_free (GMT, cont[c].L);
				continue;
			}

			GMT_report (GMT, GMT_MSG_NORMAL, "Tracing the %g contour\n", cont[c].val);

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

			while (cont[c].nl) {
				this_c = last_c->next = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_CHAIN);
				k = 0;
				this_c->begin = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_PT);
				this_c->end = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_PT);
				this_c->begin->x = cont[c].L[k].x0;
				this_c->begin->y = cont[c].L[k].y0;
				this_c->end->x = cont[c].L[k].x1;
				this_c->end->y = cont[c].L[k].y1;
				this_c->begin->next = this_c->end;
				cont[c].nl--;
				cont[c].L[k] = cont[c].L[cont[c].nl];
				while (k < cont[c].nl) {
					add = 0;
					if (fabs(cont[c].L[k].x0 - this_c->begin->x) < GMT_SMALL && fabs(cont[c].L[k].y0 - this_c->begin->y) < GMT_SMALL) {
						p = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_PT);
						p->x = cont[c].L[k].x1;
						p->y = cont[c].L[k].y1;
						p->next = this_c->begin;
						add = -1;
					}
					else if (fabs(cont[c].L[k].x1 - this_c->begin->x) < GMT_SMALL && fabs(cont[c].L[k].y1 - this_c->begin->y) < GMT_SMALL) {
						p = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_PT);
						p->x = cont[c].L[k].x0;
						p->y = cont[c].L[k].y0;
						p->next = this_c->begin;
						add = -1;
					}
					else if (fabs(cont[c].L[k].x0 - this_c->end->x) < GMT_SMALL && fabs(cont[c].L[k].y0 - this_c->end->y) < GMT_SMALL) {
						p = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_PT);
						p->x = cont[c].L[k].x1;
						p->y = cont[c].L[k].y1;
						this_c->end->next = p;
						add = 1;
					}
					else if (fabs(cont[c].L[k].x1 - this_c->end->x) < GMT_SMALL && fabs(cont[c].L[k].y1 - this_c->end->y) < GMT_SMALL) {
						p = GMT_memory (GMT, NULL, 1, struct PSCONTOUR_PT);
						p->x = cont[c].L[k].x0;
						p->y = cont[c].L[k].y0;
						this_c->end->next = p;
						add = 1;
					}
					if (add) {	/* Got one */
						if (add == -1)
							this_c->begin = p;
						else if (add == 1)
							this_c->end = p;
						cont[c].nl--;
						cont[c].L[k] = cont[c].L[cont[c].nl];
						k = 0;
					}
					else
						k++;
				}
				last_c = this_c;
			}
			GMT_free (GMT, cont[c].L);

			this_c = head_c->next;
			while (this_c) {
				xp = GMT_memory (GMT, NULL, GMT_SMALL_CHUNK, double);
				yp = GMT_memory (GMT, NULL, GMT_SMALL_CHUNK, double);
				n_alloc = GMT_SMALL_CHUNK;
				p = this_c->begin;
				n = 0;
				while (p) {
					xp[n] = p->x;
					yp[n++] = p->y;
					q = p;
					p = p->next;
					GMT_free (GMT, q);
					if (n == n_alloc) {
						n_alloc <<= 1;
						xp = GMT_memory (GMT, xp, n_alloc, double);
						yp = GMT_memory (GMT, yp, n_alloc, double);
					}
				}
				last_c = this_c;
				this_c = this_c->next;
				GMT_free (GMT, last_c);

				closed = !GMT_polygon_is_open (GMT, xp, yp, n);

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
					GMT_get_format (GMT, cont[c].val, Ctrl->contour.unit, CNULL, format);
					sprintf (cont_label, format, cont[c].val);
				}
				if (Ctrl->D.active) {
					GMT_LONG count;
					double *xtmp = NULL, *ytmp = NULL;
					/* Must first apply inverse map transform */
					xtmp = GMT_memory (GMT, NULL, n, double);
					ytmp = GMT_memory (GMT, NULL, n, double);
					for (count = 0; count < n; count++) GMT_xy_to_geo (GMT, &xtmp[count], &ytmp[count], xp[count], yp[count]);
					S = GMT_dump_contour (GMT, xtmp, ytmp, n, cont[c].val);
					/* Select which table this segment should be added to */
					tbl = (io_mode == GMT_WRITE_TABLES) ? ((two_only) ? closed : tbl_scl * c) : 0;
					if (n_seg[tbl] == n_seg_alloc[tbl]) D->table[tbl]->segment = GMT_memory (GMT, D->table[tbl]->segment, (n_seg_alloc[tbl] += GMT_SMALL_CHUNK), struct GMT_LINE_SEGMENT *);
					D->table[tbl]->segment[n_seg[tbl]++] = S;
					D->table[tbl]->n_segments++;	D->n_segments++;
					D->table[tbl]->n_records += n;	D->n_records += n;
					/* Generate a file name and increment cont_counts, if relevant */
					if (io_mode == GMT_WRITE_TABLES && !D->table[tbl]->file[GMT_OUT])
						D->table[tbl]->file[GMT_OUT] = GMT_make_filename (Ctrl->D.file, fmt, cont[c].val, closed, cont_counts);
					else if (io_mode == GMT_WRITE_SEGMENTS)
						S->file[GMT_OUT] = GMT_make_filename (Ctrl->D.file, fmt, cont[c].val, closed, cont_counts);
					GMT_free (GMT, xtmp);
					GMT_free (GMT, ytmp);
				}

				if (make_plot) {
					if (cont[c].do_tick && closed) {	/* Must store the entire contour for later processing */
						if (n_save == n_save_alloc) n_save_alloc = GMT_malloc (GMT, save, n_save, n_alloc, struct SAVE);
						(void)GMT_malloc2 (GMT, save[n_save].x, save[n_save].y, n, 0, double);
						GMT_memcpy (save[n_save].x, xp, n, double);
						GMT_memcpy (save[n_save].y, yp, n, double);
						save[n_save].n = n;
						GMT_memcpy (&save[n_save].pen, &Ctrl->W.pen[id], 1, struct GMT_PEN);
						save[n_save].do_it = TRUE;
						save[n_save].cval = cont[c].val;
						n_save++;
					}
					GMT_hold_contour (GMT, &xp, &yp, n, cont[c].val, cont_label, cont[c].type, cont[c].angle, closed, &Ctrl->contour);
				}

				GMT_free (GMT, xp);
				GMT_free (GMT, yp);
			}
			GMT_free (GMT, head_c);
		}
		if (make_plot) {
			if (Ctrl->T.active && n_save) {	/* Finally sort and plot ticked innermost contours */
				(void)GMT_malloc (GMT, save, 0, n_save, struct SAVE);

				sort_and_plot_ticks (GMT, PSL, save, n_save, x, y, z, n, Ctrl->T.spacing, Ctrl->T.length, Ctrl->T.low, Ctrl->T.high, Ctrl->T.label, Ctrl->T.txt);
				for (i = 0; i < n_save; i++) {
					GMT_free (GMT, save[i].x);
					GMT_free (GMT, save[i].y);
				}
				GMT_free (GMT, save);
			}
			GMT_contlabel_plot (GMT, PSL, &Ctrl->contour);
			GMT_contlabel_free (GMT, &Ctrl->contour);
		}
	}

	if (Ctrl->D.active) {	/* Write the contour line output file(s) */
		for (tbl = 0; tbl < D->n_tables; tbl++) D->table[tbl]->segment = GMT_memory (GMT, D->table[tbl]->segment, n_seg[tbl], struct GMT_LINE_SEGMENT *);
		if ((error = GMT_Put_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, NULL, io_mode, (void **)&(Ctrl->D.file), (void *)D))) Return (error);
		if ((error = GMT_End_IO (API, GMT_OUT, 0))) Return (error);	/* Disables further data output */
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&D);
		GMT_free (GMT, n_seg_alloc);
		GMT_free (GMT, n_seg);
	}

	if (make_plot) {
		if (!(Ctrl->N.active || Ctrl->contour.delay)) GMT_map_clip_off (GMT, PSL);
		GMT_map_basemap (GMT, PSL);
		GMT_plane_perspective (GMT, PSL, -1, 0.0);
		GMT_plotend (GMT, PSL);
	}

	GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&P);
	GMT_free (GMT, x);
	GMT_free (GMT, y);
	GMT_free (GMT, z);
	GMT_free (GMT, cont);
	if (Ctrl->Q.active)
		GMT_free (GMT, ind);	/* Allocated above by GMT_memory */
	else {
#ifdef TRIANGLE_D
#ifdef DEBUG
	/* Shewchuk's function allocated the memory separately */
		GMT_memtrack_off (GMT, GMT_mem_keeper);
#endif
#endif
		GMT_free (GMT, ind);
#ifdef TRIANGLE_D
#ifdef DEBUG
		GMT_memtrack_on (GMT, GMT_mem_keeper);
#endif
#endif
	}

	Return (GMT_OK);
}
