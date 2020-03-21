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
 *
 * Brief synopsis: psconvert converts one or several PostScript file(s) to other formats using Ghostscript.
 * It works by modifying the page size in order that the image will have a size
 * which is specified by the BoundingBox.
 * As an option, a tight BoundingBox may be computed.
 * psconvert uses the ideas of the EPS2XXX.m from Primoz Cermelj published in MatLab Central
 * and of psbbox.sh of Remko Scharroo.
 *
 *
 *--------------------------------------------------------------------*/
/*
 * Authors:	Joaquim Luis and Remko Scharroo
 * Date:	1-JAN-2010
 * Version:	6 API
 */

#include "gmt_dev.h"

#define THIS_MODULE_CLASSIC_NAME	"psconvert"
#define THIS_MODULE_MODERN_NAME	"psconvert"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Convert [E]PS file(s) to other formats using Ghostscript"
#define THIS_MODULE_KEYS	"<X{+,FI)"
#define THIS_MODULE_NEEDS	""
#define THIS_MODULE_OPTIONS "-V"

#ifdef	__APPLE__
	/* Apple Xcode expects _Nullable to be defined but it is not if gcc */
#ifndef _Nullable
#	define _Nullable
#	endif
#	endif

#ifdef _WIN32
#	define dup2 _dup2
	/* Various shared-library functions declared in gmt_sharedlibs.c */
	EXTERN_MSC void *dlopen (const char *module_name, int mode);
	EXTERN_MSC int dlclose (void *handle);
	EXTERN_MSC void *dlsym (void *handle, const char *name);
	EXTERN_MSC char *dlerror (void);
#	ifndef RTLD_LAZY
#		define RTLD_LAZY 1
#	endif
#else	/* Standard Unix things */
#	include <dlfcn.h>
#endif

EXTERN_MSC void gmt_str_toupper (char *string);
EXTERN_MSC void gmt_handle5_plussign (struct GMT_CTRL *GMT, char *in, char *mods, unsigned way);

#ifdef WIN32	/* Special for Windows */
#	include <windows.h>
#	include <process.h>
#	define getpid _getpid
#	define dup2 _dup2
#	define execv _execv
#	define pipe _pipe
	static char quote = '\"';
	static char *squote = "\"";
#else
	static char quote = '\'';
	static char *squote = "\'";
	struct popen2 {
		int fd[2];		/* The input [0] and output [1] file descriptors */
		int n_closed;		/* Number of times we have called gmt_pclose */
		pid_t child_pid;	/* Pid of child */
	};
#endif

enum GMT_GS_Devices {
	GS_DEV_EPS = 0,
	GS_DEV_PDF,
	GS_DEV_SVG,
	GS_DEV_JPG,
	GS_DEV_PNG,
	GS_DEV_PPM,
	GS_DEV_TIF,
	GS_DEV_BMP,
	GS_DEV_TPNG,      /* PNG with transparency */
	GS_DEV_JPGG,      /* These are grayscale versions */
	GS_DEV_PNGG,
	GS_DEV_TIFG,
	GS_DEV_BMPG,
	N_GS_DEVICES};	/* Number of supported GS output devices */

enum GMT_KML_Elevation {
	KML_GROUND_ABS = 0,
	KML_GROUND_REL,
	KML_ABS,
	KML_SEAFLOOR_REL,
	KML_SEAFLOOR_ABS,
	N_KML_ELEVATIONS};

enum psconv_alias {
	PSC_LINES = 0,
	PSC_TEXT = 1,
	PSC_GEO = 2
};

#define add_to_list(list,item) { if (list[0]) strcat (list, " "); strcat (list, item); }
#define add_to_qlist(list,item) { if (list[0]) strcat (list, " "); strcat (list, squote);  strcat (list, item); strcat (list, squote); }

struct PS2RASTER_CTRL {
	struct PS2R_In {	/* Input file info */
		unsigned int n_files;
	} In;
	struct PS2R_A {             /* -A[+f<fade>][+g<fill>][+m<margins>][+n][+p<pen>][+r][+s|S[m]<width>[/<height>]][+u] */
		bool active;
		bool max;          /* Only scale if dim exceeds the given size */
		bool round;        /* Round HiRes BB instead of ceil (+r) */
		bool strip;        /* Remove the -U time-stamp (+u)*/
		bool resize;       /* Resize to a user selected size */
		bool rescale;      /* Resize to a user selected scale factor */
		bool outline;      /* Draw frame around plot with selected pen (+p) [0.25p] */
		bool paint;        /* Paint box behind plot with selected fill (+g) */
		bool crop;         /* If true we must find the BB; turn off via -A+n */
		bool fade;         /* If true we must fade out the plot to black*/
		double scale;      /* Scale factor to go along with the 'rescale' option */
		double fade_level;      /* Fade to black at this level of transparency */
		double new_size[2];
		double margin[4];
		double new_dpi_x, new_dpi_y;
		struct GMT_PEN pen;
		struct GMT_FILL fill;
	} A;
	struct PS2R_C {	/* -C<option> */
		bool active;
		char arg[GMT_LEN256];
	} C;
	struct PS2R_D {	/* -D<dir> */
		bool active;
		char *dir;
	} D;
	struct PS2R_E {	/* -E<resolution> */
		bool active;
		double dpi;
	} E;
	struct PS2R_F {	/* -F<out_name> */
		bool active;
		char *file;
	} F;
	struct PS2R_G {	/* -G<GSpath> */
		bool active;
		char *file;
	} G;
	struct PS2R_H {	/* -H<factor> */
		bool active;
		int factor;
	} H;
	struct PS2R_I {	/* -I */
		bool active;
	} I;
	struct PS2R_L {	/* -L<listfile> */
		bool active;
		char *file;
	} L;
	struct PS2R_M {	/* -Mb|f<PSfile> */
		bool active;
		char *file;
	} M[2];
	struct PS2R_P {	/* -P */
		bool active;
	} P;
	struct PS2R_Q {	/* -Q[g|t]<bits> -Qp */
		bool active;
		bool on[3];	/* [0] for graphics, [1] for text antialiasing, [2] for pdfmark GeoPDF */
		unsigned int bits[2];
	} Q;
	struct PS2R_S {	/* -S */
		bool active;
	} S;
	struct PS2R_T {	/* -T */
		bool active;
		int eps;	/* 1 if we want to make EPS, -1 with setpagedevice (possibly in addition to another format) */
		int ps;		/* 1 if we want to save the final PS under "modern" setting */
		int device;	/* May be negative */
	} T;
	struct PS2R_W {	/* -W -- for world file production */
		bool active;
		bool folder;
		bool warp;
		bool kml;
		unsigned int mode;	/* 0 = clamp at ground, 1 is relative to ground, 2 is absolute 3 is relative to seafloor, 4 is clamp at seafloor */
		int min_lod, max_lod;	/* minLodPixels and maxLodPixels settings */
		int min_fade, max_fade;	/* minFadeExtent and maxFadeExtent settings */
		char *doctitle;		/* Name of KML document */
		char *overlayname;	/* Name of the image overlay */
		char *URL;		/* URL of remote site */
		char *foldername;	/* Name of KML folder */
		double altitude;
	} W;
	struct PS2R_Z { /* -Z */
		bool active;
	} Z;
};

#ifdef WIN32	/* Special for Windows */
	GMT_LOCAL int ghostbuster(struct GMTAPI_CTRL *API, struct PS2RASTER_CTRL *C);
#else
	/* Abstraction to get popen to do bidirectional read/write */
struct popen2 * gmt_popen2 (const char *cmdline) {
	struct popen2 *F = NULL;
	/* Must implement a bidirectional popen instead */
	pid_t p;
	int pipe_stdin[2] = {0, 0}, pipe_stdout[2] = {0, 0};

	if (pipe (pipe_stdin))  return NULL;
	if (pipe (pipe_stdout)) return NULL;

	printf ("pipe_stdin[0] = %d,  pipe_stdin[1]  = %d\n", pipe_stdin[0], pipe_stdin[1]);
	printf ("pipe_stdout[0] = %d, pipe_stdout[1] = %d\n", pipe_stdout[0], pipe_stdout[1]);

	if ((p = fork()) < 0) return NULL; /* Fork failed */

	if (p == 0) { /* child */
		close (pipe_stdin[1]);
		dup2 (pipe_stdin[0], 0);
		close (pipe_stdout[0]);
		dup2 (pipe_stdout[1], 1);
		execl ("/bin/sh", "sh", "-c", cmdline, NULL);
		perror ("execl"); exit (99);
	}
	/* Return the file handles back via structure */
	F = calloc (1, sizeof (struct popen2));
	F->child_pid = p;
	F->fd[1] = pipe_stdin[1];
	F->fd[0] = pipe_stdout[0];
	return F;
}

#ifndef _WIN32
#include <signal.h>
#include <sys/wait.h>
#endif

void gmt_pclose2 (struct popen2 **Faddr, int dir) {
	struct popen2 *F = *Faddr;
	F->n_closed++;
	close (F->fd[dir]);	/* Close this pipe */
	if (F->n_closed == 2) {	/* Done so free object */
		/* Done, kill child */
#ifndef _WIN32
    	printf("kill(%d, 0) -> %d\n", F->child_pid, kill(F->child_pid, 0));
    	printf("waitpid() -> %d\n", waitpid(F->child_pid, NULL, 0));
    	printf("kill(%d, 0) -> %d\n", F->child_pid, kill(F->child_pid, 0));
#endif
		free (F);
		*Faddr = NULL;
	}
}
#endif

GMT_LOCAL int parse_A_settings (struct GMT_CTRL *GMT, char *arg, struct PS2RASTER_CTRL *Ctrl) {
	/* Syntax:
	 * New : -A[+f<fade>][+g<fill>][+m<margins>][+n][+p<pen>][+r][+s|S[m]<width>[/<height>]][+u]
	 * Old : -A[-][u][<margins>][+g<fill>][+p<pen>][+r][+s|S[m]<width>[/<height>]]
	 */

	unsigned int pos = 0, error = 0;
	int j, k = 0, trim_j = -1;
	char txt[GMT_LEN128] = {""}, p[GMT_LEN128] = {""};
	char txt_a[GMT_LEN64] = {""}, txt_b[GMT_LEN64] = {""}, txt_c[GMT_LEN64] = {""}, txt_d[GMT_LEN64] = {""};

	Ctrl->A.active = Ctrl->A.crop = true;	/* The default is to crop unless we get -A+n */

	/* For historical reasons we may encounter -A-<stuff> or -Au- so check for both */

	/* The next lines down to NEW is simply backwards compatible parsing */
	if (arg[k] == 'u') {Ctrl->A.strip = true; k++;}		/* Eliminate GMT time stamp */
	else if (arg[k] == '-') {Ctrl->A.crop = false; k++;}	/* No cropping requested */
	/* If there are +modifiers later we need to temporarily disable them: */
	for (j = k; arg[j] && arg[j] != '+'; j++);	/* Find position of first + */
	if (arg[j] == '+') arg[j] = 0, trim_j = j;	/* Remember that position and chop off modifiers */
	if (arg[k] && arg[k] != '+') {	/* Also specified desired margin(s) using the old syntax */
		j = sscanf (&arg[k], "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
		switch (j) {
			case 1:	/* Got uniform margin */
				Ctrl->A.margin[XLO] = Ctrl->A.margin[XHI] = Ctrl->A.margin[YLO] = Ctrl->A.margin[YHI] = gmt_M_to_points (GMT, txt_a);
				break;
			case 2:	/* Got separate x/y margins */
				Ctrl->A.margin[XLO] = Ctrl->A.margin[XHI] = gmt_M_to_points (GMT, txt_a);
				Ctrl->A.margin[YLO] = Ctrl->A.margin[YHI] = gmt_M_to_points (GMT, txt_b);
				break;
			case 4:	/* Got different margins for all sides */
				Ctrl->A.margin[XLO] = gmt_M_to_points (GMT, txt_a);
				Ctrl->A.margin[XHI] = gmt_M_to_points (GMT, txt_b);
				Ctrl->A.margin[YLO] = gmt_M_to_points (GMT, txt_c);
				Ctrl->A.margin[YHI] = gmt_M_to_points (GMT, txt_d);
				break;
			default:
				error++;
				GMT_Report (Ctrl, GMT_MSG_ERROR, "-A: Must specify 1, 2, or 4 margins\n");
				break;
		}
	}
	if (trim_j >= 0) arg[trim_j] = '+';	/* Restore the chopped off section */
	/* Here we are doing the NEW syntax parsing */
	strncpy (txt, arg, GMT_LEN128-1);
	while (!error && (gmt_strtok (txt, "+", &pos, p))) {
		switch (p[0]) {
			case 'f':	/* Fade out */
				Ctrl->A.fade = true;
				if (!p[1]) {
					GMT_Report (Ctrl, GMT_MSG_ERROR, "-A+f: Append the fade setting in 0-100 range.\n");
					error++;
				}
				else if ((Ctrl->A.fade_level = atof (&p[1])) < 0.0 || Ctrl->A.fade_level > 100.0) {
					GMT_Report (Ctrl, GMT_MSG_ERROR, "-A+f: Must specify fading in 0 (no fade) - 100 (fully faded) range.\n");
					error++;
				}
				Ctrl->A.fade_level *= 0.01;	/* Normalized */
				break;
			case 'g':	/* Fill background */
				Ctrl->A.paint = true;
				if (!p[1]) {
					GMT_Report (Ctrl, GMT_MSG_ERROR, "-A+g: Append the background fill\n");
					error++;
				}
				else if (gmt_getfill (GMT, &p[1], &Ctrl->A.fill)) {
					gmt_pen_syntax (GMT, 'A', NULL, "sets background fill attributes", 0);
					error++;
				}
				break;
			case 'm':	/* Margins */
				j = sscanf (&p[1], "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
				switch (j) {
					case 1:	/* Got uniform margin */
						Ctrl->A.margin[XLO] = Ctrl->A.margin[XHI] = Ctrl->A.margin[YLO] = Ctrl->A.margin[YHI] = gmt_M_to_points (GMT, txt_a);
						break;
					case 2:	/* Got separate x/y margins */
						Ctrl->A.margin[XLO] = Ctrl->A.margin[XHI] = gmt_M_to_points (GMT, txt_a);
						Ctrl->A.margin[YLO] = Ctrl->A.margin[YHI] = gmt_M_to_points (GMT, txt_b);
						break;
					case 4:	/* Got different margins for all sides */
						Ctrl->A.margin[XLO] = gmt_M_to_points (GMT, txt_a);
						Ctrl->A.margin[XHI] = gmt_M_to_points (GMT, txt_b);
						Ctrl->A.margin[YLO] = gmt_M_to_points (GMT, txt_c);
						Ctrl->A.margin[YHI] = gmt_M_to_points (GMT, txt_d);
						break;
					default:
						error++;
						GMT_Report (Ctrl, GMT_MSG_ERROR, "-A+m: Must specify 1, 2, or 4 margins\n");
						break;
				}
				break;
			case 'n':	/* No crop */
				Ctrl->A.crop = false;
				break;
			case 'p':	/* Draw outline */
				Ctrl->A.outline = true;
				if (!p[1])
					Ctrl->A.pen = GMT->current.setting.map_default_pen;
				else if (gmt_getpen (GMT, &p[1], &Ctrl->A.pen)) {
					gmt_pen_syntax (GMT, 'A', NULL, "sets background outline pen attributes", 0);
					error++;
				}
				break;
			case 'r':	/* Round */
				Ctrl->A.round = true;
				break;
			case 'S':	/* New size via a scale factor */
				Ctrl->A.rescale = true;
				Ctrl->A.scale = atof(&p[1]);
				break;
			case 's':	/* New size +s[m]<width>[/<height>] */
				Ctrl->A.resize = true;
				if (p[1] == 'm') { Ctrl->A.max = true, k = 2;} else k = 1;
				j = sscanf (&p[k], "%[^/]/%s", txt_a, txt_b);
				switch (j) {
					case 1:	/* Got width only. Height will be computed later */
						Ctrl->A.new_size[0] = gmt_M_to_points (GMT, txt_a);
						break;
					case 2:	/* Got separate width/height */
						Ctrl->A.new_size[0] = gmt_M_to_points (GMT, txt_a);
						Ctrl->A.new_size[1] = gmt_M_to_points (GMT, txt_b);
						break;
					default:
						GMT_Report (Ctrl, GMT_MSG_ERROR, "Option -A+s[m]<width[/height]>: Wrong size parameters\n");
						error++;
						break;
					}
				break;
			case 'u':	/* Strip timestamp */
				Ctrl->A.strip = true;
				break;
		}
	}

	if (Ctrl->A.rescale && Ctrl->A.resize) {
		GMT_Report (Ctrl, GMT_MSG_ERROR, "Option -A+s|S: Cannot set both -A+s and -A+S\n");
		error++;
	}
	else if (Ctrl->A.rescale)    /* But we can. This makes the coding simpler later on */
		Ctrl->A.resize = true;

	return (error);
}

GMT_LOCAL int parse_GE_settings (struct GMT_CTRL *GMT, char *arg, struct PS2RASTER_CTRL *C) {
	/* Syntax: -W[+g][+k][+t<doctitle>][+n<layername>][+a<altmode>][+l<lodmin>/<lodmax>] */

	unsigned int pos = 0, error = 0;
	char p[GMT_LEN256] = {""};
	gmt_M_unused(GMT);

	C->W.active = true;
	while (gmt_getmodopt (GMT, 'W', arg, "afgklnotu", &pos, p, &error) && error == 0) {	/* Looking for +a, etc */
		switch (p[0]) {
			case 'a':	/* Altitude setting */
				switch (p[1]) {	/* Check which altitude mode we selected */
					case 'G':
						C->W.mode = KML_GROUND_ABS;
						break;
					case 'g':
						C->W.mode = KML_GROUND_REL;
						C->W.altitude = atof (&p[2]);
						break;
					case 'A':
						C->W.mode = KML_ABS;
						C->W.altitude = atof (&p[2]);
						break;
					case 's':
						C->W.mode = KML_SEAFLOOR_REL;
						C->W.altitude = atof (&p[2]);
						break;
					case 'S':
						C->W.mode = KML_SEAFLOOR_ABS;
						break;
					default:
						GMT_Report (C, GMT_MSG_ERROR, "Option -W+a<mode>[par]: Unrecognized altitude mode %c\n", p[1]);
						error++;
						break;
				}
				break;
			case 'f':	/* Set fading options in KML */
				sscanf (&p[1], "%d/%d", &C->W.min_fade, &C->W.max_fade);
				break;
			case 'g':	/* Use gdal to make geotiff */
				C->W.warp = true;
				break;
			case 'k':	/* Produce a KML file */
				C->W.kml = true;
				break;
			case 'l':	/* Set KML level of detail for image */
				sscanf (&p[1], "%d/%d", &C->W.min_lod, &C->W.max_lod);
				break;
			case 'n':	/* Set KML document layer name */
				gmt_M_str_free (C->W.overlayname);	/* Already set, free then reset */
				C->W.overlayname = gmt_assign_text (GMT, p);
				break;
			case 'o':	/* Produce a KML overlay as a folder subset */
				C->W.folder = true;
				gmt_M_str_free (C->W.foldername);	/* Already set, free then reset */
				C->W.foldername = gmt_assign_text (GMT, p);
				break;
			case 't':	/* Set KML document title */
				gmt_M_str_free (C->W.doctitle);	/* Already set, free then reset */
				C->W.doctitle = gmt_assign_text (GMT, p);
				break;
			case 'u':	/* Specify a remote address for image */
				gmt_M_str_free (C->W.URL);	/* Already set, free then reset */
				C->W.URL = gmt_assign_text (GMT, p);
				break;
			default:
				GMT_Report (C, GMT_MSG_ERROR, "Option -W+<opt>: Unrecognized option selection %c\n", p[1]);
				error++;
				break;
		}
	}
	return (error);
}

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PS2RASTER_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct PS2RASTER_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
#ifdef WIN32
	if (ghostbuster(GMT->parent, C) != GMT_NOERROR)  /* Try first to find the gspath from registry */
		C->G.file = strdup ("gswin64c");     /* Fall back to this default and expect a miracle */
#else
	C->G.file = strdup ("gs");
#endif
	C->D.dir = strdup (".");

	C->W.doctitle = strdup ("GMT KML Document");
	C->W.overlayname = strdup ("GMT Image Overlay");
	C->W.foldername = strdup ("GMT Image Folder");

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct PS2RASTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->D.dir);
	gmt_M_str_free (C->F.file);
	gmt_M_str_free (C->G.file);
	gmt_M_str_free (C->L.file);
	gmt_M_str_free (C->M[0].file);
	gmt_M_str_free (C->M[1].file);
	gmt_M_str_free (C->W.doctitle);
	gmt_M_str_free (C->W.overlayname);
	gmt_M_str_free (C->W.foldername);
	gmt_M_str_free (C->W.URL);
	gmt_M_free (GMT, C);
}

