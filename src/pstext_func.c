/*--------------------------------------------------------------------
 *	$Id: pstext_func.c,v 1.9 2011-03-26 18:34:17 guru Exp $
 *
 *	Copyright (c) 1991-2011 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 of the License.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
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

#include "pslib.h"
#include "gmt.h"

#define PSTEXT_CLIP		1
#define PSTEXT_PLOT		2
#define PSTEXT_TERMINATE	3

struct PSTEXT_CTRL {
	struct A {	/* -A */
		GMT_LONG active;
	} A;
	struct C {	/* -C<dx>/<dy> */
		GMT_LONG active;
		GMT_LONG percent;
		double dx, dy;
	} C;
	struct D {	/* -D[j]<dx>/<dy>[v[<pen>] */
		GMT_LONG active;
		GMT_LONG justify;
		GMT_LONG line;
		double dx, dy;
		struct GMT_PEN pen;
	} D;
	struct F {	/* -F[+f<fontinfo>+a<angle>+j<justification>] */
		GMT_LONG active;
		struct GMT_FONT font;
		double angle;
		GMT_LONG justify, nread;
		char read[3];	/* Contains f, a, and/or j in order required to be read from input */
	} F;
	struct G {	/* -G<fill> */
		GMT_LONG active;
		GMT_LONG mode;
		struct GMT_FILL fill;
	} G;
	struct L {	/* -L */
		GMT_LONG active;
	} L;
	struct M {	/* -M */
		GMT_LONG active;
	} M;
	struct N {	/* -N */
		GMT_LONG active;
	} N;
	struct Q {	/* -Q<case> */
		GMT_LONG active;
		GMT_LONG mode;	/* 0 = do nothing, -1 = force lower case, +1 = force upper case */
	} Q;
	struct S {	/* -S<pen> */
		GMT_LONG active;
		struct GMT_PEN pen;
	} S;
	struct T {	/* -To|O|c|C */
		GMT_LONG active;
		char mode;
	} T;
	struct W {	/* -W[<pen>] */
		GMT_LONG active;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z<z_level> */
		GMT_LONG active;
	} Z;
};

struct PSTEXT_INFO {
	GMT_LONG text_justify;
	GMT_LONG block_justify;
	GMT_LONG boxflag;
	GMT_LONG space_flag;
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

EXTERN_MSC void GMT_enforce_rgb_triplets (struct GMT_CTRL *C, char *text, GMT_LONG size);

void *New_pstext_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSTEXT_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PSTEXT_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */

	C->D.pen = C->S.pen = C->W.pen = GMT->current.setting.map_default_pen;
	C->C.dx = C->C.dy = 15.0;	/* 15% of font size is default clearance */
	C->C.percent = TRUE;
	C->F.justify = 6;	/* CM */
	C->F.font = GMT->current.setting.font_annot[0];		/* Default font */
	GMT_init_fill (GMT, &C->G.fill, -1.0, -1.0, -1.0);	/* No fill */

	return ((void *)C);
}

void Free_pstext_Ctrl (struct GMT_CTRL *GMT, struct PSTEXT_CTRL *C) {	/* Deallocate control structure */
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
	else {
		offset[0] = T->x_space;
		offset[1] = T->y_space;
	}

	/* Set some paragraph parameters */
	PSL_setparagraph (PSL, T->line_spacing, T->paragraph_width, T->text_justify);
	PSL_setfont (PSL, T->font.id);

	if (T->boxflag & 32) {	/* Need to draw a vector from (x,y) to the offset text */
		GMT_setpen (GMT, PSL, &(T->vecpen));
		PSL_plotsegment (PSL, x, y, x + T->x_offset, y + T->y_offset);
	}
	x += T->x_offset;	y += T->y_offset;	/* Move to the actual reference point */
	if (T->boxflag) {	/* Need to lay down the box first, then place text */
		GMT_LONG mode;
		if (T->boxflag & 1) GMT_setpen (GMT, PSL, &(T->boxpen));			/* Change current pen */
		if (T->boxflag & 2) GMT_setfill (GMT, PSL, &(T->boxfill), T->boxflag & 1);	/* Change curent fill */
		if (T->boxflag & 1) mode = PSL_RECT_STRAIGHT;	/* Set the correct box shape */
		if (T->boxflag & 4) mode = PSL_RECT_ROUNDED;
		if (T->boxflag & 8) mode = PSL_RECT_CONCAVE;
		if (T->boxflag & 16) mode = PSL_RECT_CONVEX;
		/* Compute text box, draw/fill it, and in the process store the text in the PS file for next command */
		PSL_plotparagraphbox (PSL, x, y, T->font.size, text, T->paragraph_angle, T->block_justify, offset, mode);
		/* Passing NULL means we typeset using the last stored paragraph info */
		GMT_setfont (GMT, PSL, &T->font);
		PSL_plotparagraph (PSL, x, y, T->font.size, NULL, T->paragraph_angle, T->block_justify);
	}
	else {	/* No box beneath */
		GMT_setfont (GMT, PSL, &T->font);
		PSL_plotparagraph (PSL, x, y, T->font.size, text, T->paragraph_angle, T->block_justify);
	}
}

