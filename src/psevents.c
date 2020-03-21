/*
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
#define THIS_MODULE_PURPOSE	"Plot event symbols and labels for a moment in time"
#define THIS_MODULE_KEYS	"<D{,CC(,>X}"
#define THIS_MODULE_NEEDS	"JR"
#define THIS_MODULE_OPTIONS	"-:>BJKOPRUVXYabdefhipq"

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
	PSEVENTS_VAR_ENDTIME = 3
};

#define PSEVENTS_MODS "dfloOpr"

/* Control structure for psevents */

struct PSEVENTS_CTRL {
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
	struct PSEVENTS_G {	/* 	-G<color> */
		bool active;
		char *color;
	} G;
	struct PSEVENTS_L {	/* 	-L[t|<length>] */
		bool active;
		unsigned int mode;
		double length;
	} L;
	struct PSEVENTS_M {	/* 	-M[i|s|t]<val1>[+c<val2] */
		bool active[3];
		double value[3][2];
	} M;
	struct PSEVENTS_Q {	/* 	-Q<prefix> */
		bool active;
		char *file;
	} Q;
	struct PSEVENTS_S {	/* 	-S<symbol>[<size>] */
		bool active;
		unsigned int mode;
		char *symbol;
		double size;
	} S;
	struct PSEVENTS_T {	/* 	-T<nowtime> */
		bool active;
		double now;
	} T;
	struct PSEVENTS_W {	/* 	-W<pen> */
		bool active;
		char *pen;
	} W;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSEVENTS_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSEVENTS_CTRL);
	C->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL1] = C->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL2] = 100.0;	/* Rise from and fade to invisibility */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSEVENTS_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->D.string);
	gmt_M_str_free (C->F.string);
	gmt_M_str_free (C->G.color);
	gmt_M_str_free (C->Q.file);
	gmt_M_str_free (C->S.symbol);
	gmt_M_str_free (C->W.pen);
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] %s %s -S<symbol>[<size>]\n", name, GMT_J_OPT, GMT_Rgeoz_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t-T<now> [-C<cpt>] [-D[j|J]<dx>[/<dy>][+v[<pen>]] [-E[s|t][+o|O<dt>][+r<dt>][+p<dt>][+d<dt>][+f<dt>][+l<dt>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-F[+a<angle>][+f<font>][+r[<first>]|+z[<fmt>]][+j<justify>]] [-G<color>] [-L[t|<length>]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-Mi|s|t<val1>[+c<val2]] [-Q<prefix>] [-W[<pen>] [%s] [%s]\n", GMT_V_OPT, GMT_b_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t%s[%s] [%s] [%s] [%s]\n\t[%s] [%s] [%s] [%s]\n\n",
		API->c_OPT, GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_h_OPT, GMT_i_OPT, GMT_qi_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Option (API, "J-Z,R");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Append symbol code and optionally <size>.  If no size we read it from the data file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify the time for preparing events.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t   Record format: lon lat [z] [size] time [length|time2].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Give <cpt> and obtain symbol color via z-value in 3rd data column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Add <add_x>,<add_y> to the event text origin AFTER projecting with -J [0/0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Dj to move text origin away from point (direction determined by text's justification).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Upper case -DJ will shorten diagonal shifts at corners by sqrt(2).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +v[<pen>] to draw line from text to original point.  If <add_y> is not given it equals <add_x>.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set offset, rise, plateau, decay, and fade intervals, for symbols (-Es [Default]) or text (-Et):\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t-L Set finite length of events, otherwise we assume they are all infinite.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no arg we read lengths from file; append t for reading end times instead.\n");
	gmt_fill_syntax (API->GMT, 'G', NULL, "Specify a fixed symbol color [no fill].");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Append i for intensity, s for size, or t for transparency; repeatable.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append value to use during rise, plateau, or decay phases.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c to set a separate terminal value for the coda [no coda].\n");
	GMT_Option (API, "K,O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Save intermediate events symbol and labels files; append file prefix [temporary files deleted].\n");
	gmt_pen_syntax (API->GMT, 'W', NULL, "Set symbol outline pen attributes [Default pen is %s]:", 0);
	GMT_Option (API, "V,bi2,c,di,e,f,h,i,p,qi,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSEVENTS_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psevents and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, pos, n_col = 3, k = 0, id = 0;
	char *c = NULL, txt[GMT_LEN128] = {""}, *t_string = NULL;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */

			case 'C':	/* Set a cpt for converting z column to color */
				Ctrl->C.active = true;
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;

			case 'D':
				Ctrl->D.active = true;
				Ctrl->D.string = strdup (opt->arg);
				break;

			case 'E':	/* Set event times. If -T is abstime then these are in units of TIME_UNIT [s] */
				switch (opt->arg[0]) {
					case 's':	id = PSEVENTS_SYMBOL;	k = 1;	break;
					case 't':	id = PSEVENTS_TEXT;		k = 1;	break;
					default:	id = PSEVENTS_SYMBOL;	k = 0;	break;
				}
				Ctrl->E.active[id] = true;
				if (gmt_validate_modifiers (GMT, &opt->arg[k], 'C', PSEVENTS_MODS)) n_errors++;
				if ((c = gmt_first_modifier (GMT, &opt->arg[k], PSEVENTS_MODS)) == NULL) {	/* This should not happen given the above check, but Coverity prefers it */
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -E: No modifiers given?\n");
					n_errors++;
					break;
				}
				pos = 0;	txt[0] = 0;
				while (gmt_getmodopt (GMT, 'C', c, PSEVENTS_MODS, &pos, txt, &n_errors) && n_errors == 0) {
					switch (txt[0]) {
						case 'd': Ctrl->E.dt[id][PSEVENTS_DECAY]   = atof (&txt[1]);	break;	/* Decay duration */
						case 'f': Ctrl->E.dt[id][PSEVENTS_FADE]    = atof (&txt[1]);	break;	/* Fade duration */
						case 'p': Ctrl->E.dt[id][PSEVENTS_PLATEAU] = atof (&txt[1]);	break;	/* Plateau duration */
						case 'r': Ctrl->E.dt[id][PSEVENTS_RISE]    = atof (&txt[1]);	break;	/* Rise duration */
						case 'O': Ctrl->E.trim[id] = true;	/* Intentionally fall through - offset start but not end. Fall through to case 'o' */
						case 'o': Ctrl->E.dt[id][PSEVENTS_OFFSET]   = atof (&txt[1]);	break;	/* Event time offset */
						case 'l':	/* Event length override for text */
							if (id == PSEVENTS_SYMBOL) {
								GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -E[s]: The +l modifier is only allowed for -Et\n");
								n_errors++;
							}
							else
								Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_LENGTH] = atof (&txt[1]);
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
				Ctrl->F.string = strdup (opt->arg);
				break;

			case 'G':	/* Set a fixed symbol color */
				Ctrl->G.active = true;
				Ctrl->G.color = strdup (opt->arg);
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

			case 'Q':	/* Save events file for posterity */
				Ctrl->Q.active = true;
				Ctrl->Q.file = strdup (opt->arg);
				break;

			case 'S':	/* Set symbol type and size (append units) */
				Ctrl->S.active = true;
				if (strchr ("kK", opt->arg[0])) {	/* Custom symbol may have a slash before size */
					if ((c = strrchr (opt->arg, '/'))) {	/* Gave size */
						Ctrl->S.size = gmt_M_to_inch (GMT, &c[1]);
						c[0] = '0';
						Ctrl->S.symbol = strdup (opt->arg);
						c[0] = '/';
						GMT->current.setting.proj_length_unit = GMT_INCH;	/* Since S.size is now in inches */
					}
					else {	/* Gave no size so get the whole thing and read size from file */
						Ctrl->S.symbol = strdup (opt->arg);
						Ctrl->S.mode = 1;
					}
				}
				else if (opt->arg[0] == 'E' && opt->arg[1] == '-') {
					if (opt->arg[2])	/* Gave a fixed size */
						Ctrl->S.size = atof (&opt->arg[2]);
					else	/* Must read individual event symbol sizes for file using prevailing length-unit setting */
						Ctrl->S.mode = 1;
					Ctrl->S.symbol = strdup ("E-");
				}
				else if (strchr ("-+aAcCdDgGhHiInNsStTxy", opt->arg[0])) {	/* Regular symbols of form <code>[<size>], where <code> is 1-char */
					if (opt->arg[1] && !strchr (GMT_DIM_UNITS, opt->arg[1])) {	/* Gave a fixed size */
						Ctrl->S.size = gmt_M_to_inch (GMT, &opt->arg[1]);	/* Now in inches */
						GMT->current.setting.proj_length_unit = GMT_INCH;	/* Since S.size is now in inches */
						sprintf (txt, "%c", opt->arg[0]);			/* Just the symbol code */
						Ctrl->S.symbol = strdup (txt);
					}
					else if (opt->arg[1] && strchr (GMT_DIM_UNITS, opt->arg[1])) {	/* Must read symbol sizes in this unit from file */
						Ctrl->S.mode = 1;
						gmt_set_measure_unit (GMT, opt->arg[1]);
						sprintf (txt, "%c", opt->arg[0]);	/* Just the symbol code */
						Ctrl->S.symbol = strdup (txt);
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
				Ctrl->W.pen = strdup (opt->arg);
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
	else {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -T: -T<now> is a required option.\n");
		n_errors++;
	}
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = n_col;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < n_col, "Binary input data (-bi) must have at least %u columns.\n", n_col);
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && Ctrl->G.active, "Cannot specify both -C and -G.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.active && (!Ctrl->C.active && !Ctrl->G.active && !Ctrl->W.active), "Must specify at least one of -C, -G, -W to plot visible symbols.\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.R.active[RSET], "Must specify -R option\n");
	n_errors += gmt_M_check_condition (GMT, !GMT->common.J.active, "Must specify a map projection with the -J option\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}


/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_events (void *V_API, int mode, void *args) {
	/* This is the GMT6 modern mode name */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC && !API->usage) {
		GMT_Report (API, GMT_MSG_ERROR, "Shared GMT module not found: events\n");
		return (GMT_NOT_A_VALID_MODULE);
	}
	return GMT_psevents (V_API, mode, args);
}

int GMT_psevents (void *V_API, int mode, void *args) {
	char tmp_file_symbols[PATH_MAX] = {""}, tmp_file_labels[PATH_MAX] = {""}, cmd[BUFSIZ] = {""};

	bool do_coda, finite_duration;

	int error;

	unsigned int n_cols_needed = 3, s_in = 2, t_in = 2, d_in = 3, s_col = 2, i_col = 3, t_col = 4;

	uint64_t n_total_read = 0, n_symbols_plotted = 0, n_labels_plotted = 0;

	double out[6] = {0, 0, 0, 0, 0, 0}, t_end = DBL_MAX, *in = NULL;
	double t_event, t_rise, t_plateau, t_decay, t_fade, x, size;

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

	GMT_Report (API, GMT_MSG_INFORMATION, "Processing input table data\n");

	/* Now we are ready to take on some input values */

	/* We read as points. */
	if (Ctrl->C.active) n_cols_needed++;	/* Must allow for z in input */
	if (Ctrl->S.mode) n_cols_needed++;	/* Must allow for size in input */
	if (Ctrl->L.mode == PSEVENTS_VAR_DURATION || Ctrl->L.mode == PSEVENTS_VAR_ENDTIME) n_cols_needed++;	/* Must allow for length/time in input */
	GMT_Set_Columns (API, GMT_IN, n_cols_needed, GMT_COL_FIX);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {		/* Enables data input and sets access mode */
		Return (API->error);
	}

	if (Ctrl->C.active) s_in++, t_in++, d_in++, s_col++, i_col++, t_col++;	/* Must allow for z-value in input before size, time, length */
	if (Ctrl->S.mode) t_in++, d_in++;	/* Must allow for size in input before time and length */
	/* Determine if there is a coda phase where symbols remain visible after the event ends: */
	do_coda = (Ctrl->M.value[PSEVENTS_SIZE][PSEVENTS_VAL2] > 0.0 || !gmt_M_is_zero (Ctrl->M.value[PSEVENTS_INT][PSEVENTS_VAL2]) || Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL2] < 100.0);
	finite_duration = (Ctrl->L.mode != PSEVENTS_INFINITE);

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				if (fp_labels) fclose (fp_labels);
				if (fp_symbols) fclose (fp_symbols);
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 	/* Reached end of file */
				break;
			else if (gmt_M_rec_is_any_header (GMT)) {	/* Skip all types of headers */
				/* Do nothing */
			}
			continue;	/* Go back and read the next record */
		}

		/* Data record to process */

		in = In->data;	/* The numerical part */
		n_total_read++;	/* Successfully read an input record */

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
				else	/* Temporariy file to be deleted after use */
					sprintf (tmp_file_symbols, "%s/GMT_psevents_symbols_%d.txt", API->tmp_dir, (int)getpid());
				if ((fp_symbols = fopen (tmp_file_symbols, "w")) == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "Unable to create file %s\n", tmp_file_symbols);
					if (fp_labels) fclose (fp_labels);
					Return (GMT_RUNTIME_ERROR);
				}
			}
			out[GMT_X] = in[GMT_X];	out[GMT_Y] = in[GMT_Y];	/* Pass out the input coordinates unchanged */
			if (Ctrl->C.active) out[GMT_Z] = in[GMT_Z];	/* Also pass along the optional z-value */

			t_plateau = t_event + Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_PLATEAU];	/* End of the plateau phase */
			t_decay = t_plateau + Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_DECAY];	/* End of the decay phase */
			size = (Ctrl->S.mode) ? in[s_in] : Ctrl->S.size;	/* Fixed or variable nominal symbol size */
			if (Ctrl->T.now < t_event) {	/* We are within the rise phase */
				x = pow ((Ctrl->T.now - t_rise)/Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_RISE], 2.0);	/* Quadratic function that goes from 0 to 1 */
				out[s_col] = Ctrl->M.value[PSEVENTS_SIZE][PSEVENTS_VAL1] * x * size;	/* Magnification of amplitude */
				out[i_col] = Ctrl->M.value[PSEVENTS_INT][PSEVENTS_VAL1] * x;		/* Magnification of intensity */
				out[t_col] = Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL1] * (1.0-x);	/* Magnification of opacity */
			}
			else if (Ctrl->T.now < t_plateau) {	/* We are within the plateau phase, keep everything constant */
				out[s_col] = Ctrl->M.value[PSEVENTS_SIZE][PSEVENTS_VAL1] * size;
				out[i_col] = Ctrl->M.value[PSEVENTS_INT][PSEVENTS_VAL1];
				out[t_col] = 0.0;	/* No transparency during plateau phase */
			}
			else if (Ctrl->T.now < t_decay) {	/* We are withn the decay phase */
				x = pow ((t_decay - Ctrl->T.now)/Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_DECAY], 2.0);	/* Quadratic function that goes from 1 to 0 */
				out[s_col] = size * (Ctrl->M.value[PSEVENTS_SIZE][PSEVENTS_VAL1] * x + (1.0 - x));	/* Reduction of size down to the nominal size */
				out[i_col] = Ctrl->M.value[PSEVENTS_INT][PSEVENTS_VAL1] * x;	/* Reduction of intensity down to 0 */
				out[t_col] = 0.0;
			}
			else if (finite_duration && Ctrl->T.now < t_end) {	/* We are within the normal display phase with nominal symbol size */
				out[s_col] = size;
				out[i_col] = out[t_col] = 0.0;	/* No intensity or transparency during normal phase */
			}
			else if (finite_duration && Ctrl->T.now <= t_fade) {	/* We are within the fade phase */
				x = pow ((t_fade - Ctrl->T.now)/Ctrl->E.dt[PSEVENTS_SYMBOL][PSEVENTS_FADE], 2.0);	/* Quadratic function that goes from 1 to 0 */
				out[s_col] = size * (x + (1.0 - x) * Ctrl->M.value[PSEVENTS_SIZE][PSEVENTS_VAL2]);	/* Reduction of size down to coda size */
				out[i_col] = Ctrl->M.value[PSEVENTS_INT][PSEVENTS_VAL2] * (1.0 - x);		/* Reduction of intensity down to coda intensity */
				out[t_col] = Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL2] * (1.0 - x);		/* Increase of transparency up to code transparency */
			}
			else if (do_coda) {	/* If there is a coda then the symbol remains visible given those terminal attributes */
				out[s_col] = size * Ctrl->M.value[PSEVENTS_SIZE][PSEVENTS_VAL2];
				out[i_col] = Ctrl->M.value[PSEVENTS_INT][PSEVENTS_VAL2];
				out[t_col] = Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL2];
			}
			if (Ctrl->C.active)	/* Need to pass on the z-value for cpt lookup in psxy */
				fprintf (fp_symbols, "%.16g\t%.16g\t%.16g\t%g\t%g\t%g\n", out[GMT_X], out[GMT_Y], out[GMT_Z], out[s_col], out[i_col], out[t_col]);
			else
				fprintf (fp_symbols, "%.16g\t%.16g\t%g\t%g\t%g\n", out[GMT_X], out[GMT_Y], out[s_col], out[i_col], out[t_col]);
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
				else	/* Temporariy file to be deleted after use */
					sprintf (tmp_file_labels, "%s/GMT_psevents_labels_%d.txt", API->tmp_dir, (int)getpid());
				if ((fp_labels = fopen (tmp_file_labels, "w")) == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "Unable to create file %s\n", tmp_file_labels);
					Return (GMT_RUNTIME_ERROR);
				}
			}
			out[GMT_X] = in[GMT_X];	out[GMT_Y] = in[GMT_Y];	/* Pass out the input coordinates unchanged */

			/* Labels have variable transparency during optional rise and fade, and fully opaque during normal section, and skipped oterhwise unless coda */

			if (Ctrl->T.now < t_event) {	/* We are within the rise phase */
				x = pow ((Ctrl->T.now - t_rise)/Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_RISE], 2.0);	/* Quadratic function that goes from 1 to 0 */
				out[GMT_Z] = Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL1] * (1.0-x);		/* Magnification of opacity */
			}
			else if (finite_duration && Ctrl->T.now < t_end)	/* We are within the plateau, decay or normal phases, keep everything constant */
				out[GMT_Z] = 0.0;	/* No transparency during these  phases */
			else if (finite_duration && Ctrl->T.now <= t_fade) {	/* We are within the fade phase */
				x = pow ((t_fade - Ctrl->T.now)/Ctrl->E.dt[PSEVENTS_TEXT][PSEVENTS_FADE], 2.0);	/* Quadratic function that goes from 1 to 0 */
				out[GMT_Z] = Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL2] * (1.0 - x);		/* Increase of transparency up to code transparency */
			}
			else if (do_coda)	/* If there is a coda then the label is visible given its coda attributes */
				out[GMT_Z] = Ctrl->M.value[PSEVENTS_TRANSP][PSEVENTS_VAL2];
			fprintf (fp_labels, "%.16g\t%.16g\t%g\t%s\n", out[GMT_X], out[GMT_Y], out[GMT_Z], In->text);
			n_labels_plotted++;	/* Count output labels */
		}
	} while (true);

	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		if (fp_symbols) fclose (fp_symbols);
		if (fp_labels) fclose (fp_labels);
		Return (API->error);
	}
	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), "")) {	/* Set up map projection */
		if (fp_symbols) fclose (fp_symbols);
		if (fp_labels) fclose (fp_labels);
		Return (GMT_PROJECTION_ERROR);
	}

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	gmt_map_basemap (GMT);	/* Plot basemap if requested */

	if (fp_symbols) { /* Here we have event symbols to plot as an overlay via a call to psxy */
		fclose (fp_symbols);	/* First close the file so symbol output is flushed */
		/* Build psxy command with fixed options and those that depend on -C -G -W.
		 * We must set symbol unit as inch since we are passing sizes in inches directly (dimensions are in inches internally in GMT).  */
		sprintf (cmd, "%s -R -J -O -K -I -t -S%s --GMT_HISTORY=false --PROJ_LENGTH_UNIT=%s", tmp_file_symbols, Ctrl->S.symbol, GMT->session.unit_name[GMT->current.setting.proj_length_unit]);
		if (Ctrl->C.active) {strcat (cmd, " -C"); strcat (cmd, Ctrl->C.file);}
		if (Ctrl->G.active) {strcat (cmd, " -G"); strcat (cmd, Ctrl->G.color);}
		if (Ctrl->W.pen) {strcat (cmd, " -W"); strcat (cmd, Ctrl->W.pen);}
		GMT_Report (API, GMT_MSG_DEBUG, "cmd: gmt psxy %s\n", cmd);

		if (GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {	/* Plot the symbols */
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
		fclose (fp_labels);	/* First close the file so label output is flushed */
		/* Build pstext command with fixed options and those that depend on -D -F */
		sprintf (cmd, "%s -R -J -O -K -t --GMT_HISTORY=false", tmp_file_labels);
		if (Ctrl->D.active) {strcat (cmd, " -D"); strcat (cmd, Ctrl->D.string);}
		if (Ctrl->F.active) {strcat (cmd, " -F"); strcat (cmd, Ctrl->F.string);}
		GMT_Report (API, GMT_MSG_DEBUG, "cmd: gmt pstext %s\n", cmd);

		if (GMT_Call_Module (API, "pstext", GMT_MODULE_CMD, cmd) != GMT_NOERROR) {	/* Plot the labels */
			Return (API->error);
		}
		if (!Ctrl->Q.active && gmt_remove_file (GMT, tmp_file_labels)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to remove file %s\n", tmp_file_labels);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	/* Finalize plot and we are done */

	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);

	GMT_Report (API, GMT_MSG_INFORMATION, "Events read: %" PRIu64 " Event symbols plotted: %" PRIu64 " Event labels plotted: %" PRIu64 "\n", n_total_read, n_symbols_plotted, n_labels_plotted);

	Return (GMT_NOERROR);
}
