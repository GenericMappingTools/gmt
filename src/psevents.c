/*
 *
 *	Copyright (c) 1991-2021 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * API functions to support the psevents application.
 *
 * Author:	Paul Wessel
 * Date:	5-MAY-2019
 * Version:	6 API
 *
 * Brief synopsis: psevents handles the plotting of events for one frame of a movie.
 */
#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psevents"
#define THIS_MODULE_MODERN_NAME	"events"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Plot event symbols, lines, polygons and labels for a moment in time"
#define THIS_MODULE_KEYS	"<D{,CC(,>?}"
#define THIS_MODULE_NEEDS	"JR"
#define THIS_MODULE_OPTIONS	"-:>BJKOPRUVXYabdefhipqw"

enum Psevent {	/* Misc. named array indices */
	PSEVENTS_SYMBOL = 0,
	PSEVENTS_TEXT = 1,
	PSEVENTS_INT = 0,
	PSEVENTS_SIZE = 1,
	PSEVENTS_TRANSP = 2,
	PSEVENTS_OFFSET = 0,
	PSEVENTS_RISE = 1,
	PSEVENTS_PLATEAU = 2,
	PSEVENTS_DECAY = 3,
	PSEVENTS_FADE = 4,
	PSEVENTS_LENGTH = 5,
	PSEVENTS_VAL1 = 0,
	PSEVENTS_VAL2 = 1,
	PSEVENTS_INFINITE = 0,
	PSEVENTS_FIXED_DURATION = 1,
	PSEVENTS_VAR_DURATION = 2,
	PSEVENTS_VAR_ENDTIME = 3,
	PSEVENTS_LINE_REC = 1,
	PSEVENTS_LINE_SEG = 2,
	PSEVENTS_LINE_TO_POINTS = 4
};

#define PSEVENTS_MODS "dfloOpr"

/* Control structure for psevents */

struct PSEVENTS_CTRL {
	struct PSEVENTS_A {	/* 	-Ar[<dpu>]|s */
		bool active;
		unsigned int mode;
		double dpu;		/* For resampling lines into points */
	} A;
	struct PSEVENTS_C {	/* 	-C<cpt> */
		bool active;
		char *file;
	} C;
	struct PSEVENTS_D {	/*	-D[j]<dx>[/<dy>][v[<pen>]] */
		bool active;
		char *string;
	} D;
	struct PSEVENTS_E {	/* 	-E[s|t][+o|O<dt>][+r<dt>][+p<dt>][+d<dt>][+f<dt>][+l<dt>] */
		bool active[2];
		bool trim[2];
		double dt[2][6];
	} E;
	struct PSEVENTS_F {	/*	-F[+f<fontinfo>+a<angle>+j<justification>+r|z] */
		bool active;
		char *string;
	} F;
	struct PSEVENTS_G {	/* 	-G<fill> */
		bool active;
		char *fill;
	} G;
	struct PSEVENTS_H {	/* 	-H[+c<dx/dy>][+g<fill>]]+p<pen>][+r][+s[<dx/dy>/][<fill>][ */
		bool active;
		bool boxoutline, boxfill, boxshade;
		int box;	/* Box shape */
		char fill[GMT_LEN64];	/* Textbox fill [none] */
		char sfill[GMT_LEN64];	/* Shade fill [gray50] */
		char pen[GMT_LEN64];	/* Textbox outline */
		char clearance[GMT_LEN64];	/* Text box clearance */
		double soff[2];	/* Shade box offset */
	} H;
	struct PSEVENTS_L {	/* 	-L[t|<length>] */
		bool active;
		unsigned int mode;
		double length;
	} L;
	struct PSEVENTS_M {	/* 	-M[i|s|t]<val1>[+c<val2] */
		bool active[3];
		double value[3][2];
	} M;
	struct PSEVENTS_N {	/* -N[r|c] */
		bool active;
		char *arg;	/* We will pass this to psxy */
	} N;
	struct PSEVENTS_Q {	/* 	-Q<prefix> */
		bool active;
		char *file;
	} Q;
	struct PSEVENTS_S {	/* 	-S<symbol>[<size>] */
		bool active;
		unsigned int mode;
		char *symbol;
	} S;
	struct PSEVENTS_T {	/* 	-T<nowtime> */
		bool active;
		double now;
	} T;
	struct PSEVENTS_W {	/* 	-W<pen> */
		bool active;
		char *pen;
	} W;
	struct PSEVENTS_Z {	/* 	-Z<cmd> */
		bool active;
		char *module;
		char *cmd;
	} Z;
};

/* THe names of the three external modules.  We skip first 2 letters if in modern mode */
GMT_LOCAL char *coupe = "pscoupe", *meca = "psmeca", *velo = "psvelo";

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSEVENTS_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSEVENTS_CTRL);
	sprintf (C->H.clearance, "%d%%", GMT_TEXT_CLEARANCE);	/* 15% of font size */
	C->H.soff[GMT_X] = GMT->session.u2u[GMT_PT][GMT_INCH] * GMT_FRAME_CLEARANCE;	/* Default is 4p */
	C->H.soff[GMT_Y] = -C->H.soff[GMT_X];	/* Set the shadow offsets [default is (4p, -4p)] */
	strcpy (C->H.pen, gmt_putpen (GMT, &GMT->current.setting.map_default_pen));	/* Default outline pen */
	C->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL1] = C->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL2] = 100.0;	/* Rise from and fade to invisibility */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct PSEVENTS_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->D.string);
	gmt_M_str_free (C->F.string);
	gmt_M_str_free (C->G.fill);
	gmt_M_str_free (C->N.arg);
	gmt_M_str_free (C->Q.file);
	gmt_M_str_free (C->S.symbol);
	gmt_M_str_free (C->W.pen);
	gmt_M_str_free (C->Z.module);
	gmt_M_str_free (C->Z.cmd);
	gmt_M_free (GMT, C);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] %s %s -S<symbol>[<size>]\n", name, GMT_J_OPT, GMT_Rgeoz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t-T<now> [-Ar[<dpu>[c|i]]|s] [-C<cpt>] [-D[j|J]<dx>[/<dy>][+v[<pen>]] [-E[s|t][+o|O<dt>][+r<dt>][+p<dt>][+d<dt>][+f<dt>][+l<dt>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-F[+a<angle>][+f<font>][+r[<first>]|+z[<fmt>]][+j<justify>]] [-G<fill>] [-H<labelinfo>] [-L[t|<length>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-Mi|s|t<val1>[+c<val2]] [-N[c|r]] [-Q<prefix>] [-W[<pen>] [-Z\"<command>\"] [%s] [%s]\n", GMT_V_OPT, GMT_b_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		API->c_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_qi_OPT, GMT_w_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "J-Z,R");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify the time for preparing events.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t   Record format: lon lat [z] [size] time [length|time2].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Select plotting of lines or polygons when no -S is given.  Choose input mode:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append r to read records for lines with time in column 3.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append <dpu> to convert your line records into dense point records that can be plotted as circles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     The resampled line will be written to stdout (requires options -R -J, optionally -C).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     The <dpu> must be the same as the intended <dpu> for the movie frames.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append i if dpi and c if dpc [Default will consult GMT_LENGTH_UNIT setting, currently %s]\n", API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append s to read whole segments (lines or polygons) with no time column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Time is set via segment header -T<start>, -T<start>,<end>, or -T<start>,<duration (see -L).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Give <cpt> and obtain symbol color via z-value in 3rd data column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Add <add_x>,<add_y> to the event text origin AFTER projecting with -J [0/0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Dj to move text origin away from point (direction determined by text's justification).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Upper case -DJ will shorten diagonal shifts at corners by sqrt(2).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +v[<pen>] to draw line from text to original point.  If <add_y> is not given it equals <add_x>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set offset, rise, plateau, decay, and fade intervals for symbols (-Es [Default])\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or offset, rise,, and fade intervals for text (-Et):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +o<dt> offsets event start and end times by <dt> [no offset].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use +O<dt> to only offset event start time and leave end time alone.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +r<dt> sets the rise-time before the event start time [no rise time].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +p<dt> sets the length of the plateau after event happens [no plateau].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +d<dt> sets the decay-time after the plateau [no decay].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +f<dt> sets the fade-time after the event ends [no fade time].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +l<dt> sets alternative label duration [same as symbol].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Specify values for text attributes that apply to all text records:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +a<angle> specifies the baseline angle for all text [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use +A to force text-baselines in the -90/+90 range.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +f<fontinfo> sets the size, font, and optionally the text color [%s].\n",
		gmt_putfont (API->GMT, &API->GMT->current.setting.font_annot[GMT_PRIMARY]));
	GMT_Message (API, GMT_TIME_NONE, "\t   +j<justify> sets text justification relative to given (x,y) coordinate.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Give a 2-char combo from [T|M|B][L|C|R] (top/middle/bottom/left/center/right) [CM].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Normally, the text is read from the data records.  Alternative ways to provide text:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +r[<first>] will use the current record number, starting at <first> [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     +z[<fmt>] will use formatted input z values (requires -C) via format <fmt> [FORMAT_FLOAT_MAP].\n");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Specify a fixed symbol fill [no fill].");
	GMT_Message (API, GMT_TIME_NONE, "\t-H Control attributes of optional label bounding box fill, outline, clearance, shade, and box shape [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c<dx>[/<dy>] for the clearance between label and surrounding box [%d%% of font size].\n", GMT_TEXT_CLEARANCE);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +g<fill> to set the fill for the text box [no fill].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p[<pen>] to set the pen [%s] and draw the outline of the text box [no outline].\n", gmt_putpen (API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +r to select rounded rectangular box shape [straight].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +s[<dx>/<dy>/][<shade>] to plot a shadow behind the text box [%gp/%gp/gray50].\n", GMT_FRAME_CLEARANCE, -GMT_FRAME_CLEARANCE);
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set finite length of events, otherwise we assume they are all infinite.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no arg we read lengths from file; append t for reading end times instead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -L0 is given the event only lasts one frame.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Do not skip or clip symbols that fall outside the map border [clipping is on].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Nr to turn off clipping and plot repeating symbols for periodic maps.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Nc to retain clipping but turn off plotting of repeating symbols for periodic maps.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default will clip or skip symbols that fall outside and plot repeating symbols].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Append i for intensity, s for size, or t for transparency; repeatable.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append value to use during rise, plateau, or decay phases.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c to set a separate terminal value for the coda [no coda].\n");
	GMT_Option (API, "K,O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Save intermediate events symbol and labels files; append file prefix [temporary files deleted].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Append symbol code and optionally <size>.  If no size we read it from the data file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default plots lines or polygons; see -A for further instructions.\n");
	GMT_Option (API, "V");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Set symbol outline pen attributes [Default pen is %s]:", 0);
	GMT_Option (API, "X");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Append core external <command> and required options that must include -S<format><size>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The quoted <command> must start with [ps]coupe, [ps]meca, or [ps]velo.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (Note: The <command> cannot contain options -C, -G, -I, -J, -N, -R, -W, -t).\n");
	GMT_Option (API, "bi2,c,di,e,f,h,i,p,qi,w,:,.");

	return (GMT_MODULE_USAGE);
}