void load_parameters_pstext (struct GMT_CTRL *GMT, struct PSTEXT_INFO *T, struct PSTEXT_CTRL *C)
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

#ifdef GMT_COMPAT
GMT_LONG check_for_old_format (struct GMT_CTRL *C, char *buffer, GMT_LONG mode)
{
	/* Try to determine if input is the old GMT4-style format.
	 * mode = 0 means normal textrec, mode = 1 means paragraph mode. */
	
	GMT_LONG n, k;
	char size[GMT_LONG_TEXT], angle[GMT_LONG_TEXT], font[GMT_LONG_TEXT], just[GMT_LONG_TEXT], txt[BUFSIZ];
	char spacing[GMT_LONG_TEXT], width[GMT_LONG_TEXT], pjust[GMT_LONG_TEXT];
	
	if (mode) {	/* Paragraph control record */
		n = sscanf (buffer, "%s %s %s %s %s %s %s\n", size, angle, font, just, spacing, width, pjust);
		if (n < 7) return (FALSE);	/* Clearly not the old format since missing items */
	}
	else {		/* Regular text record */
		n = sscanf (buffer, "%s %s %s %s %[^\n]", size, angle, font, just, txt);
		if (n < 5) return (FALSE);	/* Clearly not the old format since missing items */
	}
	if (GMT_not_numeric (C, angle)) return (FALSE);	/* Since angle is not a number */
	k = strlen (size) - 1;
	if (size[k] == 'c' || size[k] == 'i' || size[k] == 'm' || size[k] == 'p') size[k] = '\0';	/* Chop of unit */
	if (GMT_not_numeric (C, size)) return (FALSE);	/* Since size is not a number */
	if (GMT_just_decode (C, just, 12) == -99) return (FALSE);	/* Since justify not in correct format */
	if (mode) {	/* A few more checks for paragraph mode */
		k = strlen (spacing) - 1;
		if (spacing[k] == 'c' || spacing[k] == 'i' || spacing[k] == 'm' || spacing[k] == 'p') spacing[k] = '\0';	/* Chop of unit */
		if (GMT_not_numeric (C, spacing)) return (FALSE);	/* Since spacing is not a number */
		k = strlen (width) - 1;
		if (width[k] == 'c' || width[k] == 'i' || width[k] == 'm' || width[k] == 'p') width[k] = '\0';	/* Chop of unit */
		if (GMT_not_numeric (C, width)) return (FALSE);		/* Since width is not a number */
		if (!(pjust[0] == 'j' && pjust[1] == '\0') && GMT_just_decode (C, pjust, 0) == -99) return (FALSE);
	}

	/* Well, seems like the old format so far */
	GMT_report (C, GMT_MSG_COMPAT, "Warning: use of old style pstext input is deprecated.\n");
	return (TRUE);
}
#endif

