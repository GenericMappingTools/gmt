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
 *
 * Brief synopsis: psconvert converts one or several PostScript file(s) to other formats using GhostScript.
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
 * Version:	5 API
 */

#define THIS_MODULE_NAME	"psconvert"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Convert [E]PS file(s) to other formats using GhostScript"
#define THIS_MODULE_KEYS	"--O"

#include "gmt_dev.h"

#define GMT_PROG_OPTIONS "-V"

void GMT_str_toupper (char *string);

#ifdef WIN32	/* Special for Windows */
#	include <windows.h>
#	include <process.h>
#	define getpid _getpid
	int ghostbuster(struct GMTAPI_CTRL *API, struct PS2RASTER_CTRL *C);
	static char quote = '\"';
#else
	static char quote = '\'';
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

#define add_to_list(list,item) { if (list[0]) strcat (list, " "); strcat (list, item); }

struct PS2RASTER_CTRL {
	struct PS2R_In {	/* Input file info */
		unsigned int n_files;
	} In;
	struct PS2R_A {             /* -A[u][-] [Adjust boundingbox] */
		bool active;
		bool round;        /* Round HiRes BB instead of ceil */
		bool strip;        /* Remove the -U time-stamp */
		bool reset;        /* The -A- turns -A off, overriding any automode in effect */
		bool resize;       /* Resize to a user selected size */
		bool rescale;      /* Resize to a user selected scale factor */
		double scale;      /* Scale factor to go along with the 'rescale' option */
		double new_size[2];
		double margin[4];
		double new_dpi_x, new_dpi_y;
	} A;
	struct PS2R_C {	/* -C<option> */
		bool active;
		char arg[GMT_BUFSIZ];
	} C;
	struct PS2R_D {	/* -D<dir> */
		bool active;
		char *dir;
	} D;
	struct PS2R_E {	/* -E<resolution> */
		bool active;
		unsigned int dpi;
	} E;
	struct PS2R_F {	/* -F<out_name> */
		bool active;
		char *file;
	} F;
	struct PS2R_G {	/* -G<GSpath> */
		bool active;
		char *file;
	} G;
	struct PS2R_I {	/* -I */
		bool active;
	} I;
	struct PS2R_L {	/* -L<listfile> */
		bool active;
		char *file;
	} L;
	struct PS2R_P {	/* -P */
		bool active;
	} P;
	struct PS2R_Q {	/* -Q[g|t]<bits> */
		bool active;
		bool on[2];	/* [0] for graphics, [1] for text antialiasing */
		unsigned int bits[2];
	} Q;
	struct PS2R_S {	/* -S */
		bool active;
	} S;
	struct PS2R_T {	/* -T */
		bool active;
		int eps;	/* 1 if we want to make EPS, -1 with /PageSize (possibly in addition to another format) */
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

int parse_A_settings (struct GMT_CTRL *GMT, char *arg, struct PS2RASTER_CTRL *Ctrl) {
	/* Syntax: -A[u][<margins>][-][+r][+s<width>[u][/<height>[u]]] */

	bool error = false;
	unsigned int pos = 0;
	int j, k = 0;
	char txt[GMT_LEN128] = {""}, p[GMT_LEN128] = {""};
	char txt_a[GMT_LEN64] = {""}, txt_b[GMT_LEN64] = {""}, txt_c[GMT_LEN64] = {""}, txt_d[GMT_LEN64] = {""};

	Ctrl->A.active = true;

	if (arg[k] == 'u') {Ctrl->A.strip = true; k++;}
	if (*arg != '\0' && arg[strlen(arg)-1] == '-') {
		Ctrl->A.reset = true;
		return (error);
	}

	if (arg[k] && arg[k] != '+') {	/* Also specified margin(s) */
		j = sscanf (&arg[k], "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
		switch (j) {
			case 1:	/* Got uniform margin */
				Ctrl->A.margin[XLO] = Ctrl->A.margin[XHI] = Ctrl->A.margin[YLO] = Ctrl->A.margin[YHI] = GMT_to_points (GMT, txt_a);
				break;
			case 2:	/* Got seprate x/y margins */
				Ctrl->A.margin[XLO] = Ctrl->A.margin[XHI] = GMT_to_points (GMT, txt_a);
				Ctrl->A.margin[YLO] = Ctrl->A.margin[YHI] = GMT_to_points (GMT, txt_b);
				break;
			case 4:	/* Got uniform margin */
				Ctrl->A.margin[XLO] = GMT_to_points (GMT, txt_a);
				Ctrl->A.margin[XHI] = GMT_to_points (GMT, txt_b);
				Ctrl->A.margin[YLO] = GMT_to_points (GMT, txt_c);
				Ctrl->A.margin[YHI] = GMT_to_points (GMT, txt_d);
				break;
			default:
				error++;
				GMT_Report (Ctrl, GMT_MSG_NORMAL, "-A: Give 1, 2, or 4 margins\n");
				break;
		}
	}

	strncpy (txt, arg, GMT_LEN128);
	while (!error && (GMT_strtok (txt, "+", &pos, p))) {
		switch (p[0]) {
			case 'r':	/* Round */
				Ctrl->A.round = true;
				break;
			case 'S':	/* New size via a scale factor */
				Ctrl->A.rescale = true;
				Ctrl->A.scale = atof(&p[1]);
				break;
			case 's':	/* New size */
				Ctrl->A.resize = true;
				j = sscanf (&p[1], "%[^/]/%s", txt_a, txt_b);
				switch (j) {
					case 1:	/* Got width only. Height will be computed later */
						Ctrl->A.new_size[0] = GMT_to_points (GMT, txt_a);
						break;
					case 2:	/* Got seprate width/height */
						Ctrl->A.new_size[0] = GMT_to_points (GMT, txt_a);
						Ctrl->A.new_size[1] = GMT_to_points (GMT, txt_b);
						break;
					default:
						GMT_Report (Ctrl, GMT_MSG_NORMAL, "GMT ERROR -A+s<width[/height]>: Wrong size parameters\n");
						error++;
						break;
					}
				break;
		}
	}

	if (Ctrl->A.rescale && Ctrl->A.resize) {
		GMT_Report (Ctrl, GMT_MSG_NORMAL, "GMT ERROR -A+s|S: Cannot set both -A+s and -A+S\n");
		error++;
	}
	else if (Ctrl->A.rescale)    /* But we can. This makes the coding simpler later on */
		Ctrl->A.resize = true;

	return (error);
}

int parse_GE_settings (struct GMT_CTRL *GMT, char *arg, struct PS2RASTER_CTRL *C)
{
	/* Syntax: -W[+g][+k][+t<doctitle>][+n<layername>][+a<altmode>][+l<lodmin>/<lodmax>] */

	GMT_UNUSED(GMT);
	bool error = false;
	unsigned int pos = 0;
	char txt[GMT_BUFSIZ] = {""}, p[GMT_BUFSIZ] = {""};

	C->W.active = true;
	strncpy (txt, arg, GMT_BUFSIZ);
	while (!error && (GMT_strtok (txt, "+", &pos, p))) {
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
						GMT_Report (C, GMT_MSG_NORMAL, "GMT ERROR -W+a<mode>[par]: Unrecognized altitude mode %c\n", p[1]);
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
				if (C->W.overlayname) free (C->W.overlayname);	/* Already set, free then reset */
				C->W.overlayname = strdup (&p[1]);
				break;
			case 'o':	/* Produce a KML overlay as a folder subset */
				C->W.folder = true;
				C->W.foldername = strdup (&p[1]);
				break;
			case 't':	/* Set KML document title */
				if (C->W.doctitle) free (C->W.doctitle);	/* Already set, free then reset */
				C->W.doctitle = strdup (&p[1]);
				break;
			case 'u':	/* Specify a remote address for image */
				if (C->W.URL) free (C->W.URL);	/* Already set, free then reset */
				C->W.URL = strdup (&p[1]);
				break;
			default:
				GMT_Report (C, GMT_MSG_NORMAL, "GMT ERROR -W+<opt>: Unrecognized option selection %c\n", p[1]);
				error++;
				break;
		}
	}
	return (error);
}

void *New_psconvert_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PS2RASTER_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PS2RASTER_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */
#ifdef WIN32
	if ( ghostbuster(GMT->parent, C) != GMT_OK ) { /* Try first to find the gspath from registry */
		C->G.file = strdup ("gswin64c");     /* Fall back to this default and expect a miracle */
	}
#else
	C->G.file = strdup ("gs");
#endif
	C->D.dir = strdup (".");

