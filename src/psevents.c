/*
 *
 *	Copyright (c) 1991-2019 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * API functions to support the psevents application.
 *
 * Author:	Paul Wessel
 * Date:	5-MAY-2019
 * Version:	6 API
 *
 * Brief synopsis: psevents handles the plotting of events for a movie.
 */
#include "gmt_dev.h"

#define THIS_MODULE_NAME	"psevents"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Construct event symbols for making movies"
#define THIS_MODULE_KEYS	"<D{,>X}"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"-:>BJKOPRUVXYabdefhip"

enum Psevent {
	PSEVENTS_RISE = 0,
	PSEVENTS_PLATEAU = 1,
	PSEVENTS_DECAY = 2,
	PSEVENTS_FADE = 3,
	PSEVENTS_MAG1 = 0,
	PSEVENTS_MAG2 = 1,
	PSEVENTS_INFINITE = 0,
	PSEVENTS_FIXED_DURATION = 1,
	PSEVENTS_VAR_DURATION = 2,
	PSEVENTS_VAR_ENDTIME = 3
};

/* Control structure for psevents */

struct PSEVENTS_CTRL {
	struct A {	/* 	-A<magnify>[+c<magnify2] */
		bool active;
		double magnify[2];
	} A;
	struct C {	/* 	-C<cpt> */
		bool active;
		char *cpt;
	} C;
	struct E {	/* 	-E[+r<dt>][+p<dt>][+d<dt>][+f<dt>] */
		bool active;
		double dt[4];
	} E;
	struct D {	/* 	-D[t|<duration>] */
		bool active;
		unsigned int mode;
		double duration;
	} D;
	struct F {	/* 	-F[+c<transparency2] */
		bool active;
		double transparency[2];
	} F;
	struct I {	/* 	-I<instensity>[+c<intensity2] */
		bool active;
		double magnify[2];
	} I;
	struct Q {	/* 	-Q<file> */
		bool active;
		char *file;
	} Q;
	struct S {	/* 	-S<symbol>[/<size] */
		bool active;
		char symbol;
		char *string;
		double size;
	} S;
	struct T {	/* 	-T<time> */
		bool active;
		double now;
	} T;
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PSEVENTS_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PSEVENTS_CTRL);
	
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PSEVENTS_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->C.cpt);	
	gmt_M_str_free (C->S.string);	
	gmt_M_free (GMT, C);	
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s [<table>] -T<now> [-A<magnify>[+c<magnify2]] [-C<cpt>] [-E[+r<dt>][+p<dt>][+d<dt>][+f<dt>]]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-D[i|<duration>][+t]] [-F[+c<transparency2]] [-I<instensity>[+c<intensity2]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-Q<file>] [-S<symbol>[/<size>]] [%s] [%s]\n", GMT_V_OPT, GMT_b_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s] [%s] [%s]\n\n",
		GMT_d_OPT, GMT_e_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_o_OPT, GMT_colon_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t-T Specify the current time.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Set size magnification for event announcement phase.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c to set a separate terminal magnification for the coda [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Give <cpt> and obtain color with z-value in 4th column\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set rise, plateau, decay, and fade intervals, if needed:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +r<dt> sets the rise-time before the event zero time [no rise time].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +p<dt> sets the length of the plateau after event happens [no plateau].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +d<dt> sets the decay-time after the plateau [no decay].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +f<dt> sets the fade-time after the event ends [no fade time].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set duration of event, otherwise we assume it is infinite.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If no arg we read duration from file; give t for reading end time intead.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Set transparency to fade out symbol during fade phase.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c to change terminal transparency for the coda [100].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Set intensity magnitude for event announcement phase [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c to set a separate terminal intensity for the coda [0].\n");
	GMT_Option (API, "O,P");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q To save the intermediate events file, supply a file name [temporary file deleted].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Append symbol code and /size[u].  If no size we read it from the data file.\n");
	GMT_Option (API, "V,bi2,di,e,f,h,i,p,:,.");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PSEVENTS_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psevents and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, pos;
	char *c = NULL, txt[GMT_LEN64] = {""};
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Skip input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				break;

			/* Processes program-specific parameters */
			
			case 'A':	/* Set magnifications of symbol */
				Ctrl->A.active = true;
				if ((c = strstr (opt->arg, "+c"))) {
					Ctrl->A.magnify[PSEVENTS_MAG2] = atof (&c[2]);
					c[0] = '\0';	/* Truncate modifier */
				}
				Ctrl->A.magnify[PSEVENTS_MAG1] = atof (opt->arg);
				if (c) c[0] = '+';	/* Restore modifier */
				break;

			case 'C':	/* Set eventtimes */
				Ctrl->C.active = true;
				Ctrl->C.cpt = strdup (opt->arg);
				break;

			case 'E':	/* Set eventtimes */
				Ctrl->E.active = true;
				if (gmt_validate_modifiers (GMT, opt->arg, 'C', "dfpr")) n_errors++;
				if ((p = gmt_first_modifier (GMT, opt->arg, "dfpr")) == NULL) {	/* This cannot happen given the strstr checks, but Coverity prefers it */
					GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -C: No modifiers?\n");
					n_errors++;
					break;
				}
				pos = 0;	txt[0] = 0;
				while (gmt_getmodopt (GMT, 'C', p, "dfpr", &pos, txt, &n_errors) && n_errors == 0) {
					switch (txt[0]) {
						case 'd': Ctrl->E.dt[PSEVENTS_DECAY]   = atof (&txt[1]);	break;	/* Decay duration */
						case 'f': Ctrl->E.dt[PSEVENTS_FADE]    = atof (&txt[1]);	break;	/* Fade duration */
						case 'p': Ctrl->E.dt[PSEVENTS_PLATEAU] = atof (&txt[1]);	break;	/* Plateau duration */
						case 'r': Ctrl->E.dt[PSEVENTS_RISE]    = atof (&txt[1]);	break;	/* Rise duration */
						default: break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
					}
				}
				break;

			case 'D':	/* Set duration */
				Ctrl->D.active = true;
				if (opt->arg[0] == 't')	/* Get individual event end times from file */
					Ctrl->D.mode = PSEVENTS_VAR_ENDTIME;
				else if (opt->arg[0])	/* Get fixed event duration here */
					Ctrl->I.duration = atof (opt->arg);
					Ctrl->D.mode = PSEVENTS_FIXED_DURATION;
				}
				else	/* Get individual event durations from file */
					Ctrl->D.mode = PSEVENTS_VAR_DURATION;
				break;

			case 'F':	/* Set transparency of symbol */
				Ctrl->F.active = true;
				if ((c = strstr (opt->arg, "+c"))) {
					Ctrl->F.transparency[PSEVENTS_MAG2] = atof (&c[2]);
					c[0] = '\0';	/* Truncate modifier */
				}
				if (opt->arg[0]) Ctrl->F.transparency[PSEVENTS_MAG1] = atof (opt->arg);
				if (c) c[0] = '+';	/* Restore modifier */
				break;

			case 'I':	/* Set intensity of symbol */
				Ctrl->I.active = true;
				if ((c = strstr (opt->arg, "+c"))) {
					Ctrl->I.intensity[PSEVENTS_MAG2] = atof (&c[2]);
					c[0] = '\0';	/* Truncate modifier */
				}
				Ctrl->I.intensity[PSEVENTS_MAG1] = atof (opt->arg);
				if (c) c[0] = '+';	/* Restore modifier */
				break;

			case 'Q':	/* Save events file for posterity */
				Ctrl->Q.active = true;
				Ctrl->Q.file = strdup (opt->arg);
				break;

			case 'S':	/* Set symbol type and size*/
				Ctrl->S.active = true;
				Ctrl->S.string = strdup (opt->arg);
				Ctrl->S.symbol = opt->arg[0];
				if ((c = strchr (opt->arg, '/')))
					Ctrl->S.size = gmt_M_to_inch (GMT, &c[2]);
				else
					Ctrl->S.mode = 1;	/* Must read symbol size */
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 4;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] < 4, "Syntax error: Binary input data (-bi) must have at least 4 columns.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}


