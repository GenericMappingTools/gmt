/*--------------------------------------------------------------------
 *
 *	Copyright (c) 1991-2023 by the GMT Team (https://www.generic-mapping-tools.org/team.html)
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
 * and foreground layer to be specified via separate modern mode scripts and
 * a single frame-script that uses special variables to make slightly
 * different plots for each frame.  The user only needs to compose these
 * simple one-plot scripts and then movie takes care of the automation,
 * processing to PNG, assembly of a movie or animated GIF, and cleanup.
 * Frame scripts are run in parallel without need for OpenMP, etc.
 * Optionally, you can supply a title page script and request fading of
 * the title page and/or the main animation.  The fore-, back-ground, and
 * title page scripts can also just be ready-to-use PostScripts plot of
 * correct canvas size.
 *
 * Note 1: movie has options -L and -P that let each frame plot have an
 * overlay of one or more movie labels and one or more sets of movie progress
 * indicators.  Like tags in subplots (e.g., "a)"), these must be plotted
 * ON TOP OF everything else plotted in each frame.  When movie is run it
 * does not have the brains to figure that out, but it knows that these items,
 * if specified, must be plotted at the end, yet the options are given to
 * movie.  Here is how I implemented this:  This black magic is similar to
 * what happens in subplot (see the Note in subplot.c first) but there are
 * differences.  movie creates a bunch of parameter files and a master
 * script, and then we launch that master script with an frame number that lets
 * it read the corresponding parameters set.  Many of the items in the parameter
 * file are things that change from frame to frame and may have originated in
 * the time file (-T).  For instance, we may have a shell assignment of a variable
 * like MOVIE_COL0=-77.2085847092.  This may be a quantity that the master
 * script can use to plot a string or use to get a color via a CPT, for instance.
 * That is nice, but how about labels? Well, this is the problem: We want those
 * labels to just happen, not make the user write pstext calls using something
 * like a MOVIE_WORD0="Time is 64.5".  If it worked that way, why would there be
 * a -L (or -P) option if you have to do all the work anyway? So, we want movie
 * to automatically set these labels, just like it works for subplot tags.
 * The solution chosen for this is write this content as comments.
 * As an example, the parameter file for frame 29 may have this comment:
 *
 * # MOVIE_L: L|0.1|4.7|0.5|0|9|0.0416667|0.0416667|-|-|-|-|20p,Helvetica,black|29
 *
 * or in DOS
 *
 * REM MOVIE_L: L|0.1|4.7|0.5|0|9|0.0416667|0.0416667|-|-|-|-|20p,Helvetica,black|29
 *
 * Here, we only have one movie label [-L is repeatable so we could have more].
 * In this case, we want to place the frame string "29".  The rest is information gmt needs
 * to know what to do: placement, offsets, font, etc. OK, then the rest of the frame script
 * composed by the user (which knows nothing of the above) happens.  Unbeknownst to
 * the user, her script is actually embellished by movie in various ways.  For instance,
 * we call gmt figure to define the plot format before the user's commands are appended.
 * When gmt figure runs we end up calling gmt_add_figure (gmt_init.c) and it actually
 * is passed a special option -I<parameterfile>.  If gmt figure is given this special option
 * we get its value and learn (1) that gmt figure is called from a movie script and (2)
 * that we have labels to place.  Now, we extract all such labels (here just 1) from
 * that parameter file.  These labels are then written to a file under the session directory
 * called gmt.movie_labels.  Only the current frame process will find that file since each
 * frame is run in separate session directories.
 * If you used -P then the exact same thing happens for movie progress bars and there will
 * be a file called gmt.movie_prog_indicators created.  So -L -P lead to one or two files
 * being created in the session directory before the first gmt command after gmt figure
 * starts.  From here on it is very similar to the subplot story: In gmt_plotinit, we
 * check if these files exist, and if they do we build a PostScript function called
 * PSL_movie_label_completion [for labels] or PSL_movie_prog_indicator_completion [for
 * progress indicators].  These are used a bit differently from the PSL_plot_completion
 * function for subplots since there are many panels and finishing one panel is not
 * necessarily the end of the plot.  However, for the movie embellishments, we know
 * that when gmt end comes and calls PSL_endplot, it is time to execute these two
 * PostScript functions, should they exist.  Once completed, they are redefined as
 * NULL functions.  Unlike PSL_plot_completion, the movie functions are more complicated
 * and may plot more than one label and more than one progress indicator.
 */

#include "gmt_dev.h"
#include "longopt/movie_inc.h"
#ifdef WIN32
#include <windows.h>
#endif
#include "gmt_gsformats.h"

#define THIS_MODULE_CLASSIC_NAME	"movie"
#define THIS_MODULE_MODERN_NAME	"movie"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Create animation sequences and movies"
#define THIS_MODULE_KEYS	"<D("
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS	"-Vf"

#define MOVIE_PREFLIGHT		0
#define MOVIE_POSTFLIGHT	1

#define MOVIE_WAIT_TO_CHECK	10000	/* In microseconds, so 0.01 seconds */
#define MOVIE_PAUSE_A_SEC	1000000	/* In microseconds, so 1 seconds */

#define MOVIE_RASTER_EXTENSION	"png"	/* Fixed raster format extension */
#define MOVIE_DEBUG_FORMAT	",ps"	/* Comma is intentional since we append to a list of formats */

enum enum_video {
	MOVIE_PNG,	/* Create PNGs */
	MOVIE_GIF,	/* Create an animated GIF*/
	MOVIE_MP4,	/* Create a H.264 MP4 video */
	MOVIE_WEBM,		/* Create a WebM video */
	MOVIE_N_FORMATS};	/* Number of video formats above */

enum enum_label {MOVIE_LABEL_IS_FRAME = 1,
	MOVIE_LABEL_IS_ELAPSED,
	MOVIE_LABEL_IS_PERCENT,
	MOVIE_LABEL_IS_COL_C,
	MOVIE_LABEL_IS_COL_T,
	MOVIE_LABEL_IS_STRING};

struct MOVIE_ITEM {
	struct GMT_FONT font;
	char format[GMT_LEN128];
	char fill[GMT_LEN64], fill2[GMT_LEN64], sfill[GMT_LEN64];
	char pen[GMT_LEN64], pen2[GMT_LEN64];
	char kind;				/* Either a-f|A-F for progress indicators or L for label */
	unsigned int box;		/* 1 if rounded text box, 0 if rectangular */
	unsigned int mode;		/* What type of "time" selected for label */
	unsigned int justify;	/* Placement location of label or indicator */
	unsigned int col;		/* Which data column to use for labels (if active) */
	unsigned int ne;		/* Number of elements in elapsed format */
	unsigned int n_labels;	/* Number of labels for progress indicator */
	double scale;			/* Scaling from frame number to elapsed time [1/framerate] */
	double width;			/* Width of progress indicator */
	double x, y;			/* Placement of progress indicator or label in inches */
	double off[2];			/* Offset from justification point for progress indicators */
	double clearance[2];	/* Space between label and text box (if selected) for -L labels */
	double soff[2];			/* Space between text box and shaded box background (if selected) for -L labels */
};

/* Control structure for movie */

struct MOVIE_CTRL {
	bool animate;	/* True if we are making any animated product (GIF, movies) */
	struct MOVIE_ITEM item[2][GMT_LEN32];	/* 0 for labels, 1 for progress indicators */
	unsigned int n_items[2];				/* 0 for labels, 1 for progress indicators */
	bool item_active[2];					/* 0 for labels, 1 for progress indicators */
	struct MOVIE_In {	/* mainscript (Bourne, Bourne Again, csh, or DOS (bat) script) */
		bool active;
		enum GMT_enum_script mode;
		char *file;	/* Name of main script */
		FILE *fp;	/* Open file pointer to main script */
	} In;
	struct MOVIE_A {	/* -A<audiofile>[+e] */
		bool active;
		bool exact;
		double duration;	/* Original length of audio track */
		char *file;
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
	struct MOVIE_E {	/* -E<title>[+d<duration>[s]][+f[i|o][<fade>[s]]][+g<fill>] */
		bool active;
		bool PS;		/* true if we got a plot instead of a script */
		char *file;		/* Name of title script */
		char *fill;		/* Fade color [black] */
		unsigned int duration;	/* Total number of frames of title/fade sequence */
		unsigned int fade[2];	/* Duration of fade title in, fade title out [none]*/
		FILE *fp;			/* Open file pointer to title script */
	} E;
	struct MOVIE_F {	/* -F<videoformat>[+l<n>][+o<options>][+s<stride>][+t][+v] - repeatable */
		bool active[MOVIE_N_FORMATS];
		bool view[MOVIE_N_FORMATS];
		bool transparent;
		bool loop;
		bool skip;
		unsigned int loops;
		unsigned int stride;
		char *format[MOVIE_N_FORMATS];
		char *options[MOVIE_N_FORMATS];
	} F;
	struct MOVIE_G {	/* -G<canvasfill>[+p<pen>] */
		bool active;
		unsigned int mode;
		char *fill;		/* Canvas constant fill */
		char pen[GMT_LEN64];	/* Canvas outline pen */
	} G;
	struct MOVIE_H {	/* -H<scale> */
		bool active;
		int factor;	/* Amount of subpixel rendering */
	} H;
	struct MOVIE_I {	/* -I<includefile> */
		bool active;
		char *file;	/* Name of include script */
		FILE *fp;	/* Open file pointer to include script */
	} I;
	struct MOVIE_K {	/* -K[+f[i|o][<fade>[s]]][+g<fill>][+p[i|o]] */
		bool active;
		bool preserve[2];	/* Preserve first and last frames */
		char *fill;		/* Fade color [black] */
		unsigned int fade[2];	/* Duration of movie in and movie out fades [none]*/
	} K;
	struct MOVIE_L {	/* Repeatable: -L[e|f|c#|t#|s<string>][+c<clearance>][+f<font>][+g<fill>][+h[<dx>/<dy>/][<shade>]][+j<justify>][+o<offset>][+p<pen>][+r][+t<fmt>][+s<scl>] */
		bool active;
	} L;
	struct MOVIE_M {	/* -M[<frame>|f|l|m][,format][+r<dpu>][+v] */
		bool active;
		bool exit;
		bool dpu_set;
		bool view;
		unsigned int update;	/* 1 = set middle, 2 = set last frame */
		unsigned int frame;	/* Frame selected as master frame */
		double dpu;
		char *format;	/* Plot format for master frame */
	} M;
	struct MOVIE_N {	/* -N<movieprefix> */
		bool active;
		char *prefix;	/* Movie prefix and also name of working directory (but see -W) */
	} N;
	struct MOVIE_P {	/* Repeatable: -P[<kind>][+w<width>][+p<pen1>][+P<pen2>][+g<fill1>][+G<fill2>][+o<offset>][+j<justify>][+a[e|f|c#|t#|s<string>][+t<fmt>][+s<scl>] */
		bool active;
	} P;
	struct MOVIE_Q {	/* -Q[s] */
		bool active;
		bool scripts;
	} Q;
	struct MOVIE_S {	/* -Sb|f<script>|<psfile> */
		bool active;
		bool PS;	/* true if we got a plot instead of a script */
		char *file;	/* Name of script or PostScript file */
		FILE *fp;	/* Open file pointer to script */
	} S[2];
	struct MOVIE_T {	/* -T<n_frames>|<min>/<max/<inc>[+n]|<timefile>[+p<precision>][+s<frame>][+w[<str>]] */
		bool active;
		bool split;		/* true means we must split any trailing text in to words, using separators in <str> [" \t"] */
		unsigned int n_frames;	/* Total number of frames */
		unsigned int start_frame;	/* First frame [0] */
		unsigned int precision;	/* Decimals used in making unique frame tags */
		char sep[GMT_LEN8];		/* word separator(s) */
		char *file;		/* timefile name */
	} T;
	struct MOVIE_W {	/* -W<workingdirectory> */
		bool active;
		char *dir;	/* Alternative working directory than implied by -N */
	} W;
	struct MOVIE_Z {	/* -Z[s] */
		bool active;	/* Delete temporary files when completed */
		bool delete;	/* Also delete all files including the mainscript and anything passed via -E, -I, -S */
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

static void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct MOVIE_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct MOVIE_CTRL);
	C->C.unit = 'c';	/* c for SI units */
	C->D.framerate = 24.0;	/* 24 frames/sec */
	C->F.loops = 1;		/* No loop, just play once */
	C->F.options[MOVIE_WEBM] = strdup ("-crf 10 -b:v 1.2M");	/* Default WebM options for now */
	strcpy (C->T.sep, "\t ");	/* Any white space */
	C->x.n_threads = GMT->parent->n_cores;	/* Use all cores available unless -x is set */
	return (C);
}

static void Free_Ctrl (struct GMT_CTRL *GMT, struct MOVIE_CTRL *C) {	/* Deallocate control structure */
	gmt_M_unused (GMT);
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->A.file);
	gmt_M_str_free (C->C.string);
	gmt_M_str_free (C->E.fill);
	gmt_M_str_free (C->M.format);
	for (unsigned int k = 0; k < MOVIE_N_FORMATS; k++) {
		gmt_M_str_free (C->F.format[k]);
		gmt_M_str_free (C->F.options[k]);
	}
	gmt_M_str_free (C->G.fill);
	gmt_M_str_free (C->I.file);
	gmt_M_str_free (C->K.fill);
	gmt_M_str_free (C->N.prefix);
	gmt_M_str_free (C->S[MOVIE_PREFLIGHT].file);
	gmt_M_str_free (C->S[MOVIE_POSTFLIGHT].file);
	gmt_M_str_free (C->T.file);
	gmt_M_str_free (C->W.dir);
	gmt_M_free (GMT, C);
}

/*! -x[[-]<ncores>] parsing needed but here not related to OpenMP etc - it is just a local option */
GMT_LOCAL int movie_parse_x_option (struct GMT_CTRL *GMT, struct MOVIE_CTRL *Ctrl, char *arg) {
	if (!arg) return (GMT_PARSE_ERROR);	/* -x requires a non-NULL argument */
	if (arg[0])
		Ctrl->x.n_threads = atoi (arg);

	if (Ctrl->x.n_threads == 0)	/* Not having any of that.  At least one */
		Ctrl->x.n_threads = 1;
	else if (Ctrl->x.n_threads < 0)	/* Meant to reduce the number of threads */
		Ctrl->x.n_threads = MAX(GMT->parent->n_cores + Ctrl->x.n_threads, 1);		/* Max-n but at least one */
	return (GMT_NOERROR);
}

