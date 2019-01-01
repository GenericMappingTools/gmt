/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2019 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: grdcontour reads a 2-D grid file and contours it,
 * controlled by a variety of options.
 *
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"grdcontour"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Make contour map using a grid"
#define THIS_MODULE_KEYS	"<G{,AT)=t,CC(,DDD,>X},G?(=1"
#define THIS_MODULE_NEEDS	"gJ"
#define THIS_MODULE_OPTIONS "-BJKOPRUVXYbdfhptxy" GMT_OPT("EMmc")

/* Control structure for grdcontour */

struct GRDCONTOUR_CTRL {
	struct GRDCONTOUR_In {
		bool active;
		char *file;
	} In;
	struct GMT_CONTOUR contour;
	struct GRDCONTOUR_A {	/* -A[-][labelinfo] */
		bool active;
		unsigned int mode;	/* 1 turns off all labels */
		double interval;
		double single_cont;
	} A;
	struct GRDCONTOUR_C {	/* -C<cont_int> */
		bool active;
		bool cpt;
		char *file;
		double interval;
		double single_cont;
	} C;
	struct GRDCONTOUR_D {	/* -D<dumpfile> */
		bool active;
		char *file;
	} D;
	struct GRDCONTOUR_F {	/* -F<way> */
		bool active;
		int value;
	} F;
	struct GRDCONTOUR_G {	/* -G[d|f|n|l|L|x|X]<params> */
		bool active;
	} G;
	struct GRDCONTOUR_L {	/* -L<Low/high> */
		bool active;
		double low, high;
	} L;
	struct GRDCONTOUR_Q {	/* -Q<cut> */
		bool active;
		unsigned int min;
	} Q;
	struct GRDCONTOUR_S {	/* -S<smooth> */
		bool active;
		unsigned int value;
	} S;
	struct GRDCONTOUR_T {	/* -T[+|-][+d<gap>[c|i|p][/<length>[c|i|p]]][+lLH|"low,high"] */
		bool active;
		bool label;
		bool low, high;	/* true to tick low and high locals */
		double dim[2];	/* spacing, length */
		char *txt[2];	/* Low and high label [-+] */
	} T;
	struct GRDCONTOUR_W {	/* -W[a|c]<pen>[+c[l|f]] */
		bool active;
		bool cpt_effect;
		unsigned int cptmode;	/* Apply to both a&c */
		struct GMT_PEN pen[2];
	} W;
	struct GRDCONTOUR_Z {	/* -Z[<fact>[/shift>]][p] */
		bool active;
		bool periodic;
		double scale, offset;
	} Z;
};

#define GRDCONTOUR_MIN_LENGTH 0.01	/* Contours shorter than this are skipped */
#define TICKED_SPACING	15.0		/* Spacing between ticked contour ticks (in points) */
#define TICKED_LENGTH	3.0		/* Length of ticked contour ticks (in points) */

enum grdcontour_contour_type {cont_is_not_closed = 0,	/* Not a closed contour of any sort */
	cont_is_closed = 1,				/* Clear case of closed contour away from boundaries */
	cont_is_closed_straddles_west = -2,		/* Part of a closed contour that exits west for periodic boundary */
	cont_is_closed_straddles_east = +2,		/* Part of a closed contour that exits east for periodic boundary */
	cont_is_closed_around_south_pole = -3,		/* Closed contour in southern hemisphere that encloses the south pole */
	cont_is_closed_around_north_pole = +3,		/* Closed contour in northern hemisphere that encloses the north pole */
	cont_is_closed_straddles_equator_south = -4,	/* Closed contour crossing equator that encloses the south pole */
	cont_is_closed_straddles_equator_north = +4};	/* Closed contour crossing equator that encloses the north pole */

struct SAVE {
	double *x, *y;
	double *xp, *yp;
	double cval;
	double xlabel, ylabel;
	double y_min, y_max;
	int n, np;
	struct GMT_PEN pen;
	struct GMT_FONT font;
	int do_it, high;
	enum grdcontour_contour_type kind;
	char label[GMT_LEN64];
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GRDCONTOUR_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GRDCONTOUR_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	gmt_contlabel_init (GMT, &C->contour, 1);
	C->A.single_cont = GMT->session.d_NaN;
	C->C.single_cont = GMT->session.d_NaN;
	C->L.low = -DBL_MAX;
	C->L.high = DBL_MAX;
	C->T.dim[GMT_X] = TICKED_SPACING * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 14p */
	C->T.dim[GMT_Y] = TICKED_LENGTH  * GMT->session.u2u[GMT_PT][GMT_INCH];	/* 3p */
	C->W.pen[0] = C->W.pen[1] = GMT->current.setting.map_default_pen;
	C->W.pen[1].width *= 3.0;
	C->Z.scale = 1.0;

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GRDCONTOUR_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->D.file);
	gmt_M_str_free (C->T.txt[0]);
	gmt_M_str_free (C->T.txt[1]);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	struct GMT_PEN P;

	/* This displays the grdcontour synopsis and optionally full usage information */

	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: grdcontour <grid> -C[+]<cont_int>|<cpt> [-A[-|[+]<annot_int>][<labelinfo>] [%s ]\n", GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-D<template>] [-F[l|r]] [%s] [%s] [-K]\n", GMT_J_OPT, GMT_Jz_OPT, GMT_CONTG);
	GMT_Message (API, GMT_TIME_NONE, "\t[-L<low>/<high>] [-O] [-P] [-Q<cut>] [%s]\n", GMT_Rgeoz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-S<smooth>] [%s]\n", GMT_CONTT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [-W[a|c]<pen>[+c[l|f]]]\n\t[%s] [%s] [-Z[+s<fact>][+o<shift>][+p]\n",
	                                 GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n", GMT_bo_OPT, GMT_do_OPT, GMT_ho_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\n", GMT_p_OPT, GMT_t_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<grid> is the grid file to be contoured.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Contours to be drawn can be specified in one of three ways:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     1. Fixed contour interval, or a single contour if prepended with a + sign.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     2. File with contour levels in col 1 and C(ont) or A(nnot) in col 2\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        [and optionally an individual annotation angle in col 3].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     3. Name of a CPT.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -T is used, only contours with upper case C or A is ticked\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     [CPT contours are set to C unless the CPT flags are set;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use -A to force all to become A].\n");
	GMT_Option (API, "J-Z");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Annotation label settings [Default is no annotated contours].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give annotation interval OR - to disable all contour annotations\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   implied by the information provided in -C.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively prepend + to annotation interval to plot that as a single contour.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <labelinfo> controls the specifics of the labels.  Choose from:\n");
	gmt_label_syntax (API->GMT, 5, 0);
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
	GMT_Message (API, GMT_TIME_NONE, "\t-F Force dumped contours to be oriented so that the higher z-values\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   are to the left (-Fl [Default]) or right (-Fr) as we move along\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the contour lines [Default is not oriented].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Control placement of labels along contours.  Choose from:\n");
	gmt_cont_syntax (API->GMT, 3, 0);
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Only contour inside this range.\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Do not draw closed contours with less than <cut> points [Draw all contours].\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is extent of grid].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Will Smooth contours by splining and resampling at\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   approximately gridsize/<smooth> intervals.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Will embellish innermost, closed contours with ticks pointing in\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the downward direction.  User may specify to tick only highs.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (-T+) or lows (-T-) [-T implies both extrema].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +d<spacing>[/<ticklength>] (with units) to change defaults [%gp/%gp].\n",
	                                 TICKED_SPACING, TICKED_LENGTH);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +lXY (or +l\"low,high\") to place X and Y (or low and high) at the center\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of local lows and highs.  If no labels are given we default to - and +.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If two characters are passed (e.g., +lLH) we use the as L and H.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For string labels, simply give two strings separated by a comma (e.g., +llo,hi).\n");
	GMT_Option (API, "U,V");
	gmt_pen_syntax (API->GMT, 'W', "Set pen attributes. Append a<pen> for annotated or (default) c<pen> for regular contours", 0);
	GMT_Message (API, GMT_TIME_NONE, "\t   The default pen settings are\n");
	P = API->GMT->current.setting.map_default_pen;
	GMT_Message (API, GMT_TIME_NONE, "\t   Contour pen:  %s.\n", gmt_putpen (API->GMT, &P));
	P.width *= 3.0;
	GMT_Message (API, GMT_TIME_NONE, "\t   Annotate pen: %s.\n", gmt_putpen (API->GMT, &P));
	GMT_Message (API, GMT_TIME_NONE, "\t     +c Controls how pens and fills are affected if a CPT is specified via -C:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append l to let pen colors follow the CPT setting (requires -C).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Append f to let fill/font colors follow the CPT setting.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t        Default is both effects.\n");
	GMT_Option (API, "X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Subtract <shift> (via +o<shift> [0]) and multiply data by <fact> (via +s<fact> [1])\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   before contouring. Append + for z-data that are periodic in 360 (i.e., phase data).\n");
	GMT_Option (API, "bo3,do,f,h,p,t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL unsigned int grdcontour_old_T_parser (struct GMT_CTRL *GMT, char *arg, struct GRDCONTOUR_CTRL *Ctrl) {
	/* The backwards compatible parser for old-style -T option: */
	/* -T[+|-][<gap>[c|i|p]/<length>[c|i|p]][:LH] */
	int n, j;
	unsigned int n_errors = 0;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""};
	if (strchr (arg, '/')) {	/* Gave gap/length */
		n = sscanf (arg, "%[^/]/%[^:]", txt_a, txt_b);
		if (n == 2) {
			Ctrl->T.dim[GMT_X] = gmt_M_to_inch (GMT, txt_a);
			Ctrl->T.dim[GMT_Y] = gmt_M_to_inch (GMT, txt_b);
		}
	}
	for (j = 0; arg[j] && arg[j] != ':'; j++);
	if (arg[j] == ':') Ctrl->T.label = true, j++;
	if (arg[j]) {	/* Override high/low markers */
		if (strlen (&(arg[j])) == 2) {	/* Standard :LH syntax */
			txt_a[0] = arg[j++];	txt_a[1] = '\0';
			txt_b[0] = arg[j++];	txt_b[1] = '\0';
		}
		else if (strchr (&(arg[j]), ',')) {	/* Found :<labellow>,<labelhigh> */
			sscanf (&(arg[j]), "%[^,],%s", txt_a, txt_b);
		}
		else {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL,
			            "Syntax error -T option: Give low and high labels either as -:LH or -:<low>,<high>.\n");
			Ctrl->T.label = false;
			n_errors++;
		}
		if (Ctrl->T.label) {	/* Replace defaults */
			Ctrl->T.txt[0] = strdup (txt_a);
			Ctrl->T.txt[1] = strdup (txt_b);
		}
	}
	return (n_errors);
}

