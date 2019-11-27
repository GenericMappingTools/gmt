/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2019 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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

#define THIS_MODULE_CLASSIC_NAME	"movie"
#define THIS_MODULE_MODERN_NAME	"movie"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Create animation sequences and movies"
#define THIS_MODULE_KEYS	"<T("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"-Vf"

#define MOVIE_PREFLIGHT		0
#define MOVIE_POSTFLIGHT	1

#define MOVIE_WAIT_TO_CHECK	10000	/* In microseconds, so 0.01 seconds */

#define MOVIE_RASTER_FORMAT	"png"	/* Lossless transparent raster format */
#define MOVIE_DEBUG_FORMAT	",ps"	/* Comma is intentional since we append to a list of formats */

enum enum_script {BASH_MODE = 0,	/* Write Bash script */
	CSH_MODE,			/* Write C-shell script */
	DOS_MODE};			/* Write DOS script */

enum enum_video {MOVIE_MP4,	/* Create a H.264 MP4 video */
	MOVIE_WEBM,		/* Create a WebM video */
	MOVIE_N_FORMATS};	/* Number of video formats above */

enum enum_label {MOVIE_LABEL_IS_FRAME = 1,
	MOVIE_LABEL_IS_ELAPSED,
	MOVIE_LABEL_IS_COL_C,
	MOVIE_LABEL_IS_COL_T};

/* Control structure for movie */

struct MOVIE_CTRL {
	bool animate;	/* True if we are making any animated product (GIF, movies) */
	struct MOVIE_In {	/* mainscript (Bourne, Bourne Again, csh, or DOS (bat) script) */
		bool active;
		enum enum_script mode;
		char *file;
		FILE *fp;
	} In;
	struct MOVIE_A {	/* -A[+l[<nloops>]][+s<stride>]  */
		bool active;
		bool loop;
		bool skip;
		unsigned int loops;
		unsigned int stride;
	} A;
	struct MOVIE_C {	/* -C<namedcanvas>|<canvas_and_dpu> */
		bool active;
		double dim[3];
		char unit;
		char *string;
	} C;
	struct MOVIE_D {	/* -D<displayrate> */
		bool active;
		double framerate;
	} D;
	struct MOVIE_F {	/* -F<videoformat>[+o<options>] */
		bool active[MOVIE_N_FORMATS];
		char *format[MOVIE_N_FORMATS];
		char *options[MOVIE_N_FORMATS];
	} F;
	struct MOVIE_G {	/* -G<canvasfill> */
		bool active;
		char *fill;
	} G;
	struct MOVIE_H {	/* -H<factor> */
		bool active;
		int factor;
	} H;
	struct MOVIE_I {	/* -I<includefile> */
		bool active;
		char *file;
		FILE *fp;
	} I;
	struct MOVIE_L {	/* Repeatable: -L[e|f|c#|t#][+c<clearance>][+f<font>][+g<fill>][+j<justify>][+o<offset>][+p<pen>][+t<fmt>][+s<scl>] */
		bool active;
		unsigned int n_tags;
		struct MOVIE_TAG {
			struct GMT_FONT font;
			char format[GMT_LEN64];
			char fill[GMT_LEN64];
			char pen[GMT_LEN64];
			char placement[3];
			unsigned int mode;
			unsigned int col;
			unsigned int ne;	/* Number of elements in elapsed format */
			double scale;
			double x, y;
			double off[2];
			double clearance[2];
		} tag[GMT_LEN32];
	} L;
	struct MOVIE_M {	/* -M[<frame>][,format] */
		bool active;
		bool exit;
		unsigned int frame;
		char *format;
	} M;
	struct MOVIE_N {	/* -N<movieprefix> */
		bool active;
		char *prefix;
	} N;
	struct MOVIE_Q {	/* -Q[s] */
		bool active;
		bool scripts;
	} Q;
	struct MOVIE_S {	/* -Sb|f<script> */
		bool active;
		char *file;
		FILE *fp;
	} S[2];
	struct MOVIE_T {	/* -T<n_frames>|<timefile>[+p<precision>][+s<frame>][+w] */
		bool active;
		bool split;
		unsigned int n_frames;
		unsigned int start_frame;
		unsigned int precision;
		char *file;
	} T;
	struct MOVIE_W {	/* -W<workingdirectory> */
		bool active;
		char *dir;
	} W;
	struct MOVIE_Z {	/* -Z */
		bool active;
	} Z;
	struct MOVIE_x {	/* -x[[-]<ncores>] */
		bool active;
		int n_threads;
	} x;
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
	C->x.n_threads = GMT->parent->n_cores;	/* Use all cores available unless -x is set */
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

/*! -x[[-]<ncores>] parsing needed but here not related to OpenMP etc - it is just a local option */
GMT_LOCAL int parse_x_option (struct GMT_CTRL *GMT, struct MOVIE_CTRL *Ctrl, char *arg) {
	if (!arg) return (GMT_PARSE_ERROR);	/* -x requires a non-NULL argument */
	if (arg[0])
		Ctrl->x.n_threads = atoi (arg);

	if (Ctrl->x.n_threads == 0)	/* Not having any of that.  At least one */
		Ctrl->x.n_threads = 1;
	else if (Ctrl->x.n_threads < 0)	/* Meant to reduce the number of threads */
		Ctrl->x.n_threads = MAX(GMT->parent->n_cores + Ctrl->x.n_threads, 1);		/* Max-n but at least one */
	return (GMT_NOERROR);
}

GMT_LOCAL int gmt_sleep (unsigned int microsec) {
	/* Waiting before checking if the PNG has been completed */
#ifdef WIN32
	Sleep ((uint32_t)microsec/1000);	/* msec are microseconds but Sleep wants millisecs */
	return 0;
#else
	return (usleep ((useconds_t)microsec));
#endif
}

GMT_LOCAL void set_value (struct GMT_CTRL *GMT, FILE *fp, int mode, int col, char *name, double value) {
	/* Assigns a single named data floating point variable given the script mode
	 * Here, col indicates which input column in case special formatting is implied via -f */
	char string[GMT_LEN64] = {""};
	gmt_ascii_format_one (GMT, string, value, gmt_M_type (GMT, GMT_IN, col));
	switch (mode) {
		case BASH_MODE: fprintf (fp, "%s=%s", name, string);       break;
		case CSH_MODE:  fprintf (fp, "set %s = %s", name, string); break;
		case DOS_MODE:  fprintf (fp, "set %s=%s", name, string);   break;
	}
	fprintf (fp, "\n");
}

GMT_LOCAL void set_dvalue (FILE *fp, int mode, char *name, double value, char unit) {
	/* Assigns a single named Cartesian floating point variable given the script mode */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "%s=%.12g", name, value);       break;
		case CSH_MODE:  fprintf (fp, "set %s = %.12g", name, value); break;
		case DOS_MODE:  fprintf (fp, "set %s=%.12g", name, value);   break;
	}
	if (unit) fprintf (fp, "%c", unit);	/* Append the unit [c|i|p] unless 0 */
	fprintf (fp, "\n");
}

GMT_LOCAL void set_ivalue (FILE *fp, int mode, bool env, char *name, int value) {
	/* Assigns a single named integer variable given the script mode */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "%s=%d\n", name, value);       break;
		case CSH_MODE:  if (env)
					fprintf (fp, "%s %d\n", name, value);
				else
					fprintf (fp, "set %s = %d\n", name, value);
				break;
		case DOS_MODE:  fprintf (fp, "set %s=%d\n", name, value);   break;
	}
}

GMT_LOCAL void set_tvalue (FILE *fp, int mode, bool env, char *name, char *value) {
	/* Assigns a single named text variable given the script mode */
	if (strchr (value, ' ') || strchr (value, '\t')) {	/* String has spaces or tabs */
		switch (mode) {
			case BASH_MODE: fprintf (fp, "%s=\"%s\"\n", name, value);       break;
			case CSH_MODE:  if (env)
						fprintf (fp, "%s \"%s\"\n", name, value);
					else
						fprintf (fp, "set %s = \"%s\"\n", name, value);
					break;
			case DOS_MODE:  fprintf (fp, "set %s=\"%s\"\n", name, value);   break;
		}
	}
	else {	/* Single word */
		switch (mode) {
			case BASH_MODE: fprintf (fp, "%s=%s\n", name, value);       break;
			case CSH_MODE:  if (env)
						fprintf (fp, "%s %s\n", name, value);
					else
						fprintf (fp, "set %s = %s\n", name, value);
					break;
			case DOS_MODE:  fprintf (fp, "set %s=%s\n", name, value);   break;
		}
	}
}