static int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Usage (API, 0, "usage: %s <mainscript> -C<canvas>|<width>x<height>x<dpu> -N<prefix> -T<nframes>|<min>/<max>/<inc>[+n]|<timefile>[+p<width>][+s<first>][+w[<str>]|W] "
		"-A<audiofile>[+e]] [-D<rate>] [-E<titlepage>[+d[<duration>[s]]][+f[i|o][<fade>[s]]][+g<fill>]] [-Fgif|mp4|webm|png[+l[<n>]][+o<opts>][+s<stride>][+t][+v]] "
		"[-G[<fill>][+p<pen>]] [-H<scale>] [-I<includefile>] [-K[+f[i|o][<fade>[s]]][+g<fill>][+p[i|o]]] [-L<labelinfo>] [-M[<frame>|f|m|l,][<format>][+r<dpu>][+v]] "
		"[-P<progressinfo>] [-Q[s]] [-Sb<background>] [-Sf<foreground>] [%s] [-W[<dir>]] [-Z[s]] [%s] [-x[[-]<n>]] [%s]\n", name, GMT_V_OPT, GMT_f_OPT, GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "  REQUIRED ARGUMENTS:\n");

	GMT_Usage (API, 1, "\n<mainscript>");
	GMT_Usage (API, -2, "<mainscript> is the main GMT modern script that builds a single frame image.");
	GMT_Usage (API, 1, "\n-C<canvas>|<width>x<height>x<dpu>");
	GMT_Usage (API, -2, "Specify canvas. Choose either a known named canvas or set custom dimensions.");
	GMT_Usage (API, -2, "%s Recognized 16:9-ratio names and associated dimensions:", GMT_LINE_BULLET);
	/* Using GMT_Message here for verbatim text and alignment since GMT_Usage will eat repeated spaces */
	GMT_Message (API, GMT_TIME_NONE, "        name:  pixel size:    canvas (SI):       dpc:      canvas (US):       dpi:\n");
	GMT_Message (API, GMT_TIME_NONE, "       ---------------------------------------------------------------------------\n");
	GMT_Message (API, GMT_TIME_NONE, "       4320p:  7680 x 4320    24 x 13.5 cm   320.0000    9.6 x 5.4 inch   800.0000\n");
	GMT_Message (API, GMT_TIME_NONE, "       2160p:  3840 x 2160    24 x 13.5 cm   160.0000    9.6 x 5.4 inch   400.0000\n");
	GMT_Message (API, GMT_TIME_NONE, "       1080p:  1920 x 1080    24 x 13.5 cm    80.0000    9.6 x 5.4 inch   200.0000\n");
	GMT_Message (API, GMT_TIME_NONE, "        720p:  1280 x  720    24 x 13.5 cm    53.3333    9.6 x 5.4 inch   133.3333\n");
	GMT_Message (API, GMT_TIME_NONE, "        540p:   960 x  540    24 x 13.5 cm    40.0000    9.6 x 5.4 inch   100.0000\n");
	GMT_Message (API, GMT_TIME_NONE, "        480p:   854 x  480    24 x 13.5 cm    35.5833    9.6 x 5.4 inch    88.9583\n");
	GMT_Message (API, GMT_TIME_NONE, "        360p:   640 x  360    24 x 13.5 cm    26.6667    9.6 x 5.4 inch    66.6667\n");
	GMT_Message (API, GMT_TIME_NONE, "        240p:   426 x  240    24 x 13.5 cm    17.7500    9.6 x 5.4 inch    44.3750\n");
	GMT_Usage (API, -2, "%s Recognized 4:3-ratio names and associated dimensions:", GMT_LINE_BULLET);
	GMT_Message (API, GMT_TIME_NONE, "        name:  pixel size:    canvas (SI):       dpc:      canvas (US):       dpi:\n");
	GMT_Message (API, GMT_TIME_NONE, "       ---------------------------------------------------------------------------\n");
	GMT_Message (API, GMT_TIME_NONE, "        uxga:  1600 x 1200    24 x 18 cm      66.6667    9.6 x 7.2 inch   166.6667\n");
	GMT_Message (API, GMT_TIME_NONE, "       sxga+:  1400 x 1050    24 x 18 cm      58.3333    9.6 x 7.2 inch   145.8333\n");
	GMT_Message (API, GMT_TIME_NONE, "         xga:  1024 x  768    24 x 18 cm      42.6667    9.6 x 7.2 inch   106.6667\n");
	GMT_Message (API, GMT_TIME_NONE, "        svga:   800 x  600    24 x 18 cm      33.3333    9.6 x 7.2 inch    83.3333\n");
	GMT_Message (API, GMT_TIME_NONE, "         dvd:   640 x  480    24 x 18 cm      26.6667    9.6 x 7.2 inch    66.6667\n");
	GMT_Usage (API, -2, "Note: uhd-2 and 8k can be used for 4320p, uhd and 4k for 2160p, and fhd or hd for 1080p. "
		"Current PROJ_LENGTH_UNIT determines if you get SI or US canvas dimensions and dpu. "
		"Alternatively, set a custom canvas with dimensions and dots-per-unit manually by "
		"providing <width>x<height>x<dpu> (e.g., 15cx10cx50, 6ix6ix100, etc.).");
	GMT_Usage (API, 1, "\n-N<prefix>");
	GMT_Usage (API, -2, "Set the <prefix> used for movie files and directory names. "
		"The directory cannot already exist; see -Z to remove such directories at the end.");
	GMT_Usage (API, 1, "\n-T<nframes>|<min>/<max>/<inc>[+n]|<timefile>[+p<width>][+s<first>][+w[<str>]|W]");
	GMT_Usage (API, -2, "Set number of frames, create times from <min>/<max>/<inc>[+n] or give file with frame-specific information. "
		"If <min>/<max>/<inc> is used then +n is used to indicate that <inc> is in fact number of frames instead. "
		"If <timefile> does not exist it must be created by the background script given via -Sb.");
	GMT_Usage (API, 3, "+p Append number of digits used in creating the frame tags [automatic].");
	GMT_Usage (API, 3, "+s Append <first> to change the value of the first frame [0].");
	GMT_Usage (API, 3, "+w Let trailing text in <timefile> be split into individual word variables. "
		"We use space or TAB as separators; append <str> to set custom characters as separators instead.");
	GMT_Usage (API, 3, "+W Same as +w but only use TAB as separator.");
	GMT_Message (API, GMT_TIME_NONE, "\n  OPTIONAL ARGUMENTS:\n");
	GMT_Usage (API, 1, "\n-A<audiofile>[+e]");
	GMT_Usage (API, -2, "Merge in an audio track, starting at first frame.");
	GMT_Usage (API, 3, "+e Adjust length of audio track to fit the length of the movie exactly.");
	GMT_Usage (API, 1, "\n-D<rate>");
	GMT_Usage (API, -2, "Set movie display frame rate in frames/second [24].");
	GMT_Usage (API, 1, "\n-E<titlepage>[+d[<duration>[s]]][+f[i|o][<fade>[s]]][+g<fill>]");
	GMT_Usage (API, -2, "Give name of optional <titlepage> script to build title page displayed before the animation [no title page]. "
		"Alternatively, give PostScript file of correct canvas size that will be the title page.");
	GMT_Usage (API, 3, "+d Set duration of the title frames (append s for seconds) [4s].");
	GMT_Usage (API, 3, "+f Fade in and out over the title via black [1s]. "
		"Use +fi and/or +fo to set unequal fade lengths or to just select one of them.");
	GMT_Usage (API, 3, "+g Select another terminal fade fill than black.");
	GMT_Usage (API, 1, "\n-Fgif|mp4|webm|png[+l[<n>]][+o<opts>][+s<stride>][+t]");
	GMT_Usage (API, -2, "Select the desired video format(s) from available choices. Repeatable:");
	GMT_Usage (API, 3, "%s gif: Make and convert PNG frames into an animated GIF.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s mp4: Make and convert PNG frames into an MP4 movie.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s webm: Make and convert PNG frames into an WebM movie.", GMT_LINE_BULLET);
	GMT_Usage (API, 3, "%s png: Just make the PNG frames.", GMT_LINE_BULLET);
	GMT_Usage (API, -2, "Note: gif|mp4|webm all imply png. Two modifiers are available for mp4 or webm:");
	GMT_Usage (API, 3, "+o Append custom FFmpeg encoding options (in quotes) [none].");
	GMT_Usage (API, 3, "+t Build transparent images [opaque].");
	GMT_Usage (API, 3, "+v Open the movie in the default viewer when completed.");
	GMT_Usage (API, -2, "Two modifiers are available for gif:");
	GMT_Usage (API, 3, "+l Enable looping [no loop]; optionally append number of loops [infinite loop].");
	GMT_Usage (API, 3, "+s Set stride: If -Fmp4|webm is also used you may restrict the GIF animation to use every <stride> frame only [all]. "
		"<stride> must be taken from the list 2, 5, 10, 20, 50, 100, 200, or 500.");
	GMT_Usage (API, -2, "Default is no video products; this requires -M.");
	GMT_Usage (API, 1, "\n-G[<fill>][+p<pen>]");
	GMT_Usage (API, -2, "Set the canvas background color [none].  Append +p<pen> to draw canvas outline [none].");
	GMT_Usage (API, 1, "\n-H<scale>");
	GMT_Usage (API, -2, "Temporarily increase <dpu> by <scale>, rasterize, then downsample [no downsampling]. "
		"Stabilizes sub-pixel changes between frames, such as moving text and lines.");
	GMT_Usage (API, 1, "\n-I<includefile>");
	GMT_Usage (API, -2, "Include a script file to be inserted into the movie_init.sh script [none]. "
		"Used to add constant variables needed by all movie scripts.");
	GMT_Usage (API, 1, "\n-K[+f[i|o][<fade>[s]]][+g<fill>][+p[i|o]]");
	GMT_Usage (API, -2, "Fade in and out over the main animation via black [no fading]:");
	GMT_Usage (API, 3, "+f Set duration of fading in frames (append s for seconds) [1s]. "
		"Use +fi and/or +fo to set unequal fade times or to just select one of them.");
	GMT_Usage (API, 3, "+p Preserve all frames by repeated use of first and last frames during fade sequence [Fade all frames at start and end of movie]. "
		"Append i or o to only have the in- or out-fade repeat a single frame [both ends].");
	GMT_Usage (API, 3, "+g Select terminal fade fill [black].");
	GMT_Usage (API, 1, "\n-L<labelinfo>");
	GMT_Usage (API, -2, "Automatic labeling of frames; repeatable (max 32).  Places chosen label at the frame perimeter:");
	GMT_Usage (API, 3, "e: Select elapsed time as label. Append +s<scl> to set time in sec per frame [1/<framerate>].");
	GMT_Usage (API, 3, "f: Select running frame numbers as label.");
	GMT_Usage (API, 3, "p: Select percent of completion as label.");
	GMT_Usage (API, 3, "s: Select fixed text <string> as label.");
	GMT_Usage (API, 3, "c: Use the value in appended column <col> of <timefile> (first column is 0).");
	GMT_Usage (API, 3, "t: Use word number <col> from the trailing text in <timefile> (first word is 0).");
	GMT_Usage (API, -2, "Several modifiers control label appearance:");
	GMT_Usage (API, 3, "+c Set clearance <dx>[/<dy>] between label and surrounding box.  Only "
		"used if +g or +p are set.  Append units {%s} or %% of fontsize [%d%%].", GMT_DIM_UNITS_DISPLAY, GMT_TEXT_CLEARANCE);
	GMT_Usage (API, 3, "+f Set the font used for the label [%s].", gmt_putfont (API->GMT, &API->GMT->current.setting.font_tag));
	GMT_Usage (API, 3, "+g Fill the label textbox with specified <fill> [no fill].");
	GMT_Usage (API, 3, "+h Set offset label textbox shade color fill (requires +g also). "
		"Append <dx>/<dy> to change offset [%gp/%gp] and/or <shade> to change the shade [gray50].", GMT_FRAME_CLEARANCE, -GMT_FRAME_CLEARANCE);
	GMT_Usage (API, 3, "+j Specify where the label should be plotted [TL].");
	GMT_Usage (API, 3, "+o Offset label by <dx>[/<dy>] in direction implied by <justify> [%d%% of font size].", GMT_TEXT_OFFSET);
	GMT_Usage (API, 3, "+p Draw the outline of the textbox using selected pen [no outline].");
	GMT_Usage (API, 3, "+r Select a rounded rectangular textbox (requires +g or +p) [rectangular].");
	GMT_Usage (API, 3, "+t Provide a C-format statement to be used with the item selected [none].");
	GMT_Usage (API, 1, "\n-M[<frame>|f|m|l,][<format>][+r<dpu>][+v]");
	GMT_Usage (API, -2, "Create a master frame plot as well; append comma-separated frame number [0] and format [pdf]. "
		"Master plot will be named <prefix>.<format> and placed in the current directory. "
		"Instead of frame number you can specify f(irst), m(iddle), or l(last) frame.");
		GMT_Usage (API, 3, "+r For a raster master frame you may optionally select another <dpu> [same as movie].");
		GMT_Usage (API, 3, "+v Open the master frame in the default viewer.");
	GMT_Usage (API, 1, "\n-P<progressinfo>");
	GMT_Usage (API, -2, "Automatic plotting of progress indicator(s); repeatable (max 32).  Places chosen indicator at frame perimeter. "
		"Append desired indicator (a-f) [a] and consult the movie documentation for which attributes are needed:");
	GMT_Usage (API, 3, "+a Add annotations via e|f|p|c<col> (see -L for details).");
	GMT_Usage (API, 3, "+f Set font attributes for the label [%s].", gmt_putfont (API->GMT, &API->GMT->current.setting.font_annot[GMT_SECONDARY]));
	GMT_Usage (API, 3, "+G Set background (static) color fill for indicator [Depends in indicator selected].");
	GMT_Usage (API, 3, "+g Set foreground (moving) color fill for indicator [Depends in indicator selected].");
	GMT_Usage (API, 3, "+j Specify where the indicator should be plotted [TR for circles, BC for axes].");
	GMT_Usage (API, 3, "+o Offset indicator by <dx>[/<dy>] in direction implied by <justify> [%d%% of font size].", GMT_TEXT_OFFSET);
	GMT_Usage (API, 3, "+P Set background (static) pen for indicator [Depends in indicator selected].");
	GMT_Usage (API, 3, "+p Set foreground (moving) pen for indicator [Depends in indicator selected].");
	GMT_Usage (API, 3, "+s Compute elapsed time as frame counter times appended <scale> [no scaling].");
	GMT_Usage (API, 3, "+t Provide a C-format statement to be used with the item selected [none].");
	GMT_Usage (API, 3, "+w Specify indicator size [5%% of max canvas dimension for circles, 60%% for axes].");
	GMT_Usage (API, 1, "\n-Q[s]");
	GMT_Usage (API, -2, "Debugging: Leave all intermediate files and directories behind for inspection. "
		"Append s to only create the work scripts but none will be executed (except for background script).");
	GMT_Usage (API, 1, "\n-Sb<background>");
	GMT_Usage (API, -2, "Append name of background GMT modern script that may compute "
		"files needed by <mainscript> and/or build a static background plot layer. "
		"If a plot is generated then the script must be in GMT modern mode. "
		"Alternatively, give PostScript file of correct canvas size that will be the background.");
	GMT_Usage (API, 1, "\n-Sf<foreground>");
	GMT_Usage (API, -2, "Append name of foreground GMT modern mode script which will "
		"build a static foreground plot overlay appended to all frames. "
		"Alternatively, give PostScript file of correct canvas size that will be the foreground.");
	GMT_Option (API, "V");
	GMT_Usage (API, 1, "\n-W[<dir>]");
	GMT_Usage (API, -2, "Give <dir> where temporary files will be built [Default <dir> = <prefix> set by -N]. "
		"If <dir> is not given we create one in the system temp directory named <prefix> (from -N).");
	GMT_Usage (API, 1, "\n-Z[s]");
	GMT_Usage (API, -2, "Erase directory <prefix> after converting to movie [leave directory with PNGs alone]. "
		"Append s to also delete all input scripts (mainscript and any files via -E, -I, -S).");
	GMT_Option (API, "f");
	/* Number of threads (re-purposed from -x in GMT_Option since this local option is always available and we are not using OpenMP) */
	GMT_Usage (API, 1, "\n-x[[-]<n>]");
	GMT_Usage (API, -2, "Limit the number of cores used in frame generation [Default uses all %d cores]. "
		"If <n> is negative then we select (%d - <n>) cores (or at least 1). Note: 1 core is used by movie itself.", API->n_cores, API->n_cores);
	GMT_Option (API, ".");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL void movie_set_default_width (struct GMT_CTRL *GMT, struct MOVIE_CTRL *Ctrl, struct MOVIE_ITEM *I) {
	double def_width = (strchr ("defDEF", I->kind) && (I->justify == PSL_ML || I->justify == PSL_MR)) ? Ctrl->C.dim[GMT_Y] : Ctrl->C.dim[GMT_X];
	if (I->width > 0.0) return;
	/* Assign default widths */
	I->width = (strchr ("abcABC", I->kind)) ? 0.05 * def_width : 0.6 * def_width;
	GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "No width given for progress indicator %c. Setting width to %g%c.\n", I->kind, I->width, Ctrl->C.unit);
	if (Ctrl->C.unit == 'c') I->width /= 2.54; else if (Ctrl->C.unit == 'p') I->width /= 72.0;	/* Now in inches */
}

GMT_LOCAL unsigned int movie_get_item_default (struct GMT_CTRL *GMT, struct MOVIE_CTRL *Ctrl, char *arg, struct MOVIE_ITEM *I) {
	unsigned int n_errors = 0;
	/* Default progress indicator: Pie-wedge with different colors */
	movie_set_default_width (GMT, Ctrl, I);	/* Initialize progress indicator width if not set */
	if (I->fill[0] == '-') /* Give default color for foreground wedge */
		strcpy (I->fill, "lightred");
	if (gmt_get_modifier (arg, 'G', I->fill2) && I->fill2[0]) {	/* Found +G<fill> */
		struct GMT_FILL fill;	/* Only used to make sure fill is given with correct syntax */
		if (gmt_getfill (GMT, I->fill2, &fill)) n_errors++;
	}
	if (I->fill2[0] == '-') /* Give default fixed circle color */
		strcpy (I->fill2, "lightgreen");
	if (I->kind == 'A') {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Progress indicator a does not place any labels. modifier +a ignored\n");
			I->kind = 'a';
	}
	return (n_errors);
}

GMT_LOCAL unsigned int movie_get_item_two_pens (struct GMT_CTRL *GMT, struct MOVIE_CTRL *Ctrl, char *arg, struct MOVIE_ITEM *I) {
	unsigned int n_errors = 0;
	struct GMT_PEN pen;	/* Only used to make sure any pen is given with correct syntax */
	char kind = tolower (I->kind);
	gmt_M_memset (&pen, 1, struct GMT_PEN);
	movie_set_default_width (GMT, Ctrl, I);	/* Initialize progress indicator width if not set */
	if (I->pen[0] == '-') {	/* Set pen for foreground (changing) feature */
		switch (kind) {
			case 'b': sprintf (I->pen, "%gp,blue", 0.1 * rint (I->width * 1.5 * 72.0)); break; /* Give default moving ring pen width (15% of width) and blue color */
			case 'c': sprintf (I->pen, "%gp,red", 0.1 * rint (I->width * 0.5 * 72.0)); break; /* Give default moving math angle pen width (5% of width) and red color */
			case 'd': sprintf (I->pen, "%gp,yellow", 0.1 * MIN (irint (I->width * 0.05 * 72.0), 80)); break; /* Give default crossbar pen width (0.5% of length) and color yellow */
			case 'e': sprintf (I->pen, "%gp,red", 0.1 * MIN (rint (I->width * 0.25 * 72.0), 80)); break;	/* Give a variable pen thickness >= 8p in red */
		}
	}
	if (gmt_get_modifier (arg, 'P', I->pen2) && I->pen2[0]) {	/* Found +P<pen> */
		if (gmt_getpen (GMT, I->pen2, &pen)) n_errors++;
	}
	if (I->pen2[0] == '-') {	/* Set pen for background (static) feature */
		switch (kind) {
			case 'b': sprintf (I->pen2, "%gp,lightblue", 0.1 * rint (I->width * 1.5 * 72.0)); break; /* Give default static ring pen width (15% of width) and color lightblue */
			case 'c': sprintf (I->pen2, "%gp,darkred,-", 0.1 * rint (I->width * 0.1 * 72.0)); break; /* Give default static ring dashed pen width (1% of width) and color darkred */
			case 'd': sprintf (I->pen2, "%gp,black", 0.1 * MIN (irint (I->width * 0.25 * 72.0), 80)); break;	/* Give a variable pen thickness <= 8p in black */
			case 'e': sprintf (I->pen2, "%gp,lightgreen", 0.1 * MIN (irint (I->width * 0.25 * 72.0), 80)); break;/* Give a variable pen thickness <= 8p in lightgreen */
		}
	}
	return (n_errors);
}

GMT_LOCAL unsigned int movie_get_item_pen_fill (struct GMT_CTRL *GMT, struct MOVIE_CTRL *Ctrl, char *arg, struct MOVIE_ITEM *I) {
	unsigned int n_errors = 0;
	gmt_M_unused (GMT);
	gmt_M_unused (arg);
	/* Default progress indicator: line and filled symbol */
	movie_set_default_width (GMT, Ctrl, I);	/* Initialize progress indicator width if not set */
	if (I->pen2[0] == '-')	/* Give default static line pen thickness <= 3p in black */
			sprintf (I->pen2, "%gp,black", 0.1 * MIN (irint (I->width * 0.15 * 72.0), 30));
	if (I->fill[0] == '-')	/* Give default moving triangle the red color */
		strcpy (I->fill, "red"); /* Give default moving color */
	return (n_errors);
}

GMT_LOCAL unsigned int movie_parse_common_item_attributes (struct GMT_CTRL *GMT, char option, char *arg, struct MOVIE_ITEM *I) {
	/* Initialize and parse the modifiers for item attributes for both labels and progress indicators */
	unsigned int n_errors = 0;
	char *c = NULL, *t = NULL, string[GMT_LEN128] = {""}, placement[4] = {""};
	char *allowed_mods = (option == 'L') ? "cfghjoprst" : "acfgGjopPstw";	/* Tolerate +G+P for progress bars even if not processed in this function */
	struct GMT_FILL fill;	/* Only used to make sure any fill is given with correct syntax */
	struct GMT_PEN pen;	/* Only used to make sure any pen is given with correct syntax */

	I->fill[0] = I->fill2[0] = I->sfill[0] = I->pen[0] = I->pen2[0] = '-';	/* No fills or pens set yet */
	I->off[GMT_X] = I->off[GMT_Y] = 0.01 * GMT_TEXT_OFFSET * GMT->current.setting.font_tag.size / PSL_POINTS_PER_INCH; /* 20% offset of TAG font size */
	I->clearance[GMT_X] = I->clearance[GMT_Y] = 0.01 * GMT_TEXT_CLEARANCE * GMT->current.setting.font_tag.size / PSL_POINTS_PER_INCH;	/* 15% of TAG font size */
	if (I->kind == 'L')	/* Default label placement is top left of canvas */
		I->justify = PSL_TL;
	else if (strchr ("abc", I->kind))	/* Default circular progress indicator placement is top right of canvas */
		I->justify = PSL_TR;
	else 	/* Default line progress indicator placement is bottom center of canvas */
		I->justify = PSL_BC;

		/* The modifiers for labels and progress bars are not identical, so we need to be specific here */

	/* Check for:
	 *	1) common modifiers [+c<dx/dy>][+f<fmt>][+g<fill>][+j<justify>][+o<dx/dy>][+p<pen>][+s<scl>][+t<fmt>]
	 *  2) the extra [+h[<dx>/<dy>/][<shade>]][+r] for labels
	 *  3) the extra +a<labelinfo>+w<width> for progress bars
	 */
	if (gmt_validate_modifiers (GMT, arg, option, allowed_mods, GMT_MSG_ERROR)) n_errors++;
	if (gmt_get_modifier (arg, 'c', string) && string[0])	/* Clearance for a text box */
		if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, I->clearance) < 0) n_errors++;
	if (gmt_get_modifier (arg, 'f', string)) {	/* Gave a separate font for labeling */
		if (!string[0]) {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -%c: +f not given any font\n", option);
			n_errors++;
		}
		else
			n_errors += gmt_getfont (GMT, string, &(I->font));
	}
	if (gmt_get_modifier (arg, 'g', I->fill) && I->fill[0]) {	/* Primary fill color */
		if (gmt_getfill (GMT, I->fill, &fill)) n_errors++;
	}
	if (gmt_get_modifier (arg, 'r', string))	/* Rounded text box */
		I->box = 4;
	if (gmt_get_modifier (arg, 'h', string)) {	/* Shaded text box fill color +h[<dx>/<dy>/][<shade>] */
		strcpy (I->sfill, "gray50");	/* Default shade color */
		I->soff[GMT_X] = GMT->session.u2u[GMT_PT][GMT_INCH] * GMT_FRAME_CLEARANCE;	/* Default is 4p */
		I->soff[GMT_Y] = -I->soff[GMT_X];	/* Set the shadow offsets [default is (4p, -4p)] */
		I->box++;	/* Rectangular shade = 1 and rounded rectangular shade = 5 */
		if (I->fill[0] == '-') {
			GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -%c: Modifier +h requires +g as well\n", option);
			n_errors++;
		}
		else if (string[0]) {	/* Gave an argument to +h */
			char txt_a[GMT_LEN64] = {""}, txt_b[GMT_LEN64] = {""}, txt_c[GMT_LEN64] = {""};
			int n = sscanf (string, "%[^/]/%[^/]/%s", txt_a, txt_b, txt_c);
			if (n == 1)	/* Just got a new fill */
				strcpy (I->sfill, txt_a);
			else if (n == 2) {	/* Just got a new offset */
				if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, I->soff) < 0) n_errors++;
			}
			else if (n == 3) {	/* Got offset and fill */
				I->soff[GMT_X] = gmt_M_to_inch (GMT, txt_a);
				I->soff[GMT_Y] = gmt_M_to_inch (GMT, txt_b);
				strcpy (I->sfill, txt_c);
			}
			else n_errors++;
		}
		if (gmt_getfill (GMT, I->sfill, &fill)) n_errors++;
	}
	if (gmt_get_modifier (arg, 'j', placement)) {	/* Placement on canvas for this item */
		if (I->kind == 'L')	/* Default label placement is top left of canvas */
			gmt_just_validate (GMT, placement, "TL");
		else if (strchr ("abc", I->kind))	/* Default circular progress indicator placement is top right of canvas */
			gmt_just_validate (GMT, placement, "TR");
		else 	/* Default line progress indicator placement is bottom center of canvas */
			gmt_just_validate (GMT, placement, "BC");
		I->justify = gmt_just_decode (GMT, placement, PSL_NO_DEF);
	}
	if (gmt_get_modifier (arg, 'o', string))	/* Offset of refpoint */
		if (gmt_get_pair (GMT, string, GMT_PAIR_DIM_DUP, I->off) < 0) n_errors++;
	if (gmt_get_modifier (arg, 'p', I->pen) && I->pen[0]) {	/* Primary pen */
		if (gmt_getpen (GMT, I->pen, &pen)) n_errors++;
	}
	if (gmt_get_modifier (arg, 't', I->format) && !I->format[0]) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -%c: +t not given any format\n", option);
		n_errors++;
	}

	if (I->kind == 'L') {	/* This is a label item with type the first letter in arg */
		if ((c = strchr (arg, '+'))) c[0] = '\0';	/* Chop off all modifiers for now */
		t = arg;	/* Start of required label type information */
		I->mode = MOVIE_LABEL_IS_FRAME;	/* Frame number is default for -L just in case nothing is set */
	}
	else if ((t = strstr (arg, "+a")))	/* A progress indicator wants to be labeled */
		t += 2;		/* Start of optional progress indicator time information following +a */

	if (t) {	/* Parse attributes specific to the movie labels */
		switch (t[0]) {	/* We got some */
			case 'c':	I->mode = MOVIE_LABEL_IS_COL_C;	I->col = atoi (&t[1]);	break;
			case 't':	I->mode = MOVIE_LABEL_IS_COL_T;	I->col = atoi (&t[1]);	break;
			case 'e':	I->mode = MOVIE_LABEL_IS_ELAPSED;	break;
			case 'p':	I->mode = MOVIE_LABEL_IS_PERCENT;	break;
			case 's':	I->mode = MOVIE_LABEL_IS_STRING;	strncpy (I->format, &t[1], GMT_LEN128-1); break;
			case 'f': case '\0': I->mode = MOVIE_LABEL_IS_FRAME;	break;	/* Frame number is default */
			default:	/* Not recognized argument */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -%c: Select label flag e|f|p|s, c<col> or t<col>\n", option);
				n_errors++;
				break;
		}
		I->kind = toupper ((int)I->kind);	/* Use upper case B-F to indicate that labeling is requested */
		I->n_labels = (strchr ("EF", I->kind)) ? 2 : 1;
		if (I->mode == MOVIE_LABEL_IS_ELAPSED && gmt_get_modifier (arg, 'z', string)) {
			/* Gave frame time length-scale */
			I->scale = atof (string);
		}
	}
	if (c) c[0] = '+';	/* Restore the modifiers */
	return (n_errors);
}

