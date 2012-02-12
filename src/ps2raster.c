/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2012 by P. Wessel, W. H. F. Smith, R. Scharroo, and J. Luis
 *	See LICENSE.TXT file for copying and redistribution conditions.
 *
 *	This program is free software; you can redistribute it and/or modify
 *	it under the terms of the GNU General Public License as published by
 *	the Free Software Foundation; version 2 or any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *	Contact info: gmt.soest.hawaii.edu
 *--------------------------------------------------------------------*/
/*
 *
 * Brief synopsis: ps2raster converts one or several PostScript file(s) to other formats using GhostScript.
 * It works by modifying the page size in order that the image will have a size
 * which is specified by the BoundingBox.
 * As an option, a tight BoundingBox may be computed.
 * ps2raster uses the ideas of the EPS2XXX.m from Primoz Cermelj published in MatLab Central
 * and of psbbox.sh of Remko Scharroo.
 *
 *
 *--------------------------------------------------------------------*/
/*
 * Authors:	Joaquim Luis and Remko Scharroo
 * Date:	1-JAN-2010
 * Version:	5 API
 */

#define GMT_WITH_NO_PS
#include "gmt.h"

EXTERN_MSC void GMT_str_toupper (char *string);

#ifdef WIN32	/* Special for Windows */
#	include <windows.h>
#	include <process.h>
#	define getpid _getpid
	GMT_LONG ghostbuster(struct GMT_CTRL *GMT, struct PS2RASTER_CTRL *C);
#endif

#define N_GS_DEVICES		12	/* Number of supported GS output devices */
#define GS_DEV_EPS		0
#define GS_DEV_PDF		1
#define GS_DEV_JPG		2
#define GS_DEV_PNG		3
#define GS_DEV_PPM		4
#define GS_DEV_TIF		5
#define GS_DEV_BMP		6
#define GS_DEV_TPNG		7	/* PNG with transparency */
#define GS_DEV_JPGG		8	/* These are grayscale versions */
#define GS_DEV_PNGG		9
#define GS_DEV_TIFG		10
#define GS_DEV_BMPG		11

#define KML_GROUND_ABS		0
#define KML_GROUND_REL		1
#define KML_ABS				2
#define KML_SEAFLOOR_REL	3
#define KML_SEAFLOOR_ABS	4

#define add_to_list(list,item) { if (list[0]) strcat (list, " "); strcat (list, item); }

struct PS2RASTER_CTRL {
	struct In {	/* Input file info */
		GMT_LONG n_files;
	} In;
	struct A {	/* -A[u][-] [Adjust boundingbox] */
		GMT_LONG active;
		GMT_LONG strip;	/* Remove the -U time-stamp */
		GMT_LONG reset;	/* The -A- turns -A off, overriding any automode in effect */
		double margin[4];
	} A;
	struct C {	/* -C<option> */
		GMT_LONG active;
		char arg[GMT_BUFSIZ];
	} C;
	struct D {	/* -D<dir> */
		GMT_LONG active;
		char *dir;
	} D;
	struct E {	/* -E<resolution> */
		GMT_LONG active;
		GMT_LONG dpi;
	} E;
	struct F {	/* -F<out_name> */
		GMT_LONG active;
		char *file;
	} F;
	struct G {	/* -G<GSpath> */
		GMT_LONG active;
		char *file;
	} G;
	struct L {	/* -L<listfile> */
		GMT_LONG active;
		char *file;
	} L;
	struct P2 {	/* -P */
		GMT_LONG active;
	} P;
	struct Q {	/* -Q[g|t]<bits> */
		GMT_LONG active;
		GMT_LONG on[2];	/* [0] for graphics, [1] for text antialiasing */
		GMT_LONG bits[2];
	} Q;
	struct S {	/* -S */
		GMT_LONG active;
	} S;
	struct T {	/* -T */
		GMT_LONG active;
		GMT_LONG eps;	/* 1 if we want to make EPS, -1 with /PageSize (possibly in addition to another format) */
		GMT_LONG device;
	} T;
	struct W {	/* -W -- for world file production */
		GMT_LONG active;
		GMT_LONG folder;
		GMT_LONG warp;
		GMT_LONG kml;
		GMT_LONG mode;	/* 0 = clamp at ground, 1 is relative to ground, 2 is absolute 3 is relative to seafloor, 4 is clamp at seafloor */
		GMT_LONG min_lod, max_lod;	/* minLodPixels and maxLodPixels settings */
		GMT_LONG min_fade, max_fade;	/* minFadeExtent and maxFadeExtent settings */
		char *doctitle;		/* Name of KML document */
		char *overlayname;	/* Name of the image overlay */
		char *URL;		/* URL of remote site */
		char *foldername;	/* Name of KML folder */
		double altitude;
	} W;
};

GMT_LONG parse_GE_settings (struct GMT_CTRL *GMT, char *arg, struct PS2RASTER_CTRL *C)
{
	/* Syntax: -W[+g][+k][+t<doctitle>][+n<layername>][+a<altmode>][+l<lodmin>/<lodmax>] */
	
	GMT_LONG error = FALSE;
	GMT_LONG pos = 0;
	char txt[GMT_BUFSIZ], p[GMT_BUFSIZ];
	
	C->W.active = TRUE;
	strcpy (txt, arg);
	while (!error && (GMT_strtok (GMT, txt, "+", &pos, p))) {
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
						GMT_report (GMT, GMT_MSG_FATAL, "GMT ERROR -W+a<mode>[par]: Unrecognized altitude mode %c\n", p[1]);
						error++;
						break;
				}
				break;
			case 'f':	/* Set fading options in KML */
				sscanf (&p[1], "%" GMT_LL "d/%" GMT_LL "d", &C->W.min_fade, &C->W.max_fade);
				break;
			case 'g':	/* Use gdal to make geotiff */
				C->W.warp = TRUE;
				break;
			case 'k':	/* Produce a KML file */
				C->W.kml = TRUE;
				break;
			case 'l':	/* Set KML level of detail for image */
				sscanf (&p[1], "%" GMT_LL "d/%" GMT_LL "d", &C->W.min_lod, &C->W.max_lod);
				break;
			case 'n':	/* Set KML document layer name */
				if (C->W.overlayname) free (C->W.overlayname);	/* Already set, free then reset */
				C->W.overlayname = strdup (&p[1]);
				break;
			case 'o':	/* Produce a KML overlay as a folder subset */
				C->W.folder = TRUE;
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
				GMT_report (GMT, GMT_MSG_FATAL, "GMT ERROR -W+<opt>: Unrecognized option selection %c\n", p[1]);
				error++;
				break;
		}
	}
	return (error);
}

void *New_ps2raster_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct PS2RASTER_CTRL *C;

	C = GMT_memory (GMT, NULL, 1, struct PS2RASTER_CTRL);

	/* Initialize values whose defaults are not 0/FALSE/NULL */