static int parse (struct GMT_CTRL *GMT, struct PSEVENTS_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psevents and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, pos, n_col = 3, k = 0, id = 0;
	unsigned int s = (GMT->current.setting.run_mode == GMT_MODERN) ? 2 : 0;
	char *c = NULL, *t_string = NULL, txt_a[GMT_LEN256] = {""}, *events = "psevents";
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (GMT_Get_FilePath (GMT->parent, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(opt->arg))) n_errors++;;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Plotting lines or polygons, how are they given, or alternatively resample the line to an equivalent point file */
				Ctrl->A.active = true;
				switch (opt->arg[0]) {
					case 'r':
						if (opt->arg[1] && (Ctrl->A.dpu = atof (&opt->arg[1])) > 0.0) {
							Ctrl->A.mode = PSEVENTS_LINE_TO_POINTS;	/* Gave a dpu for guidance on line resampling into points */
							if (strchr (&opt->arg[1], 'c'))	/* Explicitly said dpu is in cm */
								Ctrl->A.dpu *= 2.54;	/* Now dpi */
							else if (strchr (&opt->arg[1], 'i'))	/* Explicitly said dpu is in inch - do nothing */
								Ctrl->A.dpu *= 1;	/* Still dpi */
							else if (GMT->current.setting.proj_length_unit == GMT_CM)	/* Default length unit is cm so convert */
								Ctrl->A.dpu *= 2.54;	/* Now dpi */
						}
						else
							Ctrl->A.mode = PSEVENTS_LINE_REC;	/* Read line (x,y,t) records */
						break;
					case 's':	Ctrl->A.mode = PSEVENTS_LINE_SEG; break;	/* Read polygons/lines segments */
					default:
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -A: Specify -Ar[<dpu>]|s\n");
						n_errors++;
						break;
				}
				break;

			case 'C':	/* Set a cpt for converting z column to color */
				Ctrl->C.active = true;
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;

			case 'D':
				Ctrl->D.active = true;
				if (opt->arg[0]) Ctrl->D.string = strdup (opt->arg);
				break;

			case 'E':	/* Set event times. If -T is abstime then these are in units of TIME_UNIT [s] */
				switch (opt->arg[0]) {
					case 's':	id = PSEVENTS_SYMBOL;	k = 1;	break;
					case 't':	id = PSEVENTS_TEXT;		k = 1;	break;
					default:	id = PSEVENTS_SYMBOL;	k = 0;	break;
				}
				Ctrl->E.active[id] = true;
				if (gmt_validate_modifiers (GMT, &opt->arg[k], 'E', PSEVENTS_MODS, GMT_MSG_ERROR)) n_errors++;
				if ((c = gmt_first_modifier (GMT, &opt->arg[k], PSEVENTS_MODS)) == NULL) {	/* Just sticking to the event range */
					break;
				}
				pos = 0;	txt_a[0] = 0;
				while (gmt_getmodopt (GMT, 'E', c, PSEVENTS_MODS, &pos, txt_a, &n_errors) && n_errors == 0) {
					switch (txt_a[0]) {
						case 'd': Ctrl->E.dt[id][PSEVENTS_DECAY]   = atof (&txt_a[1]);	break;	/* Decay duration */
						case 'f': Ctrl->E.dt[id][PSEVENTS_FADE]    = atof (&txt_a[1]);	break;	/* Fade duration */
						case 'p': Ctrl->E.dt[id][PSEVENTS_PLATEAU] = atof (&txt_a[1]);	break;	/* Plateau duration */
						case 'r': Ctrl->E.dt[id][PSEVENTS_RISE]    = atof (&txt_a[1]);	break;	/* Rise duration */
						case 'O': Ctrl->E.trim[id] = true;	/* Intentionally fall through - offset start but not end. Fall through to case 'o' */
						case 'o': Ctrl->E.dt[id][PSEVENTS_OFFSET]   = atof (&txt_a[1]);	break;	/* Event time offset */
						case 'l':	/* Event length override for text */
							if (id == PSEVENTS_SYMBOL) {
								GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -E[s]: The +l modifier is only allowed for -Et\n");
								n_errors++;
							}
							else
								Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_LENGTH] = atof (&txt_a[1]);
							break;
						default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
					}
				}
				if (k == 0) {	/* Duplicate to text settings */
					Ctrl->E.active[PSEVENTS_TEXT] = true;
					gmt_M_memcpy (Ctrl->E.dt[PSEVENTS_TEXT], Ctrl->E.dt[PSEVENTS_SYMBOL], 5U, double);
				}
				break;

			case 'F':
				Ctrl->F.active = true;
				if (opt->arg[0]) Ctrl->F.string = strdup (opt->arg);
				break;

			case 'G':	/* Set a fixed symbol fill */
				Ctrl->G.active = true;
				if (opt->arg[0]) Ctrl->G.fill = strdup (opt->arg);
				break;

			case 'H':	/* Label text box settings */
				Ctrl->H.active = true;
				if (opt->arg[0] == '\0' || gmt_validate_modifiers (GMT, opt->arg, 'H', "cgprs", GMT_MSG_ERROR))
					n_errors++;
				else {
					struct GMT_FILL fill;	/* Only used to make sure any fill is given with correct syntax */
					struct GMT_PEN pen;	/* Only used to make sure any pen is given with correct syntax */
					char string[GMT_LEN128] = {""};
					if (gmt_get_modifier (opt->arg, 'c', string) && string[0])	/* Clearance around text in textbox */
						strncpy (Ctrl->H.clearance, string, GMT_LEN64);
					if (gmt_get_modifier (opt->arg, 'g', Ctrl->H.fill) && Ctrl->H.fill[0]) {	/* Textbox fill color */
						if (gmt_getfill (GMT, Ctrl->H.fill, &fill)) n_errors++;
						Ctrl->H.boxfill = true;
					}
					if (gmt_get_modifier (opt->arg, 'p', string)) {	/* Textbox outline pen */
						if (string[0]) strcpy (Ctrl->H.pen, string);	/* Gave specific pen */
						if (gmt_getpen (GMT, Ctrl->H.pen, &pen)) n_errors++;
						Ctrl->H.boxoutline = true;
					}
					if (gmt_get_modifier (opt->arg, 'r', string))	/* Rounded text box */
						Ctrl->H.box = 1;
					if (gmt_get_modifier (opt->arg, 's', string)) {	/* Shaded text box fill color */
						Ctrl->H.boxshade = true;
						strcpy (Ctrl->H.sfill, "gray50");	/* Default shade color */
						Ctrl->H.soff[GMT_X] = GMT->session.u2u[GMT_PT][GMT_INCH] * GMT_FRAME_CLEARANCE;	/* Default is 4p */
						Ctrl->H.soff[GMT_Y] = -Ctrl->H.soff[GMT_X];	/* Set the shadow offsets [default is (4p, -4p)] */
						if (!Ctrl->H.boxfill) {
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -H: Modifier +h requires +g as well\n");
							n_errors++;
						}
						else if (string[0]) {	/* Gave an argument to +b */
							char txt_a[GMT_LEN64] = {""}, txt_b[GMT_LEN64] = {""}, txt_c[GMT_LEN64] = {""};
							int n = sscanf (string, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
							if (n == 1)	/* Just got a new fill */
								strcpy (Ctrl->H.sfill, txt_a);
							else if (n == 2) {	/* Just got a new offset */
								if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->H.soff) < 0) n_errors++;
							}
							else if (n == 3) {	/* Got offset and fill */
								Ctrl->H.soff[GMT_X] = gmt_M_to_inch (GMT, txt_a);
								Ctrl->H.soff[GMT_Y] = gmt_M_to_inch (GMT, txt_b);
								strcpy (Ctrl->H.sfill, txt_c);
							}
							else n_errors++;
						}
						if (gmt_getfill (GMT, Ctrl->H.sfill, &fill)) n_errors++;
					}
				}
				break;

			case 'L':	/* Set length of events */
				Ctrl->L.active = true;
				n_col = 4;	/* Need to read one extra column, possibly */
				if (opt->arg[0] == 't')	/* Get individual event end-times from column in file */
					Ctrl->L.mode = PSEVENTS_VAR_ENDTIME;
				else if (opt->arg[0]) {	/* Get a fixed event length for all events here */
					Ctrl->L.length = atof (opt->arg);
					Ctrl->L.mode = PSEVENTS_FIXED_DURATION;
					n_col = 3;
					if (gmt_M_is_zero (Ctrl->L.length)) Ctrl->L.length = GMT_CONV8_LIMIT;	/* Let -L0 be -L<tiny> */
				}
				else	/* Get individual event lengths from column in file */
					Ctrl->L.mode = PSEVENTS_VAR_DURATION;
				break;

			case 'M':	/* Set size, intensity, or transparency of symbol during optional rise [0] and fade [0] phases */
				switch (opt->arg[0]) {
					case 'i':	id = 0;	k = 1;	break;	/* Intensity settings */
					case 's':	id = 1;	k = 1;	break;	/* Size settings */
					case 't':	id = 2;	k = 1;	break;	/* Transparency settings */
					default:
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -M: Directive %c not valid\n", opt->arg[1]);
						n_errors++;
						break;
				}
				Ctrl->M.active[id] = true;
				if ((c = strstr (&opt->arg[k], "+c"))) {
					Ctrl->M.value[id][PSEVENTS_VAL2] = atof (&c[2]);
					c[0] = '\0';	/* Truncate modifier */
				}
				if (opt->arg[k]) Ctrl->M.value[id][PSEVENTS_VAL1] = atof (&opt->arg[k]);
				if (c) c[0] = '+';	/* Restore modifier */
				break;

			case 'N':		/* Do not skip points outside border and don't clip labels */
				Ctrl->N.active = true;
				if (!(opt->arg[0] == '\0' || strchr ("rc", opt->arg[0]))) {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -N: Unrecognized argument %s\n", opt->arg);
					n_errors++;
				}
				else {	/* Create option to pass to plot/text */
					snprintf (txt_a, GMT_LEN128, " -N%s", opt->arg);
					Ctrl->N.arg = strdup (txt_a);
				}
				break;
			case 'Q':	/* Save events file for posterity */
				Ctrl->Q.active = true;
				if (opt->arg[0]) Ctrl->Q.file = strdup (opt->arg);
				break;

			case 'S':	/* Set symbol type and size (append units) */
				Ctrl->S.active = true;
				if (strchr ("kK", opt->arg[0])) {	/* Custom symbol may have a slash before size */
					Ctrl->S.symbol = strdup (opt->arg);
					if ((c = strrchr (opt->arg, '/')) == NULL)	/* Gave no size so get the whole thing and read size from file */
						Ctrl->S.mode = 1;
				}
				else if (opt->arg[0] == 'E' && opt->arg[1] == '-') {
					Ctrl->S.symbol = strdup ("E-");
					if (!opt->arg[2])	/*  Must read individual event symbol sizes for file using prevailing length-unit setting */
						Ctrl->S.mode = 1;
				}
				else if (strchr ("-+aAcCdDgGhHiInNsStTxy", opt->arg[0])) {	/* Regular symbols of form <code>[<size>], where <code> is 1-char */
					if (opt->arg[1] && !strchr (GMT_DIM_UNITS, opt->arg[1]))	/* Gave a fixed size */
						Ctrl->S.symbol = strdup (opt->arg);
					else if (opt->arg[1] && strchr (GMT_DIM_UNITS, opt->arg[1])) {	/* Must read symbol sizes in this unit from file */
						Ctrl->S.mode = 1;
						gmt_set_measure_unit (GMT, opt->arg[1]);
						sprintf (txt_a, "%c", opt->arg[0]);	/* Just the symbol code */
						Ctrl->S.symbol = strdup (txt_a);
					}
					else {	/* Must read individual event symbol sizes for file using prevailing length-unit setting */
						Ctrl->S.mode = 1;
						Ctrl->S.symbol = strdup (opt->arg);
					}
				}
				else {	/* Odd symbols not yet possible in this module */
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -S: Cannot (yet) handle symbol code %c\n", opt->arg[0]);
					n_errors++;
				}
				break;

			case 'T':	/* Get time (-fT will be set if these are absolute times and not dummy times or frames) */
				Ctrl->T.active = true;
				t_string = opt->arg;
				break;

			case 'W':	/* Set symbol outline pen */
				Ctrl->W.active = true;
				if (opt->arg[0]) Ctrl->W.pen = strdup (opt->arg);
				break;

			case 'Z':	/* Select advanced seismologic/geodetic symbols */
				Ctrl->Z.active = true;
				if (opt->arg[0] && strstr (opt->arg, "-S")) {	/* Got the required -S option as part of the command */
					if ((c = strchr (opt->arg, ' '))) {	/* First space in the command ends the module name */
						char *q;
						c[0] = '\0';	/* Temporarily hide the rest of the command so we can isolate the module name */
						Ctrl->Z.module = strdup (opt->arg);	/* Make a copy of the module name */
						c[0] = ' ';	/* Restore space */
						while (c[0] == ' ' || c[0] == '\t') c++;	/* Move to first word after the module (user may have more than one space...) */
						strncpy (txt_a, c, GMT_LEN256);	/* Copy the remaining text into a temporary buffer */
						q = strstr (txt_a, "-S") + 3;	/* Determine the start position of the required symbol size in the command */
						if (!(isdigit (q[0]) || (q[0] == '.' && isdigit (q[1]))))	/* Got no size, must read from data file */
							Ctrl->S.mode = 1;
						if (strstr (txt_a, "-C") || strstr (txt_a, "-G") || strstr (txt_a, "-H") || strstr (txt_a, "-I") || strstr (txt_a, "-J") || strstr (txt_a, "-N") || strstr (txt_a, "-R") \
						  || strstr (txt_a, "-W") || strstr (txt_a, "-t")) {	/* Sanity check */
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Z: Cannot include options -C, -G, -H, -I, -J, -N, -R, -W, or -t in the %s command\n", Ctrl->Z.module);
							GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Z: The -C, -G, -J, -N, -R, -W options may be given to %s instead, while -H, -I, -t are not allowed\n", &events[s]);
							n_errors++;							
						}
						else	/* Keep a copy of the final command that has -S with no symbol-size specified */
							Ctrl->Z.cmd = strdup (txt_a);
					}
				}
				if (Ctrl->Z.cmd == NULL || Ctrl->Z.module == NULL) {	/* Sanity checks */
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -Z: Requires a core [ps]coupe, [ps]meca, or [ps]velo command with valid -S option and fixed size\n");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	gmt_consider_current_cpt (GMT->parent, &Ctrl->C.active, &(Ctrl->C.file));

	if (Ctrl->C.active) n_col++;	/* Need to read one more column for z */
	if (Ctrl->S.mode) n_col++;	/* Must allow for size in input before time and length */
	if (t_string) {	/* Do a special check for absolute time since auto-detection based on input file has not happened yet and user may have forgotten about -f */
		enum gmt_col_enum type = (strchr (t_string, 'T')) ? GMT_IS_ABSTIME : gmt_M_type (GMT, GMT_IN, n_col-1);
		n_errors += gmt_verify_expectations (GMT, type, gmt_scanf_arg (GMT, t_string, type, false, &Ctrl->T.now), t_string);
	}
	else if (Ctrl->A.mode != PSEVENTS_LINE_TO_POINTS) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -T: -T<now> is a required option.\n");
		n_errors++;
	}
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = n_col;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < n_col, "Binary input data (-bi) must have at least %u columns.\n", n_col);
	n_errors += gmt_M_check_condition (GMT, (Ctrl->A.active + Ctrl->S.active + Ctrl->Z.active) != 1 && Ctrl->E.active[PSEVENTS_SYMBOL], "Must specify either -A, -S or -Z unless just plotting labels.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->G.active, "Cannot specify both -C and -G.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.mode == PSEVENTS_LINE_REC && Ctrl->G.active, "Option -G: Cannot be used with lines (-Ar).\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.mode == PSEVENTS_LINE_REC && Ctrl->C.active, "Option -C: Cannot be used with lines (-Ar).\n");
	if (Ctrl->A.mode == PSEVENTS_LINE_TO_POINTS) {	/* Most options are not valid with -Ar<dpu> since we are not plotting */
		n_errors += gmt_M_check_condition (GMT, Ctrl->D.active, "Option -D: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->E.active[0], "Option -Es: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->E.active[1], "Option -Et: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->F.active, "Option -F: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->G.active, "Option -G: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->L.active, "Option -L: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->M.active[0], "Option -Mi: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->M.active[1], "Option -Ms: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->M.active[2], "Option -Mt: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->S.active, "Option -S: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active, "Option -Z: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->T.active, "Option -T: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, Ctrl->W.active, "Option -W: Not allowed with -Ar<dpu>.\n");
		n_errors += gmt_M_check_condition (GMT, GMT->current.setting.proj_length_unit == GMT_PT, "PROJ_LENGTH_UNIT: Must be either cm or inch for -Ar<dpu> to work.\n");
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && Ctrl->D.string == NULL, "Option -D: No argument given\n");
	n_errors += gmt_M_check_condition (GMT, !gmt_M_is_zero (Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_DECAY]), "Option -Et: No decay phase for labels.\n");
	n_errors += gmt_M_check_condition (GMT, !gmt_M_is_zero (Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_PLATEAU]), "Option -Et: No plateau phase for labels.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.active && Ctrl->F.string == NULL, "Option -F: No argument given\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->G.active && Ctrl->G.fill == NULL, "Option -G: No argument given\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && Ctrl->Q.file == NULL, "Option -Q: No file name given\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && (!Ctrl->C.active && !Ctrl->G.active && !Ctrl->W.active), "Option -S: Must specify at least one of -C, -G, -W to plot visible symbols.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && (!Ctrl->C.active && !Ctrl->G.active && !Ctrl->W.active), "Option -Z: Must specify at least one of -C, -G, -W to plot visible symbols.\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Must specify a map projection with the -J option\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && !(strstr (Ctrl->Z.module, &coupe[2]) || strstr (Ctrl->Z.module, &meca[2]) || strstr (Ctrl->Z.module, &velo[2])),
		"Option Z: Module command must begin with one of [ps]coupe, [ps]meca, or [ps]velo\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void psevents_set_XY (struct GMT_CTRL *GMT, unsigned int x_type, unsigned int y_type, double out[], char *X, char *Y) {
	/* Format the x and y outputs */
	if (x_type == GMT_IS_ABSTIME)
		gmt_ascii_format_one (GMT, X, out[GMT_X], x_type);
	else
		sprintf (X, "%.16g", out[GMT_X]);
	if (y_type == GMT_IS_ABSTIME)
		gmt_ascii_format_one (GMT, Y, out[GMT_Y], y_type);
	else
		sprintf (Y, "%.16g", out[GMT_Y]);
}

