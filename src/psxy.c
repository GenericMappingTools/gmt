/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2020 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 *	Contact info: www.generic-mapping-tools.org
 *--------------------------------------------------------------------*/
/*
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 *
 * Brief synopsis: psxy will read <x,y> points and plot symbols, lines,
 * or polygons on maps.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psxy"
#define THIS_MODULE_MODERN_NAME	"plot"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot lines, polygons, and symbols in 2-D"
#define THIS_MODULE_KEYS	"<D{,CC(,T-<,S?(=2,ZD(=,>X}"
#define THIS_MODULE_NEEDS	"Jd"
#define THIS_MODULE_OPTIONS "-:>BJKOPRUVXYabdefghilpqt" GMT_OPT("HMmc")

/* Control structure for psxy */

struct PSXY_CTRL {
	struct PSXY_A {	/* -A[m|y|p|x|step] */
		bool active;
		unsigned int mode;
		double step;
	} A;
	struct PSXY_C {	/* -C<cpt> or -C<color1>,<color2>[,<color3>,...] */
		bool active;
		char *file;
	} C;
	struct PSXY_D {	/* -D<dx>/<dy> */
		bool active;
		double dx, dy;
	} D;
	struct PSXY_E {	/* -E[x[+]|X][y[+]|Y][cap][/[+|-]<pen>] */
		bool active;
		unsigned int xbar, ybar;	/* 0 = not used, 1 = error bar, 2 = asumemtrical error bar, 3 = box-whisker, 4 = notched box-whisker */
		unsigned int mode;		/* 0 = normal, 1 = -C applies to error pen color, 2 = -C applies to symbol fill & error pen color */
		double size;
		struct GMT_PEN pen;
	} E;
	struct PSXY_F {	/* -F<mode> */
		bool active;
		struct GMT_SEGMENTIZE S;
	} F;
	struct PSXY_G {	/* -G<fill> */
		bool active;
		struct GMT_FILL fill;
	} G;
	struct PSXY_I {	/* -I[<intensity>] */
		bool active;
		unsigned int mode;	/* 0 if constant, 1 if read from file (symbols only) */
		double value;
	} I;
	struct PSXY_L {	/* -L[+xl|r|x0][+yb|t|y0][+e|E][+p<pen>] */
		bool active;
		bool polygon;		/* true when just -L is given */
		int outline;		/* true when +p<pen> is given */
		unsigned int mode;	/* Which side for the anchor */
		unsigned int anchor;	/* 0 not used, 1 = x anchors, 2 = y anchors, 3 = +/-dy, 4 = -dy1, +dy2 */
		double value;
		struct GMT_PEN pen;
	} L;
	struct PSXY_N {	/* -N[r|c] */
		bool active;
		unsigned int mode;
	} N;
	struct PSXY_S {	/* -S */
		bool active;
		char *arg;
	} S;
	struct PSXY_T {	/* -T */
		bool active;
	} T;
	struct PSXY_W {	/* -W<pen> */
		bool active;
		bool cpt_effect;
		struct GMT_PEN pen;
	} W;
	struct PSXY_Z {	/* -Z[l|f]<value> */
		bool active;
		unsigned int mode;	/* 1 for line, 2 for fill, 3 for both */
		double value;
		char *file;
	} Z;
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

EXTERN_MSC double gmt_half_map_width (struct GMT_CTRL *GMT, double y);

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSXY_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSXY_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->E.pen = C->W.pen = GMT->current.setting.map_default_pen;
	gmt_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* Default is no fill */
	C->E.size = EBAR_CAP_WIDTH  * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 7p */
	C->N.mode = PSXY_CLIP_REPEAT;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSXY_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->S.arg);
	gmt_freepen (GMT, &C->W.pen);
	gmt_M_free (GMT, C);
}

GMT_LOCAL void plot_x_errorbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double delta_x[], double error_width2, int line, int kind) {
	double x_1, x_2, y_1, y_2;
	bool tip1, tip2;
	unsigned int first = 0, second = (kind == EBAR_ASYMMETRICAL) ? 1 : 0;	/* first and second are either both 0 or second is 1 for asymmetrical bars */

	tip1 = tip2 = (error_width2 > 0.0);
	gmt_geo_to_xy (GMT, x - fabs (delta_x[first]),  y, &x_1, &y_1);
	gmt_geo_to_xy (GMT, x + fabs (delta_x[second]), y, &x_2, &y_2);
	if (gmt_M_is_dnan (x_1)) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "X error bar exceeded domain near line %d. Reset to x_min\n", line);
		x_1 = GMT->current.proj.rect[XLO];
		tip1 = false;
	}
	if (gmt_M_is_dnan (x_2)) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "X error bar exceeded domain near line %d. Reset to x_max\n", line);
		x_2 = GMT->current.proj.rect[XHI];
		tip2 = false;
	}
	PSL_plotsegment (PSL, x_1, y_1, x_2, y_2);
	if (tip1) PSL_plotsegment (PSL, x_1, y_1 - error_width2, x_1, y_1 + error_width2);
	if (tip2) PSL_plotsegment (PSL, x_2, y_2 - error_width2, x_2, y_2 + error_width2);
}

GMT_LOCAL void plot_y_errorbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double delta_y[], double error_width2, int line, int kind) {
	double x_1, x_2, y_1, y_2;
	bool tip1, tip2;
	unsigned int first = 0, second = (kind == EBAR_ASYMMETRICAL) ? 1 : 0;	/* first and second are either both 0 or second is 1 for asymmetrical bars */

	tip1 = tip2 = (error_width2 > 0.0);
	gmt_geo_to_xy (GMT, x, y - fabs (delta_y[first]),  &x_1, &y_1);
	gmt_geo_to_xy (GMT, x, y + fabs (delta_y[second]), &x_2, &y_2);
	if (gmt_M_is_dnan (y_1)) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Y error bar exceeded domain near line %d. Reset to y_min\n", line);
		y_1 = GMT->current.proj.rect[YLO];
		tip1 = false;
	}
	if (gmt_M_is_dnan (y_2)) {
		GMT_Report (GMT->parent, GMT_MSG_WARNING, "Y error bar exceeded domain near line %d. Reset to y_max\n", line);
		y_2 = GMT->current.proj.rect[YHI];
		tip2 = false;
	}
	PSL_plotsegment (PSL, x_1, y_1, x_2, y_2);
	if (tip1) PSL_plotsegment (PSL, x_1 - error_width2, y_1, x_1 + error_width2, y_1);
	if (tip2) PSL_plotsegment (PSL, x_2 - error_width2, y_2, x_2 + error_width2, y_2);
}

GMT_LOCAL void plot_x_whiskerbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double hinge[], double error_width2, double rgb[], int line, int kind) {
	unsigned int i;
	static unsigned int q[4] = {0, 25, 75, 100};
	double xx[4], yy[4];

	for (i = 0; i < 4; i++) {	/* for 0, 25, 75, 100% hinges */
		gmt_geo_to_xy (GMT, hinge[i], y, &xx[i], &yy[i]);
		if (gmt_M_is_dnan (xx[i])) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "X %d %% hinge exceeded domain near line %d\n", q[i], line);
			xx[i] = (i < 2) ? GMT->current.proj.rect[XLO] : GMT->current.proj.rect[XHI];
		}
	}
	yy[1] -= error_width2;
	yy[2] += error_width2;

	PSL_plotsegment (PSL, xx[0], yy[1], xx[0], yy[2]);		/* Left whisker */
	PSL_plotsegment (PSL, xx[0], yy[0], xx[1], yy[0]);

	PSL_plotsegment (PSL, xx[3], yy[1], xx[3], yy[2]);		/* Right whisker */
	PSL_plotsegment (PSL, xx[3], yy[0], xx[2], yy[0]);

	PSL_setfill (PSL, rgb, 1);
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

GMT_LOCAL void plot_y_whiskerbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, double hinge[], double error_width2, double rgb[], int line, int kind) {
	unsigned int i;
	static unsigned int q[4] = {0, 25, 75, 100};
	double xx[4], yy[4];

	for (i = 0; i < 4; i++) {	/* for 0, 25, 75, 100% hinges */
		gmt_geo_to_xy (GMT, x, hinge[i], &xx[i], &yy[i]);
		if (gmt_M_is_dnan (yy[i])) {
			GMT_Report (GMT->parent, GMT_MSG_WARNING, "Y %d %% hinge exceeded domain near line %d\n", q[i], line);
			yy[i] = (i < 2) ? GMT->current.proj.rect[YLO] : GMT->current.proj.rect[YHI];
		}
	}
	xx[1] -= error_width2;
	xx[2] += error_width2;

	PSL_plotsegment (PSL, xx[1], yy[0], xx[2], yy[0]);		/* bottom whisker */
	PSL_plotsegment (PSL, xx[0], yy[0], xx[0], yy[1]);

	PSL_plotsegment (PSL, xx[1], yy[3], xx[2], yy[3]);		/* Top whisker */
	PSL_plotsegment (PSL, xx[0], yy[3], xx[0], yy[2]);

	PSL_setfill (PSL, rgb, 1);
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

GMT_LOCAL int plot_decorations (struct GMT_CTRL *GMT, struct GMT_DATASET *D) {
	/* Accept the dataset D with records of {x, y, size, angle, symbol} and plot rotated symbols at those locations.
	 * Note: The x,y are projected coordinates in inches, hence our -R -J choice below. */
	size_t len;
	char string[GMT_VF_LEN] = {""}, buffer[GMT_BUFSIZ] = {""}, tmp_file[PATH_MAX] = {""};
	FILE *fp = NULL;
	gmt_set_dataset_minmax (GMT, D);	/* Determine min/max for each column and add up total records */
	if (D->n_records == 0)	/* No symbols to plot */
		return GMT_NOERROR;

	/* Here we have symbols.  Open up virtual file for the call to psxy */
	if (GMT_Open_VirtualFile (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, D, string) != GMT_NOERROR)
		return (GMT->parent->error);
	if (GMT->parent->tmp_dir)	/* Make unique file in temp dir */
		sprintf (tmp_file, "%s/GMT_symbol%d.def", GMT->parent->tmp_dir, (int)getpid());
	else	/* Make unique file in current dir */
		sprintf (tmp_file, "GMT_symbol%d.def", (int)getpid());
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Number of decorated line symbols: %d\n", (int)D->n_records);
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Temporary decorated line symbol .def file created: %s\n", tmp_file);
	if ((fp = fopen (tmp_file, "w")) == NULL) {	/* Disaster */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to create symbol file needed for decorated lines: %s\n", tmp_file);
		return GMT_ERROR_ON_FOPEN;
	}
	/* Make a rotated plain symbol of type picked up from input file */
	fprintf (fp, "# Rotated standard symbol, need size and symbol code from data file\nN: 1 o\n$1 R\n0 0 1 ?\n");
	fclose (fp);
	len = strlen (tmp_file) - 4;	/* Position of the '.' since we know extension is .def */
	tmp_file[len] = '\0';	/* Temporarily hide the ".def" extension */
	/* Use -SK since our kustom symbol has a variable standard symbol ? that we must get from each data records */
	sprintf (buffer, "-R%g/%g/%g/%g -Jx1i -O -K -SK%s %s --GMT_HISTORY=false", GMT->current.proj.rect[XLO], GMT->current.proj.rect[XHI],
		GMT->current.proj.rect[YLO], GMT->current.proj.rect[YHI], tmp_file, string);
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Calling psxy with args %s\n", buffer);
	if (GMT_Call_Module (GMT->parent, "psxy", GMT_MODULE_CMD, buffer) != GMT_NOERROR)	/* Plot all the symbols */
		return (GMT->parent->error);
	if (GMT_Close_VirtualFile (GMT->parent, string) != GMT_NOERROR)
		return (GMT->parent->error);
	tmp_file[len] = '.';	/* Restore the ".def" extension so we can delete the file (unless -Vd) */
	if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG)) {	/* Leave the symbol def and txt files in the temp directory */
		char tmp_file2[GMT_LEN64] = {""};
		bool was = GMT->current.setting.io_header[GMT_OUT];	/* Save current setting */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Temporary symbol file for decorated lines saved: %s\n", tmp_file);
		if (GMT->parent->tmp_dir)	/* Make unique file in tmp dir */
			sprintf (tmp_file2, "%s/GMT_symbol%d.txt", GMT->parent->tmp_dir, (int)getpid());
		else	/* Make unique file in current dir */
			sprintf (tmp_file2, "GMT_symbol%d.txt", (int)getpid());
		sprintf (buffer, "-R -J -O -K -SK%s %s", tmp_file, tmp_file2);
		GMT_Set_Comment (GMT->parent, GMT_IS_DATASET, GMT_COMMENT_IS_TEXT | GMT_COMMENT_IS_COMMAND, buffer, D);
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Temporary data file for decorated lines saved: %s\n", tmp_file2);
		gmt_set_tableheader (GMT, GMT_OUT, true);	/* We need to ensure we write the header here */
		if (GMT_Write_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_IO_RESET, NULL, tmp_file2, D) != GMT_NOERROR) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to write file: %s\n", tmp_file2);
		}
		gmt_set_tableheader (GMT, GMT_OUT, was);	/* Restore what we had */
	}
	else {
		if (gmt_remove_file (GMT, tmp_file))	/* Just remove the symbol def file */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to delete file: %s\n", tmp_file);
	}

	return GMT_NOERROR;
}

