/*--------------------------------------------------------------------
 *	$Id: psxy_func.c,v 1.17 2011-04-29 03:08:12 guru Exp $
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
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: psxy will read <x,y> points and plot symbols, lines,
 * or polygons on maps.
 */

#include "pslib.h"
#include "gmt.h"

/* Control structure for psxy */

struct PSXY_CTRL {
	struct A {	/* -A[m|p|step] */
		GMT_LONG active;
		GMT_LONG mode;
		double step;
	} A;
	struct C {	/* -C<cpt> */
		GMT_LONG active;
		char *file;
	} C;
	struct D {	/* -D<dx>/<dy> */
		GMT_LONG active;
		double dx, dy;
	} D;
	struct E {	/* -E[x|X][y|Y][cap][/[+|-]<pen>] */
		GMT_LONG active;
		GMT_LONG xbar, ybar;	/* 0 = not used, 1 = error bar, 2 = box-whisker, 3 notched box-whisker */
		GMT_LONG mode;	/* 0 = normal, 1 = -C applies to error pen color, 2 = -C applies to symbol fill & error pen color */
		double size;
		struct GMT_PEN pen;
	} E;
	struct G {	/* -G<fill> */
		GMT_LONG active;
		struct GMT_FILL fill;
	} G;
	struct I {	/* -I<intensity> */
		GMT_LONG active;
		double value;
	} I;
	struct L {	/* -L */
		GMT_LONG active;
	} L;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct S {	/* -S */
		GMT_LONG active;
		char *arg;
	} S;
	struct T {	/* -T */
		GMT_LONG active;
	} T;
	struct W {	/* -W<pen> */
		GMT_LONG active;
		GMT_LONG mode;	/* 0 = normal, 1 = -C applies to pen color only, 2 = -C applies to symbol fill & pen color */
		struct GMT_PEN pen;
	} W;
};

#define CAP_WIDTH		7.0	/* Error bar cap width */

EXTERN_MSC GMT_LONG GMT_parse_symbol_option (struct GMT_CTRL *C, char *text, struct GMT_SYMBOL *p, GMT_LONG mode, GMT_LONG cmd);

void *New_psxy_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSXY_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSXY_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->E.pen = C->W.pen = GMT->current.setting.map_default_pen;
	GMT_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* Default is no fill */
	C->E.size = CAP_WIDTH  * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 7p */
	return ((void *)C);
}

void Free_psxy_Ctrl (struct GMT_CTRL *GMT, struct PSXY_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C && C->C.file) free ((void *)C->C.file);
	if (C && C->S.arg) free ((void *)C->S.arg);
	GMT_free (GMT, C);
}

void plot_x_errorbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double delta_x, double error_width2, GMT_LONG line) {
	double x_1, x_2, y_1, y_2;
	GMT_LONG tip1, tip2;

	tip1 = tip2 = (error_width2 > 0.0);
	GMT_geo_to_xy (GMT, x - delta_x, y, &x_1, &y_1);
	GMT_geo_to_xy (GMT, x + delta_x, y, &x_2, &y_2);
	if (GMT_is_dnan (x_1)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: X error bar exceeded domain near line %ld. Set to x_min\n", line);
		x_1 = GMT->current.proj.rect[XLO];
		tip1 = FALSE;
	}
	if (GMT_is_dnan (x_2)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: X error bar exceeded domain near line %ld. Set to x_max\n", line);
		x_2 = GMT->current.proj.rect[XHI];
		tip2 = FALSE;
	}
	PSL_plotsegment (PSL, x_1, y_1, x_2, y_2);
	if (tip1) PSL_plotsegment (PSL, x_1, y_1 - error_width2, x_1, y_1 + error_width2);
	if (tip2) PSL_plotsegment (PSL, x_2, y_2 - error_width2, x_2, y_2 + error_width2);
}

void plot_y_errorbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double delta_y, double error_width2, GMT_LONG line) {
	double x_1, x_2, y_1, y_2;
	GMT_LONG tip1, tip2;

	tip1 = tip2 = (error_width2 > 0.0);
	GMT_geo_to_xy (GMT, x, y - delta_y, &x_1, &y_1);
	GMT_geo_to_xy (GMT, x, y + delta_y, &x_2, &y_2);
	if (GMT_is_dnan (y_1)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: Y error bar exceeded domain near line %ld. Set to y_min\n", line);
		y_1 = GMT->current.proj.rect[YLO];
		tip1 = FALSE;
	}
	if (GMT_is_dnan (y_2)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: Y error bar exceeded domain near line %ld. Set to y_max\n", line);
		y_2 = GMT->current.proj.rect[YHI];
		tip2 = FALSE;
	}
	PSL_plotsegment (PSL, x_1, y_1, x_2, y_2);
	if (tip1) PSL_plotsegment (PSL, x_1 - error_width2, y_1, x_1 + error_width2, y_1);
	if (tip2) PSL_plotsegment (PSL, x_2 - error_width2, y_2, x_2 + error_width2, y_2);
}

void plot_x_whiskerbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double hinge[], double error_width2, double rgb[], GMT_LONG line, GMT_LONG kind) {
	GMT_LONG i;
	static GMT_LONG q[4] = {0, 25, 75, 100};
	double xx[4], yy[4];

	for (i = 0; i < 4; i++) {	/* for 0, 25, 75, 100% hinges */
		GMT_geo_to_xy (GMT, hinge[i], y, &xx[i], &yy[i]);
		if (GMT_is_dnan (xx[i])) {
			GMT_report (GMT, GMT_MSG_FATAL, "Warning: X %ld %% hinge exceeded domain near line %ld\n", q[i], line);
			xx[i] = (i <2 ) ? GMT->current.proj.rect[XLO] : GMT->current.proj.rect[XHI];
		}
	}
	yy[1] -= error_width2;
	yy[2] += error_width2;

	PSL_plotsegment (PSL, xx[0], yy[1], xx[0], yy[2]);		/* Left whisker */
	PSL_plotsegment (PSL, xx[0], yy[0], xx[1], yy[0]);

	PSL_plotsegment (PSL, xx[3], yy[1], xx[3], yy[2]);		/* Right whisker */
	PSL_plotsegment (PSL, xx[3], yy[0], xx[2], yy[0]);

	PSL_setfill (PSL, rgb, TRUE);
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
		PSL_plotpolygon (PSL, xp, yp, (GMT_LONG)10);
		PSL_plotsegment (PSL, x, yp[7], x, yp[2]);		/* Median line */
	}
	else {
		PSL_plotbox (PSL, xx[1], yy[1], xx[2], yy[2]);		/* Main box */
		PSL_plotsegment (PSL, x, yy[1], x, yy[2]);		/* Median line */
	}
}

