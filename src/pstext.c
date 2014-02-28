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
 * Brief synopsis: pstext will read (x, y[, font, angle, justify], text) from GMT->session.std[GMT_IN]
 * or file and plot the textstrings at (x,y) on a map using the font attributes
 * and justification selected by the user.  Alternatively (with -M), read
 * one or more text paragraphs to be typeset.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"pstext"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot or typeset text on maps"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-:>BJKOPRUVXYacfhptxy" GMT_OPT("E")

void GMT_enforce_rgb_triplets (struct GMT_CTRL *GMT, char *text, unsigned int size);
bool GMT_is_a_blank_line (char *line);

#define PSTEXT_CLIP		1
#define PSTEXT_PLOT		2
#define PSTEXT_TERMINATE	3

struct PSTEXT_CTRL {
	struct PSTEXT_A {	/* -A */
		bool active;
	} A;
	struct PSTEXT_C {	/* -C<dx>/<dy> */
		bool active;
		bool percent;
		double dx, dy;
	} C;
	struct PSTEXT_D {	/* -D[j]<dx>[/<dy>][v[<pen>]] */
		bool active;
		bool line;
		int justify;
		double dx, dy;
		struct GMT_PEN pen;
	} D;
	struct PSTEXT_F {	/* -F[+f<fontinfo>+a<angle>+j<justification>+l|h] */
		bool active;
		struct GMT_FONT font;
		double angle;
		int justify, R_justify, nread;
		unsigned int get_text;	/* 0 = from data record, 1 = segment label (+l), or 2 = segment header (+h) */
		char read[4];		/* Contains a, c, f, and/or j in order required to be read from input */
	} F;
	struct PSTEXT_G {	/* -G<fill> */
		bool active;
		bool mode;
		struct GMT_FILL fill;
	} G;
	struct PSTEXT_L {	/* -L */
		bool active;
	} L;
	struct PSTEXT_M {	/* -M */
		bool active;
	} M;
	struct PSTEXT_N {	/* -N */
		bool active;
	} N;
	struct PSTEXT_Q {	/* -Q<case> */
		bool active;
		int mode;	/* 0 = do nothing, -1 = force lower case, +1 = force upper case */
	} Q;
	struct PSTEXT_S {	/* -S<pen> */
		bool active;
		struct GMT_PEN pen;
	} S;
	struct PSTEXT_T {	/* -To|O|c|C */
		bool active;
		char mode;
	} T;
	struct PSTEXT_W {	/* -W[<pen>] */
		bool active;
		struct GMT_PEN pen;
	} W;
	struct PSTEXT_Z {	/* -Z<z_level> */
		bool active;
	} Z;
};

struct PSTEXT_INFO {
	int text_justify;
	int block_justify;
	int boxflag;
	int space_flag;
	double x_offset, y_offset;	/* Offset from reference point */
	double line_spacing;
	double paragraph_width;
	double paragraph_angle;
	double x_space, y_space;	/* Extra spacing between box and text */
	struct GMT_FONT font;
	struct GMT_PEN boxpen;
	struct GMT_PEN vecpen;
	struct GMT_FILL boxfill;
};

void *New_pstext_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSTEXT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSTEXT_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->D.pen = C->W.pen = GMT->current.setting.map_default_pen;
	C->C.dx = C->C.dy = 15.0;	/* 15% of font size is default clearance */
	C->C.percent = true;
	C->F.justify = PSL_MC;		/* MC is the default */
	C->F.font = GMT->current.setting.font_annot[0];		/* Default font */
	GMT_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* No fill */
	C->S.pen = GMT->current.setting.map_default_pen;

	return (C);
}

void Free_pstext_Ctrl (struct GMT_CTRL *GMT, struct PSTEXT_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	GMT_free (GMT, C);
}

void GMT_putwords (struct GMT_CTRL *GMT, struct PSL_CTRL *PSL, double x, double y, char *text, struct PSTEXT_INFO *T) {
	double offset[2];

	GMT_memcpy (PSL->current.rgb[PSL_IS_FILL], GMT->session.no_rgb, 3, double);	/* Reset to -1,-1,-1 since text setting must set the color desired */
	GMT_memcpy (PSL->current.rgb[PSL_IS_STROKE], GMT->session.no_rgb, 3, double);	/* Reset to -1,-1,-1 since text setting must set the color desired */
	if (T->space_flag) {	/* Meant % of fontsize */
		offset[0] = 0.01 * T->x_space * T->font.size / PSL_POINTS_PER_INCH;
		offset[1] = 0.01 * T->y_space * T->font.size / PSL_POINTS_PER_INCH;
	}
	else {	/* Gave in distance units */
		offset[0] = T->x_space;
		offset[1] = T->y_space;
	}

	/* Set some paragraph parameters */
	PSL_setparagraph (PSL, T->line_spacing, T->paragraph_width, T->text_justify);
	PSL_setfont (PSL, T->font.id);

	if (T->boxflag & 32) {	/* Need to draw a vector from (x,y) to the offset text */
		GMT_setpen (GMT, &(T->vecpen));
		PSL_plotsegment (PSL, x, y, x + T->x_offset, y + T->y_offset);
	}
	x += T->x_offset;	y += T->y_offset;	/* Move to the actual reference point */
	if (T->boxflag) {	/* Need to lay down the box first, then place text */
		int mode = 0;
		if (T->boxflag & 1) GMT_setpen (GMT, &(T->boxpen));			/* Change current pen */
		if (T->boxflag & 2) GMT_setfill (GMT, &(T->boxfill), T->boxflag & 1);	/* Change curent fill */
		if (T->boxflag & 1) mode = PSL_RECT_STRAIGHT;	/* Set the correct box shape */
		if (T->boxflag & 4) mode = PSL_RECT_ROUNDED;
		if (T->boxflag & 8) mode = PSL_RECT_CONCAVE;
		if (T->boxflag & 16) mode = PSL_RECT_CONVEX;
		/* Compute text box, draw/fill it, and in the process store the text in the PS file for next command */
		PSL_plotparagraphbox (PSL, x, y, T->font.size, text, T->paragraph_angle, T->block_justify, offset, mode);
		/* Passing NULL means we typeset using the last stored paragraph info */
		GMT_setfont (GMT, &T->font);
		PSL_plotparagraph (PSL, x, y, T->font.size, NULL, T->paragraph_angle, T->block_justify);
	}
	else {	/* No box beneath */
		GMT_setfont (GMT, &T->font);
		PSL_plotparagraph (PSL, x, y, T->font.size, text, T->paragraph_angle, T->block_justify);
	}
}