GMT_LOCAL unsigned int parse_Z_opt (struct GMT_CTRL *GMT, char *txt, struct GRDCONTOUR_CTRL *Ctrl) {
	/* Parse the -Z option: -Z[+s<scale>][+o<offset>][+p] */
	unsigned int uerr = 0;
	if (!txt || txt[0] == '\0') {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL,
    		"Syntax error -Z option: No arguments given\n");
		return (GMT_PARSE_ERROR);
	}
	if (strstr (txt, "+s") || strstr (txt, "+o") || strstr (txt, "+p")) {
		char p[GMT_LEN64] = {""};
		unsigned int pos = 0;
		while (gmt_getmodopt (GMT, 'Z', txt, "sop", &pos, p, &uerr) && uerr == 0) {
			switch (p[0]) {
				case 's':	Ctrl->Z.scale = atof (&p[1]);	break;
				case 'o':	Ctrl->Z.offset = atof (&p[1]);	break;
				case 'p':	Ctrl->Z.periodic = true;		break;
				default: 	/* These are caught in gmt_getmodopt so break is just for Coverity */
					break;
			}
		}
	}
	else {	/* Old syntax */
		if (txt[0] && txt[0] != 'p') sscanf (txt, "%lf/%lf", &Ctrl->Z.scale, &Ctrl->Z.offset);
		Ctrl->Z.periodic = (txt[strlen(txt)-1] == 'p');	/* Phase data */
	}
	return (uerr ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GRDCONTOUR_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to grdcontour and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, id, reset = 0;
	int j, k, n;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, string[GMT_LEN256] = {""}, *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input file (only one is accepted) */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_GRID)) != 0)
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Annotation control */
				Ctrl->A.active = true;
				if (gmt_contlabel_specs (GMT, opt->arg, &Ctrl->contour)) {
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -A option: Expected\n\t-A[-|[+]<aint>][+a<angle>|n|p[u|d]][+c<dx>[/<dy>]][+d][+e][+f<font>][+g<fill>][+j<just>][+l<label>][+n|N<dx>[/<dy>]][+o][+p<pen>][+r<min_rc>][+t[<file>]][+u<unit>][+v][+w<width>][+=<prefix>]\n");
					n_errors ++;
				}
				else if (opt->arg[0] == '-')
					Ctrl->A.mode = 1;	/* Turn off all labels */
				else if (opt->arg[0] == '+' && (isdigit(opt->arg[1]) || strchr ("-+.", opt->arg[1]))) {
					Ctrl->A.single_cont = atof (&opt->arg[1]);
					Ctrl->contour.annot = true;
				}
				else {
					Ctrl->A.interval = atof (opt->arg);
					Ctrl->contour.annot = true;
				}
				break;
			case 'C':	/* Contour interval/cpt */
				Ctrl->C.active = true;
				if (gmt_M_file_is_memory (opt->arg)) {	/* Passed a memory reference from a module */
					Ctrl->C.interval = 1.0;
					Ctrl->C.cpt = true;
					gmt_M_str_free (Ctrl->C.file);
					Ctrl->C.file = strdup (opt->arg);
				}
				else if (!gmt_access (GMT, opt->arg, R_OK)) {	/* Gave a readable file */
					Ctrl->C.interval = 1.0;
					Ctrl->C.cpt = (!strncmp (&opt->arg[strlen(opt->arg)-4], ".cpt", 4U)) ? true : false;
					gmt_M_str_free (Ctrl->C.file);
					Ctrl->C.file = strdup (opt->arg);
				}
				else if (opt->arg[0] == '+' && (isdigit(opt->arg[1]) || strchr ("-+.", opt->arg[1])))
					Ctrl->C.single_cont = atof (&opt->arg[1]);
				else
					Ctrl->C.interval = atof (opt->arg);
				break;
			case 'D':	/* Dump file name */
				Ctrl->D.active = true;
				if (opt->arg[0]) Ctrl->D.file = strdup (opt->arg);
				break;
			case 'F':	/* Orient dump contours */
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case '\0': case 'l':
						Ctrl->F.value = -1;
						break;
					case 'r':
						Ctrl->F.value = +1;
						break;
					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Syntax error: Expected -F[l|r]\n");
						break;
				}
				break;
			case 'G':	/* Contour labeling position control */
				Ctrl->G.active = true;
				n_errors += gmt_contlabel_info (GMT, 'G', opt->arg, &Ctrl->contour);
				break;
			case 'M': case 'm':	/* GMT4 Old-options no longer required - quietly skipped under compat */
				if (!gmt_M_compat_check (GMT, 4)) n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'L':	/* Limits on range */
				Ctrl->L.active = true;
				sscanf (opt->arg, "%lf/%lf", &Ctrl->L.low, &Ctrl->L.high);
				break;
			case 'Q':	/* Skip small closed contours */
				Ctrl->Q.active = true;
				n = atoi (opt->arg);
				n_errors += gmt_M_check_condition (GMT, n < 0, "Syntax error -Q option: Value must be >= 0\n");
				Ctrl->Q.min = n;
				break;
			case 'S':	/* Smoothing of contours */
				Ctrl->S.active = true;
				j = atoi (opt->arg);
				n_errors += gmt_M_check_condition (GMT, j < 0, "Syntax error -S option: Smooth_factor must be > 0\n");
				Ctrl->S.value = j;
				break;
			case 'T':	/* Ticking of innermost closed contours */
				Ctrl->T.active = Ctrl->T.high = Ctrl->T.low = true;	/* Default if just -T is given */
				if (opt->arg[0]) {	/* But here we gave more options */
					if (opt->arg[0] == '+' && !strchr ("dl", opt->arg[1]))			/* Only tick local highs */
						Ctrl->T.low = false, j = 1;
					else if (opt->arg[0] == '-')		/* Only tick local lows */
						Ctrl->T.high = false, j = 1;
					else
						j = 0;
					if (strstr (opt->arg, "+d") || strstr (opt->arg, "+l")) {	/* New parser */
						if (gmt_get_modifier (opt->arg, 'd', string))
							if ((n = gmt_get_pair (GMT, string, GMT_PAIR_DIM_NODUP, Ctrl->T.dim)) < 1) n_errors++;
						if (gmt_get_modifier (opt->arg, 'l', string)) {	/* Want to label innermost contours */
							Ctrl->T.label = true;
							if (string[0] == 0)
								;	/* Use default labels */
							else if (strlen (string) == 2) {	/* Standard +lLH syntax */
								char A[2] = {0, 0};
								A[0] = string[0];	Ctrl->T.txt[0] = strdup (A);
								A[0] = string[1];	Ctrl->T.txt[1] = strdup (A);
							}
							else if (strchr (string, ',') && (n = sscanf (string, "%[^,],%s", txt_a, txt_b)) == 2) {	/* Found :<labellow>,<labelhigh> */
								Ctrl->T.txt[0] = strdup (txt_a);
								Ctrl->T.txt[1] = strdup (txt_b);
							}
							else {
								GMT_Report (API, GMT_MSG_NORMAL,
								            "Syntax error -T option: Give low and high labels either as +lLH or +l<low>,<high>.\n");
								n_errors++;
							}
						}
					}
					else {
						if (gmt_M_compat_check (API->GMT, 4))  {
							GMT_Report (API, GMT_MSG_COMPAT, "Warning: Your format for -T is deprecated (but accepted); use -T[+|-][+d<tick_gap>[%s][/<tick_length>[%s]]][+lLH] instead\n",
								GMT_DIM_UNITS_DISPLAY, GMT_DIM_UNITS_DISPLAY);
							n_errors += grdcontour_old_T_parser (GMT, &opt->arg[j], Ctrl);
						}
						else {
							GMT_Report (API, GMT_MSG_COMPAT, "Syntax error -T option: Your format for -T is deprecated; use -T[+|-][+d<tick_gap>[%s][/<tick_length>[%s]]][+lLH] instead\n",
								GMT_DIM_UNITS_DISPLAY, GMT_DIM_UNITS_DISPLAY);
							n_errors++;
						}
					}
					n_errors += gmt_M_check_condition (GMT, Ctrl->T.dim[GMT_X] <= 0.0 || Ctrl->T.dim[GMT_Y] == 0.0,
					                "Syntax error -T option: Expected\n\t-T[+|-][+d<tick_gap>[%s][/<tick_length>[%s]]][+lLH], <tick_gap> must be > 0\n",
					                	GMT_DIM_UNITS_DISPLAY, GMT_DIM_UNITS_DISPLAY);
				}
				break;
			case 'W':	/* Pen settings */
				Ctrl->W.active = true;
				k = reset = 0;
				if (opt->arg[0] == '-' || (opt->arg[0] == '+' && opt->arg[1] != 'c')) {	/* Definitively old-style args */
					if (opt->arg[k] == '+') Ctrl->W.cptmode = 1, k++;
					if (opt->arg[k] == '-') Ctrl->W.cptmode = 3, k++;
					j = (opt->arg[k] == 'a' || opt->arg[k] == 'c') ? k+1 : k;
				}
				else {
					if ((c = strstr (opt->arg, "+c"))) {	/* Gave +c modifier - apply to both pens */
						switch (c[2]) {
							case 'l': Ctrl->W.cptmode = 1; break;
							case 'f': Ctrl->W.cptmode = 2; break;
							default:  Ctrl->W.cptmode = 3; break;
						}
						if (!strncmp (&c[2], "lf", 2U) || !strncmp (&c[2], "fl", 2U))	/* Catch any odd +clf or +cfl modifiers */
							Ctrl->W.cptmode = 3;
						c[0] = 0;	/* Temporarily chop of */
						reset = 1;
					}
					j = (opt->arg[0] == 'a' || opt->arg[0] == 'c') ? k+1 : k;
				}
				if (j == k && opt->arg[j]) {	/* Set both */
					if (gmt_getpen (GMT, &opt->arg[j], &Ctrl->W.pen[0])) {
						gmt_pen_syntax (GMT, 'W', " ", 0);
						n_errors++;
					}
					else
						Ctrl->W.pen[1] = Ctrl->W.pen[0];
				}
				else if (opt->arg[j]) {	/* Gave a or c.  Because the user may say -Wcyan we must prevent this from being seen as -Wc and color yan! */
					/* Get the argument following a or c and up to first comma, slash (or to the end) */
					n = k+1;
					while (!(opt->arg[n] == ',' || opt->arg[n] == '/' || opt->arg[n] == '\0')) n++;
					strncpy (txt_a, &opt->arg[k], (size_t)(n-k));	txt_a[n-k] = '\0';
					if (gmt_colorname2index (GMT, txt_a) >= 0) j = k;	/* Found a colorname; wind j back by 1 */
					id = (opt->arg[k] == 'a') ? 1 : 0;
					if (gmt_getpen (GMT, &opt->arg[j], &Ctrl->W.pen[id])) {
						gmt_pen_syntax (GMT, 'W', " ", 0);
						n_errors++;
					}
					if (j == k) Ctrl->W.pen[1] = Ctrl->W.pen[0];	/* Must copy since it was not -Wc nor -Wa after all */
				}
				if (reset) c[0] = '+';	/* Restore */
				if (Ctrl->W.cptmode) Ctrl->W.cpt_effect = true;
				break;
			case 'Z':	/* For scaling or phase data */
				Ctrl->Z.active = true;
				n_errors += parse_Z_opt (GMT, opt->arg, Ctrl);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (Ctrl->A.interval > 0.0 && (!Ctrl->C.file && Ctrl->C.interval == 0.0)) Ctrl->C.interval = Ctrl->A.interval;

	n_errors += gmt_M_check_condition (GMT, n_files != 1, "Syntax error: Must specify a single grid file\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active && !Ctrl->D.active,
	                                 "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->C.file && Ctrl->C.interval <= 0.0 &&
			gmt_M_is_dnan (Ctrl->C.single_cont) && gmt_M_is_dnan (Ctrl->A.single_cont),
	                     "Syntax error -C option: Must specify contour interval, file name with levels, or CPT\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.low > Ctrl->L.high, "Syntax error -L option: lower limit > upper!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && !Ctrl->D.active, "Syntax error -F option: Must also specify -D\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->contour.label_dist_spacing <= 0.0 || Ctrl->contour.half_width <= 0,
	                                 "Syntax error -G option: Correct syntax:\n\t-G<annot_dist>/<npoints>, both values must be > 0\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.scale == 0.0, "Syntax error -Z option: factor must be nonzero\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.cptmode && !Ctrl->C.cpt,
	                                 "Syntax error -W option: Modifier +c only valid if -C sets a CPT\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

/* Three sub functions used by GMT_grdcontour */

GMT_LOCAL void grd_sort_and_plot_ticks (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, struct SAVE *save, size_t n, struct GMT_GRID *G, double tick_gap, double tick_length, bool tick_low, bool tick_high, bool tick_label, char *in_lbl[], unsigned int mode, struct GMT_TEXTSET *T) {
	/* Labeling and ticking of inner-most contours cannot happen until all contours are found and we can determine
	   which are the innermost ones. Here, all the n candidate contours are passed via the save array.
	   We need to do several types of testing here:
	   1) First we exclude closed contours around a single node as too small.
	   2) Next, we mark closed contours with other contours inside them as not "innermost"
	   3) We then determine if the remaining closed polygons contain highs or lows.

	   Note on mode bitflags: mode = 1 (plot only), 2 (save labels only), 3 (both)
	*/
	int np, j, k, inside, col, row, stop, n_ticks, way, form;
	uint64_t ij;
	size_t pol, pol2;
	bool done, match, found;
	char *lbl[2], *def[2] = {"-", "+"};
	double add, dx, dy, x_back, y_back, x_front, y_front, x_end, y_end, x_lbl, y_lbl;
	double xmin, xmax, ymin, ymax, inc, dist, a, this_lon, this_lat, sa, ca;
	double *s = NULL, *xp = NULL, *yp = NULL;
	double da, db, dc, dd;

	lbl[0] = (in_lbl[0]) ? in_lbl[0] : def[0];
	lbl[1] = (in_lbl[1]) ? in_lbl[1] : def[1];
	/* The x/y coordinates in SAVE in original cooordinates */

	for (pol = 0; pol < n; pol++) {	/* Set y min/max for polar caps */
		if (abs (save[pol].kind) < 3) continue;	/* Skip all but polar caps */
		save[pol].y_min = save[pol].y_max = save[pol].y[0];
		for (j = 1; j < save[pol].n; j++) {
			if (save[pol].y[j] < save[pol].y_min) save[pol].y_min = save[pol].y[j];
			if (save[pol].y[j] > save[pol].y_max) save[pol].y_max = save[pol].y[j];
		}
	}

	for (pol = 0; pol < n; pol++) {	/* Mark polygons that have other polygons inside them */
		np = save[pol].n;	/* Length of this polygon */
		for (pol2 = 0; save[pol].do_it && pol2 < n; pol2++) {
			if (pol == pol2) continue;		/* Cannot be inside itself */
			if (!save[pol2].do_it) continue;	/* No point checking contours that have already failed */
			if (abs (save[pol].kind) == 4) {	/* These are closed "polar caps" that crosses equator (in lon/lat) */
				save[pol].do_it = false;	/* This may be improved in the future */
				continue;
			}
			if (abs (save[pol].kind) != 3) {	/* Not a polar cap so we can call gmt_non_zero_winding */
				col = save[pol2].n / 2;	/* Pick the half-point for testing */
				inside = gmt_non_zero_winding (GMT, save[pol2].x[col], save[pol2].y[col], save[pol].x, save[pol].y, np);
				if (inside == 2) save[pol].do_it = false;	/* Not innermost so mark it for exclusion */
			}
			if (abs (save[pol2].kind) == 3) {	/* Polar caps needs a different test */
				if (abs (save[pol].kind) == 3) {	/* Both are caps */
					if (save[pol].kind != save[pol2].kind) continue;	/* One is S and one is N cap as far as we can tell, so we skip */
					/* Crude test to determine if one is closer to the pole than the other; if so exclude the far one */
					if ((save[pol2].kind == 3 && save[pol2].y_min > save[pol].y_min) || (save[pol2].kind == cont_is_closed_around_south_pole && save[pol2].y_min < save[pol].y_min)) save[pol].do_it = false;
				}
			}
		}
	}
	for (pol = 0; pol < n; pol++) if (abs (save[pol].kind) == 2) save[pol].n--;	/* Chop off the extra duplicate point for split periodic contours */

	/* Must make sure that for split periodic contour that if one fails to be innermost then both should fail */

	for (pol = 0; pol < n; pol++) {
		if (abs (save[pol].kind) != 2) continue;	/* Not a split polygon */
		for (pol2 = 0, found = false; !found && pol2 < n; pol2++) {
			if (pol == pol2) continue;
			if (abs (save[pol2].kind) != 2) continue;	/* Not a split polygon */
			/* The following uses 10^-7 as limit, hence the 0.1 */
			da = save[pol].y[0] - save[pol2].y[0]; db = save[pol].y[save[pol].n-1] - save[pol2].y[save[pol2].n-1];
			dc = save[pol].y[0] - save[pol2].y[save[pol2].n-1];	dd = save[pol].y[save[pol].n-1] - save[pol2].y[0];
			match = ((gmt_M_is_zero (0.1*da) && gmt_M_is_zero (0.1*db)) || (gmt_M_is_zero (0.1*dc) && gmt_M_is_zero (0.1*dd)));
			if (!match) continue;
			if ((save[pol].do_it + save[pol2].do_it) == 1) save[pol].do_it = save[pol2].do_it = false;
			found = true;
		}
		if (!found) save[pol].do_it = false;	/* Probably was not a split closed contour */
	}

	/* Here, only the polygons that are innermost (containing the local max/min, will have do_it = true */

	PSL_comment (PSL, "Start Embellishment of innermost contours\n");
	for (pol = 0; pol < n; pol++) {
		if (!save[pol].do_it) continue;
		np = save[pol].n;

		/* Need to determine if this is a local high or low */

		if (abs (save[pol].kind) == 2) {	/* Closed contour split across a periodic boundary */
			/* Determine row, col for a point ~mid-way along the vertical periodic boundary */
			col = (save[pol].kind == cont_is_closed_straddles_west) ? 0 : G->header->n_columns - 1;
			row = (int)gmt_M_grd_y_to_row (GMT, save[pol].y[0], G->header);		/* Get start j-row */
			row += (int)gmt_M_grd_y_to_row (GMT, save[pol].y[np-1], G->header);	/* Get stop j-row */
			row /= 2;
		}
		else if (abs (save[pol].kind) >= 3) {	/* Polar cap, pick point at midpoint along top or bottom boundary */
			col = G->header->n_columns / 2;
			row = (save[pol].kind < 0) ? G->header->n_rows - 1 : 0;
		}
		else {
			/* Loop around the contour and get min/max original x,y (longitude, latitude) coordinates */

			xmin = xmax = save[pol].x[0];	ymin = ymax = save[pol].y[0];
			for (j = 1; j < np; j++) {
				xmin = MIN (xmin, save[pol].x[j]);
				xmax = MAX (xmax, save[pol].x[j]);
				ymin = MIN (ymin, save[pol].y[j]);
				ymax = MAX (ymax, save[pol].y[j]);
			}

			/* Pick the mid-latitude and march along that line from east to west */

			this_lat = 0.5 * (ymax + ymin);	/* Mid latitude, probably not exactly on a y-node */
			row = (int)MIN (G->header->n_rows - 1, gmt_M_grd_y_to_row (GMT, this_lat, G->header));	/* Get closest j-row */
			this_lat = gmt_M_grd_row_to_y (GMT, row, G->header);	/* Get its matching latitude */
			col  = (int)gmt_M_grd_x_to_col (GMT, xmin, G->header);		/* Westernmost point */
			stop = (int)gmt_M_grd_x_to_col (GMT, xmax, G->header);		/* Eastermost point */
			done = false;
			while (!done && col <= stop) {
				this_lon = gmt_M_grd_col_to_x (GMT, col, G->header);	/* Current longitude */
				inside = gmt_non_zero_winding (GMT, this_lon, this_lat, save[pol].x, save[pol].y, np);
				/* Worry about this point being ~exactly on the border but returned inside = 2, but
				 * the grid node will be exactly the contour value and then fail the assignment of high
				 * below.  We therefore check if the value is not equal to the contour value as well */
				if (inside == 2) {	/* Might be inside */
					ij = gmt_M_ijp (G->header, row, col);
					if (!doubleAlmostEqual (G->data[ij], save[pol].cval))	/* OK, this point is truly inside */
						done = true;
					else	/* March along to the next point */
						col++;
				}
				else	/* March along to the next point */
					col++;
			}
			if (!done) continue;	/* Failed to find an inside point */
		}
		ij = gmt_M_ijp (G->header, row, col);
		save[pol].high = (G->data[ij] > save[pol].cval);

		if (save[pol].high && !tick_high) continue;	/* Do not tick highs */
		if (!save[pol].high && !tick_low) continue;	/* Do not tick lows */

		np = (int)gmt_clip_to_map (GMT, save[pol].x, save[pol].y, np, &xp, &yp);	/* Convert to inches */
		if (np == 0) continue;

		s = gmt_M_memory (GMT, NULL, np, double);	/* Compute distance along the contour */
		for (j = 1, s[0] = 0.0; j < np; j++)
			s[j] = s[j-1] + hypot (xp[j]-xp[j-1], yp[j]-yp[j-1]);
		n_ticks = irint (floor (s[np-1] / tick_gap));
		if (s[np-1] < GRDCONTOUR_MIN_LENGTH || n_ticks == 0) {	/* Contour is too short to be ticked or labeled */
			save[pol].do_it = false;
			gmt_M_free (GMT, s);	gmt_M_free (GMT, xp);	gmt_M_free (GMT, yp);
			continue;
		}

		way = gmt_polygon_centroid (GMT, xp, yp, np, &save[pol].xlabel, &save[pol].ylabel);	/* -1 is CCW, +1 is CW */
		/* Compute mean location of closed contour ~hopefully a good point inside to place label. */

		if (mode & 1) {	/* Tick the innermost contour */
			x_back = xp[np-1];	/* Last point along contour */
			y_back = yp[np-1];
			dist = 0.0;
			j = 0;
			add = M_PI_2 * ((save[pol].high) ? -way : +way);	/* So that tick points in the right direction */
			inc = s[np-1] / n_ticks;
			gmt_setpen (GMT, &save[pol].pen);
			while (j < np-1) {
				x_front = xp[j+1];
				y_front = yp[j+1];
				if (s[j] >= dist) {	/* Time for tick */
					dx = x_front - x_back;
					dy = y_front - y_back;
					a = atan2 (dy, dx) + add;
					sincos (a, &sa, &ca);
					x_end = xp[j] + tick_length * ca;
					y_end = yp[j] + tick_length * sa;
					PSL_plotsegment (PSL, xp[j], yp[j], x_end, y_end);
					dist += inc;
				}
				x_back = x_front;
				y_back = y_front;
				j++;
			}
		}
		gmt_M_free (GMT, s);	gmt_M_free (GMT, xp);	gmt_M_free (GMT, yp);
	}

	/* Still not finished with labeling polar caps when the pole point plots as a line (e.g., -JN).
	 * One idea would be to add help points to include the pole and used this polygon to compute where to
	 * plot the label but then skip those points when drawing the line. */

	for (pol = 0; tick_label && pol < n; pol++) {	/* Finally, do labels */
		if (!save[pol].do_it) continue;
		if (abs (save[pol].kind) == 2 && gmt_M_is_azimuthal (GMT)) {	/* Only plot once at mean location */
			for (pol2 = 0, k = -1; pol2 < n && k == -1; pol2++) {	/* Finally, do labels */
				if (save[pol2].kind != -save[pol].kind) continue;
				if (save[pol2].cval != save[pol].cval) continue;
				k = (int)pol2;	/* Found its counterpart */
			}
			if (k == -1) continue;
			x_lbl = 0.5 * (save[pol].xlabel + save[k].xlabel);
			y_lbl = 0.5 * (save[pol].ylabel + save[k].ylabel);
			if (mode & 1) {
				form = gmt_setfont (GMT, &save[pol].font);
				PSL_plottext (PSL, x_lbl, y_lbl, GMT->current.setting.font_annot[GMT_PRIMARY].size, lbl[save[pol].high], 0.0, PSL_MC, form);
			}
			save[k].do_it = false;
			if (mode & 2) gmt_add_label_record (GMT, T, x_lbl, y_lbl, 0.0, lbl[save[pol].high]);
		}
		else {
			if (mode & 1) {
				form = gmt_setfont (GMT, &save[pol].font);
				PSL_plottext (PSL, save[pol].xlabel, save[pol].ylabel, GMT->current.setting.font_annot[GMT_PRIMARY].size, lbl[save[pol].high], 0.0, PSL_MC, form);
			}
			if (mode & 2) gmt_add_label_record (GMT, T, save[pol].xlabel, save[pol].ylabel, 0.0, lbl[save[pol].high]);
		}
	}
	PSL_comment (PSL, "End Embellishment of innermost contours\n");
}

GMT_LOCAL void adjust_hill_label (struct GMT_CTRL *GMT, struct GMT_CONTOUR *G, struct GMT_GRID *Grid) {
	/* Modify orientation of contours to have top of annotation facing the local hill top */
	int col, row;
	uint64_t k, seg, ij;
	double n_columns, n_rows, x_on, y_on, x_node, y_node, x_node_p, y_node_p, dx, dy, dz, dot, angle;
	struct GMT_CONTOUR_LINE *C = NULL;

	for (seg = 0; seg < G->n_segments; seg++) {
		C = G->segment[seg];	/* Pointer to current segment */
		for (k = 0; k < C->n_labels; k++) {
			gmt_xy_to_geo (GMT, &x_on, &y_on, C->L[k].x, C->L[k].y);	/* Retrieve original coordinates */
			row = (int)gmt_M_grd_y_to_row (GMT, y_on, Grid->header);
			if (row < 0 || row >= (int)Grid->header->n_rows) continue;		/* Somehow, outside y range */
			while (gmt_M_x_is_lon (GMT, GMT_IN) && x_on < Grid->header->wesn[XLO]) x_on += 360.0;
			while (gmt_M_x_is_lon (GMT, GMT_IN) && x_on > Grid->header->wesn[XHI]) x_on -= 360.0;
			col = (int)gmt_M_grd_x_to_col (GMT, x_on, Grid->header);
			if (col < 0 || col >= (int)Grid->header->n_columns) continue;		/* Somehow, outside x range */
			angle = fmod (2.0 * C->L[k].angle, 360.0) * 0.5;	/* 0-180 range */
			if (angle > 90.0) angle -= 180.0;
			sincosd (angle + 90, &n_rows, &n_columns);	/* Coordinate of normal to label line */
			x_node = gmt_M_grd_col_to_x (GMT, col, Grid->header);
			y_node = gmt_M_grd_row_to_y (GMT, row, Grid->header);
			gmt_geo_to_xy (GMT, x_node, y_node, &x_node_p, &y_node_p);	/* Projected coordinates of nearest node point */
			dx = x_node_p - C->L[k].x;
			dy = y_node_p - C->L[k].y;
			if (gmt_M_is_zero (hypot (dx, dy))) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to adjust hill label contour orientation (node point on contour)\n");
				continue;
			}
			ij = gmt_M_ijp (Grid->header, row, col);
			dz = Grid->data[ij] - C->z;
			if (doubleAlmostEqualZero (Grid->data[ij], C->z)) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to adjust hill label contour orientation (node value = contour value)\n");
				continue;
			}
			dot = dx * n_columns + dy * n_rows;	/* 2-D Dot product of n and vector from contour to node. +ve if on same side of contour line */
			if (lrint (copysign (1.0, dot * dz)) != G->hill_label) C->L[k].angle += 180.0;	/* Must turn upside-down */
		}
	}
}