	C->W.doctitle = strdup ("GMT KML Document");
	C->W.overlayname = strdup ("GMT Image Overlay");
	C->W.foldername = strdup ("GMT Image Folder");

	return (C);
}

void Free_psconvert_Ctrl (struct GMT_CTRL *GMT, struct PS2RASTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	free (C->D.dir);
	if (C->F.file) free (C->F.file);
	free (C->G.file);
	if (C->L.file) free (C->L.file);
	free (C->W.doctitle);
	free (C->W.overlayname);
	free (C->W.foldername);
	if (C->W.URL) free (C->W.URL);
	GMT_free (GMT, C);
}

int GMT_psconvert_usage (struct GMTAPI_CTRL *API, int level)
{
	GMT_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: psconvert <psfile1> <psfile2> <...> -A[u][<margins>][-][+r][+s|S<width[u]>[/<height>[u]]]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-C<gs_command>] [-D<dir>] [-E<resolution>] [-F<out_name>] [-G<gs_path>] [-L<listfile>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-N] [-P] [-Q[g|t]1|2|4] [-S] [-Tb|e|E|f|F|g|G|j|m|s|t] [%s]\n", GMT_V_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[-W[+a<mode>[<alt]][+f<minfade>/<maxfade>][+g][+k][+l<lodmin>/<lodmax>][+n<name>][+o<folder>][+t<title>][+u<URL>]]\n\n");

	if (level == GMT_SYNOPSIS) return (EXIT_FAILURE);

	GMT_Message (API, GMT_TIME_NONE, "Works by modifying the page size in order that the resulting\n");
	GMT_Message (API, GMT_TIME_NONE, "image will have the size specified by the BoundingBox.\n");
	GMT_Message (API, GMT_TIME_NONE, "As an option, a tight BoundingBox may be computed.\n\n");
	GMT_Message (API, GMT_TIME_NONE, "<psfile(s)> PostScript file(s) to be converted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Adjust the BoundingBox to the minimum required by the image contents.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append u to strip out time-stamps (produced by GMT -U options).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append - to make sure -A is NOT activated by -W.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append margin(s) to adjusted BoundingBox.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -A<off>[u] sets uniform margin for all 4 sides.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -A<xoff>[u]/<yoff>[u] set separate x- and y-margins.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     -A<woff>[u]/<eoff>[u]/<soff>[u]/<noff>[u] set separate w-,e-,s-,n-margins.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use the -A+s<width[u]>[/<height>[u]] option the select a new image size\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   but maintaining the DPI set by -E (ghostscript does the re-interpolation work).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Append unit u (%s) [%c].\n", GMT_DIM_UNITS_DISPLAY, API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit][0]);
	GMT_Message (API, GMT_TIME_NONE, "\t   Alternatively use -A+S<scale> to scale the image by the <scale> factor.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -A+r to force rounding of HighRes BoundingBox instead of ceil.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Specify a single, custom option that will be passed on to GhostScript\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   as is. Repeat to add several options [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D Set an alternative output directory (which must exist)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is same directory as PS files].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -D. to place the output in the current directory.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Set raster resolution in dpi [default = 720 for PDF, 300 for others].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Force the output file name. By default output names are constructed\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   using the input names as base, which are appended with an appropriate\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   extension. Use this option to provide a different name, but WITHOUT\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   extension. Extension is still determined automatically.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-G Full path to your ghostscript executable.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   NOTE: Under Unix systems this is generally not necessary.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Under Windows, ghostscript path is fished from the registry.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If this fails you can still add the GS path to system's path\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   or give the full path here.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (e.g., -Gc:\\programs\\gs\\gs9.02\\bin\\gswin64c).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I Ghostscript versions >= 9.00 change gray-shades by using ICC profiles.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   GS 9.05 and above provide the '-dUseFastColor=true' option to prevent that\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   and that is what psconvert does by default, unless option -I is set.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Note that for GS >= 9.00 and < 9.05 the gray-shade shifting is applied\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to all but PDF format. We have no solution to offer other than ... upgrade GS\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L The <listfile> is an ASCII file with names of files to be converted.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-P Force Portrait mode. All Landscape mode plots will be rotated back\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   so that they show unrotated in Portrait mode.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   This is practical when converting to image formats or preparing\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   EPS or PDF plots for inclusion in documents.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Anti-aliasing setting for (g)raphics or (t)ext; append size (1,2,4)\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   of sub-sampling box.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default is no anti-aliasing, which is the same as specifying size 1.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Apart from executing it, also writes the ghostscript command to\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   standard error and keeps all intermediate files.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Set output format [default is jpeg]:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   b means BMP.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   e means EPS.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   E means EPS with /PageSize command.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   f means PDF.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   F means multi-page PDF (requires -F).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   g means PNG.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   G means PNG (transparent where nothing is plotted).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   j means JPEG.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   m means PPM.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   s means SVG.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   t means TIF.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For b, g, j, t, append - to get a grayscale image [24-bit color].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   The EPS format can be combined with any of the other formats.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   For example, -Tef creates both an EPS and PDF file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-V Provide progress report [default is silent] and shows the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   gdal_translate command, in case you want to use this program\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   to create a geoTIFF file.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-W Write a ESRI type world file suitable to make (e.g.,) .tif files be\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   recognized as geotiff by softwares that know how to do it. Be aware,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   however, that different results are obtained depending on the image\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   contents and if the -B option has been used or not. The trouble with\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -B is that it creates a frame and very likely its annotations and\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   that introduces pixels outside the map data extent. As a consequence,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the map extents estimation will be wrong. To avoid this problem, use\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   the --MAP_FRAME_TYPE=inside option which plots all annotations related\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   stuff inside the image and does not compromise the coordinate.\n");
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
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Remove input PostScript file(s) after successful conversion.\n");

	return (EXIT_FAILURE);
}

