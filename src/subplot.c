/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Date:	1-Jul-2017
 * Version:	6 API
 *
 * Brief synopsis: gmt subplot determines dimensions and offsets for a multi-panel figure.
 *	gmt subplot begin -N<nrows>/<ncols> [-A<labels>] [-D[x][y]] [-F[f|s][<WxH>][+g<fill>][+p<pen>][+d]] [-L<layout>] [-M<margins>] [-T<title>] [-V]
 *	gmt subplot [set] <row>,<col> [-A<fixlabel>] [-C<side><clearance>[u]] [-V]
 *	gmt subplot end [-V]
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"subplot"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Set multi-panel figure attributes under a GMT modern mode session"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"VXY"

/* Control structure for subplot */

#define SUBPLOT_CM_TO_INCH 	(1.0/2.54)
#define SUBPLOT_BEGIN		1
#define SUBPLOT_END		0
#define SUBPLOT_SET		2
#define SUBPLOT_FIGURE		1
#define SUBPLOT_PANEL		0
#define SUBPLOT_LABEL_IS_LETTER	0
#define SUBPLOT_LABEL_IS_NUMBER	1
#define SUBPLOT_COL_FIXED_X	1
#define SUBPLOT_ROW_FIXED_Y	2
#define SUBPLOT_PANEL_COL_TITLE	1
#define SUBPLOT_PANEL_TITLE	2
#define SUBPLOT_PLACE_AT_MIN	1
#define SUBPLOT_PLACE_AT_MAX	2
#define SUBPLOT_PLACE_AT_BOTH	3

struct SUBPLOT_CTRL {
	struct In {	/* begin | end | set */
		bool active;
		unsigned int mode;	/* SUBPLOT_BEGIN|SET|END*/
		unsigned int row, col;
	} In;
	struct A {	/* -A[<letter>|<number>][+c<clearance>][+g<fill>][+j|J<pos>][+o<offset>][+p<pen>][+r|R][+v] */
		bool active;
		char format[GMT_LEN16];
		char fill[GMT_LEN64];
		char pen[GMT_LEN64];
		unsigned int mode;
		unsigned int nstart;
		unsigned int way;
		unsigned int roman;
		char cstart;
		char placement[3];
		char justify[3];
		double off[2];
		double clearance[2];
	} A;
	struct C {	/* -C[side]<clearance>[u]  */
		bool active;
		double gap[4];
	} C;
	struct D {	/* -D[x][y]]  */
		bool active;
		int dir[2];
	} D;
	struct F {	/* -F[f|s][<width>[u]/<height>[u]][+d][+g<fill>][+p<pen>] */
		bool active;
		bool debug;
		unsigned int mode;
		double dim[2];
		char fill[GMT_LEN64];
		char pen[GMT_LEN64];
	} F;
	struct L {	/* -L<layout> */
		bool active;
		bool has_label[2];	/* True if x and y labels */
		unsigned int mode;	/* 0 for -L, 1 for -LC, 2 for -LR (3 for both) */
		unsigned int parallel;	/* 0 for horizontal annotations, 1 for axis-parallel */
		char *axes;		/* WESNwesn for -L */
		char *label[2];		/* The constant x and y labels */
		unsigned int ptitle;	/* 0 = no panel titles, 1 = column titles, 2 = all panel titles */
		unsigned annotate[2];	/* 1 if only l|r or t|b, 0 for both */
		unsigned tick[2];		/* 1 if only l|r or t|b, 0 for both */
	} L;
	struct M {	/* -M<margin>[u] | <xmargin>[u]/<ymargin>[u]  | <wmargin>[u]/<emargin>[u]/<smargin>[u]/<nmargin>[u]  */
		bool active;
		double margin[4];
	} M;
	struct N {	/* NrowsxNcolumns */
		bool active;
		unsigned int dim[2];
		unsigned int n_subplots;
	} N;
	struct T {	/* -T<title> */
		bool active;
		char *title;
	} T;
};