void plot_y_whiskerbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double hinge[], double error_width2, double rgb[], GMT_LONG line, GMT_LONG kind) {
	GMT_LONG i;
	static GMT_LONG q[4] = {0, 25, 75, 100};
	double xx[4], yy[4];

	for (i = 0; i < 4; i++) {	/* for 0, 25, 75, 100% hinges */
		GMT_geo_to_xy (GMT, x, hinge[i], &xx[i], &yy[i]);
		if (GMT_is_dnan (yy[i])) {
			GMT_report (GMT, GMT_MSG_FATAL, "Warning: Y %ld %% hinge exceeded domain near line %ld\n", q[i], line);
			yy[i] = (i <2 ) ? GMT->current.proj.rect[YLO] : GMT->current.proj.rect[YHI];
		}
	}
	xx[1] -= error_width2;
	xx[2] += error_width2;

	PSL_plotsegment (PSL, xx[1], yy[0], xx[2], yy[0]);		/* bottom whisker */
	PSL_plotsegment (PSL, xx[0], yy[0], xx[0], yy[1]);

	PSL_plotsegment (PSL, xx[1], yy[3], xx[2], yy[3]);		/* Top whisker */
	PSL_plotsegment (PSL, xx[0], yy[3], xx[0], yy[2]);

	PSL_setfill (PSL, rgb, TRUE);
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
		PSL_plotpolygon (PSL, xp, yp, (GMT_LONG)10);
		PSL_plotsegment (PSL, xp[7], y, xp[2], y);		/* Median line */
	}
	else {
		PSL_plotbox (PSL, xx[2], yy[2], xx[1], yy[1]);		/* Main box */
		PSL_plotsegment (PSL, xx[1], y, xx[2], y);		/* Median line */
	}
}