/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_psevents (void *V_API, int mode, void *args) {
	char tmp_file[PATH_MAX] = {""}, cmd[BUFSIZ] = {""};
	int error;
	unsigned int smode = GMT_NO_STRINGS, s_in = 3, d_in = 3, s_out = 2, i_out = 3, t_out = 4;
	
	uint64_t n_total_read = 0, n_total_used = 0;
	
	double out[5] = {0, 0, 0, 0, 0}, t_end = DBL_MAX;
	double t_event, d_rise, t_plateau, t_decay, t_fade, x, size;
	
	FILE *fp = NULL;
	
	struct PSEVENTS_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	
	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);
	
	GMT_Report (API, GMT_MSG_NORMAL, "psevents not ready for use yet\n");
	/*---------------------------- This is the psevents main code ----------------------------*/

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Processing input table data\n");
	
	/* Now we are ready to take on some input values */
	
	/* We read as points. */
	GMT_Set_Columns (API, GMT_IN, n_needed, GMT_COL_FIX);

	if (GMT_Init_IO (API, GMT_IS_DATASET, GMT_IS_POINT, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_DATASET, GMT_IN, GMT_HEADER_ON) != GMT_NOERROR) {		/* Enables data input and sets access mode */
		Return (API->error);
	}

	if (Ctrl->C.active) s_in++, d_in++, s_out++, i_out++, t_out++;	/* Must allow for z-value in input*/
	if (Ctrl->S.mode) d_in++;	/* Must allow for size in input */

	do {	/* Keep returning records until we reach EOF */
		if ((In = GMT_Get_Record (API, GMT_READ_DATA, NULL)) == NULL) {	/* Read next record, get NULL if special case */
			if (gmt_M_rec_is_error (GMT)) {		/* Bail if there are any read errors */
				Return (GMT_RUNTIME_ERROR);
			}
			else if (gmt_M_rec_is_eof (GMT)) 		/* Reached end of file */
				break;
			else if (gmt_M_rec_is_segment_header (GMT)) {			/* Parse segment headers */
				/* Do nothing */
			}
			continue;							/* Go back and read the next record */
		}

		/* Data record to process */
		
		in = In->data;
		n_total_read++;
		
		t_event = in[0];
		t_rise = t_event - Ctrl->E.dt[PSEVENTS_RISE];
		if (t_rise < Ctrl->T.now) continue;	/* Event is in the future */
		if (Ctrl->D.mode == PSEVENTS_FIXED_DURATION)
			t_end = in[0] + Ctrl->D.duration;
		else if (Ctrl->D.mode == PSEVENTS_VAR_DURATION)
			t_end = in[0] + in[d_in];
		else if (Ctrl->D.mode == PSEVENTS_VAR_ENDTIME)
			t_end = in[d_in];
		if ((t_end + Ctrl->E.dt[PSEVENTS_FADE]) > Ctrl->T.now) continue;	/* Event is in the past */
		
		/* Here we must plot a phase of this event */
		
		if (n_total_used == 0) {	/* Open file the first time */
			if (Ctrl->Q.active)
				strcpy (tmp_file, Ctrl->Q.file);
			else {
				sprintf (tmp_file, "%s/GMT_events_%d.txt", API->tmp_dir, (int)getpid());
			if ((fp = fopen (tmp_file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to create file %s\n", tmp_file);
				Return (GMT_RUNTIME_ERROR);
			}
		}
		out[GMT_X] = in[1];	out[GMT_Y] = in[2];
		if (Ctrl->C.active) out[GMT_Z] = in[3];
		
		t_plateau = t_event + Ctrl->E.dt[PSEVENTS_PLATEAU];
		t_decay = t_plateau + Ctrl->E.dt[PSEVENTS_DECAY];
		t_fade = t_end + Ctrl->E.dt[PSEVENTS_FADE];
		size = (Ctrl->S.mode) ? in[s_in] : Ctrl->S.size;
		if (Ctrl->T.now < t_event) {	/* We are in the rise phase */
			x = pow ((Ctrl->T.now - t_rise)/Ctrl->E.dt[PSEVENTS_RISE], 2.0);	/* 0 to 1 */
			out[s_out] = Ctrl->A.magnify[PSEVENTS_MAG1] * x * size;
			out[i_out] = Ctrl->I.intensity[PSEVENTS_MAG1] * x;
			out[t_out] = Ctrl->I.transparency[PSEVENTS_MAG1] * (1.0-x);
		}
		else if (Ctrl->T.now < t_plateau) {	/* We are in the plateau phase */
			out[s_out] = Ctrl->A.magnify[PSEVENTS_MAG1] * size;
			out[i_out] = Ctrl->I.intensity[PSEVENTS_MAG1];
			out[t_out] = 0.0;
		}
		else if (Ctrl->T.now < t_decay) {	/* We are in the decay phase */
			x = pow ((t_decay - Ctrl->T.now)/Ctrl->E.dt[PSEVENTS_DECAY], 2.0);	/* 1 to 0 */
			out[s_out] = size * (Ctrl->A.magnify[PSEVENTS_MAG1] * x + (1.0 - x));
			out[i_out] = Ctrl->I.intensity[PSEVENTS_MAG1] * x;
			out[t_out] = 0.0;
		}
		else if (Ctrl->T.now < t_end) {	/* We are in the normal display phase */
			out[s_out] = size;
			out[i_out] = out[t_out] = 0.0;
		}
		else if (Ctrl->T.now < t_fade) {	/* We are in the fade phase */
			x = pow ((t_fade - Ctrl->T.now)/trl->E.dt[PSEVENTS_FADE], 2.0);	/* From 1 to 0 */
			out[s_out] = size * x + (1.0 - x) * Ctrl->A.magnify[PSEVENTS_MAG2]);
			out[i_out] = Ctrl->I.intensity[PSEVENTS_MAG2] * (1.0 - x);
			out[t_out] = Ctrl->F.transparency[PSEVENTS_MAG2] * (1.0 - x);
		}
		else if (do_coda) {
			out[s_out] = Ctrl->A.magnify[PSEVENTS_MAG2];
			out[i_out] = Ctrl->I.intensity[PSEVENTS_MAG2];
			out[t_out] = Ctrl->F.transparency[PSEVENTS_MAG2];
		}
		if (Ctrl->C.active)
			fprintf (fp, "%g\t%g\t%g\t%g\t%g\t%g\n", out[GMT_X], out[GMT_Y], out[GMT_Z], out[s_out], out[i_out], out[t_out]);
		else
			fprintf (fp, "%g\t%g\t%g\t%g\t%g\n", out[GMT_X], out[GMT_Y], out[s_out], out[i_out], out[t_out]);
		n_total_used++;
	}
	if (GMT_End_IO (API, GMT_IN, 0) != GMT_NOERROR) {	/* Disables further data input */
		Return (API->error);
	}
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Points in: %" PRIu64 " Points used: %" PRIu64 "\n", n_total_read, n_total_used);
	
	if (gmt_M_err_pass (GMT, gmt_map_setup (GMT, GMT->common.R.wesn), ""))
		Return (GMT_PROJECTION_ERROR);

	if ((PSL = gmt_plotinit (GMT, options)) == NULL) Return (GMT_RUNTIME_ERROR);
	gmt_plane_perspective (GMT, GMT->current.proj.z_project.view_plane, GMT->current.proj.z_level);

	gmt_plotcanvas (GMT);	/* Fill canvas if requested */
	gmt_map_basemap (GMT);	/* Plot basemap if requested */

	if (fp == NULL) /* No events survived, just return */
		Return (GMT_NOERROR);
	
	/* Here we have something to plot as overlay via psxy */
	
	sprintf (cmd, "%s -R -J -O -K -S%s -I -t --GMT_HISTORY=false", tmp_file, Ctrl->S.string);
	if (Ctrl->C.active) {strcat (cmd, " "); strcat (cmd, Ctrl->C.cpt);}
	
	if (GMT_Call_Module (API, "psxy", GMT_MODULE_CMD, cnd) != GMT_NOERROR) {	/* Plot the symbols */
		Return (API->error);
	}
	
	gmt_plane_perspective (GMT, -1, 0.0);
	gmt_plotend (GMT);
	
	Return (GMT_NOERROR);
}
