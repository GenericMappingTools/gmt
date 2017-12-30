/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2018 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
 * Brief synopsis: gmt movie automates the generation of frames in an animation
 */

#include "gmt_dev.h"

#define THIS_MODULE_NAME	"movie"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Create animation sequences"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"V"

/* Control structure for movie */

#define MOVIE_GIF		1
#define MOVIE_MP4		0
#define MOVIE_PREFLIGHT		0
#define MOVIE_POSTFLIGHT	1

enum enum_script {BASH_MODE = 0,	/* Write Bash script */
	CSH_MODE,			/* Write C-shell script */
	DOS_MODE};			/* Write DOS script */

struct MOVIE_CTRL {
	struct In {	/* mainscript (Bourne, Bourne Again, Csh, or BAT script) */
		bool active;
		enum enum_script mode;
		char *file;
		FILE *fp;
	} In;
	struct D {	/* -D<knownformat>|<dimensions> */
		bool active;
		double dim[3];
		char unit;
		char *string;
	} D;
	struct E {	/* -E */
		bool active;
	} E;
	struct F {	/* -F<videoformat> */
		bool active;
		unsigned int mode;
		char *format;
	} F;
	struct G {	/* -G<canvasfill> */
		bool active;
		char *fill;
	} G;
	struct N {	/* -N<movieprefix> */
		bool active;
		char *prefix;
	} N;
	struct Q {	/* -Q[<frame>] */
		bool active;
		unsigned int frame;
	} Q;
	struct S {	/* -Sb|f<script> */
		bool active;
		char *file;
		FILE *fp;
	} S[2];
	struct T {	/* -T<nframes>|<timefile> */
		bool active;
		unsigned int n_frames;
		char *file;
	} T;
	struct W {	/* -W<rate>[+l[<n>]]  */
		bool active;
		bool loop;
		double rate;
		unsigned int n_loops;
	} W;
};

EXTERN_MSC void gmtlib_str_tolower (char *string);

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MOVIE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct MOVIE_CTRL);
	C->D.unit = 'i';	/* Inches unless set */
	C->W.rate = 24.0;	/* 24 frames/sec */
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct MOVIE_CTRL *C) {	/* Deallocate control structure */
	gmt_M_unused (GMT);
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->D.string);
	gmt_M_str_free (C->F.format);
	gmt_M_str_free (C->G.fill);
	gmt_M_str_free (C->N.prefix);
	gmt_M_str_free (C->S[MOVIE_PREFLIGHT].file);
	gmt_M_str_free (C->S[MOVIE_POSTFLIGHT].file);
	gmt_M_str_free (C->T.file);
	gmt_M_free (GMT, C);
}

static void set_dvalue (FILE *fp, int mode, char *name, double value, char unit)
{	/* Assigns the double variable given the script mode */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "%s=%g", name, value); break;
		case CSH_MODE:  fprintf (fp, "set %s = %g", name, value); break;
		case DOS_MODE:  fprintf (fp, "set %s=%g", name, value); break;
	}
	if (unit) fprintf (fp, "%c", unit);
	fprintf (fp, "\n");
}

static void set_ivalue (FILE *fp, int mode, char *name, int value)
{	/* Assigns the double variable given the script mode */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "%s=%d\n", name, value); break;
		case CSH_MODE:  fprintf (fp, "set %s = %d\n", name, value); break;
		case DOS_MODE:  fprintf (fp, "set %s=%d\n", name, value); break;
	}
}

static void set_tvalue (FILE *fp, int mode, char *name, char *value)
{	/* Assigns the text variable given the script mode */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "%s=%s\n", name, value); break;
		case CSH_MODE:  fprintf (fp, "set %s = %s\n", name, value); break;
		case DOS_MODE:  fprintf (fp, "set %s=%s\n", name, value); break;
	}
}