#define GMT_CONV3_LIMIT	 1.0e-3
GMT_LOCAL double smart_ceil (double x) {
	/* Avoid ceil (integer+[0-0.0099999999999]) from increasing size by a full pixel */
	double fx = floor(x);
	if ((x - fx) > GMT_CONV3_LIMIT)	/* Only ceil if we exceed this limit */
		fx = ceil (x);
	return (fx);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	const char *name = gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_CLASSIC_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: %s <psfile1> <psfile2> <...> -A[+f<fade>][+g<fill>][+m<margins>][+n][+p[<pen>]][+r][+s[m]|S<width>[/<height>]][+u]\n", name);
	GMT_Message (API, GMT_TIME_NONE, "\t[-C<gs_option>] [-D<dir>] [-E<resolution>] [-F<out_name>] [-G<gs_path>] [-H<factor>] [-I] [-L<listfile>] [-Mb|f<psfile>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-P] [-Q[g|p|t]1|2|4] [-S] [-Tb|e|E|f|F|g|G|j|m|s|t[+m]] [%s]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-W[+a<mode>[<alt]][+f<minfade>/<maxfade>][+g][+k][+l<lodmin>/<lodmax>][+n<name>][+o<folder>][+t<title>][+u<URL>]]\n");
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC)
		GMT_Message (API, GMT_TIME_NONE, "\t[-Z] ");
	GMT_Message (API, GMT_TIME_NONE, "[%s]\n", GMT_PAR_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\tWorks by modifying the page size in order that the resulting\n");
	GMT_Message (API, GMT_TIME_NONE, "\timage will have the size specified by the BoundingBox.\n");
	GMT_Message (API, GMT_TIME_NONE, "\tAs an option, a tight BoundingBox may be computed.\n\n");
	GMT_Message (API, GMT_TIME_NONE, "\t<psfile(s)> PostScript file(s) to be converted.\n");
	if (API->external)
		GMT_Message (API, GMT_TIME_NONE, "\tTo access the current internal GMT plot, specify <psfile> as \"=\".\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Adjust the BoundingBox to the minimum required by the image contents.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +f<fade> (0-100) to fade entire plot to black (100%% fade)[no fading].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +g<paint> to paint the BoundingBox [no paint].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +m<margin(s)> to enlarge the BoundingBox, with <margin(s)> being\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     <off> for uniform margin for all 4 sides,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     <xoff>/<yoff> for separate x- and y-margins, or\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     <woff>/<eoff>/<soff>/<noff> for separate w-,e-,s-,n-margins.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +n to leave the BoundingBox as is.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +p[<pen>] to outline the BoundingBox [%s].\n",
	             gmt_putpen (API->GMT, &API->GMT->current.setting.map_default_pen));
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +r to force rounding of HighRes BoundingBox instead of ceil.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +s[m]<width>[/<height>] option the select a new image size\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     but maintaining the DPI set by -E (Ghostscript does the re-interpolation work).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Use +sm to only change size if figure size exceeds the new maximum size(s).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     Append measurement unit u (%s) [%c].\n",
	             GMT_DIM_UNITS_DISPLAY, API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit][0]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively, use -A+S<scale> to scale the image by the <scale> factor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append +u to strip out time-stamps (produced by GMT -U options).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Specify a single, custom option that will be passed on to Ghostscript\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   as is. Repeat to add several options [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set an alternative output directory (which must exist)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is same directory as PS files].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -D. to place the output in the current directory.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set raster resolution in dpi [default = 720 for images in a PDF, 300 for other formats].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Force the output file name. By default output names are constructed\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   using the input names as base, which are appended with an appropriate\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   extension. Use this option to provide a different name, but WITHOUT\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   extension. Extension is still determined and appended automatically.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Full path to your Ghostscript executable.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   NOTE: Under Unix systems this is generally not necessary.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Under Windows, Ghostscript path is fished from the registry.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If this fails you can still add the GS path to system's path\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or give the full path here.\n");
#ifdef WIN32
	GMT_Message (API, GMT_TIME_NONE, "\t   (e.g., -Gc:\\programs\\gs\\gs9.27\\bin\\gswin64c).\n");
#else
	GMT_Message (API, GMT_TIME_NONE, "\t   (e.g., -G/some/unusual/dir/bin/gs).\n");
#endif
	GMT_Message (API, GMT_TIME_NONE, "\t-H Temporarily increase dpi by <factor>, rasterize, then downsample [no downsampling].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Used to improve raster image quality, especially for lower raster resolutions.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Ghostscript versions >= 9.00 change gray-shades by using ICC profiles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   GS 9.05 and above provide the '-dUseFastColor=true' option to prevent that\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and that is what psconvert does by default, unless option -I is set.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note that for GS >= 9.00 and < 9.05 the gray-shade shifting is applied\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to all but PDF format. We have no solution to offer other than ... upgrade GS\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L The <listfile> is an ASCII file with names of files to be converted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-M Sandwich current psfile between background and foreground plots:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Mb Append the name of a background PostScript plot [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -Mf Append name of foreground PostScript plot [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-P Force Portrait mode. All Landscape mode plots will be rotated back\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   so that they show unrotated in Portrait mode.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This is practical when converting to image formats or preparing\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   EPS or PDF plots for inclusion in documents.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Anti-aliasing setting for (g)raphics or (t)ext; append size (1,2,4) of sub-sampling box.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For PDF and EPS output, default is no anti-aliasing, which is the same as specifying size 1.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For raster formats the defaults are -Qg4 -Qt4 unless overridden explicitly.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, use -Qp to create a GeoPDF (requires -Tf).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Apart from executing it, also writes the Ghostscript command to\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   standard error and keeps all intermediate files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set output format [default is jpeg]:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   b means BMP.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   e means EPS.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   E means EPS with setpagedevice command.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f means PDF.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   F means multi-page PDF (requires -F).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g means PNG.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   G means PNG (transparent where nothing is plotted).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   j means JPEG.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m means PPM.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s means SVG [if supported by your Ghostscript version].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t means TIF.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For b, g, j, and t, append +m to get a monochrome (grayscale) image [color].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The EPS format can be combined with any of the other formats.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For example, -Tef creates both an EPS and PDF file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-V Provide progress report [default is silent] and show the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   gdal_translate command, in case you want to use this program\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to create a geoTIFF file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Write a ESRI type world file suitable to make (e.g.,) .tif files\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   recognized as geotiff by software that know how to do it. Be aware,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   however, that different results are obtained depending on the image\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   contents and if the -B option has been used or not. The trouble with\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -B is that it creates a frame and very likely its ticks and annotations\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   introduces pixels outside the map data extent. As a consequence,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the map extent estimation will be wrong. To avoid this problem, use\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the --MAP_FRAME_TYPE=inside option which plots all annotation-related\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   items inside the image and therefore does not compromise the coordinate\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   computations. The world file naming follows the convention of jamming\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   a 'w' in the file extension. So, if the output is tif (-Tt) the world\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   file is a .tfw, for jpeg a .jgw, and so on.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -W+g to do a system call to gdal_translate and produce a true\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   geoTIFF image right away. The output file will have the extension\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   .tiff. See the man page for other 'gotchas'. Automatically sets -A -P.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -W+k to create a minimalist KML file that allows loading the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   image in Google Earth. Note that for this option the image must be\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   in geographical coordinates. If not, a warning is issued but the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   KML file is created anyway.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Several modifiers allow you to specify the content in the KML file:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +a<altmode>[<altitude>] sets the altitude mode of this layer, where\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      <altmode> is one of 5 recognized by Google Earth:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      G clamped to the ground [Default].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      g Append altitude (in m) relative to ground.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      A Append absolute altitude (in m).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      s Append altitude (in m) relative to seafloor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t      S clamped to the seafloor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +f<minfade>/<maxfade>] sets distances over which we fade from opaque\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     to transparent [no fading].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +l<minLOD>/<maxLOD>] sets Level Of Detail when layer should be\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     active [always active]. Image goes inactive when there are fewer\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     than minLOD pixels or more than maxLOD pixels visible.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -1 means never invisible.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +n<layername> sets the name of this particular layer\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     [\"GMT Image Overlay\"].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +o<foldername> sets the name of this particular folder\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     [\"GMT Image Folder\"].  This yields a KML snipped without header/trailer.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +t<doctitle> sets the document name [\"GMT KML Document\"].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +u<URL> prepands this URL to the name of the image referenced in the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     KML [local file].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Escape any +? modifier inside strings with \\.\n");
	if (API->GMT->current.setting.run_mode == GMT_CLASSIC)
		GMT_Message (API, GMT_TIME_NONE, "\t-Z Remove input PostScript file(s) after successful conversion.\n");
	GMT_Option (API, ".");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct PS2RASTER_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to psconvert and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, mode;
	int j = 0;
	bool grayscale = false, halfbaked = false;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files [Allow for file "=" under API calls] */
				if (strstr (opt->arg, ".ps-")) halfbaked = true;
				if (!(GMT->parent->external && !strncmp (opt->arg, "=", 1))) {	/* Can check if file is sane */
					if (!halfbaked && !gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_DATASET)) n_errors++;
				}
				Ctrl->In.n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Adjust BoundingBox: -A[u][<margins>][+n][+r][+s<width>[/<height>]] */
				n_errors += parse_A_settings (GMT, opt->arg, Ctrl);
				break;
			case 'C':	/* Append extra custom GS options */
				strcat (Ctrl->C.arg, " ");
				strncat (Ctrl->C.arg, opt->arg, GMT_LEN256-1);	/* Append to list of extra GS options */
				break;
			case 'D':	/* Change output directory */
				if ((Ctrl->D.active = gmt_check_filearg (GMT, 'D', opt->arg, GMT_OUT, GMT_IS_DATASET)) != 0) {
					gmt_M_str_free (Ctrl->D.dir);
					Ctrl->D.dir = strdup (opt->arg);
				}
				else
					n_errors++;
				break;
			case 'E':	/* Set output dpi */
				Ctrl->E.active = true;
				Ctrl->E.dpi = atof (opt->arg);
				break;
			case 'F':	/* Set explicitly the output file name */
				if ((Ctrl->F.active = gmt_check_filearg (GMT, 'F', opt->arg, GMT_OUT, GMT_IS_DATASET)) != 0) {
					Ctrl->F.file = gmt_strdup_noquote (opt->arg);
					gmt_filename_get (Ctrl->F.file);
				}
				else
					n_errors++;
				break;
			case 'G':	/* Set GS path */
				if ((Ctrl->G.active = gmt_check_filearg (GMT, 'G', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0) {
					gmt_M_str_free (Ctrl->G.file);
					Ctrl->G.file = malloc (strlen (opt->arg)+3);	/* Add space for quotes */
					sprintf (Ctrl->G.file, "%c%s%c", quote, opt->arg, quote);
				}
				else
					n_errors++;
				break;
			case 'H':	/* RIP at a higher dpi, then downsample in gs */
				Ctrl->H.active = true;
				Ctrl->H.factor = atoi (opt->arg);
				break;
			case 'I':	/* Do not use the ICC profile when converting gray shades */
				Ctrl->I.active = true;
				break;
			case 'L':	/* Give list of files to convert */
				if ((Ctrl->L.active = gmt_check_filearg (GMT, 'L', opt->arg, GMT_IN, GMT_IS_DATASET)) != 0)
					Ctrl->L.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'M':	/* Manage background and foreground layers for PostScript sandwich */
				switch (opt->arg[0]) {
					case 'b':	j = 0;	break;	/* background */
					case 'f':	j = 1;	break;	/* foreground */
					default:	/* Bad argument */
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -M: Select -Mb or -Sf\n");
						n_errors++;
						break;
				}
				if (!n_errors && !Ctrl->M[j].active) {	/* Got -Mb<file> or -Mf<file> */
					if (!gmt_access (GMT, &opt->arg[1], R_OK)) {	/* The plot file exists */
						Ctrl->M[j].active = true;
						Ctrl->M[j].file = strdup (&opt->arg[1]);
					}
					else {
						GMT_Report (GMT->parent, GMT_MSG_ERROR, "Option -M%c: Cannot read file %s\n", opt->arg[0], &opt->arg[1]);
						n_errors++;
					}
				}
				break;
			case 'P':	/* Force Portrait mode */
				Ctrl->P.active = true;
				break;
			case 'Q':	/* Anti-aliasing settings */
				Ctrl->Q.active = true;
				if (opt->arg[0] == 'g')
					mode = PSC_LINES;
				else if (opt->arg[0] == 't')
					mode = PSC_TEXT;
				else if (opt->arg[0] == 'p')
					mode = PSC_GEO;
				else {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "The -Q option requires setting -Qg, -Qp, or -Qt!\n");
					n_errors++;
					continue;

				}
				Ctrl->Q.on[mode] = true;
				if (mode < PSC_GEO) Ctrl->Q.bits[mode] = (opt->arg[1]) ? atoi (&opt->arg[1]) : 4;
				break;
			case 'S':	/* Write the GS command to STDERR */
				Ctrl->S.active = true;
				break;
			case 'T':	/* Select output format (optionally also request EPS) */
				Ctrl->T.active = true;
				if ((j = (int)strlen(opt->arg)) > 1 && opt->arg[j-1] == '-')	/* Old deprecated way of appending a single - sign at end */
					grayscale = true;
				else if (strstr (opt->arg, "+m"))	/* Modern monochrome option (like -M in grdimage) */
					grayscale = true;
				for (j = 0; opt->arg[j]; j++) {
					switch (opt->arg[j]) {
						case 'e':	/* EPS */
							Ctrl->T.eps = 1;
							break;
						case 'E':	/* EPS with setpagedevice */
							Ctrl->T.eps = -1;
							break;
						case 'f':	/* PDF */
							Ctrl->T.device = GS_DEV_PDF;
							break;
						case 'F':	/* PDF (multipages) */
							Ctrl->T.device = -GS_DEV_PDF;
							break;
						case 'b':	/* BMP */
							Ctrl->T.device = (grayscale) ? GS_DEV_BMPG : GS_DEV_BMP;
							break;
						case 'j':	/* JPEG */
							Ctrl->T.device = (grayscale) ? GS_DEV_JPGG : GS_DEV_JPG;
							break;
						case 'g':	/* PNG */
							Ctrl->T.device = (grayscale) ? GS_DEV_PNGG : GS_DEV_PNG;
							break;
						case 'G':	/* PNG (transparent) */
							Ctrl->T.device = GS_DEV_TPNG;
							break;
						case 'm':	/* PPM */
							Ctrl->T.device = GS_DEV_PPM;
							break;
						case 'p':	/* PS */
							Ctrl->T.ps = 1;
							break;
						case 's':	/* SVG */
							Ctrl->T.device = GS_DEV_SVG;
							break;
						case 't':	/* TIFF */
							Ctrl->T.device = (grayscale) ? GS_DEV_TIFG : GS_DEV_TIF;
							break;
						case '-':	/* Just skip the trailing - for grayscale since it is handled separately above */
							break;
						case '+':	/* Just skip the trailing +m for grayscale since it is handled separately above */
							j++;
							break;
						default:
							gmt_default_error (GMT, opt->option);
							n_errors++;
							break;
					}
				}
				break;
			case 'W':	/* Save world file */
				n_errors += parse_GE_settings (GMT, opt->arg, Ctrl);
				break;

			case 'Z':
				if (GMT->current.setting.run_mode == GMT_CLASSIC)
					Ctrl->Z.active = true;
				else {
					GMT_Report (GMT->parent, GMT_MSG_ERROR, "The -Z option is not available in MODERN mode!\n");
					n_errors++;
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}

	if (!Ctrl->T.active) Ctrl->T.device = GS_DEV_JPG;	/* Default output device if none is specified */

	if (Ctrl->T.device > GS_DEV_SVG) {	/* Raster output, apply default -Q for rasters unless already specified */
		/* For rasters, we should always add -Qt4 unless -Q was set manually.  This will improve the rasterization of text */
		if (!Ctrl->Q.on[PSC_TEXT]) {	/* Only override if not set */
			Ctrl->Q.on[PSC_TEXT] = true;
			Ctrl->Q.bits[PSC_TEXT] = 4;
		}
		/* "ghostlines" seen in pscoast plots are usually a feature of a PDF or PS viewer and may be changed via their view settings.
		 * We have found that the -TG device creates ghostlines in the rasters and that these are suppressed by -Qg2 so we enfore that here for all rasters */
		if (!Ctrl->Q.on[PSC_LINES]) {	/* Transparent PNG needs this antialiasing option as well, at least in gs 9.22 */
			Ctrl->Q.on[PSC_LINES] = true;
			Ctrl->Q.bits[PSC_LINES] = 2;
		}
	}

	for (j = 0; j < 2; j++) {
		if (Ctrl->M[j].active) {
			static char *layer[2] = {"Back", "Fore"};
			if (Ctrl->M[j].file && access (Ctrl->M[j].file, F_OK)) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "-Mb: %sground file %s cannot be found!\n", layer[j], Ctrl->M[j].file);
				n_errors++;
			}
		}
	}

	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.on[PSC_GEO] && Ctrl->T.device != GS_DEV_PDF,
	                                   "Creating GeoPDF format requires -Tf\n");

	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.on[PSC_LINES] && (Ctrl->Q.bits[PSC_LINES] < 1 || Ctrl->Q.bits[PSC_LINES] > 4),
	                                   "Anti-aliasing for graphics requires sub-sampling box of 1,2, or 4\n");

	n_errors += gmt_M_check_condition (GMT, Ctrl->Q.on[PSC_TEXT] && (Ctrl->Q.bits[PSC_TEXT] < 1 || Ctrl->Q.bits[PSC_TEXT] > 4),
	                                   "Anti-aliasing for text requires sub-sampling box of 1,2, or 4\n");

	n_errors += gmt_M_check_condition (GMT, Ctrl->In.n_files > 1 && Ctrl->L.active,
	                                   "Cannot handle both a file list and multiple ps files in input\n");

	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && access (Ctrl->L.file, R_OK),
	                                   "Option -L: Cannot read list file %s\n", Ctrl->L.file);

	n_errors += gmt_M_check_condition (GMT, Ctrl->T.device == -GS_DEV_PDF && !Ctrl->F.active,
	                                   "Creation of Multipage PDF requires setting -F option\n");

	n_errors += gmt_M_check_condition (GMT, Ctrl->L.active && (GMT->current.setting.run_mode == GMT_MODERN),
	                                   "Option -L Cannot use -L for list file under modern GMT mode\n");

	n_errors += gmt_M_check_condition (GMT, GMT->current.setting.run_mode == GMT_MODERN && !(Ctrl->In.n_files == 0 || (Ctrl->In.n_files == 1 && halfbaked)),
	                                   "No listed input files allowed under modern GMT mode\n");

	n_errors += gmt_M_check_condition (GMT, !Ctrl->F.active && GMT->current.setting.run_mode == GMT_MODERN,
	                                   "Modern GMT mode requires the -F option\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int64_t file_line_reader (struct GMT_CTRL *GMT, char **L, size_t *size, FILE *fp, char *notused1, uint64_t *notused2) {
	/* file_line_reader gets the next logical record from filepointer fp and passes it back via L.
	 * We use this function since PS files may be shitty and contain a mixture of \r or \n
	 * to indicate end of a logical record.
	 * all_PS and pos are not used.
	 * Empty lines are suppressed.
	 * If the last end-of-line is missing, the last line is still produced with an end-of-line.
	 */
	int c;
	int64_t out = 0;
	char *line = *L;
	if (notused1 == NULL){};			/* Just to shut up a compiler warning of "unreferenced formal parameter" */
	if (notused2 == NULL){};			/* 		""		*/
	while ((c = fgetc (fp)) > 0) {	/* Keep reading until End-Of-File */
		if (c == '\r' || c == '\n') {	/* Got logical end of record */
			if (!out) continue; /* Nothing in buffer ... skip */
			line[out] = '\0';	/* Terminate output string */
			return out;	/* Return number of characters in L */
		}
		/* Got a valid character in current record */
		if ((size_t)out == (*size-1)) {	/* Need to extend our buffer; the -1 makes room for an \0 as needed to end a string */
			(*size) <<= 1;	/* Double the current buffer space */
			line = *L = gmt_M_memory (GMT, *L, *size, char);
		}
		line[out++] = (char)c;	/* Add this char to our buffer */
	}
	if (!out) return EOF;	/* Nothing in buffer ... exit with EOF */
	line[out] = '\0';	/* Terminate output string */
	return out;			/* Return number of characters in L. The next call to the routine will return EOF. */
}

