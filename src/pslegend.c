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
 * Brief synopsis: pslegend will make map legends from input that specifies what will go
 * into the legend, such as headers, symbols with explanatory text,
 * paragraph text, and empty space and horizontal/vertical lines.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"pslegend"
#define THIS_MODULE_MODERN_NAME	"legend"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot a legend"
#define THIS_MODULE_KEYS	"<D{,>X}"
#define THIS_MODULE_NEEDS	"jr"
#define THIS_MODULE_OPTIONS "->BJKOPRUVXYpqt" GMT_OPT("c")

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
	struct PSLEGND_S {	/* -S<scale> */
		bool active;
		double scale;
	} S;
	struct PSLEGND_T {	/* -T<legendfile> */
		bool active;
		char *file;
	} T;
#ifdef DEBUG
	struct PSLEGND_DEBUG {	/* -; */
		bool active;
	} DBG;
#endif
};

struct PSLEGEND_TXT {
	char *text;
	bool string;	/* true if label or header string */
	struct GMT_FONT font;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSLEGEND_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSLEGEND_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->C.off[GMT_X] = C->C.off[GMT_Y] = GMT->session.u2u[GMT_PT][GMT_INCH] * GMT_FRAME_CLEARANCE;	/* 4 pt */
	C->D.spacing = 1.1;
	C->S.scale = 1.0;	/* Full given size of symbols */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSLEGEND_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_free_refpoint (GMT, &C->D.refpoint);
	gmt_M_free (GMT, C->F.panel);
	gmt_M_str_free (C->T.file);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the pslegend synopsis and optionally full usage information */

	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<specfile>] -D%s[+w<width>[/<height>]][+l<spacing>]%s [%s]\n", name, GMT_XYANCHOR, GMT_OFFSET, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-C<dx>[/<dy>]] [-F%s]\n", GMT_PANEL);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] %s%s%s[%s] [-S<scale>]\n", GMT_J_OPT, API->K_OPT, API->O_OPT, API->P_OPT, GMT_Rgeo_OPT);
	if (API->GMT->current.setting.run_mode == GMT_MODERN)
		GMT_Message (API, GMT_TIME_NONE, "\t[-T<file>] [%s] [%s] [%s]\n\t[%s]%s[%s]\n\t[%s] [%s] [%s]\n\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, API->c_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_t_OPT, GMT_PAR_OPT);
	else
		GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s]%s[%s]\n\t[%s] [%s] [%s]\n\n", GMT_U_OPT, GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT, API->c_OPT, GMT_p_OPT, GMT_qi_OPT, GMT_t_OPT, GMT_PAR_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\tReads legend layout specification from <specfile> [or stdin].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t(See module documentation for more information and <specfile> format).\n\n");

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	gmt_refpoint_syntax (API->GMT, "D", "Specify position and size of the legend rectangle", GMT_ANCHOR_LEGEND, 1);
	GMT_Message (API, GMT_TIME_NONE, "\t   Specify legend width with +w<width>; <height> is optional [estimated from <specfile>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If only codes A, C, D, G, H, L, and S are used the <width> is optional as well.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The remaining arguments are optional:\n");
	gmt_refpoint_syntax (API->GMT, "D", NULL, GMT_ANCHOR_LEGEND, 2);
	GMT_Message (API, GMT_TIME_NONE, "\t   +l sets the linespacing factor in units of the current annotation font size [1.1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<specfile> is one or more ASCII specification files with legend commands.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read.\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Set the clearance between legend frame and internal items [%gp].\n", GMT_FRAME_CLEARANCE);
	gmt_mappanel_syntax (API->GMT, 'F', "Specify a rectangular panel behind the legend", 2);
	GMT_Option (API, "J-,K");
	GMT_Option (API, "O,P,R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Scale all symbol sizes by <scale> [1].\n");
	if (API->GMT->current.setting.run_mode == GMT_MODERN)
		GMT_Message (API, GMT_TIME_NONE, "\t-T Write hidden legend specfile to <file>.\n");
	GMT_Option (API, "U,V,X,c,p,qi,t,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSLEGEND_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to pslegend and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0;
	int n;
	char xx[GMT_LEN256] = {""}, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""};
	char yy[GMT_LEN256] = {""}, txt_d[GMT_LEN256] = {""}, txt_e[GMT_LEN256] = {""}, string[GMT_LEN256] = {""};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Sets the clearance between frame and internal items */
				Ctrl->C.active = true;
				if ((n = gmt_get_pair (GMT, opt->arg, GMT_PAIR_DIM_DUP, Ctrl->C.off)) < 0) n_errors++;
				break;
			case 'D':	/* Sets position and size of legend */
				Ctrl->D.active = true;
				if (strlen (opt->arg) < 5 || strchr ("jgn", opt->arg[0]) || strstr (opt->arg, "+j") || strstr (opt->arg, "+l") || strstr (opt->arg, "+o") || strstr (opt->arg, "+w")) {	/* New syntax: 	*/
					if ((Ctrl->D.refpoint = gmt_get_refpoint (GMT, opt->arg, 'D')) == NULL) {
						n_errors++;	/* Failed basic parsing */
						continue;
					}
					/* Args are +w<width>[/<height>][+j<justify>][+l<spacing>][+o<dx>[/<dy>]] */
					if (gmt_validate_modifiers (GMT, Ctrl->D.refpoint->args, 'D', "jlow")) n_errors++;
					if (gmt_get_modifier (Ctrl->D.refpoint->args, 'j', string))
						Ctrl->D.justify = gmt_just_decode (GMT, string, PSL_NO_DEF);
					else	/* With -Dj or -DJ, set default to reference justify point, else BL */
						Ctrl->D.justify = gmt_M_just_default (GMT, Ctrl->D.refpoint, PSL_BL);
					if (gmt_get_modifier (Ctrl->D.refpoint->args, 'l', string)) {
						Ctrl->D.spacing = atof (string);
					}
					if (gmt_get_modifier (Ctrl->D.refpoint->args, 'o', string)) {
						if ((n = gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->D.off)) < 0) n_errors++;
					}
					if (gmt_get_modifier (Ctrl->D.refpoint->args, 'w', string)) {
						if ((n = gmt_get_pair (GMT, string, GMT_PAIR_DIM_NODUP, Ctrl->D.dim)) < 0) n_errors++;
					}
				}
				else {	/* Backwards handling of old syntax. Args are args are [x]<x>/<y>/<width>[/<height>][/<justify>][/<dx>/<dy>] */
					Ctrl->D.refpoint = gmt_M_memory (GMT, NULL, 1, struct GMT_REFPOINT);
					Ctrl->D.refpoint->mode = GMT_REFPOINT_PLOT;
					Ctrl->D.justify = PSL_TC;	/* Backwards compatible default justification */
					n = sscanf (opt->arg, "%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%[^/]/%s", xx, yy, txt_a, txt_b, txt_c, txt_d, txt_e);
					n_errors += gmt_M_check_condition (GMT, n < 3, "Old syntax is -D[x]<x0>/<y0>/<width>[/<height>][/<justify>][/<dx>/<dy>]\n");
					if (n_errors) break;
					if (xx[0] == 'x') {
						Ctrl->D.refpoint->x = gmt_M_to_inch (GMT, &xx[1]);
						Ctrl->D.refpoint->y = gmt_M_to_inch (GMT, yy);
					}
					else {	/* The equivalent of -Dg<lon>/<lat>... */
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_X), gmt_scanf (GMT, xx, gmt_M_type (GMT, GMT_IN, GMT_X), &Ctrl->D.refpoint->x), xx);
						n_errors += gmt_verify_expectations (GMT, gmt_M_type (GMT, GMT_IN, GMT_Y), gmt_scanf (GMT, yy, gmt_M_type (GMT, GMT_IN, GMT_Y), &Ctrl->D.refpoint->y), yy);
						Ctrl->D.refpoint->mode = GMT_REFPOINT_MAP;
					}
					Ctrl->D.dim[GMT_X] = gmt_M_to_inch (GMT, txt_a);	/* Width is always given */
					n -= 2;	/* Remove the x/y count */
					switch (n) {
						case 1: /* Only gave reference point and width; change default justify if -Dj */
							if (Ctrl->D.refpoint->mode == GMT_REFPOINT_JUST)	/* For -Dj with no 2nd justification, use same code as reference coordinate */
								Ctrl->D.justify = Ctrl->D.refpoint->justify;
							break;
						case 2:	/* Gave width and (height or justify) */
							if (strlen (txt_b) == 2 && strchr ("LMRBCT", txt_b[GMT_X]) && strchr ("LMRBCT", txt_b[GMT_Y]))	/* Gave a 2-char justification code */
								Ctrl->D.justify = gmt_just_decode (GMT, txt_b, PSL_NO_DEF);
							else /* Got height */
								Ctrl->D.dim[GMT_Y] = gmt_M_to_inch (GMT, txt_b);
							break;
						case 3:	/* Gave width and (height and justify) or (dx/dy) */
							if (strlen (txt_c) == 2 && strchr ("LMRBCT", txt_c[GMT_X]) && strchr ("LMRBCT", txt_c[GMT_Y])) {	/* Gave a 2-char justification code */
								Ctrl->D.dim[GMT_Y] = gmt_M_to_inch (GMT, txt_b);
								Ctrl->D.justify = gmt_just_decode (GMT, txt_c, PSL_NO_DEF);
							}
							else {	/* Just got offsets */
								Ctrl->D.off[GMT_X] = gmt_M_to_inch (GMT, txt_b);
								Ctrl->D.off[GMT_Y] = gmt_M_to_inch (GMT, txt_c);
							}
							break;
						case 4:	/* Gave width and (height or justify) and dx/dy */
							if (strlen (txt_b) == 2 && strchr ("LMRBCT", txt_b[GMT_X]) && strchr ("LMRBCT", txt_b[GMT_Y]))	/* Gave a 2-char justification code */
								Ctrl->D.justify = gmt_just_decode (GMT, txt_b, PSL_NO_DEF);
							else
								Ctrl->D.dim[GMT_Y] = gmt_M_to_inch (GMT, txt_b);
							Ctrl->D.off[GMT_X] = gmt_M_to_inch (GMT, txt_c);
							Ctrl->D.off[GMT_Y] = gmt_M_to_inch (GMT, txt_d);
							break;
						case 5:	/* Got them all */
							Ctrl->D.dim[GMT_Y] = gmt_M_to_inch (GMT, txt_b);
							Ctrl->D.justify = gmt_just_decode (GMT, txt_c, PSL_NO_DEF);
							Ctrl->D.off[GMT_X] = gmt_M_to_inch (GMT, txt_d);
							Ctrl->D.off[GMT_Y] = gmt_M_to_inch (GMT, txt_e);
							break;
					}
				}
				break;
			case 'F':
				Ctrl->F.active = true;
				if (gmt_getpanel (GMT, opt->option, opt->arg, &(Ctrl->F.panel))) {
					gmt_mappanel_syntax (GMT, 'F', "Specify a rectangular panel behind the legend", 2);
					n_errors++;
				}
				Ctrl->F.debug = Ctrl->F.panel->debug;	/* Hidden +d processing; this may go away */
				if (gmt_M_compat_check (GMT, 4) && !opt->arg[0]) Ctrl->F.panel->mode |= GMT_PANEL_OUTLINE;	/* Draw frame if just -F is given if in compatibility mode */
				if (!Ctrl->F.panel->clearance) gmt_M_memset (Ctrl->F.panel->padding, 4, double);	/* No clearance is default since handled via -C */
				break;
			case 'G':	/* Inside legend box fill [OBSOLETE] */
				if (gmt_M_compat_check (GMT, 4)) {
					char tmparg[GMT_LEN32] = {""};
					GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Option -G is deprecated; -F...+g%s was set instead, use this in the future.\n", opt->arg);
					Ctrl->F.active = true;
					sprintf (tmparg, "+g%s", opt->arg);
					if (gmt_getpanel (GMT, opt->option, tmparg, &(Ctrl->F.panel))) {
						gmt_mappanel_syntax (GMT, 'F', "Specify a rectangular panel behind the legend", 2);
						n_errors++;
					}
					Ctrl->F.panel->mode |= GMT_PANEL_FILL;
				}
				else
					n_errors += gmt_default_error (GMT, opt->option);
				break;
			case 'L':	/* Sets linespacing in units of fontsize [1.1] */
				GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Option -L is deprecated; -D...+l%s was set instead, use this in the future.\n", opt->arg);
				Ctrl->D.spacing = atof (opt->arg);
				break;

			case 'S':	/* Sets common symbol scale factor [1] */
				Ctrl->S.scale = atof (opt->arg);
				break;

			case 'T':	/* Sets legendfile for saving the hidden file */
				Ctrl->T.active = true;
				if (opt->arg[0])
					Ctrl->T.file = strdup (opt->arg);
				else {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -T requires a filename\n");
					n_errors++;
				}
				break;