GMT_LOCAL void plot_end_vectors (struct GMT_CTRL *GMT, double *x, double *y, uint64_t n, struct GMT_PEN *P) {
	/* Maybe add vector heads.  Here, x,y are in inches on the plot */
	unsigned int k, current[2] = {0,0}, next[2] = {1,0};
	double dim[PSL_MAX_DIMS], angle, s, c, L;
	char *end[2] = {"start", "end"};
	if (n < 2) return;	/* No line of mine */
	if (P->end[0].V == NULL && P->end[1].V == NULL) return;	/* No arrow heads requested */
	current[1] = (unsigned int)n-1;	next[1] = (unsigned int)n-2;
	PSL_command (GMT->PSL, "V\n");
	GMT->PSL->current.linewidth = -1.0;	/* Will be changed by next PSL_setlinewidth */
	gmt_M_memset (dim, PSL_MAX_DIMS, double);
	for (k = 0; k < 2; k++) {
		if (P->end[k].V == NULL) continue;
		/* Add vector heads to this end */
		PSL_comment (GMT->PSL, "Add vector head to %s of line\n", end[k]);
		angle = d_atan2d (y[current[k]] - y[next[k]], x[current[k]] - x[next[k]]);
		sincosd (angle, &s, &c);
		L = (P->end[k].V->v.v_kind[1] == PSL_VEC_TERMINAL) ? 1e-3 : P->end[k].length;
		P->end[k].V->v.v_width = (float)(P->end[k].V->v.pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);	/* Set symbol pen width */
		dim[0] = x[current[k]] + c * L; dim[1] = y[current[k]] + s * L;
		dim[2] = P->end[k].V->v.v_width, dim[3] = P->end[k].V->v.h_length, dim[4] = P->end[k].V->v.h_width;
		dim[5] = P->end[k].V->v.v_shape;
		dim[6] = (double)P->end[k].V->v.status;
		dim[7] = (double)P->end[k].V->v.v_kind[0];	dim[8]  = (double)P->end[k].V->v.v_kind[1];
		dim[9] = (double)P->end[k].V->v.v_trim[0];	dim[10] = (double)P->end[k].V->v.v_trim[1];
		if (P->end[k].V->v.status & PSL_VEC_OUTLINE2) {	/* Gave specific head outline pen */
			PSL_defpen (GMT->PSL, "PSL_vecheadpen", P->end[k].V->v.pen.width, P->end[k].V->v.pen.style, P->end[k].V->v.pen.offset, P->end[k].V->v.pen.rgb);
			dim[11] = P->end[k].V->v.pen.width;
		}
		else {	/* Set default based on line pen */
			PSL_defpen (GMT->PSL, "PSL_vecheadpen", 0.5 * P->width, P->style, P->offset, P->rgb);
			dim[11] = 0.5 * P->width;
		}
		gmt_setfill (GMT, &P->end[k].V->v.fill, (P->end[k].V->v.status & PSL_VEC_OUTLINE2) == PSL_VEC_OUTLINE2);
		PSL_plotsymbol (GMT->PSL, x[current[k]], y[current[k]], dim, PSL_VECTOR);
	}
	GMT->PSL->current.linewidth = -1.0;	/* Will be changed by next PSL_setlinewidth */
	PSL_command (GMT->PSL, "U\n");
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the psxy synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] %s %s [-A[m|p|x|y]]\n", name, GMT_J_OPT, GMT_Rgeoz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-C<cpt>] [-D<dx>/<dy>] [-E[x|y|X|Y][+a][+c[l|f]][+n][+p<pen>][+w<width>]] [-F<arg>] [-G<fill>]\n", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-I[<intens>]] %s[-L[+b|d|D][+xl|r|x0][+yb|t|y0][+p<pen>]] [-N[c|r]] %s%s\n", API->K_OPT, API->O_OPT, API->P_OPT);
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC)	/* -T has no purpose in modern mode */
		GMT_Message (API, GMT_TIME_NONE, "\t[-S[<symbol>][<size>]] [-T] [%s] [%s] [-W[<pen>][<attr>]]\n\t[%s] [%s] [-Z<value>|<file>[+f|l]] [%s]\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_a_OPT);
	else
		GMT_Message (API, GMT_TIME_NONE, "\t[-S[<symbol>][<size>]] [%s] [%s] [-W[<pen>][<attr>]]\n\t[%s] [%s] [-Z<arg>[+f|l]] [%s]\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_a_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] %s[%s] [%s]\n\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s]\n\t[%s] [%s] [%s] [%s]\n\n", GMT_bi_OPT, API->c_OPT, GMT_di_OPT, GMT_e_OPT, \
		GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_l_OPT, GMT_p_OPT, GMT_q_OPT, GMT_tv_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "J-Z,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Suppress drawing geographic line segments as great circle arcs, i.e., draw\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   straight lines unless m or p is appended to first follow meridian\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   then parallel, or vice versa.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For Cartesian data, use -Ax or -Ay to draw x- or y-staircase curves.\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Use CPT (or specify -Ccolor1,color2[,color3,...]) to assign symbol\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   colors based on z-value in 3rd column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: requires -S. Without -S, psxy excepts lines/polygons\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and looks for -Z<value> options in each segment header. Then, color is\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   applied for polygon fill (-L) or polygon pen (no -L).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Offset symbol or line positions by <dx>/<dy> [no offset].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Draw (symmetrical) standard error bars for x and/or y.  Append +a for\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   asymmetrical errors (reads two columns) [symmetrical reads one column].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If X or Y are specified instead then a box-and-whisker diagram is drawn,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   requiring four extra columns with the 0%%, 25%%, 75%%, and 100%% quantiles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (The x or y coordinate is expected to represent the 50%% quantile.)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Add cap-width with +w [%gp] and error pen attributes with +p<pen>\n", EBAR_CAP_WIDTH);
	GMT_Message (API, GMT_TIME_NONE, "\t   Given -C, use +cl to apply cpt color to error pen and +cf for error fill [both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +n for a notched box-and whisker (notch width represents uncertainty.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   in the median.  A 5th extra column with the sample size is required.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The settings of -W, -G affect the appearance of the 25-75%% box.\n");
	gmt_segmentize_syntax (API->GMT, 'F', 1);
	gmt_fill_syntax (API->GMT, 'G', NULL, "Specify color or pattern [no fill].");
	GMT_Message (API, GMT_TIME_NONE, "\t   -G option can be present in all segment headers (not with -S).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Use the intensity to modulate the fill color (requires -C or -G).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no intensity is given we expect it to follow symbol size in the data record.\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t   (k)ustom, (l)etter, (m)athangle, pe(n)tagon, (p)oint, (q)uoted line, (r)ectangle,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (R)ounded rectangle, (s)quare, (t)riangle, (v)ector, (w)edge, (x)cross, (y)dash,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   =(geovector, i.e., great or small circle vectors) or ~(decorated line).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no size is specified, then the 3rd column must have sizes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no symbol is specified, then last column must have symbol codes.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Note: if -C is selected then 3rd means 4th column, etc.]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Symbols A, C, D, G, H, I, N, S, T are adjusted to have same area\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   as a circle of the specified diameter.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Bars: Append +b[<base>] to give the y-value of the base [Default = 0 (1 for log-scales)].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Append u if width is in x-input units [Default is %s].\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t      Use +B instead if heights are measured relative to base [relative to origin].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Use upper case -SB for horizontal bars (<base> then refers to x\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      and width may be in y-units [Default is vertical]. To read the <base>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      value from file, specify +b with no trailing value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Decorated line: Give [d|f|l|n|s|x]<info>[:<symbolinfo>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     <code><info> controls placement of a symbol along lines.  Select\n");
	gmt_cont_syntax (API->GMT, 7, 2);
	GMT_Message (API, GMT_TIME_NONE, "\t     <symbolinfo> controls the symbol attributes.  Choose from\n");
	gmt_label_syntax (API->GMT, 7, 2);
	GMT_Message (API, GMT_TIME_NONE, "\t   Ellipses: Direction, major, and minor axis must be in columns 3-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SE rather than -Se is selected, psxy will expect azimuth, and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     axes [in km], and convert azimuths based on map projection.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use -SE- for a degenerate ellipse (circle) with only its diameter given\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     in column 3, or append a fixed diameter to -SE- instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append any of the units in %s to the axes [Default is k].\n", GMT_LEN_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t     For linear projection we scale the axes by the map scale.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Rotatable Rectangle: Direction, x- and y-dimensions in columns 3-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SJ rather than -Sj is selected, psxy will expect azimuth, and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     dimensions [in km] and convert azimuths based on map projection.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use -SJ- for a degenerate rectangle (square w/no rotation) with one dimension given\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     in column 3, or append a fixed dimension to -SJ- instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append any of the units in %s to the dimensions [Default is k].\n", GMT_LEN_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t     For linear projection we scale dimensions by the map scale.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Fronts: Give <tickgap>[/<ticklen>][+l|r][+<type>][+o<offset>][+p[<pen>]].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If <tickgap> is negative it means the number of gaps instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     The <ticklen> defaults to 15%% of <tickgap> if not given.  Append\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +l or +r   : Plot symbol to left or right of front [centered]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +<type>    : +b(ox), +c(ircle), +f(ault), +s|S(lip), +t(riangle) [f]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     	      +s optionally accepts the arrow angle [20].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       box      : square when centered, half-square otherwise.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       circle   : full when centered, half-circle otherwise.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       fault    : centered cross-tick or tick only in specified direction.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       slip     : left-or right-lateral strike-slip arrows.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       Slip     : Same but with curved arrow-heads.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       triangle : diagonal square when centered, directed triangle otherwise.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +o<offset> : Plot first symbol when along-front distance is offset [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +p[<pen>]  : Alternate pen for symbol outline; if no <pen> then no outline [Outline with -W pen].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Kustom: Append <symbolname> immediately after 'k'; this will look for\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     <symbolname>.def in the current directory, in $GMT_USERDIR,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     or in $GMT_SHAREDIR (searched in that order).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use upper case 'K' if your custom symbol refers a variable symbol, ?.\n");
	gmt_list_custom_symbols (API->GMT);
	GMT_Message (API, GMT_TIME_NONE, "\t   Letter: append +t<string> after symbol size, and optionally +f<font> and +j<justify>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Mathangle: radius, start, and stop directions of math angle must be in columns 3-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SM rather than -Sm is used, we draw straight angle symbol if 90 degrees.\n");
	gmt_vector_syntax (API->GMT, 0);
	GMT_Message (API, GMT_TIME_NONE, "\t   Quoted line: Give [d|f|l|n|s|x]<info>[:<labelinfo>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     <code><info> controls placement of labels along lines.  Select\n");
	gmt_cont_syntax (API->GMT, 7, 1);
	GMT_Message (API, GMT_TIME_NONE, "\t     <labelinfo> controls the label attributes.  Choose from\n");
	gmt_label_syntax (API->GMT, 7, 1);
	GMT_Message (API, GMT_TIME_NONE, "\t   Rectangles: x- and y-dimensions must be in columns 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +s if instead the diagonal corner coordinates are given in columns 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Rounded rectangles: x- and y-dimensions and corner radius must be in columns 3-5.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Vectors: Direction and length must be in columns 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SV rather than -Sv is selected, psxy will expect azimuth and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     length and convert azimuths based on the chosen map projection.\n");
	gmt_vector_syntax (API->GMT, 19);
	GMT_Message (API, GMT_TIME_NONE, "\t   Wedges: Start and stop directions of wedge must be in columns 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     If -SW rather than -Sw is selected, specify two azimuths instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -SW: Specify <size><unit> with units either from %s or %s [Default is k].\n", GMT_LEN_UNITS_DISPLAY, GMT_DIM_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t     -Sw: Specify <size><unit> with units from %s [Default is %s].\n", GMT_DIM_UNITS_DISPLAY,
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +a[<dr>] to just draw arc(s) or +r[<da>] to just draw radial lines [wedge].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Geovectors: Azimuth and length must be in columns 3-4.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append any of the units in %s to length [k].\n", GMT_LEN_UNITS_DISPLAY);
	gmt_vector_syntax (API->GMT, 3);
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC)	/* -T has no purpose in modern mode */
		GMT_Message (API, GMT_TIME_NONE, "\t-T Ignore all input files.\n");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Set pen attributes [Default pen is %s]:", 15);
	GMT_Option (API, "X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Use <value> with -C <cpt> to determine <color> instead of via -G<color> or -W<pen>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, specify <file> with z-values in last data column instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +l for just pen color or +f for fill color [Default sets both].\n");
	GMT_Option (API, "a,bi");
	if (gmt_M_showusage (API)) GMT_Message (API, GMT_TIME_NONE, "\t   Default is the required number of columns.\n");
	GMT_Option (API, "c,di,e,f,g,h,i,l,p,qi,t");
	GMT_Message (API, GMT_TIME_NONE, "\t   For plotting symbols with variable transparency read from file, give no value.\n");
	GMT_Option (API, ":,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL unsigned int parse_old_W (struct GMTAPI_CTRL *API, struct PSXY_CTRL *Ctrl, char *text) {
	unsigned int j = 0, n_errors = 0;
	if (text[j] == '-') {Ctrl->W.pen.cptmode = 1; j++;}
	if (text[j] == '+') {Ctrl->W.pen.cptmode = 3; j++;}
	if (text[j] && gmt_getpen (API->GMT, &text[j], &Ctrl->W.pen)) {
		gmt_pen_syntax (API->GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", 15);
		n_errors++;
	}
	return n_errors;
}

GMT_LOCAL unsigned int parse_old_E (struct GMTAPI_CTRL *API, struct PSXY_CTRL *Ctrl, char *text) {
	unsigned int j = 0, j0, n_errors = 0;
	char txt_a[GMT_LEN256] = {""};

	while (text[j] && text[j] != '/') {
		switch (text[j]) {
			case 'x':	/* Error bar for x */
				Ctrl->E.xbar = EBAR_NORMAL;
				if (text[j+1] == '+') { Ctrl->E.xbar = EBAR_ASYMMETRICAL; j++;}
				break;
			case 'X':	/* Box-whisker instead */
				Ctrl->E.xbar = EBAR_WHISKER;
				if (text[j+1] == 'n') {Ctrl->E.xbar = EBAR_NOTCHED_WHISKER; j++;}
				break;
			case 'y':	/* Error bar for y */
				Ctrl->E.ybar = EBAR_NORMAL;
				if (text[j+1] == '+') { Ctrl->E.ybar = EBAR_ASYMMETRICAL; j++;}
				break;
			case 'Y':	/* Box-whisker instead */
				Ctrl->E.ybar = EBAR_WHISKER;
				if (text[j+1] == 'n') {Ctrl->E.ybar = EBAR_NOTCHED_WHISKER; j++;}
				break;
			case '+':	/* Only allowed for -E+ as shorthand for -Ex+y+ */
				if (j == 0) Ctrl->E.xbar = Ctrl->E.ybar = EBAR_ASYMMETRICAL;
				break;
			default:	/* Get error 'cap' width */
				strncpy (txt_a, &text[j], GMT_LEN256);
				j0 = 0;
				while (txt_a[j0] && txt_a[j0] != '/') j0++;
				txt_a[j0] = 0;
				Ctrl->E.size = gmt_M_to_inch (API->GMT, txt_a);
				while (text[j] && text[j] != '/') j++;
				j--;
				break;
		}
		j++;
	}
	if (text[j] == '/') {
		j++;
		if (text[j] == '-') {Ctrl->E.mode = 1; j++;}
		if (text[j] == '+') {Ctrl->E.mode = 2; j++;}
		if (text[j] && gmt_getpen (API->GMT, &text[j], &Ctrl->E.pen)) {
			gmt_pen_syntax (API->GMT, 'E', NULL, "sets error bar pen attributes", 0);
			n_errors++;
		}
	}
	return (n_errors);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSXY_CTRL *Ctrl, struct GMT_OPTION *options, struct GMT_SYMBOL *S) {
	/* This parses the options provided to psxy and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, ztype, n_files = 0;
	int j;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Turn off draw_arc mode */
				Ctrl->A.active = true;
				switch (opt->arg[0]) {
					case 'm': case 'y': Ctrl->A.mode = GMT_STAIRS_Y; break;
					case 'p': case 'x': Ctrl->A.mode = GMT_STAIRS_X; break;

#ifdef DEBUG
					default: Ctrl->A.step = atof (opt->arg); break; /* Undocumented test feature */
#endif
				}
				break;
			case 'C':	/* Vary symbol color with z */
				Ctrl->C.active = true;
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':
				if ((j = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b)) < 1) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -D: Give x [and y] offsets\n");
					n_errors++;
				}
				else {
					Ctrl->D.dx = gmt_M_to_inch (GMT, txt_a);
					Ctrl->D.dy = (j == 2) ? gmt_M_to_inch (GMT, txt_b) : Ctrl->D.dx;
					Ctrl->D.active = true;
				}
				break;
			case 'E':	/* Get info for error bars and box-whisker bars */
				Ctrl->E.active = true;
				if (strstr (opt->arg, "+a") || strstr (opt->arg, "+c") || strstr (opt->arg, "+n") || strstr (opt->arg, "+p") || strstr (opt->arg, "+w")) {
					/* New parser for -E[x|y|X|Y][+a][+cl|f][+n][+p<pen>][+w<capwidth>] */
					char p[GMT_LEN64] = {""};
					unsigned int pos = 0;
					j = 0;
					while (opt->arg[j] != '+' && j < 2) {	/* Process one or two selections */
						switch (opt->arg[j]) {
							case 'x':	Ctrl->E.xbar = EBAR_NORMAL;  break;	/* Error bar for x */
							case 'y':	Ctrl->E.ybar = EBAR_NORMAL;  break;	/* Error bar for x */
							case 'X':	Ctrl->E.xbar = EBAR_WHISKER; break;	/* Box-whisker for x */
							case 'Y':	Ctrl->E.ybar = EBAR_WHISKER; break;	/* Box-whisker for y */
							default:
								GMT_Report (API, GMT_MSG_ERROR, "Option -E: Unrecognized error bar selection %c\n", opt->arg[j]);
								n_errors++;	break;
						}
						j++;
					}
					if (j == 0) Ctrl->E.xbar = Ctrl->E.ybar = EBAR_NORMAL;	/* Default is both x and y error bars */
					while (gmt_getmodopt (GMT, 'E', &(opt->arg[j]), "acnpw", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'a':	/* Asymmetrical error bars */
								if (Ctrl->E.xbar == EBAR_NORMAL) Ctrl->E.xbar = EBAR_ASYMMETRICAL;
								if (Ctrl->E.ybar == EBAR_NORMAL) Ctrl->E.ybar = EBAR_ASYMMETRICAL;
								break;
							case 'c':	/* Control CPT usage for fill or pen or both */
								switch (p[1]) {
									case 'l': Ctrl->E.mode = 1; break;
									case 'f': Ctrl->E.mode = 2; break;
									default:  Ctrl->E.mode = 3; break;
								}
								break;
							case 'n':	/* Notched box-and-whisker */
								if (Ctrl->E.xbar == EBAR_WHISKER) Ctrl->E.xbar = EBAR_NOTCHED_WHISKER;
								if (Ctrl->E.ybar == EBAR_WHISKER) Ctrl->E.ybar = EBAR_NOTCHED_WHISKER;
								break;
							case 'p':	/* Error bar pen */
								if (p[1] && gmt_getpen (GMT, &p[1], &Ctrl->E.pen)) {
									gmt_pen_syntax (GMT, 'E', NULL, "sets error bar pen attributes", 0);
									n_errors++;
								}
								break;
							case 'w':	/* Error bar cap width */
								Ctrl->E.size = gmt_M_to_inch (GMT, &p[1]);
								break;
							default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
				}
				else	/* Old parsing of -E[x[+]|y[+]|X|Y][n][cap][/[+|-]<pen>] */
					n_errors += parse_old_E (API, Ctrl, opt->arg);
				GMT_Report (API, GMT_MSG_DEBUG, "Settings for -E: x = %d y = %d\n", Ctrl->E.xbar, Ctrl->E.ybar);
				break;
			case 'F':
				Ctrl->F.active = true;
				n_errors += gmt_parse_segmentize (GMT, opt->option, opt->arg, 0, &(Ctrl->F.S));
				break;
			case 'G':		/* Set fill for symbols or polygon */
				Ctrl->G.active = true;
				if (!opt->arg[0] || gmt_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					gmt_fill_syntax (GMT, 'G', NULL, " "); n_errors++;
				}
				break;
			case 'I':	/* Adjust symbol color via intensity */
				Ctrl->I.active = true;
				if (opt->arg[0])
					Ctrl->I.value = atof (opt->arg);
				else
					Ctrl->I.mode = 1;
				break;
			case 'L':		/* Close line segments */
				Ctrl->L.active = true;
				if ((c = strstr (opt->arg, "+b")) != NULL)	/* Build asymmetric polygon from lower and upper bounds */
					Ctrl->L.anchor = PSXY_POL_ASYMM_ENV;
				else if ((c = strstr (opt->arg, "+d")) != NULL)	/* Build symmetric polygon from deviations about y(x) */
					Ctrl->L.anchor = PSXY_POL_SYMM_DEV;
				else if ((c = strstr (opt->arg, "+D")) != NULL)	/* Build asymmetric polygon from deviations about y(x) */
					Ctrl->L.anchor = PSXY_POL_ASYMM_DEV;
				else if ((c = strstr (opt->arg, "+x")) != NULL) {	/* Parse x anchors for a polygon */
					switch (c[2]) {
						case 'l':	Ctrl->L.mode = XLO;	break;	/* Left side anchors */
						case 'r':	Ctrl->L.mode = XHI;	break;	/* Right side anchors */
						default:	Ctrl->L.mode = ZLO;	Ctrl->L.value = atof (&c[2]);	break;	/* Arbitrary x anchor */
					}
					Ctrl->L.anchor = PSXY_POL_X;
				}
				else if ((c = strstr (opt->arg, "+y")) != NULL) {	/* Parse y anchors for a polygon */
					switch (c[2]) {
						case 'b':	Ctrl->L.mode = YLO;	break;	/* Bottom side anchors */
						case 't':	Ctrl->L.mode = YHI;	break;	/* Top side anchors */
						default:	Ctrl->L.mode = ZHI;	Ctrl->L.value = atof (&c[2]);	break;	/* Arbitrary y anchor */
					}
					Ctrl->L.anchor = PSXY_POL_Y;
				}
				else	/* Just force a closed polygon */
					Ctrl->L.polygon = true;
				if ((c = strstr (opt->arg, "+p")) != NULL) {	/* Want outline */
					if (c[2] && gmt_getpen (GMT, &c[2], &Ctrl->L.pen)) {
						gmt_pen_syntax (GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", 3);
						n_errors++;
					}
					Ctrl->L.outline = 1;
				}
				break;
			case 'N':		/* Do not skip points outside border */
				Ctrl->N.active = true;
				if (opt->arg[0] == 'r') Ctrl->N.mode = PSXY_NO_CLIP_REPEAT;
				else if (opt->arg[0] == 'c') Ctrl->N.mode = PSXY_CLIP_NO_REPEAT;
				else if (opt->arg[0] == '\0') Ctrl->N.mode = PSXY_NO_CLIP_NO_REPEAT;
				else {
					GMT_Report (API, GMT_MSG_ERROR, "Option -N: Unrecognized argument %s\n", opt->arg);
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
				if (opt->arg[0] == '-' || (opt->arg[0] == '+' && opt->arg[1] != 'c')) {	/* Definitively old-style args */
					if (gmt_M_compat_check (API->GMT, 5)) {	/* Sorry */
						GMT_Report (API, GMT_MSG_ERROR, "Your -W syntax is obsolete; see program usage.\n");
						n_errors++;
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Your -W syntax is obsolete; see program usage.\n");
						n_errors += parse_old_W (API, Ctrl, opt->arg);
					}
				}
				else if (opt->arg[0]) {
					if (gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
						gmt_pen_syntax (GMT, 'W', NULL, "sets pen attributes [Default pen is %s]:", 11);
						n_errors++;
					}
				}
				if (Ctrl->W.pen.cptmode) Ctrl->W.cpt_effect = true;
				break;

			case 'Z':		/* Get value for CPT lookup */
				Ctrl->Z.active = true;	j = 0;
				if ((c = strstr (opt->arg, "+f")) || (c = strstr (opt->arg, "+l"))) {	/* New syntax */
					if (c[1] == 'l') Ctrl->Z.mode = 1;
					else if (c[1] == 'f') Ctrl->Z.mode = 2;
					c[0] = '\0';	/* Chop off modifier */
				}
				else if (strchr ("lf", opt->arg[0]) && !gmt_not_numeric (GMT, &opt->arg[1])) {	/* Old-style leading l|f followed by value */
					j = 1;
					switch (opt->arg[0]) {
						case 'l': Ctrl->Z.mode = 1; break;
						case 'f': Ctrl->Z.mode = 2; break;
						default:  break;	/* Cannot get here */
					}
				}
				else	/* No specifications for f or l or +f or +l */
					Ctrl->Z.mode = 3;
				if (gmt_not_numeric (GMT, &opt->arg[j]) && !gmt_access (GMT, &opt->arg[j], R_OK)) {	/* Got a file */
					Ctrl->Z.file = strdup (&opt->arg[j]);
					n_errors += gmt_M_check_condition (GMT, Ctrl->Z.file && gmt_access (GMT, Ctrl->Z.file, R_OK),
					                                   "Option -Z: Cannot read file %s!\n", Ctrl->Z.file);
				}
				else {	/* Got a value */
					ztype = (strchr (opt->arg, 'T')) ? GMT_IS_ABSTIME : gmt_M_type (GMT, GMT_IN, GMT_Z);
					n_errors += gmt_verify_expectations (GMT, ztype, gmt_scanf_arg (GMT, &opt->arg[j], ztype, false, &Ctrl->Z.value), &opt->arg[j]);
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	gmt_consider_current_cpt (API, &Ctrl->C.active, &(Ctrl->C.file));

	/* Check that the options selected are mutually consistent */

	if (Ctrl->T.active && n_files) GMT_Report (API, GMT_MSG_WARNING, "Option -T ignores all input files\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && !Ctrl->C.active, "Option -Z: No CPT given via -C\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->G.active, "Option -Z: Not compatible with -G\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && (Ctrl->C.file == NULL || Ctrl->C.file[0] == '\0'), "Option -C: No CPT given\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && gmt_parse_symbol_option (GMT, Ctrl->S.arg, S, 0, true), "Option -S: Parsing failure\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && (S->symbol == PSL_VECTOR || S->symbol == GMT_SYMBOL_GEOVECTOR || S->symbol == PSL_MARC \
		|| S->symbol == PSL_ELLIPSE || S->symbol == GMT_SYMBOL_FRONT || S->symbol == GMT_SYMBOL_QUOTED_LINE || S->symbol == GMT_SYMBOL_DECORATED_LINE || S->symbol == PSL_ROTRECT),
		"Option -E: Incompatible with -Se, -Sf, -Sj, -Sm|M, -Sq, -Sv|V, -S=\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && S->symbol == GMT_SYMBOL_NOT_SET, "Binary input data cannot have symbol information\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && Ctrl->E.mode && !Ctrl->C.active, "Option -E: +|-<pen> requires the -C option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.pen.cptmode && !Ctrl->C.active, "Option -W modifier +c requires the -C option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && (Ctrl->W.pen.cptmode + Ctrl->E.mode) == 3, "Conflicting -E and -W options regarding -C option application\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.anchor && (!Ctrl->G.active && !Ctrl->Z.active) && !Ctrl->L.outline, "Option -L<modifiers> must include +p<pen> if -G not given\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->I.mode == 1 && !Ctrl->S.active, "Option -I with no argument is only applicable for symbols\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_plot (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: plot\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psxy (V_API, mode, args);
}

int GMT_psxy (void *V_API, int mode, void *args) {
	/* High-level function that implements the psxy task */
	bool polygon, penset_OK = true, not_line, old_is_world, rgb_from_z = false;
	bool get_rgb = false, clip_set = false, fill_active, may_intrude_inside = false;
	bool error_x = false, error_y = false, def_err_xy = false, can_update_headpen = true;
	bool default_outline, outline_active, geovector = false, save_W = false, save_G = false, QR_symbol = false;
	unsigned int n_needed, n_cols_start = 2, justify, tbl;
	unsigned int n_total_read = 0, j, geometry, icol = 0, tcol = 0;
	unsigned int bcol, ex1, ex2, ex3, change = 0, pos2x, pos2y, save_u = false;
	unsigned int xy_errors[2], error_type[2] = {EBAR_NONE, EBAR_NONE}, error_cols[5] = {0,1,2,4,5};
	int error = GMT_NOERROR, outline_setting;

	char s_args[GMT_BUFSIZ] = {""};

	double direction, length, dx, dy, d, *in = NULL, *z_for_cpt = NULL;
	double s, c, plot_x, plot_y, x_1, x_2, y_1, y_2, headpen_width = 0.25;

	struct GMT_PEN current_pen, default_pen, save_pen, last_headpen, last_spiderpen;
	struct GMT_FILL current_fill, default_fill, black, no_fill, save_fill;
	struct GMT_SYMBOL S;
	struct GMT_PALETTE *P = NULL;
	struct GMT_PALETTE_HIDDEN *PH = NULL;
	struct GMT_DATASET *Decorate = NULL, *Zin = NULL;
	struct GMT_DATASEGMENT *L = NULL;
	struct PSXY_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);

	/* Initialize GMT_SYMBOL structure */

	gmt_M_memset (&S, 1, struct GMT_SYMBOL);
	gmt_M_memset (&last_headpen, 1, struct GMT_PEN);
	gmt_M_memset (&last_spiderpen, 1, struct GMT_PEN);

	gmt_contlabel_init (GMT, &S.G, 0);
	xy_errors[GMT_X] = xy_errors[1] = 0;	/* These will be col # of where to find this info in data */
	gmt_init_fill (GMT, &black, 0.0, 0.0, 0.0);	/* Default fill for points, if needed */
	gmt_init_fill (GMT, &no_fill, -1.0, -1.0, -1.0);

	S.base = GMT->session.d_NaN;
	S.font = GMT->current.setting.font_annot[GMT_PRIMARY];
	S.u = GMT->current.setting.proj_length_unit;
	S.justify = PSL_MC;
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options, &S)) != 0) Return (error);

	/*---------------------------- This is the psxy main code ----------------------------*/

	if (!Ctrl->T.active) GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");
	if (Ctrl->E.active && S.symbol == GMT_SYMBOL_LINE)	/* Assume user only wants error bars */
		S.symbol = GMT_SYMBOL_NONE;
	/* Do we plot actual symbols, or lines */
	not_line = !(S.symbol == GMT_SYMBOL_FRONT || S.symbol == GMT_SYMBOL_QUOTED_LINE || S.symbol == GMT_SYMBOL_DECORATED_LINE || S.symbol == GMT_SYMBOL_LINE);
	if (Ctrl->E.active) {	/* Set error bar parameters */
		j = 2;	/* Normally, error bar related columns start in column 2 */
		if (Ctrl->E.xbar != EBAR_NONE) { xy_errors[GMT_X] = j;	j += error_cols[Ctrl->E.xbar]; error_type[GMT_X] = Ctrl->E.xbar;}
		if (Ctrl->E.ybar != EBAR_NONE) { xy_errors[GMT_Y] = j;	error_type[GMT_Y] = Ctrl->E.ybar;}
		if (!(xy_errors[GMT_X] || xy_errors[GMT_Y])) {	/* Default is plain error bars for both */
			def_err_xy = true;
			xy_errors[GMT_X] = 2;	/* Assumes xy input, later check for -: */
			xy_errors[GMT_Y] = 3;
			error_type[GMT_X] = error_type[GMT_Y] = EBAR_NORMAL;
		}
	}

	if (Ctrl->C.active) {
		if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
		get_rgb = not_line;	/* Need to assign color from either z or text from input data file */
		PH = gmt_get_C_hidden (P);
		if (Ctrl->Z.active) {	/* Get color from cpt -Z and store in -G */
			if (Ctrl->Z.file) {
				if ((Zin = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_IO_ASCII, NULL, Ctrl->Z.file, NULL)) == NULL) {
					Return (API->error);
				}
				if (Ctrl->Z.mode & 1)	/* To be used in polygon or symbol outline */
					Ctrl->W.active = true;	/* -C plus -Zl = -W */
				if (Ctrl->Z.mode & 2)	/* To be used in polygon or symbol fill */
					Ctrl->G.active = true;	/* -C plus -Zf = -G */
			}
			else {
				double rgb[4];
				(void)gmt_get_rgb_from_z (GMT, P, Ctrl->Z.value, rgb);
				if (Ctrl->Z.mode & 1) {	/* To be used in polygon or symbol outline */
					gmt_M_rgb_copy (Ctrl->W.pen.rgb, rgb);
					Ctrl->W.active = true;	/* -C plus -Zl = -W */
				}
				if (Ctrl->Z.mode & 2) {	/* To be used in polygon or symbol fill */
					gmt_M_rgb_copy (Ctrl->G.fill.rgb, rgb);
					Ctrl->G.active = true;	/* -C plus -Zf = -G */
				}
			}
			get_rgb = false;	/* Not reading z from data */
		}
		else if ((P->categorical & 2))	/* Get rgb from trailing text, so read no extra z columns */
			rgb_from_z = false;
		else {	/* Read extra z column for symbols only */
			rgb_from_z = not_line;
			if (rgb_from_z && (P->categorical & 2) == 0) n_cols_start++;
		}
	}

	polygon = (S.symbol == GMT_SYMBOL_LINE && (Ctrl->G.active || Ctrl->L.polygon) && !Ctrl->L.anchor);
	if (S.symbol == PSL_DOT) penset_OK = false;	/* Dots have no outline */

	Ctrl->E.size *= 0.5;	/* Since we draw half-way in either direction */

	current_pen = default_pen = Ctrl->W.pen;
	current_fill = default_fill = (S.symbol == PSL_DOT && !Ctrl->G.active) ? black : Ctrl->G.fill;
	default_outline = Ctrl->W.active;
	if (Ctrl->I.active && Ctrl->I.mode == 0) {
		gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
		gmt_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
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
	ex1 = (rgb_from_z) ? 3 : 2;
	ex2 = (rgb_from_z) ? 4 : 3;
	ex3 = (rgb_from_z) ? 5 : 4;
	pos2x = ex1 + GMT->current.setting.io_lonlat_toggle[GMT_IN];	/* Column with a 2nd longitude (for VECTORS with two sets of coordinates) */
	pos2y = ex2 - GMT->current.setting.io_lonlat_toggle[GMT_IN];	/* Column with a 2nd latitude (for VECTORS with two sets of coordinates) */

	if (Ctrl->E.active) {
		if (S.read_size) gmt_set_column (GMT, GMT_IN, ex1, GMT_IS_DIMENSION);	/* Must read symbol size from data record */
		if (def_err_xy && GMT->current.setting.io_lonlat_toggle[GMT_IN]) {	/* With -:, -E should become -Eyx */
			gmt_M_uint_swap (xy_errors[GMT_X], xy_errors[GMT_Y]);
			gmt_M_uint_swap (error_type[GMT_X], error_type[GMT_Y]);
		}
		if (xy_errors[GMT_X]) n_cols_start += error_cols[error_type[GMT_X]], error_x = true;
		if (xy_errors[GMT_Y]) n_cols_start += error_cols[error_type[GMT_Y]], error_y = true;
		xy_errors[GMT_X] += (S.read_size + rgb_from_z);	/* Move 0-2 columns over */
		xy_errors[GMT_Y] += (S.read_size + rgb_from_z);
	}
	else if (not_line)	/* Here we have the usual x y [z] [size] [other args] [symbol] record */
		for (j = n_cols_start; j < 6; j++) gmt_set_column (GMT, GMT_IN, j, GMT_IS_DIMENSION);		/* Since these may have units appended */
	for (j = 0; j < S.n_nondim; j++) gmt_set_column (GMT, GMT_IN, S.nondim_col[j]+rgb_from_z, GMT_IS_FLOAT);	/* Since these are angles, not dimensions */

	n_needed = n_cols_start + S.n_required;
	if (Ctrl->I.mode) {
		n_needed++;	/* Read intensity from data file */
		icol = n_needed - 1;
		gmt_set_column (GMT, GMT_IN, icol, GMT_IS_FLOAT);
	}
	if (GMT->common.t.variable) {
		n_needed++;	/* Read transparency from data file */
		tcol = n_needed - 1;
		gmt_set_column (GMT, GMT_IN, tcol, GMT_IS_FLOAT);
	}
	if (gmt_check_binary_io (GMT, n_needed))
		Return (GMT_RUNTIME_ERROR);
	GMT_Report (API, GMT_MSG_DEBUG, "Operation will require %d input columns [n_cols_start = %d]\n", n_needed, n_cols_start);

	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), ""))
		Return (GMT_PROJECTION_ERROR);
	if (S.u_set) {	/* When -Sc<unit> is given we temporarily reset the system unit to these units so conversions will work */
		save_u = GMT->current.setting.proj_length_unit;
		GMT->current.setting.proj_length_unit = S.u;
	}

	if (S.G.delay) GMT->current.ps.nclip = +2;	/* Signal that this program initiates clipping that will outlive this process */

	if ((PSL = gmt_plotinit (GMT, options)) == NULL)
		Return (GMT_RUNTIME_ERROR);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	if (Ctrl->T.active) {	/* Honor canvas fill and box draw */
		gmt_map_basemap (GMT);
		gmt_plane_perspective (GMT, -1, 0.0);
		gmt_plotend (GMT);
		Return (GMT_NOERROR);
	}

	gmt_map_basemap (GMT);	/* Lay down any gridlines before symbols */

	if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {
		if (gmt_contlabel_prep (GMT, &S.G, NULL)) Return (GMT_RUNTIME_ERROR);	/* Needed after map_setup */
		penset_OK = false;	/* The pen for quoted lines are set within the PSL code itself so we don't do it here in psxy */
	}
	else if (S.symbol == GMT_SYMBOL_DECORATED_LINE) {
		if (gmt_decorate_prep (GMT, &S.D, NULL)) Return (GMT_RUNTIME_ERROR);	/* Needed after map_setup */
	}

	gmt_set_line_resampling (GMT, Ctrl->A.active, Ctrl->A.mode);	/* Possibly change line resampling mode */
