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
 * Date:	1-Jan-2018
 * Version:	6 API
 *
 * Brief synopsis: gmt movie automates the generation of animations
 *
 * movie automates the main frame loop and much of the machinery needed
 * to script a movie sequence.  It allows for an optional background
 * and foreground layer to be specified via separate modern scripts and
 * a single frame-script that uses special variables to mode slightly
 * different plots for each frame.  The user only needs to compose these
 * simple one-plot scripts and then movie takes care of the automation,
 * processing to PNG, assembly to a movie or animated GIF, and cleanup.
 * Frame scripts are run in parallel without need for OpenMP etc.
 * Experimental.
 */

#include "gmt_dev.h"
#ifdef WIN32
#include <windows.h> 
#endif

#define THIS_MODULE_NAME	"movie"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Create animation sequences and movies"
#define THIS_MODULE_KEYS	""
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"Vx"

#define MOVIE_PREFLIGHT		0
#define MOVIE_POSTFLIGHT	1

#define MOVIE_WAIT_TO_CHECK	10000	/* In microseconds, so 0.01 seconds */

#define MOVIE_RASTER_FORMAT	"png"	/* Lossless raster format */
#define MOVIE_DEBUG_FORMAT	",ps"	/* Comma is intentional since we append to a list of formats */

enum enum_script {BASH_MODE = 0,	/* Write Bash script */
	CSH_MODE,			/* Write C-shell script */
	DOS_MODE};			/* Write DOS script */

enum enum_video {MOVIE_MP4,	/* Create a H.264 MP4 video */
	MOVIE_WEBM,		/* Create a WebM video */
	MOVIE_N_FORMATS};	/* Number of video formats above */

enum enum_label {MOVIE_LABEL_IS_FRAME = 1,
	MOVIE_LABEL_IS_COL_C = 2,
	MOVIE_LABEL_IS_COL_T};

/* Control structure for movie */

struct MOVIE_CTRL {
	bool animate;	/* True if we are making any animated product (GIF, movies) */
	struct In {	/* mainscript (Bourne, Bourne Again, csh, or DOS (bat) script) */
		bool active;
		enum enum_script mode;
		char *file;
		FILE *fp;
	} In;
	struct A {	/* -A[+l[<nloops>]][+s<stride>]  */
		bool active;
		bool loop;
		bool skip;
		unsigned int loops;
		unsigned int stride;
	} A;
	struct C {	/* -C<namedcanvas>|<canvas_and_dpu> */
		bool active;
		double dim[3];
		char unit;
		char *string;
	} C;
	struct D {	/* -D<displayrate> */
		bool active;
		double framerate;
	} D;
	struct F {	/* -F<videoformat>[+o<options>] */
		bool active[MOVIE_N_FORMATS];
		char *format[MOVIE_N_FORMATS];
		char *options[MOVIE_N_FORMATS];
	} F;
	struct G {	/* -G<canvasfill> */
		bool active;
		char *fill;
	} G;
	struct H {	/* -H<factor> */
		bool active;
		int factor;
	} H;
	struct I {	/* -I<includefile> */
		bool active;
		char *file;
		FILE *fp;
	} I;
	struct L {	/* -L[f|c#|t#][+c<clearance>][+f<fmt>][+g<fill>][+j<justify>][+o<offset>][+p<pen>] */
		bool active;
		char format[GMT_LEN64];
		char fill[GMT_LEN64];
		char pen[GMT_LEN64];
		char placement[3];
		unsigned int mode;
		unsigned int col;
		double x, y;
		double off[2];
		double clearance[2];
	} L;
	struct M {	/* -M[<frame>][,format] */
		bool active;
		bool exit;
		unsigned int frame;
		char *format;
	} M;
	struct N {	/* -N<movieprefix> */
		bool active;
		char *prefix;
	} N;
	struct Q {	/* -Q[s] */
		bool active;
		bool scripts;
	} Q;
	struct S {	/* -Sb|f<script> */
		bool active;
		char *file;
		FILE *fp;
	} S[2];
	struct T {	/* -T<n_frames>|<timefile>[+p<precision>][+s<frame>][+w] */
		bool active;
		bool split;
		unsigned int n_frames;
		unsigned int start_frame;
		unsigned int precision;
		char *file;
	} T;
	struct W {	/* -W<dir> */
		bool active;
		char *dir;
	} W;
	struct Z {	/* -Z */
		bool active;
	} Z;
};

struct MOVIE_STATUS {
	/* Used to monitor the start, running, and completion of frame jobs running in parallel */
	bool started;	/* true if frame job has started */
	bool completed;	/* true if PNG has been successfully produced */
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MOVIE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct MOVIE_CTRL);
	C->A.loops = 1;		/* No loop, just play once */
	C->C.unit = 'c';	/* c for SI units */
	C->D.framerate = 24.0;	/* 24 frames/sec */
	C->F.options[MOVIE_WEBM] = strdup ("-crf 10 -b:v 1.2M");	/* Default WebM options for now */
	C->L.fill[0] = C->L.pen[0] = '-';	/* No fill or outline */
	C->L.off[GMT_X] = C->L.off[GMT_Y] = 0.01 * GMT_TEXT_OFFSET * GMT->current.setting.font_tag.size / PSL_POINTS_PER_INCH; /* 20% */
	C->L.clearance[GMT_X] = C->L.clearance[GMT_Y] = 0.01 * GMT_TEXT_CLEARANCE * GMT->current.setting.font_tag.size / PSL_POINTS_PER_INCH;	/* 15% */
	sprintf (C->L.placement, "TL");
	GMT->common.x.n_threads = GMT->parent->n_cores;
	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct MOVIE_CTRL *C) {	/* Deallocate control structure */
	gmt_M_unused (GMT);
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->C.string);
	gmt_M_str_free (C->M.format);
	for (unsigned int k = 0; k < MOVIE_N_FORMATS; k++) {
		gmt_M_str_free (C->F.format[k]);
		gmt_M_str_free (C->F.options[k]);
	}
	gmt_M_str_free (C->G.fill);
	gmt_M_str_free (C->I.file);
	gmt_M_str_free (C->N.prefix);
	gmt_M_str_free (C->S[MOVIE_PREFLIGHT].file);
	gmt_M_str_free (C->S[MOVIE_POSTFLIGHT].file);
	gmt_M_str_free (C->T.file);
	gmt_M_str_free (C->W.dir);
	gmt_M_free (GMT, C);
}

/*! -x[[-]<ncores>] parsing needed even if no OPenMP since cores are not managed by OpenMP in this
 * module, yet we want to use -x for that purpose */
#ifndef GMT_MP_ENABLED
GMT_LOCAL int gmtinit_parse_x_option (struct GMT_CTRL *GMT, char *arg) {
	if (!arg) return (GMT_PARSE_ERROR);	/* -x requires a non-NULL argument */
	if (arg[0])
		GMT->common.x.n_threads = atoi (arg);

	if (GMT->common.x.n_threads == 0)
		GMT->common.x.n_threads = 1;
	else if (GMT->common.x.n_threads < 0)
		GMT->common.x.n_threads = MAX(GMT->parent->n_cores - GMT->common.x.n_threads, 1);		/* Max-n but at least one */
	return (GMT_NOERROR);
}
#endif

GMT_LOCAL int gmt_sleep (unsigned int microsec) {
	/* Waiting before checking if the PNG has been completed */
#ifdef WIN32
	Sleep ((uint32_t)microsec/1000);	/* msec are microseconds but Sleep wants millisecs */
	return 0;
#else
	return (usleep ((useconds_t)microsec));
#endif
}

GMT_LOCAL void set_dvalue (FILE *fp, int mode, char *name, double value, char unit) {
	/* Assigns a single named floating point variable given the script mode */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "%s=%g", name, value);       break;
		case CSH_MODE:  fprintf (fp, "set %s = %g", name, value); break;
		case DOS_MODE:  fprintf (fp, "set %s=%g", name, value);   break;
	}
	if (unit) fprintf (fp, "%c", unit);	/* Append the unit [c|i|p] unless 0 */
	fprintf (fp, "\n");
}

GMT_LOCAL void set_ivalue (FILE *fp, int mode, char *name, int value) {
	/* Assigns a single named integer variable given the script mode */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "%s=%d\n", name, value);       break;
		case CSH_MODE:  fprintf (fp, "set %s = %d\n", name, value); break;
		case DOS_MODE:  fprintf (fp, "set %s=%d\n", name, value);   break;
	}
}

GMT_LOCAL void set_tvalue (FILE *fp, int mode, char *name, char *value) {
	/* Assigns a single named text variable given the script mode */
	if (strchr (value, ' ') || strchr (value, '\t')) {	/* String has spaces or tabs */
		switch (mode) {
			case BASH_MODE: fprintf (fp, "%s=\"%s\"\n", name, value);       break;
			case CSH_MODE:  fprintf (fp, "set %s = \"%s\"\n", name, value); break;
			case DOS_MODE:  fprintf (fp, "set %s=\"%s\"\n", name, value);   break;
		}
	}
	else {	/* Single word */
		switch (mode) {
			case BASH_MODE: fprintf (fp, "%s=%s\n", name, value);       break;
			case CSH_MODE:  fprintf (fp, "set %s = %s\n", name, value); break;
			case DOS_MODE:  fprintf (fp, "set %s=%s\n", name, value);   break;
		}
	}
}

GMT_LOCAL void set_script (FILE *fp, int mode) {
	/* Writes the script's incantation line (or a comment for DOS, turning off default echo) */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "#!/bin/bash\n"); break;
		case CSH_MODE:  fprintf (fp, "#!/bin/csh\n"); break;
		case DOS_MODE:  fprintf (fp, "@echo off\nREM Start of script\n"); break;
	}
}