void load_parameters_pstext (struct PSTEXT_INFO *T, struct PSTEXT_CTRL *C)
{
	GMT_memset (T, 1, struct PSTEXT_INFO);
	T->x_space = C->C.dx;
	T->y_space = C->C.dy;
	T->space_flag = (C->C.percent) ? 1 : 0;
	if (C->D.active) {
		T->x_offset = C->D.dx;
		T->y_offset = C->D.dy;
		if (C->D.line) T->boxflag |= 32;
		T->vecpen = C->D.pen;
	}
	if (C->W.active || C->G.active) {
		if (C->W.active) T->boxflag |= 1;	/* Want box outline */
		if (C->G.active) T->boxflag |= 2;	/* Want filled box */
		if (C->T.mode == 'O') T->boxflag |= 4;	/* Want rounded box outline */
		if (C->T.mode == 'c') T->boxflag |= 8;	/* Want concave box outline */
		if (C->T.mode == 'C') T->boxflag |= 16;	/* Want convex box outline */
		T->boxpen = C->W.pen;
		T->boxfill = C->G.fill;
	}
	/* Initialize default attributes */
	T->font = C->F.font;
	T->paragraph_angle = C->F.angle;
	T->block_justify = C->F.justify;
}

int get_input_format_version (struct GMT_CTRL *GMT, char *buffer, int mode)
{
	/* Try to determine if input is the old GMT4-style format.
	 * mode = 0 means normal textrec, mode = 1 means paragraph mode.
	 * Return 4 if GMT 4, 5 if GMT 5, -1 if nothing can be done */
	
	int n, k;
	char size[GMT_LEN256] = {""}, angle[GMT_LEN256] = {""}, font[GMT_LEN256] = {""}, just[GMT_LEN256] = {""}, txt[GMT_BUFSIZ] = {""};
	char spacing[GMT_LEN256] = {""}, width[GMT_LEN256] = {""}, pjust[GMT_LEN256] = {""};
	
	if (!buffer || !buffer[0]) return (-1);	/* Nothing to work with */
	
	if (mode) {	/* Paragraph control record */
		n = sscanf (buffer, "%s %s %s %s %s %s %s\n", size, angle, font, just, spacing, width, pjust);
		if (n < 7) return (5);	/* Clearly not the old format since missing items */
	}
	else {		/* Regular text record */
		n = sscanf (buffer, "%s %s %s %s %[^\n]", size, angle, font, just, txt);
		if (n < 5) return (5);	/* Clearly not the old format since missing items */
	}
	if (GMT_not_numeric (GMT, angle)) return (5);	/* Since angle is not a number */
	k = (int)strlen (size) - 1;
	if (size[k] == 'c' || size[k] == 'i' || size[k] == 'm' || size[k] == 'p') size[k] = '\0';	/* Chop of unit */
	if (GMT_not_numeric (GMT, size)) return (5);	/* Since size is not a number */
	if (GMT_just_decode (GMT, just, 12) == -99) return (5);	/* Since justify not in correct format */
	if (mode) {	/* A few more checks for paragraph mode */
		k = (int)strlen (spacing) - 1;
		if (spacing[k] == 'c' || spacing[k] == 'i' || spacing[k] == 'm' || spacing[k] == 'p') spacing[k] = '\0';	/* Chop of unit */
		if (GMT_not_numeric (GMT, spacing)) return (5);	/* Since spacing is not a number */
		k = (int)strlen (width) - 1;
		if (width[k] == 'c' || width[k] == 'i' || width[k] == 'm' || width[k] == 'p') width[k] = '\0';	/* Chop of unit */
		if (GMT_not_numeric (GMT, width)) return (5);		/* Since width is not a number */
		if (!(pjust[0] == 'j' && pjust[1] == '\0') && GMT_just_decode (GMT, pjust, 0) == -99) return (5);
	}

	/* Well, seems like the old format so far */
	GMT_Report (GMT->parent, GMT_MSG_COMPAT, "Warning: use of old style pstext input is deprecated.\n");
	return (4);
}