void file_rewind (FILE *fp, uint64_t *notused) {	/* Rewinds to start of file */
	if (notused == NULL){};			/* Just to shut up a compiler warning of "unreferenced formal parameter" */
	rewind (fp);
}

#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LOCAL inline char *alpha_bits (struct PS2RASTER_CTRL *Ctrl) {
	/* return alpha bits which are valid for the selected driver */
	static char alpha[48];
	char *c = alpha;
	unsigned int bits;
	*alpha = '\0'; /* reset string */
	if (Ctrl->Q.on[PSC_LINES] && Ctrl->Q.bits[PSC_LINES]) {
		if (Ctrl->T.device == GS_DEV_PDF || Ctrl->T.device == -GS_DEV_PDF)
			/* Note: cannot set GraphicsAlphaBits > 1 with a vector device */
			bits = 1;
		else
			bits = Ctrl->Q.bits[PSC_LINES];
		sprintf (c, " -dGraphicsAlphaBits=%d", bits);
		c = strrchr(c, '\0'); /* advance to end of string */
	}
	if (Ctrl->Q.on[PSC_TEXT] && Ctrl->Q.bits[PSC_TEXT]) {
		if (Ctrl->T.device == GS_DEV_PDF || Ctrl->T.device == -GS_DEV_PDF)
			/* Note: cannot set GraphicsAlphaBits > 1 with a vector device */
			bits = 1;
		else
			bits = Ctrl->Q.bits[PSC_TEXT];
		sprintf (c, " -dTextAlphaBits=%d", bits);
	}
	return alpha;
}

GMT_LOCAL void possibly_fill_or_outline_BoundingBox (struct GMT_CTRL *GMT, struct PS2R_A *A, FILE *fp) {
	/* Check if user wanted to paint or outline the BoundingBox - otherwise do nothing */
	char *ptr = NULL;
	GMT->PSL->internal.dpp = PSL_DOTS_PER_INCH / 72.0;	/* Dots pr. point resolution of output device, set here since no PSL initialization */
	if (A->paint) {	/* Paint the background of the page */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Paint background BoundingBox using paint %s\n", gmt_putrgb (GMT, A->fill.rgb));
		if (GMT->PSL->internal.comments) fprintf (fp, "%% Paint background BoundingBox using paint %s\n", gmt_putrgb (GMT, A->fill.rgb));
		ptr = PSL_makecolor (GMT->PSL, A->fill.rgb);
		fprintf (fp, "gsave clippath %s F N U\n", ptr);
	}
	if (A->outline) {	/* Draw the outline of the page */
		GMT_Report (GMT->parent, GMT_MSG_INFORMATION, "Outline background BoundingBox using pen %s\n", gmt_putpen (GMT, &A->pen));
		if (GMT->PSL->internal.comments) fprintf (fp, "%% Outline background BoundingBox using pen %s\n", gmt_putpen (GMT, &A->pen));
		/* Double pen thickness since half will be clipped by BoundingBox */
		ptr = PSL_makepen (GMT->PSL, 2.0*A->pen.width, A->pen.rgb, A->pen.style, A->pen.offset);
		fprintf (fp, "gsave %s clippath S U\n", ptr);
	}
}

/* ---------------------------------------------------------------------------------------------- */
GMT_LOCAL int pipe_HR_BB(struct GMTAPI_CTRL *API, struct PS2RASTER_CTRL *Ctrl, char *gs_BB, double margin, double *w, double *h) {
	/* Do what we do in the main code for the -A (if used here) option but on a in-memory PS 'file' */
	char      cmd[GMT_LEN512] = {""}, buf[GMT_LEN128] = {""}, t[32] = {""}, *pch, c;
	int       fh, r, c_begin = 0;
	size_t    n;
	bool      landscape = false;
	double    x0, y0, x1, y1, xt = 0, yt = 0;
	struct GMT_POSTSCRIPT *PS = NULL;
#ifdef _WIN32
	int       fd[2] = { 0, 0 };
	FILE     *fp = NULL;
#else
	struct popen2 *H = NULL;
#endif

	sprintf (cmd, "%s %s %s -", Ctrl->G.file, gs_BB, Ctrl->C.arg);	/* Set up gs command */

#ifdef _WIN32
	if (_pipe(fd, 512, O_BINARY) == -1) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to open the pipe.\n");
		return GMT_RUNTIME_ERROR;
	}
	if (dup2 (fd[1], fileno (stderr)) < 0) {
		GMT_Report (API, GMT_MSG_ERROR, "Failed to duplicate pipe.\n");
		return GMT_RUNTIME_ERROR;
	}
	if (close (fd[1]) == -1) { 		/* Close original write end of pipe */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to close write end of pipe.\n");
		return GMT_RUNTIME_ERROR;
	}
	if ((fp = popen (cmd, "w")) == NULL) {	/* Failed popen-job, exit */
		GMT_Report(API, GMT_MSG_ERROR, "Cannot execute Ghostscript command.\n");
		return GMT_RUNTIME_ERROR;
	}
#else
	if ((H = gmt_popen2 (cmd)) == NULL) {	/* Failed popen-job, exit */
		GMT_Report(API, GMT_MSG_ERROR, "Cannot execute Ghostscript command.\n");
		return GMT_RUNTIME_ERROR;
	}
#endif

	/* Allocate GMT_POSTSCRIPT struct to hold the string that lives inside GMT->PSL */
	PS = gmt_M_memory (API->GMT, NULL, 1, struct GMT_POSTSCRIPT);
	PS->data = PSL_getplot (API->GMT->PSL);		/* Get pointer to the internal plot buffer */
	PS->n_bytes = API->GMT->PSL->internal.n;	/* Length of plot buffer; note P->n_alloc = 0 since nothing was allocated here */

	/* Send the PS down Ghostscript's throat */
#ifdef _WIN32
	fwrite (PS->data, sizeof(char), PS->n_bytes, fp);
	fflush (fp);
	if (pclose (fp) == -1)
		GMT_Report(API, GMT_MSG_ERROR, "Failure while closing pipe used for Ghostscript command.\n");
	fh = fd[0];	/* File handle for reading */
#else
 	write (H->fd[1], PS->data, PS->n_bytes);
	/* Now closed for writing */
	gmt_pclose2 (&H, 1);
	fh = H->fd[0];	/* File handle for reading */
#endif

	/* Now read the image from input pipe fd[0] */
	while (read(fh, t, 1U) && t[0] != '\n'); 	/* Consume first line that has the BoundingBox */
	n = 0;
	while (read(fh, t, 1U) && t[0] != '\n')		/* Read second line which has the HiResBoundingBox */
		buf[n++] = t[0];
#ifdef _WIN32
	if (close (fd[0]) == -1) { 		/* Close read end of pipe */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to close read end of pipe.\n");
		gmt_M_free (API->GMT, PS);
		return GMT_RUNTIME_ERROR;
	}
#else
	/* Now closed for reading */
 	gmt_pclose2 (&H, 0);
#endif

	sscanf (buf, "%s %lf %lf %lf %lf", t, &x0, &y0, &x1, &y1);
	c = PS->data[500];
	PS->data[500] = '\0';			/* Temporary cut the string to not search the whole file */
	pch = strstr(PS->data, "Landscape");
	if (pch != NULL) landscape = true;
	PS->data[500] = c;				/* Restore the deleted character */

	if (Ctrl->A.crop) {
		if (landscape)					/* We will need these in pipe_ghost() */
			xt = -x1, yt = -y0, *w = y1-y0, *h = x1-x0, r = -90;
		else
			xt = -x0, yt = -y0, *w = x1-x0, *h = y1-y0, r = 0;

		if (margin == 0) margin = Ctrl->A.margin[0];		/* We may have margins set via -A*/
	}
	else {		/* Get the page size from original PS headers */
		if ((pch = strstr(PS->data, "%%HiResB")) == NULL) {
			x0 = y0 = 0;	x1 = y1 = 10;
			*w = x1, *h = y1, r = 0;
			GMT_Report (API, GMT_MSG_ERROR, "Something bad, the GMT PS does not have a %%HiResBoundingBox! Expect the worst.\n");
		}
		else {
			sscanf (&pch[20], "%lf %lf %lf %lf", &x0, &y0, &x1, &y1);
			if (landscape)
				*w = y1, *h = x1, r = -90;
			else
				*w = x1, *h = y1, r = 0;
		}
	}

	/* Add a margin if user requested it (otherwise margin = 0) */
	*w += 2 * margin;       *h += 2 * margin;
	xt += margin;           yt += margin;

	sprintf (buf, "BoundingBox: 0 0 %.0f %.0f", ceil(*w), ceil(*h));
	if ((pch = strstr (PS->data, "BoundingBox")) != NULL) {		/* Find where is the BB */
		for (n = 0; n < strlen(buf); n++)                       /* and update it */
			pch[n] = buf[n];
		while (pch[n] != '\n') pch[n++] = ' ';                  /* Make sure that there are only spaces till EOL */
	}
	else
		GMT_Report (API, GMT_MSG_ERROR, "Something very odd the GMT PS does not have a %%BoundingBox\n");

	sprintf (buf, "HiResBoundingBox: 0 0 %.4f %.4f", *w, *h);
	if ((pch = strstr(PS->data, "HiResBoundingBox")) != NULL) {	/* Find where is the HiResBB */
		for (n = 0; n < strlen(buf); n++)                       /* and update it */
			pch[n] = buf[n];
	}
	else
		GMT_Report (API, GMT_MSG_WARNING, "Something very odd the GMT PS does not have a %%HiResBoundingBox\n");

	/* Find where is the setpagedevice line */
	if ((pch = strstr(PS->data, "setpagedevice")) != NULL) {
		while (pch[c_begin] != '\n') c_begin--;
		c_begin++;	/* It receded one too much */
		/* So now we know where the line starts. Put a 'translate' command in its place. */
		(r == 0) ? sprintf(buf, "%.3f %.3f translate", xt, yt) : sprintf(buf, "%d rotate\n%.3f %.3f translate", r, xt, yt);
		for (n = 0; n < strlen(buf); n++, c_begin++) pch[c_begin] = buf[n];
		while (pch[c_begin] != '\n')  pch[c_begin++] = ' ';     /* Make sure there are only spaces till EOL */
	}
	else {
		if ((pch = strstr(PS->data, " translate")) != NULL) {		/* If user runs through this function twice 'setpagedevice' was changed to 'translate' */
			double old_xt, old_yt;
			while (pch[c_begin] != '\n') c_begin--;
			c_begin++;	/* Goto line start but it receded one too much */
			sscanf (&pch[c_begin], "%lf %lf", &old_xt, &old_yt);
			(r == 0) ? sprintf(buf, "%.3f %.3f translate", xt + old_xt, yt + old_xt) : sprintf(buf, "%d rotate\n%.3f %.3f translate", r, xt + old_xt, yt + old_xt);
			for (n = 0; n < strlen(buf); n++, c_begin++) pch[c_begin] = buf[n];
			while (pch[c_begin] != '\n') pch[c_begin++] = ' ';
		}
		else
			GMT_Report(API, GMT_MSG_WARNING, "Something very odd the GMT PS does not have the setpagedevice line\n");
	}

	gmt_M_free (API->GMT, PS);

	return GMT_NOERROR;
}
/* ---------------------------------------------------------------------------------------------- */