GMT_LOCAL void set_comment (FILE *fp, int mode, char *comment) {
	/* Write a comment line given the script mode */
	switch (mode) {
		case BASH_MODE: case CSH_MODE:  fprintf (fp, "# %s\n", comment); break;
		case DOS_MODE:  fprintf (fp, "REM %s\n", comment); break;
	}
}

GMT_LOCAL char *place_var (int mode, char *name) {
	/* Prints a single variable to stdout where needed in the script via the string static variable.
	 * PS!  Only one call per printf statement since static string cannot hold more than one item at the time */
	static char string[GMT_LEN128] = {""};	/* So max length of variable name is 127 */
	if (mode == DOS_MODE)
		sprintf (string, "%%%s%%", name);
	else
		sprintf (string, "${%s}", name);
	return (string);
}

GMT_LOCAL int dry_run_only (const char *cmd) {
	/* Dummy function to not actually run the loop script when -Q is used */
	gmt_M_unused (cmd);
	return 0;
}

GMT_LOCAL unsigned int check_language (struct GMT_CTRL *GMT, unsigned int mode, char *file) {
	unsigned int n_errors = 0;
	/* Examines file extension and compares to known mode from mainscript */
	switch (mode) {
		case BASH_MODE:
			if (!(strstr (file, ".bash") || strstr (file, ".sh"))) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Main script is bash/sh but %s is not!\n", file);
				n_errors++;
			}
			break;
		case CSH_MODE:
			if (!strstr (file, ".csh")) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Main script is csh but %s is not!\n", file);
				n_errors++;
			}
			break;
		case DOS_MODE:
			if (!strstr (file, ".bat")) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Main script is bat but %s is not!\n", file);
				n_errors++;
			}
			break;
	}
	return (n_errors);
}