int GMT_pstext_usage (struct GMTAPI_CTRL *API, int level, int show_fonts)
{
	/* This displays the pstext synopsis and optionally full usage information */

	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: pstext [<table>] %s %s [-A] [%s]\n", GMT_J_OPT, GMT_Rgeoz_OPT, GMT_B_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-C<dx>/<dy>] [-D[j|J]<dx>[/<dy>][v[<pen>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-F[+a[<angle>]][+c[<justify>]][+f[<font>]][+h|l][+j[<justify>]]] [-G<color>] [%s] [-K]\n", GMT_Jz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-L] [-M] [-N] [-O] [-P] [-Q<case>] [-To|O|c|C] [%s] [%s]\n", GMT_U_OPT, GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-W[<pen>] [%s] [%s] [-Z[<zlevel>|+]]\n", GMT_X_OPT, GMT_Y_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s]\n\t[%s]\n", GMT_a_OPT, GMT_c_OPT, GMT_f_OPT, GMT_h_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s]\n\t[%s]\n\n", GMT_p_OPT, GMT_t_OPT, GMT_colon_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\tReads (x,y[,fontinfo,angle,justify],text) from <table> [or stdin].\n");
	GMT_Message (API, GMT_TIME_NONE, "\tOR (with -M) one or more text paragraphs with formatting info in the segment header.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tBuilt-in escape sequences:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   @~ toggles between current font and Symbol font.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   @%%<no>%% switches to font number <no>; @%%%% resets font.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   @:<size>: switches font size; @:: resets font size.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   @;<color>; switches font color; @;; resets font color.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   @+ toggles between normal and superscript mode.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   @- toggles between normal and subscript mode.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   @# toggles between normal and Small Caps mode.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   @_ toggles between normal and underlined text.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   @!<char1><char2> makes one composite character.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   @@ prints the @ sign itself.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use @a, @c, @e, @n, @o, @s, @u, @A, @C @E, @N, @O, @U for accented European characters.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t(See manual page for more information).\n");

	if (show_fonts) {	/* List fonts */
		unsigned int i;
		GMT_Message (API, GMT_TIME_NONE, "\n\tFont #	Font Name\n");
		GMT_Message (API, GMT_TIME_NONE, "\t------------------------------------\n");
		for (i = 0; i < API->GMT->session.n_fonts; i++)
			GMT_Message (API, GMT_TIME_NONE, "\t%3ld\t%s\n", i, API->GMT->session.font[i].name);
	}

	if (show_fonts) return (EXIT_SUCCESS);
	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Option (API, "J-Z,R");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<table> is one or more ASCII files with text to be plotted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no files are given, standard input is read.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Angles given as azimuths; convert to directions using current projection.\n");
	GMT_Option (API, "B-");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Set the clearance between characters and surrounding box.  Only used\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   if -W has been set.  Append units {%s} or %% of fontsize [15%%].\n", GMT_DIM_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t-D Add <add_x>,<add_y> to the text origin AFTER projecting with -J [0/0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Dj to move text origin away from point (direction determined by text's justification).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Upper case -DJ will shorten diagonal shifts at corners by sqrt(2).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append v[<pen>] to draw line from text to original point.  If <add_y> is not given it equal <add_x>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify values for text attributes that apply to all text records:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +a[<angle>] specifies the baseline angle for all text [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +c<justify> get the corresponding coordinate from the -R string instead of a given (x,y).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +f[<fontinfo>] sets the size, font, and optionally the text color [%s].\n",
		GMT_putfont (API->GMT, API->GMT->current.setting.font_annot[0]));
	GMT_Message (API, GMT_TIME_NONE, "\t   +j[<justify>] sets text justification relative to given (x,y) coordinate.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Give a 2-char combo from [T|M|B][L|C|R] (top/middle/bottom/left/center/right) [CM].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +h will use as text the most recent segment header.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +l will use as text the label specified via -L<label> in the most recent segment header.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If an attribute +f|+a|+j is not followed by a value we read the information from the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   data file in the order given by the -F option.  Only one of +h or +l can be specified.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: +h|l modifiers cannot be used in paragraph mode (-M).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Paint the box underneath the text with specified color [Default is no paint].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, append c to set these boxes as clip paths based on text (and -C).  No text is plotted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   See psclip -Cs to plot the hidden text.  Cannot be used with paragraph mode (-M).\n");
	GMT_Option (API, "K");
	GMT_Message (API, GMT_TIME_NONE, "\t-L List the font-numbers and font-names available, then exits.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Set paragraph text mode [Default is single item mode].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Expects (x y size angle fontno justify linespace parwidth parjust) in segment header\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   followed by lines with one or more paragraphs of text.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   parjust is one of (l)eft, (c)enter, (r)ight, or (j)ustified.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do Not clip text that exceeds the map boundaries [Default will clip].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q For all text to be (l)lower or (u)pper-case [Default leaves text as is].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set shape of textbox when using -G and/or -W.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Add o for rectangle [Default] or O for rectangle with rounded corners,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   When -M is used you can also set c for concave and C for convex rectangle.\n");
	GMT_Option (API, "U,V");
	GMT_pen_syntax (API->GMT, 'W', "Draw a box around the text with the specified pen [Default pen is %s].");
	GMT_Option (API, "X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z For 3-D plots: expect records to have a z value in the 3rd column (i.e., x y z size ...).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note that -Z+ also sets -N.\n");
	GMT_Option (API, "a,c,f,h,p,t,:,.");
	
	return (EXIT_FAILURE);
}

#define GET_REC_TEXT	0
#define GET_SEG_LABEL	1
#define GET_SEG_HEADER	2