#ifdef DEBUG
			case '+':	/* Dump temp files */
				Ctrl->DBG.active = true;
				break;
#endif

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	n_errors += gmt_M_check_condition (GMT, Ctrl->C.off[GMT_X] < 0.0 || Ctrl->C.off[GMT_Y] < 0.0, "Option -C: clearances cannot be negative!\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->D.active, "The -D option is required!\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.scale <= 0.0, "The -S option cannot set a zero scale!\n");

	if (!Ctrl->D.refpoint) return (GMT_PARSE_ERROR);	/* Need to exit because next ones to not apply */

	n_errors += gmt_M_check_condition (GMT, Ctrl->D.dim[GMT_Y] < 0.0, "Option -D: legend box height cannot be negative!\n");
	if (Ctrl->D.refpoint->mode != GMT_REFPOINT_PLOT) {	/* Anything other than -Dx need -R -J; other cases don't */
		static char *kind = "gjJnx";	/* The five types of refpoint specifications */
		n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Option -D%c requires the -R option\n", kind[Ctrl->D.refpoint->mode]);
		n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Option -D%c requires the -J option\n", kind[Ctrl->D.refpoint->mode]);
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

/* Used to draw the current y-line for debug purposes only. */
GMT_LOCAL void drawbase (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x0, double x1, double y0) {
	struct GMT_PEN faint_pen;
	gmt_init_pen (GMT, &faint_pen, 0.0);
	gmt_setpen (GMT, &faint_pen);
	PSL_plotsegment (PSL, x0, y0, x1, y0);
}

/* Used to fill the cells in the the current y-line. */
GMT_LOCAL void fillcell (struct GMT_CTRL *GMT, double x0, double y0, double y1, double xoff[], double *d_gap, unsigned int n_cols, char *fill[]) {
	unsigned int col;
	double dim[2];
	struct GMT_FILL F;
	dim[1] = y1 - y0 + *d_gap;
	y0 = 0.5 * (y0 + y1 + *d_gap);	/* Recycle y0 to mean mid level */
	for (col = 0; col < n_cols; col++) {
		if (!fill[col]) continue;	/* No fill for this cell */
		if (gmt_getfill (GMT, fill[col], &F)) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to interpret %s as a valid fill, skipped\n", fill[col]);
			continue;
		}
		gmt_setfill (GMT, &F, 0);
		dim[0] = xoff[col+1] - xoff[col];
		PSL_plotsymbol (GMT->PSL, x0 + 0.5 * (xoff[col+1] + xoff[col]), y0, dim, PSL_RECT);
	}
	*d_gap = 0.0;	/* Reset any "gap after D operator" once we use it */
}