#ifdef WIN32
	if ( ghostbuster(GMT, C) != GMT_OK ) { /* Try first to find the gspath from registry */
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

void Free_ps2raster_Ctrl (struct GMT_CTRL *GMT, struct PS2RASTER_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	if (C->D.dir) free (C->D.dir);
	if (C->F.file) free (C->F.file);
	if (C->G.file) free (C->G.file);
	if (C->L.file) free (C->L.file);
	free (C->W.doctitle);
	free (C->W.overlayname);
	free (C->W.foldername);
	if (C->W.URL) free (C->W.URL);
	GMT_free (GMT, C);
}

GMT_LONG GMT_ps2raster_usage (struct GMTAPI_CTRL *C, GMT_LONG level)
{
	struct GMT_CTRL *GMT = C->GMT;

	GMT_message (GMT, "ps2raster %s [API] - Convert [E]PS file(s) to other formats using GhostScript.\n\n", GMT_VERSION);
	GMT_message (GMT, "usage: ps2raster <psfile1> <psfile2> <...> [-A[u][-][<margin(s)>]] [-C<gs_command>]\n");
	GMT_message (GMT, "\t[-D<dir>] [-E<resolution>] [-F<out_name>] [-G<gs_path>] [-L<listfile>]\n");
	GMT_message (GMT, "\t[-N] [-P] [-Q[g|t]1|2|4] [-S] [-Tb|e|f|F|g|G|j|m|p|t] [%s]\n", GMT_V_OPT);
	GMT_message (GMT, "\t[-W[+a<mode>[<alt]][+f<minfade>/<maxfade>][+g][+k][+l<lodmin>/<lodmax>][+n<name>][+o<folder>][+t<title>][+u<URL>]]\n\n");

	if (level == GMTAPI_SYNOPSIS) return (EXIT_FAILURE);

	GMT_message (GMT, "Works by modifying the page size in order that the resulting\n");
	GMT_message (GMT, "image will have the size specified by the BoundingBox.\n");
	GMT_message (GMT, "As an option, a tight BoundingBox may be computed.\n\n");
	GMT_message (GMT, "<psfile(s)> postscript file(s) to be converted.\n");
	GMT_message (GMT, "\n\tOPTIONS:\n");
	GMT_message (GMT, "\t-A Adjust the BoundingBox to the minimum required by the image contents.\n");
	GMT_message (GMT, "\t   Append u to strip out time-stamps (produced by GMT -U options).\n");
	GMT_message (GMT, "\t   Append - to make sure -A is NOT activated by -W.\n");
	GMT_message (GMT, "\t   Optionally, append margin(s) to adjusted BoundingBox.\n");
	GMT_message (GMT, "\t     -A<off>[u] sets uniform margin for all 4 sides.\n");
	GMT_message (GMT, "\t     -A<xoff>[u]/<yoff>[u] set separate x- and y-margins.\n");
	GMT_message (GMT, "\t     -A<woff>[u]/<eoff>[u]/<soff>[u]/<noff>[u] set separate w-,e-,s-,n-margins.\n");
	GMT_message (GMT, "\t   Append unit u (%s) [%c].\n", GMT_DIM_UNITS_DISPLAY, GMT->session.unit_name[GMT->current.setting.proj_length_unit][0]);
	GMT_message (GMT, "\t-C Specify a single, custom option that will be passed on to GhostScript\n");
	GMT_message (GMT, "\t   as is. Repeat to add several options [none].\n");
	GMT_message (GMT, "\t-D Set an alternative output directory (which must exist)\n");
	GMT_message (GMT, "\t   [Default is same directory as PS files].\n");
	GMT_message (GMT, "\t   Use -D. to place the output in the current directory.\n");
	GMT_message (GMT, "\t-E Set raster resolution in dpi [default = 720 for PDF, 300 for others].\n");
	GMT_message (GMT, "\t-F Force the output file name. By default output names are constructed\n");
	GMT_message (GMT, "\t   using the input names as base, which are appended with an appropriate\n");
	GMT_message (GMT, "\t   extension. Use this option to provide a different name, but WITHOUT\n");
	GMT_message (GMT, "\t   extension. Extension is still determined automatically.\n");
	GMT_message (GMT, "\t-G Full path to your ghostscript executable.\n");
	GMT_message (GMT, "\t   NOTE: Under Unix systems this is generally not necessary.\n");
	GMT_message (GMT, "\t   Under Windows, ghostscript path is fished from the registry.\n");
	GMT_message (GMT, "\t   If this fails you can still add the GS path to system's path\n");
	GMT_message (GMT, "\t   or give the full path here.\n");
	GMT_message (GMT, "\t   (e.g. -Gc:\\programs\\gs\\gs9.02\\bin\\gswin64c).\n");
	GMT_message (GMT, "\t-L The <listfile> is an ASCII file with names of files to be converted.\n");
	GMT_message (GMT, "\t-P Force Portrait mode. All Landscape mode plots will be rotated back\n");
	GMT_message (GMT, "\t   so that they show unrotated in Portrait mode.\n");
	GMT_message (GMT, "\t   This is practical when converting to image formats or preparing\n");
	GMT_message (GMT, "\t   EPS or PDF plots for inclusion in documents.\n");
	GMT_message (GMT, "\t-Q Anti-aliasing setting for (g)raphics or (t)ext; append size (1,2,4)\n");
	GMT_message (GMT, "\t   of sub-sampling box.\n");
	GMT_message (GMT, "\t   Default is no anti-aliasing, which is the same as specifying size 1.\n");
	GMT_message (GMT, "\t-S Apart from executing it, also writes the ghostscript command to\n");
	GMT_message (GMT, "\t   standard output.\n");
	GMT_message (GMT, "\t-T Set output format [default is jpeg]:\n");
	GMT_message (GMT, "\t   b means BMP.\n");
	GMT_message (GMT, "\t   e means EPS.\n");
	GMT_message (GMT, "\t   E means EPS with /PageSize command.\n");
	GMT_message (GMT, "\t   f means PDF.\n");
	GMT_message (GMT, "\t   F means multi-page PDF (requires -F).\n");
	GMT_message (GMT, "\t   g means PNG.\n");
	GMT_message (GMT, "\t   G means PNG (with transparency).\n");
	GMT_message (GMT, "\t   j means JPEG.\n");
	GMT_message (GMT, "\t   m means PPM.\n");
	GMT_message (GMT, "\t   t means TIF.\n");
	GMT_message (GMT, "\t   For b, g, j, t, append - to get a grayscale image [24-bit color].\n");
	GMT_message (GMT, "\t   The EPS format can be combined with any of the other formats.\n");
	GMT_message (GMT, "\t   For example, -Tef creates both an EPS and PDF file.\n");
	GMT_message (GMT, "\t-V Provide progress report [default is silent] and shows the\n");
	GMT_message (GMT, "\t   gdal_translate command, in case you want to use this program\n");
	GMT_message (GMT, "\t   to create a geoTIFF file.\n");
	GMT_message (GMT, "\t-W Write a ESRI type world file suitable to make (e.g.) .tif files be\n");
	GMT_message (GMT, "\t   recognized as geotiff by softwares that know how to do it. Be aware,\n");
	GMT_message (GMT, "\t   however, that different results are obtained depending on the image\n");
	GMT_message (GMT, "\t   contents and if the -B option has been used or not. The trouble with\n");
	GMT_message (GMT, "\t   -B is that it creates a frame and very likely its annotations and\n");
	GMT_message (GMT, "\t   that introduces pixels outside the map data extent. As a consequence,\n");
	GMT_message (GMT, "\t   the map extents estimation will be wrong. To avoid this problem, use\n");
	GMT_message (GMT, "\t   the --MAP_FRAME_TYPE=inside option which plots all annotations related\n");
	GMT_message (GMT, "\t   stuff inside the image and does not compromise the coordinate.\n");
	GMT_message (GMT, "\t   computations. The world file naming follows the convention of jamming\n");
	GMT_message (GMT, "\t   a 'w' in the file extension. So, if the output is tif (-Tt) the world\n");
	GMT_message (GMT, "\t   file is a .tfw, for jpeg a .jgw, and so on.\n");
	GMT_message (GMT, "\t   Use -W+g to do a system call to gdal_translate and produce a true\n");
	GMT_message (GMT, "\t   geoTIFF image right away. The output file will have the extension\n");
	GMT_message (GMT, "\t   .tiff. See the man page for other 'gotchas'. Automatically sets -A -P.\n");
	GMT_message (GMT, "\t   Use -W+k to create a minimalist KML file that allows loading the\n");
	GMT_message (GMT, "\t   image in Google Earth. Note that for this option the image must be\n");
	GMT_message (GMT, "\t   in geographical coordinates. If not, a warning is issued but the\n");
	GMT_message (GMT, "\t   KML file is created anyway.\n");
	GMT_message (GMT, "\t   Several modifiers allow you to specify the content in the KML file:\n");
	GMT_message (GMT, "\t   +a<altmode>[<altitude>] sets the altitude mode of this layer, where\n");
	GMT_message (GMT, "\t      <altmode> is one of 5 recognized by Google Earth:\n");
	GMT_message (GMT, "\t      G clamped to the ground [Default].\n");
	GMT_message (GMT, "\t      g Append altitude (in m) relative to ground.\n");
	GMT_message (GMT, "\t      A Append absolute altitude (in m).\n");
	GMT_message (GMT, "\t      s Append altitude (in m) relative to seafloor.\n");
	GMT_message (GMT, "\t      S clamped to the seafloor.\n");
	GMT_message (GMT, "\t   +f<minfade>/<maxfade>] sets distances over which we fade from opaque\n");
	GMT_message (GMT, "\t     to transparent [no fading].\n");
	GMT_message (GMT, "\t   +l<minLOD>/<maxLOD>] sets Level Of Detail when layer should be\n");
	GMT_message (GMT, "\t     active [always active]. Image goes inactive when there are fewer\n");
	GMT_message (GMT, "\t     than minLOD pixels or more than maxLOD pixels visible.\n");
	GMT_message (GMT, "\t     -1 means never invisible.\n");
	GMT_message (GMT, "\t   +n<layername> sets the name of this particular layer\n");
	GMT_message (GMT, "\t     [\"GMT Image Overlay\"].\n");
	GMT_message (GMT, "\t   +o<foldername> sets the name of this particular folder\n");
	GMT_message (GMT, "\t     [\"GMT Image Folder\"].  This yields a KML snipped without header/trailer.\n");
	GMT_message (GMT, "\t   +t<doctitle> sets the document name [\"GMT KML Document\"].\n");
	GMT_message (GMT, "\t   +u<URL> prepands this URL to the name of the image referenced in the\n");
	GMT_message (GMT, "\t     KML [local file].\n");

	return (EXIT_FAILURE);
}

GMT_LONG GMT_ps2raster_parse (struct GMTAPI_CTRL *C, struct PS2RASTER_CTRL *Ctrl, struct GMT_OPTION *options)
{
	/* This parses the options provided to ps2raster and sets parameters in CTRL.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	GMT_LONG j, k, n_errors = 0, mode, grayscale;
	char text[GMT_BUFSIZ], txt_a[GMT_TEXT_LEN64], txt_b[GMT_TEXT_LEN64], txt_c[GMT_TEXT_LEN64], txt_d[GMT_TEXT_LEN64], *anti = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMT_CTRL *GMT = C->GMT;

	for (opt = options; opt; opt = opt->next) {
		switch (opt->option) {

			case '<':	/* Input files */
				Ctrl->In.n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Adjust BoundingBox: -A[u][<margins>] or -A- */
				Ctrl->A.active = TRUE;
				k = 0;
				if (opt->arg[k] == 'u') {Ctrl->A.strip = TRUE; k++;}
				if (opt->arg[strlen(opt->arg)-1] == '-')
					Ctrl->A.reset = TRUE;
				else if (opt->arg[k]) {	/* Also specified margin(s) */
					j = sscanf (&opt->arg[k], "%[^/]/%[^/]/%[^/]/%s", txt_a, txt_b, txt_c, txt_d);
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
							n_errors++;
							GMT_report (GMT, GMT_MSG_FATAL, "-A: Give 1, 2, or 4 margins\n");
							break;
					}
				}
				break;
			case 'C':	/* Append extra custom GS options */
				add_to_list (Ctrl->C.arg, opt->arg);	/* Append to list of extra GS options */
				break;
			case 'D':	/* Change output directory */
				Ctrl->D.active = TRUE;
				Ctrl->D.dir = strdup (opt->arg);
				break;
			case 'E':	/* Set output dpi */
				Ctrl->E.active = TRUE;
				Ctrl->E.dpi = atoi (opt->arg);
				break;
			case 'F':	/* Set explicitly the output file name */
				Ctrl->F.active = TRUE;
				Ctrl->F.file = strdup (opt->arg);
				GMT_chop_ext (GMT, Ctrl->F.file);	/* Make sure file name has no extension */
				break;
			case 'G':	/* Set GS path */
				Ctrl->G.active = TRUE;
				free (Ctrl->G.file);
				Ctrl->G.file = strdup (opt->arg);
				break;
			case 'L':	/* Give list of files to convert */
				Ctrl->L.active = TRUE;
				Ctrl->L.file = strdup (opt->arg);
				break;
			case 'P':	/* Force Portrait mode */
				Ctrl->P.active = TRUE;
				break;
			case 'Q':	/* Anti-aliasing settings */
				Ctrl->Q.active = TRUE;
				if (opt->arg[0] == 'g') {
					mode = 0;
					anti = "-dGraphicsAlphaBits=";
				}
				else if (opt->arg[0] == 't') {
					mode = 1;
					anti = "-dTextAlphaBits=";
				}
				else {
					GMT_default_error (GMT, opt->option);
					n_errors++;
					continue;

				}
				Ctrl->Q.on[mode] = TRUE;
				Ctrl->Q.bits[mode] = (opt->arg[1]) ? atoi (&opt->arg[1]) : 4;
				sprintf (text, "%s%ld", anti, Ctrl->Q.bits[mode]);
				add_to_list (Ctrl->C.arg, text);	/* Append to list of extra GS options */
				break;
			case 'S':	/* Write the GS command to STDOUT */
				Ctrl->S.active = TRUE;
				break;
			case 'T':	/* Select output format (optionally also request EPS) */
				Ctrl->T.active = TRUE;
				grayscale = ((j = (GMT_LONG)strlen(opt->arg)) > 3 && opt->arg[j-1] == '-');
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
							add_to_list (Ctrl->C.arg, "-dMaxBitmap=100000000");	/* Add this as GS option to fix bug in GS */
							break;
						case 'm':	/* PPM */
							Ctrl->T.device = GS_DEV_PPM;
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

			default:	/* Report bad options */
				n_errors += GMT_default_error (GMT, opt->option);
				break;
		}
	}

	if (!Ctrl->T.active) Ctrl->T.device = GS_DEV_JPG;	/* Default output device if none is specified */

	n_errors += GMT_check_condition (GMT, Ctrl->Q.on[0] && (Ctrl->Q.bits[0] < 1 || Ctrl->Q.bits[0] > 4),
		"Syntax error: Anti-aliasing for graphics requires sub-samplib box of 1,2, or 4\n");

	n_errors += GMT_check_condition (GMT, Ctrl->Q.on[1] && (Ctrl->Q.bits[1] < 1 || Ctrl->Q.bits[1] > 4),
		"Syntax error: Anti-aliasing for text requires sub-samplib box of 1,2, or 4\n");

	n_errors += GMT_check_condition (GMT, Ctrl->In.n_files > 1 && Ctrl->L.active,
		"Syntax error: Cannot handle both a file list and multiple ps files in input\n");

	n_errors += GMT_check_condition (GMT, Ctrl->L.active && access (Ctrl->L.file, R_OK),
		"Error: Cannot read list file %s\n", Ctrl->L.file);

	n_errors += GMT_check_condition (GMT, Ctrl->T.device == -GS_DEV_PDF && !Ctrl->F.active,
		"Syntax error: Creation of Multipage PDF requires setting -F option\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_OK);
}