int GMT_pstext_parse (struct GMT_CTRL *GMT, struct PSTEXT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pstext and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	int j, k;
	unsigned int pos, n_errors = 0;
	bool explicit_justify = false;
	char txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""}, p[GMT_BUFSIZ] = {""};
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Getting azimuths rather than directions, must convert via map projection */
				Ctrl->A.active = true;
				break;
			case 'C':
				Ctrl->C.active = true;
				if (opt->arg[0]) {	/* Replace default settings with user settings */
					Ctrl->C.percent = (strchr (opt->arg, '%')) ? true : false;
					k = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b);
					for (j = 0; txt_a[j]; j++) if (txt_a[j] == '%') txt_a[j] = '\0';	/* Remove % signs before processing values */
					for (j = 0; k == 2 && txt_b[j]; j++) if (txt_b[j] == '%') txt_b[j] = '\0';
					Ctrl->C.dx = GMT_to_inch (GMT, txt_a);
					Ctrl->C.dy = (k == 2) ? GMT_to_inch (GMT, txt_b) : Ctrl->C.dx;
				}
				break;
			case 'D':
				Ctrl->D.active = true;
				k = 0;
				if (opt->arg[k] == 'j') { Ctrl->D.justify = 1, k++; }
				else if (opt->arg[k] == 'J') { Ctrl->D.justify = 2, k++; }
				for (j = k; opt->arg[j] && opt->arg[j] != 'v'; j++);
				if (opt->arg[j] == 'v') {
					Ctrl->D.line = true;
					n_errors += GMT_check_condition (GMT, opt->arg[j+1] && GMT_getpen (GMT, &opt->arg[j+1], &Ctrl->D.pen), "Syntax error -D option: Give pen after c\n");
					opt->arg[j] = 0;
				}
				j = sscanf (&opt->arg[k], "%[^/]/%s", txt_a, txt_b);
				Ctrl->D.dx = GMT_to_inch (GMT, txt_a);
				Ctrl->D.dy = (j == 2) ? GMT_to_inch (GMT, txt_b) : Ctrl->D.dx;
				break;
			case 'F':
				Ctrl->F.active = true;
				pos = 0;
				
				while (GMT_getmodopt (GMT, opt->arg, "afjclh", &pos, p) && Ctrl->F.nread < 4) {	/* Looking for +f, +a, +j, +c, +l|h */
					switch (p[0]) {
						case 'a':	/* Angle */
							if (p[1] == '+' || p[1] == '\0') { Ctrl->F.read[Ctrl->F.nread] = p[0]; Ctrl->F.nread++; }
							else Ctrl->F.angle = atof (&p[1]);
							break;
						case 'c':	/* -R corner justification */
							if (p[1] == '+' || p[1] == '\0') { Ctrl->F.read[Ctrl->F.nread] = p[0]; Ctrl->F.nread++; }
							else {
								Ctrl->F.R_justify = GMT_just_decode (GMT, &p[1], 12);
								if (!explicit_justify)	/* If not set explicitly, default to same justification as corner */
									Ctrl->F.justify = Ctrl->F.R_justify;
							}
							break;
						case 'f':	/* Font info */
							if (p[1] == '+' || p[1] == '\0') { Ctrl->F.read[Ctrl->F.nread] = p[0]; Ctrl->F.nread++; }
							else n_errors += GMT_getfont (GMT, &p[1], &(Ctrl->F.font));
							break;
						case 'j':	/* Justification */
							if (p[1] == '+' || p[1] == '\0') { Ctrl->F.read[Ctrl->F.nread] = p[0]; Ctrl->F.nread++; }
							else {
								Ctrl->F.justify = GMT_just_decode (GMT, &p[1], 12);
								explicit_justify = true;
							}
							break;
						case 'l':	/* Segment label request */
							if (Ctrl->F.get_text) {
								GMT_Report (API, GMT_MSG_COMPAT, "Error: Only one of +l and +h can be selected in -F.\n");
								n_errors++;
							}
							else
								Ctrl->F.get_text = GET_SEG_LABEL;
							break;
						case 'h':	/* Segment header request */
							if (Ctrl->F.get_text) {
								GMT_Report (API, GMT_MSG_COMPAT, "Error: Only one of +l and +h can be selected in -F.\n");
								n_errors++;
							}
							else
								Ctrl->F.get_text = GET_SEG_HEADER;
							break;
						default:
							n_errors++;
							break;
					}
				}
				break;
			case 'G':
				Ctrl->G.active = true;
				if (opt->arg[0] == 'c' && !opt->arg[1])
					Ctrl->G.mode = true;
				else if (GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'L':
				Ctrl->L.active = true;
				break;
			case 'm':
				if (GMT_compat_check (GMT, 4)) /* Warn and pass through */
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -m option is deprecated and reverted back to -M to indicate paragraph mode.\n");
				else
					n_errors += GMT_default_error (GMT, opt->option);
			case 'M':	/* Paragraph mode */
				Ctrl->M.active = true;
				break;
			case 'N':	/* Do not clip at border */
				Ctrl->N.active = true;
				break;
			case 'S':
				if (GMT_compat_check (GMT, 4)) { /* Warn and pass through */
					GMT_Report (API, GMT_MSG_COMPAT, "Warning: -S option is deprecated; use font pen setting instead.\n");
					Ctrl->S.active = true;
					if (GMT_getpen (GMT, opt->arg, &Ctrl->S.pen)) {
						GMT_pen_syntax (GMT, 'S', "draws outline of characters.  Append pen attributes [Default pen is %s]");
						n_errors++;
					}
				}
				else
					n_errors += GMT_default_error (GMT, opt->option);
				break;
			case 'Q':
				Ctrl->Q.active = true;
				if (opt->arg[0] == 'l') Ctrl->Q.mode = -1;
				if (opt->arg[0] == 'u') Ctrl->Q.mode = +1;
				break;
			case 'T':
				Ctrl->T.active = true;
				Ctrl->T.mode = opt->arg[0];
				n_errors += GMT_check_condition (GMT, !strchr("oOcC", Ctrl->T.mode), "Syntax error -T option: must add o, O, c, or C\n");
				break;
			case 'W':
				Ctrl->W.active = true;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', "draws a box around the text with the specified pen [Default pen is %s]");
					n_errors++;
				}
				break;
			case 'Z':
				/* For backward compatibility we will see -Z+ as the current -Z
				 * and -Z<level> as an alternative to -p<az>/<el>/<level> */
				if (opt->arg[0] == '+' && !opt->arg[1])
					Ctrl->Z.active = true;
				else if (opt->arg[0])
					GMT->current.proj.z_level = atof(opt->arg);
				else
					Ctrl->Z.active = true;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	n_errors += GMT_check_condition (GMT, !Ctrl->L.active && !GMT->common.R.active, "Syntax error: Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->L.active && !GMT->common.J.active, "Syntax error: Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.dx < 0.0 || Ctrl->C.dy < 0.0, "Syntax error -C option: clearances cannot be negative!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.dx == 0.0 && Ctrl->C.dy == 0.0 && Ctrl->T.mode && Ctrl->T.mode != 'o', "Warning: non-rectangular text boxes require a non-zero -C\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && !Ctrl->G.active && !Ctrl->W.active, "Warning: -T requires -W and/or -G\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.dx == 0.0 && Ctrl->D.dy == 0.0 && Ctrl->D.line, "Warning: -D<x/y>v requires one nonzero <x/y>\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && abs (Ctrl->Q.mode) > 1, "Syntax error -Q option: Use l or u for lower/upper-case.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.mode && Ctrl->M.active, "Syntax error -Gc option: Cannot be used with -M.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.mode && Ctrl->W.active, "Syntax error -Gc option: Cannot be used with -W.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.mode && Ctrl->D.line, "Syntax error -Gc option: Cannot be used with -D...v<pen>.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->M.active && Ctrl->F.get_text, "Syntax error -M option: Cannot be used with -F...+l|h.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int validate_coord_and_text (struct GMT_CTRL *GMT, struct PSTEXT_CTRL *Ctrl, int rec_no, char *record, char buffer[])
{	/* Parse x,y [and z], check for validity, and return the rest of the text in buffer */
	int ix, iy, nscan = 0;
	unsigned int pos = 0;
	char txt_x[GMT_LEN256] = {""}, txt_y[GMT_LEN256] = {""}, txt_z[GMT_LEN256] = {""};

	ix = (GMT->current.setting.io_lonlat_toggle[GMT_IN]);	iy = 1 - ix;
	buffer[0] = '\0';	/* Initialize buffer to NULL */

	if (Ctrl->Z.active) {	/* Expect z in 3rd column */
		if (GMT_strtok (record, " \t,", &pos, txt_x)) nscan++;	/* Returns xcol and update pos */
		if (GMT_strtok (record, " \t,", &pos, txt_y)) nscan++;	/* Returns ycol and update pos */
		if (GMT_strtok (record, " \t,", &pos, txt_z)) nscan++;	/* Returns zcol and update pos */
		strcpy (buffer, &record[pos]);
		sscanf (&record[pos], "%[^\n]\n", buffer);	nscan++;	/* Since sscanf could return -1 if nothing we increment nscan always */
		if ((GMT_scanf (GMT, txt_z, GMT->current.io.col_type[GMT_IN][GMT_Z], &GMT->current.io.curr_rec[GMT_Z]) == GMT_IS_NAN)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Record %d had bad z coordinate, skipped)\n", rec_no);
			return (-1);
		}
	}
	else if (Ctrl->F.R_justify) {
		int i, j;

		i = Ctrl->F.R_justify % 4;		/* See GMT_just_decode in gmt_support.c */
		j = Ctrl->F.R_justify / 4;
		if (i == 1)
			GMT->current.io.curr_rec[ix] = GMT->common.R.wesn[XLO];
		else if (i == 2)
			GMT->current.io.curr_rec[ix] = (GMT->common.R.wesn[XLO] + GMT->common.R.wesn[XHI]) / 2;
		else
			GMT->current.io.curr_rec[ix] = GMT->common.R.wesn[XHI];

		if (j == 0)
			GMT->current.io.curr_rec[iy] = GMT->common.R.wesn[YLO];
		else if (j == 1)
			GMT->current.io.curr_rec[iy] = (GMT->common.R.wesn[YLO] + GMT->common.R.wesn[YHI]) / 2;
		else
			GMT->current.io.curr_rec[iy] = GMT->common.R.wesn[YHI];

		nscan = 2;	/* Since x,y are implicit */
		nscan += sscanf (record, "%[^\n]\n", buffer);
		GMT->current.io.curr_rec[GMT_Z] = GMT->current.proj.z_level;
	}
	else {
		if (GMT_strtok (record, " \t,", &pos, txt_x)) nscan++;	/* Returns xcol and update pos */
		if (GMT_strtok (record, " \t,", &pos, txt_y)) nscan++;	/* Returns ycol and update pos */
		sscanf (&record[pos], "%[^\n]\n", buffer);	nscan++;	/* Since sscanf could return -1 if nothing we increment nscan always */
		GMT->current.io.curr_rec[GMT_Z] = GMT->current.proj.z_level;
	}

	if (!Ctrl->F.R_justify) {
		if (GMT_scanf (GMT, txt_x, GMT->current.io.col_type[GMT_IN][GMT_X], &GMT->current.io.curr_rec[ix]) == GMT_IS_NAN) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Record %d had bad x coordinate, skipped)\n", rec_no);
			return (-1);
		}
		if (GMT_scanf (GMT, txt_y, GMT->current.io.col_type[GMT_IN][GMT_Y], &GMT->current.io.curr_rec[iy]) == GMT_IS_NAN) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Record %d had bad y coordinate, skipped)\n", rec_no);
			return (-1);
		}
	}
	return (nscan);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_pstext_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_pstext (void *V_API, int mode, void *args)
{	/* High-level function that implements the pstext task */

	int  error = 0, k, fmode, nscan;
	bool master_record = false, skip_text_records = false, old_is_world;
	
	unsigned int length = 0, n_paragraphs = 0, n_add, m = 0, pos, text_col;
	unsigned int n_read = 0, n_processed = 0, txt_alloc = 0, add, n_expected_cols;
	
	size_t n_alloc = 0;

	double plot_x = 0.0, plot_y = 0.0, save_angle = 0.0, xx[2] = {0.0, 0.0}, yy[2] = {0.0, 0.0}, *in = NULL;
	double offset[2], tmp, *c_x = NULL, *c_y = NULL, *c_angle = NULL;

	char text[GMT_BUFSIZ] = {""}, buffer[GMT_BUFSIZ] = "", label[GMT_BUFSIZ] = {""}, pjust_key[5] = {""}, txt_a[GMT_LEN256] = {""}, txt_b[GMT_LEN256] = {""};
	char *paragraph = NULL, *line = NULL, *curr_txt = NULL, *in_txt = NULL, **c_txt = NULL;
	char this_size[GMT_LEN256] = {""}, this_font[GMT_LEN256] = {""}, just_key[5] = {""}, txt_f[GMT_LEN256] = {""};
	int input_format_version = GMT_NOTSET;
	struct PSTEXT_INFO T;
	struct PSTEXT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_pstext_usage (API, GMT_MODULE_PURPOSE, false));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_pstext_usage (API, GMT_USAGE, false));	/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_pstext_usage (API, GMT_SYNOPSIS, false));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_pstext_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pstext_parse (GMT, Ctrl, options))) Return (error);
	if (Ctrl->L.active) Return (GMT_pstext_usage (API, GMT_SYNOPSIS, true));	/* Return the synopsis with font listing */

	/*---------------------------- This is the pstext main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input text table data\n");
	load_parameters_pstext (&T, Ctrl);	/* Pass info from Ctrl to T */