GMT_LOCAL struct GMT_DATASET *get_dataset_pointer (struct GMTAPI_CTRL *API, struct GMT_DATASET *Din, unsigned int geometry, uint64_t n_segments, uint64_t n_rows, uint64_t n_cols, bool text) {
	uint64_t seg, dim[GMT_DIM_SIZE] = {1, n_segments, n_rows, n_cols};	/* We will a 1 or 2-row data set for up to n_segments segments; allocate just once */
	unsigned int mode = (text) ? GMT_WITH_STRINGS : 0;
	struct GMT_DATASET *D = NULL;
	if (Din) return Din;	/* Already done this */
	if (D == NULL && (D = GMT_Create_Data (API, GMT_IS_DATASET, geometry, mode, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to create a data set for pslegend\n");
		return (NULL);
	}
	/* Initialize counters to zero */
	D->n_records = D->table[0]->n_records = 0;
	for (seg = 0; seg < n_segments; seg++)
		D->table[0]->segment[seg]->n_rows = 0;
	return (D);
}

GMT_LOCAL struct GMT_DATASEGMENT * get_segment (struct GMT_DATASET **D, unsigned id, uint64_t seg) {
	return D[id]->table[0]->segment[seg];	/* Get next segment from first table */
}

GMT_LOCAL void maybe_realloc_segment (struct GMT_CTRL *GMT, struct GMT_DATASEGMENT *S) {
	struct GMT_DATASEGMENT_HIDDEN *SH = gmt_get_DS_hidden (S);
	if (S->n_rows < SH->n_alloc) return;	/* Not yet */
	SH->n_alloc += GMT_SMALL_CHUNK;
	if (S->n_columns) {	/* Numerical data */
		uint64_t col;
		for (col = 0; col < S->n_columns; col++)
			S->data[col] = gmt_M_memory (GMT, S->data[col], SH->n_alloc, double);
	}
	if (S->text) S->text = gmt_M_memory (GMT, S->text, SH->n_alloc, char *);
}

GMT_LOCAL double get_image_aspect (struct GMTAPI_CTRL *API, char *file) {
	double aspect;
	struct GMT_IMAGE *I = NULL;
	if (strstr (file, ".eps") || strstr (file, ".ps") || strstr (file, ".epsi") || strstr (file, ".epsf")) {	/* EPS file */
		struct imageinfo h;
		if (PSL_loadeps (API->GMT->PSL, file, &h, NULL)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to read EPS file %s, no pattern set\n", file);
			return (-1.0);
		}
		aspect = (double)h.height / (double)h.width;
		return aspect;
	}
	if ((I = GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_ONLY, NULL, file, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to read image %s, no pattern set\n", file);
		return (-1.0);
	}
	aspect = (double)I->header->n_rows / (double)I->header->n_columns;
	GMT_Destroy_Data (API, &I);
	return aspect;
}

GMT_LOCAL bool new_fontsyntax (struct GMT_CTRL *GMT, char *word1, char *word2) {
	/* Old syntax expect fontsize and font to be given as two items, while new (GMT5)
	 * syntax expects fontsize,fontname,fontcolor to be a single item with optional,
	 * comma-separated parts.  This function determines what we were given... */
	bool new;
	if (!strcmp (word1, "-") && !strcmp (word2, "-")) new = false;	/* Gave - for both size and font defaults means old syntax */
	else if (strchr (word1, ',')) new = true;			/* Got a comma-separated list of font attributes */
	else if (!strcmp (word1, "-")) new = (gmt_getfonttype (GMT, word2) == -1);	/* Detect old syntax if word1 is - and word2 is a font name or integer */
	else if (!gmt_not_numeric (GMT, word1) && !strcmp (word2, "-")) new = false;	/* Detect old syntax if word1 is a size and word2 - for default font */
	else if (!gmt_not_numeric (GMT, word1) && gmt_getfonttype (GMT, word2) >= 0) new = false;	/* Detect old syntax if word1 is a size and word2 is a font name or integer */
	else if (gmt_not_numeric (GMT, word2)) new = true;		/* Must be start of the main text */
	else new = true;	/* Must assume current syntax */
	if (!new && gmt_M_compat_check (GMT, 4))
		GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Your GMT4 font specification [%s %s] is deprecated; use [<size>][,<name>][,<fill>][=<pen>] in the future.\n", word1, word2);
	return new;
}

/* Define the fraction of the height of the font to the font size */
#define FONT_HEIGHT_PRIMARY (GMT->session.font[GMT->current.setting.font_annot[GMT_PRIMARY].id].height)
#define FONT_HEIGHT(font_id) (GMT->session.font[font_id].height)
#define FONT_HEIGHT_LABEL (GMT->session.font[GMT->current.setting.font_label.id].height)

#define SYM 	0
#define FRONT	1
#define QLINE	2
#define TXT	3
#define PAR	4
#define N_DAT	5

#define PSLEGEND_MAX_COLS	100

int GMT_legend (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: legend\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_pslegend (V_API, mode, args);
}

int GMT_pslegend (void *V_API, int mode, void *args) {
	/* High-level function that implements the pslegend task */
	unsigned int tbl, pos, first = 0, ID, n_item = 0;
	int i, justify = 0, n = 0, n_columns = 1, n_col, col, error = 0, column_number = 0, id, n_scan, status = 0, max_cols = 0;
	bool flush_paragraph = false, v_line_draw_now = false, gave_label, gave_mapscale_options, did_old = false;
	bool drawn = false, b_cpt = false, C_is_active = false, do_width = false, in_PS_ok = true;
	uint64_t seg, row, n_fronts = 0, n_quoted_lines = 0, n_symbols = 0, n_par_lines = 0, n_par_total = 0, krow[N_DAT];
	int64_t n_para = -1;
	size_t n_char = 0;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, txt_c[GMT_LEN256] = {""}, txt_d[GMT_LEN256] = {""};
	char txt_e[GMT_LEN256] = {""}, txt_f[GMT_LEN256] = {""}, key[GMT_LEN256] = {""}, sub[GMT_LEN256] = {""}, just;
	char tmp[GMT_LEN256] = {""}, symbol[GMT_LEN256] = {""}, text[GMT_BUFSIZ] = {""}, image[GMT_BUFSIZ] = {""}, xx[GMT_LEN256] = {""};
	char yy[GMT_LEN256] = {""}, size[GMT_LEN256] = {""}, angle[GMT_LEN256] = {""}, mapscale[GMT_LEN256] = {""};
	char font[GMT_LEN256] = {""}, lspace[GMT_LEN256] = {""}, tw[GMT_LEN256] = {""}, jj[GMT_LEN256] = {""};
	char bar_cpt[GMT_LEN256] = {""}, bar_gap[GMT_LEN256] = {""}, bar_height[GMT_LEN256] = {""}, bar_modifiers[GMT_LEN256] = {""};
	char module_options[GMT_LEN256] = {""}, r_options[GMT_LEN256] = {""}, xy_mode[3] = {""};
	char txtcolor[GMT_LEN256] = {""}, def_txtcolor[GMT_LEN256] = {""}, buffer[GMT_BUFSIZ] = {""}, A[GMT_LEN32] = {""}, legend_file[PATH_MAX] = {""};
	char path[PATH_MAX] = {""}, B[GMT_LEN32] = {""}, C[GMT_LEN32] = {""}, p[GMT_LEN256] = {""};
	char *plot_points[2] = {"psxy", "plot"}, *plot_text[2] = {"pstext", "text"};
	char *line = NULL, string[GMT_VF_LEN] = {""}, *c = NULL, *fill[PSLEGEND_MAX_COLS];
#ifdef DEBUG
	char *dname[N_DAT] = {"symbol", "front", "qline", "textline", "partext"};
#endif

	double x_orig, y_orig, x_off, x, y, r, col_left_x, row_base_y, dx, d_line_half_width, d_line_hor_offset, off_ss, off_tt, def_dx2 = 0.0;
	double v_line_ver_offset = 0.0, height, az1, az2, m_az, row_height, scl, aspect, xy_offset[2], C_rgb[4] = {0.0, 0.0, 0.0, 0.0};
	double half_line_spacing, quarter_line_spacing, one_line_spacing, v_line_y_start = 0.0, d_off, def_size = 0.0, shrink[4] = {0.0, 0.0, 0.0, 0.0};
	double sum_width, h, gap, d_line_after_gap = 0.0, d_line_last_y0 = 0.0, col_width[PSLEGEND_MAX_COLS], x_off_col[PSLEGEND_MAX_COLS];

	struct imageinfo header;
	struct PSLEGEND_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL, *opt = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMT_FONT ifont;
	struct GMT_PEN current_pen;
	struct PSLEGEND_TXT *legend_item = NULL;
	struct GMT_DATASET *In = NULL;
	struct GMT_DATASET *D[N_DAT];
	struct GMT_DATASEGMENT *S[N_DAT];
	struct GMT_DATASEGMENT_HIDDEN *SH = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	gmt_M_memset (&header, 1, struct imageinfo);	/* initialize struct */
	gmt_M_memset (fill, PSLEGEND_MAX_COLS, char *);	/* initialize array */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments; return if errors are encountered */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the pslegend main code ----------------------------*/

	gmt_M_memset (D, N_DAT, struct GMT_DATASET *);		/* Set these arrays to NULL */
	gmt_M_memset (S, N_DAT, struct GMT_DATASEGMENT *);
	gmt_M_memset (krow, N_DAT, uint64_t);

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input text table data\n");
	if (gmt_M_compat_check (GMT, 4)) {
		/* Since pslegend v4 used '>' to indicate a paragraph record we avoid confusion with multiple segment-headers by *
		 * temporarily setting # as segment header flag since all headers are skipped anyway */
		GMT->current.setting.io_seg_marker[GMT_IN] = '#';
	}

	if (gmt_legend_file (API, legend_file) == 1) {	/* Running modern mode and we have a hidden legend file to read */
		GMT_Report (API, GMT_MSG_INFORMATION, "Processing hidden legend specfile %s\n", legend_file);
		if ((In = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_TEXT, GMT_READ_NORMAL, NULL, legend_file, NULL)) == NULL) {
			Return (API->error);
		}
		if (Ctrl->T.active && GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_TEXT, 0, NULL, Ctrl->T.file, In) != GMT_NOERROR) {
			Return (API->error);
		}
	}
	else {	/* Possibly register stdin and/or read specified input file(s) */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register data input */
			Return (API->error);
		}
		if ((In = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
			Return (API->error);
		}
	}

	ID = GMT->current.setting.run_mode;	/* Use as index to arrays with correct module names for classic [0] or modern [1] */

	if (Ctrl->D.dim[GMT_X] == 0.0) {	/* Compute legend width */
		legend_item = gmt_M_memory (GMT, NULL, In->n_records, struct PSLEGEND_TXT);	/* Array to hold all labels */
		do_width = true;
	}

	/* First attempt to compute the legend height */

	one_line_spacing = Ctrl->D.spacing * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;
	half_line_spacing    = 0.5  * one_line_spacing;
	quarter_line_spacing = 0.25 * one_line_spacing;

	height = 2.0 * Ctrl->C.off[GMT_Y];
	for (tbl = 0; tbl < In->n_tables; tbl++) {	/* We only expect one table but who knows what the user does */
		for (seg = 0; seg < In->table[tbl]->n_segments; seg++) {	/* We only expect one segment in each table but again... */
			for (row = 0; row < In->table[tbl]->segment[seg]->n_rows; row++) {	/* Finally processing the rows */
				line = In->table[tbl]->segment[seg]->text[row];
				if (line[0] == GMT->current.setting.io_head_marker[GMT_IN] || gmt_is_a_blank_line (line)) continue;	/* Skip all headers or blank lines  */

				/* Data record to process */

				if (line[0] != 'T' && flush_paragraph) {	/* Flush contents of pending paragraph [Call GMT_text] */
					flush_paragraph = false;
					column_number = 0;
				}

				switch (line[0]) {
					case 'B':	/* Color scale Bar [Use GMT_psscale] */
						/* B cptname offset height[+modifiers] [ Optional psscale args -B -I -L -M -N -S -Z -p ] */
						sscanf (&line[2], "%*s %*s %s", bar_height);
						if ((c = strchr (bar_height, '+')) != NULL) c[0] = 0;	/* Chop off any modifiers so we can compute the height */
						height += gmt_M_to_inch (GMT, bar_height) + GMT->current.setting.map_tick_length[0] + GMT->current.setting.map_annot_offset[0] + FONT_HEIGHT_PRIMARY * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;
						column_number = 0;
						if (strstr (&line[2], "-B")) b_cpt = true;	/* Passed -B options with the bar presecription */
						in_PS_ok = false;
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
						height += (txt_a[strlen(txt_a)-1] == 'l') ? atoi (txt_a) * one_line_spacing : gmt_M_to_inch (GMT, txt_a);
						column_number = 0;
						break;

					case 'H':	/* Header record */
						sscanf (&line[2], "%s %s %[^\n]", size, font, text);
						if (new_fontsyntax (GMT, size, font)) {	/* GMT5 font specification */
							sscanf (&line[2], "%s %[^\n]", font, text);
							if (font[0] == '-')
								sprintf (tmp, "%s", gmt_putfont (GMT, &GMT->current.setting.font_title));
							else
								strcpy (tmp, font);	/* Gave a font specification */
						}
						else {	/* Old GMT4 syntax for fontsize and font */
							if (size[0] == '-') size[0] = 0;
							if (font[0] == '-') font[0] = 0;
							sprintf (tmp, "%s,%s", size, font);	/* Put size, font together for parsing by gmt_getfont */
						}
						ifont = GMT->current.setting.font_title;	/* Set default font */
						gmt_getfont (GMT, tmp, &ifont);
						height += Ctrl->D.spacing * ifont.size / PSL_POINTS_PER_INCH;
						column_number = 0;
						if (do_width) {
							gmt_M_memcpy (&legend_item[n_item].font, &ifont, 1, struct GMT_FONT);
							legend_item[n_item].string = true;
							legend_item[n_item++].text = strdup (text);
						}
						break;

					case 'I':	/* Image record [use GMT_psimage] */
						sscanf (&line[2], "%s %s %s", image, size, key);
						first = gmt_download_file_if_not_found (GMT, image, GMT_CACHE_DIR);
						if (gmt_getdatapath (GMT, &image[first], path, R_OK) == NULL) {
							GMT_Report (API, GMT_MSG_ERROR, "Cannot find/open file %s.\n", &image[first]);
							continue;
						}
						if ((aspect = get_image_aspect (API, path)) < 0.0) {
							GMT_Report (API, GMT_MSG_ERROR, "Trouble reading %s! - Skipping.\n", &image[first]);
							continue;
						}
						height += gmt_M_to_inch (GMT, size) * aspect;
						column_number = 0;
						in_PS_ok = false;
						break;

					case 'L':	/* Label record */
						sscanf (&line[2], "%s %s %s %[^\n]", size, font, key, text);
						if (new_fontsyntax (GMT, size, font)) {	/* GMT5 font specification */
							sscanf (&line[2], "%s %s %[^\n]", font, key, text);
							if (font[0] == '-')	/* Want default font */
								sprintf (tmp, "%s", gmt_putfont (GMT, &GMT->current.setting.font_label));
							else
								strcpy (tmp, font);	/* Gave a font specification */
						}
						else {	/* Old GMT4 syntax for fontsize and font */
							if (size[0] == '-') size[0] = 0;
							if (font[0] == '-') font[0] = 0;
							sprintf (tmp, "%s,%s", size, font);	/* Put size, font together for parsing by gmt_getfont */
						}
						ifont = GMT->current.setting.font_label;	/* Set default font */
						gmt_getfont (GMT, tmp, &ifont);
						if (column_number%n_columns == 0) {
							height += Ctrl->D.spacing * ifont.size / PSL_POINTS_PER_INCH;
							column_number = 0;
						}
						column_number++;
						if (do_width) {
							gmt_M_memcpy (&legend_item[n_item].font, &ifont, 1, struct GMT_FONT);
							legend_item[n_item].string = true;
							legend_item[n_item++].text = strdup (text);
						}
						break;

					case 'M':	/* Map scale record M lon0|- lat0 length[n|m|k][+opts] f|p  [-R -J] */
						sscanf (&line[2], "%s %s %s %s %s %s", txt_a, txt_b, txt_c, txt_d, txt_e, txt_f);
						for (i = 0, gave_mapscale_options = false; txt_c[i] && !gave_mapscale_options; i++) if (txt_c[i] == '+') gave_mapscale_options = true;
						/* Default assumes label is added on top */
						just = 't';
						gave_label = true;
						d_off = FONT_HEIGHT_LABEL * GMT->current.setting.font_label.size / PSL_POINTS_PER_INCH + fabs(GMT->current.setting.map_label_offset);
						if ((txt_d[0] == 'f' || txt_d[0] == 'p') && gmt_get_modifier (txt_c, 'j', string))	/* Specified alternate justification old-style */
							just = string[0];
						else if (gmt_get_modifier (txt_c, 'a', string))	/* Specified alternate alignment */
							just = string[0];
						if (gmt_get_modifier (txt_c, 'u', string))	/* Specified alternate alignment */
							gave_label = false;	/* Not sure why I do this, will find out */
						if (gave_label && (just == 't' || just == 'b')) height += d_off;
						height += GMT->current.setting.map_scale_height + FONT_HEIGHT_PRIMARY * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH + GMT->current.setting.map_annot_offset[0];
						column_number = 0;
						in_PS_ok = false;
						break;

					case 'N':	/* n_columns or column width record */
						pos = n_columns = 0;
						while ((gmt_strtok (&line[2], " \t", &pos, p))) {
							n_columns++;
							if (n_columns == PSLEGEND_MAX_COLS) {
								GMT_Report (API, GMT_MSG_ERROR, "Exceeding maximum columns (%d) in N operator\n", PSLEGEND_MAX_COLS);
								Return (GMT_RUNTIME_ERROR);
							}
						}
						if (n_columns == 0) n_columns = 1;	/* Default to 1 if nothing is given */
						/* Check if user gave just the number of columns */
						if (n_columns == 1 && (pos = atoi (&line[2])) > 1) n_columns = pos;
						if (n_columns > max_cols) max_cols = n_columns;
						column_number = 0;
						break;

					case '>':	/* Paragraph text header */
						if (gmt_M_compat_check (GMT, 4)) /* Warn and fall through on purpose */
							GMT_Report (API, GMT_MSG_COMPAT, "Paragraph text header flag > is deprecated; use P instead\n");
						else {
							GMT_Report (API, GMT_MSG_ERROR, "Unrecognized record (%s)\n", line);
							Return (GMT_RUNTIME_ERROR);
							break;
						}
						/* Intentionally fall through */
					case 'P':	/* Paragraph text header */
						flush_paragraph = true;
						column_number = 0;
						n_par_total++;
						in_PS_ok = false;
						break;

					case 'S':	/* Symbol record: S [dx1 symbol size fill pen [ dx2 text ]] */
						if (column_number%n_columns == 0) {
							height += one_line_spacing;
							column_number = 0;
						}
						column_number++;
						text[0] = '\0';
						n_scan = sscanf (line, "%*s %*s %*s %s %*s %*s %s %[^\n]", size, txt_b, text);
						/* Find the largest symbol size specified */
						if ((c = strrchr (size, '/')))	/* Front, use the last arg as size since closest to height */
							x = gmt_M_to_inch (GMT, &c[1]);
						else
							x = (strcmp (size, "-")) ? gmt_M_to_inch (GMT, size) : 0.0;
						if (x > def_size) def_size = x;
						if (n_scan > 1 && strcmp (txt_b, "-")) {
							x = gmt_M_to_inch (GMT, txt_b);
							if (x > def_dx2) def_dx2 = x;
						}
						if (do_width && n_scan == 3 && strlen (text)) {
							gmt_M_memcpy (&legend_item[n_item].font, &(GMT->current.setting.font_annot[GMT_PRIMARY]), 1, struct GMT_FONT);
							legend_item[n_item++].text = strdup (text);
						}
						break;

					case 'T':	/* paragraph text record */
						n_char += strlen (line) - 2;
						if (!flush_paragraph) n_par_total++;	/* paragraph text without leading header provided */
						flush_paragraph = true;
						column_number = 0;
						n_par_lines++;
						in_PS_ok = false;
						break;

					case 'V':	/* Vertical line from here to next V */
						column_number = 0;
						break;

					default:
						GMT_Report (API, GMT_MSG_ERROR, "Unrecognized record (%s)\n", line);
						Return (GMT_RUNTIME_ERROR);
					break;
				}
			}
		}
	}

	if (do_width) {
		if (max_cols > 1) {
			GMT_Report (API, GMT_MSG_ERROR, "Must specify -D...+w<width> if more than one symbol column (N = %d)\n", max_cols);
			Return (GMT_RUNTIME_ERROR);
		}
		if (!in_PS_ok) {
			GMT_Report (API, GMT_MSG_ERROR, "Must specify -D...+w<width> if codes other than D, H, L, S, V are used\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (def_size == 0.0)	/* No sizes specified in input file; default to 0.5 cm */
		def_size = 0.5 / 2.54;	/* In inches */
	if (def_dx2 == 0.0)	/* No dist to text label given; default to 2x default symbol size */
		def_dx2 = Ctrl->S.scale * GMT_LEGEND_DX2_MUL * def_size;	/* In inches */
	GMT_Report (API, GMT_MSG_DEBUG, "Default symbol size = %g and default distance to text label is %g\n", def_size, def_dx2);

	if (n_char) {	/* Typesetting paragraphs, make a guesstimate of number of typeset lines */
		int n_lines;
		double average_char_width = 0.44;	/* There is no such thing but this is just a 1st order estimate */
		double x_lines;
		/* Guess: Given legend width and approximate char width, do the simple expression */
		x_lines = n_char * (average_char_width * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH) / ((Ctrl->D.dim[GMT_X] - 2 * Ctrl->C.off[GMT_X]));
		n_lines = irint (ceil (x_lines));
		height += n_lines * Ctrl->D.spacing * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;
		GMT_Report (API, GMT_MSG_DEBUG, "Estimating %d lines of typeset paragraph text [%.1f].\n", n_lines, x_lines);
	}

	scl = gmt_convert_units (GMT, "1", GMT_INCH, GMT->current.setting.proj_length_unit);
	if (Ctrl->D.dim[GMT_Y] == 0.0) {	/* Use the computed height */
		Ctrl->D.dim[GMT_Y] = height;
		GMT_Report (API, GMT_MSG_INFORMATION, "Legend height not given, using estimated height of %g %s.\n", scl*height,
			GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	}
	else
		GMT_Report (API, GMT_MSG_INFORMATION, "Legend height given as %g %s; estimated height is %g %s.\n",
		            scl*Ctrl->D.dim[GMT_Y], GMT->session.unit_name[GMT->current.setting.proj_length_unit],
		            scl*height, GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
	if (do_width) Ctrl->D.dim[GMT_X] = Ctrl->D.dim[GMT_Y];	/* Temporarily needed in gmt_map_setup */

	if (!(GMT->common.R.active[RSET] && GMT->common.J.active)) {	/* When no projection specified (i.e, -Dx is used), use fake linear projection -Jx1i */
		double wesn[4];
		gmt_M_memset (wesn, 4, double);
		GMT->common.R.active[RSET] = true;
		GMT->common.J.active = false;
		gmt_parse_common_options (GMT, "J", 'J', "x1i");
		wesn[XHI] = Ctrl->D.dim[GMT_X];	wesn[YHI] = Ctrl->D.dim[GMT_Y];
		if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, wesn), "")) Return (GMT_PROJECTION_ERROR);
		if (GMT->common.B.active[GMT_PRIMARY] || GMT->common.B.active[GMT_SECONDARY]) {	/* Cannot use -B if no -R -J */
			GMT->common.B.active[GMT_PRIMARY] = GMT->common.B.active[GMT_SECONDARY] = false;
			GMT_Report (API, GMT_MSG_INFORMATION, "Disabling your -B option since -R -J were not set\n");
		}
	}
	else if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), ""))
		Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);

	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	gmt_map_basemap (GMT);	/* Plot basemap if requested */

	if (GMT->current.map.frame.draw && b_cpt)	/* Two conflicting -B settings, reset main -B since we just finished the frame */
		gmt_M_memset (&(GMT->current.map.frame), 1, struct GMT_PLOT_FRAME);

	/* Must save status of -X -Y as passed to PSL since we must undo at the end of pslegend */
	gmt_M_memcpy (xy_mode, PSL->internal.origin, 2, char);
	gmt_M_memcpy (xy_offset, PSL->internal.offset, 2, double);

	if ((opt = GMT_Find_Option (API, 'X', options)) != NULL)
		GMT_Delete_Option (API, opt, &options);
	if ((opt = GMT_Find_Option (API, 'Y', options)) != NULL)
		GMT_Delete_Option (API, opt, &options);

	if (do_width && n_item) {	/* Determine max text width in PostScript */
		PSL_comment (PSL, "Determine maximum width of one or more strings\n");
		PSL_comment (PSL, "Initialize to zero\n");
		PSL_defunits (PSL, "PSL_legend_label_width", 0.0);
		PSL_defunits (PSL, "PSL_legend_string_width", 0.0);
		for (row = 0; row < n_item; row++) {
			PSL_comment (PSL, "Set font and get width of: %s\n", legend_item[row].text);
			PSL_setfont (PSL, legend_item[row].font.id);
			PSL_deftextdim (PSL, "PSL_tmp", legend_item[row].font.size, legend_item[row].text);
			gmt_M_str_free (legend_item[row].text);
			PSL_comment (PSL, "Update max if exceeded\n");
			if (legend_item[row].string)
				PSL_command (PSL, "PSL_tmp_w PSL_legend_string_width gt { /PSL_legend_string_width PSL_tmp_w def } if\n");
			else
				PSL_command (PSL, "PSL_tmp_w PSL_legend_label_width gt { /PSL_legend_label_width PSL_tmp_w def } if\n");
		}
		PSL_comment (PSL, "Symbol labels start at PSL_tmp_w\n");
		PSL_defunits (PSL, "PSL_tmp_w", def_dx2);
		PSL_defunits (PSL, "PSL_legend_clear_x", 2.0 * Ctrl->C.off[GMT_X]);
		PSL_comment (PSL, "Determine largest possible width\n");
		PSL_command (PSL, "/PSL_tmp_w PSL_tmp_w PSL_legend_label_width add def\n");
		PSL_command (PSL, "/PSL_legend_box_width PSL_tmp_w PSL_legend_string_width gt { PSL_tmp_w } { PSL_legend_string_width} ifelse PSL_legend_clear_x add def\n");
		gmt_M_free (GMT, legend_item);
	}
	else {	/* Hardwired width */
		PSL_defunits (PSL, "PSL_legend_box_width", Ctrl->D.dim[GMT_X]);
	}
	PSL_defunits (PSL, "PSL_legend_box_height", Ctrl->D.dim[GMT_Y]);

	gmt_set_refpoint (GMT, Ctrl->D.refpoint);	/* Finalize reference point plot coordinates, if needed */

	/* Allow for justification and offset so that the reference point is the plot location of the bottom left corner of box */

	gmt_adjust_refpoint (GMT, Ctrl->D.refpoint, Ctrl->D.dim, Ctrl->D.off, Ctrl->D.justify, PSL_BL);

	if (GMT->current.setting.run_mode == GMT_MODERN) {
		struct GMT_SUBPLOT *SP = &(GMT->current.plot.panel);
		if (SP->active)	/* Subplot panel mode is in effect; we must undo any of these gaps here */
			gmt_M_memcpy (shrink, SP->gap, 4U, double);
	}

	/* Set new origin */

	PSL_setorigin (PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->y, 0.0, PSL_FWD);
	x_orig = Ctrl->D.refpoint->x;	y_orig = Ctrl->D.refpoint->y;
	Ctrl->D.refpoint->x = Ctrl->D.refpoint->y = 0.0;	/* For now */

	current_pen = GMT->current.setting.map_default_pen;

	/* We use a standard x/y inch coordinate system here, unlike old pslegend. */

	if (do_width)	/* Adjust for mismatch between width and actual width */
		PSL_command (PSL, "PSL_legend_box_width PSL_legend_box_height sub %d 2 div mul neg 0 translate\n", (Ctrl->D.justify%4)-1);
	if (Ctrl->F.active) {	/* First place legend frame fill */
		if (do_width) {	/* Doing it all via PostScript settings */
			if (Ctrl->F.panel->mode & GMT_PANEL_ROUNDED)
				GMT_Report (API, GMT_MSG_WARNING, "Disabling your -F... +r modifier since not implemented yet for automatic width setting\n");
			PSL_command (PSL, "V\n");
			if (Ctrl->F.panel->mode & GMT_PANEL_SHADOW) {	/* Draw offset background shadow first */
				gmt_setfill (GMT, &Ctrl->F.panel->sfill, 0);	/* The shadow has no outline */
				PSL_setcurrentpoint (PSL, Ctrl->F.panel->off[GMT_X], Ctrl->F.panel->off[GMT_Y]);
				PSL_command (PSL, "PSL_legend_box_width 0 D 0 PSL_legend_box_height D PSL_legend_box_width neg 0 D P FO N\n");
			}
			if (Ctrl->F.panel->mode & GMT_PANEL_OUTLINE) gmt_setpen (GMT, &Ctrl->F.panel->pen1);
			gmt_setfill (GMT, & Ctrl->F.panel->fill, (Ctrl->F.panel->mode & GMT_PANEL_OUTLINE) > 0);
			PSL_command (PSL, "0 0 M PSL_legend_box_width 0 D 0 PSL_legend_box_height D PSL_legend_box_width neg 0 D P FO N\n");
			if (Ctrl->F.panel->mode & GMT_PANEL_INNER) {	/* Also draw secondary frame on the inside */
				PSL_defunits (PSL, "PSL_legend_box_shrink", 2.0 * Ctrl->F.panel->gap);
				gmt_setpen (GMT, &Ctrl->F.panel->pen2);	/* Set inner border pen */
				gmt_setfill (GMT, NULL, 1);	/* Never fill for inner frame */
				PSL_setcurrentpoint (PSL, Ctrl->F.panel->gap, Ctrl->F.panel->gap);
				PSL_command (PSL, "PSL_legend_box_width PSL_legend_box_shrink sub 0 D 0 PSL_legend_box_height PSL_legend_box_shrink sub D PSL_legend_box_width PSL_legend_box_shrink sub neg 0 D P FO N\n");
			}
			PSL_command (PSL, "U\n");
		}
		else {	/* Via C function */
			Ctrl->F.panel->width = Ctrl->D.dim[GMT_X];	Ctrl->F.panel->height = Ctrl->D.dim[GMT_Y];
			gmt_draw_map_panel (GMT, Ctrl->D.refpoint->x + 0.5 * Ctrl->D.dim[GMT_X], Ctrl->D.refpoint->y + 0.5 * Ctrl->D.dim[GMT_Y], 1U, Ctrl->F.panel);
		}
	}

	col_left_x = Ctrl->D.refpoint->x + Ctrl->C.off[GMT_X];			/* Left justification edge of items inside legend box accounting for clearance */
	row_base_y = Ctrl->D.refpoint->y + Ctrl->D.dim[GMT_Y] - Ctrl->C.off[GMT_Y];	/* Top justification edge of items inside legend box accounting for clearance  */
	column_number = 0;	/* Start at first column in multi-column setup */
	n_columns = 1;		/* Reset to default number of columns */
	/* Reset to annotation font text color */
	sprintf (def_txtcolor, "%s", gmt_putcolor (GMT, GMT->current.setting.font_annot[GMT_PRIMARY].fill.rgb));
	sprintf (txtcolor, "%s", gmt_putcolor (GMT, GMT->current.setting.font_annot[GMT_PRIMARY].fill.rgb));
	x_off_col[0] = 0.0;	/* The x-coordinate of left side of first column */
	x_off_col[n_columns] = Ctrl->D.dim[GMT_X];	/* Holds width of a row */

	if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);

	flush_paragraph = false;
	gap = Ctrl->C.off[GMT_Y];	/* This gets reset to 0 once we finish the first printable row */

	/* Tech, note: Using GMT->current.setting.io_seg_marker[GMT_IN] instead of GMT_OUT when writing data records as segment records
	 * since these will become input to plot, and plot will use the GMT_IN marker to identify these as header records. */

	for (tbl = 0; tbl < In->n_tables; tbl++) {	/* We only expect one table but who knows what the user does */
		for (seg = 0; seg < In->table[tbl]->n_segments; seg++) {	/* We only expect one segment in each table but again... */
			for (row = 0; row < In->table[tbl]->segment[seg]->n_rows; row++) {	/* Finally processing the rows */
				line = In->table[tbl]->segment[seg]->text[row];
				if (line[0] == GMT->current.setting.io_head_marker[GMT_IN] || gmt_is_a_blank_line (line)) continue;	/* Skip all headers */

				/* Data record to process */

				if (line[0] != 'T' && flush_paragraph) {	/* Flush contents of pending paragraph [Call GMT_text] */
					flush_paragraph = false;
					column_number = 0;
				}

				switch (line[0]) {
					case 'A':	/* Z lookup color table change: A CPT */
						if (P && GMT_Destroy_Data (API, &P) != GMT_NOERROR)	/* Remove the previous CPT from registration */
							Return (API->error);
						for (col = 1; line[col] == ' '; col++);	/* Wind past spaces */
						if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, &line[col], NULL)) == NULL)
							Return (API->error);
						GMT->current.setting.color_model = GMT_RGB;	/* Since we will be interpreting r/g/b triplets via z=<value> */
						break;

					case 'B':	/* B cptname offset height[+modifiers] [ Optional psscale args -B -I -L -M -N -S -Z -p ] */
						/* Color scale Bar [via GMT_psscale] */
						module_options[0] = '\0';
						sscanf (&line[2], "%s %s %s %[^\n]", bar_cpt, bar_gap, bar_height, module_options);
						strcpy (bar_modifiers, bar_height);	/* Save the entire modifier string */
						if ((c = strchr (bar_height, '+')) != NULL) c[0] = 0;	/* Chop off any modifiers so we can compute the height */
						row_height = gmt_M_to_inch (GMT, bar_height) + GMT->current.setting.map_tick_length[0] + GMT->current.setting.map_annot_offset[0] + FONT_HEIGHT_PRIMARY * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;
						fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-row_height, row_base_y+gap, x_off_col, &d_line_after_gap, 1, fill);
						x_off = gmt_M_to_inch (GMT, bar_gap);
						sprintf (buffer, "-C%s -O -K -Dx%gi/%gi+w%gi/%s+h+jTC %s --GMT_HISTORY=false", bar_cpt, Ctrl->D.refpoint->x + 0.5 * Ctrl->D.dim[GMT_X], row_base_y, Ctrl->D.dim[GMT_X] - 2 * x_off, bar_modifiers, module_options);
						GMT_Report (API, GMT_MSG_DEBUG, "RUNNING: gmt psscale %s\n", buffer);
						status = GMT_Call_Module (API, "psscale", GMT_MODULE_CMD, buffer);	/* Plot the colorbar */
						if (status) {
							GMT_Report (API, GMT_MSG_ERROR, "GMT_psscale returned error %d.\n", status);
							Return (GMT_RUNTIME_ERROR);
						}
						row_base_y -= row_height;
						column_number = 0;
						API->io_enabled[GMT_IN] = true;	/* UNDOING SETTING BY psscale */
						drawn = true;
						break;

					case 'C':	/* Font color change: C textcolor */
						C_is_active = true;
						sscanf (&line[2], "%[^\n]", txtcolor);
						if (!strcmp (txtcolor, "-"))	/* Reset to default color */
							strcpy (txtcolor, def_txtcolor);
						else if ((API->error = gmt_get_rgbtxt_from_z (GMT, P, txtcolor)) != 0)
							Return (GMT_RUNTIME_ERROR);	/* If given z=value then we look up colors */
						if (gmt_getrgb (GMT, txtcolor, C_rgb)) {
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Text color %s not recognized!\n", txtcolor);
							Return (GMT_RUNTIME_ERROR);
						}
						break;

					case 'D':	/* Delimiter record: D [offset] <pen>|- [-|=|+] */
						n_scan = sscanf (&line[2], "%s %s %s", txt_a, txt_b, txt_c);
						if (n_scan < 1) {	/* Clearly a bad record */
							GMT_Report (API, GMT_MSG_ERROR, "Not enough arguments given to D operator\n");
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
						d_line_hor_offset = gmt_M_to_inch (GMT, txt_a);
						if (txt_b[0] == '-')	/* Gave - as pen, meaning just note at what y-value we are but draw no line */
							d_line_half_width = 0.0;
						else {	/* Process the pen specification */
							if (txt_b[0] && gmt_getpen (GMT, txt_b, &current_pen)) gmt_pen_syntax (GMT, 'W', NULL, " ", 0);
							gmt_setpen (GMT, &current_pen);
							d_line_half_width = 0.5 * current_pen.width / PSL_POINTS_PER_INCH;	/* Half the pen width */
						}
						if (!(txt_c[0] == '-' || txt_c[0] == '=')) {	/* Fill the gap before the line, if fill is active */
							fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-quarter_line_spacing+d_line_half_width, row_base_y+gap, x_off_col, &d_line_after_gap, n_columns, fill);
							row_base_y -= quarter_line_spacing;
						}
						d_line_last_y0 = row_base_y;	/* Remember the y-value were we potentially draw the horizontal line */
						if (d_line_half_width > 0.0) {
							if (do_width) {
								PSL_defunits (PSL, "PSL_legend_hor_off", 2.0 * d_line_hor_offset);
								PSL_setcurrentpoint (PSL, Ctrl->D.refpoint->x + d_line_hor_offset, row_base_y);
								PSL_command (PSL, "PSL_legend_box_width PSL_legend_hor_off sub 0 D S\n");
							}
						else
							PSL_plotsegment (PSL, Ctrl->D.refpoint->x + d_line_hor_offset, row_base_y, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X] - d_line_hor_offset, row_base_y);
						}
						d_line_after_gap = (txt_c[0] == '+' || txt_c[0] == '=') ? 0.0 : quarter_line_spacing;
						row_base_y -= d_line_after_gap;
						d_line_after_gap -= d_line_half_width;	/* Shrink the gap fill-height after a D line by half the line width so we don't overwrite the line */
						column_number = 0;	/* Reset to new row */
						if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						drawn = true;
						break;

					case 'F':	/* Cell color: F fill1[,fill2,...,filln]  */
						/* First free all previous entries in fill array */
						for (col = 0; col < PSLEGEND_MAX_COLS; col++) gmt_M_str_free (fill[col]);
						pos = n_col = 0;
						while ((gmt_strtok (&line[2], " \t", &pos, p))) {
							if ((API->error = gmt_get_rgbtxt_from_z (GMT, P, p)) != 0) Return (GMT_RUNTIME_ERROR);	/* If given z=value then we look up colors */
							if (strcmp (p, "-")) fill[n_col++] = strdup (p);
							if (n_col > n_columns) {
								GMT_Report (API, GMT_MSG_ERROR, "Exceeding specified N columns (%d) in F operator (%d)\n", n_columns, n_col);
								Return (GMT_RUNTIME_ERROR);
							}
						}
						if (n_col == 1 && n_columns > 1) {	/* Only gave a constant color or - for the entire row, duplicate per column */
							for (col = 1; col < n_columns; col++) if (fill[0]) fill[col] = strdup (fill[0]);
						}
						break;

					case 'G':	/* Gap record: G gap (will be filled with current fill[0] setting if active) */
						sscanf (&line[2], "%s", txt_a);
						row_height = (txt_a[strlen(txt_a)-1] == 'l') ? atoi (txt_a) * one_line_spacing : gmt_M_to_inch (GMT, txt_a);
						fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-row_height, row_base_y+gap, x_off_col, &d_line_after_gap, n_columns, fill);
						row_base_y -= row_height;
						column_number = 0;
						if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						drawn = true;
						break;

					case 'H':	/* Header record: H fontsize|- font|- header */
						sscanf (&line[2], "%s %s %[^\n]", size, font, text);
						if (new_fontsyntax (GMT, size, font)) {	/* GMT5 font specification */
							sscanf (&line[2], "%s %[^\n]", font, text);
							if (size[0] == '-')	/* Want the default title font */
								sprintf (tmp, "%s", gmt_putfont (GMT, &GMT->current.setting.font_title));
							else
								strcpy (tmp, font);	/* Gave a font specification */
						}
						else {	/* Old GMT4 syntax for fontsize and font and must manually add color (e.g., via -C) */
							if (size[0] == '-') size[0] = 0;
							if (font[0] == '-') font[0] = 0;
							sprintf (tmp, "%s,%s,%s", size, font, txtcolor);		/* Put size, font and color together for parsing by gmt_getfont */
						}
						ifont = GMT->current.setting.font_title;	/* Set default font */
						gmt_getfont (GMT, tmp, &ifont);
						if (C_is_active) gmt_M_rgb_copy (ifont.fill.rgb, C_rgb);	/* Must update text color */
						d_off = 0.5 * (Ctrl->D.spacing - FONT_HEIGHT (ifont.id)) * ifont.size / PSL_POINTS_PER_INCH;	/* To center the text */
						row_height = Ctrl->D.spacing * ifont.size / PSL_POINTS_PER_INCH;
						if (do_width) {
							row_base_y -= row_height;
							PSL_setcurrentpoint (PSL, Ctrl->D.refpoint->x, row_base_y + d_off);
							PSL_command (PSL, "PSL_legend_box_width 2 div 0 G\n");
							PSL_plottext (PSL, 0.0, 0.0, -ifont.size, text, 0.0, PSL_BC, 0);
						}
						else {
							sprintf (buffer, "%s BC %s", gmt_putfont (GMT, &ifont), text);
							fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-row_height, row_base_y+gap, x_off_col, &d_line_after_gap, n_columns, fill);
							row_base_y -= row_height;
							/* Build output segment */
							if ((D[TXT] = get_dataset_pointer (API, D[TXT], GMT_IS_NONE, 1U, 64U, 2U, true)) == NULL) return (API->error);
							S[TXT] = get_segment (D, TXT, 0);	/* Since there will only be one table with one segment for each set, except for fronts */
							S[TXT]->data[GMT_X][krow[TXT]] = Ctrl->D.refpoint->x + 0.5 * Ctrl->D.dim[GMT_X];
							S[TXT]->data[GMT_Y][krow[TXT]] = row_base_y + d_off;
							S[TXT]->text[krow[TXT]++] = strdup (buffer);
							S[TXT]->n_rows++;
							D[TXT]->n_records++;
							GMT_Report (API, GMT_MSG_DEBUG, "TXT: %s\n", buffer);
							maybe_realloc_segment (GMT, S[TXT]);
						}
						column_number = 0;
						if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						drawn = true;
						break;

					case 'I':	/* Image record [use GMT_psimage]: I imagefile width justification */
						sscanf (&line[2], "%s %s %s", image, size, key);
						first = gmt_download_file_if_not_found (GMT, image, GMT_CACHE_DIR);
						if (gmt_getdatapath (GMT, &image[first], path, R_OK) == NULL) {
							GMT_Report (API, GMT_MSG_ERROR, "Cannot find/open file %s.\n", &image[first]);
							Return (GMT_FILE_NOT_FOUND);
						}
						if ((aspect = get_image_aspect (API, path)) < 0.0) {
							GMT_Report (API, GMT_MSG_ERROR, "Trouble reading %s! - Skipping.\n", &image[first]);
							continue;
						}
						justify = gmt_just_decode (GMT, key, PSL_NO_DEF);
						row_height = gmt_M_to_inch (GMT, size) * aspect;
						fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-row_height, row_base_y+gap, x_off_col, &d_line_after_gap, n_columns, fill);
						x_off = Ctrl->D.refpoint->x;
						x_off += (justify%4 == 1) ? Ctrl->C.off[GMT_X] : ((justify%4 == 3) ? Ctrl->D.dim[GMT_X] - Ctrl->C.off[GMT_X] : 0.5 * Ctrl->D.dim[GMT_X]);
						sprintf (buffer, "-O -K %s -Dx%gi/%gi+j%s+w%s --GMT_HISTORY=false", &image[first], x_off, row_base_y, key, size);
						GMT_Report (API, GMT_MSG_DEBUG, "RUNNING: gmt psimage %s\n", buffer);
						status = GMT_Call_Module (API, "psimage", GMT_MODULE_CMD, buffer);	/* Plot the image */
						if (status) {
							GMT_Report (API, GMT_MSG_ERROR, "GMT_psimage returned error %d.\n", status);
							Return (GMT_RUNTIME_ERROR);
						}
						row_base_y -= row_height;
						column_number = 0;
						drawn = true;
						break;

					case 'L':	/* Label record: L font|- justification label */
						text[0] = '\0';
						sscanf (&line[2], "%s %s %s %[^\n]", size, font, key, text);
						if (new_fontsyntax (GMT, size, font)) {	/* GMT5 font specification */
							sscanf (&line[2], "%s %s %[^\n]", font, key, text);
							if (font[0] == '-')	/* Want the default label font */
								sprintf (tmp, "%s", gmt_putfont (GMT, &GMT->current.setting.font_label));
							else
								strcpy (tmp, font);	/* Gave a font specification */
						}
						else {	/* Old GMT4 syntax for fontsize and font and must manually add color (e.g., via -C) */
							if (size[0] == '-') size[0] = 0;
							if (font[0] == '-') font[0] = 0;
							sprintf (tmp, "%s,%s,%s", size, font, txtcolor);	/* Put size, font and color together for parsing by gmt_getfont */
						}
						ifont = GMT->current.setting.font_label;	/* Set default font */
						gmt_getfont (GMT, tmp, &ifont);
						if (C_is_active) gmt_M_rgb_copy (ifont.fill.rgb, C_rgb);	/* Must update text color */
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
							GMT_Report (API, GMT_MSG_DEBUG, "The L record gave no info so skip to next cell\n");
							drawn = true;
							break;
						}
						justify = gmt_just_decode (GMT, key, 0);
						if (do_width) {
							PSL_setcurrentpoint (PSL, Ctrl->D.refpoint->x + Ctrl->C.off[GMT_X], row_base_y + d_off);
							if (justify == PSL_BR)
								PSL_command (PSL, "PSL_legend_box_width PSL_legend_clear_x sub 0 G\n");
							else if (justify == PSL_BC)
								PSL_command (PSL, "PSL_legend_box_width PSL_legend_clear_x sub 2 div 0 G\n");
							PSL_plottext (PSL, 0.0, 0.0, -ifont.size, text, 0.0, justify, 0);
						}
						else {
							x_off = Ctrl->D.refpoint->x + x_off_col[column_number];
							x_off += (justify%4 == 1) ? Ctrl->C.off[GMT_X] : ((justify%4 == 3) ? (x_off_col[column_number+1]-x_off_col[column_number]) - Ctrl->C.off[GMT_X] : 0.5 * (x_off_col[column_number+1]-x_off_col[column_number]));
							sprintf (buffer, "%s B%s %s", gmt_putfont (GMT, &ifont), key, text);
							if ((D[TXT] = get_dataset_pointer (API, D[TXT], GMT_IS_NONE, 1U, 64U, 2U, true)) == NULL) return (API->error);
							S[TXT] = get_segment (D, TXT, 0);	/* Since there will only be one table with one segment for each set, except for fronts */
							S[TXT]->data[GMT_X][krow[TXT]] = x_off;
							S[TXT]->data[GMT_Y][krow[TXT]] = row_base_y + d_off;
							S[TXT]->text[krow[TXT]++] = strdup (buffer);
							S[TXT]->n_rows++;
							GMT_Report (API, GMT_MSG_DEBUG, "TXT: %s\n", buffer);
							maybe_realloc_segment (GMT, S[TXT]);
						}
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
						row_height = GMT->current.setting.map_scale_height + FONT_HEIGHT_PRIMARY * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH + GMT->current.setting.map_annot_offset[0];
						d_off = FONT_HEIGHT_LABEL * GMT->current.setting.font_label.size / PSL_POINTS_PER_INCH + fabs(GMT->current.setting.map_label_offset);
						if ((txt_d[0] == 'f' || txt_d[0] == 'p') && gmt_get_modifier (txt_c, 'j', string))	/* Specified alternate justification old-style */
							just = string[0];
						else if (gmt_get_modifier (txt_c, 'a', string))	/* Specified alternate alignment */
							just = string[0];
						if (gmt_get_modifier (txt_c, 'u', string))	/* Specified alternate alignment */
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
							gmt_set_missing_options (GMT, "RJ");	/* If mode is modern, -R -J exist in the history, and if an overlay we may add these from history automatically */
							if (!GMT->common.R.active[RSET] || !GMT->common.J.active) {
								GMT_Report (API, GMT_MSG_ERROR, "The M record must have map -R -J if -Dx and no -R -J is used\n");
								Return (GMT_RUNTIME_ERROR);
							}
							sprintf (buffer, "-R%s -J%s -O -K -L%s", GMT->common.R.string, GMT->common.J.string, mapscale);
						}
						if (module_options[0]) strcat (buffer, module_options);
						strcat (buffer, " --GMT_HISTORY=false");
						GMT_Report (API, GMT_MSG_DEBUG, "RUNNING: gmt psbasemap %s\n", buffer);
						status = GMT_Call_Module (API, "psbasemap", GMT_MODULE_CMD, buffer);	/* Plot the scale */
						if (status) {
							GMT_Report (API, GMT_MSG_ERROR, "GMT_psbasemap returned error %d.\n", status);
							Return (GMT_RUNTIME_ERROR);
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
						while ((gmt_strtok (&line[2], " \t", &pos, p))) {
							col_width[n_columns++] = atof (p);
							if (n_columns == PSLEGEND_MAX_COLS) {
								GMT_Report (API, GMT_MSG_ERROR, "Exceeding maximum columns (%d) in N operator\n", PSLEGEND_MAX_COLS);
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
						if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG)) {
							double s = gmt_convert_units (GMT, "1", GMT_INCH, GMT->current.setting.proj_length_unit);
							GMT_Report (API, GMT_MSG_DEBUG, "Selected %d columns.  Column widths are:\n", n_columns);
							for (col = 1; col <= n_columns; col++)
								GMT_Report (API, GMT_MSG_DEBUG, "Column %d: %g %s\n", col, s*(x_off_col[col]-x_off_col[col-1]), GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
						}
						break;

					case '>':	/* Paragraph text header */
						if (gmt_M_compat_check (GMT, 4)) {	/* Warn and fall through on purpose */
							GMT_Report (API, GMT_MSG_COMPAT, "Paragraph text header flag > is deprecated; use P instead\n");
							n = sscanf (&line[1], "%s %s %s %s %s %s %s %s %s", xx, yy, size, angle, font, key, lspace, tw, jj);
							if (n < 0) n = 0;	/* Since -1 is returned if no arguments */
							if (!(n == 0 || n == 9)) {
								GMT_Report (API, GMT_MSG_ERROR, "The > record must have 0 or 9 arguments (only %d found)\n", n);
								Return (GMT_RUNTIME_ERROR);
							}
							if (n == 0 || size[0] == '-') sprintf (size, "%g", GMT->current.setting.font_annot[GMT_PRIMARY].size);
							if (n == 0 || font[0] == '-') sprintf (font, "%d", GMT->current.setting.font_annot[GMT_PRIMARY].id);
							sprintf (tmp, "%s,%s,", size, font);
							did_old = true;
						}
						else {
							GMT_Report (API, GMT_MSG_ERROR, "Unrecognized record (%s)\n", line);
							Return (GMT_RUNTIME_ERROR);
							break;
						}
						/* Intentionally fall through */
					case 'P':	/* Paragraph text header: P paragraph-mode-header-for-text */
						if (!did_old) {
							n = sscanf (&line[1], "%s %s %s %s %s %s %s %s", xx, yy, tmp, angle, key, lspace, tw, jj);
							if (n < 0) n = 0;	/* Since -1 is returned if no arguments */
							if (!(n == 0 || n == 8)) {
								GMT_Report (API, GMT_MSG_ERROR, "The P record must have 0 or 8 arguments (only %d found)\n", n);
								Return (GMT_RUNTIME_ERROR);
							}
						}
						did_old = false;
						if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						if (n == 0 || xx[0] == '-') sprintf (xx, "%g", col_left_x);
						if (n == 0 || yy[0] == '-') sprintf (yy, "%g", row_base_y);
						if (n == 0 || tmp[0] == '-') sprintf (tmp, "%gp,%d,%s", GMT->current.setting.font_annot[GMT_PRIMARY].size, GMT->current.setting.font_annot[GMT_PRIMARY].id, txtcolor);
						if (n == 0 || angle[0] == '-') sprintf (angle, "0");
						if (n == 0 || key[0] == '-') sprintf (key, "TL");
						if (n == 0 || lspace[0] == '-') sprintf (lspace, "%gi", one_line_spacing);
						if (n == 0 || tw[0] == '-') sprintf (tw, "%gi", Ctrl->D.dim[GMT_X] - 2.0 * Ctrl->C.off[GMT_X]);
						if (n == 0 || jj[0] == '-') sprintf (jj, "j");
						if (n_para >= 0) {	/* End of previous paragraph for sure */
							S[PAR]->n_rows = krow[PAR];
							S[PAR] = D[PAR]->table[0]->segment[n_para] = GMT_Alloc_Segment (GMT->parent, GMT_WITH_STRINGS, krow[PAR], 0U, NULL, S[PAR]);
						}
						if ((D[PAR] = get_dataset_pointer (API, D[PAR], GMT_IS_TEXT, 1U, n_par_total, 0U, true)) == NULL) return (API->error);
						sprintf (buffer, "%s %s %s %s %s %s %s %s", xx, yy, angle, tmp, key, lspace, tw, jj);
						S[PAR] = get_segment (D, PAR, ++n_para);	/* We store the header as one of the text records for simplicity */
						GMT_Report (API, GMT_MSG_DEBUG, "PAR: %s\n", buffer);
						S[PAR]->header = strdup (buffer);
						maybe_realloc_segment (GMT, S[PAR]);
						flush_paragraph = true;
						column_number = 0;
						drawn = true;
						krow[PAR] = 0;	/* Start fresh with new segment */
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
						if ((API->error = gmt_get_rgbtxt_from_z (GMT, P, txt_c)) != 0) Return (GMT_RUNTIME_ERROR);	/* If given z=value then we look up colors */
						if (strchr ("LCR", txt_a[0])) {	/* Gave L, C, or R justification relative to current cell */
							justify = gmt_just_decode (GMT, txt_a, 0);
							off_ss = (justify%4 == 1) ? Ctrl->C.off[GMT_X] : ((justify%4 == 3) ? (x_off_col[column_number+1]-x_off_col[column_number]) - Ctrl->C.off[GMT_X] : 0.5 * (x_off_col[column_number+1]-x_off_col[column_number]));
							x_off = Ctrl->D.refpoint->x + x_off_col[column_number];
						}
						else if (!strcmp (txt_a, "-")) {	/* Automatic margin offset */
							off_ss = GMT_LEGEND_DX1_MUL * Ctrl->S.scale * def_size;
							x_off = col_left_x + x_off_col[column_number];
						}
						else {	/* Gave a specific offset */
							off_ss = gmt_M_to_inch (GMT, txt_a);
							x_off = col_left_x + x_off_col[column_number];
						}
						if (!strcmp (txt_b, "-"))	/* Automatic label offset */
							off_tt = GMT_LEGEND_DX2_MUL * Ctrl->S.scale * def_size;
						else	/* Gave a specific offset */
							off_tt = gmt_M_to_inch (GMT, txt_b);
						d_off = 0.5 * (Ctrl->D.spacing - FONT_HEIGHT_PRIMARY) * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;	/* To center the text */
						row_base_y += half_line_spacing;	/* Move to center of box */
						if (symbol[0] == '-' && !strcmp (size, "-")) sprintf (size, "%gi", def_size);	/* If no size given then we must pick what we learned above */
						if (symbol[0] == 'f') {	/* Front is different, must plot as a line segment */
							double length, tlen, gap;
							int n = sscanf (size, "%[^/]/%[^/]/%s", A, B, C);

							if (n == 3) {	/* Got line length, tickgap, and ticklength */
								length = Ctrl->S.scale * gmt_M_to_inch (GMT, A);	/* The length of the line */
								tlen = Ctrl->S.scale * gmt_M_to_inch (GMT, C);	/* The length of the tick */
							}
							else if (n == 2 && B[0] != '-') {	/* Got line length and tickgap only */
								length = Ctrl->S.scale * gmt_M_to_inch (GMT, A);	/* The length of the line */
								gap = Ctrl->S.scale * gmt_M_to_inch (GMT, B);	/* The tick gap */
								tlen = 0.3 * gap;		/* The default length of the tick is 30% of gap */
							}
							else {	/* Got line length, select defaults for other things */
								length = Ctrl->S.scale * gmt_M_to_inch (GMT, A);	/* The length of the line */
								strcpy (B, "-1");		/* One centered tick */
								tlen = 0.3 * length;		/* The default length of the tick is 30% of length */
							}
							if ((c = strchr (symbol, '+')) != NULL)	/* Pass along all the given modifiers */
								strcpy (sub, c);
							else	/* The necessary arguments not supplied, provide reasonable defaults */
								sprintf (sub, "+l+b");	/* Box to the left of the line is our default front symbol */
							x = 0.5 * length;
							/* Place pen and fill colors in segment header */
							sprintf (buffer, "-Sf%s/%gi%s", B, tlen, sub);
							if (txt_c[0] != '-') {strcat (buffer, " -G"); strcat (buffer, txt_c);}
							if (txt_d[0] != '-') {strcat (buffer, " -W"); strcat (buffer, txt_d);}
							/* Prepare next output segment */
							if ((D[FRONT] = get_dataset_pointer (API, D[FRONT], GMT_IS_LINE, 64U, 2U, 2U, false)) == NULL) return (API->error);
							S[FRONT] = get_segment (D, FRONT, n_fronts);	/* Next front segment */
							S[FRONT]->header = strdup (buffer);
							GMT_Report (API, GMT_MSG_DEBUG, "FRONT: %s\n", buffer);
							/* Set begin and end coordinates of the line segment */
							S[FRONT]->data[GMT_X][0] = x_off + off_ss-x;	S[FRONT]->data[GMT_Y][0] = row_base_y;
							S[FRONT]->data[GMT_X][1] = x_off + off_ss+x;	S[FRONT]->data[GMT_Y][1] = row_base_y;
							S[FRONT]->n_rows = 2;
							D[FRONT]->n_records += 2;
							n_fronts++;
							if (n_fronts == GMT_SMALL_CHUNK) {
								GMT_Report (API, GMT_MSG_ERROR, "Can handle max %d front lines.  Let us know if this is a problem.\n", GMT_SMALL_CHUNK);
								return (API->error);
							}
						}
						else if (symbol[0] == 'q' || symbol[0] == '~') {	/* Quoted and decorated line is different, must plot as a line segment */
							double length = Ctrl->S.scale * gmt_M_to_inch (GMT, size);	/* The length of the line */;

							if ((D[QLINE] = get_dataset_pointer (API, D[QLINE], GMT_IS_LINE, 64U, 2U, 2U, false)) == NULL) return (API->error);
							x = 0.5 * length;
							/* Place pen and fill colors in segment header */
							sprintf (buffer, "-S%s", symbol);
							if (txt_d[0] != '-') {strcat (buffer, " -W"); strcat (buffer, txt_d);}
							S[QLINE] = get_segment (D, QLINE, n_quoted_lines);	/* Next quoted line segment */
							S[QLINE]->header = strdup (buffer);
							GMT_Report (API, GMT_MSG_DEBUG, "QLINE: %s\n", buffer);
							/* Set begin and end coordinates of the line segment */
							S[QLINE]->data[GMT_X][0] = x_off + off_ss-x;	S[QLINE]->data[GMT_Y][0] = row_base_y;
							S[QLINE]->data[GMT_X][1] = x_off + off_ss+x;	S[QLINE]->data[GMT_Y][1] = row_base_y;
							S[QLINE]->n_rows = 2;
							D[QLINE]->n_records += 2;
							n_quoted_lines++;
							if (n_quoted_lines == GMT_SMALL_CHUNK) {
								GMT_Report (API, GMT_MSG_ERROR, "Can handle max %d quoted/decorated lines.  Let us know if this is a problem.\n", GMT_SMALL_CHUNK);
								return (API->error);
							}
						}
						else {	/* Regular symbols */
							if ((D[SYM] = get_dataset_pointer (API, D[SYM], GMT_IS_POINT, 64U, 1U, 6U, true)) == NULL) return (API->error);
							S[SYM] = get_segment (D, SYM, n_symbols);	/* Since there will only be one table with one segment for each single row */
							S[SYM]->data[GMT_X][0] = x_off + off_ss;
							S[SYM]->data[GMT_Y][0] = row_base_y;
							S[SYM]->n_rows = 1;
							if (symbol[0] == 'k' || symbol[0] == 'K')	/* Custom symbols need the full name after k */
								sprintf (sub, "%s", symbol);
							else	/* Just the symbol code is needed */
								sprintf (sub, "%c", symbol[0]);
							if (symbol[0] == 'E' || symbol[0] == 'e') {	/* Ellipse */
								if (strchr (size, ',')) {	/* We got dir,major,minor instead of just size; parse and use */
									sscanf (size, "%[^,],%[^,],%s", A, B, C);
									az1 = atof (A);
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, B);
									y = Ctrl->S.scale * gmt_M_to_inch (GMT, C);
								}
								else {	/* Ellipse needs more arguments; we use minor = 0.65*major, az = 0 */
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, size);
									if (gmt_M_is_zero (x)) x = Ctrl->S.scale * def_size;	/* Safety valve */
									az1 = 0.0;
									y = 0.65 * x;
								}
								S[SYM]->data[2][0] = az1;
								S[SYM]->data[3][0] = x;
								S[SYM]->data[4][0] = y;
							}
							else if (symbol[0] == 'J' || symbol[0] == 'j') {	/* rotated rectangle */
								if (strchr (size, ',')) {	/* We got dir,w,h instead of just size; parse and use */
									sscanf (size, "%[^,],%[^,],%s", A, B, C);
									az1 = atof (A);
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, B);
									y = Ctrl->S.scale * gmt_M_to_inch (GMT, C);
								}
								else {	/* Rotated rectangle needs more arguments; we use height = 0.65*width, az = 30 */
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, size);
									if (gmt_M_is_zero (x)) x = Ctrl->S.scale * def_size;	/* Safety valve */
									y = 0.65 * x;
									az1 = 30.0;
								}
								S[SYM]->data[2][0] = az1;
								S[SYM]->data[3][0] = x;
								S[SYM]->data[4][0] = y;
							}
							else if (symbol[0] == 'V' || symbol[0] == 'v') {	/* Vector */
								/* Because we support both GMT4 and GMT5 vector notations this section is a bit messy */
								if (strchr (size, ',')) {	/* We got dir,length combined as one argument */
									sscanf (size, "%[^,],%s", A, B);
									az1 = atof (A);
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, B);
								}
								else {	/* No dir given, default to horizontal */
									az1 = 0.0;
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, size);
								}
								if (strchr (size, '/') && gmt_M_compat_check (GMT, 4))  {	/* The necessary arguments was supplied via GMT4 size arguments */
									i = 0;
									while (size[i] != '/' && size[i]) i++;
									size[i++] = '\0';	/* So gmt_M_to_inch won't complain */
									sprintf (sub, "%s%s+jc+e", symbol, &size[i]);
								}
								else if ((c = strchr (symbol, '+')) != NULL) {	/* GMT5 syntax: Pass along all the given modifiers */
									strcpy (sub, symbol);
									if ((c = strstr (sub, "+j")) != NULL && c[2] != 'c') {	/* Got justification, check if it is +jc */
										GMT_Report (API, GMT_MSG_ERROR, "Vector justification changed from +j%c to +jc\n", c[2]);
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
								else {
									struct GMT_PEN pen;
									gmt_M_memset (&pen, 1, struct GMT_PEN);	/* Wipe the pen */
									gmt_getpen (GMT, txt_d, &pen);
									pen.width *= 0.5;
									strcat (sub, "+p"); strcat (sub, gmt_putpen (API->GMT, &pen));
								}
								S[SYM]->data[2][0] = az1;
								S[SYM]->data[3][0] = x;
							}
							else if (symbol[0] == 'r') {	/* Rectangle  */
								if (strchr (size, ',')) {	/* We got w,h */
									sscanf (size, "%[^,],%s", A, B);
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, A);
									y = Ctrl->S.scale * gmt_M_to_inch (GMT, B);
								}
								else {	/* Rectangle also need more args, we use h = 0.65*w */
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, size);
									if (gmt_M_is_zero (x)) x = Ctrl->S.scale * def_size;	/* Safety valve */
									y = 0.65 * x;
								}
								S[SYM]->data[2][0] = x;
								S[SYM]->data[3][0] = y;
							}
							else if (symbol[0] == 'R') {	/* Rounded rectangle  */
								if (strchr (size, ',')) {	/* We got w,h,r */
									sscanf (size, "%[^,],%[^,],%s", A, B, C);
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, A);
									y = Ctrl->S.scale * gmt_M_to_inch (GMT, B);
									r = Ctrl->S.scale * gmt_M_to_inch (GMT, C);
								}
								else {	/* Rounded rectangle also need more args, we use h = 0.65*w and r = 0.1*w */
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, size);
									if (gmt_M_is_zero (x)) x = Ctrl->S.scale * def_size;	/* Safety valve */
									y = 0.65 * x;
									r = 0.1 * x;
								}
								S[SYM]->data[2][0] = x;
								S[SYM]->data[3][0] = y;
								S[SYM]->data[4][0] = r;
							}
							else if (symbol[0] == 'm') {	/* Math angle  */
								if (strchr (size, ',')) {	/* We got r,az1,az2 */
									sscanf (size, "%[^,],%[^,],%s", A, B, C);
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, A);
									az1 = atof (B);
									az2 = atof (C);
								}
								else {	/* Math angle need more args, we set fixed az1,az22 as 10 45 */
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, size);
									if (gmt_M_is_zero (x)) x = Ctrl->S.scale * def_size;	/* Safety valve */
									az1 = 10;	az2 = 45;
								}
								/* We want to center the arc around its mid-point */
								m_az = 0.5 * (az1 + az2);
								dx = 0.25 * x * cosd (m_az);
								if (!strchr (symbol, '+'))  {	/* The necessary arguments not supplied! */
									sprintf (sub, "m%gi+b+e", 0.3*x);	/* Double heads, head size 30% of diameter */
								}
								if (txt_c[0] == '-') strcat (sub, "+g-");
								else { strcat (sub, "+g"); strcat (sub, txt_c);}
								if (txt_d[0] == '-') strcat (sub, "+p-");
								else { strcat (sub, "+p"); strcat (sub, txt_d);}
								S[SYM]->data[GMT_X][0] -= dx;
								S[SYM]->data[2][0] = x;
								S[SYM]->data[3][0] = az1;
								S[SYM]->data[4][0] = az2;
							}
							else if (symbol[0] == 'w') {	/* Wedge also need more args; we set fixed az1,az2 as -30 30 */
								if (strchr (size, ',')) {	/* We got az1,az2,d */
									sscanf (size, "%[^,],%[^,],%s", A, B, C);
									az1 = atof (A);
									az2 = atof (B);
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, C);
								}
								else {
									x = Ctrl->S.scale * gmt_M_to_inch (GMT, size);
									if (gmt_M_is_zero (x)) x = Ctrl->S.scale * def_size;	/* Safety valve */
									az1 = -30;	az2 = 30;
								}
								/* We want to center the wedge around its mid-point */
								m_az = 0.5 * (az1 + az2);
								dx = 0.25 * x * cosd (m_az);
								S[SYM]->data[GMT_X][0] -= dx;
								S[SYM]->data[2][0] = x_off + off_ss - dx;
								S[SYM]->data[3][0] = az1;
								S[SYM]->data[4][0] = az2;
							}
							else {
								x = Ctrl->S.scale * gmt_M_to_inch (GMT, size);
								if (gmt_M_is_zero (x)) x = Ctrl->S.scale * def_size;	/* Safety valve */
								S[SYM]->data[2][0] = x;
							}
							/* Place pen and fill colors in segment header */
							sprintf (buffer, "-G"); strcat (buffer, txt_c);
							strcat (buffer, " -W"); strcat (buffer, txt_d);
							S[SYM]->header = strdup (buffer);
							GMT_Report (API, GMT_MSG_DEBUG, "SYM: %s\n", buffer);
							GMT_Report (API, GMT_MSG_DEBUG, "SYM: %s\n", sub);
							S[SYM]->text[0] = strdup (sub);
							SH = gmt_get_DS_hidden (S[SYM]);
							if (S[SYM]->n_rows == SH->n_alloc) S[SYM]->data = gmt_M_memory (GMT, S[SYM]->data, SH->n_alloc += GMT_SMALL_CHUNK, char *);
							n_symbols++;
						}
						/* Finally, print text; skip when empty */
						row_base_y -= half_line_spacing;	/* Go back to bottom of box */
						if (n_scan == 7) {	/* Place symbol text */
							if ((D[TXT] = get_dataset_pointer (API, D[TXT], GMT_IS_NONE, 1U, 64U, 2U, true)) == NULL) return (API->error);
							S[TXT] = get_segment (D, TXT, 0);	/* Since there will only be one table with one segment for each set, except for fronts */
							sprintf (buffer, "%gp,%d,%s BL %s", GMT->current.setting.font_annot[GMT_PRIMARY].size, GMT->current.setting.font_annot[GMT_PRIMARY].id, txtcolor, text);
							S[TXT]->data[GMT_X][krow[TXT]] = x_off + off_tt;
							S[TXT]->data[GMT_Y][krow[TXT]] = row_base_y + d_off;
							S[TXT]->text[krow[TXT]++] = strdup (buffer);
							S[TXT]->n_rows++;
							maybe_realloc_segment (GMT, S[TXT]);
							GMT_Report (API, GMT_MSG_DEBUG, "TXT: %s\n", buffer);
						}
						column_number++;
						if (Ctrl->F.debug) drawbase (GMT, PSL, Ctrl->D.refpoint->x, Ctrl->D.refpoint->x + Ctrl->D.dim[GMT_X], row_base_y);
						drawn = true;
						break;

					case 'T':	/* paragraph text record: T paragraph-text */
						if ((D[PAR] = get_dataset_pointer (API, D[PAR], GMT_IS_TEXT, n_par_total, n_par_lines, 0U, true)) == NULL) return (API->error);
						/* If no previous > record, then use defaults */
						if (!flush_paragraph) {	/* No header record, but a new paragraph. */
							if (n_para >= 0) {	/* End of previous paragraph for sure */
								S[PAR]->n_rows = krow[PAR];
								S[PAR] = D[PAR]->table[0]->segment[n_para] = GMT_Alloc_Segment (GMT->parent, GMT_WITH_STRINGS, krow[PAR], 0U, NULL, S[PAR]);
							}
							n_para++;
							krow[PAR] = 0;	/* Start fresh with new segment */
						}
						if ((S[PAR] = get_segment (D, PAR, n_para)) == NULL)	/* Get/Allocate this paragraph segment */
							S[PAR] = D[PAR]->table[0]->segment[n_para] = GMT_Alloc_Segment (GMT->parent, GMT_WITH_STRINGS, n_par_lines, 0U, NULL, NULL);
						if (!flush_paragraph) {	/* No header record, create one and add as segment header */
							d_off = 0.5 * (Ctrl->D.spacing - FONT_HEIGHT_PRIMARY) * GMT->current.setting.font_annot[GMT_PRIMARY].size / PSL_POINTS_PER_INCH;
							sprintf (buffer, "%g %g 0 %gp,%d,%s TL %gi %gi j", col_left_x, row_base_y - d_off, GMT->current.setting.font_annot[GMT_PRIMARY].size, GMT->current.setting.font_annot[GMT_PRIMARY].id, txtcolor, one_line_spacing, Ctrl->D.dim[GMT_X] - 2.0 * Ctrl->C.off[GMT_X]);
							S[PAR]->header = strdup (buffer);
							GMT_Report (API, GMT_MSG_DEBUG, "PAR: %s\n", buffer);
						}
						/* Now processes paragraph text */
						sscanf (&line[2], "%[^\n]", text);
						S[PAR]->text[krow[PAR]++] = strdup (text);
						S[PAR]->n_rows++;
						maybe_realloc_segment (GMT, S[PAR]);
						GMT_Report (API, GMT_MSG_DEBUG, "PAR: %s\n", text);
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
							v_line_ver_offset = gmt_M_to_inch (GMT, txt_a);
							if (txt_b[0] && gmt_getpen (GMT, txt_b, &current_pen)) {
								gmt_pen_syntax (GMT, 'V', NULL, " ", 0);
								Return (GMT_RUNTIME_ERROR);
							}
							gmt_setpen (GMT, &current_pen);
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
						GMT_Report (API, GMT_MSG_ERROR, "Unrecognized record (%s)\n", line);
						Return (GMT_RUNTIME_ERROR);
					break;
				}
				if (drawn) gap = 0.0;	/* No longer first record that draws on page */
			}
		}
	}
	/* If there is clearance and fill is active we must paint the clearance row */
	if (Ctrl->C.off[GMT_Y] > 0.0) fillcell (GMT, Ctrl->D.refpoint->x, row_base_y-Ctrl->C.off[GMT_Y], row_base_y, x_off_col, &d_line_after_gap, n_columns, fill);

	if (GMT_Destroy_Data (API, &In) != GMT_NOERROR) {	/* Remove the main input file from registration */
		Return (API->error);
	}
	if (P && GMT_Destroy_Data (API, &P) != GMT_NOERROR)	/* Remove the last CPT from registration */
		Return (API->error);

	if (Ctrl->F.active && !do_width) {	/* Draw legend frame box */
		gmt_draw_map_panel (GMT, Ctrl->D.refpoint->x + 0.5 * Ctrl->D.dim[GMT_X], Ctrl->D.refpoint->y + 0.5 * Ctrl->D.dim[GMT_Y], 2U, Ctrl->F.panel);
	}

	PSL_setorigin (PSL, -shrink[XLO], -shrink[YLO], 0.0, PSL_FWD);	/* Adjustments due to subplot set -C */

	/* Time to plot any symbols, text, fronts, quoted lines, and paragraphs we collected in the loop */

	if (D[FRONT]) {
		/* Create option list, register D[FRONT] as input source */
		D[FRONT]->table[0]->n_segments = n_fronts;	/* Set correct number of fronts */
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, D[FRONT], string) != GMT_NOERROR) {
			Return (API->error);
		}
		sprintf (buffer, "-R0/%g/0/%g -Jx1i -O -K -N -Sf0.1i %s --GMT_HISTORY=false", GMT->current.proj.rect[XHI], GMT->current.proj.rect[YHI], string);
		GMT_Report (API, GMT_MSG_DEBUG, "RUNNING: FRONT: gmt %s %s\n", plot_points[ID], buffer);
		if (GMT_Call_Module (API, plot_points[ID], GMT_MODULE_CMD, buffer) != GMT_NOERROR) {	/* Plot the fronts */
			Return (API->error);
		}
		if (GMT_Close_VirtualFile (API, string) != GMT_NOERROR) {
			Return (API->error);
		}