GMT_LOCAL int pipe_ghost (struct GMTAPI_CTRL *API, struct PS2RASTER_CTRL *Ctrl, char *gs_params, double w, double h, char *out_file) {
	/* Run the command that converts the PostScript into a raster format, but using a in-memory PS as source.
	   For that we use the popen function to run the GS command. The biggest problem, however, is that popen only
	   access one pipe and we need two. One for the PS input and the other for the raster output. There are popen
	   versions (popen2) that have that capacity, but not the one on Windows so we resort to a trick using both
	   popen() and pipe().

	   w,h are the raster Width and Height calculated by a previous call to pipe_HR_BB().
	   out_file is a string with two different meanings:
	      1. If it's empty than we save the raster in a GMT_IMAGE structure
	      2. If it holds a file name plus the settings for that driver, than we save the result in a file.
	*/
	char      cmd[1024] = {""}, buf[GMT_LEN128], t[16] = {""};
	int       fd[2] = {0, 0}, fh, n, k, pix_w, pix_h, junk_n;
	uint64_t  dim[4], nXY, row, col, band, nCols, nRows, nBands;
	unsigned char *tmp;
	unsigned int nopad[4] = {0, 0, 0, 0};
	struct GMT_IMAGE *I = NULL;
	struct GMT_POSTSCRIPT *PS = NULL;
#ifdef _WIN32
	uint64_t  n_bytes;
#else
	struct popen2 *H = NULL;
#endif
	FILE     *fp = NULL;

	/* sprintf(cmd, "gswin64c -q -r300x300 -sDEVICE=ppmraw -sOutputFile=- -"); */
	pix_w = urint (smart_ceil (w * Ctrl->E.dpi / 72.0));
	pix_h = urint (smart_ceil (h * Ctrl->E.dpi / 72.0));
	sprintf (cmd, "%s -r%g -g%dx%d ", Ctrl->G.file, Ctrl->E.dpi, pix_w, pix_h);
	if (strlen(gs_params) < 450) strcat (cmd, gs_params);	/* We know it is but Coverity doesn't, and complains */

	if (out_file[0] == '\0') {		/* Need to open a pipe to capture the popen output, which will contain the raster */
		strcat (cmd, " -sDEVICE=ppmraw -sOutputFile=- -");
#ifdef _WIN32
		if (_pipe(fd, 145227600, O_BINARY) == -1) {
			GMT_Report (API, GMT_MSG_ERROR, "Failed to open the pipe.\n");
			return GMT_RUNTIME_ERROR;
		}
		if (dup2 (fd[1], fileno (stdout)) < 0) {
			GMT_Report (API, GMT_MSG_ERROR, "Failed to duplicate pipe.\n");
			return GMT_RUNTIME_ERROR;
		}
		if (close (fd[1]) == -1) { 		/* Close original write end of pipe */
			GMT_Report (API, GMT_MSG_ERROR, "Failed to close write end of pipe.\n");
			return GMT_RUNTIME_ERROR;
		}
		if ((fp = popen (cmd, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot execute Ghostscript command.\n");
			gmt_M_free (API->GMT, PS);
			return GMT_RUNTIME_ERROR;
		}
#else
		if ((H = gmt_popen2 (cmd)) == NULL) {	/* Failed popen-job, exit */
			GMT_Report(API, GMT_MSG_ERROR, "Cannot execute Ghostscript command.\n");
			return GMT_RUNTIME_ERROR;
		}
#endif
	}
	else {	/* Only need a unidirectional pipe as supported by all since gs output is written to a file */
		strncat (cmd, out_file, 1023);
		if ((fp = popen (cmd, "w")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot execute Ghostscript command.\n");
			return GMT_RUNTIME_ERROR;
		}
	}

	PS = gmt_M_memory (API->GMT, NULL, 1, struct GMT_POSTSCRIPT);
	PS->data = PSL_getplot (API->GMT->PSL);		/* Get pointer to the plot buffer */
	PS->n_bytes = API->GMT->PSL->internal.n;	/* Length of plot buffer; note P->n_alloc = 0 since nothing was allocated here */

	/* Send the PS down Ghostscript's throat */
#ifdef _WIN32
	if ((n_bytes = fwrite (PS->data, sizeof(char), PS->n_bytes, fp)) != PS->n_bytes)
		GMT_Report (API, GMT_MSG_ERROR,
		            "Failure while writing PostScript buffer to Ghostscript process. Bytes written = %ld, should have been %ld\n", n_bytes, PS->n_bytes);
	if (fflush (fp) == EOF)
		GMT_Report (API, GMT_MSG_ERROR, "Failure while flushing Ghostscript process.\n");
	if (pclose (fp) == -1)
		GMT_Report (API, GMT_MSG_ERROR, "Failure while closing Ghostscript process.\n");
	fh = fd[0];	/* File handle for reading */
#else
	if (fp) {	/* Did popen after all since we have an output filename */
		if (fwrite (PS->data, sizeof(char), PS->n_bytes, fp) != PS->n_bytes)
			GMT_Report (API, GMT_MSG_ERROR, "Failure while writing PostScript buffer to Ghostscript process.\n");
		if (fflush (fp) == EOF)
			GMT_Report (API, GMT_MSG_ERROR, "Failure while flushing Ghostscript process.\n");
		if (pclose (fp) == -1)
			GMT_Report (API, GMT_MSG_ERROR, "Failure while closing Ghostscript process.\n");
	}
	else {	/* On non-Windows and want a raster back */
 		write (H->fd[1], PS->data, PS->n_bytes);
		/* Now closed for writing */
		gmt_pclose2 (&H, 1);
		fh = H->fd[0];	/* File handle for reading */
	}
#endif

	/* And now restore the original BB & HiResBB so that the PS data can be reused if wanted */
	gmt_M_free (API->GMT, PS);

	/* ----------- IF WE WROTE A FILE THEN WE ARE DONE AND WILL RETURN RIGHT NOW -------------- */
	if (out_file[0] != '\0')
		return GMT_NOERROR;
	/* ---------------------------------------------------------------------------------------- */

	if ((n = read (fh, buf, 3U)) != 3)				/* Consume first header line */
		GMT_Report (API, GMT_MSG_ERROR, "pipe_ghost: failed read first line in popen store. Expect failures.\n");
	while (read (fh, buf, 1U) && buf[0] != '\n'); 	/* OK, by the end of this we are at the end of second header line */
	n = 0;
	while (read(fh, buf, 1U) && buf[0] != ' ') 		/* Get string with number of columns from 3rd header line */
		t[n++] = buf[0];
	dim[GMT_X] = atoi (t);
	n = 0;
	while (read(fh, buf, 1U) && buf[0] != '\n') 		/* Get string with number of rows from 3rd header line */
		t[n++] = buf[0];
	t[n] = '\0';						/* Make sure no character is left from previous usage */

	while (read(fh, buf, 1U) && buf[0] != '\n');		/* Consume fourth header line */

	dim[GMT_Y] = atoi (t);
	dim[GMT_Z] = 3;	/* This might change if we do monochrome at some point */
	GMT_Report (API, GMT_MSG_INFORMATION, "Image dimensions %d\t%d\n", dim[GMT_X], dim[GMT_Y]);

	if ((I = GMT_Create_Data (API, GMT_IS_IMAGE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, dim, NULL, NULL, 1, 0, NULL)) == NULL) {
		GMT_Report (API, GMT_MSG_ERROR, "Could not create Image structure\n");
		return GMT_RUNTIME_ERROR;
	}
	nCols = dim[GMT_X];		nRows = dim[GMT_Y];		nBands = dim[2];	nXY = nRows * nCols;
	tmp   = gmt_M_memory(API->GMT, NULL, nCols * nBands, char);
	if (!strncmp (I->header->mem_layout, "TCP", 3)) {		/* Images.jl in Julia wants this */
		for (row = 0; row < nRows; row++) {
			if ((k = read (fd[0], tmp, (unsigned int)(nCols * nBands))) == 0) {	/* Read a row of nCols by nBands bytes of data */
				GMT_Report (API, GMT_MSG_ERROR, "Could not read row from pipe into Image structure\n");
				gmt_M_free (API->GMT, tmp);
				return GMT_RUNTIME_ERROR;
			}
			for (col = n = 0; col < nCols; col++)
				for (band = 0; band < nBands; band++)
					I->data[row*nBands + col*nBands*nRows + band] = tmp[n++];
		}
	}
	else if (!strncmp (I->header->mem_layout, "TRP", 3)) {	/* Very cheap this one since is gs native order. */
		junk_n = read (fd[0], I->data, (unsigned int)(nCols * nRows * nBands));		/* ... but may overflow */
	}
	else {	/* For MEX, probably */
		for (row = 0; row < nRows; row++) {
			junk_n = read (fd[0], tmp, (unsigned int)(nCols * nBands));	/* Read a row of nCols by nBands bytes of data */
			for (col = n = 0; col < nCols; col++)
				for (band = 0; band < nBands; band++)
					I->data[row + col*nRows + band*nXY] = tmp[n++];	/* Band interleaved, the best for MEX. */
		}
	}
	gmt_M_free (API->GMT, tmp);

#ifdef _WIN32
	if (close (fh) == -1)	/* Close read end of pipe */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to close read end of pipe.\n");
#else
	/* Now closed for reading */
 	gmt_pclose2 (&H, 0);
#endif

	I->type = GMT_CHAR;
	I->header->n_columns = (uint32_t)dim[GMT_X];	I->header->n_rows = (uint32_t)dim[GMT_Y];	I->header->n_bands = (uint32_t)dim[GMT_Z];
	I->header->registration = GMT_GRID_PIXEL_REG;
	gmt_M_grd_setpad (API->GMT, I->header, nopad);          /* Copy the no pad to the header */
	if (GMT_Write_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->F.file, I) != GMT_NOERROR)
		return GMT_RUNTIME_ERROR;

	return GMT_NOERROR;	/* Done here */
}

/* ---------------------------------------------------------------------------------------------- */
GMT_LOCAL int in_mem_PS_convert(struct GMTAPI_CTRL *API, struct PS2RASTER_CTRL *Ctrl, char **ps_names, char *gs_BB, char *gs_params, char *device[], char *device_options[], char *ext[]) {
	char out_file[PATH_MAX] = {""};
	int    error = 0;
	double margin = 0, w = 0, h = 0;	/* Width and height in pixels of the final raster cropped of the outer white spaces */

	if (ps_names[0][1])		/* See if we have a margin request */
		margin = gmt_M_to_points (API->GMT, &ps_names[0][1]);

	if (API->GMT->PSL->internal.pmode != 3) {
		GMT_Report (API, GMT_MSG_ERROR, "Internal PSL PostScript is only half-baked [mode = %d]\n",
		            API->GMT->PSL->internal.pmode);
		error++;
	}
	if (error) 			/* Return in error state */
		return error;

	if (pipe_HR_BB (API, Ctrl, gs_BB, margin, &w, &h))		/* Apply the -A stuff to the in-memory PS */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to fish the HiResBoundingBox from PS-in-memory .\n");

	if (Ctrl->T.active) {		/* Then write the converted file into a file instead of storing it into a Image struct */
		char t[PATH_MAX] = {""};
		sprintf (t, " -sDEVICE=%s %s -sOutputFile=", device[Ctrl->T.device], device_options[Ctrl->T.device]);
		strcat (out_file, t);
		if (API->external && Ctrl->F.active && !gmt_M_file_is_memory (Ctrl->F.file)) {
			strncpy (t, Ctrl->F.file, PATH_MAX-1);
		}
		else {
			if (API->tmp_dir)	/* Use the established temp directory */
				sprintf (t, "%s/psconvert_tmp", API->tmp_dir);
			else	/* Must dump it in current directory */
				sprintf (t, "psconvert_tmp");
		}
		strncat (t, ext[Ctrl->T.device], 5);
		if (API->external) {		/* Apparently we cannot have the output file name inside quotes */
			strcat (out_file, t);		strcat (out_file, " -");
		}
		else {
			strncat (out_file, squote, 1);	strcat (out_file, t);	strncat (out_file, squote, 1);
			strcat (out_file, " -");
		}
	}
	if (pipe_ghost(API, Ctrl, gs_params, w, h, out_file)) {	/* Run Ghostscript to convert the PS to the desired output format */
		GMT_Report (API, GMT_MSG_ERROR, "Failed to wrap Ghostscript in pipes.\n");
		error++;
	}
	return error;
}

GMT_LOCAL int wrap_the_PS_sandwich (struct GMT_CTRL *GMT, char *main, char *bfile, char *ffile) {
	/* Replace main file with the sandwich of [bfile] + main + [ffile].  We know at least
	   one of bfile or ffile exists when this function is called, as do main of course. */
	char newfile[PATH_MAX] = {""};
	FILE *fp = NULL;

	/* Create a temporary file name for the sandwich file and open it for writing */
	sprintf (newfile, "%s/psconvert_sandwich_%d.ps", GMT->parent->tmp_dir, (int)getpid());
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Creating %s\n", newfile);
	if ((fp = fopen (newfile, "w")) == NULL) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to create temporary file %s.\n", newfile);
		return (GMT_RUNTIME_ERROR);
	}
	if (bfile) {	/* We have a background PS layer that should go first */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Prepend %s as background in %s\n", bfile, newfile);
		gmt_ps_append (GMT, bfile, 1, fp);	/* Start with this file but skip trailer */
		if (ffile) {	/* There is also a foreground PS layer that should go last */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Append %s as main content in %s\n", main, newfile);
			gmt_ps_append (GMT, main,  0, fp);	/* Append main file first but exclude both header and trailer */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Append %s as foreground in %s\n", ffile, newfile);
			gmt_ps_append (GMT, ffile, 2, fp);	/* Append foreground file but exclude header */
		}
		else {	/* No foreground, append main and its trailer */
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Append %s as main content in %s\n", main, newfile);
			gmt_ps_append (GMT, main, 2, fp);	/* Append main file; skip header but include trailer */
		}
	}
	else {	/* Just a foreground layer to append */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Prepend %s as main in %s\n", main, newfile);
		gmt_ps_append (GMT, main,  1, fp);	/* Begin with main file but exclude trailer */
		GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Append %s as foreground in %s\n", ffile, newfile);
		gmt_ps_append (GMT, ffile, 2, fp);	/* Append foreground file but exclude header */
	}
	fclose (fp);	/* Completed */

	/* Now remove original ps_file and rename the new file to main */
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Remove old %s\n", main);
	if (gmt_remove_file (GMT, main)) {
		GMT_Report (GMT->parent, GMT_MSG_ERROR, "Failed to remove original file %s.\n", main);
		return (GMT_RUNTIME_ERROR);
	}
	GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Rename %s -> %s\n", newfile, main);
	if (gmt_rename_file (GMT, newfile, main, GMT_RENAME_FILE))
		return (GMT_RUNTIME_ERROR);
	return (GMT_NOERROR);
}

GMT_LOCAL int get_extension_period (char *file) {
	int i, pos_ext = 0;
	for (i = (int)strlen(file) - 1; i > 0; i--) {
		if (file[i] == '.') { 	/* Beginning of file extension */
			pos_ext = i;
			break;
		}
	}
	return (pos_ext);
}

EXTERN_MSC int gmt_copy (struct GMTAPI_CTRL *API, enum GMT_enum_family family, unsigned int direction, char *ifile, char *ofile);

GMT_LOCAL int make_dir_if_needed (struct GMTAPI_CTRL *API, char *dir) {
	struct stat S;
	int err = stat (dir, &S);
	if (err && errno == ENOENT && gmt_mkdir (dir)) {	/* Does not exist - try to create it */
		GMT_Report (API, GMT_MSG_ERROR, "Unable to create directory %s.\n", dir);
		return (GMT_RUNTIME_ERROR);
	}
	return (GMT_NOERROR);
}


int GMT_psconvert (void *V_API, int mode, void *args) {
	unsigned int i, j, k, pix_w = 0, pix_h = 0, got_BBatend;
	int sys_retval = 0, r, pos_file, pos_ext, error = 0;
	size_t len, line_size = 0U, half_baked_size = 0;
	uint64_t pos = 0;
	bool got_BB, got_HRBB, file_has_HRBB, got_end, landscape, landscape_orig, set_background = false;
	bool excessK, setup, found_proj = false, isGMT_PS = false, return_image = false, delete = false, file_processing = true;
	bool transparency = false, look_for_transparency, BeginPageSetup_here = false, add_grestore = false;

	double xt, yt, xt_bak, yt_bak, w, h, x0 = 0.0, x1 = 612.0, y0 = 0.0, y1 = 828.0;
	double west = 0.0, east = 0.0, south = 0.0, north = 0.0;
	double gmtBB_x0 = 0.0, gmtBB_y0 = 0.0, gmtBB_height = 0, gmtBB_width = 0;
	double old_scale_x = 1, old_scale_y = 1;

	size_t n_alloc = GMT_SMALL_CHUNK;

	char **ps_names = NULL;
	char ps_file[PATH_MAX] = "", no_U_file[PATH_MAX] = "", clean_PS_file[PATH_MAX] = "", tmp_file[PATH_MAX] = "",
	     out_file[PATH_MAX] = "", BB_file[PATH_MAX] = "", resolution[GMT_LEN128] = "";
	char *line = NULL, c1[20] = {""}, c2[20] = {""}, c3[20] = {""}, c4[20] = {""},
	     cmd[GMT_BUFSIZ] = {""}, proj4_name[20] = {""}, *quiet = NULL;
	char *gs_BB = NULL, *proj4_cmd = NULL;
	char *device[N_GS_DEVICES] = {"", "pdfwrite", "svg", "jpeg", "png16m", "ppmraw", "tiff24nc", "bmp16m", "pngalpha",
	                              "jpeggray", "pnggray", "tiffgray", "bmpgray"};
	char *device_options[N_GS_DEVICES] = {
		/* extra options to pass to individual drivers */
		"",
		"", /* pdfwrite */
		"", /* svg */
		"-dJPEGQ=90", /* jpeg */
		"", /* png16m */
		"", /* ppmraw */
		"-sCompression=lzw", /* tiff24nc */
		"", /* bmp16m */
		"", /* pngalpha */
		"-dJPEGQ=90", /* jpeggray */
		"", /* pnggray */
		"-sCompression=lzw", /* tiffgray */
		""}; /* bmpgray */
	char *ext[N_GS_DEVICES] = {".eps", ".pdf", ".svg", ".jpg", ".png", ".ppm", ".tif", ".bmp", ".png", ".jpg", ".png", ".tif", ".bmp"};
	char *RefLevel[N_KML_ELEVATIONS] = {"clampToGround", "relativeToGround", "absolute", "relativeToSeaFloor", "clampToSeaFloor"};
#ifdef WIN32
	char at_sign[2] = "@";
#else
	char at_sign[2] = "";
#endif
	/* Define the 4 different sets of GS parameters for PDF or rasters, before and after SCANCONVERTERTYPE=2 added in 9.21 */
	/* 2018=09-18 [PW]: Removed -DSAFER since it now prevents using the Adobe transparency extensions.  All our test pass with no -DSAFER so I have removed it */
	/* 2020-03-14 [PW]: Added -dALLOWPSTRANSPARENCY since 9.51 onwards requires it to do transparency.  It is simply ignored by older gs versions */
	static char *gs_params_pdfnew = "-q -dNOPAUSE -dBATCH -dNOSAFER -dPDFSETTINGS=/prepress -dDownsampleColorImages=false -dDownsampleGrayImages=false -dDownsampleMonoImages=false -dUseFlateCompression=true -dEmbedAllFonts=true -dSubsetFonts=true -dMonoImageFilter=/FlateEncode -dAutoFilterGrayImages=false -dGrayImageFilter=/FlateEncode -dAutoFilterColorImages=false -dColorImageFilter=/FlateEncode -dSCANCONVERTERTYPE=2 -dALLOWPSTRANSPARENCY";
	static char *gs_params_pdfold = "-q -dNOPAUSE -dBATCH -dNOSAFER -dPDFSETTINGS=/prepress -dDownsampleColorImages=false -dDownsampleGrayImages=false -dDownsampleMonoImages=false -dUseFlateCompression=true -dEmbedAllFonts=true -dSubsetFonts=true -dMonoImageFilter=/FlateEncode -dAutoFilterGrayImages=false -dGrayImageFilter=/FlateEncode -dAutoFilterColorImages=false -dColorImageFilter=/FlateEncode";
	static char *gs_params_rasnew = "-q -dNOPAUSE -dBATCH -dNOSAFER -dSCANCONVERTERTYPE=2";
	static char *gs_params_rasold = "-q -dNOPAUSE -dBATCH -dNOSAFER";
	static char *gs_params = NULL;
#ifdef HAVE_GDAL
	struct GMT_GDALREAD_IN_CTRL  *to_gdalread = NULL;
	struct GMT_GDALREAD_OUT_CTRL *from_gdalread = NULL;
#endif

	FILE *fp = NULL, *fpo = NULL, *fpb = NULL, *fp2 = NULL, *fpw = NULL;

	struct GMT_OPTION *opt = NULL;
	struct PS2RASTER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	struct { int major, minor; } gsVersion = {0, 0};
	struct GMT_POSTSCRIPT *PS = NULL;

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

	/*---------------------------- This is the psconvert main code ----------------------------*/

	if (!Ctrl->L.active && (GMT->current.setting.run_mode == GMT_CLASSIC) && (Ctrl->In.n_files == 0)) {	/* No files given, bail */
		GMT_Report (API, GMT_MSG_ERROR, "No PostScript files specified - exiting.\n");
		Return (GMT_NOERROR);
	}

	/* Test if Ghostscript can be executed (version query) */
	if (gmt_check_executable (GMT, Ctrl->G.file, "--version", NULL, cmd)) {	/* Found Ghostscript */
		int n = sscanf (cmd, "%d.%d", &gsVersion.major, &gsVersion.minor);
		if (n != 2) {
			/* command execution failed or cannot parse response */
			GMT_Report (API, GMT_MSG_ERROR, "Failed to parse response to Ghostscript version query [n = %d %d %d].\n",
			            n, gsVersion.major, gsVersion.minor);
			Return (GMT_RUNTIME_ERROR);
		}
	}
	else {	/* Failure to open Ghostscript */
		GMT_Report (API, GMT_MSG_ERROR, "Cannot execute Ghostscript (%s).\n", Ctrl->G.file);
		Return (GMT_RUNTIME_ERROR);
	}

	if (Ctrl->T.device == GS_DEV_SVG && (gsVersion.major > 9 || (gsVersion.major == 9 && gsVersion.minor >= 16))) {
		GMT_Report (API, GMT_MSG_ERROR, "Your Ghostscript version (%d.%d) no longer supports the SVG device.\n",
		            gsVersion.major, gsVersion.minor);
		GMT_Report (API, GMT_MSG_ERROR, "We recommend converting to PDF and then installing the pdf2svg package.\n");
		Return (GMT_RUNTIME_ERROR);
	}
	if (Ctrl->F.active && Ctrl->L.active) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -F and -L are mutually exclusive. Ignoring option -F.\n");
		Ctrl->F.active = false;
	}

	if (Ctrl->F.active && Ctrl->In.n_files > 1 && Ctrl->T.device != -GS_DEV_PDF) {
		GMT_Report (API, GMT_MSG_ERROR, "Option -F is incompatible with multiple inputs. Ignoring option -F.\n");
		Ctrl->F.active = false;
	}

	if (Ctrl->F.active && gmt_M_file_is_memory (Ctrl->F.file)) {
		if (Ctrl->T.device <= GS_DEV_SVG || Ctrl->In.n_files > 1) {
			GMT_Report (API, GMT_MSG_ERROR, "Can only return one (PS or PDF) raster image to calling program via memory array.\n");
			Return (GMT_RUNTIME_ERROR);
		}
		return_image = true;
#ifndef HAVE_GDAL
		GMT_Report (API, GMT_MSG_WARNING, "Selecting ppmraw device since GDAL not available.\n");
		Ctrl->T.device = GS_DEV_PPM;
#endif
	}

	if (Ctrl->D.active && (error = make_dir_if_needed (API, Ctrl->D.dir))) {	/* Specified output directory; create if it does not exists */
		Return (error);
	}

	/* Parameters for all the formats available */

	/* Artifex says that the -dSCANCONVERTERTYPE=2 new scan converter is faster (confirmed) and
   	   will be the default in a future release. Since it was introduced in 9.21 we start using it
   	   right now and remove this conditional once it becomes the default */

	/* INitial assignment of gs_params. Note: If we detect transparency then we must select the PDF settings since we must convert to PDF first */
	if (Ctrl->T.device == GS_DEV_PDF)	/* For PDF (and PNG via PDF) we want a bunch of prepress and other settings to maximize quality */
		gs_params = (gsVersion.major >= 9 && gsVersion.minor >= 21) ? gs_params_pdfnew : gs_params_pdfold;
	else	/* For rasters */
		gs_params = (gsVersion.major >= 9 && gsVersion.minor >= 21) ? gs_params_rasnew : gs_params_rasold;

	gs_BB = "-q -dNOSAFER -dNOPAUSE -dBATCH -sDEVICE=bbox -DPSL_no_pagefill"; /* -r defaults to 4000, see http://pages.cs.wisc.edu/~ghost/doc/cvs/Devices.htm#Test */

	add_to_list (Ctrl->C.arg, "-dMaxBitmap=2147483647");	/* Add this as GS option to fix bug in GS */

	if (Ctrl->W.kml && !(Ctrl->T.device == GS_DEV_JPG ||
	    Ctrl->T.device == GS_DEV_JPGG || Ctrl->T.device == GS_DEV_TIF ||
	    Ctrl->T.device == GS_DEV_TIFG || Ctrl->T.device == GS_DEV_PNG ||
	    Ctrl->T.device == GS_DEV_TPNG || Ctrl->T.device == GS_DEV_PNGG) ) {
		GMT_Report (API, GMT_MSG_ERROR, "As far as we know selected raster type is unsupported by GE.\n");
	}

	if (Ctrl->W.active) {	/* Implies -P and -A (unless -A- is set ) */
		if (!Ctrl->A.active) Ctrl->A.active = Ctrl->A.crop = true;
		Ctrl->P.active = true;
	}

	/* Use default DPI if not already set */
	if (Ctrl->E.dpi <= 0.0) Ctrl->E.dpi = (Ctrl->T.device == GS_DEV_PDF) ? 720 : 300;

	line_size = GMT_LEN128;
	line = gmt_M_memory (GMT, NULL, line_size, char);	/* Initial buffer size */

	/* Multiple files in a file with their names */
	if (Ctrl->L.active) {
		struct GMT_DATASET *T = NULL;
		if ((T = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, GMT_IS_TEXT, GMT_READ_NORMAL, NULL, Ctrl->L.file, NULL)) == NULL) {
			gmt_M_free (GMT, line);
			Return (GMT_RUNTIME_ERROR);
		}

		Ctrl->In.n_files = (unsigned int)T->n_records;
		ps_names = gmt_M_memory (GMT, NULL, T->n_records, char *);
		for (k = 0; k < T->table[0]->segment[0]->n_rows; k++)	/* Set pointers */
			ps_names[k] = gmt_strdup_noquote (T->table[0]->segment[0]->text[k]);
	}
	else if (Ctrl->In.n_files) {	/* One or more files given on command line */
		ps_names = gmt_M_memory (GMT, NULL, Ctrl->In.n_files, char *);
		j = 0;
		for (opt = options; opt; opt = opt->next) {
			if (opt->option != '<') continue;
			ps_names[j++] = gmt_strdup_noquote (opt->arg);
		}
	}
	if (GMT->current.setting.run_mode == GMT_MODERN) {	/* Need to complete the half-baked PS file */
		if (Ctrl->In.n_files == 0) {	/* Add the default hidden PS file */
			if ((k = gmt_set_psfilename (GMT)) == 0) {	/* Get hidden file name for current PS */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "No hidden PS file %s found\n", GMT->current.ps.filename);
				Return (GMT_RUNTIME_ERROR);
			}
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Hidden PS file %s found\n", GMT->current.ps.filename);
			ps_names = gmt_M_memory (GMT, NULL, 1, char *);
			ps_names[0] = gmt_strdup_noquote (GMT->current.ps.filename);
			Ctrl->In.n_files = 1;
		}
		if (access (ps_names[0], F_OK) == 0) {	/* File exist, so complete it */
			struct stat buf;
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Complete partial PS file %s\n", ps_names[0]);
			if (stat (ps_names[0], &buf))
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Could not determine size of file %s\n", ps_names[0]);
			else
				GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Size of half-baked PS file = %" PRIuS ".\n", buf.st_size);
			half_baked_size = buf.st_size;	/* Remember the original size */
			if ((fp = PSL_fopen (GMT->PSL, ps_names[0], "a")) == NULL) {	/* Must open inside PSL DLL */
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Cannot append to file %s\n", ps_names[0]);
				gmt_M_str_free (ps_names[0]);
				Return (GMT_RUNTIME_ERROR);
			}
			GMT->PSL->internal.call_level++;	/* Must increment here since PSL_beginplot not called, and PSL_endplot will decrement */
			PSL_endplot (GMT->PSL, 1);	/* Finalize the PS plot */
			if (PSL_fclose (GMT->PSL)) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to close hidden PS file %s!\n", ps_names[0]);
				gmt_M_str_free (ps_names[0]);
				Return (GMT_RUNTIME_ERROR);
			}
			GMT_Report (GMT->parent, GMT_MSG_DEBUG, "Fattened up PS file %s\n", ps_names[0]);
		}
	}

	/* -------------------- Special case of in-memory PS. Process it and return ------------------- */
	if (API->external && Ctrl->In.n_files == 1 && ps_names[0][0] == '=') {
		if (!return_image && !Ctrl->T.active) {
			GMT_Report (API, GMT_MSG_ERROR, "Internal PSL PostScript rip requires output file via -F\n");
			error++;
		}
		else
			error = in_mem_PS_convert(API, Ctrl, ps_names, gs_BB, gs_params, device, device_options, ext);

		gmt_M_str_free (ps_names[0]);
		gmt_M_free (GMT, ps_names);
		gmt_M_free (GMT, line);
		Return (error ? GMT_RUNTIME_ERROR : GMT_NOERROR);		/* Done here */
	}
	/* --------------------------------------------------------------------------------------------- */

	/* Let gray 50 be rasterized as 50/50/50. See http://gmtrac.soest.hawaii.edu/issues/50 */
	if (!Ctrl->I.active && ((gsVersion.major == 9 && gsVersion.minor >= 5) || gsVersion.major > 9))
		add_to_list (Ctrl->C.arg, "-dUseFastColor=true");

	/* --------------------------------------------------------------------------------------------- */
	/* ------    If a multi-page PDF file creation is requested, do it and exit.   ------------------*/
	/* --------------------------------------------------------------------------------------------- */
	if (Ctrl->T.active && Ctrl->T.device == -GS_DEV_PDF) {
		char *all_names_in = NULL, *cmd2 = NULL;
		int   error = 0;

		n_alloc = 0;
		for (k = 0; k < Ctrl->In.n_files; k++)
			n_alloc += (strlen (ps_names[k]) + 3);	/* 3 = 2 quotes plus space */
		all_names_in = gmt_M_memory (GMT, NULL, n_alloc, char);
		for (k = 0; k < Ctrl->In.n_files; k++) {
			add_to_qlist (all_names_in, ps_names[k]);
			gmt_M_str_free (ps_names[k]);
		}
		cmd2 = gmt_M_memory (GMT, NULL, n_alloc + PATH_MAX, char);
		sprintf (cmd2, "%s%s -q -dNOPAUSE -dBATCH -dNOSAFER -sDEVICE=pdfwrite %s%s -r%g -sOutputFile=%c%s.pdf%c %s",
			at_sign, Ctrl->G.file, Ctrl->C.arg, alpha_bits(Ctrl), Ctrl->E.dpi, quote, Ctrl->F.file, quote, all_names_in);

		GMT_Report (API, GMT_MSG_DEBUG, "Running: %s\n", cmd2);
		sys_retval = system (cmd2);		/* Execute the Ghostscript command */
		if (sys_retval) {
			GMT_Report (API, GMT_MSG_ERROR, "System call [%s] returned error %d.\n", cmd2, sys_retval);
			error++;
		}
		if (!error && Ctrl->S.active) {
			API->print_func (GMT->session.std[GMT_ERR], cmd2);
			API->print_func (GMT->session.std[GMT_ERR], "\n");
		}
		gmt_M_free (GMT, all_names_in);
		gmt_M_free (GMT, cmd2);
		gmt_M_free (GMT, ps_names);
		gmt_M_free (GMT, line);
		Return (error ? GMT_RUNTIME_ERROR : GMT_NOERROR);		/* Done here */
	}
	/* ----------------------------------------------------------------------------------------------- */

	PS = gmt_M_memory (GMT, NULL, 1, struct GMT_POSTSCRIPT);	/* Only used if API passes = */

	/* Loop over all input files */

	for (k = 0; k < Ctrl->In.n_files; k++) {
		excessK = delete = false;
		*out_file = '\0'; /* truncate string */
		if (gmt_M_file_is_memory (ps_names[k])) {	/* For now we create temp file from PS given via memory so code below will work */
			sprintf (ps_file, "%s/psconvert_stream_%d.ps", API->tmp_dir, (int)getpid());
			if (gmt_copy (API, GMT_IS_POSTSCRIPT, GMT_OUT, ps_names[k], ps_file)) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to make temp file %s from %s. Skipping.\n", ps_file, ps_names[k]);
				continue;
			}
			delete = true;	/* Must delete this temporary file when done */
		}
		else
			strncpy (ps_file, ps_names[k], PATH_MAX-1);

		if (Ctrl->M[0].active || Ctrl->M[1].active) {	/* Must sandwich file in-between one or two layers */
			if (wrap_the_PS_sandwich (GMT, ps_file, Ctrl->M[0].file, Ctrl->M[1].file)) {
				GMT_Report (GMT->parent, GMT_MSG_ERROR, "Unable to wrap %s inside optional back- and fore-ground layers!\n", ps_file);
				if (fp) fclose (fp);
				Return (GMT_RUNTIME_ERROR);
			}
		}
		if (file_processing && (fp = fopen (ps_file, "r")) == NULL) {
			GMT_Report (API, GMT_MSG_ERROR, "Cannot open file %s\n", ps_file);
			continue;
		}

		GMT_Report (API, GMT_MSG_INFORMATION, "Processing %s...\n", ps_file);

		if (Ctrl->A.strip) {	/* Must strip off the GMT timestamp stuff, but pass any font encodings */
			int dump = true;
			GMT_Report (API, GMT_MSG_INFORMATION, "Strip GMT time-stamp...\n");
			if (GMT->current.setting.run_mode == GMT_MODERN)	/* Place temporary EPS files in session dir */
				sprintf (no_U_file, "%s/psconvert_%db.eps", API->gwf_dir, (int)getpid());
			else
				sprintf (no_U_file, "%s/psconvert_%db.eps", Ctrl->D.dir, (int)getpid());
			if (fp2) {fclose (fp2);		fp2 = NULL;}
			if ((fp2 = fopen (no_U_file, "w+")) == NULL) {
				unsigned int kk;
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create a temporary file\n");
				if (file_processing) {fclose (fp);	fp = NULL;}	/* Close original PS file */
				if (gmt_truncate_file (API, ps_file, half_baked_size)) {
					if (fpo) {fclose (fpo);		fpo = NULL;}
					Return (GMT_RUNTIME_ERROR);
				}
				if (delete && gmt_remove_file (GMT, ps_file)) {	/* Since we created a temporary file from the memdata */
					if (fpo) {fclose (fpo);		fpo = NULL;}
					Return (GMT_RUNTIME_ERROR);
				}
				for (kk = 0; kk < Ctrl->In.n_files; kk++) gmt_M_str_free (ps_names[kk]);
				gmt_M_free (GMT, ps_names);
				gmt_M_free (GMT, PS);
				gmt_M_free (GMT, line);
				if (fpo) {fclose (fpo);		fpo = NULL;}
				Return (GMT_RUNTIME_ERROR);
			}
			while (file_line_reader (GMT, &line, &line_size, fp, PS->data, &pos) != EOF) {
				if (dump && !strncmp (line, "% Begin GMT time-stamp", 22))
					dump = false;
				if (dump)
					fprintf (fp2, "%s\n", line);
				else if (!strncmp (line, "PSL_font_encode", 15))	/* Must always pass any font reencodings */
					fprintf (fp2, "%s\n", line);
				if (!dump && !strncmp (line, "% End GMT time-stamp", 20))
					dump = true;
			}
			if (file_processing) fclose (fp);	/* Close original PS file */
			rewind (fp2);	/* Rewind new file without timestamp */
			fp = fp2;	/* Set original file pointer to this file instead */
			file_processing = true;			/* Since we now are reading a temporary file */
		}

		got_BB = got_HRBB = file_has_HRBB = got_end = landscape = landscape_orig = setup = false;
		got_BBatend = 0;

		len = strlen (ps_file);
		j = (unsigned int)len - 1;
		pos_file = -1;
		pos_ext = -1;	/* In case file has no extension */
		for (i = 0; i < len; i++, j--) {
			if (pos_ext < 0 && ps_file[j] == '.') pos_ext = j;	/* Beginning of file extension */
			if (pos_file < 0 && (ps_file[j] == '/' || ps_file[j] == '\\')) pos_file = j + 1;	/* Beginning of file name */
		}
		if (pos_ext == -1) pos_ext = (unsigned int)len - 1;	/* File has no extension */
		if (!Ctrl->D.active || pos_file == -1) pos_file = 0;	/* File either has no leading directory or we want to use it */

		/* Adjust to a tight BoundingBox if user requested so */

		if (Ctrl->A.crop) {
			char *psfile_to_use = NULL;
			GMT_Report (API, GMT_MSG_INFORMATION, "Find HiResBoundingBox ...\n");
			if (GMT->current.setting.run_mode == GMT_MODERN)	/* Place BB file in session dir */
				sprintf (BB_file, "%s/psconvert_%dc.bb", API->gwf_dir, (int)getpid());
			else
				sprintf (BB_file, "%s/psconvert_%dc.bb", Ctrl->D.dir, (int)getpid());
			psfile_to_use = Ctrl->A.strip ? no_U_file : ((strlen (clean_PS_file) > 0) ? clean_PS_file : ps_file);
			sprintf (cmd, "%s%s %s %s %c%s%c 2> %c%s%c",
			         at_sign, Ctrl->G.file, gs_BB, Ctrl->C.arg, quote, psfile_to_use, quote, quote, BB_file, quote);
			GMT_Report (API, GMT_MSG_DEBUG, "Running: %s\n", cmd);
			sys_retval = system (cmd);		/* Execute the command that computes the tight BB */
			if (sys_retval) {
				GMT_Report (API, GMT_MSG_ERROR, "System call [%s] returned error %d.\n", cmd, sys_retval);
				fclose (fp);
				if (fp2) fclose (fp2);
				fp = fp2 = NULL;
				gmt_M_free (GMT, PS);
				if (gmt_remove_file (GMT, BB_file))
					Return (GMT_RUNTIME_ERROR);
				if (gmt_truncate_file (API, ps_file, half_baked_size))
					Return (GMT_RUNTIME_ERROR);
				if (delete && gmt_remove_file (GMT, ps_file))	/* Since we created a temporary file from the memdata */
					Return (GMT_RUNTIME_ERROR);
				Return (GMT_RUNTIME_ERROR);
			}
			if ((fpb = fopen (BB_file, "r")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open file %s\n", BB_file);
				gmt_M_free (GMT, PS);
				fclose (fp);	fclose (fp2);
				fp = fp2 = NULL;
				if (gmt_truncate_file (API, ps_file, half_baked_size))
					Return (GMT_RUNTIME_ERROR);
				if (delete && gmt_remove_file (GMT, ps_file))	/* Since we created a temporary file from the memdata */
					Return (GMT_RUNTIME_ERROR);
				Return (GMT_ERROR_ON_FOPEN);
			}
			while ((file_line_reader (GMT, &line, &line_size, fpb, NULL, NULL) != EOF) && !got_BB) {
				/* We only use the High resolution BB */
				if ((strstr (line,"%%HiResBoundingBox:"))) {
					sscanf (&line[19], "%s %s %s %s", c1, c2, c3, c4);
					x0 = atof (c1);		y0 = atof (c2);
					x1 = atof (c3);		y1 = atof (c4);
					x0 -= Ctrl->A.margin[XLO];	x1 += Ctrl->A.margin[XHI];	/* If not given, margin = 0/0/0/0 */
					y0 -= Ctrl->A.margin[YLO];	y1 += Ctrl->A.margin[YHI];
					if (x1 <= x0 || y1 <= y0) {
						GMT_Report (API, GMT_MSG_ERROR, "Unable to decode BoundingBox file %s (maybe no non-white features were plotted?)\n", BB_file);
						fclose (fpb);	fpb = NULL;            /* so we don't accidentally close twice */
						if (Ctrl->D.active)
							sprintf (tmp_file, "%s/", Ctrl->D.dir);
						strncat (tmp_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));
						strcat (tmp_file, ext[Ctrl->T.device]);
						sprintf (cmd, "%s%s %s %s%s -sDEVICE=%s %s -g1x1 -r%g -sOutputFile=%c%s%c %c%s%c",
							at_sign, Ctrl->G.file, gs_params, Ctrl->C.arg, alpha_bits(Ctrl), device[Ctrl->T.device],
							device_options[Ctrl->T.device],
							Ctrl->E.dpi, quote, tmp_file, quote, quote, ps_file, quote);
						GMT_Report (API, GMT_MSG_DEBUG, "Running: %s\n", cmd);
						sys_retval = system (cmd);		/* Execute the Ghostscript command */
						if (Ctrl->S.active) {
							API->print_func (GMT->session.std[GMT_ERR], cmd);
							API->print_func (GMT->session.std[GMT_ERR], "\n");
						}
						if (sys_retval) {
							GMT_Report (API, GMT_MSG_ERROR, "System call [%s] returned error %d.\n", cmd, sys_retval);
							if (gmt_remove_file (GMT, tmp_file))	/* Remove the file */
								Return (GMT_RUNTIME_ERROR);
							if (gmt_truncate_file (API, ps_file, half_baked_size))
								Return (GMT_RUNTIME_ERROR);
							if (delete && gmt_remove_file (GMT, ps_file))	/* Since we created a temporary file from the memdata */
								Return (GMT_RUNTIME_ERROR);
							gmt_M_free (GMT, PS);
							fclose (fp);	fclose (fp2);
							fp = fp2 = NULL;
							Return (GMT_RUNTIME_ERROR);
						}
						/* must leave loop because fpb has been closed and file_line_reader would
						 * read from closed file: */
						break;
					}
					got_BB = got_HRBB = true;
					GMT_Report (API, GMT_MSG_INFORMATION, "Figure dimensions: Width: %g points [%g %s]  Height: %g points [%g %s]\n",
					            x1-x0, (x1-x0)*GMT->session.u2u[GMT_PT][GMT->current.setting.proj_length_unit],
					            API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit],
					            y1-y0, (y1-y0)*GMT->session.u2u[GMT_PT][GMT->current.setting.proj_length_unit],
					            API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
				}
			}
			if (fpb != NULL) { /* don't close twice */
				fclose (fpb);	fpb = NULL;
			}
			if (!Ctrl->S.active && gmt_remove_file (GMT, BB_file)) {	/* Remove the file with BB info */
				fclose (fp);	fclose (fp2);
				fp = fp2 = NULL;
				Return (GMT_RUNTIME_ERROR);
			}
			if (got_BB) GMT_Report (API, GMT_MSG_INFORMATION, "[%g %g %g %g]...\n", x0, y0, x1, y1);
		}

		/* Open temporary file to be processed by Ghostscript. When -Te or -TE is used, tmp_file is for keeps */

		if (Ctrl->T.eps) {
			GMT_Report (API, GMT_MSG_INFORMATION, "Format EPS file...\n");
			if (Ctrl->D.active) sprintf (tmp_file, "%s/", Ctrl->D.dir);	/* Use specified output directory */
			if (!Ctrl->F.active || return_image)
				strncat (tmp_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));
			else
				strcat (tmp_file, Ctrl->F.file);
			if (strlen(tmp_file) < PATH_MAX-4)		/* To please Coverity */
				strcat (tmp_file, ext[GS_DEV_EPS]);
			if ((fpo = fopen (tmp_file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open file %s for writing\n", tmp_file);
				continue;
			}
		}
		else {
			if (GMT->current.setting.run_mode == GMT_MODERN)	/* Place temporary EPS files in session dir */
				sprintf (tmp_file, "%s/psconvert_%dd.eps", API->gwf_dir, (int)getpid());
			else
				sprintf (tmp_file, "%s/psconvert_%dd.eps", Ctrl->D.dir, (int)getpid());
			if ((fpo = fopen (tmp_file, "w+")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create a temporary file\n");
				continue;
			}
		}

		/* Scan first 20 lines of input file for [HiRes]BoundingBox and Orientation statements.
		 * Since we prefer the HiResBB over BB we must continue to read until both are found or 20 lines have past */

		i = 0;
		while ((file_line_reader (GMT, &line, &line_size, fp, PS->data, &pos) != EOF) && i < 20 && !(got_BB && got_HRBB && got_end)) {
			i++;
			if (!line[0] || line[0] != '%')
				{ /* Skip empty and non-comment lines */ }
			else if (!got_BB && strstr (line, "%%BoundingBox:")) {
				sscanf (&line[14], "%s %s %s %s",c1,c2,c3,c4);
				if (strncmp (c1, "(atend)", 7)) {	/* Got actual numbers */
					if (!got_HRBB) {	/* Only assign values if we haven't seen the high-res version yet */
						x0 = atoi (c1);		y0 = atoi (c2);
						x1 = atoi (c3);		y1 = atoi (c4);
					}
					got_BB = true;
				}
				else
					got_BBatend++;
			}
			else if ((strstr (line, "%%HiResBoundingBox:"))) {
				file_has_HRBB = true;
				if (!got_HRBB) {
					sscanf (&line[19], "%s %s %s %s",c1,c2,c3,c4);
					if (strncmp (c1, "(atend)", 7)) {	/* Got actual numbers */
						x0 = atof (c1);		y0 = atof (c2);
						x1 = atof (c3);		y1 = atof (c4);
						got_HRBB = got_BB = true;
					}
				}
			}
			else if ((strstr (line, "%%Creator:"))) {
				if (!strncmp (&line[11], "GMT", 3))
					isGMT_PS = true;
			}
			else if ((strstr (line, "%%Orientation:"))) {
				if (!strncmp (&line[15], "Landscape", 9))
					landscape = landscape_orig = true;
			}
			else if ((strstr (line, "%%EndComments")))
				got_end = true;
			if (got_BBatend == 1 && (got_end || i == 19)) {	/* Now is the time to look at the end of the file */
				got_BBatend++;			/* Avoid jumping more than once to the end */
				if (file_processing) {
					if (fseek (fp, (off_t)-256, SEEK_END))
						GMT_Report (API, GMT_MSG_ERROR, "Seeking to start of last 256 bytes failed\n");
				}
				else {	/* Get towards end of string */
					pos = (PS->n_bytes > 256) ? PS->n_bytes - 256 : 0;
				}
			}
		}

		/* Cannot proceed without knowing the BoundingBox */

		if (!got_BB) {
			GMT_Report (API, GMT_MSG_ERROR,
			            "The file %s has no BoundingBox in the first 20 lines or last 256 bytes. Use -A option.\n", ps_file);
			if (!Ctrl->T.eps && gmt_remove_file (GMT, tmp_file)) {	/* Remove the temporary EPS file */
				fclose (fp);	fclose (fp2);	fclose (fpo);
				Return (GMT_RUNTIME_ERROR);
			}
			continue;
		}

		/* Do the math on the BoundingBox and translation coordinates */

		if ((Ctrl->P.active && landscape) || (landscape && Ctrl->A.crop))
			xt = -x1, yt = -y0, w = y1-y0, h = x1-x0, r = -90;
		else
			xt = -x0, yt = -y0, w = x1-x0, h = y1-y0, r = 0;

		xt_bak = xt;	yt_bak = yt;		/* Needed when Ctrl->A.resize */

		/* Set new height when width was set and no height given */
		if (Ctrl->A.new_size[0] > 0.0 && Ctrl->A.new_size[1] == 0.0) Ctrl->A.new_size[1] = Ctrl->A.new_size[0] * h / w;

		/* ****************************************************************** */
		/*         Rewind the input file and start copying and replacing      */
		/* ****************************************************************** */

		file_rewind (fp, &pos);

		/* To produce non-PDF output from PS with transparency we must determine if transparency is requested in the PS */
		look_for_transparency = Ctrl->T.device != GS_DEV_PDF && Ctrl->T.device != -GS_DEV_PDF;
		transparency = add_grestore = false;
		set_background = (Ctrl->A.paint || Ctrl->A.outline);

		while (file_line_reader (GMT, &line, &line_size, fp, PS->data, &pos) != EOF) {
			if (line[0] != '%') {	/* Copy any non-comment line, except one containing setpagedevice in the Setup block */
				if (look_for_transparency && strstr (line, " PSL_transp")) {
					transparency = true;		/* Yes, found transparency */
					look_for_transparency = false;	/* No need to check anymore */
				}
				if (setup && strstr(line,"setpagedevice") != NULL)	/* This is a "setpagedevice" command that should be avoided */
					continue;
				fprintf (fpo, "%s\n", line);
				continue;
			}
			else if (!found_proj && !strncmp (&line[2], "PROJ", 4)) {	/* Search for the PROJ tag in the ps file */
				char *ptmp = NULL, xx1[128], xx2[128], yy1[128], yy2[128];
				sscanf (&line[8], "%s %s %s %s %s %s %s %s %s",proj4_name,xx1,xx2,yy1,yy2,c1,c2,c3,c4);
				west = atof (c1);		east = atof (c2);
				south = atof (c3);		north = atof (c4);
				GMT->common.R.wesn[XLO] = atof (xx1);		GMT->common.R.wesn[XHI] = atof (xx2);
				if (GMT->common.R.wesn[XLO] > 180.0 && GMT->common.R.wesn[XHI] > 180.0) {
					GMT->common.R.wesn[XLO] -= 360.0;
					GMT->common.R.wesn[XHI] -= 360.0;
				}
				GMT->common.R.wesn[YLO] = atof (yy1);	GMT->common.R.wesn[YHI] = atof (yy2);
				found_proj = true;
				if ((ptmp = strstr (&line[2], "+proj")) != NULL) {  /* Search for the +proj in the comment line */
					proj4_cmd = strdup (&line[(int)(ptmp - &line[0])]);
					gmt_chop (proj4_cmd);		/* Remove the new line char */
				}
				if (!strcmp (proj4_name,"latlong") || !strcmp (proj4_name,"xy")) {	/* Linear case, use original coords */
					west  = atof(xx1);		east  = atof(xx2);
					south = atof(yy1);		north = atof(yy2);
					/* One further test. +xy was found, but have we geog coords? Check that */
					if (!strcmp (proj4_name,"xy") &&
							(west >= -180) && ((east <= 360) && ((east - west) <= 360)) &&
							(south >= -90) && (north <= 90) ) {
						proj4_cmd = strdup ("latlon");
						GMT_Report (API, GMT_MSG_INFORMATION, "An unknown psconvert setting was found but since "
								"image coordinates seem to be geographical, a linear transformation "
								"will be used.\n");
					}
					else if (!strcmp (proj4_name,"xy") && Ctrl->W.warp) {	/* Do not operate on a twice unknown setting */
						GMT_Report (API, GMT_MSG_ERROR, "Requested an automatic geotiff generation, but "
								"no recognized psconvert option was used for the PS creation.\n");
					}
				}
				else if (Ctrl->W.kml) {
					GMT_Report (API, GMT_MSG_ERROR, "To GE images must be in geographical coordinates. Very likely "
								"this won't work as you wish inside GE.\n");
				}
			}
			else if (!strncmp (line, "%GMTBoundingBox:", 16)) {
				sscanf (&line[16], "%s %s %s %s",c1,c2,c3,c4);
				gmtBB_x0 = atof (c1);		gmtBB_y0 = atof (c2);
				gmtBB_width = atof (c3);	gmtBB_height = atof (c4);
				continue;
			}

			if (!strncmp (line, "%%BoundingBox:", 14)) {
				double w_t, h_t;
				w_t = w;	h_t = h;
				if (Ctrl->A.resize) {		/* Here the BB is the new size itself */
					w_t = Ctrl->A.new_size[0];		h_t = Ctrl->A.new_size[1];
				}
				if (got_BB && !Ctrl->A.round)
					fprintf (fpo, "%%%%BoundingBox: 0 0 %ld %ld\n", lrint (smart_ceil(w_t)), lrint (smart_ceil(h_t)));
				else if (got_BB && Ctrl->A.round)		/* Go against Adobe Law and round HRBB instead of ceil */
					fprintf (fpo, "%%%%BoundingBox: 0 0 %ld %ld\n", lrint (w_t), lrint (h_t));

				got_BB = false;
				if (file_has_HRBB)
					continue;	/* High-res BB will be put elsewhere */
				if (got_HRBB)
					fprintf (fpo, "%%%%HiResBoundingBox: 0 0 %.4f %.4f\n", w_t, h_t);
				got_HRBB = false;
				continue;
			}
			else if (!strncmp (line, "%%HiResBoundingBox:", 19)) {
				double w_t, h_t;
				w_t = w;	h_t = h;
				if (Ctrl->A.resize) {		/* Here the BB is the new size itself */
					w_t = Ctrl->A.new_size[0];		h_t = Ctrl->A.new_size[1];
				}
				if (got_HRBB)
					fprintf (fpo, "%%%%HiResBoundingBox: 0 0 %.4f %.4f\n", w_t, h_t);
				got_HRBB = false;
				continue;
			}
			else if (Ctrl->P.active && landscape && !strncmp (line, "%%Orientation:", 14)) {
				fprintf (fpo, "%%%%Orientation: Portrait\n");
				landscape = false;
				continue;
			}
			else if (!strncmp (line, "%%BeginSetup", 12))
				setup = true;
			else if (!strncmp (line, "%%EndSetup", 10)) {
				setup = false;
				if (Ctrl->T.eps == -1)	/* Write out setpagedevice command */
					fprintf (fpo, "<< /PageSize [%g %g] >> setpagedevice\n", w, h);
				if (r != 0)
					fprintf (fpo, "%d rotate\n", r);
				if (!gmt_M_is_zero (xt) || !gmt_M_is_zero (yt))
					fprintf (fpo, "%g %g translate\n", xt, yt);
				xt = yt = 0.0;
				r = 0;
			}
			else if (Ctrl->A.resize) {
				/* We are going to trick Ghostscript to do what -dEPSFitPage was supposed to do but doesn't
				   because it's bugged. For that we recompute a new scale, offsets and DPIs such that at the
				   end we will end up with an image with the imposed size and the current -E dpi setting.
				*/
				double new_scale_x, new_scale_y, new_off_x, new_off_y, r_x, r_y;
				char t1[GMT_LEN8], t2[GMT_LEN8];	/* To hold the translate part when landscape */
				char t3[128];		/* To hold a copy of the last commented (%%) line */
				if (!strncmp (line, "%%BeginPageSetup", 16)) {
					int n_scan = 0;
					size_t Lsize = 128U;
					char *line_ = gmt_M_memory (GMT, NULL, Lsize, char);
					BeginPageSetup_here = true;	/* Signal that on next line the job must be done */
					file_line_reader (GMT, &line_, &Lsize, fp, PS->data, &pos);   /* Read also next line which is to be overwritten (unless a comment) */
					while (line_[0] == '%') {	/* Skip all comments until we get the first actionable line */
						strncpy(t3, line_, 127);
						file_line_reader (GMT, &line_, &Lsize, fp, PS->data, &pos);
					}

					/* The trouble is that we can have things like "V 612 0 T 90 R 0.06 0.06 scale" or "V 0.06 0.06 scale" */
					if (landscape_orig)
						n_scan = sscanf (line_, "%s %s %s %*s %*s %*s %s %s", c1, t1, t2, c2, c3);
					else
						n_scan = sscanf (line_, "%s %s %s", c1, c2, c3);
					if ((strcmp (c1, "V") && strcmp (c1, "gsave")) || !(n_scan == 3 || n_scan == 5)) {
						int c;
						GMT_Report (API, GMT_MSG_ERROR, "Parsing of scale after %%%%BeginPageSetup failed\n"
						                                 "\tMaking some assumptions to try to save the situation.\n");
						old_scale_x = 1;		old_scale_y = 1;	/* Assume this */
						/* Receed two lines to position us again in a comment line (hopefully %%EndPageSetup) */
						fseek (fp, -(off_t)(strlen(line_)+strlen(t3)+2U), SEEK_CUR);
						/* But because the line termination issue we must test if we sought back long enough */
						c = fgetc (fp);
						if (c == '%')
							fseek (fp, -(off_t)(1U), SEEK_CUR);		/* Just receed the read character (file is unix terminate) */
						else
							fseek (fp, -(off_t)(3U), SEEK_CUR);		/* Receed the character and 2 more because file is Win terminated */
					}
					else {
						old_scale_x = atof (c2);		old_scale_y = atof (c3);
					}
					gmt_M_free (GMT, line_);
				}
				else if (BeginPageSetup_here) {
					BeginPageSetup_here = false;
					Ctrl->A.resize = false;       /* Need to reset so it doesn't keep checking inside this branch */
					/* Now we must calculate the new scale */
					if (Ctrl->A.rescale)          /* except if it was set as an option */
						r_x = Ctrl->A.scale;
					else if (Ctrl->A.max) {	/* Check if plot size exceeds specified limits */
						r_x = (w > Ctrl->A.new_size[0]) ? Ctrl->A.new_size[0] / w : 1.0;
						if (Ctrl->A.new_size[1] / h < r_x) r_x = Ctrl->A.new_size[1] / h;	/* Pick the smallest scale */
					}
					else
						r_x = Ctrl->A.new_size[0] / w;

					new_scale_x = new_scale_y = old_scale_x * r_x;
					new_off_x = -xt_bak + xt_bak * r_x;     /* Need to recompute the new offsets as well */
					new_off_y = -yt_bak + yt_bak * r_x;
					Ctrl->A.new_dpi_x = Ctrl->A.new_dpi_y = Ctrl->E.dpi * r_x;
					if (!Ctrl->A.max && Ctrl->A.new_size[1]) {
						r_y = Ctrl->A.new_size[1] / h;
						new_scale_y = old_scale_y * r_y;
						new_off_y = -yt_bak + yt_bak * r_y;
						Ctrl->A.new_dpi_y = Ctrl->E.dpi * r_y;
					}
					else
						r_y = r_x;	/* Not sure of this. Added later to shut up a compiler warning of r_y potentially used uninitialized */

					fprintf (fpo, "%% Recalculate translation and scale to obtain a resized image\n");
					fprintf (fpo, "%g %g translate\n", new_off_x, new_off_y);
					if (landscape_orig) {
						fprintf (fpo, "gsave %g %g translate 90 rotate %g %g scale\n", atof(t1)*r_x, atof(t2)*r_y, new_scale_x, new_scale_y);
					}
					else
						fprintf (fpo, "gsave %g %g scale\n", new_scale_x, new_scale_y);
					set_background = false;
					add_grestore = true;
					possibly_fill_or_outline_BoundingBox (GMT, &(Ctrl->A), fpo);
				}
			}
			else if (set_background) {
				/* Paint or outline the background rectangle. */
				if (!strncmp (line, "%%EndPageSetup", 14)) {
					set_background = false;
					possibly_fill_or_outline_BoundingBox (GMT, &(Ctrl->A), fpo);
				}
			}
			else if (!strncmp (line, "%%Page:", 7)) {
				if (r != 0)
					fprintf (fpo, "%d rotate\n", r);
				if (!gmt_M_is_zero (xt) || !gmt_M_is_zero (yt))
					fprintf (fpo, "%g %g translate\n", xt, yt);
				xt = yt = 0.0;
				r = 0;
			}
			else if (Ctrl->A.fade && !strncmp (line, "%%PageTrailer", 13) && Ctrl->A.fade_level > 0.0) {
				/* Place a transparent black rectangle over everything, at level of transparency */
				GMT_Report (API, GMT_MSG_INFORMATION, "Append fading to black at %d%%.\n", irint (100.0*Ctrl->A.fade_level));
				fprintf (fpo, "gsave clippath 0 0 0 %g /Normal PSL_transp F N U\n", Ctrl->A.fade_level);
				transparency = true;
			}
#ifdef HAVE_GDAL
			else if (Ctrl->A.crop && found_proj && !strncmp (line, "%%PageTrailer", 13)) {
				file_line_reader (GMT, &line, &line_size, fp, PS->data, &pos);
				fprintf (fpo, "%%%%PageTrailer\n");
				fprintf (fpo, "%s\n", line);

				if (Ctrl->Q.on[PSC_GEO]) {	/* Write a GeoPDF registration info */

					/* Allocate new control structures */
					to_gdalread = gmt_M_memory (GMT, NULL, 1, struct GMT_GDALREAD_IN_CTRL);
					from_gdalread = gmt_M_memory (GMT, NULL, 1, struct GMT_GDALREAD_OUT_CTRL);
					to_gdalread->W.active = true;
					from_gdalread->ProjRefPROJ4 = proj4_cmd;
					gmt_gdalread (GMT, NULL, to_gdalread, from_gdalread);
					if (from_gdalread->ProjRefWKT != NULL) {
						char  *new_wkt = NULL;

						fprintf (fpo, "\n%% embed georegistation info\n");
						fprintf (fpo, "[ {ThisPage} <<\n");
						fprintf (fpo, "\t/VP [ <<\n");
						fprintf (fpo, "\t\t/Type /Viewport\n");
						fprintf (fpo, "\t\t/BBox[0 0 %g %g]\n", w, h);
						fprintf (fpo, "\t\t/Measure <<\n");
						fprintf (fpo, "\t\t\t/Type /Measure\n");
						fprintf (fpo, "\t\t\t/Subtype /GEO\n");
						fprintf (fpo, "\t\t\t/Bounds[0 0 0 1 1 1 1 0]\n");
						fprintf (fpo, "\t\t\t/GPTS[%f %f %f %f %f %f %f %f]\n",
						         south, west, north, west, north, east, south, east);
						if (gmtBB_width == 0)	/* Older PS files that do not have yet the GMTBoundingBox */
							fprintf (fpo, "\t\t\t/LPTS[0 0 0 1 1 1 1 0]\n");
						else {
							/* Compute the LPTS. Takes the projected coordinate system into the page coordinate system */
							double h0, v0, x1,x2,y1,y2;
							h0 = gmtBB_x0 + xt_bak;		/* x|yt_bak are negative */
							v0 = gmtBB_y0 + yt_bak;
							x1 = h0 / w;	x2 = (h0 + gmtBB_width) / w;
							y1 = v0 / h;	y2 = (v0 + gmtBB_height) / h;
							if (landscape_orig) {		/* Ugly hack but so far have no better solution */
								x2 = gmtBB_width / w;		x1 = 1 - x2;
								y2 = gmtBB_height / h;		y1 = 1 - y2;
							}
							fprintf (fpo, "\t\t\t/LPTS[%f %f %f %f %f %f %f %f]\n", x1,y1, x1,y2, x2,y2, x2,y1);
						}
						fprintf (fpo, "\t\t\t/GCS <<\n");
						fprintf (fpo, "\t\t\t\t/Type /PROJCS\n");
						fprintf (fpo, "\t\t\t\t/WKT\n");
						new_wkt = gmt_strrep(from_gdalread->ProjRefWKT, "Mercator_1SP", "Mercator");	/* Because AR is dumb */
						fprintf (fpo, "\t\t\t\t(%s)\n", new_wkt);
						fprintf (fpo, "\t\t\t>>\n");
						fprintf (fpo, "\t\t>>\n");
						fprintf (fpo, "\t>>]\n");
						fprintf (fpo, ">> /PUT pdfmark\n\n");
						if (strlen(new_wkt) != strlen(from_gdalread->ProjRefWKT)) free(new_wkt);	/* allocated in strrep */
					}
					gmt_M_free (GMT, to_gdalread);
					gmt_M_free (GMT, from_gdalread);
				}
				continue;
			}
#endif
			fprintf (fpo, "%s\n", line);
		}
		if (add_grestore)
			fprintf (fpo, "grestore\n");	/* Since we started with gsave to change size */
		/* Recede a bit to test the contents of last line. -7 for when PS has CRLF endings */
		if (fseek (fp, (off_t)-7, SEEK_END))
			GMT_Report (API, GMT_MSG_ERROR, "Seeking to spot 7 bytes earlier failed\n");
		/* Read until last line is encountered */
		while (file_line_reader (GMT, &line, &line_size, fp, PS->data, &pos) != EOF);
		if (strncmp (line, "%%EOF", 5U))
			/* Possibly a non-closed GMT PS file. To be confirmed later */
			excessK = true;

		fclose (fpo);	fpo = NULL;
		fclose (fp);	fp  = NULL;

		if (transparency && Ctrl->T.device != GS_DEV_PDF)	/* Must reset to PDF settings since we have transparency */
			gs_params = (gsVersion.major >= 9 && gsVersion.minor >= 21) ? gs_params_pdfnew : gs_params_pdfold;

		/* Build the converting Ghostscript command and execute it */

		if (Ctrl->T.device != GS_DEV_EPS) {
			char tag[16] = {""};
			int dest_device = Ctrl->T.device;	/* Keep copy in case of temp change below */

			strncpy (tag, &ext[Ctrl->T.device][1], 15U);
			gmt_str_toupper (tag);

			if (transparency) {	/* Get here when PDF is _NOT_ the final output format but an intermediate format */
				GMT_Report (API, GMT_MSG_INFORMATION, "PS file with transparency must be converted to PDF before creating %s\n", tag);
				/* Temporarily change output device to PDF to get the PDF tmp file */
				Ctrl->T.device = GS_DEV_PDF;
				/* After conversion, we convert the tmp PDF file to desired format via a 2nd gs call */
				GMT_Report (API, GMT_MSG_INFORMATION, "Convert to intermediate PDF...\n");
				strncat (out_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));
				strcat (out_file, "_intermediate");
			}
			else {	/* Output is the final result */
					GMT_Report (API, GMT_MSG_INFORMATION, "Convert to %s...\n", tag);

				if (Ctrl->D.active) sprintf (out_file, "%s/", Ctrl->D.dir);	/* Use specified output directory */
				if (!Ctrl->F.active || return_image)
					strncat (out_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));
				else
					strcat (out_file, Ctrl->F.file);
			}
			strcat (out_file, ext[Ctrl->T.device]);

			if (Ctrl->A.new_dpi_x) {	/* We have a resize request (was Ctrl->A.resize = true;) */
				pix_w = urint (smart_ceil (w * Ctrl->A.new_dpi_x / 72.0));
				pix_h = urint (smart_ceil (h * Ctrl->A.new_dpi_y / 72.0));
			}
			else {
				pix_w = urint (smart_ceil (w * Ctrl->E.dpi / 72.0));
				pix_h = urint (smart_ceil (h * Ctrl->E.dpi / 72.0));
			}

			if (Ctrl->H.active)	/* Rasterize at higher resolution, then downsample in gs */
				sprintf (resolution, "-g%dx%d -r%g -dDownScaleFactor=%d", pix_w * Ctrl->H.factor, pix_h * Ctrl->H.factor, Ctrl->E.dpi * Ctrl->H.factor, Ctrl->H.factor);
			else
				sprintf (resolution, "-g%dx%d -r%g", pix_w, pix_h, Ctrl->E.dpi);
			sprintf (cmd, "%s%s %s %s%s -sDEVICE=%s %s %s -sOutputFile=%c%s%c %c%s%c",
				at_sign, Ctrl->G.file, gs_params, Ctrl->C.arg, alpha_bits(Ctrl), device[Ctrl->T.device],
				device_options[Ctrl->T.device], resolution, quote, out_file, quote, quote, tmp_file, quote);

			if (Ctrl->S.active) {	/* Print Ghostscript command */
				API->print_func (GMT->session.std[GMT_ERR], cmd);
				API->print_func (GMT->session.std[GMT_ERR], "\n");
			}

			/* Execute the Ghostscript command */
			GMT_Report (API, GMT_MSG_DEBUG, "Running: %s\n", cmd);
			sys_retval = system (cmd);
			if (sys_retval) {
				GMT_Report (API, GMT_MSG_ERROR, "System call [%s] returned error %d.\n", cmd, sys_retval);
				gmt_remove_file (GMT, tmp_file);	/* Since we created a temporary file from the memdata */
				Return (GMT_RUNTIME_ERROR);
			}

			/* Check output file */
			if (access (out_file, R_OK)) {		/* output file not created */
				if (isGMT_PS && excessK)
					/* non-closed GMT input PS file */
					GMT_Report (API, GMT_MSG_ERROR,
					            "%s: GMT PS format detected but file is not finalized. Maybe a -K in excess? No output created.\n", ps_file);
				else
					/* Either a bad closed GMT PS file or one of unknown origin */
					GMT_Report (API, GMT_MSG_ERROR,
					            "Could not create %s. Maybe input file does not fulfill PS specifications.\n", out_file);
			}
			else {								/* output file exists */
				if (isGMT_PS && excessK)
					/* non-closed GMT input PS file */
					GMT_Report (API, GMT_MSG_ERROR,
					            "%s: GMT PS format detected but file is not finalized. Maybe a -K in excess? %s could be messed up.\n",
					            ps_file, out_file);
				/* else: Either a good closed GMT PS file or one of unknown origin */
			}
			if (transparency) {	/* Now convert temporary PDF to desired format */
				char pdf_file[PATH_MAX] = {""};
				GMT_Report (API, GMT_MSG_INFORMATION, "Convert PDF with transparency to %s...\n", tag);
				Ctrl->T.device = dest_device;	/* Reset output device type */
				strcpy (pdf_file, out_file);	/* Now the PDF is the infile */
				*out_file = '\0'; /* truncate string to build new output file */
				if (Ctrl->D.active) sprintf (out_file, "%s/", Ctrl->D.dir);	/* Use specified output directory */
				if (!Ctrl->F.active || return_image)
					strncat (out_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));
				else
					strcat (out_file, Ctrl->F.file);
				strcat (out_file, ext[Ctrl->T.device]);
				/* After conversion, convert the tmp PDF file to desired format via a 2nd gs call */
				sprintf (cmd, "%s%s %s %s%s -sDEVICE=%s %s %s -sOutputFile=%c%s%c %c%s%c",
					at_sign, Ctrl->G.file, gs_params, Ctrl->C.arg, alpha_bits(Ctrl), device[Ctrl->T.device],
					device_options[Ctrl->T.device],
					resolution, quote, out_file, quote, quote, pdf_file, quote);
				if (Ctrl->S.active) {	/* Print 2nd Ghostscript command */
					API->print_func (GMT->session.std[GMT_ERR], cmd);
					API->print_func (GMT->session.std[GMT_ERR], "\n");
				}
				/* Execute the 2nd Ghostscript command */
				GMT_Report (API, GMT_MSG_DEBUG, "Running: %s\n", cmd);
				sys_retval = system (cmd);
				if (sys_retval) {
					GMT_Report (API, GMT_MSG_ERROR, "System call [%s] returned error %d.\n", cmd, sys_retval);
					Return (GMT_RUNTIME_ERROR);
				}
				if (!Ctrl->S.active && gmt_remove_file (GMT, pdf_file))	/* The temporary PDF file is no longer needed */
					Return (GMT_RUNTIME_ERROR);
			}
		}

		if (GMT->current.setting.run_mode == GMT_MODERN) {
			if (Ctrl->T.ps) {	/* Under modern mode we can also save the PS file by renaming it */
				strncpy (out_file, Ctrl->F.file, PATH_MAX-1);
				strcat (out_file, ".ps");
				GMT_Report (API, GMT_MSG_DEBUG, "Rename %s -> %s\n", tmp_file, out_file);
				if (gmt_rename_file (GMT, tmp_file, out_file, GMT_COPY_FILE))
					Return (GMT_RUNTIME_ERROR);
			}
		}
		else if (Ctrl->Z.active && !delete) {		/* Remove input file, if requested */
			if (gmt_remove_file (GMT, ps_file))
				Return (GMT_RUNTIME_ERROR);
		}

		if (gmt_truncate_file (API, ps_file, half_baked_size))
			Return (GMT_RUNTIME_ERROR);
		if (delete && gmt_remove_file (GMT, ps_file))	/* Since we created a temporary file from the memdata */
			Return (GMT_RUNTIME_ERROR);

		if (return_image) {	/* Must read in the saved raster image and return via Ctrl->F.file pointer */
			struct GMT_IMAGE *I = NULL;
#ifdef HAVE_GDAL	/* Since GMT_Read_Data with GMT_IS_IMAGE, GMT_IS_FILE means a call to GDAL */
			gmt_set_pad (GMT, 0U);	/* Temporary turn off padding (and thus BC setting) since we will use image exactly as is */
			/* State how we wish to receive images from GDAL */
			if (GMT->current.gdal_read_in.O.mem_layout[0])		/* At one point this should never be allowed to be empty */
				GMT_Set_Default (API, "API_IMAGE_LAYOUT", GMT->current.gdal_read_in.O.mem_layout);
			else
				GMT_Set_Default (API, "API_IMAGE_LAYOUT", "TCBa");

			if ((I = GMT_Read_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, out_file, NULL)) == NULL) {
				Return (API->error);
			}
