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
 * Date:	1-Jul-2017
 * Version:	6 API
 *
 * Brief synopsis: gmt subplot determines dimensions and offsets for a multi-subplot figure.
 * It has three modes of operation:
 *	1) Initialize a new figure with subplots, which determines dimensions and sets parameters:
 *	   gmt subplot begin <nrows>x<ncols> -F[f|s][<W/H>[+f<wfracs/hfracs>]] [-A<labels>]
 *		[-SC<layout>] [-SR<layout>] [-M<margins>] [-T<title>] [-R<region>] [-J<proj>] [-V]
 *	2) Select the current subplot window for plotting, usually so we can use -A or -C (since -crow,col is faster):
 *	   gmt subplot [set] <row>,<col>|<index> [-A<fixlabel>] [-C<side><clearance>] [-V]
 *	3) Finalize the figure:
 *	   gmt subplot end [-V]
 */

/* Note: subplot is currently not able to be nested (i.e., a subplot inside another subplot).
 * This will be required to do more complicated layouts where the number of items vary for
 * different columns and/or rows.  So until such time, all subplots must have a constant number
 * of items in all rows and a constant number of items in all columns.  The dimensions can vary
 * on a per-column or per-row basis, but the number of items must remain the same. To implement
 * nesting we would place all the subplot control files inside a subdirectory and have nested
 * subdirectories.
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"subplot"
#define THIS_MODULE_MODERN_NAME	"subplot"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Manage modern mode figure subplot configuration and selection"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"JRVXY"

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
		bool next;
		bool no_B;
		unsigned int mode;	/* SUBPLOT_BEGIN|SUBPLOT_SET|SUBPLOT_END*/
		int row, col;		/* Current (row,col) subplot */
	} In;
	struct A {	/* -A[<letter>|<number>][+c<clearance>][+g<fill>][+j|J<pos>][+o<offset>][+p<pen>][+r|R][+v] */
		bool active;			/* Want to plot subplot tags */
		char format[GMT_LEN128];	/* Format for plotting tag (or constant string when done via subplot set) */
		char fill[GMT_LEN64];		/* Color fill for optional rectangle behind the tag [none] */
		char pen[GMT_LEN64];		/* Outline pen for optional rectangle behind the tag [none] */
		unsigned int mode;		/* Either letter (0) of number (1) tag */
		unsigned int nstart;		/* Offset count number for first subplot (0) */
		unsigned int way;		/* Whether we go down columns or across rows first [Default] */
		unsigned int roman;		/* Place roman upper or lower case letters instead of arabic [Default] */
		char cstart;			/* Start letter for first subplot [a] */
		char placement[3];		/* Placement of tag [TL] */
		char justify[3];		/* Justification of tag [TL] */
		double off[2];			/* Offset from placement location [20% of font size] */
		double clearance[2];		/* Padding around text for rectangle behind the tag [15%] */
	} A;
	struct C {	/* -C[side]<clearance>  */
		bool active;
		double gap[4];	/* Internal margins (in inches) on the 4 sides [0/0/0/0] */
	} C;
	struct F {	/* -F[f|s][<width>/<height>][+f<wfracs/hfracs>][+p<pen>][+g<fill>][+c<clearance>][+d][+w<pen>] */
		bool active;
		bool lines;
		bool debug;		/* Draw red faint lines to illustrate the result of space partitioning */
		bool reset_h;		/* True when height was given as 0 and we need to update based on what was learned */
		unsigned int mode;	/* Whether dimensions given are for figure or individual subplots */
		double dim[2];		/* Figure dimension (0/0 if subplot dimensions were given) */
		double *w, *h;		/* Arrays with variable (or constant) normalized subplot widths and heights fractions */
		double clearance[2];	/* Optionally extend the figure rectangle with padding [0] */
		char fill[GMT_LEN64];	/* Fill for the entire figure canvas */
		char pen[GMT_LEN64];	/* Pen outline for the entire figure canvas */
		char Lpen[GMT_LEN64];	/* Pen to draw midlines */
	} F;
	struct S {	/* -S[C|R]<layout> */
		bool active;
		bool has_label;		/* True if we want y labels */
		char axes[4];		/* W|e|w|e|l|r for -SR,  S|s|N|n|b|t for -SC [Default is MAP_FRAME_AXES] */
		char *b;		/* Any hardwired choice for afg settings for this axis [af] */
		char *label[2];		/* The constant primary [and alternate] y labels */
		char *extra;		/* Special -B frame args, such as fill */
		unsigned int ptitle;	/* 0 = no subplot titles, 1 = column titles, 2 = all subplot titles */
		unsigned annotate;	/* 1 if only l|r or t|b, 0 for both */
		unsigned tick;		/* 1 if only l|r or t|b, 0 for both */
		unsigned parallel;	/* 1 if we want axis parallel annotations */
	} S[2];
	struct M {	/* -M<margin> | <xmargin>/<ymargin>  | <wmargin>/<emargin>/<smargin>/<nmargin>  */
		bool active;
		double margin[4];
	} M;
	struct N {	/* NrowsxNcolumns (is not used as -N<> but without the option which is just internal) */
		bool active;
		unsigned int dim[2];		/* nrows, rcols */
		unsigned int n_subplots;	/* The product of the two dims */
	} N;
	struct T {	/* -T<figuretitle> */
		bool active;
		char *title;	/* Title above the entire set of subplots */
	} T;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SUBPLOT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct SUBPLOT_CTRL);
	C->In.row = C->In.col = GMT_NOTSET;
	sprintf (C->A.placement, "TL");
	sprintf (C->A.justify, "TL");
	C->A.off[GMT_X] = C->A.off[GMT_Y] = 0.01 * GMT_TEXT_OFFSET * GMT->current.setting.font_tag.size / PSL_POINTS_PER_INCH; /* 20% */
	C->A.clearance[GMT_X] = C->A.clearance[GMT_Y] = 0.01 * GMT_TEXT_CLEARANCE * GMT->current.setting.font_tag.size / PSL_POINTS_PER_INCH;	/* 15% */
	C->A.fill[0] = C->A.pen[0] = '-';	/* No fill or outline */
	C->F.fill[0] = C->F.pen[0] = '-';	/* No fill or outline */
	for (unsigned int k = 0; k < 4; k++) C->M.margin[k] = 0.5 * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;	/* Split annot font across two sides */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct SUBPLOT_CTRL *C) {	/* Deallocate control structure */
	gmt_M_unused (GMT);
	if (!C) return;
	gmt_M_str_free (C->T.title);
	gmt_M_str_free (C->S[GMT_X].label[GMT_PRIMARY]);
	gmt_M_str_free (C->S[GMT_X].label[GMT_SECONDARY]);
	gmt_M_str_free (C->S[GMT_Y].label[GMT_PRIMARY]);
	gmt_M_str_free (C->S[GMT_Y].label[GMT_SECONDARY]);
	gmt_M_str_free (C->S[GMT_X].extra);
	gmt_M_free (GMT, C->F.w);
	gmt_M_free (GMT, C->F.h);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s begin <nrows>x<ncols> -F[f|s]<width(s)>/<height(s)>[+f<wfracs/hfracs>][+c<gap>][+g<fill>][+p<pen>][+w<pen>]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-A<autolabelinfo>] [-C[<side>]<clearance>] [%s] [-SC<layout>][+<mods>] [-SR<layout>][+<mods>]\n\t[-M<margins>] [%s] [-T<title>] [%s] [%s]\n\t[%s] [%s]\n\n",
	 	GMT_J_OPT, GMT_Rgeo_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_PAR_OPT);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s set [<row>,<col>|<index>] [-A<fixedlabel>] [-C<side><clearance>] [%s]\n", name, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\tSet <row>,<col> in 0-(nrows-1),0-(ncols-1) range, or <index> in 0 to (nrows*ncols-1) range [next subplot].\n\n");
	GMT_Message (API, GMT_TIME_NONE, "usage: %s end [%s]\n\n", name, GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<nrows>x<ncols> is the number of rows and columns of subplots in this figure.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F or -Ff: Specify dimension of the whole figure plot area. Subplot sizes will be computed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +f<wfracs/hfracs> to variable widths and heights by giving comma-separated lists\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of relative values, one per row or column, which we scale to match figure dimension.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If only columns or rows should have variable dimension you can set the other arg as 1.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Fs: Set dimensions of area that each multi-subplot figure may occupy.  If these\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   should differ from column to column or row to row you can give a comma-separated\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   list of widths and/or heights.  A single value means constant width or height.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   As an option the composite figure rectangle may be extended, drawn or filled.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This is most useful if you are not plotting any map frames in the subplots.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c<dx>[/<dy>] for extending the figure rectangle outwards [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +g to fill the figure rectangle with <fill> color [no fill].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p to draw the outline of the figure rectangle using selected pen [no outline].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +w to draw dividing lines between interior subplots using selected pen [no lines].\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t-C Specify a gap of dimension <clearance> to the <side> (w|e|s|n) of the plottable subplot.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Shrinks the size for the main plot to make room for scales, bars, etc.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Repeatable for more than one side. Use <side> = x or y to set w|e or s|n, respectively.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   No specified <side> means set the same clearance on all sides [no clearances].\n");
	GMT_Option (API, "J-");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Adds space around each subplot. Append a uniform <margin>, separate <xmargin>/<ymargin>,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or individual <wmargin>/<emargin>/<smargin>/<nmargin> for each side [%gp].\n", 0.5*API->GMT->current.setting.font_annot[GMT_PRIMARY].size);
	GMT_Message (API, GMT_TIME_NONE, "\t   Actual gap between interior subplots is always the sum of two opposing margins.\n");
	GMT_Option (API, "R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Set shared subplot layout for rows (-SR) and columns (-SC); can be overridden via -B:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-SC Each subplot Column shares a common x-range. First row (top axis) and last row (bottom axis) are annotated;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append t or b to select only one of those two axes annotations instead [both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append +l if annotated x-axes should have a label [none]; optionally append the label if it is fixed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Alternatively, you can also use +s.  If no label is given then you must set it when the subplot is plotted via -B.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append +t to make space for individual titles for all subplots; use +tc for top row titles only [no subplot titles].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-SR Each subplot Row shares a common y-range. First column (left axis) and last column (right axis) are annotated;\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append l or r to select only one of those two axes annotations instead [both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append +l if annotated y-axes will have a label [none]; optionally append the label if fixed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Alternatively, you can also use +s.  If no label is given then you msut set it when the subplot is plotted via -B.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t    Append +p if y-axes annotations should be parallel to axis [horizontal].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify a main heading to be centered above the figure [none].\n");
	GMT_Option (API, "VX.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SUBPLOT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k, j, n = 0;
	bool B_args = false, noB = false;
	char *c = NULL, add[2] = {0, 0}, string[GMT_LEN128] = {""};
	struct GMT_OPTION *opt = NULL, *Bframe = NULL, *Bx = NULL, *By = NULL, *Bxy = NULL;
	struct GMT_PEN pen;	/* Only used to make sure any pen is given with correct syntax */
	struct GMT_FILL fill;	/* Only used to make sure any fill is given with correct syntax */

	opt = options;	/* The first argument is the subplot command */
	if (opt->option != GMT_OPT_INFILE) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "No subplot command given\n");
		return GMT_PARSE_ERROR;
	}
	if (!strncmp (opt->arg, "begin", 5U)) {	/* Initiate new subplot */
		Ctrl->In.mode = SUBPLOT_BEGIN;
		opt = opt->next;	/* The required number of rows and columns */
		if (opt->option != GMT_OPT_INFILE || (n = sscanf (opt->arg, "%dx%d", &Ctrl->N.dim[GMT_Y], &Ctrl->N.dim[GMT_X]) < 1)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "subplot begin: Unable to extract nrows and ncols from %s\n", opt->arg);
			return GMT_PARSE_ERROR;
		}
		Ctrl->N.n_subplots = Ctrl->N.dim[GMT_X] * Ctrl->N.dim[GMT_Y];
		Ctrl->N.active = true;
	}
	else if (!strncmp (opt->arg, "end", 3U))	/* End of the subplot */
		Ctrl->In.mode = SUBPLOT_END;
	else if (!strncmp (opt->arg, "set", 3U)) {	/* Explicitly called set row,col or set index */
		opt = opt->next;	/* The row,col part */
		if (opt) {	/* There is an argument */
			if (isdigit (opt->arg[0]) && (n = sscanf (opt->arg, "%d,%d", &Ctrl->In.row, &Ctrl->In.col)) < 1) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to parse row,col: %s\n", opt->arg);
				return GMT_PARSE_ERROR;
			}
			if (n == 1) Ctrl->In.col = INT_MAX;	/* Flag we gave the 1-D index only */
			if (Ctrl->In.row == GMT_NOTSET || Ctrl->In.col == GMT_NOTSET) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to parse row,col|index: %s\n", opt->arg);
				return GMT_PARSE_ERROR;
			}
		}
		else {	/* Default to go to next subplot */
			Ctrl->In.row = 0;
			Ctrl->In.next = true;
		}
		Ctrl->In.mode = SUBPLOT_SET;
	}
	else if (strchr (opt->arg, ',')) {	/* Implicitly called set without using the word "set" */
		if (sscanf (opt->arg, "%d,%d", &Ctrl->In.row, &Ctrl->In.col) < 2 || Ctrl->In.row < 0 || Ctrl->In.col < 0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Not a subplot command: %s\n", opt->arg);
			return GMT_PARSE_ERROR;
		}
		Ctrl->In.mode = SUBPLOT_SET;	/* Implicit set command */
	}
	else if (gmt_is_integer (opt->arg)) {	/* Implicitly called set via an index without using the word "set" */
		if ((Ctrl->In.row = atoi (opt->arg)) < 0) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to parse index: %s\n", opt->arg);
			return GMT_PARSE_ERROR;
		}
		Ctrl->In.col = INT_MAX;	/* Flag we gave the 1-D index only */
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Not a recognized subplot command: %s\n", opt->arg);
		return GMT_PARSE_ERROR;
	}
	if (opt) opt = opt->next;	/* Position to the next argument */
	if (Ctrl->In.mode == SUBPLOT_END && opt && !(opt->option == 'V' && opt->next == NULL)) {	/* Only -V is optional for end or set */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "subplot end: Unrecognized option: %s\n", opt->arg);
		return GMT_PARSE_ERROR;
	}

	while (opt) {	/* Process all the options given */

		switch (opt->option) {

			case 'A':	/* Enable subplot tags and get attributes */
				Ctrl->A.active = true;
				if (Ctrl->In.mode == SUBPLOT_SET) {	/* Override the auto-annotation for this subplot only */
					strncpy (Ctrl->A.format, opt->arg, GMT_LEN128);
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
						if (gmt_validate_modifiers (GMT, opt->arg, 'A', "cgjJoprRv")) n_errors++;
						if (gmt_get_modifier (opt->arg, 'j', Ctrl->A.placement)) {	/* Inside placement */
							gmt_just_validate (GMT, Ctrl->A.placement, "TL");
							strncpy (Ctrl->A.justify, Ctrl->A.placement, 2);
						}
						else if (gmt_get_modifier (opt->arg, 'J', Ctrl->A.placement)) {	/* Outside placement */
							int pos = gmt_just_decode (GMT, Ctrl->A.placement, PSL_NO_DEF);
							pos = gmt_flip_justify (GMT, pos);
							gmt_just_to_code (GMT, pos, Ctrl->A.justify);
							GMT_Report (GMT->parent, GMT_MSG_DEBUG, "The mirror of %s is %s\n", Ctrl->A.placement, Ctrl->A.justify);
						}
						if (gmt_get_modifier (opt->arg, 'c', string) && string[0])	/* Clearance for box */
							if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->A.clearance) < 0) n_errors++;
						if (gmt_get_modifier (opt->arg, 'g', Ctrl->A.fill) && Ctrl->A.fill[0]) {
							if (gmt_getfill (GMT, Ctrl->A.fill, &fill)) n_errors++;
						}
						if (gmt_get_modifier (opt->arg, 'o', string))	/* Offset refpoint */
							if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->A.off) < 0) n_errors++;
						if (gmt_get_modifier (opt->arg, 'p', Ctrl->A.pen) && Ctrl->A.pen[0]) {
							if (gmt_getpen (GMT, Ctrl->A.pen, &pen)) n_errors++;
						}
						if (gmt_get_modifier (opt->arg, 'r', string))	/* Want lower-case roman numbering */
							Ctrl->A.roman = GMT_IS_ROMAN_LCASE;
						else if (gmt_get_modifier (opt->arg, 'R', string))	/* Want upper-case roman numbering */
							Ctrl->A.roman = GMT_IS_ROMAN_UCASE;
						if (gmt_get_modifier (opt->arg, 'v', string))	/* Tag order is vertical down columns */
							Ctrl->A.way = GMT_IS_COL_FORMAT;
					}
				}
				break;

			case 'B':	/* Get a handle on -B args if any */
				B_args = true;
				if (strstr (opt->arg, "+n")) noB = true;	/* Turn off all annotations */
				if (opt->arg[0] == 'x') Bx = opt;		/* Got options for x-axis only */
				else if (opt->arg[0] == 'y') By = opt;		/* Got options for y-axis only */
				else if (strchr ("WESNwesnlrbt", opt->arg[0])) Bframe = opt;	/* Frame settings */
				else Bxy = opt;	/* Shared by both x and y axes */
				break;
			case 'C':	/* Clearance/inside margins */
				Ctrl->C.active = true;
				if (strchr ("wesnxy", opt->arg[0])) {	/* Gave a side directive */
					switch (opt->arg[0]) {
						case 'w':	Ctrl->C.gap[XLO] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						case 'e':	Ctrl->C.gap[XHI] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						case 's':	Ctrl->C.gap[YLO] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						case 'n':	Ctrl->C.gap[YHI] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						case 'x':	Ctrl->C.gap[XLO] = Ctrl->C.gap[XHI] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
						case 'y':	Ctrl->C.gap[YLO] = Ctrl->C.gap[YHI] = gmt_M_to_inch (GMT, &opt->arg[1]); break;
					}
				}
				else {	/* Constant clearance on all side */
					Ctrl->C.gap[XLO] = Ctrl->C.gap[XHI] = Ctrl->C.gap[YLO] = Ctrl->C.gap[YHI] = gmt_M_to_inch (GMT, opt->arg);
				}
				break;

			case 'F':
				Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 'f':	Ctrl->F.mode = SUBPLOT_FIGURE;  n = 1;	break; /* Figure dimension */
					case 's':	Ctrl->F.mode = SUBPLOT_PANEL;   n = 1;	break; /* Panel dimension */
					default:	Ctrl->F.mode = SUBPLOT_FIGURE;  n = 0;	break; /* Figure dimension is default */
				}
				if (Ctrl->F.mode == SUBPLOT_PANEL && strstr (opt->arg, "+f")) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -F: +f modifier can only be used with -F[f].\n");
					n_errors++;
				}
				Ctrl->F.w = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_X], double);	/* Normalized fractional widths */
				Ctrl->F.h = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_Y], double);	/* Normalized fractional heights */
				c = strchr (opt->arg, '+');
				if (Ctrl->F.mode == SUBPLOT_FIGURE) {
					char *q = NULL;
					double f = 0.0;
					if ((q = strstr (opt->arg, "+f")) != NULL) {	/* Gave optional fractions on how to partition width and height on a per row/col basis */
						char *ytxt = strchr (&q[2], '/');	/* Find the slash */
						if (!ytxt) {
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Ff...+f missing slash between width and height fractions.\n");
							n_errors++;
							break;
						}
						k = GMT_Get_Values (GMT->parent, &ytxt[1], Ctrl->F.h, Ctrl->N.dim[GMT_Y]);
						if (k == 1) {	/* Constant, must duplicate */
							for (j = 1; j < Ctrl->N.dim[GMT_Y]; j++) Ctrl->F.h[j] = Ctrl->F.h[j-1];
						}
						else if (k < Ctrl->N.dim[GMT_Y]) {
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Ff...+f requires as many height fractions as there are rows.\n");
							n_errors++;
						}
						ytxt[0] = '\0';	/* Chop off the slash at start of height fractions */
						k = GMT_Get_Values (GMT->parent, &q[2], Ctrl->F.w, Ctrl->N.dim[GMT_X]);
						ytxt[0] = '/';	/* Restore the slash */
						if (k == 1) {	/* Constant, must duplicate */
							for (j = 1; j < Ctrl->N.dim[GMT_X]; j++) Ctrl->F.w[j] = Ctrl->F.w[j-1];
						}
						else if (k < Ctrl->N.dim[GMT_X]) {
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Ff...+f requires as many width fractions as there are columns.\n");
							n_errors++;
						}
						/* Normalize fractions */
						for (j = 0, f = 0.0; j < Ctrl->N.dim[GMT_X]; j++) f += Ctrl->F.w[j];
						for (j = 0; j < Ctrl->N.dim[GMT_X]; j++) Ctrl->F.w[j] /= f;
						for (j = 0, f = 0.0; j < Ctrl->N.dim[GMT_Y]; j++) f += Ctrl->F.h[j];
						for (j = 0; j < Ctrl->N.dim[GMT_Y]; j++) Ctrl->F.h[j] /= f;
						q[0] = '+';	/* Restore the fraction settings for now */
					}
					else {	/* Equal rows and cols */
						for (j = 0, f = 1.0 / Ctrl->N.dim[GMT_X]; j < Ctrl->N.dim[GMT_X]; j++) Ctrl->F.w[j] = f;
						for (j = 0, f = 1.0 / Ctrl->N.dim[GMT_Y]; j < Ctrl->N.dim[GMT_Y]; j++) Ctrl->F.h[j] = f;
					}
					if (c) c[0] = '\0';	/* Chop off modifiers for now */
					if ((k = GMT_Get_Values (GMT->parent, &opt->arg[n], Ctrl->F.dim, 2)) == 1) {	/* Square panel, duplicate */
						Ctrl->F.dim[GMT_Y] = Ctrl->F.dim[GMT_X];
					}
					/* Since GMT_Get_Values returns in default project length unit, convert to inch */
					for (k = 0; k < 2; k++) Ctrl->F.dim[k] *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
				}
				else {	/* Gave subplot dimension(s)  */
					if (c) c[0] = '\0';	/* Chop off modifiers for now */
					if (strchr (opt->arg, ',')) {	/* Gave separate widths and heights */
						char *ytxt = strchr (opt->arg, '/');	/* Find the slash */
						if (ytxt == NULL) {
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Fs requires heights and width lists separated by a slash.\n");
							return GMT_PARSE_ERROR;
						}
						k = GMT_Get_Values (GMT->parent, &ytxt[1], Ctrl->F.h, Ctrl->N.dim[GMT_Y]);
						if (k == 1) {	/* Constant, must duplicate */
							for (j = 1; j < Ctrl->N.dim[GMT_Y]; j++) Ctrl->F.h[j] = Ctrl->F.h[j-1];
						}
						else if (k < Ctrl->N.dim[GMT_Y]) {
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Fs requires as many heights as there are rows, or just a constant one.\n");
							n_errors++;
						}
						ytxt[0] = '\0';	/* Chop off the slash at start of height fractions */
						k = GMT_Get_Values (GMT->parent, &opt->arg[1], Ctrl->F.w, Ctrl->N.dim[GMT_X]);
						ytxt[0] = '/';	/* Restore the slash */
						if (k == 1) {	/* Constant, must duplicate */
							for (j = 1; j < Ctrl->N.dim[GMT_X]; j++) Ctrl->F.w[j] = Ctrl->F.w[j-1];
						}
						else if (k < Ctrl->N.dim[GMT_X]) {
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Fs requires as many widths as there are columns, or just a constant one.\n");
							n_errors++;
						}
						/* Since GMT_Get_Values returns in default project length unit, convert to inch */
						for (k = 0; k < Ctrl->N.dim[GMT_X]; k++) Ctrl->F.w[k] *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
						for (k = 0; k < Ctrl->N.dim[GMT_Y]; k++) Ctrl->F.h[k] *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
					}
					else {	/* Just got a fixed width/height for each subplot */
						if ((k = GMT_Get_Values (GMT->parent, &opt->arg[n], Ctrl->F.dim, 2)) == 1)	/* Square subplot */
							Ctrl->F.dim[GMT_Y] = Ctrl->F.dim[GMT_X];
						if (Ctrl->F.dim[GMT_Y] == 0.0) Ctrl->F.reset_h = true;	/* Update h later based on map aspect ratio and width of 1st col */

						/* Since GMT_Get_Values returns in default project length unit, convert to inch */
						for (k = 0; k < 2; k++) Ctrl->F.dim[k] *= GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
						for (j = 0; j < Ctrl->N.dim[GMT_X]; j++) Ctrl->F.w[j] = Ctrl->F.dim[GMT_X];	/* Duplicate equally to all rows and cols */
						for (j = 0; j < Ctrl->N.dim[GMT_Y]; j++) Ctrl->F.h[j] = Ctrl->F.dim[GMT_Y];
						Ctrl->F.dim[GMT_X] = Ctrl->F.dim[GMT_Y] = 0.0;	/* Since not know at this time */
					}
				}
				if (c) {	/* Gave paint/pen/debug modifiers */
					c[0] = '+';	/* Restore modifiers */
					if (gmt_validate_modifiers (GMT, opt->arg, 'F', "cdgpfw")) n_errors++;
					if (gmt_get_modifier (opt->arg, 'c', string) && string[0])	/* Clearance for rectangle */
						if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->F.clearance) < 0) n_errors++;
					if (gmt_get_modifier (opt->arg, 'g', Ctrl->F.fill) && Ctrl->F.fill[0]) {
						if (gmt_getfill (GMT, Ctrl->F.fill, &fill)) n_errors++;
					}
					if (gmt_get_modifier (opt->arg, 'p', Ctrl->F.pen) && Ctrl->F.pen[0]) {
						if (gmt_getpen (GMT, Ctrl->F.pen, &pen)) n_errors++;
					}
					if (gmt_get_modifier (opt->arg, 'd', string))
						Ctrl->F.debug = true;
					if (gmt_get_modifier (opt->arg, 'w', Ctrl->F.Lpen) && Ctrl->F.Lpen[0]) {
						if (gmt_getpen (GMT, Ctrl->F.Lpen, &pen)) n_errors++;
					}
				}
				break;

			case 'S':	/* Layout */
				switch (opt->arg[0]) {	/* Type of layout option selected [C(olumn) and R(ow) only] */
					case 'C':	k = GMT_X;	break;	/* Shared column settings */
					case 'R':	k = GMT_Y;	break;	/* Shared row settings */
					default:	k = GMT_Z; 	break;	/* Bad option, see test below */
				}
				if (k == GMT_Z) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -S requires C or R as first argument!\n");
					n_errors++;
					break;
				}
				if (Ctrl->S[k].active) {
					char *flavor = "CR";
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -S%c already specified!\n", flavor[k]);
					n_errors++;
					break;
				}
				Ctrl->S[k].active = true;	/* Selected this once */
				/* Determine if we are to place annotations on the minimum or maximum side of the feature, or both sides: */
				if ((k == GMT_X && opt->arg[1] == 'b') || (k == GMT_Y && opt->arg[1] == 'l')) Ctrl->S[k].annotate = Ctrl->S[k].tick = SUBPLOT_PLACE_AT_MIN;
				else if ((k == GMT_X && opt->arg[1] == 't') || (k == GMT_Y && opt->arg[1] == 'r')) Ctrl->S[k].annotate = Ctrl->S[k].tick = SUBPLOT_PLACE_AT_MAX;
				else Ctrl->S[k].annotate = Ctrl->S[k].tick = SUBPLOT_PLACE_AT_BOTH;
				if (gmt_validate_modifiers (GMT, opt->arg, 'S', "lspt")) n_errors++;	/* Gave a bad modifier */
				if (gmt_get_modifier (opt->arg, 'l', string)) {	/* Want space for (primary) labels */
					Ctrl->S[k].has_label = true;
					if (string[0]) Ctrl->S[k].label[GMT_PRIMARY] = strdup (string);
				}
				if (gmt_get_modifier (opt->arg, 's', string)) {	/* Want space for (secondary) labels */
					Ctrl->S[k].has_label = true;
					if (string[0]) Ctrl->S[k].label[GMT_SECONDARY] = strdup (string);
				}
				if (gmt_get_modifier (opt->arg, 't', string))	/* Want space for subplot titles, this could go with either -SR or -SC so accept it but save in -SC */
					Ctrl->S[GMT_X].ptitle = (string[0] == 'c') ? SUBPLOT_PANEL_COL_TITLE : SUBPLOT_PANEL_TITLE;
				if (k == GMT_Y) {	/* Modifiers applicable for y-axis only */
					if (gmt_get_modifier (opt->arg, 'p', string))	/* Want axis-parallel annotations [horizontal] */
						Ctrl->S[k].parallel = 1;
				}
				else if (gmt_get_modifier (opt->arg, 'p', string)) {	/* Invalid, only for SR */
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Modifier +p only allowed for -SR\n");
					n_errors++;
				}
				break;

			case 'M':	/* Margins between subplots */
				Ctrl->M.active = true;
				if (opt->arg[0] == 0) {	/* Got nothing */
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -M: No margins given.\n");
					n_errors++;
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
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -M: Bad number of margins given.\n");
						n_errors++;
					}
					/* Since GMT_Get_Values returns values in default project length unit, convert to inch */
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

	if (Ctrl->In.mode == SUBPLOT_BEGIN) {	/* Setting up a subplot system */
		unsigned int px = 0, py = 0;
		/* Was both -R -J given? */
		n_errors += gmt_M_check_condition (GMT, GMT->common.J.active && !GMT->common.R.active[RSET], "Option -J: Requires -R as well!\n");
		n_errors += gmt_M_check_condition (GMT, GMT->common.J.active && Ctrl->F.mode == SUBPLOT_FIGURE, "Option -J: Requires -Fs to determine subplot height!\n");
		if (GMT->common.J.active) {	/* Compute map height from -R -J */
			if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) n_errors++;
			for (j = 0; j < Ctrl->N.dim[GMT_Y]; j++) Ctrl->F.h[j] = GMT->current.map.height;
		}
		if (B_args) {	/* Got common -B settings that applies to all axes not controlled by -SR, -SC */
			if (!noB) {
				if ((Bxy && (Bx || By)) || (!Bxy && ((Bx && !By) || (By && !Bx)))) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -B: Must either set -Bx and -By or -B that applies to both axes.\n");
					n_errors++;
				}
			}
			if (Bxy || Bx)	/* Did get x-axis annotation settings */
				Ctrl->S[GMT_X].b = (Bx) ? strdup (Bx->arg) : strdup (Bxy->arg);
			if (Bxy || By)	/* Did get y-axis annotation settings */
				Ctrl->S[GMT_Y].b = (By) ? strdup (By->arg) : strdup (Bxy->arg);
			if (noB) { /* Make sure we turn off everything */
				Ctrl->S[GMT_X].extra = strdup ("+n");
				Ctrl->In.no_B = true;
			}
			else if (Bframe) {
				static char *Bx_items = "SsNnbt", *By_items = "WwEelr";
				if ((c = gmt_first_modifier (GMT, Bframe->arg, "bginot"))) {	/* Gave valid frame modifiers */
					Ctrl->S[GMT_X].extra = strdup (c);
					c[0] = '\0';	/* Chop off for now */
				}
				for (k = px = 0; k < 6; k++) /* Build up x-axis args from seletion */
					if (strchr (Bframe->arg, Bx_items[k])) Ctrl->S[GMT_X].axes[px++] = Bx_items[k];
				if (Ctrl->S[GMT_X].axes[0] && Ctrl->S[GMT_X].active) gmt_str_tolower (Ctrl->S[GMT_X].axes);	/* Used to control the non-annotated axes */
				for (k = py = 0; k < 6; k++)/* Build up y-axis args from seletion */
					if (strchr (Bframe->arg, By_items[k])) Ctrl->S[GMT_Y].axes[py++] = By_items[k];
				if (Ctrl->S[GMT_Y].axes[0] && Ctrl->S[GMT_Y].active) gmt_str_tolower (Ctrl->S[GMT_Y].axes);	/* Used to control the non-annotated axes */
				if (c) c[0] = '+';	/* Restore modifiers */
			}
		}
		if (!Bframe) {	/* No override, examine the default frame setting instead */
			if (strchr (GMT->current.setting.map_frame_axes, 'S')) Ctrl->S[GMT_X].axes[px++] = 'S';
			else if (strchr (GMT->current.setting.map_frame_axes, 's')) Ctrl->S[GMT_X].axes[px++] = 's';
			else if (strchr (GMT->current.setting.map_frame_axes, 'b')) Ctrl->S[GMT_X].axes[px++] = 'b';
			if (strchr (GMT->current.setting.map_frame_axes, 'N')) Ctrl->S[GMT_X].axes[px++] = 'N';
			else if (strchr (GMT->current.setting.map_frame_axes, 'n')) Ctrl->S[GMT_X].axes[px++] = 'n';
			else if (strchr (GMT->current.setting.map_frame_axes, 't')) Ctrl->S[GMT_X].axes[px++] = 't';
			if (strchr (GMT->current.setting.map_frame_axes, 'W')) Ctrl->S[GMT_Y].axes[py++] = 'W';
			else if (strchr (GMT->current.setting.map_frame_axes, 'w')) Ctrl->S[GMT_Y].axes[py++] = 'w';
			else if (strchr (GMT->current.setting.map_frame_axes, 'l')) Ctrl->S[GMT_Y].axes[py++] = 'l';
			if (strchr (GMT->current.setting.map_frame_axes, 'E')) Ctrl->S[GMT_Y].axes[py++] = 'E';
			else if (strchr (GMT->current.setting.map_frame_axes, 'e')) Ctrl->S[GMT_Y].axes[py++] = 'e';
			else if (strchr (GMT->current.setting.map_frame_axes, 'r')) Ctrl->S[GMT_Y].axes[py++] = 'r';
		}
		if (Ctrl->S[GMT_X].b == NULL) Ctrl->S[GMT_X].b = strdup ("af");	/* Default is -Baf if not set */
		if (Ctrl->S[GMT_Y].b == NULL) Ctrl->S[GMT_Y].b = strdup ("af");
		n_errors += gmt_M_check_condition (GMT, Ctrl->A.mode == SUBPLOT_LABEL_IS_LETTER && Ctrl->A.roman, "Option -A: Cannot select Roman numerals AND letters!\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->N.n_subplots == 0, "Number of RowsxCols is required!\n");
		n_errors += gmt_M_check_condition (GMT, !Ctrl->F.active, "Option -F: Must specify figure and subplot dimensions!\n");
	}
	else if (Ctrl->In.mode == SUBPLOT_SET) {	/* Move focus to a particular subplot, so flag other options */
		n_errors += gmt_M_check_condition (GMT, Ctrl->F.active, "Option -F: Only available for gmt subset begin!\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S[GMT_X].active || Ctrl->S[GMT_Y].active, "Option -S: Only available for gmt subset begin!\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->T.active, "Option -T: Only available for gmt subset begin!\n");

	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_subplot (void *V_API, int mode, void *args) {
	int error = 0, fig;
	char file[PATH_MAX] = {""}, command[GMT_LEN256] = {""};
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_DATASET *D = NULL, *L = NULL;
	struct GMT_OPTION *options = NULL, *opt = NULL;
	struct GMT_SUBPLOT *P = NULL;
	struct SUBPLOT_CTRL *Ctrl = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC) {
		GMT_Report (API, GMT_MSG_ERROR, "Not available in classic mode\n");
		bailout (GMT_NOT_MODERN_MODE);
	}

	/* When gmt subplot begin is called the gmt.history file for the figure or session may have a record of -R -J -X -Y
	 * settings.  Once the subplot begin starts we may or may not be given -R -J -X -Y arguments.  If not, then we do not
	 * want old history settings to leak into the subplot panels.  The past history is read during sessino creation in
	 * GMT_Create_Session so at this point it is already stored in memory.  Hence we need to reset all of that before we
	 * parse the common and specific arguments to this module */

	if ((opt = GMT_Find_Option (API, GMT_OPT_INFILE, options)) && !strcmp (opt->arg, "begin"))	/* Called gmt subplot begin */
		gmt_reset_history (API->GMT);	/* Remove any history obtained by GMT_Create_Session */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);


	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the subplot main code ----------------------------*/

	fig = gmt_get_current_figure (API);	/* Get current figure number */

	if (Ctrl->In.mode == SUBPLOT_BEGIN) {	/* Determine and save subplot attributes once */
		unsigned int row, col, k, panel, nx, ny, factor, last_row, last_col, *Lx = NULL, *Ly = NULL;
		uint64_t seg;
		double x, y, width = 0.0, height = 0.0, tick_height, annot_height, label_height, title_height, y_header_off = 0.0;
		double *cx = NULL, *cy = NULL, *px = NULL, *py = NULL, y_heading, fluff[2] = {0.0, 0.0}, off[2] = {0.0, 0.0}, GMT_LETTER_HEIGHT = 0.736;
		char **Bx = NULL, **By = NULL, *cmd = NULL, axes[3] = {""}, Bopt[GMT_LEN256] = {""};
		char vfile[GMT_VF_LEN] = {""}, xymode = 'r';
		bool add_annot, no_frame = false;
		FILE *fp = NULL;

		/* Determine if the subplot itself is an overlay of an existing plot */
		sprintf (file, "%s/gmt_%d.ps-", API->gwf_dir, fig);
		if (!access (file, F_OK)) {	/* Plot file already exists, so enter overlay mode if -X -Y nare ot set */
			if (GMT->common.X.mode == 'a' && GMT->common.Y.mode == 'a')
				xymode = 'a';
			else {
				if (!GMT->common.X.active) GMT->current.setting.map_origin[GMT_X] = 0.0;
				if (!GMT->common.Y.active) GMT->current.setting.map_origin[GMT_Y] = 0.0;
			}
		}
		else {	/* Start of new plot.  See if we gave absolute or relative coordinates in -X -Y */
			if (GMT->common.X.mode == 'a' && GMT->common.Y.mode == 'a')
				xymode = 'a';
		}

		if (xymode == 'a') gmt_M_memcpy (off, GMT->current.setting.map_origin, 2, double);
		sprintf (file, "%s/gmt.subplot.%d", API->gwf_dir, fig);
		if (!access (file, F_OK))	{	/* Subplot information file already exists, two begin subplot commands? */
			GMT_Report (API, GMT_MSG_ERROR, "Subplot information file already exists: %s\n", file);
			GMT_Report (API, GMT_MSG_ERROR, "Please run 'gmt clear sessions' (or equivalent if from Julia, Matlab, Python, etc...) to solve this issue.\n");
			Return (GMT_RUNTIME_ERROR);
		}
		/* COmpute dimensions such as ticks and distance from tick to top of annotation etc */
		tick_height   = MAX(0,GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER]);	/* Allow for axis ticks */
		annot_height  = (GMT_LETTER_HEIGHT * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH) + MAX (0.0, GMT->current.setting.map_annot_offset[GMT_PRIMARY]);	/* Allow for space between axis and annotations */
		label_height  = (GMT_LETTER_HEIGHT * GMT->current.setting.font_label.size / PSL_POINTS_PER_INCH) + MAX (0.0, GMT->current.setting.map_label_offset);
		title_height = (GMT_LETTER_HEIGHT * GMT->current.setting.font_title.size / PSL_POINTS_PER_INCH) + GMT->current.setting.map_title_offset;
		y_header_off = GMT->current.setting.map_heading_offset;
		if (Ctrl->In.no_B) tick_height = annot_height = 0.0;	/* No tick or annotations on subplot frames */
		/* Report plot/media area dimensions as we shrink them.  Here is the starting point: */
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: origin          = %g/%g\n", off[GMT_X], off[GMT_Y]);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: rows            = %d\n", Ctrl->N.dim[GMT_Y]);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: columns         = %d\n", Ctrl->N.dim[GMT_X]);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: height          = %g\n", height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: width           = %g\n", width);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: tick_height     = %g\n", tick_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: annot_height    = %g\n", annot_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: label_height    = %g\n", label_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: title_height    = %g\n", title_height);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: header_offset   = %g\n", y_header_off);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: subplot margin  = %g/%g/%g/%g\n", Ctrl->M.margin[XLO], Ctrl->M.margin[XHI], Ctrl->M.margin[YLO], Ctrl->M.margin[YHI]);
		/* Shrink these if media margins were requested */
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Start: fluff = {%g, %g}\n", fluff[GMT_X], fluff[GMT_Y]);
		/* Add up space used by interior subplot margins */
		fluff[GMT_X] += (Ctrl->N.dim[GMT_X] - 1) * (Ctrl->M.margin[XLO] + Ctrl->M.margin[XHI]);
		fluff[GMT_Y] += (Ctrl->N.dim[GMT_Y] - 1) * (Ctrl->M.margin[YLO] + Ctrl->M.margin[YHI]);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After correcting for inside subplot margins: fluff = {%g, %g}\n", fluff[GMT_X], fluff[GMT_Y]);

		/* ROW SETTINGS:  Limit tickmarks to 1 or 2 W/E axes per row or per subplot */
		/* Note: Usually, we will tick both the west and east side of a subplot.  With -SR in effect
		 * we will have selected either the left, right, or both end sides (i.e., the two outer subplots).
		 * However, this can be overridden by selecting "l" and/or "r" in the general -B string which controls
		 * all the axes not implicitly handled by -SR.  Hence, we assume two sides of ticks will need space
		 * but override if users have turned any of those sides off with l or r */

		nx = 2;	/* Default is ticks on both W and E */
		if (strchr (Ctrl->S[GMT_Y].axes, 'l')) nx--;	/* But not on the west (left) side */
		if (strchr (Ctrl->S[GMT_Y].axes, 'r')) nx--;	/* But not on the east (right) side */
		fluff[GMT_X] += (Ctrl->N.dim[GMT_X] - 1) * nx * tick_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d rows of map ticks: fluff = {%g, %g}\n", nx, fluff[GMT_X], fluff[GMT_Y]);
		/* Limit annotations/labels to 1 or 2 axes per row or per subplot */
		if (Ctrl->S[GMT_Y].annotate)	/* Used -SR so check settings */
			nx = 0;	/* For -SR there are no internal annotations, only ticks */
		else {	/* Must see what -B specified instead since that will apply to all subplots */
			nx = 0;
			if (strchr (Ctrl->S[GMT_Y].axes, 'W')) nx++;	/* All subplots need west annotations */
			if (strchr (Ctrl->S[GMT_Y].axes, 'E')) nx++;	/* All subplots need east annotations */
		}
		if (!Ctrl->S[GMT_Y].active) {
			nx *= (Ctrl->N.dim[GMT_X] - 1);	/* Each column needs separate y-axis [and labels] */
			fluff[GMT_X] += nx * annot_height;
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d row annots: fluff = {%g, %g}\n", nx, fluff[GMT_X], fluff[GMT_Y]);
		if (Ctrl->S[GMT_Y].has_label)	/* Want labels, so increase fluff */
			fluff[GMT_X] += nx * label_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d row labels: fluff = {%g, %g}\n", nx, fluff[GMT_X], fluff[GMT_Y]);

		/* COLUMN SETTINGS: Limit annotations/labels to 1 or 2 axes per column or per subplot */
		/* Note: Usually, we will tick both the south and north side of a subplot.  With -SC in effect
		 * we will have selected either the bottom, top, or both end sides (i.e., the two outer subplots).
		 * However, this can be overridden by selecting "b" and/or "t" in the general -B string which controls
		 * all the axes not implicitly handled by -SC.  Hence, we assume two sides of ticks will need space
		 * but override if users have turned any of those sides off with b or t */

		ny = 2;	/* Default is ticks on both S and N */
		if (strchr (Ctrl->S[GMT_X].axes, 'b')) ny--;	/* But not on the south (bottom) side */
		if (strchr (Ctrl->S[GMT_X].axes, 't')) ny--;	/* But not on the north (top) side */
		else y_header_off += tick_height;
		fluff[GMT_Y] += (Ctrl->N.dim[GMT_Y] - 1) * ny * tick_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d col ticks: fluff = {%g, %g}\n", ny, fluff[GMT_X], fluff[GMT_Y]);
		if (Ctrl->S[GMT_X].annotate) {	/* Gave -SC so check settings */
			ny = 0;	/* For -Sc there are no internal annotations, only ticks */
		}
		else {	/* Must see what -B specified instead since that will apply to all subplots */
			ny = 0;
			if (strchr (Ctrl->S[GMT_X].axes, 'S')) ny++;	/* All subplots need south annotations */
			if (strchr (Ctrl->S[GMT_X].axes, 'N')) {
				ny++;	/* All subplots need north annotations */
				y_header_off += annot_height;
			}
		}
		factor = (Ctrl->S[GMT_X].active) ? 1 : Ctrl->N.dim[GMT_Y];	/* Only one or maybe each row may need separate x-axis [and labels] */
		ny *= (factor - 1);
		fluff[GMT_Y] += ny * annot_height;
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d col annot: fluff = {%g, %g}\n", ny, fluff[GMT_X], fluff[GMT_Y]);
		if (Ctrl->S[GMT_X].has_label) {
			fluff[GMT_Y] += ny * label_height;
			if (ny == 2 || Ctrl->S[GMT_X].annotate & SUBPLOT_PLACE_AT_MAX) y_header_off += label_height;
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d col labels: fluff = {%g, %g}\n", ny, fluff[GMT_X], fluff[GMT_Y]);
		if (Ctrl->S[GMT_X].ptitle == SUBPLOT_PANEL_TITLE) {	/* Each subplot in column need space for title */
			fluff[GMT_Y] += (Ctrl->N.dim[GMT_Y]-1) * title_height;
			y_header_off += title_height;
		}
		else if (Ctrl->S[GMT_X].ptitle == SUBPLOT_PANEL_COL_TITLE) {	/* Just one title on top row, per column */
			y_header_off += title_height;
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: After %d subplot titles: fluff = {%g, %g}\n", factor, fluff[GMT_X], fluff[GMT_Y]);
		if (Ctrl->F.mode == SUBPLOT_FIGURE) {	/* Got figure dimension, compute subplot dimensions */
			for (col = 0; col < Ctrl->N.dim[GMT_X]; col++) Ctrl->F.w[col] *= (Ctrl->F.dim[GMT_X] - fluff[GMT_X]);
			for (row = 0; row < Ctrl->N.dim[GMT_Y]; row++) Ctrl->F.h[row] *= (Ctrl->F.dim[GMT_Y] - fluff[GMT_Y]);
		}
		else {	/* Already got subplot dimension, compute total figure dimension */
			if (Ctrl->F.reset_h) {	/* Update h based on map aspect ratio and width of a constant column */
				for (row = 0; row < Ctrl->N.dim[GMT_Y]; row++) Ctrl->F.h[row] = Ctrl->F.w[0] * (GMT->current.map.height / GMT->current.map.width);
			}
			/* Sum up individual widths or heights and add the fluff space */
			for (col = 0, Ctrl->F.dim[GMT_X] = fluff[GMT_X]; col < Ctrl->N.dim[GMT_X]; col++) Ctrl->F.dim[GMT_X] += Ctrl->F.w[col];
			for (row = 0, Ctrl->F.dim[GMT_Y] = fluff[GMT_Y]; row < Ctrl->N.dim[GMT_Y]; row++) Ctrl->F.dim[GMT_Y] += Ctrl->F.h[row];
		}
		/* Plottable area: */
		width  = Ctrl->F.dim[GMT_X];
		height = Ctrl->F.dim[GMT_Y];
		y_heading = height + y_header_off + Ctrl->M.margin[YHI];
		//y_heading = height + y_header_off;
		if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG)) {	/* Lots of debug calculations and reporting here */
			size_t len, tlen;
			char report[GMT_LEN256] = {""}, txt[GMT_LEN32] = {""};
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Figure width:  %g\n", Ctrl->F.dim[GMT_X]);
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Figure height: %g\n", Ctrl->F.dim[GMT_Y]);
			sprintf (report, "%g", Ctrl->F.w[0]);
			for (col = 1; col < Ctrl->N.dim[GMT_X]; col++) {
				len = strlen (report);
				sprintf (txt, ", %g", Ctrl->F.w[col]);
				tlen = strlen (txt);
				if ((len+tlen) < GMT_LEN256) {	/* OK to add to report */
					if (col == 10) {	/* Don't bother to give more than the first 10 columns */
						sprintf (txt, ", ...");
						col = Ctrl->N.dim[GMT_X];	/* This will exit the loop */
					}
					strcat (report, txt);
				}
			}
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Column dimensions: %s\n", report);
			sprintf (report, "%g", Ctrl->F.h[0]);
			for (row = 1; row < Ctrl->N.dim[GMT_Y]; row++) {
				len = strlen (report);
				sprintf (txt, ", %g", Ctrl->F.h[row]);
				tlen = strlen (txt);
				if ((len+tlen) < GMT_LEN256) {	/* OK to add to report */
					if (row == 10) {	/* Don't bother to give more than the first 10 rows */
						sprintf (txt, ", ...");
						row = Ctrl->N.dim[GMT_Y];	/* This will exit the loop */
					}
					strcat (report, txt);
				}
			}
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Row dimensions: %s\n", report);
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Main heading BC point: %g %g\n", 0.5 * width, y_heading);
		}

		/* Allocate subplot info array */
		cx = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_X], double);
		cy = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_Y], double);
		px = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_X], double);
		py = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_Y], double);
		Bx = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_X], char *);
		By = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_Y], char *);
		Lx = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_X], unsigned int);
		Ly = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_Y], unsigned int);

		if (Ctrl->A.roman) {	/* Replace %d with %s since roman numerals are typeset as strings */
			char *c = strchr (Ctrl->A.format, '%');
			c[1] = 's';
		}

		last_row = Ctrl->N.dim[GMT_Y] - 1;
		last_col = Ctrl->N.dim[GMT_X] - 1;

		if (Ctrl->F.debug) {	/* Must draw helper lines so we need to allocate a dataset */
			uint64_t dim[4] = {1, Ctrl->N.n_subplots, 4, 2};	/* One segment polygon per subplot */
			if ((D = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_POLY, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Subplot: Unable to allocate a dataset\n");
				Return (error);
			}
		}

		/* Need to determine how many y-labels, annotations and ticks for each row since it affects subplot size */

		y = Ctrl->F.dim[GMT_Y];	/* Start off at top edge of figure area */
		for (row = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each row of subplots */
			axes[0] = axes[1] = 0;	k = 0;
			if (row) y -= Ctrl->M.margin[YHI];	/* Add margin for the top of this row (except top row) */
			if (Ctrl->F.debug) {	/* All rows share this upper y-coordinate */
				for (col = 0; col < Ctrl->N.dim[GMT_X]; col++) {	/* For each col of subplots */
					seg = (uint64_t)(row * Ctrl->N.dim[GMT_X]) + col;
					D->table[0]->segment[seg]->data[GMT_Y][2] = D->table[0]->segment[seg]->data[GMT_Y][3] = y + off[GMT_Y];
					D->table[0]->segment[seg]->n_rows = 4;
				}
			}
			if ((row == 0 && Ctrl->S[GMT_X].ptitle == SUBPLOT_PANEL_COL_TITLE) || (Ctrl->S[GMT_X].ptitle == SUBPLOT_PANEL_TITLE)) {
				if (row) y -= title_height;	/* Make space for subplot title if after row 0 */
			}
			if (Ctrl->S[GMT_X].active)	/* May need shared annotation at top N frame */
				add_annot = (row == 0 && (Ctrl->S[GMT_X].annotate & SUBPLOT_PLACE_AT_MAX));
			else	/* Not shared, if so all or none of N axes are annotated */
				add_annot = (Ctrl->S[GMT_X].annotate & SUBPLOT_PLACE_AT_MAX);
			if (add_annot || (!Ctrl->S[GMT_X].active && strchr (Ctrl->S[GMT_X].axes, 'N'))) {
				axes[k++] = 'N';	/* Need annotation at N */
				if (row) y -= (annot_height + tick_height);
				if (Ctrl->S[GMT_X].has_label) {
					if (row) y -= label_height;	/* Also has label at N */
					Ly[row] |= SUBPLOT_PLACE_AT_MAX;
				}
			}
			else if (strchr (Ctrl->S[GMT_X].axes, 'n')) {
				axes[k++] = 'n';	/* Only need ticks at north */
				if (row) y -= tick_height;
			}
			else if (strchr (Ctrl->S[GMT_X].axes, 't'))
				axes[k++] = 't';	/* No ticks, just draw line axis */
			y -= Ctrl->F.h[row];	/* Now position y at to bottom of subplot */
			py[row] = y;
			if (Ctrl->S[GMT_X].active)	/* May need shared annotation at bottom S */
				add_annot = (row == last_row && (Ctrl->S[GMT_X].annotate & SUBPLOT_PLACE_AT_MIN));
			else	/* Not shared, if so all or none of N axes are annotated */
				add_annot = (Ctrl->S[GMT_X].annotate & SUBPLOT_PLACE_AT_MIN);
			if (add_annot || (!Ctrl->S[GMT_X].active && strchr (Ctrl->S[GMT_X].axes, 'S'))) {
				axes[k++] = 'S';	/* Need annotation on south frame */
				if (row < last_row) y -= (annot_height + tick_height);
				if (Ctrl->S[GMT_X].has_label)
					if (row < last_row) y -= label_height;	/* Also has label at S */
				Ly[row] |= SUBPLOT_PLACE_AT_MIN;
			}
			else if (strchr (Ctrl->S[GMT_X].axes, 's')) {
				axes[k++] = 's';	/* Just needs ticks on south frame */
				if (row < last_row) y -= tick_height;
			}
			else if (strchr (Ctrl->S[GMT_X].axes, 'b'))
				axes[k++] = 'b';	/* Just draw line frame at south end */
			By[row] = strdup (axes);	/* Those are all the y-frame settings */
			if (Ctrl->F.debug) {	/* All rows share this lower y-coordinate */
				for (col = 0; col < Ctrl->N.dim[GMT_X]; col++) {	/* For each col of subplots */
					seg = (uint64_t)(row * Ctrl->N.dim[GMT_X]) + col;
					D->table[0]->segment[seg]->data[GMT_Y][0] = D->table[0]->segment[seg]->data[GMT_Y][1] = y + off[GMT_Y];
				}
			}
			if (row < last_row) y -= Ctrl->M.margin[YLO];	/* Only add interior margins */
			cy[row] = y;	/* Center y-value between two rows */
		}
		x = 0.0;	/* Start off at left edge of plot area */
		for (col = 0; col < Ctrl->N.dim[GMT_X]; col++) {	/* For each col of subplots */
			axes[0] = axes[1] = 0;	k = 0;
			if (col) x += Ctrl->M.margin[XLO];	/* Only add interior margins */
			if (Ctrl->F.debug) {	/* All cols share this left x-coordinate */
				for (row = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each col of subplots */
					seg = (uint64_t)(row * Ctrl->N.dim[GMT_X]) + col;
					D->table[0]->segment[seg]->data[GMT_X][0] = D->table[0]->segment[seg]->data[GMT_X][3] = x + off[GMT_X];
				}
			}
			if (Ctrl->S[GMT_Y].active)	/* May need shared annotation at left W */
				add_annot = (col == 0 && Ctrl->S[GMT_Y].annotate & SUBPLOT_PLACE_AT_MIN);
			else	/* Not shared, if so all or none of W axes are annotated */
				add_annot = (Ctrl->S[GMT_Y].annotate & SUBPLOT_PLACE_AT_MIN);

			if (add_annot || (!Ctrl->S[GMT_Y].active && strchr (Ctrl->S[GMT_Y].axes, 'W'))) {
				axes[k++] = 'W';	/* Need annotations on west frame */
				if (col) x += (annot_height + tick_height);
				if (Ctrl->S[GMT_Y].has_label) {
					if (col) x += label_height;	/* Also has label at W */
					Lx[col] |= SUBPLOT_PLACE_AT_MIN;
				}
			}
			else if (strchr (Ctrl->S[GMT_Y].axes, 'w')) {
				if (col) x += tick_height;
				axes[k++] = 'w';	/* Only ticks at west frame */
			}
			else if (strchr (Ctrl->S[GMT_Y].axes, 'l'))
				axes[k++] = 'l';	/* Just draw west frame simple line */
			px[col] = x;	/* Now at correct x for left side or this subplot */
			x += Ctrl->F.w[col];	/* Position x at the right side of subplot */
			if (Ctrl->S[GMT_Y].active)	/* May need shared annotation at right E */
				add_annot = (col == last_col && (Ctrl->S[GMT_Y].annotate & SUBPLOT_PLACE_AT_MAX));
			else	/* Not shared, if so all or none of W axes are annotated */
				add_annot = (Ctrl->S[GMT_Y].annotate & SUBPLOT_PLACE_AT_MAX);
			if (add_annot || (!Ctrl->S[GMT_Y].active && strchr (Ctrl->S[GMT_Y].axes, 'E'))) {
				axes[k++] = 'E';	/* Annotate frame on right side */
				if (col < last_col) x += (annot_height + tick_height);
				if (Ctrl->S[GMT_Y].has_label) {
					if (col < last_col) x += label_height;	/* Also has label at E */
					Lx[col] |= SUBPLOT_PLACE_AT_MAX;
				}
			}
			else if (strchr (Ctrl->S[GMT_Y].axes, 'e')) {
				if (col < last_col) x += tick_height;
				axes[k++] = 'e';	/* Only add ticks at east frame */
			}
			else if (strchr (Ctrl->S[GMT_Y].axes, 'r'))
				axes[k++] = 'r';	/* Just draw line at east frame */
			if (Ctrl->F.debug) {	/* All cols share this right x-coordinate */
				for (row = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each col of subplots */
					seg = (uint64_t)(row * Ctrl->N.dim[GMT_X]) + col;
					D->table[0]->segment[seg]->data[GMT_X][1] = D->table[0]->segment[seg]->data[GMT_X][2] = x + off[GMT_X];
				}
			}
			if (col < last_col) x += Ctrl->M.margin[XHI];	/* Only add interior margins */
			cx[col] = x;	/* Center x between two tiles */
			Bx[col] = strdup (axes);	/* All the x-frame settings */
		}

		if (Ctrl->S[GMT_X].extra && !strncmp (Ctrl->S[GMT_X].extra, "+n", 2U)) /* Did not want the frame */
			no_frame = true;

		/* Write out the subplot information file */

		if ((fp = fopen (file, "w")) == NULL) {	/* Not good */
			GMT_Report (API, GMT_MSG_ERROR, "Cannot create file %s\n", file);
			Return (GMT_ERROR_ON_FOPEN);
		}
		fprintf (fp, "# subplot information file\n");
		cmd = GMT_Create_Cmd (API, options);
		fprintf (fp, "# Command: %s %s\n", THIS_MODULE_CLASSIC_NAME, cmd);
		gmt_M_free (GMT, cmd);
		if (Ctrl->T.active) fprintf (fp, "# HEADING: %g %g %s\n", 0.5 * width, y_heading, Ctrl->T.title);
		fprintf (fp, "# ORIGIN: %g %g\n", off[GMT_X], off[GMT_Y]);
		fprintf (fp, "# DIMENSION: %g %g\n", width, height);
		fprintf (fp, "# PARALLEL: %d\n", Ctrl->S[GMT_Y].parallel);
		if (Ctrl->C.active) {	/* Got common gaps setting */
			gmt_M_memcpy (GMT->current.plot.panel.gap, Ctrl->C.gap, 4, double);
			fprintf (fp, "# GAPS: %g %g %g %g\n", Ctrl->C.gap[XLO], Ctrl->C.gap[XHI], Ctrl->C.gap[YLO], Ctrl->C.gap[YHI]);
		}
		if (GMT->common.J.active && GMT->current.proj.projection == GMT_LINEAR)	/* Write axes directions for Cartesian plots as +1 or -1 */
			fprintf (fp, "# DIRECTION: %d %d\n", 2*GMT->current.proj.xyz_pos[GMT_X]-1, 2*GMT->current.proj.xyz_pos[GMT_Y]-1);
		/* Write header record */
		fprintf (fp, "#subplot\trow\tcol\tnrow\tncol\tx0\ty0\tw\th\ttag\ttag_dx\ttag_dy\ttag_clearx\ttag_cleary\ttag_pos\ttag_just\ttag_fill\ttag_pen\tBframe\tBx\tBy\tAx\tAy\n");
		for (row = panel = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each row of subplots */
			for (col = 0; col < Ctrl->N.dim[GMT_X]; col++, panel++) {	/* For each col of subplots */
				k = (Ctrl->A.way == GMT_IS_COL_FORMAT) ? col * Ctrl->N.dim[GMT_Y] + row : row * Ctrl->N.dim[GMT_X] + col;
				fprintf (fp, "%d\t%d\t%d\t%d\t%d\t%.4f\t%.4f\t%.4f\t%.4f\t", panel, row, col, Ctrl->N.dim[GMT_Y], Ctrl->N.dim[GMT_X], px[col], py[row], Ctrl->F.w[col], Ctrl->F.h[row]);
				if (Ctrl->A.active) {	/* Requested automatic tags per subplot */
					if (Ctrl->A.mode == SUBPLOT_LABEL_IS_NUMBER) {	/* Use numbers in the tag */
						if (Ctrl->A.roman) {	/* But formatted using upper- or lower-case Roman numerals */
							char roman[GMT_LEN32] = {""};
							(void)gmt_arabic2roman (Ctrl->A.nstart + k, roman, GMT_LEN32, Ctrl->A.roman == GMT_IS_ROMAN_LCASE);
							fprintf (fp, Ctrl->A.format, roman);
						}
						else	/* Typeset as arabic integer */
							fprintf (fp, Ctrl->A.format, Ctrl->A.nstart + k);
					}
					else	/* A character, perhaps offset from a) */
						fprintf (fp, Ctrl->A.format, Ctrl->A.cstart + k);
					fprintf (fp, "\t%g\t%g\t%g\t%g\t%s\t%s", Ctrl->A.off[GMT_X], Ctrl->A.off[GMT_Y], Ctrl->A.clearance[GMT_X], Ctrl->A.clearance[GMT_Y], Ctrl->A.placement, Ctrl->A.justify);
					fprintf (fp, "\t%s\t%s", Ctrl->A.fill, Ctrl->A.pen);
				}
				else	/* No subplot tags, write default fillers */
					fprintf (fp, "-\t0\t0\t0\t0\tBL\tBL\t-\t-");
				/* Now the four -B settings items placed between GMT_ASCII_GS characters since there may be spaces in them */
				if (no_frame)	/* Default filler since we don't want any frames */
					fprintf (fp, "\t%c+n%c%c%c%c%c\n", GMT_ASCII_GS, GMT_ASCII_GS, GMT_ASCII_GS, GMT_ASCII_GS, GMT_ASCII_GS, GMT_ASCII_GS);	/* Pass -B+n only */
				else {
					if ((Bx[col] && Bx[col][0]) || (By[row] && By[row][0]))
						fprintf (fp, "\t%c%s%s", GMT_ASCII_GS, Bx[col], By[row]);	/* These are the axes to draw/annotate for this subplot */
					else
						fprintf (fp, "\t%c+n", GMT_ASCII_GS);	/* No frames for this subplot */
					if (Ctrl->S[GMT_X].extra) fprintf (fp, "%s", Ctrl->S[GMT_X].extra);	/* Add frame modifiers to the axes */
					fprintf (fp,"%c", GMT_ASCII_GS); 	/* Next is x labels,  Either given or just XLABEL */
					if (Ly[row] && Ctrl->S[GMT_X].label[GMT_PRIMARY]) fprintf (fp,"%s", Ctrl->S[GMT_X].label[GMT_PRIMARY]);
					fprintf (fp, "%c", GMT_ASCII_GS); 	/* Next is y labels,  Either given or just YLABEL */
					if (Lx[col] && Ctrl->S[GMT_Y].label[GMT_PRIMARY]) fprintf (fp, "%s", Ctrl->S[GMT_Y].label[GMT_PRIMARY]);
					fprintf (fp, "%c%s", GMT_ASCII_GS, Ctrl->S[GMT_X].b); 	/* Next is x annotations [af] */
					fprintf (fp, "%c%s", GMT_ASCII_GS, Ctrl->S[GMT_Y].b); 	/* Next is y annotations [af] */
					fprintf (fp, "%c\n", GMT_ASCII_GS);	/* End of this subplot */
				}
			}
			gmt_M_str_free (By[row]);	/* Done with this guy */
		}
		fclose (fp);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Wrote %d subplot settings to information file %s\n", panel, file);
		sprintf (file, "%s/gmt.subplotorder.%d", API->gwf_dir, fig);	/* File with dimensions and ordering */
		if ((fp = fopen (file, "w")) == NULL) {	/* Not good */
			GMT_Report (API, GMT_MSG_ERROR, "Cannot create file %s\n", file);
			Return (GMT_ERROR_ON_FOPEN);
		}
		fprintf (fp, "%d %d %d\n", Ctrl->N.dim[GMT_Y], Ctrl->N.dim[GMT_X], Ctrl->A.way);
		fclose (fp);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Wrote subplot order to information file %s\n", file);

		if (Ctrl->F.Lpen[0]) {	/* Must draw divider lines between subplots so we need to allocate a dataset */
			uint64_t dim[4] = {1, last_row + last_col, 2, 2};	/* One segment line per interior row/col */
			if ((L = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_LINE, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Subplot: Unable to allocate a dataset\n");
				Return (error);
			}
			for (row = 0, seg = 0; row < last_row; row++, seg++) {	/* Horizontal lines */
				L->table[0]->segment[seg]->data[GMT_X][0] = 0.0;
				L->table[0]->segment[seg]->data[GMT_X][1] = Ctrl->F.dim[GMT_X];
				L->table[0]->segment[seg]->data[GMT_Y][0] = L->table[0]->segment[seg]->data[GMT_Y][1] = cy[row];
			}
			for (col = 0; col < last_col; col++, seg++) {	/* Vertical lines */
				L->table[0]->segment[seg]->data[GMT_Y][0] = 0.0;
				L->table[0]->segment[seg]->data[GMT_Y][1] = Ctrl->F.dim[GMT_Y];
				L->table[0]->segment[seg]->data[GMT_X][0] = L->table[0]->segment[seg]->data[GMT_X][1] = cx[col];
			}
		}

		/* Start the subplot with a blank canvas and place the optional title.
		   The blank canvas dimensions should become the -R and -Jx1 once subplot ends */

		if (Ctrl->F.fill[0] != '-' && Ctrl->F.pen[0] != '-')	/* Need to fill and draw the canvas box */
			sprintf (Bopt, " -B+g%s -B0 --MAP_FRAME_PEN=%s", Ctrl->F.fill, Ctrl->F.pen);
		else if (Ctrl->F.fill[0] != '-')	/* Need to just fill the canvas box */
			sprintf (Bopt, " -B+g%s", Ctrl->F.fill);
		else if (Ctrl->F.pen[0] != '-')	/* Need to just draw the canvas box */
			sprintf (Bopt, " -B0 --MAP_FRAME_PEN=%s", Ctrl->F.pen);
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot Bopt: [%s]\n", Bopt);
		width  += 2.0 * Ctrl->F.clearance[GMT_X];
		height += 2.0 * Ctrl->F.clearance[GMT_Y];

		if (Ctrl->T.title) {	/* Must call text to place a heading */
			uint64_t dim[4] = {1, 1, 1, 2};	/* A single record */
			struct GMT_DATASET *T = NULL;
			if ((T = GMT_Create_Data (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_WITH_STRINGS, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Subplot: Unable to allocate a dataset\n");
				Return (error);
			}
			T->table[0]->segment[0]->data[GMT_X][0] = 0.5 * width;	/* Centered */
			T->table[0]->segment[0]->data[GMT_Y][0] = y_heading + Ctrl->F.clearance[GMT_Y];	/* On top */
			T->table[0]->segment[0]->text[0] = strdup (Ctrl->T.title);
			T->table[0]->segment[0]->n_rows = 1;
			T->n_records = T->table[0]->n_records = T->table[0]->segment[0]->n_rows = 1;
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN, T, vfile) != GMT_NOERROR) {
				Return (API->error);
			}
			sprintf (command, "-R0/%g/0/%g -Jx1i -N -F+jBC+f%s %s -X%c%gi -Y%c%gi --GMT_HISTORY=false",
				width, height, gmt_putfont (GMT, &GMT->current.setting.font_heading), vfile, xymode, GMT->current.setting.map_origin[GMT_X]-Ctrl->F.clearance[GMT_X], xymode, GMT->current.setting.map_origin[GMT_Y]-Ctrl->F.clearance[GMT_Y]);
			if (Bopt[0] == ' ') strcat (command, Bopt);	/* The -B was set above, so include it in the command */
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot command for text: %s\n", command);
			if (GMT_Call_Module (API, "text", GMT_MODULE_CMD, command) != GMT_OK)	/* Plot the canvas with heading */
				Return (API->error);
			if (GMT_Destroy_Data (API, &T) != GMT_OK)
				Return (API->error);
		}
		else {	/* plot is required, since nothing is plotted (except for possibly the canvas fill/outline) */
			sprintf (command, "-R0/%g/0/%g -Jx1i -T -X%c%gi -Y%c%gi --GMT_HISTORY=false", width, height, xymode, GMT->current.setting.map_origin[GMT_X]-Ctrl->F.clearance[GMT_X], xymode, GMT->current.setting.map_origin[GMT_Y]-Ctrl->F.clearance[GMT_Y]);
			if (Bopt[0]) strcat (command, Bopt);	/* The -B was set above, so include it in the command */
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot command for : %s\n", command);
			if (GMT_Call_Module (API, "plot", GMT_MODULE_CMD, command) != GMT_OK)	/* Plot the canvas with heading */
				Return (API->error);
		}
		if (fabs (Ctrl->F.clearance[GMT_X]) > 0.0 || fabs (Ctrl->F.clearance[GMT_Y]) > 0.0) {	/* Must reset origin */
			width  -= 2.0 * Ctrl->F.clearance[GMT_X];
			height -= 2.0 * Ctrl->F.clearance[GMT_Y];
			sprintf (command, "-R0/%g/0/%g -Jx1i -T -X%c%gi -Y%c%gi --GMT_HISTORY=false", width, height, 'r', Ctrl->F.clearance[GMT_X], 'r', Ctrl->F.clearance[GMT_Y]);
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot command for plot: %s\n", command);
			if (GMT_Call_Module (API, "plot", GMT_MODULE_CMD, command) != GMT_OK)	/* Plot the canvas with heading */
				Return (API->error);
		}
		if (Ctrl->F.Lpen[0]) {	/* Draw lines between interior tiles */
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, L, vfile) != GMT_NOERROR) {
				Return (API->error);
			}
			sprintf (command, "-R0/%g/0/%g -Jx1i -W%s %s --GMT_HISTORY=false", Ctrl->F.dim[GMT_X] + GMT->current.setting.map_origin[GMT_X], Ctrl->F.dim[GMT_Y] + GMT->current.setting.map_origin[GMT_Y], Ctrl->F.Lpen, vfile);
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot command for plot: %s\n", command);
			if (GMT_Call_Module (API, "plot", GMT_MODULE_CMD, command) != GMT_OK)	/* Plot the canvas with heading */
				Return (API->error);
		}
		if (Ctrl->F.debug) {	/* Write a debug file with subplot polygons for use by "gmt subplot end" */
			bool save = GMT->current.setting.io_header[GMT_OUT];
			sprintf (file, "%s/gmt.subplotdebug.%d", API->gwf_dir, fig);
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
		gmt_M_free (GMT, cx);
		gmt_M_free (GMT, cy);
		gmt_M_free (GMT, px);
		gmt_M_free (GMT, py);
		gmt_M_free (GMT, Bx);
		gmt_M_free (GMT, By);
		gmt_M_free (GMT, Lx);
		gmt_M_free (GMT, Ly);
	}
	else if (Ctrl->In.mode == SUBPLOT_SET) {	/* SUBPLOT_SET */
		char legend_justification[4] = {""};
		double gap[4], legend_width = 0.0, legend_scale = 1.0;

		if (gmt_get_legend_info (API, &legend_width, &legend_scale, legend_justification)) {	/* Unplaced legend file */
			char cmd[GMT_LEN128] = {""};
			if ((P = gmt_subplot_info (API, fig)) == NULL) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "No subplot information file!\n");
				Return (GMT_ERROR_ON_FOPEN);
			}
			/* Default to white legend with 1p frame offset 0.2 cm from selected justification point [TR] */
			snprintf (cmd, GMT_LEN128, "-Dj%s+w%gi+o0.2c -F+p1p+gwhite -S%g -Xa%gi -Ya%gi", legend_justification, legend_width, legend_scale, P->origin[GMT_X] + P->x, P->origin[GMT_Y] + P->y);
			if ((error = GMT_Call_Module (API, "legend", GMT_MODULE_CMD, cmd))) {
				GMT_Report (API, GMT_MSG_ERROR, "Failed to place legend on current subplot figure\n");
				Return (error);
			}
		}

		/* Update the previous plot width and height to that of the entire subplot instead of just last subplot */
		gmt_subplot_gaps (API, fig, gap);	/* First see if there were subplot-wide -Cgaps settings in effect */
		if (Ctrl->In.col == INT_MAX || Ctrl->In.next) {	/* Auto-determine which subplot */
			if ((error = gmt_get_next_panel (API, fig, &Ctrl->In.row, &Ctrl->In.col)))	/* Bad */
				Return (error)
		}
		if ((error = gmt_set_current_panel (API, fig, Ctrl->In.row, Ctrl->In.col, Ctrl->C.active ? Ctrl->C.gap : gap, Ctrl->A.format, 1)))
			Return (error)
	}
	else {	/* SUBPLOT_END */
		int k, id;
		char *wmode[2] = {"w","a"}, vfile[GMT_VF_LEN] = {""}, Rtxt[GMT_LEN64] = {""}, tag[GMT_LEN16] = {""};
		char legend_justification[4] = {""}, Jstr[3] = {"J"};
		double legend_width = 0.0, legend_scale = 1.0;
		FILE *fp = NULL;

		if ((P = gmt_subplot_info (API, fig)) == NULL) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "No subplot information file!\n");
			Return (GMT_ERROR_ON_FOPEN);
		}

		if (gmt_get_legend_info (API, &legend_width, &legend_scale, legend_justification)) {	/* Unplaced legend file */
			char cmd[GMT_LEN128] = {""};
			/* Default to white legend with 1p frame offset 0.2 cm from selected justification point [TR] */
			snprintf (cmd, GMT_LEN128, "-Dj%s+w%gi+o0.2c -F+p1p+gwhite -S%g -Xa%gi -Ya%gi", legend_justification, legend_width, legend_scale, P->origin[GMT_X] + P->x, P->origin[GMT_Y] + P->y);
			if ((error = GMT_Call_Module (API, "legend", GMT_MODULE_CMD, cmd))) {
				GMT_Report (API, GMT_MSG_ERROR, "Failed to place legend on current subplot figure\n");
				Return (error);
			}
		}

		/* Update the previous plot width and height to that of the entire subplot instead of just last subplot */
		API->GMT->current.map.width  = P->dim[GMT_X];
		API->GMT->current.map.height = P->dim[GMT_Y];
		P->active = 0;	/* Ensure subplot mode is now terminated */

		if ((k = gmt_set_psfilename (GMT)) == GMT_NOTSET) {	/* Get hidden file name for PS */
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "No workflow directory\n");
			Return (GMT_ERROR_ON_FOPEN);
		}
		if ((fp = PSL_fopen (GMT->PSL, GMT->current.ps.filename, wmode[k])) == NULL) {	/* Must open inside PSL DLL */
			GMT_Report (API, GMT_MSG_ERROR, "Cannot open %s with mode %s\n", GMT->current.ps.filename, wmode[k]);
			Return (GMT_ERROR_ON_FOPEN);
		}
		/* Must force PSL_plot_completion procedure to run, if it was set */
		PSL_command (GMT->PSL, "PSL_plot_completion /PSL_plot_completion {} def\n");	/* Run once, then make it a null function */
		if (PSL_fclose (GMT->PSL)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to close hidden PS file %s!\n", GMT->current.ps.filename);
			Return (GMT_RUNTIME_ERROR);
		}
		/* Remove all subplot information files */
		gmt_history_tag (API, tag);
		sprintf (file, "%s/%s.%s", GMT->parent->gwf_dir, GMT_HISTORY_FILE, tag);
		gmt_remove_file (GMT, file);
		sprintf (file, "%s/gmt.subplot.%d", API->gwf_dir, fig);
		gmt_remove_file (GMT, file);
		sprintf (file, "%s/gmt.subplotorder.%d", API->gwf_dir, fig);
		gmt_remove_file (GMT, file);
		sprintf (file, "%s/gmt.panel.%d", API->gwf_dir, fig);
		gmt_remove_file (GMT, file);
		/* Check if we should draw debug lines */
		sprintf (file, "%s/gmt.subplotdebug.%d", API->gwf_dir, fig);
		if (!access (file, R_OK)) {	/* Yes, must draw debug lines on top */
			if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POLY, 0, NULL, file, NULL)) == NULL) {
				Return (API->error);
			}
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POLY, GMT_IN, D, vfile) != GMT_NOERROR) {
				Return (API->error);
			}
			sprintf (command, "-R%s -Jx1i %s -L -Wfaint,red -Xa0i -Ya0i --GMT_HISTORY=false", D->table[0]->header[0], vfile);
			GMT_Report (API, GMT_MSG_DEBUG, "Subplot command for plot to draw debug lines: %s\n", command);
			if (GMT_Call_Module (API, "plot", GMT_MODULE_CMD, command) != GMT_OK)	/* Plot the canvas with heading */
				Return (API->error);
			if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {
				Return (API->error);
			}
			gmt_remove_file (GMT, file);
		}
		GMT_Report (API, GMT_MSG_DEBUG, "Subplot: Removed subplot files\n");

		/* Set -R and J to match subplot frame size so later calls, e.g., colorbar, can use -DJ for positioning */
		/* First set R (i.e., RP suitable for plotting) */
		id = gmt_get_option_id (0, "R");	/* The -RP history index */
		if (GMT->init.history[id]) gmt_M_str_free (GMT->init.history[id]);	/* Remove whatever this was */
		sprintf (Rtxt, "0/%.16g/0/%.16g", P->dim[GMT_X], P->dim[GMT_Y]);	/* Range for the subplot frame */
		GMT->init.history[id] = strdup (Rtxt);	/* Update with the dimension of the whole subplot frame */
		/* Now add a -Jx1i projection to the history */
		id = gmt_get_option_id (0, "J");	/* Top level -J history */
		if (id > 0 && GMT->init.history[id]) {	/* There should/must be an entry but we check id nevertheless */
			Jstr[1] = GMT->init.history[id][0];	/* The actual -J? that was used in the last subplot panel */
			gmt_M_str_free (GMT->init.history[id]);	/* Remove whatever that was */
			GMT->init.history[id] = strdup ("x");	/* Just place the linear projection code */
			if ((id = gmt_get_option_id (id + 1, Jstr)) >= 0 && GMT->init.history[id]) gmt_M_str_free (GMT->init.history[id]);	/* Remove the subplot -J? entry */
		}
		id = gmt_get_option_id (id, "Jx");	/* Find previous -Jx history, if any */
		if (id > 0 && GMT->init.history[id]) gmt_M_str_free (GMT->init.history[id]);	/* Remove what this was */
		GMT->init.history[id] = strdup ("x1i");	/* Add a scale of 1 inch per unit to match the inches we gave in the -R history */

		gmt_M_memset (&GMT->current.plot.panel, 1, struct GMT_SUBPLOT);	/* Wipe that smile off your face */
	}

	Return (error);
}