GMT_LONG GMT_pstext_usage (struct GMTAPI_CTRL *C, GMT_LONG level, GMT_LONG show_fonts)
{
	struct GMT_CTRL *GMT = C->GMT;

	/* This displays the pstext synopsis and optionally full usage information */

	GMT_message (GMT, "pstext %s [API] - To plot text on maps\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: pstext <txtfile> %s %s\n", GMT_J_OPT, GMT_Rgeoz_OPT);
	GMT_message (GMT, "\t[-A] [%s] [-C<dx>/<dy>] [-D[j]<dx>[/<dy>][v[<pen>]]\n", GMT_B_OPT);
	GMT_message (GMT, "\t[-F[a+<angle>][+f<font>][+j<justify>]] [-G<fill>] [%s] [-K] [-L]\n", GMT_Jz_OPT);
	GMT_message (GMT, "\t[-M] [-N] [-O] [-P] [-Q<case>] [-To|O|c|C] [%s]\n", GMT_U_OPT);
	GMT_message (GMT, "\t[%s] [-W[<fill>] [%s] [%s]\n", GMT_V_OPT, GMT_X_OPT, GMT_Y_OPT);
	GMT_message (GMT, "\t[-Z[<zlevel>|+]] [%s] [%s] [%s] [%s]\n", GMT_a_OPT, GMT_c_OPT, GMT_f_OPT, GMT_h_OPT);
	GMT_message (GMT, "\t[%s] [%s] [%s]\n\n", GMT_p_OPT, GMT_t_OPT, GMT_colon_OPT);
	GMT_message (GMT, "\tReads (x,y[,fontinfo,angle,justify],text) from <txtfile> [or stdin]\n");
	GMT_message (GMT, "\tOR (with -M) one or more text paragraphs with formatting info in the segment header.\n");
	GMT_message (GMT, "\tBuilt-in escape sequences:\n");
	GMT_message (GMT, "\t   @~ toggles between current font and Symbol font\n");
	GMT_message (GMT, "\t   @%%<no>%% switches to font number <no>; @%%%% resets font\n");
	GMT_message (GMT, "\t   @:<size>: switches font size; @:: resets font size\n");
	GMT_message (GMT, "\t   @;<color>; switches font color; @;; resets font color\n");
	GMT_message (GMT, "\t   @+ toggles between normal and superscript mode\n");
	GMT_message (GMT, "\t   @- toggles between normal and subscript mode\n");
	GMT_message (GMT, "\t   @# toggles between normal and Small Caps mode\n");
	GMT_message (GMT, "\t   @_ toggles between normal and underlined text\n");
	GMT_message (GMT, "\t   @!<char1><char2> makes one composite character\n");
	GMT_message (GMT, "\t   @@ prints the @ sign itself\n");
	GMT_message (GMT, "\t   Use @a, @c, @e, @n, @o, @s, @u, @A, @C @E, @N, @O, @U for accented European characters\n");
	GMT_message (GMT, "\t(See manual page for more information)\n");

	if (show_fonts) {	/* List fonts */
		GMT_LONG i;
		GMT_message (GMT, "\n\tFont #	Font Name\n");
		GMT_message (GMT, "\t------------------------------------\n");
		for (i = 0; i < GMT->session.n_fonts; i++)
			GMT_message (GMT, "\t%3ld\t%s\n", i, GMT->session.font[i].name);
	}

	if (show_fonts) return (EXIT_SUCCESS);
	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_explain_options (GMT, "jZR");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Angles given as azimuths; convert to directions using current projection\n");
	GMT_explain_options (GMT, "b");
	GMT_message (GMT, "\t-C Sets the clearance between characters and surrounding box.  Only used\n");
	GMT_message (GMT, "\t   if -W has been set.  Append units {%s} or %% of fontsize [15%%]\n", GMT_DIM_UNITS_DISPLAY);
	GMT_message (GMT, "\t-D Adds <add_x>,<add_y> to the text origin AFTER projecting with -J. [0/0]\n");
	GMT_message (GMT, "\t   Use -Dj to move text origin away from point (direction determined by text's justification)\n");
	GMT_message (GMT, "\t   Append v[<pen>] to draw line from text to original point.  If <add_y> is not given it equal <add_x>\n");
	GMT_message (GMT, "\t-F Specify values for text attributes that apply to all text records:\n");
	GMT_message (GMT, "\t   +a<angle> specifies the baseline angle for all text [0]\n");
	GMT_message (GMT, "\t   +f<fontinfo> sets the size, font, and optionally the text color [%s]\n", GMT_putfont (GMT, GMT->current.setting.font_annot[0]));
	GMT_message (GMT, "\t   +j<justify> sets text justification relative to given (x,y) coordinate.\n");
	GMT_message (GMT, "\t     Give a 2-char combo from [T|M|B][L|C|R] (top/middle/bottom/left/center/right) [CM]\n");
	GMT_message (GMT, "\t   If an attribute +f|+a|+j is not followed by a value we read the information from the\n");
	GMT_message (GMT, "\t   data file in the order given on the -F option.\n");
	GMT_message (GMT, "\t-G Paints the box underneath the text with specified color [Default is no paint]\n");
	GMT_message (GMT, "\t   Alternatively, append c to set clip paths based on text (and -C).  No text is plotted.\n");
	GMT_message (GMT, "\t   See psclip -Ct to plot the hidden text.  Cannot be used with paragraph mode (-M).\n");
	GMT_explain_options (GMT, "K");
	GMT_message (GMT, "\t-L lists the font-numbers and font-names available, then exits.\n");
	GMT_message (GMT, "\t-M Paragraph text mode [Default is single item mode]\n");
	GMT_message (GMT, "\t   Expects (x y size angle fontno justify linespace parwidth parjust) in segment header\n");
	GMT_message (GMT, "\t   followed by lines with one or more paragraphs of text.\n");
	GMT_message (GMT, "\t   parjust is one of (l)eft, (c)enter, (r)ight, or (j)ustified.\n");
	GMT_message (GMT, "\t-N Do Not clip text that exceeds the map boundaries [Default will clip]\n");
	GMT_explain_options (GMT, "OP");
	GMT_message (GMT, "\t-Q For all text to be (l)lower or (u)pper-case [Default leaves text as is].\n");
	GMT_message (GMT, "\t-T Sets shape of textbox when using -G and/or -W\n");
	GMT_message (GMT, "\t   Add o for rectangle [Default], O for rectangle with rounded corners,\n");
	GMT_message (GMT, "\t   c for concave rectangle, C for convex rectangle\n");
	GMT_explain_options (GMT, "UV");
	GMT_pen_syntax (GMT, 'W', "Draws a box around the text with the specified pen [Default pen is %s]");
	GMT_explain_options (GMT, "X");
	GMT_message (GMT, "\t-Z For 3-D plots: expect records to have a z value in the 3rd column (i.e., x y z size ...)\n");
	GMT_message (GMT, "\t   Note that -Z+ also sets -N\n");
	GMT_explain_options (GMT, "acfhpt:.");
	
	return (EXIT_FAILURE);
}