#else			/* Here we have already set device to PPM which we can read ourselves. */
			uint64_t dim[GMT_DIM_SIZE] = {0U, 0U, 3U, 0U}; 	/* 3 bands. This might change if we do monochrome at some point */
			uint64_t row, col, band, nCols, nRows, nBands, nXY;
			FILE *fp_raw = NULL;
			unsigned char *tmp;
			if ((fp_raw = fopen (out_file, "rb")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open image file %s\n", out_file);
				Return (GMT_ERROR_ON_FOPEN);
			}
			gmt_fgets (GMT, line, GMT_LEN128, fp_raw);	/* Skip 1st line */
			gmt_fgets (GMT, line, GMT_LEN128, fp_raw);	/* Skip 2nd line */
			gmt_fgets (GMT, line, GMT_LEN128, fp_raw);	/* Get 3rd line */
			if (sscanf (line, "%" PRIu64 " %" PRIu64, &dim[GMT_X], &dim[GMT_Y]) != 2) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to decipher size of image in file %s\n", out_file);
				fclose (fp_raw);
				Return (GMT_PARSE_ERROR);
			}
			gmt_fgets (GMT, line, GMT_LEN128, fp_raw);	/* Skip 4th line */
			gmt_set_pad (GMT, 0U);	/* Temporary turn off padding (and thus BC setting) since we will use image exactly as is */
			if ((I = GMT_Create_Data (API, GMT_IS_IMAGE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, dim, NULL, NULL, 0, 0, NULL)) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to create image structure\n");
				fclose (fp_raw);
				Return (API->error);
			}

			nCols = dim[GMT_X];		nRows = dim[GMT_Y];		nBands = dim[2];	nXY = nRows * nCols;
			tmp   = gmt_M_memory(GMT, NULL, nCols * nBands, char);
			for (row = 0; row < nRows; row++) {
				fread(tmp, sizeof (unsigned char), nCols * nBands, fp_raw);		/* Read a row of nCols by nBands bytes of data */
				for (col = 0; col < nCols; col++) {
					for (band = 0; band < nBands; band++) {
						I->data[row + col*nRows + band*nXY] = tmp[band + col*nBands];
					}
				}
			}
			gmt_M_free (GMT, tmp);
			fclose (fp_raw);	fp_raw = NULL;