#ifdef DEBUG
		if (Ctrl->DBG.active) {
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_IO_RESET, NULL, "dump_front.txt", D[FRONT]) != GMT_NOERROR) {
				Return (API->error);
			}
		}
#endif
		D[FRONT]->table[0]->n_segments = GMT_SMALL_CHUNK;	/* Reset to allocation limit */
	}
	if (D[QLINE]) {
		/* Create option list, register D[QLINE] as input source */
		D[QLINE]->table[0]->n_segments = n_quoted_lines;	/* Set correct number of lines */
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, D[QLINE], string) != GMT_NOERROR) {
			Return (API->error);
		}
		sprintf (buffer, "-R0/%g/0/%g -Jx1i -O -K -N -Sqn1 %s --GMT_HISTORY=false", GMT->current.proj.rect[XHI], GMT->current.proj.rect[YHI], string);
		GMT_Report (API, GMT_MSG_DEBUG, "RUNNING: QLINE: gmt %s %s\n", plot_points[ID], buffer);
		if (GMT_Call_Module (API, plot_points[ID], GMT_MODULE_CMD, buffer) != GMT_NOERROR) {	/* Plot the fronts */
			Return (API->error);
		}
		if (GMT_Close_VirtualFile (API, string) != GMT_NOERROR) {
			Return (API->error);
		}
