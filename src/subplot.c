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
 *	gmt subplot enter -N<nrows>/<ncols> [-A<labels>] [-F<WxH>] [-L<layout>] [-M[m|p]<margins>] [-T<title>] [-V]
 *	gmt subplot exit [-V]
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"subplot"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Set multi-panel figure attributes under a GMT modern mode session"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

/* Control structure for subplot */

#define CM_TO_INCH (1.0/2.54)
#define ENTER	1
#define EXIT	0
#define MEDIA	1
#define PANEL	0
#define LABEL_IS_LETTER	0
#define LABEL_IS_NUMBER	1
#define COL_FIXED_X	1
#define ROW_FIXED_Y	2
#define PANEL_COL_TITLE	1
#define PANEL_TITLE	2
#define ANNOT_AT_MIN	1
#define ANNOT_AT_MAX	2
#define ANNOT_AT_BOTH	3

struct SUBPLOT_CTRL {
	struct In {	/* enter | exit */
		bool active;
		unsigned int mode;	/* 0 = exit, 1 = enter */
	} In;
	struct A {	/* -A[<letter>|<number>][+c][+j<just>] */
		bool active;
		char format[GMT_LEN16];
		unsigned int mode;
		unsigned int nstart;
		unsigned int way;
		char cstart;
		int justify;
	} A;
	struct F {	/* -F<width>[u]/<height>[u] */
		bool active;
		double dim[2];
	} F;
	struct L {	/* -L<layout> */
		bool active;
		bool set_cpt, set_fill;
		bool has_label[2];	/* True if x and y labels */
		unsigned int mode;	/* 0 for -L, 1 for -LC, 2 for -LR (3 for both) */
		char *axes;		/* WESNwesn for -L */
		char *cpt;		/* WESNwesn for -L */
		char *label[2];	/* The constant x and y labels */
		unsigned int ptitle;	/* 0 = no panel titles, 1 = column titles, 2 = all panel titles */
		unsigned selected[2];	/* 1 if only l|r or t|b, 0 for both */
		struct GMT_FILL fill;
	} L;
	struct M {	/* -M[type]<xmargin>[u]/<ymargin>[u] */
		bool active[2];
		double pmargin[2];
		double mmargin[2];
	} M;
	struct N {	/* -Nrows/Ncolumns*/
		bool active;
		unsigned int dim[2];
		unsigned int n_panels;
	} N;
	struct T {	/* -T<title> */
		bool active;
		char *title;
	} T;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct SUBPLOT_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct SUBPLOT_CTRL);
	C->A.justify = PSL_TL;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct SUBPLOT_CTRL *C) {	/* Deallocate control structure */
	gmt_M_unused (GMT);
	if (!C) return;
	gmt_M_str_free (C->T.title);
	gmt_M_str_free (C->L.label[GMT_X]);
	gmt_M_str_free (C->L.label[GMT_Y]);
	gmt_M_str_free (C->L.axes);
	gmt_M_str_free (C->L.cpt);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: subplot -N<nrows>/<ncols> [-A<labels>] [-F<WxH>] [-L<layout>] [-M[m|p]<margins>] [-T<title>] [%s]\n\n", GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-N<nrows>/<ncols> is the number of rows and columns of panels for this figure.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Specify labeling of each panel.  Append either a number or letter [a].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This sets the label of the top-left panel and others follow incrementally.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Surround number or letter by parentheses on any side if these should be typeset.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Panels are numbered across rows.  Append +c to number down columns instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +j<justify> to specify where the label should be plotted in the panel [TL].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify dimension of area that the multi-panel figure may occupy [entire page].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set panel layout. May be set once (-L) or separately for rows (-LR) and columns (-LC):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -L:  Append WESNwesn to indicate which panel frames should be drawn and annotated.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +l to make space for axes labels; applies equally to all panels [no labels].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       Append x to only use x-labels or y for only y-labels [both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +p to make space for all panel titles; use +pc for top row titles only [no panel titles].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -LC: Each panel Column share the x-range. Only first (above) and last (below) rows will be annotated.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append t or b to select only one of those two rows instead [both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +l if annotated x-axes should have a label [none]; optionally append the label.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +s to make space for all panel titles; use +pc for top row titles only [no panel titles].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -LR: Each panel Row share the y-range. Only first (left) and last (right) columns will be annotated.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append l or r to select only one of those two columns [both].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append +l if annotated y-axes will have a label [none]; optionally append the label\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +g to set constant background fill for all panels [no fill].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c<cpt> to determine background using panel number and <cpt> lookup [no fill].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Specify the two types of margins:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Mp<xmargin>/<ymargin> sets the whitespace around each panel plot [0.5c/0.5c].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Mm<xmargin>/<ymargin> sets the media whitespace around each figure [1c/1c].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify a main title to be centered above all the panels [none]\n");
	GMT_Option (API, "V");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct SUBPLOT_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, k;
	char *c = NULL, add[2] = {0, 0}, string[GMT_LEN128] = {""};
	struct GMT_OPTION *opt = NULL;

	opt = options;	/* The first argument is the subplot command */
	if (opt->option != GMT_OPT_INFILE) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: No subplot command given\n");
		return GMT_PARSE_ERROR;
	}
	if (!strncmp (opt->arg, "enter", 5U))
		Ctrl->In.mode = ENTER;
	else if (!strncmp (opt->arg, "exit", 4U))
		Ctrl->In.mode = EXIT;
	else {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Not a subplot command: %s\n", opt->arg);
		return GMT_PARSE_ERROR;
	}
	opt = opt->next;
	if (Ctrl->In.mode == EXIT && opt && !(opt->option == 'V' && opt->next == NULL)) {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: subplot exit: Unrecognized option\n", opt->arg);
		return GMT_PARSE_ERROR;
	}
	
	/* Here we are either doing ENTER or just EXIT with -V */
	
	while (opt) {	/* Process all the options given */

		switch (opt->option) {

			case 'N':	/* The number of rows and columns */
				Ctrl->N.active = true;
				if (sscanf (opt->arg, "%dx%d", &Ctrl->N.dim[GMT_Y], &Ctrl->N.dim[GMT_X]) == 1)
					Ctrl->N.dim[GMT_X] = Ctrl->N.dim[GMT_Y];
				Ctrl->N.n_panels = Ctrl->N.dim[GMT_X] * Ctrl->N.dim[GMT_Y];
				break;
			case 'A':
				Ctrl->A.active = true;
				if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
				for (k = 0; k < strlen (opt->arg); k++) {
					if (isdigit (opt->arg[k])) {	/* Number labeling */
						Ctrl->A.nstart = atoi (&opt->arg[k]);
						Ctrl->A.mode = LABEL_IS_NUMBER;
						strcat (Ctrl->A.format, "%d");
					}
					else if (isalpha (opt->arg[k])) {	/* Letter labeling */
						Ctrl->A.cstart = opt->arg[k];
						Ctrl->A.mode = LABEL_IS_LETTER;
						strcat (Ctrl->A.format, "%c");
					}
					else {	/* Part of format string */
						add[0] = opt->arg[k];
						strcat (Ctrl->A.format, add);
					}
				}
				if (c) {
					c[0] = '+';	/* Restore modifiers */
					/* modifiers are [+j<justify>][+c] */
					if (gmt_get_modifier (opt->arg, 'j', string))
						Ctrl->A.justify = gmt_just_decode (GMT, string, PSL_NO_DEF);
					if (gmt_get_modifier (opt->arg, 'c', string))
						Ctrl->A.way = GMT_IS_COL_FORMAT;
				}
				break;
			case 'F':
				Ctrl->F.active = true;
				if ((k = GMT_Get_Values (GMT->parent, opt->arg, Ctrl->F.dim, 2)) < 1) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Option -F requies width and height of plot area.\n");
				}
				break;
			case 'L':	/* Layout */
				Ctrl->L.active = true;
				switch (opt->arg[0]) {
					case 'C':	/* Column setting */
						Ctrl->L.mode |= COL_FIXED_X;
						if (opt->arg[1] == 'b') Ctrl->L.selected[GMT_X] = ANNOT_AT_MIN;
						else if (opt->arg[1] == 't') Ctrl->L.selected[GMT_X] = ANNOT_AT_MAX;
						else Ctrl->L.selected[GMT_X] = ANNOT_AT_BOTH;
						if (gmt_get_modifier (opt->arg, 'l', string)) {	/* Want space for x-labels */
							Ctrl->L.has_label[GMT_X] = true;
							if (string[0]) Ctrl->L.label[GMT_X] = strdup (string);
						}
						break;
					case 'R':	/* Row setting */
						Ctrl->L.mode |= ROW_FIXED_Y;
						if (opt->arg[1] == 'l') Ctrl->L.selected[GMT_Y] = ANNOT_AT_MIN;
						else if (opt->arg[1] == 'r') Ctrl->L.selected[GMT_Y] = ANNOT_AT_MAX;
						else Ctrl->L.selected[GMT_Y] = ANNOT_AT_BOTH;
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
						if (strchr (Ctrl->L.axes, 'W')) Ctrl->L.selected[GMT_Y] |= ANNOT_AT_MIN;
						if (strchr (Ctrl->L.axes, 'E')) Ctrl->L.selected[GMT_Y] |= ANNOT_AT_MAX;
						if (strchr (Ctrl->L.axes, 'S')) Ctrl->L.selected[GMT_X] |= ANNOT_AT_MIN;
						if (strchr (Ctrl->L.axes, 'N')) Ctrl->L.selected[GMT_X] |= ANNOT_AT_MAX;
						break;
				}
				/* Common modifiers */
				if (gmt_get_modifier (opt->arg, 'p', string))	/* Want space for panel titles */
					Ctrl->L.ptitle = (string[0] == 'c') ? PANEL_COL_TITLE : PANEL_TITLE;
				if (gmt_get_modifier (opt->arg, 'g', string) && Ctrl->L.set_fill == false) {	/* Want color fill of panels */
					if (gmt_getfill (GMT, string, &Ctrl->L.fill)) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Bad +g<fill> modifier %c\n", &opt->arg[1]);
						n_errors++;
					}
					Ctrl->L.set_fill = true;
				}
				if (gmt_get_modifier (opt->arg, 'c', string) && Ctrl->L.set_cpt == false) {	/* Want color fill of panels via cpt */
					Ctrl->L.cpt = strdup (string);
					Ctrl->L.set_cpt = true;
				}
				break;
			case 'M':	/* Panel and media margins */
				switch (opt->arg[0]) {
					case 'm':	/* Media margin */
						Ctrl->M.active[MEDIA] = true;
						if (opt->arg[1] && (k = GMT_Get_Values (GMT->parent, &opt->arg[1], Ctrl->M.mmargin, 2)) < 1)
							n_errors++;
						else if (!opt->arg[1]) {
							Ctrl->M.mmargin[GMT_X] = Ctrl->M.mmargin[GMT_Y] = 1.0 * CM_TO_INCH;
						}
						break;
					case 'p':	/* Panel margin */
						Ctrl->M.active[PANEL] = true;
						if (opt->arg[1] && (k = GMT_Get_Values (GMT->parent, &opt->arg[1], Ctrl->M.pmargin, 2)) < 1)
							n_errors++;
						else if (!opt->arg[1]) {
							Ctrl->M.pmargin[GMT_X] = Ctrl->M.pmargin[GMT_Y] = 0.5 * CM_TO_INCH;
						}
						break;
				}
				break;

			case 'T':
				Ctrl->T.active = true;
				Ctrl->T.title = strdup (opt->arg);
				break;
				
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
		opt = opt->next;
	}
	
	n_errors += gmt_M_check_condition (GMT, !Ctrl->N.active, "Syntax error -N: Number of RowsxCols is required!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.n_panels == 0, "Syntax error -M: Number of RowsxCols is required!\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->L.active, "Syntax error -L: Must specify panel layout!\n");
	
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_subplot (void *V_API, int mode, void *args) {
	int error = 0;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct SUBPLOT_CTRL *Ctrl = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));		/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the subplot main code ----------------------------*/

	fprintf (stderr, "subplot is not operational yet.\n");
	if (Ctrl->In.mode == ENTER) {	/* Determine and save subplot panel attributes */
		unsigned int row, col, panel, nx, ny, factor, last_row, last_col;
		double x, y, area_dim[2], plot_dim[2], width, height, annot_width, label_width, ptitle_height, title_height, title_only;
		double *px = NULL, *py = NULL;
		annot_width = (GMT_LET_HEIGHT * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH) + MAX(0,GMT->current.setting.map_tick_length[GMT_ANNOT_UPPER]) + MAX (0.0, GMT->current.setting.map_annot_offset[GMT_PRIMARY]);	/* Allow for space between axis and annotations */
		label_width = (GMT_LET_HEIGHT * GMT->current.setting.font_label.size / PSL_POINTS_PER_INCH) + MAX (0.0, GMT->current.setting.map_label_offset);
		ptitle_height = (GMT_LET_HEIGHT * GMT->current.setting.font_title.size / PSL_POINTS_PER_INCH) + GMT->current.setting.map_title_offset;
		title_only = 1.5 * (GMT_LET_HEIGHT * GMT->current.setting.font_title.size / PSL_POINTS_PER_INCH);
		title_height = title_only + 1.5 * GMT->current.setting.map_title_offset;
		/* Get plot/media area dimensions */
		width  = area_dim[GMT_X] = (Ctrl->F.active) ? Ctrl->F.dim[GMT_X] : GMT->current.setting.ps_page_size[GMT_X] / PSL_POINTS_PER_INCH;
		height = area_dim[GMT_Y] = (Ctrl->F.active) ? Ctrl->F.dim[GMT_Y] : GMT->current.setting.ps_page_size[GMT_Y] / PSL_POINTS_PER_INCH;
		fprintf(stderr, "height = %g\n", height);
		fprintf(stderr, "width = %g\n", width);
		fprintf(stderr, "annot_width = %g\n", annot_width);
		fprintf(stderr, "label_width = %g\n", label_width);
		fprintf(stderr, "ptitle_height = %g\n", ptitle_height);
		fprintf(stderr, "title_height = %g\n", title_height);
		fprintf(stderr, "media margin = %g/%g\n", Ctrl->M.mmargin[GMT_X], Ctrl->M.mmargin[GMT_Y]);
		fprintf(stderr, "panel margin = %g/%g\n", Ctrl->M.pmargin[GMT_X], Ctrl->M.pmargin[GMT_Y]);
		/* Shrink these if media margins were requested */
		fprintf(stderr, "Start: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		if (Ctrl->M.active[MEDIA]) {	/* Remove space used by media margins */
			area_dim[GMT_X] -= 2.0 * Ctrl->M.mmargin[GMT_X];
			area_dim[GMT_Y] -= 2.0 * Ctrl->M.mmargin[GMT_Y];
		}
		fprintf(stderr, "After media margins: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		if (Ctrl->M.active[PANEL]) {	/* Remove space used by panel margins */
			area_dim[GMT_X] -= 2.0 * Ctrl->N.dim[GMT_X] * Ctrl->M.pmargin[GMT_X];
			area_dim[GMT_Y] -= 2.0 * Ctrl->N.dim[GMT_Y] * Ctrl->M.pmargin[GMT_Y];
		}
		fprintf(stderr, "After panel margins: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		/* Limit annotations/labels to 1 or 2 axes per row or per panel */
		nx = (Ctrl->L.selected[GMT_Y] == ANNOT_AT_BOTH) ? 2 : 1;
		if ((Ctrl->L.mode & ROW_FIXED_Y) == 0) 
			nx *= Ctrl->N.dim[GMT_X];	/* Each column needs separate y-axis [and labels] */
		area_dim[GMT_X] -= nx * annot_width;
		fprintf(stderr, "After row annots: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		if (Ctrl->L.has_label[GMT_Y])
			area_dim[GMT_X] -= nx * label_width;
		fprintf(stderr, "After row labels: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		plot_dim[GMT_X] = area_dim[GMT_X] / Ctrl->N.dim[GMT_X];
		/* Limit annotations/labels to 1 or 2 axes per column or per panel */
		if (Ctrl->T.active)
			area_dim[GMT_Y] -= title_height;
		fprintf(stderr, "After main title: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		ny = (Ctrl->L.selected[GMT_X] == ANNOT_AT_BOTH) ? 2 : 1;
		factor = (Ctrl->L.mode & COL_FIXED_X) ? 1 : Ctrl->N.dim[GMT_Y];		/* Each row may need separate x-axis [and labels] */
		ny *= factor;
		fprintf(stderr, "nx = %d, ny = %d, factor = %d\n", nx, ny, factor);
		area_dim[GMT_Y] -= ny * annot_width;
		fprintf(stderr, "After col annot: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		if (Ctrl->L.has_label[GMT_X])
			area_dim[GMT_Y] -= ny * label_width;
		fprintf(stderr, "After col labels: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		if (Ctrl->L.ptitle == PANEL_COL_TITLE)
			area_dim[GMT_Y] -= ptitle_height;
		else if (Ctrl->L.ptitle == PANEL_TITLE)
			area_dim[GMT_Y] -= factor * ptitle_height;
		fprintf(stderr, "After panel titles: area_dim = {%g, %g}\n", area_dim[GMT_X], area_dim[GMT_Y]);
		plot_dim[GMT_Y] = area_dim[GMT_Y] / Ctrl->N.dim[GMT_Y];
		fprintf(stderr, "Main title BC: %g  %g\n", 0.5 * width, height - title_only - Ctrl->M.mmargin[GMT_Y]);
		/* Allocate panel info array */
		px = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_X], double);
		py = gmt_M_memory (GMT, NULL, Ctrl->N.dim[GMT_Y], double);
		/* Need to determine how many y-labels and annotations for each row */
		last_row = Ctrl->N.dim[GMT_Y] - 1;
		last_col = Ctrl->N.dim[GMT_X] - 1;
		y = height;	/* Start off at top edge of plot area */
		if (Ctrl->M.active[MEDIA])
			y -= Ctrl->M.mmargin[GMT_Y];	/* Skip space used by paper margins */
		if (Ctrl->T.active)
			y -= title_height;	/* Skip space for main figure title */
		for (row = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each row of panels */
			y -= Ctrl->M.pmargin[GMT_Y];
			if ((row == 0 && (Ctrl->L.ptitle == PANEL_COL_TITLE)) || (Ctrl->L.ptitle == PANEL_TITLE))
				y -= ptitle_height;	/* Make space for panel title */
			if ((row == 0 || ((Ctrl->L.mode & COL_FIXED_X) == 0)) && (Ctrl->L.selected[GMT_X] & ANNOT_AT_MAX)) {	/* Need annotation at N */
				y -= annot_width;
				if (Ctrl->L.has_label[GMT_X]) {
					y -= label_width;	/* Also has label at N */
				}
			}
			y -= plot_dim[GMT_Y];	/* Now at correct y for this panel */
			py[row] = y;
			if ((row == last_row || ((Ctrl->L.mode & COL_FIXED_X) == 0)) && (Ctrl->L.selected[GMT_X] & ANNOT_AT_MIN)) {	/* Need annotation at S */
				y -= annot_width;
				if (Ctrl->L.has_label[GMT_X])
					y -= label_width;	/* Also has label at S */
			}
			y -= Ctrl->M.pmargin[GMT_Y];
		}
		x = 0.0;	/* Start off at left edge of plot area */
		if (Ctrl->M.active[MEDIA])
			x += Ctrl->M.mmargin[GMT_X];	/* Skip space used by paper margins */
		for (col = 0; col < Ctrl->N.dim[GMT_X]; col++) {	/* For each col of panels */
			x += Ctrl->M.pmargin[GMT_X];
			if ((col == 0 || ((Ctrl->L.mode & ROW_FIXED_Y) == 0)) && (Ctrl->L.selected[GMT_X] & ANNOT_AT_MIN)) {	/* Need annotation at W */
				x += annot_width;
				if (Ctrl->L.has_label[GMT_Y])
					x += label_width;	/* Also has label at W */
			}
			px[col] = x;	/* Now at correct x for this panel */
			x += plot_dim[GMT_X];
			if ((col == last_col || ((Ctrl->L.mode & ROW_FIXED_Y) == 0)) && (Ctrl->L.selected[GMT_Y] & ANNOT_AT_MAX)) {	/* Need annotation at E */
				x += annot_width;
				if (Ctrl->L.has_label[GMT_Y])
					x += label_width;	/* Also has label at E */
			}
			x += Ctrl->M.pmargin[GMT_X];
		}
		for (row = panel = 0; row < Ctrl->N.dim[GMT_Y]; row++) {	/* For each row of panels */
			for (col = 0; col < Ctrl->N.dim[GMT_X]; col++, panel++) {	/* For each col of panels */
				printf ("%d\t%d\t%d\t%7.3f\t%7.3f\t%7.3f\t%7.3f\tWESN\n", panel, row, col, px[col], py[row], plot_dim[GMT_X], plot_dim[GMT_Y]);
			}
		}
		
		gmt_M_free (GMT, px);
		gmt_M_free (GMT, py);
	}
	else {	/* EXIT */
		fprintf (stderr, "Done\n");
	}
		
	Return (error);
}
