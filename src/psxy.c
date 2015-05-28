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
 *	This program is distrdeibuted in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 * 
 * Brief synopsis: psxy will read <x,y> points and plot symbols, lines,
 * or polygons on maps.
 */

#define THIS_MODULE_NAME	"psxy"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot lines, polygons, and symbols on maps"
#define THIS_MODULE_KEYS	"<DI,CCi,-Xo"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>BJKOPRUVXYabcdfghipst" GMT_OPT("HMm")

/* Control structure for psxy */

struct PSXY_CTRL {
	struct A {	/* -A[m|p|step] */
		bool active;
		unsigned int mode;
		double step;
	} A;
	struct C {	/* -C<cpt> or -C<color1>,<color2>[,<color3>,...] */
		bool active;
		char *file;
	} C;
	struct D {	/* -D<dx>/<dy> */
		bool active;
		double dx, dy;
	} D;
	struct E {	/* -E[x[+]|X][y[+]|Y][cap][/[+|-]<pen>] */
		bool active;
		unsigned int xbar, ybar;	/* 0 = not used, 1 = error bar, 2 = asumemtrical error bar, 3 = box-whisker, 4 = notched box-whisker */
		unsigned int mode;		/* 0 = normal, 1 = -C applies to error pen color, 2 = -C applies to symbol fill & error pen color */
		double size;
		struct GMT_PEN pen;
	} E;
	struct G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -I<intensity> */
		bool active;
		double value;
	} I;
	struct L {	/* -L[+xl|r|x0][+yb|t|y0][+e|E][+p<pen>] */
		bool active;
		bool polygon;		/* true when just -L is given */
		bool outline;		/* true when +p<pen> is given */
		unsigned int mode;	/* Which side for the anchor */
		unsigned int anchor;	/* 0 not used, 1 = x anchors, 2 = y anchors, 3 = +/-dy, 4 = -dy1, +dy2 */
		double value;
		struct GMT_PEN pen;
	} L;
	struct N {	/* -N[r|c] */
		bool active;
		unsigned int mode;
	} N;
	struct S {	/* -S */
		bool active;
		char *arg;
	} S;
	struct T {	/* -T */
		bool active;
	} T;
	struct W {	/* -W<pen> */
		bool active;
		unsigned int mode;	/* 0 = normal, 1 = -C applies to pen color only, 2 = -C applies to symbol fill & pen color */
		struct GMT_PEN pen;
	} W;
};

#define EBAR_CAP_WIDTH		7.0	/* Error bar cap width */

enum Psxy_ebartype {
	EBAR_NONE		= 0,
	EBAR_NORMAL		= 1,
	EBAR_ASYMMETRICAL	= 2,
	EBAR_WHISKER		= 3,
	EBAR_NOTCHED_WHISKER	= 4};

enum Psxy_cliptype {
	PSXY_CLIP_REPEAT 	= 0,
	PSXY_CLIP_NO_REPEAT,
	PSXY_NO_CLIP_REPEAT,
	PSXY_NO_CLIP_NO_REPEAT};

enum Psxy_poltype {
	PSXY_POL_X 		= 1,
	PSXY_POL_Y,
	PSXY_POL_SYMM_DEV,
	PSXY_POL_ASYMM_DEV,
	PSXY_POL_ASYMM_ENV};

EXTERN_MSC double GMT_half_map_width (struct GMT_CTRL *GMT, double y);

void *New_psxy_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSXY_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSXY_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->E.pen = C->W.pen = GMT->current.setting.map_default_pen;
	GMT_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* Default is no fill */
	C->E.size = EBAR_CAP_WIDTH  * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 7p */
	C->N.mode = PSXY_CLIP_REPEAT;
	return (C);
}

void Free_psxy_Ctrl (struct GMT_CTRL *GMT, struct PSXY_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C && C->C.file) free (C->C.file);
	if (C && C->S.arg) free (C->S.arg);
	GMT_free (GMT, C);
}

void plot_x_errorbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double delta_x[], double error_width2, int line, int kind) {
	double x_1, x_2, y_1, y_2;
	bool tip1, tip2;
	unsigned int first = 0, second = (kind == EBAR_ASYMMETRICAL) ? 1 : 0;	/* first and second are either both 0 or second is 1 for asymmetrical bars */

	tip1 = tip2 = (error_width2 > 0.0);
	GMT_geo_to_xy (GMT, x - fabs (delta_x[first]),  y, &x_1, &y_1);
	GMT_geo_to_xy (GMT, x + fabs (delta_x[second]), y, &x_2, &y_2);
	if (GMT_is_dnan (x_1)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: X error bar exceeded domain near line %d. Set to x_min\n", line);
		x_1 = GMT->current.proj.rect[XLO];
		tip1 = false;
	}
	if (GMT_is_dnan (x_2)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: X error bar exceeded domain near line %d. Set to x_max\n", line);
		x_2 = GMT->current.proj.rect[XHI];
		tip2 = false;
	}
	PSL_plotsegment (PSL, x_1, y_1, x_2, y_2);
	if (tip1) PSL_plotsegment (PSL, x_1, y_1 - error_width2, x_1, y_1 + error_width2);
	if (tip2) PSL_plotsegment (PSL, x_2, y_2 - error_width2, x_2, y_2 + error_width2);
}

void plot_y_errorbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double delta_y[], double error_width2, int line, int kind) {
	double x_1, x_2, y_1, y_2;
	bool tip1, tip2;
	unsigned int first = 0, second = (kind == EBAR_ASYMMETRICAL) ? 1 : 0;	/* first and second are either both 0 or second is 1 for asymmetrical bars */

	tip1 = tip2 = (error_width2 > 0.0);
	GMT_geo_to_xy (GMT, x, y - fabs (delta_y[first]),  &x_1, &y_1);
	GMT_geo_to_xy (GMT, x, y + fabs (delta_y[second]), &x_2, &y_2);
	if (GMT_is_dnan (y_1)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Y error bar exceeded domain near line %d. Set to y_min\n", line);
		y_1 = GMT->current.proj.rect[YLO];
		tip1 = false;
	}
	if (GMT_is_dnan (y_2)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Y error bar exceeded domain near line %d. Set to y_max\n", line);
		y_2 = GMT->current.proj.rect[YHI];
		tip2 = false;
	}
	PSL_plotsegment (PSL, x_1, y_1, x_2, y_2);
	if (tip1) PSL_plotsegment (PSL, x_1 - error_width2, y_1, x_1 + error_width2, y_1);
	if (tip2) PSL_plotsegment (PSL, x_2 - error_width2, y_2, x_2 + error_width2, y_2);
}

void plot_x_whiskerbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double hinge[], double error_width2, double rgb[], int line, int kind) {
	unsigned int i;
	static unsigned int q[4] = {0, 25, 75, 100};
	double xx[4], yy[4];

	for (i = 0; i < 4; i++) {	/* for 0, 25, 75, 100% hinges */
		GMT_geo_to_xy (GMT, hinge[i], y, &xx[i], &yy[i]);
		if (GMT_is_dnan (xx[i])) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: X %d %% hinge exceeded domain near line %d\n", q[i], line);
			xx[i] = (i < 2) ? GMT->current.proj.rect[XLO] : GMT->current.proj.rect[XHI];
		}
	}
	yy[1] -= error_width2;
	yy[2] += error_width2;

	PSL_plotsegment (PSL, xx[0], yy[1], xx[0], yy[2]);		/* Left whisker */
	PSL_plotsegment (PSL, xx[0], yy[0], xx[1], yy[0]);

	PSL_plotsegment (PSL, xx[3], yy[1], xx[3], yy[2]);		/* Right whisker */
	PSL_plotsegment (PSL, xx[3], yy[0], xx[2], yy[0]);

	PSL_setfill (PSL, rgb, true);
	if (kind == 2) {	/* Notched box-n-whisker plot */
		double xp[10], yp[10], s, p;
		s = 1.7 * ((1.25 * (xx[2] - xx[1])) / (1.35 * hinge[4]));	/* 5th term in hinge has n */
		xp[0] = xp[9] = xx[1];
		xp[1] = xp[8] = ((p = (x - s)) < xp[0]) ? xp[0] : p;
		xp[2] = xp[7] = x;
		xp[4] = xp[5] = xx[2];
		xp[3] = xp[6] = ((p = (x + s)) > xp[4]) ? xp[4] : p;
		yp[0] = yp[1] = yp[3] = yp[4] = yy[1];
		yp[5] = yp[6] = yp[8] = yp[9] = yy[2];
		yp[2] = yy[0] - 0.5 * error_width2;
		yp[7] = yy[0] + 0.5 * error_width2;
		PSL_plotpolygon (PSL, xp, yp, 10);
		PSL_plotsegment (PSL, x, yp[7], x, yp[2]);		/* Median line */
	}
	else {
		PSL_plotbox (PSL, xx[1], yy[1], xx[2], yy[2]);		/* Main box */
		PSL_plotsegment (PSL, x, yy[1], x, yy[2]);		/* Median line */
	}
}

void plot_y_whiskerbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double hinge[], double error_width2, double rgb[], int line, int kind) {
	unsigned int i;
	static unsigned int q[4] = {0, 25, 75, 100};
	double xx[4], yy[4];

	for (i = 0; i < 4; i++) {	/* for 0, 25, 75, 100% hinges */
		GMT_geo_to_xy (GMT, x, hinge[i], &xx[i], &yy[i]);
		if (GMT_is_dnan (yy[i])) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Warning: Y %d %% hinge exceeded domain near line %d\n", q[i], line);
			yy[i] = (i < 2) ? GMT->current.proj.rect[YLO] : GMT->current.proj.rect[YHI];
		}
	}
	xx[1] -= error_width2;
	xx[2] += error_width2;

	PSL_plotsegment (PSL, xx[1], yy[0], xx[2], yy[0]);		/* bottom whisker */
	PSL_plotsegment (PSL, xx[0], yy[0], xx[0], yy[1]);

	PSL_plotsegment (PSL, xx[1], yy[3], xx[2], yy[3]);		/* Top whisker */
	PSL_plotsegment (PSL, xx[0], yy[3], xx[0], yy[2]);

	PSL_setfill (PSL, rgb, true);
	if (kind == 2) {	/* Notched box-n-whisker plot */
		double xp[10], yp[10], s, p;
		s = 1.7 * ((1.25 * (yy[2] - yy[1])) / (1.35 * hinge[4]));	/* 5th term in hinge has n */
		xp[0] = xp[1] = xp[3] = xp[4] = xx[2];
		xp[5] = xp[6] = xp[8] = xp[9] = xx[1];
		xp[2] = xx[0] + 0.5 * error_width2;
		xp[7] = xx[0] - 0.5 * error_width2;
		yp[0] = yp[9] = yy[1];
		yp[1] = yp[8] = ((p = (y - s)) < yp[0]) ? yp[0] : p;
		yp[2] = yp[7] = y;
		yp[4] = yp[5] = yy[2];
		yp[3] = yp[6] = ((p = (y + s)) > yp[4]) ? yp[4] : p;
		PSL_plotpolygon (PSL, xp, yp, 10);
		PSL_plotsegment (PSL, xp[7], y, xp[2], y);		/* Median line */
	}
	else {
		PSL_plotbox (PSL, xx[2], yy[2], xx[1], yy[1]);		/* Main box */
		PSL_plotsegment (PSL, xx[1], y, xx[2], y);		/* Median line */
	}
}