#endif
			if (GMT_Write_Data (API, GMT_IS_IMAGE, GMT_IS_FILE, GMT_IS_SURFACE, GMT_CONTAINER_AND_DATA, NULL, Ctrl->F.file, I) != GMT_NOERROR)
				Return (API->error);
			gmt_set_pad (GMT, API->pad);	/* Reset padding to GMT default */
			if (Ctrl->Z.active) {	/* Remove the image since it is returned to a calling program and -Z was given */
				GMT_Report (API, GMT_MSG_INFORMATION, "Removing %s...\n", out_file);
				if (gmt_remove_file (GMT, out_file))
					Return (GMT_RUNTIME_ERROR);
			}
		}

		if (!Ctrl->S.active) {
			if (!Ctrl->T.eps && gmt_remove_file (GMT, tmp_file))
				Return (GMT_RUNTIME_ERROR);
			if (strlen (no_U_file) > 0 && gmt_remove_file (GMT, no_U_file)) /* empty string == file was not created */
				Return (GMT_RUNTIME_ERROR);
			if (strlen (clean_PS_file) > 0 && gmt_remove_file (GMT, clean_PS_file)) /* empty string == file was not created */
				Return (GMT_RUNTIME_ERROR);
		}

		if (Ctrl->W.active && found_proj && !Ctrl->W.kml) {	/* Write a world file */
			double x_inc, y_inc;
			char world_file[PATH_MAX] = "", *wext = NULL, *s = NULL;

			if (gmtBB_width != 0) {			/* Got the BB via GMT internal message system. */
				double dpi_x, dpi_y;
				if (Ctrl->A.new_dpi_x) {
					dpi_x = Ctrl->A.new_dpi_x;		dpi_y = Ctrl->A.new_dpi_y;
				}
				else {
					dpi_x = Ctrl->E.dpi;		dpi_y = Ctrl->E.dpi;
				}
				x_inc = (east  - west)  / urint(ceil((gmtBB_width * dpi_x / 72.0)));
				y_inc = (north - south) / urint(ceil((gmtBB_height * dpi_y / 72.0)));
				west -= urint(ceil((gmtBB_x0 + xt_bak) * dpi_x / 72.0)) * x_inc;	/* Trick, extend West */
				north += urint(ceil((h - (gmtBB_y0 + yt_bak + gmtBB_height)) * dpi_y / 72.0)) * y_inc;	/* and North */
			}
			else {
				x_inc = (east  - west)  / pix_w;
				y_inc = (north - south) / pix_h;
			}
			GMT_Report (API, GMT_MSG_INFORMATION, "width = %d\theight = %d\tX res = %f\tY res = %f\n",
			                                        pix_w, pix_h, x_inc, y_inc);

			/* West and North of the world file contain the coordinates of the center of the pixel
				but our current values are of the NW corner of the pixel (pixel registration).
				So we'll move halph pixel inward. */
			west  += x_inc / 2.0;	north -= y_inc / 2.0;

			if (Ctrl->D.active) sprintf (world_file, "%s/", Ctrl->D.dir);	/* Use specified output directory */
			if (Ctrl->F.active) {		/* Must rip the raster file extension before adding the world one */
				pos_ext = get_extension_period (out_file);	/* Get beginning of file extension */
				out_file[pos_ext] = '\0';	/* Remove/restore extension to get world_file prefix */
				strcat(world_file, out_file);
				out_file[pos_ext] = '.';
			}
			else
				strncat (world_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));

			s = ext[Ctrl->T.device];
			wext = strdup(ext[Ctrl->T.device]);
			wext[1] = s[1];		wext[2] = s[3];		wext[3] = 'w';
			strcat (world_file, wext);

			if ((fpw = fopen (world_file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open file %s for writing\n", world_file);
			}
			else {
				fprintf (fpw, "%.12f\n0.0\n0.0\n%.12f\n%.12f\n%.12f", x_inc, -y_inc, west, north);
				fclose (fpw);	fpw = NULL;
				GMT_Report (API, GMT_MSG_INFORMATION, "Wrote world file %s\n", world_file);
				if (proj4_cmd)
					GMT_Report (API, GMT_MSG_INFORMATION, "Proj4 definition: %s\n", proj4_cmd);
			}

			gmt_M_str_free (wext);

			if (Ctrl->W.warp && proj4_cmd && proj4_cmd[1] == 'p') {	/* We got a usable Proj4 string. Run it (if gdal is around) */
				/* The true geotiff file will have the same base name plus a .tiff extension.
				   We will reuse the world_file variable because all it is need is to replace the extension */
				pos_ext = get_extension_period (world_file);	/* Get beginning of file extension */
				world_file[pos_ext] = '\0';
				strcat (world_file, ".tiff");

				if (GMT->current.setting.verbose < GMT_MSG_WARNING)	/* Shut up the gdal_translate (low level) verbosity */
					quiet = " -quiet";
				else
					quiet = "";

				sprintf (cmd, "gdal_translate -mo TIFFTAG_XRESOLUTION=%g -mo TIFFTAG_YRESOLUTION=%g -a_srs %c%s%c "
				              "-co COMPRESS=LZW -co TILED=YES %s %c%s%c %c%s%c",
					Ctrl->E.dpi, Ctrl->E.dpi, quote, proj4_cmd, quote, quiet, quote, out_file, quote, quote, world_file, quote);
				gmt_M_str_free (proj4_cmd);
				sys_retval = system (cmd);		/* Execute the gdal_translate command */
				GMT_Report (API, GMT_MSG_INFORMATION, "The gdal_translate command: \n%s\n", cmd);
				if (sys_retval) {
					GMT_Report (API, GMT_MSG_ERROR, "System call [%s] returned error %d.\n", cmd, sys_retval);
					Return (GMT_RUNTIME_ERROR);
				}
			}
			else if (Ctrl->W.warp && !proj4_cmd)
				GMT_Report (API, GMT_MSG_ERROR, "Could not find the Proj4 command in the PS file. No conversion performed.\n");
		}

		else if (Ctrl->W.kml) {	/* Write a basic kml file */
			char kml_file[PATH_MAX] = "";
			if (Ctrl->D.active)
				sprintf (kml_file, "%s/", Ctrl->D.dir);	/* Use specified output directory */
			if (Ctrl->F.active) {		/* Must rip the raster file extension before adding the kml one */
				pos_ext = get_extension_period (out_file);	/* Get beginning of file extension */
				out_file[pos_ext] = '\0';
				strcat (kml_file, out_file);
				out_file[pos_ext] = '.';	/* Reset the extension */
			}
			else
				strncat (kml_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));

			strcat (kml_file, ".kml");

			if ((fpw = fopen (kml_file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_ERROR, "Unable to open file %s for writing\n", kml_file);
			}
			else {
				if (Ctrl->W.folder) {	/* Control KML overlay vs full file */
					fprintf (fpw, "<Folder>\n\t<name>%s</name>\n", Ctrl->W.foldername);
				}
				else {
					fprintf (fpw, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
					fprintf (fpw, "<kml xmlns=\"http://earth.google.com/kml/2.1\">\n");
				}
				fprintf (fpw, "<Document>\n\t<name>%s</name>\n", Ctrl->W.doctitle);
				fprintf (fpw, "\t<GroundOverlay>\n\t\t<name>%s</name>\n", Ctrl->W.overlayname);
				fprintf (fpw, "\t\t<Icon>\n");
				fprintf (fpw, "\t\t\t<href>");
				if (Ctrl->W.URL) fprintf (fpw, "%s/", Ctrl->W.URL);
				if (Ctrl->D.active) {		/* Do not write the directory name */
					char *p;
					p = strrchr (out_file, '/');
					(p == NULL) ? fprintf (fpw, "%s</href>\n", out_file) : fprintf (fpw, "%s</href>\n", ++p);
				}
				else
					fprintf (fpw, "%s</href>\n", out_file);
				fprintf (fpw, "\t\t</Icon>\n");
				fprintf (fpw, "\t\t<altitudeMode>%s</altitudeMode>\n", RefLevel[Ctrl->W.mode]);
				if (Ctrl->W.mode > KML_GROUND_ABS && Ctrl->W.mode < KML_SEAFLOOR_ABS)
					fprintf (fpw, "\t\t<altitude>%g</altitude>\n", Ctrl->W.altitude);
				fprintf (fpw, "\t\t<LatLonBox>\n");
				fprintf (fpw, "\t\t\t<north>%f</north>\n", GMT->common.R.wesn[YHI]);
				fprintf (fpw, "\t\t\t<south>%f</south>\n", GMT->common.R.wesn[YLO]);
				fprintf (fpw, "\t\t\t<east>%f</east>\n", GMT->common.R.wesn[XHI]);
				fprintf (fpw, "\t\t\t<west>%f</west>\n", GMT->common.R.wesn[XLO]);
				fprintf (fpw, "\t\t</LatLonBox>\n");
				fprintf (fpw, "\t\t<Region>\n");
				fprintf (fpw, "\t\t<LatLonAltBox>\n");
				fprintf (fpw, "\t\t\t<north>%f</north>\n", north);
				fprintf (fpw, "\t\t\t<south>%f</south>\n", south);
				fprintf (fpw, "\t\t\t<east>%f</east>\n", east);
				fprintf (fpw, "\t\t\t<west>%f</west>\n", west);
				fprintf (fpw, "\t\t</LatLonAltBox>\n");
				if (Ctrl->W.min_lod != Ctrl->W.max_lod) {	/* Control layer visibility */
 					fprintf (fpw, "\t\t<Lod>\n");
 					fprintf (fpw, "\t\t\t<minLodPixels>%d</minLodPixels>\n", Ctrl->W.min_lod);
 					fprintf (fpw, "\t\t\t<maxLodPixels>%d</maxLodPixels>\n", Ctrl->W.max_lod);
					if (Ctrl->W.min_fade) fprintf (fpw, "\t\t\t<minFadeExtent>%d</minFadeExtent>\n", Ctrl->W.min_fade);
	 				if (Ctrl->W.max_fade) fprintf (fpw, "\t\t\t<maxFadeExtent>%d</maxFadeExtent>\n", Ctrl->W.max_fade);
 					fprintf (fpw, "\t\t</Lod>\n");
				}
				fprintf (fpw, "\t\t</Region>\n");
				fprintf (fpw, "\t</GroundOverlay>\n");
				if (Ctrl->W.folder)	/* Control KML overlay vs full file */
					fprintf (fpw, "</Folder>\n");
				else
					fprintf (fpw, "</Document>\n</kml>\n");
				fclose (fpw);	fpw = NULL;
				GMT_Report (API, GMT_MSG_INFORMATION, "Wrote KML file %s\n", kml_file);
			}
		}

		else if (Ctrl->W.active && !found_proj) {
			GMT_Report (API, GMT_MSG_ERROR, "Could not find the 'PROJ' tag in the PS file. No world file created.\n");
			GMT_Report (API, GMT_MSG_ERROR, "This situation occurs when one of the two next cases is true:\n");
			GMT_Report (API, GMT_MSG_ERROR, "1) the PS file was created with a pre-GMT v4.5.0 version\n");
			GMT_Report (API, GMT_MSG_ERROR, "2) the PS file was not created by GMT\n");
		}
	}

	for (k = 0; k < Ctrl->In.n_files; k++) gmt_M_str_free (ps_names[k]);
	gmt_M_free (GMT, ps_names);
	gmt_M_free (GMT, line);
	gmt_M_free (GMT, PS);
	/* According to Coverity there are still paths that may reach here with the files not closed */
	if (fp != NULL) fclose (fp);
	if (fpo != NULL) fclose (fpo);
	if (fpb != NULL) fclose (fpb);
	if (fp2 != NULL) fclose (fp2);
	if (fpw != NULL) fclose (fpw);
	GMT_Report (API, GMT_MSG_DEBUG, "Final input buffer length was % "PRIuS "\n", line_size);

	Return (GMT_NOERROR);
}