EXTERN_MSC void gmtlib_str_tolower (char *string);

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SUBPLOT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct SUBPLOT_CTRL);
	strncpy (C->A.placement, "TL", 2);
	strncpy (C->A.justify,   "TL", 2);
	C->A.off[GMT_X] = C->A.off[GMT_Y] = 0.01 * GMT_TEXT_OFFSET * GMT->current.setting.font_tag.size / PSL_POINTS_PER_INCH; /* 20% */
	C->A.clearance[GMT_X] = C->A.clearance[GMT_Y] = 0.01 * GMT_TEXT_CLEARANCE * GMT->current.setting.font_tag.size / PSL_POINTS_PER_INCH;	/* 15% */
	
	C->D.dir[GMT_X] = C->D.dir[GMT_Y] = +1;	/* Normal axes directions for Cartesian plots */
	C->A.fill[0] = C->A.pen[0] = '-';	/* No fill or outline */
	C->F.fill[0] = C->F.pen[0] = '-';	/* No fill or outline */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct SUBPLOT_CTRL *C) {	/* Deallocate control structure */
	gmt_M_unused (GMT);
	if (!C) return;
	gmt_M_str_free (C->T.title);
	gmt_M_str_free (C->L.label[GMT_X]);
	gmt_M_str_free (C->L.label[GMT_Y]);
	gmt_M_str_free (C->L.axes);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: subplot begin <nrows>x<ncols> [-A<autolabelinfo>] [-D[x][y]] [-F[[f[|s]]<width>/<height>][+f<fill>][+p<pen>][+d]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-L<layout>] [-M<margins>] [-T<title>] [%s]\n\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "usage: subplot <row>,<col> [-A<fixedlabel>] [-C<side><clearance>[u]] [%s]\n\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "usage: subplot end [%s]\n\n", GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<nrows>x<ncols> is the number of rows and columns of panels in this figure.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Specify automatic tagging of each subplot.  Append either a number or letter [a].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This sets the tag of the top-left subplot and others follow sequentially.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Surround number or letter by parentheses on any side if these should be typeset.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +j|J<refpoint> to specify where the tag should be plotted in the subplot [TL].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: +j sets justification = <refpoint> while +J selects the mirror opposite.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c<dx>[/<dy>] for the clearance between tag and surrounding box.  Only used\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   if +g or +p are set.  Append units {%s} or %% of fontsize [%d%%].\n", GMT_DIM_UNITS_DISPLAY, GMT_TEXT_CLEARANCE);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +g to fill the textbox with <fill> color [no fill].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +o<dx>[/<dy>] to offset tag in direction implied by <justify> [%d%% of font size].\n", GMT_TEXT_OFFSET);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p to draw the outline of the textbox using selected pen [no outline].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +r to set number using Roman numerals; use +R for uppercase [arabic].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +v to number down columns [subplots are numbered across rows].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Specify a gap of dimension <clearance>[u] to the <side> (w|e|s|n) of the plottable subplot.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Shrinks the size for the main plot to make room for scales, bars, etc.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Repeatable for more than one side [no clearances].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D For Cartesian subplots only: If x and/or the y-axis should be reversed, append x and/or y.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F or -Ff: Specify dimension of the whole figure plot area.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, use -Fs to set dimensions of area that each multi-subplot figure may occupy.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append +g<fill> to paint canvas and +p<pen> to draw outline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +d to draw faint red lines outlining each subplot\'s map area (for debugging).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set subplot layout. May be set once (-L) or separately for rows (-LR) and columns (-LC):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -L: Append codes from WESNwesnlrbt to indicate which subplot frames should be drawn, ticked and annotated.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +l to make space for axes labels; applies equally to all subplots [no labels].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       Append x to only use x-labels or y for only y-labels [both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +t to make space for all subplot titles; use +tc for top row titles only [no subplot titles].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -LC: Each subplot Column share common x-range. Only first (top) and last (bottom) rows are annotated.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append t or b to select only one of those two rows instead [both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +l if annotated x-axes should have a label [none]; optionally append the label if fixed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +t to make space for all subplot titles; use +tc for top row titles only [no subplot titles].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -LR: Each subplot Row share common y-range. Only first (left) and last (right) columns are annotated.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append l or r to select only one of those two columns [both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +l if annotated y-axes will have a label [none]; optionally append the label if fixed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +p if axes annotations should be parallel to axis [horizontal].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Adds space around each subplot. Append  a uniform <margin>, separate x and y <xmargin>/<ymargin>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     or individual <wmargin>/<emargin>/<smargin>/<nmargin> for each side [0.5c].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify a main heading to be centered above all the subplots [none].\n");
	GMT_Option (API, "V");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SUBPLOT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, n;
	char *c = NULL, add[2] = {0, 0}, string[GMT_LEN128] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMT_PEN pen;	/* Only used to make sure any pen is given with correct syntax */
	struct GMT_FILL fill;	/* Only used to make sure any fill is given with correct syntax */

	opt = options;	/* The first argument is the subplot command */
	if (opt->option != GMT_OPT_INFILE) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: No subplot command given\n");
		return GMT_PARSE_ERROR;
	}
	if (!strncmp (opt->arg, "begin", 5U)) {	/* Initiate new subplot */
		Ctrl->In.mode = SUBPLOT_BEGIN;
		opt = opt->next;	/* The required number of rows and columns */
		if (opt == NULL || opt->option != GMT_OPT_INFILE || (n = sscanf (opt->arg, "%dx%d", &Ctrl->N.dim[GMT_Y], &Ctrl->N.dim[GMT_X]) < 1)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: subplot begin: Unable to extract nrows and ncols from %s\n", opt->arg);
			return GMT_PARSE_ERROR;
		}
		if (n == 1) Ctrl->N.dim[GMT_X] = Ctrl->N.dim[GMT_Y];
		Ctrl->N.n_subplots = Ctrl->N.dim[GMT_X] * Ctrl->N.dim[GMT_Y];
		Ctrl->N.active = true;
	}
	else if (!strncmp (opt->arg, "end", 3U))
		Ctrl->In.mode = SUBPLOT_END;
	else if (!strncmp (opt->arg, "set", 3U)) {	/* Explicitly called set */
		opt = opt->next;
		if (sscanf (opt->arg, "%d,%d", &Ctrl->In.row, &Ctrl->In.col) < 2 || Ctrl->In.row <= 0 || Ctrl->In.col <= 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error set: Unable to parse row,col: %s\n", opt->arg);
			return GMT_PARSE_ERROR;
		}
		Ctrl->In.mode = SUBPLOT_SET;
	}
	else if (strchr (opt->arg, ',')) {	/* Implicitly called set */
		if (sscanf (opt->arg, "%d,%d", &Ctrl->In.row, &Ctrl->In.col) < 2 || Ctrl->In.row <= 0 || Ctrl->In.col <= 0) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Not a subplot command: %s\n", opt->arg);
			return GMT_PARSE_ERROR;
		}
		Ctrl->In.mode = SUBPLOT_SET;	/* Implicit set command */
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Not a subplot command: %s\n", opt->arg);
		return GMT_PARSE_ERROR;
	}
	opt = opt->next;	/* Position to the next argument */
	if (Ctrl->In.mode == SUBPLOT_END && opt && !(opt->option == 'V' && opt->next == NULL)) {	/* Only -V is optional for end or set */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: subplot end: Unrecognized option: %s\n", opt->arg);
		return GMT_PARSE_ERROR;
	}

	while (opt) {	/* Process all the options given */

		switch (opt->option) {

			case 'A':	/* Enable panel tags and get attributes */
				Ctrl->A.active = true;
				if (Ctrl->In.mode == SUBPLOT_SET) {	/* Override the auto-annotation for this panel */
					strncpy (Ctrl->A.format, opt->arg, GMT_LEN16);
				}
				else {	/* The full enchilada for begin */
					if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
					for (k = 0; k < strlen (opt->arg); k++) {	/* Decode the tag format */
						if (isdigit (opt->arg[k])) {	/* Want number labeling */
							Ctrl->A.nstart = atoi (&opt->arg[k]);	/* Starting number */
							Ctrl->A.mode = SUBPLOT_LABEL_IS_NUMBER;
							strcat (Ctrl->A.format, "%d");
						}
						else if (isalpha (opt->arg[k])) {	/* Want letter labeling */
							Ctrl->A.cstart = opt->arg[k];
							Ctrl->A.mode = SUBPLOT_LABEL_IS_LETTER;
							strcat (Ctrl->A.format, "%c");
						}
						else {	/* Just part of the format string */
							add[0] = opt->arg[k];
							strcat (Ctrl->A.format, add);
						}
					}
					if (!opt->arg[0]) {	/* Just gave -A with no arguments, set defaults */
						Ctrl->A.cstart = 'a';
						Ctrl->A.mode = SUBPLOT_LABEL_IS_LETTER;
						strcpy (Ctrl->A.format, "%c)");
					}
					if (c) {	/* Gave modifiers so we must parse those now */
						c[0] = '+';	/* Restore modifiers */
						/* Modifiers are [+c<dx/dy>][+g<fill>][+j|J<justify>][+o<dx/dy>][+p<pen>][+r|R][+v] */
						if (gmt_get_modifier (opt->arg, 'j', Ctrl->A.placement)) {	/* Inside placement (?) */
							gmt_just_validate (GMT, Ctrl->A.placement, "TL");
							strncpy (Ctrl->A.justify, Ctrl->A.placement, 2);
						}
						else if (gmt_get_modifier (opt->arg, 'J', Ctrl->A.placement)) {	/* Outside placement (?) */
							int pos = gmt_just_decode (GMT, Ctrl->A.placement, PSL_NO_DEF);
							pos = gmt_flip_justify (GMT, pos);
							gmt_just_to_code (GMT, pos, Ctrl->A.justify);
							GMT_Report (GMT->parent, GMT_MSG_DEBUG, "The mirror of %s is %s\n", Ctrl->A.placement, Ctrl->A.justify);
						}
						if (gmt_get_modifier (opt->arg, 'c', string))	/* Clearance for box */
							if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->A.clearance) < 0) n_errors++;
						if (gmt_get_modifier (opt->arg, 'g', Ctrl->A.fill) && Ctrl->A.fill[0]) {
							if (gmt_getfill (GMT, Ctrl->A.fill, &fill)) n_errors++;
						}
						if (gmt_get_modifier (opt->arg, 'o', string))	/* Offset refpoint */
							if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->A.off) < 0) n_errors++;
						if (gmt_get_modifier (opt->arg, 'p', Ctrl->A.pen) && Ctrl->A.pen[0]) {
							if (gmt_getpen (GMT, Ctrl->A.pen, &pen)) n_errors++;
						}
						if (gmt_get_modifier (opt->arg, 'r', string))
							Ctrl->A.roman = GMT_IS_ROMAN_LCASE;
						else if (gmt_get_modifier (opt->arg, 'R', string))
							Ctrl->A.roman = GMT_IS_ROMAN_UCASE;
						if (gmt_get_modifier (opt->arg, 'v', string))	/* Tag order is vertical */
							Ctrl->A.way = GMT_IS_COL_FORMAT;
					}
				}
				break;
				
			case 'C':
				if (Ctrl->In.mode != SUBPLOT_SET) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Option -C is only available when setting the active panel\n");
					n_errors++;
				}
				else {
					Ctrl->C.active = true;
					switch (opt->arg[0]) {
						case 'w':	Ctrl->C.gap[XLO] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						case 'e':	Ctrl->C.gap[XHI] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						case 's':	Ctrl->C.gap[YLO] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						case 'n':	Ctrl->C.gap[YHI] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						default:
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Option -C requires w|e|s|n<clearance>[u]\n");
							n_errors++;
							break;
					}
				}
				break;

			case 'D':
				Ctrl->C.active = true;
				k = 0;
				while (opt->arg[k]) {	/* Expecting -Dx, -Dy, -Dxy or -Dyx */
					if (opt->arg[k] == 'x') Ctrl->D.dir[GMT_X] = -1;
					else if (opt->arg[k] == 'y') Ctrl->D.dir[GMT_Y] = -1;
					else {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error Option -C: Bad axis direction %c\n", opt->arg[k]);
						n_errors++;
					}
					k++;
				}
				break;

			case 'F':
				if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
				if (opt->arg[0]) {	/* Set active canvas size */
					Ctrl->F.active = true;
					switch (opt->arg[0]) {
						case 'f':	Ctrl->F.mode = SUBPLOT_FIGURE; n = 1;	break; /* Figure dimension */
						case 's':	Ctrl->F.mode = SUBPLOT_PANEL;  n = 1;	break; /* Panel dimension */
						default:	Ctrl->F.mode = SUBPLOT_PANEL;  n = 0;	break; /* Panel dimension */
					}
					if ((k = GMT_Get_Values (GMT->parent, &opt->arg[n], Ctrl->F.dim, 2)) < 2) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Option -F requires width and height of plot area.\n");
						n_errors++;
					}
					/* Since GMT_Get_Values returns in default project length unit, convert to inch */
					for (k = 0; k < 2; k++) Ctrl->F.dim[k] *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
				}
				if (c) {	/* Gave paint/pen/debug modifiers */
					c[0] = '+';	/* Restore modifiers */
					if (gmt_get_modifier (opt->arg, 'g', Ctrl->F.fill) && Ctrl->F.fill[0]) {
						if (gmt_getfill (GMT, Ctrl->F.fill, &fill)) n_errors++;
					}
					if (gmt_get_modifier (opt->arg, 'p', Ctrl->F.pen) && Ctrl->F.pen[0]) {
						if (gmt_getpen (GMT, Ctrl->F.pen, &pen)) n_errors++;
					}
					if (gmt_get_modifier (opt->arg, 'd', string))
						Ctrl->F.debug = true;
				}
				break;
			
			case 'L':	/* Layout */
				Ctrl->L.active = true;
				switch (opt->arg[0]) {	/* Type of layout option */
					case 'C':	/* Column setting */
						Ctrl->L.mode |= SUBPLOT_COL_FIXED_X;
						if (opt->arg[1] == 'b') Ctrl->L.annotate[GMT_X] = Ctrl->L.tick[GMT_X] = SUBPLOT_PLACE_AT_MIN;
						else if (opt->arg[1] == 't') Ctrl->L.annotate[GMT_X] = Ctrl->L.tick[GMT_X] = SUBPLOT_PLACE_AT_MAX;
						else Ctrl->L.annotate[GMT_X] = Ctrl->L.tick[GMT_X] = SUBPLOT_PLACE_AT_BOTH;
						if (gmt_get_modifier (opt->arg, 'l', string)) {	/* Want space for x-labels */
							Ctrl->L.has_label[GMT_X] = true;
							if (string[0]) Ctrl->L.label[GMT_X] = strdup (string);
						}
						break;
					case 'R':	/* Row setting */
						Ctrl->L.mode |= SUBPLOT_ROW_FIXED_Y;
						if (opt->arg[1] == 'l') Ctrl->L.annotate[GMT_Y] = Ctrl->L.tick[GMT_Y] = SUBPLOT_PLACE_AT_MIN;
						else if (opt->arg[1] == 'r') Ctrl->L.annotate[GMT_Y] = Ctrl->L.tick[GMT_Y] = SUBPLOT_PLACE_AT_MAX;
						else Ctrl->L.annotate[GMT_Y] = Ctrl->L.tick[GMT_Y] = SUBPLOT_PLACE_AT_BOTH;
						if (gmt_get_modifier (opt->arg, 'l', string)) {	/* Want space for y-labels */
							Ctrl->L.has_label[GMT_Y] = true;
							if (string[0]) Ctrl->L.label[GMT_Y] = strdup (string);
						}
						break;
					default:	/* Regular -LWESNwesn */
						if (gmt_get_modifier (opt->arg, 'l', string)) {	/* Want space for x y-labels */
							if (string[0] == 'x') Ctrl->L.has_label[GMT_X] = true;
							else if (string[0] == 'y') Ctrl->L.has_label[GMT_Y] = true;
							else Ctrl->L.has_label[GMT_X] = Ctrl->L.has_label[GMT_Y] = true;
						}
						if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
						Ctrl->L.axes = strdup (opt->arg);
						if (c) c[0] = '+';	/* Restore modifiers */
						if (strchr (Ctrl->L.axes, 'W')) {
							Ctrl->L.annotate[GMT_Y] |= SUBPLOT_PLACE_AT_MIN;
							Ctrl->L.tick[GMT_Y]     |= SUBPLOT_PLACE_AT_MIN;
						}
						else if (strchr (Ctrl->L.axes, 'w'))
							Ctrl->L.tick[GMT_Y]     |= SUBPLOT_PLACE_AT_MIN;
						if (strchr (Ctrl->L.axes, 'E')) {
							Ctrl->L.annotate[GMT_Y] |= SUBPLOT_PLACE_AT_MAX;
							Ctrl->L.tick[GMT_Y]     |= SUBPLOT_PLACE_AT_MAX;
						}
						else if (strchr (Ctrl->L.axes, 'e'))
							Ctrl->L.tick[GMT_Y]     |= SUBPLOT_PLACE_AT_MAX;
						if (strchr (Ctrl->L.axes, 'S')) {
							Ctrl->L.annotate[GMT_X] |= SUBPLOT_PLACE_AT_MIN;
							Ctrl->L.tick[GMT_X]     |= SUBPLOT_PLACE_AT_MIN;
						}
						else if (strchr (Ctrl->L.axes, 's'))
							Ctrl->L.tick[GMT_X]     |= SUBPLOT_PLACE_AT_MIN;
						if (strchr (Ctrl->L.axes, 'N')) {
							Ctrl->L.annotate[GMT_X] |= SUBPLOT_PLACE_AT_MAX;
							Ctrl->L.tick[GMT_X]     |= SUBPLOT_PLACE_AT_MAX;
						}
						else if (strchr (Ctrl->L.axes, 'n'))
							Ctrl->L.tick[GMT_X]     |= SUBPLOT_PLACE_AT_MAX;
						break;
				}
				/* Axis annotation orientation is a common modifier */
				if (gmt_get_modifier (opt->arg, 'p', string))	/* Want axis-parallel annotations [horizontal] */
					Ctrl->L.parallel = 1;
				/* Panel title is a common modifier */
				if (gmt_get_modifier (opt->arg, 't', string))	/* Want space for panel titles */
					Ctrl->L.ptitle = (string[0] == 'c') ? SUBPLOT_PANEL_COL_TITLE : SUBPLOT_PANEL_TITLE;
				break;
				
			case 'M':	/* Subplot margins */
				Ctrl->M.active = true;
				if (opt->arg[n] == 0) {	/* Accept default margins */
					for (k = 0; k < 4; k++) Ctrl->M.margin[k] = 0.5 * SUBPLOT_CM_TO_INCH;
				}
				else {	/* Process 1, 2, or 4 margin values */
					k = GMT_Get_Values (GMT->parent, opt->arg, Ctrl->M.margin, 4);
					if (k == 1)	/* Same page margin in all directions */
						Ctrl->M.margin[XHI] = Ctrl->M.margin[YLO] = Ctrl->M.margin[YHI] = Ctrl->M.margin[XLO];
					else if (k == 2) {	/* Separate page margins in x and y */
						Ctrl->M.margin[YLO] = Ctrl->M.margin[YHI] = Ctrl->M.margin[XHI];
						Ctrl->M.margin[XHI] = Ctrl->M.margin[XLO];
					}
					else if (k != 4) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -M: Bad number of margins given.\n");
						n_errors++;
					}
					/* Since GMT_Get_Values returns in default project length unit, convert to inch */
					for (k = 0; k < 4; k++) Ctrl->M.margin[k] *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
				}
				break;

			case 'T':	/* Gave figure heading */
				Ctrl->T.active = true;
				Ctrl->T.title = strdup (opt->arg);
				break;
				
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
		opt = opt->next;
	}
	
	if (Ctrl->In.mode == SUBPLOT_BEGIN) {
		if (Ctrl->L.axes && Ctrl->L.mode) gmtlib_str_tolower (Ctrl->L.axes);	/* Used to control the non-annotated axes */
		n_errors += gmt_M_check_condition (GMT, Ctrl->A.mode == SUBPLOT_LABEL_IS_LETTER && Ctrl->A.roman, "Syntax error -A: Cannot select Roman numerals AND letters!\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->N.n_subplots == 0, "Syntax error: Number of RowsxCols is required!\n");
		n_errors += gmt_M_check_condition (GMT, !Ctrl->L.active, "Syntax error -L: Must specify panel layout!\n");
	}
	else if (Ctrl->In.mode == SUBPLOT_SET) {
		n_errors += gmt_M_check_condition (GMT, Ctrl->F.active, "Syntax error -F: Only available for gmt subset begin!\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->L.active, "Syntax error -L: Only available for gmt subset begin!\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->T.active, "Syntax error -T: Only available for gmt subset begin!\n");
		
	}
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_subplot (void *V_API, int mode, void *args) {
	int error = 0, fig;
	char file[PATH_MAX] = {""}, command[GMT_LEN256] = {""};
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_OPTION *options = NULL;
	struct SUBPLOT_CTRL *Ctrl = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));		/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_NORMAL, "Not available in classic mode\n");
		bailout (GMT_NOT_MODERN_MODE);
	}

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the subplot main code ----------------------------*/

	GMT_Report (API, GMT_MSG_NORMAL, "Warning: subplot is experimental and not complete.\n");
	
	fig = gmt_get_current_figure (API);	/* Get current figure number */

	if (Ctrl->In.mode == SUBPLOT_BEGIN) {	/* Determine and save subplot panel attributes */
		unsigned int row, col, k, panel, nx, ny, factor, last_row, last_col, *Lx = NULL, *Ly = NULL;
		uint64_t seg;
		double x, y, plot_dim[2], width = 0.0, height = 0.0, tick_height, annot_height, label_height, title_height, y_header_off = 0.0;
		double *px = NULL, *py = NULL, y_heading, fluff[2] = {0.0, 0.0}, off[2] = {0.0, 0.0}, GMT_LETTER_HEIGHT = 0.736;
		char **Bx = NULL, **By = NULL, *cmd = NULL, axes[3] = {""}, Bopt[GMT_LEN64] = {""};
		char vfile[GMT_STR16] = {""}, xymode = 'r';
		bool add_annot;
		FILE *fp = NULL;
		
		/* Determine if the subplot itself is an overlay */
		sprintf (file, "%s/gmt_%d.ps-", API->gwf_dir, fig);
		if (!access (file, F_OK))	{	/* Plot file already exists, so enter overlay mode if -X -Y not set */
			if (GMT->common.X.mode == 'a' && GMT->common.Y.mode == 'a')
				xymode = 'a';
			else {
				if (!GMT->common.X.active) GMT->current.setting.map_origin[GMT_X] = 0.0;
				if (!GMT->common.Y.active) GMT->current.setting.map_origin[GMT_Y] = 0.0;
			}
		}
		else {	/* Start of plot.  See if we gave absolute or relative coordinates in -X -Y */
			if (GMT->common.X.mode == 'a' && GMT->common.Y.mode == 'a')
				xymode = 'a';
		}

		if (xymode == 'a') gmt_M_memcpy (off, GMT->current.setting.map_origin, 2, double);
		sprintf (file, "%s/gmt_%d.subplot", API->gwf_dir, fig);
		if (!access (file, F_OK))	{	/* Subplot information file already exists */
			GMT_Report (API, GMT_MSG_NORMAL, "Error: Subplot information file already exists: %s\n", file);
			Return (GMT_RUNTIME_ERROR);
		}
		tick_height   = MAX(0,GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER]);	/* Allow for axis ticks */
		annot_height  = (GMT_LETTER_HEIGHT * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH) + MAX (0.0, GMT->current.setting.map_annot_offset[GMT_PRIMARY]);	/* Allow for space between axis and annotations */
		label_height  = (GMT_LETTER_HEIGHT * GMT->current.setting.font_label.size / PSL_POINTS_PER_INCH) + MAX (0.0, GMT->current.setting.map_label_offset);
		title_height = (GMT_LETTER_HEIGHT * GMT->current.setting.font_title.size / PSL_POINTS_PER_INCH) + GMT->current.setting.map_title_offset;
		y_header_off = GMT->current.setting.map_heading_offset;
		/* Get plot/media area dimensions */
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: origin          = %g/%g\n", off[GMT_X], off[GMT_Y]);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: rows            = %d\n", Ctrl->N.dim[GMT_Y]);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: columns         = %d\n", Ctrl->N.dim[GMT_X]);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: height          = %g\n", height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: width           = %g\n", width);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: tick_height     = %g\n", tick_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: annot_height    = %g\n", annot_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: label_height    = %g\n", label_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: title_height    = %g\n", title_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: panel margin    = %g/%g/%g/%g\n", Ctrl->M.margin[XLO], Ctrl->M.margin[XHI], Ctrl->M.margin[YLO], Ctrl->M.margin[YHI]);
		/* Shrink these if media margins were requested */
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Start: fluff = {%g, %g}\n", fluff[GMT_X], fluff[GMT_Y]);
		if (Ctrl->M.active) {	/* Remove space used by inside panel margins */
			fluff[GMT_X] += (Ctrl->N.dim[GMT_X] - 1) * (Ctrl->M.margin[XLO] + Ctrl->M.margin[XHI]);
			fluff[GMT_Y] += (Ctrl->N.dim[GMT_Y] - 1) * (Ctrl->M.margin[YLO] + Ctrl->M.margin[YHI]);
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After inside panel margins: fluff = {%g, %g}\n", fluff[GMT_X], fluff[GMT_Y]);
		/* Limit tickmarks to 1 or 2 axes per row or per panel */
		if (Ctrl->L.tick[GMT_Y]) nx = (Ctrl->L.tick[GMT_Y] == SUBPLOT_PLACE_AT_BOTH) ? 2 : 1; else nx = 0;
		if ((Ctrl->L.mode & SUBPLOT_ROW_FIXED_Y) == 0) {
			nx *= (Ctrl->N.dim[GMT_X] - 1);	/* Each column needs separate y-axis [and labels] */
			fluff[GMT_X] += nx * tick_height;
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d row ticks: fluff = {%g, %g}\n", nx, fluff[GMT_X], fluff[GMT_Y]);
		/* Limit annotations/labels to 1 or 2 axes per row or per panel */
		if (Ctrl->L.annotate[GMT_Y]) nx = (Ctrl->L.annotate[GMT_Y] == SUBPLOT_PLACE_AT_BOTH) ? 2 : 1; else nx = 0;
		if ((Ctrl->L.mode & SUBPLOT_ROW_FIXED_Y) == 0) {
			nx *= (Ctrl->N.dim[GMT_X] - 1);	/* Each column needs separate y-axis [and labels] */
			fluff[GMT_X] += nx * annot_height;
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d row annots: fluff = {%g, %g}\n", nx, fluff[GMT_X], fluff[GMT_Y]);
		if (Ctrl->L.has_label[GMT_Y])
			fluff[GMT_X] += nx * label_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d row labels: fluff = {%g, %g}\n", nx, fluff[GMT_X], fluff[GMT_Y]);
		/* Limit annotations/labels to 1 or 2 axes per column or per panel */
		if (Ctrl->L.tick[GMT_X]) ny = (Ctrl->L.tick[GMT_X] == SUBPLOT_PLACE_AT_BOTH) ? 2 : 1; else ny = 0;
		if (ny == 2) y_header_off += tick_height;
		factor = (Ctrl->L.mode & SUBPLOT_COL_FIXED_X) ? 1 : Ctrl->N.dim[GMT_Y];		/* Each row may need separate x-axis ticks */
		ny *= (factor - 1);
		fluff[GMT_Y] += ny * tick_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d col ticks: fluff = {%g, %g}\n", ny, fluff[GMT_X], fluff[GMT_Y]);
		if (Ctrl->L.annotate[GMT_X]) ny = (Ctrl->L.annotate[GMT_X] == SUBPLOT_PLACE_AT_BOTH) ? 2 : 1; else ny = 0;
		if (ny == 2) y_header_off += annot_height;
		factor = (Ctrl->L.mode & SUBPLOT_COL_FIXED_X) ? 1 : Ctrl->N.dim[GMT_Y];		/* Each row may need separate x-axis [and labels] */
		ny *= (factor - 1);
		fluff[GMT_Y] += ny * annot_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d col annot: fluff = {%g, %g}\n", ny, fluff[GMT_X], fluff[GMT_Y]);
		if (Ctrl->L.has_label[GMT_X]) {
			fluff[GMT_Y] += ny * label_height;
			y_header_off += label_height;
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d col labels: fluff = {%g, %g}\n", ny, fluff[GMT_X], fluff[GMT_Y]);
		if (Ctrl->L.ptitle == SUBPLOT_PANEL_TITLE)
			fluff[GMT_Y] += (factor-1) * title_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d panel titles: fluff = {%g, %g}\n", factor, fluff[GMT_X], fluff[GMT_Y]);
		if (Ctrl->F.mode == SUBPLOT_FIGURE) {	/* Got figure dimension, compute subplot dimension */
			plot_dim[GMT_X] = (Ctrl->F.dim[GMT_X] - fluff[GMT_X]) / Ctrl->N.dim[GMT_X];
			plot_dim[GMT_Y] = (Ctrl->F.dim[GMT_Y] - fluff[GMT_Y]) / Ctrl->N.dim[GMT_Y];
		}
		else {	/* Got subplot dimension, compute media dimension */
			plot_dim[GMT_X] = Ctrl->F.dim[GMT_X];
			plot_dim[GMT_Y] = Ctrl->F.dim[GMT_Y];
			Ctrl->F.dim[GMT_X] = Ctrl->N.dim[GMT_X] * plot_dim[GMT_X] + fluff[GMT_X];
			Ctrl->F.dim[GMT_Y] = Ctrl->N.dim[GMT_Y] * plot_dim[GMT_Y] + fluff[GMT_Y];
		}
		/* Plottable area: */
		width  = Ctrl->F.dim[GMT_X];
		height = Ctrl->F.dim[GMT_Y];
		y_heading = height + y_header_off + Ctrl->M.margin[YHI];
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Figure dimensions: {%g, %g}\n", Ctrl->F.dim[GMT_X], Ctrl->F.dim[GMT_Y]);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Panel dimensions:  {%g, %g}\n", plot_dim[GMT_X], plot_dim[GMT_Y]);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Main heading BC point: %g %g\n", 0.5 * width, y_heading);

		/* Allocate panel info array */
		px = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_X], double);
		py = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_Y], double);
		Bx = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_X], char *);
		By = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_Y], char *);
		Lx = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_X], unsigned int);
		Ly = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_Y], unsigned int);
		
		if (Ctrl->A.roman) {	/* Replace %d with %s since roman numerals are typset as strings */
			char *c = strchr (Ctrl->A.format, '%');
			c[1] = 's';
		}
		
		last_row = Ctrl->N.dim[GMT_Y] - 1;
		last_col = Ctrl->N.dim[GMT_X] - 1;
		
		if (Ctrl->F.debug) {	/* Must draw helper lines so we need to allocate a dataset */
			uint64_t dim[4] = {1, Ctrl->N.n_subplots, 4, 2};	/* One segment polygon per panel */
			if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Subplot: Unable to allocate a dataset\n");
				Return (error);
			}
		}
		
		/* Need to determine how many y-labels and annotations for each row since it affects panel size */

		y = Ctrl->F.dim[GMT_Y];	/* Start off at top edge of figure area */
		last_row = Ctrl->N.dim[GMT_Y] - 1;
		for (row = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each row of panels */
			axes[0] = axes[1] = k = 0;
			if (row) y -= Ctrl->M.margin[YHI];
			if (Ctrl->F.debug) {	/* All rows share this upper y */
				for (col = 0; col < Ctrl->N.dim[GMT_X]; col++) {	/* For each col of panels */
					seg = row * Ctrl->N.dim[GMT_X] + col;
					D->table[0]->segment[seg]->data[GMT_Y][2] = D->table[0]->segment[seg]->data[GMT_Y][3] = y + off[GMT_Y];
				}
			}
			if ((row == 0 && (Ctrl->L.ptitle == SUBPLOT_PANEL_COL_TITLE)) || (Ctrl->L.ptitle == SUBPLOT_PANEL_TITLE)) {
				if (row) y -= title_height;	/* Make space for panel title */
			}
			if (Ctrl->L.mode && ((row == 0 || ((Ctrl->L.mode & SUBPLOT_COL_FIXED_X) == 0)) && (Ctrl->L.annotate[GMT_X] & SUBPLOT_PLACE_AT_MAX)))	/* Need annotation at N */
				add_annot = true;
			else if (Ctrl->L.mode == 0 && Ctrl->L.axes && strchr (Ctrl->L.axes, 'N'))
				add_annot = true;
			else if (Ctrl->L.axes && strchr (Ctrl->L.axes, 'n'))
				add_annot = false;
			else
				add_annot = false;
			if (add_annot) {	/* Need annotation at N */
				axes[k++] = 'N';
				if (row) y -= (annot_height + tick_height);
				if (Ctrl->L.has_label[GMT_X]) {
					if (row) y -= label_height;	/* Also has label at N */
					Ly[row] |= SUBPLOT_PLACE_AT_MAX;
				}
			}
			else if (Ctrl->L.axes && strchr (Ctrl->L.axes, 'n')) {
				axes[k++] = 'n';
				if (row) y -= tick_height;
			}
			else if (Ctrl->L.axes && strchr (Ctrl->L.axes, 't'))
				axes[k++] = 't';
			y -= plot_dim[GMT_Y];	/* Now at correct y for this panel */
			py[row] = y;
			if (Ctrl->L.mode && (row == last_row || ((Ctrl->L.mode & SUBPLOT_COL_FIXED_X) == 0)) && (Ctrl->L.annotate[GMT_X] & SUBPLOT_PLACE_AT_MIN))	/* Need annotation at S */
				add_annot = true;
			else if (Ctrl->L.mode == 0 && Ctrl->L.axes && strchr (Ctrl->L.axes, 'S'))
				add_annot = true;
			else
				add_annot = false;
			if (add_annot) {
				axes[k++] = 'S';
				if (row < last_row) y -= (annot_height + tick_height);
				if (Ctrl->L.has_label[GMT_X])
					if (row < last_row)y -= label_height;	/* Also has label at S */
				Ly[row] |= SUBPLOT_PLACE_AT_MIN;
			}
			else if (Ctrl->L.axes && strchr (Ctrl->L.axes, 's')) {
				axes[k++] = 's';
				if (row < last_row) y -= tick_height;
			}
			else if (Ctrl->L.axes && strchr (Ctrl->L.axes, 'b'))
				axes[k++] = 'b';
			By[row] = strdup (axes);
			if (Ctrl->F.debug) {	/* ALl rows share this lower y */
				for (col = 0; col < Ctrl->N.dim[GMT_X]; col++) {	/* For each col of panels */
					seg = row * Ctrl->N.dim[GMT_X] + col;
					D->table[0]->segment[seg]->data[GMT_Y][0] = D->table[0]->segment[seg]->data[GMT_Y][1] = y + off[GMT_Y];
				}
			}
			if (row < last_row) y -= Ctrl->M.margin[YLO];
		}
		x = 0.0;	/* Start off at left edge of plot area */
		last_col = Ctrl->N.dim[GMT_X] - 1;
		for (col = 0; col < Ctrl->N.dim[GMT_X]; col++) {	/* For each col of panels */
			axes[0] = axes[1] = k = 0;
			if (col) x += Ctrl->M.margin[XLO];
			if (Ctrl->F.debug) {	/* All cols share this left x */
				for (row = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each col of panels */
					seg = row * Ctrl->N.dim[GMT_X] + col;
					D->table[0]->segment[seg]->data[GMT_X][0] = D->table[0]->segment[seg]->data[GMT_X][3] = x + off[GMT_X];
				}
			}
			if (Ctrl->L.mode && (col == 0 || ((Ctrl->L.mode & SUBPLOT_ROW_FIXED_Y) == 0)) && (Ctrl->L.annotate[GMT_X] & SUBPLOT_PLACE_AT_MIN))	/* Need annotation at W */
				add_annot = true;
			else if (Ctrl->L.mode == 0 && Ctrl->L.axes && strchr (Ctrl->L.axes, 'W'))
				add_annot = true;
			else
				add_annot = false;
			if (add_annot) {
				axes[k++] = 'W';
				if (col) x += (annot_height + tick_height);
				if (Ctrl->L.has_label[GMT_Y]) {
					if (col) x += label_height;	/* Also has label at W */
					Lx[col] |= SUBPLOT_PLACE_AT_MIN;
				}
			}
			else if (Ctrl->L.axes && strchr (Ctrl->L.axes, 'w')) {
				if (col) x += tick_height;
				axes[k++] = 'w';
			}
			else if (Ctrl->L.axes && strchr (Ctrl->L.axes, 'l'))
				axes[k++] = 'l';
			px[col] = x;	/* Now at correct x for this panel */
			x += plot_dim[GMT_X];
			if (Ctrl->L.mode && (col == last_col || ((Ctrl->L.mode & SUBPLOT_ROW_FIXED_Y) == 0)) && (Ctrl->L.annotate[GMT_Y] & SUBPLOT_PLACE_AT_MAX))	/* Need annotation at E */
				add_annot = true;
			else if (Ctrl->L.mode == 0 && Ctrl->L.axes && strchr (Ctrl->L.axes, 'E'))
				add_annot = true;
			else
				add_annot = false;
			if (add_annot) {
				axes[k++] = 'E';
				if (col < last_col) x += (annot_height + tick_height);
				if (Ctrl->L.has_label[GMT_Y]) {
					if (col < last_col) x += label_height;	/* Also has label at E */
					Lx[col] |= SUBPLOT_PLACE_AT_MAX;
				}
			}
			else if (Ctrl->L.axes && strchr (Ctrl->L.axes, 'e')) {
				if (col < last_col) x += tick_height;
				axes[k++] = 'e';
			}
			else if (Ctrl->L.axes && strchr (Ctrl->L.axes, 'r'))
				axes[k++] = 'r';
			if (Ctrl->F.debug) {	/* ALl cols share this right x */
				for (row = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each col of panels */
					seg = row * Ctrl->N.dim[GMT_X] + col;
					D->table[0]->segment[seg]->data[GMT_X][1] = D->table[0]->segment[seg]->data[GMT_X][2] = x + off[GMT_X];
				}
			}
			if (col < last_col) x += Ctrl->M.margin[XHI];
			Bx[col] = strdup (axes);
		}
		
		/* Write out the subplot information file */
		
		fp = fopen (file, "w");
		fprintf (fp, "# subplot panel information file\n");
		cmd = GMT_Create_Cmd (API, options);
		fprintf (fp, "# Command: %s %s\n", THIS_MODULE_NAME, cmd);
		gmt_M_free (GMT, cmd);
		if (Ctrl->T.active) fprintf (fp, "# HEADING: %g %g %s\n", 0.5 * width, y_heading, Ctrl->T.title);
		fprintf (fp, "# ORIGIN: %g %g\n", off[GMT_X], off[GMT_Y]);
		fprintf (fp, "# PARALLEL: %d\n", Ctrl->L.parallel);
		if (Ctrl->D.dir[GMT_X] == -1 || Ctrl->D.dir[GMT_Y] == -1) fprintf (fp, "# DIRECTION: %d %d\n", Ctrl->D.dir[GMT_X], Ctrl->D.dir[GMT_Y]);
		fprintf (fp, "#panel\trow\tcol\tnrow\tncol\tx0\ty0\tw\th\ttag\ttag_dx\ttag_dy\ttag_clearx\ttag_cleary\ttag_pos\ttag_just\ttag_fill\ttag_pen\tBframe\tBx\tBy\n");
		for (row = panel = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each row of panels */
			for (col = 0; col < Ctrl->N.dim[GMT_X]; col++, panel++) {	/* For each col of panels */
				k = (Ctrl->A.way == GMT_IS_COL_FORMAT) ? col * Ctrl->N.dim[GMT_Y] + row : row * Ctrl->N.dim[GMT_X] + col;
				fprintf (fp, "%d\t%d\t%d\t%d\t%d\t%.4f\t%.4f\t%.4f\t%.4f\t", panel, row, col, Ctrl->N.dim[GMT_Y], Ctrl->N.dim[GMT_X], px[col], py[row], plot_dim[GMT_X], plot_dim[GMT_Y]);
				if (Ctrl->A.active) {
					if (Ctrl->A.mode == SUBPLOT_LABEL_IS_NUMBER) {
						if (Ctrl->A.roman) {
							char roman[GMT_LEN32] = {""};
							(void)gmt_arabic2roman (Ctrl->A.nstart + k, roman, GMT_LEN32, Ctrl->A.roman == GMT_IS_ROMAN_LCASE);
							fprintf (fp, Ctrl->A.format, roman);
						}
						else
							fprintf (fp, Ctrl->A.format, Ctrl->A.nstart + k);
					}
					else
						fprintf (fp, Ctrl->A.format, Ctrl->A.cstart + k);
					fprintf (fp, "\t%g\t%g\t%g\t%g\t%s\t%s", Ctrl->A.off[GMT_X], Ctrl->A.off[GMT_Y], Ctrl->A.clearance[GMT_X], Ctrl->A.clearance[GMT_Y], Ctrl->A.placement, Ctrl->A.justify);
					fprintf (fp, "\t%s\t%s", Ctrl->A.fill, Ctrl->A.pen);
				}
				else
					fprintf (fp, "-\t0\t0\t0\t0\tBL\tBL\t-\t-");
				/* Now the four -B settings items placed between GMT_ASCII_GS characters */
				fprintf (fp, "\t%c%s%s", GMT_ASCII_GS, Bx[col], By[row]);	/* These are the axes to draw/annotate for this panel */
				fprintf (fp,"%c", GMT_ASCII_GS); 	/* Next is x labels,  Either given of just XLABEL */
				if (Ly[row] && Ctrl->L.label[GMT_X]) fprintf (fp,"%s", Ctrl->L.label[GMT_X]); 
				fprintf (fp, "%c", GMT_ASCII_GS); 	/* Next is y labels,  Either given of just YLABEL */
				if (Lx[col] && Ctrl->L.label[GMT_Y]) fprintf (fp, "%s", Ctrl->L.label[GMT_Y]);
				fprintf (fp, "%c\n", GMT_ASCII_GS);
			}
			gmt_M_str_free (By[row]);
		}
		fclose (fp);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Wrote %d panel settings to information file %s\n", panel, file);
		
		/* Start the subplot with a blank canvas and place the optional title */
		
		if (Ctrl->F.fill[0] != '-' && Ctrl->F.pen[0] != '-')	/* Need to fill and the canvas box */
			sprintf (Bopt, " -B+g%s -B0 --MAP_FRAME_PEN=%s", Ctrl->F.fill, Ctrl->F.pen);
		else if (Ctrl->F.fill[0] != '-')	/* Need to just fill the canvas box */
			sprintf (Bopt, " -B+g%s", Ctrl->F.fill);
		else if (Ctrl->F.pen[0] != '-')	/* Need to just draw the canvas box */
			sprintf (Bopt, " -B0 --MAP_FRAME_PEN=%s", Ctrl->F.pen);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot Bopt: [%s]\n", Bopt);
			
		if (Ctrl->T.title) {	/* Must call pstext to place the heading */
			uint64_t dim[3] = {1, 1, 1};	/* A single record */
			struct GMT_TEXTSET *T = NULL;
			if ((T = GMT_Create_Data (API, GMT_IS_TEXTSET, GMT_IS_NONE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Subplot: Unable to allocate a textset\n");
				Return (error);
			}
			sprintf (command, "%g %g %s", 0.5 * width, y_heading, Ctrl->T.title);
			T->table[0]->segment[0]->data[0] = strdup (command);
			T->n_records = T->table[0]->n_records = T->table[0]->segment[0]->n_rows = 1;
			if (GMT_Open_VirtualFile (API, GMT_IS_TEXTSET, GMT_IS_NONE, GMT_IN, T, vfile) != GMT_NOERROR) {
				Return (API->error);
			}
			sprintf (command, "-R0/%g/0/%g -Jx1i -P -N -F+jBC+f%s %s -X%c%gi -Y%c%gi --GMT_HISTORY=false",
				width, height, gmt_putfont (GMT, &GMT->current.setting.font_heading), vfile, xymode, GMT->current.setting.map_origin[GMT_X], xymode, GMT->current.setting.map_origin[GMT_Y]);
			if (Bopt[0] == ' ') strcat (command, Bopt);
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot to pstext: %s\n", command);
			if (GMT_Call_Module (API, "pstext", GMT_MODULE_CMD, command) != GMT_OK)	/* Plot the cancas with heading */
				Return (API->error);
			if (GMT_Destroy_Data (API, &T) != GMT_OK)
				Return (API->error);
		}
		else {	/* psxy will do, since nothing is plotted (except for possibly the canvas fill/outline) */
			sprintf (command, "-R0/%g/0/%g -Jx1i -P -T -X%c%gi -Y%c%gi --GMT_HISTORY=false", width, height, xymode, GMT->current.setting.map_origin[GMT_X], xymode, GMT->current.setting.map_origin[GMT_Y]);
			if (Bopt[0]) strcat (command, Bopt);
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot to psxy: %s\n", command);
			if (GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, command) != GMT_OK)	/* Plot the cancas with heading */
				Return (API->error);
		}
		
		if (Ctrl->F.debug) {	/* Write a debug file with panel polygons for use by "gmt subplot end" */
			bool save = GMT->current.setting.io_header[GMT_OUT];
			sprintf (file, "%s/gmt_%d.subplotdebug", API->gwf_dir, fig);
			sprintf (command, "0/%g/0/%g", Ctrl->F.dim[GMT_X] + GMT->current.setting.map_origin[GMT_X], Ctrl->F.dim[GMT_Y] + GMT->current.setting.map_origin[GMT_Y]);	/* Save page region */
			GMT_Set_Comment (API, GMT_IS_DATASET, GMT_COMMENT_IS_TEXT, command, D);
			gmt_set_tableheader (API->GMT, GMT_OUT, true);	/* So header is written */
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, 0, NULL, file, D) != GMT_NOERROR) {
				Return (API->error);
			}
			gmt_set_tableheader (API->GMT, GMT_OUT, save);	/* Restore the state of affairs */
		}

		/* Free up memory */
		
		for (col = 0; col < Ctrl->N.dim[GMT_X]; col++) gmt_M_str_free (Bx[col]);
		gmt_M_free (GMT, px);
		gmt_M_free (GMT, py);
		gmt_M_free (GMT, Bx);
		gmt_M_free (GMT, By);
		gmt_M_free (GMT, Lx);
		gmt_M_free (GMT, Ly);
	}
	else if (Ctrl->In.mode == SUBPLOT_SET) {	/* SUBPLOT_SET */
		if ((error = gmt_set_current_panel (API, fig, Ctrl->In.row, Ctrl->In.col, Ctrl->C.gap, Ctrl->A.format, 1)))
			Return (error)
	}
	else {	/* SUBPLOT_END */
		int k;
		char *wmode[2] = {"w","a"}, vfile[GMT_STR16] = {""};
		FILE *fp = NULL;
		
		/* Must force PSL_completion to run, if set */
		if ((k = gmt_set_psfilename (GMT)) == GMT_NOTSET) {	/* Get hidden file name for PS */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: No workflow directory\n");
			Return (GMT_ERROR_ON_FOPEN);
		}
		if ((fp = PSL_fopen (GMT->PSL, GMT->current.ps.filename, wmode[k])) == NULL) {	/* Must open inside PSL DLL */
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open %s with mode %s\n", GMT->current.ps.filename, wmode[k]);
			Return (GMT_ERROR_ON_FOPEN);
		}
		PSL_command (GMT->PSL, "PSL_completion /PSL_completion {} def\n");	/* Run then make it a null function */
		if (PSL_fclose (GMT->PSL)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Unable to close hidden PS file %s!\n", GMT->current.ps.filename);
			Return (GMT_RUNTIME_ERROR);
		}
		sprintf (file, "%s/gmt_%d.subplot", API->gwf_dir, fig);
		gmt_remove_file (GMT, file);
		sprintf (file, "%s/gmt_%d.panel", API->gwf_dir, fig);
		gmt_remove_file (GMT, file);
		/* Check if we should draw debug lines */
		sprintf (file, "%s/gmt_%d.subplotdebug", API->gwf_dir, fig);
		if (!access (file, R_OK)) {	/* Yes, must draw debug lines on top */
			if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, 0, NULL, file, NULL)) == NULL) {
				Return (API->error);
			}
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POLY, GMT_IN, D, vfile) != GMT_NOERROR) {
				Return (API->error);
			}
			sprintf (command, "-R%s -Jx1i %s -L -Wfaint,red -Xa0i -Ya0i --GMT_HISTORY=false", D->table[0]->header[0], vfile);
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot to psxy 4 debug: %s\n", command);
			if (GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, command) != GMT_OK)	/* Plot the canvas with heading */
				Return (API->error);
			if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
				Return (API->error);
			}
			gmt_remove_file (GMT, file);
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Removed panel and subplot files\n");
		gmt_M_memset (&GMT->current.plot.panel, 1, struct GMT_SUBPLOT);
	}
		
	Return (error);
}