int GMT_psxy_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the psxy synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: psxy [<table>] %s %s [-A[m|p]]\n", GMT_J_OPT, GMT_Rgeoz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-C<cpt>] [-D<dx>/<dy>] [-E[x[+]|y[+]|X|Y][n][cap][/[+|-]<pen>]] [-G<fill>]\n", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-I<intens>] [-K] [-L[+b|d|D][+xl|r|x0][+yb|t|y0][+p<pen>]] [-N[c|r]] [-O] [-P] [-S[<symbol>][<size>[unit]]]\n", GMT_Jz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-T] [%s] [%s] [-W[+|-][<pen>]]\n\t[%s] [%s] [%s]\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_a_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_bi_OPT, GMT_di_OPT, \
		GMT_c_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_s_OPT, GMT_t_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Option (API, "J-Z,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Suppress drawing line segments as great circle arcs, i.e., draw\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   straight lines unless m or p is appended to first follow meridian\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   then parallel, or vice versa.\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Use cpt-file (or specify -Ccolor1,color2[,color3,...]) to assign symbol\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   colors based on z-value in 3rd column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: requires -S. Without -S, psxy excepts lines/polygons\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and looks for -Z<val> options in each multiheader. Then, color is\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   applied for polygon fill (-L) or polygon pen (no -L).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Offset symbol or line positions by <dx>/<dy> [no offset].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Draw (symmetrical) error bars for x, y, or both.  Add cap-width [%gp].\n", EBAR_CAP_WIDTH);
	GMT_Message (API, GMT_TIME_NONE, "\t   If + is appended after x|y then we expect asymmetrical errors (two columns) [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append pen attributes. A leading + applies cpt color (-C) to symbol\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   fill and error pen; - applies it to pen only.  If X or Y is used then\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a box-and-whisker diagram is drawn instead, using data from 4 extra\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   columns to get the 0 %%, 25 %%, 75 %%, and 100%% quantiles (point value\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   is assumed to be 50%%).  If n is appended after X (or Y) we expect a\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   5th extra column with the sample size, which is needed to draw a\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   notched box-and whisker diagram (notch width represents uncertainty.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   in the median).  Finally, use -W, -G to affect the 25-75%% box.\n");
	GMT_fill_syntax (API->GMT, 'G', "Specify color or pattern [no fill].");
	GMT_Message (API, GMT_TIME_NONE, "\t   -G option can be present in all subheaders (not with -S).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Use the intensity to modulate the fill color (requires -C or -G).\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Force closed polygons.  Alternatively, append modifiers to build polygon from a line.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +d to build symmetrical envelope around y(x) using deviations dy(x) from col 3.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +D to build asymmetrical envelope around y(x) using deviations dy1(x) and dy2(x) from cols 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +b to build asymmetrical envelope around y(x) using bounds yl(x) and yh(x) from cols 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +xl|r|x0 to connect 1st and last point to anchor points at xmin, xmax, or x0, or\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   append +yb|t|y0 to connect 1st and last point to anchor points at ymin, ymax, or y0.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Polygon may be painted (-G) and optionally outlined via +p<pen> [no outline].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do not skip or clip symbols that fall outside the map border [clipping is on]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Nr to turn off clipping and plot repeating symbols for periodic maps.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Nc to retain clipping but turn off plotting of repeating symbols for periodic maps.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default will clip or skip symbols that fall outside and plot repeating symbols].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Select symbol type and symbol size (in %s).  Choose between\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t   -(xdash), +(plus), st(a)r, (b|B)ar, (c)ircle, (d)iamond, (e)llipse,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (f)ront, octa(g)on, (h)exagon, (i)nvtriangle, (j)rotated rectangle,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (k)ustom, (l)etter, (m)athangle, pe(n)tagon, (p)oint, (q)uoted line, (r)ectangle\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (R)ounded rectangle, (s)quare, (t)riangle, (v)ector, (w)edge, (x)cross, (y)dash.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or =(geovector, i.e., great or small circle vectors).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no size is specified, then the 3rd column must have sizes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no symbol is specified, then last column must have symbol codes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Note: if -C is selected then 3rd means 4th column, etc.]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Symbols A, C, D, G, H, I, N, S, T are adjusted to have same area\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   as a circle of the specified diameter.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Bars: Append b[<base>] to give the y-value of the base [Default = 0 (1 for log-scales)].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Append u if width is in x-input units [Default is %s].\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t      Use upper case -SB for horizontal bars (<base> then refers to x\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      and width may be in y-units [Default is vertical]. To read the <base>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      value from file, specify b with no trailing value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Ellipses: Direction, major, and minor axis must be in columns 3-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SE rather than -Se is selected, psxy will expect azimuth, and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     axes in km, and convert azimuths based on map projection.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If projection is linear then we scale the axes by the map scale.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use -SE- for a degenerate ellipse (circle) with only diameter in km given\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     in column 3, or append a fixed diameter in km to -SE- instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Rotatable Rectangle: Direction, x- and y-dimensions in columns 3-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SJ rather than -Sj is selected, psxy will expect azimuth, and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     dimensions in km and convert azimuths based on map projection.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use -SJ- for a degenerate rectangle (square w/no rotation) with only diameter in km given\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     in column 3, or append a fixed diameter in km to -SJ- instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     For linear projection we scale dimensions by the map scale.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Fronts: Give <tickgap>[/<ticklen>][+l|+r][+<type>][+o<offset>][+p[<pen>]].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If <tickgap> is negative it means the number of gaps instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     The <ticklen> defaults to 15%% of <tickgap> if not given.  Append\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +l or +r   : Plot symbol to left or right of front [centered]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +<type>    :  +b(ox), +c(ircle), +f(ault), +s(lip), +t(riangle) [f]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       box      : square when centered, half-square otherwise.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       circle   : full when centered, half-circle otherwise.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       fault    : centered cross-tick or tick only in specified direction.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       slip     : left-or right-lateral strike-slip arrows.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       triangle : diagonal square when centered, directed triangle otherwise.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +o<offset> : Plot first symbol when along-front distance is offset [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +p[<pen>]  : Alternate pen for symbol outline; if no <pen> then no outline [Outline with -W pen].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Kustom: Append <symbolname> immediately after 'k'; this will look for\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     <symbolname>.def in the current directory, in $GMT_USERDIR,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     or in $GMT_SHAREDIR (searched in that order).\n");
	GMT_list_custom_symbols (API->GMT);
	GMT_Message (API, GMT_TIME_NONE, "\t   Letter: append +t<string> after symbol size, and optionally +f<font> and +j<justify>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Mathangle: radius, start, and stop directions of math angle must be in columns 3-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SM rather than -Sm is used, we draw straight angle symbol if 90 degrees.\n");
	GMT_vector_syntax (API->GMT, 0);
	GMT_Message (API, GMT_TIME_NONE, "\t   Quoted line: Give [d|f|n|l|x]<info>[:<labelinfo>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     <code><info> controls placement of labels along lines.  Select\n");
	GMT_cont_syntax (API->GMT, 7, 1);
	GMT_Message (API, GMT_TIME_NONE, "\t     <labelinfo> controls the label attributes.  Choose from\n");
	GMT_label_syntax (API->GMT, 7, 1);
	GMT_Message (API, GMT_TIME_NONE, "\t   Rectangles: x- and y-dimensions must be in columns 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Rounded rectangles: x- and y-dimensions and corner radius must be in columns 3-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Vectors: Direction and length must be in columns 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SV rather than -Sv is selected, psxy will expect azimuth and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     length and convert azimuths based on the chosen map projection.\n");
	GMT_vector_syntax (API->GMT, 19);
	GMT_Message (API, GMT_TIME_NONE, "\t   Wedges: Start and stop directions of wedge must be in columns 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SW rather than -Sw is selected, specify two azimuths instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Geovectors: Azimuth and length (in km) must be in columns 3-4.\n");
	GMT_vector_syntax (API->GMT, 3);
	GMT_Message (API, GMT_TIME_NONE, "\t-T Ignore all input files.\n");
	GMT_Option (API, "U,V");
	GMT_pen_syntax (API->GMT, 'W', "Set pen attributes [Default pen is %s]:");
	GMT_Message (API, GMT_TIME_NONE, "\t   A leading + applies cpt color (-C) to both symbol fill and pen.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   A leading - applies cpt color (-C) to the pen only.\n");
	GMT_Option (API, "X,a,bi");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is the required number of columns.\n");
	GMT_Option (API, "c,di,f,g,h,i,p,s,t,:,.");

	return (EXIT_FAILURE);
}