GMT_LONG GMT_pstext_parse (struct GMTAPI_CTRL *C, struct PSTEXT_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to pstext and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG j, k, pos, n_errors = 0;
	char txt_a[GMT_LONG_TEXT], txt_b[GMT_LONG_TEXT], p[BUFSIZ];
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Getting azimuths rather than directions, must convert via map projection */
				Ctrl->A.active = TRUE;
				break;
			case 'C':
				Ctrl->C.active = TRUE;
				if (opt->arg[0]) {	/* Replace default settings with user settings */
					Ctrl->C.percent = (strchr (opt->arg, '%')) ? TRUE : FALSE;
					k = sscanf (opt->arg, "%[^/]/%s", txt_a, txt_b);
					for (j = 0; txt_a[j]; j++) if (txt_a[j] == '%') txt_a[j] = '\0';	/* Remove % signs before processing values */
					for (j = 0; k == 2 && txt_b[j]; j++) if (txt_b[j] == '%') txt_b[j] = '\0';
					Ctrl->C.dx = GMT_to_inch (GMT, txt_a);
					Ctrl->C.dy = (k == 2) ? GMT_to_inch (GMT, txt_b) : Ctrl->C.dx;
				}
				break;
			case 'D':
				Ctrl->D.active = TRUE;
				k = 0;
				if (opt->arg[k] == 'j') Ctrl->D.justify = TRUE, k++;
				for (j = k; opt->arg[j] && opt->arg[j] != 'v'; j++);
				if (opt->arg[j] == 'v') {
					Ctrl->D.line = TRUE;
					n_errors += GMT_check_condition (GMT, opt->arg[j+1] && GMT_getpen (GMT, &opt->arg[j+1], &Ctrl->D.pen), "GMT SYNTAX ERROR -D option: Give pen after c\n");
					opt->arg[j] = 0;
				}
				j = sscanf (&opt->arg[k], "%[^/]/%s", txt_a, txt_b);
				Ctrl->D.dx = GMT_to_inch (GMT, txt_a);
				Ctrl->D.dy = (j == 2) ? GMT_to_inch (GMT, txt_b) : Ctrl->D.dx;
				break;
			case 'F':
				Ctrl->F.active = TRUE;
				pos = 0;
				
				while (GMT_getmodopt (opt->arg, "afj", &pos, p) && Ctrl->F.nread < 3) {	/* Looking for +f, +a, +j */
					switch (p[0]) {
						case 'f':	/* Font info */
							if (p[1] == '+' || p[1] == '\0') { Ctrl->F.read[Ctrl->F.nread] = p[0]; Ctrl->F.nread++; }
							else n_errors += GMT_getfont (GMT, &p[1], &(Ctrl->F.font));
							break;
						case 'a':	/* Angle */
							if (p[1] == '+' || p[1] == '\0') { Ctrl->F.read[Ctrl->F.nread] = p[0]; Ctrl->F.nread++; }
							else Ctrl->F.angle = atof (&p[1]);
							break;
						case 'j':	/* Justification */
							if (p[1] == '+' || p[1] == '\0') { Ctrl->F.read[Ctrl->F.nread] = p[0]; Ctrl->F.nread++; }
							else Ctrl->F.justify = GMT_just_decode (GMT, &p[1], 12);
							break;
						default:
							n_errors++;
							break;
					}
				}
				break;
			case 'G':
				Ctrl->G.active = TRUE;
				if (opt->arg[0] == 'c' && !opt->arg[1])
					Ctrl->G.mode = TRUE;
				else if (GMT_getfill (GMT, opt->arg, &Ctrl->G.fill)) {
					GMT_fill_syntax (GMT, 'G', " ");
					n_errors++;
				}
				break;
			case 'L':
				Ctrl->L.active = TRUE;
				break;
#ifdef GMT_COMPAT
			case 'm':
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -m option is deprecated and reverted back to -M to indicate paragraph mode.\n");
#endif
			case 'M':	/* Paragraph mode */
				Ctrl->M.active = TRUE;
				break;
			case 'N':	/* Do not clip at border */
				Ctrl->N.active = TRUE;
				break;
#ifdef GMT_COMPAT
			case 'S':
				GMT_report (GMT, GMT_MSG_COMPAT, "Warning: -S option is deprecated; use font pen setting instead.\n");
				Ctrl->S.active = TRUE;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->S.pen)) {
					GMT_pen_syntax (GMT, 'S', "draws outline of characters.  Append pen attributes [Default pen is %s]");
					n_errors++;
				}
				break;