GMT_LOCAL void set_script (FILE *fp, int mode) {
	/* Writes the script's incantation line (or a comment for DOS, turning off default echo) */
	switch (mode) {
		case BASH_MODE: fprintf (fp, "#!/usr/bin/env bash\n"); break;
		case CSH_MODE:  fprintf (fp, "#!/usr/bin/env csh\n"); break;
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
	bool modern = false;
	char line[PATH_MAX] = {""};
	while (!modern && gmt_fgets (GMT, line, PATH_MAX, fp)) {
		if (strstr (line, "gmt ") == NULL) continue;	/* Does not start with gmt */
		if (strstr (line, " begin"))		/* A modern mode script */
			modern = true;
		else if (strstr (line, " figure"))	/* A modern mode script */
			modern = true;
		else if (strstr (line, " subplot"))	/* A modern mode script */
			modern = true;
		else if (strstr (line, " inset"))	/* A modern mode script */
			modern = true;
		else if (strstr (line, " end"))		/* A modern mode script */
			modern = true;
	}
	rewind (fp);	/* Go back to beginning of file */
	return (!modern);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <mainscript> -C<canvas> -N<prefix> -T<nframes>|<timefile>[+p<width>][+s<first>][+w]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-A[+l[<n>]][+s<stride>]] [-D<rate>] [-F<format>[+o<opts>]] [-G<fill>] [-H<factor>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-I<includefile>] [-L<labelinfo>] [-M[<frame>,][<format>]] [-Q[s]] [-Sb<script>] [-Sf<script>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [-W<workdir>] [-Z] [%s] [-x[[-]<n>]] [%s]\n\n", GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

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
	GMT_Message (API, GMT_TIME_NONE, "\t   Note: UHD-2 and 8k can be used for 4320p, UHD and 4k for 2160p, and HD for 1080p.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Recognized 4:3-ratio formats:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      name:	pixel size:   canvas size (SI):	 canvas size (US):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      UXGA:	1600 x 1200	24 x 18 cm	 9.6 x 7.2 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     SXGA+:	1400 x 1050	24 x 18 cm	 9.6 x 7.2 inch\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       XGA:	1024 x  768	24 x 18 cm	 9.6 x 7.2 inch\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +w to <timefile> to have trailing text be split into individual word variables.\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t-H Temporarily increase <dpu> by <factor>, rasterize, then downsample [no downsampling].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Stabilizes sub-pixel changes between frames, such as moving text and lines.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Include a script file to be inserted into the movie_init.sh script [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Used to add constant variables needed by all movie scripts.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Automatic labeling of frames; repeatable (max 32).  Places chosen label at the frame perimeter:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     e selects elapsed time as the label. Use +s<scl> to set time in sec per frame [1/<framerate>].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     f selects the running frame number as the label.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     c<col> uses the value in column <col> of <timefile> (first column is 0).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     t<col> uses word number <col> from the trailing text in <timefile> (requires -T...+w; first word is 0).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +c<dx>[/<dy>] for the clearance between label and surrounding box.  Only\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     used if +g or +p are set.  Append units {%s} or %% of fontsize [%d%%].\n", GMT_DIM_UNITS_DISPLAY, GMT_TEXT_CLEARANCE);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +f[<fontinfo>] to set the size, font, and optionally the label color [%s].\n",
		gmt_putfont (API->GMT, &API->GMT->current.setting.font_tag));
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +g to fill the label textbox with <fill> color [no fill].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use +j<refpoint> to specify where the label should be plotted [TL].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +o<dx>[/<dy>] to offset label in direction implied by <justify> [%d%% of font size].\n", GMT_TEXT_OFFSET);
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p to draw the outline of the textbox using selected pen [no outline].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +t to provide a C-format statement to be used with the item selected [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Create a master frame plot as well; append comma-separated frame number [0] and format [pdf].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Master plot will be named <prefix>.<format> and placed in the current directory.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Debugging: Leave all intermediate files and directories behind for inspection.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append s to only create the work scripts but none will be executed (except for background script).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Given names for the optional background and foreground GMT scripts [none]:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sb Append name of background GMT modern script that may pre-compute\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       files needed by <mainscript> and/or build a static background plot layer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       If a plot is generated then the script must be in GMT modern mode.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Sf Append name of foreground GMT modern mode script which will\n");
	GMT_Message (API, GMT_TIME_NONE, "\t       build a static foreground plot overlay appended to all frames.\n");
	GMT_Option (API, "V");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Give <workdir> where temporary files will be built [<workdir> = <prefix> set by -N].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Erase directory <prefix> after converting to movie [leave directory with PNGs alone].\n");
	GMT_Option (API, "f");
	/* Number of threads (repurposed from -x in GMT_Option since this local option is always available and not using OpenMP) */
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

	unsigned int n_errors = 0, n_files = 0, k, pos, mag, T;
	int n;
	char txt_a[GMT_LEN32] = {""}, txt_b[GMT_LEN32] = {""}, arg[GMT_LEN64] = {""}, p[GMT_LEN256] = {""};
	char *c = NULL, *s = NULL, string[GMT_LEN128] = {""};
	double width = 24.0, height16x9 = 13.5, height4x3 = 18.0, dpu = 160.0;	/* SI values for dimensions and dpu */
	struct GMT_FILL fill;	/* Only used to make sure any fill is given with correct syntax */
	struct GMT_PEN pen;	/* Only used to make sure any pen is given with correct syntax */
	struct GMT_OPTION *opt = NULL;
	
	if (GMT->current.setting.proj_length_unit == GMT_INCH) {	/* Switch from SI to US dimensions in inches given format names */
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
				strncpy (arg, opt->arg, GMT_LEN64-1);	/* Get a copy... */
				gmt_str_tolower (arg);		/* ..so we can make it lower case */
				/* 16x9 formats */
				if (!strcmp (arg, "8k") || !strcmp (arg, "uhd-2") || !strcmp (arg, "4320p")) {	/* 4320x7680 */
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
					else {	/* Got three items; let's check */
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
				if ((c = strstr (opt->arg, "+o"))) {	/* Gave encoding options */
					s = &c[2];	/* Retain start of encoding options for later */
					c[0] = '\0';	/* Chop off options */
				}
				else
					s = NULL;	/* No encoding options given */
				strncpy (arg, opt->arg, GMT_LEN64-1);	/* Get a copy of the args (minus encoding options)... */
				gmt_str_tolower (arg);	/* ..so we can convert it to lower case for comparisons */
				if (c) c[0] = '+';	/* Now we can restore the optional text we chopped off */
				if (!strcmp (opt->arg, "none")) {	/* Do not make those PNGs at all, just a master plot */
					Ctrl->M.exit = true;
					break;
				}
				else if (!strcmp (opt->arg, "mp4"))	/* Make a MP4 movie */
					k = MOVIE_MP4;
				else if (!strcmp (opt->arg, "webm"))	/* Make a WebM movie */
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
				if ((T = Ctrl->L.n_tags) == GMT_LEN32) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -L: Cannot handle more than %d tags\n", GMT_LEN32);
					n_errors++;
					break;
				}
				Ctrl->L.tag[T].fill[0] = Ctrl->L.tag[T].pen[0] = '-';	/* No fill or outline */
				Ctrl->L.tag[T].off[GMT_X] = Ctrl->L.tag[T].off[GMT_Y] = 0.01 * GMT_TEXT_OFFSET * GMT->current.setting.font_tag.size / PSL_POINTS_PER_INCH; /* 20% */
				Ctrl->L.tag[T].clearance[GMT_X] = Ctrl->L.tag[T].clearance[GMT_Y] = 0.01 * GMT_TEXT_CLEARANCE * GMT->current.setting.font_tag.size / PSL_POINTS_PER_INCH;	/* 15% */
				sprintf (Ctrl->L.tag[T].placement, "TL");
				gmt_M_memcpy (&(Ctrl->L.tag[T].font), &(GMT->current.setting.font_tag), 1, struct GMT_FONT);
				if ((c = strchr (opt->arg, '+'))) c[0] = '\0';	/* Chop off modifiers for now */
				switch (opt->arg[0]) {
					case 'c':	Ctrl->L.tag[T].mode = MOVIE_LABEL_IS_COL_C;	Ctrl->L.tag[T].col = atoi (&opt->arg[1]);	break;
					case 't':	Ctrl->L.tag[T].mode = MOVIE_LABEL_IS_COL_T;	Ctrl->L.tag[T].col = atoi (&opt->arg[1]);	break;
					case 'e':	Ctrl->L.tag[T].mode = MOVIE_LABEL_IS_ELAPSED;	break;
					case 'f': case '\0': Ctrl->L.tag[T].mode = MOVIE_LABEL_IS_FRAME;	break;	/* Frame number is default */
					default:	/* Not recognized */
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -L: Select -Lf, -Lc<col> or -Lt<col>\n");
						n_errors++;
						break;
				}
				if (c) {	/* Gave modifiers so we must parse those now */
					c[0] = '+';	/* Restore modifiers */
					/* Modifiers are [+c<dx/dy>][+f<fmt>][+g<fill>][+j<justify>][+o<dx/dy>][+p<pen>][+s<scl>][+t<fmt>] */
					if (gmt_validate_modifiers (GMT, opt->arg, 'L', "cfgjopst")) n_errors++;
					if (gmt_get_modifier (&opt->arg[1], 'c', string) && string[0])	/* Clearance for box */
						if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->L.tag[T].clearance) < 0) n_errors++;
					if (gmt_get_modifier (&opt->arg[1], 'f', string)) {	/* Gave a separate font */
						if (!string[0]) {
							GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -L: +f not given any font\n");
							n_errors++;
						}
						else
							n_errors += gmt_getfont (GMT, string, &(Ctrl->L.tag[T].font));
					}
					if (gmt_get_modifier (&opt->arg[1], 'g', Ctrl->L.tag[T].fill) && Ctrl->L.tag[T].fill[0]) {
						if (gmt_getfill (GMT, Ctrl->L.tag[T].fill, &fill)) n_errors++;
					}
					if (gmt_get_modifier (&opt->arg[1], 'j', Ctrl->L.tag[T].placement)) {	/* Inside placement */
						gmt_just_validate (GMT, Ctrl->L.tag[T].placement, "TL");
					}
					if (gmt_get_modifier (&opt->arg[1], 'o', string))	/* Offset refpoint */
						if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, Ctrl->L.tag[T].off) < 0) n_errors++;
					if (gmt_get_modifier (&opt->arg[1], 'p', Ctrl->L.tag[T].pen) && Ctrl->L.tag[T].pen[0]) {
						if (gmt_getpen (GMT, Ctrl->L.tag[T].pen, &pen)) n_errors++;
					}
					if (Ctrl->L.tag[T].mode == MOVIE_LABEL_IS_ELAPSED && gmt_get_modifier (&opt->arg[1], 's', string)) {	/* Gave frame time length */
						Ctrl->L.tag[T].scale = atof (string);
					}
					if (gmt_get_modifier (&opt->arg[1], 't', Ctrl->L.tag[T].format) && !Ctrl->L.tag[T].format[0]) {
						GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Syntax error option -L: +t not given any format\n");
						n_errors++;
					}
				}
				Ctrl->L.n_tags++;
				break;

			case 'M':	/* Create a single frame plot as well as movie (unless -Q is active) */
				Ctrl->M.active = true;
				if ((c = strchr (opt->arg, ',')) ) {	/* Gave frame and format */
					Ctrl->M.format = strdup (&c[1]);
					c[0] = '\0';	/* Chop off format */
					Ctrl->M.frame = atoi (opt->arg);
					c[0] = ',';	/* Restore format */
				}
				else if (isdigit (opt->arg[0])) {	/* Gave just a frame, default to PDF format */
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
			
			case 'Q':	/* Debug - leave temp files and directories behind; Use -Qs to only write scripts */
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
							case 'p':	/* Set a fixed precision in frame naming ###### */
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

			case 'x':
				n_errors += parse_x_option (GMT, Ctrl, opt->arg);
				Ctrl->x.active = true;
				break;
		
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
	if (!Ctrl->T.split) {
		for (T = 0; T < Ctrl->L.n_tags; T++)
			n_errors += gmt_M_check_condition (GMT, Ctrl->L.tag[T].mode == MOVIE_LABEL_IS_COL_T, "Syntax error -L: Must split trailing text to use -Lt\n");
	}
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
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->W.active && !strcmp (Ctrl->W.dir, "/tmp"),
					"Syntax error option -Z: Cannot delete working directory %s\n", Ctrl->W.dir);

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
		/* Open the main script for reading here */
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