#ifdef DEBUG
		if (Ctrl->DBG.active) {
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_IO_RESET, NULL, "dump_qline.txt", D[QLINE]) != GMT_NOERROR) {
				Return (API->error);
			}
		}
#endif
		D[QLINE]->table[0]->n_segments = GMT_SMALL_CHUNK;	/* Reset to allocation limit */
	}
	if (D[SYM]) {
		D[SYM]->table[0]->n_segments = n_symbols;	/* Set correct number of segments */
		/* Create option list, register D[SYM] as input source */
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, D[SYM], string) != GMT_NOERROR) {
			Return (API->error);
		}
		/* Because the sizes internally are in inches we must tell plot that inch is the current length unit */
		sprintf (buffer, "-R0/%g/0/%g -Jx1i -O -K -N -S %s --PROJ_LENGTH_UNIT=inch --GMT_HISTORY=false", GMT->current.proj.rect[XHI], GMT->current.proj.rect[YHI], string);
		GMT_Report (API, GMT_MSG_DEBUG, "RUNNING: SYM: gmt %s %s\n", plot_points[ID], buffer);
		if (GMT_Call_Module (API, plot_points[ID], GMT_MODULE_CMD, buffer) != GMT_NOERROR) {	/* Plot the symbols */
			Return (API->error);
		}
		if (GMT_Close_VirtualFile (API, string) != GMT_NOERROR) {
			Return (API->error);
		}