#define bailout(code) {GMT_Free_Options (mode); return (code);}
#define Return(code) {Free_ps2raster_Ctrl (GMT, Ctrl); GMT_end_module (GMT, GMT_cpy); bailout (code);}

GMT_LONG GMT_ps2raster (struct GMTAPI_CTRL *API, GMT_LONG mode, void *args)
{
	GMT_LONG error = FALSE, found_proj = FALSE, setup, i_unused = 0;
	GMT_LONG isGMT_PS = FALSE, excessK;
	GMT_LONG i, j, k, len, r, pos_file, pos_ext, pix_w = 0, pix_h = 0;
	GMT_LONG got_BB, got_HRBB, got_BBatend, file_has_HRBB, got_end, landscape;

	double xt, yt, w, h, x0 = 0.0, x1 = 612.0, y0 = 0.0, y1 = 828.0;
	double west = 0.0, east = 0.0, south = 0.0, north = 0.0;

	size_t n_alloc = GMT_SMALL_CHUNK;

	char **ps_names = NULL;
	char ps_file[GMT_BUFSIZ] = "", no_U_file[GMT_BUFSIZ] = "",
			 clean_PS_file[GMT_BUFSIZ] = "", tmp_file[GMT_BUFSIZ] = "",
	     out_file[GMT_BUFSIZ] = "", BB_file[GMT_BUFSIZ] = "";
	char line[GMT_BUFSIZ], c1[20], c2[20], c3[20], c4[20],
	     cmd[GMT_BUFSIZ], proj4_name[20], *quiet = NULL;
	char *gs_params = NULL, *gs_BB = NULL, *proj4_cmd = NULL;
	char *device[N_GS_DEVICES] = {"", "pdfwrite", "jpeg", "png16m", "ppmraw", "tiff24nc", "bmp16m", "pngalpha", "jpeggray", "pnggray", "tiffgray", "bmpgray"};
	char *ext[N_GS_DEVICES] = {".eps", ".pdf", ".jpg", ".png", ".ppm", ".tif", ".bmp", ".png", ".jpg", ".png", ".tif", ".bmp"};
	char *RefLevel[5] = {"clampToGround", "relativeToGround", "absolute", "relativeToSeaFloor", "clampToSeaFloor"};
#ifdef WIN32
	char at_sign[2] = "@";
#else
	char at_sign[2] = "";
#endif

#ifdef USE_GDAL
	struct GDALREAD_CTRL *to_gdalread = NULL;
	struct GD_CTRL *from_gdalread = NULL;
#endif

	FILE *fp = NULL, *fpo = NULL, *fpb = NULL, *fpl = NULL, *fp2 = NULL, *fpw = NULL;

	struct GMT_OPTION *opt = NULL;
	struct PS2RASTER_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;
	struct GMT_OPTION *options = NULL;

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_Report_Error (API, GMT_NOT_A_SESSION));
	options = GMT_Prep_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMTAPI_OPT_USAGE) bailout (GMT_ps2raster_usage (API, GMTAPI_USAGE));/* Return the usage message */
	if (options->option == GMTAPI_OPT_SYNOPSIS) bailout (GMT_ps2raster_usage (API, GMTAPI_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = GMT_begin_module (API, "GMT_ps2raster", &GMT_cpy);	/* Save current state */
	if (GMT_Parse_Common (API, "-V", "", options)) Return (API->error);
	Ctrl = New_ps2raster_Ctrl (GMT);	/* Allocate and initialize a new control structure */
	if ((error = GMT_ps2raster_parse (API, Ctrl, options))) Return (error);

	/*---------------------------- This is the ps2raster main code ----------------------------*/

	if (Ctrl->F.active && (Ctrl->L.active || Ctrl->D.active)) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: Option -F and options -L OR -D are mutually exclusive. Ignoring option -F.\n");
		Ctrl->F.active = FALSE;
	}

	if (Ctrl->F.active && Ctrl->In.n_files > 1 && Ctrl->T.device != -GS_DEV_PDF) {
		GMT_report (GMT, GMT_MSG_FATAL, "Warning: Option -F is incompatible with multiple inputs. Ignoring option -F.\n");
		Ctrl->F.active = FALSE;
	}

	/* Parameters for all the formats available */

	gs_params = "-q -dSAFER -dNOPAUSE -dBATCH -dUseFlateCompression=true -dPDFSETTINGS=/prepress -dEmbedAllFonts=true -dSubsetFonts=true -dMonoImageFilter=/FlateEncode -dAutoFilterGrayImages=false -dGrayImageFilter=/FlateEncode -dAutoFilterColorImages=false -dColorImageFilter=/FlateEncode";
	gs_BB = "-q -dSAFER -dNOPAUSE -dBATCH -sDEVICE=bbox"; /* -r defaults to 4000, see http://pages.cs.wisc.edu/~ghost/doc/cvs/Devices.htm#Test */

	if (Ctrl->W.kml && !(Ctrl->T.device == GS_DEV_JPG ||
	    Ctrl->T.device == GS_DEV_JPGG || Ctrl->T.device == GS_DEV_TIF ||
	    Ctrl->T.device == GS_DEV_TIFG || Ctrl->T.device == GS_DEV_PNG ||
	    Ctrl->T.device == GS_DEV_TPNG || Ctrl->T.device == GS_DEV_PNGG) ) {
		GMT_report (GMT, GMT_MSG_FATAL, "Error: As far as we know selected raster type is unsuported by GE.\n");
	}

	if (Ctrl->W.active) {	/* Implies -P and -A (unless -A- is set ) */
		Ctrl->P.active = Ctrl->A.active = TRUE;
		if (Ctrl->A.reset) Ctrl->A.active = FALSE;
	}

	/* Use default DPI if not already set */
	if (Ctrl->E.dpi <= 0) Ctrl->E.dpi = (Ctrl->T.device == GS_DEV_PDF) ? 720 : 300;

	/* Multiple files in a file with their names */
	if (Ctrl->L.active) {
		if ((fpl = fopen (Ctrl->L.file, "r")) == NULL) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: Cannot to open list file %s\n", Ctrl->L.file);
			Return (EXIT_FAILURE);
		}
		ps_names = GMT_memory (GMT, NULL, n_alloc, char *);
		while (GMT_fgets_chop (GMT, line, GMT_BUFSIZ, fpl) != NULL) {
			if (!*line || *line == '#') /* Empty line or comment */
				continue;
			ps_names[Ctrl->In.n_files++] = strdup (line);
			if (Ctrl->In.n_files > (GMT_LONG)n_alloc) {
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

	/* --------------------------------------------------------------------------------------------- */
	/* ------    If a multi-page PDF file creation is requested, do it and exit.   ------------------*/
	/* --------------------------------------------------------------------------------------------- */
	if (Ctrl->T.active && Ctrl->T.device == -GS_DEV_PDF) {
		char *all_names_in = NULL, *cmd2 = NULL;

		for (k = n_alloc = 0; k < Ctrl->In.n_files; k++) n_alloc += (strlen (ps_names[k]) + 1);
		all_names_in = GMT_memory (GMT, NULL, n_alloc, char);
		for (k = 0; k < Ctrl->In.n_files; k++) {
			add_to_list (all_names_in, ps_names[k]);
			free (ps_names[k]);
		}
		cmd2 = GMT_memory (GMT, NULL, n_alloc + GMT_BUFSIZ, char);
		sprintf (cmd2, "%s%s -q -dNOPAUSE -dBATCH -sDEVICE=pdfwrite %s -r%ld -sOutputFile=%s.pdf %s",
			at_sign, Ctrl->G.file, Ctrl->C.arg, Ctrl->E.dpi, Ctrl->F.file, all_names_in);

		system (cmd2);		/* Execute the GhostScript command */
		if (Ctrl->S.active) fprintf (stdout, "%s\n", cmd2);

		GMT_free (GMT, all_names_in);
		GMT_free (GMT, cmd2);
		GMT_free (GMT, ps_names);
		Return (GMT_OK);
	}
	/* ----------------------------------------------------------------------------------------------- */

	/* Loop over all input files */

	for (k = 0; k < Ctrl->In.n_files; k++) {
		excessK = FALSE;
		*out_file = '\0'; /* truncate string */
		strcpy (ps_file, ps_names[k]);
		if ((fp = fopen (ps_file, "r")) == NULL) {
			GMT_report (GMT, GMT_MSG_FATAL, "Cannot to open file %s\n", ps_file);
			continue;
		}

		GMT_report (GMT, GMT_MSG_NORMAL, "Processing %s...", ps_file);

		if (Ctrl->A.strip) {	/* Must strip off the GMT timestamp stuff */
			GMT_LONG dump = TRUE;
			GMT_report (GMT, GMT_MSG_VERBOSE, " Strip GMT time-stamp...");
			sprintf (no_U_file, "%s/ps2raster_%db.eps", Ctrl->D.dir, (int)getpid());
			if ((fp2 = fopen (no_U_file, "w+")) == NULL) {
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to create a temporary file\n");
				Return (EXIT_FAILURE);
			}
			while (GMT_fgets_chop (GMT, line, GMT_BUFSIZ, fp) != NULL) {
				if (dump && !strncmp (line, "% Begin GMT time-stamp", 22))
					dump = FALSE;
				if (dump)
					fprintf (fp2, "%s\n", line);
				if (!dump && !strncmp (line, "% End GMT time-stamp", 20))
					dump = TRUE;
			}
			fclose (fp);	/* Close original PS file */
			rewind (fp2);	/* Rewind new file without timestamp */
			fp = fp2;	/* Set original file pointer to this file instead */
		}

		got_BB = got_HRBB = file_has_HRBB = got_BBatend = got_end = landscape = setup = FALSE;

		len = (GMT_LONG)strlen(ps_file);
		j = len - 1;
		pos_file = -1;
		pos_ext = -1;	/* In case file has no extension */
		for (i = 0; i < len; i++, j--) {
			if (pos_ext < 0 && ps_file[j] == '.') pos_ext = j;	/* Beginning of file extension */
			if (pos_file < 0 && (ps_file[j] == '/' || ps_file[j] == '\\')) pos_file = j + 1;	/* Beginning of file name */
		}
		if (pos_ext == -1) pos_ext = len - 1;	/* File has no extension */
		if (!Ctrl->D.active || pos_file == -1) pos_file = 0;	/* File either has no leading directory or we want to use it */

		/* Adjust to a tight BoundingBox if user requested so */

		if (Ctrl->A.active) {
			char *psfile_to_use;
			GMT_report (GMT, GMT_MSG_VERBOSE, " Find HiResBoundingBox ");
			sprintf (BB_file, "%s/ps2raster_%ldc.bb", Ctrl->D.dir, (GMT_LONG)getpid());
			psfile_to_use = Ctrl->A.strip ? no_U_file : ((strlen (clean_PS_file) > 0) ? clean_PS_file : ps_file);
			sprintf (cmd, "%s%s %s %s %s 2> %s", at_sign, Ctrl->G.file, gs_BB, Ctrl->C.arg, psfile_to_use, BB_file);
			i_unused = system (cmd);		/* Execute the command that computes the tight BB */
			if ((fpb = fopen (BB_file, "r")) == NULL) {
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to open file %s\n", BB_file);
				Return (EXIT_FAILURE);
			}
			while (GMT_fgets (GMT, line, GMT_BUFSIZ, fpb) != NULL && !got_BB) {
				/* We only use the High resolution BB */
				if ((strstr (line,"%%HiResBoundingBox:"))) {
					sscanf (&line[19], "%s %s %s %s", c1, c2, c3, c4);
					x0 = atof (c1);		y0 = atof (c2);
					x1 = atof (c3);		y1 = atof (c4);
					x0 -= Ctrl->A.margin[XLO];	x1 += Ctrl->A.margin[XHI];	/* If not given, margin = 0/0/0/0 */
					y0 -= Ctrl->A.margin[YLO];	y1 += Ctrl->A.margin[YHI];
					if (x1 <= x0 || y1 <= y0) {
						GMT_report (GMT, GMT_MSG_FATAL, "Unable to decode BoundingBox file %s\n", BB_file);
						fclose (fpb);
						remove (BB_file);	/* Remove the file */
						sprintf (tmp_file, "%s/", Ctrl->D.dir);
						strncat (tmp_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));
						strcat (tmp_file, ext[Ctrl->T.device]);
						sprintf (cmd, "%s%s %s %s -sDEVICE=%s -g1x1 -r%ld -sOutputFile=%s -f%s", 
							at_sign, Ctrl->G.file, gs_params, Ctrl->C.arg, device[Ctrl->T.device],
							Ctrl->E.dpi, tmp_file, ps_file);
						i_unused = system (cmd);		/* Execute the GhostScript command */
						if (Ctrl->S.active) fprintf (stdout, "%s\n", cmd);

						continue;
					}
					got_BB = got_HRBB = TRUE;
				}
			}
			fclose (fpb);
			remove (BB_file);	/* Remove the file with BB info */
			if (got_BB) GMT_report (GMT, GMT_MSG_VERBOSE, "[%g %g %g %g]...", x0, y0, x1, y1);
		}

		/* Open temporary file to be processed by ghostscript. When -Te is used, tmp_file is for keeps */

		if (Ctrl->T.eps)
			GMT_report (GMT, GMT_MSG_VERBOSE, " Format EPS file...");
		if (Ctrl->T.eps) {
			if (Ctrl->D.active) sprintf (tmp_file, "%s/", Ctrl->D.dir);	/* Use specified output directory */
			if (!Ctrl->F.active)
				strncat (tmp_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));
			else
				strcat (tmp_file, Ctrl->F.file);
			strcat (tmp_file, ext[GS_DEV_EPS]);
			if ((fpo = fopen (tmp_file, "w")) == NULL) {
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to open file %s for writing\n", tmp_file);
				continue;
			}
		}
		else {
			sprintf (tmp_file, "%s/ps2raster_%ldd.eps", Ctrl->D.dir, (GMT_LONG)getpid());
			if ((fpo = fopen (tmp_file, "w+")) == NULL) {
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to create a temporary file\n");
				continue;
			}
		}

		/* Scan first 20 lines of input file for [HiRes]BoundingBox and Orientation statements.
		 * Since we prefer the HiResBB over BB we must continue to read until both are found or 20 lines have past */

		i = 0;
		while ((GMT_fgets (GMT, line, GMT_BUFSIZ, fp) != NULL) && i < 20 && !(got_BB && got_HRBB && got_end)) {
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
					got_BB = TRUE;
				}
				else
					got_BBatend++;
			}
			else if ((strstr (line, "%%HiResBoundingBox:"))) {
				file_has_HRBB = TRUE;
				if (!got_HRBB) {
					sscanf (&line[19], "%s %s %s %s",c1,c2,c3,c4);
					if (strncmp (c1, "(atend)", 7)) {	/* Got actual numbers */
						x0 = atof (c1);		y0 = atof (c2);
						x1 = atof (c3);		y1 = atof (c4);
						got_HRBB = got_BB = TRUE;
					}
				}
			}
			else if ((strstr (line, "%%Creator:"))) {
				if (!strncmp (&line[11], "GMT", 3))
					isGMT_PS = TRUE;
			}
			else if ((strstr (line, "%%Orientation:"))) {
				if (!strncmp (&line[15], "Landscape", 9))
					landscape = TRUE;
			}
			else if ((strstr (line, "%%EndComments")))
				got_end = TRUE;
			if (got_BBatend == 1 && (got_end || i == 19)) {	/* Now is the time to look at the end of the file */
				got_BBatend++;			/* Avoid jumping more than once to the end */
				if (!fseek (fp, (long)-256, SEEK_END)) i = -30;
			}
		}

		/* Cannot proceed without knowing the BoundingBox */

		if (!got_BB) {
			GMT_report (GMT, GMT_MSG_FATAL, "Error: The file %s has no BoundingBox in the first 20 lines or last 256 bytes. Use -A option.\n", ps_file);
			continue;
		}

		/* Do the math on the BoundingBox and translation coordinates */

		if (Ctrl->P.active && landscape)
			xt = -x1, yt = -y0, w = y1-y0, h = x1-x0, r = -90;
		else
			xt = -x0, yt = -y0, w = x1-x0, h = y1-y0, r = 0;

		/* Rewind the input file and start copying and replacing */

		rewind (fp);
		while (GMT_fgets_chop (GMT, line, GMT_BUFSIZ, fp) != NULL) {
			if (line[0] != '%') {	/* Copy any non-comment line, except one containing /PageSize in the Setup block */
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
					found_proj = TRUE;
					if ((ptmp = strstr (&line[2], "+proj")) != NULL) {  /* Search for the +proj in the comment line */
						proj4_cmd = strdup (&line[(int)(ptmp - &line[0])]);
						GMT_chop (GMT, proj4_cmd);		/* Remove the new line char */
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
							GMT_report (GMT, GMT_MSG_FATAL, "Warning: An unknown ps2raster setting was found but since "
									"image coordinates seem to be geographical, a linear transformation "
									"will be used.\n");
						}
						else if (!strcmp (proj4_name,"xy") && Ctrl->W.warp) {	/* Do not operate on a twice unknown setting */
							GMT_report (GMT, GMT_MSG_FATAL, "Error: requested an automatic geotiff generation, but "
									"no recognized ps2raster option was used for the PS creation.\n"); 
						}
					}
					else if (Ctrl->W.kml) {
						GMT_report (GMT, GMT_MSG_FATAL, "Error: To GE images must be in geographical coords. Very likely "
									"this won't work as you wish inside GE.\n"); 
					}
				}
			}

			if (!strncmp (line, "%%BoundingBox:", 14)) {
				if (got_BB)
					fprintf (fpo, "%%%%BoundingBox: 0 0 %ld %ld\n", (GMT_LONG)ceil(w), (GMT_LONG)ceil(h));
				got_BB = FALSE;
				if (file_has_HRBB)
					continue;	/* High-res BB will be put elsewhere */
				if (got_HRBB)
					fprintf (fpo, "%%%%HiResBoundingBox: 0 0 %g %g\n", w, h);
				got_HRBB = FALSE;
				continue;
			}
			else if (!strncmp (line, "%%HiResBoundingBox:", 19)) {
				if (got_HRBB)
					fprintf (fpo, "%%%%HiResBoundingBox: 0 0 %g %g\n", w, h);
				got_HRBB = FALSE;
				continue;
			}
			else if (Ctrl->P.active && landscape && !strncmp (line, "%%Orientation:", 14)) {
				fprintf (fpo, "%%%%Orientation: Portrait\n");
				landscape = FALSE;
				continue;
			}
			else if (!strncmp (line, "%%BeginSetup", 12))
				setup = TRUE;
			else if (!strncmp (line, "%%EndSetup", 10)) {
				setup = FALSE;
				if (Ctrl->T.eps != 1)	/* Write out /PageSize command */
					fprintf (fpo, "<< /PageSize [%g %g] >> setpagedevice\n", w, h);
				if (r != 0)
					fprintf (fpo, "%ld rotate\n", r);
				if (!GMT_IS_ZERO (xt) || !GMT_IS_ZERO (yt))
					fprintf (fpo, "%g %g translate\n", xt, yt);
				xt = yt = 0.0;
				r = 0;
			}
			else if (!strncmp (line, "%%Page:", 7)) {
				if (r != 0)
					fprintf (fpo, "%ld rotate\n", r);
				if (!GMT_IS_ZERO (xt) || !GMT_IS_ZERO (yt))
					fprintf (fpo, "%g %g translate\n", xt, yt);
				xt = yt = 0.0;
				r = 0;
			}