static void set_script (FILE *fp, int mode)
{	/* Assigns the text variable given the script mode */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "#!/bin/bash -xv\n"); break;
		case CSH_MODE:  fprintf (fp, "#!/bin/csh\n"); break;
		case DOS_MODE:  fprintf (fp, "REM Start of script\n"); break;
	}
}

static void set_comment (FILE *fp, int mode, char *comment)
{	/* Write a comment given the script mode */
	switch (mode) {
		case BASH_MODE: case CSH_MODE:  fprintf (fp, "# %s\n", comment); break;
		case DOS_MODE:  fprintf (fp, "REM %s\n", comment); break;
	}
}

static char *place_var (int mode, char *name)
{	/* Prints this variable to stdout where needed in the script via the assignment static variable.
	 * PS!  Only one call per printf statement since string cannot hold more than one item at the time */
	static char string[128] = {""};
	if (mode == DOS_MODE)
		sprintf (string, "%%%s%%", name);
	else
		sprintf (string, "${%s}", name);
	return (string);
}

static void place_script (struct GMT_CTRL *GMT, FILE *fp, FILE *fpi) {	/* Place script commands */
	char line[GMT_LEN256] = {""};
	while (gmt_fgets (GMT, line, GMT_BUFSIZ, fpi)) {
		if (!strstr (line, "#!/bin"))	/* Skip any leading shell incantation since we control that line */
			fprintf (fp, "%s", line);
	}
	fclose (fpi);	/* Close when done with this file */
}

