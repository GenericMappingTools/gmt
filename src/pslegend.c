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
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU Lesser General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 * Brief synopsis: pslegend will make map legends from input that specifies what will go
 * into the legend, such as headers, symbols with explanatory text,
 * paragraph text, and empty space and horizontal/vertical lines.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"pslegend"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot legends on maps"
#define THIS_MODULE_KEYS	"<TI,ACi,-Xo"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "->BJKOPRUVXYcpt"

struct PSLEGEND_CTRL {
	struct PSLEGND_C {	/* -C<dx>[/<dy>] */
		bool active;
		double off[2];
	} C;
	struct PSLEGND_D {	/* -D[g|j|n|x]<refpoint>+w<width>[/<height>][+j<justify>][+l<spacing>][+o<dx>[/<dy>]] */
		bool active;
		struct GMT_REFPOINT *refpoint;
		double dim[2], off[2];
		double spacing;
		int justify;
	} D;
	struct PSLEGND_F {	/* -F[+r[<radius>]][+g<fill>][+p[<pen>]][+i[<off>/][<pen>]][+s[<dx>/<dy>/][<shade>]][+d] */
		bool active;
		bool debug;			/* If true we draw guide lines */
		struct GMT_MAP_PANEL *panel;
	} F;
};

void *New_pslegend_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSLEGEND_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSLEGEND_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->C.off[GMT_X] = C->C.off[GMT_Y] = GMT->session.u2u[GMT_PT][GMT_INCH] * GMT_FRAME_CLEARANCE;	/* 4 pt */
	C->D.spacing = 1.1;
	return (C);
}

void Free_pslegend_Ctrl (struct GMT_CTRL *GMT, struct PSLEGEND_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free_refpoint (GMT, &C->D.refpoint);
	if (C->F.panel) GMT_free (GMT, C->F.panel);
	GMT_free (GMT, C);
}