GMT_LOCAL unsigned int psevents_determine_columns (struct GMT_CTRL *GMT, char *module, char *cmd, unsigned int mode) {
	/* Return how many data columns are needed for the selected seismo/geodetic symbol */
	unsigned int n = 0;
	char *S = strstr (cmd, "-S");	/* Pointer to start of symbol option */
	gmt_M_unused (GMT);
	S += 2;	/* Now at format designation */

	if (strstr (module, &coupe[2])) {	/* Using coupe/pscoupe */
		switch (S[0]) {
			case 'a': n = 7; break;
			case 'c': n = 11; break;
			case 'm': case 'd': case 'z': n = 10; break;
			case 'p': n = 8; break;
			case 'x': n = 13; break;
			default: n = 0; break;
		}
	}
	else if (strstr (module, &meca[2])) {	/* Using meca/psmeca */
		switch (S[0]) {
			case 'a': n = 7; break;
			case 'c': n = 11; break;
			case 'm': case 'd': case 'z': n = 10; break;
			case 'p': n = 8; break;
			case 'x': case 'y': case 't': n = 13; break;
			default: n = 0; break;
		}
		if (strstr (cmd, "-Fo")) n--;	/* No depth column in this deprecated format */
	}
	else if (strstr (module, &velo[2])) {	/* Using velo/psvelo */
		switch (S[0]) {
			case 'e': case 'r': n = 7; break;
			case 'n': case 'w': n = 4; break;
			case 'x': n = 5; break;
			default: n = 0; break;
		}
		if (strstr (cmd, "+Zu")) n++;	/* Add in user column for coloring symbols via CPT lookup */
	}
	if (mode) n++;	/* Must also read symbol size from input file (separate from scaling) */
	return n;
}