#ifdef DEBUG
	/* Change default step size (in degrees) used for interpolation of line segments along great circles (if requested) */
	if (Ctrl->A.active) Ctrl->A.step = Ctrl->A.step / GMT->current.proj.scale[GMT_X] / GMT->current.proj.M_PR_DEG;
#endif
	if ((gmt_M_is_conical(GMT) && gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]))) {	/* Must turn clipping on for 360-range conical */
		/* Special case of 360-range conical (which is periodic but do not touch at w=e) so we must clip to ensure nothing is plotted in the gap between west and east border */
		clip_set = true;
	}
	else if (not_line && (Ctrl->N.mode == PSXY_CLIP_REPEAT || Ctrl->N.mode == PSXY_CLIP_NO_REPEAT))	/* Only set clip if plotting symbols and -N not used */
		clip_set = true;
	if (clip_set) gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);
	if (S.symbol == GMT_SYMBOL_TEXT && Ctrl->G.active && !Ctrl->W.active) PSL_setcolor (PSL, current_fill.rgb, PSL_IS_FILL);
	if (S.symbol == GMT_SYMBOL_TEXT) gmt_setfont (GMT, &S.font);	/* Set the required font */
	if (S.symbol == GMT_SYMBOL_BARX && !S.base_set) S.base = GMT->common.R.wesn[XLO];	/* Default to west level for horizontal log10 bars */
	if (S.symbol == GMT_SYMBOL_BARY && !S.base_set) S.base = GMT->common.R.wesn[YLO];	/* Default to south level for vertical log10 bars */
	if ((S.symbol == PSL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR) && S.v.status & PSL_VEC_JUST_S) {	/* One of the vector symbols, and require 2nd point */
		/* Reading 2nd coordinate so must set column types */
		gmt_set_column (GMT, GMT_IN, pos2x, gmt_M_type (GMT, GMT_IN, GMT_X));
		gmt_set_column (GMT, GMT_IN, pos2y, gmt_M_type (GMT, GMT_IN, GMT_Y));
	}
	if (S.symbol == PSL_RECT && S.diagonal) {	/* Getting lon1,lat1,lon2,lat2 of a diagonal and will draw the rectangle subject to -A */
		/* Reading 2nd coordinate so must set column types */
		gmt_set_column (GMT, GMT_IN, pos2x, gmt_M_type (GMT, GMT_IN, GMT_X));
		gmt_set_column (GMT, GMT_IN, pos2y, gmt_M_type (GMT, GMT_IN, GMT_Y));
	}
	if (S.symbol == PSL_VECTOR && S.v.status & PSL_VEC_COMPONENTS)
		gmt_set_column (GMT, GMT_IN, pos2y, GMT_IS_FLOAT);	/* Just the users dy component, not length */
	if (S.symbol == PSL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR || S.symbol == PSL_MARC ) {	/* One of the vector symbols */
		geovector = (S.symbol == GMT_SYMBOL_GEOVECTOR);
		if ((S.v.status & PSL_VEC_FILL) == 0 && !S.v.parsed_v4) Ctrl->G.active = false;	/* Want no fill so override -G */
		if (S.v.status & PSL_VEC_FILL2) {	/* Gave +g<fill> to set head fill; odd, but overrides -G (and sets -G true) */
			current_fill = S.v.fill;	/* Override any -G<fill> with specified head fill */
			if (S.v.status & PSL_VEC_FILL) Ctrl->G.active = true;
		}
		else if (S.v.status & PSL_VEC_FILL) {
			current_fill = default_fill, Ctrl->G.active = true;	/* Return to default fill */
		}
		if (S.v.status & PSL_VEC_OUTLINE2) {	/* Vector head outline pen specified separately */
			PSL_defpen (PSL, "PSL_vecheadpen", S.v.pen.width, S.v.pen.style, S.v.pen.offset, S.v.pen.rgb);
			headpen_width = S.v.pen.width;
			last_headpen = S.v.pen;
		}
		else {	/* Reset to default pen */
			current_pen = default_pen, Ctrl->W.active = true;	/* Return to default pen */
			if (Ctrl->W.active) {	/* Vector head outline pen default is half that of stem pen */
				PSL_defpen (PSL, "PSL_vecheadpen", current_pen.width, current_pen.style, current_pen.offset, current_pen.rgb);
				headpen_width = 0.5 * current_pen.width;
				last_headpen = current_pen;
			}
		}
		if (Ctrl->C.active) {	/* Head fill and/or pen will be set via CPT lookup */
			if (!Ctrl->W.cpt_effect || (Ctrl->W.cpt_effect && (Ctrl->W.pen.cptmode & 2)))
				Ctrl->G.active = false;	/* Must turn off -G so that color is not reset to Ctrl->G.fill after the -C effect */
		}
	}
	bcol = (S.read_size) ? ex2 : ex1;
	if (S.symbol == GMT_SYMBOL_BARX && S.base_set & 2) gmt_set_column (GMT, GMT_IN, bcol, gmt_M_type (GMT, GMT_IN, GMT_X));
	if (S.symbol == GMT_SYMBOL_BARY && S.base_set & 2) gmt_set_column (GMT, GMT_IN, bcol, gmt_M_type (GMT, GMT_IN, GMT_Y));
	if (S.symbol == GMT_SYMBOL_GEOVECTOR && (S.v.status & PSL_VEC_JUST_S) == 0)
		gmt_set_column (GMT, GMT_IN, ex2, GMT_IS_GEODIMENSION);
	else if ((S.symbol == PSL_ELLIPSE || S.symbol == PSL_ROTRECT) && S.convert_angles) {
		if (S.n_required == 1)
			gmt_set_column (GMT, GMT_IN, ex1, GMT_IS_GEODIMENSION);
		else {
			gmt_set_column (GMT, GMT_IN, ex2, GMT_IS_GEODIMENSION);
			gmt_set_column (GMT, GMT_IN, ex3, GMT_IS_GEODIMENSION);
		}
	}
	if (S.symbol == PSL_WEDGE) {
		if (S.v.status == PSL_VEC_OUTLINE2) {	/* Wedge spider pen specified separately */
			PSL_defpen (PSL, "PSL_spiderpen", S.v.pen.width, S.v.pen.style, S.v.pen.offset, S.v.pen.rgb);
			last_spiderpen = S.v.pen;
		}
		else if (Ctrl->W.active || S.w_type || !(Ctrl->G.active || Ctrl->C.active)) {	/* Use -W as wedge pen as well as outline, and default to this pen if neither -C, -W or -G given */
			current_pen = default_pen, Ctrl->W.active = true;	/* Return to default pen */
			if (Ctrl->W.active) {	/* Vector head outline pen default is half that of stem pen */
				PSL_defpen (PSL, "PSL_spiderpen", current_pen.width, current_pen.style, current_pen.offset, current_pen.rgb);
				last_spiderpen = current_pen;
			}
		}
	}
	if (penset_OK) gmt_setpen (GMT, &current_pen);

	QR_symbol = (S.symbol == GMT_SYMBOL_CUSTOM && (!strcmp (S.custom->name, "QR") || !strcmp (S.custom->name, "QR_transparent")));
	fill_active = Ctrl->G.active;	/* Make copies because we will change the values */
	outline_active =  Ctrl->W.active;
	if (not_line && !outline_active && S.symbol != PSL_WEDGE && !fill_active && !get_rgb && !QR_symbol) outline_active = true;	/* If no fill nor outline for symbols then turn outline on */

	if (Ctrl->D.active) PSL_setorigin (PSL, Ctrl->D.dx, Ctrl->D.dy, 0.0, PSL_FWD);	/* Shift plot a bit */

	old_is_world = GMT->current.map.is_world;
	geometry = not_line ? GMT_IS_POINT : ((polygon) ? GMT_IS_POLY: GMT_IS_LINE);
	in = GMT->current.io.curr_rec;

	if (not_line) {	/* Symbol part (not counting GMT_SYMBOL_FRONT, GMT_SYMBOL_QUOTED_LINE, GMT_SYMBOL_DECORATED_LINE) */
		bool periodic = false, delayed_unit_scaling = false;
		unsigned int n_warn[3] = {0, 0, 0}, warn, item, n_times;
		double xpos[2], width = 0.0, dim[PSL_MAX_DIMS];
		struct GMT_RECORD *In = NULL;
		struct GMT_DATASET *Diag = NULL;
		struct GMT_DATASEGMENT *S_Diag = NULL;

		if (S.read_symbol_cmd)	/* Must prepare for a rough ride */
			GMT_Set_Columns (API, GMT_IN, 0, GMT_COL_VAR);
		else { /* Fixed symbol type throughout */
			GMT_Set_Columns (API, GMT_IN, n_needed, GMT_COL_FIX);
			if (GMT->common.l.active && !get_rgb && (!S.read_size || GMT->common.l.item.size > 0.0)) {
				/* For specified symbol, size, color we can do an auto-legend entry under modern mode */
				gmt_add_legend_item (API, &S, Ctrl->G.active, &(Ctrl->G.fill), Ctrl->W.active, &(Ctrl->W.pen), &(GMT->common.l.item));
			}
		}
		/* Determine if we need to worry about repeating periodic symbols */
		if ((Ctrl->N.mode == PSXY_CLIP_REPEAT || Ctrl->N.mode == PSXY_NO_CLIP_REPEAT) && gmt_M_360_range (GMT->common.R.wesn[XLO], GMT->common.R.wesn[XHI]) && gmt_M_is_geographic (GMT, GMT_IN)) {
			/* Only do this for projection where west and east are split into two separate repeating boundaries */
			periodic = (gmt_M_is_cylindrical (GMT) || gmt_M_is_misc (GMT));
		}
		n_times = (periodic) ? 2 : 1;	/* For periodic boundaries we plot each symbol twice to allow for periodic clipping */

		if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
			Return (API->error);
		}
		if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {		/* Enables data input and sets access mode */
			Return (API->error);
		}
		PSL_command (GMT->PSL, "V\n");	/* Place all symbols under a gsave/grestore clause */

		if (S.read_size && GMT->current.io.col[GMT_IN][ex1].convert) {	/* Doing math on the size column, must delay unit conversion unless inch */
			gmt_set_column (GMT, GMT_IN, ex1, GMT_IS_FLOAT);
			if (S.u_set)	/* Specified a particular unit, so scale values unless we chose inches */
				delayed_unit_scaling = (S.u != GMT_INCH);
			else if (GMT->current.setting.proj_length_unit != GMT_INCH) {	/* Gave no unit but default unit is not inch so we must scale */
				delayed_unit_scaling = true;
				S.u = GMT->current.setting.proj_length_unit;
			}
		}
		if (QR_symbol) {
			if (!Ctrl->G.active)	/* Default to black */
				PSL_command (PSL, "/QR_fill {0 A} def\n");
			if (!Ctrl->W.active)	/* No outline of QR code */
				PSL_command (PSL, "/QR_outline false def\n");
		}

		if (S.diagonal) {
			uint64_t dim[GMT_DIM_SIZE] = {1, 1, 5, 2};	/* Put everything in one table */
			if ((Diag = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POLYGON, GMT_NO_STRINGS, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);
			S_Diag = Diag->table[0]->segment[0];
		}
		do {	/* Keep returning records until we reach EOF */
			if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				}
				else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
					break;
				else if (gmt_M_rec_is_segment_header (GMT)) {			/* Parse segment headers */
					PSL_comment (PSL, "Segment header: %s\n", GMT->current.io.segment_header);
					(void)gmt_parse_segment_header (GMT, GMT->current.io.segment_header, P, &fill_active, &current_fill, &default_fill, &outline_active, &current_pen, &default_pen, default_outline, NULL);
					if (Ctrl->I.active && Ctrl->I.mode == 0) {
						gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
						gmt_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
					}
					if (S.read_symbol_cmd) API->object[API->current_item[GMT_IN]]->n_expected_fields = GMT_MAX_COLUMNS;
					if (gmt_parse_segment_item (GMT, GMT->current.io.segment_header, "-S", s_args)) {	/* Found -Sargs */
						if (!(s_args[0] == 'q'|| s_args[0] == 'f')) { /* Update parameters */
							gmt_parse_symbol_option (GMT, s_args, &S, 0, false);
						}
						else
							GMT_Report (API, GMT_MSG_ERROR, "Segment header tries to switch to a line symbol like quoted line or fault - ignored\n");
					}
				}
				continue;							/* Go back and read the next record */
			}
			outline_setting = (outline_active) ? 1 : 0;

			/* Data record to process */

			in = In->data;
			n_total_read++;

			if (S.read_symbol_cmd) {	/* Must do special processing depending on variable symbol */
				/* First establish the symbol type given at the end of the record */
				if (!In->text) {
					GMT_Report (API, GMT_MSG_ERROR, "ERROR: no symbol code was provided in the -S option.\n");
					Return (GMT_RUNTIME_ERROR);
				}
				if (S.read_symbol_cmd == 1) gmt_parse_symbol_option (GMT, In->text, &S, 0, false);
				QR_symbol = (S.symbol == GMT_SYMBOL_CUSTOM && (!strcmp (S.custom->name, "QR") || !strcmp (S.custom->name, "QR_transparent")));
				/* Since we only now know if some of the input columns should NOT be considered dimensions we
				 * must visit such columns and if the current length unit is NOT inch then we must undo the scaling */
				if (S.n_nondim && GMT->current.setting.proj_length_unit != GMT_INCH) {	/* Since these are not dimensions but angles or other quantities */
					for (j = 0; j < S.n_nondim; j++) in[S.nondim_col[j]+rgb_from_z] *= GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];
				}

				if (S.symbol == PSL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR || S.symbol == PSL_MARC) {	/* One of the vector symbols */
					save_pen = current_pen; save_fill = current_fill; save_W = Ctrl->W.active; save_G = Ctrl->G.active;	/* Save status before change */
					can_update_headpen = true;
					if (S.v.status & PSL_VEC_OUTLINE2) {	/* Vector head ouline pen specified separately */
						if (!gmt_M_same_pen (S.v.pen, last_headpen)) {
							PSL_defpen (PSL, "PSL_vecheadpen", S.v.pen.width, S.v.pen.style, S.v.pen.offset, S.v.pen.rgb);
							headpen_width = S.v.pen.width;
							last_headpen = S.v.pen;
						}
						can_update_headpen = false;
					}
					else {	/* Reset to default pen (or possibly not used) */
						current_pen = default_pen, Ctrl->W.active = true;	/* Return to default pen */
						if (Ctrl->W.active && !gmt_M_same_pen (current_pen, last_headpen)) {	/* Vector head outline pen default is half that of stem pen */
							PSL_defpen (PSL, "PSL_vecheadpen", current_pen.width, current_pen.style, current_pen.offset, current_pen.rgb);
							headpen_width = 0.5 * current_pen.width;
							last_headpen = current_pen;
						}
					}
					if (S.v.status & PSL_VEC_FILL2) {
						current_fill = S.v.fill;	/* Override -G<fill> with specified head fill */
						if (S.v.status & PSL_VEC_FILL) Ctrl->G.active = true;
					}
					else if (S.v.status & PSL_VEC_FILL) {
						current_fill = default_fill, Ctrl->G.active = true;	/* Return to default fill */
					}
					if (S.v.status & PSL_VEC_JUST_S) {	/* Got coordinates of tip instead of dir/length so need to undo dimension scaling */
						in[pos2x] *= GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];
						in[pos2y] *= GMT->session.u2u[GMT_INCH][GMT->current.setting.proj_length_unit];
					}
				}
				else if (S.symbol == PSL_WEDGE) {
					if (S.v.status == PSL_VEC_OUTLINE2) {	/* Wedge spider pen specified separately */
						PSL_defpen (PSL, "PSL_spiderpen", S.v.pen.width, S.v.pen.style, S.v.pen.offset, S.v.pen.rgb);
						last_spiderpen = S.v.pen;
					}
					else if (outline_active && !gmt_M_same_pen (current_pen, last_spiderpen)) {	/* Reset to new pen */
							PSL_defpen (PSL, "PSL_spiderpen", current_pen.width, current_pen.style, current_pen.offset, current_pen.rgb);
							last_spiderpen = current_pen;
					}
				}
				else if (S.symbol == PSL_DOT && !Ctrl->G.active)	/* Must switch on default black fill */
					current_fill = black;
			}

			if (get_rgb) {
				if (P->categorical & 2)
					gmt_get_fill_from_key (GMT, P, In->text, &current_fill);
				else
					gmt_get_fill_from_z (GMT, P, in[GMT_Z], &current_fill);
				if (PH->skip) continue;	/* Chosen CPT indicates skip for this z */
				if (Ctrl->I.active) {
					if (Ctrl->I.mode == 0)
						gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
					else
						gmt_illuminate (GMT, in[icol], current_fill.rgb);
				}
			}
			else if (Ctrl->I.mode == 1) {	/* Must reset current file and then apply illumination */
				current_fill = default_fill = (S.symbol == PSL_DOT && !Ctrl->G.active) ? black : Ctrl->G.fill;
				gmt_illuminate (GMT, in[icol], current_fill.rgb);
			}

			if (!Ctrl->N.active && S.symbol != GMT_SYMBOL_BARX && S.symbol != GMT_SYMBOL_BARY) {
				/* Skip points outside map */
				gmt_map_outside (GMT, in[GMT_X], in[GMT_Y]);
				may_intrude_inside = false;
				if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) {
					if (S.symbol == PSL_ELLIPSE || S.symbol == PSL_ROTRECT)
						may_intrude_inside = true;
					else
						continue;
				}
			}

			if (QR_symbol) {
				if (Ctrl->G.active)	/* Change color of QR code */
					PSL_command (PSL, "/QR_fill {%s} def\n", PSL_makecolor (PSL, current_fill.rgb));
				if (outline_active) {	/* Draw outline of QR code */
					PSL_command (PSL, "/QR_outline true def\n");
					PSL_command (PSL, "/QR_pen {%s} def\n",  PSL_makepen (PSL, (1.0/6.0) * Ctrl->W.pen.width, Ctrl->W.pen.rgb, Ctrl->W.pen.style, Ctrl->W.pen.offset));
				}
				else
					PSL_command (PSL, "/QR_outline false def\n");
			}
			if (S.symbol == GMT_SYMBOL_BARX && (S.base_set & 4))
				in[GMT_X] += S.base;
			else if (S.symbol == GMT_SYMBOL_BARY && (S.base_set & 4))
				in[GMT_Y] += S.base;

			if (gmt_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y)) continue;	/* NaNs on input */

			if (gmt_M_is_dnan (plot_x)) {	/* Transformation of x yielded a NaN (e.g. log (-ve)) */
				GMT_Report (API, GMT_MSG_INFORMATION, "Data point with x = NaN near line %d\n", n_total_read);
				continue;
			}
			if (gmt_M_is_dnan (plot_y)) {	/* Transformation of y yielded a NaN (e.g. log (-ve)) */
				GMT_Report (API, GMT_MSG_INFORMATION, "Data point with y = NaN near line %d\n", n_total_read);
				continue;
			}
			if (GMT->common.t.variable)	/* Update the transparency for current symbol (or -t was given) */
				PSL_settransparency (PSL, 0.01 * in[tcol]);

			if (Ctrl->E.active) {
				if (Ctrl->E.mode) gmt_M_rgb_copy (Ctrl->E.pen.rgb, current_fill.rgb);
				if (Ctrl->E.mode & 1) gmt_M_rgb_copy (current_fill.rgb, GMT->session.no_rgb);
				gmt_setpen (GMT, &Ctrl->E.pen);
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

			if (Ctrl->W.cpt_effect) {
				if (Ctrl->W.pen.cptmode & 1) {	/* Change pen color via CPT */
					gmt_M_rgb_copy (Ctrl->W.pen.rgb, current_fill.rgb);
					current_pen = Ctrl->W.pen;
					if (can_update_headpen && !gmt_M_same_pen (current_pen, last_headpen)) {	/* Since color may have changed */
						PSL_defpen (PSL, "PSL_vecheadpen", current_pen.width, current_pen.style, current_pen.offset, current_pen.rgb);
						last_headpen = current_pen;
					}
				}
				if ((Ctrl->W.pen.cptmode & 2) == 0 && !Ctrl->G.active)	/* Turn off CPT fill */
					gmt_M_rgb_copy (current_fill.rgb, GMT->session.no_rgb);
				else if (Ctrl->G.active)
					current_fill = Ctrl->G.fill;
			}

			if (geovector) {	/* Vectors do it separately */
				if (get_rgb) S.v.fill = current_fill;
			}
			else if (!may_intrude_inside) {
				gmt_setfill (GMT, &current_fill, outline_setting);
				gmt_setpen (GMT, &current_pen);
			}

			if ((S.symbol == PSL_ELLIPSE || S.symbol == PSL_ROTRECT)) {	/* Ellipses and rectangles */
				if (S.n_required == 0)	/* Degenerate ellipse or rectangle, got diameter via S.size_x */
					in[ex2] = in[ex3] = S.size_x;	/* Duplicate diameter as major and minor axes */
				else if (S.n_required == 1)	/* Degenerate ellipse or rectangle, expect single diameter via input */
					in[ex2] = in[ex3] = in[ex1];	/* Duplicate diameter as major and minor axes */
			}

			if (S.base_set & 2) {
				bcol = (S.read_size) ? ex2 : ex1;
				S.base = in[bcol];	/* Got base from input column */
			}
			if (S.read_size) {
				S.size_x = in[ex1] * S.factor;	/* Got size from input column; scale by factor if area unifier is on */
				if (delayed_unit_scaling) S.size_x *= GMT->session.u2u[S.u][GMT_INCH];
			}
			gmt_M_memset (dim, PSL_MAX_DIMS, double);
			dim[0] = S.size_x;

			/* For global periodic maps, symbols plotted close to a periodic boundary may be clipped and should appear
			 * at the other periodic boundary.  We try to handle this below */

			xpos[0] = plot_x;
			if (periodic) {
				width = 2.0 * gmt_half_map_width (GMT, plot_y);	/* Width of map at current latitude (not all projections have straight w/e boundaries */
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
							gmt_geo_to_xy (GMT, S.base, in[GMT_Y] - 0.5 * dim[0], &x_1, &y_1);
							gmt_geo_to_xy (GMT, in[GMT_X], in[GMT_Y] + 0.5 * dim[0], &x_2, &y_2);
						}
						else {
							gmt_geo_to_xy (GMT, S.base, GMT->common.R.wesn[YLO], &x_1, &y_1);	/* Zero x level for horizontal bars */
							x_2 = plot_x;
							y_1 = plot_y - 0.5 * dim[0]; y_2 = plot_y + 0.5 * dim[0];
						}
						PSL_plotbox (PSL, x_1, y_1, x_2, y_2);
						break;
					case GMT_SYMBOL_BARY:
						if (!Ctrl->N.active) in[GMT_Y] = MAX (GMT->common.R.wesn[YLO], MIN (in[GMT_Y], GMT->common.R.wesn[YHI]));
						if (S.user_unit[GMT_X]) {	/* Width measured in x units */
							gmt_geo_to_xy (GMT, in[GMT_X] - 0.5 * dim[0], S.base, &x_1, &y_1);
							gmt_geo_to_xy (GMT, in[GMT_X] + 0.5 * dim[0], in[GMT_Y], &x_2, &y_2);
						}
						else {
							gmt_geo_to_xy (GMT, GMT->common.R.wesn[XLO], S.base, &x_1, &y_1);	/* Zero y level for vertical bars */
							x_1 = plot_x - 0.5 * dim[0]; x_2 = plot_x + 0.5 * dim[0];
							y_2 = plot_y;
						}
						PSL_plotbox (PSL, x_1, y_1, x_2, y_2);
						break;
					case PSL_CROSS:
					case  PSL_PLUS:
					case PSL_DOT:
					case PSL_XDASH:
					case PSL_YDASH:
					case PSL_STAR:
					case PSL_CIRCLE:
					case PSL_SQUARE:
					case PSL_HEXAGON:
					case PSL_PENTAGON:
					case PSL_OCTAGON:
					case PSL_TRIANGLE:
					case PSL_INVTRIANGLE:
					case PSL_DIAMOND:
						PSL_plotsymbol (PSL, xpos[item], plot_y, dim, S.symbol);
						break;
					case PSL_RNDRECT:
						dim[2] = in[ex3];
						if (gmt_M_is_dnan (dim[2])) {
							GMT_Report (API, GMT_MSG_WARNING, "Rounded rectangle corner radius = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						/* Intentionally fall through - to pick up the other parameters */
					case PSL_RECT:
						if (S.diagonal) {	/* Special rectangle give by opposing corners on a diagonal */
							if (gmt_M_is_dnan (in[pos2x])) {
								GMT_Report (API, GMT_MSG_WARNING, "Diagonal longitude = NaN near line %d. Skipped\n", n_total_read);
								continue;
							}
							if (gmt_M_is_dnan (in[pos2y])) {
								GMT_Report (API, GMT_MSG_WARNING, "Diagonal latitude = NaN near line %d. Skipped\n", n_total_read);
								continue;
							}
							S_Diag->data[GMT_X][0] = S_Diag->data[GMT_X][3] = S_Diag->data[GMT_X][4] = in[GMT_X];
							S_Diag->data[GMT_X][1] = S_Diag->data[GMT_X][2] = in[pos2x];
							S_Diag->data[GMT_Y][0] = S_Diag->data[GMT_Y][1] = S_Diag->data[GMT_Y][4] = in[GMT_Y];
							S_Diag->data[GMT_Y][2] = S_Diag->data[GMT_Y][3] = in[pos2y];
							S_Diag->n_rows = 5;
							if (gmt_M_is_geographic (GMT, GMT_IN) && !Ctrl->A.active)	/* May need to resample */
								S_Diag->n_rows = gmt_fix_up_path (GMT, &S_Diag->data[GMT_X], &S_Diag->data[GMT_Y], S_Diag->n_rows, Ctrl->A.step, Ctrl->A.mode);
							/* Plot it */
							gmt_setfill (GMT, &current_fill, outline_setting);
							gmt_geo_polygons (GMT, S_Diag);
							break;
						}
						dim[0] = in[ex1];
						if (gmt_M_is_dnan (dim[0])) {
							GMT_Report (API, GMT_MSG_WARNING, "Rounded rectangle width = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						dim[1] = in[ex2];
						if (gmt_M_is_dnan (dim[1])) {
							GMT_Report (API, GMT_MSG_WARNING, "Rounded rectangle height = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						PSL_plotsymbol (PSL, plot_x, plot_y, dim, S.symbol);
						break;
					case PSL_ROTRECT:
					case PSL_ELLIPSE:
						if (gmt_M_is_dnan (in[ex1])) {
							GMT_Report (API, GMT_MSG_WARNING, "Ellipse/Rectangle angle = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						if (gmt_M_is_dnan (in[ex2])) {
							GMT_Report (API, GMT_MSG_WARNING, "Ellipse/Rectangle width or major axis = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						if (gmt_M_is_dnan (in[ex3])) {
							GMT_Report (API, GMT_MSG_WARNING, "Ellipse/Rectangle height or minor axis = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						if (!S.convert_angles) {	/* Got axes in current plot units, change to inches */
							dim[0] = in[ex1];
							gmt_flip_angle_d (GMT, &dim[0]);
							dim[1] = in[ex2];
							dim[2] = in[ex3];
							PSL_plotsymbol (PSL, xpos[item], plot_y, dim, S.symbol);
						}
						else if (gmt_M_is_cartesian (GMT, GMT_IN)) {	/* Got axes in user units, change to inches */
							dim[0] = 90.0 - in[ex1];	/* Cartesian azimuth */
							gmt_flip_angle_d (GMT, &dim[0]);
							dim[1] = in[ex2] * GMT->current.proj.scale[GMT_X];
							dim[2] = in[ex3] * GMT->current.proj.scale[GMT_Y];
							PSL_plotsymbol (PSL, xpos[item], plot_y, dim, S.symbol);
						}
						else if (S.symbol == PSL_ELLIPSE) {	/* Got axis in km */
							if (may_intrude_inside) {	/* Must plot fill and outline separately */
								gmt_setfill (GMT, &current_fill, 0);
								gmt_plot_geo_ellipse (GMT, in[GMT_X], in[GMT_Y], in[ex2], in[ex3], in[ex1]);
								gmt_setpen (GMT, &current_pen);
								PSL_setfill (PSL, GMT->session.no_rgb, outline_setting);
								gmt_plot_geo_ellipse (GMT, in[GMT_X], in[GMT_Y], in[ex2], in[ex3], in[ex1]);
							}
							else
								gmt_plot_geo_ellipse (GMT, in[GMT_X], in[GMT_Y], in[ex2], in[ex3], in[ex1]);
						}
						else {
							if (may_intrude_inside) {	/* Must plot fill and outline separately */
								gmt_setfill (GMT, &current_fill, 0);
								gmt_geo_rectangle (GMT, in[GMT_X], in[GMT_Y], in[ex2], in[ex3], in[ex1]);
								gmt_setpen (GMT, &current_pen);
								PSL_setfill (PSL, GMT->session.no_rgb, outline_setting);
								gmt_geo_rectangle (GMT, in[GMT_X], in[GMT_Y], in[ex2], in[ex3], in[ex1]);
							}
							else
								gmt_geo_rectangle (GMT, in[GMT_X], in[GMT_Y], in[ex2], in[ex3], in[ex1]);
						}
						break;
					case GMT_SYMBOL_TEXT:
						if (Ctrl->G.active && !outline_active)
							PSL_setcolor (PSL, current_fill.rgb, PSL_IS_FILL);
						else if (!Ctrl->G.active)
							PSL_setfill (PSL, GMT->session.no_rgb, outline_setting);
						(void) gmt_setfont (GMT, &S.font);
						PSL_plottext (PSL, xpos[item], plot_y, dim[0] * PSL_POINTS_PER_INCH, S.string, 0.0, S.justify, outline_setting);
						break;
					case PSL_VECTOR:
						gmt_init_vector_param (GMT, &S, false, false, NULL, false, NULL);	/* Update vector head parameters */
						if (S.v.status & PSL_VEC_COMPONENTS)	/* Read dx, dy in user units */
							length = hypot (in[ex1+S.read_size], in[ex2+S.read_size]) * S.v.comp_scale;
						else
							length = in[ex2+S.read_size];
						if (gmt_M_is_dnan (length)) {
							GMT_Report (API, GMT_MSG_WARNING, "Vector length = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						if (S.v.status & PSL_VEC_COMPONENTS)	/* Read dx, dy in user units */
							d = d_atan2d (in[ex2+S.read_size], in[ex1+S.read_size]);
						else
							d = in[ex1+S.read_size];

						if (!S.convert_angles)	/* Use direction as given */
							direction = d;
						else if (gmt_M_is_cartesian (GMT, GMT_IN))	/* Cartesian angle; change to azimuth */
							direction = 90.0 - d;
						else	/* Convert geo azimuth to map direction */
							direction = gmt_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, d);

						if (gmt_M_is_dnan (direction)) {
							GMT_Report (API, GMT_MSG_WARNING, "Vector direction = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						if (S.v.status & PSL_VEC_JUST_S) {	/* Got coordinates of tip instead of dir/length */
							gmt_geo_to_xy (GMT, in[pos2x], in[pos2y], &x_2, &y_2);
							if (gmt_M_is_dnan (x_2) || gmt_M_is_dnan (y_2)) {
								GMT_Report (API, GMT_MSG_WARNING, "Vector head coordinates contain NaNs near line %d. Skipped\n", n_total_read);
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
							gmt_flip_angle_d (GMT, &direction);
							sincosd (direction, &s, &c);
							x_2 = xpos[item] + length * c;
							y_2 = plot_y + length * s;
							justify = PSL_vec_justify (S.v.status);	/* Return justification as 0-2 */
							if (justify) {	/* Meant to center the vector at center (1) or tip (2) */
								dx = justify * 0.5 * (x_2 - xpos[item]);	dy = justify * 0.5 * (y_2 - plot_y);
								xpos[item] -= dx;		plot_y -= dy;
								x_2 -= dx;		y_2 -= dy;
							}
						}
						if (S.v.parsed_v4) {	/* Got v_width directly from V4 syntax so no messing with it here if under compatibility */
							/* Now plot the old GMT V4 vector instead */
							/* But have to improvise as far as outline|fill goes... */
							if (outline_active) S.v.status |= PSL_VEC_OUTLINE;	/* Choosing to draw head outline */
							if (fill_active) S.v.status |= PSL_VEC_FILL;		/* Choosing to fill head */
							if (!(S.v.status & PSL_VEC_OUTLINE) && !(S.v.status & PSL_VEC_FILL)) S.v.status |= PSL_VEC_OUTLINE;	/* Gotta do something */
						}
						else
							S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
						if (length < S.v.h_length && S.v.v_norm < 0.0)	/* No shrink requested but head length exceeds total vector length */
							GMT_Report (API, GMT_MSG_INFORMATION, "Vector head length exceeds overall vector length near line %d. Consider using +n<norm>\n", n_total_read);
						s = (length < S.v.v_norm) ? length / S.v.v_norm : 1.0;
						dim[0] = x_2, dim[1] = y_2;
						dim[2] = s * S.v.v_width, dim[3] = s * S.v.h_length, dim[4] = s * S.v.h_width;
						if (S.v.parsed_v4) {	/* Parsed the old ways so plot the old ways... */
							double *v4_rgb = NULL;
							int v4_outline = Ctrl->W.active;
							if (Ctrl->G.active)
								v4_rgb = Ctrl->G.fill.rgb;
							else if (Ctrl->C.active)
								v4_rgb = current_fill.rgb;
							else
								v4_rgb = GMT->session.no_rgb;
							if (v4_outline) gmt_setpen (GMT, &Ctrl->W.pen);
							if (S.v.status & PSL_VEC_BEGIN) v4_outline += 8;	/* Double-headed */
							dim[5] = GMT->current.setting.map_vector_shape;
							dim[4] *= 0.5;	/* Since it was double in the parsing */
							psl_vector_v4 (PSL, xpos[item], plot_y, dim, v4_rgb, v4_outline);
						}
						else {
							dim[5] = S.v.v_shape;
							dim[6] = (double)S.v.status;
							dim[7] = (double)S.v.v_kind[0];	dim[8] = (double)S.v.v_kind[1];
							dim[9] = (double)S.v.v_trim[0];	dim[10] = (double)S.v.v_trim[1];
							dim[11] = s * headpen_width;	/* Possibly shrunk head pen width */
							PSL_plotsymbol (PSL, xpos[item], plot_y, dim, PSL_VECTOR);
						}
						break;
					case GMT_SYMBOL_GEOVECTOR:
						gmt_init_vector_param (GMT, &S, true, Ctrl->W.active, &Ctrl->W.pen, Ctrl->G.active, &Ctrl->G.fill);	/* Update vector head parameters */
						if (S.v.status & PSL_VEC_OUTLINE2)
							S.v.v_width = (float)(S.v.pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
						else
							S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
						if (gmt_M_is_dnan (in[ex1+S.read_size])) {
							GMT_Report (API, GMT_MSG_WARNING, "Geovector azimuth = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						if (gmt_M_is_dnan (in[ex2+S.read_size])) {
							GMT_Report (API, GMT_MSG_WARNING, "Geovector length = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						warn = gmt_geo_vector (GMT, in[GMT_X], in[GMT_Y], in[ex1+S.read_size], in[ex2+S.read_size], &current_pen, &S);
						n_warn[warn]++;
						break;
					case PSL_MARC:
						gmt_init_vector_param (GMT, &S, false, false, NULL, false, NULL);	/* Update vector head parameters */
						S.v.v_width = (float)(current_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH]);
						dim[0] = in[ex1+S.read_size];
						dim[1] = in[ex2+S.read_size];
						dim[2] = in[ex3+S.read_size];
						length = fabs (dim[2]-dim[1]);	/* Arc length in degrees */
						if (gmt_M_is_dnan (length)) {
							GMT_Report (API, GMT_MSG_WARNING, "Math angle arc length = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						s = (length < S.v.v_norm) ? length / S.v.v_norm : 1.0;
						dim[3] = s * S.v.h_length, dim[4] = s * S.v.h_width, dim[5] = s * S.v.v_width;
						dim[6] = S.v.v_shape;
						dim[7] = (double)S.v.status;
						dim[8] = (double)S.v.v_kind[0];	dim[9] = (double)S.v.v_kind[1];
						dim[10] = (double)S.v.v_trim[0];	dim[11] = (double)S.v.v_trim[1];
						dim[12] = s * headpen_width;	/* Possibly shrunk head pen width */
						PSL_plotsymbol (PSL, xpos[item], plot_y, dim, S.symbol);
						break;
					case PSL_WEDGE:
						if (gmt_M_is_dnan (in[ex1+S.read_size])) {
							GMT_Report (API, GMT_MSG_WARNING, "Wedge start angle = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						if (gmt_M_is_dnan (in[ex2+S.read_size])) {
							GMT_Report (API, GMT_MSG_WARNING, "Wedge stop angle = NaN near line %d. Skipped\n", n_total_read);
							continue;
						}
						if (!S.convert_angles) {
							dim[1] = in[ex1+S.read_size];
							dim[2] = in[ex2+S.read_size];
						}
						else if (gmt_M_is_cartesian (GMT, GMT_IN)) {
							/* Note that the direction of the arc gets swapped when converting from azimuth */
							dim[2] = 90.0 - in[ex1+S.read_size];
							dim[1] = 90.0 - in[ex2+S.read_size];
						}
						else {
							dim[2] = gmt_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, in[ex1+S.read_size]);
							dim[1] = gmt_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, in[ex2+S.read_size]);
						}
						if (S.w_active)	/* Geo-wedge */
							gmt_geo_wedge (GMT, in[GMT_X], in[GMT_Y], S.w_radius_i, S.w_radius, S.w_dr, dim[1], dim[2], S.w_da, S.w_type, fill_active || get_rgb, outline_active);
						else {	/* Cartesian wedge */
							dim[0] *= 0.5;	/* Change from diameter to radius */
							dim[3] = S.w_type;
							dim[4] = 0.5 * S.w_radius_i;	/* In case there is an inner diameter */
							dim[5] = S.w_dr;	/* In case there is a request for radially spaced arcs */
							dim[6] = S.w_da;	/* In case there is a request for angularly spaced radial lines */
							dim[7] = 0.0;	/* Reset */
							if (fill_active || get_rgb) dim[7] = 1;	/* Lay down filled wedge */
							if (outline_active) dim[7] += 2;	/* Draw wedge outline */
							PSL_plotsymbol (PSL, xpos[item], plot_y, dim, S.symbol);
						}
						break;
					case GMT_SYMBOL_CUSTOM:
#if 0
						for (j = 0; S.custom->type && j < S.n_required; j++) {	/* Convert any azimuths to plot angles first */
							if (S.custom->type[j] == GMT_IS_AZIMUTH) {	/* Make sure plot angles are 0-360 for macro conditionals */
								dim[j+1] = gmt_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, in[ex1+S.read_size+j]);
								if (dim[j+1] < 0.0) dim[j+1] += 360.0;
							}
							else {	/* Angles (enforce 0-360), dimensions or other quantities */
								dim[j+1] = in[ex1+S.read_size+j];
								if (S.custom->type[j] == GMT_IS_ANGLE && dim[j+1] < 0.0) dim[j+1] += 360.0;
							}
						}
#endif
						for (j = 0; S.custom->type && j < S.n_required; j++) {
							/* Angles (enforce 0-360), dimensions or other quantities */
							dim[j+1] = in[ex1+S.read_size+j];
							if (S.custom->type[j] == GMT_IS_ANGLE && dim[j+1] < 0.0) dim[j+1] += 360.0;
						}
						if (!S.custom->start) S.custom->start = (get_rgb) ? 3 : 2;
						gmt_draw_custom_symbol (GMT, xpos[item], plot_y, dim, In->text, S.custom, &current_pen, &current_fill, outline_setting);
						break;
				}
			}
			if (S.read_symbol_cmd && (S.symbol == PSL_VECTOR || S.symbol == GMT_SYMBOL_GEOVECTOR || S.symbol == PSL_MARC)) {	/* Reset status */
				current_pen = save_pen; current_fill = save_fill; Ctrl->W.active = save_W; Ctrl->G.active = save_G;
			}
		} while (true);
		if (GMT->common.t.variable)	/* Reset the transparency */
			PSL_settransparency (PSL, 0.0);
		PSL_command (GMT->PSL, "U\n");
		if (n_warn[1]) GMT_Report (API, GMT_MSG_INFORMATION, "%d vector heads had length exceeding the vector length and were skipped. Consider the +n<norm> modifier to -S\n", n_warn[1]);
		if (n_warn[2]) GMT_Report (API, GMT_MSG_INFORMATION, "%d vector heads had to be scaled more than implied by +n<norm> since they were still too long. Consider changing the +n<norm> modifier to -S\n", n_warn[2]);

		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &Diag) != GMT_NOERROR) {	/* Be gone with the diagonal */
			Return (API->error);
		}
	}
	else {	/* Line/polygon part */
		uint64_t seg, seg_out = 0, n_new, n_cols = 2;
		bool duplicate, resampled;
		struct GMT_DATASET *D = NULL;	/* Pointer to GMT multisegment table(s) */
		struct GMT_DATASET_HIDDEN *DH = NULL;
		struct GMT_DATASEGMENT_HIDDEN *SH = NULL;

		gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);

		if (Ctrl->L.anchor == PSXY_POL_SYMM_DEV) n_cols = 3;
		else if (Ctrl->L.anchor == PSXY_POL_ASYMM_DEV || Ctrl->L.anchor == PSXY_POL_ASYMM_ENV) n_cols = 4;

		if (GMT_Init_IO (API, GMT_IS_DATASET, geometry, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
			Return (API->error);
		}
		if ((error = GMT_Set_Columns (API, GMT_IN, (unsigned int)n_cols, GMT_COL_FIX_NO_TEXT)) != GMT_NOERROR) {
			/* We don't want trailing text because we may need to resample lines below */
			Return (API->error);
		}
		if ((S.symbol == GMT_SYMBOL_QUOTED_LINE || S.symbol == GMT_SYMBOL_DECORATED_LINE) && S.G.segmentize) {	/* Special quoted/decorated line where each point-pair should be considered a line segment */
			struct GMT_SEGMENTIZE S;
			struct GMT_DATASET *Dtmp = NULL;	/* Pointer to GMT multisegment table(s) */
			if ((Dtmp = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
				Return (API->error);
			}
			if (Dtmp->n_records) {
				S.method = SEGM_REFPOINT;	S.level = SEGM_RECORD;
				D = gmt_segmentize_data (GMT, Dtmp, &S);	/* Segmentize the original data */
				if (GMT_Destroy_Data (API, &Dtmp) != GMT_NOERROR) {	/* Be gone with the original */
					Return (API->error);
				}
			}
			else
				D = Dtmp;	/* To avoid issues in the loop over no records */
		}
		else if (Ctrl->F.active) {
			struct GMT_DATASET *Dtmp = NULL;	/* Pointer to GMT multisegment table(s) */
			if ((Dtmp = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
				Return (API->error);
			}
			if (Dtmp->n_records) {
				D = gmt_segmentize_data (GMT, Dtmp, &(Ctrl->F.S));	/* Segmentize the data */
				if (GMT_Destroy_Data (API, &Dtmp) != GMT_NOERROR) {	/* Be gone with the original */
					Return (API->error);
				}
			}
			else
				D = Dtmp;	/* To avoid issues in the loop over no records */
		}
		else if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
			Return (API->error);
		}
		if (D->n_records && D->n_columns < 2) {
			GMT_Report (API, GMT_MSG_ERROR, "Input data have %d column(s) but at least 2 are needed\n", (int)D->n_columns);
			Return (GMT_DIM_TOO_SMALL);
		}

		if (Zin) {	/* Check that the Z file matches our polygon file */
			if (Zin->n_records < D->n_segments) {
				GMT_Report (API, GMT_MSG_ERROR, "Number of Z values is less then number of polygons\n");
				Return (API->error);
			}
			if (Zin->n_segments > 1) {
				GMT_Report (API, GMT_MSG_ERROR, "The file given via -Z must have a single segment with one z-value for each polygon in the input file\n");
				Return (API->error);
			}
			if (Zin->n_columns == 0) {
				GMT_Report (API, GMT_MSG_ERROR, "The file given via -Z must have a at least one data column and we will choose the last column\n");
				Return (API->error);
			}
			z_for_cpt = Zin->table[0]->segment[0]->data[Zin->n_columns-1];	/* Short hand to the required z-array */
		}

		DH = gmt_get_DD_hidden (D);
		if (S.symbol == GMT_SYMBOL_DECORATED_LINE) {	/* Get a dataset with one table, segment, 0 rows, but 4 columns */
			uint64_t dim[GMT_DIM_SIZE] = {1, 0, 0, 4};	/* Put everything in one table */
			dim[GMT_SEG] = D->n_segments;	/* Make one segment for each input segment so segment headers can be preserved */
			if ((Decorate = GMT_Create_Data (GMT->parent, GMT_IS_DATASET, GMT_IS_POINT, GMT_WITH_STRINGS, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);
		}
		if (GMT->current.io.OGR && (GMT->current.io.OGR->geometry == GMT_IS_POLYGON || GMT->current.io.OGR->geometry == GMT_IS_MULTIPOLYGON)) polygon = true;

		if (GMT->common.l.active && S.symbol == GMT_SYMBOL_LINE) {
			if (polygon) {	/* Place a rectangle in the legend */
				int symbol = S.symbol;
				S.symbol = PSL_RECT;
				gmt_add_legend_item (API, &S, Ctrl->G.active, &(Ctrl->G.fill), Ctrl->W.active, &(Ctrl->W.pen), &(GMT->common.l.item));
				S.symbol = symbol;
			}
			else	/* For specified line, width, color we can do an auto-legend entry under modern mode */
				gmt_add_legend_item (API, &S, false, NULL, Ctrl->W.active, &(Ctrl->W.pen), &(GMT->common.l.item));
		}

		if (Ctrl->W.cpt_effect && Ctrl->W.pen.cptmode & 2) polygon = true;
		for (tbl = 0; tbl < D->n_tables; tbl++) {
			if (D->table[tbl]->n_headers && S.G.label_type == GMT_LABEL_IS_HEADER)	/* Get potential label from first header */
				gmt_extract_label (GMT, D->table[tbl]->header[0], S.G.label, NULL);

			for (seg = 0; seg < D->table[tbl]->n_segments; seg++, seg_out++) {	/* For each segment in the table */

				L = D->table[tbl]->segment[seg];	/* Set shortcut to current segment */
				SH = gmt_get_DS_hidden (L);

				if (polygon && gmt_polygon_is_hole (GMT, L)) continue;	/* Holes are handled together with perimeters */
				resampled = false;
				if (!polygon && gmt_trim_requested (GMT, &current_pen)) {	/* Needs a haircut */
					if (L->n_rows == 2) {	/* Given endpoints we need to resample in order to trim */
						/* The whole trimming stuff requires at least 2 points per line so we resample */
						if (gmt_M_is_geographic (GMT, GMT_IN))
							n_new = gmt_fix_up_path (GMT, &L->data[GMT_X], &L->data[GMT_Y], L->n_rows, Ctrl->A.step, Ctrl->A.mode);
						else
							n_new = gmt_resample_path (GMT, &L->data[GMT_X], &L->data[GMT_Y], L->n_rows, 0.5 * hypot (L->data[GMT_X][1]-L->data[GMT_X][0], L->data[GMT_Y][1]-L->data[GMT_Y][0]), GMT_TRACK_FILL);
						if (n_new == 0) {
							Return (GMT_RUNTIME_ERROR);
						}
						L->n_rows = SH->n_alloc = n_new;
						gmt_set_seg_minmax (GMT, D->geometry, 2, L);	/* Update min/max of x/y only */
						resampled = true;	/* To avoid doing it twice */
					}
					if (gmt_trim_line (GMT, &L->data[GMT_X], &L->data[GMT_Y], &L->n_rows, &current_pen)) continue;	/* Trimmed away completely */
				}

				GMT_Report (API, GMT_MSG_INFORMATION, "Plotting table %" PRIu64 " segment %" PRIu64 "\n", tbl, seg);

				/* We had here things like:	x = D->table[tbl]->segment[seg]->data[GMT_X];
				 * but reallocating x below lead to disasters.  */

				outline_setting = outline_active ? 1 : 0;
				if (Zin != NULL) {
					double rgb[4];
					(void)gmt_get_rgb_from_z (GMT, P, z_for_cpt[seg], rgb);
					if (Ctrl->Z.mode & 1) {	/* To be used in polygon or symbol outline */
						gmt_M_rgb_copy (current_pen.rgb, rgb);
						gmt_setpen (GMT, &current_pen);
					}
					if (Ctrl->Z.mode & 2) {	/* To be used in polygon or symbol fill */
						gmt_M_rgb_copy (current_fill.rgb, rgb);
						gmt_setfill (GMT, &current_fill, outline_setting);
					}
				}
				else {
					change = gmt_parse_segment_header (GMT, L->header, P, &fill_active, &current_fill, &default_fill, &outline_active, &current_pen, &default_pen, default_outline, SH->ogr);
					outline_setting = outline_active ? 1 : 0;
				}
				if (P && PH->skip) continue;	/* Chosen CPT indicates skip for this z */

				duplicate = (DH->alloc_mode == GMT_ALLOC_EXTERNALLY && ((polygon && gmt_polygon_is_open (GMT, L->data[GMT_X], L->data[GMT_Y], L->n_rows)) || GMT->current.map.path_mode == GMT_RESAMPLE_PATH));
				if (duplicate)	/* Must duplicate externally allocated segment since it needs to be resampled below */
					L = gmt_duplicate_segment (GMT, D->table[tbl]->segment[seg]);

				if (L->header && L->header[0]) {
					PSL_comment (PSL, "Segment header: %s\n", L->header);
					if (gmt_parse_segment_item (GMT, L->header, "-S", s_args)) {	/* Found -S, which only can apply to front, quoted or decorated lines */
						if ((S.symbol == GMT_SYMBOL_QUOTED_LINE && s_args[0] == 'q') || (S.symbol == GMT_SYMBOL_DECORATED_LINE && s_args[0] == '~') || (S.symbol == GMT_SYMBOL_FRONT && s_args[0] == 'f')) { /* Update parameters */
							gmt_contlabel_plot (GMT, &S.G);
							gmt_contlabel_free (GMT, &S.G);
							gmt_parse_symbol_option (GMT, s_args, &S, 0, false);
							if (change & 1) change -= 1;	/* Don't want polygon to be true later */
						}
						else if (S.symbol == GMT_SYMBOL_QUOTED_LINE || S.symbol == GMT_SYMBOL_DECORATED_LINE || S.symbol == GMT_SYMBOL_FRONT)
							GMT_Report (API, GMT_MSG_ERROR, "Segment header tries to switch from -S%c to another symbol (%s) - ignored\n", S.symbol, s_args);
						else	/* Probably just junk -S in header */
							GMT_Report (API, GMT_MSG_INFORMATION, "Segment header contained -S%s - ignored\n", s_args);
					}
				}
				if (S.symbol == GMT_SYMBOL_DECORATED_LINE) {
					s_args[0] = '\0';	/* Recycle this string for this purpose */
					strcat (s_args, " -G");
					if (S.D.fill[0]) strcat (s_args, S.D.fill);	/* Set specific fill */
					strcat (s_args, " -W");
					if (S.D.pen[0])  strcat (s_args, S.D.pen);	/* Set specific outline */
					Decorate->table[0]->segment[seg_out]->header = strdup (s_args);
					if (change & 1) change -= 1;	/* Don't want polygon to be true if -G was found */
				}

				if (S.fq_parse) { /* Did not supply -Sf or -Sq in the segment header */
					if (S.symbol == GMT_SYMBOL_QUOTED_LINE) /* Did not supply -Sf in the segment header */
						GMT_Report (API, GMT_MSG_ERROR, "Segment header did not supply enough parameters for -Sf; skipping this segment\n");
					else if (S.symbol == GMT_SYMBOL_DECORATED_LINE) /* Did not supply -S_ in the segment header */
						GMT_Report (API, GMT_MSG_ERROR, "Segment header did not supply enough parameters for -S~; skipping this segment\n");
					else
						GMT_Report (API, GMT_MSG_ERROR, "Segment header did not supply enough parameters for -Sq; skipping this segment\n");
					continue;
				}
				if (Ctrl->I.active) {
					gmt_illuminate (GMT, Ctrl->I.value, current_fill.rgb);
					gmt_illuminate (GMT, Ctrl->I.value, default_fill.rgb);
				}
				if (Ctrl->W.cpt_effect) {
					if (Ctrl->W.pen.cptmode & 1) {	/* Change pen color via CPT */
						gmt_M_rgb_copy (Ctrl->W.pen.rgb, current_fill.rgb);
						current_pen = Ctrl->W.pen;
						gmt_setpen (GMT, &current_pen);
					}
					if ((Ctrl->W.pen.cptmode & 2) == 0 && !Ctrl->G.active)	/* Turn off CPT fill */
						gmt_M_rgb_copy (current_fill.rgb, GMT->session.no_rgb);
					else if (Ctrl->G.active)
						current_fill = Ctrl->G.fill;
				}
				else if (Zin == NULL) {
					if (change & 1) polygon = true;
					if (change & 2 && !Ctrl->L.polygon) {
						polygon = false;
						PSL_setcolor (PSL, current_fill.rgb, PSL_IS_STROKE);
					}
					if (change & 4) gmt_setpen (GMT, &current_pen);
				}
				if (S.G.label_type == GMT_LABEL_IS_HEADER) {	/* Get potential label from segment header */
					SH = gmt_get_DS_hidden (L);
					gmt_extract_label (GMT, L->header, S.G.label, SH->ogr);
				}

				if (polygon && gmt_polygon_is_open (GMT, L->data[GMT_X], L->data[GMT_Y], L->n_rows)) {
					/* Explicitly close polygon so that arc will work */
					size_t n_alloc;
					L->n_rows++;
					n_alloc = L->n_rows;
					gmt_M_malloc2 (GMT, L->data[GMT_X], L->data[GMT_Y], 0, &n_alloc, double);
					L->data[GMT_X][L->n_rows-1] = L->data[GMT_X][0];
					L->data[GMT_Y][L->n_rows-1] = L->data[GMT_Y][0];
				}

				if (GMT->current.map.path_mode == GMT_RESAMPLE_PATH && !resampled) {	/* Resample if spacing is too coarse */
					if ((n_new = gmt_fix_up_path (GMT, &L->data[GMT_X], &L->data[GMT_Y], L->n_rows, Ctrl->A.step, Ctrl->A.mode)) == 0) {
						Return (GMT_RUNTIME_ERROR);
					}
					L->n_rows = n_new;
					gmt_set_seg_minmax (GMT, D->geometry, 2, L);	/* Update min/max of x/y only */
				}

				if (polygon) {	/* Want a closed polygon (with or without fill and with or without outline) */
					gmt_setfill (GMT, &current_fill, outline_setting);
					gmt_geo_polygons (GMT, L);
				}
				else if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {	/* Labeled lines are dealt with by the contour machinery */
					bool closed, split = false;
					uint64_t k0;
					if ((GMT->current.plot.n = gmt_geo_to_xy_line (GMT, L->data[GMT_X], L->data[GMT_Y], L->n_rows)) == 0) continue;
					S.G.line_pen = current_pen;
					/* gmt_geo_to_xy_line may have chopped the line into multiple pieces if exiting and reentering the domain */
					for (k0 = 0; !split && k0 < GMT->current.plot.n; k0++) if (GMT->current.plot.pen[k0] & PSL_MOVE) split = true;
					if (split) {	/* Must write out separate sections via gmt_hold_contour */
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
							/* Get temp array of length n_section since gmt_hold_contour may change length */
							n_alloc = 0;
							gmt_M_malloc2 (GMT, xxx, yyy, n_section, &n_alloc, double);
							gmt_M_memcpy (xxx, &GMT->current.plot.x[k0], n_section, double);
							gmt_M_memcpy (yyy, &GMT->current.plot.y[k0], n_section, double);
							gmt_hold_contour (GMT, &xxx, &yyy, n_section, 0.0, "N/A", 'A', S.G.label_angle, false, false, &S.G);
							k0 = k1;	/* Goto start of next section */
							gmt_M_free (GMT, xxx);	gmt_M_free (GMT, yyy);
						}
					}
					else {	/* Just one line, which may even be closed */
						closed = !(gmt_polygon_is_open (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n));
						gmt_hold_contour (GMT, &GMT->current.plot.x, &GMT->current.plot.y, GMT->current.plot.n, 0.0, "N/A", 'A', S.G.label_angle, closed, false, &S.G);
						GMT->current.plot.n_alloc = GMT->current.plot.n;	/* Since gmt_hold_contour reallocates to fit the array */
					}
				}
				else if (S.symbol == GMT_SYMBOL_DECORATED_LINE) {	/* Decorated lines are dealt with by the contour machinery */
					bool split = false;
					uint64_t k0;
					if ((GMT->current.plot.n = gmt_geo_to_xy_line (GMT, L->data[GMT_X], L->data[GMT_Y], L->n_rows)) == 0) continue;
					gmt_plot_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n, PSL_LINEAR);
					/* gmt_geo_to_xy_line may have chopped the line into multiple pieces if exiting and reentering the domain */
					for (k0 = 1; !split && k0 < GMT->current.plot.n; k0++) if (GMT->current.plot.pen[k0] & PSL_MOVE) split = true;
					if (split) {	/* Must write out separate sections via gmt_hold_contour */
						uint64_t k1, n_section;
						size_t n_alloc;
						double *xxx = NULL, *yyy = NULL;
						k0 = 0;	/* Start of first section */
						while (k0 < GMT->current.plot.n) {	/* While more sections... */
							k1 = k0 + 1;	/* First point after anchor point */
							while (k1 < GMT->current.plot.n && GMT->current.plot.pen[k1] == PSL_DRAW) k1++;	/* Find next section anchor */
							/* k1 is now pointing to next move (anchor) point or it is GMT->current.plot.n */
							n_section = k1 - k0;	/* Number of points in this section */
							GMT_Report (API, GMT_MSG_DEBUG, "Decorated Sub-line starts at point %d and have length %d\n", (int)k0, (int)n_section);
							/* Make a copy of this section's coordinates */
							/* Get temp array of length n_section since gmt_decorated_line may change length */
							n_alloc = 0;
							gmt_M_malloc2 (GMT, xxx, yyy, n_section, &n_alloc, double);
							gmt_M_memcpy (xxx, &GMT->current.plot.x[k0], n_section, double);
							gmt_M_memcpy (yyy, &GMT->current.plot.y[k0], n_section, double);
							gmt_decorated_line (GMT, &xxx, &yyy, n_section, &S.D, Decorate, seg_out);
							k0 = k1;	/* Goto start of next section */
							gmt_M_free (GMT, xxx);	gmt_M_free (GMT, yyy);
						}
					}
					else	/* Just one line, which may even be closed */
						gmt_decorated_line (GMT, &GMT->current.plot.x, &GMT->current.plot.y, GMT->current.plot.n, &S.D, Decorate, seg_out);
				}
				else {	/* Plot line */
					uint64_t end;
					bool draw_line = true;
					if (Ctrl->L.anchor) {	/* Build a polygon in one of several ways */
						if (Ctrl->L.anchor == PSXY_POL_SYMM_DEV || Ctrl->L.anchor == PSXY_POL_ASYMM_DEV) {	/* Build envelope around y(x) from delta y values in 1 or 2 extra columns */
							uint64_t k, n, col = (Ctrl->L.anchor == PSXY_POL_ASYMM_DEV) ? 3 : 2;
							end = 2 * L->n_rows + 1;
							gmt_prep_tmp_arrays (GMT, GMT_IN, end, 2);	/* Init or reallocate tmp vectors */
							/* First go in positive x direction and build part of envelope */
							gmt_M_memcpy (GMT->hidden.mem_coord[GMT_X], L->data[GMT_X], L->n_rows, double);
							for (k = 0; k < L->n_rows; k++)
								GMT->hidden.mem_coord[GMT_Y][k] = L->data[GMT_Y][k] - fabs (L->data[2][k]);
							/* Then go in negative x direction and build rest of envelope */
							for (k = n = L->n_rows; k > 0; k--, n++) {
								GMT->hidden.mem_coord[GMT_X][n] = L->data[GMT_X][k-1];
								GMT->hidden.mem_coord[GMT_Y][n] = L->data[GMT_Y][k-1] + fabs (L->data[col][k-1]);
							}
							/* Explicitly close polygon */
							GMT->hidden.mem_coord[GMT_X][end-1] = GMT->hidden.mem_coord[GMT_X][0];
							GMT->hidden.mem_coord[GMT_Y][end-1] = GMT->hidden.mem_coord[GMT_Y][0];
						}
						else if (Ctrl->L.anchor == PSXY_POL_ASYMM_ENV) {	/* Build envelope around y(x) from low and high 2 extra columns */
							uint64_t k, n;
							end = 2 * L->n_rows + 1;
							gmt_prep_tmp_arrays (GMT, GMT_IN, end, 2);	/* Init or reallocate tmp vectors */
							/* First go in positive x direction and build part of envelope */
							gmt_M_memcpy (GMT->hidden.mem_coord[GMT_X], L->data[GMT_X], L->n_rows, double);
							for (k = 0; k < L->n_rows; k++)
								GMT->hidden.mem_coord[GMT_Y][k] = L->data[2][k];
							/* Then go in negative x direction and build rest of envelope */
							for (k = n = L->n_rows; k > 0; k--, n++) {
								GMT->hidden.mem_coord[GMT_X][n] = L->data[GMT_X][k-1];
								GMT->hidden.mem_coord[GMT_Y][n] = L->data[3][k-1];
							}
							/* Explicitly close polygon */
							GMT->hidden.mem_coord[GMT_X][end-1] = GMT->hidden.mem_coord[GMT_X][0];
							GMT->hidden.mem_coord[GMT_Y][end-1] = GMT->hidden.mem_coord[GMT_Y][0];
						}
						else {	/* First complete polygon via anchor points and paint the area, optionally with outline */
							uint64_t off = 0U;
							double value;
							end = L->n_rows;
							gmt_prep_tmp_arrays (GMT, GMT_IN, end+3, 2);	/* Init or reallocate tmp vectors */
							/* First copy the given line segment */
							gmt_M_memcpy (GMT->hidden.mem_coord[GMT_X], L->data[GMT_X], end, double);
							gmt_M_memcpy (GMT->hidden.mem_coord[GMT_Y], L->data[GMT_Y], end, double);
							/* Now add 2 anchor points and explicitly close by repeating 1st point */
							switch (Ctrl->L.mode) {
								case XHI:	off = 1;	/* Intentionally fall through - to select the x max entry */
								case XLO:
								case ZLO:
									value = (Ctrl->L.mode == ZLO) ? Ctrl->L.value : GMT->common.R.wesn[XLO+off];
									GMT->hidden.mem_coord[GMT_X][end] = GMT->hidden.mem_coord[GMT_X][end+1] = value;
									GMT->hidden.mem_coord[GMT_Y][end] = L->data[GMT_Y][end-1];
									GMT->hidden.mem_coord[GMT_Y][end+1] = L->data[GMT_Y][0];
									break;
								case YHI:	off = 1;	/* Intentionally fall through - to select the y max entry */
								case YLO:
								case ZHI:
									value = (Ctrl->L.mode == ZHI) ? Ctrl->L.value : GMT->common.R.wesn[YLO+off];
									GMT->hidden.mem_coord[GMT_Y][end] = GMT->hidden.mem_coord[GMT_Y][end+1] = value;
									GMT->hidden.mem_coord[GMT_X][end] = L->data[GMT_X][end-1];
									GMT->hidden.mem_coord[GMT_X][end+1] = L->data[GMT_X][0];
									break;
							}
							/* Explicitly close polygon */
							GMT->hidden.mem_coord[GMT_X][end+2] = L->data[GMT_X][0];
							GMT->hidden.mem_coord[GMT_Y][end+2] = L->data[GMT_Y][0];
							end += 3;
						}
						/* Project and get ready */
						if ((GMT->current.plot.n = gmt_geo_to_xy_line (GMT, GMT->hidden.mem_coord[GMT_X], GMT->hidden.mem_coord[GMT_Y], end)) == 0) continue;
						if (Ctrl->L.outline) gmt_setpen (GMT, &Ctrl->L.pen);	/* Select separate pen for polygon outline */
						if (Ctrl->G.active)	/* Specify the fill, possibly set outline */
							gmt_setfill (GMT, &current_fill, Ctrl->L.outline);
						else	/* No fill, just outline */
							gmt_setfill (GMT, NULL, Ctrl->L.outline);
						PSL_plotpolygon (PSL, GMT->current.plot.x, GMT->current.plot.y, (int)GMT->current.plot.n);
						if (!Ctrl->W.active) draw_line = false;	/* Did not want to actually draw the main line */
						if (Ctrl->L.outline) gmt_setpen (GMT, &current_pen);	/* Reset the pen to what -W indicates */
					}
					if (draw_line) {
						if ((GMT->current.plot.n = gmt_geo_to_xy_line (GMT, L->data[GMT_X], L->data[GMT_Y], L->n_rows)) == 0) continue;
						gmt_plot_line (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.pen, GMT->current.plot.n, current_pen.mode);
						plot_end_vectors (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n, &current_pen);	/* Maybe add vector heads */
					}
				}
				if (S.symbol == GMT_SYMBOL_FRONT) { /* Must also draw fault crossbars */
					gmt_setfill (GMT, &current_fill, (S.f.f_pen == -1) ? false : true);
					gmt_draw_front (GMT, GMT->current.plot.x, GMT->current.plot.y, GMT->current.plot.n, &S.f);
					if (S.f.f_pen == 0) gmt_setpen (GMT, &current_pen);	/* Reinstate current pen */
				}
				if (duplicate)	/* Free duplicate segment */
					gmt_free_segment (GMT, &L);
			}
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			Return (API->error);
		}
		gmt_map_clip_off (GMT);
	}

	if (S.u_set) GMT->current.setting.proj_length_unit = save_u;	/* Reset unit */

	if (S.symbol == GMT_SYMBOL_QUOTED_LINE) {
		if (S.G.save_labels) {	/* Want to save the line label locations (lon, lat, angle, label) */
			if ((error = gmt_contlabel_save_begin (GMT, &S.G)) != 0) Return (error);
			if ((error = gmt_contlabel_save_end (GMT, &S.G)) != 0) Return (error);
		}
		gmt_contlabel_plot (GMT, &S.G);
	}

	if (Ctrl->D.active) PSL_setorigin (PSL, -Ctrl->D.dx, -Ctrl->D.dy, 0.0, PSL_FWD);	/* Reset shift */

	if (clip_set && !S.G.delay) gmt_map_clip_off (GMT);	/* We delay map clip off if text clipping was chosen via -Sq<args:+e */

	PSL_setdash (PSL, NULL, 0);
	GMT->current.map.is_world = old_is_world;
	if (geovector) PSL->current.linewidth = 0.0;	/* Since we changed things under clip; this will force it to be set next */

	gmt_plane_perspective (GMT, -1, 0.0);

	if (S.symbol == GMT_SYMBOL_DECORATED_LINE) {	/* Plot those line decorating symbols via call to psxy */
		if ((error = plot_decorations (GMT, Decorate)) != 0)	/* Cannot possibly be a good thing */
			Return (error);
		if (GMT_Destroy_Data (API, &Decorate) != GMT_NOERROR) {	/* Might as well delete since no good later */
			Return (API->error);
		}
	}
	gmt_symbol_free (GMT, &S);

	gmt_plotend (GMT);

	Return (GMT_NOERROR);
}