GMT_LOCAL enum grdcontour_contour_type gmt_is_closed (struct GMT_CTRL *GMT, struct GMT_GRID *G, double *x, double *y, int n) {
	/* Determine if this is a closed contour; returns a flag in the -4/+4 range */
	enum grdcontour_contour_type closed = cont_is_not_closed;
	int k;
	double small_x = 0.01 * G->header->inc[GMT_X], small_y = 0.01 * G->header->inc[GMT_Y];	/* Use 1% noise to find near-closed contours */
	double y_min, y_max;

	if (fabs (x[0] - x[n-1]) < small_x && fabs (y[0] - y[n-1]) < small_y) {	/* Closed interior contour */
		closed = cont_is_closed;
		x[n-1] = x[0];	y[n-1] = y[0];	/* Force exact closure */
	}
	else if (gmt_M_is_geographic (GMT, GMT_IN) && gmt_M_grd_is_global (GMT, G->header)) {	/* Global geographic grids are special */
		if (fabs (x[0] - G->header->wesn[XLO]) < small_x && fabs (x[n-1] - G->header->wesn[XLO]) < small_x) {	/* Split periodic boundary contour */
			closed = cont_is_closed_straddles_west;	/* Left periodic */
			x[0] = x[n-1] = G->header->wesn[XLO];	/* Force exact closure */
		}
		else if (fabs (x[0] - G->header->wesn[XHI]) < small_x && fabs (x[n-1] - G->header->wesn[XHI]) < small_x) {	/* Split periodic boundary contour */
			closed = cont_is_closed_straddles_east;	/* Right periodic */
			x[0] = x[n-1] = G->header->wesn[XHI];	/* Force exact closure */
		}
		else if (gmt_M_360_range (x[0], x[n-1])) {	/* Must be a polar cap */
			/* Determine if the polar cap stays on one side of the equator */
			y_min = y_max = y[0];
			for (k = 1; k < n; k++) {
				if (y[k] < y_min) y_min = y[k];
				if (y[k] > y_max) y_max = y[k];
			}
			if (y_min < 0.0 && y_max > 0.0)
				/* Special flags for meandering closed contours otherwise indistinguishable from polar caps */
				closed = (y_max > fabs (y_min)) ? cont_is_closed_straddles_equator_north : cont_is_closed_straddles_equator_south;
			else
				/* N or S polar cap; do not force closure though */
				closed = (y[0] > 0.0) ? cont_is_closed_around_north_pole : cont_is_closed_around_south_pole;
		}
	}
	return (closed);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_grdcontour (void *V_API, int mode, void *args) {
	/* High-level function that implements the grdcontour task */
	int error, c;
	bool need_proj, make_plot, two_only = false, begin, is_closed, data_is_time = false, use_t_offset = false;

	enum grdcontour_contour_type closed;

	unsigned int id, n_contours, n_edges, tbl_scl = 1, io_mode = 0, uc, tbl, label_mode = 0;
	unsigned int cont_counts[2] = {0, 0}, i, n, nn, *edge = NULL, n_tables = 1, fmt[3] = {0, 0, 0};

	uint64_t ij, *n_seg = NULL;

	size_t n_save = 0, n_alloc = 0, n_save_alloc = 0, n_tmp, *n_seg_alloc = NULL;

	char *cont_type = NULL, *cont_do_tick = NULL;
	char cont_label[GMT_LEN256] = {""}, format[GMT_LEN256] = {""};

	double aval, cval, small, xyz[2][3], z_range, t_offset = 0.0, wesn[4], rgb[4];
	double *xp = NULL, *yp = NULL, *contour = NULL, *x = NULL, *y = NULL, *cont_angle = NULL;

	struct GRDCONTOUR_CTRL *Ctrl = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_DATASEGMENT *S = NULL;
	struct GMT_CLOCK_IO Clock;
	struct GMT_DATE_IO Date;
	struct SAVE *save = NULL;
	struct GMT_GRID *G = NULL, *G_orig = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;	/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the grdcontour main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input grid\n");
	if (Ctrl->D.active && !Ctrl->D.file) GMT_Report (API, GMT_MSG_VERBOSE, "Contours will be written to standard output\n");

	GMT->current.map.z_periodic = Ctrl->Z.periodic;	/* Phase data */
	GMT_Report (API, GMT_MSG_VERBOSE, "Allocate memory and read data file\n");

	if ((G = GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, Ctrl->In.file, NULL)) == NULL) {	/* Get header only */
		Return (API->error);
	}

	make_plot = !Ctrl->D.active;	/* Turn off plotting if -D was used */
	need_proj = !Ctrl->D.active || Ctrl->contour.save_labels;	/* Turn off mapping if -D was used, unless +t was set */

	/* Determine what wesn to pass to map_setup */

	if (!GMT->common.R.active[RSET])	/* -R was not set so we use the grid domain */
		gmt_set_R_from_grd (GMT, G->header);

	if (need_proj && gmt_map_setup (GMT, GMT->common.R.wesn)) Return (GMT_PROJECTION_ERROR);

	/* Determine the wesn to be used to actually read the grid file */

	if (!gmt_grd_setregion (GMT, G->header, wesn, BCR_BILINEAR)) {
		/* No grid to plot; just do empty map and return */
		GMT_Report (API, GMT_MSG_VERBOSE, "Warning: No data within specified region\n");
		if (make_plot) {
			if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
			gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
			gmt_plotcanvas (GMT);	/* Fill canvas if requested */
			gmt_map_basemap (GMT);
			gmt_plane_perspective (GMT, -1, 0.0);
			gmt_plotend (GMT);
		}
		if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}

	/* Read data */

	if (GMT_Read_Data (API, GMT_IS_GRID, GMT_IS_FILE, GMT_IS_SURFACE, GMT_DATA_ONLY, wesn, Ctrl->In.file, G) == NULL) {
		Return (API->error);
	}

	if (GMT->current.io.col_type[GMT_IN][GMT_Z] == GMT_IS_ABSTIME) {	/* Grid data is time */
		/* To properly label contours using GMT time formatting we will rely on the machinery
		 * for output data formatting.  Thus, we temporarily overwrite those settings with
		 * the selected map settings, then undo the damage before GMT_basemap is called.
		 */
		double t_epoch_unit, t_epoch_unit_even, tmp;
		data_is_time = true;
		/* Save original date and clock output formatting structures */
		gmt_M_memcpy (&Clock, &GMT->current.io.clock_output, 1, struct GMT_CLOCK_IO);
		gmt_M_memcpy (&Date, &GMT->current.io.date_output, 1, struct GMT_DATE_IO);
		/* Set date and clock output to the corresponding MAP versions */
		gmt_M_memcpy (&GMT->current.io.clock_output, &GMT->current.plot.calclock.clock, 1, struct GMT_CLOCK_IO);
		gmt_M_memcpy (&GMT->current.io.date_output,  &GMT->current.plot.calclock.date, 1, struct GMT_DATE_IO);
		/* We want time to align on integer TIME_UNIT.  If epoch is not a multiple of TIME_UNIT
		 * then we compute the offset and add to z */
		tmp = GMT->current.setting.time_system.epoch_t0; GMT->current.setting.time_system.epoch_t0 = 0.0;	/* Save */
		t_epoch_unit = gmt_rdc2dt (GMT, GMT->current.setting.time_system.rata_die, tmp * GMT_DAY2SEC_F);	/* Epoch in user units */
		GMT->current.setting.time_system.epoch_t0 = tmp;	/* Restore */
		t_epoch_unit_even = floor (t_epoch_unit / Ctrl->A.interval) * Ctrl->A.interval;	/* Offset to grid values in user units reuired to align contours on even epoch */
		t_offset = t_epoch_unit - t_epoch_unit_even;	/* Offset to grid values in user units to align on even epoch */
		use_t_offset = !doubleAlmostEqualZero (t_epoch_unit, t_epoch_unit_even);
		if (use_t_offset) {	/* Must temporarily add t_offset to grid, quietly */
			GMT_Report (API, GMT_MSG_DEBUG, "Adding %g to align grid times with TIME_UNIT steps\n", t_offset);
			gmt_scale_and_offset_f (GMT, G->data, G->header->size, 1.0, t_offset);
			G->header->z_min += t_offset;
			G->header->z_max += t_offset;
		}
	}
	if (!(Ctrl->Z.scale == 1.0 && Ctrl->Z.offset == 0.0)) {	/* Must transform z grid */
		GMT_Report (API, GMT_MSG_VERBOSE, "Subtracting %g and multiplying grid by %g\n", Ctrl->Z.offset, Ctrl->Z.scale);
		G->header->z_min = (G->header->z_min - Ctrl->Z.offset) * Ctrl->Z.scale;
		G->header->z_max = (G->header->z_max - Ctrl->Z.offset) * Ctrl->Z.scale;
		if (Ctrl->Z.scale < 0.0) gmt_M_double_swap (G->header->z_min, G->header->z_max);
		/* Since gmt_scale_and_offset_f applies z' = z * scale + offset we must adjust Z.offset first: */
		Ctrl->Z.offset *= Ctrl->Z.scale;
		gmt_scale_and_offset_f (GMT, G->data, G->header->size, Ctrl->Z.scale, -Ctrl->Z.offset);
	}
	if (Ctrl->L.low > G->header->z_min) G->header->z_min = Ctrl->L.low;	/* Possibly clip the z range */
	if (Ctrl->L.high < G->header->z_max) G->header->z_max = Ctrl->L.high;
	if (Ctrl->L.active && G->header->z_max < G->header->z_min) {	/* Specified contour range outside range of data - quit */
		GMT_Report (API, GMT_MSG_VERBOSE, "Warning: No contours within specified -L range\n");
		if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}

	if (!strcmp (Ctrl->contour.unit, "z")) strncpy (Ctrl->contour.unit, G->header->z_units, GMT_LEN64-1);
	if (Ctrl->A.interval == 0.0) Ctrl->A.interval = Ctrl->C.interval;

	if (Ctrl->contour.annot) {	/* Want annotated contours */
		/* Determine the first annotated contour level */
		aval = floor (G->header->z_min / Ctrl->A.interval) * Ctrl->A.interval;
		if (aval < G->header->z_min) aval += Ctrl->A.interval;
	}
	else	/* No annotations, set aval outside range */
		aval = G->header->z_max + 1.0;

	if (Ctrl->C.cpt) {	/* Presumably got a CPT */
		if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (P->categorical) {
			GMT_Report (API, GMT_MSG_NORMAL, "Warning: Categorical data (as implied by CPT) do not have contours.  Check plot.\n");
		}
		/* Set up which contours to draw based on the CPT slices and their attributes */
		n_contours = P->n_colors + 1;	/* Since n_colors refer to slices */
		n_tmp = 0;
		gmt_M_malloc2 (GMT, contour, cont_angle, n_contours, &n_tmp, double);
		gmt_M_malloc2 (GMT, cont_type, cont_do_tick, n_contours, &n_alloc, char);
		for (i = c = 0; i < P->n_colors; i++) {
			if (P->data[i].skip) continue;
			contour[c] = P->data[i].z_low;
			if (Ctrl->A.mode)
				cont_type[c] = 'C';
			else if (P->data[i].annot)
				cont_type[c] = 'A';
			else
				cont_type[c] = (Ctrl->contour.annot) ? 'A' : 'C';
			cont_angle[c] = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
			cont_do_tick[c] = (char)Ctrl->T.active;
			c++;
		}
		contour[c] = P->data[P->n_colors-1].z_high;
		if (Ctrl->A.mode)
			cont_type[c] = 'C';
		else if (P->data[P->n_colors-1].annot & 2)
			cont_type[c] = 'A';
		else
			cont_type[c] = (Ctrl->contour.annot) ? 'A' : 'C';
		cont_angle[c] = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
		cont_do_tick[c] = (char)Ctrl->T.active;
		n_contours = c + 1;
	}
	else if (Ctrl->C.file) {	/* read contour info from file with cval C|A [angle] records */
		char *record = NULL;
		int got, in_ID;
		double tmp;

		n_contours = 0;
		/* Must register Ctrl->C.file first since we are going to read rec-by-rec from all available source */
		if ((in_ID = GMT_Register_IO (API, GMT_IS_TEXTSET, GMT_IS_FILE, GMT_IS_NONE, GMT_IN, NULL, Ctrl->C.file)) == GMT_NOTSET) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error registering contour info file %s\n", Ctrl->C.file);
			Return (GMT_RUNTIME_ERROR);
		}

		/* Initialize the i/o since we are doing record-by-record reading/writing */
		if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_NONE, GMT_IN, GMT_ADD_EXISTING, 0, options) != GMT_NOERROR) {
			Return (API->error);	/* Establishes data input */
		}
		if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {	/* Enables data input and sets access mode */
			GMT_Report (API, GMT_MSG_NORMAL, "Error enabling contour info file %s\n", Ctrl->C.file);
			Return (API->error);
		}
		do {	/* Keep returning records until we reach EOF */
			if ((record = GMT_Get_Record (API, GMT_READ_TEXT, NULL)) == NULL) {	/* Read next record, get NULL if special case */
				if (gmt_M_rec_is_error (GMT)) 		/* Bail if there are any read errors */
					Return (GMT_RUNTIME_ERROR);
				if (gmt_M_rec_is_any_header (GMT)) 	/* Skip all table and segment headers */
					continue;
				if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
					break;
				assert (record != NULL);						/* Should never get here */
			}
			if (gmt_is_a_blank_line (record)) continue;	/* Nothing in this record */

			/* Data record to process */

			if (n_contours == n_alloc) {
				n_tmp = n_alloc;
				gmt_M_malloc2 (GMT, contour, cont_angle, n_contours, &n_tmp, double);
				gmt_M_malloc2 (GMT, cont_type, cont_do_tick, n_contours, &n_alloc, char);
			}
			got = sscanf (record, "%lf %c %lf", &contour[n_contours], &cont_type[n_contours], &tmp);
			if (cont_type[n_contours] == '\0') cont_type[n_contours] = 'C';
			cont_do_tick[n_contours] = (Ctrl->T.active && (cont_type[n_contours] == 'C' || cont_type[n_contours] == 'A')) ? 1 : 0;
			cont_angle[n_contours] = (got == 3) ? tmp : GMT->session.d_NaN;
			if (got == 3) Ctrl->contour.angle_type = 2;	/* Must set this directly if angles are provided */
			n_contours++;
		} while (true);
		if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further grid data input */
			Return (API->error);
		}
	}
	else if (!gmt_M_is_dnan (Ctrl->C.single_cont) || !gmt_M_is_dnan (Ctrl->A.single_cont)) {	/* Plot one or two contours only  */
		n_contours = 0;
		n_tmp = 0;
		gmt_M_malloc2 (GMT, contour, cont_angle, 2, &n_tmp, double);		/* Allocate 2, even if we only have 1 */
		gmt_M_malloc2 (GMT, cont_type, cont_do_tick, 2, &n_alloc, char);
		if (!gmt_M_is_dnan (Ctrl->C.single_cont)) {
			cont_type[n_contours] = 'C';
			contour[n_contours++] = Ctrl->C.single_cont;
		}
		if (!gmt_M_is_dnan (Ctrl->A.single_cont)) {
			cont_type[n_contours] = 'A';
			contour[n_contours] = Ctrl->A.single_cont;
			cont_do_tick[n_contours] = Ctrl->T.active ? 1 : 0;
			cont_angle[n_contours] = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
			n_contours++;
		}
	}
	else {	/* Set up contour intervals automatically from Ctrl->C.interval and Ctrl->A.interval */
		double min, max;
		min = floor (G->header->z_min / Ctrl->C.interval) * Ctrl->C.interval;
		if (!GMT->current.map.z_periodic && min < G->header->z_min) min += Ctrl->C.interval;
		max = ceil (G->header->z_max / Ctrl->C.interval) * Ctrl->C.interval;
		if (max > G->header->z_max) max -= Ctrl->C.interval;
		if (Ctrl->A.interval > Ctrl->C.interval && fabs ((Ctrl->A.interval/Ctrl->C.interval) - irint (Ctrl->A.interval/Ctrl->C.interval)) > GMT_CONV4_LIMIT)
			GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Annotation interval is not a multiple of contour interval - no annotated contours will be drawn.\n");
		else if (Ctrl->contour.annot && Ctrl->A.interval < Ctrl->C.interval)
			GMT_Report (API, GMT_MSG_VERBOSE, "Warning: Annotation interval < contour interval - some/all annotated contours will not be drawn.\n");
		for (c = irint (min/Ctrl->C.interval), n_contours = 0; c <= irint (max/Ctrl->C.interval); c++, n_contours++) {
			if (n_contours == n_alloc) {
				n_tmp = n_alloc;
				gmt_M_malloc2 (GMT, contour, cont_angle, n_contours, &n_tmp, double);
				gmt_M_malloc2 (GMT, cont_type, cont_do_tick, n_contours, &n_alloc, char);
			}
			contour[n_contours] = c * Ctrl->C.interval;
			if (Ctrl->contour.annot && (contour[n_contours] - aval) > GMT_CONV4_LIMIT) aval += Ctrl->A.interval;
			if (Ctrl->A.mode)	/* No labels */
				cont_type[n_contours] = 'C';
			else
				cont_type[n_contours] = (fabs (contour[n_contours] - aval) < GMT_CONV4_LIMIT) ? 'A' : 'C';
			cont_angle[n_contours] = (Ctrl->contour.angle_type == 2) ? Ctrl->contour.label_angle : GMT->session.d_NaN;
			cont_do_tick[n_contours] = (char)Ctrl->T.active;
		}
	}