int GMT_psconvert_parse (struct GMT_CTRL *GMT, struct PS2RASTER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to psconvert and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, mode;
	int j;
	bool grayscale;
	struct GMT_OPTION *opt = NULL;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				if (!GMT_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_TEXTSET)) n_errors++;
				Ctrl->In.n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Adjust BoundingBox: -A[u][<margins>][-][+r][+s<width>[u][/<height>[u]]] or -A- */
				n_errors = parse_A_settings (GMT, opt->arg, Ctrl);
				break;
			case 'C':	/* Append extra custom GS options */
				add_to_list (Ctrl->C.arg, opt->arg);	/* Append to list of extra GS options */
				break;
			case 'D':	/* Change output directory */
				if ((Ctrl->D.active = GMT_check_filearg (GMT, 'D', opt->arg, GMT_OUT, GMT_IS_TEXTSET))) {
					free (Ctrl->D.dir);
					Ctrl->D.dir = strdup (opt->arg);
				}
				else
					n_errors++;
				break;
			case 'E':	/* Set output dpi */
				Ctrl->E.active = true;
				Ctrl->E.dpi = atoi (opt->arg);
				break;
			case 'F':	/* Set explicitly the output file name */
				if ((Ctrl->F.active = GMT_check_filearg (GMT, 'F', opt->arg, GMT_OUT, GMT_IS_TEXTSET))) {
					Ctrl->F.file = strdup (opt->arg);
					GMT_chop_ext (Ctrl->F.file);	/* Make sure file name has no extension */
				}
				else
					n_errors++;
				break;
			case 'G':	/* Set GS path */
				if ((Ctrl->G.active = GMT_check_filearg (GMT, 'G', opt->arg, GMT_IN, GMT_IS_TEXTSET))) {
					free (Ctrl->G.file);
					Ctrl->G.file = malloc (strlen (opt->arg)+3);	/* Add space for quotes */
					sprintf (Ctrl->G.file, "%c%s%c", quote, opt->arg, quote);
				}
				else
					n_errors++;
				break;
			case 'I':	/* Do not use the ICC profile when converting gray shades */
				Ctrl->I.active = true;
				break;
			case 'L':	/* Give list of files to convert */
				if ((Ctrl->L.active = GMT_check_filearg (GMT, 'L', opt->arg, GMT_IN, GMT_IS_TEXTSET)))
					Ctrl->L.file = strdup (opt->arg);
				else
					n_errors++;
				break;
			case 'P':	/* Force Portrait mode */
				Ctrl->P.active = true;
				break;
			case 'Q':	/* Anti-aliasing settings */
				Ctrl->Q.active = true;
				if (opt->arg[0] == 'g')
					mode = 0;
				else if (opt->arg[0] == 't')
					mode = 1;
				else {
					GMT_default_error (GMT, opt->option);
					n_errors++;
					continue;

				}
				Ctrl->Q.on[mode] = true;
				Ctrl->Q.bits[mode] = (opt->arg[1]) ? atoi (&opt->arg[1]) : 4;
				break;
			case 'S':	/* Write the GS command to STDERR */
				Ctrl->S.active = true;
				break;
			case 'T':	/* Select output format (optionally also request EPS) */
				Ctrl->T.active = true;
				grayscale = ((j = (int)strlen(opt->arg)) > 1 && opt->arg[j-1] == '-');
				for (j = 0; opt->arg[j]; j++) {
					switch (opt->arg[j]) {
						case 'e':	/* EPS */
							Ctrl->T.eps = 1;
							break;
						case 'E':	/* EPS with /PageSize */
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
						case 's':	/* SVG */
							Ctrl->T.device = GS_DEV_SVG;
							break;
						case 't':	/* TIFF */
							Ctrl->T.device = (grayscale) ? GS_DEV_TIFG : GS_DEV_TIF;
							break;
						case '-':	/* Just skip the trailing - for grayscale since it is handled separately */
							break;
						default:
							GMT_default_error (GMT, opt->option);
							n_errors++;
							break;
					}
				}
				break;
			case 'W':	/* Save world file */
				n_errors = parse_GE_settings (GMT, opt->arg, Ctrl);
				break;

			case 'Z':
				Ctrl->Z.active = true;

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (!Ctrl->T.active) Ctrl->T.device = GS_DEV_JPG;	/* Default output device if none is specified */

	n_errors += GMT_check_condition (GMT, Ctrl->Q.on[0] && (Ctrl->Q.bits[0] < 1 || Ctrl->Q.bits[0] > 4),
		"Syntax error: Anti-aliasing for graphics requires sub-sampling box of 1,2, or 4\n");

	n_errors += GMT_check_condition (GMT, Ctrl->Q.on[1] && (Ctrl->Q.bits[1] < 1 || Ctrl->Q.bits[1] > 4),
		"Syntax error: Anti-aliasing for text requires sub-sampling box of 1,2, or 4\n");

	n_errors += GMT_check_condition (GMT, Ctrl->In.n_files > 1 && Ctrl->L.active,
		"Syntax error: Cannot handle both a file list and multiple ps files in input\n");

	n_errors += GMT_check_condition (GMT, Ctrl->L.active && access (Ctrl->L.file, R_OK),
		"Error: Cannot read list file %s\n", Ctrl->L.file);

	n_errors += GMT_check_condition (GMT, Ctrl->T.device == -GS_DEV_PDF && !Ctrl->F.active,
		"Syntax error: Creation of Multipage PDF requires setting -F option\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

int64_t line_reader (struct GMT_CTRL *GMT, char **L, size_t *size, FILE *fp)
{
	int c;
	int64_t in = 0;
	char *line = *L;
	while ((c = fgetc (fp)) != EOF) {
		if (c == '\r' || c == '\n') {	/* Got logical end of record */
			line[in] = '\0';
			while (c == '\n' || c == '\r') c = fgetc (fp);	/* Skip past any repeating \r \n */
			if (c != EOF) ungetc (c, fp);	/* Put back the next char unless we got to EOF */
			return in;	/* How many characters we return */
		}
		if ((size_t)in == (*size-1)) {	/* Need to extend our buffer; the -1 makes room for an \0 as needed */
			(*size) <<= 1;	/* Double the current buffer space */
			line = *L = GMT_memory (GMT, *L, *size, char);
		}
		line[in++] = c;	/* Add this char to our buffer */
	}
	if (c == EOF) return EOF;
	if (in) line[in] = '\0';
	return in;
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_psconvert_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

static inline char * alpha_bits (struct PS2RASTER_CTRL *Ctrl) {
	/* return alpha bits which are valid for the selected driver */
	static char alpha[48];
	char *c = alpha;
	*alpha = '\0'; /* reset string */
	if (Ctrl->Q.on[0]) {
		unsigned int bits;
		if (Ctrl->T.device == GS_DEV_PDF || Ctrl->T.device == -GS_DEV_PDF)
			/* Note: cannot set GraphicsAlphaBits > 1 with a vector device */
			bits = 1;
		else
			bits = Ctrl->Q.bits[0];
		sprintf (c, " -dGraphicsAlphaBits=%d", bits);
		c = strrchr(c, '\0'); /* advance to end of string */
	}
	if (Ctrl->Q.on[1])
		sprintf (c, " -dTextAlphaBits=%d", Ctrl->Q.bits[1]);
	return alpha;
}

int GMT_psconvert (void *V_API, int mode, void *args)
{
	unsigned int i, j, k, pix_w = 0, pix_h = 0, got_BBatend;
	int sys_retval = 0, r, pos_file, pos_ext, error = 0;
	size_t len, line_size = 0U;
	bool got_BB, got_HRBB, file_has_HRBB, got_end, landscape, landscape_orig;
	bool excessK, setup, found_proj = false, isGMT_PS = false;
	bool transparency = false, look_for_transparency, BeginPageSetup_here = false;

	double xt, yt, xt_bak, yt_bak, w, h, x0 = 0.0, x1 = 612.0, y0 = 0.0, y1 = 828.0;
	double west = 0.0, east = 0.0, south = 0.0, north = 0.0;
	double old_scale_x = 1, old_scale_y = 1;

	size_t n_alloc = GMT_SMALL_CHUNK;

	char **ps_names = NULL;
	char ps_file[GMT_BUFSIZ] = "", no_U_file[GMT_BUFSIZ] = "",
			clean_PS_file[GMT_BUFSIZ] = "", tmp_file[GMT_BUFSIZ] = "",
			out_file[GMT_BUFSIZ] = "", BB_file[GMT_BUFSIZ] = "";
	char *line = NULL, c1[20] = {""}, c2[20] = {""}, c3[20] = {""}, c4[20] = {""},
			cmd[GMT_BUFSIZ] = {""}, proj4_name[20] = {""}, *quiet = NULL;
	char *gs_params = NULL, *gs_BB = NULL, *proj4_cmd = NULL;
	char *device[N_GS_DEVICES] = {"", "pdfwrite", "svg", "jpeg", "png16m", "ppmraw", "tiff24nc", "bmp16m", "pngalpha", "jpeggray", "pnggray", "tiffgray", "bmpgray"};
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

#ifdef HAVE_GDAL
	struct GDALREAD_CTRL *to_gdalread = NULL;
	struct GD_CTRL *from_gdalread = NULL;
#endif

	FILE *fp = NULL, *fpo = NULL, *fpb = NULL, *fpl = NULL, *fp2 = NULL, *fpw = NULL;

	struct GMT_OPTION *opt = NULL;
	struct PS2RASTER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMTAPI_CTRL *API = GMT_get_API_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */
	struct { int major, minor; } gsVersion = {0, 0};

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (GMT_psconvert_usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (GMT_psconvert_usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (GMT_psconvert_usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_psconvert_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_psconvert_parse (GMT, Ctrl, options))) Return (error);

	/*---------------------------- This is the psconvert main code ----------------------------*/

	/* Test if GhostScript can be executed (version query) */
	sprintf(cmd, "%s --version", Ctrl->G.file);
	if ((fp = popen(cmd, "r")) != NULL) {
		int n;
		n = fscanf(fp, "%d.%d", &gsVersion.major, &gsVersion.minor);
		if (pclose(fp) == -1)
			GMT_Report (API, GMT_MSG_NORMAL, "Error closing GhostScript version query.\n");
		if (n != 2) {
			/* command execution failed or cannot parse response */
			GMT_Report (API, GMT_MSG_NORMAL, "Failed to parse response to GhostScript version query.\n");
			Return (EXIT_FAILURE);
		}
	}
	else { /* failed to open pipe */
		GMT_Report (API, GMT_MSG_NORMAL, "Cannot execute GhostScript (%s).\n", Ctrl->G.file);
		Return (EXIT_FAILURE);
	}

	if (Ctrl->F.active && (Ctrl->L.active || Ctrl->D.active)) {
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: Option -F and options -L OR -D are mutually exclusive. Ignoring option -F.\n");
		Ctrl->F.active = false;
	}

	if (Ctrl->F.active && Ctrl->In.n_files > 1 && Ctrl->T.device != -GS_DEV_PDF) {
		GMT_Report (API, GMT_MSG_NORMAL, "Warning: Option -F is incompatible with multiple inputs. Ignoring option -F.\n");
		Ctrl->F.active = false;
	}

	/* Parameters for all the formats available */

	gs_params = "-q -dSAFER -dNOPAUSE -dBATCH -dUseFlateCompression=true -dPDFSETTINGS=/prepress -dEmbedAllFonts=true -dSubsetFonts=true -dMonoImageFilter=/FlateEncode -dAutoFilterGrayImages=false -dGrayImageFilter=/FlateEncode -dAutoFilterColorImages=false -dColorImageFilter=/FlateEncode";
	gs_BB = "-q -dSAFER -dNOPAUSE -dBATCH -sDEVICE=bbox"; /* -r defaults to 4000, see http://pages.cs.wisc.edu/~ghost/doc/cvs/Devices.htm#Test */

	add_to_list (Ctrl->C.arg, "-dMaxBitmap=2147483647");	/* Add this as GS option to fix bug in GS */

	if (Ctrl->W.kml && !(Ctrl->T.device == GS_DEV_JPG ||
	    Ctrl->T.device == GS_DEV_JPGG || Ctrl->T.device == GS_DEV_TIF ||
	    Ctrl->T.device == GS_DEV_TIFG || Ctrl->T.device == GS_DEV_PNG ||
	    Ctrl->T.device == GS_DEV_TPNG || Ctrl->T.device == GS_DEV_PNGG) ) {
		GMT_Report (API, GMT_MSG_NORMAL, "Error: As far as we know selected raster type is unsuported by GE.\n");
	}

	if (Ctrl->W.active) {	/* Implies -P and -A (unless -A- is set ) */
		Ctrl->P.active = Ctrl->A.active = true;
		if (Ctrl->A.reset) Ctrl->A.active = false;
	}

	/* Use default DPI if not already set */
	if (Ctrl->E.dpi <= 0) Ctrl->E.dpi = (Ctrl->T.device == GS_DEV_PDF) ? 720 : 300;

	line_size = GMT_BUFSIZ;
	line = GMT_memory (GMT, NULL, line_size, char);	/* Initial buffer size */

	/* Multiple files in a file with their names */
	if (Ctrl->L.active) {
		if ((fpl = fopen (Ctrl->L.file, "r")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: Cannot open list file %s\n", Ctrl->L.file);
			Return (EXIT_FAILURE);
		}
		ps_names = GMT_memory (GMT, NULL, n_alloc, char *);
		while (line_reader (GMT, &line, &line_size, fpl) != EOF) {
			if (!*line || *line == '#') /* Empty line or comment */
				continue;
			ps_names[Ctrl->In.n_files++] = strdup (line);
			if (Ctrl->In.n_files > n_alloc) {
				n_alloc <<= 1;
				ps_names = GMT_memory (GMT, ps_names, n_alloc, char *);
			}
		}
		fclose (fpl);
	}

	/* One or more files given on command line */

	else if (Ctrl->In.n_files) {
		ps_names = GMT_memory (GMT, NULL, Ctrl->In.n_files, char *);
		j = 0;
		for (opt = options; opt; opt = opt->next) {
			if (opt->option != '<') continue;
			ps_names[j++] = strdup (opt->arg);
		}
	}

	/* Let gray 50 be rasterized as 50/50/50. See http://gmtrac.soest.hawaii.edu/issues/50 */
	if (!Ctrl->I.active &&
			((gsVersion.major == 9 && gsVersion.minor >= 5) || gsVersion.major > 9))
		add_to_list (Ctrl->C.arg, "-dUseFastColor=true");


	/* --------------------------------------------------------------------------------------------- */
	/* ------    If a multi-page PDF file creation is requested, do it and exit.   ------------------*/
	/* --------------------------------------------------------------------------------------------- */
	if (Ctrl->T.active && Ctrl->T.device == -GS_DEV_PDF) {
		char *all_names_in = NULL, *cmd2 = NULL;

		n_alloc = 0;
		for (k = 0; k < Ctrl->In.n_files; k++)
			n_alloc += (strlen (ps_names[k]) + 1);
		all_names_in = GMT_memory (GMT, NULL, n_alloc, char);
		for (k = 0; k < Ctrl->In.n_files; k++) {
			add_to_list (all_names_in, ps_names[k]);
			free (ps_names[k]);
		}
		cmd2 = GMT_memory (GMT, NULL, n_alloc + GMT_BUFSIZ, char);
		sprintf (cmd2, "%s%s -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite %s%s -r%d -sOutputFile=%c%s.pdf%c %c%s%c",
			at_sign, Ctrl->G.file, Ctrl->C.arg, alpha_bits(Ctrl), Ctrl->E.dpi, quote, Ctrl->F.file, quote, quote, all_names_in, quote);

		GMT_Report (API, GMT_MSG_DEBUG, "Running: %s\n", cmd2);
		sys_retval = system (cmd2);		/* Execute the GhostScript command */
		if (sys_retval) {
			GMT_Report (API, GMT_MSG_NORMAL, "System call [%s] returned error %d.\n", cmd2, sys_retval);
			Return (EXIT_FAILURE);
		}
		if (Ctrl->S.active)
			GMT_Report (API, GMT_MSG_NORMAL, "%s\n", cmd2);

		GMT_free (GMT, all_names_in);
		GMT_free (GMT, cmd2);
		GMT_free (GMT, ps_names);
		Return (GMT_OK);
	}
	/* ----------------------------------------------------------------------------------------------- */

	/* Loop over all input files */

	for (k = 0; k < Ctrl->In.n_files; k++) {
		excessK = false;
		*out_file = '\0'; /* truncate string */
		strncpy (ps_file, ps_names[k], GMT_BUFSIZ);
		if ((fp = fopen (ps_file, "r")) == NULL) {
			GMT_Report (API, GMT_MSG_NORMAL, "Cannot open file %s\n", ps_file);
			continue;
		}

		GMT_Report (API, GMT_MSG_VERBOSE, "Processing %s...\n", ps_file);

		if (Ctrl->A.strip) {	/* Must strip off the GMT timestamp stuff, but pass any font encodings */
			int dump = true;
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Strip GMT time-stamp...\n");
			sprintf (no_U_file, "%s/psconvert_%db.eps", Ctrl->D.dir, (int)getpid());
			if ((fp2 = fopen (no_U_file, "w+")) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a temporary file\n");
				Return (EXIT_FAILURE);
			}
			while (line_reader (GMT, &line, &line_size, fp) != EOF) {
				if (dump && !strncmp (line, "% Begin GMT time-stamp", 22))
					dump = false;
				if (dump)
					fprintf (fp2, "%s\n", line);
				else if (!strncmp (line, "PSL_font_encode", 15))	/* Must always pass any font reencodings */
					fprintf (fp2, "%s\n", line);
				if (!dump && !strncmp (line, "% End GMT time-stamp", 20))
					dump = true;
			}
			fclose (fp);	/* Close original PS file */
			rewind (fp2);	/* Rewind new file without timestamp */
			fp = fp2;	/* Set original file pointer to this file instead */
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

		if (Ctrl->A.active) {
			char *psfile_to_use;
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Find HiResBoundingBox ...\n");
			sprintf (BB_file, "%s/psconvert_%dc.bb", Ctrl->D.dir, (int)getpid());
			psfile_to_use = Ctrl->A.strip ? no_U_file : ((strlen (clean_PS_file) > 0) ? clean_PS_file : ps_file);
			sprintf (cmd, "%s%s %s %s %c%s%c 2> %c%s%c", at_sign, Ctrl->G.file, gs_BB, Ctrl->C.arg, quote, psfile_to_use, quote, quote, BB_file, quote);
			GMT_Report (API, GMT_MSG_DEBUG, "Running: %s\n", cmd);
			sys_retval = system (cmd);		/* Execute the command that computes the tight BB */
			if (sys_retval) {
				GMT_Report (API, GMT_MSG_NORMAL, "System call [%s] returned error %d.\n", cmd, sys_retval);
				Return (EXIT_FAILURE);
			}
			if ((fpb = fopen (BB_file, "r")) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to open file %s\n", BB_file);
				Return (EXIT_FAILURE);
			}
			while ((line_reader (GMT, &line, &line_size, fpb) != EOF) && !got_BB) {
				/* We only use the High resolution BB */
				if ((strstr (line,"%%HiResBoundingBox:"))) {
					sscanf (&line[19], "%s %s %s %s", c1, c2, c3, c4);
					x0 = atof (c1);		y0 = atof (c2);
					x1 = atof (c3);		y1 = atof (c4);
					x0 -= Ctrl->A.margin[XLO];	x1 += Ctrl->A.margin[XHI];	/* If not given, margin = 0/0/0/0 */
					y0 -= Ctrl->A.margin[YLO];	y1 += Ctrl->A.margin[YHI];
					if (x1 <= x0 || y1 <= y0) {
						GMT_Report (API, GMT_MSG_NORMAL, "Unable to decode BoundingBox file %s\n", BB_file);
						fclose (fpb);
						fpb = NULL;                      /* so we don't accidentally close twice */
						if (!Ctrl->S.active) remove (BB_file);                /* Remove the file */
						if (Ctrl->D.active)
							sprintf (tmp_file, "%s/", Ctrl->D.dir);
						strncat (tmp_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));
						strcat (tmp_file, ext[Ctrl->T.device]);
						sprintf (cmd, "%s%s %s %s%s -sDEVICE=%s %s -g1x1 -r%d -sOutputFile=%c%s%c -f%c%s%c",
							at_sign, Ctrl->G.file, gs_params, Ctrl->C.arg, alpha_bits(Ctrl), device[Ctrl->T.device],
							device_options[Ctrl->T.device],
							Ctrl->E.dpi, quote, tmp_file, quote, quote, ps_file, quote);
						GMT_Report (API, GMT_MSG_DEBUG, "Running: %s\n", cmd);
						sys_retval = system (cmd);		/* Execute the GhostScript command */
						if (Ctrl->S.active)
							GMT_Report (API, GMT_MSG_NORMAL, "%s\n", cmd);
						if (sys_retval) {
							GMT_Report (API, GMT_MSG_NORMAL, "System call [%s] returned error %d.\n", cmd, sys_retval);
							Return (EXIT_FAILURE);
						}
						/* must leave loop because fpb has been closed and line_reader would
						 * read from closed file: */
						break;
					}
					got_BB = got_HRBB = true;
					GMT_Report (API, GMT_MSG_VERBOSE, "Figure dimensions: Width: %g points [%g %s]  Height: %g points [%g %s]\n",
						x1-x0,
						(x1-x0)*GMT->session.u2u[GMT_PT][GMT->current.setting.proj_length_unit], API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit],
						y1-y0,
						(y1-y0)*GMT->session.u2u[GMT_PT][GMT->current.setting.proj_length_unit], API->GMT->session.unit_name[API->GMT->current.setting.proj_length_unit]);
				}
			}
			if (fpb != NULL) /* don't close twice */
				fclose (fpb);
			if (!Ctrl->S.active) remove (BB_file);	/* Remove the file with BB info */
			if (got_BB) GMT_Report (API, GMT_MSG_LONG_VERBOSE, "[%g %g %g %g]...\n", x0, y0, x1, y1);
		}

		/* Open temporary file to be processed by ghostscript. When -Te is used, tmp_file is for keeps */

		if (Ctrl->T.eps)
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Format EPS file...\n");
		if (Ctrl->T.eps) {
			if (Ctrl->D.active) sprintf (tmp_file, "%s/", Ctrl->D.dir);	/* Use specified output directory */
			if (!Ctrl->F.active)
				strncat (tmp_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));
			else
				strcat (tmp_file, Ctrl->F.file);
			strcat (tmp_file, ext[GS_DEV_EPS]);
			if ((fpo = fopen (tmp_file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to open file %s for writing\n", tmp_file);
				continue;
			}
		}
		else {
			sprintf (tmp_file, "%s/psconvert_%dd.eps", Ctrl->D.dir, (int)getpid());
			if ((fpo = fopen (tmp_file, "w+")) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to create a temporary file\n");
				continue;
			}
		}

		/* Scan first 20 lines of input file for [HiRes]BoundingBox and Orientation statements.
		 * Since we prefer the HiResBB over BB we must continue to read until both are found or 20 lines have past */

		i = 0;
		while ((line_reader (GMT, &line, &line_size, fp) != EOF) && i < 20 && !(got_BB && got_HRBB && got_end)) {
			i++;
			if (!line[0] || line[0] != '%')
				{ /* Skip empty and non-comment lines */ }
			else if (!got_BB && strstr (line, "%%BoundingBox:")) {
				sscanf (&line[14], "%s %s %s %s",c1,c2,c3,c4);
				if (strncmp (c1, "(atend)", 7)) {	/* Got actual numbers */
					if (!got_HRBB) {	/* Only assign values if we havent seen the high-res version yet */
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
				if (!fseek (fp, (off_t)-256, SEEK_END)) i = -30;
			}
		}

		/* Cannot proceed without knowing the BoundingBox */

		if (!got_BB) {
			GMT_Report (API, GMT_MSG_NORMAL, "Error: The file %s has no BoundingBox in the first 20 lines or last 256 bytes. Use -A option.\n", ps_file);
			continue;
		}

		/* Do the math on the BoundingBox and translation coordinates */

		if (Ctrl->P.active && landscape)
			xt = -x1, yt = -y0, w = y1-y0, h = x1-x0, r = -90;
		else
			xt = -x0, yt = -y0, w = x1-x0, h = y1-y0, r = 0;

		xt_bak = xt;	yt_bak = yt;		/* Needed when Ctrl->A.resize */

		/* ****************************************************************** */
		/*         Rewind the input file and start copying and replacing      */
		/* ****************************************************************** */

		rewind (fp);

		/* To produce non-PDF output from PS with transparency we must determine if transparency is requested in the PS */
		look_for_transparency = Ctrl->T.device != GS_DEV_PDF && Ctrl->T.device != -GS_DEV_PDF;
		transparency = false;
		while (line_reader (GMT, &line, &line_size, fp) != EOF) {
			if (line[0] != '%') {	/* Copy any non-comment line, except one containing /PageSize in the Setup block */
				if (look_for_transparency && strstr (line, " PSL_transp")) {
					transparency = true;		/* Yes, found transparency */
					look_for_transparency = false;	/* No need to check anymore */
				}
				if (setup && strstr(line,"/PageSize") != NULL)
					continue;
				fprintf (fpo, "%s\n", line);
				continue;
			}
			else if (Ctrl->W.active && !found_proj) {
				if (!strncmp (&line[2], "PROJ", 4)) { /* Search for the PROJ tag in the ps file */
					char *ptmp = NULL, xx1[20], xx2[20], yy1[20], yy2[20];
					sscanf (&line[8], "%s %s %s %s %s %s %s %s %s",proj4_name,xx1,xx2,yy1,yy2,c1,c2,c3,c4);
					west = atof (c1);		east = atof (c2);
					south = atof (c3);		north = atof (c4);
					GMT->common.R.wesn[XLO] = west = atof (xx1);		GMT->common.R.wesn[XHI] = east = atof (xx2);
					if (GMT->common.R.wesn[XLO] > 180.0 && GMT->common.R.wesn[XHI] > 180.0) {
						GMT->common.R.wesn[XLO] -= 360.0;
						GMT->common.R.wesn[XHI] -= 360.0;
					}
					GMT->common.R.wesn[YLO] = south = atof (yy1);	GMT->common.R.wesn[YHI] = north = atof (yy2);
					found_proj = true;
					if ((ptmp = strstr (&line[2], "+proj")) != NULL) {  /* Search for the +proj in the comment line */
						proj4_cmd = strdup (&line[(int)(ptmp - &line[0])]);
						GMT_chop (proj4_cmd);		/* Remove the new line char */
					}
					if (!strcmp (proj4_name,"latlong") || !strcmp (proj4_name,"xy") ||
						!strcmp (proj4_name,"eqc") ) {		/* Linear case, use original coords */
						west  = atof(xx1);		east  = atof(xx2);
						south = atof(yy1);		north = atof(yy2);
						/* One further test. +xy was found, but have we geog coords? Check that */
						if (!strcmp (proj4_name,"xy") &&
								(west >= -180) && ((east <= 360) && ((east - west) <= 360)) &&
								(south >= -90) && (north <= 90) ) {
							proj4_cmd = strdup ("latlon");
							GMT_Report (API, GMT_MSG_NORMAL, "Warning: An unknown psconvert setting was found but since "
									"image coordinates seem to be geographical, a linear transformation "
									"will be used.\n");
						}
						else if (!strcmp (proj4_name,"xy") && Ctrl->W.warp) {	/* Do not operate on a twice unknown setting */
							GMT_Report (API, GMT_MSG_NORMAL, "Error: requested an automatic geotiff generation, but "
									"no recognized psconvert option was used for the PS creation.\n");
						}
					}
					else if (Ctrl->W.kml) {
						GMT_Report (API, GMT_MSG_NORMAL, "Error: To GE images must be in geographical coordinates. Very likely "
									"this won't work as you wish inside GE.\n");
					}
				}
			}

			if (!strncmp (line, "%%BoundingBox:", 14)) {
				if (got_BB && !Ctrl->A.round)
					fprintf (fpo, "%%%%BoundingBox: 0 0 %ld %ld\n", lrint (ceil(w)), lrint (ceil(h)));
				else if (got_BB && Ctrl->A.round)		/* Go against Adobe Law and round HRBB instead of ceil */
					fprintf (fpo, "%%%%BoundingBox: 0 0 %ld %ld\n", lrint (w), lrint (h));

				got_BB = false;
				if (file_has_HRBB)
					continue;	/* High-res BB will be put elsewhere */
				if (got_HRBB)
					fprintf (fpo, "%%%%HiResBoundingBox: 0 0 %g %g\n", w, h);
				got_HRBB = false;
				continue;
			}
			else if (!strncmp (line, "%%HiResBoundingBox:", 19)) {
				if (got_HRBB)
					fprintf (fpo, "%%%%HiResBoundingBox: 0 0 %g %g\n", w, h);
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
				if (Ctrl->T.eps == -1)	/* Write out /PageSize command */
					fprintf (fpo, "<< /PageSize [%g %g] >> setpagedevice\n", w, h);
				if (r != 0)
					fprintf (fpo, "%d rotate\n", r);
				if (!GMT_IS_ZERO (xt) || !GMT_IS_ZERO (yt))
					fprintf (fpo, "%g %g translate\n", xt, yt);
				xt = yt = 0.0;
				r = 0;
			}
			else if (Ctrl->A.resize) {
				/* We are going to trick ghostscript to do what -dEPSFitPage was supposed to do but doesn't
				   because it's bugged. For that we recompute a new scale, offsets and DPIs such that at the
				   end we will end up with an image with the imposed size and the current -E dpi setting.
				*/
				double new_scale_x, new_scale_y, new_off_x, new_off_y, r_x, r_y;
				char t1[8], t2[8];	/* To hold the translate part when landscape */
				if (!strncmp (line, "%%BeginPageSetup", 16)) {
					size_t Lsize = 128U;
					char dumb1[8], dumb2[8], dumb3[8];
					char *line_ = GMT_memory (GMT, NULL, Lsize, char);
					BeginPageSetup_here = true;             /* Signal that on next line the job must be done */
					line_reader (GMT, &line_, &Lsize, fp);   /* Read also next line which is to be overwritten */
					/* The trouble is that we can have things like "V 612 0 T 90 R 0.06 0.06 scale" or "V 0.06 0.06 scale" */
					if (landscape_orig)
						sscanf(line_, "%s %s %s %s %s %s %s %s",c1, t1, t2, dumb1, dumb2, dumb3, c2, c3);
					else
						sscanf(line_, "%s %s %s",c1, c2,c3);
					old_scale_x = atof (c2);		old_scale_y = atof (c3);
					GMT_free (GMT, line_);
				}
				else if (BeginPageSetup_here) {
					BeginPageSetup_here = false;
					Ctrl->A.resize = false;       /* Need to reset so it doesn't keep checking inside this branch */
					/* Now we must calculate the new scale */
					if (Ctrl->A.rescale)          /* except if it was set as an option */
						r_x = Ctrl->A.scale;
					else
						r_x = Ctrl->A.new_size[0] / w;

					new_scale_x = new_scale_y = old_scale_x * r_x;
					new_off_x = -xt_bak + xt_bak * r_x;     /* Need to recompute the new offsets as well */
					new_off_y = -yt_bak + yt_bak * r_x;
					Ctrl->A.new_dpi_x = Ctrl->A.new_dpi_y = Ctrl->E.dpi * r_x;
					if (Ctrl->A.new_size[1]) {
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
						fprintf (fpo, "V %g %g T 90 R %g %g scale\n", atof(t1)*r_x, atof(t2)*r_y, new_scale_x, new_scale_y);
					}
					else
						fprintf (fpo, "V %g %g scale\n", new_scale_x, new_scale_y);
				}
			}
			else if (!strncmp (line, "%%Page:", 7)) {
				if (r != 0)
					fprintf (fpo, "%d rotate\n", r);
				if (!GMT_IS_ZERO (xt) || !GMT_IS_ZERO (yt))
					fprintf (fpo, "%g %g translate\n", xt, yt);
				xt = yt = 0.0;
				r = 0;
			}
#ifdef HAVE_GDAL
			else if (found_proj && !strncmp (line, "%%PageTrailer", 13)) {
				line_reader (GMT, &line, &line_size, fp);
				fprintf (fpo, "%%%%PageTrailer\n");
				fprintf (fpo, "%s\n", line);

				/* Write a GeoPDF registration info */

				/* Allocate new control structures */
				to_gdalread = GMT_memory (GMT, NULL, 1, struct GDALREAD_CTRL);
				from_gdalread = GMT_memory (GMT, NULL, 1, struct GD_CTRL);
				to_gdalread->W.active = true;
				from_gdalread->ProjectionRefPROJ4 = proj4_cmd;
				GMT_gdalread (GMT, NULL, to_gdalread, from_gdalread);
				if (from_gdalread->ProjectionRefWKT != NULL) {
					double x0, y0, x1;	/* Projected coordinates */
					double h0, v0, h1;	/* Correspnding point coordinates */
					double a, H, V;		/* a -> coeff of affine matrix; H,V -> origin shift in projected coords */
					double pX[4], pY[4], lptsX[4], lptsY[4];

					x0 = west;		y0 = south;		x1 = east;	y1 = north;
					h0 = v0 = 0;	/* because we used -A option so origin in points is at (0,0) */
					h1 = h0 + w;
					/* takes the projected coordinate system into the page coordinate system */
					a = (h1 - h0)/(x1 - x0);
					H = h0 - a*x0;
					V = v0 - a*y0;

					/* Use the above matrix to discover where the corners of the map will fall */
					pX[0] = a*x0 + H;	pX[1] = a*x0 + H;	pX[2] = a*x1 + H;	pX[3] = a*x1 + H;
					pY[0] = a*y0 + V;	pY[1] = a*y1 + V;	pY[2] = a*y1 + V;	pY[3] = a*y0 + V;

					/* Now compute the LPTS array */
					lptsX[0] = pX[0] / w;	lptsX[1] = pX[1] / w;	lptsX[2] = pX[2] / w;	lptsX[3] = pX[3] / w;
					lptsY[0] = pY[0] / h;	lptsY[1] = pY[1] / h;	lptsY[2] = pY[2] / h;	lptsY[3] = pY[3] / h;

					fprintf (fpo, "\n%% embed georegistation info\n");
					fprintf (fpo, "[ {ThisPage} <<\n");
					fprintf (fpo, "\t/VP [ <<\n");
					fprintf (fpo, "\t\t/Type /Viewport\n");
					fprintf (fpo, "\t\t/BBox[0 0 %.1f %.1f]\n", w, h);
					fprintf (fpo, "\t\t/Measure <<\n");
					fprintf (fpo, "\t\t\t/Type /Measure\n");
					fprintf (fpo, "\t\t\t/Subtype /GEO\n");
					fprintf (fpo, "\t\t\t/Bounds[0 0 0 1 1 1 1 0]\n");
					fprintf (fpo, "\t\t\t/GPTS[%f %f %f %f %f %f %f %f]\n",
						south, west, north, west, north, east, south, east);
					fprintf (fpo, "\t\t\t/LPTS[%f %f %f %f %f %f %f %f]\n",
						lptsX[0],lptsY[0], lptsX[1],lptsY[1], lptsX[2],lptsY[2], lptsX[3],lptsY[3]);
					fprintf (fpo, "\t\t\t/GCS <<\n");
					fprintf (fpo, "\t\t\t\t/Type /PROJCS\n");
					fprintf (fpo, "\t\t\t\t/WKT\n");
					fprintf (fpo, "\t\t\t\t(%s)\n", from_gdalread->ProjectionRefWKT);
					fprintf (fpo, "\t\t\t>>\n");
					fprintf (fpo, "\t\t>>\n");
					fprintf (fpo, "\t>>]\n");
					fprintf (fpo, ">> /PUT pdfmark\n\n");
				}
				GMT_free (GMT, to_gdalread);
				GMT_free (GMT, from_gdalread);
				continue;
			}
#endif
			fprintf (fpo, "%s\n", line);
		}

		/* Recede a bit to test the contents of last line. -7 for when PS has CRLF endings */
		fseek (fp, (off_t)-7, SEEK_END);
		/* Read until last line is encountered */
		while (line_reader (GMT, &line, &line_size, fp) != EOF);
		if (strncmp (line, "%%EOF", 5U))
			/* Possibly a non-closed GMT PS file. To be confirmed later */
			excessK = true;

		fclose (fpo);
		fclose (fp);

		/* Build the converting ghostscript command and execute it */

		if (Ctrl->T.device != GS_DEV_EPS) {
			char tag[16];
			int dest_device = Ctrl->T.device;	/* Keep copy in case of temp change below */

			strncpy (tag, &ext[Ctrl->T.device][1], 16U);
			GMT_str_toupper (tag);

			if (transparency) {
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "PS file with transparency must be converted to PDF before creating %s\n", tag);
				/* Temporarily change output device to PDF to get the PDF tmp file */
				Ctrl->T.device = GS_DEV_PDF;
				/* After conversion, convert the tmp PDF file to desired format via a 2nd gs call */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Convert to PDF...\n");
			}
			else
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Convert to %s...\n", tag);

			if (!Ctrl->F.active) {
				if (Ctrl->D.active) sprintf (out_file, "%s/", Ctrl->D.dir);	/* Use specified output directory */
				strncat (out_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));
			}
			else
				strncpy (out_file, Ctrl->F.file, GMT_BUFSIZ);
			strcat (out_file, ext[Ctrl->T.device]);

			if (Ctrl->A.new_dpi_x) {	/* We have a resize request (was Ctrl->A.resize = true;) */
				pix_w = urint (ceil (w * Ctrl->A.new_dpi_x / 72.0));
				pix_h = urint (ceil (h * Ctrl->A.new_dpi_y / 72.0));
			}
			else {
				pix_w = urint (ceil (w * Ctrl->E.dpi / 72.0));
				pix_h = urint (ceil (h * Ctrl->E.dpi / 72.0));
			}

			sprintf (cmd, "%s%s %s %s%s -sDEVICE=%s %s -g%dx%d -r%d -sOutputFile=%c%s%c -f%c%s%c",
				at_sign, Ctrl->G.file, gs_params, Ctrl->C.arg, alpha_bits(Ctrl), device[Ctrl->T.device],
				device_options[Ctrl->T.device],
				pix_w, pix_h, Ctrl->E.dpi, quote, out_file, quote, quote, tmp_file, quote);

			if (Ctrl->S.active)	/* Print GhostScript command */
				GMT_Report (API, GMT_MSG_NORMAL, "%s\n", cmd);

			/* Execute the GhostScript command */
			GMT_Report (API, GMT_MSG_DEBUG, "Running: %s\n", cmd);
			sys_retval = system (cmd);
			if (sys_retval) {
				GMT_Report (API, GMT_MSG_NORMAL, "System call [%s] returned error %d.\n", cmd, sys_retval);
				Return (EXIT_FAILURE);
			}

			/* Check output file */
			if (access (out_file, R_OK)) {
				/* output file not created */
				if (isGMT_PS && excessK)
					/* non-closed GMT input PS file */
					GMT_Report (API, GMT_MSG_NORMAL, "%s: GMT PS format detected but file is not finalized. Maybe a -K in excess? No output created.\n", ps_file);
				else
					/* Either a bad closed GMT PS file or one of unknown origin */
					GMT_Report (API, GMT_MSG_NORMAL, "Could not create %s. Maybe input file does not fulfill PS specifications.\n", out_file);
			}
			else {
				/* output file exists */
				if (isGMT_PS && excessK)
					/* non-closed GMT input PS file */
					GMT_Report (API, GMT_MSG_NORMAL, "%s: GMT PS format detected but file is not finalized. Maybe a -K in excess? %s could be messed up.\n", ps_file, out_file);
				/* else: Either a good closed GMT PS file or one of unknown origin */
			}
			if (transparency) {	/* Now convert PDF to desired format */
				char pdf_file[GMT_BUFSIZ] = {""};
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Convert PDF with transparency to %s...\n", tag);
				Ctrl->T.device = dest_device;	/* Reset output device type */
				strcpy (pdf_file, out_file);	/* Now the PDF is the infile */
				*out_file = '\0'; /* truncate string to build new output file */
				if (!Ctrl->F.active) {
					if (Ctrl->D.active) sprintf (out_file, "%s/", Ctrl->D.dir);	/* Use specified output directory */
					strncat (out_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));
				}
				else
					strncpy (out_file, Ctrl->F.file, GMT_BUFSIZ);
				strcat (out_file, ext[Ctrl->T.device]);
				/* After conversion, convert the tmp PDF file to desired format via a 2nd gs call */
				sprintf (cmd, "%s%s %s %s%s -sDEVICE=%s %s -r%d -sOutputFile=%c%s%c %c%s%c",
					at_sign, Ctrl->G.file, gs_params, Ctrl->C.arg, alpha_bits(Ctrl), device[Ctrl->T.device],
					device_options[Ctrl->T.device],
					Ctrl->E.dpi, quote, out_file, quote, quote, pdf_file, quote);
				if (Ctrl->S.active)	/* Print 2nd GhostScript command */
					GMT_Report (API, GMT_MSG_NORMAL, "%s\n", cmd);
				/* Execute the 2nd GhostScript command */
				GMT_Report (API, GMT_MSG_DEBUG, "Running: %s\n", cmd);
				sys_retval = system (cmd);
				if (sys_retval) {
					GMT_Report (API, GMT_MSG_NORMAL, "System call [%s] returned error %d.\n", cmd, sys_retval);
					Return (EXIT_FAILURE);
				}
				if (!Ctrl->S.active) remove (pdf_file);	/* The temporary PDF file is no longer needed */
			}

		}

		/* Remove input file, if requested */
		if (Ctrl->Z.active) {
			GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Removing %s...\n", ps_file);
			remove (ps_file);
		}

		GMT_Report (API, GMT_MSG_VERBOSE, "Done.\n");

		if (!Ctrl->S.active) {
			if (!Ctrl->T.eps)
				remove (tmp_file);
			if ( strlen (no_U_file) > 0 ) /* empty string == file was not created */
				remove (no_U_file);
			if ( strlen (clean_PS_file) > 0 )
				remove (clean_PS_file);
		}

		if (Ctrl->W.active && found_proj && !Ctrl->W.kml) {	/* Write a world file */
			double x_inc, y_inc;
			char world_file[GMT_BUFSIZ] = "", *wext = NULL, *s = NULL;

			x_inc = (east  - west)  / pix_w;
			y_inc = (north - south) / pix_h;
			GMT_Report (API, GMT_MSG_VERBOSE, "width = %d\theight = %d\tX res = %f\tY res = %f\n", pix_w, pix_h, x_inc, y_inc);

			/* West and North of the world file contain the coordinates of the
			 * center of the pixel
				 but our current values are of the NW corner of the pixel (pixel
				 registration). So we'll move halph pixel inward. */ west  += x_inc /
			2.0; north -= y_inc / 2.0;

			if (Ctrl->D.active) sprintf (world_file, "%s/", Ctrl->D.dir);	/* Use specified output directory */
			if (Ctrl->F.active) {		/* Must rip the raster file extension before adding the world one */
				for (i = (int)strlen(out_file) - 1; i > 0; i--) {
					if (out_file[i] == '.') { 	/* Beginning of file extension */
						pos_ext = i;
						break;
					}
				}
				out_file[pos_ext] = '\0';
				strcat(world_file, out_file);
			}
			else
				strncat (world_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));

			s = ext[Ctrl->T.device];
			wext = strdup(ext[Ctrl->T.device]);
			wext[1] = s[1];		wext[2] = s[3];		wext[3] = 'w';
			strcat (world_file, wext);

			if ((fpw = fopen (world_file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to open file %s for writing\n", world_file);
			}
			else {
				fprintf (fpw, "%.12f\n0.0\n0.0\n%.12f\n%.12f\n%.12f", x_inc, -y_inc, west, north);
				fclose (fpw);
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Wrote world file %s\n", world_file);
				if (proj4_cmd)
					GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Proj4 definition: %s\n", proj4_cmd);
			}

			free (wext);

			if (Ctrl->W.warp && proj4_cmd && proj4_cmd[1] == 'p') {	/* We got a usable Proj4 string. Run it (if gdal is around) */
				/* The true geotiff file will have the same base name plus a .tiff extension.
				   We will reuse the world_file variable because all it is need is to replace the extension */
				for (i = (int)strlen(world_file) - 1; i > 0; i--) {
					if (world_file[i] == '.') { 	/* Beginning of file extension */
						pos_ext = i;
						break;
					}
				}
				world_file[pos_ext] = '\0';
				strcat (world_file, ".tiff");

				if (GMT->current.setting.verbose < GMT_MSG_VERBOSE)	/* Shut up the gdal_translate (low level) verbosity */
					quiet = " -quiet";
				else
					quiet = "";

				sprintf (cmd, "gdal_translate -a_srs %c%s%c -co COMPRESS=LZW -co TILED=YES %s %c%s%c %c%s%c",
					quote, proj4_cmd, quote, quiet, quote, out_file, quote, quote, world_file, quote);
				free(proj4_cmd);
				GMT_Report (API, GMT_MSG_DEBUG, "Running: %s\n", cmd);
				sys_retval = system (cmd);		/* Execute the gdal_translate command */
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "The gdal_translate command: \n%s\n", cmd);
				if (sys_retval) {
					GMT_Report (API, GMT_MSG_NORMAL, "System call [%s] returned error %d.\n", cmd, sys_retval);
					Return (EXIT_FAILURE);
				}
			}
			else if (Ctrl->W.warp && !proj4_cmd)
				GMT_Report (API, GMT_MSG_NORMAL, "Could not find the Proj4 command in the PS file. No conversion performed.\n");
		}

		else if (Ctrl->W.kml) {	/* Write a basic kml file */
			char kml_file[GMT_BUFSIZ] = "";
			if (Ctrl->D.active)
				sprintf (kml_file, "%s/", Ctrl->D.dir);	/* Use specified output directory */
			if (Ctrl->F.active) {		/* Must rip the raster file extension before adding the kml one */
				for (i = (int)strlen(out_file) - 1; i > 0; i--) {
					if (out_file[i] == '.') { 	/* Beginning of file extension */
						pos_ext = i;
						break;
					}
				}
				out_file[pos_ext] = '\0';
				strcat (kml_file, out_file);
				out_file[pos_ext] = '.';	/* Reset the extension */
			}
			else
				strncat (kml_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));

			strcat (kml_file, ".kml");

			if ((fpw = fopen (kml_file, "w")) == NULL) {
				GMT_Report (API, GMT_MSG_NORMAL, "Unable to open file %s for writing\n", kml_file);
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
				fclose (fpw);
				GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Wrote KML file %s\n", kml_file);
			}
		}

		else if (Ctrl->W.active && !found_proj) {
			GMT_Report (API, GMT_MSG_NORMAL, "Could not find the 'PROJ' tag in the PS file. No world file created.\n");
			GMT_Report (API, GMT_MSG_NORMAL, "This situation occurs when one of the two next cases is true:\n");
			GMT_Report (API, GMT_MSG_NORMAL, "1) the PS file was created with a pre-GMT v4.5.0 version\n");
			GMT_Report (API, GMT_MSG_NORMAL, "2) the PS file was not created by GMT\n");
		}
	}

	for (k = 0; k < Ctrl->In.n_files; k++) free (ps_names[k]);
	GMT_free (GMT, ps_names);
	GMT_free (GMT, line);
	GMT_Report (API, GMT_MSG_DEBUG, "Final input buffer length was % "PRIuS "\n", line_size);

	Return (GMT_OK);
}

#ifdef WIN32
int ghostbuster(struct GMTAPI_CTRL *API, struct PS2RASTER_CTRL *C) {
	/* Search the Windows registry for the directory containing the gswinXXc.exe
	   We do this by finding the GS_DLL that is a value of the HKLM\SOFTWARE\GPL Ghostscript\X.XX\ key
	   Things are further complicated because Win64 has TWO registries: one 32 and the other 64 bits.
	   Add to this that the installed GS version may be 32 or 64 bits, so we have to check for the
	   four GS_32|64 + GMT_32|64 combinations.

		 Adapted from snipets at http://www.daniweb.com/software-development/c/code/217174
	   and http://juknull.wordpress.com/tag/regenumkeyex-example */

	HKEY hkey;              /* Handle to registry key */
	char data[GMT_BUFSIZ] = {""}, ver[8] = {""}, *ptr;
	char key[32] = "SOFTWARE\\GPL Ghostscript\\";
	unsigned long datalen = GMT_BUFSIZ;
	unsigned long datatype;
	long RegO, rc = 0;
	int n = 0;
	bool bits64 = true;
	float maxVersion = 0;		/* In case more than one GS, hold the number of the highest version */

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
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Error opening HKLM key\n");
		return (EXIT_FAILURE);
	}

	while (rc != ERROR_NO_MORE_ITEMS) {
		rc  = RegEnumKeyEx (hkey, n++, data, &datalen, 0, NULL, NULL, NULL);
		datalen = GMT_BUFSIZ; /* reset to buffer length (including terminating \0) */
		if (rc == ERROR_SUCCESS)
			maxVersion = MAX(maxVersion, strtof(data, NULL));	/* If more than one GS, keep highest version number */
	}

	RegCloseKey(hkey);

	if (maxVersion == 0) {
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Unknown version reported in registry\n");
		return (EXIT_FAILURE);
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
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Error opening HKLM key\n");
		return (EXIT_FAILURE);
	}

	/* Read the value for "GS_DLL" via the handle 'hkey' */
	RegO = RegQueryValueEx(hkey, "GS_DLL", NULL, &datatype, (LPBYTE)data, &datalen);

	RegCloseKey(hkey);

	if (RegO != ERROR_SUCCESS) {
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "Error reading the GS_DLL value contents\n");
		return (EXIT_FAILURE);
	}

	if ( (ptr = strstr(data,"\\gsdll")) == NULL ) {
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "GS_DLL value is screwed.\n");
		return (EXIT_FAILURE);
	}

	/* Truncate string and add affix gswinXXc.exe */
	*ptr = '\0';
	strcat(data, bits64 ? "\\gswin64c.exe" : "\\gswin32c.exe");

	/* Now finally check that the gswinXXc.exe exists */
	if (access (data, R_OK)) {
		GMT_Report (API, GMT_MSG_LONG_VERBOSE, "gswinXXc.exe does not exist.\n");
		return (EXIT_FAILURE);
	}

	/* Wrap the path in double quotes to prevent troubles raised by dumb things like "Program Files" */
	C->G.file = malloc (strlen (data) + 3);	/* strlen + 2 * " + \0 */
	sprintf (C->G.file, "\"%s\"", data);

	return (GMT_OK);
}

#endif		/* WIN32 */