GMT_LOCAL unsigned int movie_get_n_frames (struct GMT_CTRL *GMT, char *txt, double framerate, char *def) {
	/* Convert user argument in frames or seconds to frames */
	char *p = (!txt || !txt[0]) ? def : txt;	/* Get frames or times in seconds */
	double fval = atof (p);	/* Get frames or times in seconds */
	gmt_M_unused (GMT);
	if (strchr (p, 's')) fval *= framerate;	/* Convert from seconds to nearest number of frames */
	return (urint (fval));
}

static int parse (struct GMT_CTRL *GMT, struct MOVIE_CTRL *Ctrl, struct GMT_OPTION *options) {

	/* This parses the options provided to movie and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 */

	unsigned int n_errors = 0, n_files = 0, k, pos, mag, T, frames;
	int n;
	bool do_view = false;
	char txt_a[GMT_LEN32] = {""}, txt_b[GMT_LEN32] = {""}, arg[GMT_LEN64] = {""}, p[GMT_LEN256] = {""};
	char *c = NULL, *s = NULL, *o = NULL, string[GMT_LEN128] = {""};
	double width = 24.0, height16x9 = 13.5, height4x3 = 18.0, dpu = 160.0;	/* SI values for dimensions and dpu */
	struct GMT_FILL fill;	/* Only used to make sure any fill is given with correct syntax */
	struct GMT_PEN pen;	/* Only used to make sure any pen is given with correct syntax */
	struct MOVIE_ITEM *I = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	if (GMT->current.setting.proj_length_unit == GMT_INCH) {	/* Switch from SI to US dimensions in inches given format names */
		width = 9.6;	height16x9 = 5.4;	height4x3 = 7.2;	dpu = 400.0;
		Ctrl->C.unit = 'i';
	}

	for (opt = options; opt; opt = opt->next) {	/* Look for -D first since frame rate is needed when parsing -E */
		if (opt->option == 'D') {	/* Display frame rate of movie */
			Ctrl->D.active = true;
			Ctrl->D.framerate = atof (opt->arg);
		}
	}

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input file */
				if (n_files++ > 0) break;
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->In.file));
				break;

			case 'A':	/* Audio track (but also backwards compatible Animated GIF [Deprecated]) */
				if (opt->arg[0] && (c = gmt_first_modifier (GMT, opt->arg, "ls")) == NULL) {	/* New audio syntax option -A<audiofile>[+e] */
					n_errors += gmt_M_repeated_module_option (API, Ctrl->A.active);
					if ((c = strstr (opt->arg, "+e"))) {	/* Stretch audio to fit video length */
						Ctrl->A.exact = true;
						c[0] = '\0';	/* Remove modifier */
					}
					Ctrl->A.file = strdup (opt->arg);	/* Get audio file name */
					if (c) c[0] = '+';	/* Restore modifier */
				}
				else if (gmt_M_compat_check (GMT, 6)) {	/* GMT6 compatibility allows backwards compatible -A[+l<loop>][+s<stride>] for animated GIF */
					GMT_Report (API, GMT_MSG_COMPAT, "Option -A[+l<loop>][+s<stride>] is deprecated - use -F instead\n");
					Ctrl->F.active[MOVIE_GIF] = Ctrl->F.active[MOVIE_PNG] = Ctrl->animate = true;	/* Old -A implies -Fpng */
					if ((c = gmt_first_modifier (GMT, opt->arg, "ls"))) {	/* Process any modifiers */
						pos = 0;	/* Reset to start of new word */
						while (gmt_getmodopt (GMT, 'A', c, "ls", &pos, p, &n_errors) && n_errors == 0) {
							switch (p[0]) {
								case 'l':	/* Specify loops */
									Ctrl->F.loop = true;
									Ctrl->F.loops = (p[1]) ? atoi (&p[1]) : 0;
									break;
								case 's':	/* Specify GIF stride, 2,5,10,20,50,100,200,500 etc. */
									Ctrl->F.skip = true;
									Ctrl->F.stride = atoi (&p[1]);
									mag = urint (pow (10.0, floor (log10 ((double)Ctrl->F.stride))));
									k = Ctrl->F.stride / mag;
									if (!(k == 1 || k == 2 || k == 5)) {
										GMT_Report (API, GMT_MSG_ERROR, "Option -A+s: Allowable strides are 2,5,10,20,50,100,200,500,...\n");
										n_errors++;
									}
									break;
								default:
									break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
							}
						}
						c[0] = '\0';
					}
				}
				else
					n_errors += gmt_default_option_error (GMT, opt);
				break;

			case 'C':	/* Known frame dimension or set a custom canvas size */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->C.active);
				strncpy (arg, opt->arg, GMT_LEN64-1);	/* Get a copy... */
				gmt_str_tolower (arg);		/* ..so we can make it lower case */
				/* 16x9 formats */
				if (!strcmp (arg, "8k") || !strcmp (arg, "uhd-2") || !strcmp (arg, "uhd2") || !strcmp (arg, "4320p")) {	/* 4320x7680 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height16x9;	Ctrl->C.dim[GMT_Z] = 2.0 * dpu;
				}
				else if (!strcmp (arg, "4k") || !strcmp (arg, "uhd") || !strcmp (arg, "2160p")) {	/* 2160x3840 */
					Ctrl->C.dim[GMT_X] = width;	Ctrl->C.dim[GMT_Y] = height16x9;	Ctrl->C.dim[GMT_Z] = dpu;
				}
				else if (!strcmp (arg, "1080p") || !strcmp (arg, "fhd") || !strcmp (arg, "hd")) {	/* 1080x1920 */
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
						GMT_Report (API, GMT_MSG_ERROR, "Option -C: Requires name of a known format or give width x height x dpu string\n");
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

			case 'D':	/* Already processed but need to have a case so we can skip */
				break;

			case 'E':	/* Title/fade sequence  */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->E.active);
				if ((c = gmt_first_modifier (GMT, opt->arg, "dfg"))) {	/* Process any modifiers */
					pos = 0;	/* Reset to start of new word */
					while (gmt_getmodopt (GMT, 'E', c, "dfg", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'd':	/* Duration of entire title/fade sequence */
								Ctrl->E.duration = movie_get_n_frames (GMT, &p[1], Ctrl->D.framerate, "4s");
								break;
							case 'f':	/* In/out fades */
								k = (p[1] && strchr ("io", p[1])) ? 2 : 1;	/* Did we get a common fade or different for in/out? */
								frames = movie_get_n_frames (GMT, &p[k], Ctrl->D.framerate, "1s");	/* Get fade length in number of frames */
								if (k == 1) /* Set equal fade frames */
									Ctrl->E.fade[GMT_IN] = Ctrl->E.fade[GMT_OUT] = frames;
								else if (p[1] == 'i') /* Set input fade frames */
									Ctrl->E.fade[GMT_IN] = frames;
								else 	/* Set output fade frames */
									Ctrl->E.fade[GMT_OUT] = frames;
								break;
							case 'g':	/* Change fade color */
								if (gmt_getfill (GMT, &p[1], &fill))	/* Bad fill */
									n_errors++;
								else	/* Fill is valid, just copy verbatim */
									Ctrl->E.fill = strdup (&p[1]);
								break;
							default:
								break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					c[0] = '\0';	/* Chop off modifiers */
				}
				if (Ctrl->E.duration == 0) Ctrl->E.duration = movie_get_n_frames (GMT, NULL, Ctrl->D.framerate, "4s");
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->E.file));
				if (c) c[0] = '+';	/* Restore modifiers */
				break;

			case 'F':	/* Set movie format and optional FFmpeg options */
				if ((c = gmt_first_modifier (GMT, opt->arg, "lostv"))) {	/* Process any modifiers */
					do_view = false;
					pos = 0;	/* Reset to start of new word */
					o = NULL;
					while (gmt_getmodopt (GMT, 'F', c, "lostv", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'l':	/* Specify loops for GIF */
								Ctrl->F.loop = true;
								Ctrl->F.loops = (p[1]) ? atoi (&p[1]) : 0;
								break;
							case 'o':	/* FFmpeg option to pass along */
								o = strdup (&p[1]);	/* Retain start of encoding options for later */
								break;
							case 's':	/* Specify GIF stride, 2,5,10,20,50,100,200,500 etc. */
								Ctrl->F.skip = true;
								Ctrl->F.stride = atoi (&p[1]);
								mag = urint (pow (10.0, floor (log10 ((double)Ctrl->F.stride))));
								k = Ctrl->F.stride / mag;
								if (!(k == 1 || k == 2 || k == 5)) {
									GMT_Report (API, GMT_MSG_ERROR, "Option -F+s: Allowable strides are 2,5,10,20,50,100,200,500,...\n");
									n_errors++;
								}
								break;
							case 't':	/* Transparent images */
								Ctrl->F.transparent = true;
								break;
							case 'v':	/* Open video in viewer */
								do_view = true;
								break;
							default:
								break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					c[0] = '\0';	/* Chop off modifiers */
				}
				strncpy (arg, opt->arg, GMT_LEN64-1);	/* Get a copy of the args (minus encoding options)... */
				gmt_str_tolower (arg);	/* ...so we can convert it to lower case for comparisons */
				if (!strcmp (opt->arg, "none")) {	/* Do not make those PNGs at all, just a master plot */
					if (gmt_M_compat_check (GMT, 6)) {	/* GMT6 compatibility allows -Fnone */
						GMT_Report (API, GMT_MSG_COMPAT, "Option -Fnone is deprecated, it is the default action\n");
						Ctrl->M.exit = true;
					}
					else {
						GMT_Report (API, GMT_MSG_ERROR, "Option -F: Unrecognized format %s\n", opt->arg);
						n_errors++;
					}
					break;
				}
				if (!strcmp (opt->arg, "png"))	/* Just make those PNGs */
					k = MOVIE_PNG;
				else if (!strcmp (opt->arg, "gif"))	/* Make animated GIF */
					k = MOVIE_GIF;
				else if (!strcmp (opt->arg, "mp4"))	/* Make a MP4 movie */
					k = MOVIE_MP4;
				else if (!strcmp (opt->arg, "webm"))	/* Make a WebM movie */
					k = MOVIE_WEBM;
				else if (opt->arg[0]) {	/* Gave another argument which is invalid */
					GMT_Report (API, GMT_MSG_ERROR, "Option -F: Unrecognized format %s\n", opt->arg);
					n_errors++;
					break;
				}
				if (opt->arg[0]) {	/* Gave a valid argument */
					if (Ctrl->F.active[k]) {	/* Can only select a format once */
						GMT_Report (API, GMT_MSG_ERROR, "Option -F: Format %s already selected\n", opt->arg);
						n_errors++;
						break;
					}
					/* Here we have a new video format selected */
					Ctrl->F.active[k] = true;
					Ctrl->F.view[k] = do_view;
					if (k != MOVIE_PNG) Ctrl->animate = true;
					if (o) {	/* Gave specific encoding options */
						if (Ctrl->F.options[k]) gmt_M_str_free (Ctrl->F.options[k]);	/* Free old setting first */
						Ctrl->F.options[k] = o;
					}
				}
				if (c) c[0] = '+';	/* Now we can restore the optional text we chopped off */
				break;

			case 'G':	/* Canvas fill */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->G.active);
				if ((c = strstr (opt->arg, "+p"))) {	/* Gave outline modifier */
					if (c[2] && gmt_getpen (GMT, &c[2], &pen)) {	/* Bad pen */
						gmt_pen_syntax (GMT, 'G', NULL, "+p<pen> sets pen attributes [no outline]", NULL, 0);
						n_errors++;
					}
					else	/* Pen is valid, just copy verbatim */
						strncpy (Ctrl->G.pen, &c[2], GMT_LEN64);
					c[0] = '\0';	/* Chop off options */
					Ctrl->G.mode |= 1;
				}
				if (opt->arg[0]) {	/* Gave fill argument */
					if (gmt_getfill (GMT, opt->arg, &fill))	/* Bad fill */
						n_errors++;
					else {	/* Fill is valid, just copy verbatim */
						Ctrl->G.fill = strdup (opt->arg);
						Ctrl->G.mode |= 2;
					}
				}
				if (c) c[0] = '+';	/* Now we can restore the optional text we chopped off */
				break;

			case 'H':	/* RIP at a higher dpu, then downsample in gs to improve sub-pixeling */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->H.active);
				n_errors += gmt_get_required_sint (GMT, opt->arg, opt->option, 0, &Ctrl->H.factor);
				break;

			case 'I':	/* Include file with settings used by all scripts */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->I.active);
				n_errors += gmt_get_required_file (GMT, opt->arg, opt->option, 0, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->I.file));
				break;

			case 'K':	/* Fade from/to a black background -K[+f[i|o][<fade>[s]]][+g<fill>][+p[i|o]] */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->K.active);
				frames = 0;
				if ((c = gmt_first_modifier (GMT, opt->arg, "fgp"))) {	/* Process any modifiers */
					pos = 0;	/* Reset to start of new word */
					while (gmt_getmodopt (GMT, 'K', c, "fgp", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'f':	/* In/out fades */
								k = (p[1] && strchr ("io", p[1])) ? 2 : 1;	/* Did we get a common fade or different for in/out? */
								frames = movie_get_n_frames (GMT, &p[k], Ctrl->D.framerate, "1s");	/* Get duration in frames */
								if (k == 1) /* Set equal length in and out fades */
									Ctrl->K.fade[GMT_IN] = Ctrl->K.fade[GMT_OUT] = frames;
								else if (p[1] == 'i') /* Set input fade frames */
									Ctrl->K.fade[GMT_IN] = frames;
								else 	/* Set output fade frames */
									Ctrl->K.fade[GMT_OUT] = frames;
								break;
							case 'g':	/* Change fade color */
								if (gmt_getfill (GMT, &p[1], &fill))	/* Bad fill */
									n_errors++;
								else	/* Fill is valid, just copy verbatim */
									Ctrl->K.fill = strdup (&p[1]);
								break;
							case 'p':	/* Persistent fade frame */
								k = (p[1] && strchr ("io", p[1])) ? 2 : 1;	/* Did we get a common persistence setting or different for in/out? */
								if (k == 1) /* Set preserve frames for both start and end of movie */
									Ctrl->K.preserve[GMT_IN] = Ctrl->K.preserve[GMT_OUT] = true;
								else if (p[1] == 'i') /* Set start frame preserve */
									Ctrl->K.preserve[GMT_IN] = true;
								else 	/* Set end frame preserve */
									Ctrl->K.preserve[GMT_OUT] = true;
								break;
							default:
								break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}
					c[0] = '\0';	/* Chop off modifiers */
				}
				if (frames == 0) Ctrl->K.fade[GMT_IN] = Ctrl->K.fade[GMT_OUT] = movie_get_n_frames (GMT, NULL, Ctrl->D.framerate, "1s"); /* Set both fades  to default*/
				if (c) c[0] = '+';	/* Restore modifier */
				break;

			case 'L':	/* Label frame and get attributes (repeatable) */
				Ctrl->L.active = Ctrl->item_active[MOVIE_ITEM_IS_LABEL] = true;
				if ((T = Ctrl->n_items[MOVIE_ITEM_IS_LABEL]) == GMT_LEN32) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -L: Cannot handle more than %d tags\n", GMT_LEN32);
					n_errors++;
					break;
				}
				I = &Ctrl->item[MOVIE_ITEM_IS_LABEL][T];	/* Shorthand for current label item */
				I->kind = 'L';			/* This is a label item */
				n_errors += movie_parse_common_item_attributes (GMT, 'L', opt->arg, I);
				Ctrl->n_items[MOVIE_ITEM_IS_LABEL]++;	/* One more label specified */
				break;

			case 'M':	/* Create a single frame plot as well as movie (unless -Q is active) */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->M.active);
				if ((c = gmt_first_modifier (GMT, opt->arg, "rv"))) {	/* Process any modifiers */
					pos = 0;	/* Reset to start of new word */
					while (gmt_getmodopt (GMT, 'M', c, "rv", &pos, p, &n_errors) && n_errors == 0) {	
						switch (p[0]) {
							case 'r':	/* Gave specific resolution for master frame */
								Ctrl->M.dpu = atof (&p[2]);
								Ctrl->M.dpu_set = true;
								break;
							case 'v':	/* View master frame */
								Ctrl->M.view = true;
								break;
							default:
								break;	/* These are caught in gmt_getmodopt so break is just for Coverity */
						}
					}								
					c[0] = '\0';	/* Chop off modifiers so we just have -M[<frame>|f|l|m][,format] */
				}
				if ((s = strchr (opt->arg, ',')) ) {	/* Gave both frame and format */
					if (!strncmp (&s[1], "view", 4U))  /* Check for using 'view' to set with GMT_GRAPHICS_FORMAT */
						Ctrl->M.format = strdup (gmt_session_format[API->GMT->current.setting.graphics_format]);
					else
						Ctrl->M.format = strdup (&s[1]);
					s[0] = '\0';	/* Chop off format specification; now we have just -M[<frame>|f|l|m] */
					switch (opt->arg[0]) {
						case 'f':	Ctrl->M.frame  = 0; break;
						case 'm':	Ctrl->M.update = 1; break;
						case 'l':	Ctrl->M.update = 2; break;
						default:	Ctrl->M.frame = atoi (opt->arg); break;
					}
					s[0] = ',';	/* Restore format specification */
				}
				else if (isdigit (opt->arg[0]) || (strchr ("fml", opt->arg[0]) && opt->arg[1] == '\0')) {	/* Gave just a frame, default to PDF format */
					Ctrl->M.format = strdup ("pdf");
					switch (opt->arg[0]) {
						case 'f':	Ctrl->M.frame  = 0; break;
						case 'm':	Ctrl->M.update = 1; break;
						case 'l':	Ctrl->M.update = 2; break;
						default:	Ctrl->M.frame = atoi (opt->arg); break;
					}
				}
				else if (opt->arg[0]) {	/* Must be format only, with frame = 0 implicit */
					if (!strncmp ("view", opt->arg, 4U)) /* Check for using 'view' to set with GMT_GRAPHICS_FORMAT */
						Ctrl->M.format = strdup (gmt_session_format[API->GMT->current.setting.graphics_format]);
					else
						Ctrl->M.format = strdup (opt->arg);
				}
				else /* Default is PDF of frame 0 */
					Ctrl->M.format = strdup ("pdf");
				if (c) c[0] = '+';	/* Restore modifier */
				break;

			case 'N':	/* Movie prefix and directory name */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->N.active);
				n_errors += gmt_get_required_string (GMT, opt->arg, opt->option, 0, &Ctrl->N.prefix);
				break;

			case 'P':	/* Movie progress bar(s) (repeatable) */
				Ctrl->P.active = Ctrl->item_active[MOVIE_ITEM_IS_PROG_INDICATOR] = true;
				if ((T = Ctrl->n_items[MOVIE_ITEM_IS_PROG_INDICATOR]) == GMT_LEN32) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -P: Cannot handle more than %d progress indicators\n", GMT_LEN32);
					n_errors++;
					break;
				}
				I = &Ctrl->item[MOVIE_ITEM_IS_PROG_INDICATOR][T];	/* Shorthand for the current progress indicator item */
				I->kind = (opt->arg[0] && strchr ("abcdef", opt->arg[0])) ? opt->arg[0] : 'a';	/* This is a progress indicator item [Default to a] */
				n_errors += movie_parse_common_item_attributes (GMT, 'P', opt->arg, I);
				if (gmt_get_modifier (opt->arg, 'G', I->fill2) && I->fill2[0]) {	/* Secondary fill */
					if (gmt_getfill (GMT, I->fill2, &fill)) n_errors++;
				}
				if (gmt_get_modifier (opt->arg, 'P', I->pen2) && I->pen2[0]) {	/* Secondary pen */
					if (gmt_getpen (GMT, I->pen2, &pen)) n_errors++;
				}
				if (gmt_get_modifier (opt->arg, 'w', string))	/* Progress indicator dimension (length or width) */
					I->width = gmt_M_to_inch (GMT, string);
				switch (I->kind) {	/* Deal with any missing required attributes for each progress indicator type */
					case 'b':	case 'B':	 n_errors += movie_get_item_two_pens (GMT, Ctrl, opt->arg, I); break;	/* Progress ring */
					case 'c':	case 'C':	 n_errors += movie_get_item_two_pens (GMT, Ctrl, opt->arg, I); break;	/* Progress arrow  */
					case 'd':	case 'D':	 n_errors += movie_get_item_two_pens (GMT, Ctrl, opt->arg, I); break;	/* Progress rounded line */
					case 'e':	case 'E':	 n_errors += movie_get_item_two_pens (GMT, Ctrl, opt->arg, I); break;	/* progress line on line */
					case 'f':	case 'F':	 n_errors += movie_get_item_pen_fill (GMT, Ctrl, opt->arg, I); break;	/* Progress bar with time-axis and triangle  */
					default: n_errors += movie_get_item_default (GMT, Ctrl, opt->arg, I);  break;	/* Default pie progression circle (a)*/
				}
				if (I->kind == 'F' && I->mode == MOVIE_LABEL_IS_ELAPSED) {
					GMT_Report (API, GMT_MSG_WARNING, "Cannot handle elapsed time with progress indicator (f) yet - skipped\n");
					if (Ctrl->n_items[MOVIE_ITEM_IS_PROG_INDICATOR] == 0) Ctrl->P.active = Ctrl->item_active[MOVIE_ITEM_IS_PROG_INDICATOR] = false;
					continue;
				}
				Ctrl->n_items[MOVIE_ITEM_IS_PROG_INDICATOR]++;	/* Got one more progress indicator */
				break;

			case 'Q':	/* Debug - leave temp files and directories behind; Use -Qs to only write scripts */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Q.active);
				if (opt->arg[0] == 's') Ctrl->Q.scripts = true;
				break;

			case 'S':	/* background and foreground scripts */
				if (opt->arg[0] == 'b')
					k = MOVIE_PREFLIGHT;	/* background */
				else if (opt->arg[0] == 'f')
					k = MOVIE_POSTFLIGHT;	/* foreground */
				else {	/* Bad option */
					n_errors++;
					GMT_Report (API, GMT_MSG_ERROR, "Option -S: Select -Sb or -Sf\n");
					break;
				}
				n_errors += gmt_M_repeated_module_option (API, Ctrl->S[k].active);
				/* Got a valid f or b */
				n_errors += gmt_get_required_file (GMT, &opt->arg[1], opt->option, 0, GMT_IS_DATASET, GMT_IN, GMT_FILE_REMOTE, &(Ctrl->S[k].file));
				if (n_errors == 0 && (Ctrl->S[k].fp = fopen (Ctrl->S[k].file, "r")) == NULL) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -S%c: Unable to open file %s\n", opt->arg[0], Ctrl->S[k].file);
					n_errors++;
				}
				break;

			case 'T':	/* Number of frames or the name of file with frame information (note: file may not exist yet) */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->T.active);
				if ((c = gmt_first_modifier (GMT, opt->arg, "pswW"))) {	/* Process any modifiers */
					pos = 0;	/* Reset to start of new word */
					while (gmt_getmodopt (GMT, 'T', c, "pswW", &pos, p, &n_errors) && n_errors == 0) {
						switch (p[0]) {
							case 'p':	/* Set a fixed precision in frame naming ###### */
								Ctrl->T.precision = atoi (&p[1]);
								break;
							case 's':	/* Specify start frame other than 0 */
								Ctrl->T.start_frame = atoi (&p[1]);
								break;
							case 'w':	/* Split trailing text into words using any white space. */
								Ctrl->T.split = true;
								if (p[1]) {	/* Gave an argument, watch out for tabs given as \t */
									char *W = gmt_get_strwithtab (&p[1]);
									strncpy (Ctrl->T.sep, W, GMT_LEN8-1);
									gmt_M_str_free (W);
								}
								break;
							case 'W':	/* Split trailing text into words using only TABs. */
								Ctrl->T.split = true;
								Ctrl->T.sep[1] = '\0';	/* Truncate the space character in the list to only have TAB */
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

			case 'W':	/* Work dir where data files may be found. If not given we make one up later */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->W.active);
				if (opt->arg[0]) Ctrl->W.dir = strdup (opt->arg);
				break;

			case 'Z':	/* Erase frames after movie has been made */
				n_errors += gmt_M_repeated_module_option (API, Ctrl->Z.active);
				if (opt->arg[0] == 's') Ctrl->Z.delete = true;	/* Also delete input scripts */
				break;

			case 'x':
				n_errors += gmt_M_repeated_module_option (API, Ctrl->x.active);
				n_errors += movie_parse_x_option (GMT, Ctrl, opt->arg);
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_option_error (GMT, opt);
				break;
		}
	}

	if (Ctrl->F.active[MOVIE_GIF] || Ctrl->F.active[MOVIE_MP4] || Ctrl->F.active[MOVIE_WEBM]) Ctrl->F.active[MOVIE_PNG] = true;	/* All animations require PNGs */
	if (Ctrl->M.active && !Ctrl->F.active[MOVIE_PNG]) Ctrl->M.exit = true;	/* Only make the master frame */

	n_errors += gmt_M_check_condition (GMT, n_files != 1 || Ctrl->In.file == NULL, "Must specify a main script file\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->C.active, "Option -C: Must specify a canvas dimension\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.exit && Ctrl->animate, "Option -F: Cannot use none with other selections\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->Q.active && !Ctrl->M.active && !Ctrl->F.active[MOVIE_PNG], "Must select at least one output product (-F, -M)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.active && Ctrl->Z.active, "Cannot use -Z if -Q is also set\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->H.active && Ctrl->H.factor < 2, "Option -H: factor must be and integer > 1\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && !Ctrl->F.active[MOVIE_MP4] && !Ctrl->F.active[MOVIE_WEBM], "Option -A: Audio is only valid with -Fmp4 or -Fwebm\n");

	
	if (!Ctrl->T.split) {	/* Make sure we split text if we request word columns in the labeling */
		unsigned int n_used = 0;
		for (k = MOVIE_ITEM_IS_LABEL; k <= MOVIE_ITEM_IS_PROG_INDICATOR; k++)
			for (T = 0; T < Ctrl->n_items[k]; T++)
				if (Ctrl->item[k][T].mode == MOVIE_LABEL_IS_COL_T) n_used++;
		if (n_used) Ctrl->T.split = true;	/* Necessary setting when labels address individual words */
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->A.active && Ctrl->A.file && gmt_access (GMT, Ctrl->A.file, R_OK),
					"Option -A: Cannot read file %s!\n", Ctrl->A.file);
	n_errors += gmt_M_check_condition (GMT, gmt_set_length_unit (GMT, Ctrl->C.unit) == GMT_NOTSET,
					"Option -C: Bad unit given for canvas dimensions\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.dim[GMT_X] <= 0.0 || Ctrl->C.dim[GMT_Y] <= 0.0,
					"Option -C: Zero or negative canvas dimensions given\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.dim[GMT_Z] <= 0.0,
					"Option -C: Zero or negative canvas dots-per-unit given\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->N.active || (Ctrl->N.prefix == NULL || strlen (Ctrl->N.prefix) == 0),
					"Option -N: Must specify a movie prefix\n");
	n_errors += gmt_M_check_condition (GMT, !Ctrl->T.active,
					"Option -T: Must specify number of frames or a time file\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->T.split && Ctrl->T.sep[0] == '\0',
					"Option -T: Must specify a string of characters if using +w<str>\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && !(Ctrl->Q.active || Ctrl->F.active[MOVIE_PNG] || Ctrl->M.active),
					"Option -Z: Cannot be used without specifying a master (-M) or animation (-F) product\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.skip && !(Ctrl->F.active[MOVIE_MP4] || Ctrl->F.active[MOVIE_WEBM]),
					"Option -Fgif: Cannot specify a GIF stride > 1 without selecting a movie product (-Fmp4|webm)\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->M.active && !Ctrl->M.update && Ctrl->M.frame < Ctrl->T.start_frame,
					"Option -M: Cannot specify a frame before the first frame number set via -T\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->Z.active && Ctrl->W.active && Ctrl->W.dir && !strcmp (Ctrl->W.dir, "/tmp"),
					"Option -Z: Cannot delete working directory %s\n", Ctrl->W.dir);
	n_errors += gmt_M_check_condition (GMT, Ctrl->E.active && (Ctrl->E.fade[GMT_IN] + Ctrl->E.fade[GMT_OUT]) > Ctrl->E.duration,
					"Option -E: Combined fading duration cannot exceed title duration\n");

	if (n_errors) return (GMT_PARSE_ERROR);	/* No point going further */

	/* Note: We open script files for reading below since we are changing cwd later and sometimes need to rewind and re-read */

	if (n_files == 1) {	/* Determine scripting language from file extension and open the main script file */
		if (strstr (Ctrl->In.file, ".bash") || strstr (Ctrl->In.file, ".sh"))	/* Treat both as bash since sh is subset of bash */
			Ctrl->In.mode = GMT_BASH_MODE;
		else if (strstr (Ctrl->In.file, ".csh"))
			Ctrl->In.mode = GMT_CSH_MODE;
		else if (strstr (Ctrl->In.file, ".bat"))
			Ctrl->In.mode = GMT_DOS_MODE;
		else {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to determine script language from the extension of your script %s\n", Ctrl->In.file);
			GMT_Report (API, GMT_MSG_ERROR, "Allowable extensions are: *.sh, *.bash, *.csh, and *.bat\n");
			n_errors++;
		}
		/* Armed with script language we check that any back/fore-ground scripts are of the same kind */
		for (k = MOVIE_PREFLIGHT; !n_errors && k <= MOVIE_POSTFLIGHT; k++) {
			if (!Ctrl->S[k].active) continue;	/* Not provided */
			n_errors += gmt_check_language (GMT, Ctrl->In.mode, Ctrl->S[k].file, k, &Ctrl->S[k].PS);
		}
		if (!n_errors && Ctrl->E.active) {	/* Must also check the include file, and open it for reading */
			n_errors += gmt_check_language (GMT, Ctrl->In.mode, Ctrl->E.file, 2, &Ctrl->E.PS);
			if (n_errors == 0 && ((Ctrl->E.fp = fopen (Ctrl->E.file, "r")) == NULL)) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open title script file %s\n", Ctrl->E.file);
				n_errors++;
			}
		}
		if (!n_errors && Ctrl->I.active) {	/* Must also check the include file, and open it for reading */
			n_errors += gmt_check_language (GMT, Ctrl->In.mode, Ctrl->I.file, 3, NULL);
			if (n_errors == 0 && ((Ctrl->I.fp = fopen (Ctrl->I.file, "r")) == NULL)) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open include script file %s\n", Ctrl->I.file);
				n_errors++;
			}
		}
		/* Open the main script for reading here */
		if (n_errors == 0 && ((Ctrl->In.fp = fopen (Ctrl->In.file, "r")) == NULL)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to open main script file %s\n", Ctrl->In.file);
			n_errors++;
		}
		if (n_errors == 0 && gmt_script_is_classic (GMT, Ctrl->In.fp)) {
			GMT_Report (API, GMT_MSG_ERROR, "Your main script file %s is not in GMT modern mode\n", Ctrl->In.file);
			n_errors++;
		}
		/* Make sure all MOVIE_* variables are used with leading token ($, %) */
		if (n_errors == 0 && gmt_token_check (GMT, Ctrl->In.fp, "MOVIE_", Ctrl->In.mode)) {
			n_errors++;
		}
	}

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL void movie_close_files (struct MOVIE_CTRL *Ctrl) {
	/* Close all files when an error forces us to quit */
	fclose (Ctrl->In.fp);
	if (Ctrl->I.active) fclose (Ctrl->I.fp);
}