#ifdef DEBUG
		if (Ctrl->DBG.active) {
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_POINT, GMT_IO_RESET, NULL, "dump_sym.txt", D[SYM]) != GMT_NOERROR) {
				Return (API->error);
			}
		}
#endif
		D[SYM]->table[0]->n_segments = GMT_SMALL_CHUNK;	/* Reset to allocation limit */
	}
	if (D[TXT]) {
		D[TXT]->table[0]->segment[0]->n_rows = D[TXT]->n_records = krow[TXT];
		/* Create option list, register D[TXT] as input source */
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_IN, D[TXT], string) != GMT_NOERROR) {
			Return (API->error);
		}
		sprintf (buffer, "-R0/%g/0/%g -Jx1i -O -K -N -F+f+j %s --GMT_HISTORY=false", GMT->current.proj.rect[XHI], GMT->current.proj.rect[YHI], string);
		GMT_Report (API, GMT_MSG_DEBUG, "RUNNING: TXT: gmt %s %s\n", plot_text[ID], buffer);
		if (GMT_Call_Module (API, plot_text[ID], GMT_MODULE_CMD, buffer) != GMT_NOERROR) {	/* Plot the symbol labels */
			Return (API->error);
		}
		if (GMT_Close_VirtualFile (API, string) != GMT_NOERROR) {
			Return (API->error);
		}