#if 0
	/* PS: 4/10/2014: I commented this out as for phase grids in -180/180 range we always missed the 180 contour. */
	if (GMT->current.map.z_periodic && n_contours > 1 && fabs (contour[n_contours-1] - contour[0] - 360.0) < GMT_CONV4_LIMIT) {	/* Get rid of redundant contour */
		n_contours--;
	}
#endif
	if (n_contours == 0) {	/* No contours within range of data */
		GMT_Report (API, GMT_MSG_VERBOSE, "Warning: No contours found\n");
		if (make_plot) {
			if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
			gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
			gmt_plotcanvas (GMT);	/* Fill canvas if requested */
			gmt_map_basemap (GMT);
			gmt_plane_perspective (GMT, -1, 0.0);
			gmt_plotend (GMT);
		}
		gmt_M_free (GMT, contour);
		gmt_M_free (GMT, cont_type);
		gmt_M_free (GMT, cont_angle);
		gmt_M_free (GMT, cont_do_tick);
		if (GMT_Destroy_Data (API, &G) != GMT_NOERROR) {
			Return (API->error);
		}
		if (Ctrl->C.cpt && GMT_Destroy_Data (API, &P) != GMT_NOERROR) {
			Return (API->error);
		}
		Return (GMT_NOERROR);
	}

	/* OK, now we know we have contouring to do */

	z_range = G->header->z_max - G->header->z_min;
	if (Ctrl->C.interval == 0.0)
		small = z_range * 1.0e-6;	/* Our float noise threshold */
	else
		small = MIN (Ctrl->C.interval, z_range) * 1.0e-6;	/* Our float noise threshold */
	n_alloc = n_tmp = n_contours;
	gmt_M_malloc2 (GMT, contour, cont_angle, 0U, &n_tmp, double);
	gmt_M_malloc2 (GMT, cont_type, cont_do_tick, 0U, &n_alloc, char);

	gmt_grd_minmax (GMT, G, xyz);
	if (gmt_contlabel_prep (GMT, &Ctrl->contour, xyz)) {	/* Prep for crossing lines, if any */
		gmt_M_free (GMT, contour);		gmt_M_free (GMT, cont_type);
		gmt_M_free (GMT, cont_angle);		gmt_M_free (GMT, cont_do_tick);
		Return (GMT_RUNTIME_ERROR);
	}

	for (i = 0; i < 3; i++) GMT->current.io.col_type[GMT_OUT][i] = GMT->current.io.col_type[GMT_IN][i];	/* Used if -D is set */

	/* Because we are doing single-precision, we cannot subtract incrementally but must start with the
	 * original grid values and subtract the current contour value. */

 	if ((G_orig = GMT_Duplicate_Data (API, GMT_IS_GRID, GMT_DUPLICATE_DATA, G)) == NULL) Return (GMT_RUNTIME_ERROR); /* Original copy of grid used for contouring */
	n_edges = G->header->n_rows * (urint (ceil (G->header->n_columns / 16.0)));
	edge = gmt_M_memory (GMT, NULL, n_edges, unsigned int);	/* Bit flags used to keep track of contours */

	if (Ctrl->D.active) {
		uint64_t dim[GMT_DIM_SIZE] = {0, 0, 0, 3};
		if (!Ctrl->D.file || !strchr (Ctrl->D.file, '%'))	/* No file given or filename without C-format specifiers means a single output file */
			io_mode = GMT_WRITE_SET;
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
		dim[GMT_TBL] = n_tables;
		if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) Return (API->error);	/* An empty dataset */
		n_seg_alloc = gmt_M_memory (GMT, NULL, n_tables, size_t);
		n_seg = gmt_M_memory (GMT, NULL, n_tables, uint64_t);
	}

	gmt_M_memset (rgb, 4, double);
	
	if (make_plot) {
		if (Ctrl->contour.delay) GMT->current.ps.nclip = +2;	/* Signal that this program initiates clipping that will outlive this process */
		if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
		gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
		gmt_plotcanvas (GMT);	/* Fill canvas if requested */
		if (Ctrl->contour.delay) gmt_map_basemap (GMT);	/* Must do -B here before clipping makes it not doable */
		gmt_map_clip_on (GMT, GMT->session.no_rgb, 3);
	}

	for (c = uc = 0; uc < n_contours; c++, uc++) {	/* For each contour value cval */

		if (Ctrl->L.active && (contour[c] < Ctrl->L.low || contour[c] > Ctrl->L.high)) continue;	/* Outside desired range */

		/* Reset markers and set up new zero-contour */

		cval = contour[c];
		GMT_Report (API, GMT_MSG_VERBOSE, "Tracing the %g contour\n", cval);

		/* New approach to avoid round-off */

		for (ij = 0; ij < G->header->size; ij++) {
			G->data[ij] = G_orig->data[ij] - (float)cval;		/* If there are NaNs they will remain NaNs */
			if (G->data[ij] == 0.0) G->data[ij] += (float)small;	  /* There will be no actual zero-values, just -ve and +ve values */
		}

		id = (cont_type[c] == 'A' || cont_type[c] == 'a') ? 1 : 0;

		Ctrl->contour.line_pen = Ctrl->W.pen[id];	/* Load current pen into contour structure */
		if (Ctrl->W.cpt_effect) {
			gmt_get_rgb_from_z (GMT, P, cval, rgb);
			if (Ctrl->W.cptmode & 1)	/* Override pen color according to CPT */
				gmt_M_rgb_copy (&Ctrl->contour.line_pen.rgb, rgb);
			if (Ctrl->W.cptmode & 2)	/* Override label color according to CPT */
				gmt_M_rgb_copy (&Ctrl->contour.font_label.fill.rgb, rgb);
		}
		else
			gmt_M_rgb_copy (&Ctrl->contour.font_label.fill.rgb, Ctrl->contour.line_pen.rgb);
		n_alloc = 0;
		begin = true;

		while ((n = (unsigned int)gmt_contours (GMT, G, Ctrl->S.value, GMT->current.setting.interpolant, Ctrl->F.value, edge, &begin, &x, &y)) > 0) {

			closed = gmt_is_closed (GMT, G, x, y, n);	/* Closed interior/periodic boundary contour? */
			is_closed = (closed != cont_is_not_closed);

			if (closed == cont_is_not_closed || n >= Ctrl->Q.min) {	/* Passed our minimum point criteria for closed contours */
				if (Ctrl->D.active && n > 2) {	/* Save the contour as output data */
					S = gmt_prepare_contour (GMT, x, y, n, cval);
					/* Select which table this segment should be added to */
					tbl = (io_mode == GMT_WRITE_TABLE) ? ((two_only) ? is_closed : tbl_scl * c) : 0;
					if (n_seg[tbl] == n_seg_alloc[tbl]) {
						size_t old_n_alloc = n_seg_alloc[tbl];
						D->table[tbl]->segment = gmt_M_memory (GMT, D->table[tbl]->segment, (n_seg_alloc[tbl] += GMT_SMALL_CHUNK), struct GMT_DATASEGMENT *);
						gmt_M_memset (&(D->table[tbl]->segment[old_n_alloc]), n_seg_alloc[tbl] - old_n_alloc, struct GMT_DATASEGMENT *);	/* Set to NULL */
					}
					D->table[tbl]->segment[n_seg[tbl]++] = S;
					D->table[tbl]->n_segments++;	D->n_segments++;
					D->table[tbl]->n_records += n;	D->n_records += n;
					/* Generate a file name and increment cont_counts, if relevant */
					if (io_mode == GMT_WRITE_TABLE && !D->table[tbl]->file[GMT_OUT])
						D->table[tbl]->file[GMT_OUT] = gmt_make_filename (GMT, Ctrl->D.file, fmt, cval, is_closed, cont_counts);
					else if (io_mode == GMT_WRITE_SEGMENT)
						S->file[GMT_OUT] = gmt_make_filename (GMT, Ctrl->D.file, fmt, cval, is_closed, cont_counts);
				}
				if (make_plot && cont_do_tick[c] && is_closed) {	/* Must store the entire contour for later processing */
					/* These are original coordinates that have not yet been projected */
					int extra;
					if (n_save == n_save_alloc) save = gmt_M_malloc (GMT, save, n_save, &n_save_alloc, struct SAVE);
					extra = (abs (closed) == 2);	/* Need extra slot to temporarily close half-polygons */
					n_alloc = 0;
					gmt_M_memset (&save[n_save], 1, struct SAVE);
					gmt_M_malloc2 (GMT, save[n_save].x, save[n_save].y, n + extra, &n_alloc, double);
					gmt_M_memcpy (save[n_save].x, x, n, double);
					gmt_M_memcpy (save[n_save].y, y, n, double);
					gmt_M_memcpy (&save[n_save].pen,  &Ctrl->contour.line_pen,   1, struct GMT_PEN);
					gmt_M_memcpy (&save[n_save].font, &Ctrl->contour.font_label, 1, struct GMT_FONT);
					save[n_save].do_it = true;
					save[n_save].cval = cval;
					save[n_save].kind = closed;
					if (extra) { save[n_save].x[n] = save[n_save].x[0];	save[n_save].y[n] = save[n_save].y[0];}
					save[n_save].n = n + extra;
					n_save++;
				}
				if (need_proj && (nn = (unsigned int)gmt_clip_to_map (GMT, x, y, n, &xp, &yp)) != 0) {	/* Lines inside the region */
					/* From here on, xp/yp are map inches */
					if (cont_type[c] == 'A' || cont_type[c] == 'a') {	/* Annotated contours */
						if (data_is_time) {
							double tval = (use_t_offset) ? cval - t_offset : cval;
							char *c = NULL;
							gmt_ascii_format_col (GMT, cont_label, tval, GMT_OUT, GMT_Z);
							if ((c = strchr (cont_label, 'T')) != NULL) c[0] = ' ';	/* Replace ISO T with space for plots */
						}
						else {
							gmt_get_format (GMT, cval, Ctrl->contour.unit, NULL, format);
							sprintf (cont_label, format, cval);
						}
					}
					else
						cont_label[0] = '\0';
					gmt_hold_contour (GMT, &xp, &yp, nn, cval, cont_label, cont_type[c], cont_angle[c], closed == cont_is_closed, true, &Ctrl->contour);
					gmt_M_free (GMT, xp);
					gmt_M_free (GMT, yp);
				}
			}
			gmt_M_free (GMT, x);
			gmt_M_free (GMT, y);
		}
	}

	if (Ctrl->D.active) {	/* Write the contour line output file(s) */
		gmt_set_segmentheader (GMT, GMT_OUT, true);	/* Turn on segment headers on output */
		if ((error = gmt_set_cols (GMT, GMT_OUT, 3)) != GMT_NOERROR) {
			Return (error);
		}
		for (tbl = 0; tbl < D->n_tables; tbl++) D->table[tbl]->segment = gmt_M_memory (GMT, D->table[tbl]->segment, n_seg[tbl], struct GMT_DATASEGMENT *);
		if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, io_mode, NULL, Ctrl->D.file, D) != GMT_NOERROR) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
			Return (API->error);
		}
		gmt_M_free (GMT, n_seg_alloc);
		gmt_M_free (GMT, n_seg);
	}

	if (data_is_time) {	/* Undo earlier swapparoo */
		/* Restore original date and clock output formatting structures */
		gmt_M_memcpy (&GMT->current.io.clock_output, &Clock, 1, struct GMT_CLOCK_IO);
		gmt_M_memcpy (&GMT->current.io.date_output, &Date, 1, struct GMT_DATE_IO);
	}

	if (make_plot) label_mode |= 1;		/* Would want to plot ticks and labels if -T is set */
	if (Ctrl->contour.save_labels) {	/* Want to save the contour label locations (lon, lat, angle, label) if -T is set */
		label_mode |= 2;
		if ((error = gmt_contlabel_save_begin (GMT, &Ctrl->contour)) != 0) Return (error);
	}

	if (make_plot) PSL_setdash (PSL, NULL, 0.0);

	if (Ctrl->T.active && n_save) {	/* Finally sort and plot ticked innermost contours and plot/save L|H labels */
		save = gmt_M_memory (GMT, save, n_save, struct SAVE);

		grd_sort_and_plot_ticks (GMT, PSL, save, n_save, G_orig, Ctrl->T.dim[GMT_X], Ctrl->T.dim[GMT_Y], Ctrl->T.low, Ctrl->T.high, Ctrl->T.label, Ctrl->T.txt, label_mode, Ctrl->contour.Out);
		for (i = 0; i < n_save; i++) {
			gmt_M_free (GMT, save[i].x);
			gmt_M_free (GMT, save[i].y);
		}
		gmt_M_free (GMT, save);
	}

	if (make_plot) {
		/* Must possibly adjust label angles so that label is readable when following contours */
		if (Ctrl->contour.hill_label) adjust_hill_label (GMT, &Ctrl->contour, G);

		gmt_contlabel_plot (GMT, &Ctrl->contour);

		if (!Ctrl->contour.delay) {
			gmt_map_clip_off (GMT);
			gmt_map_basemap (GMT);
		}

		gmt_plane_perspective (GMT, -1, 0.0);

		gmt_plotend (GMT);
	}

	if (Ctrl->contour.save_labels) {	/* Close file with the contour label locations (lon, lat, angle, label) */
		if ((error = gmt_contlabel_save_end (GMT, &Ctrl->contour)) != 0) Return (error);
	}

	if (make_plot || Ctrl->contour.save_labels) gmt_contlabel_free (GMT, &Ctrl->contour);

	if (GMT_Destroy_Data (GMT->parent, &G_orig) != GMT_NOERROR) {
		GMT_Report (API, GMT_MSG_NORMAL, "Failed to free G_orig\n");
	}
	gmt_M_free (GMT, edge);
	gmt_M_free (GMT, contour);
	gmt_M_free (GMT, cont_type);
	gmt_M_free (GMT, cont_angle);
	gmt_M_free (GMT, cont_do_tick);

	GMT_Report (API, GMT_MSG_VERBOSE, "Done!\n");

	Return (GMT_NOERROR);
}
