/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2014 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
/*
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: psscale draws a grayscale or colorscale either vertically
 * or horizontally, optionally with illumination.
 *
 */

#define THIS_MODULE_NAME	"psscale"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot a gray-scale or color-scale on maps"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->BJKOPRUVXYcptxy"

void GMT_linearx_grid (struct GMT_CTRL *GMT, struct PSL_CTRL *P, double w, double e, double s, double n, double dval);
double GMT_get_map_interval (struct GMT_CTRL *GMT, struct GMT_PLOT_AXIS_ITEM *T);

#define H_BORDER 16	/* 16p horizontal border space for -T */
#define V_BORDER 8	/* 8p vertical border space for -T */

#define N_FAVOR_IMAGE	1
#define N_FAVOR_POLY	2

#ifdef DEBUG
bool dump = false;
double dump_int_val;
#endif

/* Control structure for psscale */

struct PSSCALE_CTRL {
	struct A {	/* -A */
		bool active;
		unsigned int mode;
	} A;
	struct C {	/* -C<cptfile> */
		bool active;
		char *file;
	} C;
	struct D {	/* -D<xpos/ypos/length/width[h]> */
		bool active;
		bool horizontal;
		double x, y, width, length;
	} D;
	struct E {	/* -E[b|f][<length>][+n[<text>]] */
		bool active;
		unsigned int mode;
		double length;
		char *text;
	} E;
	struct G {	/* -Glow/high for input CPT truncation */
		bool active;
		double z_low, z_high;
	} G;
	struct I {	/* -I[<intens>|<min_i>/<max_i>] */
		bool active;
		double min, max;
	} I;
	struct M {	/* -M */
		bool active;
	} M;
	struct N {	/* -N<dpi>|p */
		bool active;
		unsigned int mode;
		double dpi;
	} N;
	struct L {	/* -L[i][<gap>] */
		bool active;
		bool interval;
		double spacing;
	} L;
	struct Q {	/* -Q */
		bool active;
	} Q;
	struct S {	/* -S */
		bool active;
	} S;
	struct T {	/* -T[+l<off>][+r<off>][+b<off>][+t<off>][+g<fill>][+p<pen>] */
		bool active, do_pen, do_fill;
		double off[4];
		struct GMT_PEN pen;
		struct GMT_FILL fill;
	} T;
	struct Z {	/* -Z<zfile> */
		bool active;
		char *file;
	} Z;
};

void *New_psscale_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSSCALE_CTRL *C;
	unsigned int k;
	
	C = GMT_memory (GMT, NULL, 1, struct PSSCALE_CTRL);
	
	/* Initialize values whose defaults are not 0/false/NULL */
	C->G.z_low = C->G.z_high = GMT->session.d_NaN;	/* No truncation */
	C->N.dpi = 600.0;
	C->I.min = -1.0;
	C->I.max = +1.0;
	C->L.spacing = -1.0;
	for (k = 0; k < 2; k++) C->T.off[k] = H_BORDER * GMT->session.u2u[GMT_PT][GMT_INCH];	/* Default is 8p padding */
	for (k = 2; k < 4; k++) C->T.off[k] = V_BORDER * GMT->session.u2u[GMT_PT][GMT_INCH];	/* Default is 8p padding */
	GMT_init_fill (GMT, &C->T.fill, -1.0, -1.0, -1.0);	/* No fill */
	return (C);
}

void Free_psscale_Ctrl (struct GMT_CTRL *GMT, struct PSSCALE_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->C.file) free (C->C.file);
	if (C->E.text) free (C->E.text);
	if (C->Z.file) free (C->Z.file);
	GMT_free (GMT, C);	
}

int GMT_psscale_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the psscale synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: psscale -D<xpos>/<ypos>/<length>/<width>[h] [-A[a|l|c]] [%s] [-C<cpt>]\n", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-E[b|f][<length>][+n[<txt>]]] [-G<zlo>/<zhi>] [-I[<max_intens>|<low_i>/<high_i>]\n\t[%s] [%s] [-K] [-L[i][<gap>[<unit>]]] [-M] [-N[p|<dpi>]]\n", GMT_J_OPT, GMT_Jz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-O] [-P] [-Q] [%s] [-S]\n\t[-T[+p<pen>][+g<fill>][+l|r|b|t<off>]] [%s] [%s]\n", GMT_Rgeoz_OPT, GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-Z<zfile>] [%s]\n\t[%s] [%s]\n\n", GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "\t-D Set top mid-point position and length/width for scale.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give negative length to reverse the scalebar.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append h for horizontal scale\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Place the desired annotations/labels on the other side of the colorscale instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append a or l to move only the annotations or labels to the other side.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append c to plot vertical labels as columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-B Set scale annotation interval and label. Use y-label to set unit label.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no annotation interval is set it is taken from the cpt file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Color palette file. If not set, stdin is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   By default all color changes are annotated (but see -B).  To use a subset,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   add an extra column to the cpt-file with a L, U, or B\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to annotate Lower, Upper, or Both color segment boundaries.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If a categorical CPT file is given the -Li is set automatically.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Add sidebar triangles for back- and foreground colors.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify b(ackground) or f(oreground) to get one only [Default is both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append triangle height [Default is half the barwidth].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +n to draw rectangle with NaN color and label with <txt> [NaN].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Truncate incoming CPT to be limited to the z-range <zlo>/<zhi>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   To accept one if the incoming limits, set to other to NaN.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Add illumination for +-<max_intens> or <low_i> to <high_i> [-1.0/1.0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, specify <lower>/<upper> intensity values.\n");
	GMT_Option (API, "J-Z,K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L For equal-sized color rectangles. -B interval cannot be used.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append i to annotate the interval range instead of lower/upper.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <gap> is appended, we separate each rectangle by <gap> units and center each\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   lower (z0) annotation on the rectangle.  Ignored if not a discrete cpt table.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -I is used then each rectangle will have the illuminated constant color.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Force monochrome colorbar using GMT_YIQ transformation.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Control how color-scale is represented by PostScript.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append p to indicate a preference for using polygons, if possible;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   otherwise it indicates a preference for using colorimage, if possible.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default uses the method that produces the simplest PostScript code.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append preferred dots-per-inch for rasterization when colorimage is used [600].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Plot colorbar using logarithmic scale and annotate powers of 10 [Default is linear].\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Skip drawing color boundary lines on color scale [Default draws lines].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Place a rectangle behind the scale [No background rectangle].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Make at least one seletion of rectangle fill or outline pen.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   You can nudge the extent of the rectangle in all four directions\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   by appending any of +l<off>, +r<off>, +b<off>, +t<off>.\n");
	GMT_Option (API, "U,V,X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Give file with colorbar-width (in %s) per color entry.\n",
		API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t   By default, width of entry is scaled to color range,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   i.e., z = 0-100 gives twice the width as z = 100-150.\n");
	GMT_Option (API, "c,p");
	GMT_Message (API, GMT_TIME_NONE, "\t   (Requires -R and -J for proper functioning).\n");
	GMT_Option (API, "t,.");
	
	return (EXIT_FAILURE);
}