GMT_LOCAL void close_files (struct MOVIE_CTRL *Ctrl) {
	/* Close all files when an error forces us to quit */
	fclose (Ctrl->In.fp);
	if (Ctrl->I.active) fclose (Ctrl->I.fp);
}

GMT_LOCAL bool line_is_a_comment (char *line) {
	unsigned int k = 0;
	while (line[k] && isspace (line[k])) k++;	/* Wind past leading whitespace */
	return (line[k] == '#' || !strncasecmp (&line[k], "rem", 3U)) ? true : false;	/* Will return true for lines starting with some tabs, then comment */
}

GMT_LOCAL bool is_gmt_module (char *line, char *module) {
	/* Robustly identify the command "gmt begin" */
	char word[GMT_LEN128] = {""};
	unsigned int pos = 0;
	size_t L;
	if (strlen (line) >= GMT_LEN128) return false;	/* Cannot be gmt begin */
	/* To handle cases where there may be more than one space between gmt and module */
	if (line_is_a_comment (line)) return false;	/* Skip commented lines like "  # anything" */
	if (gmt_strtok (line, " \t\n", &pos, word) == 0) return false;	/* Get first word in the command or fail */
	if (strcmp (word, "gmt")) return false;		/* Not starting with gmt so we are done */
	if (gmt_strtok (line, " \t\n", &pos, word) == 0) return false;	/* Get second word or fail */
	L = strlen (module);				/* How many characters to compare against */
	if (!strncmp (word, module, L)) return true;	/* Command starting with gmt <module> found */
	return false;	/* Not gmt <module> */
}