#ifdef DEBUG
		if (Ctrl->DBG.active) {
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_IO_RESET, NULL, "dump_txt.txt", D[TXT]) != GMT_NOERROR) {
				Return (API->error);
			}
		}
#endif
		D[TXT]->table[0]->segment[0]->n_rows = D[TXT]->n_records = GMT_SMALL_CHUNK;	/* To free what we allocated */
	}
	if (D[PAR]) {
		if (n_para >= 0) {	/* End of last paragraph for sure */
			S[PAR]->n_rows = krow[PAR];
			D[PAR]->table[0]->n_records += S[PAR]->n_rows;
			D[PAR]->n_records = D[PAR]->table[0]->n_records;
			S[PAR] = D[PAR]->table[0]->segment[n_para] = GMT_Alloc_Segment (GMT->parent, GMT_WITH_STRINGS, krow[PAR], 0U, NULL, S[PAR]);
		}

		/* Create option list, register D[PAR] as input source */
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_TEXT, GMT_IN, D[PAR], string) != GMT_NOERROR) {
			Return (API->error);
		}
		sprintf (buffer, "-R0/%g/0/%g -Jx1i -O -K -N -M -F+a+f+j %s --GMT_HISTORY=false", GMT->current.proj.rect[XHI], GMT->current.proj.rect[YHI], string);
		GMT_Report (API, GMT_MSG_DEBUG, "RUNNING: PAR: gmt %s %s\n", plot_text[ID], buffer);
		if (GMT_Call_Module (API, plot_text[ID], GMT_MODULE_CMD, buffer) != GMT_NOERROR) {	/* Plot paragraphs */
			Return (API->error);
		}
		if (GMT_Close_VirtualFile (API, string) != GMT_NOERROR) {
			Return (API->error);
		}