#ifdef WIN32
GMT_LOCAL int ghostbuster(struct GMTAPI_CTRL *API, struct PS2RASTER_CTRL *C) {
	/* Search the Windows registry for the directory containing the gswinXXc.exe
	   We do this by finding the GS_DLL that is a value of the HKLM\SOFTWARE\GPL Ghostscript\X.XX\ key
	   Things are further complicated because Win64 has TWO registries: one 32 and the other 64 bits.
	   Add to this that the installed GS version may be 32 or 64 bits, so we have to check for the
	   four GS_32|64 + GMT_32|64 combinations.

		 Adapted from snippets at http://www.daniweb.com/software-development/c/code/217174
	   and http://juknull.wordpress.com/tag/regenumkeyex-example */

	HKEY hkey;              /* Handle to registry key */
	char data[GMT_LEN256] = {""}, ver[GMT_LEN8] = {""}, *ptr;
	char key[32] = "SOFTWARE\\GPL Ghostscript\\";
	unsigned long datalen = GMT_LEN256;
	unsigned long datatype;
	long RegO, rc = 0;
	int n = 0;
	bool bits64 = true;
	float maxVersion = 0;		/* In case more than one GS, hold the number of the highest version */

	/* Before of all rest, check if we have a ghost in GMT/bin */
	sprintf (data, "%s/gswin64c.exe", API->GMT->init.runtime_bindir);
	if (!access (data, R_OK)) goto FOUNDGS;
	sprintf (data, "%s/gswin32c.exe", API->GMT->init.runtime_bindir);
	if (!access (data, R_OK)) goto FOUNDGS;

#ifdef _WIN64
	RegO = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\GPL Ghostscript", 0, KEY_READ, &hkey);	/* Read 64 bits Reg */
	if (RegO != ERROR_SUCCESS) {		/* Try the 32 bits registry */
		RegO = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\GPL Ghostscript", 0, KEY_READ|KEY_WOW64_32KEY, &hkey);
		bits64 = false;
	}
#else
	RegO = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\GPL Ghostscript", 0, KEY_READ, &hkey);	/* Read 32 bits Reg */
	if (RegO != ERROR_SUCCESS)			/* Failed. Try the 64 bits registry */
		RegO = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\GPL Ghostscript", 0, KEY_READ|KEY_WOW64_64KEY, &hkey);
	else {
		bits64 = false;
	}
#endif

	if (RegO != ERROR_SUCCESS) {
		GMT_Report (API, GMT_MSG_ERROR, "Failure while opening HKLM key\n");
		return (GMT_RUNTIME_ERROR);
	}

	while (rc != ERROR_NO_MORE_ITEMS) {
		rc  = RegEnumKeyEx (hkey, n++, data, &datalen, 0, NULL, NULL, NULL);
		datalen = GMT_LEN256; /* reset to buffer length (including terminating \0) */
		if (rc == ERROR_SUCCESS)
			maxVersion = MAX(maxVersion, strtof(data, NULL));	/* If more than one GS, keep highest version number */
	}

	RegCloseKey(hkey);

	if (maxVersion == 0) {
		GMT_Report (API, GMT_MSG_ERROR, "Unknown version reported in registry\n");
		return (GMT_RUNTIME_ERROR);
	}

	sprintf(ver, "%.2f", maxVersion);
	strcat(key, ver);

	/* Open the HKLM key, key, from which we wish to get data.
	   But now we already know the registry bitage */
#ifdef _WIN64
	if (bits64)
		RegO = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_QUERY_VALUE, &hkey);
	else
		RegO = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_QUERY_VALUE|KEY_WOW64_32KEY, &hkey);