GMT_LOCAL int movie_delete_scripts (struct GMT_CTRL *GMT, struct MOVIE_CTRL *Ctrl) {
	/* Delete the scripts since they apparently are temporary */
	if (Ctrl->In.file && gmt_remove_file (GMT, Ctrl->In.file)) {	/* Delete the main script */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to delete the main script %s.\n", Ctrl->In.file);
		return (GMT_RUNTIME_ERROR);
	}
	if (Ctrl->E.file && gmt_remove_file (GMT, Ctrl->E.file)) {	/* Delete the title script */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to delete the title script %s.\n", Ctrl->E.file);
		return (GMT_RUNTIME_ERROR);
	}
	if (Ctrl->I.file && gmt_remove_file (GMT, Ctrl->I.file)) {	/* Delete the include script */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to delete the include script %s.\n", Ctrl->I.file);
		return (GMT_RUNTIME_ERROR);
	}
	if (Ctrl->S[MOVIE_PREFLIGHT].file && gmt_remove_file (GMT, Ctrl->S[MOVIE_PREFLIGHT].file)) {	/* Delete the background script */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to delete the background script %s.\n", Ctrl->S[MOVIE_PREFLIGHT].file);
		return (GMT_RUNTIME_ERROR);
	}
	if (Ctrl->S[MOVIE_POSTFLIGHT].file && gmt_remove_file (GMT, Ctrl->S[MOVIE_POSTFLIGHT].file)) {	/* Delete the foreground script */
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to delete the foreground script %s.\n", Ctrl->S[MOVIE_POSTFLIGHT].file);
		return (GMT_RUNTIME_ERROR);
	}
	return (GMT_NOERROR);
}