#if 0
	if (!Ctrl->F.active) Ctrl->F.nread = 4;	/* Need to be backwards compatible */
#endif
	n_expected_cols = 3 + Ctrl->Z.active + Ctrl->F.nread;
	if (Ctrl->M.active) n_expected_cols += 3;
	if (Ctrl->F.get_text) n_expected_cols--;	/* No text in the input record */
	add = !(T.x_offset == 0.0 && T.y_offset == 0.0);
	if (add && Ctrl->D.justify) T.boxflag |= 64;

	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	if (Ctrl->G.mode) GMT->current.ps.nclip = +1;	/* Signal that this program initiates clipping that will outlive this process */
	
	PSL = GMT_plotinit (GMT, options);

	GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);
	GMT_plotcanvas (GMT);	/* Fill canvas if requested */

	if (!(Ctrl->N.active || Ctrl->Z.active || Ctrl->G.mode)) GMT_map_clip_on (GMT, GMT->session.no_rgb, 3);

	in = GMT->current.io.curr_rec;
	text_col = n_expected_cols - 1;

	old_is_world = GMT->current.map.is_world;
	GMT->current.map.is_world = true;

	if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_OK) {	/* Register data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_IN, GMT_HEADER_ON) != GMT_OK) {	/* Enables data input and sets access mode */
		Return (API->error);
	}

	if (Ctrl->G.mode) {
		n_alloc = 0;
		GMT_malloc3 (GMT, c_angle, c_x, c_y, GMT_SMALL_CHUNK, &n_alloc, double);
		c_txt = GMT_memory (GMT, NULL, n_alloc, char *);
	}
	
	do {	/* Keep returning records until we have no more files */
		if ((line = GMT_Get_Record (API, GMT_READ_TEXT, NULL)) == NULL) {	/* Keep returning records until we have no more files */
			if (GMT_REC_IS_ERROR (GMT)) {
				Return (EXIT_FAILURE);
			}
			if (GMT_REC_IS_TABLE_HEADER (GMT)) {
				continue;	/* Skip table headers */
			}
			if (GMT_REC_IS_EOF (GMT)) 		/* Reached end of file */
				break;
		}

		/* Data record or segment header to process */

		if (Ctrl->M.active) {	/* Paragraph mode */
			if (GMT_REC_IS_SEGMENT_HEADER (GMT)) {
				line = GMT->current.io.segment_header;
				skip_text_records = false;
				if (n_processed) {	/* Must output what we got */
					GMT_putwords (GMT, PSL, plot_x, plot_y, paragraph, &T);
					n_processed = length = 0;
					paragraph[0] = 0;	/* Empty existing text */
					n_paragraphs++;
				}

				if ((nscan = validate_coord_and_text (GMT, Ctrl, n_read, line, buffer)) == -1) continue;	/* Failure */
				
				pos = 0;

				if (GMT_compat_check (GMT, 4)) {
					if (input_format_version == GMT_NOTSET) input_format_version = get_input_format_version (GMT, buffer, 1);
				}
				if (input_format_version == 4) {	/* Old-style GMT 4 records */
					nscan += sscanf (buffer, "%s %lf %s %s %s %s %s\n", this_size, &T.paragraph_angle, this_font, just_key, txt_a, txt_b, pjust_key);
					T.block_justify = GMT_just_decode (GMT, just_key, 12);
					T.line_spacing = GMT_to_inch (GMT, txt_a);
					T.paragraph_width  = GMT_to_inch (GMT, txt_b);
					T.text_justify = (pjust_key[0] == 'j') ? PSL_JUST : GMT_just_decode (GMT, pjust_key, 0);
					sprintf (txt_f, "%s,%s,", this_size, this_font);	/* Merge size and font to be parsed by GMT_getfont */
					T.font = Ctrl->F.font;
					if (GMT_getfont (GMT, txt_f, &T.font)) GMT_Report (API, GMT_MSG_NORMAL, "Record %d had bad font (set to %s)\n", n_read, GMT_putfont (GMT, T.font));
					in_txt = NULL;
					n_expected_cols = 9 + Ctrl->Z.active;
				}
				else if (!Ctrl->F.nread)	/* All attributes given via -F (or we accept defaults); skip to paragraph attributes */
					in_txt = buffer;
				else {	/* Must pick up 1-3 attributes from data file */
					for (k = 0; k < Ctrl->F.nread; k++) {
						nscan += GMT_strtok (buffer, " \t", &pos, text);
						switch (Ctrl->F.read[k]) {
							case 'f':
								T.font = Ctrl->F.font;
								if (GMT_getfont (GMT, text, &T.font)) GMT_Report (API, GMT_MSG_NORMAL, "Record %d had bad font (set to %s)\n", n_read, GMT_putfont (GMT, T.font));
								break;
							case 'a':
								T.paragraph_angle = atof (text);
								break;
							case 'j':
								T.block_justify = GMT_just_decode (GMT, text, 12);
								break;
						}
					}
					in_txt = &buffer[pos];
				}

				if (in_txt) {	/* Get the remaining parameters */
					nscan += sscanf (in_txt, "%s %s %s\n", txt_a, txt_b, pjust_key);
					T.text_justify = (pjust_key[0] == 'j') ? PSL_JUST : GMT_just_decode (GMT, pjust_key, 0);
					T.line_spacing = GMT_to_inch (GMT, txt_a);
					T.paragraph_width  = GMT_to_inch (GMT, txt_b);
				}
				if (T.block_justify == -99) {
					GMT_Report (API, GMT_MSG_NORMAL, "Record %d had bad justification info (set to LB)\n", n_read);
					T.block_justify = 1;
				}
				if (nscan < (int)n_expected_cols) {
					GMT_Report (API, GMT_MSG_NORMAL, "Record %d had incomplete paragraph information, skipped)\n", n_read);
					continue;
				}
				GMT_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y);
				if (!Ctrl->N.active) {
					skip_text_records = true;	/* If this record should be skipped we must skip the whole paragraph */
					GMT_map_outside (GMT, in[GMT_X], in[GMT_Y]);
					if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
					skip_text_records = false;	/* Since we got here we do not want to skip */
				}
				if (Ctrl->A.active) {
					save_angle = T.paragraph_angle;	/* Since we might overwrite the default */
					tmp = GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, save_angle);
					T.paragraph_angle = fmod (tmp + 360.0 + 90.0, 180.0) - 90.0;	/* Ensure usable angles for text plotting */
					if (fabs (T.paragraph_angle - tmp) > 179.0) T.block_justify = 4 * (T.block_justify/4) + 2 - (T.block_justify%4 - 2);	/* Flip any L/R code */
				}
				master_record = true;
			}
			else {	/* Text block record */
				if (skip_text_records) continue;	/* Skip all records for this paragraph */
				if (!master_record) {
					GMT_Report (API, GMT_MSG_NORMAL, "Text record line %d not preceded by paragraph information, skipped)\n", n_read);
					continue;
				}
				GMT_chop (line);	/* Chop of line feed */
				GMT_enforce_rgb_triplets (GMT, line, GMT_BUFSIZ);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */

				if (line[0] == 0) {	/* Blank line marked by single NULL character, replace by \r */
					n_add = 1;
					while ((length + n_add) > txt_alloc) {
						txt_alloc += GMT_BUFSIZ;
						paragraph = GMT_memory (GMT, paragraph, txt_alloc, char);
					}
					strcat (paragraph, "\r");
				}
				else {
					if (Ctrl->Q.active) GMT_str_setcase (GMT, line, Ctrl->Q.mode);
					n_add = (int)strlen (line) + 1;
					while ((length + n_add) > txt_alloc) {
						txt_alloc += GMT_BUFSIZ;
						paragraph = GMT_memory (GMT, paragraph, txt_alloc, char);
					}
					if (length) strcat (paragraph, " ");
					strcat (paragraph, line);

				}
				length += n_add;
				n_processed++;
			}
			n_read++;
		}
		else {	/* Plain style pstext input */
			if (GMT_REC_IS_SEGMENT_HEADER (GMT)) continue;	/* Skip segment headers */
			if (GMT_is_a_blank_line (line)) continue;	/* Skip blank lines or # comments */

			if ((nscan = validate_coord_and_text (GMT, Ctrl, n_read, line, buffer)) == -1) continue;	/* Failure */
			pos = 0;

			if (GMT_compat_check (GMT, 4)) {
				if (input_format_version == GMT_NOTSET) input_format_version = get_input_format_version (GMT, buffer, 0);
			}
			if (input_format_version == 4) {	/* Old-style GMT 4 records */
				nscan--; /* Since we have already counted "text" */
				nscan += sscanf (buffer, "%s %lf %s %s %[^\n]\n", this_size, &T.paragraph_angle, this_font, just_key, text);
				T.block_justify = GMT_just_decode (GMT, just_key, 12);
				sprintf (txt_f, "%s,%s,", this_size, this_font);	/* Merge size and font to be parsed by GMT_getfont */
				T.font = Ctrl->F.font;
				if (GMT_getfont (GMT, txt_f, &T.font)) GMT_Report (API, GMT_MSG_NORMAL, "Record %d had bad font (set to %s)\n", n_read, GMT_putfont (GMT, T.font));
				in_txt = text;
				n_expected_cols = 7 + Ctrl->Z.active;
			}
			else if (!Ctrl->F.nread)	/* All attributes given via -F (or we accept defaults); just need text */
				in_txt = buffer;
			else {	/* Must pick up 1-3 attributes from data file */
				for (k = 0; k < Ctrl->F.nread; k++) {
					nscan += GMT_strtok (buffer, " \t", &pos, text);
					switch (Ctrl->F.read[k]) {
						case 'a':
							T.paragraph_angle = atof (text);
							break;
						case 'f':
							T.font = Ctrl->F.font;
							if (GMT_getfont (GMT, text, &T.font)) GMT_Report (API, GMT_MSG_NORMAL, "Record %d had bad font (set to %s)\n", n_read, GMT_putfont (GMT, T.font));
							if (GMT_compat_check (GMT, 4)) {
								if (Ctrl->S.active) {
									T.font.form |= 2;
									T.font.pen = Ctrl->S.pen;
								}
							}
							break;
						case 'j':
							T.block_justify = GMT_just_decode (GMT, text, 12);
							break;
					}
				}
				if (Ctrl->F.get_text == GET_REC_TEXT) in_txt = &buffer[pos];
			}
			if (Ctrl->F.get_text == GET_SEG_HEADER) {
				if (GMT->current.io.segment_header[0] == 0)
					GMT_Report (API, GMT_MSG_NORMAL, "No active segment header to use; text is blank\n");
				strcpy (label, GMT->current.io.segment_header);
				in_txt = label;
			}
			else if (Ctrl->F.get_text == GET_SEG_LABEL) {
				if (!GMT_parse_segment_item (GMT, GMT->current.io.segment_header, "-L", label))
					GMT_Report (API, GMT_MSG_NORMAL, "No active segment label to use; text is blank\n");
				in_txt = label;
			}
			
			nscan += GMT_load_aspatial_string (GMT, GMT->current.io.OGR, text_col, in_txt);	/* Substitute OGR attribute if used */

			if (nscan < (int)n_expected_cols) {
				GMT_Report (API, GMT_MSG_NORMAL, "Record %d is incomplete (skipped)\n", n_read);
				continue;
			}
			if (T.block_justify == -99) {
				GMT_Report (API, GMT_MSG_NORMAL, "Record %d had bad justification info (set to LB)\n", n_read);
				T.block_justify = 1;
			}

			/* Here, in_text holds the text we wish to plot */
			
			GMT_enforce_rgb_triplets (GMT, in_txt, GMT_BUFSIZ);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
			if (Ctrl->Q.active) GMT_str_setcase (GMT, in_txt, Ctrl->Q.mode);
			n_read++;
			GMT_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y);
			xx[0] = plot_x;	yy[0] = plot_y;
			if (!Ctrl->N.active) {
				GMT_map_outside (GMT, in[GMT_X], in[GMT_Y]);
				if (abs (GMT->current.map.this_x_status) > 1 || abs (GMT->current.map.this_y_status) > 1) continue;
			}

			if (Ctrl->A.active) {
				save_angle = T.paragraph_angle;	/* Since we might overwrite the default */
				tmp = GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, save_angle);
				T.paragraph_angle = fmod (tmp + 360.0 + 90.0, 180.0) - 90.0;	/* Ensure usable angles for text plotting */
				if (fabs (T.paragraph_angle - tmp) > 179.0) T.block_justify = 4 * (T.block_justify/4) + 2 - (T.block_justify%4 - 2);	/* Flip any L/R code */
			}
			if (add) {
				if (Ctrl->D.justify)	/* Smart offset according to justification (from Dave Huang) */
					GMT_smart_justify (GMT, T.block_justify, T.paragraph_angle, T.x_offset, T.y_offset, &plot_x, &plot_y, Ctrl->D.justify);
				else {	/* Default hard offset */
					plot_x += T.x_offset;
					plot_y += T.y_offset;
				}
				xx[1] = plot_x;	yy[1] = plot_y;
			}
			n_paragraphs++;

			PSL_setfont (PSL, T.font.id);
			GMT_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, in[GMT_Z]);
			if (T.boxflag & 32) {	/* Draw line from original point to shifted location */
				GMT_setpen (GMT, &T.vecpen);
				PSL_plotsegment (PSL, xx[0], yy[0], xx[1], yy[1]);
			}
			if (!Ctrl->G.mode && T.boxflag & 3) {	/* Plot the box beneath the text */
				if (T.space_flag) {	/* Meant % of fontsize */
					offset[0] = 0.01 * T.x_space * T.font.size / PSL_POINTS_PER_INCH;
					offset[1] = 0.01 * T.y_space * T.font.size / PSL_POINTS_PER_INCH;
				}
				else {
					offset[0] = T.x_space;
					offset[1] = T.y_space;
				}
				GMT_setpen (GMT, &T.boxpen);			/* Box pen */
				PSL_setfill (PSL, T.boxfill.rgb, T.boxflag & 1);	/* Box color */
				PSL_plottextbox (PSL, plot_x, plot_y, T.font.size, in_txt, T.paragraph_angle, T.block_justify, offset, T.boxflag & 4);
				curr_txt = NULL;	/* Text has now been encoded in the PS file */
			}
			else
				curr_txt = in_txt;
			fmode = GMT_setfont (GMT, &T.font);
			if (Ctrl->G.mode) {
				if (m <= n_alloc) {
					GMT_malloc3 (GMT, c_angle, c_x, c_y, m, &n_alloc, double);
					c_txt = GMT_memory (GMT, c_txt, n_alloc, char *);
				}
				c_angle[m] = T.paragraph_angle;
				c_txt[m] = strdup (curr_txt);
				c_x[m] = plot_x;
				c_y[m] = plot_y;
				m++;
			}
			else {	
				PSL_plottext (PSL, plot_x, plot_y, T.font.size, curr_txt, T.paragraph_angle, T.block_justify, fmode);
			}
			if (Ctrl->A.active) T.paragraph_angle = save_angle;	/* Restore original angle */
		}

	} while (true);
	
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_OK) {	/* Disables further data input */
		Return (API->error);
	}

	if (Ctrl->M.active) {
		if (n_processed) {	/* Must output the last paragraph */
			GMT_putwords (GMT, PSL, plot_x, plot_y, paragraph, &T);
			n_paragraphs++;
		}
	 	GMT_free (GMT, paragraph);
	}
	if (Ctrl->G.mode && m) {
		int form;
		unsigned int kk;
		GMT_textpath_init (GMT, &Ctrl->W.pen, Ctrl->W.pen.rgb, &Ctrl->W.pen, Ctrl->G.fill.rgb);
		form = (T.boxflag & 4) ? 16 : 0;
		if (Ctrl->C.percent) {	/* Meant % of fontsize */
			offset[0] = 0.01 * T.x_space * T.font.size / PSL_POINTS_PER_INCH;
			offset[1] = 0.01 * T.y_space * T.font.size / PSL_POINTS_PER_INCH;
		}
		else {
			offset[0] = T.x_space;
			offset[1] = T.y_space;
		}
		PSL_plottextclip (PSL, c_x, c_y, m, T.font.size, c_txt, c_angle, T.block_justify, offset, form);	/* This turns clipping ON */
		for (kk = 0; kk < m; kk++) free (c_txt[kk]);
		GMT_free (GMT, c_angle);
		GMT_free (GMT, c_x);
		GMT_free (GMT, c_y);
		GMT_free (GMT, c_txt);
	}
	else if (!(Ctrl->N.active || Ctrl->Z.active)) GMT_map_clip_off (GMT);

	GMT->current.map.is_world = old_is_world;

	GMT_map_basemap (GMT);
	GMT_plane_perspective (GMT, -1, 0.0);
	GMT_plotend (GMT);

	GMT_Report (API, GMT_MSG_VERBOSE, Ctrl->M.active ? "pstext: Plotted %d text blocks\n" : "pstext: Plotted %d text strings\n", n_paragraphs);

	Return (GMT_OK);
}