#ifdef USE_GDAL
			else if (!strncmp (line, "%%PageTrailer", 13) && found_proj) {
				GMT_fgets_chop (GMT, line, GMT_BUFSIZ, fp);
				fprintf (fpo, "%%%%PageTrailer\n");
				fprintf (fpo, "%s\n", line);

				/* Write a GeoPDF registration info */ 

				/* Allocate new control structures */
				to_gdalread = GMT_memory (GMT, NULL, 1, struct GDALREAD_CTRL);
				from_gdalread = GMT_memory (GMT, NULL, 1, struct GD_CTRL);
				to_gdalread->W.active = TRUE;
				from_gdalread->ProjectionRefPROJ4 = proj4_cmd;
				GMT_gdalread (GMT, NULL, to_gdalread, from_gdalread);
				if (from_gdalread->ProjectionRefWKT != CNULL) {
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

		/* Receed a bit to test the contents of last line. -7 for when
		 * PS has CRLF endings */
		fseek (fp, -7, SEEK_END);
		/* Read until last line is encountered */
		while ( GMT_fgets (GMT, line, BUFSIZ, fp) );
		if ( strncmp (line, "%%EOF", 5) )
			/* Possibly a non-closed GMT PS file. To be confirmed later */
			excessK = TRUE;

		fclose (fpo);
		fclose (fp);

		/* Build the converting ghostscript command and execute it */

		if (Ctrl->T.device != GS_DEV_EPS) {
			char tag[16];
			strcpy (tag, &ext[Ctrl->T.device][1]);
			GMT_str_toupper (tag);
			GMT_report (GMT, GMT_MSG_VERBOSE, " Convert to %s...", tag);

			if (!Ctrl->F.active) {
				if (Ctrl->D.active) sprintf (out_file, "%s/", Ctrl->D.dir);		/* Use specified output directory */
				strncat (out_file, &ps_file[pos_file], (size_t)(pos_ext - pos_file));
			}
			else
				strcpy (out_file, Ctrl->F.file);
			strcat (out_file, ext[Ctrl->T.device]);
			pix_w = (GMT_LONG)ceil (w * Ctrl->E.dpi / 72.0);
			pix_h = (GMT_LONG)ceil (h * Ctrl->E.dpi / 72.0);
			sprintf (cmd, "%s%s %s %s -sDEVICE=%s -g%ldx%ld -r%ld -sOutputFile=%s -f%s",
				at_sign, Ctrl->G.file, gs_params, Ctrl->C.arg, device[Ctrl->T.device],
				pix_w, pix_h, Ctrl->E.dpi, out_file, tmp_file);

			if (Ctrl->S.active)
				/* Print GhostScript command */
				fprintf (stdout, "%s\n", cmd);

			/* Execute the GhostScript command */
			i_unused = system (cmd);

			/* Check output file */
			if (access (out_file, R_OK)) {
				/* output file not created */
				if (isGMT_PS && excessK)
					/* non-closed GMT input PS file */
					GMT_report (GMT, GMT_MSG_FATAL, "%s: GMT PS format detected but file is not finalized. Maybe a -K in excess? No output created.\n", ps_file);
				else
					/* Either a bad closed GMT PS file or one of unknown origin */
					GMT_report (GMT, GMT_MSG_FATAL, "Could not create %s. Maybe input file does not fulfill PS specifications.\n", out_file);
			}
			else {
				/* output file exists */
				if (isGMT_PS && excessK)
					/* non-closed GMT input PS file */
					GMT_report (GMT, GMT_MSG_FATAL, "%s: GMT PS format detected but file is not finalized. Maybe a -K in excess? %s could be messed up.\n", ps_file, out_file);
				/* else: Either a good closed GMT PS file or one of unknown origin */
			}

		}
		GMT_report (GMT, GMT_MSG_NORMAL, " Done.\n");

		if (!Ctrl->T.eps)
			remove (tmp_file);
		if ( strlen (no_U_file) > 0 ) /* empty string == file was not created */
			remove (no_U_file);
		if ( strlen (clean_PS_file) > 0 )
			remove (clean_PS_file);

		if (Ctrl->W.active && found_proj && !Ctrl->W.kml) {	/* Write a world file */
			double x_inc, y_inc;
			char world_file[GMT_BUFSIZ] = "", *wext = NULL, *s = NULL;

			x_inc = (east  - west)  / pix_w;
			y_inc = (north - south) / pix_h;
			GMT_report (GMT, GMT_MSG_NORMAL, "width = %ld\theight = %ld\tX res = %f\tY res = %f\n", pix_w, pix_h, x_inc, y_inc);

			/* West and North of the world file contain the coordinates of the
			 * center of the pixel
				 but our current values are of the NW corner of the pixel (pixel
				 registration). So we'll move halph pixel inward. */ west  += x_inc /
			2.0; north -= y_inc / 2.0;

			if (Ctrl->D.active) sprintf (world_file, "%s/", Ctrl->D.dir);	/* Use
																																			 specified
																																			 output
																																			 directory
																																			 */
			if (Ctrl->F.active) {		/* Must rip the raster file extension before adding the world one */
				for (i = (GMT_LONG)strlen(out_file) - 1; i > 0; i--) {
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
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to open file %s for writing\n", world_file);
			}
			else {
				fprintf (fpw, "%.12f\n0.0\n0.0\n%.12f\n%.12f\n%.12f", x_inc, -y_inc, west, north);
				fclose (fpw);
				GMT_report (GMT, GMT_MSG_VERBOSE, "Wrote world file %s\n", world_file);
				if (proj4_cmd)
					GMT_report (GMT, GMT_MSG_VERBOSE, "Proj4 definition: %s\n", proj4_cmd);
			}

			free (wext);

			if (Ctrl->W.warp && proj4_cmd && proj4_cmd[1] == 'p') {	/* We got a usable Proj4 string. Run it (if gdal is around) */
				/* The true geotiff file will have the same base name plus a .tiff extension.
				   We will reuse the world_file variable because all it is need is to replace the extension */
				for (i = (GMT_LONG)strlen(world_file) - 1; i > 0; i--) {
					if (world_file[i] == '.') { 	/* Beginning of file extension */
						pos_ext = i;
						break;
					}
				}
				world_file[pos_ext] = '\0';
				strcat (world_file, ".tiff");

				if (GMT->current.setting.verbose < GMT_MSG_NORMAL)	/* Shut up the gdal_translate (low level) verbosity */
					quiet = " -quiet";
				else
					quiet = "";

#ifdef WIN32
				sprintf (cmd, "gdal_translate -a_srs \"%s\" -co COMPRESS=LZW -co TILED=YES %s %s %s", 
						proj4_cmd, quiet, out_file, world_file); 
#else
				sprintf (cmd, "gdal_translate -a_srs '%s' -co COMPRESS=LZW -co TILED=YES %s %s %s", 
						proj4_cmd, quiet, out_file, world_file); 
#endif
				free(proj4_cmd);
				i_unused = system (cmd);		/* Execute the gdal_translate command */
				GMT_report (GMT, GMT_MSG_VERBOSE, "\nThe gdal_translate command: \n%s\n", cmd);
			}
			else if (Ctrl->W.warp && !proj4_cmd)
				GMT_report (GMT, GMT_MSG_FATAL, "Could not find the Proj4 command in the PS file. No conversion performed.\n");
		}

		else if ( Ctrl->W.kml ) {	/* Write a basic kml file */
			char kml_file[GMT_BUFSIZ] = "";
			if (Ctrl->D.active)
				sprintf (kml_file, "%s/", Ctrl->D.dir);	/* Use specified output directory */
			if (Ctrl->F.active) {		/* Must rip the raster file extension before adding the kml one */
				for (i = (GMT_LONG)strlen(out_file) - 1; i > 0; i--) {
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
				GMT_report (GMT, GMT_MSG_FATAL, "Unable to open file %s for writing\n", kml_file);
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
 					fprintf (fpw, "\t\t\t<minLodPixels>%ld</minLodPixels>\n", Ctrl->W.min_lod);
 					fprintf (fpw, "\t\t\t<maxLodPixels>%ld</maxLodPixels>\n", Ctrl->W.max_lod);
					if (Ctrl->W.min_fade) fprintf (fpw, "\t\t\t<minFadeExtent>%ld</minFadeExtent>\n", Ctrl->W.min_fade);
	 				if (Ctrl->W.max_fade) fprintf (fpw, "\t\t\t<maxFadeExtent>%ld</maxFadeExtent>\n", Ctrl->W.max_fade);
 					fprintf (fpw, "\t\t</Lod>\n");
				}
				fprintf (fpw, "\t\t</Region>\n");
				fprintf (fpw, "\t</GroundOverlay>\n");
				if (Ctrl->W.folder)	/* Control KML overlay vs full file */
					fprintf (fpw, "</Folder>\n");
				else
					fprintf (fpw, "</Document>\n</kml>\n");
				fclose (fpw);
				GMT_report (GMT, GMT_MSG_VERBOSE, "Wrote KML file %s\n", kml_file);
			}
		}

		else if (Ctrl->W.active && !found_proj) {
			GMT_report (GMT, GMT_MSG_FATAL, "Could not find the 'PROJ' tag in the PS file. No world file created.\n");
			GMT_report (GMT, GMT_MSG_FATAL, "This situation occurs when one of the two next cases is true:\n");
			GMT_report (GMT, GMT_MSG_FATAL, "1) the PS file was created with a pre-GMT v4.5.0 version\n");
			GMT_report (GMT, GMT_MSG_FATAL, "2) the PS file was not created by GMT\n");
		}
	}

	for (k = 0; k < Ctrl->In.n_files; k++) free (ps_names[k]);
	GMT_free (GMT, ps_names);

	Return (GMT_OK);
}

#ifdef WIN32
GMT_LONG ghostbuster(struct GMT_CTRL *GMT, struct PS2RASTER_CTRL *C) {
	/* Search the Windows registry for the directory containing the gswinXXc.exe
	   We do this by finding the GS_DLL that is a value of the HKLM\SOFTWARE\GPL Ghostscript\X.XX\ key
	   Things are further complicated because Win64 has TWO registries: one 32 and the other 64 bits.
	   Add to this that the installed GS version may be 32 or 64 bits, so we have to check for the
	   four GS_32|64 + GMT_32|64 combinations.

		 Adapted from snipets at http://www.daniweb.com/software-development/c/code/217174
	   and http://juknull.wordpress.com/tag/regenumkeyex-example */

	HKEY hkey;              /* Handle to registry key */
	char data[GMT_BUFSIZ], ver[8], *ptr;
	char key[32] = "SOFTWARE\\GPL Ghostscript\\";
	unsigned long datalen = GMT_BUFSIZ;
	unsigned long datatype;
	long RegO, rc = 0;
	int n = 0;
	GMT_LONG bits64 = TRUE;
	float maxVersion = 0;		/* In case more than one GS, hold the number of the highest version */

#ifdef _WIN64
	RegO = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\GPL Ghostscript", 0, KEY_READ, &hkey);	/* Read 64 bits Reg */
	if (RegO != ERROR_SUCCESS) {		/* Try the 32 bits registry */
		RegO = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\GPL Ghostscript", 0, KEY_READ|KEY_WOW64_32KEY, &hkey);
		bits64 = FALSE;
	}
#else
	RegO = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\GPL Ghostscript", 0, KEY_READ, &hkey);	/* Read 32 bits Reg */
	if (RegO != ERROR_SUCCESS)			/* Failed. Try the 64 bits registry */
		RegO = RegOpenKeyEx(HKEY_LOCAL_MACHINE, "SOFTWARE\\GPL Ghostscript", 0, KEY_READ|KEY_WOW64_64KEY, &hkey);
	else {
		bits64 = FALSE;
	}
#endif

	if (RegO != ERROR_SUCCESS) {
		GMT_report (GMT, GMT_MSG_VERBOSE, "Error opening HKLM key\n");
		return (EXIT_FAILURE);
	}

	while (rc != ERROR_NO_MORE_ITEMS) {
		rc  = RegEnumKeyEx (hkey, n++, data, &datalen, 0, NULL, NULL, NULL);
		datalen = GMT_BUFSIZ; /* reset to buffer length (including terminating \0) */
		if (rc == ERROR_SUCCESS)
			maxVersion = (float)MAX(maxVersion, atof(data));	/* If more than one GS, keep highest version number */
	}

	RegCloseKey(hkey);

	if (maxVersion == 0) {
		GMT_report (GMT, GMT_MSG_VERBOSE, "Unknown version reported in registry\n");
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
		GMT_report (GMT, GMT_MSG_VERBOSE, "Error opening HKLM key\n");
		return (EXIT_FAILURE);
	}

	/* Read the value for "GS_DLL" via the handle 'hkey' */
	RegO = RegQueryValueEx(hkey, "GS_DLL", NULL, &datatype, (LPBYTE)data, &datalen);

	RegCloseKey(hkey);

	if (RegO != ERROR_SUCCESS) {
		GMT_report (GMT, GMT_MSG_VERBOSE, "Error reading the GS_DLL value contents\n");
		return (EXIT_FAILURE);
	}

	if ( (ptr = strstr(data,"\\gsdll")) == NULL ) {
		GMT_report (GMT, GMT_MSG_VERBOSE, "GS_DLL value is screwed.\n");
		return (EXIT_FAILURE);
	}

	/* Truncate string and add affix gswinXXc.exe */
	*ptr = '\0';
	strcat(data, bits64 ? "\\gswin64c.exe" : "\\gswin32c.exe");

 	/* Now finally check that the gswinXXc.exe exists */
	if (access (data, R_OK)) {
		GMT_report (GMT, GMT_MSG_VERBOSE, "gswinXXc.exe does not exist.\n");
		return (EXIT_FAILURE);
	}

	/* Wrap the path in double quotes to prevent troubles raised by dumb things like "Program Files" */
	datalen = (unsigned long)strlen (data);
	C->G.file = (char *)malloc (datalen + 3);	/* strlen + 2 * " + \0 */
	strcpy (C->G.file, "\"");
	strcat (C->G.file, data);
	strcat (C->G.file, "\"");

	return (GMT_OK);
}

#endif		/* WIN32 */