EXTERN_MSC int gmtlib_clock_C_format (struct GMT_CTRL *GMT, char *form, struct GMT_CLOCK_IO *S, unsigned int mode);

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_psevents (void *V_API, int mode, void *args) {
	char tmp_file_symbols[PATH_MAX] = {""}, tmp_file_labels[PATH_MAX] = {""}, cmd[BUFSIZ] = {""}, *c = NULL;
	char string[GMT_LEN128] = {""}, header[GMT_BUFSIZ] = {""}, X[GMT_LEN32] = {""}, Y[GMT_LEN32] = {""},find;

	bool do_coda, finite_duration, out_segment = false;

	int error;

	enum gmt_col_enum time_type, end_type;

	unsigned int n_cols_needed, n_copy_to_out, col, s_in = 2, t_in = 2, d_in = 3, x_col = 2, i_col = 3, t_col = 4, x_type, y_type;

	uint64_t n_total_read = 0, n_symbols_plotted = 0, n_labels_plotted = 0;

	double out[20], t_end = DBL_MAX, *in = NULL;
	double t_event, t_rise, t_plateau, t_decay, t_fade, x, t_event_seg, t_end_seg;

	FILE *fp_symbols = NULL, *fp_labels = NULL;

	struct PSEVENTS_CTRL *Ctrl = NULL;
	struct GMT_RECORD *In = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct PSL_CTRL *PSL = NULL;		/* General PSL internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, NULL, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the psevents main code ----------------------------*/

	gmt_M_memset (out, 20, double);	/* Initialize the out vector */
	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");

	if (Ctrl->A.mode == PSEVENTS_LINE_TO_POINTS) {
		/* No plotting, but resampling a line data set into a densely sampled point file.
		 * We will resample the line densely so that plotting circles will look exactly the same as drawing the line, given the dpu of the movie frames.
		 * To do this we need projected coordinates (we have -R -J) in inches, the final dpu used by movie, and some GMT internal magic. The resampled line will be
		 * written to stdout.  The recipe we will perform in-memory is something like this ($n is last column number in input file):
		 *
		 * gmt mapproject line.txt -R -J -G+uC | gmt sample1d -N$n -T${d}c -Fl -AR | gmt mapproject -R -J -I -o0-$n,0 [--FORMAT_CLOCK_OUT=hh:mm:ss.xxx] > points.txt
		 *
		 * Note that for all the calls resulting in virtual files we do not need to worry about formatting since data will be in double precision.  It is only the
		 * final step 3 call that writes to stdout where we must ensure we have adequate precision in any time series.
		 */

		char Fmode, source[GMT_VF_LEN] = {""}, destination[GMT_VF_LEN] = {""}, cmd[GMT_LEN256] = {""}, TCLOCK[GMT_LEN64] = {""};
		unsigned last_col, d_col, type;
		uint64_t tbl, seg, row;
		double spacing = 1.0 / Ctrl->A.dpu;	/* Pixel size in inches regardless of dpu unit since we already converted dpu to dpi */
		struct GMT_DATASET *D = NULL;
		struct GMT_DATASEGMENT *S = NULL;

		switch (GMT->current.setting.interpolant) {	/* Determine which interpolant is chosen to ensure it is valid, and if not override it*/
			case GMT_SPLINE_LINEAR:	Fmode = 'l'; break;
			case GMT_SPLINE_AKIMA:	Fmode = 'a'; break;
			case GMT_SPLINE_CUBIC:	Fmode = 'c'; break;
			case GMT_SPLINE_NN:	Fmode = 'n'; break;
			default:
				GMT_Report (API, GMT_MSG_ERROR, "Your current setting for GMT_INTERPOLANT is invalid (only a|c|l|n may be used). Selecting linear interpolation.\n");
				Fmode = 'l';
				break;
		}

		/* Gather all listed or implicit input sources from the command line */
		if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Register all data inputs */
			Return (API->error);
		}
		/* Read all input lines into memory (D) */
		GMT_Report (API, GMT_MSG_INFORMATION, "Line sampling Step 0: Read input lines.\n");
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
			Return (API->error);
		}
		if (D->n_records < 2 || D->n_columns < 3) {
			GMT_Report (API, GMT_MSG_ERROR, "Your line data must (a) have more than 1 record and (2) have at least 3 data columns (x,y[,z][,size],time).\n");
			Return (API->error);
		}
		last_col = D->n_columns - 1;	/* Remember ID of last column in the input file which must be the time column. */
		type = gmt_M_type (GMT, GMT_IN, last_col);	/* Need to know if we have absolute time formatting or just floating point numbers */
		/* Create virtual files for using the data in mapproject and another for holding the result */
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN|GMT_IS_REFERENCE, D, source) != GMT_NOERROR) {
			Return (API->error);
		}
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT|GMT_IS_REFERENCE, NULL, destination) == GMT_NOTSET) {
			Return (API->error);
		}
		gmt_disable_bghi_opts (GMT);	/* Do not want any -b -g -h -i to affect the subsequent operations and module calls even though they may have been set to read the input correctly */
		/* Build mapproject command and run the module. Note: we want distances in inches since spacing is now in inches */
		sprintf (cmd, "%s -R%s -J%s -G+uC --PROJ_LENGTH_UNIT=inch ->%s", source, GMT->common.R.string, GMT->common.J.string, destination);
		GMT_Report (API, GMT_MSG_INFORMATION, "Line sampling Step 1: %s.\n", cmd);
		if (GMT_Call_Module (GMT->parent, "mapproject", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {	/* Convert the line to projected coordinates and compute distance in cm along the lines */
			Return (API->error);
		}
		/* Close the source virtual file and destroy the data set */
		if (GMT_Close_VirtualFile (GMT->parent, source) != GMT_NOERROR) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {	/* Completely done with the original dataset */
			Return (API->error);
		}
		/* Get the projected dataset with along-line projected distance measurements */
		if ((D = GMT_Read_VirtualFile (API, destination)) == NULL) {	/* Get the x, y[, zcols...], time, dist dataset */
			Return (API->error);
		}
		/* Close the destination virtual file */
		if (GMT_Close_VirtualFile (GMT->parent, destination) != GMT_NOERROR) {
			Return (API->error);
		}

		/* Possibly shrink spacing so we exactly step from distance 0 to max distance in steps of spacing */
		tbl = D->n_tables - 1;	seg = D->table[D->n_tables-1]->n_segments - 1;	/* Get last table and last segment in that table */
		S = D->table[tbl]->segment[seg];	/* Pointer to the last segment */
		d_col = last_col + 1;	/* Last column number with the distances */
		spacing = S->data[d_col][S->n_rows-1] / ceil (S->data[d_col][S->n_rows-1] / spacing);
		/* Create new virtual files for using the projected data in sample1d and another for holding the result */
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN|GMT_IS_REFERENCE, D, source) != GMT_NOERROR) {
			Return (API->error);
		}
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_OUT|GMT_IS_REFERENCE, NULL, destination) == GMT_NOTSET) {
			Return (API->error);
		}
		/* Build sample1d command and run it. The projected distances are always in the last data column */
		sprintf (cmd, "%s -N%d -T%.16g -F%c ->%s", source, d_col, spacing, Fmode, destination);
		GMT_Report (API, GMT_MSG_INFORMATION, "Line sampling Step 2: %s.\n", cmd);
		if (GMT_Call_Module (GMT->parent, "sample1d", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {	/* Resample all columns using the spacing */
			Return (API->error);
		}
		/* Close the latest source virtual file and destroy the data set */
		if (GMT_Close_VirtualFile (GMT->parent, source) != GMT_NOERROR) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {	/* Completely done with the original dataset */
			Return (API->error);
		}
		/* Get the resampled dataset that was computed */
		if ((D = GMT_Read_VirtualFile (API, destination)) == NULL) {
			Return (API->error);
		}
		if (type == GMT_IS_ABSTIME) {	/* Must ensure we pass a proper --FORMAT_CLOCK_OUT in the final call which will write an ASCII file */
			double this_dt, dt = DBL_MAX, dd, ds;
			int bad = 0;
			for (tbl = 0; tbl < D->n_tables; tbl++) {
				for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
					S = D->table[tbl]->segment[seg];
					for (row = 1; row < S->n_rows; row++) {
						dd = S->data[d_col][row] - S->data[d_col][row-1];
						ds = S->data[last_col][row] - S->data[last_col][row-1];
						if (ds <= 0.0 || dd <= 0.0)	/* At least dd could be bad if interpolant is not linear */
							bad++;
						else {	/* OK to estimate approximate time-increment */
							this_dt = dd / ds;
							if (this_dt < dt) dt = this_dt;
						}
					}
				}
			}
			if (bad) {
				GMT_Report (API, GMT_MSG_WARNING, "Interpolation  of time column with -F%c failed to give monotonically increasing values in %d places. Please use --GMT_INTERPOLANT=linear instead\n", Fmode, bad);
			}

			sprintf (TCLOCK, " --FORMAT_CLOCK_OUT=hh:mm:ss");
			if (dt < 1.0) {	/* Need to make sure we pass a proper FORMAT_CLOCK_IN|OUT with enough precision for Step 3 to adequately reflect precision in input data */
				int nx = gmt_get_precision_width (GMT, dt);
				strcat (TCLOCK, ".");	/* Build ss.xxxx.... */
				while (nx) {
					strcat (TCLOCK, "x");
					nx--;
				}
			}
		}
		/* Create final virtual file for using the resampled data in the inverse mapproject call and write to stdout */
		if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_LINE, GMT_IN|GMT_IS_REFERENCE, D, source) != GMT_NOERROR) {
			Return (API->error);
		}
		/* Build inverse mapproject command and run it. We are done with the along-line distances and thus use -o to retain the original (resampled) columns only */
		sprintf (cmd, "%s -R%s -J%s -I -o0:%d --PROJ_LENGTH_UNIT=inch --FORMAT_FLOAT_OUT=%%.16g %s", source, GMT->common.R.string, GMT->common.J.string, last_col, TCLOCK);
		GMT_Report (API, GMT_MSG_INFORMATION, "Line sampling Step 3: %s.\n", cmd);
		if (GMT_Call_Module (GMT->parent, "mapproject", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {	/* Recover original coordinates by inversely project the data and write to stdout */
			Return (API->error);
		}
		/* Close the latest source virtual file and destroy the data set */
		if (GMT_Close_VirtualFile (GMT->parent, source) != GMT_NOERROR) {
			Return (API->error);
		}
		if (GMT_Destroy_Data (API, &D) != GMT_NOERROR) {	/* Completely done with the original dataset */
			Return (API->error);
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Line sampling written to standard output\n");

		Return (GMT_NOERROR);
	}

	/* Now we are ready to take on some input values */

	n_cols_needed = 3;	/* We always will need lon, lat and time */
	if (Ctrl->Z.active) {	/* We read points for symbols */
		unsigned int n_cols = psevents_determine_columns (GMT, Ctrl->Z.module, Ctrl->Z.cmd, Ctrl->S.mode);	/* Must allow for number of columns needed by the selected module */
		if (n_cols == 0) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to determine columns.  Bad module %s?\n", Ctrl->Z.module);
			Return (GMT_RUNTIME_ERROR);
		}
		n_cols_needed = 1;	/* Since the lon, lat is included in n_cols for -Z */
		n_cols_needed += n_cols;	/* One more for time so far */
		n_copy_to_out = n_cols;	/* lon, lat, and all required cols, no time */
		t_in = n_cols, d_in = t_in + 1;	/* These are the 1-2 extra columns needed by event */
		x_col = n_cols, i_col = n_cols + 1, t_col = n_cols + 2;	/* Update output cols for the 3 extras */
		if (Ctrl->L.mode == PSEVENTS_VAR_DURATION || Ctrl->L.mode == PSEVENTS_VAR_ENDTIME) n_cols_needed++;	/* Must allow for length/time column in input */
	}
	else if (Ctrl->S.active || !Ctrl->A.active) {	/* We read points for symbols OR we are in fact just doing labels */
		if (Ctrl->C.active) n_cols_needed++;	/* Must allow for z in input */
		if (Ctrl->S.mode) n_cols_needed++;	/* Must allow for size in input */
		if (Ctrl->L.mode == PSEVENTS_VAR_DURATION || Ctrl->L.mode == PSEVENTS_VAR_ENDTIME) n_cols_needed++;	/* Must allow for length/time in input */
		n_copy_to_out = 2 + Ctrl->C.active + Ctrl->S.mode;
	}
	else {	/* We read lines or polygons */
		n_cols_needed = n_copy_to_out = 2;
		if (Ctrl->A.mode == PSEVENTS_LINE_REC) {
			n_cols_needed++;	/* For event time */
			if (Ctrl->L.mode == PSEVENTS_VAR_DURATION || Ctrl->L.mode == PSEVENTS_VAR_ENDTIME) n_cols_needed++;	/* Must allow for length/time in input */
		}
	}
	GMT_Set_Columns (API, GMT_IN, n_cols_needed, GMT_COL_FIX);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {		/* Enables data input and sets access mode */
		Return (API->error);
	}

	if (!Ctrl->Z.active) {
		if (Ctrl->C.active) s_in++, t_in++, d_in++, x_col++, i_col++, t_col++;	/* Must allow for z-value in input/output before size, time, length */
		if (Ctrl->S.mode) t_in++, d_in++, x_col++, i_col++, t_col++;	/* Must allow for size in input/output before time and length */
	}
	/* Determine if there is a coda phase where symbols remain visible after the event ends: */
	do_coda = (Ctrl->M.value[PSEVENTS_SIZE][PSEVENTS_VAL2] > 0.0 || !gmt_M_is_zero (Ctrl->M.value[PSEVENTS_INT][PSEVENTS_VAL2]) || Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL2] < 100.0);
	finite_duration = (Ctrl->L.mode != PSEVENTS_INFINITE);
	time_type = gmt_M_type (GMT, GMT_IN, t_in);
	end_type = (Ctrl->L.mode == PSEVENTS_VAR_ENDTIME) ? time_type : GMT_IS_FLOAT;
	find = (time_type == GMT_IS_ABSTIME) ? ',' : '/';
	x_type = gmt_M_type (GMT, GMT_IN, GMT_X);
	y_type = gmt_M_type (GMT, GMT_IN, GMT_Y);
	if (x_type == GMT_IS_ABSTIME || y_type == GMT_IS_ABSTIME) {	/* Force precision of msec */
		strncpy (GMT->current.setting.format_clock_out, "hh:mm:ss.xxx", GMT_LEN64-1);
		gmtlib_clock_C_format (GMT, GMT->current.setting.format_clock_out, &GMT->current.io.clock_output, 1);
	}

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				if (fp_labels) fclose (fp_labels);
				if (fp_symbols) fclose (fp_symbols);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 	/* Reached end of file */
				break;
			else if (gmt_M_rec_is_segment_header (GMT) && Ctrl->A.active) {	/* Process and echo any segment headers for lines and polygons */
				if (Ctrl->A.mode == PSEVENTS_LINE_SEG) {	/* We require the -Tstring option in the header for segments */
					if (gmt_parse_segment_item (GMT, GMT->current.io.segment_header, "-T", string)) {	/* Found required -Targs */
						if (Ctrl->L.mode == PSEVENTS_FIXED_DURATION) {	/* Just get event time */
							gmt_scanf_arg (GMT, string, time_type, false, &t_event_seg);
						}
						else {	/* Get both event time and end time (or duration) */
							char start[GMT_LEN64] = {""}, stop[GMT_LEN64] = {""};
							if ((c = strchr (string, ','))) c[0] = ' ';
							else if ((c = strchr (string, '/'))) c[0] = ' ';
							else {
								GMT_Report (API, GMT_MSG_ERROR, "Segment header missing required -T<time>,<end|duration> or -T<time>/<end|duration> option!\n");
								if (fp_labels) fclose (fp_labels);
								if (fp_symbols) fclose (fp_symbols);
								Return (GMT_RUNTIME_ERROR);
							}
							sscanf (string, "%s %s", start, stop);
							gmt_scanf_arg (GMT, start, time_type, false, &t_event_seg);
							gmt_scanf_arg (GMT, stop,  end_type,  false, &t_end_seg);
						}
					}
					else {	/* We require the -Tstring option in the header for segments */
						GMT_Report (API, GMT_MSG_ERROR, "Segment header missing required -T<time> option!\n");
						if (fp_labels) fclose (fp_labels);
						if (fp_symbols) fclose (fp_symbols);
						Return (GMT_RUNTIME_ERROR);
					}
				}
				/* build output segment header: Pass any -Z<val>, -G<fill> -W<pen> as given, but if no -G, -W and command line has them we add them.
				 * We do this because -t<transparency> will also be added and this is how the -G -W colors will acquire transparency in psxy */
				gmt_M_memset (header, GMT_BUFSIZ, char);	/* Wipe header record */
				if (Ctrl->C.active && gmt_parse_segment_item (GMT, GMT->current.io.segment_header, "-Z", string)) {	/* Found optional -Z<val> and -C<cpt> is in effect */
					strcat (header, " -Z"); strcat (header, string);
				}
				if (gmt_parse_segment_item (GMT, GMT->current.io.segment_header, "-W", string)) {	/* Found optional -W<pen> */
					strcat (header, " -W"); strcat (header, string);
				}
				else if (Ctrl->W.active) {	/* Issue the current pen instead */
					strcat (header, " -W"); strcat (header, Ctrl->W.pen);
				}
				if (Ctrl->A.mode == PSEVENTS_LINE_SEG) {	/* Deal with optional -G<fill> */
					if (gmt_parse_segment_item (GMT, GMT->current.io.segment_header, "-G", string)) {	/* Found optional -G<fill> */
						strcat (header, " -G"); strcat (header, string);
					}
					else if (Ctrl->G.active) {	/* Issue the current fill instead */
						strcat (header, " -G"); strcat (header, Ctrl->G.fill);
					}
				}
				out_segment = true;	/* Time to echo out a segment header */
			}
			else if (gmt_M_rec_is_any_header (GMT)) {	/* Skip all types of headers */
				/* Do nothing */
			}
			continue;	/* Go back and read the next record */
		}
		if (In->data == NULL) {
			gmt_quit_bad_record (API, In);
			Return (API->error);
		}

		/* Data record to process */

		in = In->data;	/* The numerical part */
		n_total_read++;	/* Successfully read an input record */

		if (Ctrl->A.mode == PSEVENTS_LINE_SEG) {	/* Assign new segment start/end times for lines/polygons */
			in[t_in] = t_event_seg;	/* Current segment event start time */
			if (Ctrl->L.mode != PSEVENTS_FIXED_DURATION)	/* Set segment end time or duration */
				in[d_in] = t_end_seg;
		}

		if (Ctrl->E.active[PSEVENTS_SYMBOL]) {	/* Plot event symbols */
			t_end = DBL_MAX;	/* Infinite duration until overridden below */
			t_event = in[t_in] + Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_OFFSET];	/* Nominal (or offset) start of this event */
			t_rise = t_event - Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_RISE];		/* Earliest time to plot anything at all for this event */
			if (Ctrl->T.now < t_rise) goto Do_txt;	/* This event is still in the future so we skip it */
			/* Compute the last time we need to plot the event [infinity] */
			if (Ctrl->L.mode == PSEVENTS_FIXED_DURATION)	/* Only show the event as stable during this fixed interval */
				t_end = t_event + Ctrl->L.length;
			else if (Ctrl->L.mode == PSEVENTS_VAR_DURATION)	/* Only show the event as stable during its individual interval */
				t_end = t_event + in[d_in];
			else if (Ctrl->L.mode == PSEVENTS_VAR_ENDTIME)	/* Only show the event as stable until its end time */
				t_end = in[d_in];
			if (t_end < DBL_MAX && Ctrl->E.trim[PSEVENTS_SYMBOL]) t_end -= Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_OFFSET];	/* Offset applied to t_end */
			t_fade = t_end + Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_FADE];		/* End of the fade phase */
			if (!do_coda && Ctrl->T.now > t_fade) goto Do_txt;	/* Event is in the past and there is no coda, so skip plotting it */

			/* Here we must plot one phase of this event */

			if (n_symbols_plotted == 0) {	/* Open output events file the first time */
				if (Ctrl->Q.active)	/* We want a persistent file to survive this process */
					sprintf (tmp_file_symbols, "%s_symbols.txt", Ctrl->Q.file);
				else	/* Temporary file to be deleted after use */
					sprintf (tmp_file_symbols, "%s/GMT_psevents_symbols_%d.txt", API->tmp_dir, (int)getpid());
				if ((fp_symbols = fopen (tmp_file_symbols, "w")) == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "Unable to create file %s\n", tmp_file_symbols);
					if (fp_labels) fclose (fp_labels);
					Return (GMT_RUNTIME_ERROR);
				}
			}
			gmt_M_memcpy (out, in, n_copy_to_out, double);	/* Pass out the key input parameters unchanged (but not time, duration) */

			t_plateau = t_event + Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_PLATEAU];	/* End of the plateau phase */
			t_decay = t_plateau + Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_DECAY];	/* End of the decay phase */
			if (Ctrl->T.now < t_event) {	/* We are within the rise phase */
				x = pow ((Ctrl->T.now - t_rise)/Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_RISE], 2.0);	/* Quadratic function that goes from 0 to 1 */
				if (gmt_M_is_dnan (x)) x = 0.0;	/* Probably division by zero */
				out[x_col] = Ctrl->M.value[PSEVENTS_SIZE][PSEVENTS_VAL1] * x;	/* Magnification of amplitude */
				out[i_col] = Ctrl->M.value[PSEVENTS_INT][PSEVENTS_VAL1] * x;		/* Magnification of intensity */
				out[t_col] = Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL1] * (1.0-x);	/* Magnification of opacity */
			}
			else if (Ctrl->T.now < t_plateau) {	/* We are within the plateau phase, keep everything constant */
				out[x_col] = Ctrl->M.value[PSEVENTS_SIZE][PSEVENTS_VAL1];
				out[i_col] = Ctrl->M.value[PSEVENTS_INT][PSEVENTS_VAL1];
				out[t_col] = 0.0;	/* No transparency during plateau phase */
			}
			else if (Ctrl->T.now < t_decay) {	/* We are within the decay phase */
				x = pow ((t_decay - Ctrl->T.now)/Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_DECAY], 2.0);	/* Quadratic function that goes from 1 to 0 */
				if (gmt_M_is_dnan (x)) x = 0.0;	/* Probably division by zero */
				out[x_col] = Ctrl->M.value[PSEVENTS_SIZE][PSEVENTS_VAL1] * x + (1.0 - x);	/* Reduction of size down to the nominal size */
				out[i_col] = Ctrl->M.value[PSEVENTS_INT][PSEVENTS_VAL1] * x;	/* Reduction of intensity down to 0 */
				out[t_col] = 0.0;
			}
			else if (finite_duration && Ctrl->T.now < t_end) {	/* We are within the normal display phase with nominal symbol size */
				out[x_col] = 1.0;
				out[i_col] = out[t_col] = 0.0;	/* No intensity or transparency during normal phase */
			}
			else if (finite_duration && Ctrl->T.now <= t_fade) {	/* We are within the fade phase */
				x = pow ((t_fade - Ctrl->T.now)/Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_FADE], 2.0);	/* Quadratic function that goes from 1 to 0 */
				if (gmt_M_is_dnan (x)) x = 0.0;	/* Probably division by zero */
				out[x_col] = x + (1.0 - x) * Ctrl->M.value[PSEVENTS_SIZE][PSEVENTS_VAL2];	/* Reduction of size down to coda size */
				out[i_col] = Ctrl->M.value[PSEVENTS_INT][PSEVENTS_VAL2] * (1.0 - x);		/* Reduction of intensity down to coda intensity */
				out[t_col] = Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL2] * (1.0 - x);		/* Increase of transparency up to code transparency */
			}
			else if (do_coda) {	/* If there is a coda then the symbol remains visible given those terminal attributes */
				out[x_col] = Ctrl->M.value[PSEVENTS_SIZE][PSEVENTS_VAL2];
				out[i_col] = Ctrl->M.value[PSEVENTS_INT][PSEVENTS_VAL2];
				out[t_col] = Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL2];
			}
			if (out_segment) {	/* Write segment header for lines and polygons only */
				fprintf (fp_symbols, "%c -t%g %s\n", GMT->current.setting.io_seg_marker[GMT_OUT], out[t_col], header);
				out_segment = false;	/* Wait for next segment header */
			}
			psevents_set_XY (GMT, x_type, y_type, out, X, Y);

			fprintf (fp_symbols, "%s\t%s", X, Y);	/* All need the map coordinates */
			if (Ctrl->Z.active) {	/* A variable set of coordinates */
				for (col = GMT_Z; col <= t_col; col++)
					fprintf (fp_symbols, "\t%g", out[col]);	/* Write out all required and extra columns */
				if (In->text && In->text[0])	/* Also output the trailing text */
					fprintf (fp_symbols, "\t%s", In->text);
				fprintf (fp_symbols, "\n");
			}
			else if (Ctrl->A.active)	/* Just needed the line coordinates */
				fprintf (fp_symbols, "\n");
			else {	/* Symbols -S, which may need -C and -S columns before the scale, intens, transp */ 
				for (col = GMT_Z; col < n_copy_to_out; col++)
					fprintf (fp_symbols, "\t%.16g", out[col]);
				fprintf (fp_symbols, "\t%g\t%g\t%g\n", out[x_col], out[i_col], out[t_col]);
			}
			n_symbols_plotted++;	/* Count output symbols */
		}