#ifdef DEBUG
		if (Ctrl->DBG.active) {
			if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_TEXT, GMT_IO_RESET, NULL, "dump_par.txt", D[PAR]) != GMT_NOERROR) {
				Return (API->error);
			}
		}
#endif
	}
	if (do_width)	/* Adjust for mismatch between width and actual width */
		PSL_command (PSL, "PSL_legend_box_width PSL_legend_box_height sub %d 2 div mul 0 translate\n", (Ctrl->D.justify%4)-1);

	PSL_setorigin (PSL, shrink[XLO], shrink[YLO], 0.0, PSL_FWD);	/* Undo any damage for adjustments due to subplot set -C */

	PSL_setorigin (PSL, -x_orig, -y_orig, 0.0, PSL_INV);	/* Reset */
	Ctrl->D.refpoint->x = x_orig;	Ctrl->D.refpoint->y = y_orig;

	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_M_memcpy (PSL->internal.origin, xy_mode, 2, char);
	gmt_M_memcpy (PSL->internal.offset, xy_offset, 2, double);
	gmt_plotend (GMT);

	for (id = 0; id < N_DAT; id++) {
		if (D[id]) {
#ifdef DEBUG
			if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG)) {
				char file[PATH_MAX] = {""};
				if (API->tmp_dir)	/* Use the established temp directory */
					sprintf (file, "%s/%s", API->tmp_dir, dname[id]);
				else	/* Must dump it in current directory */
					sprintf (file, "%s", dname[id]);
				if (GMT_Write_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_LINE, GMT_IO_RESET, NULL, file, D[id]) != GMT_NOERROR) {
					GMT_Report (API, GMT_MSG_DEBUG, "Dumping data to %s failed\n", file);
				}
			}
#endif
			if (GMT_Destroy_Data (API, &D[id]) != GMT_NOERROR)
				Return (API->error);
		}
	}

	if (legend_file[0] && gmt_remove_file (GMT, legend_file)) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to delete current legend file %s\n", legend_file);
		Return (API->error);
	}

	Return (GMT_NOERROR);
}
