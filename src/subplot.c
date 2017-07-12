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
 *	gmt subplot begin -N<nrows>/<ncols> [-A<labels>] [-F<WxH>] [-L<layout>] [-M[m|p]<margins>] [-T<title>] [-V]
 *	gmt subplot end [-V]
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"subplot"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Set multi-panel figure attributes under a GMT modern mode session"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

/* Control structure for subplot */

#define SUBPLOT_CM_TO_INCH 	(1.0/2.54)
#define SUBPLOT_BEGIN		1
#define SUBPLOT_END		0
#define SUBPLOT_SET		2
#define SUBPLOT_MEDIA		1
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
	struct A {	/* -A[<letter>|<number>][+c][+j|J<pos>][+o<offset>][+r|R][+g<fill>][+p<pen>] */
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
	} A;
	struct F {	/* -F[<width>[u]/<height>[u]][+g<fill>][+p<pen>][+d] */
		bool active;
		bool debug;
		double dim[2];
		char fill[GMT_LEN64];
		char pen[GMT_LEN64];
	} F;
	struct G {	/* -G[side]<gap>[u]  */
		bool active;
		double gap[4];
	} G;
	struct L {	/* -L<layout> */
		bool active;
		bool has_label[2];	/* True if x and y labels */
		unsigned int mode;	/* 0 for -L, 1 for -LC, 2 for -LR (3 for both) */
		char *axes;		/* WESNwesn for -L */
		char *label[2];		/* The constant x and y labels */
		unsigned int ptitle;	/* 0 = no panel titles, 1 = column titles, 2 = all panel titles */
		unsigned annotate[2];	/* 1 if only l|r or t|b, 0 for both */
		unsigned tick[2];		/* 1 if only l|r or t|b, 0 for both */
	} L;
	struct M {	/* -M[type]<margin>[u] | <xmargin>[u]/<ymargin>[u]  | <wmargin>[u]/<emargin>[u]/<smargin>[u]/<nmargin>[u]  */
		bool active[2];
		double margin[2][4];
	} M;
	struct N {	/* NrowsxNcolumns */
		bool active;
		unsigned int dim[2];
		unsigned int n_panels;
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
	C->A.fill[0] = C->A.pen[0] = '-';	/* No fill or outline */
	C->F.fill[0] = C->F.pen[0] = '-';	/* No fill or outline */
	C->F.dim[GMT_X] = GMT->current.setting.ps_page_size[GMT_X] / PSL_POINTS_PER_INCH;
	C->F.dim[GMT_Y] = GMT->current.setting.ps_page_size[GMT_Y] / PSL_POINTS_PER_INCH;
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
	GMT_Message (API, GMT_TIME_NONE, "usage: subplot <nrows>x<ncols> [-A<labelinfo>] [-F[<width>/<height>][+f<fill>][+p<pen>][+d]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-L<layout>] [-M[m[|p]]<margins>] [-T<title>] [%s]\n\n", GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<nrows>x<ncols> is the number of rows and columns of panels in this figure.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Specify tagging of each panel.  Append either a number or letter [a].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This sets the tag of the top-left panel and others follow sequentially.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Surround number or letter by parentheses on any side if these should be typeset.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Panels are numbered across rows.  Append +c to number down columns instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +j|J<refpoint> to specify where the tag should be plotted in the panel [TL].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: +j sets justification = <refpoint> while +J selects the mirror opposite.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +o<dx>[/<dy>] to offset tag in direction implied by <justify> [0/0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +r to set number using Roman numerals; use +R for uppercase [arabic].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +g to fill the textbox with <fill> color [no fill].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p to draw the outline of the textbox using selected pen [no outline].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify dimension of area that the multi-panel figure may occupy [current page].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append +g<fill> to paint canvas and +p<pen> to draw outline.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +d to draw faint red lines outlining each panel area (for debugging).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set panel layout. May be set once (-L) or separately for rows (-LR) and columns (-LC):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -L:  Append WESNwesn to indicate which panel frames should be drawn and annotated.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +l to make space for axes labels; applies equally to all panels [no labels].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       Append x to only use x-labels or y for only y-labels [both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +t to make space for all panel titles; use +tc for top row titles only [no panel titles].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -LC: Each panel Column share common x-range. Only first (top) and last (bottom) rows are annotated.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append t or b to select only one of those two rows instead [both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +l if annotated x-axes should have a label [none]; optionally append the label if fixed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +t to make space for all panel titles; use +tc for top row titles only [no panel titles].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -LR: Each panel Row share common y-range. Only first (left) and last (right) columns are annotated.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append l or r to select only one of those two columns [both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +l if annotated y-axes will have a label [none]; optionally append the label if fixed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Specify the two types of margins:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -M[p] adds space around each panel. Append  a uniform <margin>, separate x and y <xmargin>/<ymargin>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     or individual <wmargin>/<emargin>/<smargin>/<nmargin> for each side [0.5c].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Mm sets the media whitespace around each figure. Append 1, 2, or 4 margins [1c].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify a main heading to be centered above all the panels [none].\n");
	GMT_Option (API, "V");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SUBPLOT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, n, type;
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
		Ctrl->N.n_panels = Ctrl->N.dim[GMT_X] * Ctrl->N.dim[GMT_Y];
		Ctrl->N.active = true;
	}
	else if (!strncmp (opt->arg, "end", 3U))
		Ctrl->In.mode = SUBPLOT_END;
	else if (strchr (opt->arg, ',') && sscanf (opt->arg, "%d,%d", &Ctrl->In.row, &Ctrl->In.col) == 2 && Ctrl->In.row > 0 && Ctrl->In.col > 0) {
		Ctrl->In.mode = SUBPLOT_SET;
		return GMT_NOERROR;
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Not a subplot command: %s\n", opt->arg);
		return GMT_PARSE_ERROR;
	}
	opt = opt->next;	/* Position to the next argument */
	if ((Ctrl->In.mode == SUBPLOT_END || Ctrl->In.mode == SUBPLOT_SET) && opt && !(opt->option == 'V' && opt->next == NULL)) {	/* Only -V is optional for end or set */
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: subplot end|set: Unrecognized option: %s\n", opt->arg);
		return GMT_PARSE_ERROR;
	}

	/* Here we are either doing SUBPLOT_BEGIN or just SUBPLOT_END with -V */
	
	while (opt) {	/* Process all the options given */

		switch (opt->option) {

			case 'A':	/* Enable panel tags and get attributes */
				Ctrl->A.active = true;
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
				if (c) {	/* Gave modifiers so we must parse those now */
					c[0] = '+';	/* Restore modifiers */
					/* modifiers are [+c][+g<fill>][+j|J<justify>][+p<pen>][+r|R] */
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
					if (gmt_get_modifier (opt->arg, 'c', string))	/* Tag order is by columns */
						Ctrl->A.way = GMT_IS_COL_FORMAT;
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
				}
				if (!opt->arg[0]) {	/* Just gave -A with no arguments, set defaults */
					Ctrl->A.cstart = 'a';
					Ctrl->A.mode = SUBPLOT_LABEL_IS_LETTER;
					strcpy (Ctrl->A.format, "%c)");
				}
				break;
				
			case 'F':
				if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
				if (opt->arg[0]) {	/* Set active canvas size */
					Ctrl->F.active = true;
					if ((k = GMT_Get_Values (GMT->parent, opt->arg, Ctrl->F.dim, 2)) < 2) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Option -F requires width and height of plot area.\n");
						n_errors++;
					}
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
			
			case 'G':
				if (Ctrl->In.mode != SUBPLOT_SET) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Option -G is only available when setting the active panel\n");
					n_errors++;
				}
				else {
					switch (opt->arg[0]) {
						case 'w':	Ctrl->G.gap[XLO] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						case 'e':	Ctrl->G.gap[XHI] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						case 's':	Ctrl->G.gap[YLO] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						case 'n':	Ctrl->G.gap[YHI] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						default:
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Option -G requires w|e|s|n<gap>[u]\n");
							n_errors++;
							break;
					}
				}
				
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
				/* Panel title is a common modifier */
				if (gmt_get_modifier (opt->arg, 't', string))	/* Want space for panel titles */
					Ctrl->L.ptitle = (string[0] == 'c') ? SUBPLOT_PANEL_COL_TITLE : SUBPLOT_PANEL_TITLE;
				break;
				
			case 'M':	/* Panel and media margins */
				switch (opt->arg[0]) {
					case 'm':	type = SUBPLOT_MEDIA; n = 1;	break; /* Media margin */
					case 'p':	type = SUBPLOT_PANEL; n = 1;	break; /* Panel margin */
					default:	type = SUBPLOT_PANEL; n = 0;	break; /* Panel margin */
				}
				Ctrl->M.active[type] = true;
				if (opt->arg[n] == 0) {	/* Accept default margins */
					for (k = 0; k < 4; k++) Ctrl->M.margin[type][k] = ((type == SUBPLOT_MEDIA) ? 1.0 : 0.5) * SUBPLOT_CM_TO_INCH;
				}
				else {	/* Process 1, 2, or 4 margin values */
					k = GMT_Get_Values (GMT->parent, &opt->arg[1], Ctrl->M.margin[type], 4);
					if (k == 1)	/* Same page margin in all directions */
						Ctrl->M.margin[type][XHI] = Ctrl->M.margin[type][YLO] = Ctrl->M.margin[type][YHI] = Ctrl->M.margin[type][XLO];
					else if (k == 2) {	/* Separate page margins in x and y */
						Ctrl->M.margin[type][YLO] = Ctrl->M.margin[type][YHI] = Ctrl->M.margin[type][XHI];
						Ctrl->M.margin[type][XHI] = Ctrl->M.margin[type][XLO];
					}
					else if (k != 4) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error -M: Bad number of margins given.\n");
						n_errors++;
					}
					/* Since GMT_Get_Values returns in default project length unit, convert to inch */
					for (k = 0; k < 4; k++) Ctrl->M.margin[type][k] *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
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
		n_errors += gmt_M_check_condition (GMT, Ctrl->N.n_panels == 0, "Syntax error: Number of RowsxCols is required!\n");
		n_errors += gmt_M_check_condition (GMT, !Ctrl->L.active, "Syntax error -L: Must specify panel layout!\n");
	}
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_subplot (void *V_API, int mode, void *args) {
	int error = 0;
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
	
	if (Ctrl->In.mode == SUBPLOT_BEGIN) {	/* Determine and save subplot panel attributes */
		unsigned int row, col, k, panel, nx, ny, factor, last_row, last_col, *Lx = NULL, *Ly = NULL;
		uint64_t seg;
		double x, y, area_dim[2], plot_dim[2], width, height, tick_height, annot_height, label_height, title_height, heading_height, heading_only;
		double *px = NULL, *py = NULL, y_heading, GMT_LETTER_HEIGHT = 0.736;
		char **Bx = NULL, **By = NULL, *cmd = NULL, axes[3] = {""}, Bopt[GMT_LEN64] = {""};
		char vfile[GMT_STR16] = {""};
		bool add_annot;
		FILE *fp = NULL;
		
		sprintf (file, "%s/gmt.subplot", API->gwf_dir);
		if (!access (file, F_OK))	{	/* Subplot information file already exists */
			GMT_Report (API, GMT_MSG_NORMAL, "Error: Subplot information file already exists: %s\n", file);
			Return (GMT_RUNTIME_ERROR);
		}
		tick_height   = MAX(0,GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER]);	/* Allow for axis ticks */
		annot_height  = (GMT_LETTER_HEIGHT * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH) + MAX (0.0, GMT->current.setting.map_annot_offset[GMT_PRIMARY]);	/* Allow for space between axis and annotations */
		label_height  = (GMT_LETTER_HEIGHT * GMT->current.setting.font_label.size / PSL_POINTS_PER_INCH) + MAX (0.0, GMT->current.setting.map_label_offset);
		title_height = (GMT_LETTER_HEIGHT * GMT->current.setting.font_title.size / PSL_POINTS_PER_INCH) + GMT->current.setting.map_title_offset;
		heading_only = (GMT_LETTER_HEIGHT * GMT->current.setting.font_heading.size / PSL_POINTS_PER_INCH);
		heading_height = heading_only + GMT->current.setting.map_heading_offset;
		/* Get plot/media area dimensions */
		width  = area_dim[GMT_X] = Ctrl->F.dim[GMT_X];
		height = area_dim[GMT_Y] = Ctrl->F.dim[GMT_Y];
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: rows            = %d\n", Ctrl->N.dim[GMT_Y]);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: columns         = %d\n", Ctrl->N.dim[GMT_X]);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: height          = %g\n", height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: width           = %g\n", width);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: tick_height     = %g\n", tick_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: annot_height    = %g\n", annot_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: label_height    = %g\n", label_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: title_height    = %g\n", title_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: heading_height  = %g\n", heading_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: media margin    = %g/%g/%g/%g\n", Ctrl->M.margin[SUBPLOT_MEDIA][XLO], Ctrl->M.margin[SUBPLOT_MEDIA][XHI], Ctrl->M.margin[SUBPLOT_MEDIA][YLO], Ctrl->M.margin[SUBPLOT_MEDIA][YHI]);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: panel margin    = %g/%g/%g/%g\n", Ctrl->M.margin[SUBPLOT_PANEL][XLO], Ctrl->M.margin[SUBPLOT_PANEL][XHI], Ctrl->M.margin[SUBPLOT_PANEL][YLO], Ctrl->M.margin[SUBPLOT_PANEL][YHI]);
		/* Shrink these if media margins were requested */
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Start: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		if (Ctrl->M.active[SUBPLOT_MEDIA]) {	/* Remove space used by media margins */
			area_dim[GMT_X] -= (Ctrl->M.margin[SUBPLOT_MEDIA][XLO] + Ctrl->M.margin[SUBPLOT_MEDIA][XHI]);
			area_dim[GMT_Y] -= (Ctrl->M.margin[SUBPLOT_MEDIA][YLO] + Ctrl->M.margin[SUBPLOT_MEDIA][YHI]);
			width  -= (Ctrl->M.margin[SUBPLOT_MEDIA][XLO] + Ctrl->M.margin[SUBPLOT_MEDIA][XHI]);
			height -= (Ctrl->M.margin[SUBPLOT_MEDIA][YLO] + Ctrl->M.margin[SUBPLOT_MEDIA][YHI]);
		}
		y_heading = height - heading_only;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After media margins: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		if (Ctrl->M.active[SUBPLOT_PANEL]) {	/* Remove space used by panel margins */
			area_dim[GMT_X] -= Ctrl->N.dim[GMT_X] * (Ctrl->M.margin[SUBPLOT_PANEL][XLO] + Ctrl->M.margin[SUBPLOT_PANEL][XHI]);
			area_dim[GMT_Y] -= Ctrl->N.dim[GMT_Y] * (Ctrl->M.margin[SUBPLOT_PANEL][YLO] + Ctrl->M.margin[SUBPLOT_PANEL][YHI]);
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After panel margins: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		/* Limit tickmarks to 1 or 2 axes per row or per panel */
		nx = (Ctrl->L.tick[GMT_Y] == SUBPLOT_PLACE_AT_BOTH) ? 2 : 1;
		if ((Ctrl->L.mode & SUBPLOT_ROW_FIXED_Y) == 0) 
			nx *= Ctrl->N.dim[GMT_X];	/* Each column needs separate y-axis [and labels] */
		area_dim[GMT_X] -= nx * tick_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d row ticks: area_dim = {%g, %g}\n", nx, area_dim[GMT_X], area_dim[GMT_Y]);
		/* Limit annotations/labels to 1 or 2 axes per row or per panel */
		nx = (Ctrl->L.annotate[GMT_Y] == SUBPLOT_PLACE_AT_BOTH) ? 2 : 1;
		if ((Ctrl->L.mode & SUBPLOT_ROW_FIXED_Y) == 0) 
			nx *= Ctrl->N.dim[GMT_X];	/* Each column needs separate y-axis [and labels] */
		area_dim[GMT_X] -= nx * annot_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d row annots: area_dim = {%g, %g}\n", nx, area_dim[GMT_X], area_dim[GMT_Y]);
		if (Ctrl->L.has_label[GMT_Y])
			area_dim[GMT_X] -= nx * label_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d row labels: area_dim = {%g, %g}\n", nx, area_dim[GMT_X], area_dim[GMT_Y]);
		plot_dim[GMT_X] = area_dim[GMT_X] / Ctrl->N.dim[GMT_X];
		/* Limit annotations/labels to 1 or 2 axes per column or per panel */
		if (Ctrl->T.active)
			area_dim[GMT_Y] -= heading_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After main heading: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		ny = (Ctrl->L.tick[GMT_X] == SUBPLOT_PLACE_AT_BOTH) ? 2 : 1;
		factor = (Ctrl->L.mode & SUBPLOT_COL_FIXED_X) ? 1 : Ctrl->N.dim[GMT_Y];		/* Each row may need separate x-axis ticks */
		ny *= factor;
		area_dim[GMT_Y] -= ny * tick_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d col ticks: area_dim = {%g, %g}\n", ny, area_dim[GMT_X], area_dim[GMT_Y]);
		ny = (Ctrl->L.annotate[GMT_X] == SUBPLOT_PLACE_AT_BOTH) ? 2 : 1;
		factor = (Ctrl->L.mode & SUBPLOT_COL_FIXED_X) ? 1 : Ctrl->N.dim[GMT_Y];		/* Each row may need separate x-axis [and labels] */
		ny *= factor;
		area_dim[GMT_Y] -= ny * annot_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d col annot: area_dim = {%g, %g}\n", ny, area_dim[GMT_X], area_dim[GMT_Y]);
		if (Ctrl->L.has_label[GMT_X])
			area_dim[GMT_Y] -= ny * label_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d col labels: area_dim = {%g, %g}\n", ny, area_dim[GMT_X], area_dim[GMT_Y]);
		if (Ctrl->L.ptitle == SUBPLOT_PANEL_COL_TITLE)
			area_dim[GMT_Y] -= title_height;
		else if (Ctrl->L.ptitle == SUBPLOT_PANEL_TITLE)
			area_dim[GMT_Y] -= factor * title_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d panel titles: area_dim = {%g, %g}\n", factor, area_dim[GMT_X], area_dim[GMT_Y]);
		plot_dim[GMT_Y] = area_dim[GMT_Y] / Ctrl->N.dim[GMT_Y];
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Panel dimensions: {%g, %g}\n", plot_dim[GMT_X], plot_dim[GMT_Y]);
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
			uint64_t dim[4] = {1, Ctrl->N.n_panels, 4, 2};	/* One segment polygon per panel */
			if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Subplot: Unable to allocate a dataset\n");
				Return (error);
			}
		}
		
		/* Need to determine how many y-labels and annotations for each row since it affects panel size */

		y = Ctrl->F.dim[GMT_Y];	/* Start off at top edge of media area */
		if (Ctrl->M.active[SUBPLOT_MEDIA])
			y -= Ctrl->M.margin[SUBPLOT_MEDIA][YHI];	/* Skip space used by paper margins */
		if (Ctrl->T.active)
			y -= heading_height;	/* Skip space for main figure title */
		for (row = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each row of panels */
			axes[0] = axes[1] = k = 0;
			y -= Ctrl->M.margin[SUBPLOT_PANEL][YHI];
			if (Ctrl->F.debug) {	/* ALl rows share this upper y */
				for (col = 0; col < Ctrl->N.dim[GMT_X]; col++) {	/* For each col of panels */
					seg = row * Ctrl->N.dim[GMT_X] + col;
					D->table[0]->segment[seg]->data[GMT_Y][2] = D->table[0]->segment[seg]->data[GMT_Y][3] = y;
				}
			}
			if ((row == 0 && (Ctrl->L.ptitle == SUBPLOT_PANEL_COL_TITLE)) || (Ctrl->L.ptitle == SUBPLOT_PANEL_TITLE))
				y -= title_height;	/* Make space for panel title */
			if (Ctrl->L.mode && ((row == 0 || ((Ctrl->L.mode & SUBPLOT_COL_FIXED_X) == 0)) && (Ctrl->L.annotate[GMT_X] & SUBPLOT_PLACE_AT_MAX)))	/* Need annotation at N */
				add_annot = true;
			else if (Ctrl->L.mode == 0 && strchr (Ctrl->L.axes, 'N'))
				add_annot = true;
			else if (strchr (Ctrl->L.axes, 'n'))
				add_annot = false;
			if (add_annot) {	/* Need annotation at N */
				axes[k++] = 'N';
				y -= (annot_height + tick_height);
				if (Ctrl->L.has_label[GMT_X]) {
					y -= label_height;	/* Also has label at N */
					Ly[row] |= SUBPLOT_PLACE_AT_MAX;
				}
			}
			else if (strchr (Ctrl->L.axes, 'n')) {
				axes[k++] = 'n';
				y -= tick_height;
			}
			y -= plot_dim[GMT_Y];	/* Now at correct y for this panel */
			py[row] = y;
			if (Ctrl->L.mode && (row == last_row || ((Ctrl->L.mode & SUBPLOT_COL_FIXED_X) == 0)) && (Ctrl->L.annotate[GMT_X] & SUBPLOT_PLACE_AT_MIN))	/* Need annotation at S */
				add_annot = true;
			else if (Ctrl->L.mode == 0 && strchr (Ctrl->L.axes, 'S'))
				add_annot = true;
			else
				add_annot = false;
			if (add_annot) {
				axes[k++] = 'S';
				y -= (annot_height + tick_height);
				if (Ctrl->L.has_label[GMT_X])
					y -= label_height;	/* Also has label at S */
				Ly[row] |= SUBPLOT_PLACE_AT_MIN;
			}
			else if (strchr (Ctrl->L.axes, 's')) {
				axes[k++] = 's';
				y -= tick_height;
			}
			By[row] = strdup (axes);
			if (Ctrl->F.debug) {	/* ALl rows share this lower y */
				for (col = 0; col < Ctrl->N.dim[GMT_X]; col++) {	/* For each col of panels */
					seg = row * Ctrl->N.dim[GMT_X] + col;
					D->table[0]->segment[seg]->data[GMT_Y][0] = D->table[0]->segment[seg]->data[GMT_Y][1] = y;
				}
			}
			y -= Ctrl->M.margin[SUBPLOT_PANEL][YLO];
		}
		x = 0.0;	/* Start off at left edge of plot area */
		if (Ctrl->M.active[SUBPLOT_MEDIA])
			x += Ctrl->M.margin[SUBPLOT_MEDIA][XLO];	/* Skip space used by paper margins */
		for (col = 0; col < Ctrl->N.dim[GMT_X]; col++) {	/* For each col of panels */
			axes[0] = axes[1] = k = 0;
			x += Ctrl->M.margin[SUBPLOT_PANEL][XLO];
			if (Ctrl->F.debug) {	/* ALl cols share this left x */
				for (row = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each col of panels */
					seg = row * Ctrl->N.dim[GMT_X] + col;
					D->table[0]->segment[seg]->data[GMT_X][0] = D->table[0]->segment[seg]->data[GMT_X][3] = x;
				}
			}
			if (Ctrl->L.mode && (col == 0 || ((Ctrl->L.mode & SUBPLOT_ROW_FIXED_Y) == 0)) && (Ctrl->L.annotate[GMT_X] & SUBPLOT_PLACE_AT_MIN))	/* Need annotation at W */
				add_annot = true;
			else if (Ctrl->L.mode == 0 && strchr (Ctrl->L.axes, 'W'))
				add_annot = true;
			else
				add_annot = false;
			if (add_annot) {
				axes[k++] = 'W';
				x += (annot_height + tick_height);
				if (Ctrl->L.has_label[GMT_Y]) {
					x += label_height;	/* Also has label at W */
					Lx[col] |= SUBPLOT_PLACE_AT_MIN;
				}
			}
			else if (strchr (Ctrl->L.axes, 'w')) {
				x += tick_height;
				axes[k++] = 'w';
			}
			px[col] = x;	/* Now at correct x for this panel */
			x += plot_dim[GMT_X];
			if (Ctrl->L.mode && (col == last_col || ((Ctrl->L.mode & SUBPLOT_ROW_FIXED_Y) == 0)) && (Ctrl->L.annotate[GMT_Y] & SUBPLOT_PLACE_AT_MAX))	/* Need annotation at E */
				add_annot = true;
			else if (Ctrl->L.mode == 0 && strchr (Ctrl->L.axes, 'E'))
				add_annot = true;
			else
				add_annot = false;
			if (add_annot) {
				axes[k++] = 'E';
				x += (annot_height + tick_height);
				if (Ctrl->L.has_label[GMT_Y]) {
					x += label_height;	/* Also has label at E */
					Lx[col] |= SUBPLOT_PLACE_AT_MAX;
				}
			}
			else if (strchr (Ctrl->L.axes, 'e')) {
				x += tick_height;
				axes[k++] = 'e';
			}
			if (Ctrl->F.debug) {	/* ALl cols share this right x */
				for (row = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each col of panels */
					seg = row * Ctrl->N.dim[GMT_X] + col;
					D->table[0]->segment[seg]->data[GMT_X][1] = D->table[0]->segment[seg]->data[GMT_X][2] = x;
				}
			}
			x += Ctrl->M.margin[SUBPLOT_PANEL][XHI];
			Bx[col] = strdup (axes);
		}
		
		/* Write out the subplot information file */
		
		fp = fopen (file, "w");
		fprintf (fp, "# subplot panel information file\n");
		cmd = GMT_Create_Cmd (API, options);
		fprintf (fp, "# Command: %s %s\n", THIS_MODULE_NAME, cmd);
		gmt_M_free (GMT, cmd);
		if (Ctrl->T.active) fprintf (fp, "# HEADING: %g %g %s\n", 0.5 * width, y_heading, Ctrl->T.title);
		fprintf (fp, "#panel\trow\tcol\tnrow\tncol\tx0\ty0\tw\th\ttag\ttag_dx\ttag_dy\ttag_pos\ttag_just\ttag_fill\ttag_pen\tBframe\tBx\tBy\n");
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
					fprintf (fp, "\t%g\t%g\t%s\t%s", Ctrl->A.off[GMT_X], Ctrl->A.off[GMT_Y], Ctrl->A.placement, Ctrl->A.justify);
					fprintf (fp, "\t%s\t%s", Ctrl->A.fill, Ctrl->A.pen);
				}
				else
					fprintf (fp, "-\t0\t0\tBL\tBL\t-\t-");
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
			sprintf (command, "-R0/%g/0/%g -Jx1i -P -N -F+jBC+f%s %s -Xa%gi -Ya%gi --GMT_HISTORY=false",
				width, height, gmt_putfont (GMT, &GMT->current.setting.font_heading), vfile, Ctrl->M.margin[SUBPLOT_MEDIA][XLO], Ctrl->M.margin[SUBPLOT_MEDIA][YLO]);
			if (Bopt[0] == ' ') strcat (command, Bopt);
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot to pstext: %s\n", command);
			if (GMT_Call_Module (API, "pstext", GMT_MODULE_CMD, command) != GMT_OK)	/* Plot the cancas with heading */
				Return (API->error);
			if (GMT_Destroy_Data (API, &T) != GMT_OK)
				Return (API->error);
		}
		else {	/* psxy will do, since nothing is plotted (except for possibly the canvas fill/outline) */
			sprintf (command, "-R0/%g/0/%g -Jx1i -P -T -Xa%gi -Ya%gi --GMT_HISTORY=false", width, height, Ctrl->M.margin[SUBPLOT_MEDIA][XLO], Ctrl->M.margin[SUBPLOT_MEDIA][YLO]);
			if (Bopt[0]) strcat (command, Bopt);
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot to psxy: %s\n", command);
			if (GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, command) != GMT_OK)	/* Plot the cancas with heading */
				Return (API->error);
		}
		
		if (Ctrl->F.debug) {	/* Write a debug file with panel polygons for use by "gmt subplot end" */
			bool save = GMT->current.setting.io_header[GMT_OUT];
			sprintf (file, "%s/subplot.debug", API->gwf_dir);
			sprintf (command, "0/%g/0/%g", Ctrl->F.dim[GMT_X], Ctrl->F.dim[GMT_Y]);	/* Save page region */
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
		if ((error = gmt_set_current_panel (API, Ctrl->In.row, Ctrl->In.col, 1)))
			Return (error)
	}
	else {	/* SUBPLOT_END */
		int k;
		char *mode[2] = {"w","a"}, vfile[GMT_STR16] = {""};
		FILE *fp = NULL;
		/* Must force PSL_completion to run, if set */
		if ((k = gmt_set_psfilename (GMT)) == GMT_NOTSET) {	/* Get hidden file name for PS */
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: No workflow directory\n");
			Return (GMT_ERROR_ON_FOPEN);
		}
		if ((fp = PSL_fopen (GMT->PSL, GMT->current.ps.filename, mode[k])) == NULL) {	/* Must open inside PSL DLL */
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open %s with mode %s\n", GMT->current.ps.filename, mode[k]);
			Return (GMT_ERROR_ON_FOPEN);
		}
		PSL_command (GMT->PSL, "PSL_completion /PSL_completion {} def\n");	/* Run then make it a null function */
		if (PSL_fclose (GMT->PSL)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Unable to close hidden PS file %s!\n", GMT->current.ps.filename);
			Return (GMT_RUNTIME_ERROR);
		}
		sprintf (file, "%s/gmt.subplot", API->gwf_dir);
		gmt_remove_file (GMT, file);
		sprintf (file, "%s/gmt.panel", API->gwf_dir);
		gmt_remove_file (GMT, file);
		/* Check if we should draw debug lines */
		sprintf (file, "%s/subplot.debug", API->gwf_dir);
		if (!access (file, R_OK)) {	/* Yes, must draw debug lines on top */
			if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, 0, NULL, file, NULL)) == NULL) {
				Return (API->error);
			}
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POLY, GMT_IN, D, vfile) != GMT_NOERROR) {
				Return (API->error);
			}
			sprintf (command, "-R%s -Jx1i -Xa0 -Ya0 %s -L -Wfaint,red --GMT_HISTORY=false", D->table[0]->header[0], vfile);
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot to psxy 4 debug: %s\n", command);
			if (GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, command) != GMT_OK)	/* Plot the canvas with heading */
				Return (API->error);
			if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
				Return (API->error);
			}
			gmt_remove_file (GMT, file);
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Removed panel and subplot files\n");
	}
		
	Return (error);
}