Do_txt:	if (Ctrl->E.active[PSEVENTS_TEXT] && In->text) {	/* Also plot trailing text strings */
			t_end = DBL_MAX;	/* Infinite duration until overridden below */
			t_event = in[t_in] + Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_OFFSET];	/* Nominal (or offset) start of this event */
			t_rise = t_event - Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_RISE];	/* Earliest time to plot anything at all for this event */
			if (Ctrl->T.now < t_rise) continue;	/* This event is still in the future */
			/* Compute the last time we need to plot the event [infinity] */
			if (Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_LENGTH] > 0.0)	/* Overriding with specific label duration */
				t_end = t_event + Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_LENGTH];
			else if (Ctrl->L.mode == PSEVENTS_FIXED_DURATION)	/* Only show the label during this fixed interval given via -L or -Et+l */
				t_end = t_event + Ctrl->L.length;
			else if (Ctrl->L.mode == PSEVENTS_VAR_DURATION)	/* Only show the label during its individual interval read from file */
				t_end = t_event + in[d_in];
			else if (Ctrl->L.mode == PSEVENTS_VAR_ENDTIME)	/* Only show the label until its end time read from file */
				t_end = in[d_in];
			if (t_end < DBL_MAX && Ctrl->E.trim[PSEVENTS_TEXT]) t_end -= Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_OFFSET];	/* Offset applied to t_end */
			t_fade = t_end + Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_FADE];	/* End of the fade phase */
			if (!do_coda && Ctrl->T.now > t_fade) continue;				/* Event is in the past and there is no coda */

			/* Here we must plot a label during one phase of this event */

			if (n_labels_plotted == 0) {	/* Open output events file the first time */
				if (Ctrl->Q.active)	/* We want a persistent file to survive this process */
					sprintf (tmp_file_labels, "%s_labels.txt", Ctrl->Q.file);
				else	/* Temporary file to be deleted after use */
					sprintf (tmp_file_labels, "%s/GMT_psevents_labels_%d.txt", API->tmp_dir, (int)getpid());
				if ((fp_labels = fopen (tmp_file_labels, "w")) == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "Unable to create file %s\n", tmp_file_labels);
					Return (GMT_RUNTIME_ERROR);
				}
			}
			psevents_set_XY (GMT, x_type, y_type, in, X, Y);	/* Pass out the input coordinates unchanged */

			/* Labels have variable transparency during optional rise and fade, and fully opaque during normal section, and skipped otherwise unless coda */

			if (Ctrl->T.now < t_event) {	/* We are within the rise phase */
				x = (gmt_M_is_zero (Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_RISE])) ? 1.0 : pow ((Ctrl->T.now - t_rise)/Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_RISE], 2.0);	/* Quadratic function that goes from 1 to 0 */
				out[GMT_Z] = Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL1] * (1.0-x);		/* Magnification of opacity */
			}
			else if (finite_duration && Ctrl->T.now < t_end)	/* We are within the normal phase, keep everything constant */
				out[GMT_Z] = 0.0;	/* No transparency during this phase */
			else if (finite_duration && Ctrl->T.now <= t_fade) {	/* We are within the fade phase */
				x = (gmt_M_is_zero (Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_FADE])) ? 0.0 : pow ((t_fade - Ctrl->T.now)/Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_FADE], 2.0);	/* Quadratic function that goes from 1 to 0 */
				out[GMT_Z] = Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL2] * (1.0 - x);		/* Increase of transparency up to coda transparency */
			}
			else if (do_coda)	/* If there is a coda then the label is visible given its coda attributes */
				out[GMT_Z] = Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL2];
			fprintf (fp_labels, "%s\t%s\t%g\t%s\n", X, Y, out[GMT_Z], In->text);
			n_labels_plotted++;	/* Count output labels */
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		if (fp_symbols) fclose (fp_symbols);
		if (fp_labels) fclose (fp_labels);
		Return (API->error);
	}
	if (gmt_map_setup (GMT, GMT->common.R.wesn)) {	/* Set up map projection */
		if (fp_symbols) fclose (fp_symbols);
		if (fp_labels) fclose (fp_labels);
		Return (GMT_PROJECTION_ERROR);
	}

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	gmt_set_basemap_orders (GMT, (Ctrl->N.active && strchr (Ctrl->N.arg, 'r')) ? GMT_BASEMAP_FRAME_BEFORE : GMT_BASEMAP_FRAME_AFTER, GMT_BASEMAP_GRID_BEFORE, GMT_BASEMAP_ANNOT_AFTER);
	gmt_map_basemap (GMT);	/* Plot basemap if requested */

	if (fp_symbols) { /* Here we have event symbols to plot as an overlay via a call to plot */
		char *module = (GMT->current.setting.run_mode == GMT_CLASSIC) ? "psxy" : "plot";	/* Default module to use */
		fclose (fp_symbols);	/* First close the file so symbol output is flushed */
		/* Build plot command with fixed options and those that depend on -C -G -W.
		 * We must set symbol unit as inch since we are passing sizes in inches directly (dimensions are in inches internally in GMT).  */
		if (Ctrl->A.active) {	/* Command to plot lines or polygons */
			sprintf (cmd, "%s -R -J -O -K --GMT_HISTORY=readonly", tmp_file_symbols);
			if (Ctrl->A.mode == PSEVENTS_LINE_REC && Ctrl->W.pen) {strcat (cmd, " -W"); strcat (cmd, Ctrl->W.pen);}
			if (Ctrl->A.mode == PSEVENTS_LINE_SEG && Ctrl->C.active) {strcat (cmd, " -C"); strcat (cmd, Ctrl->C.file);}
		}
		else if (Ctrl->Z.active) {
			sprintf (cmd, "%s %s -R -J -O -K -H -I -t --GMT_HISTORY=readonly --PROJ_LENGTH_UNIT=%s", tmp_file_symbols, Ctrl->Z.cmd, GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			if (Ctrl->C.active) {strcat (cmd, " -C"); strcat (cmd, Ctrl->C.file);}
			if (Ctrl->G.active) {strcat (cmd, " -G"); strcat (cmd, Ctrl->G.fill);}
			if (Ctrl->W.pen) {strcat (cmd, " -W"); strcat (cmd, Ctrl->W.pen);}
			if (Ctrl->N.active) strcat (cmd, Ctrl->N.arg);
			module = (GMT->current.setting.run_mode == GMT_MODERN && !strncmp (Ctrl->Z.module, "ps", 2U)) ? &Ctrl->Z.module[2] : Ctrl->Z.module;
		}
		else {	/* Command to plot symbols */
			sprintf (cmd, "%s -R -J -O -K -H -I -t -S%s --GMT_HISTORY=readonly --PROJ_LENGTH_UNIT=%s", tmp_file_symbols, Ctrl->S.symbol, GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
			if (Ctrl->C.active) {strcat (cmd, " -C"); strcat (cmd, Ctrl->C.file);}
			if (Ctrl->G.active) {strcat (cmd, " -G"); strcat (cmd, Ctrl->G.fill);}
			if (Ctrl->W.pen) {strcat (cmd, " -W"); strcat (cmd, Ctrl->W.pen);}
			if (Ctrl->N.active) strcat (cmd, Ctrl->N.arg);
		}
		GMT_Report (API, GMT_MSG_DEBUG, "cmd: gmt %s %s\n", module, cmd);

		if (GMT_Call_Module (API, module, GMT_MODULE_CMD, cmd) != GMT_NOERROR) {	/* Plot the symbols */
			if (fp_labels) fclose (fp_labels);
			Return (API->error);
		}
		if (!Ctrl->Q.active && gmt_remove_file (GMT, tmp_file_symbols)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to remove file %s\n", tmp_file_symbols);
			if (fp_labels) fclose (fp_labels);
			Return (GMT_RUNTIME_ERROR);
		}
	}
	if (fp_labels) { /* Here we have event labels to plot as an overlay via a call to pstext */
		char *module = (GMT->current.setting.run_mode == GMT_CLASSIC) ? "pstext" : "text";
		fclose (fp_labels);	/* First close the file so label output is flushed */
		/* Build pstext command with fixed options and those that depend on -D -F -H */
		sprintf (cmd, "%s -R -J -O -K -t --GMT_HISTORY=readonly", tmp_file_labels);
		if (Ctrl->D.active) {strcat (cmd, " -D"); strcat (cmd, Ctrl->D.string);}
		if (Ctrl->F.active) {strcat (cmd, " -F"); strcat (cmd, Ctrl->F.string);}
		if (Ctrl->N.active) strcat (cmd, " -N");
		if (Ctrl->H.active) {	/* Combinations of -Cdx/dy[+tO], -G<fill>, -S<dx>/<dy>/<shade>, -W<pen> */
			strcat (cmd, " -C"); strcat (cmd, Ctrl->H.clearance);
			if (Ctrl->H.box) strcat (string, "+tO");
			strcat (cmd, string);
			if (Ctrl->H.boxfill) {strcat (cmd, " -G"); strcat (cmd, Ctrl->H.fill);}
			if (Ctrl->H.boxshade) {
				sprintf (string, " -S%lgi/%lgi/%s", Ctrl->H.soff[GMT_X], Ctrl->H.soff[GMT_Y], Ctrl->H.sfill);
				strcat (cmd, string);
			}
			if (Ctrl->H.boxoutline) {strcat (cmd, " -W"); strcat (cmd, Ctrl->H.pen);}
		}
		GMT_Report (API, GMT_MSG_DEBUG, "cmd: gmt %s %s\n", module, cmd);

		if (GMT_Call_Module (API, module, GMT_MODULE_CMD, cmd) != GMT_NOERROR) {	/* Plot the labels */
			Return (API->error);
		}
		if (!Ctrl->Q.active && gmt_remove_file (GMT, tmp_file_labels)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to remove file %s\n", tmp_file_labels);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	/* Finalize plot and we are done */

	gmt_map_basemap (GMT);	/* Plot basemap if requested */
	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);

	GMT_Report (API, GMT_MSG_INFORMATION, "Events read: %" PRIu64 " Event symbols plotted: %" PRIu64 " Event labels plotted: %" PRIu64 "\n", n_total_read, n_symbols_plotted, n_labels_plotted);

	Return (GMT_NOERROR);
}

EXTERN_MSC int GMT_events (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: events\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psevents (V_API, mode, args);
}