GMT_LOCAL bool movie_valid_format (char *format, char code) {
	/* Return true if the format has a format statement for code, e.g., %5.5d or %d if code = d */
	char *c = strchr (format, '%');
	if (c == NULL) return false;	/* No format whatsoever */
	c++;	/* Skip past the percentage marker */
	while (isdigit (c[0]) || c[0] == '.') c++;	/* Skip past the optional width.precision items */
	/* Here we are looking at [l]d|f|g|e|x etc */
	if (c[0] == 'l') c++;	/* Skip past any l for long */
	return (c[0] == code);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

EXTERN_MSC int GMT_movie (void *V_API, int mode, void *args) {
	int error = 0, precision, scol, srow;
	int (*run_script)(const char *);	/* pointer to system function or a dummy */

	unsigned int n_values = 0, n_frames = 0, n_data_frames, first_fade_out_frame = 0, frame, i_frame, col, p_width, p_height, k, T;
	unsigned int n_frames_not_started = 0, n_frames_completed = 0, first_i_frame = 0, data_frame, n_cores_unused, n_fade_frames = 0;
	unsigned int dd, hh, mm, ss, start, flavor[2] = {0, 0};

	bool done = false, layers = false, one_frame = false, upper_case[2] = {false, false}, has_conf = false;
	bool n_written = false, has_text = false, is_classic = false, place_background = false, issue_col0_par = false;

	static char *movie_raster_format[2] = {"png", "PNG"}, *img_type[2] = {"opaque", "transparent"}, var_token[4] = "$$%";
	static char *extension[3] = {"sh", "csh", "bat"}, *load[3] = {"source", "source", "call"}, *rmfile[3] = {"rm -f", "rm -f", "del"};
	static char *rmdir[3] = {"rm -rf", "rm -rf", "rd /s /q"}, *export[3] = {"export ", "setenv ", ""};
	static char *mvfile[3] = {"mv -f", "mv -f", "move"}, *cpconf[3] = {"\tcp -f %s .\n", "\tcp -f %s .\n", "\tcopy %s .\n"};
	static char *sys_cmd_nowait[3] = {"bash", "csh", "start /B"}, *sys_cmd_wait[3] = {"bash", "csh", "cmd /C"};

	char init_file[PATH_MAX] = {""}, state_tag[GMT_LEN16] = {""}, state_prefix[GMT_LEN128] = {""}, param_file[PATH_MAX] = {""}, cwd[PATH_MAX] = {""};
	char pre_file[PATH_MAX] = {""}, post_file[PATH_MAX] = {""}, main_file[PATH_MAX] = {""}, line[PATH_MAX] = {""}, version[GMT_LEN32] = {""};
	char string[GMT_LEN128] = {""}, extra[GMT_BUFSIZ] = {""}, cmd[PATH_MAX] = {""}, cleanup_file[PATH_MAX] = {""}, L_txt[GMT_LEN128] = {""};
	char png_file[PATH_MAX] = {""}, topdir[PATH_MAX] = {""}, workdir[PATH_MAX] = {""}, datadir[PATH_MAX] = {""}, frame_products[GMT_LEN32] = {""};
	char intro_file[PATH_MAX] = {""}, conf_file[PATH_MAX], tmpwpath[PATH_MAX] = {""}, *script_file =  NULL, which[2] = {"LP"}, spacer, dir_sep;
	char audio_option[PATH_MAX] = {""};

	double percent = 0.0, L_col = 0, sx, sy, fade_level = 0.0, audio_stretch = 0.0, dpu;

	FILE *fp = NULL;

	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_DATASET *D = NULL;
	struct GMT_OPTION *options = NULL;
	struct MOVIE_STATUS *status = NULL;
	struct MOVIE_CTRL *Ctrl = NULL;
	struct MOVIE_ITEM *I = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if ((error = gmt_report_usage (API, options, 0, usage)) != GMT_NOERROR) bailout (error);	/* Give usage if requested */

	/* Parse the command-line arguments */

	if ((GMT = gmt_init_module (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_KEYS, THIS_MODULE_NEEDS, module_kw, &options, &GMT_cpy)) == NULL) bailout (API->error); /* Save current state */
	if (GMT_Parse_Common (API, THIS_MODULE_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the movie main code ----------------------------*/

	dir_sep = (Ctrl->In.mode == GMT_DOS_MODE) ? '\\' : '/';		/* On Windows to do remove a file in a subdir one need to use back slashes */

	if (Ctrl->F.transparent) GMT_Report (API, GMT_MSG_WARNING, "Building transparent PNG images is an experimental feature\n");
	/* Determine pixel dimensions of individual images */
	dpu = Ctrl->C.dim[GMT_Z];	/* As given, suiting the provided canvas dim units */
	one_frame = (Ctrl->M.active && Ctrl->M.exit);	/* true if we want to create a single master plot only (no frames nor animations) */
	if (Ctrl->C.unit == 'c') Ctrl->C.dim[GMT_Z] *= 2.54;		/* Since gs requires dots per inch but we gave dots per cm */
	else if (Ctrl->C.unit == 'p') Ctrl->C.dim[GMT_Z] *= 72.0;	/* Since gs requires dots per inch but we gave dots per point */
	strcpy (frame_products, movie_raster_format[Ctrl->F.transparent]);	/* psconvert code for the desired PNG image type */

	for (k = MOVIE_ITEM_IS_LABEL; k <= MOVIE_ITEM_IS_PROG_INDICATOR; k++) {
		/* Compute auto label placement */
		for (T = 0; T < Ctrl->n_items[k]; T++) {
			I = &Ctrl->item[k][T];	/* Shorthand for this item */
			scol = (I->justify % 4) - 1;	/* Split the 2-D justify code into x just 0-2 */
			srow = I->justify / 4;			/* Split the 2-D justify code into y just 0-2 */
			/* Must compute x,y in inches since off is in inches already */
			I->x = 0.5 * Ctrl->C.dim[GMT_X] * scol * GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
			I->y = 0.5 * Ctrl->C.dim[GMT_Y] * srow * GMT->session.u2u[GMT->current.setting.proj_length_unit][GMT_INCH];
			sx = (scol == 2) ? -1 : 1;
			sy = (srow == 2) ? -1 : 1;
			I->x += sx * I->off[GMT_X];
			I->y += sy * I->off[GMT_Y];
			if (I->mode == MOVIE_LABEL_IS_COL_T) {
				if (!movie_valid_format (I->format, 's')) {
					GMT_Report (API, GMT_MSG_ERROR, "Option -%c: Using +f<format> with word variables requires a \'%%s\'-style format.\n", which[k]);
					movie_close_files (Ctrl);
					Return (GMT_PARSE_ERROR);
				}
			}
			else if (I->mode != MOVIE_LABEL_IS_STRING && I->format[0] && !(movie_valid_format (I->format, 'd') || movie_valid_format (I->format, 'e') || movie_valid_format (I->format, 'f') || movie_valid_format (I->format, 'g'))) {
				GMT_Report (API, GMT_MSG_ERROR, "Option -%c: Using +f<format> with frame or data variables requires a \'%%d\', \'%%e\', \'%%f\', or \'%%g\'-style format.\n", which[k]);
				movie_close_files (Ctrl);
				Return (GMT_PARSE_ERROR);
			}
		}
	}

	/* Get canvas size in pixels */
	p_width =  urint (round (Ctrl->C.dim[GMT_X] * dpu));
	p_height = urint (round (Ctrl->C.dim[GMT_Y] * dpu));
	if (p_width % 2) {	/* Don't like odd pixel widths */
		unsigned int p2_width;
		GMT_Report (API, GMT_MSG_WARNING, "Your frame width is an odd number of pixels (%u). This will not work with FFmpeg\n", p_width);
		do {	/* Make small increments to width in 0.1 pixels until we hit an even integer */
			Ctrl->C.dim[GMT_X] += 0.1 / dpu;
			p2_width = urint (round (Ctrl->C.dim[GMT_X] * dpu));
		} while (p2_width == p_width);	/* Ends when we go from odd to even */
		p_width = p2_width;
		GMT_Report (API, GMT_MSG_WARNING, "Your frame width was adjusted to %g%c, giving an even width of %u pixels\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit , p_width);
	}
	if (p_height % 2) {	/* Don't like odd pixel heights */
		unsigned int p2_height;
		GMT_Report (API, GMT_MSG_WARNING, "Your frame height is an odd number of pixels (%u). This will not work with FFmpeg\n", p_height);
		do {	/* Make small increments to height in 0.1 pixels until we hit an even integer */
			Ctrl->C.dim[GMT_Y] += 0.1 / dpu;
			p2_height = urint (round (Ctrl->C.dim[GMT_Y] * dpu));
		} while (p2_height == p_height);	/* Ends when we go from odd to even */
		p_height = p2_height;
		GMT_Report (API, GMT_MSG_WARNING, "Your frame height was adjusted to %g%c, giving an even height of %u pixels\n", Ctrl->C.dim[GMT_Y], Ctrl->C.unit , p_height);
	}

	if (Ctrl->Q.scripts) {	/* No movie, but scripts will be produced */
		GMT_Report (API, GMT_MSG_INFORMATION, "Dry-run enabled - Movie scripts will be created and any pre/post scripts will be executed.\n");
		if (Ctrl->M.active) {
			if (Ctrl->M.update || Ctrl->M.frame == 0) {
				char *page[3] = {"first", "middle", "last"};
				GMT_Report (API, GMT_MSG_INFORMATION, "A single plot for the %s frame will be create and named %s.%s\n", page[Ctrl->M.update], Ctrl->N.prefix, Ctrl->M.format);
			}
			else
				GMT_Report (API, GMT_MSG_INFORMATION, "A single plot for frame %d will be create and named %s.%s\n", Ctrl->M.frame, Ctrl->N.prefix, Ctrl->M.format);
		}
		run_script = gmt_dry_run_only;	/* This prevents the main frame loop from executing the script */
	}
	else {	/* Will run scripts and may even need to make a movie */
		run_script = system;	/* The standard system function will be used */

		if (Ctrl->F.active[MOVIE_GIF]) {	/* Ensure we have the GraphicsMagick executable "gm" installed in the path */
			if (gmt_check_executable (GMT, "gm", "version", "www.GraphicsMagick.org", line)) {
				sscanf (line, "%*s %s %*s", version);
				GMT_Report (API, GMT_MSG_INFORMATION, "GraphicsMagick %s found.\n", version);
			}
			else {
				GMT_Report (API, GMT_MSG_ERROR, "GraphicsMagick is not installed or not in your executable path - cannot build animated GIF.\n");
				movie_close_files (Ctrl);
				Return (GMT_RUNTIME_ERROR);
			}
		}
		if (Ctrl->F.active[MOVIE_MP4] || Ctrl->F.active[MOVIE_WEBM]) {	/* Ensure we have FFmpeg installed */
			if (gmt_check_executable (GMT, "ffmpeg", "-version", "FFmpeg developers", line)) {
				sscanf (line, "%*s %*s %s %*s", version);
				GMT_Report (API, GMT_MSG_INFORMATION, "FFmpeg %s found.\n", version);
			}
			else {
				GMT_Report (API, GMT_MSG_ERROR, "FFmpeg is not installed - cannot build MP4 or WEbM movies.\n");
				movie_close_files (Ctrl);
				Return (GMT_RUNTIME_ERROR);
			}
		}
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Paper dimensions: Width = %g%c Height = %g%c\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
	GMT_Report (API, GMT_MSG_INFORMATION, "Pixel dimensions: %u x %u\n", p_width, p_height);
	GMT_Report (API, GMT_MSG_INFORMATION, "Building %s PNG images.\n", img_type[Ctrl->F.transparent]);

	/* First try to read -T<timefile> in case it is prescribed directly (and before we change directory) */
	if (!gmt_access (GMT, Ctrl->T.file, R_OK)) {	/* A file by that name exists and is readable */
		if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->T.file, NULL)) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to read time file: %s - exiting\n", Ctrl->T.file);
			movie_close_files (Ctrl);
			Return (API->error);
		}
		if (D->n_segments > 1) {	/* We insist on a simple file structure with a single segment */
			GMT_Report (API, GMT_MSG_ERROR, "Your time file %s has more than one segment - reformat first\n", Ctrl->T.file);
			movie_close_files (Ctrl);
			Return (API->error);
		}
		n_frames = (unsigned int)D->n_records;	/* Number of records means number of frames */
		n_values = (unsigned int)D->n_columns;	/* The number of per-frame parameters we need to place into the per-frame parameter files */
		has_text = (D && D->table[0]->segment[0]->text);	/* Trailing text present */
		if (n_frames == 0) {	/* So not good... */
			GMT_Report (API, GMT_MSG_ERROR, "Your time file %s has no records - exiting\n", Ctrl->T.file);
			movie_close_files (Ctrl);
			Return (GMT_RUNTIME_ERROR);
		}
		gmt_check_abstime_format (GMT, D, 20);	/* Make sure we have enough ss.xxxx decimals given the time spacing */
	}
	n_data_frames = n_frames;

	/* Get full path to the current working directory */
	if (getcwd (topdir, PATH_MAX) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to determine current working directory - exiting.\n");
		movie_close_files (Ctrl);
		Return (GMT_RUNTIME_ERROR);
	}
	gmt_replace_backslash_in_path (topdir);

	if (Ctrl->W.active) {	/* Do all the work in a temporary directory elsewhere */
		if (Ctrl->W.dir) {	/* Specified the path to either a relative or absolute temp directory */
#ifdef WIN32
			if (Ctrl->W.dir[0] == '/' || Ctrl->W.dir[1] == ':')	/* Absolute path under Windows */
#else
			if (Ctrl->W.dir[0] == '/')	/* Absolute path under *nix|macOS */
#endif
				strcpy (workdir, Ctrl->W.dir);	/* Use exactly as given */
			else	/* Gave a relative path */
				sprintf (workdir, "%s/%s", topdir, Ctrl->W.dir);
		}
		else 	/* Make one in the official temp directory based on N.prefix */
			sprintf (workdir, "%s/%s", API->tmp_dir, Ctrl->N.prefix);
	}
	else	/* Use a subdirectory of the current directory */
		sprintf (workdir, "%s/%s", topdir, Ctrl->N.prefix);

	{	/* Block to check for existing workdir */
		struct stat S;
		int err = stat (workdir, &S);	/* Stat the workdir path (which may not exist) */
		if (err == 0 && !S_ISDIR (S.st_mode)) {	/* Path already exists, but it is not a directory */
			GMT_Report (API, GMT_MSG_ERROR, "A file named %s already exist and prevents us from creating a directory by that name; please delete file or adjust -N - exiting.\n", workdir);
			movie_close_files (Ctrl);
			Return (GMT_RUNTIME_ERROR);
		}
		else if (err == 0 && S_ISDIR (S.st_mode)) {	/* Directory already exists */
			if (Ctrl->Z.active) {	/* Zap it */
				if (gmt_remove_dir (API, workdir, false)) {
					movie_close_files (Ctrl);
					GMT_Report (API, GMT_MSG_ERROR, "Unable to remove old directory %s - exiting\n", workdir);
					Return (GMT_RUNTIME_ERROR);
				}
			}
			else {	/* No can do */
				GMT_Report (API, GMT_MSG_ERROR, "Working directory %s already exist and -Z was not specified - exiting\n", workdir);
				movie_close_files (Ctrl);
				Return (GMT_RUNTIME_ERROR);
			}
		}
	}

	if (!access ("gmt.conf", R_OK)) {	/* User has a gmt.conf file in the top directory that needs to be shared with the jobs */
		has_conf = true;
		sprintf (conf_file, "%s/gmt.conf", topdir);
		gmt_replace_backslash_in_path (conf_file);
		if (Ctrl->In.mode == GMT_DOS_MODE)	/* Make it suitable for DOS command line copying */
			gmt_strrepc (conf_file, '/', '\\');
	}

	/* Create a working directory which will house every local file and all subdirectories created */
	if (gmt_mkdir (workdir)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to create new working directory %s - exiting.\n", workdir);
		movie_close_files (Ctrl);
		Return (GMT_RUNTIME_ERROR);
	}
	/* Make this directory the current working directory */
	if (chdir (workdir)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to change directory to %s - exiting.\n", workdir);
		perror (workdir);
		movie_close_files (Ctrl);
		Return (GMT_RUNTIME_ERROR);
	}
	/* Get full path to this working directory */
	if (getcwd (cwd, PATH_MAX) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to determine current working directory - exiting.\n");
		movie_close_files (Ctrl);
		Return (GMT_RUNTIME_ERROR);
	}

	/* We use DATADIR to include the top and working directory so any files we supply or create can be found while inside frame directory */

	if (GMT->session.DATADIR)	/* Prepend initial and subdir as new datadirs to the existing search list */
		sprintf (datadir, "%s,%s,%s", topdir, cwd, GMT->session.DATADIR);	/* Start with topdir */
	else	/* Set the initial and prefix subdirectory as data dirs */
		sprintf (datadir, "%s,%s", topdir, cwd);

	gmt_replace_backslash_in_path (datadir);	/* Since we will be fprintf the path we must use // for a slash */
	gmt_replace_backslash_in_path (workdir);

	/* Make a path to workdir that works on current architecture (for command line calls) */
	strncpy (tmpwpath, workdir, PATH_MAX);
	if (Ctrl->In.mode == GMT_DOS_MODE)		/* On Windows to do remove a file in a subdir one need to use back slashes */
		gmt_strrepc (tmpwpath, '/', '\\');

	/* Create the initialization file with settings common to all frames */

	n_written = (n_frames > 0);
	sprintf (init_file, "movie_init.%s", extension[Ctrl->In.mode]);
	GMT_Report (API, GMT_MSG_INFORMATION, "Create parameter initiation script %s\n", init_file);
	if ((fp = fopen (init_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to create file %s - exiting\n", init_file);
		movie_close_files (Ctrl);
		Return (GMT_ERROR_ON_FOPEN);
	}

	sprintf (string, "Static parameters set for animation sequence %s", Ctrl->N.prefix);
	gmt_set_comment (fp, Ctrl->In.mode, string);
	gmt_set_dvalue (fp, Ctrl->In.mode, "MOVIE_WIDTH",  Ctrl->C.dim[GMT_X], Ctrl->C.unit);
	gmt_set_dvalue (fp, Ctrl->In.mode, "MOVIE_HEIGHT", Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
	gmt_set_dvalue (fp, Ctrl->In.mode, "MOVIE_DPU",    Ctrl->C.dim[GMT_Z], 0);
	gmt_set_dvalue (fp, Ctrl->In.mode, "MOVIE_RATE",   Ctrl->D.framerate, 0);
	if (n_written) gmt_set_ivalue (fp, Ctrl->In.mode, false, "MOVIE_NFRAMES", n_data_frames);	/* Total frames (write to init since known) */
	if (Ctrl->I.active) {	/* Append contents of an include file */
		gmt_set_comment (fp, Ctrl->In.mode, "Static parameters set via user include file");
		while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->I.fp)) {	/* Read the include file and copy to init script with some exceptions */
			if (gmt_is_gmtmodule (line, "begin")) continue;		/* Skip gmt begin */
			if (gmt_is_gmtmodule (line, "end")) continue;		/* Skip gmt end */
			if (strstr (line, "#!/")) continue;			/* Skip any leading shell incantation */
			if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
			fprintf (fp, "%s", line);				/* Just copy the line as is */
		}
		fclose (Ctrl->I.fp);	/* Done reading the include script */
	}
	fclose (fp);	/* Done writing the init script */

	/* Because when a movie is plotted we do not want gmt_plot_init to contemplate orientation changes
	 * we help that decision by setting this environmental parameter.  We unset it at the end. */
#ifdef WIN32
	if (_putenv ("GMT_MOVIE=YES"))
#else
	if (setenv ("GMT_MOVIE", "YES", 1))
#endif
		GMT_Report (API, GMT_MSG_WARNING, "Unable to set GMT_MOVIE=YES to inform GMT that movie scripts will be building a movie\n");

	if (Ctrl->S[MOVIE_PREFLIGHT].active) {	/* Create the preflight script from the user's background script */
		/* The background script must be modern mode */
		unsigned int rec = 0;
		if (Ctrl->S[MOVIE_PREFLIGHT].PS)	/* Just got a PS file, nothing to do */
			fclose (Ctrl->S[MOVIE_PREFLIGHT].fp);
		else {	/* Run the preflight script */
			sprintf (pre_file, "movie_preflight.%s", extension[Ctrl->In.mode]);
			is_classic = gmt_script_is_classic (GMT, Ctrl->S[MOVIE_PREFLIGHT].fp);
			if (is_classic) {
				GMT_Report (API, GMT_MSG_ERROR, "Your preflight file %s is not in GMT modern node - exiting\n", pre_file);
				fclose (Ctrl->In.fp);
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
			GMT_Report (API, GMT_MSG_INFORMATION, "Create preflight script %s and execute it\n", pre_file);
			if ((fp = fopen (pre_file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create preflight script %s - exiting\n", pre_file);
				fclose (Ctrl->In.fp);
				error = GMT_ERROR_ON_FOPEN;
				goto out_of_here;
			}
			gmt_set_script (fp, Ctrl->In.mode);			/* Write 1st line of a script */
			gmt_set_comment (fp, Ctrl->In.mode, "Preflight script");
			fprintf (fp, "%s", export[Ctrl->In.mode]);		/* Hardwire a Session Name since sub-shells may mess things up */
			if (Ctrl->In.mode == GMT_DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to 1 since we run this separately first */
				fprintf (fp, "set GMT_SESSION_NAME=1\n");
			else	/* On UNIX we may use the calling terminal or script's PID as the GMT_SESSION_NAME */
				gmt_set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
			fprintf (fp, "%s", export[Ctrl->In.mode]);		/* Turn off auto-display of figures if scrip has gmt end show */
			gmt_set_tvalue (fp, Ctrl->In.mode, true, "GMT_END_SHOW", "off");
			fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
			while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->S[MOVIE_PREFLIGHT].fp)) {	/* Read the background script and copy to preflight script with some exceptions */
				if (gmt_is_gmtmodule (line, "begin")) {	/* Need to insert gmt figure after this line (or as first line) in case a background plot will be made */
					fprintf (fp, "gmt begin\n");	/* To ensure there are no args here since we are using gmt figure instead */
					if (has_conf && !strstr (line, "-C")) fprintf (fp, cpconf[Ctrl->In.mode], conf_file);
					gmt_set_comment (fp, Ctrl->In.mode, "\tSet fixed background output ps name");
					fprintf (fp, "\tgmt figure movie_background ps\n");
					fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
					fprintf (fp, "\tgmt set DIR_DATA \"%s\"\n", datadir);
				}
				else if (!strstr (line, "#!/"))	 {	/* Skip any leading shell incantation since already placed by gmt_set_script */
					if (gmt_is_gmt_end_show (line)) sprintf (line, "gmt end\n");		/* Eliminate show from gmt end in this script */
					else if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
					fprintf (fp, "%s", line);	/* Just copy the line as is */
				}
				rec++;
			}
			fclose (Ctrl->S[MOVIE_PREFLIGHT].fp);	/* Done reading the foreground script */
			fclose (fp);	/* Done writing the preflight script */
#ifndef WIN32	/* Set executable bit if not Windows cmd */
			if (chmod (pre_file, S_IRWXU)) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to make preflight script %s executable - exiting.\n", pre_file);
				fclose (Ctrl->In.fp);
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
#endif
			/* Run the pre-flight now which may or may not create a <timefile> needed later via -T, as well as a background plot */
			sprintf (cmd, "%s %s", sys_cmd_wait[Ctrl->In.mode], pre_file);
			if ((error = system (cmd))) {
				GMT_Report (API, GMT_MSG_ERROR, "Running preflight script %s returned error %d - exiting.\n", pre_file, error);
				fclose (Ctrl->In.fp);
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
		}
		place_background = (!access ("movie_background.ps", R_OK));	/* Need to place a background layer in the main frames */
	}

	/* Now we can complete the -T parsing since any given file has now been created by pre_file */

	if (n_frames == 0) {	/* Must check again for a file or decode the argument as a number */
		if (!gmt_access (GMT, Ctrl->T.file, R_OK)) {	/* A file by that name was indeed created by preflight and is now available */
			if ((D = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->T.file, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to read time file: %s - exiting\n", Ctrl->T.file);
				fclose (Ctrl->In.fp);
				error = API->error;
				goto out_of_here;
			}
			if (D->n_segments > 1) {	/* We insist on a simple file structure with a single segment */
				GMT_Report (API, GMT_MSG_ERROR, "Your time file %s has more than one segment - reformat first\n", Ctrl->T.file);
				fclose (Ctrl->In.fp);
				error = API->error;
				goto out_of_here;
			}
			n_frames = n_data_frames = (unsigned int)D->n_records;	/* Number of records means number of frames */
			n_values = (unsigned int)D->n_columns;	/* The number of per-frame parameters we need to place into the per-frame parameter files */
			has_text = (D && D->table[0]->segment[0]->text);	/* Trailing text present */
			gmt_check_abstime_format (GMT, D, 20);	/* Make sure we have enough ss.xxxx decimals given the time spacing */
		}
		else if (gmt_count_char (GMT, Ctrl->T.file, '/') == 2) {	/* Give a vector specification -Tmin/max/inc, call gmtmath */
			char output[GMT_VF_LEN] = {""}, cmd[GMT_LEN128] = {""};
			unsigned int V = GMT->current.setting.verbose;
			if (GMT_Open_VirtualFile (API, GMT_IS_DATASET, GMT_IS_NONE, GMT_OUT|GMT_IS_REFERENCE, NULL, output) == GMT_NOTSET) {
				error = API->error;
				goto out_of_here;
			}
			if (GMT->common.f.active[GMT_IN])
				sprintf (cmd, "-T%s -o1 -f%s --GMT_HISTORY=readonly T = %s", Ctrl->T.file, GMT->common.f.string, output);
			else
				sprintf (cmd, "-T%s -o1 --GMT_HISTORY=readonly T = %s", Ctrl->T.file, output);
			GMT_Report (API, GMT_MSG_INFORMATION, "Calling gmtmath with args %s\n", cmd);
			GMT->current.setting.verbose = GMT_MSG_ERROR;	/* So we don't get unwanted verbosity from gmtmath */
  			if (GMT_Call_Module (API, "gmtmath", GMT_MODULE_CMD, cmd)) {	/* Some sort of failure */
				error = API->error;
				goto out_of_here;
			}
			GMT->current.setting.verbose = V;	/* Restore */
			if ((D = GMT_Read_VirtualFile (API, output)) == NULL) {	/* Load in the data array */
				error = API->error;
				goto out_of_here;
			}
			n_frames = n_data_frames = (unsigned int)D->n_records;	/* Number of records means number of frames */
			n_values = (unsigned int)D->n_columns;	/* The number of per-frame parameters we need to place into the per-frame parameter files */
			has_text = (D && D->table[0]->segment[0]->text);	/* Trailing text present */
			if (strchr (Ctrl->T.file, 'T'))	/* Check here since gmtmath does not pass that info back */
				gmt_set_column_type (GMT, GMT_IN, GMT_X, GMT_IS_ABSTIME);	/* Set first input column type as absolute time */

		}
		else {	/* Just gave the number of frames (we hope, or we got a bad filename and atoi should return 0) */
			n_frames = n_data_frames = atoi (Ctrl->T.file);
			issue_col0_par = true;
		}
	}
	if (n_frames == 0) {	/* So not good... */
		GMT_Report (API, GMT_MSG_ERROR, "No frames specified! - exiting.\n");
		fclose (Ctrl->In.fp);
		error = GMT_RUNTIME_ERROR;
		goto out_of_here;
	}

	if (Ctrl->M.update)	/* Now we can determine and update last or middle frame number */
		Ctrl->M.frame = (Ctrl->M.update == 2) ? n_frames - 1 : n_frames / 2;
	if (Ctrl->K.active) {
		n_fade_frames = Ctrl->K.fade[GMT_IN] + Ctrl->K.fade[GMT_OUT];	/* Extra frames if preserving */
		if (!(Ctrl->K.preserve[GMT_IN] || Ctrl->K.preserve[GMT_OUT]) && n_fade_frames > n_data_frames) {
			GMT_Report (API, GMT_MSG_ERROR, "Option -K: Combined fading duration cannot exceed animation duration\n");
			fclose (Ctrl->In.fp);
			error = GMT_RUNTIME_ERROR;
			goto out_of_here;
		}
		n_fade_frames = 0;	/* If clobbering */
		if (Ctrl->K.preserve[GMT_IN])  n_fade_frames += Ctrl->K.fade[GMT_IN];	/* We are preserving, not clobbering */
		if (Ctrl->K.preserve[GMT_OUT]) n_fade_frames += Ctrl->K.fade[GMT_OUT];	/* We are preserving, not clobbering */
	}

	for (k = MOVIE_ITEM_IS_LABEL; k <= MOVIE_ITEM_IS_PROG_INDICATOR; k++) {
		/* Make sure we have the information requested if data columns or trailing text is needed */
		for (T = 0; T < Ctrl->n_items[k]; T++) {
			I = &Ctrl->item[k][T];	/* Shorthand for this item */
			if ((I->mode == MOVIE_LABEL_IS_COL_C || I->mode == MOVIE_LABEL_IS_COL_T) && D == NULL) {    /* Need a floating point number */
				GMT_Report (API, GMT_MSG_ERROR, "No table given via -T for data-based labels in %c - exiting\n", which[k]);
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
			if (I->mode == MOVIE_LABEL_IS_COL_C && I->col >= D->n_columns) {
				GMT_Report (API, GMT_MSG_ERROR, "Data table does not have enough columns for your -%c c%d request - exiting\n", which[k], I->col);
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
			if (I->mode == MOVIE_LABEL_IS_COL_T && D->table[0]->segment[0]->text == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Data table does not have trailing text for your -%c t%d request - exit\n", which[k], I->col);
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
		}
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Number of main animation frames: %d\n", n_data_frames);
	if (Ctrl->T.precision)	/* Precision was prescribed */
		precision = Ctrl->T.precision;
	else	/* Compute width from largest frame number */
		precision = irint (ceil (log10 ((double)(Ctrl->T.start_frame+n_frames+Ctrl->E.duration+n_fade_frames-1))+0.1));	/* Width needed to hold largest frame number, guaranteed to give at least 1 */

	if (Ctrl->S[MOVIE_POSTFLIGHT].active) {	/* Prepare the postflight script */
		if (Ctrl->S[MOVIE_POSTFLIGHT].PS)	/* Just got a PS file, nothing to do */
			fclose (Ctrl->S[MOVIE_POSTFLIGHT].fp);
		else {	/* Must run postflight script */
			sprintf (post_file, "movie_postflight.%s", extension[Ctrl->In.mode]);
			is_classic = gmt_script_is_classic (GMT, Ctrl->S[MOVIE_POSTFLIGHT].fp);
			if (is_classic) {
				GMT_Report (API, GMT_MSG_ERROR, "Your postflight file %s is not in GMT modern node - exiting\n", post_file);
				fclose (Ctrl->In.fp);
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
			GMT_Report (API, GMT_MSG_INFORMATION, "Create postflight script %s\n", post_file);
			if ((fp = fopen (post_file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create postflight file %s - exiting\n", post_file);
				fclose (Ctrl->In.fp);
				error = GMT_ERROR_ON_FOPEN;
				goto out_of_here;
			}
			gmt_set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
			gmt_set_comment (fp, Ctrl->In.mode, "Postflight script");
			fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a SESSION_NAME since sub-shells may mess things up */
			if (Ctrl->In.mode == GMT_DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to 1 since we run this separately */
				fprintf (fp, "set GMT_SESSION_NAME=1\n");
			else	/* On UNIX we may use the script's PID as GMT_SESSION_NAME */
				gmt_set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
			fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Turn off auto-display of figures if scrip has gmt end show */
			gmt_set_tvalue (fp, Ctrl->In.mode, true, "GMT_END_SHOW", "off");
			fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
			while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->S[MOVIE_POSTFLIGHT].fp)) {	/* Read the foreground script and copy to postflight script with some exceptions */
				if (gmt_is_gmtmodule (line, "begin")) {	/* Need to insert gmt figure after this line */
					fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
					if (has_conf && !strstr (line, "-C")) fprintf (fp, cpconf[Ctrl->In.mode], conf_file);
					gmt_set_comment (fp, Ctrl->In.mode, "\tSet fixed foreground output ps name");
					fprintf (fp, "\tgmt figure movie_foreground ps\n");
					fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
					fprintf (fp, "\tgmt set DIR_DATA \"%s\"\n", datadir);
				}
				else if (!strstr (line, "#!/"))	{	/* Skip any leading shell incantation since already placed */
					if (gmt_is_gmt_end_show (line)) sprintf (line, "gmt end\n");		/* Eliminate show from gmt end in this script */
					else if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
					fprintf (fp, "%s", line);	/* Just copy the line as is */
				}
			}
			fclose (Ctrl->S[MOVIE_POSTFLIGHT].fp);	/* Done reading the foreground script */
			fclose (fp);	/* Done writing the postflight script */
#ifndef WIN32	/* Set executable bit if not Windows cmd */
				if (chmod (post_file, S_IRWXU)) {
					GMT_Report (API, GMT_MSG_ERROR, "Unable to make postflight script %s executable - exiting\n", post_file);
					fclose (Ctrl->In.fp);
					error = GMT_RUNTIME_ERROR;
					goto out_of_here;
				}
#endif
			/* Run post-flight now before dealing with the loop so the overlay exists */
			sprintf (cmd, "%s %s", sys_cmd_wait[Ctrl->In.mode], post_file);
			if ((error = run_script (cmd))) {
				GMT_Report (API, GMT_MSG_ERROR, "Running postflight script %s returned error %d - exiting.\n", post_file, error);
				fclose (Ctrl->In.fp);
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
		}
	}

	for (k = MOVIE_ITEM_IS_LABEL; k <= MOVIE_ITEM_IS_PROG_INDICATOR; k++) {
		/* Set elapse time label format if chosen */
		char *format = NULL;
		for (T = 0; T < Ctrl->n_items[k]; T++) {
			I = &Ctrl->item[k][T];	/* Shorthand for this item */
			if (I->mode != MOVIE_LABEL_IS_ELAPSED) continue;
			frame = n_frames + Ctrl->T.start_frame;	/* Max frame number */
			L_col = (I->scale > 0.0) ? frame * I->scale : frame / Ctrl->D.framerate;
			ss = urint (fmod (L_col, 60.0));	L_col = (L_col - ss) / 60.0;	/* Get seconds and switch to minutes */
			mm = urint (fmod (L_col, 60.0));	L_col = (L_col - mm) / 60.0;	/* Get seconds and switch to hours */
			hh = urint (fmod (L_col, 24.0));	L_col = (L_col - hh) / 24.0;	/* Get seconds and switch to days */
			dd = urint (L_col);
			if (dd > 0)
				I->ne = 4, strcpy (I->format, "%d %2.2d:%2.2d:%2.2d");
			else if (hh > 0)
				I->ne = 3, strcpy (I->format, "%2.2d:%2.2d:%2.2d");
			else if (mm > 0)
				I->ne = 2, strcpy (I->format, "%2.2d:%2.2d");
			else
				I->ne = 1, strcpy (I->format, "%2.2d");
		}
		format = (GMT->current.map.frame.primary) ? GMT->current.setting.format_time[GMT_PRIMARY] : GMT->current.setting.format_time[GMT_SECONDARY];
		switch (format[0]) {	/* This parameter controls which version of month/day text strings we use for plotting */
			case 'F':	/* Full name, upper case */
				upper_case[k] = true;
				/* Intentionally fall through - to 'f' */
			case 'f':	/* Full name, lower case */
				flavor[k] = 0;
				break;
			case 'A':	/* Abbreviated name, upper case */
				upper_case[k] = true;
				/* Intentionally fall through - to 'a' */
			case 'a':	/* Abbreviated name, lower case */
				flavor[k] = 1;
				break;
			case 'C':	/* 1-char name, upper case */
				upper_case[k] = true;
				/* Intentionally fall through - to 'c' */
			case 'c':	/* 1-char name, lower case */
				flavor[k] = 2;
				break;
			default:
				break;
		}
	}

	if (Ctrl->Q.active) strcat (frame_products, MOVIE_DEBUG_FORMAT);	/* Want to save original PS file for debug */

	if (Ctrl->E.active) {
		/* The title/fade sequence will add more frames to the job.  However, we do not want the frame number
		 * used to place labels or progress indicators to count them.  We need to add an offset that applies
		 * to the naming of parameter files for the actual movie script but not the frame values used.
		 */

		GMT_Report (API, GMT_MSG_INFORMATION, "Parameter files for title sequence: %d\n", Ctrl->E.duration);
		for (i_frame = 0; i_frame < Ctrl->E.duration; i_frame++) {
			frame = i_frame + Ctrl->T.start_frame;	/* Actual frame number */
			if (one_frame && frame != Ctrl->M.frame) continue;	/* Just doing a single frame for debugging */
			sprintf (state_tag, "%*.*d", precision, precision, frame);
			sprintf (state_prefix, "movie_params_%s", state_tag);
			sprintf (param_file, "%s.%s", state_prefix, extension[Ctrl->In.mode]);
			if ((fp = fopen (param_file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create frame parameter file %s - exiting\n", param_file);
				fclose (Ctrl->In.fp);
				error = GMT_ERROR_ON_FOPEN;
				goto out_of_here;
			}
			sprintf (state_prefix, "Parameter file for pre-frame %s", state_tag);
			gmt_set_comment (fp, Ctrl->In.mode, state_prefix);
			sprintf (state_prefix, "%s_%s", Ctrl->N.prefix, state_tag);
			gmt_set_tvalue (fp, Ctrl->In.mode, false, "MOVIE_NAME", state_prefix);	/* Current frame name prefix */
			if (i_frame < Ctrl->E.fade[GMT_IN])	/* During title fade-in */
				fade_level = 50 * (1.0 + cos (M_PI * i_frame / Ctrl->E.fade[GMT_IN]));
			else if (i_frame > (start = (Ctrl->E.duration -Ctrl->E.fade[GMT_OUT])))	/* During title fade-out */
				fade_level = 50 * (1.0 - cos (M_PI * (i_frame - start) / Ctrl->E.fade[GMT_OUT]));
			else /* No fading during main part of title */
				fade_level = 0.0;
			gmt_set_dvalue (fp, Ctrl->In.mode, "MOVIE_FADE", fade_level, 0);
			fclose (fp);	/* Done writing this parameter file */
		}

		sprintf (intro_file, "movie_intro.%s", extension[Ctrl->In.mode]);
		GMT_Report (API, GMT_MSG_INFORMATION, "Create movie intro frame script %s\n", intro_file);
		if ((fp = fopen (intro_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create movie intro script file %s - exiting\n", intro_file);
			fclose (Ctrl->E.fp);
			error = GMT_ERROR_ON_FOPEN;
			goto out_of_here;
		}
		sprintf (extra, "A+M+r,N+f%s", gmt_place_var (Ctrl->In.mode, "MOVIE_FADE"));	/* No cropping, image size is fixed, possibly fading */
		if (Ctrl->E.fill) {strcat (extra, "+g"); strcat (extra, Ctrl->E.fill);}	/* Chose another fade color than black */
		if (Ctrl->E.PS) {	/* Need to place a background title first (which will be in parent dir when loop script is run) */
			strcat (extra, ",Mb../../");
			strcat (extra, Ctrl->E.file);
		}
		if (Ctrl->H.active) {	/* Must pass the DownScaleFactor option to psconvert */
			char htxt[GMT_LEN16] = {""};
			sprintf (htxt, ",H%d", Ctrl->H.factor);
			strcat (extra, htxt);
		}
		gmt_set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
		gmt_set_comment (fp, Ctrl->In.mode, "Movie intro frame loop script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a GMT_SESSION_NAME since sub-shells may mess things up */
		if (Ctrl->In.mode == GMT_DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to be the frame number */
			fprintf (fp, "set GMT_SESSION_NAME=%c1\n", var_token[Ctrl->In.mode]);
		else	/* On UNIX we use the script's PID as GMT_SESSION_NAME */
			gmt_set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Turn off auto-display of figures if scrip has gmt end show */
		gmt_set_tvalue (fp, Ctrl->In.mode, true, "GMT_END_SHOW", "off");
		gmt_set_comment (fp, Ctrl->In.mode, "Include static and frame-specific parameters");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		fprintf (fp, "%s movie_params_%c1.%s\n", load[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Include the frame parameters */
		fprintf (fp, "mkdir %s\n", gmt_place_var (Ctrl->In.mode, "MOVIE_NAME"));	/* Make a temp directory for this frame */
		fprintf (fp, "cd %s\n", gmt_place_var (Ctrl->In.mode, "MOVIE_NAME"));		/* cd to the temp directory */
		if (Ctrl->E.PS) {	/* There is no title script, just a PS, so we make a dummy script that plots nothing */
			fclose (Ctrl->E.fp);
			fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
			gmt_set_comment (fp, Ctrl->In.mode, "\tSet output PNG name and plot conversion parameters");
			fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c GMT_MAX_CORES 1\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
			fprintf (fp, "\tgmt figure ../%s %s", gmt_place_var (Ctrl->In.mode, "MOVIE_NAME"), frame_products);
			fprintf (fp, " E%s,%s\n", gmt_place_var (Ctrl->In.mode, "MOVIE_DPU"), extra);
			fprintf (fp, "\tgmt plot -R0/%g/0/%g -Jx1%c -X0 -Y0 -T\n", Ctrl->C.dim[GMT_X], Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
			fprintf (fp, "gmt end\n");		/* Eliminate show from gmt end in this script */
		}
		else {	/* Read the title script */
			while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->E.fp)) {	/* Read the main script and copy to loop script, with some exceptions */
				if (gmt_is_gmtmodule (line, "begin")) {	/* Need to insert a gmt figure call after this line */
					fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
					if (has_conf && !strstr (line, "-C")) fprintf (fp, cpconf[Ctrl->In.mode], conf_file);
					gmt_set_comment (fp, Ctrl->In.mode, "\tSet output PNG name and plot conversion parameters");
					fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
					fprintf (fp, "\tgmt set DIR_DATA \"%s\"\n", datadir);
					fprintf (fp, "\tgmt figure ../%s %s", gmt_place_var (Ctrl->In.mode, "MOVIE_NAME"), frame_products);
					fprintf (fp, " E%s,%s\n", gmt_place_var (Ctrl->In.mode, "MOVIE_DPU"), extra);
				}
				else if (!strstr (line, "#!/")) {		/* Skip any leading shell incantation since already placed */
					if (gmt_is_gmt_end_show (line)) sprintf (line, "gmt end\n");		/* Eliminate show from gmt end in this script */
					else if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
					fprintf (fp, "%s", line);	/* Just copy the line as is */
				}
			}
			fclose (Ctrl->E.fp);	/* Done reading the main script */
		}
		fprintf (fp, "cd ..\n");	/* cd up to parent dir */
		if (!Ctrl->Q.active) {	/* Delete evidence; otherwise we want to leave debug evidence when doing a single frame only */
			gmt_set_comment (fp, Ctrl->In.mode, "Remove frame directory and frame parameter file");
			fprintf (fp, "%s %s\n", rmdir[Ctrl->In.mode], gmt_place_var (Ctrl->In.mode, "MOVIE_NAME"));	/* Remove the work dir and any files in it */
			fprintf (fp, "%s movie_params_%c1.%s\n", rmfile[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Remove the parameter file for this frame */
		}
		if (Ctrl->In.mode == GMT_DOS_MODE)	/* This is crucial to the "start /B ..." statement below to ensure the DOS process terminates */
			fprintf (fp, "exit\n");
		fclose (fp);	/* Done writing loop script */

#ifndef WIN32	/* Set executable bit if not Windows cmd */
		if (chmod (intro_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to make script %s executable - exiting\n", intro_file);
			error = GMT_RUNTIME_ERROR;
			goto out_of_here;
		}
#endif
	}

	/* Create parameter include files, one for each frame */

	if (Ctrl->K.active) {	/* Must make room for the extra frames need for fading in and out, unless preserve is true */
		unsigned int pre_fade = (Ctrl->K.preserve[GMT_IN]) ? Ctrl->K.fade[GMT_IN] : 0;	/* Extra frames added due to preserve at the beginning */
		if (Ctrl->K.preserve[GMT_OUT])	/* Extra frames are created at the end, so we start fading after the last frame */
			first_fade_out_frame = pre_fade + n_frames;
		else	/* No extra frames are created - just fading being applied to some of them */
			first_fade_out_frame = pre_fade + n_frames - Ctrl->K.fade[GMT_OUT] - 1;
		n_frames += n_fade_frames;
		GMT_Report (API, GMT_MSG_INFORMATION, "Parameter files for fade in/out of main animation: %d\n", n_fade_frames);
	}

	GMT_Report (API, GMT_MSG_INFORMATION, "Parameter files for main animation: %d\n", n_frames);
	for (i_frame = 0; i_frame < n_frames; i_frame++) {
		frame = data_frame = i_frame + Ctrl->T.start_frame;	/* The frame is normally same as data_frame number */
		if (one_frame && (frame + Ctrl->E.duration) != Ctrl->M.frame) continue;	/* Just doing a single frame for debugging */
		sprintf (state_tag, "%*.*d", precision, precision, frame + Ctrl->E.duration);
		sprintf (state_prefix, "movie_params_%s", state_tag);
		sprintf (param_file, "%s.%s", state_prefix, extension[Ctrl->In.mode]);
		if ((fp = fopen (param_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create frame parameter file %s - exiting\n", param_file);
			fclose (Ctrl->In.fp);
			error = GMT_ERROR_ON_FOPEN;
			goto out_of_here;
		}
		sprintf (state_prefix, "Parameter file for frame %s", state_tag);
		gmt_set_comment (fp, Ctrl->In.mode, state_prefix);
		sprintf (state_prefix, "%s_%s", Ctrl->N.prefix, state_tag);
		gmt_set_tvalue (fp, Ctrl->In.mode, false, "MOVIE_NAME", state_prefix);	/* Current frame name prefix */

		if (Ctrl->K.active) {	/* Must stay on either first or last actual frame repeatedly to fade in/out the first/last true frame plot */
			if (i_frame < Ctrl->K.fade[GMT_IN]) { /* Must keep fixed first data frame but change fading in */
				if (Ctrl->K.preserve[GMT_IN]) data_frame = 0;	/* Keep plotting the first data frame */
				fade_level = 50 * (1.0 + cos (M_PI * i_frame / Ctrl->K.fade[GMT_IN]));
			}
			else if (i_frame >= first_fade_out_frame) { /* Must keep fixed plot but change fading out */
				if (Ctrl->K.preserve[GMT_OUT]) data_frame = n_data_frames - 1;	/* Keep plotting the last data frame */
				fade_level = 50 * (1.0 - cos (M_PI * (i_frame - first_fade_out_frame) / Ctrl->K.fade[GMT_OUT]));
			}
			else {
				fade_level = 0.0;	/* No fading */
				if (Ctrl->K.preserve[GMT_IN]) data_frame = i_frame - Ctrl->K.fade[GMT_IN];
			}
			gmt_set_dvalue (fp, Ctrl->In.mode, "MOVIE_FADE", fade_level, 0);
		}
		if (Ctrl->E.active) sprintf (state_tag, "%*.*d", precision, precision, data_frame);	/* Reset frame tag */
		gmt_set_ivalue (fp, Ctrl->In.mode, false, "MOVIE_FRAME", data_frame);		/* Current frame number */
		if (!n_written) gmt_set_ivalue (fp, Ctrl->In.mode, false, "MOVIE_NFRAMES", n_data_frames);	/* Total frames (write here since n_frames was not yet known when init was written) */
		gmt_set_tvalue (fp, Ctrl->In.mode, false, "MOVIE_ITEM", state_tag);		/* Current frame tag (formatted frame number) */
		if (place_background)	/* Need to place a background layer first (which is in parent dir when loop script is run) */
			gmt_set_tvalue (fp, Ctrl->In.mode, false, "MOVIE_BACKGROUND", ",Mb../movie_background.ps");		/* Current frame tag (formatted frame number) */
		else
			gmt_set_tvalue (fp, Ctrl->In.mode, false, "MOVIE_BACKGROUND", "");		/* Nothing */
		if (Ctrl->F.transparent && Ctrl->F.active[MOVIE_GIF]) place_background = false;	/* Only place background once if transparent images */
		for (col = 0; col < n_values; col++) {	/* Derive frame variables from <timefile> in each parameter file */
			sprintf (string, "MOVIE_COL%u", col);
			gmt_set_value (GMT, fp, Ctrl->In.mode, col, string, D->table[0]->segment[0]->data[col][data_frame]);
		}
		if (issue_col0_par) gmt_set_ivalue (fp, Ctrl->In.mode, false, "MOVIE_COL0", data_frame);	/* Same as current frame number */
		if (has_text) {	/* Also place any string parameter as a single string variable */
			gmt_set_tvalue (fp, Ctrl->In.mode, false, "MOVIE_TEXT", D->table[0]->segment[0]->text[data_frame]);
			if (Ctrl->T.split) {	/* Also split the string into individual words MOVIE_WORD1, MOVIE_WORD2, etc. */
				char *word = NULL, *trail = NULL, *orig = strdup (D->table[0]->segment[0]->text[data_frame]);
				col = 0;
				trail = orig;
				while ((word = strsep (&trail, Ctrl->T.sep)) != NULL) {
					if (*word != '\0') {	/* Skip empty strings */
						sprintf (string, "MOVIE_WORD%u", col++);
						gmt_set_tvalue (fp, Ctrl->In.mode, false, string, word);
					}
				}
				gmt_M_str_free (orig);
			}
		}
		spacer = ' ';	/* Use a space to separate date and clock for labels; this will change to T for progress -R settings */
		for (k = MOVIE_ITEM_IS_LABEL; k <= MOVIE_ITEM_IS_PROG_INDICATOR; k++) {
			if (Ctrl->item_active[k]) {	/* Want to place a user label or progress indicator */
				char label[GMT_LEN256] = {""}, font[GMT_LEN64] = {""};
				unsigned int type, use_frame, p;
				double t;
				struct GMT_FONT *F = (k == MOVIE_ITEM_IS_LABEL) ? &GMT->current.setting.font_tag : &GMT->current.setting.font_annot[GMT_SECONDARY];	/* Default font for labels and progress indicators  */
				/* Place all movie labels and progress indicator information as comments in the parameter file - there will be read by gmt figure and converted to gmt.movie{labels,progress_indicators} files */
				/* Note: All dimensions are written in inches and read as inches in gmt_plotinit */

				for (T = 0; T < Ctrl->n_items[k]; T++) {
					t = frame / (n_frames - 1.0);	/* Relative time 0-1 for selected frame */
					I = &Ctrl->item[k][T];	/* Shorthand for this item */
					/* Set selected font: Prepend + if user specified a font, else just give current default font */
					if (I->font.size > 0.0)	/* Users selected font */
						sprintf (font, "+%s", gmt_putfont (GMT, &I->font));
					else	/* Default font */
						sprintf (font, "%s", gmt_putfont (GMT, F));
					/* Place kind|x|y|t|width|just|clearance_x|clearance_Y|pen|pen2|fill|fill2|box|box_X|box_Y|sfill|font|txt in MOVIE_{LABEL|PROG_INDICATOR}_ARG */
					sprintf (label, "MOVIE_%c: %c|%g|%g|%g|%g|%d|%g|%g|%s|%s|%s|%s|%d|%g|%g|%s|%s|", which[k], I->kind, I->x, I->y, t, I->width,
						I->justify, I->clearance[GMT_X], I->clearance[GMT_Y], I->pen, I->pen2, I->fill, I->fill2, I->box, I->soff[GMT_X], I->soff[GMT_Y], I->sfill, font);
					string[0] = '\0';
					for (p = 0; p < I->n_labels; p++) {	/* Here, n_labels is 0 (no labels), 1 (just at the current time) or 2 (start/end times) */
						if (I->n_labels == 2)	/* Want start/stop values, not current frame value */
							use_frame = (p == 0) ? 0 : n_frames - 1;
						else	/* Current frame only */
							use_frame = data_frame;
						if (I->kind == 'F' && p == 0) strcat (label, "-R");	/* We will write a functioning -R option to plot the time-axis */
						t = use_frame / (n_frames - 1.0);	/* Relative time 0-1 for selected frame */
						if (I->mode == MOVIE_LABEL_IS_FRAME) {	/* Place a frame counter */
							if (I->format[0] && movie_valid_format (I->format, 'd'))	/* Set as integer */
								sprintf (string, I->format, (int)use_frame);
							else if (I->format[0])	/* Set as floating point */
								sprintf (string, I->format, (double)use_frame);
							else	/* Default to the frame tag string format */
								sprintf (string, "%*.*d", precision, precision, use_frame);
						}
						else if (I->mode == MOVIE_LABEL_IS_PERCENT) {	/* Place a percent counter */
							if (I->format[0] && movie_valid_format (I->format, 'd'))	/* Set as integer */
								sprintf (string, I->format, (int)irint (100.0 * t));
							else if (I->format[0])	/* Set as floating point */
								sprintf (string, I->format, (100.0 * t));
							else	/* Default to xxx % */
								sprintf (string, "%3d%%", (int)irint (100.0 * t));
						}
						else if (I->mode == MOVIE_LABEL_IS_ELAPSED) {	/* Place elapsed time */
							gmt_M_memset (string, GMT_LEN128, char);
							L_col = (I->scale > 0.0) ? use_frame * I->scale : use_frame / Ctrl->D.framerate;
							ss = urint (fmod (L_col, 60.0));	L_col = (L_col - ss) / 60.0;	/* Get seconds and switch to minutes*/
							mm = urint (fmod (L_col, 60.0));	L_col = (L_col - mm) / 60.0;	/* Get seconds and switch to hours */
							hh = urint (fmod (L_col, 24.0));	L_col = (L_col - hh) / 24.0;	/* Get seconds and switch to days */
							dd = urint (L_col);
							switch (I->ne) {
								case 4: 	sprintf (string, I->format, dd, hh, mm, ss); break;
								case 3: 	sprintf (string, I->format, hh, mm, ss); break;
								case 2: 	sprintf (string, I->format, mm, ss); break;
								case 1: 	sprintf (string, I->format, ss); break;
							}
						}
						else if (I->mode == MOVIE_LABEL_IS_COL_C) {	/* Format a floatingpoint number */
							L_col = D->table[0]->segment[0]->data[I->col][use_frame];
							if ((type = gmt_M_type (GMT, GMT_IN, I->col)) == GMT_IS_ABSTIME) {	/* Time formatting */
								char date[GMT_LEN16] = {""}, clock[GMT_LEN16] = {""};
								if (I->kind == 'F')	/* Must give a proper ISO region */
									gmt_format_calendar (GMT, date, clock, &GMT->current.io.date_output, &GMT->current.io.clock_output, false, 1, L_col);
								else
									gmt_format_calendar (GMT, date, clock, &GMT->current.plot.calclock.date, &GMT->current.plot.calclock.clock, upper_case[k], flavor[k], L_col);
								if (GMT->current.plot.calclock.clock.skip)
									sprintf (string, "%s", date);
								else if (GMT->current.plot.calclock.date.skip)
									sprintf (string, "%s", clock);
								else
									sprintf (string, "%s%c%s", date, spacer, clock);
							}
							else {	/* Regular floating point (or latitude, etc.) */
								if (I->format[0] && movie_valid_format (I->format, 'd'))	/* Set as an integer */
									sprintf (string, I->format, (int)irint (L_col));
								else if (I->format[0])	/* Set as floating point */
									sprintf (string, I->format, L_col);
								else	/* Uses standard formatting*/
									gmt_ascii_format_one (GMT, string, L_col, type);
							}
						}
						else if (I->mode == MOVIE_LABEL_IS_COL_T) {	/* Place a word label */
							char *word = NULL, *trail = NULL, *orig = strdup (D->table[0]->segment[0]->text[use_frame]);
							col = 0;	trail = orig;
							while ((word = strsep (&trail, " \t")) != NULL && col != I->col) {
								if (*word != '\0')	/* Skip empty strings */
									col++;
							}
							if (word == NULL) {
								GMT_Report (API, GMT_MSG_ERROR, "Requested word number %d for label was not found - label skippedL\n", I->col);
								continue;
							}
							strcpy (L_txt, word);
							gmt_M_str_free (orig);
							if (I->format[0])	/* Use the given string format */
								sprintf (string, I->format, L_txt);
							else	/* Plot as is */
								strcpy (string, L_txt);
						}
						else if (I->mode == MOVIE_LABEL_IS_STRING)	/* Place a fixed string */
							strcpy (string, I->format);
						strcat (label, string);
						if (I->kind == 'F' && p == 0)
							strcat (label, "/");
						else if (p < (I->n_labels-1)) strcat (label, ";");
					}
					if (I->kind == 'F') strcat (label, "/0/1");
					gmt_set_comment (fp, Ctrl->In.mode, label);
				}
			}
			spacer = 'T';	/* For ISO time stamp as used in -R */
		}
		fclose (fp);	/* Done writing this parameter file */
	}

	if (Ctrl->M.active) {	/* Make the master frame plot */
		char master_file[GMT_LEN32] = {""};
		bool is_title = (Ctrl->M.frame < Ctrl->E.duration);
		/* Because format may differ and name will differ we just make a special script for this job */
		sprintf (master_file, "movie_master.%s", extension[Ctrl->In.mode]);
		GMT_Report (API, GMT_MSG_INFORMATION, "Create master frame script %s\n", master_file);
		if ((fp = fopen (master_file, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to create loop frame script file %s - exiting\n", master_file);
			fclose (Ctrl->In.fp);
			error = GMT_ERROR_ON_FOPEN;
			goto out_of_here;
		}
		if (is_title) {	/* Master frame is from the title sequence */
			if (Ctrl->M.frame < Ctrl->E.fade[GMT_IN])	/* During title fade-in */
				fade_level = 50 * (1.0 + cos (M_PI * Ctrl->M.frame / Ctrl->E.fade[GMT_IN]));
			else if (Ctrl->M.frame > (start = (Ctrl->E.duration -Ctrl->E.fade[GMT_OUT])))	/* During title fade-out */
				fade_level = 50 * (1.0 - cos (M_PI * (Ctrl->M.frame - start) / Ctrl->E.fade[GMT_OUT]));
			else /* No fading during main part of title */
				fade_level = 0.0;
			gmt_set_dvalue (fp, Ctrl->In.mode, "MOVIE_FADE", fade_level, 0);
		}
		else if (Ctrl->K.active) {
			sprintf (extra, "A+M+r,N+f%s", gmt_place_var (Ctrl->In.mode, "MOVIE_FADE"));	/* No cropping, image size is fixed, but fading may be in effect for some frames */
			if (Ctrl->K.fill) {strcat (extra, "+g"); strcat (extra, Ctrl->K.fill);}	/* Chose another fade color than black */
		}
		else
			sprintf (extra, "A+M+r");	/* No cropping, image size is fixed */
		if (Ctrl->G.active) {	/* Want to set a fixed background canvas color and/or outline - we do this via the psconvert -N option */
			if (!Ctrl->K.active) strcat (extra, ",N");	/* Need to switch to the -N option first */
			if (Ctrl->G.mode & 1) strcat (extra, "+p"), strcat (extra, Ctrl->G.pen);
			if (Ctrl->G.mode & 2) strcat (extra, "+g"), strcat (extra, Ctrl->G.fill);
		}
		if (is_title) {	/* Master frame is from the title sequence */
			if (Ctrl->E.PS) {	/* Need to place a background title first (which will be in parent dir when loop script is run) */
				strcat (extra, ",Mb../../");
				strcat (extra, Ctrl->E.file);
			}
		}
		else {	/* master is from main sequence */
			if (!access ("movie_background.ps", R_OK))	/* Need to place a background layer first (which is in parent dir when loop script is run) */
				strcat (extra, ",Mb../movie_background.ps");
			else if (Ctrl->S[MOVIE_PREFLIGHT].PS) {	/* Got a background PS layer directly */
				strcat (extra, ",Mb../../");
				strcat (extra, Ctrl->S[MOVIE_PREFLIGHT].file);
			}
			if (!access ("movie_foreground.ps", R_OK))	/* Need to append foreground layer at end (which is in parent dir when script is run) */
				strcat (extra, ",Mf../movie_foreground.ps");
			else if (Ctrl->S[MOVIE_POSTFLIGHT].PS) {	/* Got a foreground PS layer directly */
				strcat (extra, ",Mf../../");
				strcat (extra, Ctrl->S[MOVIE_POSTFLIGHT].file);
			}
		}
		if (Ctrl->H.active) {	/* Must pass the DownScaleFactor option to psconvert */
			sprintf (line, ",H%d", Ctrl->H.factor);
			strcat (extra, line);
		}
		gmt_set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
		gmt_set_comment (fp, Ctrl->In.mode, "Master frame loop script");
		fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a GMT_SESSION_NAME since sub-shells may mess things up */
		if (Ctrl->In.mode == GMT_DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to be the frame number */
			fprintf (fp, "set GMT_SESSION_NAME=%c1\n", var_token[Ctrl->In.mode]);
		else	/* On UNIX we use the script's PID as GMT_SESSION_NAME */
			gmt_set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
		gmt_set_comment (fp, Ctrl->In.mode, "Include static and frame-specific parameters");
		fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
		fprintf (fp, "%s movie_params_%c1.%s\n", load[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Include the frame parameters */
		fprintf (fp, "mkdir master\n");	/* Make a temp directory for this frame */
		fprintf (fp, "cd master\n");		/* cd to the temp directory */
		if (is_title) {	/* Process title page script or PS */
			if (Ctrl->E.PS) {	/* There is no title script, just a PS, so we make a dummy script that plots nothing */
				fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
				gmt_set_comment (fp, Ctrl->In.mode, "\tSet output PNG name and plot conversion parameters");
				fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
				fprintf (fp, "\tgmt figure %s %s", Ctrl->N.prefix, Ctrl->M.format);
				fprintf (fp, " %s", extra);
				if (strstr (Ctrl->M.format, "pdf") || strstr (Ctrl->M.format, "eps") || strstr (Ctrl->M.format, "ps"))
					{}	/* No dpu needed */
				else if (Ctrl->M.dpu_set)
					fprintf (fp, ",E%g", Ctrl->M.dpu);
				else
					fprintf (fp, ",E%s", gmt_place_var (Ctrl->In.mode, "MOVIE_DPU"));
				fprintf (fp, " -I../movie_params_%c1.%s\n", var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Pass the frame parameter file to figure via undocumented option -I */
				fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c DIR_DATA \"%s\"\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit, datadir);
				fprintf (fp, "\tgmt plot -R0/%g/0/%g -Jx1%c -X0 -Y0 -T\n", Ctrl->C.dim[GMT_X], Ctrl->C.dim[GMT_Y], Ctrl->C.unit);
				fprintf (fp, "gmt end\n");		/* Eliminate show from gmt end in this script */
			}
			else {	/* Read the title script */
				while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->E.fp)) {	/* Read the main script and copy to loop script, with some exceptions */
					if (gmt_is_gmtmodule (line, "begin")) {	/* Need to insert a gmt figure call after this line */
						fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
						if (has_conf && !strstr (line, "-C")) fprintf (fp, cpconf[Ctrl->In.mode], conf_file);
						gmt_set_comment (fp, Ctrl->In.mode, "\tSet output name and plot conversion parameters");
						fprintf (fp, "\tgmt figure %s %s", Ctrl->N.prefix, Ctrl->M.format);
						fprintf (fp, " %s", extra);
						if (strstr (Ctrl->M.format, "pdf") || strstr (Ctrl->M.format, "eps") || strstr (Ctrl->M.format, "ps"))
							{}	/* No dpu needed */
						else if (Ctrl->M.dpu_set)
							fprintf (fp, ",E%g", Ctrl->M.dpu);
						else
							fprintf (fp, ",E%s", gmt_place_var (Ctrl->In.mode, "MOVIE_DPU"));
						fprintf (fp, " -I../movie_params_%c1.%s\n", var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Pass the frame parameter file to figure via undocumented option -I */
						fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c DIR_DATA \"%s\"\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit, datadir);
					}
					else if (!strstr (line, "#!/")) {		/* Skip any leading shell incantation since already placed */
						if (gmt_is_gmt_end_show (line)) sprintf (line, "gmt end\n");		/* Eliminate show from gmt end in this script */
						else if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
						fprintf (fp, "%s", line);	/* Just copy the line as is */
					}
				}
				rewind (Ctrl->E.fp);	/* Get ready for main_frame reading */
			}
		}
		else {	/* Process main script */
			while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->In.fp)) {	/* Read the mainscript and copy to loop script, with some exceptions */
				if (gmt_is_gmtmodule (line, "begin")) {	/* Need to insert a gmt figure call after this line */
					fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
					if (has_conf && !strstr (line, "-C")) fprintf (fp, cpconf[Ctrl->In.mode], conf_file);
					gmt_set_comment (fp, Ctrl->In.mode, "\tSet output name and plot conversion parameters");
					fprintf (fp, "\tgmt figure %s %s", Ctrl->N.prefix, Ctrl->M.format);
					fprintf (fp, " %s", extra);
					if (strstr (Ctrl->M.format, "pdf") || strstr (Ctrl->M.format, "eps") || strstr (Ctrl->M.format, "ps"))
						{}	/* No dpu needed */
					else if (Ctrl->M.dpu_set)
						fprintf (fp, ",E%g", Ctrl->M.dpu);
					else
						fprintf (fp, ",E%s", gmt_place_var (Ctrl->In.mode, "MOVIE_DPU"));
					fprintf (fp, " -I../movie_params_%c1.%s\n", var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Pass the frame parameter file to figure via undocumented option -I */
					fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c DIR_DATA \"%s\"\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit, datadir);
				}
				else if (!strstr (line, "#!/"))	{	/* Skip any leading shell incantation since already placed */
					if (gmt_is_gmt_end_show (line)) {	/* Want to open the master plot and movie at the end */
						sprintf (line, "gmt end\n");	/* Eliminate show from gmt end in this script */
						Ctrl->M.view = true;			/* Backwards compatibility: Use +v modifier instead */
					}
					else if (strchr (line, '\n') == NULL) strcat (line, "\n");	/* In case the last line misses a newline */
					fprintf (fp, "%s", line);	/* Just copy the line as is */
				}
			}
			rewind (Ctrl->In.fp);	/* Get ready for main_frame reading */
		}
		gmt_set_comment (fp, Ctrl->In.mode, "Move master file up to top directory and cd up one level");
		if (Ctrl->In.mode == GMT_DOS_MODE)
			gmt_strrepc (topdir, '/', '\\');	/* Temporarily make DOS compatible */
		fprintf (fp, "%s %s.%s %s\n", mvfile[Ctrl->In.mode], Ctrl->N.prefix, Ctrl->M.format, topdir);	/* Move master plot up to top dir */
		if (Ctrl->In.mode == GMT_DOS_MODE)
			gmt_strrepc (topdir, '\\', '/');	/* Revert */
		fprintf (fp, "cd ..\n");	/* cd up to workdir */
		if (!Ctrl->Q.active) {	/* Remove the work dir and any files in it */
			gmt_set_comment (fp, Ctrl->In.mode, "Remove frame directory");
			fprintf (fp, "%s master\n", rmdir[Ctrl->In.mode]);
		}
		fclose (fp);	/* Done writing loop script */

#ifndef WIN32	/* Set executable bit if not Windows cmd */
		if (chmod (master_file, S_IRWXU)) {
			GMT_Report (API, GMT_MSG_ERROR, "Unable to make script %s executable - exiting\n", master_file);
			fclose (Ctrl->In.fp);
			error = GMT_RUNTIME_ERROR;
			goto out_of_here;
		}
#endif
		sprintf (cmd, "%s %s %*.*d", sys_cmd_nowait[Ctrl->In.mode], master_file, precision, precision, Ctrl->M.frame);
		if ((error = run_script (cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Running script %s returned error %d - exiting.\n", cmd, error);
			fclose (Ctrl->In.fp);
			error = GMT_RUNTIME_ERROR;
			goto out_of_here;
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "Single master plot (frame %d) built: %s.%s\n", Ctrl->M.frame, Ctrl->N.prefix, Ctrl->M.format);
		if (Ctrl->M.view) {	/* Play the movie master frame via gmt docs */
			snprintf (cmd, PATH_MAX, "%s/%s.%s", topdir, Ctrl->N.prefix, Ctrl->M.format);
			gmt_filename_set (cmd);	/* Protect filename spaces by substitution */
			if ((error = GMT_Call_Module (API, "docs", GMT_MODULE_CMD, cmd))) {
				GMT_Report (API, GMT_MSG_ERROR, "Failed to call docs\n");
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
		}
		if (!Ctrl->Q.active) {
			/* Delete the masterfile script */
			if (gmt_remove_file (GMT, master_file)) {	/* Delete the master_file script */
				GMT_Report (API, GMT_MSG_ERROR, "Unable to delete the master file script %s.\n", master_file);
				fclose (Ctrl->In.fp);
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
		}
		if (Ctrl->M.exit) {	/* Well, we are done */
			fclose (Ctrl->In.fp);
			/* Cd back up to the starting directory */
			if (chdir (topdir)) {	/* Should never happen but we do check */
				GMT_Report (API, GMT_MSG_ERROR, "Unable to change directory to starting directory - exiting.\n");
				perror (topdir);
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
			if (!Ctrl->Q.active) {	/* Delete the entire directory */
				/* Delete the entire working directory with batch jobs and tmp files */
				sprintf (line, "%s %s\n", rmdir[Ctrl->In.mode], tmpwpath);
				if ((error = system (line))) {
					GMT_Report (API, GMT_MSG_ERROR, "Deleting the working directory %s returned error %d.\n", workdir, error);
					error = GMT_RUNTIME_ERROR;
					goto out_of_here;
				}
			}
			if (Ctrl->Z.delete) {	/* Delete input scripts */
				if ((error = movie_delete_scripts (GMT, Ctrl)))
					goto out_of_here;
			}
			error = GMT_NOERROR;
			goto out_of_here;
		}
	}

	/* Now build the main loop script from the mainscript */

	sprintf (main_file, "movie_frame.%s", extension[Ctrl->In.mode]);
	GMT_Report (API, GMT_MSG_INFORMATION, "Create main movie frame script %s\n", main_file);
	if ((fp = fopen (main_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to create loop frame script file %s - exiting\n", main_file);
		fclose (Ctrl->In.fp);
		error = GMT_ERROR_ON_FOPEN;
		goto out_of_here;
	}
	if (Ctrl->K.active) {
		sprintf (extra, "A+M+r,N+f%s", gmt_place_var (Ctrl->In.mode, "MOVIE_FADE"));	/* No cropping, image size is fixed, but fading may be in effect for some frames */
		if (Ctrl->K.fill) {strcat (extra, "+g"); strcat (extra, Ctrl->K.fill);}	/* Chose another fade color than black */
	}
	else
		sprintf (extra, "A+M+r");	/* No cropping, image size is fixed */
	if (Ctrl->G.active) {	/* Want to set a fixed background canvas color and/or outline - we do this via the psconvert -N option */
		if (!Ctrl->K.active) strcat (extra, ",N");	/* Need to switch to the -N option first */
		if (Ctrl->G.mode & 1) strcat (extra, "+p"), strcat (extra, Ctrl->G.pen);
		if (Ctrl->G.mode & 2) strcat (extra, "+g"), strcat (extra, Ctrl->G.fill);
	}
	if (!access ("movie_background.ps", R_OK)) {	/* Need to place a background layer first (which will be in parent dir when loop script is run) */
		layers = true;
	}
	else if (Ctrl->S[MOVIE_PREFLIGHT].PS) {	/* Got a background PS layer directly */
		strcat (extra, ",Mb../../");
		strcat (extra, Ctrl->S[MOVIE_PREFLIGHT].file);
	}
	if (!access ("movie_foreground.ps", R_OK)) {	/* Need to append foreground layer at end (which will be in parent dir when script is run) */
		strcat (extra, ",Mf../movie_foreground.ps");
		layers = true;
	}
	else if (Ctrl->S[MOVIE_POSTFLIGHT].PS) {	/* Got a foreground PS layer directly */
		strcat (extra, ",Mf../../");
		strcat (extra, Ctrl->S[MOVIE_POSTFLIGHT].file);
	}
	if (Ctrl->H.active) {	/* Must pass the DownScaleFactor option to psconvert */
		char htxt[GMT_LEN16] = {""};
		sprintf (htxt, ",H%d", Ctrl->H.factor);
		strcat (extra, htxt);
	}
	gmt_set_script (fp, Ctrl->In.mode);					/* Write 1st line of a script */
	gmt_set_comment (fp, Ctrl->In.mode, "Main frame loop script");
	fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Hardwire a GMT_SESSION_NAME since sub-shells may mess things up */
	if (Ctrl->In.mode == GMT_DOS_MODE)	/* Set GMT_SESSION_NAME under Windows to be the frame number */
		fprintf (fp, "set GMT_SESSION_NAME=%c1\n", var_token[Ctrl->In.mode]);
	else	/* On UNIX we use the script's PID as GMT_SESSION_NAME */
		gmt_set_tvalue (fp, Ctrl->In.mode, true, "GMT_SESSION_NAME", "$$");
	fprintf (fp, "%s", export[Ctrl->In.mode]);			/* Turn off auto-display of figures if script has gmt end show */
	gmt_set_tvalue (fp, Ctrl->In.mode, true, "GMT_END_SHOW", "off");
	gmt_set_comment (fp, Ctrl->In.mode, "Include static and frame-specific parameters");
	fprintf (fp, "%s %s\n", load[Ctrl->In.mode], init_file);	/* Include the initialization parameters */
	fprintf (fp, "%s movie_params_%c1.%s\n", load[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Include the frame parameters */
	fprintf (fp, "mkdir %s\n", gmt_place_var (Ctrl->In.mode, "MOVIE_NAME"));	/* Make a temp directory for this frame */
	fprintf (fp, "cd %s\n", gmt_place_var (Ctrl->In.mode, "MOVIE_NAME"));		/* cd to the temp directory */
	while (gmt_fgets (GMT, line, PATH_MAX, Ctrl->In.fp)) {	/* Read the main script and copy to loop script, with some exceptions */
		if (gmt_is_gmtmodule (line, "begin")) {	/* Need to insert a gmt figure call after this line */
			fprintf (fp, "gmt begin\n");	/* Ensure there are no args here since we are using gmt figure instead */
			if (has_conf && !strstr (line, "-C")) fprintf (fp, cpconf[Ctrl->In.mode], conf_file);
			gmt_set_comment (fp, Ctrl->In.mode, "\tSet output PNG name and plot conversion parameters");
			fprintf (fp, "\tgmt figure ../%s %s", gmt_place_var (Ctrl->In.mode, "MOVIE_NAME"), frame_products);
			fprintf (fp, " E%s,%s", gmt_place_var (Ctrl->In.mode, "MOVIE_DPU"), extra);
			fprintf (fp, "%s", gmt_place_var (Ctrl->In.mode, "MOVIE_BACKGROUND"));
			fprintf (fp, " -I../movie_params_%c1.%s\n", var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Pass the frame parameter file to figure via undocumented option -I */
			fprintf (fp, "\tgmt set PS_MEDIA %g%cx%g%c DIR_DATA \"%s\" GMT_MAX_CORES 1\n", Ctrl->C.dim[GMT_X], Ctrl->C.unit, Ctrl->C.dim[GMT_Y], Ctrl->C.unit, datadir);
		}
		else if (!strstr (line, "#!/")) {		/* Skip any leading shell incantation since already placed */
			if (gmt_is_gmt_end_show (line)) sprintf (line, "gmt end\n");		/* Eliminate show from gmt end in this script */
			else if (strchr (line, '\n') == NULL)	/* In case the last line misses a newline */
				strcat (line, "\n");
			fprintf (fp, "%s", line);	/* Just copy the line as is */
		}
	}
	fclose (Ctrl->In.fp);	/* Done reading the main script */
	fprintf (fp, "cd ..\n");	/* cd up to parent dir */
	if (!Ctrl->Q.active) {	/* Delete evidence; otherwise we want to leave debug evidence when doing a single frame only */
		gmt_set_comment (fp, Ctrl->In.mode, "Remove frame directory and frame parameter file");
		fprintf (fp, "%s %s\n", rmdir[Ctrl->In.mode], gmt_place_var (Ctrl->In.mode, "MOVIE_NAME"));	/* Remove the work dir and any files in it */
		fprintf (fp, "%s movie_params_%c1.%s\n", rmfile[Ctrl->In.mode], var_token[Ctrl->In.mode], extension[Ctrl->In.mode]);	/* Remove the parameter file for this frame */
	}
	if (Ctrl->In.mode == GMT_DOS_MODE)	/* This is crucial to the "start /B ..." statement below to ensure the DOS process terminates */
		fprintf (fp, "exit\n");
	fclose (fp);	/* Done writing loop script */

#ifndef WIN32	/* Set executable bit if not Windows cmd */
	if (chmod (main_file, S_IRWXU)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to make script %s executable - exiting\n", main_file);
		error = GMT_RUNTIME_ERROR;
		goto out_of_here;
	}
#endif

	n_frames += Ctrl->E.duration;	/* This is the total set of frames to process */
	GMT_Report (API, GMT_MSG_INFORMATION, "Total frames to process: %u\n", n_frames);

	if (Ctrl->Q.scripts) {	/* No animation sequence generated */
		error = GMT_NOERROR;	/* We are done */
		goto out_of_here;
	}

	/* Finally, we can run all the frames in a controlled loop, launching new parallel jobs as cores become available */

	i_frame = first_i_frame = 0; n_frames_not_started = n_frames;
	frame = Ctrl->T.start_frame;
	n_cores_unused = MAX (1, Ctrl->x.n_threads - 1);			/* Remove one for the main movie module thread */
	status = gmt_M_memory (GMT, NULL, n_frames, struct MOVIE_STATUS);	/* Used to keep track of frame status */
	if (Ctrl->E.active) script_file = intro_file; else script_file = main_file;
	GMT_Report (API, GMT_MSG_INFORMATION, "Build frames using %u cores\n", n_cores_unused);
	/* START PARALLEL EXECUTION OF FRAME SCRIPTS */
	GMT_Report (API, GMT_MSG_INFORMATION, "Execute movie frame scripts in parallel\n");
	while (!done) {	/* Keep running jobs until all frames have completed */
		while (n_frames_not_started && n_cores_unused) {	/* Launch new jobs if possible */
#ifdef WIN32
			if (Ctrl->In.mode < 2)		/* A bash or csh run from Windows. Need to call via "start" to get parallel */
				sprintf (cmd, "start /B %s %s %*.*d", sys_cmd_nowait[Ctrl->In.mode], script_file, precision, precision, frame);
			else						/* Running batch, so no need for the above trick */
				sprintf (cmd, "%s %s %*.*d &", sys_cmd_nowait[Ctrl->In.mode], script_file, precision, precision, frame);
#else		/* Regular Linux/macOS shell behavior */
			sprintf (cmd, "%s %s %*.*d &", sys_cmd_nowait[Ctrl->In.mode], script_file, precision, precision, frame);
#endif

			GMT_Report (API, GMT_MSG_DEBUG, "Launch script for frame %*.*d\n", precision, precision, frame);
			if ((error = system (cmd))) {
				GMT_Report (API, GMT_MSG_ERROR, "Running script %s returned error %d - aborting.\n", cmd, error);
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
			status[i_frame].started = true;	/* We have now launched this frame job */
			frame++;			/* Advance to next frame for next launch */
			i_frame++;			/* Advance to next frame for next launch */
			n_frames_not_started--;		/* One less frame remaining */
			n_cores_unused--;		/* This core is now busy */
			if (Ctrl->E.active && frame == Ctrl->E.duration)	/* Past the title intro, now point to main script */
				script_file = main_file;
		}
		gmt_sleep (MOVIE_WAIT_TO_CHECK);	/* Wait 0.01 second - then check for completion of the PNG images */
		for (k = first_i_frame; k < i_frame; k++) {	/* Only loop over the range of frames that we know are currently in play */
			if (status[k].completed) continue;	/* Already finished with this frame */
			if (!status[k].started) continue;	/* Not started this frame yet */
			/* Here we can check if the frame job has completed by looking for the PNG product */
			sprintf (png_file, "%s_%*.*d.%s", Ctrl->N.prefix, precision, precision, Ctrl->T.start_frame+k, MOVIE_RASTER_EXTENSION);
			if (access (png_file, F_OK)) continue;	/* Not found yet */
			n_frames_completed++;		/* One more frame completed */
			status[k].completed = true;	/* Flag this frame as completed */
			n_cores_unused++;		/* Free up the core */
			percent = 100.0 * n_frames_completed / n_frames;
			GMT_Report (API, GMT_MSG_INFORMATION, "Frame %*.*d of %d completed [%5.1f %%]\n", precision, precision, k+1, n_frames, percent);
		}
		/* Adjust first_i_frame, if needed */
		while (first_i_frame < n_frames && status[first_i_frame].completed) first_i_frame++;
		if (n_frames_completed == n_frames) done = true;	/* All frames completed! */
	}
	/* END PARALLEL EXECUTION OF FRAME SCRIPTS */

	gmt_M_free (GMT, status);	/* Done with this structure array */

	/* Cd back up to the parent directory */
	if (chdir (topdir)) {	/* Should never happen but we should check */
		GMT_Report (API, GMT_MSG_ERROR, "Unable to change directory to starting directory - exiting.\n");
		perror (topdir);
		error = GMT_RUNTIME_ERROR;
		goto out_of_here;
	}

	if (Ctrl->F.active[MOVIE_GIF]) {	/* Want an animated GIF */
		/* Set up system call to gm (which we know exists) */
		unsigned int delay = urint (100.0 / Ctrl->D.framerate);	/* Delay to nearest ~1/100 s */
		char files[GMT_LEN32] = {""};
		files[0] = '*';
		if (Ctrl->F.skip) {	/* Only use every stride file */
			if (Ctrl->F.stride == 2 || Ctrl->F.stride == 20 || Ctrl->F.stride == 200 || Ctrl->F.stride == 2000)
				strcat (files, "[02468]");
			else if (Ctrl->F.stride == 5 || Ctrl->F.stride == 50 || Ctrl->F.stride == 500 || Ctrl->F.stride == 5000)
				strcat (files, "[05]");
			else	/* 10, 100, 1000, etc */
				strcat (files, "[0]");
			if (Ctrl->F.stride > 1000)
				strcat (files, "000");
			else if (Ctrl->F.stride > 100)
				strcat (files, "00");
			else if (Ctrl->F.stride > 10)
				strcat (files, "0");
		}
		sprintf (cmd, "gm convert -delay %u -loop %u +dither \"%s%c%s_%s.%s\" \"%s.gif\"", delay, Ctrl->F.loops, tmpwpath, dir_sep, Ctrl->N.prefix, files, MOVIE_RASTER_EXTENSION, Ctrl->N.prefix);
		gmt_sleep (MOVIE_PAUSE_A_SEC);	/* Wait 1 second to ensure all files are synced before building the movie */
		GMT_Report (API, GMT_MSG_NOTICE, "Running: %s\n", cmd);
		if ((error = system (cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Running GIF conversion returned error %d - exiting.\n", error);
			error = GMT_RUNTIME_ERROR;
			goto out_of_here;
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "GIF animation built: %s.gif\n", Ctrl->N.prefix);
		if (Ctrl->F.skip) GMT_Report (API, GMT_MSG_INFORMATION, "GIF animation reflects every %d frame only\n", Ctrl->F.stride);
	}
	if (Ctrl->A.active) {	/* Need to include audio track, possibly scaled to fit */
		if (Ctrl->A.exact) {	/* Need to get an exact fit */
			double video_duration = n_frames / Ctrl->D.framerate;	/* We can easily compute the animation length in seconds */
			/* Create ffprobe arguments to get duration of audio track in seconds */
			sprintf (cmd, "-i %s -show_entries format=duration -v quiet -of csv=\"p=0\"", Ctrl->A.file);
			GMT_Report (API, GMT_MSG_INFORMATION, "Running: ffprobe %s\n", cmd);
			if (gmt_run_process_get_first_line (GMT, "ffprobe", cmd, line))	/* Success */
				Ctrl->A.duration = atof (line);
			else {
				error = GMT_RUNTIME_ERROR;
				GMT_Report (API, GMT_MSG_ERROR, "Determining length of audio track %s returned error %d - exiting.\n", Ctrl->A.file, error);
				goto out_of_here;
			}
			if ((audio_stretch = (Ctrl->A.duration / video_duration)) < 0.5 || audio_stretch > 2.0) {
				GMT_Report (API, GMT_MSG_ERROR, "Audio track %s must be stretched by %lg which exceeds the ffmpeg 0.5-2.0 valid range - exiting.\n", Ctrl->A.file, audio_stretch);
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
			/* Create audio options for ffmpeg to include the audiofile but first scale it by adio_stretch so it fits the length of the animation */
			sprintf (audio_option, " -i %s -af atempo=%lg", Ctrl->A.file, audio_stretch);
		}
		else	/* No stretching - just include the audio file as is */
			sprintf (audio_option, " -i %s", Ctrl->A.file);
	}
	if (Ctrl->F.active[MOVIE_MP4]) {
		/* Set up system call to FFmpeg (which we know exists) */
		if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG))
			sprintf (extra, "verbose");
		if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION))
			sprintf (extra, "warning");
		else if (gmt_M_is_verbose (GMT, GMT_MSG_WARNING))
			sprintf (extra, "warning");
		else
			sprintf (extra, "quiet");
		sprintf (png_file, "%%0%dd", precision);
		sprintf (cmd, "ffmpeg -loglevel %s -f image2 -framerate %g -y -i \"%s%c%s_%s.%s\"%s -vcodec libx264 %s -pix_fmt yuv420p %s.mp4",
			extra, Ctrl->D.framerate, tmpwpath, dir_sep, Ctrl->N.prefix, png_file, MOVIE_RASTER_EXTENSION, audio_option,
			(Ctrl->F.options[MOVIE_MP4]) ? Ctrl->F.options[MOVIE_MP4] : "", Ctrl->N.prefix);
		gmt_sleep (MOVIE_PAUSE_A_SEC);	/* Wait 1 second to ensure all files are synced before building the movie */
		GMT_Report (API, GMT_MSG_NOTICE, "Running: %s\n", cmd);
		if ((error = system (cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Running FFmpeg conversion to MP4 returned error %d - exiting.\n", error);
			error = GMT_RUNTIME_ERROR;
			goto out_of_here;
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "MP4 movie built: %s.mp4\n", Ctrl->N.prefix);
		if (Ctrl->F.view[MOVIE_MP4]) {	/* Play the movie automatically via gmt docs */
			snprintf (cmd, PATH_MAX, "%s.mp4", Ctrl->N.prefix);
			gmt_filename_set (cmd);	/* Protect filename spaces by substitution */
			if ((error = GMT_Call_Module (API, "docs", GMT_MODULE_CMD, cmd))) {
				GMT_Report (API, GMT_MSG_ERROR, "Failed to call docs\n");
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
		}
	}
	if (Ctrl->F.active[MOVIE_WEBM]) {
		static char *vpx[2] = {"libvpx", "libvpx-vp9"}, *pix_fmt[2] = {"yuv420p", "yuva420p"};
		/* Set up system call to FFmpeg (which we know exists) */
		if (gmt_M_is_verbose (GMT, GMT_MSG_DEBUG))
			sprintf (extra, "verbose");
		if (gmt_M_is_verbose (GMT, GMT_MSG_INFORMATION))
			sprintf (extra, "warning");
		else if (gmt_M_is_verbose (GMT, GMT_MSG_WARNING))
			sprintf (extra, "warning");
		else
			sprintf (extra, "quiet");
		sprintf (png_file, "%%0%dd", precision);
		sprintf (cmd, "ffmpeg -loglevel %s -f image2 -framerate %g -y -i \"%s%c%s_%s.%s\"%s -vcodec %s %s -pix_fmt %s %s.webm",
			extra, Ctrl->D.framerate, tmpwpath, dir_sep, Ctrl->N.prefix, png_file, MOVIE_RASTER_EXTENSION, audio_option, vpx[Ctrl->F.transparent],
			(Ctrl->F.options[MOVIE_WEBM]) ? Ctrl->F.options[MOVIE_WEBM] : "", pix_fmt[Ctrl->F.transparent], Ctrl->N.prefix);
		gmt_sleep (MOVIE_PAUSE_A_SEC);	/* Wait 1 second to ensure all files are synced before building the movie */
		GMT_Report (API, GMT_MSG_NOTICE, "Running: %s\n", cmd);
		if ((error = system (cmd))) {
			GMT_Report (API, GMT_MSG_ERROR, "Running FFmpeg conversion to webM returned error %d - exiting.\n", error);
			error = GMT_RUNTIME_ERROR;
			goto out_of_here;
		}
		GMT_Report (API, GMT_MSG_INFORMATION, "WebM movie built: %s.webm\n", Ctrl->N.prefix);
		if (Ctrl->F.view[MOVIE_WEBM]) {	/* Play the movie automatically via gmt docs */
			snprintf (cmd, PATH_MAX, "%s.webm", Ctrl->N.prefix);
			gmt_filename_set (cmd);	/* Protect filename spaces by substitution */
			if ((error = GMT_Call_Module (API, "docs", GMT_MODULE_CMD, cmd))) {
				GMT_Report (API, GMT_MSG_ERROR, "Failed to call docs\n");
				error = GMT_RUNTIME_ERROR;
				goto out_of_here;
			}
		}
	}

	/* Prepare the cleanup script */
	sprintf (cleanup_file, "movie_cleanup.%s", extension[Ctrl->In.mode]);
	if ((fp = fopen (cleanup_file, "w")) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to create cleanup file %s - exiting\n", cleanup_file);
		error = GMT_ERROR_ON_FOPEN;
		goto out_of_here;
	}
	gmt_set_script (fp, Ctrl->In.mode);		/* Write 1st line of a script */
	if (Ctrl->Z.active) {	/* Want to delete the entire frame directory */
		gmt_set_comment (fp, Ctrl->In.mode, "Cleanup script removes working directory with frame files");
		/* Delete the entire working directory with batch jobs and tmp files */
		fprintf (fp, "%s %s\n", rmdir[Ctrl->In.mode], tmpwpath);
	}
	else {	/* Just delete the remaining scripts and PS files */
		GMT_Report (API, GMT_MSG_INFORMATION, "%u frame PNG files saved in directory: %s\n", n_frames, workdir);
		if (Ctrl->S[MOVIE_PREFLIGHT].active)	/* Remove the preflight script */
			fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], tmpwpath, dir_sep, pre_file);
		if (Ctrl->S[MOVIE_POSTFLIGHT].active)	/* Remove the postflight script */
			fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], tmpwpath, dir_sep, post_file);
		fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], tmpwpath, dir_sep, init_file);	/* Delete the init script */
		fprintf (fp, "%s %s%c%s\n", rmfile[Ctrl->In.mode], tmpwpath, dir_sep, main_file);	/* Delete the main script */
		if (layers)
			fprintf (fp, "%s %s%c*.ps\n", rmfile[Ctrl->In.mode], tmpwpath, dir_sep);	/* Delete any PostScript layers */
	}
	fclose (fp);
#ifndef WIN32	/* Set executable bit if not Windows cmd */
	if (chmod (cleanup_file, S_IRWXU)) {
		GMT_Report (API, GMT_MSG_ERROR, "Unable to make cleanup script %s executable - exiting\n", cleanup_file);
		error = GMT_RUNTIME_ERROR;
		goto out_of_here;
	}
#endif
	if (!Ctrl->Q.active) {
		/* Run cleanup script at the end */
		if (Ctrl->In.mode == GMT_DOS_MODE)
			error = system (cleanup_file);
		else {
			sprintf (cmd, "%s %s", sys_cmd_nowait[Ctrl->In.mode], cleanup_file);
			error = system (cmd);
		}
		if (error) {
			GMT_Report (API, GMT_MSG_ERROR, "Running cleanup script %s returned error %d - exiting.\n", cleanup_file, error);
			error = GMT_RUNTIME_ERROR;
			goto out_of_here;
		}
	}

	if (Ctrl->Z.delete) {	/* Delete input scripts */
		if ((error = movie_delete_scripts (GMT, Ctrl)))
			goto out_of_here;
	}

	/* Finally, delete the clean-up script separately since under DOS we got complaints when we had it delete itself (which works under *nix) */
	if (!Ctrl->Q.active && gmt_remove_file (GMT, cleanup_file)) {	/* Delete the cleanup script itself */
		GMT_Report (API, GMT_MSG_ERROR, "Unable to delete the cleanup script %s.\n", cleanup_file);
		error = GMT_RUNTIME_ERROR;
		goto out_of_here;
	}

out_of_here:
	/* Remove the environmental reminder we are building a movie */
#ifdef WIN32
	if (_putenv ("GMT_MOVIE="))
#else
	if (unsetenv ("GMT_MOVIE"))
#endif
		GMT_Report (API, GMT_MSG_WARNING, "Unable to unset GMT_MOVIE which was used to inform GMT that movie scripts will be building a movie\n");

	Return (error);
}