GMT_LOCAL bool script_is_classic (struct GMT_CTRL *GMT, FILE *fp) {
	/* Read script to determine if it is in GMT classic mode or not, then rewind */
	bool classic = true;
	char line[PATH_MAX] = {""};
	while (classic && gmt_fgets (GMT, line, PATH_MAX, fp)) {
		if (strstr (line, "gmt begin"))	/* A modern mode script */
			classic = false;
	}
	rewind (fp);	/* Go back to beginning of file */
	return (classic);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: movie <mainscript> -C<canvas> -N<prefix> -T<n_frames>|<timefile>[+p<width>][+s<first>][+w]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-A[+l[<n>]][+s<stride>]] [-D<rate>] [-F<format>[+o<opts>]] [-G<fill>] [-H<factor>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-I<includefile>] [-L<labelinfo>] [-M[<frame>,][<format>]] [-Q[s]] [-Sb<script>] [-Sf<script>] [%s]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-W<workdir>] [-Z] [-x[[-]<n>]]\n\n");

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\t<mainscript> is the main GMT modern script that builds a single frame image.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Specify canvas size. Choose a known named canvas or set custom dimensions:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Recognized 16:9-ratio formats:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      name:	pixel size:   canvas size (SI):  canvas size (US):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     4320p:	7680 x 4320	24 x 13.5 cm	 9.6 x 5.4 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     2160p:	3840 x 2160	24 x 13.5 cm	 9.6 x 5.4 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     1080p:	1920 x 1080	24 x 13.5 cm	 9.6 x 5.4 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      720p:	1280 x  720	24 x 13.5 cm	 9.6 x 5.4 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      540p:	 960 x  540	24 x 13.5 cm	 9.6 x 5.4 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      480p:	 854 x  480	24 x 13.5 cm	 9.6 x 5.4 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      360p:	 640 x  360	24 x 13.5 cm	 9.6 x 5.4 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      240p:	 426 x  240	24 x 13.5 cm	 9.6 x 5.4 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: 8k can be used for 4320p, UHD and 4k for 2160p and HD for 1080p.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Recognized 4:3-ratio formats:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      name:	pixel size:   canvas size (SI):	 canvas size (US):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      UXGA: 1600 x 1200	24 x 18 cm	 9.6 x 7.2 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     SXGA+: 1400 x 1050	24 x 18 cm	 9.6 x 7.2 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       XGA: 1024 x  768	24 x 18 cm	 9.6 x 7.2 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      SVGA:	 800 x  600	24 x 18 cm	 9.6 x 7.2 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       DVD:	 640 x  480	24 x 18 cm	 9.6 x 7.2 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: Current PROJ_LENGTH_UNIT determines if you get SI or US canvas dimensions.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, set a custom canvas with dimensions and dots-per-unit manually by\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     providing <width>[<unit>]x<height>[<unit>]x<dpu> (e.g., 15cx10cx50, 6ix6ix100, etc.).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Set the <prefix> used for movie files and directory names.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set number of frames, or give name of file with frame-specific information.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If <timefile> does not exist it must be created by the -Sf script.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p<width> to set number of digits used in creating the frame tags [automatic].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +s<first> to change the value of the first frame [0].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +w to <timefile> to have trailing text be split into word variables.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Animated GIF: Append +l for looping [no loop]; optionally append number of loops [infinite loop].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If -F is used you may restrict the GIF animation to use every <stride> frame only [all];\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   <stride> must be taken from the list 2, 5, 10, 20, 50, 100, 200, or 500.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set movie display frame rate in frames/second [24].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Select the final video format(s) from among these choices. Repeatable:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     mp4:    Convert PNG frames into an MP4 movie.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     webm:   Convert PNG frames into an WebM movie.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     none:   Make no PNG frames - requires -M.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Optionally, append +o<options> to add custom encoding options for mp4 or webm.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     [Default is no video products; just create the PNG frames].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Set the canvas background color [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-H Temporarily increase dpu by <factor>, rasterize, then downsample [no downsampling].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Stabilize sub-pixel changes between frames, such as moving text and lines.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Include script file to be inserted into the movie_init.sh script [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Used to add constant variables needed by all movie scripts.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Automatic labeling of frames.  Places chosen label at the frame perimeter:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f selectes the running frame number as the label.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c<col> uses the value in column <col> of <timefile> (first column is 1).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     t<col> uses word number <col> from the trailing text in <timefile> (requires -T...+w).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c<dx>[/<dy>] for the clearance between label and surrounding box.  Only\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     used if +g or +p are set.  Append units {%s} or %% of fontsize [%d%%].\n", GMT_DIM_UNITS_DISPLAY, GMT_TEXT_CLEARANCE);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +f to provide a format statement to be used with the item selected [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +g to fill the label textbox with <fill> color [no fill].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +j<refpoint> to specify where the label should be plotted [TL].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +o<dx>[/<dy>] to offset label in direction implied by <justify> [%d%% of font size].\n", GMT_TEXT_OFFSET);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p to draw the outline of the textbox using selected pen [no outline].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Create a master frame plot as well; append comma-separated frame number [0] and format [pdf].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Master plot will be named <prefix>.<format> and placed in the current directory.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Debugging: Leave all intermediate files and directores behind for inspection.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append s to only create the work scripts but none will be executed.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Set the optional background and foreground GMT scripts [none]:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sb Append the name of a background script that may pre-compute\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       files needed by <mainscript> and build a static background plot layer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       If a plot is generated then the script must be in GMT modern mode.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sf Append name of foreground GMT modern mode script which will\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       build a static foreground plot overlay added to all frames.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Add the given <workdir> to search path where data files may be found.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default searches current dir, <prefix> dir, and directories set via DIR_DATA].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Erase directory <prefix> after converting to movie [leave directory with PNGs alone].\n");
	/* Number of threads (repurposed from -x in GMT_Option) */
	GMT_Message (API, GMT_TIME_NONE, "\t-x Limit the number of cores used in frame generation [Default uses all cores = %d].\n", API->n_cores);
	GMT_Message (API, GMT_TIME_NONE, "\t   -x<n>  Select <n> cores (up to all available).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -x-<n> Select (all - <n>) cores (or at least 1).\n");
	GMT_Option (API, ".");
	
	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct MOVIE_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to grdcut and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 */

	unsigned int n_errors = 0, n_files = 0, k, pos, mag;
	int n;
	char txt_a[GMT_LEN32] = {""}, txt_b[GMT_LEN32] = {""}, arg[GMT_LEN64] = {""}, p[GMT_LEN256] = {""};
	char *c = NULL, *s = NULL, string[GMT_LEN128] = {""};
	double width = 24.0, height16x9 = 13.5, height4x3 = 18.0, dpu = 160.0;	/* SI values */
	struct GMT_FILL fill;	/* Only used to make sure any fill is given with correct syntax */
	struct GMT_PEN pen;	/* Only used to make sure any pen is given with correct syntax */
	struct GMT_OPTION *opt = NULL;
	
	if (GMT->current.setting.proj_length_unit == GMT_INCH) {	/* Switch from SI to US dimensions in inch given format names */
		width = 9.6;	height16x9 = 5.4;	height4x3 = 7.2;	dpu = 400.0;
		Ctrl->C.unit = 'i';
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
				
			case 'A':	/* Animated GIF */
				Ctrl->A.active = Ctrl->animate = true;
				if ((c = gmt_first_modifier (GMT, opt->arg, "ls"))) {	/* Process any modifiers */
					pos = 0;	/* Reset to start of new word */
					while (gmt_getmodopt (GMT, 'A', c, "ls", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'l':	/* Specify loops */
								Ctrl->A.loop = true; 
								Ctrl->A.loops = (p[1]) ? atoi (&p[1]) : 0;
								break;
							case 's':	/* Specify GIF stride, 2,5,10,20,50,100,200,500 etc. */
								Ctrl->A.skip = true;
								Ctrl->A.stride = atoi (&p[1]);
								mag = urint (pow (10.0, floor (log10 ((double)Ctrl->A.stride))));
								k = Ctrl->A.stride / mag;
								if (!(k == 1 || k == 2 || k == 5)) {
									GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -A+s: Allowable strides are 2,5,10,20,50,100,200,500,...\n");
									n_errors++;
								}	
								break;
							default:
								break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					c[0] = '\0';
				}
				break;

			case 'C':	/* Known frame dimension or set a custom canvas size */
				Ctrl->C.active = true;
				strcpy (arg, opt->arg);		/* Get a copy... */
				gmt_str_tolower (arg);		/* ..so we can make it lower case */
				/* 16x9 formats */
				if (!strcmp (arg, "8k") || !strcmp (arg, "4320p")) {	/* 4320x7680 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height16x9;	Ctrl->C.dim[GMT_Z] = 2.0 * dpu;
				}
				else if (!strcmp (arg, "4k") || !strcmp (arg, "uhd") || !strcmp (arg, "2160p")) {	/* 2160x3840 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height16x9;	Ctrl->C.dim[GMT_Z] = dpu;
				}
				else if (!strcmp (arg, "1080p") || !strcmp (arg, "hd")) {	/* 1080x1920 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height16x9;	Ctrl->C.dim[GMT_Z] = dpu / 2.0;
				}
				else if (!strcmp (arg, "720p")) {	/* 720x1280 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height16x9;	Ctrl->C.dim[GMT_Z] = dpu / 3.0;
				}
				else if (!strcmp (arg, "540p")) {	/* 540x960 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height16x9;	Ctrl->C.dim[GMT_Z] = dpu / 4.0;
				}	/* 4x3 formats below */
				else if (!strcmp (arg, "480p")) {	/* 480x854 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height16x9;	Ctrl->C.dim[GMT_Z] = dpu / 6.0;
				}
				else if (!strcmp (arg, "360p")) {	/* 360x640 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height16x9;	Ctrl->C.dim[GMT_Z] = dpu / 8.0;
				}
				else if (!strcmp (arg, "240p")) {	/* 240x426 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height16x9;	Ctrl->C.dim[GMT_Z] = dpu / 12.0;
				}	/* Below are 4x3 formats */
				else if (!strcmp (arg, "uxga") ) {	/* 1200x1600 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height4x3;	Ctrl->C.dim[GMT_Z] = 2.5 * dpu / 6.0;
				}
				else if (!strcmp (arg, "sxga+") ) {	/* 1050x1400 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height4x3;	Ctrl->C.dim[GMT_Z] = 2.1875 * dpu / 6.0;
				}
				else if (!strcmp (arg, "xga") ) {	/* 768x1024 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height4x3;	Ctrl->C.dim[GMT_Z] = 1.6 * dpu / 6.0;
				}
				else if (!strcmp (arg, "sgva") ) {	/* 600x800 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height4x3;	Ctrl->C.dim[GMT_Z] = 1.25 * dpu / 6.0;
				}
				else if (!strcmp (arg, "dvd")) {	/* 480x640 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height4x3;	Ctrl->C.dim[GMT_Z] = dpu / 6.0;
				}
				else {	/* Custom canvas dimensions */
					if ((n = sscanf (arg, "%[^x]x%[^x]x%lg", txt_a, txt_b, &Ctrl->C.dim[GMT_Z])) != 3) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -C: Requires name of a known format or give width x height x dpu string\n");
						n_errors++;
					}
					else {	/* Got three items; lets check */
						Ctrl->C.dim[GMT_X] = atof (txt_a);	
						Ctrl->C.dim[GMT_Y] = atof (txt_b);
						if (strchr ("cip", txt_a[strlen(txt_a)-1]))	/* Width had recognized unit, set it */
							Ctrl->C.unit = txt_a[strlen(txt_a)-1];
						else if (strchr ("cip", txt_b[strlen(txt_b)-1]))	/* Height had recognized unit, set it instead */
							Ctrl->C.unit = txt_b[strlen(txt_b)-1];
						else	/* Must use GMT default setting as unit */
							Ctrl->C.unit = GMT->session.unit_name[GMT->current.setting.proj_length_unit][0];
					}
				}
				break;

			case 'D':	/* Display frame rate of movie */
				Ctrl->D.active = true;
				Ctrl->D.framerate = atof (opt->arg);
				break;
				
			case 'F':	/* Set movie format and optional ffmpeg options */
				if ((c = strchr (opt->arg, '+')) && c[1] == 'o') {	/* Gave encoding options */
					s = &c[2];	/* Retain start of encoding options for later */
					c[0] = '\0';	/* Chop off options */
				}
				else
					s = NULL;	/* No encoding options given */
				strcpy (arg, opt->arg);	/* Get a copy of the args (minus encoding options)... */
				gmt_str_tolower (arg);	/* ..so we can convert it to lower case for comparisions */
				if (c) c[0] = '+';	/* Now we can restore the optional text we chopped off */
				if (!strcmp (opt->arg, "none")) {	/* Do not make those PNGs at all, just a master plot */
					Ctrl->M.exit = true;
					break;
				}
				else if (!strcmp (opt->arg, "mp4"))
					k = MOVIE_MP4;
				else if (!strcmp (opt->arg, "webm"))
					k = MOVIE_WEBM;
				else {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -F: Unrecognized format %s\n", opt->arg);
					n_errors++;
					break;
				}
				if (Ctrl->F.active[k]) {	/* Can only select a format once */
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -F: Format %s already selected\n", opt->arg);
					n_errors++;
					break;
				}
				/* Here we have a new video format selected */
				Ctrl->F.active[k] = Ctrl->animate = true;
				if (s) {	/* Gave specific encoding options */
					if (Ctrl->F.options[k]) gmt_M_str_free (Ctrl->F.options[k]);	/* Free old setting first */
					Ctrl->F.options[k] = strdup (s);
				}
				break;

			case 'G':	/* Canvas fill */
				Ctrl->G.active = true;
				if (gmt_getfill (GMT, opt->arg, &fill))
					n_errors++;
				else
					Ctrl->G.fill = strdup (opt->arg);
				break;

			case 'H':	/* RIP at a higher dpu, then downsample in gs to improve sub-pixeling */
				Ctrl->H.active = true;
				Ctrl->H.factor = atoi (opt->arg);
				break;

			case 'I':	/* Include file with settings used by all scripts */
				Ctrl->I.active = true;
				Ctrl->I.file = strdup (opt->arg);
				break;

			case 'L':	/* Label frame and get attributes */
				Ctrl->L.active = true;
				if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
				switch (opt->arg[0]) {
					case 'c':	Ctrl->L.mode = MOVIE_LABEL_IS_COL_C;	Ctrl->L.col = atoi (&opt->arg[1]);	break;
					case 't':	Ctrl->L.mode = MOVIE_LABEL_IS_COL_T;	Ctrl->L.col = atoi (&opt->arg[1]);	break;
					case 'f': case '\0': Ctrl->L.mode = MOVIE_LABEL_IS_FRAME;	break;
					default:	/* Not recognized */
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -L: Select -Lf, -Lc<col> or -Lt<col>\n");
						n_errors++;
						break;
				}
				if (c) {	/* Gave modifiers so we must parse those now */
					c[0] = '+';	/* Restore modifiers */
					/* Modifiers are [+c<dx/dy>][+f<fmt>][+g<fill>][+j<justify>][+o<dx/dy>][+p<pen>] */
					if (gmt_get_modifier (opt->arg, 'c', string) && string[0])	/* Clearance for box */
						if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->L.clearance) < 0) n_errors++;
					if (gmt_get_modifier (opt->arg, 'f', Ctrl->L.format) && !Ctrl->L.format[0]) {
						n_errors++;
					}
					if (gmt_get_modifier (opt->arg, 'g', Ctrl->L.fill) && Ctrl->L.fill[0]) {
						if (gmt_getfill (GMT, Ctrl->L.fill, &fill)) n_errors++;
					}
					if (gmt_get_modifier (opt->arg, 'j', Ctrl->L.placement)) {	/* Inside placement */
						gmt_just_validate (GMT, Ctrl->L.placement, "TL");
					}
					if (gmt_get_modifier (opt->arg, 'o', string))	/* Offset refpoint */
						if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->L.off) < 0) n_errors++;
					if (gmt_get_modifier (opt->arg, 'p', Ctrl->L.pen) && Ctrl->L.pen[0]) {
						if (gmt_getpen (GMT, Ctrl->L.pen, &pen)) n_errors++;
					}
				}
				break;

			case 'M':	/* Create a single frame plot as well as movie (unless -Q is active) */
				Ctrl->M.active = true;
				if ((c = strchr (opt->arg, ',')) ) {	/* Gave frame and format */
					Ctrl->M.format = strdup (&c[1]);
					c[0] = '\0';	/* Chop off format */
					Ctrl->M.frame = atoi (opt->arg);
					c[0] = ',';	/* Restore format */
				}
				else if (isdigit (opt->arg[0])) {	/* Gave just a frame, deafult to PDF format */
					Ctrl->M.format = strdup ("pdf");
					Ctrl->M.frame = atoi (opt->arg);
				}
				else if (opt->arg[0])	/* Must be format, with frame = 0 implicit */
					Ctrl->M.format = strdup (opt->arg);
				else /* Default is PDF of frame 0 */
					Ctrl->M.format = strdup ("pdf");
				break;

			case 'N':	/* Movie prefix and directory name */
				Ctrl->N.active = true;
				Ctrl->N.prefix = strdup (opt->arg);
				break;
			
			case 'Q':	/* Debug - leave temp files and directories behind; Use -Qs tp only write scripts */
				Ctrl->Q.active = true;
				if (opt->arg[0] == 's') Ctrl->Q.scripts = true;
				break;

			case 'S':	/* background and foreground scripts */
				if (opt->arg[0] == 'b')
					k = MOVIE_PREFLIGHT;	/* background */
				else if (opt->arg[0] == 'f')
					k = MOVIE_POSTFLIGHT;	/* foreground */
				else {	/* Bad option */
					n_errors++;
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -S: Select -Sb or -Sf\n");
					break;
				}
				/* Got a valid f or b */
				Ctrl->S[k].active = true;
				Ctrl->S[k].file = strdup (&opt->arg[1]);
				if ((Ctrl->S[k].fp = fopen (Ctrl->S[k].file, "r")) == NULL) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -S%c: Unable to open script %s\n", opt->arg[0], Ctrl->S[k].file);
					n_errors++;
				}
				break;
	
			case 'T':	/* Number of frames or the name of file with frame information (note: file may not exist yet) */
				Ctrl->T.active = true;
				if ((c = gmt_first_modifier (GMT, opt->arg, "psw"))) {	/* Process any modifiers */
					pos = 0;	/* Reset to start of new word */
					while (gmt_getmodopt (GMT, 'T', c, "psw", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'p':	/* Set precision in frame naming ###### */
								Ctrl->T.precision = atoi (&p[1]);
								break;
							case 's':	/* Specify start frame other than 0 */
								Ctrl->T.start_frame = atoi (&p[1]);
								break;
							case 'w':	/* Split trailing text into words. */
								Ctrl->T.split = true;
								break;
							default:
								break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					c[0] = '\0';
				}
				Ctrl->T.file = strdup (opt->arg);
				if (c) c[0] = '+';	/* Restore modifiers */
				break;
				
			case 'W':	/* Work dir where data files may be found */
				Ctrl->W.active = true;
				Ctrl->W.dir = strdup (opt->arg);
				break;
				
			case 'Z':	/* Erase frames after movie has been made */
				Ctrl->Z.active = true;
				break;

#ifndef GMT_MP_ENABLED
			/* Do this to ensure we can access the x parsing when OpenMP has turn it off */
			case 'x':
				n_errors += gmtinit_parse_x_option (GMT, opt->arg);
				GMT->common.x.active = true;
				break;
#endif
		
			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	n_errors += gmt_M_check_condition (GMT, n_files != 1 || Ctrl->In.file == NULL, "Syntax error: Must specify a main script file\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->C.active, "Syntax error -C: Must specify a canvas dimension\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.exit && Ctrl->animate, "Syntax error -F: Cannot use none with other selections\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->Q.active && !Ctrl->M.active && !Ctrl->animate, "Syntax error: Must select at least one output product (-A, -F, -M)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && Ctrl->Z.active, "Syntax error: Cannot use -Z if -Q is also set\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->H.active && Ctrl->H.factor < 2, "Syntax error -H: factor must be and integer > 1\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->L.mode == MOVIE_LABEL_IS_COL_T && !Ctrl->T.split, "Syntax error -L: Must split trailing text to use -Lt\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.dim[GMT_X] <= 0.0 || Ctrl->C.dim[GMT_Y] <= 0.0,
					"Syntax error -C: Zero or negative canvas dimensions given\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.dim[GMT_Z] <= 0.0,
					"Syntax error -C: Zero or negative canvas dots-per-unit given\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->N.active || (Ctrl->N.prefix == NULL || strlen (Ctrl->N.prefix) == 0),
					"Syntax error -N: Must specify a movie prefix\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active,
					"Syntax error -T: Must specify number of frames or a time file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && !(Ctrl->Q.active || Ctrl->animate || Ctrl->M.active),
					"Syntax error -Z: Cannot be used without specifying a GIF (-A), master (-M) or movie (-F) product\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.skip && !(Ctrl->F.active[MOVIE_MP4] || Ctrl->F.active[MOVIE_WEBM]),
					"Syntax error -A: Cannot specify a GIF stride > 1 without selecting a movie product (-F)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && Ctrl->M.frame < Ctrl->T.start_frame,
					"Syntax error -M: Cannot specify a frame before the first frame number set via -T\n");
	
	if (n_errors) return (GMT_PARSE_ERROR);	/* No point going further */
	
	/* Note: We open script files for reading below since we are changing cwd later and sometimes need to rewind and re-read */
	
	if (n_files == 1) {	/* Determine scripting language from file extension and open the main script file */
		if (strstr (Ctrl->In.file, ".bash") || strstr (Ctrl->In.file, ".sh"))	/* Treat both as bash since sh is subset of bash */
			Ctrl->In.mode = BASH_MODE;
		else if (strstr (Ctrl->In.file, ".csh"))
			Ctrl->In.mode = CSH_MODE;
		else if (strstr (Ctrl->In.file, ".bat"))
			Ctrl->In.mode = DOS_MODE;
		else {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to determine script language from the extension of your script %s\n", Ctrl->In.file);
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Allowable extensions are: *.sh, *.bash, *.csh, and *.bat\n");
			n_errors++;
		}
		/* Armed with script language we check that any back/fore-ground scripts are of the same kind */
		for (k = MOVIE_PREFLIGHT; !n_errors && k <= MOVIE_POSTFLIGHT; k++) {
			if (!Ctrl->S[k].active) continue;	/* Not provided */
			n_errors += check_language (GMT, Ctrl->In.mode, Ctrl->S[k].file);
		}
		if (!n_errors && Ctrl->I.active) {	/* Must also check the include file, and open it for reading */
			n_errors += check_language (GMT, Ctrl->In.mode, Ctrl->I.file);
			if (n_errors == 0 && ((Ctrl->I.fp = fopen (Ctrl->I.file, "r")) == NULL)) {
				GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to open include script file %s\n", Ctrl->I.file);
				n_errors++;
			}
		}
		if (n_errors == 0 && ((Ctrl->In.fp = fopen (Ctrl->In.file, "r")) == NULL)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Unable to open main script file %s\n", Ctrl->In.file);
			n_errors++;
		}
		if (n_errors == 0 && script_is_classic (GMT, Ctrl->In.fp)) {
			GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Your main script file %s is not in GMT modern mode\n", Ctrl->In.file);
			n_errors++;
		}
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_movie (void *V_API, int mode, void *args) {
	int error = 0, precision;
	int (*run_script)(const char *);	/* pointer to system function or a dummy */
	
	unsigned int n_values = 0, n_frames = 0, frame, i_frame, col, p_width, p_height, k;
	unsigned int n_frames_not_started = 0, n_frames_completed = 0, first_frame = 0, n_cores_unused;
	
	bool done = false, layers = false, one_frame = false, has_text = false, is_classic = false;
	
	char *extension[3] = {"sh", "csh", "bat"}, *load[3] = {"source", "source", "call"}, *rmfile[3] = {"rm -f", "rm -f", "del"};
	char *rmdir[3] = {"rm -rf", "rm -rf", "rd /s /q"}, *export[3] = {"export ", "export ", ""};
	char *mvfile[3] = {"mv -f", "mv -rf", "move"}, *sc_call[3] = {"bash ", "csh ", "start /B"};
	char var_token[3] = "$$%", path_sep[3] = "::;";
	char init_file[GMT_LEN64] = {""}, state_tag[GMT_LEN16] = {""}, state_prefix[GMT_LEN64] = {""}, state_file[GMT_LEN64] = {""}, cwd[PATH_MAX] = {""};
	char pre_file[GMT_LEN64] = {""}, post_file[GMT_LEN64] = {""}, main_file[GMT_LEN64] = {""}, line[PATH_MAX] = {""};
	char string[GMT_LEN64] = {""}, extra[GMT_LEN64] = {""}, cmd[GMT_LEN256] = {""}, cleanup_file[GMT_LEN64] = {""}, L_txt[GMT_LEN64] = {""};
	char png_file[GMT_LEN64] = {""}, topdir[PATH_MAX] = {""}, datadir[PATH_MAX] = {""}, frame_products[GMT_LEN8] = {MOVIE_RASTER_FORMAT};
#ifdef _WIN32
	char dir_sep = '\\';
#else
	char dir_sep = '/';
#endif
	double percent, L_col;
	
	FILE *fp = NULL;
	
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_OPTION *options = NULL;
	struct MOVIE_STATUS *status = NULL;
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

	/*---------------------------- This is the movie main code ----------------------------*/

	/* Determine pixel dimensions of individual images */
	p_width =  urint (ceil (Ctrl->C.dim[GMT_X] * Ctrl->C.dim[GMT_Z]));
	p_height = urint (ceil (Ctrl->C.dim[GMT_Y] * Ctrl->C.dim[GMT_Z]));
	one_frame = (Ctrl->M.active && (Ctrl->Q.scripts || !Ctrl->animate || Ctrl->M.exit));	/* true if we want to create a single master plot only */
	if (Ctrl->C.unit == 'c') Ctrl->C.dim[GMT_Z] *= 2.54;		/* Since gs requires dots per inches but we were given dots per cm */
	else if (Ctrl->C.unit == 'p') Ctrl->C.dim[GMT_Z] *= 72.0;	/* Since gs requires dots per inches but we were given dots per point */
	
	if (Ctrl->L.active) {	/* Compute label placement */
		int i, j, justify = gmt_just_decode (GMT, Ctrl->L.placement, PSL_TL);
		i = (justify % 4) - 1;	/* Split the 2-D justify code into x just 0-2 */
		j = justify / 4;	/* Split the 2-D justify code into y just 0-2 */
		Ctrl->L.x = 0.5 * Ctrl->C.dim[GMT_X] * i;
		Ctrl->L.y = 0.5 * Ctrl->C.dim[GMT_Y] * j;
		if (i != 1) Ctrl->L.x += (1-i) * Ctrl->L.off[GMT_X];
		if (j != 1) Ctrl->L.y += (1-j) * Ctrl->L.off[GMT_Y];
		Ctrl->L.col--;	/* So 0 becomes -1 (INTMAX, actually), 1 becomes 0, etc */
		if (Ctrl->L.mode == MOVIE_LABEL_IS_COL_T && !strchr (Ctrl->L.format, 's')) {
			GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L: Using +f<format> with word variables requires a \'%%s\'-style format.\n");
			Return (GMT_PARSE_ERROR);
		}
	}
	
	if (Ctrl->Q.scripts) {	/* No movie, but scripts will be produced */
		GMT_Report (API, GMT_MSG_NORMAL, "Dry-run enabled - Movie scripts will be created and any pre/post scripts will be executed.\n");
		if (Ctrl->M.active) GMT_Report (API, GMT_MSG_NORMAL, "A single plot for frame %d will be create and named %s.%s\n", Ctrl->M.frame, Ctrl->N.prefix, Ctrl->M.format);
		run_script = dry_run_only;	/* This prevents the main frame loop from executing the script */
	}
	else {	/* Will run scripts and may even need to make a movie */
		run_script = system;	/* The standard system function */
	
		if (Ctrl->A.active) {	/* Ensure we have GraphicsMagick installed */
			sprintf (cmd, "gm version");
			if ((fp = popen (cmd, "r")) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "GraphicsMagick is not installed - cannot build animated GIF.\n");
				fclose (Ctrl->In.fp);
				Return (GMT_RUNTIME_ERROR);
			}
			else pclose (fp);
		}
		else if (Ctrl->F.active[MOVIE_MP4] || Ctrl->F.active[MOVIE_WEBM]) {	/* Ensure we have ffmpeg installed */
			sprintf (cmd, "ffmpeg -version");
			if ((fp = popen (cmd, "r")) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "ffmpeg is not installed - cannot build MP4 or WEbM movies.\n");
				fclose (Ctrl->In.fp);
				Return (GMT_RUNTIME_ERROR);
			}
			else {	/* OK, but check if width is odd or even */
				pclose (fp);
				if (p_width % 2)	/* Don't like odd pixel widths */
					GMT_Report (API, GMT_MSG_NORMAL, "Your frame width is an odd number of pixels (%u). This may not work with ffmpeg...\n", p_width);
			}
		}
	}
	
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Paper dimensions: Width = %g%c Height = %g%c\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Pixel dimensions: %u x %u\n", p_width, p_height);

	/* First try to read -T<timefile> in case it is prescribed directly (and before we change directory) */
	if (!gmt_access (GMT, Ctrl->T.file, R_OK)) {	/* A file by that name exists and is readable */
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->T.file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to read time file: %s - exiting\n", Ctrl->T.file);
			fclose (Ctrl->In.fp);
			Return (API->error);
		}
		if (D->n_segments > 1) {	/* We insist on a simple file structure with a single segment */
			GMT_Report (API, GMT_MSG_NORMAL, "Your time file %s has more than one segment - reformat first\n", Ctrl->T.file);
			fclose (Ctrl->In.fp);
			Return (API->error);
		}
		n_frames = (unsigned int)D->n_records;	/* Number of records means number of frames */
		n_values = (unsigned int)D->n_columns;	/* The number of per-frame parameters we need to place into the per-frame parameter files */
		has_text = (D && D->table[0]->segment[0]->text);	/* Trailing text present */
		if (n_frames == 0) {	/* So not good... */
			GMT_Report (API, GMT_MSG_NORMAL, "Your time file %s has no records - exiting\n", Ctrl->T.file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
	}
	
	/* Get full path to the current working directory */
	if (getcwd (topdir, PATH_MAX) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Unable to determine current working directory - exiting.\n");
		fclose (Ctrl->In.fp);
		Return (GMT_RUNTIME_ERROR);
	}
	
	/* Create a working directory which will house every local file and all subdirectories created */
	if (gmt_mkdir (Ctrl->N.prefix)) {
		GMT_Report (API, GMT_MSG_NORMAL, "An old directory named %s exists OR we were unable to create new working directory %s - exiting.\n", Ctrl->N.prefix, Ctrl->N.prefix);
		fclose (Ctrl->In.fp);
		Return (GMT_RUNTIME_ERROR);
	}
	/* Make this directory the current working directory */
	if (chdir (Ctrl->N.prefix)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to change directory to %s - exiting.\n", Ctrl->N.prefix);
		perror (Ctrl->N.prefix);
		fclose (Ctrl->In.fp);
		Return (GMT_RUNTIME_ERROR);
	}
	/* Get full path to this working directory */
	if (getcwd (cwd, PATH_MAX) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Unable to determine current working directory.\n");
		fclose (Ctrl->In.fp);
		Return (GMT_RUNTIME_ERROR);
	}

	/* We use DATADIR to include the top and working directory so any files we supply or create can be found while inside frame directory */
	if (GMT->session.DATADIR)	/* Prepend initial and subdir as new datadirs to the existing search list */
		sprintf (datadir, "%s%c%s%c%s", topdir, path_sep[Ctrl->In.mode], cwd, path_sep[Ctrl->In.mode], GMT->session.DATADIR);
	else	/* Set the initial and prefix subdirectory as data dirs */
		sprintf (datadir, "%s%c%s", topdir, path_sep[Ctrl->In.mode], cwd);
	if (Ctrl->W.active && strlen (Ctrl->W.dir) > 2) {	/* Also append a specific work directory with data files that we should search */
		char work_dir[PATH_MAX] = {""};
#ifdef WIN32
		if (Ctrl->W.dir[1] != ':') /* Not hard path */
#else
		if (Ctrl->W.dir[0] != '/') /* Not hard path */
#endif
			/* Prepend cwd to the given relative path and update Ctrl->D.dir */
			sprintf (work_dir, "%c%s%c%s", path_sep[Ctrl->In.mode], topdir, dir_sep, Ctrl->W.dir);
		else
			sprintf (work_dir, "%c%s", path_sep[Ctrl->In.mode], Ctrl->W.dir);
		strcat (datadir, work_dir);
	}
		
	/* Create the initialization file with settings common to all frames */
	
	sprintf (init_file, "movie_init.%s", extension[Ctrl->In.mode]);
	if ((fp = fopen (init_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create file %s - exiting\n", init_file);
		fclose (Ctrl->In.fp);
		Return (GMT_ERROR_ON_FOPEN);
	}
	
	sprintf (string, "Static parameters set for animation sequence %s", Ctrl->N.prefix);
	set_comment (fp, Ctrl->In.mode, string);
	set_dvalue (fp, Ctrl->In.mode, "MOVIE_WIDTH",  Ctrl->C.dim[GMT_X], Ctrl->C.unit);
	set_dvalue (fp, Ctrl->In.mode, "MOVIE_HEIGHT", Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
	set_dvalue (fp, Ctrl->In.mode, "MOVIE_DPU",    Ctrl->C.dim[GMT_Z], 0);
	set_dvalue (fp, Ctrl->In.mode, "MOVIE_RATE",   Ctrl->D.framerate, 0);
	if (Ctrl->I.active) {	/* Append contents of an include file */
		set_comment (fp, Ctrl->In.mode, "Static parameters set via user include file");
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->I.fp)) {	/* Read the include file and copy to init script with some exceptions */
			if (strstr (line, "gmt begin")) continue;	/* Skip gmt begin */
			if (strstr (line, "gmt end")) continue;		/* Skip gmt end */
			if (strstr (line, "#!/")) continue;		/* Skip any leading shell incantation */
			fprintf (fp, "%s", line);			/* Just copy the line as is */
		}
		fclose (Ctrl->I.fp);	/* Done reading the include script */
	}
	fclose (fp);	/* Done writing the init script */
	
	if (Ctrl->S[MOVIE_PREFLIGHT].active) {	/* Create the preflight script from the user's background script */
		/* The background script is allowed to be classic if no plot is generated */
		unsigned int rec = 0;
		is_classic = script_is_classic (GMT, Ctrl->S[MOVIE_PREFLIGHT].fp);
		sprintf (pre_file, "movie_preflight.%s", extension[Ctrl->In.mode]);
		if ((fp = fopen (pre_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create preflight script %s - exiting\n", pre_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
		set_comment (fp, Ctrl->In.mode, "Preflight script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a PPID since subshells may mess things up */
		if (Ctrl->In.mode == DOS_MODE)	/* Set PPID under Windows to 1 since we run this separately first */
			fprintf (fp, "set GMT_PPID=1\n");
		else	/* On UNIX we may use the script's PID as PPID */
			set_tvalue (fp, Ctrl->In.mode, "GMT_PPID", "$$");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->S[MOVIE_PREFLIGHT].fp)) {	/* Read the background script and copy to preflight script with some exceptions */
			if ((is_classic && rec == 0) || strstr (line, "gmt begin")) {	/* Need to insert gmt figure after this line (or as first line) in case a background plot will be made */
				fprintf (fp, "gmt begin\n");	/* To ensure there are no args here since we are using gmt figure instead */
				set_comment (fp, Ctrl->In.mode, "\tSet fixed background output ps name");
				fprintf (fp, "\tgmt figure movie_background ps\n");
				fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
				fprintf (fp, "\tgmt set DIR_DATA %s\n", datadir);
				if (is_classic)	/* Also write the current line since it was not a gmt begin line */
					fprintf (fp, "%s", line);
			}
			else if (!strstr (line, "#!/"))	/* Skip any leading shell incantation since already placed by set_script */
				fprintf (fp, "%s", line);	/* Just copy the line as is */
			rec++;
		}
		fclose (Ctrl->S[MOVIE_PREFLIGHT].fp);	/* Done reading the foreground script */
		if (is_classic)	/* Must close modern session explicitly */
			fprintf (fp, "gmt end\n");
		fclose (fp);	/* Done writing the preflight script */
#ifndef WIN32
		/* Set executable bit if not on Windows */
		if (chmod (pre_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to make preflight script %s executable - exiting.\n", pre_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
#endif
		/* Run the pre-flight now which may or may not create a <timefile> needed later via -T, as well as a background plot */
		if ((error = system (pre_file))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running preflight script %s returned error %d - exiting.\n", pre_file, error);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
	}
	
	/* Now we can complete the -T parsing since any given file has now been created by pre_file */
	
	if (n_frames == 0) {	/* Must check again for a file or decode the argument as a number */
		if (!gmt_access (GMT, Ctrl->T.file, R_OK)) {	/* A file by that name was indeed created by preflight and is now available */
			if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->T.file, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to read time file: %s - exiting\n", Ctrl->T.file);
				fclose (Ctrl->In.fp);
				Return (API->error);
			}
			if (D->n_segments > 1) {	/* We insist on a simple file structure with a single segment */
				GMT_Report (API, GMT_MSG_NORMAL, "Your time file %s has more than one segment - reformat first\n", Ctrl->T.file);
				fclose (Ctrl->In.fp);
				Return (API->error);
			}
			n_frames = (unsigned int)D->n_records;	/* Number of records means number of frames */
			n_values = (unsigned int)D->n_columns;	/* The number of per-frame parameters we need to place into the per-frame parameter files */
			has_text = (D && D->table[0]->segment[0]->text);	/* Trailing text present */
		}
		else	/* Just gave the number of frames (we hope, or we got a bad filename and atoi should return 0) */
			n_frames = atoi (Ctrl->T.file);
	}
	if (n_frames == 0) {	/* So not good... */
		GMT_Report (API, GMT_MSG_NORMAL, "No frames specified! - exiting.\n");
		fclose (Ctrl->In.fp);
		Return (GMT_RUNTIME_ERROR);
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Number of animation frames: %d\n", n_frames);
	if (Ctrl->T.precision)	/* Precision was prescribed */
		precision = Ctrl->T.precision;
	else	/* Compute from largest frame number */
		precision = irint (ceil (log10 ((double)(Ctrl->T.start_frame+n_frames))));	/* Width needed to hold largest frame number */
	
	if (Ctrl->S[MOVIE_POSTFLIGHT].active) {	/* Prepare the postflight script */
		is_classic = script_is_classic (GMT, Ctrl->S[MOVIE_POSTFLIGHT].fp);
		if (is_classic) {
			GMT_Report (API, GMT_MSG_NORMAL, "Your postflight file %s is not in GMT modern node - exiting\n", post_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
		sprintf (post_file, "movie_postflight.%s", extension[Ctrl->In.mode]);
		if ((fp = fopen (post_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create postflight file %s - exiting\n", post_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
		set_comment (fp, Ctrl->In.mode, "Postflight script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a PPID since subshells may mess things up */
		if (Ctrl->In.mode == DOS_MODE)	/* Set PPID under Windows to 1 since we run this separately */
			fprintf (fp, "set GMT_PPID=1\n");
		else	/* On UNIX we may use the script's PID as PPID */
			set_tvalue (fp, Ctrl->In.mode, "GMT_PPID", "$$");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->S[MOVIE_POSTFLIGHT].fp)) {	/* Read the foreground script and copy to postflight script with some exceptions */
			if (strstr (line, "gmt begin")) {	/* Need to insert gmt figure after this line */
				fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
				set_comment (fp, Ctrl->In.mode, "\tSet fixed foreground output ps name");
				fprintf (fp, "\tgmt figure movie_foreground ps\n");
				fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
				fprintf (fp, "\tgmt set DIR_DATA %s\n", datadir);
			}
			else if (!strstr (line, "#!/"))	/* Skip any leading shell incantation since already placed */
				fprintf (fp, "%s", line);	/* Just copy the line as is */
		}
		fclose (Ctrl->S[MOVIE_POSTFLIGHT].fp);	/* Done reading the foreground script */
		fclose (fp);	/* Done writing the postflight script */
#ifndef WIN32
		/* Set executable bit if not Windows */
		if (chmod (post_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to make postflight script %s executable - exiting\n", post_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
#endif
		/* Run post-flight now before dealing with the loop so the overlay exists */
		if ((error = run_script (post_file))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running postflight script %s returned error %d - exiting.\n", post_file, error);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
	}
	
	/* Create parameter include files, one for each frame */
	
	for (i_frame = 0; i_frame < n_frames; i_frame++) {
		frame = i_frame + Ctrl->T.start_frame;	/* Actual frame number */
		if (one_frame && frame != Ctrl->M.frame) continue;	/* Just doing a single frame for debugging */
		sprintf (state_tag, "%*.*d", precision, precision, frame);
		sprintf (state_prefix, "movie_params_%s", state_tag);
		sprintf (state_file, "%s.%s", state_prefix, extension[Ctrl->In.mode]);
		if ((fp = fopen (state_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create frame parameter file %s - exiting\n", state_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		sprintf (state_prefix, "Parameter file for frame %s", state_tag);
		set_comment (fp, Ctrl->In.mode, state_prefix);
		sprintf (state_prefix, "%s_%s", Ctrl->N.prefix, state_tag);
		set_ivalue (fp, Ctrl->In.mode, "MOVIE_FRAME", frame);		/* Current frame number */
		set_ivalue (fp, Ctrl->In.mode, "MOVIE_NFRAMES", n_frames);	/* Total frames */
		set_tvalue (fp, Ctrl->In.mode, "MOVIE_TAG", state_tag);		/* Current frame tag (formatted frame number) */
		set_tvalue (fp, Ctrl->In.mode, "MOVIE_NAME", state_prefix);	/* Current frame name prefix */
		for (col = 0; col < n_values; col++) {	/* Derive frame variables from <timefile> in each parameter file */
			sprintf (string, "MOVIE_COL%u", col+1);
			set_dvalue (fp, Ctrl->In.mode, string, D->table[0]->segment[0]->data[col][frame], 0);
			if (Ctrl->L.mode == MOVIE_LABEL_IS_COL_C && Ctrl->L.col == col) L_col = D->table[0]->segment[0]->data[col][frame];
		}
		if (has_text) {	/* Also place any string parameter as a single string variable */
			set_tvalue (fp, Ctrl->In.mode, "MOVIE_TEXT", D->table[0]->segment[0]->text[frame]);
			if (Ctrl->T.split) {	/* Also split the string into individual words MOVIE_WORD1, MOVIE_WORD2, etc. */
				char *word = NULL;
				col = 0;
				while ((word = strsep (&D->table[0]->segment[0]->text[frame], " \t")) != NULL) {
					if (*word != '\0') {	/* Skip empty strings */
						sprintf (string, "MOVIE_WORD%u", ++col);
						set_tvalue (fp, Ctrl->In.mode, string, word);
						if (Ctrl->L.mode == MOVIE_LABEL_IS_COL_T && Ctrl->L.col == col) strcpy (L_txt, word);
					}
				}
			}
		}
		if (Ctrl->L.active) {	/* Want to place a user label in a corner of the frame */
			char label[GMT_LEN256] = {""};
			/* Place x/y/just/clearance_x/clearance_Y/pen/fill/txt in MOVIE_LABEL_ARG */
			sprintf (label, "%g%c,%g%c,%s,%g,%g,%s,%s,", Ctrl->L.x, Ctrl->C.unit, Ctrl->L.y, Ctrl->C.unit,
				Ctrl->L.placement, Ctrl->L.clearance[GMT_X], Ctrl->L.clearance[GMT_Y], Ctrl->L.pen, Ctrl->L.fill);
			if (Ctrl->L.mode == MOVIE_LABEL_IS_FRAME) {
				if (Ctrl->L.format[0] && strchr (Ctrl->L.format, 'd'))	/* Set as integer */
					sprintf (string, Ctrl->L.format, (int)frame);
				else if (Ctrl->L.format[0])	/* Set as floating point */
					sprintf (string, Ctrl->L.format, (double)frame);
				else
					strcpy (string, state_tag);
			}
			else if (Ctrl->L.mode == MOVIE_LABEL_IS_COL_C) {	/* Format a number */
				if (Ctrl->L.format[0] && strchr (Ctrl->L.format, 'd'))	/* Set as integer */
					sprintf (string, Ctrl->L.format, (int)L_col);
				else if (Ctrl->L.format[0])	/* Set as floating point */
					sprintf (string, Ctrl->L.format, L_col);
				else	/* Uses FORMAT_FLOAT_MAP */
					sprintf (string, GMT->current.setting.format_float_map, L_col);
			}
			else if (Ctrl->L.mode == MOVIE_LABEL_IS_COL_T) {
				if (Ctrl->L.format[0])
					sprintf (string, Ctrl->L.format, L_txt);
				else
					strcpy (string, L_txt);
			}
			strcat (label, string);
			/* Set MOVIE_LABEL_ARG as exported variable. gmt figure will check for this and if found create gmt.movie in session directory */
			fprintf (fp, "%s", export[Ctrl->In.mode]);
			set_tvalue (fp, Ctrl->In.mode, "MOVIE_LABEL_ARG", label);
		}
		fclose (fp);	/* Done writing this parameter file */
	}

	if (Ctrl->M.active) {	/* Make the master frame plot */
		char master_file[GMT_LEN64] = {""};
		/* Because format may differ and name will differ we just make a special script for this job */
		sprintf (master_file, "movie_master.%s", extension[Ctrl->In.mode]);
		if ((fp = fopen (master_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create loop frame script file %s - exiting\n", master_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		if (Ctrl->G.active)	/* Want to set a fixed background canvas color - we do this via the psconvert -A option */
			sprintf (extra, "A+g%s", Ctrl->G.fill);
		if (!access ("movie_background.ps", R_OK)) {	/* Need to place a background layer first (which is in parent dir when loop script is run) */
#ifdef WIN32
			strcat (extra, ",Mb..\\movie_background.ps");
#else
			strcat (extra, ",Mb../movie_background.ps");
#endif
		}
		if (!access ("movie_foreground.ps", R_OK)) {	/* Need to append foreground layer at end (which is in parent dir when script is run) */
#ifdef WIN32
			strcat (extra, ",Mf..\\movie_foreground.ps");
#else
			strcat (extra, ",Mf../movie_foreground.ps");
#endif
		}
		if (Ctrl->H.active) {	/* Must pass the downscalefactor option to psconvert */
			sprintf (line, ",H%d", Ctrl->H.factor);
			strcat (extra, line);
		}
		set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
		set_comment (fp, Ctrl->In.mode, "Master frame loop script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a PPID since subshells may mess things up */
		if (Ctrl->In.mode == DOS_MODE)	/* Set PPID under Windows to be the frame number */
			fprintf (fp, "set GMT_PPID=%c1\n", var_token[Ctrl->In.mode]);
		else	/* On UNIX we use the script's PID as PPID */
			set_tvalue (fp, Ctrl->In.mode, "GMT_PPID", "$$");
		set_comment (fp, Ctrl->In.mode, "Include static and frame-specific parameters");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		fprintf (fp, "%s movie_params_%c1.%s\n", load[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Include the frame parameters */
		fprintf (fp, "mkdir master\n");	/* Make a temp directory for this frame */
		fprintf (fp, "cd master\n");		/* cd to the temp directory */
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->In.fp)) {	/* Read the mainscript and copy to loop script, with some exceptions */
			if (strstr (line, "gmt begin")) {	/* Need to insert a gmt figure call after this line */
				fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
				set_comment (fp, Ctrl->In.mode, "\tSet output name and plot conversion parameters");
				fprintf (fp, "\tgmt figure %s %s", Ctrl->N.prefix, Ctrl->M.format);
				fprintf (fp, " %s", extra);
				if (strstr(Ctrl->M.format,"pdf") || strstr(Ctrl->M.format,"eps") || strstr(Ctrl->M.format,"ps"))
					fprintf (fp, "\n");	/* No dpu needed */
				else
					fprintf (fp, ",E%s\n", place_var (Ctrl->In.mode, "MOVIE_DPU"));
				fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c DIR_DATA %s\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit, datadir);
			}
			else if (!strstr (line, "#!/"))		/* Skip any leading shell incantation since already placed */
				fprintf (fp, "%s", line);	/* Just copy the line as is */
		}
		rewind (Ctrl->In.fp);	/* Get ready for main_frame reading */
		set_comment (fp, Ctrl->In.mode, "Move master file up to top directory and cd up one level");
		fprintf (fp, "%s %s.%s ..%c..\n", mvfile[Ctrl->In.mode], Ctrl->N.prefix, Ctrl->M.format, dir_sep);	/* Move master plot up to parent dir */
		fprintf (fp, "cd ..\n");	/* cd up to prefix dir */
		set_comment (fp, Ctrl->In.mode, "Remove frame directory");
		if (!Ctrl->Q.active) fprintf (fp, "%s master\n", rmdir[Ctrl->In.mode]);	/* Remove the work dir and any files in it */
		fclose (fp);	/* Done writing loop script */

#ifndef WIN32
		/* Set executable bit if not Windows */
		if (chmod (master_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to make script %s executable - exiting\n", master_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
#endif
		sprintf (cmd, "%s %*.*d", master_file, precision, precision, Ctrl->M.frame);
		if ((error = run_script (cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running script %s returned error %d - exiting.\n", cmd, error);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Single master plot (frame %d) built: %s.%s\n", Ctrl->M.frame, Ctrl->N.prefix, Ctrl->M.format);
		if (!Ctrl->Q.active) {
			/* Delete the masterfile script */
			sprintf (line, "%s %s\n", rmfile[Ctrl->In.mode], master_file);	/* Delete the master_file script */
			if ((error = system (line))) {
				GMT_Report (API, GMT_MSG_NORMAL, "Deleting the master file script %s returned error %d.\n", master_file, error);
				fclose (Ctrl->In.fp);
				Return (GMT_RUNTIME_ERROR);
			}
		}
		if (Ctrl->M.exit) {	/* Well, we are done */
			fclose (Ctrl->In.fp);
			/* Cd back up to the parent directory */
			if (chdir ("..")) {	/* Should never happen but we do check */
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to change directory to parent directory - exiting.\n");
				perror (Ctrl->N.prefix);
				Return (GMT_RUNTIME_ERROR);
			}
			if (!Ctrl->Q.active) {
				sprintf (line, "%s %s\n", rmdir[Ctrl->In.mode], Ctrl->N.prefix);	/* Delete the entire directory */
				if ((error = system (line))) {
					GMT_Report (API, GMT_MSG_NORMAL, "Deleting the prefix directory %s returned error %d.\n", Ctrl->N.prefix, error);
					Return (GMT_RUNTIME_ERROR);
				}
			}
			Return (GMT_NOERROR);
		}
	}
	
	/* Now build the main loop script from the mainscript */
	if (Ctrl->Q.active) strcat (frame_products, MOVIE_DEBUG_FORMAT);	/* Want to save original PS file for devug */
	
	sprintf (main_file, "movie_frame.%s", extension[Ctrl->In.mode]);
	if ((fp = fopen (main_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create loop frame script file %s - exiting\n", main_file);
		fclose (Ctrl->In.fp);
		Return (GMT_ERROR_ON_FOPEN);
	}
	extra[0] = '\0';	/* Reset */
	if (Ctrl->G.active)	/* Want to set a fixed background canvas color - we do this via the psconvert -A option */
		sprintf (extra, "A+g%s", Ctrl->G.fill);
	if (!access ("movie_background.ps", R_OK)) {	/* Need to place a background layer first (which is in parent dir when loop script is run) */
#ifdef WIN32
		strcat (extra, ",Mb..\\movie_background.ps");
#else
		strcat (extra, ",Mb../movie_background.ps");
#endif
		layers = true;
	}
	if (!access ("movie_foreground.ps", R_OK)) {	/* Need to append foreground layer at end (which is in parent dir when script is run) */
#ifdef WIN32
		strcat (extra, ",Mf..\\movie_foreground.ps");
#else
		strcat (extra, ",Mf../movie_foreground.ps");
#endif
		layers = true;
	}
	if (Ctrl->H.active) {	/* Must pass the downscalefactor option to psconvert */
		char htxt[GMT_LEN16] = {""};
		sprintf (htxt, ",H%d", Ctrl->H.factor);
		strcat (extra, htxt);
	}
	set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
	set_comment (fp, Ctrl->In.mode, "Main frame loop script");
	fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a PPID since subshells may mess things up */
	if (Ctrl->In.mode == DOS_MODE)	/* Set PPID under Windows to be the frame number */
		fprintf (fp, "set GMT_PPID=%c1\n", var_token[Ctrl->In.mode]);
	else	/* On UNIX we use the script's PID as PPID */
		set_tvalue (fp, Ctrl->In.mode, "GMT_PPID", "$$");
	set_comment (fp, Ctrl->In.mode, "Include static and frame-specific parameters");
	fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
	fprintf (fp, "%s movie_params_%c1.%s\n", load[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Include the frame parameters */
	fprintf (fp, "mkdir %s\n", place_var (Ctrl->In.mode, "MOVIE_NAME"));	/* Make a temp directory for this frame */
	fprintf (fp, "cd %s\n", place_var (Ctrl->In.mode, "MOVIE_NAME"));		/* cd to the temp directory */
	while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->In.fp)) {	/* Read the mainscript and copy to loop script, with some exceptions */
		if (strstr (line, "gmt begin")) {	/* Need to insert a gmt figure call after this line */
			fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
			set_comment (fp, Ctrl->In.mode, "\tSet output PNG name and plot conversion parameters");
			fprintf (fp, "\tgmt figure %s %s", place_var (Ctrl->In.mode, "MOVIE_NAME"), frame_products);
			fprintf (fp, " E%s,%s\n", place_var (Ctrl->In.mode, "MOVIE_DPU"), extra);
			fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c DIR_DATA %s\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit, datadir);
		}
		else if (!strstr (line, "#!/"))		/* Skip any leading shell incantation since already placed */
			fprintf (fp, "%s", line);	/* Just copy the line as is */
	}
	fclose (Ctrl->In.fp);	/* Done reading the main script */
	set_comment (fp, Ctrl->In.mode, "Move PNG file up to parent directory and cd up one level");
	fprintf (fp, "%s %s.%s ..\n", mvfile[Ctrl->In.mode], place_var (Ctrl->In.mode, "MOVIE_NAME"), MOVIE_RASTER_FORMAT);	/* Move PNG plot up to parent dir */
	fprintf (fp, "cd ..\n");	/* cd up to parent dir */
	if (!Ctrl->Q.active) {	/* Delete evidence; otherwise we want to leave debug evidence when doing a single frame only */
		set_comment (fp, Ctrl->In.mode, "Remove frame directory and frame parameter file");
		fprintf (fp, "%s %s\n", rmdir[Ctrl->In.mode], place_var (Ctrl->In.mode, "MOVIE_NAME"));	/* Remove the work dir and any files in it */
		fprintf (fp, "%s movie_params_%c1.%s\n", rmfile[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Remove the parameter file for this frame */
	}
	if (Ctrl->In.mode == DOS_MODE)	/* This is crucial to the "start /B ..." statement below to ensure the process terminates */
		fprintf (fp, "exit\n");
	fclose (fp);	/* Done writing loop script */
	
#ifndef WIN32
	/* Set executable bit if not Windows */
	if (chmod (main_file, S_IRWXU)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to make script %s executable - exiting\n", main_file);
		Return (GMT_RUNTIME_ERROR);
	}
#endif
	
	if (Ctrl->Q.scripts) {	/* No animation sequence generated */
		Return (GMT_NOERROR);	/* We are done */
	}

	/* Finally, we can run all the frames in a controlled loop, launching new parallel jobs as cores become available */

	i_frame = first_frame = 0; n_frames_not_started = n_frames;
	frame = Ctrl->T.start_frame;
	n_cores_unused = MAX (1, GMT->common.x.n_threads - 1);			/* Remove one for the main movie module thread */
	status = gmt_M_memory (GMT, NULL, n_frames, struct MOVIE_STATUS);	/* Used to keep track of frame status */
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Build frames using %u cores\n", n_cores_unused);
	/* START PARALLEL EXECUTION OF FRAME SCRIPTS */
	while (!done) {	/* Keep running jobs until all frames have completed */
		while (n_frames_not_started && n_cores_unused) {	/* Launch new jobs if possible */
			sprintf (cmd, "%s %s %*.*d &", sc_call[Ctrl->In.mode], main_file, precision, precision, frame);

			GMT_Report (API, GMT_MSG_DEBUG, "Launch script for frame %*.*d\n", precision, precision, frame);
			if ((error = system (cmd))) {
				GMT_Report (API, GMT_MSG_NORMAL, "Running script %s returned error %d - aborting.\n", cmd, error);
				Return (GMT_RUNTIME_ERROR);
			}
			status[frame].started = true;	/* We have now launched this frame job */
			frame++;			/* Advance to next frame for next launch */
			i_frame++;			/* Advance to next frame for next launch */
			n_frames_not_started--;		/* One fewer frames remaining */
			n_cores_unused--;		/* This core is now busy */
		}
		gmt_sleep (MOVIE_WAIT_TO_CHECK);	/* Wait 0.01 second - then check for completion of the PNG images */
		for (k = first_frame; k < i_frame; k++) {	/* Only loop over the range of frames that we know are currently in play */
			if (status[k].completed) continue;	/* Already finished with this frame */
			if (!status[k].started) continue;	/* Not started this frame yet */
			/* Here we can check if the frame job has completed */
			sprintf (png_file, "%s_%*.*d.%s", Ctrl->N.prefix, precision, precision, Ctrl->T.start_frame+k, MOVIE_RASTER_FORMAT);
			if (access (png_file, F_OK)) continue;	/* Not found yet */
			n_frames_completed++;		/* One more frame completed */
			status[k].completed = true;	/* Flag this frame as completed */
			n_cores_unused++;		/* Free up the core */
			percent = 100.0 * n_frames_completed / n_frames;
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Frame %*.*d of %d completed [%5.1f %%]\n", precision, precision, k, n_frames, percent);
		}
		/* Adjust first_frame, if needed */
		while (first_frame < n_frames && status[first_frame].completed) first_frame++;
		if (n_frames_completed == n_frames) done = true;	/* All frames completed! */
	}
	/* END PARALLEL EXECUTION OF FRAME SCRIPTS */

	gmt_M_free (GMT, status);	/* Done with this structure array */
	
	/* Cd back up to the parent directory */
	if (chdir ("..")) {	/* Should never happen but we should check */
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to change directory to parent directory - exiting.\n");
		perror (Ctrl->N.prefix);
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->A.active) {	/* Want an anmated GIF */
		/* Set up system call to gm (which we know exists) */
		unsigned int delay = urint (100.0 / Ctrl->D.framerate);	/* Delay to nearest ~1/100 s */
		char files[GMT_LEN32] = {""};
		files[0] = '*';
		if (Ctrl->A.skip) {	/* Only use every stride file */
			if (Ctrl->A.stride == 2 || Ctrl->A.stride == 20 || Ctrl->A.stride == 200 || Ctrl->A.stride == 2000)
				strcat (files, "[02348]");
			else if (Ctrl->A.stride == 5 || Ctrl->A.stride == 50 || Ctrl->A.stride == 500 || Ctrl->A.stride == 5000)
				strcat (files, "[05]");
			else	/* 10, 100 etc */
				strcat (files, "[0]");
			if (Ctrl->A.stride > 1000)
				strcat (files, "000");
			else if (Ctrl->A.stride > 100)
				strcat (files, "00");
			else if (Ctrl->A.stride > 10)
				strcat (files, "0");
		}
		sprintf (cmd, "gm convert -delay %u -loop %u +dither %s%c%s_%s.%s %s.gif", delay, Ctrl->A.loops, Ctrl->N.prefix, dir_sep, Ctrl->N.prefix, files, MOVIE_RASTER_FORMAT, Ctrl->N.prefix);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Running: %s\n", cmd);
		if ((error = system (cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running GIF conversion returned error %d - exiting.\n", error);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "GIF animation built: %s.gif\n", Ctrl->N.prefix);
		if (Ctrl->A.skip) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "GIF animation reflects every %d frame only\n", Ctrl->A.stride);
	}
	if (Ctrl->F.active[MOVIE_MP4]) {
		/* Set up system call to ffmpeg (which we know exists) */
		if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG))
			sprintf (extra, "verbose");
		if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE))
			sprintf (extra, "warning");
		else if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE))
			sprintf (extra, "warning");
		else
			sprintf (extra, "quiet");
		sprintf (png_file, "%%0%dd", precision);
		sprintf (cmd, "ffmpeg -loglevel %s -f image2 -framerate %g -y -i \"%s%c%s_%s.%s\" -vcodec libx264 %s -pix_fmt yuv420p %s.mp4",
			extra, Ctrl->D.framerate, Ctrl->N.prefix, dir_sep, Ctrl->N.prefix, png_file, MOVIE_RASTER_FORMAT, (Ctrl->F.options[MOVIE_MP4]) ? Ctrl->F.options[MOVIE_MP4] : "", Ctrl->N.prefix);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Running: %s\n", cmd);
		if ((error = system (cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running ffmpeg conversion to MP4 returned error %d - exiting.\n", error);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "MP4 movie built: %s.mp4\n", Ctrl->N.prefix);
	}
	if (Ctrl->F.active[MOVIE_WEBM]) {
		/* Set up system call to ffmpeg (which we know exists) */
		if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG))
			sprintf (extra, "verbose");
		if (gmt_M_is_verbose (GMT, GMT_MSG_LONG_VERBOSE))
			sprintf (extra, "warning");
		else if (gmt_M_is_verbose (GMT, GMT_MSG_VERBOSE))
			sprintf (extra, "warning");
		else
			sprintf (extra, "quiet");
		sprintf (png_file, "%%0%dd", precision);
		sprintf (cmd, "ffmpeg -loglevel %s -f image2 -framerate %g -y -i \"%s%c%s_%s.%s\" -vcodec libvpx %s -pix_fmt yuv420p %s.webm",
			extra, Ctrl->D.framerate, Ctrl->N.prefix, dir_sep, Ctrl->N.prefix, png_file, MOVIE_RASTER_FORMAT, (Ctrl->F.options[MOVIE_WEBM]) ? Ctrl->F.options[MOVIE_WEBM] : "", Ctrl->N.prefix);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Running: %s\n", cmd);
		if ((error = system (cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running ffmpeg conversion to webM returned error %d - exiting.\n", error);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "WebM movie built: %s.webm\n", Ctrl->N.prefix);
	}

	/* Prepare the cleanup script */
	sprintf (cleanup_file, "movie_cleanup.%s", extension[Ctrl->In.mode]);
	if ((fp = fopen (cleanup_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create cleanup file %s - exiting\n", cleanup_file);
		Return (GMT_ERROR_ON_FOPEN);
	}
	set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
	if (Ctrl->Z.active) {	/* Want to delete the entire frame directory */
		set_comment (fp, Ctrl->In.mode, "Cleanup script removes directory with frame files");
		fprintf (fp, "%s %s\n", rmdir[Ctrl->In.mode], Ctrl->N.prefix);	/* Delete the entire directory with PNG frames and tmp files */
	}
	else {	/* Just delete the remaining scripts and PS files */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%u frame PNG files saved in directory: %s\n", n_frames, Ctrl->N.prefix);
		if (Ctrl->S[MOVIE_PREFLIGHT].active)	/* Remove the preflight script */
			fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], Ctrl->N.prefix, dir_sep, pre_file);
		if (Ctrl->S[MOVIE_POSTFLIGHT].active)	/* Remove the postflight script */
			fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], Ctrl->N.prefix, dir_sep, post_file);
		fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], Ctrl->N.prefix, dir_sep, init_file);	/* Delete the init script */
		fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], Ctrl->N.prefix, dir_sep, main_file);	/* Delete the main script */
		if (layers) 
			fprintf (fp, "%s %s%c*.ps\n", rmfile[Ctrl->In.mode], Ctrl->N.prefix, dir_sep);	/* Delete any PostScript layers */
	}
	fclose (fp);
#ifndef _WIN32
	/* Set executable bit if not on Windows */
	if (chmod (cleanup_file, S_IRWXU)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to make cleanup script %s executable - exiting\n", cleanup_file);
		Return (GMT_RUNTIME_ERROR);
	}
#endif
	if (!Ctrl->Q.active) {
		/* Run cleanup script at the end */
		if (Ctrl->In.mode == DOS_MODE)
			error = system (cleanup_file);
		else {
			sprintf (cmd, "%s %s", sc_call[Ctrl->In.mode], cleanup_file);
			error = system (cmd);
		}
	}
	if (error) {
		GMT_Report (API, GMT_MSG_NORMAL, "Running cleanup script %s returned error %d - exiting.\n", cleanup_file, error);
		Return (GMT_RUNTIME_ERROR);
	}

	/* Finally, delete the clean-up script separately since under DOS we got complaints when we had it delete itself (which works under *nix) */
	if (!Ctrl->Q.active) {
		sprintf (line, "%s %s\n", rmfile[Ctrl->In.mode], cleanup_file);	/* Delete the cleanup script itself */
		if ((error = system (line))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Deleting the cleanup script %s returned error %d.\n", cleanup_file, error);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	Return (GMT_NOERROR);
}