#endif
			case 'Q':
				Ctrl->Q.active = TRUE;
				if (opt->arg[0] == 'l') Ctrl->Q.mode = -1;
				if (opt->arg[0] == 'u') Ctrl->Q.mode = +1;
				break;
			case 'T':
				Ctrl->T.active = TRUE;
				Ctrl->T.mode = opt->arg[0];
				n_errors += GMT_check_condition (GMT, !strchr("oOcC", Ctrl->T.mode), "GMT SYNTAX ERROR -T option:  must add o, O, c, or C\n");
				break;
			case 'W':
				Ctrl->W.active = TRUE;
				if (GMT_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
					GMT_pen_syntax (GMT, 'W', "draws a box around the text with the specified pen [Default pen is %s]");
					n_errors++;
				}
				break;
			case 'Z':
				/* For backward compatibility we will see -Z+ as the current -Z
				 * and -Z<level> as an alternative to -p<az>/<el>/<level> */
				if (opt->arg[0] == '+' && !opt->arg[1])
					Ctrl->Z.active = TRUE;
				else if (opt->arg[0])
					GMT->current.proj.z_level = atof(opt->arg);
				else
					Ctrl->Z.active = TRUE;
				break;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	/* Check that the options selected are mutually consistent */

	n_errors += GMT_check_condition (GMT, !Ctrl->L.active && !GMT->common.R.active, "GMT SYNTAX ERROR:  Must specify -R option\n");
	n_errors += GMT_check_condition (GMT, !Ctrl->L.active && !GMT->common.J.active, "GMT SYNTAX ERROR:  Must specify a map projection with the -J option\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.dx < 0.0 || Ctrl->C.dy < 0.0, "GMT SYNTAX ERROR -C option:  clearances cannot be negative!\n");
	n_errors += GMT_check_condition (GMT, Ctrl->C.dx == 0.0 && Ctrl->C.dy == 0.0 && Ctrl->T.mode && Ctrl->T.mode != 'o', "Warning: non-rectangular text boxes require a non-zero -C\n");
	n_errors += GMT_check_condition (GMT, Ctrl->T.active && !Ctrl->G.active && !Ctrl->W.active, "Warning: -T requires -W and/or -G\n");
	n_errors += GMT_check_condition (GMT, Ctrl->D.dx == 0.0 && Ctrl->D.dy == 0.0 && Ctrl->D.line, "Warning: -D<x/y>v requires one nonzero <x/y>\n");
	n_errors += GMT_check_condition (GMT, Ctrl->Q.active && GMT_abs (Ctrl->Q.mode) > 1, "GMT SYNTAX ERROR -Q option: Use l or u for lower/upper-case.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.mode && Ctrl->M.active, "GMT SYNTAX ERROR -Gc option: Cannot be used with -M.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.mode && Ctrl->W.active, "GMT SYNTAX ERROR -Gc option: Cannot be used with -W.\n");
	n_errors += GMT_check_condition (GMT, Ctrl->G.mode && Ctrl->D.line, "GMT SYNTAX ERROR -Gc option: Cannot be used with -D...v<pen>.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

GMT_LONG validate_coord_and_text (struct GMT_CTRL *GMT, GMT_LONG has_z, GMT_LONG rec_no, char *record, char buffer[])
{	/* Parse x,y [and z], check for validity, and return the rest of the text in buffer */
	GMT_LONG ix, iy, nscan;
	char txt_x[GMT_LONG_TEXT], txt_y[GMT_LONG_TEXT], txt_z[GMT_LONG_TEXT];

	if (has_z) {	/* Expect z in 3rd column */
		nscan = sscanf (record, "%s %s %s %[^\n]\n", txt_x, txt_y, txt_z, buffer);
		if ((GMT_scanf (GMT, txt_z, GMT->current.io.col_type[GMT_IN][GMT_Z], &GMT->current.io.curr_rec[GMT_Z]) == GMT_IS_NAN)) {
			GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad z coordinate, skipped)\n", rec_no);
			return (-1);
		}
	}
	else {
		nscan = sscanf (record, "%s %s %[^\n]\n", txt_x, txt_y, buffer);
		GMT->current.io.curr_rec[GMT_Z] = GMT->current.proj.z_level;
	}
	ix = (GMT->current.setting.io_lonlat_toggle[GMT_IN]);	iy = 1 - ix;
	if (GMT_scanf (GMT, txt_x, GMT->current.io.col_type[GMT_IN][GMT_X], &GMT->current.io.curr_rec[ix]) == GMT_IS_NAN) {
		GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad x coordinate, skipped)\n", rec_no);
		return (-1);
	}
	if (GMT_scanf (GMT, txt_y, GMT->current.io.col_type[GMT_IN][GMT_Y], &GMT->current.io.curr_rec[iy]) == GMT_IS_NAN) {
		GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad y coordinate, skipped)\n", rec_no);
		return (-1);
	}
	return (nscan);
}

#define Return(code) {Free_pstext_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); return (code);}

GMT_LONG GMT_pstext (struct GMTAPI_CTRL *API, struct GMT_OPTION *options)
{	/* High-level function that implements the pstext task */

	GMT_LONG i, nscan, length = 0, n_paragraphs = 0, n_add, n_fields, mode, n_alloc, m = 0;
	GMT_LONG n_read = 0, n_processed = 0, txt_alloc = 0, old_is_world, add, n_expected_cols;
	GMT_LONG error = FALSE, master_record = FALSE, skip_text_records = FALSE, pos, text_col;

	double plot_x = 0.0, plot_y = 0.0, save_angle = 0.0, xx[2] = {0.0, 0.0}, yy[2] = {0.0, 0.0}, *in = NULL;
	double offset[2], *c_x = NULL, *c_y = NULL, *c_angle = NULL;

	char text[BUFSIZ], buffer[BUFSIZ], pjust_key[5], txt_a[GMT_LONG_TEXT], txt_b[GMT_LONG_TEXT];
	char *paragraph = NULL, *line = NULL, *curr_txt = NULL, *in_txt = NULL, **c_txt = NULL;
#ifdef GMT_COMPAT
	char this_size[GMT_LONG_TEXT], this_font[GMT_LONG_TEXT], just_key[5], txt_f[GMT_LONG_TEXT];
	GMT_LONG is_old_format = GMTAPI_NOTSET;
#endif
	struct PSTEXT_INFO T;
	struct PSTEXT_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT interal parameters */
	struct PSL_CTRL *PSL = NULL;		/* General PSL interal parameters */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));

	if (!options || options->option == GMTAPI_OPT_USAGE) return (GMT_pstext_usage (API, GMTAPI_USAGE, FALSE));	/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) return (GMT_pstext_usage (API, GMTAPI_SYNOPSIS, FALSE));	/* Return the synopsis */

	/* Parse the command-line arguments; return if errors are encountered */

	GMT = GMT_begin_module (API, "GMT_pstext", &GMT_cpy);	/* Save current state */
	if ((error = GMT_Parse_Common (API, "-VJRf:", "BKOPUXxYyachpt>" GMT_OPT("E"), options))) Return (error);
	Ctrl = (struct PSTEXT_CTRL *)New_pstext_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_pstext_parse (API, Ctrl, options))) Return (error);
	if (Ctrl->L.active) Return (GMT_pstext_usage (API, GMTAPI_SYNOPSIS, TRUE));	/* Return the synopsis with font listing */
	PSL = GMT->PSL;		/* This module also needs PSL */

	/*---------------------------- This is the pstext main code ----------------------------*/

	load_parameters_pstext (GMT, &T, Ctrl);	/* Pass info from Ctrl to T */