int GMT_psxy_parse (struct GMT_CTRL *GMT, struct PSXY_CTRL *Ctrl, struct GMT_OPTION *options, struct GMT_SYMBOL *S)
{
	/* This parses the options provided to psxy and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int j0, n_errors = 0;
	int j;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Turn off draw_arc mode */
				Ctrl->A.active = true;
				switch (opt->arg[0]) {
					case 'm': Ctrl->A.mode = 1; break;
					case 'p': Ctrl->A.mode = 2; break;
#ifdef DEBUG
					default: Ctrl->A.step = atof (opt->arg); break; /* Undocumented test feature */
#endif
				}
				break;
			case 'C':	/* Vary symbol color with z */
				if (Ctrl->C.file) free (Ctrl->C.file);
				Ctrl->C.file = strdup (opt->arg);
				Ctrl->C.active = true;
				break;
			case 'D':
				if ((j = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b)) < 1) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -D option: Give x [and y] offsets\n");
					n_errors++;
				}
				else {
					Ctrl->D.dx = GMT_to_inch (GMT, txt_a);
					Ctrl->D.dy = (j == 2) ? GMT_to_inch (GMT, txt_b) : Ctrl->D.dx;
					Ctrl->D.active = true;
				}
				break;
			case 'E':	/* Get info for error bars and bow-whisker bars */
				Ctrl->E.active = true;
				j = 0;
				while (opt->arg[j] && opt->arg[j] != '/') {
					switch (opt->arg[j]) {
						case 'x':	/* Error bar for x */
							Ctrl->E.xbar = EBAR_NORMAL;
							if (opt->arg[j+1] == '+') { Ctrl->E.xbar = EBAR_ASYMMETRICAL; j++;}
							break;
						case 'X':	/* Box-whisker instead */
							Ctrl->E.xbar = EBAR_WHISKER;
							if (opt->arg[j+1] == 'n') {Ctrl->E.xbar = EBAR_NOTCHED_WHISKER; j++;}
							break;
						case 'y':	/* Error bar for y */
							Ctrl->E.ybar = EBAR_NORMAL;
							if (opt->arg[j+1] == '+') { Ctrl->E.ybar = EBAR_ASYMMETRICAL; j++;}
							break;
						case 'Y':	/* Box-whisker instead */
							Ctrl->E.ybar = EBAR_WHISKER;
							if (opt->arg[j+1] == 'n') {Ctrl->E.ybar = EBAR_NOTCHED_WHISKER; j++;}
							break;
						case '+':	/* Only allowed for -E+ as shorthand for -Ex+y+ */
							if (j == 0) Ctrl->E.xbar = Ctrl->E.ybar = EBAR_ASYMMETRICAL;
							break;
						default:	/* Get error 'cap' width */
							strncpy (txt_a, &opt->arg[j], GMT_LEN256);
							j0 = 0;
							while (txt_a[j0] && txt_a[j0] != '/') j0++;
							txt_a[j0] = 0;
							Ctrl->E.size = GMT_to_inch (GMT, txt_a);
							while (opt->arg[j] && opt->arg[j] != '/') j++;
							j--;
							break;
					}
					j++;
				}
				if (opt->arg[j] == '/') {
					j++;
					if (opt->arg[j] == '-') {Ctrl->E.mode = 1; j++;}
					if (opt->arg[j] == '+') {Ctrl->E.mode = 2; j++;}
					if (opt->arg[j] && GMT_getpen (GMT, &opt->arg[j], &Ctrl->E.pen)) {
						GMT_pen_syntax (GMT, 'E', "sets error bar pen attributes");
						n_errors++;
					}
				}
				GMT_Report (API, GMT_MSG_DEBUG, "Settings for -E: x = %d y = %d\n", Ctrl->E.xbar, Ctrl->E.ybar);
				break;
			case 'G':		/* Set fill for symbols or polygon */
				Ctrl->G.active = true;
				if (!opt->arg[0] || GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " "); n_errors++;
				}
				break;
			case 'I':	/* Adjust symbol color via intensity */
				Ctrl->I.value = atof (opt->arg);
				Ctrl->I.active = true;
				break;
			case 'L':		/* Close line segments */
				Ctrl->L.active = true;
				if ((c = strstr (opt->arg, "+b")))	/* Build asymmetric polygon from lower and upper bounds */
					Ctrl->L.anchor = PSXY_POL_ASYMM_ENV;
				else if ((c = strstr (opt->arg, "+d")))	/* Build symmetric polygon from deviations about y(x) */
					Ctrl->L.anchor = PSXY_POL_SYMM_DEV;
				else if ((c = strstr (opt->arg, "+D")))	/* Build asymmetric polygon from deviations about y(x) */
					Ctrl->L.anchor = PSXY_POL_ASYMM_DEV;
				else if ((c = strstr (opt->arg, "+x"))) {	/* Parse x anchors for a polygon */
					switch (c[2]) {
						case 'l':	Ctrl->L.mode = XLO;	break;	/* Left side anchors */
						case 'r':	Ctrl->L.mode = XHI;	break;	/* Right side anchors */
						default:	Ctrl->L.mode = ZLO;	Ctrl->L.value = atof (&c[2]);	break;	/* Arbitrary x anchor */
					}
					Ctrl->L.anchor = PSXY_POL_X;
				}
				else if ((c = strstr (opt->arg, "+y"))) {	/* Parse y anchors for a polygon */
					switch (c[2]) {
						case 'b':	Ctrl->L.mode = YLO;	break;	/* Bottom side anchors */
						case 't':	Ctrl->L.mode = YHI;	break;	/* Top side anchors */
						default:	Ctrl->L.mode = ZHI;	Ctrl->L.value = atof (&c[2]);	break;	/* Arbitrary y anchor */
					}
					Ctrl->L.anchor = PSXY_POL_Y;
				}
				else	/* Just force a closed polygon */
					Ctrl->L.polygon = true;
				if ((c = strstr (opt->arg, "+p"))) {	/* Want outline */
					if (opt->arg[j] && GMT_getpen (GMT, &c[2], &Ctrl->L.pen)) {
						GMT_pen_syntax (GMT, 'W', "sets pen attributes [Default pen is %s]:");
						n_errors++;
					}
					Ctrl->L.outline = true;
				}
				break;
			case 'N':		/* Do not skip points outside border */
				Ctrl->N.active = true;
				if (opt->arg[0] == 'r') Ctrl->N.mode = PSXY_NO_CLIP_REPEAT;
				else if (opt->arg[0] == 'c') Ctrl->N.mode = PSXY_CLIP_NO_REPEAT;
				else if (opt->arg[0] == '\0') Ctrl->N.mode = PSXY_NO_CLIP_NO_REPEAT;
				else {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -N option: Unrecognized argument %s\n", opt->arg);
					n_errors++;
				}
				break;
			case 'S':		/* Get symbol [and size] */
				Ctrl->S.active = true;
				Ctrl->S.arg = strdup (opt->arg);
				break;
			case 'T':		/* Skip all input files */
				Ctrl->T.active = true;
				break;
			case 'W':		/* Set line attributes */
				Ctrl->W.active = true;
				j = 0;
				if (opt->arg[j] == '-') {Ctrl->W.mode = 1; j++;}
				if (opt->arg[j] == '+') {Ctrl->W.mode = 2; j++;}
				if (opt->arg[j] && GMT_getpen (GMT, &opt->arg[j], &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', "sets pen attributes [Default pen is %s]:");
					GMT_Report (API, GMT_MSG_NORMAL, "\t   A leading + applies cpt color (-C) to both symbol fill and pen.\n");
					GMT_Report (API, GMT_MSG_NORMAL, "\t   A leading - applies cpt color (-C) to the pen only.\n");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	n_errors += GMT_check_condition (GMT, Ctrl->S.active && GMT_parse_symbol_option (GMT, Ctrl->S.arg, S, 0, true), "Syntax error -S option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && (S->symbol == GMT_SYMBOL_VECTOR || S->symbol == GMT_SYMBOL_GEOVECTOR || S->symbol == GMT_SYMBOL_MARC || S->symbol == GMT_SYMBOL_ELLIPSE || S->symbol == GMT_SYMBOL_FRONT || S->symbol == GMT_SYMBOL_QUOTED_LINE || S->symbol == GMT_SYMBOL_ROTRECT), "Syntax error -E option: Incompatible with -Se, -Sf, -Sj, -Sm|M, -Sq, -Sv|V, -S=\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && S->symbol == GMT_SYMBOL_NOT_SET, "Syntax error: Binary input data cannot have symbol information\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && Ctrl->E.mode && !Ctrl->C.active, "Syntax error: -E option +|-<pen> requires the -C option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->W.mode && !Ctrl->C.active, "Syntax error: -W option +|-<pen> requires the -C option\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->W.mode + Ctrl->E.mode) == 3, "Syntax error: Conflicting -E and -W options regarding -C option application\n");
	n_errors += GMT_check_condition (GMT, Ctrl->L.anchor && !Ctrl->G.active && !Ctrl->L.outline, "Syntax error: -L<modifiers> must include +p<pen> if -G not given\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_psxy_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_psxy (void *V_API, int mode, void *args)
{	/* High-level function that implements the psxy task */
	bool polygon, penset_OK = true, not_line, old_is_world;
	bool get_rgb, read_symbol, clip_set = false, fill_active;
	bool error_x = false, error_y = false, def_err_xy = false;
	bool default_outline, outline_active, geovector = false;
	unsigned int set_type, n_needed, n_cols_start = 2, justify, tbl;
	unsigned int i, n_total_read = 0, j, geometry, read_mode;
	unsigned int bcol, ex1, ex2, ex3, change, pos2x, pos2y, save_u = false;
	unsigned int xy_errors[2], error_type[2] = {EBAR_NONE, EBAR_NONE}, error_cols[5] = {0,1,2,4,5};
	int error = GMT_NOERROR;

	char *text_rec = NULL, s_args[GMT_BUFSIZ] = {""};

	double direction, length, dx, dy, d, dim[PSL_MAX_DIMS], *in = NULL;
	double s, c, plot_x, plot_y, x_1, x_2, y_1, y_2;

	struct GMT_PEN current_pen, default_pen;
	struct GMT_FILL current_fill, default_fill, black;
	struct GMT_SYMBOL S;
	struct GMT_PALETTE *P = NULL;
	struct GMT_DATASEGMENT *L = NULL;
	struct PSXY_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	void *record = NULL;	/* Opaque pointer to either a text (buffer) or double (in) record */
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_psxy_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_psxy_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_psxy_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);

	/* Initialize GMT_SYMBOL structure */

	GMT_memset (&S, 1, struct GMT_SYMBOL);
	GMT_contlabel_init (GMT, &S.G, 0);
	xy_errors[GMT_X] = xy_errors[1] = 0;	/* These will be col # of where to find this info in data */
	GMT_init_fill (GMT, &black, 0.0, 0.0, 0.0);	/* Default fill for points, if needed */

	S.base = GMT->session.d_NaN;
	S.font = GMT->current.setting.font_annot[0];
	S.u = GMT->current.setting.proj_length_unit;
	S.justify = PSL_MC;
	Ctrl = New_psxy_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_psxy_parse (GMT, Ctrl, options, &S))) Return (error);

	/*---------------------------- This is the psxy main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	if (Ctrl->E.active && S.symbol == GMT_SYMBOL_LINE)	/* Assume user only wants error bars */
		S.symbol = GMT_SYMBOL_NONE;
	/* Do we plot actual symbols, or lines */
	not_line = (S.symbol != GMT_SYMBOL_FRONT && S.symbol != GMT_SYMBOL_QUOTED_LINE && S.symbol != GMT_SYMBOL_LINE);
	if (Ctrl->E.active) {	/* Set error bar parameters */
		j = 2;	/* Normally, error bar related columns start in column 2 */
		if (Ctrl->E.xbar != EBAR_NONE) { xy_errors[GMT_X] = j;	j += error_cols[Ctrl->E.xbar]; error_type[GMT_X] = Ctrl->E.xbar;}
		if (Ctrl->E.ybar != EBAR_NONE) { xy_errors[GMT_Y] = j;	j += error_cols[Ctrl->E.ybar]; error_type[GMT_Y] = Ctrl->E.ybar;}
		if (!(xy_errors[GMT_X] || xy_errors[GMT_Y])) {	/* Default is plain error bars for both */
			def_err_xy = true;
			xy_errors[GMT_X] = 2;	/* Assumes xy input, later check for -: */
			xy_errors[GMT_Y] = 3;
			error_type[GMT_X] = error_type[GMT_Y] = EBAR_NORMAL;
		}
	}

	if (Ctrl->T.active) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Warning: Option -T ignores all input files\n");

	get_rgb = (not_line && Ctrl->C.active);	/* Need to read z-vales from input data file */
	read_symbol = (S.symbol == GMT_SYMBOL_NOT_SET);	/* Only for ASCII input */
	polygon = (S.symbol == GMT_SYMBOL_LINE && (Ctrl->G.active || Ctrl->L.polygon) && !Ctrl->L.anchor);
	if (S.symbol == GMT_SYMBOL_DOT) penset_OK = false;	/* Dots have no outline */

	current_pen = default_pen = Ctrl->W.pen;
	current_fill = default_fill = (S.symbol == GMT_SYMBOL_DOT && !Ctrl->G.active) ? black : Ctrl->G.fill;
	default_outline = Ctrl->W.active;
	if (Ctrl->I.active) {
		GMT_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
		GMT_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
	}

	Ctrl->E.size *= 0.5;	/* Since we draw half-way in either direction */

	if (Ctrl->C.active) {
		if ((P = GMT_Read_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (get_rgb) n_cols_start++;
	}
	if (Ctrl->L.anchor == PSXY_POL_SYMM_DEV) n_cols_start += 1;
	if (Ctrl->L.anchor == PSXY_POL_ASYMM_DEV || Ctrl->L.anchor == PSXY_POL_ASYMM_ENV) n_cols_start += 2;
	
	/* For most symbols, the data columns beyond two will be dimensions that either have the units appended (e.g., 2c)
	 * or they are assumed to be in the current measure unit (PROJ_LENGTH_UNIT).  We therefore set the in_col_type to be
	 * GMT_IS_DIMENSION for these so that unit conversions are handled correctly.  However, some symbols also require
	 * angles via the input data file.  S.n_nondim and S.nondim_col are used to reset the in_col_type back to GMT_IS_FLOAT
	 * for those columns are expected to contain angles.  When NO SYMBOL is specified in -S we must parse the ASCII data
	 * record to determine the symbol, and this happens AFTER we have already converted the dimensions.  We must therefore
	 * undo this scaling based on what columns might be angles. */

	/* Extra columns 1, 2 and 3 */
	ex1 = (get_rgb) ? 3 : 2;
	ex2 = (get_rgb) ? 4 : 3;
	ex3 = (get_rgb) ? 5 : 4;
	pos2x = ex1 + GMT->current.setting.io_lonlat_toggle[GMT_IN];	/* Column with a 2nd longitude (for VECTORS with two sets of coordinates) */
	pos2y = ex2 - GMT->current.setting.io_lonlat_toggle[GMT_IN];	/* Column with a 2nd latitude (for VECTORS with two sets of coordinates) */

	if (Ctrl->E.active) {
		if (S.read_size) GMT->current.io.col_type[GMT_IN][ex1] = GMT_IS_DIMENSION;	/* Must read symbol size from data record */
		if (def_err_xy && GMT->current.setting.io_lonlat_toggle[GMT_IN]) {	/* With -:, -E should become -Eyx */
			uint_swap (xy_errors[GMT_X], xy_errors[GMT_Y]);
			uint_swap (error_type[GMT_X], error_type[GMT_Y]);
		}
		if (xy_errors[GMT_X]) n_cols_start += error_cols[error_type[GMT_X]], error_x = true;
		if (xy_errors[GMT_Y]) n_cols_start += error_cols[error_type[GMT_Y]], error_y = true;
		xy_errors[GMT_X] += (S.read_size + get_rgb);	/* Move 0-2 columns over */
		xy_errors[GMT_Y] += (S.read_size + get_rgb);
	}
	else	/* Here we have the usual x y [z] [size] [other args] [symbol] record */
		for (j = n_cols_start; j < 6; j++) GMT->current.io.col_type[GMT_IN][j] = GMT_IS_DIMENSION;		/* Since these may have units appended */
	for (j = 0; j < S.n_nondim; j++) GMT->current.io.col_type[GMT_IN][S.nondim_col[j]+get_rgb] = GMT_IS_FLOAT;	/* Since these are angles, not dimensions */

	n_needed = n_cols_start + S.n_required;
	error += GMT_check_binary_io (GMT, n_needed);
	GMT_Report (API, GMT_MSG_DEBUG, "Operation will require %d input columns [n_cols_start = %d]\n", n_needed, n_cols_start);

	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);
	if (S.u_set) {	/* When -Sc<unit> is given we temporarily reset the system unit to these units so conversions will work */
		save_u = GMT->current.setting.proj_length_unit;
		GMT->current.setting.proj_length_unit = S.u;
	}

	if (S.G.delay) GMT->current.ps.nclip = +2;	/* Signal that this program initiates clipping that will outlive this process */
	
	PSL = GMT_plotinit (GMT, options);
	if (Ctrl->T.active) {
		GMT_plotend (GMT);
		Return (GMT_OK);
	}
	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	GMT_plotcanvas (GMT);	/* Fill canvas if requested */

	if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {
		if (GMT_contlabel_prep (GMT, &S.G, NULL)) Return (EXIT_FAILURE);
		penset_OK = false;	/* The pen for quoted lines are set within the PSL code itself so we dont do it here in psxy */
		if (S.G.delay) GMT_map_basemap (GMT);	/* Must do it here due to clipping */
	}

	if ((Ctrl->A.active && Ctrl->A.mode == 0) || !GMT_is_geographic (GMT, GMT_IN)) GMT->current.map.path_mode = GMT_LEAVE_PATH;	/* Turn off resampling */
#ifdef DEBUG
	/* Change default step size (in degrees) used for interpolation of line segments along great circles (if requested) */
	if (Ctrl->A.active) Ctrl->A.step = Ctrl->A.step / GMT->current.proj.scale[GMT_X] / GMT->current.proj.M_PR_DEG;
#endif
	if ((GMT_IS_CONICAL(GMT) && GMT_360_RANGE (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]))) {	/* Must turn clipping on for 360-range conical */
		/* Special case of 360-range conical (which is periodic but do not touch at w=e) so we must clip to ensure nothing is plotted in the gap between west and east border */
		clip_set = true;
	}
	else if (not_line && (Ctrl->N.mode == PSXY_CLIP_REPEAT || Ctrl->N.mode == PSXY_CLIP_NO_REPEAT))	/* Only set clip if plotting symbols and -N not used */
		clip_set = true;
	if (clip_set) GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);
	if (S.symbol == GMT_SYMBOL_TEXT && Ctrl->G.active && !Ctrl->W.active) PSL_setcolor (PSL, current_fill.rgb, PSL_IS_FILL);
	if (S.symbol == GMT_SYMBOL_TEXT) GMT_setfont (GMT, &S.font);	/* Set the required font */
	if (S.symbol == GMT_SYMBOL_BARX && !S.base_set) S.base = GMT->common.R.wesn[XLO];	/* Default to west level for horizontal log10 bars */
	if (S.symbol == GMT_SYMBOL_BARY && !S.base_set) S.base = GMT->common.R.wesn[YLO];	/* Default to south level for vertical log10 bars */
	if ((S.symbol == GMT_SYMBOL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR) && S.v.status & GMT_VEC_JUST_S) {	/* One of the vector symbols, and require 2nd point */
		/* Reading 2nd coordinate so must set column types */
		GMT->current.io.col_type[GMT_IN][pos2x] = GMT->current.io.col_type[GMT_IN][GMT_X];
		GMT->current.io.col_type[GMT_IN][pos2y] = GMT->current.io.col_type[GMT_IN][GMT_Y];
	}
	if (S.symbol == GMT_SYMBOL_VECTOR && S.v.status & GMT_VEC_COMPONENTS)
		GMT->current.io.col_type[GMT_IN][pos2y] = GMT_IS_FLOAT;	/* Just the users dy component, not length */
	if (S.symbol == GMT_SYMBOL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR || S.symbol == GMT_SYMBOL_MARC ) {	/* One of the vector symbols */
		geovector = (S.symbol == GMT_SYMBOL_GEOVECTOR);
		if ((S.v.status & GMT_VEC_FILL) == 0) Ctrl->G.active = false;	/* Want no fill so override -G*/
		if (S.v.status & GMT_VEC_FILL) S.v.fill = current_fill;		/* Override -G<fill> (if set) with specified head fill */
	}
	bcol = (S.read_size) ? ex2 : ex1;
	if (S.symbol == GMT_SYMBOL_BARX && S.base_set == 2) GMT->current.io.col_type[GMT_IN][bcol] = GMT->current.io.col_type[GMT_IN][GMT_X];
	if (S.symbol == GMT_SYMBOL_BARY && S.base_set == 2) GMT->current.io.col_type[GMT_IN][bcol] = GMT->current.io.col_type[GMT_IN][GMT_Y];
	
	if (penset_OK) GMT_setpen (GMT, &current_pen);

	fill_active = Ctrl->G.active;	/* Make copies because we will change the values */
	outline_active =  Ctrl->W.active;
	if (not_line && !outline_active && !fill_active && !get_rgb) outline_active = true;	/* If no fill nor outline for symbols then turn outline on */

	if (Ctrl->D.active) PSL_setorigin (PSL, Ctrl->D.dx, Ctrl->D.dy, 0.0, PSL_FWD);	/* Shift plot a bit */

	old_is_world = GMT->current.map.is_world;
	geometry = not_line ? GMT_IS_POINT : ((polygon) ? GMT_IS_POLY: GMT_IS_LINE);
	in = GMT->current.io.curr_rec;
	if (read_symbol) {	/* If symbol info is given we must process text records */
		set_type = GMT_IS_TEXTSET;
		read_mode = GMT_READ_TEXT;
	}
	else {	/* Here we can process data records (ASCII or binary) */
		set_type = GMT_IS_DATASET;
		read_mode = GMT_READ_DOUBLE;
	}
	if ((error = GMT_set_cols (GMT, GMT_IN, n_needed)) != GMT_OK) {
		Return (error);
	}
	if (not_line) {	/* Symbol part (not counting GMT_SYMBOL_FRONT and GMT_SYMBOL_QUOTED_LINE) */
		bool periodic = false;
		unsigned int n_warn[3] = {0, 0, 0}, warn, item, n_times;
		double in2[6] = {0.0, 0.0, 0.0, 0.0, 0.0, 0.0}, *p_in = GMT->current.io.curr_rec;
		double xpos[2], width;

		/* Determine if we need to worry about repeating periodic symbols */
		if ((Ctrl->N.mode == PSXY_CLIP_REPEAT || Ctrl->N.mode == PSXY_NO_CLIP_REPEAT) && GMT_360_RANGE (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]) && GMT_is_geographic (GMT, GMT_IN)) {
			/* Only do this for projection where west and east are split into two separate repeating boundaries */
			periodic = (GMT_IS_CYLINDRICAL (GMT) || GMT_IS_MISC (GMT));
		}
		n_times = (periodic) ? 2 : 1;	/* For periodic boundaries we plot each symbol twice to allow for periodic clipping */

		if (GMT_Init_IO (API, set_type, geometry, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Register data input */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, set_type, GMT_IN, GMT_HEADER_ON) != GMT_OK) {		/* Enables data input and sets access mode */
			Return (API->error);
		}
		PSL_command (GMT->PSL, "V\n");
		//GMT->current.map.is_world = !(S.symbol == GMT_SYMBOL_ELLIPSE && S.convert_angles);
		if ((S.symbol == GMT_SYMBOL_ELLIPSE || S.symbol == GMT_SYMBOL_ROTRECT) && S.n_required <= 1) p_in = in2;
		do {	/* Keep returning records until we reach EOF */
			if ((record = GMT_Get_Record (API, read_mode, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (GMT_REC_IS_ERROR (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (GMT_REC_IS_TABLE_HEADER (GMT)) {	/* Skip table headers */
					continue;
				}
				if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
					break;
				else if (GMT_REC_IS_SEGMENT_HEADER (GMT)) {			/* Parse segment headers */
					PSL_comment (PSL, "Segment header: %s\n", GMT->current.io.segment_header);
					change = GMT_parse_segment_header (GMT, GMT->current.io.segment_header, P, &fill_active, &current_fill, default_fill, &outline_active, &current_pen, default_pen, default_outline, NULL);
					if (Ctrl->I.active) {
						GMT_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
						GMT_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
					}
					if (read_symbol) API->object[API->current_item[GMT_IN]]->n_expected_fields = GMT_MAX_COLUMNS;
					if (GMT_parse_segment_item (GMT, GMT->current.io.segment_header, "-S", s_args)) {	/* Found -Sargs */
						if (!(s_args[0] == 'q'|| s_args[0] == 'f')) { /* Update parameters */
							GMT_parse_symbol_option (GMT, s_args, &S, 0, false);
						}
						else
							GMT_Report (API, GMT_MSG_NORMAL, "Segment header tries to switch to a line symbol like quoted line or fault - ignored\n");
					}
					continue;
				}
			}

			/* Data record to process */

			n_total_read++;

			if (read_symbol) {	/* Must do special processing */
				int save[2];
				bool col_reset = false;
				text_rec = (char *)record;
				
				/* First establish the symbol type given at the end of the record */
				GMT_chop (text_rec);	/* Get rid of \n \r */
				i = (unsigned int)strlen (text_rec) - 1;
				while (text_rec[i] && !strchr (" \t", (int)text_rec[i])) i--;
				GMT_parse_symbol_option (GMT, &text_rec[i+1], &S, 0, false);
				
				for (j = n_cols_start; j < 6; j++) GMT->current.io.col_type[GMT_IN][j] = GMT_IS_DIMENSION;		/* Since these may have units appended */
				for (j = 0; j < S.n_nondim; j++) GMT->current.io.col_type[GMT_IN][S.nondim_col[j]+get_rgb] = GMT_IS_FLOAT;	/* Since these are angles, not dimensions */
				if ((S.symbol == GMT_SYMBOL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR) && S.v.status & GMT_VEC_JUST_S) {	/* One of the vector symbols, and require 2nd point */
					/* Reading 2nd point coordinates so must set column types to be the same as for x,y */
					col_reset = true;
					save[GMT_X] = GMT->current.io.col_type[GMT_IN][pos2x];
					save[GMT_Y] = GMT->current.io.col_type[GMT_IN][pos2y];
					GMT->current.io.col_type[GMT_IN][pos2x] = GMT->current.io.col_type[GMT_IN][GMT_X];
					GMT->current.io.col_type[GMT_IN][pos2y] = GMT->current.io.col_type[GMT_IN][GMT_Y];
				}
				/* Now convert the leading text items to doubles; col_type[GMT_IN] might have been updated above */
				if (GMT_conv_intext2dbl (GMT, text_rec, 6U)) {	/* Max 6 columns needs to be parsed */
					GMT_Report (API, GMT_MSG_NORMAL, "Record %d had bad x and/or y coordinates, skipped)\n", n_total_read);
					continue;
				}
				if (S.symbol == GMT_SYMBOL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR || S.symbol == GMT_SYMBOL_MARC) {	/* One of the vector symbols */
					if (S.v.status & GMT_VEC_OUTLINE2) {
						current_pen = S.v.pen, Ctrl->W.active = true;	/* Override -W (if set) with specified pen */
					}
					else if (S.v.status & GMT_VEC_OUTLINE) {
						current_pen = default_pen, Ctrl->W.active = true;	/* Return to default pen */
					}
					if (S.v.status & GMT_VEC_FILL2) {
						current_fill = S.v.fill;	/* Override -G<fill> with specified head fill */
						if (S.v.status & GMT_VEC_FILL) Ctrl->G.active = true;
					}
					else if (S.v.status & GMT_VEC_FILL) {
						current_fill = default_fill, Ctrl->G.active = true;	/* Return to default fill */
					}
				}
				else if (S.symbol == GMT_SYMBOL_DOT && !Ctrl->G.active) {	/* Must switch on default black fill */
					current_fill = black;
				}
				if (col_reset) {	/* Reset type above since there may not be +s in the next record */
					GMT->current.io.col_type[GMT_IN][pos2x] = save[GMT_X];
					GMT->current.io.col_type[GMT_IN][pos2y] = save[GMT_Y];
				}
			}

			if (get_rgb) {
				GMT_get_fill_from_z (GMT, P, in[GMT_Z], &current_fill);
				if (Ctrl->I.active) GMT_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
				if (P->skip) continue;	/* Chosen cpt file indicates skip for this z */
			}

			if (!Ctrl->N.active && S.symbol != GMT_SYMBOL_BARX && S.symbol != GMT_SYMBOL_BARY) {
				/* Skip points outside map */
				GMT_map_outside (GMT, in[GMT_X], in[GMT_Y]);
				if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
			}

			if (GMT_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y)) continue;	/* NaNs on input */

			if (GMT_is_dnan (plot_x)) {	/* Transformation of x yielded a NaN (e.g. log (-ve)) */
				GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Data point with x = NaN near line %d\n", n_total_read);
				continue;
			}
			if (GMT_is_dnan (plot_y)) {	/* Transformation of y yielded a NaN (e.g. log (-ve)) */
				GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Data point with y = NaN near line %d\n", n_total_read);
				continue;
			}

			if (Ctrl->E.active) {
				if (Ctrl->E.mode) GMT_rgb_copy (Ctrl->E.pen.rgb, current_fill.rgb);
				if (Ctrl->E.mode & 1) GMT_rgb_copy (current_fill.rgb, GMT->session.no_rgb);
				GMT_setpen (GMT, &Ctrl->E.pen);
				if (error_x) {
					if (error_type[GMT_X] < EBAR_WHISKER)
						plot_x_errorbar (GMT, PSL, in[GMT_X], in[GMT_Y], &in[xy_errors[GMT_X]], Ctrl->E.size, n_total_read, error_type[GMT_X]);
					else
						plot_x_whiskerbar (GMT, PSL, plot_x, in[GMT_Y], &in[xy_errors[GMT_X]], Ctrl->E.size, current_fill.rgb, n_total_read, error_type[GMT_X]);
				}
				if (error_y) {
					if (error_type[GMT_Y] < EBAR_WHISKER)
						plot_y_errorbar (GMT, PSL, in[GMT_X], in[GMT_Y], &in[xy_errors[GMT_Y]], Ctrl->E.size, n_total_read, error_type[GMT_Y]);
					else
						plot_y_whiskerbar (GMT, PSL, in[GMT_X], plot_y, &in[xy_errors[GMT_Y]], Ctrl->E.size, current_fill.rgb, n_total_read, error_type[GMT_Y]);
				}
			}

			if (Ctrl->W.mode) {
				GMT_rgb_copy (Ctrl->W.pen.rgb, current_fill.rgb);
				current_pen = Ctrl->W.pen;
			}
			if (Ctrl->W.mode & 1) GMT_rgb_copy (current_fill.rgb, GMT->session.no_rgb);

			if (geovector) {
				if (get_rgb) S.v.fill = current_fill;
			}
			else {	/* Vectors do it separately */
				GMT_setfill (GMT, &current_fill, outline_active);
				GMT_setpen (GMT, &current_pen);
			}
			
			if (S.symbol == GMT_SYMBOL_ELLIPSE || S.symbol == GMT_SYMBOL_ROTRECT) {	/* Ellipses and rectangles */
				if (S.n_required == 0) {	/* Degenerate ellipse or rectangle, got diameter via S.size_x */
					in2[ex2] = in2[ex3] = S.size_x;	/* Duplicate diameter as major and minor axes */
				}
				else if (S.n_required == 1) {	/* Degenerate ellipse or rectangle, expect single diameter via input */
					in2[ex2] = in2[ex3] = in[ex1];	/* Duplicate diameter as major and minor axes */
				}
			}

			if (S.base_set == 2) {
				bcol = (S.read_size) ? ex2 : ex1;
				S.base = in[bcol];	/* Got base from input column */
			}
			if (S.read_size) S.size_x = in[ex1];	/* Got size from input column */
			dim[0] = S.size_x;

			/* For global periodic maps, symbols plotted close to a periodic boundary may be clipped and should appear
			 * at the other periodic boundary.  We try to handle this below */
			
			xpos[0] = plot_x;
			if (periodic) {
				width = 2.0 * GMT_half_map_width (GMT, plot_y);	/* Width of map at current latitude (not all projections have straight w/e boundaries */
				if (plot_x < GMT->current.map.half_width)     /* Might reappear at right edge */
					xpos[1] = xpos[0] + width;	/* Outside the right edge */
				else      /* Might reappear at left edge */
			              xpos[1] = xpos[0] - width;         /* Outside the left edge */
			}
			for (item = 0; item < n_times; item++) {	/* Plot symbols once or twice, depending on periodic (see above) */
				switch (S.symbol) {
				case GMT_SYMBOL_NONE:
					break;
				case GMT_SYMBOL_BARX:
					if (!Ctrl->N.active) in[GMT_X] = MAX (GMT->common.R.wesn[XLO], MIN (in[GMT_X], GMT->common.R.wesn[XHI]));
					if (S.user_unit[GMT_X]) {	/* Width measured in y units */
						GMT_geo_to_xy (GMT, S.base, in[GMT_Y] - 0.5 * dim[0], &x_1, &y_1);
						GMT_geo_to_xy (GMT, in[GMT_X], in[GMT_Y] + 0.5 * dim[0], &x_2, &y_2);
					}
					else {
						GMT_geo_to_xy (GMT, S.base, GMT->common.R.wesn[YLO], &x_1, &y_1);	/* Zero x level for horizontal bars */
						x_2 = plot_x;
						y_1 = plot_y - 0.5 * dim[0]; y_2 = plot_y + 0.5 * dim[0];
					}
					PSL_plotbox (PSL, x_1, y_1, x_2, y_2);
					break;
				case GMT_SYMBOL_BARY:
					if (!Ctrl->N.active) in[GMT_Y] = MAX (GMT->common.R.wesn[YLO], MIN (in[GMT_Y], GMT->common.R.wesn[YHI]));
					if (S.user_unit[GMT_X]) {	/* Width measured in x units */
						GMT_geo_to_xy (GMT, in[GMT_X] - 0.5 * dim[0], S.base, &x_1, &y_1);
						GMT_geo_to_xy (GMT, in[GMT_X] + 0.5 * dim[0], in[GMT_Y], &x_2, &y_2);
					}
					else {
						GMT_geo_to_xy (GMT, GMT->common.R.wesn[XLO], S.base, &x_1, &y_1);	/* Zero y level for vertical bars */
						x_1 = plot_x - 0.5 * dim[0]; x_2 = plot_x + 0.5 * dim[0];
						y_2 = plot_y;
					}
					PSL_plotbox (PSL, x_1, y_1, x_2, y_2);
					break;
				case GMT_SYMBOL_CROSS:
				case GMT_SYMBOL_PLUS:
				case GMT_SYMBOL_DOT:
				case GMT_SYMBOL_XDASH:
				case GMT_SYMBOL_YDASH:
				case GMT_SYMBOL_STAR:
				case GMT_SYMBOL_CIRCLE:
				case GMT_SYMBOL_SQUARE:
				case GMT_SYMBOL_HEXAGON:
				case GMT_SYMBOL_PENTAGON:
				case GMT_SYMBOL_OCTAGON:
				case GMT_SYMBOL_TRIANGLE:
				case GMT_SYMBOL_INVTRIANGLE:
				case GMT_SYMBOL_DIAMOND:
					PSL_plotsymbol (PSL, xpos[item], plot_y, dim, S.symbol);
					break;
				case GMT_SYMBOL_RNDRECT:
					dim[2] = in[ex3];
					if (GMT_is_dnan (dim[2])) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Rounded rectangle corner radius = NaN near line %d\n", n_total_read);
						continue;
					}
				case GMT_SYMBOL_RECT:
					dim[0] = in[ex1];
					if (GMT_is_dnan (dim[0])) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Rounded rectangle width = NaN near line %d\n", n_total_read);
						continue;
					}
					dim[1] = in[ex2];
					if (GMT_is_dnan (dim[1])) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Rounded rectangle height = NaN near line %d\n", n_total_read);
						continue;
					}
					PSL_plotsymbol (PSL, plot_x, plot_y, dim, S.symbol);
					break;
				case GMT_SYMBOL_ROTRECT:
				case GMT_SYMBOL_ELLIPSE:
					if (GMT_is_dnan (p_in[ex1])) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Ellipse/Rectangle angle = NaN near line %d\n", n_total_read);
						continue;
					}
					if (GMT_is_dnan (p_in[ex2])) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Ellipse/Rectangle width or major axis = NaN near line %d\n", n_total_read);
						continue;
					}
					if (GMT_is_dnan (p_in[ex3])) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Ellipse/Rectangle height or minor axis = NaN near line %d\n", n_total_read);
						continue;
					}
					if (!S.convert_angles) {	/* Got axes in current plot units, change to inches */
						dim[0] = p_in[ex1];
						GMT_flip_angle_d (GMT, &dim[0]);
						dim[1] = p_in[ex2];
						dim[2] = p_in[ex3];
						PSL_plotsymbol (PSL, xpos[item], plot_y, dim, S.symbol);
					}
					else if (!GMT_is_geographic (GMT, GMT_IN)) {	/* Got axes in user units, change to inches */
						dim[0] = 90.0 - p_in[ex1];	/* Cartesian azimuth */
						GMT_flip_angle_d (GMT, &dim[0]);
						dim[1] = p_in[ex2] * GMT->current.proj.scale[GMT_X];
						dim[2] = p_in[ex3] * GMT->current.proj.scale[GMT_X];
						PSL_plotsymbol (PSL, xpos[item], plot_y, dim, S.symbol);
					}
					else if (S.symbol == GMT_SYMBOL_ELLIPSE)	/* Got axis in km */
						GMT_geo_ellipse (GMT, in[GMT_X], in[GMT_Y], p_in[ex2], p_in[ex3], p_in[ex1]);
					else
						GMT_geo_rectangle (GMT, in[GMT_X], in[GMT_Y], p_in[ex2], p_in[ex3], p_in[ex1]);
					break;
				case GMT_SYMBOL_TEXT:
					if (Ctrl->G.active && !outline_active)
						PSL_setcolor (PSL, current_fill.rgb, PSL_IS_FILL);
					else if (!Ctrl->G.active)
						PSL_setfill (PSL, GMT->session.no_rgb, outline_active);
					(void) GMT_setfont (GMT, &S.font);
					PSL_plottext (PSL, xpos[item], plot_y, dim[0] * PSL_POINTS_PER_INCH, S.string, 0.0, S.justify, outline_active);
					break;
				case GMT_SYMBOL_VECTOR:
					GMT_init_vector_param (GMT, &S, false, false, NULL, false, NULL);	/* Update vector head parameters */
					if (S.v.status & GMT_VEC_COMPONENTS)	/* Read dx, dy in user units */
						length = hypot (in[ex1+S.read_size], in[ex2+S.read_size]) * S.v.comp_scale;
					else
						length = in[ex2+S.read_size];
					if (GMT_is_dnan (length)) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Vector length = NaN near line %d\n", n_total_read);
						continue;
					}
					if (S.v.status & GMT_VEC_COMPONENTS)	/* Read dx, dy in user units */
						d = d_atan2d (in[ex2+S.read_size], in[ex1+S.read_size]);
					else
						d = in[ex1+S.read_size];
						
					if (!S.convert_angles)	/* Use direction as given */
						direction = d;
					else if (!GMT_is_geographic (GMT, GMT_IN))	/* Cartesian angle; change to azimuth */
						direction = 90.0 - d;
					else	/* Convert geo azimuth to map direction */
						direction = GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, d);

					if (GMT_is_dnan (direction)) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Vector direction = NaN near line %d\n", n_total_read);
						continue;
					}
					if (S.v.status & GMT_VEC_JUST_S) {	/* Got coordinates of tip instead of dir/length */
						GMT_geo_to_xy (GMT, in[pos2x], in[pos2y], &x_2, &y_2);
						if (GMT_is_dnan (x_2) || GMT_is_dnan (y_2)) {
							GMT_Report (API, GMT_MSG_NORMAL, "Warning: Vector head coordinates contain NaNs near line %d. Skipped\n", n_total_read);
							continue;
						}
						if (item == 1) {    /* Deal with periodicity */
							if (x_2 < GMT->current.map.half_width)     /* Might reappear at right edge */
								x_2 += width;	/* Outside the right edge */
							else      /* Might reappear at left edge */
					              		x_2 -= width;         /* Outside the left edge */
						}
						length = hypot (plot_x - x_2, plot_y - y_2);	/* Compute vector length in case of shrinking */
					}
					else {	/* Compute tip coordinates from tail and length */
						GMT_flip_angle_d (GMT, &direction);
						sincosd (direction, &s, &c);
						x_2 = xpos[item] + length * c;
						y_2 = plot_y + length * s;
						justify = GMT_vec_justify (S.v.status);	/* Return justification as 0-2 */
						if (justify) {	/* Meant to center the vector at center (1) or tip (2) */
							dx = justify * 0.5 * (x_2 - xpos[item]);	dy = justify * 0.5 * (y_2 - plot_y);
							xpos[item] -= dx;		plot_y -= dy;
							x_2 -= dx;		y_2 -= dy;
						}
					}
					if (S.v.parsed_v4 && GMT_compat_check (GMT, 4)) {	/* Got v_width directly from V4 syntax so no messing with it here if under compatibility */
						/* But have to improvise as far as outline|fill goes... */
						if (outline_active) S.v.status |= PSL_VEC_OUTLINE;	/* Choosing to draw head outline */
						if (fill_active) S.v.status |= PSL_VEC_FILL;		/* Choosing to fill head */
						if (!(S.v.status & PSL_VEC_OUTLINE) && !(S.v.status & PSL_VEC_FILL)) S.v.status |= PSL_VEC_OUTLINE;	/* Gotta do something */
					}
					else
						S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					s = (length < S.v.v_norm) ? length / S.v.v_norm : 1.0;
					dim[0] = x_2, dim[1] = y_2;
					dim[2] = s * S.v.v_width, dim[3] = s * S.v.h_length, dim[4] = s * S.v.h_width;
					dim[5] = GMT->current.setting.map_vector_shape;
					dim[6] = (double)S.v.status;
					dim[7] = (double)S.v.v_kind[0];	dim[8] = (double)S.v.v_kind[1];
					PSL_plotsymbol (PSL, xpos[item], plot_y, dim, PSL_VECTOR);
					break;
				case GMT_SYMBOL_GEOVECTOR:
					GMT_init_vector_param (GMT, &S, true, Ctrl->W.active, &Ctrl->W.pen, Ctrl->G.active, &Ctrl->G.fill);	/* Update vector head parameters */
					if (S.v.status & GMT_VEC_OUTLINE2)
						S.v.v_width = (float)(S.v.pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					else
						S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					if (GMT_is_dnan (in[ex1+S.read_size])) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Geovector azimuth = NaN near line %d\n", n_total_read);
						continue;
					}
					if (GMT_is_dnan (in[ex2+S.read_size])) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Geovector length = NaN near line %d\n", n_total_read);
						continue;
					}
					warn = GMT_geo_vector (GMT, in[GMT_X], in[GMT_Y], in[ex1+S.read_size], in[ex2+S.read_size], &current_pen, &S);
					n_warn[warn]++;
					break;
				case GMT_SYMBOL_MARC:
					GMT_init_vector_param (GMT, &S, false, false, NULL, false, NULL);	/* Update vector head parameters */
					S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
					dim[0] = in[ex1+S.read_size];
					dim[1] = in[ex2+S.read_size];
					dim[2] = in[ex3+S.read_size];
					length = fabs (dim[2]-dim[1]);	/* Arc length in degrees */
					if (GMT_is_dnan (length)) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Math angle arc length = NaN near line %d\n", n_total_read);
						continue;
					}
					s = (length < S.v.v_norm) ? length / S.v.v_norm : 1.0;
					dim[3] = s * S.v.h_length, dim[4] = s * S.v.h_width, dim[5] = s * S.v.v_width;
					dim[6] = GMT->current.setting.map_vector_shape;
					dim[7] = (double)S.v.status;
					dim[8] = (double)S.v.v_kind[0];	dim[9] = (double)S.v.v_kind[1];
					PSL_plotsymbol (PSL, xpos[item], plot_y, dim, S.symbol);
					break;
				case GMT_SYMBOL_WEDGE:
					if (GMT_is_dnan (in[ex1+S.read_size])) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Wedge start angle = NaN near line %d\n", n_total_read);
						continue;
					}
					if (GMT_is_dnan (in[ex2+S.read_size])) {
						GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Wedge stop angle = NaN near line %d\n", n_total_read);
						continue;
					}
					if (!S.convert_angles) {
						dim[1] = in[ex1+S.read_size];
						dim[2] = in[ex2+S.read_size];
					}
					else if (!GMT_is_geographic (GMT, GMT_IN)) {
						/* Note that the direction of the arc gets swapped when converting from azimuth */
						dim[2] = 90.0 - in[ex1+S.read_size];
						dim[1] = 90.0 - in[ex2+S.read_size];
					}
					else {
						dim[2] = GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, 90.0 - in[ex1+S.read_size]);
						dim[1] = GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, 90.0 - in[ex2+S.read_size]);
					}
					dim[0] *= 0.5;
					PSL_plotsymbol (PSL, xpos[item], plot_y, dim, S.symbol);
					break;
				case GMT_SYMBOL_CUSTOM:
					for (j = 0; S.custom->type && j < S.n_required; j++) {	/* Deal with any geo-angles first */
						dim[j+1] = (S.custom->type[j] == GMT_IS_GEOANGLE) ? GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, 90.0 - in[ex1+S.read_size+j]) : in[ex1+S.read_size+j];
					}
					if (!S.custom->start) S.custom->start = (get_rgb) ? 3 : 2;
					GMT_draw_custom_symbol (GMT, xpos[item], plot_y, dim, S.custom, &current_pen, &current_fill, outline_active);
					break;
			}
			}
		} while (true);
		PSL_command (GMT->PSL, "U\n");
		if (n_warn[1]) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: %d vector heads had length exceeding the vector length and were skipped. Consider the +n<norm> modifier to -S\n", n_warn[1]);
		if (n_warn[2]) GMT_Report (API, GMT_MSG_VERBOSE, "Warning: %d vector heads had to be scaled more than implied by +n<norm> since they were still too long. Consider changing the +n<norm> modifier to -S\n", n_warn[2]);
		
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
			Return (API->error);
		}
	}
	else {	/* Line/polygon part */
		uint64_t seg;
		bool duplicate;
		struct GMT_DATASET *D = NULL;	/* Pointer to GMT multisegment table(s) */

		if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Register data input */
			Return (API->error);
		}
		if (S.symbol == GMT_SYMBOL_QUOTED_LINE && S.G.segmentize) {	/* Special quoted line where each point-pair should be considered a line segment */
			struct GMT_DATASET *Dtmp = NULL;	/* Pointer to GMT multisegment table(s) */
			if ((Dtmp = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
				Return (API->error);
			}
			D = GMT_segmentize_data (GMT, Dtmp);	/* Segmentize the data */
			if (GMT_Destroy_Data (API, &Dtmp) != GMT_OK) {	/* Be gone with the original */
				Return (API->error);
			}
		}
		else if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
			Return (API->error);
		}

		for (tbl = 0; tbl < D->n_tables; tbl++) {
			if (D->table[tbl]->n_headers && S.G.label_type == GMT_LABEL_IS_HEADER)	/* Get potential label from first header */
				GMT_extract_label (GMT, D->table[tbl]->header[0], S.G.label, NULL);

			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */

				L = D->table[tbl]->segment[seg];	/* Set shortcut to current segment */
				if (polygon && GMT_polygon_is_hole (L)) continue;	/* Holes are handled together with perimeters */

				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Plotting table %" PRIu64 " segment %" PRIu64 "\n", tbl, seg);
				
				/* We had here things like:	x = D->table[tbl]->segment[seg]->coord[GMT_X];
				 * but reallocating x below lead to disasters.  */

				change = GMT_parse_segment_header (GMT, L->header, P, &fill_active, &current_fill, default_fill, &outline_active, &current_pen, default_pen, default_outline, L->ogr);
				

				if (P && P->skip) continue;	/* Chosen cpt file indicates skip for this z */

				duplicate = (D->alloc_mode == GMT_ALLOC_EXTERNALLY && ((polygon && GMT_polygon_is_open (GMT, L->coord[GMT_X], L->coord[GMT_Y], L->n_rows)) || GMT->current.map.path_mode == GMT_RESAMPLE_PATH));
				if (duplicate)	/* Must duplicate externally allocated segment since it needs to be resampled below */
					L = GMT_duplicate_segment (GMT, D->table[tbl]->segment[seg]);
				
				if (L->header && L->header[0]) {
					PSL_comment (PSL, "Segment header: %s\n", L->header);
					if (GMT_parse_segment_item (GMT, L->header, "-S", s_args)) {	/* Found -S, which only can apply to front or quoted line */
						if ((S.symbol == GMT_SYMBOL_QUOTED_LINE && s_args[0] == 'q') || (S.symbol == GMT_SYMBOL_FRONT && s_args[0] == 'f')) { /* Update parameters */
							GMT_parse_symbol_option (GMT, s_args, &S, 0, false);
							if (change & 1) change -= 1;	/* Don't want polygon to be true later */
						}
						else if (S.symbol == GMT_SYMBOL_QUOTED_LINE || S.symbol == GMT_SYMBOL_FRONT)
							GMT_Report (API, GMT_MSG_NORMAL, "Segment header tries to switch from -S%c to another symbol (%s) - ignored\n", S.symbol, s_args);
						else	/* Probably just junk -S in header */
							GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Segment header contained -S%s - ignored\n", s_args);
					}
				}

				if (S.fq_parse) { /* Did not supply -Sf or -Sq in the segment header */
					if (S.symbol == GMT_SYMBOL_QUOTED_LINE) /* Did not supply -Sf in the segment header */
						GMT_Report (API, GMT_MSG_NORMAL, "Segment header did not supply enough parameters for -Sf; skipping this segment\n");
					else
						GMT_Report (API, GMT_MSG_NORMAL, "Segment header did not supply enough parameters for -Sq; skipping this segment\n");
					continue;
				}
				if (Ctrl->I.active) {
					GMT_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
					GMT_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
				}
				if (change & 4 && penset_OK) GMT_setpen (GMT, &current_pen);
				if (change & 1) polygon = true;
				if (change & 2 && !Ctrl->L.polygon) {
					polygon = false;
					PSL_setcolor (PSL, current_fill.rgb, PSL_IS_STROKE);
				}
				if (S.G.label_type == GMT_LABEL_IS_HEADER)	/* Get potential label from segment header */
					GMT_extract_label (GMT, L->header, S.G.label, L->ogr);

				if (polygon && GMT_polygon_is_open (GMT, L->coord[GMT_X], L->coord[GMT_Y], L->n_rows)) {
					/* Explicitly close polygon so that arc will work */
					size_t n_alloc;
					L->n_rows++;
					n_alloc = L->n_rows;
					GMT_malloc2 (GMT, L->coord[GMT_X], L->coord[GMT_Y], 0, &n_alloc, double);
					L->coord[GMT_X][L->n_rows-1] = L->coord[GMT_X][0];
					L->coord[GMT_Y][L->n_rows-1] = L->coord[GMT_Y][0];
				}

				if (GMT->current.map.path_mode == GMT_RESAMPLE_PATH)	/* Resample if spacing is too coarse */
					L->n_rows = GMT_fix_up_path (GMT, &L->coord[GMT_X], &L->coord[GMT_Y], L->n_rows, Ctrl->A.step, Ctrl->A.mode);

				if (polygon) {	/* Want a closed polygon (with or without fill and with or without outline) */
					GMT_setfill (GMT, &current_fill, outline_active);
					GMT_geo_polygons (GMT, L);
				}
				else if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {	/* Labeled lines are dealt with by the contour machinery */
					bool closed, split = false;
					uint64_t k0;
					if ((GMT->current.plot.n = GMT_geo_to_xy_line (GMT, L->coord[GMT_X], L->coord[GMT_Y], L->n_rows)) == 0) continue;
					S.G.line_pen = current_pen;
					/* GMT_geo_to_xy_line may have chopped the line into multiple pieces if exiting and reentering the domain */
					for (k0 = 0; !split && k0 < GMT->current.plot.n; k0++) if (GMT->current.plot.pen[k0] == PSL_MOVE) split = true;
					if (split) {	/* Must write out separate sections via GMT_hold_contour */
						uint64_t k1, n_section;
						size_t n_alloc;
						double *xxx = NULL, *yyy = NULL;
						k0 = 0;	/* Start of first section */
						while (k0 < GMT->current.plot.n) {	/* While more sections... */
							k1 = k0 + 1;	/* First point after anchor point */
							while (k1 < GMT->current.plot.n && GMT->current.plot.pen[k1] == PSL_DRAW) k1++;	/* Find next section anchor */
							/* k1 is now pointing to next move (anchor) point or it is GMT->current.plot.n */
							n_section = k1 - k0;	/* Number of points in this section */
							GMT_Report (API, GMT_MSG_DEBUG, "Quoted Sub-line starts at point %d and have length %d\n", (int)k0, (int)n_section);
							/* Make a copy of this section's coordinates */
							/* Get temp array of length n_section since GMT_hold_contour may change length */
							n_alloc = 0;
							GMT_malloc2 (GMT, xxx, yyy, n_section, &n_alloc, double);
							GMT_memcpy (xxx, &GMT->current.plot.x[k0], n_section, double);
							GMT_memcpy (yyy, &GMT->current.plot.y[k0], n_section, double);
							GMT_hold_contour (GMT, &xxx, &yyy, n_section, 0.0, "N/A", 'A', S.G.label_angle, false, false, &S.G);
							k0 = k1;	/* Goto start of next section */
							GMT_free (GMT, xxx);
							GMT_free (GMT, yyy);
						}
					}
					else {	/* Just one line, which may even be closed */
						closed = !(GMT_polygon_is_open (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n));
						GMT_hold_contour (GMT, &GMT->current.plot.x, &GMT->current.plot.y, GMT->current.plot.n, 0.0, "N/A", 'A', S.G.label_angle, closed, false, &S.G);
						GMT->current.plot.n_alloc = GMT->current.plot.n;	/* Since GMT_hold_contour reallocates to fit the array */
					}
				}
				else {	/* Plot line */
					uint64_t end;
					bool draw_line = true;
					if (Ctrl->L.anchor) {	/* Build a polygon in one of several ways */
						if (Ctrl->L.anchor == PSXY_POL_SYMM_DEV || Ctrl->L.anchor == PSXY_POL_ASYMM_DEV) {	/* Build envelope around y(x) from delta y values in 1 or 2 extra columns */
							uint64_t k, n, col = (Ctrl->L.anchor == PSXY_POL_ASYMM_DEV) ? 3 : 2;
							end = 2 * L->n_rows + 1;
							GMT_prep_tmp_arrays (GMT, end, 2);	/* Init or reallocate tmp vectors */
							/* First go in positive x direction and build part of envelope */
							GMT_memcpy (GMT->hidden.mem_coord[GMT_X], L->coord[GMT_X], L->n_rows, double);
							for (k = 0; k < L->n_rows; k++)
								GMT->hidden.mem_coord[GMT_Y][k] = L->coord[GMT_Y][k] - fabs (L->coord[2][k]);
							/* Then go in negative x direction and build rest of envelope */
							for (k = n = L->n_rows; k > 0; k--, n++) {
								GMT->hidden.mem_coord[GMT_X][n] = L->coord[GMT_X][k-1];
								GMT->hidden.mem_coord[GMT_Y][n] = L->coord[GMT_Y][k-1] + fabs (L->coord[col][k-1]);
							}
							/* Explicitly close polygon */
							GMT->hidden.mem_coord[GMT_X][end-1] = GMT->hidden.mem_coord[GMT_X][0];
							GMT->hidden.mem_coord[GMT_Y][end-1] = GMT->hidden.mem_coord[GMT_Y][0];
						}
						else if (Ctrl->L.anchor == PSXY_POL_ASYMM_ENV) {	/* Build envelope around y(x) from low and high 2 extra columns */
							uint64_t k, n;
							end = 2 * L->n_rows + 1;
							GMT_prep_tmp_arrays (GMT, end, 2);	/* Init or reallocate tmp vectors */
							/* First go in positive x direction and build part of envelope */
							GMT_memcpy (GMT->hidden.mem_coord[GMT_X], L->coord[GMT_X], L->n_rows, double);
							for (k = 0; k < L->n_rows; k++)
								GMT->hidden.mem_coord[GMT_Y][k] = L->coord[2][k];
							/* Then go in negative x direction and build rest of envelope */
							for (k = n = L->n_rows; k > 0; k--, n++) {
								GMT->hidden.mem_coord[GMT_X][n] = L->coord[GMT_X][k-1];
								GMT->hidden.mem_coord[GMT_Y][n] = L->coord[3][k-1];
							}
							/* Explicitly close polygon */
							GMT->hidden.mem_coord[GMT_X][end-1] = GMT->hidden.mem_coord[GMT_X][0];
							GMT->hidden.mem_coord[GMT_Y][end-1] = GMT->hidden.mem_coord[GMT_Y][0];
						}
						else {	/* First complete polygon via anchor points and paint the area, optionally with outline */
							uint64_t off = 0U;
							double value;
							end = L->n_rows;
							GMT_prep_tmp_arrays (GMT, end+3, 2);	/* Init or reallocate tmp vectors */
							/* First copy the given line segment */
							GMT_memcpy (GMT->hidden.mem_coord[GMT_X], L->coord[GMT_X], end, double);
							GMT_memcpy (GMT->hidden.mem_coord[GMT_Y], L->coord[GMT_Y], end, double);
							/* Now add 2 anchor points and explicitly close by repeating 1st point */
							switch (Ctrl->L.mode) {
								case XHI:	off = 1;	/* To select the x max entry */
								case XLO:
								case ZLO:
									value = (Ctrl->L.mode == ZLO) ? Ctrl->L.value : GMT->common.R.wesn[XLO+off];
									GMT->hidden.mem_coord[GMT_X][end] = GMT->hidden.mem_coord[GMT_X][end+1] = value;
									GMT->hidden.mem_coord[GMT_Y][end] = L->coord[GMT_Y][end-1];
									GMT->hidden.mem_coord[GMT_Y][end+1] = L->coord[GMT_Y][0];
									break;
								case YHI:	off = 1;	/* To select the y max entry */
								case YLO:
								case ZHI:
									value = (Ctrl->L.mode == ZHI) ? Ctrl->L.value : GMT->common.R.wesn[YLO+off];
									GMT->hidden.mem_coord[GMT_Y][end] = GMT->hidden.mem_coord[GMT_Y][end+1] = value;
									GMT->hidden.mem_coord[GMT_X][end] = L->coord[GMT_X][end-1];
									GMT->hidden.mem_coord[GMT_X][end+1] = L->coord[GMT_X][0];
									break;
							}
							/* Explicitly close polygon */
							GMT->hidden.mem_coord[GMT_X][end+2] = L->coord[GMT_X][0];
							GMT->hidden.mem_coord[GMT_Y][end+2] = L->coord[GMT_Y][0];
							end += 3;
						}
						/* Project and get ready */
						if ((GMT->current.plot.n = GMT_geo_to_xy_line (GMT, GMT->hidden.mem_coord[GMT_X], GMT->hidden.mem_coord[GMT_Y], end)) == 0) continue;
						if (Ctrl->L.outline) GMT_setpen (GMT, &Ctrl->L.pen);	/* Select separate pen for polygon outline */
						if (Ctrl->G.active)	/* Specify the fill, possibly set outline */
							GMT_setfill (GMT, &current_fill, Ctrl->L.outline);
						else	/* No fill, just outline */
							GMT_setfill (GMT, NULL, Ctrl->L.outline);
						PSL_plotpolygon (PSL, GMT->current.plot.x, GMT->current.plot.y, (int)GMT->current.plot.n);
						if (!Ctrl->W.active) draw_line = false;	/* Did not want to actually draw the main line */
						if (Ctrl->L.outline) GMT_setpen (GMT, &current_pen);	/* Reset the pen to what -W indicates */
					}
					if (draw_line) {
						if ((GMT->current.plot.n = GMT_geo_to_xy_line (GMT, L->coord[GMT_X], L->coord[GMT_Y], L->n_rows)) == 0) continue;
						GMT_plot_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n);
					}
				}
				if (S.symbol == GMT_SYMBOL_FRONT) { /* Must also draw fault crossbars */
					GMT_setfill (GMT, &current_fill, (S.f.f_pen == -1) ? false : true);
					GMT_draw_front (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n, &S.f);
					if (S.f.f_pen == 0) GMT_setpen (GMT, &current_pen);	/* Reinstate current pen */
				}
				if (duplicate)	/* Free duplicate segment */
					GMT_free_segment (GMT, &L, GMT_ALLOC_INTERNALLY);
			}
		}
		if (GMT_Destroy_Data (API, &D) != GMT_OK) {
			Return (API->error);
		}
	}

	if (S.u_set) GMT->current.setting.proj_length_unit = save_u;	/* Reset unit */

	if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {
		if (S.G.save_labels) {	/* Want to save the line label locations (lon, lat, angle, label) */
			if ((error = GMT_contlabel_save_begin (GMT, &S.G))) Return (error);
			if ((error = GMT_contlabel_save_end (GMT, &S.G))) Return (error);
		}
		GMT_contlabel_plot (GMT, &S.G);
		GMT_contlabel_free (GMT, &S.G);
	}

	if (Ctrl->D.active) PSL_setorigin (PSL, -Ctrl->D.dx, -Ctrl->D.dy, 0.0, PSL_FWD);	/* Reset shift */

	if (clip_set && !S.G.delay) GMT_map_clip_off (GMT);	/* We delay map clip off if text clipping was chosen via -Sq<args:+e */

	if (current_pen.style) PSL_setdash (PSL, NULL, 0);
	GMT->current.map.is_world = old_is_world;
	if (geovector) PSL->current.linewidth = 0.0;	/* Since we changed things under clip; this will force it to be set next */

	if (!S.G.delay) GMT_map_basemap (GMT);
	GMT_plane_perspective (GMT, -1, 0.0);
	GMT_plotend (GMT);

	Return (GMT_OK);
}