int GMT_psscale_parse (struct GMT_CTRL *GMT, struct PSSCALE_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to psscale and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n, pos, n_errors = 0, n_files = 0;
	int j;
	char flag, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, *c = NULL;
	char txt_c[GMT_LEN256] = {""}, txt_d[GMT_LEN256] = {""}, p[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files (should be 0) */
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':
				Ctrl->A.active = true;
				if (!opt->arg[0]) Ctrl->A.mode |= 3;
				for (j = 0; opt->arg[j]; j++) {
					switch (opt->arg[j]) {
						case 'a': Ctrl->A.mode |= 1; break;
						case 'l': Ctrl->A.mode |= 2; break;
						case 'c': Ctrl->A.mode |= 4; break;
					}
				}
				break;
			case 'C':
				Ctrl->C.active = true;
				if (Ctrl->C.file) free (Ctrl->C.file);
				Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':
				Ctrl->D.active = true;
				n = (unsigned int)strlen (opt->arg) - 1;
				flag = opt->arg[n];
				if (flag == 'h' || flag == 'H') {
					Ctrl->D.horizontal = true;
					opt->arg[n] = 0;	/* Temporarily remove this for sscanf */
				}
				sscanf (opt->arg, "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
				if (Ctrl->D.horizontal) opt->arg[n] = flag;	/* Restore the flag */
				Ctrl->D.x   = GMT_to_inch (GMT, txt_a);
				Ctrl->D.y   = GMT_to_inch (GMT, txt_b);
				Ctrl->D.length = GMT_to_inch (GMT, txt_c);
				Ctrl->D.width  = GMT_to_inch (GMT, txt_d);
				break;
			case 'E':
				Ctrl->E.active = true;
				if ((c = strchr (opt->arg, 'n'))) {	/* Got +n[<text>] */
					c--;	*c = 0; c += 2;
					Ctrl->E.text = (*c) ? strdup (c) : strdup ("NaN");
					Ctrl->E.mode = 4;
					c -= 2;
				}
				j = 0;
				if (opt->arg[0] == 'b') {
					Ctrl->E.mode |= 1;
					j = 1;
				}
				else if (opt->arg[j] == 'f') {
					Ctrl->E.mode |= 2;
					j = 1;
				}
				if (opt->arg[j] == 'b') {
					Ctrl->E.mode |= 1;
					j++;
				}
				else if (opt->arg[j] == 'f') {
					Ctrl->E.mode |= 2;
					j++;
				}
				if (j == 0) Ctrl->E.mode |= 3;	/* No b|f added */
				if (opt->arg[j]) Ctrl->E.length = GMT_to_inch (GMT, &opt->arg[j]);
				if (c) *c = '+';	/* Put back the + sign */
				break;
			case 'G':	/* truncate incoming CPT */
				Ctrl->G.active = true;
				j = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b);
				n_errors += GMT_check_condition (GMT, j < 2, "Syntax error -G option: Must specify z_low/z_high\n");
				if (!(txt_a[0] == 'N' || txt_a[0] == 'n') || !strcmp (txt_a, "-")) Ctrl->G.z_low = atof (txt_a);
				if (!(txt_b[0] == 'N' || txt_b[0] == 'n') || !strcmp (txt_b, "-")) Ctrl->G.z_high = atof (txt_b);
				n_errors += GMT_check_condition (GMT, GMT_is_dnan (Ctrl->G.z_low) && GMT_is_dnan (Ctrl->G.z_high), "Syntax error -G option: Both of z_low/z_high cannot be NaN\n");
				break;
			case 'I':
				Ctrl->I.active = true;
				if (opt->arg[0]) {
					j = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b);
					if (j == 1) {
						Ctrl->I.max = atof (txt_a);
						Ctrl->I.min = -Ctrl->I.max;
					}
					else {
						Ctrl->I.min = atof (txt_a);
						Ctrl->I.max = atof (txt_b);
					}
				} 
				break;
			case 'L':
				Ctrl->L.active = true;
				j = 0;
				if (opt->arg[0] == 'i') Ctrl->L.interval = true, j = 1;
				if (opt->arg[j]) Ctrl->L.spacing = GMT_to_inch (GMT, &opt->arg[j]);
				break;
			case 'M':
				Ctrl->M.active = true;
				break;
			case 'N':
				Ctrl->N.active = true;
				/* Default will use image@600 dpi or polygons according to what is simplest */
				if (opt->arg[0] == 'p')	/* Preferentially use polygon fill if at all possible */
					Ctrl->N.mode = N_FAVOR_POLY;
				else {			/* Preferentially use image if at all possible */
					Ctrl->N.mode = N_FAVOR_IMAGE;
					if (opt->arg[0]) Ctrl->N.dpi = atof (opt->arg);
				}
				break;
			case 'Q':
				Ctrl->Q.active = true;
				break;
			case 'S':
				Ctrl->S.active = true;
				break;
			case 'T':
				Ctrl->T.active = true;
				pos = 0;
				while (GMT_strtok (opt->arg, "+", &pos, p)) {
					switch (p[0]) {
						case 'l':	/* Left nudge */
							Ctrl->T.off[XLO] = atof (&p[1]);
							break;
						case 'r':	/* Right nudge */
							Ctrl->T.off[XHI] = atof (&p[1]);
							break;
						case 'b':	/* Bottom nudge */
							Ctrl->T.off[YLO] = atof (&p[1]);
							break;
						case 't':	/* Top nudge */
							Ctrl->T.off[YHI] = atof (&p[1]);
							break;
						case 'g':	/* Fill */
							if (GMT_getfill (GMT, &p[1], &Ctrl->T.fill)) {
								GMT_fill_syntax (GMT, 'T', " ");
								n_errors++;
							}
							Ctrl->T.do_fill = true;
							break;
						case 'p':	/* Pen */
							if (GMT_getpen (GMT, &p[1], &Ctrl->T.pen)) {
								GMT_pen_syntax (GMT, 'W', " ");
								n_errors++;
							}
							Ctrl->T.do_pen = true;
							break;
					}
				}
				break;
			case 'Z':
				Ctrl->Z.active = true;
				Ctrl->Z.file = strdup (opt->arg);
				break;
#ifdef DEBUG
			case 'd':	/* Dump out interpolated colors for debugging */
				dump = true;
				dump_int_val = (opt->arg[0]) ? atof (opt->arg) : 1.0;
				break;
#endif

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	if (!Ctrl->D.active) {
		GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: -D is required and must be specified\n");
		n_errors++;
	}
	else {
		n_errors += GMT_check_condition (GMT, fabs (Ctrl->D.length) < GMT_SMALL , "Syntax error -D option: scale length must be nonzero\n");
		n_errors += GMT_check_condition (GMT, Ctrl->D.width <= 0.0, "Syntax error -D option: scale width must be positive\n");
	}
	n_errors += GMT_check_condition (GMT, n_files > 0, "Syntax error: No input files are allowed\n");
	n_errors += GMT_check_condition (GMT, Ctrl->L.active && GMT->current.map.frame.set, "Syntax error -L option: Cannot be used if -B option sets increments.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->N.active && Ctrl->N.dpi <= 0.0, "Syntax error -N option: The dpi must be > 0.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && !(Ctrl->T.do_pen || Ctrl->T.do_fill), "Syntax error -T option: Must set pen or fill.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Z.active && !Ctrl->Z.file, "Syntax error -Z option: No file given\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Z.active && Ctrl->Z.file && GMT_access (GMT, Ctrl->Z.file, R_OK), "Syntax error -Z option: Cannot access file %s\n", Ctrl->Z.file);

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

double get_z (struct GMT_PALETTE *P, double x, double *width, unsigned int n)
{
	unsigned int i = 0;
	double tmp;

	tmp = width[0];
	while (i < n && x > tmp) tmp += width[++i];
	if (i == n) return (P->range[P->n_colors-1].z_high);
	return (P->range[i].z_low + (x - tmp + width[i]) * (P->range[i].z_high - P->range[i].z_low) / width[i]);
}

void fix_format (char *unit, char *format)
{
	unsigned int i, j;
	char text[GMT_LEN64] = {""}, new_format[GMT_BUFSIZ] = {""};

	/* Check if annotation units should be added */

	if (unit && unit[0]) {	/* Must append the unit string */
		if (!strchr (unit, '%'))	/* No percent signs */
			strncpy (text, unit, GMT_LEN64);
		else {
			for (i = j = 0; i < strlen (unit); i++) {
				text[j++] = unit[i];
				if (unit[i] == '%') text[j++] = unit[i];
			}
			text[j] = 0;
		}
		if (text[0] == '-')	/* No space between annotation and unit */
			sprintf (new_format, "%s%s", format, &text[1]);
		else		/* 1 space between annotation and unit */
			sprintf (new_format, "%s %s", format, text);
		strcpy (format, new_format);
	}
}

#define FONT_HEIGHT_PRIMARY (GMT->session.font[GMT->current.setting.font_annot[0].id].height)

void gmt_draw_colorbar (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct GMT_PALETTE *P, double length, double width, double *z_width, \
	double bit_dpi, unsigned int N_mode, unsigned int flip, bool B_set, bool equi, bool horizontal, bool logscl, bool intens, \
	double *max_intens, bool skip_lines, unsigned int extend, \
	double e_length, char *nan_text, double gap, bool interval_annot, bool monochrome, struct T Ctrl_T)
{
	unsigned int i, ii, id, j, nb, ndec = 0, dec, depth, Label_justify, form;
	unsigned int cap = PSL->internal.line_cap, join = PSL->internal.line_join;
	unsigned int nx = 0, ny = 0, nm, barmem, k, justify, l_justify, n_use_labels = 0;
	int this_just, p_val;
	bool reverse, all = true, use_image, center = false, const_width = true, do_annot, use_labels, cpt_auto_fmt = true;
	char format[GMT_LEN256] = {""}, text[GMT_LEN256] = {""}, test[GMT_LEN256] = {""}, unit[GMT_LEN256] = {""}, label[GMT_LEN256] = {""};
	static char *method[2] = {"polygons", "colorimage"};
	unsigned char *bar = NULL, *tmp = NULL;
	double off, annot_off, label_off, len, len2, size, x0, x1, dx, xx, dir, y_base, y_annot, y_label, xd = 0.0, yd = 0.0, xt = 0.0;
	double z = 0.0, xleft, xright, inc_i, inc_j, start_val, stop_val, nan_off = 0.0, rgb[4], rrggbb[4], xp[4], yp[4], prev_del_z, this_del_z = 0.0;
	struct GMT_FILL *f = NULL;
	struct GMT_PLOT_AXIS *A;
#ifdef DEBUG
	unsigned int dump_k_val, zzz;
#endif

	GMT->current.setting.map_annot_offset[0] = fabs (GMT->current.setting.map_annot_offset[0]);	/* No 'inside' annotations allowed in colorbar */

	/* Temporarily change to miter join so boxes and end triangles have near corners */
	PSL_setlinejoin (PSL, PSL_MITER_JOIN);

#ifdef DEBUG
	if (!intens) dump = false;	/* Must be run with -I */
	if (dump) {
		ny = urint (width * bit_dpi);
		inc_j = (ny > 1) ? (max_intens[1] - max_intens[0]) / (ny - 1) : 0.0;
		dump_k_val = ny - urint ((dump_int_val - max_intens[0])/inc_j) - 1;
		fprintf (stderr, "#Produced with dump_int_val = %g, dump_k_val = %u of %u\n", dump_int_val, dump_k_val, ny);
		fprintf (stderr, "#z\tred\tgreen\tblue\tr_int\tg_int\tb_int\tps_r\tps_g\tps_b\n");
	}
#endif

	/* Find max decimals needed */

	start_val = P->range[0].z_low;
	stop_val  = P->range[P->n_colors-1].z_high;
	if (B_set) {	/* Let the general -B machinery do the annotation labeling */
		if (logscl) {	/* Must set original start/stop values for axis */
			start_val = pow (10.0, P->range[0].z_low);
			stop_val  = pow (10.0, P->range[P->n_colors-1].z_high);
		}
		ndec = GMT_get_format (GMT, GMT->current.map.frame.axis[GMT_X].item[GMT_ANNOT_UPPER].interval, GMT->current.map.frame.axis[GMT_X].unit, GMT->current.map.frame.axis[GMT_X].prefix, format);
	}
	else {	/* Do annotations explicitly, one by one.  Try automatic guessing of decimals if equidistant cpt */
		bool const_interval = true, exp_notation = false;
		for (i = 0; i < P->n_colors; i++) {
			if (P->range[i].label) n_use_labels++;
			if (P->range[i].annot & 1) {
				z = P->range[i].z_low;
				if ((dec = GMT_get_format (GMT, z, NULL, NULL, text)) > ndec) {
					strncpy (format, text, GMT_LEN256);
					ndec = dec;
				}
			}
			if (P->range[i].annot & 2) {
				z = P->range[i].z_high;
				if ((dec = GMT_get_format (GMT, z, NULL, NULL, text)) > ndec) {
					strncpy (format, text, GMT_LEN256);
					ndec = dec;
				}
			}
			if (format[0] && !exp_notation) {
				sprintf (text, format, z);
				if (strchr (text, 'e')) exp_notation = true;	/* Got exponential notation in at least one place */
			}
			prev_del_z = this_del_z;
			this_del_z = P->range[i].z_high - P->range[i].z_low;
			if (i && !doubleAlmostEqualZero (this_del_z, prev_del_z)) const_interval = false;
			if (P->range[i].annot) all = false;
		}
		if (!const_interval) {	/* May have to abandon automatic format guessing */
			if (exp_notation && !strchr (GMT->current.setting.format_float_map, 'e')) {	/* Unwanted exponential notation: Abandon automatic format detection and use FORMAT_FLOAT_MAP */
				cpt_auto_fmt = false;
				ndec = 0;
			}
		}
	}
	if (equi && n_use_labels == P->n_colors)
		all = use_labels = true;	/* Only use optional text labels for equal length scales */
	else
		use_labels = false;

	/* Check if steps in color map have constant width */
	for (i = 1; i < P->n_colors && const_width; i++)
		if (fabs (z_width[i] - z_width[0]) > GMT_SMALL) const_width = false;
	
	if (ndec == 0) {	/* Not -B and no decimals are needed */
		strncpy (format, GMT->current.setting.format_float_map, GMT_LEN256);
		fix_format (GMT->current.map.frame.axis[GMT_X].unit, format);	/* Add units if needed */
	}

	len = GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER];	/* +ve means draw on the outside of bar */
	len2 = GMT->current.setting.map_tick_length[GMT_TICK_UPPER];
	xleft = x0 = 0.0;

	reverse = (length < 0.0);
	length = fabs (length);
	xright = length;
	if (N_mode == N_FAVOR_IMAGE)	/* favor image if possible */
		use_image = (!P->has_pattern && gap <= 0.0);
	else if (N_mode == N_FAVOR_POLY)	/* favor polygon if possible */
		use_image = P->is_continuous;
	else	/* Auto mode */
		use_image = (!P->has_pattern && gap <= 0.0 && (equi || const_width || P->is_continuous));
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Color bar will be plotted using %s\n", method[use_image]);
	

	if ((gap >= 0.0 || interval_annot) && !P->is_continuous) {	/* Want to center annotations for discrete colortable, using lower z0 value */
		center = (interval_annot || gap >= 0.0);
		if (gap > 0.0) skip_lines = true;
		gap *= 0.5;
		if (interval_annot) {
			sprintf (text, "%s - %s", format, format);
			strncpy (format, text, GMT_LEN256);
		}
	}
	if (gap < 0.0) gap = 0.0;

	if (use_image || intens) {	/* Make bitimage for colorbar using bit_dpi */
		bool resample;
		if (N_mode == N_FAVOR_IMAGE) {	/* Honor the given image dpi */
			nx = (!use_image && gap > 0.0) ? P->n_colors : urint (length * bit_dpi);
			ny = urint (width * bit_dpi);
			resample = true;
		}
		else {	/* Do the smallest possible image */
			resample = P->is_continuous;
			nx = (P->is_continuous) ? urint (length * bit_dpi) : P->n_colors;
			ny = (intens) ? urint (width * bit_dpi) : 1;
		}
		nm = nx * ny;
		inc_i = length / nx;
		inc_j = (ny > 1) ? (max_intens[1] - max_intens[0]) / (ny - 1) : 0.0;
		barmem = (monochrome || P->is_gray) ? nm : 3 * nm;
		bar = GMT_memory (GMT, NULL, barmem, unsigned char);

		/* Load bar image */

		for (i = 0; i < nx; i++) {
			z = (resample) ? get_z (P, (i+0.5) * inc_i, z_width, P->n_colors) : P->range[i].z_low;
			GMT_get_rgb_from_z (GMT, P, z, rrggbb);
			ii = (reverse) ? nx - i - 1 : i;
			for (j = 0; j < ny; j++) {
				GMT_rgb_copy (rgb, rrggbb);
				k = j * nx + ii;
#ifdef DEBUG
				if (dump && z > 2499.2 && z < 2500.25) {
					zzz = 1;
				}
#endif
				if (intens) GMT_illuminate (GMT, max_intens[1] - j * inc_j, rgb);
				if (P->is_gray)	/* All gray, pick red */
					bar[k] = GMT_u255 (rgb[0]);
				else if (monochrome)	/* Convert to gray using the GMT_YIQ transformation */
					bar[k] = GMT_u255 (GMT_YIQ (rgb));
				else {
					k *= 3;
					bar[k++] = GMT_u255 (rgb[0]);
					bar[k++] = GMT_u255 (rgb[1]);
					bar[k++] = GMT_u255 (rgb[2]);
#ifdef DEBUG
					if (dump && j == dump_k_val) fprintf (stderr, "%g\t%g\t%g\t%g\t%g\t%g\t%g\t%d\t%d\t%d\n",
						z, rrggbb[0], rrggbb[1], rrggbb[2], rgb[0], rgb[1], rgb[2], bar[k-3], bar[k-2], bar[k-1]);
#endif
				}
			}
		}
	}

	if (Ctrl_T.active) {	/* Place rectangle behind the color bar */
		double x_center, y_center, dim[2] = {1.0,0.0}, u_off = 0.0, v_off = 0.0;
			
		GMT_setfill (GMT, &Ctrl_T.fill, Ctrl_T.do_pen);
		GMT_setpen (GMT, &Ctrl_T.pen);
		annot_off = MAX (0.0, GMT->current.setting.map_annot_offset[0]);	/* Allow for space between bar and annotations */
		if (horizontal) {	/* Determine center and dimensions of horizontal background rectangle */
			x_center = 0.5 * length; y_center = 0.5 * width;
			/* extend box height by annotation size */
			annot_off += GMT->current.setting.font_annot[0].size * GMT->session.u2u[GMT_PT][GMT_INCH];
			/* If a label then add space for offset and label height */
			if (GMT->current.map.frame.axis[GMT_X].label[0]) annot_off += MAX (0.0, GMT->current.setting.map_label_offset) + GMT->current.setting.font_label.size * GMT->session.u2u[GMT_PT][GMT_INCH];
			/* If a unit then add space on the right to deal with the unit */
			if (GMT->current.map.frame.axis[GMT_Y].label[0]) u_off = MAX (0.0, GMT->current.setting.map_annot_offset[0]) + strlen (GMT->current.map.frame.axis[GMT_Y].label) * GMT_DEC_SIZE * GMT->current.setting.font_annot[0].size * GMT->session.u2u[GMT_PT][GMT_INCH];
			/* Adjust center and dimensions of the modified box */
			x_center += 0.5 * (u_off + Ctrl_T.off[XHI] - Ctrl_T.off[XLO]);
			y_center += 0.5 * (Ctrl_T.off[YHI] - Ctrl_T.off[YLO] - annot_off);
			dim[GMT_X] = length + u_off + Ctrl_T.off[XHI] + Ctrl_T.off[XLO];
			dim[GMT_Y] = width + annot_off + Ctrl_T.off[YHI] + Ctrl_T.off[YLO];
		}
		else {	/* Determine center and dimensions of vertical background rectangle */
			y_center = 0.5 * length; x_center = 0.5 * width;
			/* Must guesstimate the width of the largest horizontal annotation */
			sprintf (text, "%ld", lrint (floor (P->range[0].z_low)));
			sprintf (test, "%ld", lrint (ceil (center ? P->range[P->n_colors-1].z_low : P->range[P->n_colors-1].z_high)));
			off = ((MAX ((int)strlen (text), (int)strlen (test)) + ndec) * GMT_DEC_SIZE +
				((ndec > 0) ? GMT_PER_SIZE : 0.0))
				* GMT->current.setting.font_annot[0].size * GMT->session.u2u[GMT_PT][GMT_INCH];
			annot_off += off;
			/* Increase width if there is a label */
			if (GMT->current.map.frame.axis[GMT_X].label[0]) annot_off += MAX (0.0, GMT->current.setting.map_label_offset) + GMT->current.setting.font_label.size * GMT->session.u2u[GMT_PT][GMT_INCH];
			/* If a unit then consider if its width exceeds the bar width; then use half the excess to adjust center and width of box, and its height to adjust the height of box */
			if (GMT->current.map.frame.axis[GMT_Y].label[0]) {
				u_off = 0.5 * MAX (0.0, strlen (GMT->current.map.frame.axis[GMT_Y].label) * GMT_DEC_SIZE * GMT->current.setting.font_annot[0].size * GMT->session.u2u[GMT_PT][GMT_INCH] - width);
				v_off = MAX (0.0, GMT->current.setting.map_annot_offset[0]) + GMT->current.setting.font_annot[0].size * FONT_HEIGHT_PRIMARY * GMT->session.u2u[GMT_PT][GMT_INCH];
			}
			/* Adjust center and dimensions of the modified box */
			x_center += 0.5 * (Ctrl_T.off[XHI] + annot_off - Ctrl_T.off[XLO] - u_off);
			y_center += 0.5 * (Ctrl_T.off[YHI] + v_off - Ctrl_T.off[YLO]);
			dim[GMT_Y] = length + v_off + Ctrl_T.off[XHI] + Ctrl_T.off[XLO];
			dim[GMT_X] = width + u_off + annot_off + Ctrl_T.off[YHI] + Ctrl_T.off[YLO];
		}
		PSL_plotsymbol (PSL, x_center, y_center, dim, PSL_RECT);
	}
	GMT_setpen (GMT, &GMT->current.setting.map_frame_pen);

	unit[0] = label[0] = 0;
	/* Defeat the auto-repeat of axis info */
	if (!strcmp (GMT->current.map.frame.axis[GMT_X].label, GMT->current.map.frame.axis[GMT_Y].label)) GMT->current.map.frame.axis[GMT_Y].label[0] = 0;
	/* Save label and unit, because we are going to switch them off in GMT->current.map.frame and do it ourselves */
	strncpy (label, GMT->current.map.frame.axis[GMT_X].label, GMT_LEN256);
	strncpy (unit, GMT->current.map.frame.axis[GMT_Y].label, GMT_LEN256);
	GMT->current.map.frame.axis[GMT_X].label[0] = GMT->current.map.frame.axis[GMT_Y].label[1] = 0;

	if (flip & 1) {
		justify = l_justify = (horizontal) ? 2 : 7;
		y_base = width;
		dir = 1.0;
	}
	else {
		justify = (horizontal) ? 10 : 7;
		l_justify = (horizontal) ? 10 : 5;
		y_base = 0.0;
		dir = -1.0;
	}
	if (flip & 2) {
		Label_justify = 2;
		y_label = width + GMT->current.setting.map_label_offset;
	}
	else {
		Label_justify = 10;
		y_label = -GMT->current.setting.map_label_offset;
	}

	if (extend) {	/* Adjustment to triangle base coordinates so pen abuts perfectly with bar */
		double theta, s, c, w = GMT->current.setting.map_frame_pen.width * GMT->session.u2u[GMT_PT][GMT_INCH];
		theta = atan (2.0 * e_length / width);	/* triangle base angle in radians */
		sincos (theta, &s, &c);
		xd = 0.5 * w * (c - 1.0);
		yd = 0.5 * w * (s - 1.0);
		xt = yd * 2.0 * e_length / width;	/* Shift of triangle peak x-pos to maintain original angle theta */
	}

	/* Current point (0,0) is now at lower left location of bar */

	depth = (monochrome || P->is_gray) ? 8 : 24;
	if (horizontal) {
		form = GMT_setfont (GMT, &GMT->current.setting.font_annot[0]);

		if (use_image)	/* Must plot as image */
			PSL_plotcolorimage (PSL, 0.0, 0.0, length, width, PSL_BL, bar, nx, ny, depth);
		else {			/* Plot as rectangles */
			x0 = x1 = 0.0;
			for (i = 0; i < P->n_colors; i++) {
				ii = (reverse) ? P->n_colors - i - 1 : i;
				x1 += z_width[ii];
				if ((f = P->range[ii].fill))	/* Using pattern fills */
					GMT_setfill (GMT, f, center);
				else if (intens) {
					nb = (P->is_gray || monochrome) ? 1 : 3;
					tmp = GMT_memory (GMT, NULL, ny*nb, unsigned char);
					for (j = 0, k = i*nb; j < ny*nb; k+=(nx-1)*nb) {
						tmp[j++] = bar[k++];
						tmp[j++] = bar[k++];
						tmp[j++] = bar[k++];
					}
					PSL_plotcolorimage (PSL, x0 + gap, 0.0, z_width[ii] - 2*gap, width, PSL_BL, tmp, 1, ny, depth);
					GMT_free (GMT, tmp);
					PSL_setfill (PSL, GMT->session.no_rgb, center);
				}
				else {
					GMT_rgb_copy (rgb, P->range[ii].rgb_low);
					if (intens) GMT_illuminate (GMT, max_intens[1], rgb);
					if (monochrome) rgb[0] = rgb[1] = rgb[2] = GMT_YIQ (rgb);
					PSL_setfill (PSL, rgb, center);
				}
				PSL_plotbox (PSL, x0+gap, 0.0, x1-gap, width);
				x0 = x1;
			}
		}

		annot_off = ((len > 0.0 && !center) ? len : 0.0) + GMT->current.setting.map_annot_offset[0];
		label_off = annot_off + GMT->current.setting.font_annot[0].size * GMT->session.u2u[GMT_PT][GMT_INCH] + GMT->current.setting.map_label_offset;
		y_annot = y_base + dir * annot_off;
		if ((flip & 1) == (flip & 2) / 2) y_label = y_base + dir * label_off;

		PSL_setlinecap (PSL, PSL_BUTT_CAP);	/* Butt cap required for outline of triangle */
		
		if (extend & (reverse + 1)) {	/* Add color triangle on left side */
			xp[0] = xp[2] = xleft - gap;	xp[1] = xleft - gap - e_length;
			yp[0] = width - yd;	yp[2] = yd;	yp[1] = 0.5 * width;
			for (i = 0; i < 3; i++) xp[i] += xd;
			xp[1] += xt;
			id = (reverse) ? GMT_FGD : GMT_BGD;
			if ((f = P->patch[id].fill))
				GMT_setfill (GMT, f, false);
			else {
				GMT_rgb_copy (rgb, P->patch[id].rgb);
				if (monochrome) rgb[0] = rgb[1] = rgb[2] = GMT_YIQ (rgb);
				PSL_setfill (PSL, rgb, false);
			}
			PSL_plotpolygon (PSL, xp, yp, 3);
			PSL_plotline (PSL, xp, yp, 3, PSL_MOVE + PSL_STROKE);
			nan_off = e_length - xd;	/* Must make space for the triangle */
		}
		if (extend & 4) {	/* Add NaN rectangle on left side */
			nan_off += e_length;
			xp[0] = xp[1] = xleft - gap;	xp[2] = xp[3] = xp[0] - MAX (width, e_length);
			for (i = 0; i < 4; i++) xp[i] -= nan_off;
			yp[0] = yp[3] = width;	yp[1] = yp[2] = 0.0;
			if ((f = P->patch[GMT_NAN].fill))
				GMT_setfill (GMT, f, true);
			else {
				GMT_rgb_copy (rgb, P->patch[GMT_NAN].rgb);
				if (monochrome) rgb[0] = rgb[1] = rgb[2] = GMT_YIQ (rgb);
				PSL_setfill (PSL, rgb, true);
			}
			PSL_plotpolygon (PSL, xp, yp, 4);
			PSL_plottext (PSL, xp[2] - fabs (GMT->current.setting.map_annot_offset[0]), 0.5 * width, GMT->current.setting.font_annot[0].size, nan_text, 0.0, 7, form);
		}
		if (extend & (2 - reverse)) {	/* Add color triangle on right side */
			xp[0] = xp[2] = xright + gap;	xp[1] = xp[0] + e_length;
			yp[0] = width - yd;	yp[2] = yd;	yp[1] = 0.5 * width;
			for (i = 0; i < 3; i++) xp[i] -= xd;
			xp[1] -= xt;
			id = (reverse) ? GMT_BGD : GMT_FGD;
			if ((f = P->patch[id].fill))
				GMT_setfill (GMT, f, false);
			else {
				GMT_rgb_copy (rgb, P->patch[id].rgb);
				if (monochrome) rgb[0] = rgb[1] = rgb[2] = GMT_YIQ (rgb);
				PSL_setfill (PSL, rgb, false);
			}
			PSL_plotpolygon (PSL, xp, yp, 3);
			PSL_plotline (PSL, xp, yp, 3, PSL_MOVE + PSL_STROKE);
		}

		PSL_setlinecap (PSL, PSL_SQUARE_CAP);	/* Square cap required for box of scale bar */

		if (gap == 0.0) {
			if ((flip & 1) || !B_set) PSL_plotsegment (PSL, xleft, 0.0, xleft + length, 0.0);
			if (!(flip & 1) || !B_set) PSL_plotsegment (PSL, xleft, width, xleft + length, width);
			PSL_plotsegment (PSL, xleft, 0.0, xleft, width);
			PSL_plotsegment (PSL, xright, 0.0, xright, width);
		}

		if (B_set) {	/* Used -B */
			GMT_auto_frame_interval (GMT, GMT_X, GMT_ANNOT_UPPER);
			A = &GMT->current.map.frame.axis[GMT_X];
			GMT_xy_axis (GMT, xleft, y_base, length, start_val, stop_val, A, !(flip & 1), GMT->current.map.frame.side[flip & 1 ? N_SIDE : S_SIDE] & 2);
			if (A->item[GMT_GRID_UPPER].active) {
				dx = GMT_get_map_interval (GMT, &A->item[GMT_GRID_UPPER]);
				GMT_setpen (GMT, &GMT->current.setting.map_grid_pen[0]);
				GMT_linearx_grid (GMT, PSL, P->range[0].z_low, P->range[P->n_colors-1].z_high, 0.0, width, dx);
			}
		}
		else {
			/* First draw gridlines, unless skip_lines is true */

			if (!skip_lines) {
				GMT_setpen (GMT, &GMT->current.setting.map_grid_pen[0]);
				x1 = xleft;
				for (i = 0; i < P->n_colors; i++) {
					xx = (reverse) ? xright - x1 : x1;
					PSL_plotsegment (PSL, xx, 0.0, xx, width);
					x1 += z_width[i];
				}
				xx = (reverse) ? xleft : xright;
				PSL_plotsegment (PSL, xx, 0.0, xx, width);
			}

			/* Then annotate and draw tickmarks */

			GMT_setpen (GMT, &GMT->current.setting.map_tick_pen[0]);
			x1 = xleft;
			if (center) x1 += 0.5 * z_width[0];
			for (i = 0; i < P->n_colors; i++) {
				xx = (reverse) ? xright - x1 : x1;
				if (!center) PSL_plotpoint (PSL, xx, y_base, PSL_MOVE);
				if (all || (P->range[i].annot & 1)) {	/* Annotate this */
					if (!center) PSL_plotpoint (PSL, 0.0, dir * len, PSL_DRAW + PSL_STROKE + PSL_REL);
					this_just = justify;
					do_annot = true;
					if (use_labels && P->range[i].label) {
						strncpy (text, P->range[i].label, GMT_LEN256);
						this_just = l_justify;
					}
					else if (center && interval_annot)
						sprintf (text, format, P->range[i].z_low, P->range[i].z_high);
					else if (logscl) {
						p_val = irint (P->range[i].z_low);
						if (doubleAlmostEqualZero (P->range[i].z_low, (double)p_val))
							sprintf (text, "10@+%d@+", p_val);
						else
							do_annot = false;
					}
					else
						sprintf (text, format, P->range[i].z_low);
					if (do_annot) PSL_plottext (PSL, xx, y_annot, GMT->current.setting.font_annot[0].size, text, 0.0, -this_just, form);
				}
				else
					if (!center) PSL_plotpoint (PSL, 0.0, dir * len2, PSL_DRAW + PSL_STROKE + PSL_REL);
				x1 += z_width[i];
			}
			if (!center && !use_labels) {
				xx = (reverse) ? xleft : xright;
				PSL_plotpoint (PSL, xx, y_base, PSL_MOVE);
				i = P->n_colors-1;
				if (all || (P->range[i].annot & 2)) {
					PSL_plotpoint (PSL, 0.0, dir * len, PSL_DRAW + PSL_STROKE + PSL_REL);
					this_just = justify;
					do_annot = true;
					if (logscl) {
						p_val = irint (P->range[i].z_high);
						if (doubleAlmostEqualZero (P->range[i].z_high, (double)p_val))
							sprintf (text, "10@+%d@+", p_val);
						else
							do_annot = false;
					}
					else
						sprintf (text, format, P->range[i].z_high);
					if (do_annot) PSL_plottext (PSL, xx, y_annot, GMT->current.setting.font_annot[0].size, text, 0.0, -this_just, form);
				}
				else
					PSL_plotpoint (PSL, 0.0, dir * len2, PSL_DRAW + PSL_STROKE + PSL_REL);
			}
		}
		if (label[0]) {	/* Add label */
			form = GMT_setfont (GMT, &GMT->current.setting.font_label);
			PSL_plottext (PSL, xleft + 0.5 * length, y_label, GMT->current.setting.font_label.size, label, 0.0, Label_justify, form);
		}
		if (unit[0]) {	/* Add unit label */
			form = GMT_setfont (GMT, &GMT->current.setting.font_annot[0]);
			PSL_plottext (PSL, xright + e_length + GMT->current.setting.map_annot_offset[0], 0.5 * width, GMT->current.setting.font_annot[0].size, unit, 0.0, 5, form);
		}
	}
	else {	/* Vertical scale */
		form = GMT_setfont (GMT, &GMT->current.setting.font_annot[0]);

		PSL_setorigin (PSL, width, 0.0, 90.0, PSL_FWD);
		if (use_image)	/* Must plot with image */
			PSL_plotcolorimage (PSL, 0.0, 0.0, length, width, PSL_BL, bar, nx, ny, depth);
		else {			/* Plot as rectangles */
			x0 = x1 = 0.0;
			for (i = 0; i < P->n_colors; i++) {
				ii = (reverse) ? P->n_colors - i - 1 : i;
				x1 += z_width[ii];
				if ((f = P->range[ii].fill))	/* Using pattern fills */
					GMT_setfill (GMT, f, center);
				else if (intens) {
					nb = (P->is_gray || monochrome) ? 1 : 3;
					tmp = GMT_memory (GMT, NULL, ny*nb, unsigned char);
					for (j = 0, k = i*nb; j < ny*nb; k+=(nx-1)*nb) {
						tmp[j++] = bar[k++];
						tmp[j++] = bar[k++];
						tmp[j++] = bar[k++];
					}
					PSL_plotcolorimage (PSL, x0 + gap, 0.0, z_width[ii] - 2*gap, width, PSL_BL, tmp, 1, ny, depth);
					GMT_free (GMT, tmp);
					PSL_setfill (PSL, GMT->session.no_rgb, center);
				}
				else {
					GMT_rgb_copy (rgb, P->range[ii].rgb_low);
					if (intens) GMT_illuminate (GMT, max_intens[1], rgb);
					if (monochrome) rgb[0] = rgb[1] = rgb[2] = GMT_YIQ (rgb);
					PSL_setfill (PSL, rgb, center);
				}
				if (P->range[ii].fill) {	/* Must undo rotation so patterns remain aligned with original setup */
					PSL_setorigin (PSL, x0 + gap, 0.0, -90.0, PSL_FWD);
					PSL_plotbox (PSL, -width, 0.0, 0.0, x1 - x0 - 2.0 * gap);
					PSL_setorigin (PSL, -(x0 + gap), 0.0, 90.0, PSL_INV);
				}
				else
					PSL_plotbox (PSL, x0 + gap, 0.0, x1 - gap, width);
				x0 = x1;
			}
		}

		if (center && interval_annot) {
			sprintf (text, "%ld - %ld", lrint (floor (P->range[0].z_low)), lrint (ceil (P->range[0].z_high)));
			sprintf (test, "%ld - %ld", lrint (floor (P->range[P->n_colors-1].z_low)), lrint (ceil (P->range[P->n_colors-1].z_high)));
			off = ((MAX ((int)strlen (text), (int)strlen (test)) + 2*ndec) * GMT_DEC_SIZE - 0.4 + 
				((ndec > 0) ? 2*GMT_PER_SIZE : 0.0))
				* GMT->current.setting.font_annot[0].size * GMT->session.u2u[GMT_PT][GMT_INCH];
		}
		else {
			sprintf (text, "%ld", lrint (floor (P->range[0].z_low)));
			sprintf (test, "%ld", lrint (ceil (center ? P->range[P->n_colors-1].z_low : P->range[P->n_colors-1].z_high)));
			off = ((MAX ((int)strlen (text), (int)strlen (test)) + ndec) * GMT_DEC_SIZE +
				((ndec > 0) ? GMT_PER_SIZE : 0.0))
				* GMT->current.setting.font_annot[0].size * GMT->session.u2u[GMT_PT][GMT_INCH];
		}

		annot_off = ((len > 0.0 && !center) ? len : 0.0) + GMT->current.setting.map_annot_offset[0] + off;
		label_off = annot_off + GMT->current.setting.map_label_offset;
		if (use_labels || (flip & 1) || logscl) annot_off -= off;
		y_annot = y_base + dir * annot_off;
		if ((flip & 1) == (flip & 2) / 2) y_label = y_base + dir * label_off;

		PSL_setlinecap (PSL, PSL_BUTT_CAP);	/* Butt cap required for outline of triangle */

		if (extend & (reverse + 1)) {	/* Add color triangle at bottom */
			xp[0] = xp[2] = xleft - gap;	xp[1] = xleft - gap - e_length;
			yp[0] = width - yd;	yp[2] = yd;	yp[1] = 0.5 * width;
			for (i = 0; i < 3; i++) xp[i] += xd;
			xp[1] += xt;
			id = (reverse) ? GMT_FGD : GMT_BGD;
			if ((f = P->patch[id].fill))
				GMT_setfill (GMT, f, false);
			else {
				GMT_rgb_copy (rgb, P->patch[id].rgb);
				if (monochrome) rgb[0] = rgb[1] = rgb[2] = GMT_YIQ (rgb);
				PSL_setfill (PSL, rgb, false);
			}
			PSL_plotpolygon (PSL, xp, yp, 3);
			PSL_plotline (PSL, xp, yp, 3, PSL_MOVE + PSL_STROKE);
			nan_off = e_length - xd;	/* Must make space for the triangle */
		}
		if (extend & 4) {	/* Add NaN rectangle on left side */
			nan_off += e_length;
			xp[0] = xp[1] = xleft - gap;	xp[2] = xp[3] = xp[0] - MAX (width, e_length);
			for (i = 0; i < 4; i++) xp[i] -= nan_off;
			yp[0] = yp[3] = width;	yp[1] = yp[2] = 0.0;
			if ((f = P->patch[GMT_NAN].fill))
				GMT_setfill (GMT, f, true);
			else {
				GMT_rgb_copy (rgb, P->patch[GMT_NAN].rgb);
				if (monochrome) rgb[0] = rgb[1] = rgb[2] = GMT_YIQ (rgb);
				PSL_setfill (PSL, rgb, true);
			}
			PSL_plotpolygon (PSL, xp, yp, 4);
			PSL_plottext (PSL, xp[2] - fabs (GMT->current.setting.map_annot_offset[0]), 0.5 * width, GMT->current.setting.font_annot[0].size, nan_text, -90.0, 10, form);
		}
		if (extend & (2 - reverse)) {	/* Add color triangle at top */
			xp[0] = xp[2] = xright + gap;	xp[1] = xp[0] + e_length;
			yp[0] = width - yd;	yp[2] = yd;	yp[1] = 0.5 * width;
			for (i = 0; i < 3; i++) xp[i] -= xd;
			xp[1] -= xt;
			id = (reverse) ? GMT_BGD : GMT_FGD;
			if ((f = P->patch[id].fill))
				GMT_setfill (GMT, f, false);
			else {
				GMT_rgb_copy (rgb, P->patch[id].rgb);
				if (monochrome) rgb[0] = rgb[1] = rgb[2] = GMT_YIQ (rgb);
				PSL_setfill (PSL, rgb, false);
			}
			PSL_plotpolygon (PSL, xp, yp, 3);
			PSL_plotline (PSL, xp, yp, 3, PSL_MOVE + PSL_STROKE);
		}

		PSL_setlinecap (PSL, PSL_SQUARE_CAP);	/* Square cap required for box of scale bar */

		if (gap == 0.0) {
			if ((flip & 1) || !B_set) PSL_plotsegment (PSL, xleft, 0.0, xleft + length, 0.0);
			if (!(flip & 1) || !B_set) PSL_plotsegment (PSL, xleft, width, xleft + length, width);
			PSL_plotsegment (PSL, xleft, 0.0, xleft, width);
			PSL_plotsegment (PSL, xright, 0.0, xright, width);
		}
		if (B_set) {	/* Used -B. Must kludge by copying x-axis and scaling to y since we must use GMT_xy_axis to draw a y-axis based on x parameters. */
			void (*tmp) (struct GMT_CTRL *, double, double *) = NULL;
			char *custum;
			
			A = &GMT->current.map.frame.axis[GMT_X];
			GMT_auto_frame_interval (GMT, GMT_X, GMT_ANNOT_UPPER);
			if (A->item[GMT_GRID_UPPER].active) {	/* Gridlines work fine without kludging since no annotations involved */
				dx = GMT_get_map_interval (GMT, &A->item[GMT_GRID_UPPER]);
				GMT_setpen (GMT, &GMT->current.setting.map_grid_pen[0]);
				GMT_linearx_grid (GMT, PSL, P->range[0].z_low, P->range[P->n_colors-1].z_high, 0.0, width, dx);
			}
			PSL_setorigin (PSL, 0.0, 0.0, -90.0, PSL_FWD);	/* Rotate back so we can plot y-axis */
			/* Copy x-axis annotation and scale info to y-axis.  We dont need to undo this since GMT_end_module will restore it for us */
			custum = GMT->current.map.frame.axis[GMT_Y].file_custom;	/* Need to remember what this was */
			GMT_memcpy (&GMT->current.map.frame.axis[GMT_Y], &GMT->current.map.frame.axis[GMT_X], 1, struct GMT_PLOT_AXIS);
			double_swap (GMT->current.proj.scale[GMT_X], GMT->current.proj.scale[GMT_Y]);
			double_swap (GMT->current.proj.origin[GMT_X], GMT->current.proj.origin[GMT_Y]);
			uint_swap (GMT->current.proj.xyz_projection[GMT_X], GMT->current.proj.xyz_projection[GMT_Y]);
			tmp = GMT->current.proj.fwd_x; GMT->current.proj.fwd_y = GMT->current.proj.fwd_x; GMT->current.proj.fwd_x = tmp;
			GMT->current.map.frame.axis[GMT_Y].id = GMT_Y;
			for (i = 0; i < 5; i++) GMT->current.map.frame.axis[GMT_Y].item[i].parent = GMT_Y;
			GMT_xy_axis (GMT, -y_base, 0.0, length, start_val, stop_val, &GMT->current.map.frame.axis[GMT_Y], flip & 1, GMT->current.map.frame.side[flip & 1 ? W_SIDE : E_SIDE] & 2);
			PSL_setorigin (PSL, 0.0, 0.0, 90.0, PSL_INV);	/* Rotate back to where we started in this branch */
			GMT->current.map.frame.axis[GMT_Y].file_custom = custum;	/* Restore correct pointer */
		}
		else {
			if (!skip_lines) {	/* First draw gridlines */
				GMT_setpen (GMT, &GMT->current.setting.map_grid_pen[0]);
				x1 = xleft;
				for (i = 0; i < P->n_colors; i++) {
					xx = (reverse) ? xright - x1 : x1;
					PSL_plotsegment (PSL, xx, 0.0, xx, width);
					x1 += z_width[i];
				}
				xx = (reverse) ? xleft : xright;
				PSL_plotsegment (PSL, xx, 0.0, xx, width);
			}

			/* Then annotate and draw tickmarks */

			GMT_setpen (GMT, &GMT->current.setting.map_tick_pen[0]);
			x1 = xleft;
			if (center) x1 += 0.5 * z_width[0];
			for (i = 0; i < P->n_colors; i++) {
				xx = (reverse) ? xright - x1 : x1;
				if (!center) PSL_plotpoint (PSL, xx, y_base, PSL_MOVE);
				if (all || (P->range[i].annot & 1)) {
					if (!center) PSL_plotpoint (PSL, 0.0, dir * len, PSL_DRAW + PSL_STROKE + PSL_REL);
					this_just = justify;
					do_annot = true;
					if (use_labels && P->range[i].label) {
						strncpy (text, P->range[i].label, GMT_LEN256);
						this_just = l_justify;
					}
					else if (center && interval_annot)
						sprintf (text, format, P->range[i].z_low, P->range[i].z_high);
					else if (logscl) {
						p_val = irint (P->range[i].z_low);
						if (doubleAlmostEqualZero (P->range[i].z_low, (double)p_val))
							sprintf (text, "10@+%d@+", p_val);
						else
							do_annot = false;
						this_just = l_justify;
					}
					else
						sprintf (text, format, P->range[i].z_low);
					if (!cpt_auto_fmt) this_just = l_justify;
					if (do_annot) PSL_plottext (PSL, xx, y_annot, GMT->current.setting.font_annot[0].size, text, -90.0, -this_just, form);
				}
				else
					if (!center) PSL_plotpoint (PSL, 0.0, dir * len2, PSL_DRAW + PSL_STROKE + PSL_REL);
				x1 += z_width[i];
			}
			if (!center && !use_labels) {
				xx = (reverse) ? xleft : xright;
				PSL_plotpoint (PSL, xx, y_base, PSL_MOVE);
				i = P->n_colors-1;
				if (all || (P->range[i].annot & 2)) {
					PSL_plotpoint (PSL, 0.0, dir * len, PSL_DRAW + PSL_STROKE + PSL_REL);
					this_just = justify;
					do_annot = true;
					if (logscl) {
						p_val = irint (P->range[i].z_high);
						if (doubleAlmostEqualZero (P->range[i].z_high, (double)p_val))
							sprintf (text, "10@+%d@+", p_val);
						else
							do_annot = false;
						this_just = l_justify;
					}
					else
						sprintf (text, format, P->range[i].z_high);
					if (!cpt_auto_fmt) this_just = l_justify;
					if (do_annot) PSL_plottext (PSL, xx, y_annot, GMT->current.setting.font_annot[0].size, text, -90.0, -this_just, form);
				}
				else
					PSL_plotpoint (PSL, 0.0, dir * len2, PSL_DRAW + PSL_STROKE + PSL_REL);
			}
		}

		if (label[0]) {	/* Add label */
			form = GMT_setfont (GMT, &GMT->current.setting.font_label);
			if (strchr (label, '@') || strchr (label, '(') || !(flip & 4)) { /* Must set text along-side color bar */
				PSL_plottext (PSL, xleft + 0.5 * length, y_label, GMT->current.setting.font_label.size, label, 0.0, Label_justify, form);
			}
			else {	/* Set label text as column (AARRGGHH) */
				int dir = (flip & 2) - 1;
				y_label += 0.5 * dir * GMT->current.setting.font_label.size * GMT->session.u2u[GMT_PT][GMT_INCH];
				size = 0.9 * GMT->current.setting.font_label.size * GMT->session.u2u[GMT_PT][GMT_INCH];
				x0 = 0.5 * (length + ((int)strlen (label) -1) * size);
				text[1] = 0;
				for (i = 0; i < strlen (label); i++) {
					x1 = x0 - i * size;
					text[0] = label[i];
					PSL_plottext (PSL, x1, y_label, GMT->current.setting.font_label.size, text, -90.0, 6, form);
				}
			}
		}
		if (unit[0]) {	/* Add unit label */
			form = GMT_setfont (GMT, &GMT->current.setting.font_annot[0]);
			PSL_plottext (PSL, xright + GMT->current.setting.map_annot_offset[0] + e_length, 0.5 * width, GMT->current.setting.font_annot[0].size, unit, -90.0, 2, form);
		}
		PSL_setorigin (PSL, -width, 0.0, -90.0, PSL_INV);
	}
	if (use_image || intens) GMT_free (GMT, bar);
	/* Reset back to original line cap and join */
	PSL_setlinecap (PSL, cap);
	PSL_setlinejoin (PSL, join);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_psscale_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_psscale (void *V_API, int mode, void *args)
{	/* High-level function that implements the psscale task */
	int error = 0;
	unsigned int i;

	char text[GMT_LEN256] = {""};

	double max_intens[2], dz, *z_width = NULL;
	double start_val, stop_val, wesn[4];

	struct PSSCALE_CTRL *Ctrl = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_DATASET *D = NULL;

	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_psscale_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_psscale_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_psscale_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	/* Overrule GMT settings of MAP_FRAME_AXES. Use WESN */
	GMT->current.map.frame.side[S_SIDE] = GMT->current.map.frame.side[E_SIDE] = GMT->current.map.frame.side[N_SIDE] = GMT->current.map.frame.side[W_SIDE] = 3;
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_psscale_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_psscale_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the psscale main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input CPT table\n");
	if ((P = GMT_Read_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
		Return (API->error);
	}
	if (Ctrl->G.active) P = GMT_truncate_cpt (GMT, P, Ctrl->G.z_low, Ctrl->G.z_high);	/* Possibly truncate the CPT */

	if (P->categorical) {
		Ctrl->L.active = Ctrl->L.interval = true;
		GMT_Report (API, GMT_MSG_VERBOSE, "CPT is for categorical data.\n");
	}
	
	GMT_Report (API, GMT_MSG_VERBOSE, "  CPT range from %g to %g\n", P->range[0].z_low, P->range[P->n_colors-1].z_high);

	if (Ctrl->Q.active) {	/* Take log of all z values */
		for (i = 0; i < P->n_colors; i++) {
			if (P->range[i].z_low <= 0.0 || P->range[i].z_high <= 0.0) {
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -Q option: All z-values must be positive for logarithmic scale\n");
				Return (EXIT_FAILURE);
			}
			P->range[i].z_low = d_log10 (GMT, P->range[i].z_low);
			P->range[i].z_high = d_log10 (GMT, P->range[i].z_high);
			P->range[i].i_dz = 1.0 / (P->range[i].z_high - P->range[i].z_low);
		}
	}
	
	if (Ctrl->E.mode && Ctrl->E.length == 0.0) Ctrl->E.length = Ctrl->D.width * 0.5;
	max_intens[0] = Ctrl->I.min;
	max_intens[1] = Ctrl->I.max;

	if (Ctrl->Z.active) {
		if (GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->Z.file, &D)) {
			Return (API->error);
		}
		z_width = D->table[0]->segment[0]->coord[GMT_X];
		if (D->table[0]->segment[0]->n_rows < (uint64_t)P->n_colors) {
			GMT_Report (API, GMT_MSG_NORMAL, "-Z file %s has fewer slices than -C file %s!\n", Ctrl->Z.file, Ctrl->C.file);
			Return (EXIT_FAILURE);
		}
	}
	else if (Ctrl->L.active) {
		z_width = GMT_memory (GMT, NULL, P->n_colors, double);
		dz = fabs (Ctrl->D.length) / P->n_colors;
		for (i = 0; i < P->n_colors; i++) z_width[i] = dz;
	}
	else {
		z_width = GMT_memory (GMT, NULL, P->n_colors, double);
		for (i = 0; i < P->n_colors; i++) z_width[i] = fabs (Ctrl->D.length) * (P->range[i].z_high - P->range[i].z_low) / (P->range[P->n_colors-1].z_high - P->range[0].z_low);
	}

	/* Because psscale uses -D to position things we need to make some
	 * changes so that BoundingBox and others are set ~correctly */

	if (Ctrl->Q.active && GMT->current.map.frame.draw) {
		sprintf (text, "X%gil/%gi", Ctrl->D.length, Ctrl->D.width);
		start_val = pow (10.0, P->range[0].z_low);
		stop_val  = pow (10.0, P->range[P->n_colors-1].z_high);
	}
	else {
		sprintf (text, "X%gi/%gi", Ctrl->D.length, Ctrl->D.width);
		start_val = P->range[0].z_low;
		stop_val  = P->range[P->n_colors-1].z_high;
	}

	GMT_memset (wesn, 4, double);
	if (!(GMT->common.R.active && GMT->common.J.active)) {	/* When no projection specified, use fake linear projection */
		GMT->common.R.active = true;
		GMT->common.J.active = false;
		GMT_parse_common_options (GMT, "J", 'J', text);
		wesn[XLO] = start_val;	wesn[XHI] = stop_val;	wesn[YHI] = Ctrl->D.width;
		GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");
		PSL = GMT_plotinit (GMT, options);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	}
	else {	/* First use current projection, project, then use fake projection */
		if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);
		PSL = GMT_plotinit (GMT, options);
		GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		GMT->common.J.active = false;
		GMT_parse_common_options (GMT, "J", 'J', text);
		wesn[XLO] = start_val;	wesn[XHI] = stop_val;	wesn[YHI] = Ctrl->D.width;
		GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");
	}

	/* We must do any origin translation manually in psscale */

	if (Ctrl->D.horizontal) {
		Ctrl->D.x -= 0.5 * fabs (Ctrl->D.length);
		Ctrl->D.y -= Ctrl->D.width;
		GMT->current.map.frame.side[E_SIDE] = GMT->current.map.frame.side[W_SIDE] = 0;
	}
	else {
		Ctrl->D.y -= 0.5 * fabs (Ctrl->D.length);
		GMT->current.map.frame.side[S_SIDE] = GMT->current.map.frame.side[N_SIDE] = 0;
		double_swap (GMT->current.proj.z_project.xmin, GMT->current.proj.z_project.ymin);
		double_swap (GMT->current.proj.z_project.xmax, GMT->current.proj.z_project.ymax);
	}
	PSL_setorigin (PSL, Ctrl->D.x, Ctrl->D.y, 0.0, PSL_FWD);
	
	gmt_draw_colorbar (GMT, PSL, P, Ctrl->D.length, Ctrl->D.width, z_width, Ctrl->N.dpi, Ctrl->N.mode, Ctrl->A.mode, 
		GMT->current.map.frame.draw, Ctrl->L.active, Ctrl->D.horizontal, Ctrl->Q.active, Ctrl->I.active,
		max_intens, Ctrl->S.active, Ctrl->E.mode, Ctrl->E.length, Ctrl->E.text, Ctrl->L.spacing,
		Ctrl->L.interval, Ctrl->M.active, Ctrl->T);
	PSL_setorigin (PSL, -Ctrl->D.x, -Ctrl->D.y, 0.0, PSL_FWD);
	GMT_plane_perspective (GMT, -1, 0.0);

	GMT_plotend (GMT);

	if (!Ctrl->Z.active) GMT_free (GMT, z_width);

	Return (EXIT_SUCCESS);
}