static int local_system (char *cmd) {
	/* Temporary dummy to not actually run the loop script */
	fprintf (stderr, "Run via system: %s\n", cmd);
	return 0;
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: movie <mainscript> -D<layout> -F<format> -N<prefix> -T<times>\n");
	GMT_Message (API, GMT_TIME_NONE, "\t [-G<fill>] [-Q[<frame>]] [-Sb<script>] [-Sf<script>] [%s] [-W<rate>[+l[<n>]]]\n\n", GMT_V_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<mainscript> is the main GMT modern script that builds the time-varying image.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Specify layout of the custom paper size. Choose from known dimensions or set them manually:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   16:9 ratio formats:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     name:	pixel size	page size (SI)	page size (US)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     4k:	3840x2160	24 x 13.5 cm	 9.6 x 5.4 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     1080p:	1920x1080	24 x 13.5 cm	 9.6 x 5.4 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     720p:	1280x720	24 x 13.5 cm	 9.6 x 5.4 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     540p:	 960x540	24 x 13.5 cm	 9.6 x 5.4 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   4:3 ratio formats:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     name:	pixel size	page size (SI)	page size (US)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     480p:	 640x480	24 x 18 cm	 9.6 x 7.2 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     360p:	 480x360	24 x 18 cm	 9.6 x 7.2 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t  Note: PROJ_LENGTH_UNIT determines if you set SI or US dimensions.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, set a custom format:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Give <width>x<height>x<dpi> for a custom frame dimension.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Set the final movie format. Choose from these selections:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     gif:  Make an animated GIF.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     mp4:  Make an MP4 movie.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     none: No animated product, just the PNG frames.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set project prefix which is used for movie-, frame-names and directories.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set number of frames or give filename with frame-specific information.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set the canvas background color [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Just plot the first PNG (frame 0) or append desired frame [make the movie].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Sets the optional background and foreground GMT modern scripts:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sb Append the name of a background script [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      Pre-compute items needed by <mainscript> and a static background plot layer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sf Append name of foreground script [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      This script must make an static plot overlay that is added to all frames.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Set frame rate in frames/second [24]. Append +l to enable looping [GIF only].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append a specific number of loops we set an infinite loop.\n");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct MOVIE_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, n_files = 0, k;
	int n;
	char txt_a[GMT_LEN32] = {""}, txt_b[GMT_LEN32] = {""}, *c = NULL;
	double width, height16x9, height4x3, dpu;
	struct GMT_FILL fill;	/* Only used to make sure any fill is given with correct syntax */
	struct GMT_OPTION *opt = NULL;
	
	if (GMT->current.setting.proj_length_unit == GMT_INCH) {	/* US settings in inch given format names */
		width = 9.6;	height16x9 = 5.4;	height4x3 = 7.2;	dpu = 400.0;
		Ctrl->D.unit = 'i';
	}
	else {	/* SI settings in cm given format names */
		width = 24.0;	height16x9 = 13.5;	height4x3 = 18.0;	dpu = 160.0;
		Ctrl->D.unit = 'c';
	}

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input file */
				if (n_files++ > 0) break;
				if ((Ctrl->In.active = gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_TEXT)))
					Ctrl->In.file = strdup (opt->arg);
				else
					n_errors++;
				break;
				
			case 'D':	/* Frame dimension */
				Ctrl->D.active = true;
				if (!strcmp (opt->arg, "4k")) {
					Ctrl->D.dim[GMT_X] = width;	Ctrl->D.dim[GMT_Y] = height16x9;	Ctrl->D.dim[GMT_Z] = dpu;
				}
				else if (!strcmp (opt->arg, "1080p")) {
					Ctrl->D.dim[GMT_X] = width;	Ctrl->D.dim[GMT_Y] = height16x9;	Ctrl->D.dim[GMT_Z] = dpu / 2.0;
				}
				else if (!strcmp (opt->arg, "720p")) {
					Ctrl->D.dim[GMT_X] = width;	Ctrl->D.dim[GMT_Y] = height16x9;	Ctrl->D.dim[GMT_Z] = dpu / 3.0;
				}
				else if (!strcmp (opt->arg, "540p")) {
					Ctrl->D.dim[GMT_X] = width;	Ctrl->D.dim[GMT_Y] = height16x9;	Ctrl->D.dim[GMT_Z] = dpu / 4.0;
				}
				else if (!strcmp (opt->arg, "480p")) {
					Ctrl->D.dim[GMT_X] = width;	Ctrl->D.dim[GMT_Y] = height4x3;	Ctrl->D.dim[GMT_Z] = dpu / 6.0;
				}
				else if (!strcmp (opt->arg, "360p")) {
					Ctrl->D.dim[GMT_X] = width;	Ctrl->D.dim[GMT_Y] = height4x3;	Ctrl->D.dim[GMT_Z] = dpu / 8.0;
				}
				else {	/* Custom */
					if ((n = sscanf (opt->arg, "%[^x]x%[^x]x%lg", txt_a, txt_b, &Ctrl->D.dim[GMT_Z])) != 3) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -D: Requires know format or 3 width x height x dpi\n");
						n_errors++;
					}
					else {
						Ctrl->D.dim[GMT_X] = gmt_M_to_inch (GMT, txt_a);	
						Ctrl->D.dim[GMT_Y] = gmt_M_to_inch (GMT, txt_b);
						if (strchr ("cip", txt_a[strlen(txt_a)-1]))
							Ctrl->D.unit = txt_a[strlen(txt_a)-1];
						else if (strchr ("cip", txt_b[strlen(txt_b)-1]))
							Ctrl->D.unit = txt_b[strlen(txt_b)-1];
						else	/* Use GMT default setting */
							Ctrl->D.unit = GMT->session.unit_name[GMT->current.setting.proj_length_unit][0];
					}
				}
				break;

			case 'E':	/* Erase frames */
				Ctrl->E.active = true;
				break;
			case 'F':	/* Get movie format */
				Ctrl->F.active = true;
				if (!strcmp (opt->arg, "gif") || !strcmp (opt->arg, "GIF"))
					Ctrl->F.mode = MOVIE_GIF;
				else if (!strcmp (opt->arg, "mp4") || !strcmp (opt->arg, "MP4"))
					Ctrl->F.mode = MOVIE_MP4;
				break;
			case 'G':	/* Canvas fill */
				Ctrl->G.active = true;
				if (gmt_getfill (GMT, opt->arg, &fill))
					n_errors++;
				else
					Ctrl->G.fill = strdup (opt->arg);
				break;
			case 'N':	/* Movie prefix */
				Ctrl->N.active = true;
				Ctrl->N.prefix = strdup (opt->arg);
				break;
			
			case 'Q':	/* Debug frame */
				Ctrl->Q.active = true;
				if (opt->arg[0]) Ctrl->Q.frame = atoi (opt->arg);
				break;
			case 'S':	/* background and foreground scripts */
				switch (opt->arg[0]) {
					case 'b':	k = MOVIE_PREFLIGHT;	break;	/* background */
					case 'f':	k = MOVIE_POSTFLIGHT;	break;	/* foreground */
					default:	n_errors++; 	break;	/* Bad option */
				}
				Ctrl->S[k].active = true;
				Ctrl->S[k].file = strdup (&opt->arg[1]);
				if ((Ctrl->S[k].fp = fopen (Ctrl->S[k].file, "r")) == NULL) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -S%c: Unable to open file %s\n", opt->arg[0], Ctrl->S[k].file);
					n_errors++;
				}
				break;
	
			case 'T':	/* Number of frames or file */
				Ctrl->T.active = true;
				Ctrl->T.file = strdup (opt->arg);
				break;
			case 'W':	/* Frame rate */
				Ctrl->W.active = true;
				Ctrl->W.rate = atof (opt->arg);
				if ((c = strstr (opt->arg, "+l"))) {
					Ctrl->W.loop = true;
					if (c[2]) Ctrl->W.n_loops = atoi (&c[2]);
				}
				break;
				
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
		
	n_errors += gmt_M_check_condition (GMT, !Ctrl->D.active, "Syntax error -D: Must specify frame dimension\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->F.active, "Syntax error -F: Must specify animation format\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->N.active, "Syntax error -N: Must specify a movie prefix\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active, "Syntax error -T: Must specify number of frames or time file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.loop && Ctrl->F.mode != MOVIE_GIF, "Syntax error -W: The loop modifier +l only applies to GIF animations\n");
	
	if (n_files == 1) {
		if (strstr (Ctrl->In.file, ".bash") || strstr (Ctrl->In.file, ".sh"))
			Ctrl->In.mode = BASH_MODE;
		else if (strstr (Ctrl->In.file, ".csh"))
			Ctrl->In.mode = CSH_MODE;
		else if (strstr (Ctrl->In.file, ".bat"))
			Ctrl->In.mode = DOS_MODE;
		else {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to determine script language from the extension of your script %s\n", Ctrl->In.file);
			n_errors++;
		}
		if (n_errors == 0 && ((Ctrl->In.fp = fopen (Ctrl->In.file, "r")) == NULL)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to open main script file %s\n", Ctrl->In.file);
			n_errors++;
		}
	}
	else {
		GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Must specify a main script file!");
		n_errors++;
	}
	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_movie (void *V_API, int mode, void *args) {
	int error = 0;
	unsigned int n_values = 0, n_frames = 0, n_begin = 0, frame, col;
	char *extension[3] = {"sh", "csh", "bat"}, *load[3] = {"source", "source", "call"}, *rmfile[3] = {"rm -f", "rm -f", "del"};
	char var[3] = "$$%", *rmdir[3] = {"rm -rf", "rm -rf", "deltree"}, *mvfile[3] = {"mv -f", "mv -rf", "move"}, *export[3] = {"export ", "export ", ""};
	char init_file[GMT_LEN128] = {""}, state_prefix[GMT_LEN128] = {""}, state_file[GMT_LEN128] = {""}, cwd[PATH_MAX] = {""};
	char pre_file[GMT_LEN128] = {""}, post_file[GMT_LEN128] = {""}, main_file[GMT_LEN128] = {""}, line[GMT_BUFSIZ] = {""};
	char string[GMT_LEN64] = {""}, extra[GMT_LEN64] = {""}, cmd[GMT_LEN256] = {""};
	FILE *fp = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_OPTION *options = NULL;
	struct MOVIE_CTRL *Ctrl = NULL;
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

	if (Ctrl->F.mode == MOVIE_GIF) {	/* Ensure we have GraphicsMagick installed */
		sprintf (cmd, "gm version");
		if ((fp = popen (cmd, "r")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "GraphicsMagick is not installed - cannot build animated GIF.\n");
			Return (GMT_RUNTIME_ERROR);
		}
		pclose (fp);
	}
	else if (Ctrl->F.mode == MOVIE_MP4) {	/* Ensure we have ffmpeg installed */
		sprintf (cmd, "ffmpeg -version");
		if ((fp = popen (cmd, "r")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "ffmpeg is not installed - cannot build MP4 movie.\n");
			Return (GMT_RUNTIME_ERROR);
		}
		pclose (fp);
	}
	
	GMT_Report (API, GMT_MSG_NORMAL, "Paper dimensions will be %g%c x %g%c\n", Ctrl->D.dim[GMT_X], Ctrl->D.unit, Ctrl->D.dim[GMT_Y], Ctrl->D.unit);
	GMT_Report (API, GMT_MSG_NORMAL, "Pixel dimensions will be %u x %u\n", urint (Ctrl->D.dim[GMT_X] * Ctrl->D.dim[GMT_Z]), urint (Ctrl->D.dim[GMT_Y] * Ctrl->D.dim[GMT_Z]));
	
	/* Create a working directory which will house every local file and subdirectories created */
	
	if (gmt_mkdir (Ctrl->N.prefix)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create working directory %s - exiting\n", Ctrl->N.prefix);
		Return (GMT_RUNTIME_ERROR);
	}
	/* Make this directory the current directory */
	if (chdir (Ctrl->N.prefix)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to change directory to %s - exiting\n", Ctrl->N.prefix);
		perror (Ctrl->N.prefix);
		return GMT_RUNTIME_ERROR;
	}
	/* Get full path to this working directory */
	if (getcwd (cwd, GMT_BUFSIZ) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Unable to determine current working directory.\n");
		return GMT_RUNTIME_ERROR;
	}
	
	/* Create the initialization file for all scripts */
	
	sprintf (init_file, "movie_init.%s", extension[Ctrl->In.mode]);
	if ((fp = fopen (init_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create file %s - exiting\n", init_file);
		return GMT_ERROR_ON_FOPEN;
	}
	
	sprintf (string, "Static parameters for movie %s", Ctrl->N.prefix);
	set_comment (fp, Ctrl->In.mode, string);
	set_dvalue (fp, Ctrl->In.mode, "GMT_MOVIE_WIDTH", Ctrl->D.dim[GMT_X], Ctrl->D.unit);
	set_dvalue (fp, Ctrl->In.mode, "GMT_MOVIE_HEIGHT", Ctrl->D.dim[GMT_Y], Ctrl->D.unit);
	set_dvalue (fp, Ctrl->In.mode, "GMT_MOVIE_DPI", Ctrl->D.dim[GMT_Z], 0);
	set_tvalue (fp, Ctrl->In.mode, "GMT_MOVIE_DIR", cwd);
	if (Ctrl->G.active)	/* Want to set background color - we do this in psconvert */
		sprintf (extra, "A+g%s,P", Ctrl->G.fill);
	else	/* In either case we set Portrait mode */
		sprintf (extra, "P");
	fclose (fp);
	
	if (Ctrl->S[MOVIE_PREFLIGHT].active) {	/* Build the preflight script from the users background script */
		sprintf (pre_file, "movie_preflight.%s", extension[Ctrl->In.mode]);
		if ((fp = fopen (pre_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create file %s - exiting\n", pre_file);
			return GMT_ERROR_ON_FOPEN;
		}
		set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
		set_comment (fp, Ctrl->In.mode, "Preflight script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a PPID since subshells may mess things up */
		set_tvalue (fp, Ctrl->In.mode, "GMT_PPID", "$$");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		place_script (GMT, fp, Ctrl->S[MOVIE_PREFLIGHT].fp);		/* Place rest of script commands as is */
		fprintf (fp, "%s %s\n", rmfile[Ctrl->In.mode], pre_file);	/* Delete the preflight script when it finishes */
		fclose (fp);
#ifndef WIN32
		/* Set executable bit if not Windows */
		if (chmod (pre_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to make preflight script executable: %s - exiting\n", pre_file);
			return GMT_RUNTIME_ERROR;
		}
#endif
		/* Run the pre-flight which may create a file needed later via -T */
		if ((error = system (pre_file))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running preflight script %s returned error %d - exiting.\n", pre_file, error);
			return GMT_RUNTIME_ERROR;
		}
	}
	
	/* Now we can check -T since any needed file will have been created */
	
	if (!gmt_access (GMT, Ctrl->T.file, R_OK)) {	/* A file by that name exists and is readable */
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->T.file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to read time file: %s - exiting\n", Ctrl->T.file);
			Return (API->error);
		}
		if (D->n_segments > 1) {	/* We insist on a simple file structure with a single segment */
			GMT_Report (API, GMT_MSG_NORMAL, "Your time file %s has more than one segment - reformat first\n", Ctrl->T.file);
			Return (API->error);
		}
		n_frames = D->n_records;	/* Number of records means number of frames */
		n_values = D->n_columns;	/* The number of per-frame parameters we need to place into the per-frame parameter files */
	}
	else	/* Just gave the number of frames */
		n_frames = atoi (Ctrl->T.file);
	
	if (n_frames == 0) {	/* But need to check it */
		GMT_Report (API, GMT_MSG_NORMAL, "No frames specified! - exiting.\n");
		return GMT_RUNTIME_ERROR;
	}
	
	if (Ctrl->S[MOVIE_POSTFLIGHT].active) {	/* Prepare the postflight script */
		sprintf (post_file, "movie_postflight.%s", extension[Ctrl->In.mode]);
		if ((fp = fopen (post_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create postflight file %s - exiting\n", post_file);
			return GMT_ERROR_ON_FOPEN;
		}
		set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
		set_comment (fp, Ctrl->In.mode, "Postflight script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a PPID since subshells may mess things up */
		set_tvalue (fp, Ctrl->In.mode, "GMT_PPID", "$$");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		place_script (GMT, fp, Ctrl->S[MOVIE_POSTFLIGHT].fp);		/* Place rest of script commands as is */
		fprintf (fp, "%s %s\n", rmfile[Ctrl->In.mode], post_file);	/* Delete the postflight script when it finishes */
		fclose (fp);
#ifndef WIN32
		/* Set executable bit if not Windows */
		if (chmod (post_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to make postflight script executable: %s - exiting\n", post_file);
			return GMT_RUNTIME_ERROR;
		}
#endif
		/* Run post-flight now before dealing with the loop so the overlay exists */
		if ((error = system (post_file))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running postflight script %s returned error %d - exiting.\n", post_file, error);
			return GMT_RUNTIME_ERROR;
		}
	}
	
	/* Create parameter include files for each frame */
	
	for (frame = 0; frame < n_frames; frame++) {
		if (Ctrl->Q.active && frame != Ctrl->Q.frame) continue;	/* Just doing a single frame for debugging */
		sprintf (state_prefix, "movie_params_%6.6d", frame);
		sprintf (state_file, "%s.%s", state_prefix, extension[Ctrl->In.mode]);
		if ((fp = fopen (state_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create frame parameter file %s - exiting\n", state_file);
			return GMT_ERROR_ON_FOPEN;
		}
		sprintf (state_prefix, "Parameter file for frame %6.6d", frame);
		set_comment (fp, Ctrl->In.mode, state_prefix);
		sprintf (state_prefix, "%s_%6.6d", Ctrl->N.prefix, frame);
		set_ivalue (fp, Ctrl->In.mode, "GMT_MOVIE_FRAME", frame);	/* Current frame number */
		set_tvalue (fp, Ctrl->In.mode, "GMT_MOVIE_NAME", state_prefix);	/* Current frame name prefix */
		for (col = 0; col < n_values; col++) {	/* Place frame variables from file into parameter file */
			sprintf (string, "GMT_MOVIE_VAL%u", col+1);
			set_dvalue (fp, Ctrl->In.mode, string, D->table[0]->segment[0]->data[col][frame], 0);
		}
		if (D && D->table[0]->segment[0]->text)	/* Also place any string parameter */
			set_tvalue (fp, Ctrl->In.mode, "GMT_MOVIE_STRING", D->table[0]->segment[0]->text[frame]);
		fclose (fp);
	}
	
	/* Now build loop script from the mainscript */
	
	sprintf (main_file, "movie_frame.%s", extension[Ctrl->In.mode]);
	if ((fp = fopen (main_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create loop frame script file %s - exiting\n", main_file);
		return GMT_ERROR_ON_FOPEN;
	}
	set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
	set_comment (fp, Ctrl->In.mode, "Main frame loop script");
	fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a PPID since subshells may mess things up */
	set_tvalue (fp, Ctrl->In.mode, "GMT_PPID", "$$");
	set_comment (fp, Ctrl->In.mode, "Set static and frame-specific parameters");
	fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
	fprintf (fp, "%s movie_params_%c1.%s\n", load[Ctrl->In.mode], var[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Include the frame parameters */
	fprintf (fp, "mkdir %s\n", place_var (Ctrl->In.mode, "GMT_MOVIE_NAME"));	/* Make a temp directory for this frame */
	fprintf (fp, "cd %s\n", place_var (Ctrl->In.mode, "GMT_MOVIE_NAME"));		/* cd to the temp directory */
	while (gmt_fgets (GMT, line, GMT_BUFSIZ, Ctrl->In.fp)) {	/* Read the mainscript and copy to loop script with some exceptions */
		if (strstr (line, "gmt begin")) {	/* Need to insert gmt figure after this line */
			fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
			set_comment (fp, Ctrl->In.mode, "\tSet output PNG name and conversion parameters");
			fprintf (fp, "\tgmt figure %s png", place_var (Ctrl->In.mode, "GMT_MOVIE_NAME"));
			fprintf (fp, " E%s,%s\n", place_var (Ctrl->In.mode, "GMT_MOVIE_DPI"), extra);
			n_begin++;	/* Processed the gmt begin line */
		}
		else if (!strstr (line, "#!/bin"))	/* Skip any leading shell incantation since already placed */
			fprintf (fp, "%s", line);	/* Just copy the line as is */
	}
	fclose (Ctrl->In.fp);	/* Done reading the main script */
	set_comment (fp, Ctrl->In.mode, "Move PNG file up to parent directory and cd up one level");
	fprintf (fp, "%s %s.png ..\n", mvfile[Ctrl->In.mode], place_var (Ctrl->In.mode, "GMT_MOVIE_NAME"));	/* Move PNG plot up to parent dir */
	fprintf (fp, "cd ..\n");	/* cd up to parent dir */
	set_comment (fp, Ctrl->In.mode, "Remove frame directory and parameter file");
	fprintf (fp, "%s %s\n", rmdir[Ctrl->In.mode], place_var (Ctrl->In.mode, "GMT_MOVIE_NAME"));	/* Remove the work dir and any files in it*/
	fprintf (fp, "%s movie_params_%c1.%s\n", rmdir[Ctrl->In.mode], var[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Remove the parameter file */
	fclose (fp);
	
	if (n_begin != 1) {
		GMT_Report (API, GMT_MSG_NORMAL, "Main script %s is not in modern mode - exiting\n", main_file);
		return GMT_RUNTIME_ERROR;
	}
	
#ifndef WIN32
		/* Set executable bit if not Windows */
	if (chmod (main_file, S_IRWXU)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to make script executable: %s - exiting\n", main_file);
		return GMT_RUNTIME_ERROR;
	}
#endif
	
	if (Ctrl->Q.active) {	/* Just do one frame and then exit (no movie generated) */
		char frame_cmd[GMT_LEN32];
        	sprintf (frame_cmd, "%s %6.6d", main_file, Ctrl->Q.frame);
		if ((error = system (frame_cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running script %s returned error %d - exiting.\n", frame_cmd, error);
			return GMT_RUNTIME_ERROR;
		}
		GMT_Report (API, GMT_MSG_NORMAL, "Single PNG frame built: %s_%6.6d.png\n", Ctrl->N.prefix, Ctrl->Q.frame);
		return GMT_NOERROR;
	}

	/* Finally, we can run all the frames in an OpenMP loop (if available) */
	
#pragma omp parallel
	{
		char frame_cmd[GMT_LEN32];
#pragma omp for nowait
		for (frame = 0; frame < n_frames; frame++) {
	        	sprintf (frame_cmd, "%s %6.6d", main_file, frame);
			GMT_Report (API, GMT_MSG_NORMAL, "Running script %s\n", frame_cmd);
			if ((error = system (frame_cmd))) {
				GMT_Report (API, GMT_MSG_NORMAL, "Running script %s returned error %d - exiting.\n", frame_cmd, error);
				return GMT_RUNTIME_ERROR;
			}
		}
	}

	gmt_remove_file (GMT, main_file);	/* Remove main loop script */
	gmt_remove_file (GMT, init_file);	/* Remove initialization script */

	/* Cd back up to parent directory */
	if (chdir ("..")) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to change directory to parent directory - exiting.\n");
		perror (Ctrl->N.prefix);
		return GMT_RUNTIME_ERROR;
	}

	if (Ctrl->F.mode == MOVIE_GIF) {
		/* Set up system call to gm */
		unsigned int delay = urint (100.0 / Ctrl->W.rate);	/* Delay to nearest 1/100 s */
		if (Ctrl->W.loop)	/* Add the -loop argument */
			sprintf (extra, "-loop %u +dither", Ctrl->W.n_loops);
		else
			sprintf (extra, "+dither");
		sprintf (cmd, "gm convert -delay %u %s %s/%s_*.png %s.gif", delay, extra, Ctrl->N.prefix, Ctrl->N.prefix, Ctrl->N.prefix);
		if ((error = system (cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running GIF conversion returned error %d - exiting.\n", error);
			return GMT_RUNTIME_ERROR;
		}
		GMT_Report (API, GMT_MSG_NORMAL, "GIF animation built: %s.gif\n", Ctrl->N.prefix);
	}
	else if (Ctrl->F.mode == MOVIE_MP4) {
		/* Set up system call to ffmpeg */
		if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE))
			sprintf (extra, "verbose");
		else
			sprintf (extra, "quiet");
		sprintf (cmd, "ffmpeg -loglevel %s -f image2 -pattern_type glob -framerate %g -y -i \"%s/%s_*.png\" -pix_fmt yuv420p %s.m4v",
			extra, Ctrl->W.rate, Ctrl->N.prefix, Ctrl->N.prefix, Ctrl->N.prefix);
		if ((error = system (cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running MP4 conversion returned error %d - exiting.\n", error);
			return GMT_RUNTIME_ERROR;
		}
		GMT_Report (API, GMT_MSG_NORMAL, "MP4 movie built: %s.mv4\n", Ctrl->N.prefix);
	}
	GMT_Report (API, GMT_MSG_NORMAL, "%u frame PNG files placed in directory: %s\n", n_frames, Ctrl->N.prefix);

	Return (error);
}