GMT_LOCAL bool is_gmt_end_show (char *line) {
	char word[GMT_LEN128] = {""};
	unsigned int pos = 0;
	if (strlen (line) >= GMT_LEN128) return false;	/* Cannot be gmt end show */
	/* Robustly identify the command "gmt end show" */
	/* To handle cases where there may be more than one space between gmt and module */
	if (line_is_a_comment (line)) return false;	/* Skip commented lines like "  # anything" */
	if (gmt_strtok (line, " \t\n", &pos, word) == 0) return false;	/* Get first word in the command or fail */
	if (strcmp (word, "gmt")) return false;		/* Not starting with gmt so we are done */
	if (gmt_strtok (line, " \t\n", &pos, word) == 0) return false;	/* Get second word or fail */
	if (strcmp (word, "end")) return false;		/* Not continuing with end so we are done */
	if (gmt_strtok (line, " \t\n", &pos, word) == 0) return false;	/* Get third word or fail */
	if (!strcmp (word, "show")) return true;	/* Yes, found gmt end show */
	return false;	/* Not gmt end show */
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_movie (void *V_API, int mode, void *args) {
	int error = 0, precision;
	int (*run_script)(const char *);	/* pointer to system function or a dummy */
	
	unsigned int n_values = 0, n_frames = 0, frame, i_frame, col, p_width, p_height, k, T;
	unsigned int n_frames_not_started = 0, n_frames_completed = 0, first_frame = 0, n_cores_unused;
	unsigned int dd, hh, mm, ss, flavor = 0;
	
	bool done = false, layers = false, one_frame = false, has_text = false, is_classic = false, upper_case = false;
	bool n_written = false;
	
	char *extension[3] = {"sh", "csh", "bat"}, *load[3] = {"source", "source", "call"}, *rmfile[3] = {"rm -f", "rm -f", "del"};
	char *rmdir[3] = {"rm -rf", "rm -rf", "rd /s /q"}, *export[3] = {"export ", "setenv ", ""};
	char *mvfile[3] = {"mv -f", "mv -f", "move"}, *sc_call[3] = {"bash ", "csh ", "start /B"};
	char var_token[4] = "$$%";
	char init_file[PATH_MAX] = {""}, state_tag[GMT_LEN16] = {""}, state_prefix[GMT_LEN64] = {""}, param_file[PATH_MAX] = {""}, cwd[PATH_MAX] = {""};
	char pre_file[PATH_MAX] = {""}, post_file[PATH_MAX] = {""}, main_file[PATH_MAX] = {""}, line[PATH_MAX] = {""}, version[GMT_LEN32] = {""};
	char string[GMT_LEN128] = {""}, extra[GMT_LEN256] = {""}, cmd[GMT_LEN256] = {""}, cleanup_file[PATH_MAX] = {""}, L_txt[GMT_LEN128] = {""};
	char png_file[PATH_MAX] = {""}, topdir[PATH_MAX] = {""}, workdir[PATH_MAX] = {""}, datadir[PATH_MAX] = {""}, frame_products[GMT_LEN32] = {MOVIE_RASTER_FORMAT};
	char dir_sep = '/';
	double percent = 0.0, L_col = 0;
	
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

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the movie main code ----------------------------*/

	/* Determine pixel dimensions of individual images */
	p_width =  urint (ceil (Ctrl->C.dim[GMT_X] * Ctrl->C.dim[GMT_Z]));
	p_height = urint (ceil (Ctrl->C.dim[GMT_Y] * Ctrl->C.dim[GMT_Z]));
	one_frame = (Ctrl->M.active && (!Ctrl->animate || Ctrl->M.exit));	/* true if we want to create a single master plot only */
	if (Ctrl->C.unit == 'c') Ctrl->C.dim[GMT_Z] *= 2.54;		/* Since gs requires dots per inch but we gave dots per cm */
	else if (Ctrl->C.unit == 'p') Ctrl->C.dim[GMT_Z] *= 72.0;	/* Since gs requires dots per inch but we gave dots per point */
	
	if (Ctrl->L.active) {	/* Compute auto label placement */
		int col, row, justify;
		for (T = 0; T < Ctrl->L.n_tags; T++) {
			justify = gmt_just_decode (GMT, Ctrl->L.tag[T].placement, PSL_TL);
			col = (justify % 4) - 1;	/* Split the 2-D justify code into x just 0-2 */
			row = justify / 4;		/* Split the 2-D justify code into y just 0-2 */
			Ctrl->L.tag[T].x = 0.5 * Ctrl->C.dim[GMT_X] * col;
			Ctrl->L.tag[T].y = 0.5 * Ctrl->C.dim[GMT_Y] * row;
			if (col != 1) Ctrl->L.tag[T].x += (1-col) * Ctrl->L.tag[T].off[GMT_X];
			if (row != 1) Ctrl->L.tag[T].y += (1-row) * Ctrl->L.tag[T].off[GMT_Y];
			if (Ctrl->L.tag[T].mode == MOVIE_LABEL_IS_COL_T && !strchr (Ctrl->L.tag[T].format, 's')) {
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L: Using +f<format> with word variables requires a \'%%s\'-style format.\n");
				close_files (Ctrl);
				Return (GMT_PARSE_ERROR);
			}
			else if (Ctrl->L.active && Ctrl->L.tag[T].format[0] && !(strchr (Ctrl->L.tag[T].format, 'd') || strchr (Ctrl->L.tag[T].format, 'e') || strchr (Ctrl->L.tag[T].format, 'f') || strchr (Ctrl->L.tag[T].format, 'g'))) {
				GMT_Report (API, GMT_MSG_NORMAL, "Syntax error -L: Using +f<format> with frame or data variables requires a \'%%d\', \'%%e\', \'%%f\', or \'%%g\'-style format.\n");
				close_files (Ctrl);
				Return (GMT_PARSE_ERROR);
			}
		}
	}
	
	if (Ctrl->Q.scripts) {	/* No movie, but scripts will be produced */
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Dry-run enabled - Movie scripts will be created and any pre/post scripts will be executed.\n");
		if (Ctrl->M.active) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "A single plot for frame %d will be create and named %s.%s\n", Ctrl->M.frame, Ctrl->N.prefix, Ctrl->M.format);
		run_script = dry_run_only;	/* This prevents the main frame loop from executing the script */
	}
	else {	/* Will run scripts and may even need to make a movie */
		run_script = system;	/* The standard system function will be used */
	
		if (Ctrl->A.active) {	/* Ensure we have the GraphicsMagick executable "gm" installed in the path */
			if (gmt_check_executable (GMT, "gm", "version", "www.GraphicsMagick.org", line)) {
				sscanf (line, "%*s %s %*s", version);
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "GraphicsMagick %s found.\n", version);
			}
			else {
				GMT_Report (API, GMT_MSG_NORMAL, "GraphicsMagick is not installed or not in your executable path - cannot build animated GIF.\n");
				close_files (Ctrl);
				Return (GMT_RUNTIME_ERROR);
			}
		}
		if (Ctrl->F.active[MOVIE_MP4] || Ctrl->F.active[MOVIE_WEBM]) {	/* Ensure we have ffmpeg installed */
			if (gmt_check_executable (GMT, "ffmpeg", "-version", "FFmpeg developers", line)) {
				sscanf (line, "%*s %*s %s %*s", version);
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "FFmpeg %s found.\n", version);
				if (p_width % 2)	/* Don't like odd pixel widths */
					GMT_Report (API, GMT_MSG_NORMAL, "Your frame width is an odd number of pixels (%u). This may not work with ffmpeg...\n", p_width);
			}
			else {
				GMT_Report (API, GMT_MSG_NORMAL, "ffmpeg is not installed - cannot build MP4 or WEbM movies.\n");
				close_files (Ctrl);
				Return (GMT_RUNTIME_ERROR);
			}
		}
	}
	
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Paper dimensions: Width = %g%c Height = %g%c\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Pixel dimensions: %u x %u\n", p_width, p_height);

	/* First try to read -T<timefile> in case it is prescribed directly (and before we change directory) */
	if (!gmt_access (GMT, Ctrl->T.file, R_OK)) {	/* A file by that name exists and is readable */
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->T.file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to read time file: %s - exiting\n", Ctrl->T.file);
			close_files (Ctrl);
			Return (API->error);
		}
		if (D->n_segments > 1) {	/* We insist on a simple file structure with a single segment */
			GMT_Report (API, GMT_MSG_NORMAL, "Your time file %s has more than one segment - reformat first\n", Ctrl->T.file);
			close_files (Ctrl);
			Return (API->error);
		}
		n_frames = (unsigned int)D->n_records;	/* Number of records means number of frames */
		n_values = (unsigned int)D->n_columns;	/* The number of per-frame parameters we need to place into the per-frame parameter files */
		has_text = (D && D->table[0]->segment[0]->text);	/* Trailing text present */
		if (n_frames == 0) {	/* So not good... */
			GMT_Report (API, GMT_MSG_NORMAL, "Your time file %s has no records - exiting\n", Ctrl->T.file);
			close_files (Ctrl);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (Ctrl->W.active)
		strcpy (workdir, Ctrl->W.dir);
	else
		strcpy (workdir, Ctrl->N.prefix);

	/* Get full path to the current working directory */
	if (getcwd (topdir, PATH_MAX) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Unable to determine current working directory - exiting.\n");
		close_files (Ctrl);
		Return (GMT_RUNTIME_ERROR);
	}
	gmt_replace_backslash_in_path (topdir);
	
	/* Create a working directory which will house every local file and all subdirectories created */
	if (gmt_mkdir (workdir)) {
		GMT_Report (API, GMT_MSG_NORMAL, "An old directory named %s exists OR we were unable to create new working directory %s - exiting.\n", workdir, workdir);
		close_files (Ctrl);
		Return (GMT_RUNTIME_ERROR);
	}
	/* Make this directory the current working directory */
	if (chdir (workdir)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to change directory to %s - exiting.\n", workdir);
		perror (workdir);
		close_files (Ctrl);
		Return (GMT_RUNTIME_ERROR);
	}
	/* Get full path to this working directory */
	if (getcwd (cwd, PATH_MAX) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_VERBOSE, "Unable to determine current working directory.\n");
		close_files (Ctrl);
		Return (GMT_RUNTIME_ERROR);
	}

	/* We use DATADIR to include the top and working directory so any files we supply or create can be found while inside frame directory */

	if (GMT->session.DATADIR)	/* Prepend initial and subdir as new datadirs to the existing search list */
		sprintf (datadir, "%s,%s,%s", topdir, cwd, GMT->session.DATADIR);	/* Start with topdir */
	else	/* Set the initial and prefix subdirectory as data dirs */
		sprintf (datadir, "%s,%s", topdir, cwd);

	gmt_replace_backslash_in_path (datadir);	/* Since we will be fprintf the path we must use // for a slash */
	gmt_replace_backslash_in_path (workdir);
	
	/* Create the initialization file with settings common to all frames */

	n_written = (n_frames > 0);
	sprintf (init_file, "movie_init.%s", extension[Ctrl->In.mode]);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Create parameter initiation script %s\n", init_file);
	if ((fp = fopen (init_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create file %s - exiting\n", init_file);
		close_files (Ctrl);
		Return (GMT_ERROR_ON_FOPEN);
	}
	
	sprintf (string, "Static parameters set for animation sequence %s", Ctrl->N.prefix);
	set_comment (fp, Ctrl->In.mode, string);
	set_dvalue (fp, Ctrl->In.mode, "MOVIE_WIDTH",  Ctrl->C.dim[GMT_X], Ctrl->C.unit);
	set_dvalue (fp, Ctrl->In.mode, "MOVIE_HEIGHT", Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
	set_dvalue (fp, Ctrl->In.mode, "MOVIE_DPU",    Ctrl->C.dim[GMT_Z], 0);
	set_dvalue (fp, Ctrl->In.mode, "MOVIE_RATE",   Ctrl->D.framerate, 0);
	if (n_written) set_ivalue (fp, Ctrl->In.mode, false, "MOVIE_NFRAMES", n_frames);	/* Total frames (write to init since known) */
	if (Ctrl->I.active) {	/* Append contents of an include file */
		set_comment (fp, Ctrl->In.mode, "Static parameters set via user include file");
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->I.fp)) {	/* Read the include file and copy to init script with some exceptions */
			if (is_gmt_module (line, "begin")) continue;		/* Skip gmt begin */
			if (is_gmt_module (line, "end")) continue;		/* Skip gmt end */
			if (strstr (line, "#!/")) continue;			/* Skip any leading shell incantation */
			if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
			fprintf (fp, "%s", line);				/* Just copy the line as is */
		}
		fclose (Ctrl->I.fp);	/* Done reading the include script */
	}
	fclose (fp);	/* Done writing the init script */
	
	if (Ctrl->S[MOVIE_PREFLIGHT].active) {	/* Create the preflight script from the user's background script */
		/* The background script must be modern mode */
		unsigned int rec = 0;
		sprintf (pre_file, "movie_preflight.%s", extension[Ctrl->In.mode]);
		is_classic = script_is_classic (GMT, Ctrl->S[MOVIE_PREFLIGHT].fp);
		if (is_classic) {
			GMT_Report (API, GMT_MSG_NORMAL, "Your preflight file %s is not in GMT modern node - exiting\n", pre_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Create preflight script %s and execute it\n", pre_file);
		if ((fp = fopen (pre_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create preflight script %s - exiting\n", pre_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		set_script (fp, Ctrl->In.mode);			/* Write 1st line of a script */
		set_comment (fp, Ctrl->In.mode, "Preflight script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);		/* Hardwire a Session Name since subshells may mess things up */
		if (Ctrl->In.mode == DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to 1 since we run this separately first */
			fprintf (fp, "set GMT_SESSION_NAME=1\n");
		else	/* On UNIX we may use the calling terminal or script's PID as the GMT_SESSION_NAME */
			set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
		fprintf (fp, "%s", export[Ctrl->In.mode]);		/* Turn off auto-display of figures if scrip has gmt end show */
		set_tvalue (fp, Ctrl->In.mode, true, "GMT_END_SHOW", "off");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->S[MOVIE_PREFLIGHT].fp)) {	/* Read the background script and copy to preflight script with some exceptions */
			if (is_gmt_module (line, "begin")) {	/* Need to insert gmt figure after this line (or as first line) in case a background plot will be made */
				fprintf (fp, "gmt begin\n");	/* To ensure there are no args here since we are using gmt figure instead */
				set_comment (fp, Ctrl->In.mode, "\tSet fixed background output ps name");
				fprintf (fp, "\tgmt figure movie_background ps\n");
				fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
				fprintf (fp, "\tgmt set DIR_DATA %s\n", datadir);
			}
			else if (!strstr (line, "#!/"))	 {	/* Skip any leading shell incantation since already placed by set_script */
				if (is_gmt_end_show (line)) sprintf (line, "gmt end\n");		/* Eliminate show from gmt end in this script */
				else if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
				fprintf (fp, "%s", line);	/* Just copy the line as is */
			}
			rec++;
		}
		fclose (Ctrl->S[MOVIE_PREFLIGHT].fp);	/* Done reading the foreground script */
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
		if (Ctrl->In.mode == DOS_MODE)	/* Needs to be "cmd /C" and not "start /B" to let it have time to finish */
			sprintf (cmd, "cmd /C %s", pre_file);
		else
			sprintf (cmd, "%s %s", sc_call[Ctrl->In.mode], pre_file);
		if ((error = system (cmd))) {
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

	if (Ctrl->L.active) {    /* Make sure we have the information requested if -Lc or -Lt were used */
		for (T = 0; T < Ctrl->L.n_tags; T++) {
			if ((Ctrl->L.tag[T].mode == MOVIE_LABEL_IS_COL_C || Ctrl->L.tag[T].mode == MOVIE_LABEL_IS_COL_T) && D == NULL) {    /* Need a floatingpoint number */
				GMT_Report (API, GMT_MSG_NORMAL, "No table given via -T for data-based labels - exiting\n");
				Return (GMT_RUNTIME_ERROR);
			}
			if (Ctrl->L.tag[T].mode == MOVIE_LABEL_IS_COL_C && Ctrl->L.tag[T].col >= D->n_columns) {
				GMT_Report (API, GMT_MSG_NORMAL, "Data table does not have enough columns for your -Lc%d request - exiting\n", Ctrl->L.tag[T].col);
				Return (GMT_RUNTIME_ERROR);
			}
			if (Ctrl->L.tag[T].mode == MOVIE_LABEL_IS_COL_T && D->table[0]->segment[0]->text == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Data table does not have trailing text for your -Lt%d request - exit\n", Ctrl->L.tag[T].col);
				Return (GMT_RUNTIME_ERROR);
			}
		}
	}

	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Number of animation frames: %d\n", n_frames);
	if (Ctrl->T.precision)	/* Precision was prescribed */
		precision = Ctrl->T.precision;
	else	/* Compute width from largest frame number */
		precision = irint (ceil (log10 ((double)(Ctrl->T.start_frame+n_frames))));	/* Width needed to hold largest frame number */
	
	if (Ctrl->S[MOVIE_POSTFLIGHT].active) {	/* Prepare the postflight script */
		sprintf (post_file, "movie_postflight.%s", extension[Ctrl->In.mode]);
		is_classic = script_is_classic (GMT, Ctrl->S[MOVIE_POSTFLIGHT].fp);
		if (is_classic) {
			GMT_Report (API, GMT_MSG_NORMAL, "Your postflight file %s is not in GMT modern node - exiting\n", post_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Create postflight script %s\n", post_file);
		if ((fp = fopen (post_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create postflight file %s - exiting\n", post_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
		set_comment (fp, Ctrl->In.mode, "Postflight script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a SESSION_NAME since subshells may mess things up */
		if (Ctrl->In.mode == DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to 1 since we run this separately */
			fprintf (fp, "set GMT_SESSION_NAME=1\n");
		else	/* On UNIX we may use the script's PID as GMT_SESSION_NAME */
			set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Turn off auto-display of figures if scrip has gmt end show */
		set_tvalue (fp, Ctrl->In.mode, true, "GMT_END_SHOW", "off");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->S[MOVIE_POSTFLIGHT].fp)) {	/* Read the foreground script and copy to postflight script with some exceptions */
			if (is_gmt_module (line, "begin")) {	/* Need to insert gmt figure after this line */
				fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
				set_comment (fp, Ctrl->In.mode, "\tSet fixed foreground output ps name");
				fprintf (fp, "\tgmt figure movie_foreground ps\n");
				fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
				fprintf (fp, "\tgmt set DIR_DATA %s\n", datadir);
			}
			else if (!strstr (line, "#!/"))	{	/* Skip any leading shell incantation since already placed */
				if (is_gmt_end_show (line)) sprintf (line, "gmt end\n");		/* Eliminate show from gmt end in this script */
				else if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
				fprintf (fp, "%s", line);	/* Just copy the line as is */
			}
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
		if (Ctrl->In.mode == DOS_MODE)	/* Needs to be "cmd /C" and not "start /B" to let it have time to finish */
			sprintf (cmd, "cmd /C %s", post_file);
		else
			sprintf (cmd, "%s %s", sc_call[Ctrl->In.mode], post_file);
		if ((error = run_script (cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running postflight script %s returned error %d - exiting.\n", post_file, error);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (Ctrl->L.active) {	/* Set elapse time label format if chosen */
		char *format = NULL;
		for (T = 0; T < Ctrl->L.n_tags; T++) {
			if (Ctrl->L.tag[T].mode != MOVIE_LABEL_IS_ELAPSED) continue;
			frame = n_frames + Ctrl->T.start_frame;	/* Max frame number */
			L_col = (Ctrl->L.tag[T].scale > 0.0) ? frame * Ctrl->L.tag[T].scale : frame / Ctrl->D.framerate;
			ss = urint (fmod (L_col, 60.0));	L_col = (L_col - ss) / 60.0;	/* Get seconds and switch to minutes */
			mm = urint (fmod (L_col, 60.0));	L_col = (L_col - mm) / 60.0;	/* Get seconds and switch to hours */
			hh = urint (fmod (L_col, 24.0));	L_col = (L_col - hh) / 24.0;	/* Get seconds and switch to days */
			dd = urint (L_col);
			if (dd > 0)
				Ctrl->L.tag[T].ne = 4, strcpy (Ctrl->L.tag[T].format, "%d %2.2d:%2.2d:%2.2d");
			else if (hh > 0)
				Ctrl->L.tag[T].ne = 3, strcpy (Ctrl->L.tag[T].format, "%2.2d:%2.2d:%2.2d");
			else if (mm > 0)
				Ctrl->L.tag[T].ne = 2, strcpy (Ctrl->L.tag[T].format, "%2.2d:%2.2d");
			else
				Ctrl->L.tag[T].ne = 1, strcpy (Ctrl->L.tag[T].format, "%2.2d");
		}
		format = (GMT->current.map.frame.primary) ? GMT->current.setting.format_time[GMT_PRIMARY] : GMT->current.setting.format_time[GMT_SECONDARY];
		switch (format[0]) {	/* This parameter controls which version of month/day textstrings we use for plotting */
			case 'F':	/* Full name, upper case */
				upper_case = true;
				/* fall through on purpose to 'f' */
			case 'f':	/* Full name, lower case */
				flavor = 0;
				break;
			case 'A':	/* Abbreviated name, upper case */
				upper_case = true;
				/* fall through on purpose to 'a' */
			case 'a':	/* Abbreviated name, lower case */
				flavor = 1;
				break;
			case 'C':	/* 1-char name, upper case */
				upper_case = true;
				/* fall through on purpose to 'c' */
			case 'c':	/* 1-char name, lower case */
				flavor = 2;
				break;
			default:
				break;
		}
	}

	/* Create parameter include files, one for each frame */
	
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Create individual parameter include files per frame for %d frames\n", n_frames);
	for (i_frame = 0; i_frame < n_frames; i_frame++) {
		frame = i_frame + Ctrl->T.start_frame;	/* Actual frame number */
		if (one_frame && frame != Ctrl->M.frame) continue;	/* Just doing a single frame for debugging */
		sprintf (state_tag, "%*.*d", precision, precision, frame);
		sprintf (state_prefix, "movie_params_%s", state_tag);
		sprintf (param_file, "%s.%s", state_prefix, extension[Ctrl->In.mode]);
		if ((fp = fopen (param_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create frame parameter file %s - exiting\n", param_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		sprintf (state_prefix, "Parameter file for frame %s", state_tag);
		set_comment (fp, Ctrl->In.mode, state_prefix);
		sprintf (state_prefix, "%s_%s", Ctrl->N.prefix, state_tag);
		set_ivalue (fp, Ctrl->In.mode, false, "MOVIE_FRAME", frame);		/* Current frame number */
		if (!n_written) set_ivalue (fp, Ctrl->In.mode, false, "MOVIE_NFRAMES", n_frames);	/* Total frames (write here since n_frames was not yet known when init was written) */
		set_tvalue (fp, Ctrl->In.mode, false, "MOVIE_TAG", state_tag);		/* Current frame tag (formatted frame number) */
		set_tvalue (fp, Ctrl->In.mode, false, "MOVIE_NAME", state_prefix);	/* Current frame name prefix */
		for (col = 0; col < n_values; col++) {	/* Derive frame variables from <timefile> in each parameter file */
			sprintf (string, "MOVIE_COL%u", col);
			set_value (GMT, fp, Ctrl->In.mode, col, string, D->table[0]->segment[0]->data[col][frame]);
		}
		if (has_text) {	/* Also place any string parameter as a single string variable */
			set_tvalue (fp, Ctrl->In.mode, false, "MOVIE_TEXT", D->table[0]->segment[0]->text[frame]);
			if (Ctrl->T.split) {	/* Also split the string into individual words MOVIE_WORD1, MOVIE_WORD2, etc. */
				char *word = NULL, *trail = NULL, *orig = strdup (D->table[0]->segment[0]->text[frame]);
				col = 0;
				trail = orig;
				while ((word = strsep (&trail, " \t")) != NULL) {
					if (*word != '\0') {	/* Skip empty strings */
						sprintf (string, "MOVIE_WORD%u", col++);
						set_tvalue (fp, Ctrl->In.mode, false, string, word);
					}
				}
				gmt_M_str_free (orig);
			}
		}
		if (Ctrl->L.active) {	/* Want to place a user label in a corner of the frame */
			char label[GMT_LEN256] = {""}, name[GMT_LEN32] = {""};
			unsigned int type;
			/* Set MOVIE_N_LABELS as exported environmental variable. gmt_add_figure will check for this and if found create gmt.movie in session directory */
			fprintf (fp, "%s", export[Ctrl->In.mode]);
			set_ivalue (fp, Ctrl->In.mode, true, "MOVIE_N_LABELS", Ctrl->L.n_tags);
			for (T = 0; T < Ctrl->L.n_tags; T++) {
				sprintf (name, "MOVIE_LABEL_ARG%d", T);
				/* Place x/y/just/clearance_x/clearance_Y/pen/fill/font/txt in MOVIE_LABEL_ARG */
				sprintf (label, "%g%c/%g%c/%s/%g/%g/%s/%s/%s/", Ctrl->L.tag[T].x, Ctrl->C.unit, Ctrl->L.tag[T].y, Ctrl->C.unit,
					Ctrl->L.tag[T].placement, Ctrl->L.tag[T].clearance[GMT_X], Ctrl->L.tag[T].clearance[GMT_Y], Ctrl->L.tag[T].pen,
					Ctrl->L.tag[T].fill, gmt_putfont (GMT, &Ctrl->L.tag[T].font));
				if (Ctrl->L.tag[T].mode == MOVIE_LABEL_IS_FRAME) {	/* Place a frame counter */
					if (Ctrl->L.tag[T].format[0] && strchr (Ctrl->L.tag[T].format, 'd'))	/* Set as integer */
						sprintf (string, Ctrl->L.tag[T].format, (int)frame);
					else if (Ctrl->L.tag[T].format[0])	/* Set as floating point */
						sprintf (string, Ctrl->L.tag[T].format, (double)frame);
					else	/* Default to the frame tag string */
						strcpy (string, state_tag);
				}
				else if (Ctrl->L.tag[T].mode == MOVIE_LABEL_IS_ELAPSED) {	/* Place elapsed time */
					gmt_M_memset (string, GMT_LEN128, char);
					L_col = (Ctrl->L.tag[T].scale > 0.0) ? frame * Ctrl->L.tag[T].scale : frame / Ctrl->D.framerate;
					ss = urint (fmod (L_col, 60.0));	L_col = (L_col - ss) / 60.0;	/* Get seconds and switch to minutes*/
					mm = urint (fmod (L_col, 60.0));	L_col = (L_col - mm) / 60.0;	/* Get seconds and switch to hours */
					hh = urint (fmod (L_col, 24.0));	L_col = (L_col - hh) / 24.0;	/* Get seconds and switch to days */
					dd = urint (L_col);
					switch (Ctrl->L.tag[T].ne) {
						case 4: 	sprintf (string, Ctrl->L.tag[T].format, dd, hh, mm, ss); break;
						case 3: 	sprintf (string, Ctrl->L.tag[T].format, hh, mm, ss); break;
						case 2: 	sprintf (string, Ctrl->L.tag[T].format, mm, ss); break;
						case 1: 	sprintf (string, Ctrl->L.tag[T].format, ss); break;
					}
				}
				else if (Ctrl->L.tag[T].mode == MOVIE_LABEL_IS_COL_C) {	/* Format a floatingpoint number */
					L_col = D->table[0]->segment[0]->data[Ctrl->L.tag[T].col][frame];
					if ((type = gmt_M_type (GMT, GMT_IN, Ctrl->L.tag[T].col)) == GMT_IS_ABSTIME) {	/* Time formatting */
						char date[GMT_LEN16] = {""}, clock[GMT_LEN16] = {""};
						gmt_format_calendar (GMT, date, clock, &GMT->current.plot.calclock.date, &GMT->current.plot.calclock.clock, upper_case, flavor, L_col);
						if (GMT->current.plot.calclock.clock.skip)
							sprintf (string, "%s", date);
						else if (GMT->current.plot.calclock.date.skip)
							sprintf (string, "%s", clock);
						else
							sprintf (string, "%s %s", date, clock);
					}
					else {	/* Regular floating point (or latitude, etc.) */
						if (Ctrl->L.tag[T].format[0] && strchr (Ctrl->L.tag[T].format, 'd'))	/* Set as an integer */
							sprintf (string, Ctrl->L.tag[T].format, (int)irint (L_col));
						else if (Ctrl->L.tag[T].format[0])	/* Set as floating point */
							sprintf (string, Ctrl->L.tag[T].format, L_col);
						else	/* Uses standard formatting*/
							gmt_ascii_format_one (GMT, string, L_col, type);
					}
				}
				else if (Ctrl->L.tag[T].mode == MOVIE_LABEL_IS_COL_T) {	/* Place a word label */
					char *word = NULL, *trail = NULL, *orig = strdup (D->table[0]->segment[0]->text[frame]);
					col = 0;	trail = orig;
					while (col != Ctrl->L.tag[T].col && (word = strsep (&trail, " \t")) != NULL) {
						if (*word != '\0')	/* Skip empty strings */
							col++;
					}
					strcpy (L_txt, word);
					gmt_M_str_free (orig);
					if (Ctrl->L.tag[T].format[0])	/* Use the given string format */
						sprintf (string, Ctrl->L.tag[T].format, L_txt);
					else	/* Plot as is */
						strcpy (string, L_txt);
				}
				strcat (label, string);
				/* Set MOVIE_LABEL_ARG# as exported environmental variable. gmt figure will check for this and if found create gmt.movie in session directory */
				fprintf (fp, "%s", export[Ctrl->In.mode]);
				set_tvalue (fp, Ctrl->In.mode, true, name, label);
			}
		}
		fclose (fp);	/* Done writing this parameter file */
	}

	if (Ctrl->M.active) {	/* Make the master frame plot */
		char master_file[GMT_LEN32] = {""};
		/* Because format may differ and name will differ we just make a special script for this job */
		sprintf (master_file, "movie_master.%s", extension[Ctrl->In.mode]);
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Create master frame script %s\n", master_file);
		if ((fp = fopen (master_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to create loop frame script file %s - exiting\n", master_file);
			fclose (Ctrl->In.fp);
			Return (GMT_ERROR_ON_FOPEN);
		}
		if (Ctrl->G.active)	/* Want to set a fixed background canvas color - we do this via the psconvert -A option */
			sprintf (extra, "A+g%s+n+r", Ctrl->G.fill);
		else
			sprintf (extra, "A+n+r");	/* No cropping, image size is fixed */
		if (!access ("movie_background.ps", R_OK))	/* Need to place a background layer first (which is in parent dir when loop script is run) */
			strcat (extra, ",Mb../movie_background.ps");
		if (!access ("movie_foreground.ps", R_OK))	/* Need to append foreground layer at end (which is in parent dir when script is run) */
			strcat (extra, ",Mf../movie_foreground.ps");
		if (Ctrl->H.active) {	/* Must pass the DownScaleFactor option to psconvert */
			sprintf (line, ",H%d", Ctrl->H.factor);
			strncat (extra, line, GMT_LEN128);
		}
		set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
		set_comment (fp, Ctrl->In.mode, "Master frame loop script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a GMT_SESSION_NAME since subshells may mess things up */
		if (Ctrl->In.mode == DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to be the frame number */
			fprintf (fp, "set GMT_SESSION_NAME=%c1\n", var_token[Ctrl->In.mode]);
		else	/* On UNIX we use the script's PID as GMT_SESSION_NAME */
			set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
		set_comment (fp, Ctrl->In.mode, "Include static and frame-specific parameters");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		fprintf (fp, "%s movie_params_%c1.%s\n", load[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Include the frame parameters */
		fprintf (fp, "mkdir master\n");	/* Make a temp directory for this frame */
		fprintf (fp, "cd master\n");		/* cd to the temp directory */
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->In.fp)) {	/* Read the mainscript and copy to loop script, with some exceptions */
			if (is_gmt_module (line, "begin")) {	/* Need to insert a gmt figure call after this line */
				fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
				set_comment (fp, Ctrl->In.mode, "\tSet output name and plot conversion parameters");
				fprintf (fp, "\tgmt figure %s %s", Ctrl->N.prefix, Ctrl->M.format);
				fprintf (fp, " %s", extra);
				if (strstr (Ctrl->M.format, "pdf") || strstr (Ctrl->M.format, "eps") || strstr (Ctrl->M.format, "ps"))
					fprintf (fp, "\n");	/* No dpu needed */
				else
					fprintf (fp, ",E%s\n", place_var (Ctrl->In.mode, "MOVIE_DPU"));
				fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c DIR_DATA %s\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit, datadir);
			}
			else if (!strstr (line, "#!/"))	{	/* Skip any leading shell incantation since already placed */
				if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
				fprintf (fp, "%s", line);	/* Just copy the line as is */
			}
		}
		rewind (Ctrl->In.fp);	/* Get ready for main_frame reading */
		set_comment (fp, Ctrl->In.mode, "Move master file up to top directory and cd up one level");
		fprintf (fp, "%s %s.%s %s\n", mvfile[Ctrl->In.mode], Ctrl->N.prefix, Ctrl->M.format, topdir);	/* Move master plot up to top dir */
		fprintf (fp, "cd ..\n");	/* cd up to workdir */
		if (!Ctrl->Q.active) {	/* Remove the work dir and any files in it */
			set_comment (fp, Ctrl->In.mode, "Remove frame directory");
			fprintf (fp, "%s master\n", rmdir[Ctrl->In.mode]);
		}
		fclose (fp);	/* Done writing loop script */

#ifndef WIN32
		/* Set executable bit if not Windows */
		if (chmod (master_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_NORMAL, "Unable to make script %s executable - exiting\n", master_file);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
#endif
		sprintf (cmd, "%s %s %*.*d", sc_call[Ctrl->In.mode], master_file, precision, precision, Ctrl->M.frame);
		if ((error = run_script (cmd))) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running script %s returned error %d - exiting.\n", cmd, error);
			fclose (Ctrl->In.fp);
			Return (GMT_RUNTIME_ERROR);
		}
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Single master plot (frame %d) built: %s.%s\n", Ctrl->M.frame, Ctrl->N.prefix, Ctrl->M.format);
		if (!Ctrl->Q.active) {
			/* Delete the masterfile script */
			if (gmt_remove_file (GMT, master_file)) {	/* Delete the master_file script */
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to delete the master file script %s.\n", master_file);
				fclose (Ctrl->In.fp);
				Return (GMT_RUNTIME_ERROR);
			}
		}
		if (Ctrl->M.exit) {	/* Well, we are done */
			fclose (Ctrl->In.fp);
			/* Cd back up to the starting directory */
			if (chdir (topdir)) {	/* Should never happen but we do check */
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to change directory to starting directory - exiting.\n");
				perror (topdir);
				Return (GMT_RUNTIME_ERROR);
			}
			if (!Ctrl->Q.active) {	/* Delete the entire directory */
				sprintf (line, "%s %s\n", rmdir[Ctrl->In.mode], workdir);
				if ((error = system (line))) {
					GMT_Report (API, GMT_MSG_NORMAL, "Deleting the working directory %s returned error %d.\n", workdir, error);
					Return (GMT_RUNTIME_ERROR);
				}
			}
			Return (GMT_NOERROR);
		}
	}
	
	/* Now build the main loop script from the mainscript */
	if (Ctrl->Q.active) strcat (frame_products, MOVIE_DEBUG_FORMAT);	/* Want to save original PS file for debug */
	
	sprintf (main_file, "movie_frame.%s", extension[Ctrl->In.mode]);
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Create main movie frame script %s\n", main_file);
	if ((fp = fopen (main_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to create loop frame script file %s - exiting\n", main_file);
		fclose (Ctrl->In.fp);
		Return (GMT_ERROR_ON_FOPEN);
	}
	extra[0] = '\0';	/* Reset */
	if (Ctrl->G.active)	/* Want to set a fixed background canvas color - we do this via the psconvert -A option */
		sprintf (extra, "A+g%s+n+r", Ctrl->G.fill);
	else
		sprintf (extra, "A+n+r");	/* No cropping, image size is fixed */
	if (!access ("movie_background.ps", R_OK)) {	/* Need to place a background layer first (which will be in parent dir when loop script is run) */
		strcat (extra, ",Mb../movie_background.ps");
		layers = true;
	}
	if (!access ("movie_foreground.ps", R_OK)) {	/* Need to append foreground layer at end (which will be in parent dir when script is run) */
		strcat (extra, ",Mf../movie_foreground.ps");
		layers = true;
	}
	if (Ctrl->H.active) {	/* Must pass the DownScaleFactor option to psconvert */
		char htxt[GMT_LEN16] = {""};
		sprintf (htxt, ",H%d", Ctrl->H.factor);
		strcat (extra, htxt);
	}
	set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
	set_comment (fp, Ctrl->In.mode, "Main frame loop script");
	fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a GMT_SESSION_NAME since subshells may mess things up */
	if (Ctrl->In.mode == DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to be the frame number */
		fprintf (fp, "set GMT_SESSION_NAME=%c1\n", var_token[Ctrl->In.mode]);
	else	/* On UNIX we use the script's PID as GMT_SESSION_NAME */
		set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
	fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Turn off auto-display of figures if scrip has gmt end show */
	set_tvalue (fp, Ctrl->In.mode, true, "GMT_END_SHOW", "off");
	set_comment (fp, Ctrl->In.mode, "Include static and frame-specific parameters");
	fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
	fprintf (fp, "%s movie_params_%c1.%s\n", load[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Include the frame parameters */
	fprintf (fp, "mkdir %s\n", place_var (Ctrl->In.mode, "MOVIE_NAME"));	/* Make a temp directory for this frame */
	fprintf (fp, "cd %s\n", place_var (Ctrl->In.mode, "MOVIE_NAME"));		/* cd to the temp directory */
	while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->In.fp)) {	/* Read the main script and copy to loop script, with some exceptions */
		if (is_gmt_module (line, "begin")) {	/* Need to insert a gmt figure call after this line */
			fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
			set_comment (fp, Ctrl->In.mode, "\tSet output PNG name and plot conversion parameters");
			fprintf (fp, "\tgmt figure %s %s", place_var (Ctrl->In.mode, "MOVIE_NAME"), frame_products);
			fprintf (fp, " E%s,%s\n", place_var (Ctrl->In.mode, "MOVIE_DPU"), extra);
			fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c DIR_DATA %s\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit, datadir);
		}
		else if (!strstr (line, "#!/")) {		/* Skip any leading shell incantation since already placed */
			if (is_gmt_end_show (line)) sprintf (line, "gmt end\n");		/* Eliminate show from gmt end in this script */
			else if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
			fprintf (fp, "%s", line);	/* Just copy the line as is */
		}
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
	if (Ctrl->In.mode == DOS_MODE)	/* This is crucial to the "start /B ..." statement below to ensure the DOS process terminates */
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
	n_cores_unused = MAX (1, Ctrl->x.n_threads - 1);			/* Remove one for the main movie module thread */
	status = gmt_M_memory (GMT, NULL, n_frames, struct MOVIE_STATUS);	/* Used to keep track of frame status */
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Build frames using %u cores\n", n_cores_unused);
	/* START PARALLEL EXECUTION OF FRAME SCRIPTS */
	GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Execute movie frame scripts in parallel\n");
	while (!done) {	/* Keep running jobs until all frames have completed */
		while (n_frames_not_started && n_cores_unused) {	/* Launch new jobs if possible */
#ifdef WIN32
			if (Ctrl->In.mode < 2)		/* A bash or sh run from Windows. Need to call via "start" to get parallel */
				sprintf (cmd, "start /B %s %s %*.*d", sc_call[Ctrl->In.mode], main_file, precision, precision, frame);
			else						/* Running batch, so no need for the above trick */
				sprintf (cmd, "%s %s %*.*d &", sc_call[Ctrl->In.mode], main_file, precision, precision, frame);
#else 
			sprintf (cmd, "%s %s %*.*d &", sc_call[Ctrl->In.mode], main_file, precision, precision, frame);
#endif

			GMT_Report (API, GMT_MSG_DEBUG, "Launch script for frame %*.*d\n", precision, precision, frame);
			if ((error = system (cmd))) {
				GMT_Report (API, GMT_MSG_NORMAL, "Running script %s returned error %d - aborting.\n", cmd, error);
				Return (GMT_RUNTIME_ERROR);
			}
			status[frame].started = true;	/* We have now launched this frame job */
			frame++;			/* Advance to next frame for next launch */
			i_frame++;			/* Advance to next frame for next launch */
			n_frames_not_started--;		/* One less frame remaining */
			n_cores_unused--;		/* This core is now busy */
		}
		gmt_sleep (MOVIE_WAIT_TO_CHECK);	/* Wait 0.01 second - then check for completion of the PNG images */
		for (k = first_frame; k < i_frame; k++) {	/* Only loop over the range of frames that we know are currently in play */
			if (status[k].completed) continue;	/* Already finished with this frame */
			if (!status[k].started) continue;	/* Not started this frame yet */
			/* Here we can check if the frame job has completed by looking for the PNG product */
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
	if (chdir (topdir)) {	/* Should never happen but we should check */
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to change directory to starting directory - exiting.\n");
		perror (topdir);
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->A.active) {	/* Want an animated GIF */
		/* Set up system call to gm (which we know exists) */
		unsigned int delay = urint (100.0 / Ctrl->D.framerate);	/* Delay to nearest ~1/100 s */
		char files[GMT_LEN32] = {""};
		files[0] = '*';
		if (Ctrl->A.skip) {	/* Only use every stride file */
			if (Ctrl->A.stride == 2 || Ctrl->A.stride == 20 || Ctrl->A.stride == 200 || Ctrl->A.stride == 2000)
				strcat (files, "[02468]");
			else if (Ctrl->A.stride == 5 || Ctrl->A.stride == 50 || Ctrl->A.stride == 500 || Ctrl->A.stride == 5000)
				strcat (files, "[05]");
			else	/* 10, 100, 1000, etc */
				strcat (files, "[0]");
			if (Ctrl->A.stride > 1000)
				strcat (files, "000");
			else if (Ctrl->A.stride > 100)
				strcat (files, "00");
			else if (Ctrl->A.stride > 10)
				strcat (files, "0");
		}
		sprintf (cmd, "gm convert -delay %u -loop %u +dither %s%c%s_%s.%s %s.gif", delay, Ctrl->A.loops, workdir, dir_sep, Ctrl->N.prefix, files, MOVIE_RASTER_FORMAT, Ctrl->N.prefix);
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
			extra, Ctrl->D.framerate, workdir, dir_sep, Ctrl->N.prefix, png_file, MOVIE_RASTER_FORMAT, (Ctrl->F.options[MOVIE_MP4]) ? Ctrl->F.options[MOVIE_MP4] : "", Ctrl->N.prefix);
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
			extra, Ctrl->D.framerate, workdir, dir_sep, Ctrl->N.prefix, png_file, MOVIE_RASTER_FORMAT, (Ctrl->F.options[MOVIE_WEBM]) ? Ctrl->F.options[MOVIE_WEBM] : "", Ctrl->N.prefix);
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
	set_script (fp, Ctrl->In.mode);		/* Write 1st line of a script */
	if (Ctrl->Z.active) {	/* Want to delete the entire frame directory */
		set_comment (fp, Ctrl->In.mode, "Cleanup script removes working directory with frame files");
		fprintf (fp, "%s %s\n", rmdir[Ctrl->In.mode], workdir);	/* Delete the entire working directory with PNG frames and tmp files */
	}
	else {	/* Just delete the remaining scripts and PS files */
#ifdef WIN32		/* On Windows to do remove a file in a subdir one need to use back slashes */
		char dir_sep_ = '\\';
#else
		char dir_sep_ = '/';
#endif
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "%u frame PNG files saved in directory: %s\n", n_frames, Ctrl->N.prefix);
		if (Ctrl->S[MOVIE_PREFLIGHT].active)	/* Remove the preflight script */
			fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], workdir, dir_sep_, pre_file);
		if (Ctrl->S[MOVIE_POSTFLIGHT].active)	/* Remove the postflight script */
			fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], workdir, dir_sep_, post_file);
		fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], workdir, dir_sep_, init_file);	/* Delete the init script */
		fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], workdir, dir_sep_, main_file);	/* Delete the main script */
		if (layers) 
			fprintf (fp, "%s %s%c*.ps\n", rmfile[Ctrl->In.mode], workdir, dir_sep_);	/* Delete any PostScript layers */
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
		if (error) {
			GMT_Report (API, GMT_MSG_NORMAL, "Running cleanup script %s returned error %d - exiting.\n", cleanup_file, error);
			Return (GMT_RUNTIME_ERROR);
		}
	}

	/* Finally, delete the clean-up script separately since under DOS we got complaints when we had it delete itself (which works under *nix) */
	if (!Ctrl->Q.active && gmt_remove_file (GMT, cleanup_file)) {	/* Delete the cleanup script itself */
		GMT_Report (API, GMT_MSG_NORMAL, "Unable to delete the cleanup script %s.\n", cleanup_file);
		Return (GMT_RUNTIME_ERROR);
	}

	Return (GMT_NOERROR);
}