GMT_LONG GMT_psxy_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	/* This displays the psxy synopsis and optionally full usage information */

	GMT_message (GMT, "psxy %s [API] - Plot lines, polygons, and symbols on maps\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: psxy <infiles> %s %s [-A[m|p]]\n", GMT_J_OPT, GMT_Rgeoz_OPT);
	GMT_message (GMT, "\t[%s] [-C<cpt>] [-D<dx>/<dy>] [-E[x|y|X|Y][n][cap][/[+|-]<pen>]]\n", GMT_B_OPT);
	GMT_message (GMT, "\t[-G<fill>] [%s] [-I<intens>] [-K] [-L] [-N] [-O] [-P]\n", GMT_Jz_OPT);
	GMT_message (GMT, "\t[-S[<symbol>][<size>|+s<scaling>]] [-T] [%s]\n", GMT_U_OPT);
	GMT_message (GMT, "\t[%s] [-W[+|-][<pen>]] [%s] [%s]\n", GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s]\n\n", GMT_a_OPT, GMT_bi_OPT, \
		GMT_c_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_p_OPT, GMT_t_OPT, GMT_colon_OPT);

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "\t<infiles> is one or more files.  If no, read standard input.\n");
	GMT_explain_options (GMT, "jR");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Suppress drawing line segments as great circle arcs, i.e. draw\n");
	GMT_message (GMT, "\t   straight lines unless m or p is appended to first follow meridian\n");
	GMT_message (GMT, "\t   then parallel, or vice versa.\n");
	GMT_explain_options (GMT, "b");
	GMT_message (GMT, "\t-C Use cpt-file to assign symbol colors based on z-value in 3rd column.\n");
	GMT_message (GMT, "\t   Note: requires -S.  Without -S, psxy excepts lines/polygons\n");
	GMT_message (GMT, "\t   and looks for -Z<val> options in each multiheader.  Then, color is\n");
	GMT_message (GMT, "\t   applied for polygon fill (-L) or polygon pen (no -L).\n");
	GMT_message (GMT, "\t-D Offset symbol or line positions by <dx>/<dy> [no offset].\n");
	GMT_message (GMT, "\t-E Means draw error bars for x, y, or both.  Add cap-width [%gp].\n", CAP_WIDTH);
	GMT_message (GMT, "\t   Append pen attributes. A leading + applies cpt color (-C) to symbol\n");
	GMT_message (GMT, "\t   fill and error pen; - applies it to pen only.  If X or Y is used then\n");
	GMT_message (GMT, "\t   a box-and-whisker diagram is drawn instead, using data from 4 extra\n");
	GMT_message (GMT, "\t   columns to get the 0 %%, 25 %%, 75 %%, and 100%% quantiles (point value\n");
	GMT_message (GMT, "\t   is assumed to be 50%%).  If n is appended after X (or Y) we expect a\n");
	GMT_message (GMT, "\t   5th extra column with the sample size, which is needed to draw a\n");
	GMT_message (GMT, "\t   notched box-and whisker diagram (notch width represents uncertainty.\n");
	GMT_message (GMT, "\t   in the median).  Finally, use -W, -G to affect the 25-75%% box.\n");
	GMT_fill_syntax (GMT, 'G', "Specify color or pattern [no fill].");
	GMT_message (GMT, "\t   -G option can be present in all subheaders (not with -S).\n");
	GMT_message (GMT, "\t-I Uses the intensity to modulate the fill color (requires -C or -G).\n");
	GMT_explain_options (GMT, "ZK");
	GMT_message (GMT, "\t-L Force closed polygons.\n");
	GMT_message (GMT, "\t-N Do not skip or clip symbols that fall outside the map border\n");
	GMT_message (GMT, "\t   [Default will clip or skip symbols that fall outside].\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-S Select symbol type and symbol size (in %s).  Choose between\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	GMT_message (GMT, "\t   -(xdash), +(plus), st(a)r, (b|B)ar, (c)ircle, (d)iamond, (e)llipse,\n");
	GMT_message (GMT, "\t   (f)ront, octa(g)on, (h)exagon, (i)nvtriangle, (j)rotated rectangle,\n");
	GMT_message (GMT, "\t   (k)ustom, (l)etter, (m)athangle, pe(n)tagon, (p)oint, (q)uoted line,\n");
	GMT_message (GMT, "\t   (r)ect, (s)quare, (t)riangle, (v)ector, (w)edge, (x)cross, or (y)dash.\n");
	GMT_message (GMT, "\t   If no size is specified, then the 3rd column must have sizes and\n");
	GMT_message (GMT, "\t   you may append +s<scale>[unit][/<origin>][l] to convert the given data\n");
	GMT_message (GMT, "\t   as size = (data - origin) * scale, using log10 if l is appended.\n");
	GMT_message (GMT, "\t   [Default scale is 1 and origin is 0 (1 if log10)].\n");
	GMT_message (GMT, "\t   If no symbol is specified, then last column must have symbol codes.\n");
	GMT_message (GMT, "\t   [Note: if -C is selected then 3rd means 4th column, etc.]\n");
	GMT_message (GMT, "\t   Symbols A, C, D, G, H, I, N, S, T are adjusted to have same area\n");
	GMT_message (GMT, "\t   as a circle of the specified diameter.\n");
	GMT_message (GMT, "\t   Bars: Append b<base> to give the y-value of the base [Default = 0].\n");
	GMT_message (GMT, "\t      Append u if width is in x-input units [Default is %s].\n", GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	GMT_message (GMT, "\t      Use upper case -SB for horizontal bars (base then refers to x\n");
	GMT_message (GMT, "\t      and width may be in y-units [Default is vertical].\n");
	GMT_message (GMT, "\t   Ellipses: Direction, major, and minor axis must be in columns 3-5.\n");
	GMT_message (GMT, "\t     If -SE rather than -Se is selected, psxy will expect azimuth, and\n");
	GMT_message (GMT, "\t     axes in km, and convert azimuths based on map projection.\n");
	GMT_message (GMT, "\t     If projection is linear then we scale the axes by the map scale.\n");
	GMT_message (GMT, "\t   Rotatable Rectangle: Direction, x- and y-dimensions in columns 3-5.\n");
	GMT_message (GMT, "\t     If -SJ rather than -Sj is selected, psxy will expect azimuth, and\n");
	GMT_message (GMT, "\t     dimensions in km and convert azimuths based on map projection.\n");
	GMT_message (GMT, "\t     For linear projection we scale dimensions by the map scale.\n");
	GMT_message (GMT, "\t   Fronts: Give tickgap/ticklen[dir][type][:offset], where\n");
	GMT_message (GMT, "\t     dir    = Plot symbol to l(eft) or r(ight) of front [c=centered]\n");
	GMT_message (GMT, "\t     type   =  b(ox), c(ircle), f(ault), s(lip), t(riangle) [f]\n");
	GMT_message (GMT, "\t       box      : square when centered, half-square otherwise.\n");
	GMT_message (GMT, "\t       circle   : full when centered, half-circle otherwise.\n");
	GMT_message (GMT, "\t       fault    : centered cross-tick or tick only in <dir> direction.\n");
	GMT_message (GMT, "\t       slip     : left-or right-lateral strike-slip arrows.\n");
	GMT_message (GMT, "\t       triangle : diagonal square (c), directed triangle otherwise.\n");
	GMT_message (GMT, "\t     offset = Plot first symbol when along-front distance is offset [0].\n");
	GMT_message (GMT, "\t   Kustom: Append <symbolname> immediately after 'k'; this will look for\n");
	GMT_message (GMT, "\t     <symbolname>.def in the current directory, in $GMT_USERDIR,\n");
	GMT_message (GMT, "\t     or in $GMT_SHAREDIR (searched in that order).\n");
	GMT_list_custom_symbols (GMT);
	GMT_message (GMT, "\t   Letter: append /<string> after symbol size, and optionally %%<font>.\n");
	GMT_message (GMT, "\t   Mathangle: start/stop directions of math angle must be in columns 3-4.\n");
	GMT_message (GMT, "\t     Use -Smf for arrow at first angle and -Sml for last, -Smb for both [none].\n");
	GMT_message (GMT, "\t   Quoted line: Give [d|f|n|l|x]<info>[:<labelinfo>].\n");
	GMT_message (GMT, "\t     <code><info> controls placement of labels along lines.  Select\n");
	GMT_cont_syntax (GMT, 7, 1);
	GMT_message (GMT, "\t     <labelinfo> controls the label attributes.  Choose from\n");
	GMT_label_syntax (GMT, 7, 1);
	GMT_message (GMT, "\t   Rectangles: x- and y-dimensions must be in columns 3-4.\n");
	GMT_message (GMT, "\t   Vectors: Direction and length must be in columns 3-4.\n");
	GMT_message (GMT, "\t     Furthermore, <size> means vectorwidth/headlength/headwidth\n");
	GMT_message (GMT, "\t     [Default attributes are %gp/%gp/%gp].\n", VECTOR_LINE_WIDTH, VECTOR_HEAD_LENGTH, VECTOR_HEAD_WIDTH);
	GMT_message (GMT, "\t     If -SV rather than -Sv is use, psxy will expect azimuth and\n");
	GMT_message (GMT, "\t     length and convert azimuths based on the chosen map projection.\n");
	GMT_message (GMT, "\t     Insert h(head), b(balance point), or t(ail) after -Sv|V to \n");
	GMT_message (GMT, "\t     justify vector w.r.t. input (x,y).  Insert s(egment) if (x,y)\n");
	GMT_message (GMT, "\t     is tail and columns 3 and 4 hold the head location (x,y).\n");
	GMT_message (GMT, "\t     Upper case H, B, T, S gives double-headed vector [Default is t].\n");
	GMT_message (GMT, "\t   Wedges: Start and stop directions of wedge must be in columns 3-4.\n");
	GMT_message (GMT, "\t     If -SW rather than -Sw is selected, specify two azimuths instead.\n");
	GMT_message (GMT, "\t-T Ignores all input files.\n");
	GMT_explain_options (GMT, "UV");
	GMT_pen_syntax (GMT, 'W', "Sets pen attributes [Default pen is %s]:");
	GMT_message (GMT, "\t   A leading + applies cpt color (-C) to both symbol fill and pen.\n");
	GMT_message (GMT, "\t   A leading - applies cpt color (-C) to the pen only.\n");
	GMT_explain_options (GMT, "XaC0c");
	GMT_message (GMT, "\t   Default is the required number of columns.\n");
	GMT_explain_options (GMT, "fghipt:.");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_psxy_parse (struct GMTAPI_CTRL *C, struct PSXY_CTRL *Ctrl, struct GMT_OPTION *options, struct GMT_SYMBOL *S)
{
	/* This parses the options provided to psxy and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG j, j0, n_errors = 0;
	char txt_a[GMT_TEXT_LEN256], txt_b[GMT_TEXT_LEN256];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Turn off draw_arc mode */
				Ctrl->A.active = TRUE;
				switch (opt->arg[0]) {
					case 'm': Ctrl->A.mode = 1; break;
					case 'p': Ctrl->A.mode = 2; break;
#ifdef DEBUG
					default: Ctrl->A.step = atof (opt->arg); break; /* Undocumented test feature */
#endif
				}
				break;
			case 'C':	/* Vary symbol color with z */
				Ctrl->C.file = strdup (opt->arg);
				Ctrl->C.active = TRUE;
				break;
			case 'D':
				if ((j = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b)) < 1) {
					GMT_report (GMT, GMT_MSG_FATAL, "Syntax error -D option: Give x [and y] offsets\n");
					n_errors++;
				}
				else {
					Ctrl->D.dx = GMT_to_inch (GMT, txt_a);
					Ctrl->D.dy = (j == 2) ? GMT_to_inch (GMT, txt_b) : Ctrl->D.dx;
					Ctrl->D.active = TRUE;
				}
				break;
			case 'E':	/* Get info for error bars and bow-whisker bars */
				Ctrl->E.active = TRUE;
				j = 0;
				while (opt->arg[j] && opt->arg[j] != '/') {
					switch (opt->arg[j]) {
					case 'x':	/* Error bar for x */
						Ctrl->E.xbar = 1; break;
					case 'X':	/* Box-whisker instead */
						Ctrl->E.xbar = 2;
						if (opt->arg[j+1] == 'n') {Ctrl->E.xbar++; j++;}
						break;
					case 'y':	/* Error bar for y */
						Ctrl->E.ybar = 1; break;
					case 'Y':	/* Box-whisker instead */
						Ctrl->E.ybar = 2;
						if (opt->arg[j+1] == 'n') {Ctrl->E.ybar++; j++;}
						break;
					default:	/* Get error 'cap' width */
						strcpy (txt_a, &opt->arg[j]);
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
					if (opt->arg[j]) GMT_getpen (GMT, &opt->arg[j], &Ctrl->E.pen);
				}
				break;
			case 'G':		/* Set fill for symbols or polygon */
				Ctrl->G.active = TRUE;
				if (!opt->arg[0] || GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " "); n_errors++;
				}
				break;
			case 'I':	/* Adjust symbol color via intensity */
				Ctrl->I.value = atof (opt->arg);
				Ctrl->I.active = TRUE;
				break;
			case 'L':		/* Close line segments */
				Ctrl->L.active = TRUE;
				break;
			case 'N':		/* Do not skip points outside border */
				Ctrl->N.active = TRUE;
				break;
			case 'S':		/* Get symbol [and size] */
				Ctrl->S.active = TRUE;
				Ctrl->S.arg = strdup (opt->arg);
				break;
			case 'T':		/* Skip all input files */
				Ctrl->T.active = TRUE;
				break;
			case 'W':		/* Set line attributes */
				Ctrl->W.active = TRUE;
				j = 0;
				if (opt->arg[j] == '-') {Ctrl->W.mode = 1; j++;}
				if (opt->arg[j] == '+') {Ctrl->W.mode = 2; j++;}
				if (opt->arg[j] && GMT_getpen (GMT, &opt->arg[j], &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', "sets pen attributes [Default pen is %s]:");
					GMT_message (GMT, "\t   A leading + applies cpt color (-C) to both symbol fill and pen.\n");
					GMT_message (GMT, "\t   A leading - applies cpt color (-C) to the pen only.\n");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	n_errors += GMT_check_condition (GMT, Ctrl->S.active && GMT_parse_symbol_option (GMT, Ctrl->S.arg, S, 0, TRUE), "Syntax error -S option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && (S->symbol == GMT_SYMBOL_VECTOR || S->symbol == GMT_SYMBOL_ELLIPSE || S->symbol == GMT_SYMBOL_FRONT || S->symbol == GMT_SYMBOL_QUOTED_LINE || S->symbol == GMT_SYMBOL_ROTRECT), "Syntax error -E option: Incompatible with -Se, -Sf, -Sj, -Sq, -Sv\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, GMT->common.b.active[GMT_IN] && S->symbol == GMT_SYMBOL_NOT_SET, "Syntax error: Binary input data cannot have symbol information\n");
	n_errors += GMT_check_condition (GMT, Ctrl->E.active && Ctrl->E.mode && !Ctrl->C.active, "Syntax error: -E option +|-<pen> requires the -C option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->W.active && Ctrl->W.mode && !Ctrl->C.active, "Syntax error: -W option +|-<pen> requires the -C option\n");
	n_errors += GMT_check_condition (GMT, (Ctrl->W.mode + Ctrl->E.mode) == 3, "Syntax error: Conflicting -E and -W options regarding -C option application\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define Return(code) {Free_psxy_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_psxy (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{	/* High-level function that implements the psxy task */
	GMT_LONG polygon, penset_OK = TRUE, not_line, old_is_world;
	GMT_LONG get_rgb, read_symbol, clip_set = FALSE, fill_active;
	GMT_LONG default_outline, outline_active, set_type;
	GMT_LONG error_x = FALSE, error_y = FALSE, def_err_xy = FALSE;
	GMT_LONG i, n_total_read = 0, j, geometry, tbl, seg, read_mode;
	GMT_LONG n_cols_start = 2, n_fields, error = GMT_NOERROR;
	GMT_LONG ex1, ex2, ex3, change, pos2x, pos2y, save_u = FALSE;
	GMT_LONG xy_errors[2], error_type[2] = {0,0}, error_cols[3] = {1,4,5};

	char buffer[GMT_BUFSIZ], *text_rec = NULL;

	double dim[7], *in = NULL;
	double s, c, plot_x, plot_y, x_1, x_2, y_1, y_2;
	double direction, length, dx, dy;

	struct GMT_PEN current_pen, default_pen;
	struct GMT_FILL current_fill, default_fill;
	struct GMT_SYMBOL S;
	struct GMT_PALETTE *P = NULL;
	struct GMT_LINE_SEGMENT *L = NULL;
	struct PSXY_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */

	void **record = NULL;	/* Opaque pointer to either a text or double record */
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_psxy_usage (API, GMTAPI_USAGE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_psxy_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_psxy", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJRbf:", "BKOPUXYacghipst>" GMT_OPT("HMm"), options))) Return(error);

	/* Initialize GMT_SYMBOL structure */

	GMT_memset (&S, 1, struct GMT_SYMBOL);
	GMT_contlabel_init (GMT, &S.G, 0);
	xy_errors[GMT_X] = xy_errors[1] = 0;	/* These will be col # of where to find this info in data */

	S.base = GMT->session.d_NaN;
	S.font = GMT->current.setting.font_annot[0];
	S.u = GMT->current.setting.proj_length_unit;
	S.v_width  = VECTOR_LINE_WIDTH  * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 2p */
	S.h_width  = VECTOR_HEAD_WIDTH  * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 7p */
	S.h_length = VECTOR_HEAD_LENGTH * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 9p */

	Ctrl = (struct PSXY_CTRL *)New_psxy_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_psxy_parse (API, Ctrl, options, &S))) Return (error);
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the psxy main code ----------------------------*/

	/* Do we plot actual symbols, or lines */
	not_line = (S.symbol != GMT_SYMBOL_FRONT && S.symbol != GMT_SYMBOL_QUOTED_LINE && S.symbol != GMT_SYMBOL_LINE);

	if (Ctrl->E.active) {	/* Set error bar parameters */
		j = 2;	/* Normally, error bar related columns start in position 2 */
		if (Ctrl->E.xbar == 1) {
			xy_errors[GMT_X] = j++;
			error_type[GMT_X] = 0;
		}
		else if (Ctrl->E.xbar == 2) {	/* Box-whisker instead */
			xy_errors[GMT_X] = j++;
			error_type[GMT_X] = 1;
		}
		else if (Ctrl->E.xbar == 3) {	/* Notched Box-whisker instead */
			xy_errors[GMT_X] = j++;
			error_type[GMT_X] = 2;
		}
		if (Ctrl->E.ybar == 1) {
			xy_errors[GMT_Y] = j++;
			error_type[GMT_Y] = 0;
		}
		else if (Ctrl->E.ybar == 2) {	/* Box-whisker instead */
			xy_errors[GMT_Y] = j++;
			error_type[GMT_Y] = 1;
		}
		else if (Ctrl->E.ybar == 3) {	/* Notched Box-whisker instead */
			xy_errors[GMT_Y] = j++;
			error_type[GMT_Y] = 2;
		}
		if (!(xy_errors[GMT_X] || xy_errors[GMT_Y])) {	/* Default is plain error bars for both */
			def_err_xy = TRUE;
			xy_errors[GMT_X] = 2;	/* Assumes xy input, later check for -: */
			xy_errors[GMT_Y] = 3;
			error_type[GMT_X] = error_type[GMT_Y] = 1;
		}
	}

	if (Ctrl->T.active) GMT_report (GMT, GMT_MSG_VERBOSE, "Warning: Option -T ignores all input files\n");

	get_rgb = (not_line && Ctrl->C.active);	/* Need to read z-vales from input data file */
	read_symbol = (S.symbol == GMT_SYMBOL_NOT_SET);	/* Only for ASCII input */
	polygon = (S.symbol == GMT_SYMBOL_LINE && (Ctrl->G.active || Ctrl->L.active));

	current_pen = default_pen = Ctrl->W.pen;
	current_fill = default_fill = Ctrl->G.fill;
	default_outline = Ctrl->W.active;
	if (Ctrl->I.active) {
		GMT_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
		GMT_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
	}

	Ctrl->E.size *= 0.5;	/* Since we draw half-way in either direction */

	if (Ctrl->E.active && S.symbol == GMT_SYMBOL_LINE)	/* Assume user only wants error bars */
		S.symbol = GMT_SYMBOL_NONE;

	if (Ctrl->C.active) {
		if ((error = GMT_Begin_IO (API, GMT_IS_CPT, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
		if (GMT_Get_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_POINT, NULL, 0, (void **)&Ctrl->C.file, (void **)&P)) Return (GMT_DATA_READ_ERROR);
		if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */
		if (get_rgb) n_cols_start++;
	}

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
		if (xy_errors[GMT_X] && xy_errors[GMT_Y] && error_type[GMT_X] >= 1) xy_errors[GMT_Y] += error_cols[error_type[GMT_X]] - 1;	/* Need 3 or 4 columns for whisker bars */
		if (def_err_xy && GMT->current.setting.io_lonlat_toggle[GMT_IN]) {	/* -E should be -Eyx */
			l_swap (xy_errors[GMT_X], xy_errors[GMT_Y]);
			l_swap (error_type[GMT_X], error_type[GMT_Y]);
		}
		if (xy_errors[GMT_X]) n_cols_start += error_cols[error_type[GMT_X]], error_x = TRUE;
		if (xy_errors[GMT_Y]) n_cols_start += error_cols[error_type[GMT_Y]], error_y = TRUE;
		xy_errors[GMT_X] += (S.read_size + get_rgb);	/* Move 0-2 columns over */
		xy_errors[GMT_Y] += (S.read_size + get_rgb);
	}
	else	/* Here we have the usual x y [z] [size] [other args] [symbol] record */
		for (j = n_cols_start; j < 10; j++) GMT->current.io.col_type[GMT_IN][j] = GMT_IS_DIMENSION;		/* Since these may have units appended */
	for (j = 0; j < S.n_nondim; j++) GMT->current.io.col_type[GMT_IN][S.nondim_col[j]+get_rgb] = GMT_IS_FLOAT;	/* Since these are angles, not dimensions */

	error += GMT_check_binary_io (GMT, n_cols_start + S.n_required);

	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);
	if (S.u_set) {	/* When -Sc<unit> is given we temporarily reset the system unit to these units so conversions will work */
		save_u = GMT->current.setting.proj_length_unit;
		GMT->current.setting.proj_length_unit = S.u;
	}

	/* if (S.G.delay) GMT->current.ps.clip = +1; */	/* Signal that this program initiates clipping that wil outlive this process */
	
	GMT_plotinit (API, PSL, options);
	if (Ctrl->T.active) {
		GMT_plotend (GMT, PSL);
		Return (GMT_OK);
	}

	if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {
		if (GMT_contlabel_prep (GMT, &S.G, NULL)) Return (EXIT_FAILURE);
		penset_OK = FALSE;	/* The pen for quoted lines are set within the PSL code itself so we dont do it here in psxy */
	}

	if ((Ctrl->A.active && Ctrl->A.mode == 0) || !GMT_is_geographic (GMT, GMT_IN)) GMT->current.map.path_mode = GMT_LEAVE_PATH;	/* Turn off resampling */
#ifdef DEBUG
	/* Change default step size (in degrees) used for interpolation of line segments along great circles (if requested) */
	if (Ctrl->A.active) Ctrl->A.step = Ctrl->A.step / GMT->current.proj.scale[GMT_X] / GMT->current.proj.M_PR_DEG;
#endif
	if (S.symbol == GMT_SYMBOL_FRONT || S.symbol == GMT_SYMBOL_QUOTED_LINE || (not_line && !Ctrl->N.active)) {
		if (!S.G.delay) {
			GMT_map_clip_on (GMT, PSL, GMT->session.no_rgb, 3);
			clip_set = TRUE;
		}
	}
	if (penset_OK) GMT_setpen (GMT, PSL, &current_pen);

	if (S.symbol == GMT_SYMBOL_TEXT && Ctrl->G.active && !Ctrl->W.active) PSL_setcolor (PSL, current_fill.rgb, PSL_IS_FILL);
	if (S.symbol == GMT_SYMBOL_TEXT) GMT_setfont (GMT, PSL, &S.font);	/* Set the required font */
	if (S.symbol == GMT_SYMBOL_ELLIPSE) Ctrl->N.active = TRUE;	/* So we can see ellipses that have centers outside -R */
	if (S.symbol == GMT_SYMBOL_BARX && !S.base_set && GMT->current.proj.xyz_projection[GMT_X] == GMT_LOG10) S.base = GMT->common.R.wesn[XLO];	/* Default to west level for horizontal log10 bars */
	if (S.symbol == GMT_SYMBOL_BARY && !S.base_set && GMT->current.proj.xyz_projection[GMT_Y] == GMT_LOG10) S.base = GMT->common.R.wesn[YLO];	/* Default to south level for vertical log10 bars */
	if (S.symbol == GMT_SYMBOL_VECTOR && S.v_just == 3) {
		/* Reading 2nd coordinate so must set column types */
		GMT->current.io.col_type[GMT_IN][pos2x] = GMT->current.io.col_type[GMT_IN][GMT_X];
		GMT->current.io.col_type[GMT_IN][pos2y] = GMT->current.io.col_type[GMT_IN][GMT_Y];
	}
	fill_active = Ctrl->G.active;	/* Make copies because we will change the values */
	outline_active =  Ctrl->W.active;
	if (not_line && !outline_active && !fill_active && !get_rgb) outline_active = TRUE;	/* If no fill nor outline for symbols then turn outline on */

	if (Ctrl->D.active) PSL_setorigin (PSL, Ctrl->D.dx, Ctrl->D.dy, 0.0, PSL_FWD);	/* Shift plot a bit */

	old_is_world = GMT->current.map.is_world;
	geometry = not_line ? GMT_IS_POINT : (GMT_IS_LINE + polygon);
	if (read_symbol) {	/* If symbol info is given we must process text records */
		set_type = GMT_IS_TEXTSET;
		read_mode = GMT_READ_TEXT;
		record = (void **)buffer;
		in = GMT->current.io.curr_rec;
	}
	else {	/* Here we can process data records (ASCII or binary) */
		set_type = GMT_IS_DATASET;
		read_mode = GMT_READ_DOUBLE;
		record = (void **)&in;
	}

	if (not_line) {	/* Symbol part (not counting GMT_SYMBOL_FRONT and GMT_SYMBOL_QUOTED_LINE) */
		if ((error = GMT_Init_IO (API, set_type, geometry, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Register data input */
		if ((error = GMT_Begin_IO (API, set_type, GMT_IN, GMT_BY_REC))) Return (error);		/* Enables data input and sets access mode */
		GMT->current.map.is_world = !(S.symbol == GMT_SYMBOL_ELLIPSE && S.convert_angles);
		while ((n_fields = GMT_Get_Record (API, read_mode, record)) != EOF) {	/* Keep returning records until we have no more files */

			if (GMT_REC_IS_ERROR (GMT)) Return (EXIT_FAILURE);

			if (GMT_REC_IS_TBL_HEADER (GMT)) continue;	/* Skip table headers */
			while (GMT_REC_IS_SEG_HEADER(GMT) && !GMT_REC_IS_EOF(GMT)) {	/* Process segment headers */
				PSL_comment (PSL, "Segment header: %s\n", GMT->current.io.segment_header);
				change = GMT_parse_segment_header (GMT, GMT->current.io.segment_header, P, &fill_active, &current_fill, default_fill, &outline_active, &current_pen, default_pen, default_outline, NULL);
				if (Ctrl->I.active) {
					GMT_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
					GMT_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
				}
				n_fields = GMT_Get_Record (API, read_mode, record);
			}
			if (GMT_REC_IS_EOF (GMT)) continue;	/* At EOF */

			n_total_read++;

			if (read_symbol) {	/* Must do special processing */
				text_rec = (char *)(*record);	/* Get current text record */
				/* First establish the symbol type given at the end of the record */
				GMT_chop (text_rec);	/* Get rid of \n \r */
				i = strlen (text_rec) - 1;
				while (text_rec[i] && !strchr (" ,\t", (int)text_rec[i])) i--;
				GMT_parse_symbol_option (GMT, &text_rec[i+1], &S, 0, FALSE);
				/* Now convert the leading text items to doubles; col_type[GMT_IN] might have been updated by GMT_parse_symbol_option */
				if (GMT_conv_intext2dbl (GMT, text_rec, 6)) {	/* Max 6 columns needs to be parsed */
					GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad x and/or y coordinates, skipped)\n", n_total_read);
					continue;
				}
			}

			if (get_rgb) {
				GMT_get_rgb_from_z (GMT, P, in[GMT_Z], current_fill.rgb);
				if (Ctrl->I.active) GMT_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
				if (P->skip) continue;	/* Chosen cpt file indicates skip for this z */
			}

			if (!Ctrl->N.active && S.symbol != GMT_SYMBOL_BARX && S.symbol != GMT_SYMBOL_BARY) {
				/* Skip points outside map */
				GMT_map_outside (GMT, in[GMT_X], in[GMT_Y]);
				if (GMT_abs (GMT->current.map.this_x_status) > 1 || GMT_abs (GMT->current.map.this_y_status) > 1) continue;
			}

			if (GMT_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y)) continue;	/* NaNs on input */

			if (GMT_is_dnan (plot_x)) {	/* Transformation of x yielded a NaN (e.g. log (-ve)) */
				GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Data point with x = NaN near line %ld\n", n_total_read);
				continue;
			}
			if (GMT_is_dnan (plot_y)) {	/* Transformation of y yielded a NaN (e.g. log (-ve)) */
				GMT_report (GMT, GMT_MSG_NORMAL, "Warning: Data point with y = NaN near line %ld\n", n_total_read);
				continue;
			}

			if (Ctrl->E.active) {
				if (Ctrl->E.mode) GMT_rgb_copy (Ctrl->E.pen.rgb, current_fill.rgb);
				if (Ctrl->E.mode & 1) GMT_rgb_copy (current_fill.rgb, GMT->session.no_rgb);
				GMT_setpen (GMT, PSL, &Ctrl->E.pen);
				if (error_x) {
					if (error_type[GMT_X] == 0)
						plot_x_errorbar (GMT, PSL, in[GMT_X], in[GMT_Y], in[xy_errors[GMT_X]], Ctrl->E.size, n_total_read);
					else
						plot_x_whiskerbar (GMT, PSL, plot_x, in[GMT_Y], &in[xy_errors[GMT_X]], Ctrl->E.size, current_fill.rgb, n_total_read, error_type[GMT_X]);
				}
				if (error_y) {
					if (error_type[GMT_Y] == 0)
						plot_y_errorbar (GMT, PSL, in[GMT_X], in[GMT_Y], in[xy_errors[GMT_Y]], Ctrl->E.size, n_total_read);
					else
						plot_y_whiskerbar (GMT, PSL, in[GMT_X], plot_y, &in[xy_errors[GMT_Y]], Ctrl->E.size, current_fill.rgb, n_total_read, error_type[GMT_Y]);
				}
			}

			if (Ctrl->W.mode) {
				GMT_rgb_copy (Ctrl->W.pen.rgb, current_fill.rgb);
				current_pen = Ctrl->W.pen;
			}
			if (Ctrl->W.mode & 1) GMT_rgb_copy (current_fill.rgb, GMT->session.no_rgb);

			GMT_setfill (GMT, PSL, &current_fill, outline_active);
			GMT_setpen (GMT, PSL, &current_pen);

			dim[0] = (S.read_size) ? in[ex1] : S.size_x;
			if (S.convert_size) dim[0] = ((S.convert_size == 2) ? log10 (dim[0]) : dim[0]) * S.scale - S.origin;

			switch (S.symbol) {
				case GMT_SYMBOL_NONE:
					break;
				case GMT_SYMBOL_BARX:
					if (!Ctrl->N.active) in[GMT_X] = MAX (GMT->common.R.wesn[XLO], MIN (in[GMT_X], GMT->common.R.wesn[XHI]));
					if (S.user_unit) {	/* Width measured in y units */
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
					if (S.user_unit) {	/* Width measured in x units */
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
					PSL_plotsymbol (PSL, plot_x, plot_y, dim, S.symbol);
					break;
				case GMT_SYMBOL_RECT:
					dim[0] = in[ex1];
					dim[1] = in[ex2];
					PSL_plotsymbol (PSL, plot_x, plot_y, dim, PSL_RECT);
					break;
				case GMT_SYMBOL_ROTRECT:
				case GMT_SYMBOL_ELLIPSE:
					if (!S.convert_angles) {	/* Got axes in current plot units, change to inches */
						dim[0] = in[ex1];
						dim[1] = in[ex2];
						dim[2] = in[ex3];
						PSL_plotsymbol (PSL, plot_x, plot_y, dim, S.symbol);
					}
					else if (!GMT_is_geographic (GMT, GMT_IN)) {	/* Got axes in user units, change to inches */
						dim[0] = 90.0 - in[ex1];	/* Cartesian azimuth */
						dim[1] = in[ex2] * GMT->current.proj.scale[GMT_X];
						dim[2] = in[ex3] * GMT->current.proj.scale[GMT_X];
						PSL_plotsymbol (PSL, plot_x, plot_y, dim, S.symbol);
					}
					else if (S.symbol == GMT_SYMBOL_ELLIPSE)	/* Got axis in km */
						GMT_geo_ellipse (GMT, PSL, in[GMT_X], in[GMT_Y], in[ex2], in[ex3], in[ex1]);
					else
						GMT_geo_rectangle (GMT, PSL, in[GMT_X], in[GMT_Y], in[ex2], in[ex3], in[ex1]);
					break;
				case GMT_SYMBOL_TEXT:
					if (Ctrl->G.active && !outline_active)
						PSL_setcolor (PSL, current_fill.rgb, PSL_IS_FILL);
					else if (!Ctrl->G.active)
						PSL_setfill (PSL, GMT->session.no_rgb, outline_active);
					(void) GMT_setfont (GMT, PSL, &S.font);
					PSL_plottext (PSL, plot_x, plot_y, dim[0] * PSL_POINTS_PER_INCH, S.string, 0.0, PSL_MC, outline_active);
					break;
				case GMT_SYMBOL_VECTOR:
					length = in[ex2];
					if (!S.convert_angles)
						direction = in[ex1];
					else if (!GMT_is_geographic (GMT, GMT_IN))
						direction = 90.0 - in[ex1];
					else
						GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, in[ex1], &direction);
					if (S.v_just == 3) {
						GMT_geo_to_xy (GMT, in[pos2x], in[pos2y], &x_2, &y_2);
						if (GMT_is_dnan (x_2) || GMT_is_dnan (y_2)) {
							GMT_report (GMT, GMT_MSG_FATAL, "Warning: Vector head coordinates contain NaNs near line %ld. Skipped\n", n_total_read);
							continue;
						}
					}
					else {
						sincosd (direction, &s, &c);
						x_2 = plot_x + length * c;
						y_2 = plot_y + length * s;
						if (S.v_just) {
							dx = S.v_just * 0.5 * (x_2 - plot_x);	dy = S.v_just * 0.5 * (y_2 - plot_y);
							plot_x -= dx;		plot_y -= dy;
							x_2 -= dx;		y_2 -= dy;
						}
					}
					s = (length < S.v_norm) ? length * S.v_shrink : 1.0;
					dim[0] = x_2, dim[1] = y_2;
					dim[2] = s * S.v_width, dim[3] = s * S.h_length, dim[4] = s * S.h_width;
					dim[5] = GMT->current.setting.map_vector_shape, dim[6] = S.v_double_heads ? 1.0 : 0.0;
					PSL_plotsymbol (PSL, plot_x, plot_y, dim, PSL_VECTOR);
					break;
				case GMT_SYMBOL_MARC:
					dim[4] = GMT->current.setting.map_vector_shape, dim[3] = (double)S.v_double_heads;
				case GMT_SYMBOL_WEDGE:
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
						GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, 90.0 - in[ex1+S.read_size], &(dim[2]));
						GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, 90.0 - in[ex2+S.read_size], &(dim[1]));
					}
					dim[0] *= 0.5;
					PSL_plotsymbol (PSL, plot_x, plot_y, dim, S.symbol);
					break;
				case GMT_SYMBOL_CUSTOM:
					for (j = 0; j < S.n_required; j++) dim[j+1] = in[ex1+S.read_size+j];
					GMT_draw_custom_symbol (GMT, PSL, plot_x, plot_y, dim, S.custom, &current_pen, &current_fill, outline_active);
					break;
			}
		}
	}
	else {	/* Line/polygon part */
		struct GMT_DATASET *D = NULL;	/* Pointer to GMT multisegment table(s) */

		if ((error = GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Register data input */
		if ((error = GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_BY_SET))) Return (error);	/* Enables data input and sets access mode */
		if ((error = GMT_Get_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, NULL, 0, NULL, (void **)&D))) Return (error);

		for (tbl = 0; tbl < D->n_tables; tbl++) {
			if (D->table[tbl]->n_headers && S.G.label_type == 2)	/* Get potential label from first header */
				GMT_extract_label (GMT, D->table[tbl]->header[0], S.G.label);

			for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {	/* For each segment in the table */

				L = D->table[tbl]->segment[seg];	/* Set shortcut to current segment */
				if (polygon && GMT_polygon_is_hole (L)) continue;	/* Holes are handled together with perimeters */

				/* We had here things like:	x = D->table[tbl]->segment[seg]->coord[GMT_X];
				 * but reallocating x below lead to disasters.  */

				change = GMT_parse_segment_header (GMT, L->header, P, &fill_active, &current_fill, default_fill, &outline_active, &current_pen, default_pen, default_outline, L->ogr);

				if (P && P->skip) continue;	/* Chosen cpt file indicates skip for this z */

				if (L->header && L->header[0]) PSL_comment (PSL, "Segment header: %s\n", L->header);

				if (Ctrl->I.active) {
					GMT_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
					GMT_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
				}
				if (change & 4 && penset_OK) GMT_setpen (GMT, PSL, &current_pen);
				if (change & 1) polygon = TRUE;
				if (change & 2 && !Ctrl->L.active) {
					polygon = FALSE;
					PSL_setcolor (PSL, current_fill.rgb, PSL_IS_STROKE);
				}
				if (S.G.label_type == 2)	/* Get potential label from segment header */
					GMT_extract_label (GMT, L->header, S.G.label);

				if (polygon && GMT_polygon_is_open (GMT, L->coord[GMT_X], L->coord[GMT_Y], L->n_rows)) {
					/* Explicitly close polygon so that arc will work */
					L->n_rows = GMT_malloc2 (GMT, L->coord[GMT_X], L->coord[GMT_Y], 0, L->n_rows+1, double);
					L->coord[GMT_X][L->n_rows-1] = L->coord[GMT_X][0];
					L->coord[GMT_Y][L->n_rows-1] = L->coord[GMT_Y][0];
				}

				if (GMT->current.map.path_mode == GMT_RESAMPLE_PATH)	/* Resample if spacing is too coarse */
					L->n_rows = GMT_fix_up_path (GMT, &L->coord[GMT_X], &L->coord[GMT_Y], L->n_rows, Ctrl->A.step, Ctrl->A.mode);

				if (polygon) {	/* Want a closed polygon (with or without fill and with or without outline) */
					GMT_setfill (GMT, PSL, &current_fill, outline_active);
					GMT_geo_polygons (GMT, PSL, L);
				}
				else if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {	/* Labeled lines are dealt with by the contour machinery */
					if ((GMT->current.plot.n = GMT_geo_to_xy_line (GMT, L->coord[GMT_X], L->coord[GMT_Y], L->n_rows)) == 0) continue;
					S.G.line_pen = current_pen;
					GMT_hold_contour (GMT, &GMT->current.plot.x, &GMT->current.plot.y, GMT->current.plot.n, 0.0, "N/A", 'A', S.G.label_angle, Ctrl->L.active, &S.G);
					GMT->current.plot.n_alloc = GMT->current.plot.n;	/* Since GMT_hold_contour reallocates to fit the array */
				}
				else {	/* Plot line */
					if ((GMT->current.plot.n = GMT_geo_to_xy_line (GMT, L->coord[GMT_X], L->coord[GMT_Y], L->n_rows)) == 0) continue;
					GMT_plot_line (GMT, PSL, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n);
				}
				if (S.symbol == GMT_SYMBOL_FRONT) { /* Must draw fault crossbars */
					GMT_setfill (GMT, PSL, &current_fill, outline_active);
					GMT_draw_fence (GMT, PSL, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n, &S.f);
				}
			}
		}
		GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&D);
	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (Ctrl->C.active) GMT_Destroy_Data (API, GMT_ALLOCATED, (void **)&P);

	if (S.u_set) GMT->current.setting.proj_length_unit = save_u;	/* Reset unit */

	if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {
		GMT_contlabel_plot (GMT, PSL, &S.G);
		GMT_contlabel_free (GMT, &S.G);
	}

	if (Ctrl->D.active) PSL_setorigin (PSL, -Ctrl->D.dx, -Ctrl->D.dy, 0.0, PSL_FWD);	/* Reset shift */

	if (clip_set) GMT_map_clip_off (GMT, PSL);

	if (current_pen.style) PSL_setdash (PSL, CNULL, 0);
	GMT->current.map.is_world = old_is_world;

	GMT_map_basemap (GMT, PSL);
	GMT_plotend (GMT, PSL);

	Return (GMT_OK);
}