#else
	if (bits64)
		RegO = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_QUERY_VALUE|KEY_WOW64_64KEY, &hkey);
	else
		RegO = RegOpenKeyEx(HKEY_LOCAL_MACHINE, key, 0, KEY_QUERY_VALUE, &hkey);
#endif
	if (RegO != ERROR_SUCCESS) {
		GMT_Report (API, GMT_MSG_ERROR, "Failure while opening HKLM key\n");
		return (GMT_RUNTIME_ERROR);
	}

	/* Read the value for "GS_DLL" via the handle 'hkey' */
	RegO = RegQueryValueEx(hkey, "GS_DLL", NULL, &datatype, (LPBYTE)data, &datalen);

	RegCloseKey(hkey);

	if (RegO != ERROR_SUCCESS) {
		GMT_Report (API, GMT_MSG_ERROR, "Failure while reading the GS_DLL value contents\n");
		return (GMT_RUNTIME_ERROR);
	}

	if ((ptr = strstr(data,"\\gsdll")) == NULL ) {
		GMT_Report (API, GMT_MSG_ERROR, "GS_DLL value is screwed.\n");
		return (GMT_RUNTIME_ERROR);
	}

	/* Truncate string and add affix gswinXXc.exe */
	*ptr = '\0';
	strcat(data, bits64 ? "\\gswin64c.exe" : "\\gswin32c.exe");

	/* Now finally check that the gswinXXc.exe exists */
	if (access (data, R_OK)) {
		GMT_Report (API, GMT_MSG_ERROR, "gswinXXc.exe does not exist.\n");
		return (GMT_RUNTIME_ERROR);
	}

	/* Wrap the path in double quotes to prevent troubles raised by dumb things like "Program Files" */
FOUNDGS:		/* Arrive directly here when we found a ghost in GMT/bin */
	C->G.file = malloc (strlen (data) + 3);	/* strlen + 2 * " + \0 */
	sprintf (C->G.file, "\"%s\"", data);

	return (GMT_NOERROR);
}

#endif		/* WIN32 */