#if 0
	if (!Ctrl->F.active) Ctrl->F.nread = 4;	/* Need to be backwards compatible */
#endif
	n_expected_cols = 3 + Ctrl->Z.active + Ctrl->F.nread;
	if (Ctrl->M.active) n_expected_cols += 3;

	add = !(T.x_offset == 0.0 && T.y_offset == 0.0);
	if (add && Ctrl->D.justify) T.boxflag |= 64;

	if (GMT_err_pass (GMT, GMT_map_setup (GMT, GMT->common.R.wesn), "")) Return (GMT_RUNTIME_ERROR);

	/* if (Ctrl->G.mode) GMT->current.ps.clip = +1; */	/* Signal that this program initiates clipping that wil outlive this process */
	
	GMT_plotinit (API, PSL, options);

	GMT_plane_perspective (GMT, PSL, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

	if (!(Ctrl->N.active || Ctrl->Z.active || Ctrl->G.mode)) GMT_map_clip_on (GMT, PSL, GMT->session.no_rgb, 3);

	in = GMT->current.io.curr_rec;
	text_col = n_expected_cols - 1;

	old_is_world = GMT->current.map.is_world;
	GMT->current.map.is_world = TRUE;

	if ((error = GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_POINT, GMT_IN, GMT_REG_DEFAULT, options))) Return (error);	/* Register data input */
	if ((error = GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_IN, GMT_BY_REC))) Return (error);	/* Enables data input and sets access mode */

	if (Ctrl->G.mode) {
		n_alloc = GMT_SMALL_CHUNK;
		(void)GMT_malloc3 (GMT, c_angle, c_x, c_y, n_alloc, 0, double);
		c_txt = GMT_memory (GMT, NULL, n_alloc, char *);
	}
	
	while ((n_fields = GMT_Get_Record (API, GMT_READ_TEXT, (void **)&line)) != EOF) {	/* Keep returning records until we have no more files */

		if (GMT_REC_IS_ERROR (GMT)) Return (EXIT_FAILURE);

		if (GMT_REC_IS_TBL_HEADER (GMT)) continue;	/* Skip table headers */

		if (Ctrl->M.active) {	/* Paragraph mode */
			if (GMT_REC_IS_SEG_HEADER (GMT)) {
				line = &GMT->current.io.segment_header[1];
				skip_text_records = FALSE;
				if (n_processed) {	/* Must output what we got */
					GMT_putwords (GMT, PSL, plot_x, plot_y, paragraph, &T);
					n_processed = length = 0;
					paragraph[0] = 0;	/* Empty existing text */
					n_paragraphs++;
				}

				if ((nscan = validate_coord_and_text (GMT, Ctrl->Z.active, n_read, line, buffer)) == -1) continue;	/* Failure */
				
				pos = 0;

#ifdef GMT_COMPAT
				if (is_old_format == GMTAPI_NOTSET) is_old_format = check_for_old_format (GMT, buffer, 1);

				if (is_old_format == 1) {	/* Old-style GMT 4 records */
					nscan += sscanf (buffer, "%s %lf %s %s %s %s %s\n", this_size, &T.paragraph_angle, this_font, just_key, txt_a, txt_b, pjust_key);
					T.block_justify = GMT_just_decode (GMT, just_key, 12);
					T.line_spacing = GMT_to_inch (GMT, txt_a);
					T.paragraph_width  = GMT_to_inch (GMT, txt_b);
					T.text_justify = (pjust_key[0] == 'j') ? PSL_JUST : GMT_just_decode (GMT, pjust_key, 0);
					sprintf (txt_f, "%s,%s,", this_size, this_font);	/* Merge size and font to be parsed by GMT_getfont */
					T.font = Ctrl->F.font;
					if (GMT_getfont (GMT, txt_f, &T.font)) GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad font (set to %s)\n", n_read, GMT_putfont (GMT, T.font));
					in_txt = NULL;
					n_expected_cols = 9 + Ctrl->Z.active;
				}
				else
#endif
				if (!Ctrl->F.nread)	/* All attributes given via -F (or we accept defaults); skip to paragraph attributes */
					in_txt = buffer;
				else {	/* Must pick up 1-3 attributes from data file */
					for (i = 0; i < Ctrl->F.nread; i++) {
						nscan += GMT_strtok (buffer, " \t", &pos, text);
						switch (Ctrl->F.read[i]) {
							case 'f':
								T.font = Ctrl->F.font;
								if (GMT_getfont (GMT, text, &T.font)) GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad font (set to %s)\n", n_read, GMT_putfont (GMT, T.font));
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
					GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad justification info (set to LB)\n", n_read);
					T.block_justify = 1;
				}
				if (nscan != n_expected_cols) {
					GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had incomplete paragraph information, skipped)\n", n_read);
					continue;
				}
				GMT_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y);
				if (!Ctrl->N.active) {
					skip_text_records = TRUE;	/* If this record should be skipped we must skip the whole paragraph */
					GMT_map_outside (GMT, in[GMT_X], in[GMT_Y]);
					if (GMT_abs (GMT->current.map.this_x_status) > 1 || GMT_abs (GMT->current.map.this_y_status) > 1) continue;
					skip_text_records = FALSE;	/* Since we got here we do not want to skip */
				}
				if (Ctrl->A.active) {
					save_angle = T.paragraph_angle;	/* Since we might overwrite the default */
					GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, save_angle, &T.paragraph_angle);
					T.paragraph_angle = fmod (T.paragraph_angle + 360.0 + 90.0, 180.0) - 90.0;	/* Ensure usable angles for text plotting */
				}
				master_record = TRUE;
			}
			else {	/* Text block record */
				if (skip_text_records) continue;	/* Skip all records for this paragraph */
				if (!master_record) {
					GMT_report (GMT, GMT_MSG_FATAL, "Text record line %ld not preceded by paragraph information, skipped)\n", n_read);
					continue;
				}
				GMT_chop (line);	/* Chop of line feed */
				GMT_enforce_rgb_triplets (GMT, line, BUFSIZ);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */

				if (line[0] == 0) {	/* Blank line marked by single NULL character, replace by \r */
					n_add = 1;
					while ((length + n_add) > txt_alloc) {
						txt_alloc += BUFSIZ;
						paragraph = GMT_memory (GMT, paragraph, txt_alloc, char);
					}
					strcat (paragraph, "\r");
				}
				else {
					if (Ctrl->Q.active) GMT_str_setcase (GMT, line, Ctrl->Q.mode);
					n_add = strlen (line) + 1;
					while ((length + n_add) > txt_alloc) {
						txt_alloc += BUFSIZ;
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
			if (GMT_REC_IS_SEG_HEADER (GMT)) continue;	/* Skip segment headers */
			if (GMT_is_a_blank_line (GMT, line)) continue;	/* Skip blank lines or # comments */

			if ((nscan = validate_coord_and_text (GMT, Ctrl->Z.active, n_read, line, buffer)) == -1) continue;	/* Failure */
			pos = 0;

#ifdef GMT_COMPAT
			if (is_old_format == GMTAPI_NOTSET) is_old_format = check_for_old_format (GMT, buffer, 0);

			if (is_old_format == 1) {	/* Old-style GMT 4 records */
				nscan--; /* Since we have already counted "text" */
				nscan += sscanf (buffer, "%s %lf %s %s %[^\n]\n", this_size, &T.paragraph_angle, this_font, just_key, text);
				T.block_justify = GMT_just_decode (GMT, just_key, 12);
				sprintf (txt_f, "%s,%s,", this_size, this_font);	/* Merge size and font to be parsed by GMT_getfont */
				T.font = Ctrl->F.font;
				if (GMT_getfont (GMT, txt_f, &T.font)) GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad font (set to %s)\n", n_read, GMT_putfont (GMT, T.font));
				in_txt = text;
				n_expected_cols = 7 + Ctrl->Z.active;
			}
			else
#endif

			if (!Ctrl->F.nread)	/* All attributes given via -F (or we accept defaults); just need text */
				in_txt = buffer;
			else {	/* Must pick up 1-3 attributes from data file */
				for (i = 0; i < Ctrl->F.nread; i++) {
					nscan += GMT_strtok (buffer, " \t", &pos, text);
					switch (Ctrl->F.read[i]) {
						case 'f':
							T.font = Ctrl->F.font;
							if (GMT_getfont (GMT, text, &T.font)) GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad font (set to %s)\n", n_read, GMT_putfont (GMT, T.font));
#ifdef GMT_COMPAT
							if (Ctrl->S.active) {
								T.font.form |= 2;
								T.font.pen = Ctrl->S.pen;
							}
#endif
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

			nscan += GMT_load_aspatial_string (GMT, GMT->current.io.OGR, text_col, in_txt);	/* Substitute OGR attribute if used */

			if (nscan != n_expected_cols) {
				GMT_report (GMT, GMT_MSG_FATAL, "Record %ld is incomplete (skipped)\n", n_read);
				continue;
			}
			if (T.block_justify == -99) {
				GMT_report (GMT, GMT_MSG_FATAL, "Record %ld had bad justification info (set to LB)\n", n_read);
				T.block_justify = 1;
			}

			/* Here, in_text holds the text we wish to plot */

			GMT_enforce_rgb_triplets (GMT, in_txt, BUFSIZ);	/* If @; is used, make sure the color information passed on to ps_text is in r/b/g format */
			if (Ctrl->Q.active) GMT_str_setcase (GMT, in_txt, Ctrl->Q.mode);
			n_read++;
			GMT_geo_to_xy (GMT, in[GMT_X], in[GMT_Y], &plot_x, &plot_y);
			xx[0] = plot_x;	yy[0] = plot_y;
			if (!Ctrl->N.active) {
				GMT_map_outside (GMT, in[GMT_X], in[GMT_Y]);
				if (GMT_abs (GMT->current.map.this_x_status) > 1 || GMT_abs (GMT->current.map.this_y_status) > 1) continue;
			}

			if (Ctrl->A.active) {
				save_angle = T.paragraph_angle;	/* Since we might overwrite the default */
				GMT_azim_to_angle (GMT, in[GMT_X], in[GMT_Y], 0.1, save_angle, &T.paragraph_angle);
				T.paragraph_angle = fmod (T.paragraph_angle + 360.0 + 90.0, 180.0) - 90.0;	/* Ensure usable angles for text plotting */
			}
			if (add) {
				if (Ctrl->D.justify)	/* Smart offset according to justification (from Dave Huang) */
					GMT_smart_justify (GMT, T.block_justify, T.paragraph_angle, T.x_offset, T.y_offset, &plot_x, &plot_y);
				else {	/* Default hard offset */
					plot_x += T.x_offset;
					plot_y += T.y_offset;
				}
				xx[1] = plot_x;	yy[1] = plot_y;
			}
			n_paragraphs++;

			PSL_setfont (PSL, T.font.id);
			GMT_plane_perspective (GMT, PSL, GMT->current.proj.z_project.view_plane, in[GMT_Z]);
			if (!Ctrl->G.mode && T.boxflag & 3) {	/* Plot the box beneath the text */
				if (T.space_flag) {	/* Meant % of fontsize */
					offset[0] = 0.01 * T.x_space * T.font.size / PSL_POINTS_PER_INCH;
					offset[1] = 0.01 * T.y_space * T.font.size / PSL_POINTS_PER_INCH;
				}
				else {
					offset[0] = T.x_space;
					offset[1] = T.y_space;
				}
				GMT_setpen (GMT, PSL, &T.boxpen);			/* Box pen */
				PSL_setfill (PSL, T.boxfill.rgb, T.boxflag & 1);	/* Box color */
				PSL_plottextbox (PSL, plot_x, plot_y, T.font.size, in_txt, T.paragraph_angle, T.block_justify, offset, T.boxflag & 4);
				curr_txt = NULL;	/* Text has now been encoded in the PS file */
			}
			else
				curr_txt = in_txt;
			mode = GMT_setfont (GMT, PSL, &T.font);
			if (Ctrl->G.mode) {
				if (m <= n_alloc) {
					n_alloc = GMT_malloc3 (GMT, c_angle, c_x, c_y, m, n_alloc, double);
					c_txt = GMT_memory (GMT, c_txt, n_alloc, char *);
				}
				c_angle[m] = T.paragraph_angle;
				c_txt[m] = strdup (curr_txt);
				c_x[m] = plot_x;
				c_y[m] = plot_y;
				m++;
			}
			else {	
				PSL_plottext (PSL, plot_x, plot_y, T.font.size, curr_txt, T.paragraph_angle, T.block_justify, mode);
				if (T.boxflag & 32) {	/* Draw line from original point to shifted location */
					GMT_setpen (GMT, PSL, &T.vecpen);
					PSL_plotsegment (PSL, xx[0], yy[0], xx[1], yy[1]);
				}
			}
			if (Ctrl->A.active) T.paragraph_angle = save_angle;	/* Restore original angle */
		}

	}
	if ((error = GMT_End_IO (API, GMT_IN, 0))) Return (error);	/* Disables further data input */

	if (Ctrl->M.active) {
		if (n_processed) {	/* Must output the last paragraph */
			GMT_putwords (GMT, PSL, plot_x, plot_y, paragraph, &T);
			n_paragraphs++;
		}
	 	GMT_free (GMT, paragraph);
	}
	if (Ctrl->G.mode && m) {
		GMT_LONG form;
		GMT_textpath_init (GMT, PSL, &Ctrl->W.pen, Ctrl->W.pen.rgb, &Ctrl->W.pen, Ctrl->G.fill.rgb);
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
		GMT_free (GMT, c_angle);
		GMT_free (GMT, c_x);
		GMT_free (GMT, c_y);
		GMT_free (GMT, c_txt);
	}
	else if (!(Ctrl->N.active || Ctrl->Z.active)) GMT_map_clip_off (GMT, PSL);

	GMT->current.map.is_world = old_is_world;

	GMT_map_basemap (GMT, PSL);
	GMT_plane_perspective (GMT, PSL, -1, 0.0);
	GMT_plotend (GMT, PSL);

	GMT_report (GMT, GMT_MSG_NORMAL, Ctrl->M.active ? "pstext: Plotted %ld text blocks\n" : "pstext: Plotted %ld text strings\n", n_paragraphs);

	Return (GMT_OK);
}