int GMT_pslegend_usage (struct GMTAPI_CTRL *API, int level)
{
	/* This displays the pslegend synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: pslegend [<specfile>] -D%s+w<width>[/<height>][+l<spacing>]%s [%s]\n", GMT_XYANCHOR, GMT_OFFSET, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-C<dx>[/<dy>]] [%s]\n", GMT_PANEL);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-K] [-O] [-P] [%s]\n", GMT_J_OPT, GMT_Rgeo_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s]\n\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, GMT_c_OPT, GMT_p_OPT, GMT_t_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\tReads legend layout specification from <specfile> [or stdin].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t(See manual page for more information and <specfile> format).\n");

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_refpoint_syntax (API->GMT, 'D', "Specify position and size of the legend rectangle", GMT_ANCHOR_LEGEND, 1);
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify legend width with +w<width>; <height> is optional [estimated from <specfile>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The remaining arguments are optional:\n");
	GMT_refpoint_syntax (API->GMT, 'D', "TC", GMT_ANCHOR_LEGEND, 2);
	GMT_Message (API, GMT_TIME_NONE, "\t   +l sets the linespacing factor in units of the current annotation font size [1.1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<specfile> is one or more ASCII specification files with legend commands.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read.\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Set the clearance between legend frame and internal items [%gp].\n", GMT_FRAME_CLEARANCE);
	GMT_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the legend", 2);
	GMT_Option (API, "J-,K");
	GMT_Option (API, "O,P,R");
	GMT_Option (API, "U,V,X,p,t,.");

	return (EXIT_FAILURE);
}

int GMT_pslegend_parse (struct GMT_CTRL *GMT, struct PSLEGEND_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pslegend and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	int n;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""};
	char txt_d[GMT_LEN256] = {""}, txt_e[GMT_LEN256] = {""}, string[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_TEXTSET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Sets the clearance between frame and internal items */
				Ctrl->C.active = true;
				if ((n = GMT_get_pair (GMT, opt->arg, GMT_PAIR_DIM_DUP, Ctrl->C.off)) < 0) n_errors++;
				break;
			case 'D':	/* Sets position and size of legend */
				Ctrl->D.active = true;
				if ((Ctrl->D.refpoint = GMT_get_refpoint (GMT, opt->arg)) == NULL) {
					n_errors++;	/* Failed basic parsing */
					break;
				}
				if (strstr (opt->arg, "+w")) {	/* New syntax: 	Args are +w<width>[/<height>][+j<justify>][+o<dx>[/<dy>]] */
					if (GMT_get_modifier (Ctrl->D.refpoint->args, 'j', string))
						Ctrl->D.justify = GMT_just_decode (GMT, string, PSL_NO_DEF);
					else	/* With -Dj, set default to reference justify point, else LB */
						Ctrl->D.justify = (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST) ? Ctrl->D.refpoint->justify : PSL_BL;
					if (GMT_get_modifier (Ctrl->D.refpoint->args, 'o', string)) {
						if ((n = GMT_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->D.off)) < 0) n_errors++;
					}
					if (GMT_get_modifier (Ctrl->D.refpoint->args, 'w', string)) {
						if ((n = GMT_get_pair (GMT, string, GMT_PAIR_DIM_NODUP, Ctrl->D.dim)) < 0) n_errors++;
					}
					if (GMT_get_modifier (Ctrl->D.refpoint->args, 'l', string)) {
						Ctrl->D.spacing = atof (string);
					}
				}
				else {	/* Backwards handling of old syntax. Args are args are <width>[/<height>][/<justify>][/<dx>/<dy>] */
					Ctrl->D.justify = PSL_TC;	/* Backwards compatible default justification */
					n = sscanf (Ctrl->D.refpoint->args, "%[^/]/%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d, txt_e);
					n_errors += GMT_check_condition (GMT, n < 2, "Error: Syntax is -D[x]<x0>/<y0>/<width>[/<height>][/<justify>][/<dx>/<dy>]\n");
					Ctrl->D.dim[GMT_X] = GMT_to_inch (GMT, txt_a);
					switch (n) {
						case 1: /* Only gave reference point and width; change default justify if -Dj */
							if (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST)	/* For -Dj with no 2nd justification, use same code as reference coordinate */
								Ctrl->D.justify = Ctrl->D.refpoint->justify;
							break;
						case 2:	/* Gave width and (height or justify) */
							if (strlen (txt_b) == 2 && strchr ("LMRBCT", txt_b[GMT_X]) && strchr ("LMRBCT", txt_b[GMT_Y]))	/* Gave a 2-char justification code */
								Ctrl->D.justify = GMT_just_decode (GMT, txt_b, PSL_NO_DEF);
							else /* Got height */
								Ctrl->D.dim[GMT_Y] = GMT_to_inch (GMT, txt_b);
							break;
						case 3:	/* Gave width and (height and justify) or (dx/dy) */
							if (strlen (txt_c) == 2 && strchr ("LMRBCT", txt_c[GMT_X]) && strchr ("LMRBCT", txt_c[GMT_Y])) {	/* Gave a 2-char justification code */
								Ctrl->D.dim[GMT_Y] = GMT_to_inch (GMT, txt_b);
								Ctrl->D.justify = GMT_just_decode (GMT, txt_c, PSL_NO_DEF);
							}
							else {	/* Just got offsets */
								Ctrl->D.off[GMT_X] = GMT_to_inch (GMT, txt_b);
								Ctrl->D.off[GMT_Y] = GMT_to_inch (GMT, txt_c);
							}
							break;
						case 4:	/* Gave width and (height or justify) and dx/dy */
							if (strlen (txt_b) == 2 && strchr ("LMRBCT", txt_b[GMT_X]) && strchr ("LMRBCT", txt_b[GMT_Y]))	/* Gave a 2-char justification code */
								Ctrl->D.justify = GMT_just_decode (GMT, txt_b, PSL_NO_DEF);
							else
								Ctrl->D.dim[GMT_Y] = GMT_to_inch (GMT, txt_b);
							Ctrl->D.off[GMT_X] = GMT_to_inch (GMT, txt_c);
							Ctrl->D.off[GMT_Y] = GMT_to_inch (GMT, txt_d);
							break;
						case 5:	/* Got them all */
							Ctrl->D.dim[GMT_Y] = GMT_to_inch (GMT, txt_b);
							Ctrl->D.justify = GMT_just_decode (GMT, txt_c, PSL_NO_DEF);
							Ctrl->D.off[GMT_X] = GMT_to_inch (GMT, txt_d);
							Ctrl->D.off[GMT_Y] = GMT_to_inch (GMT, txt_e);
							break;
					}
				}
				break;
			case 'F':
				Ctrl->F.active = true;
				if (GMT_getpanel (GMT, opt->option, opt->arg, &(Ctrl->F.panel))) {
					GMT_mappanel_syntax (GMT, 'F', "Specify a rectangular panel behind the legend", 2);
					n_errors++;
				}
				Ctrl->F.debug = Ctrl->F.panel->debug;	/* Hidden +d processing; this may go away */
				if (GMT_compat_check (GMT, 4) && !opt->arg[0]) Ctrl->F.panel->mode |= GMT_PANEL_OUTLINE;	/* Draw frame if just -F is given if in compatibility mode */
				if (!Ctrl->F.panel->clearance) GMT_memset (Ctrl->F.panel->padding, 4, double);	/* No clearance is default since handled via -C */
				break;
			case 'G':	/* Inside legend box fill [OBSOLETE] */
				if (GMT_compat_check (GMT, 4)) {
					char tmparg[GMT_LEN32] = {""};
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Option -G is deprecated; -F...+g%s was set instead, use this in the future.\n", opt->arg);
					Ctrl->F.active = true;
					sprintf (tmparg, "+g%s", opt->arg);
					if (GMT_getpanel (GMT, opt->option, tmparg, &(Ctrl->F.panel))) {
						GMT_mappanel_syntax (GMT, 'F', "Specify a rectangular panel behind the legend", 2);
						n_errors++;
					}
					Ctrl->F.panel->mode |= GMT_PANEL_FILL;
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'L':			/* Sets linespacing in units of fontsize [1.1] */
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: Option -L is deprecated; -D...+l%s was set instead, use this in the future.\n", opt->arg);
				Ctrl->D.spacing = atof (opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	n_errors += GMT_check_condition (GMT, Ctrl->C.off[GMT_X] < 0.0 || Ctrl->C.off[GMT_Y] < 0.0, "Syntax error -C option: clearances cannot be negative!\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->D.active, "Syntax error: The -D option is required!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.dim[GMT_X] <= 0.0, "Syntax error -D option: legend box width must be positive\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.dim[GMT_Y] < 0.0, "Syntax error -D option: legend box height cannot be negative!\n");
	if (Ctrl->D.refpoint->mode != GMT_REFPOINT_PLOT) {	/* Anything other than -Dx need -R -J; other cases don't */
		n_errors += GMT_check_condition (GMT, !GMT->common.R.active, "Syntax error: Must specify -R option\n");
		n_errors += GMT_check_condition (GMT, !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_pslegend_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

/* Used to draw the current y-line for debug purposes only. */
void drawbase (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double x1, double y0)
{
	struct GMT_PEN faint_pen;
	GMT_init_pen (GMT, &faint_pen, 0.0);
	GMT_setpen (GMT, &faint_pen);
	PSL_plotsegment (PSL, x0, y0, x1, y0);
}

/* Used to fill the cells in the the current y-line. */
void fillcell (struct GMT_CTRL *GMT, double x0, double y0, double y1, double xoff[], double *d_gap, unsigned int n_cols, char *fill[])
{
	unsigned int col;
	double dim[2];
	struct GMT_FILL F;
	dim[1] = y1 - y0 + *d_gap;
	y0 = 0.5 * (y0 + y1 + *d_gap);	/* Recycle y0 to mean mid level */
	for (col = 0; col < n_cols; col++) {
		if (!fill[col]) continue;	/* No fill for this cell */
		if (GMT_getfill (GMT, fill[col], &F)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to interpret %s as a valid fill, skipped\n", fill[col]);
			continue;
		}
		GMT_setfill (GMT, &F, false);
		dim[0] = xoff[col+1] - xoff[col];
		PSL_plotsymbol (GMT->PSL, x0 + 0.5 * (xoff[col+1] + xoff[col]), y0, dim, GMT_SYMBOL_RECT);
	}
	*d_gap = 0.0;	/* Reset any "gap after D operator" once we use it */
}

struct GMT_TEXTSET *get_textset_pointer (struct GMTAPI_CTRL *API, struct GMT_TEXTSET *Din, unsigned int geometry)
{
	uint64_t dim[3] = {1, 1, GMT_SMALL_CHUNK};
	struct GMT_TEXTSET *D = NULL;
	if (Din) return Din;	/* Already done this */
	if ((D = GMT_Create_Data (API, GMT_IS_TEXTSET, geometry, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a text set for pslegend\n");
		return (NULL);
	}
	return (D);
}

struct GMT_DATASET *get_dataset_pointer (struct GMTAPI_CTRL *API, struct GMT_DATASET *Din, unsigned int geometry)
{
	uint64_t dim[4] = {1, GMT_SMALL_CHUNK, 2, 2};	/* We will a 2-row data set for up to 64 lines; allocate just once */
	struct GMT_DATASET *D = NULL;
	if (Din) return Din;	/* Already done this */
	if (D == NULL && (D = GMT_Create_Data (API, GMT_IS_DATASET, geometry, 0, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a data set for pslegend\n");
		return (NULL);
	}
	return (D);
}

/* Define the fraction of the height of the font to the font size */
#define FONT_HEIGHT_PRIMARY (GMT->session.font[GMT->current.setting.font_annot[0].id].height)
#define FONT_HEIGHT(font_id) (GMT->session.font[font_id].height)
#define FONT_HEIGHT_LABEL (GMT->session.font[GMT->current.setting.font_label.id].height)

#define SYM 0
#define TXT 1
#define PAR 2
#define N_TXT 3
#define FRONT 0
#define QLINE 1
#define N_DAT 2

#define PSLEGEND_MAX_COLS	100

int debug_print = 0;	/* Change to 1 to get debug print for some buffer strings */

int GMT_pslegend (void *V_API, int mode, void *args)
{	/* High-level function that implements the pslegend task */
	unsigned int tbl, pos;
	int i, justify = 0, n = 0, n_columns = 1, n_col, col, error = 0, column_number = 0, id, n_scan;
	int status = 0, object_ID;
	bool flush_paragraph = false, v_line_draw_now = false, gave_label, gave_mapscale_options, did_old = false, drawn = false;
	uint64_t seg, row, n_fronts = 0, n_quoted_lines = 0;
	size_t n_char = 0;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, txt_d[GMT_LEN256] = {""};
	char txt_e[GMT_LEN256] = {""}, txt_f[GMT_LEN256] = {""}, key[GMT_LEN256] = {""}, sub[GMT_LEN256] = {""}, just;
	char tmp[GMT_LEN256] = {""}, symbol[GMT_LEN256] = {""}, text[GMT_BUFSIZ] = {""}, image[GMT_BUFSIZ] = {""}, xx[GMT_LEN256] = {""};
	char yy[GMT_LEN256] = {""}, size[GMT_LEN256] = {""}, angle[GMT_LEN256] = {""}, mapscale[GMT_LEN256] = {""};
	char font[GMT_LEN256] = {""}, lspace[GMT_LEN256] = {""}, tw[GMT_LEN256] = {""}, jj[GMT_LEN256] = {""};
	char bar_cpt[GMT_LEN256] = {""}, bar_gap[GMT_LEN256] = {""}, bar_height[GMT_LEN256] = {""}, bar_modifiers[GMT_LEN256] = {""};
	char module_options[GMT_LEN256] = {""}, r_options[GMT_LEN256] = {""};
	char sarg[GMT_LEN256] = {""}, txtcolor[GMT_LEN256] = {""}, buffer[GMT_BUFSIZ] = {""}, A[GMT_LEN32] = {""};
	char path[GMT_BUFSIZ] = {""}, B[GMT_LEN32] = {""}, C[GMT_LEN32] = {""}, p[GMT_LEN256] = {""};
	char *line = NULL, string[GMT_STR16] = {""}, save_EOF = 0, *c = NULL, *fill[PSLEGEND_MAX_COLS];

	unsigned char *dummy = NULL;

	double x_orig, y_orig, x_off, x, y, r, col_left_x, row_base_y, dx, dy, d_line_half_width, d_line_hor_offset, off_ss, off_tt;
	double v_line_ver_offset = 0.0, height, az1, az2, m_az, row_height, scl;
	double half_line_spacing, quarter_line_spacing, one_line_spacing, v_line_y_start = 0.0, d_off;
	double sum_width, h, gap, d_line_after_gap, d_line_last_y0 = 0.0, col_width[PSLEGEND_MAX_COLS], x_off_col[PSLEGEND_MAX_COLS];

	struct imageinfo header;
	struct PSLEGEND_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */
	struct GMT_OPTION *r_ptr = NULL, *j_ptr = NULL;
	struct GMT_FONT ifont;
	struct GMT_PEN current_pen;
	struct GMT_TEXTSET *In = NULL, *T[N_TXT];
	struct GMT_TEXTSEGMENT *Ts[N_TXT];
	struct GMT_DATASET *D[N_DAT];
	struct GMT_DATASEGMENT *Ds[N_DAT];
	struct GMT_PALETTE *P = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	GMT_memset (&header, 1, struct imageinfo);	/* initialize struct */
	GMT_memset (fill, PSLEGEND_MAX_COLS, char *);	/* initialize array */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_pslegend_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_pslegend_usage (API, GMT_USAGE));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_pslegend_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_pslegend_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pslegend_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the pslegend main code ----------------------------*/

	GMT_memset (T, N_TXT, struct GMT_TEXTSET *);		/* Set these arrays to NULL */
	GMT_memset (Ts, N_TXT, struct GMT_TEXTSEGMENT *);
	GMT_memset (D, N_DAT, struct GMT_DATASET *);		/* Set these arrays to NULL */
	GMT_memset (Ds, N_DAT, struct GMT_DATASEGMENT *);
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input text table data\n");
	if (GMT_compat_check (GMT, 4)) {
		/* Since pslegend v4 used '>' to indicate a paragraph record we avoid confusion with multiple segment-headers by *
		 * temporarily setting # as segment header flag since all headers are skipped anyway */
		save_EOF = GMT->current.setting.io_seg_marker[GMT_IN];
		GMT->current.setting.io_seg_marker[GMT_IN] = '#';
	}

	if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_NONE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Register data input */
		Return (API->error);
	}
	if ((In = GMT_Read_Data (API, GMT_IS_TEXTSET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
		Return (API->error);
	}

	/* First attempt to compute the legend height */

	one_line_spacing = Ctrl->D.spacing * GMT->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH;
	half_line_spacing    = 0.5  * one_line_spacing;
	quarter_line_spacing = 0.25 * one_line_spacing;

	height = 2.0 * Ctrl->C.off[GMT_Y];
	for (tbl = 0; tbl < In->n_tables; tbl++) {	/* We only expect one table but who knows what the user does */
		for (seg = 0; seg < In->table[tbl]->n_segments; seg++) {	/* We only expect one segment in each table but again... */
			for (row = 0; row < In->table[tbl]->segment[seg]->n_rows; row++) {	/* Finally processing the rows */
				line = In->table[tbl]->segment[seg]->record[row];
				if (line[0] == '#' || GMT_is_a_blank_line (line)) continue;	/* Skip all headers or blank lines  */

				/* Data record to process */

				if (line[0] != 'T' && flush_paragraph) {	/* Flush contents of pending paragraph [Call GMT_pstext] */
					flush_paragraph = false;
					column_number = 0;
				}

				switch (line[0]) {
					case 'B':	/* Color scale Bar [Use GMT_psscale] */
						/* B cptname offset height[+modifiers] [ Optional psscale args -B -I -L -M -N -S -Z -p ] */
						sscanf (&line[2], "%*s %*s %s", bar_height);
						if ((c = strchr (bar_height, '+'))) c[0] = 0;	/* Chop off any modifiers so we can compute the height */
						height += GMT_to_inch (GMT, bar_height) + GMT->current.setting.map_tick_length[0] + GMT->current.setting.map_annot_offset[0] + FONT_HEIGHT_PRIMARY * GMT->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH;
						column_number = 0;
						break;

					case 'A':	/* Color change, no height implication */
					case 'C':
					case 'F':
						break;

					case 'D':	/* Delimiter record: D offset pen [-|=|+] */
						txt_c[0] = '\0';
						sscanf (&line[2], "%s %s %s", txt_a, txt_b, txt_c);
						if (!(txt_c[0] == '-' || txt_c[0] == '=')) height += quarter_line_spacing;
						if (!(txt_c[0] == '+' || txt_c[0] == '=')) height += quarter_line_spacing;
						column_number = 0;
						break;

					case 'G':	/* Gap record */
						sscanf (&line[2], "%s", txt_a);
						height += (txt_a[strlen(txt_a)-1] == 'l') ? atoi (txt_a) * one_line_spacing : GMT_to_inch (GMT, txt_a);
						column_number = 0;
						break;

					case 'H':	/* Header record */
						sscanf (&line[2], "%s %s %[^\n]", size, font, text);
						if (size[0] == '-') size[0] = 0;
						if (font[0] == '-') font[0] = 0;
						sprintf (tmp, "%s,%s,%s", size, font, txtcolor);	/* Put size, font and color together for parsing by GMT_getfont */
						ifont = GMT->current.setting.font_title;	/* Set default font */
						GMT_getfont (GMT, tmp, &ifont);
						height += Ctrl->D.spacing * ifont.size / PSL_POINTS_PER_INCH;
						column_number = 0;
						break;

					case 'I':	/* Image record [use GMT_psimage] */
						sscanf (&line[2], "%s %s %s", image, size, key);
						PSL_loadimage (PSL, image, &header, &dummy);
						height += GMT_to_inch (GMT, size) * (double)header.height / (double)header.width;
						PSL_free (dummy);
						column_number = 0;
						break;

					case 'L':	/* Label record */
						sscanf (&line[2], "%s %s %s %[^\n]", size, font, key, text);
						if (size[0] == '-') size[0] = 0;
						if (font[0] == '-') font[0] = 0;
						sprintf (tmp, "%s,%s,%s", size, font, txtcolor);		/* Put size, font and color together for parsing by GMT_getfont */
						ifont = GMT->current.setting.font_label;	/* Set default font */
						GMT_getfont (GMT, tmp, &ifont);
						if (column_number%n_columns == 0) {
							height += Ctrl->D.spacing * ifont.size / PSL_POINTS_PER_INCH;
							column_number = 0;
						}
						column_number++;
						break;

					case 'M':	/* Map scale record M lon0|- lat0 length[n|m|k][+opts] f|p  [-R -J] */
						n_scan = sscanf (&line[2], "%s %s %s %s %s %s", txt_a, txt_b, txt_c, txt_d, txt_e, txt_f);
						for (i = 0, gave_mapscale_options = false; txt_c[i] && !gave_mapscale_options; i++) if (txt_c[i] == '+') gave_mapscale_options = true;
						/* Default assumes label is added on top */
						just = 't';
						gave_label = true;
						d_off = FONT_HEIGHT_LABEL * GMT->current.setting.font_label.size / PSL_POINTS_PER_INCH + fabs(GMT->current.setting.map_label_offset);
						if ((txt_d[0] == 'f' || txt_d[0] == 'p') && GMT_get_modifier (txt_c, 'j', string))	/* Specified alternate justification old-style */
							just = string[0];
						else if (GMT_get_modifier (txt_c, 'a', string))	/* Specified alternate aligment */
							just = string[0];
						if (GMT_get_modifier (txt_c, 'u', string))	/* Specified alternate aligment */
							gave_label = false;	/* Not sure why I do this, will find out */
						if (gave_label && (just == 't' || just == 'b')) height += d_off;
						height += GMT->current.setting.map_scale_height + FONT_HEIGHT_PRIMARY * GMT->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH + GMT->current.setting.map_annot_offset[0];
						column_number = 0;
						break;

					case 'N':	/* n_columns or column width record */
						pos = n_columns = 0;
						while ((GMT_strtok (&line[2], " \t", &pos, p))) {
							n_columns++;
							if (n_columns == PSLEGEND_MAX_COLS) {
								GMT_Report (API, GMT_MSG_NORMAL, "Error: Exceeding maximum columns (%d) in N operator\n", PSLEGEND_MAX_COLS);
								Return (GMT_RUNTIME_ERROR);
							}
						}
						if (n_columns == 0) n_columns = 1;	/* Default to 1 if nothing is given */
						/* Check if user gave just the number of columns */
						if (n_columns == 1 && (pos = atoi (&line[2])) > 1) n_columns = pos;
						column_number = 0;
						break;
						
					case '>':	/* Paragraph text header */
						if (GMT_compat_check (GMT, 4)) /* Warn and fall through */
							GMT_Report (API, GMT_MSG_COMPAT, "Warning: paragraph text header flag > is deprecated; use P instead\n");
						else {
							GMT_Report (API, GMT_MSG_NORMAL, "Error: Unrecognized record (%s)\n", line);
							Return (GMT_RUNTIME_ERROR);
							break;
						}
					case 'P':	/* Paragraph text header */
						flush_paragraph = true;
						column_number = 0;
						break;

					case 'S':	/* Symbol record */
						if (column_number%n_columns == 0) {
							height += one_line_spacing;
							column_number = 0;
						}
						column_number++;
						break;

					case 'T':	/* paragraph text record */
						n_char += strlen (line) - 2;
						flush_paragraph = true;
						column_number = 0;
						break;

					case 'V':	/* Vertical line from here to next V */
						column_number = 0;
						break;

					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Error: Unrecognized record (%s)\n", line);
						Return (GMT_RUNTIME_ERROR);
					break;
				}
			}
		}
	}

	if (n_char) {	/* Typesetting paragraphs, make a guesstimate of number of typeset lines */
		int n_lines;
		double average_char_width = 0.44;	/* There is no such thing but this is just a 1st order estimate */
		double x_lines;
		/* Guess: Given legend width and approximate char width, do the simple expression */
		x_lines = n_char * (average_char_width * GMT->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH) / ((Ctrl->D.dim[GMT_X] - 2 * Ctrl->C.off[GMT_X]));
		n_lines = irint (ceil (x_lines));
		height += n_lines * Ctrl->D.spacing * GMT->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH;
		GMT_Report (API, GMT_MSG_DEBUG, "Estimating %d lines of typeset paragraph text [%.1f].\n", n_lines, x_lines);
	}

	scl = GMT_convert_units (GMT, "1", GMT_INCH, GMT->current.setting.proj_length_unit);
	if (Ctrl->D.dim[GMT_Y] == 0.0) {	/* Use the computed height */
		Ctrl->D.dim[GMT_Y] = height;
		GMT_Report (API, GMT_MSG_VERBOSE, "Legend height not given, use estimated height of %g %s.\n", scl*height,
			GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	}
	else
		GMT_Report (API, GMT_MSG_VERBOSE, "Legend height given as %g %s; estimated height is %g %s.\n",
		            scl*Ctrl->D.dim[GMT_Y], GMT->session.unit_name[GMT->current.setting.proj_length_unit],
		            scl*height, GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	
	if (!(GMT->common.R.active && GMT->common.J.active)) {	/* When no projection specified (i.e, -Dx is used), use fake linear projection -Jx1i */
		double wesn[4];
		GMT_memset (wesn, 4, double);
		GMT->common.R.active = true;
		GMT->common.J.active = false;
		GMT_parse_common_options (GMT, "J", 'J', "x1i");
		wesn[XHI] = Ctrl->D.dim[GMT_X];	wesn[YHI] = Ctrl->D.dim[GMT_Y];
		GMT_err_fail (GMT, GMT_map_setup (GMT, wesn), "");
	}
	else {
		r_ptr = GMT_Find_Option (API, 'R', options);
		j_ptr = GMT_Find_Option (API, 'J', options);
		if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);
	}
	PSL = GMT_plotinit (GMT, options);
	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

	/* Must reset any -X -Y to 0 so they are not used further in the GMT_modules we call below */
	GMT_memset (GMT->current.setting.map_origin, 2, double);

	GMT_set_refpoint (GMT, Ctrl->D.refpoint);	/* Finalize reference point plot coordinates, if needed */

	/* Allow for justification so that the reference point is the plot location of the lower left corner of box */

	Ctrl->D.refpoint->x -= 0.5 * ((Ctrl->D.justify-1)%4) * Ctrl->D.dim[GMT_X];
	Ctrl->D.refpoint->y -= 0.5 * (Ctrl->D.justify/4) * Ctrl->D.dim[GMT_Y];

	/* Also deal with any justified offsets if given */
	
	Ctrl->D.refpoint->x -= ((Ctrl->D.justify%4)-2) * Ctrl->D.off[GMT_X];
	Ctrl->D.refpoint->y -= ((Ctrl->D.justify/4)-1) * Ctrl->D.off[GMT_Y];
	
	/* Set new origin */
	
	PSL_setorigin (PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->y, 0.0, PSL_FWD);
	x_orig = Ctrl->D.refpoint->x;	y_orig = Ctrl->D.refpoint->y;
	Ctrl->D.refpoint->x = Ctrl->D.refpoint->y = 0.0;	/* For now */
	
	current_pen = GMT->current.setting.map_default_pen;

	if (Ctrl->F.active) {	/* First place legend frame fill */
		Ctrl->F.panel->width = Ctrl->D.dim[GMT_X];	Ctrl->F.panel->height = Ctrl->D.dim[GMT_Y];	
		GMT_draw_map_panel (GMT, Ctrl->D.refpoint->x + 0.5 * Ctrl->D.dim[GMT_X], Ctrl->D.refpoint->y + 0.5 * Ctrl->D.dim[GMT_Y], 1U, Ctrl->F.panel);
	}

	/* We use a standard x/y inch coordinate system here, unlike old pslegend. */

	col_left_x = Ctrl->D.refpoint->x + Ctrl->C.off[GMT_X];			/* Left justification edge of items inside legend box accounting for clearance */
	row_base_y = Ctrl->D.refpoint->y + Ctrl->D.dim[GMT_Y] - Ctrl->C.off[GMT_Y];	/* Top justification edge of items inside legend box accounting for clearance  */
	column_number = 0;	/* Start at first column in multi-column setup */
	n_columns = 1;		/* Reset to default number of columns */
	txtcolor[0] = 0;	/* Reset to black text color */
	x_off_col[0] = 0.0;	/* The x-coordinate of left side of first column */
	x_off_col[n_columns] = Ctrl->D.dim[GMT_X];	/* Holds width of a row */
	
	if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);

	flush_paragraph = false;
	gap = Ctrl->C.off[GMT_Y];	/* This gets reset to 0 once we finish the first printable row */
	
	for (tbl = 0; tbl < In->n_tables; tbl++) {	/* We only expect one table but who knows what the user does */
		for (seg = 0; seg < In->table[tbl]->n_segments; seg++) {	/* We only expect one segment in each table but again... */
			for (row = 0; row < In->table[tbl]->segment[seg]->n_rows; row++) {	/* Finally processing the rows */
				line = In->table[tbl]->segment[seg]->record[row];
				if (line[0] == '#' || GMT_is_a_blank_line (line)) continue;	/* Skip all headers */

				/* Data record to process */

				if (line[0] != 'T' && flush_paragraph) {	/* Flush contents of pending paragraph [Call GMT_pstext] */
					flush_paragraph = false;
					column_number = 0;
				}

				switch (line[0]) {
					case 'A':	/* Z lookup color table change: A cptfile */
						if (P && GMT_Destroy_Data (API, &P) != GMT_OK)	/* Remove the previous CPT from registration */
							Return (API->error);
						for (col = 1; line[col] == ' '; col++);	/* Wind past spaces */
						if ((P = GMT_Read_Data (API, GMT_IS_CPT, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, &line[col], NULL)) == NULL)
							Return (API->error);
						GMT->current.setting.color_model = GMT_RGB;	/* Since we will be interpreting r/g/b triplets via z=<value> */
						break;

					case 'B':	/* B cptname offset height[+modifiers] [ Optional psscale args -B -I -L -M -N -S -Z -p ] */
						/* Color scale Bar [via GMT_psscale] */
						module_options[0] = '\0';
						sscanf (&line[2], "%s %s %s %[^\n]", bar_cpt, bar_gap, bar_height, module_options);
						strcpy (bar_modifiers, bar_height);	/* Save the entire modifier string */
						if ((c = strchr (bar_height, '+'))) c[0] = 0;	/* Chop off any modifiers so we can compute the height */
						row_height = GMT_to_inch (GMT, bar_height) + GMT->current.setting.map_tick_length[0] + GMT->current.setting.map_annot_offset[0] + FONT_HEIGHT_PRIMARY * GMT->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH;
						fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-row_height, row_base_y+gap, x_off_col, &d_line_after_gap, 1, fill);
						x_off = GMT_to_inch (GMT, bar_gap);
						sprintf (buffer, "-C%s -O -K -Dx%gi/%gi+w%gi/%s+h+jTC %s", bar_cpt, Ctrl->D.refpoint->x + 0.5 * Ctrl->D.dim[GMT_X], row_base_y, Ctrl->D.dim[GMT_X] - 2 * x_off, bar_modifiers, module_options);
						status = GMT_Call_Module (API, "psscale", GMT_MODULE_CMD, buffer);	/* Plot the colorbar */
						if (status) {
							GMT_Report (API, GMT_MSG_NORMAL, "GMT_psscale returned error %d.\n", status);
							Return (EXIT_FAILURE);
						}
						row_base_y -= row_height;
						column_number = 0;
						API->io_enabled[GMT_IN] = true;	/* UNDOING SETTING BY psscale */
						drawn = true;
						break;

					case 'C':	/* Font color change: C textcolor */
						sscanf (&line[2], "%[^\n]", txtcolor);
						if ((API->error = GMT_get_rgbtxt_from_z (GMT, P, txtcolor))) Return (EXIT_FAILURE);	/* If given z=value then we look up colors */
						break;

					case 'D':	/* Delimiter record: D [offset] <pen>|- [-|=|+] */
						n_scan = sscanf (&line[2], "%s %s %s", txt_a, txt_b, txt_c);
						if (n_scan < 1) {	/* Clearly a bad record */
							GMT_Report (API, GMT_MSG_NORMAL, "Error: Not enough arguments given to D operator\n");
							Return (GMT_RUNTIME_ERROR);
						}
						if (n_scan == 2) {	/* Either got D <offset> <pen OR D <pen> <flag> */
							if (strchr ("-=+", txt_b[0])) {	/* Offset was skipped and we got D pen flag */
								GMT_Report (API, GMT_MSG_DEBUG, "Got D pen flag --> D 0 pen flag\n");
								strcpy (txt_c, txt_b);
								strcpy (txt_b, txt_a);
								strcpy (txt_a, "0");
							}
							else	/* Just wipe text_c */
								txt_c[0] = '\0';
						}
						else if (n_scan == 1) {	/* Offset was skipped and we got D pen */
							GMT_Report (API, GMT_MSG_DEBUG, "Got D pen --> D 0 pen\n");
							txt_c[0] = '\0';
							strcpy (txt_b, txt_a);
							strcpy (txt_a, "0");
						}
						d_line_hor_offset = GMT_to_inch (GMT, txt_a);
						if (txt_b[0] == '-')	/* Gave - as pen, meaning just note at what y-value we are but draw no line */
							d_line_half_width = 0.0;
						else {	/* Process the pen specification */
							if (txt_b[0] && GMT_getpen (GMT, txt_b, &current_pen)) GMT_pen_syntax (GMT, 'W', " ");
							GMT_setpen (GMT, &current_pen);
							d_line_half_width = 0.5 * current_pen.width / PSL_POINTS_PER_INCH;	/* Half the pen width */
						}
						if (!(txt_c[0] == '-' || txt_c[0] == '=')) {	/* Fill the gap before the line, if fill is active */
							fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-quarter_line_spacing+d_line_half_width, row_base_y+gap, x_off_col, &d_line_after_gap, n_columns, fill);
							row_base_y -= quarter_line_spacing;
						}
						d_line_last_y0 = row_base_y;	/* Remember the y-value were we potentially draw the horizontal line */
						if (d_line_half_width > 0.0) PSL_plotsegment (PSL, Ctrl->D.refpoint->x + d_line_hor_offset, row_base_y, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X] - d_line_hor_offset, row_base_y);
						d_line_after_gap = (txt_c[0] == '+' || txt_c[0] == '=') ? 0.0 : quarter_line_spacing;
						row_base_y -= d_line_after_gap;
						d_line_after_gap -= d_line_half_width;	/* Shrink the gap fill-height after a D line by half the line width so we dont overwrite the line */
						column_number = 0;	/* Reset to new row */
						if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						drawn = true;
						break;

					case 'F':	/* Cell color: F fill1[,fill2,...,filln]  */
						/* First free all previous entries in fill array */
						for (col = 0; col < PSLEGEND_MAX_COLS; col++) if (fill[col]) {free (fill[col]); fill[col] = NULL;}
						pos = n_col = 0;
						while ((GMT_strtok (&line[2], " \t", &pos, p))) {
							if ((API->error = GMT_get_rgbtxt_from_z (GMT, P, p))) Return (EXIT_FAILURE);	/* If given z=value then we look up colors */
							if (strcmp (p, "-")) fill[n_col++] = strdup (p);
							if (n_col > n_columns) {
								GMT_Report (API, GMT_MSG_NORMAL, "Error: Exceeding specified N columns (%d) in F operator (%d)\n", n_columns, n_col);
								Return (GMT_RUNTIME_ERROR);
							}
						}
						if (n_col == 1 && n_columns > 1) {	/* Only gave a constant color or - for the entire row, duplicate per column */
							for (col = 1; col < n_columns; col++) if (fill[0]) fill[col] = strdup (fill[0]);
						}
						break;

					case 'G':	/* Gap record: G gap (will be filled with current fill[0] setting if active) */
						sscanf (&line[2], "%s", txt_a);
						row_height = (txt_a[strlen(txt_a)-1] == 'l') ? atoi (txt_a) * one_line_spacing : GMT_to_inch (GMT, txt_a);
						fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-row_height, row_base_y+gap, x_off_col, &d_line_after_gap, n_columns, fill);
						row_base_y -= row_height;
						column_number = 0;
						if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						drawn = true;
						break;

					case 'H':	/* Header record: H fontsize|- font|- header */
						if ((T[TXT] = get_textset_pointer (API, T[TXT], GMT_IS_NONE)) == NULL) return (API->error);
						sscanf (&line[2], "%s %s %[^\n]", size, font, text);
						if (size[0] == '-') size[0] = 0;
						if (font[0] == '-') font[0] = 0;
						sprintf (tmp, "%s,%s,%s", size, font, txtcolor);		/* Put size, font and color together for parsing by GMT_getfont */
						ifont = GMT->current.setting.font_title;	/* Set default font */
						GMT_getfont (GMT, tmp, &ifont);
						d_off = 0.5 * (Ctrl->D.spacing - FONT_HEIGHT (ifont.id)) * ifont.size / PSL_POINTS_PER_INCH;	/* To center the text */
						row_height = Ctrl->D.spacing * ifont.size / PSL_POINTS_PER_INCH;
						fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-row_height, row_base_y+gap, x_off_col, &d_line_after_gap, n_columns, fill);
						row_base_y -= row_height;
						sprintf (buffer, "%g %g %s BC %s", Ctrl->D.refpoint->x + 0.5 * Ctrl->D.dim[GMT_X], row_base_y + d_off, GMT_putfont (GMT, ifont), text);
						Ts[TXT] = T[TXT]->table[0]->segment[0];	/* Since there will only be one table with one segment for each set, except for fronts */
						Ts[TXT]->record[Ts[TXT]->n_rows++] = strdup (buffer);
						if (debug_print) fprintf (stderr, "%s\n", buffer);
						if (Ts[TXT]->n_rows == Ts[TXT]->n_alloc) Ts[TXT]->record = GMT_memory (GMT, Ts[TXT]->record, Ts[TXT]->n_alloc += GMT_SMALL_CHUNK, char *);
						column_number = 0;
						if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						drawn = true;
						break;

					case 'I':	/* Image record [use GMT_psimage]: I imagefile width justification */
						sscanf (&line[2], "%s %s %s", image, size, key);
						if (GMT_getdatapath (GMT, image, path, R_OK) == NULL) {
							GMT_Report (API, GMT_MSG_NORMAL, "Cannot find/open file %s.\n", image);
							Return (EXIT_FAILURE);
						}
						PSL_loadimage (PSL, path, &header, &dummy);
						PSL_free (dummy);
						justify = GMT_just_decode (GMT, key, PSL_NO_DEF);
						row_height = GMT_to_inch (GMT, size) * (double)header.height / (double)header.width;
						fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-row_height, row_base_y+gap, x_off_col, &d_line_after_gap, n_columns, fill);
						x_off = Ctrl->D.refpoint->x;
						x_off += (justify%4 == 1) ? Ctrl->C.off[GMT_X] : ((justify%4 == 3) ? Ctrl->D.dim[GMT_X] - Ctrl->C.off[GMT_X] : 0.5 * Ctrl->D.dim[GMT_X]);
						sprintf (buffer, "-O -K %s -Dx%gi/%gi+j%s+w%s", image, x_off, row_base_y, key, size);
						status = GMT_Call_Module (API, "psimage", GMT_MODULE_CMD, buffer);	/* Plot the image */
						if (status) {
							GMT_Report (API, GMT_MSG_NORMAL, "GMT_psimage returned error %d.\n", status);
							Return (EXIT_FAILURE);
						}
						row_base_y -= row_height;
						column_number = 0;
						drawn = true;
						break;

					case 'L':	/* Label record: L fontsize|- font|- justification label */
						text[0] = '\0';
						if ((T[TXT] = get_textset_pointer (API, T[TXT], GMT_IS_NONE)) == NULL) return (API->error);
						sscanf (&line[2], "%s %s %s %[^\n]", size, font, key, text);
						if (size[0] == '-') size[0] = 0;
						if (font[0] == '-') font[0] = 0;
						sprintf (tmp, "%s,%s,%s", size, font, txtcolor);		/* Put size, font and color together for parsing by GMT_getfont */
						ifont = GMT->current.setting.font_label;	/* Set default font */
						GMT_getfont (GMT, tmp, &ifont);
						d_off = 0.5 * (Ctrl->D.spacing - FONT_HEIGHT (ifont.id)) * ifont.size / PSL_POINTS_PER_INCH;	/* To center the text */
						if (column_number%n_columns == 0) {	/* Label in first column, also fill row if requested */
							row_height = Ctrl->D.spacing * ifont.size / PSL_POINTS_PER_INCH;
							fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-row_height, row_base_y+gap, x_off_col, &d_line_after_gap, n_columns, fill);
							row_base_y -= row_height;
							column_number = 0;
							if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						}
						if (text[0] == '\0') {	/* Nothing to do, just skip to next */
							column_number++;
							GMT_Report (API, GMT_MSG_LONG_VERBOSE, "The L record give no info so skip to next cell\n");
							drawn = true;
							break;
						}
						justify = GMT_just_decode (GMT, key, 0);
						x_off = Ctrl->D.refpoint->x + x_off_col[column_number];
						x_off += (justify%4 == 1) ? Ctrl->C.off[GMT_X] : ((justify%4 == 3) ? (x_off_col[column_number+1]-x_off_col[column_number]) - Ctrl->C.off[GMT_X] : 0.5 * (x_off_col[column_number+1]-x_off_col[column_number]));
						sprintf (buffer, "%g %g %s B%s %s", x_off, row_base_y + d_off, GMT_putfont (GMT, ifont), key, text);
						Ts[TXT] = T[TXT]->table[0]->segment[0];	/* Since there will only be one table with one segment for each set, except for fronts */
						Ts[TXT]->record[Ts[TXT]->n_rows++] = strdup (buffer);
						if (debug_print) fprintf (stderr, "%s\n", buffer);
						if (Ts[TXT]->n_rows == Ts[TXT]->n_alloc) Ts[TXT]->record = GMT_memory (GMT, Ts[TXT]->record, Ts[TXT]->n_alloc += GMT_SMALL_CHUNK, char *);
						column_number++;
						drawn = true;
						break;

					case 'M':	/* Map scale record M lon0|- lat0 length[n|m|k][+mods] [-F] [-R -J] */
						/* Was: Map scale record M lon0|- lat0 length[n|m|k][+opts] f|p  [-R -J] */
						n_scan = sscanf (&line[2], "%s %s %s %s %s %s", txt_a, txt_b, txt_c, txt_d, txt_e, txt_f);
						r_options[0] = module_options[0] = '\0';
						if (txt_d[0] == 'f' || txt_d[0] == 'p') {	/* Old-style args */
							if (n_scan == 6)	/* Gave -R -J */
								sprintf (r_options, "%s %s", txt_e, txt_f);
							if (txt_d[0] == 'f') strcat (txt_c, "+f");	/* Wanted fancy scale so apped +f to length */
						}
						else {	/* New syntax */
							if (n_scan == 4)	/* Gave just -F */
								strcpy (module_options, txt_d);
							else if (n_scan == 5)	/* Gave -R -J */
								sprintf (r_options, "%s %s", txt_d, txt_e);
							else if (n_scan == 6) {	/* Gave -F -R -J which may be in any order */
								if (txt_d[1] == 'F') {	/* Got -F -R -J */
									sprintf (module_options, " %s", txt_d);
									sprintf (r_options, "%s %s", txt_e, txt_f);
								}
								else if (txt_f[1] == 'F') {	/* Got -R -J -F */
									sprintf (r_options, "%s %s", txt_d, txt_e);
									sprintf (module_options, " %s", txt_f);
								}
								else {	/* Got -R -F -J or -J -F -R */
									sprintf (r_options, "%s %s", txt_d, txt_f);
									sprintf (module_options, " %s", txt_e);
								}
							}
						}
						for (i = 0, gave_mapscale_options = false; txt_c[i] && !gave_mapscale_options; i++) if (txt_c[i] == '+') gave_mapscale_options = true;
						/* Default assumes label is added on top */
						just = 't';
						gave_label = true;
						row_height = GMT->current.setting.map_scale_height + FONT_HEIGHT_PRIMARY * GMT->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH + GMT->current.setting.map_annot_offset[0];
						d_off = FONT_HEIGHT_LABEL * GMT->current.setting.font_label.size / PSL_POINTS_PER_INCH + fabs(GMT->current.setting.map_label_offset);
						if ((txt_d[0] == 'f' || txt_d[0] == 'p') && GMT_get_modifier (txt_c, 'j', string))	/* Specified alternate justification old-style */
							just = string[0];
						else if (GMT_get_modifier (txt_c, 'a', string))	/* Specified alternate aligment */
							just = string[0];
						if (GMT_get_modifier (txt_c, 'u', string))	/* Specified alternate aligment */
							gave_label = false;	/* Not sure why I do this, will find out */
						h = row_height;
						if (gave_label && (just == 't' || just == 'b')) h += d_off;
						fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-h, row_base_y+gap, x_off_col, &d_line_after_gap, n_columns, fill);
						if (gave_label && just == 't') row_base_y -= d_off;
						if (!strcmp (txt_a, "-"))	/* No longitude needed */
							sprintf (mapscale, "x%gi/%gi+c%s+jTC+w%s", Ctrl->D.refpoint->x + 0.5 * Ctrl->D.dim[GMT_X], row_base_y, txt_b, txt_c);
						else				/* Gave both lon and lat for scale */
							sprintf (mapscale, "x%gi/%gi+c%s/%s+jTC+w%s", Ctrl->D.refpoint->x + 0.5 * Ctrl->D.dim[GMT_X], row_base_y, txt_a, txt_b, txt_c);
						if (r_options[0])	/* Gave specific -R -J on M line */
							sprintf (buffer, "%s -O -K -L%s", r_options, mapscale);
						else {	/* Must use -R -J as supplied to pslegend */
							if (!r_ptr || !j_ptr) {
								GMT_Report (API, GMT_MSG_NORMAL, "Error: The M record must have map -R -J if -Dx and no -R -J is used\n");
								Return (GMT_RUNTIME_ERROR);
							}
							sprintf (buffer, "-R%s -J%s -O -K -L%s", r_ptr->arg, j_ptr->arg, mapscale);
						}
						if (module_options[0]) strcat (buffer, module_options);
						status = GMT_Call_Module (API, "psbasemap", GMT_MODULE_CMD, buffer);	/* Plot the scale */
						if (status) {
							GMT_Report (API, GMT_MSG_NORMAL, "GMT_psbasemap returned error %d.\n", status);
							Return (EXIT_FAILURE);
						}
						if (gave_label && just == 'b') row_base_y -= d_off;
						row_base_y -= row_height;
						column_number = 0;
						if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						drawn = true;
						break;

					case 'N':	/* n_columns record: N ncolumns OR rw1 rw2 ... rwn (for relative widths) */
						n_col = n_columns;	/* Previous setting */
						pos = n_columns = 0;
						while ((GMT_strtok (&line[2], " \t", &pos, p))) {
							col_width[n_columns++] = atof (p);
							if (n_columns == PSLEGEND_MAX_COLS) {
								GMT_Report (API, GMT_MSG_NORMAL, "Error: Exceeding maximum columns (%d) in N operator\n", PSLEGEND_MAX_COLS);
								Return (GMT_RUNTIME_ERROR);
							}
						}
						if (n_columns == 0) n_columns = 1;	/* Default to 1 if nothing is given */
						/* Check if user gave just the number of columns and not relative widths */
						if (n_columns == 1 && (pos = atoi (&line[2])) > 1) {	/* Set number of columns and indicate equal widths */
							n_columns = pos;
							for (column_number = 0; column_number < n_columns; column_number++) col_width[column_number] = 1.0;
						}
						for (column_number = 0, sum_width = 0.0; column_number < n_columns; column_number++) sum_width += col_width[column_number];
						for (column_number = 1, x_off_col[0] = 0.0; column_number < n_columns; column_number++) x_off_col[column_number] = x_off_col[column_number-1] + (Ctrl->D.dim[GMT_X] * col_width[column_number-1] / sum_width);
						x_off_col[column_number] = Ctrl->D.dim[GMT_X];	/* Holds width of row */
						if (n_columns > n_col) for (col = 1; col < n_columns; col++) if (fill[0]) fill[col] = strdup (fill[0]);	/* Extend the fill array to match the new column numbers, if fill is active */
						column_number = 0;
						if (GMT_is_verbose (GMT, GMT_MSG_DEBUG)) {
							double s = GMT_convert_units (GMT, "1", GMT_INCH, GMT->current.setting.proj_length_unit);
							GMT_Report (API, GMT_MSG_DEBUG, "Selected %d columns.  Column widths are:\n", n_columns);
							for (col = 1; col <= n_columns; col++)
								GMT_Report (API, GMT_MSG_DEBUG, "Column %d: %g %s\n", col, s*(x_off_col[col]-x_off_col[col-1]), GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
						}
						break;

					case '>':	/* Paragraph text header */
						if (GMT_compat_check (GMT, 4)) {	/* Warn and fall through */
							GMT_Report (API, GMT_MSG_COMPAT, "Warning: paragraph text header flag > is deprecated; use P instead\n");
							n = sscanf (&line[1], "%s %s %s %s %s %s %s %s %s", xx, yy, size, angle, font, key, lspace, tw, jj);
							if (n < 0) n = 0;	/* Since -1 is returned if no arguments */
							if (!(n == 0 || n == 9)) {
								GMT_Report (API, GMT_MSG_NORMAL, "Error: The > record must have 0 or 9 arguments (only %d found)\n", n);
								Return (GMT_RUNTIME_ERROR);
							}
							if (n == 0 || size[0] == '-') sprintf (size, "%g", GMT->current.setting.font_annot[0].size);
							if (n == 0 || font[0] == '-') sprintf (font, "%d", GMT->current.setting.font_annot[0].id);
							sprintf (tmp, "%s,%s,", size, font);
							did_old = true;
						}
						else {
							GMT_Report (API, GMT_MSG_NORMAL, "Error: Unrecognized record (%s)\n", line);
							Return (GMT_RUNTIME_ERROR);
							break;
						}
					case 'P':	/* Paragraph text header: P paragraph-mode-header-for-pstext */
						if (!did_old) {
							n = sscanf (&line[1], "%s %s %s %s %s %s %s %s", xx, yy, tmp, angle, key, lspace, tw, jj);
							if (n < 0) n = 0;	/* Since -1 is returned if no arguments */
							if (!(n == 0 || n == 8)) {
								GMT_Report (API, GMT_MSG_NORMAL, "Error: The P record must have 0 or 8 arguments (only %d found)\n", n);
								Return (GMT_RUNTIME_ERROR);
							}
						}
						did_old = false;
						if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						if (n == 0 || xx[0] == '-') sprintf (xx, "%g", col_left_x);
						if (n == 0 || yy[0] == '-') sprintf (yy, "%g", row_base_y);
						if (n == 0 || tmp[0] == '-') sprintf (tmp, "%g,%d,%s", GMT->current.setting.font_annot[0].size, GMT->current.setting.font_annot[0].id, txtcolor);
						if (n == 0 || angle[0] == '-') sprintf (angle, "0");
						if (n == 0 || key[0] == '-') sprintf (key, "TL");
						if (n == 0 || lspace[0] == '-') sprintf (lspace, "%gi", one_line_spacing);
						if (n == 0 || tw[0] == '-') sprintf (tw, "%gi", Ctrl->D.dim[GMT_X] - 2.0 * Ctrl->C.off[GMT_X]);
						if (n == 0 || jj[0] == '-') sprintf (jj, "j");
						if ((T[PAR] = get_textset_pointer (API, T[PAR], GMT_IS_NONE)) == NULL) return (API->error);
						sprintf (buffer, "> %s %s %s %s %s %s %s %s", xx, yy, tmp, angle, key, lspace, tw, jj);
						Ts[PAR] = T[PAR]->table[0]->segment[0];	/* Since there will only be one table with one segment for each set, except for fronts */
						Ts[PAR]->record[Ts[PAR]->n_rows++] = strdup (buffer);
						if (Ts[PAR]->n_rows == Ts[PAR]->n_alloc) Ts[PAR]->record = GMT_memory (GMT, Ts[PAR]->record, Ts[PAR]->n_alloc += GMT_SMALL_CHUNK, char *);
						flush_paragraph = true;
						column_number = 0;
						drawn = true;
						break;

					case 'S':	/* Symbol record: S [dx1 symbol size fill pen [ dx2 text ]] */
						if (strlen (line) > 2)
							n_scan = sscanf (&line[2], "%s %s %s %s %s %s %[^\n]", txt_a, symbol, size, txt_c, txt_d, txt_b, text);
						else	/* No args given means skip to next cell */
							n_scan = 0;
						if (column_number%n_columns == 0) {	/* Symbol in first column, also fill row if requested */
							fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-one_line_spacing, row_base_y+gap, x_off_col, &d_line_after_gap, n_columns, fill);
							row_base_y -= one_line_spacing;
							column_number = 0;
						}
						if (n_scan <= 0) {	/* No symbol, just skip to next cell */
							column_number++;
							GMT_Report (API, GMT_MSG_DEBUG, "The S record give no info so skip to next cell\n");
							drawn = true;
							break;
						}
						if ((API->error = GMT_get_rgbtxt_from_z (GMT, P, txt_c))) Return (EXIT_FAILURE);	/* If given z=value then we look up colors */
						if (strchr ("LCR", txt_a[0])) {	/* Gave L, C, or R justification relative to current cell */
							justify = GMT_just_decode (GMT, txt_a, 0);
							off_ss = (justify%4 == 1) ? Ctrl->C.off[GMT_X] : ((justify%4 == 3) ? (x_off_col[column_number+1]-x_off_col[column_number]) - Ctrl->C.off[GMT_X] : 0.5 * (x_off_col[column_number+1]-x_off_col[column_number]));
							x_off = Ctrl->D.refpoint->x + x_off_col[column_number];
						}
						else {
							off_ss = GMT_to_inch (GMT, txt_a);
							x_off = col_left_x + x_off_col[column_number];
						}
						off_tt = GMT_to_inch (GMT, txt_b);
						d_off = 0.5 * (Ctrl->D.spacing - FONT_HEIGHT_PRIMARY) * GMT->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH;	/* To center the text */
						row_base_y += half_line_spacing;	/* Move to center of box */
						if (symbol[0] == 'f') {	/* Front is different, must plot as a line segment */
							double length, tlen, gap;
							int n = sscanf (size, "%[^/]/%[^/]/%s", A, B, C);
							
							if ((D[FRONT] = get_dataset_pointer (API, D[FRONT], GMT_IS_LINE)) == NULL) return (API->error);
							if (n == 3) {	/* Got line length, tickgap, and ticklength */
								length = GMT_to_inch (GMT, A);	/* The length of the line */
								tlen = GMT_to_inch (GMT, C);	/* The length of the tick */
							}
							else if (n == 2 && B[0] != '-') {	/* Got line length and tickgap only */
								length = GMT_to_inch (GMT, A);	/* The length of the line */
								gap = GMT_to_inch (GMT, B);	/* The tick gap */
								tlen = 0.3 * gap;		/* The default length of the tick is 30% of gap */
							}
							else {	/* Got line length, select defaults for other things */
								length = GMT_to_inch (GMT, A);	/* The length of the line */
								strcpy (B, "-1");		/* One centered tick */
								tlen = 0.3 * length;		/* The default length of the tick is 30% of length */
							}
							if ((c = strchr (symbol, '+')))	/* Pass along all the given modifiers */
								strcpy (sub, c);
							else	/* The necessary arguments not supplied, provide reasonable defaults */
								sprintf (sub, "+l+b");	/* Box to the left of the line is our default front symbol */
							x = 0.5 * length;
							/* Place pen and fill colors in segment header */
							sprintf (buffer, "> -Sf%s/%gi%s", B, tlen, sub);
							if (txt_c[0] != '-') {strcat (buffer, " -G"); strcat (buffer, txt_c);}
							if (txt_d[0] != '-') {strcat (buffer, " -W"); strcat (buffer, txt_d);}
							Ds[FRONT] = D[FRONT]->table[0]->segment[n_fronts];	/* Next segment */
							Ds[FRONT]->header = strdup (buffer);
							/* Set begin and end coordinates of the line segment */
							Ds[FRONT]->coord[GMT_X][0] = x_off + off_ss-x;	Ds[FRONT]->coord[GMT_Y][0] = row_base_y;
							Ds[FRONT]->coord[GMT_X][1] = x_off + off_ss+x;	Ds[FRONT]->coord[GMT_Y][1] = row_base_y;
							D[FRONT]->n_records += 2;
							n_fronts++;
							if (n_fronts == GMT_SMALL_CHUNK) {
								GMT_Report (API, GMT_MSG_NORMAL, "Can handle max %d front lines.  Let us know if this is a problem.\n", GMT_SMALL_CHUNK);
								return (API->error);
							}
						}
						else if (symbol[0] == 'q') {	/* Quoted line is different, must plot as a line segment */
							double length = GMT_to_inch (GMT, size);	/* The length of the line */;
							
							if ((D[QLINE] = get_dataset_pointer (API, D[QLINE], GMT_IS_LINE)) == NULL) return (API->error);
							x = 0.5 * length;
							/* Place pen and fill colors in segment header */
							sprintf (buffer, "> -S%s", symbol);
							if (txt_d[0] != '-') {strcat (buffer, " -W"); strcat (buffer, txt_d);}
							Ds[QLINE] = D[QLINE]->table[0]->segment[n_quoted_lines];	/* Next segment */
							Ds[QLINE]->header = strdup (buffer);
							/* Set begin and end coordinates of the line segment */
							Ds[QLINE]->coord[GMT_X][0] = x_off + off_ss-x;	Ds[QLINE]->coord[GMT_Y][0] = row_base_y;
							Ds[QLINE]->coord[GMT_X][1] = x_off + off_ss+x;	Ds[QLINE]->coord[GMT_Y][1] = row_base_y;
							D[QLINE]->n_records += 2;
							n_quoted_lines++;
							if (n_quoted_lines == GMT_SMALL_CHUNK) {
								GMT_Report (API, GMT_MSG_NORMAL, "Can handle max %d quoted lines.  Let us know if this is a problem.\n", GMT_SMALL_CHUNK);
								return (API->error);
							}
						}
						else {	/* Regular symbols */
							if ((T[SYM] = get_textset_pointer (API, T[SYM], GMT_IS_POINT)) == NULL) return (API->error);
							Ts[SYM] = T[SYM]->table[0]->segment[0];	/* Since there will only be one table with one segment for each set, except for fronts */
							sprintf (sub, "%s", symbol);
							if (symbol[0] == 'E' || symbol[0] == 'e') {	/* Ellipse */
								if (strchr (size, ',')) {	/* We got dir,major,minor instead of just size; parse and use */
									sscanf (size, "%[^,],%[^,],%s", A, B, C);
									az1 = atof (A);
									x = GMT_to_inch (GMT, B);
									y = GMT_to_inch (GMT, C);
								}
								else {	/* Ellipse needs more arguments; we use minor = 0.65*major, az = 0 */
									x = GMT_to_inch (GMT, size);
									az1 = 0.0;
									y = 0.65 * x;
								}
								sprintf (sarg, "%g %g %g %gi %gi", x_off + off_ss, row_base_y, az1, x, y);
							}
							else if (symbol[0] == 'J' || symbol[0] == 'j') {	/* rotated rectangle */
								if (strchr (size, ',')) {	/* We got dir,w,h instead of just size; parse and use */
									sscanf (size, "%[^,],%[^,],%s", A, B, C);
									x = GMT_to_inch (GMT, B);
									y = GMT_to_inch (GMT, C);
									sprintf (sarg, "%g %g %s %gi %gi", x_off + off_ss, row_base_y, A, x, y);
								}
								else {	/* Rotated rectangle needs more arguments; we use height = 0.65*width, az = 30 */
									x = GMT_to_inch (GMT, size);
									sprintf (sarg, "%g %g 30 %gi %gi", x_off + off_ss, row_base_y, x, 0.65 * x);
								}
							}
							else if (symbol[0] == 'V' || symbol[0] == 'v') {	/* Vector */
								/* Because we support both GMT4 and GMT5 vector notations this section is a bit messy */
								if (strchr (size, ',')) {	/* We got dir,length combined as one argument */
									sscanf (size, "%[^,],%s", A, B);
									az1 = atof (A);
									x = GMT_to_inch (GMT, B);
								}
								else {	/* No dir given, default to horizontal */
									az1 = 0.0;
									x = GMT_to_inch (GMT, size);
								}
								if (strchr (size, '/') && GMT_compat_check (GMT, 4))  {	/* The necessary arguments was supplied via GMT4 size arguments */
									i = 0;
									while (size[i] != '/' && size[i]) i++;
									size[i++] = '\0';	/* So GMT_to_inch won't complain */
									sprintf (sub, "%s%s+jc+e", symbol, &size[i]);
								}
								else if ((c = strchr (symbol, '+'))) {	/* GMT5 syntax: Pass along all the given modifiers */
									strcpy (sub, symbol);
									if ((c = strstr (sub, "+j")) && c[2] != 'c') {	/* Got justification, check if it is +jc */
										GMT_Report (API, GMT_MSG_NORMAL, "Warning: Vector justification changed from +j%c to +jc\n", c[2]);
										c[2] = 'c';	/* Replace with centered justification */
									}
									else	/* Add +jc */
										strcat (sub, "+jc");
								}
								else	/* The necessary arguments not supplied, so we make a reasonable default */
									sprintf (sub, "v%gi+jc+e", 0.3*x);	/* Head size is 30% of length */
								if (txt_c[0] == '-') strcat (sub, "+g-");
								else { strcat (sub, "+g"); strcat (sub, txt_c);}
								if (txt_d[0] == '-') strcat (sub, "+p-");
								else { strcat (sub, "+p"); strcat (sub, txt_d);}
								sprintf (sarg, "%g %g %g %gi", x_off + off_ss, row_base_y, az1, x);
							}
							else if (symbol[0] == 'r') {	/* Rectangle  */
								if (strchr (size, ',')) {	/* We got w,h */
									sscanf (size, "%[^,],%s", A, B);
									x = GMT_to_inch (GMT, A);
									y = GMT_to_inch (GMT, B);
								}
								else {	/* Rectangle also need more args, we use h = 0.65*w */
									x = GMT_to_inch (GMT, size);
									y = 0.65 * x;
								}
								sprintf (sarg, "%g %g %gi %gi", x_off + off_ss, row_base_y, x, y);
							}
							else if (symbol[0] == 'R') {	/* Rounded rectangle  */
								if (strchr (size, ',')) {	/* We got w,h,r */
									sscanf (size, "%[^,],%[^,],%s", A, B, C);
									x = GMT_to_inch (GMT, A);
									y = GMT_to_inch (GMT, B);
									r = GMT_to_inch (GMT, C);
								}
								else {	/* Rounded rectangle also need more args, we use h = 0.65*w and r = 0.1*w */
									x = GMT_to_inch (GMT, size);
									y = 0.65 * x;
									r = 0.1 * x;
								}
								sprintf (sarg, "%g %g %gi %gi %gi", x_off + off_ss, row_base_y, x, y, r);
							}
							else if (symbol[0] == 'm') {	/* Math angle  */
								if (strchr (size, ',')) {	/* We got r,az1,az2 */
									sscanf (size, "%[^,],%[^,],%s", A, B, C);
									x = GMT_to_inch (GMT, A);
									az1 = atof (B);
									az2 = atof (C);
								}
								else {	/* Math angle need more args, we set fixed az1,az22 as 10 45 */
									x = GMT_to_inch (GMT, size);
									az1 = 10;	az2 = 45;
								}
								/* We want to center the arc around its mid-point */
								m_az = 0.5 * (az1 + az2);
								dx = 0.25 * x * cosd (m_az);	dy = 0.25 * x * sind (m_az);
								sprintf (sarg, "%g %g %gi %g %g", x_off + off_ss - dx, row_base_y - dy, x, az1, az2);
								if (!strchr (symbol, '+'))  {	/* The necessary arguments not supplied! */
									sprintf (sub, "m%gi+b+e", 0.3*x);	/* Double heads, head size 30% of diameter */
								}
								if (txt_c[0] == '-') strcat (sub, "+g-");
								else { strcat (sub, "+g"); strcat (sub, txt_c);}
								if (txt_d[0] == '-') strcat (sub, "+p-");
								else { strcat (sub, "+p"); strcat (sub, txt_d);}
							}
							else if (symbol[0] == 'w') {	/* Wedge also need more args; we set fixed az1,az2 as -30 30 */
								if (strchr (size, ',')) {	/* We got az1,az2,d */
									sscanf (size, "%[^,],%[^,],%s", A, B, C);
									az1 = atof (A);
									az2 = atof (B);
									x = GMT_to_inch (GMT, C);
								}
								else {
									x = GMT_to_inch (GMT, size);
									az1 = -30;	az2 = 30;
								}
								/* We want to center the wedge around its mid-point */
								m_az = 0.5 * (az1 + az2);
								dx = 0.25 * x * cosd (m_az);	dy = 0.25 * x * sind (m_az);
								sprintf (sarg, "%g %g %gi %g %g", x_off + off_ss - dx, row_base_y - dy, x, az1, az2);
							}
							else {
								x = GMT_to_inch (GMT, size);
								sprintf (sarg, "%g %g %gi", x_off + off_ss, row_base_y, x);
							}
							/* Place pen and fill colors in segment header */
							sprintf (buffer, ">");
							strcat (buffer, " -G"); strcat (buffer, txt_c);
							strcat (buffer, " -W"); strcat (buffer, txt_d);
							Ts[SYM]->record[Ts[SYM]->n_rows++] = strdup (buffer);
							if (debug_print) fprintf (stderr, "%s\n", buffer);
							if (Ts[SYM]->n_rows == Ts[SYM]->n_alloc) Ts[SYM]->record = GMT_memory (GMT, Ts[SYM]->record, Ts[SYM]->n_alloc += GMT_SMALL_CHUNK, char *);
							sprintf (buffer, "%s %s", sarg, sub);
							if (debug_print) fprintf (stderr, "%s\n", buffer);
							
							Ts[SYM]->record[Ts[SYM]->n_rows++] = strdup (buffer);
							if (Ts[SYM]->n_rows == Ts[SYM]->n_alloc) Ts[SYM]->record = GMT_memory (GMT, Ts[SYM]->record, Ts[SYM]->n_alloc += GMT_SMALL_CHUNK, char *);
						}
						/* Finally, print text; skip when empty */
						row_base_y -= half_line_spacing;	/* Go back to bottom of box */
						if (n_scan == 7) {	/* Place symbol text */
							if ((T[TXT] = get_textset_pointer (API, T[TXT], GMT_IS_NONE)) == NULL) return (API->error);
							Ts[TXT] = T[TXT]->table[0]->segment[0];	/* Since there will only be one table with one segment for each set, except for fronts */
							sprintf (buffer, "%g %g %g,%d,%s BL %s", x_off + off_tt, row_base_y + d_off, GMT->current.setting.font_annot[0].size, GMT->current.setting.font_annot[0].id, txtcolor, text);
							if (debug_print) fprintf (stderr, "%s\n", buffer);
							Ts[TXT]->record[Ts[TXT]->n_rows++] = strdup (buffer);
							if (Ts[TXT]->n_rows == Ts[TXT]->n_alloc) Ts[TXT]->record = GMT_memory (GMT, Ts[TXT]->record, Ts[TXT]->n_alloc += GMT_SMALL_CHUNK, char *);
						}
						column_number++;
						if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						drawn = true;
						break;

					case 'T':	/* paragraph text record: T paragraph-text */
						if ((T[PAR] = get_textset_pointer (API, T[PAR], GMT_IS_NONE)) == NULL) return (API->error);
						/* If no previous > record, then use defaults */
						Ts[PAR] = T[PAR]->table[0]->segment[0];	/* Since there will only be one table with one segment for each set, except for fronts */
						if (!flush_paragraph) {
							d_off = 0.5 * (Ctrl->D.spacing - FONT_HEIGHT_PRIMARY) * GMT->current.setting.font_annot[0].size / PSL_POINTS_PER_INCH;
							sprintf (buffer, "> %g %g %g,%d,%s 0 TL %gi %gi j", col_left_x, row_base_y - d_off, GMT->current.setting.font_annot[0].size, GMT->current.setting.font_annot[0].id, txtcolor, one_line_spacing, Ctrl->D.dim[GMT_X] - 2.0 * Ctrl->C.off[GMT_X]);
							Ts[PAR]->record[Ts[PAR]->n_rows++] = strdup (buffer);
							if (Ts[PAR]->n_rows == Ts[PAR]->n_alloc) Ts[PAR]->record = GMT_memory (GMT, Ts[PAR]->record, Ts[PAR]->n_alloc += GMT_SMALL_CHUNK, char *);
						}
						sscanf (&line[2], "%[^\n]", text);
						Ts[PAR]->record[Ts[PAR]->n_rows++] = strdup (text);
						if (Ts[PAR]->n_rows == Ts[PAR]->n_alloc) Ts[PAR]->record = GMT_memory (GMT, Ts[PAR]->record, Ts[PAR]->n_alloc += GMT_SMALL_CHUNK, char *);
						flush_paragraph = true;
						column_number = 0;
						drawn = true;
						break;

					case 'V':	/* Vertical line from here to next V: V offset pen */
						n_scan = sscanf (&line[2], "%s %s", txt_a, txt_b);
						if (n_scan == 1) {	/* Assume user forgot to give offset as he/she wants 0 */
							strcpy (txt_b, txt_a);
							strcpy (txt_a, "0");
						}
						if (v_line_draw_now) {	/* Second time, now draw line */
							double v_line_y_stop = d_line_last_y0;
							v_line_ver_offset = GMT_to_inch (GMT, txt_a);
							if (txt_b[0] && GMT_getpen (GMT, txt_b, &current_pen)) {
								GMT_pen_syntax (GMT, 'V', " ");
								Return (GMT_RUNTIME_ERROR);
							}
							GMT_setpen (GMT, &current_pen);
							for (i = 1; i < n_columns; i++) {
								x_off = Ctrl->D.refpoint->x + x_off_col[i];
								PSL_plotsegment (PSL, x_off, v_line_y_start-v_line_ver_offset, x_off, v_line_y_stop+v_line_ver_offset);
							}
							v_line_draw_now = false;
						}
						else {	/* First time, mark from where we draw vertical line */
							v_line_draw_now = true;
							v_line_y_start = d_line_last_y0;
						}
						column_number = 0;
						if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						break;

					default:
						GMT_Report (API, GMT_MSG_NORMAL, "Error: Unrecognized record (%s)\n", line);
						Return (GMT_RUNTIME_ERROR);
					break;
				}
				if (drawn) gap = 0.0;	/* No longer first record that draws on page */
			}
		}
	}
	/* If there is clearance and fill is active we must paint the clearance row */
	if (Ctrl->C.off[GMT_Y] > 0.0) fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-Ctrl->C.off[GMT_Y], row_base_y, x_off_col, &d_line_after_gap, n_columns, fill);

	if (GMT_Destroy_Data (API, &In) != GMT_OK) {	/* Remove the main input file from registration */
		Return (API->error);
	}
	if (P && GMT_Destroy_Data (API, &P) != GMT_OK)	/* Remove the last CPT from registration */
		Return (API->error);

	/* Reset the flag */
	if (GMT_compat_check (GMT, 4)) GMT->current.setting.io_seg_marker[GMT_IN] = save_EOF;

	if (Ctrl->F.active)	/* Draw legend frame box */
		GMT_draw_map_panel (GMT, Ctrl->D.refpoint->x + 0.5 * Ctrl->D.dim[GMT_X], Ctrl->D.refpoint->y + 0.5 * Ctrl->D.dim[GMT_Y], 2U, Ctrl->F.panel);
	
	/* Time to plot any symbols, text, and paragraphs we collected in the loop */

	if (D[FRONT]) {
		/* Create option list, register D[FRONT] as input source */
		D[FRONT]->table[0]->n_segments = n_fronts;	/* Set correct number of fronts */
		if ((object_ID = GMT_Get_ID (API, GMT_IS_DATASET, GMT_IN, D[FRONT])) == GMT_NOTSET) {
			Return (API->error);
		}
		if (GMT_Encode_ID (API, string, object_ID) != GMT_OK) {
			Return (API->error);	/* Make filename with embedded object ID */
		}
		sprintf (buffer, "-R0/%g/0/%g -Jx1i -O -K -N -Sf0.1i %s", GMT->current.proj.rect[XHI], GMT->current.proj.rect[YHI], string);
		if (debug_print) fprintf (stderr, "%s\n", buffer);
		if (GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, buffer) != GMT_OK) {	/* Plot the fronts */
			Return (API->error);
		}
		D[FRONT]->table[0]->n_segments = GMT_SMALL_CHUNK;	/* Reset to allocation limit */
	}
	if (D[QLINE]) {
		/* Create option list, register D[QLINE] as input source */
		D[QLINE]->table[0]->n_segments = n_quoted_lines;	/* Set correct number of lines */
		if ((object_ID = GMT_Get_ID (API, GMT_IS_DATASET, GMT_IN, D[QLINE])) == GMT_NOTSET) {
			Return (API->error);
		}
		if (GMT_Encode_ID (API, string, object_ID) != GMT_OK) {
			Return (API->error);	/* Make filename with embedded object ID */
		}
		sprintf (buffer, "-R0/%g/0/%g -Jx1i -O -K -N -Sqn1 %s", GMT->current.proj.rect[XHI], GMT->current.proj.rect[YHI], string);
		if (debug_print) fprintf (stderr, "%s\n", buffer);
		if (GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, buffer) != GMT_OK) {	/* Plot the fronts */
			Return (API->error);
		}
		D[QLINE]->table[0]->n_segments = GMT_SMALL_CHUNK;	/* Reset to allocation limit */
	}
	if (T[SYM]) {
		/* Create option list, register T[SYM] as input source */
		if ((object_ID = GMT_Get_ID (API, GMT_IS_TEXTSET, GMT_IN, T[SYM])) == GMT_NOTSET) {
			Return (API->error);
		}
		if (GMT_Encode_ID (API, string, object_ID) != GMT_OK) {
			Return (API->error);	/* Make filename with embedded object ID */
		}
		sprintf (buffer, "-R0/%g/0/%g -Jx1i -O -K -N -S %s", GMT->current.proj.rect[XHI], GMT->current.proj.rect[YHI], string);
		if (debug_print) fprintf (stderr, "%s\n", buffer);
		if (GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, buffer) != GMT_OK) {	/* Plot the symbols */
			Return (API->error);
		}
	}
	if (T[TXT]) {
		/* Create option list, register T[TXT] as input source */
		if ((object_ID = GMT_Get_ID (API, GMT_IS_TEXTSET, GMT_IN, T[TXT])) == GMT_NOTSET) {
			Return (API->error);
		}
		if (GMT_Encode_ID (API, string, object_ID) != GMT_OK) {
			Return (API->error);	/* Make filename with embedded object ID */
		}
		sprintf (buffer, "-R0/%g/0/%g -Jx1i -O -K -N -F+f+j %s", GMT->current.proj.rect[XHI], GMT->current.proj.rect[YHI], string);
		if (debug_print) fprintf (stderr, "%s\n", buffer);
		if (GMT_Call_Module (API, "pstext", GMT_MODULE_CMD, buffer) != GMT_OK) {	/* Plot the symbol labels */
			Return (API->error);
		}
	}
	if (T[PAR]) {
		/* Create option list, register T[PAR] as input source */
		if ((object_ID = GMT_Get_ID (API, GMT_IS_TEXTSET, GMT_IN, T[PAR])) == GMT_NOTSET) {
			Return (API->error);
		}
		if (GMT_Encode_ID (API, string, object_ID) != GMT_OK) {
			Return (API->error);	/* Make filename with embedded object ID */
		}
		sprintf (buffer, "-R0/%g/0/%g -Jx1i -O -K -N -M -F+f+a+j %s", GMT->current.proj.rect[XHI], GMT->current.proj.rect[YHI], string);
		if (debug_print) fprintf (stderr, "%s\n", buffer);
		if (GMT_Call_Module (API, "pstext", GMT_MODULE_CMD, buffer) != GMT_OK) {	/* Plot paragraphs */
			Return (API->error);
		}
	}

	PSL_setorigin (PSL, -x_orig, -y_orig, 0.0, PSL_INV);	/* Reset */
	Ctrl->D.refpoint->x = x_orig;	Ctrl->D.refpoint->y = y_orig;

	GMT_map_basemap (GMT);
	GMT_plotend (GMT);

	if (D[FRONT] && GMT_Destroy_Data (API, &D[FRONT]) != GMT_OK) {
		Return (API->error);
	}
	
	for (id = 0; id < N_TXT; id++) {
		if (T[id] && GMT_Destroy_Data (API, &T[id]) != GMT_OK) {
			Return (API->error);
		}
	}
	
	GMT_Report (API, GMT_MSG_VERBOSE, "Done\n");

	Return (GMT_OK);
}
