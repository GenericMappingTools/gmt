/*--------------------------------------------------------------------
 *	$Id$
 *
 *	Copyright (c) 1991-2017 by P. Wessel, W. H. F. Smith, R. Scharroo, J. Luis and F. Wobbe
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
/*void *V_API, int mode
 * API functions to support the gmt2kml application.
 *
 * Author:	Paul Wessel
 * Date:	1-JAN-2010
 * Version:	5 API
 *
 * Brief synopsis: gmt2kml reads GMT tables and plots symbols, lines,
 * and polygons as KML files for Google Earth.
 */

#define THIS_MODULE_NAME	"gmt2kml"
#define THIS_MODULE_LIB		"core"
#define THIS_MODULE_PURPOSE	"Convert GMT data tables to KML files for Google Earth"
#define THIS_MODULE_KEYS	"<D{,>T},CC("

#include "gmt_dev.h"
#include <stdarg.h>

#define GMT_PROG_OPTIONS "-:>KOVabdfghi" GMT_OPT("HMm")

int gmt_parse_R_option (struct GMT_CTRL *GMT, char *item);
void gmt_get_rgb_lookup (struct GMT_CTRL *GMT, struct GMT_PALETTE *P, int index, double value, double *rgb);

#define POINT			0
#define EVENT			1
#define SPAN			2
#define LINE			3
#define POLYGON			4
#define WIGGLE			7
#define KML_GROUND		0
#define KML_GROUND_REL		1
#define KML_ABSOLUTE		2
#define KML_SEAFLOOR_REL	3
#define KML_SEAFLOOR		4
#define KML_DOCUMENT		0
#define KML_FOLDER		1

#define GET_COL_LABEL		1
#define GET_LABEL		2
#define NO_LABEL		3
#define FMT_LABEL		4

/* Need unsigned int BGR triplets */
#define GMT_3u255(t) gmt_M_u255(t[2]),gmt_M_u255(t[1]),gmt_M_u255(t[0])

#define F_ID	0	/* Indices into arrays */
#define N_ID	1
#define ALT	0
#define LOD	1
#define FADE	2

struct GMT2KML_CTRL {
	double t_transp;
	struct In {
		bool active;
		char *file;
	} In;
	struct A {	/* -A */
		bool active;
		bool get_alt;
		unsigned int mode;
		double scale;
		double altitude;
	} A;
	struct C {	/* -C<cpt> */
		bool active;
		char *file;
	} C;
	struct D {	/* -D<descriptfile> */
		bool active;
		char *file;
	} D;
	struct E {	/* -E */
		bool active;
	} E;
	struct F {	/* -F */
		bool active;
		unsigned int mode;
		unsigned int geometry;
	} F;
	struct G {	/* -G<fill> */
		bool active[2];
		struct GMT_FILL fill[2];
	} G;
	struct I {	/* -I<icon> */
		bool active;
		char *file;
	} I;
	struct L {	/* -L */
		bool active;
		unsigned int n_cols;
		char **name;
	} L;
	struct N {	/* -N */
		bool active;
		unsigned int mode;
		char *fmt;
	} N;
	struct Q {	/* -Qi|a<az> and -Qs<scale>[unit] */
		bool active;
		unsigned int mode, dmode;
		char unit;
		double value[2];
		double scale;
	} Q;
	struct R2 {	/* -R */
		bool active;
		bool automatic;
	} R2;
	struct S {	/* -S */
		bool active;
		double scale[2];
	} S;
	struct T {	/* -T */
		bool active;
		char *title;
		char *folder;
	} T;
	struct W {	/* -W<pen>[+c[l|f]] */
		bool active;
		struct GMT_PEN pen;
	} W;
	struct Z {	/* -Z */
		bool active;
		bool invisible;
		bool open;
		double min[3], max[3];
	} Z;
};

struct KML {
	double *lon, *lat, *z;	/* Points defining the data for the wiggle */
	double *flon, *flat;	/* The fake lon, lat of the wiggle on the Earth */
	uint64_t n_in;	/* Poinst making the wiggle data */
	uint64_t n_out;	/* Points in fake wiggle */
	uint64_t n_alloc;	/* Allocation size */
};

GMT_LOCAL void *New_Ctrl (struct GMT_CTRL *GMT) {	/* Allocate and initialize a new control structure */
	struct GMT2KML_CTRL *C;

	C = gmt_M_memory (GMT, NULL, 1, struct GMT2KML_CTRL);

	/* Initialize values whose defaults are not 0/false/NULL */

	C->A.mode = KML_GROUND;
	C->A.scale = 1.0;
	C->F.mode = POINT;
	C->F.geometry = GMT_IS_POINT;
	gmt_init_fill (GMT, &C->G.fill[F_ID], 1.0, 192.0 / 255.0, 128.0 / 255.0);	/* Default fill color */
	gmt_init_fill (GMT, &C->G.fill[N_ID], 1.0, 1.0, 1.0);				/* Default text color */
	C->G.fill[N_ID].rgb[3] = 0.25;	/* Default text transparency */
	C->I.file = strdup ("http://maps.google.com/mapfiles/kml/pal4/icon57.png");	/* Default icon */
	C->t_transp = 1.0;
	C->S.scale[N_ID] = C->S.scale[F_ID] = 1.0;
	C->Z.max[ALT] = -1.0;
	C->W.pen = GMT->current.setting.map_default_pen; C->W.pen.width = 1.0;		/* Default pen width = 1p */

	return (C);
}

GMT_LOCAL void Free_Ctrl (struct GMT_CTRL *GMT, struct GMT2KML_CTRL *C) {	/* Deallocate control structure */
	if (!C) return;
	gmt_M_str_free (C->In.file);
	gmt_M_str_free (C->C.file);
	gmt_M_str_free (C->D.file);
	gmt_M_str_free (C->I.file);
	gmt_M_str_free (C->N.fmt);
	gmt_M_str_free (C->T.title);
	gmt_M_str_free (C->T.folder);
	if (C->L.active) {
		unsigned int col;
		for (col = 0; col < C->L.n_cols; col++) gmt_M_str_free (C->L.name[col]);
		gmt_M_free (GMT, C->L.name);
	}
	gmt_M_free (GMT, C);
}

GMT_LOCAL int usage (struct GMTAPI_CTRL *API, int level) {
	/* This displays the gmt2kml synopsis and optionally full usage information */

	gmt_show_name_and_purpose (API, THIS_MODULE_LIB, THIS_MODULE_NAME, THIS_MODULE_PURPOSE);
	if (level == GMT_MODULE_PURPOSE) return (GMT_NOERROR);
	GMT_Message (API, GMT_TIME_NONE, "usage: gmt2kml [<table>] [-Aa|g|s[<altitude>|x<scale>]] [-C<cpt>] [-D<descriptfile>] [-E]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-Fe|s|t|l|p|w] [-Gf|n[-|<fill>] [-I<icon>] [-K] [-L<name1>,<name2>,...]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-N-|+|<template>|<name>] [-O] [-Q[a|i]<az>] [-Qs<scale>[unit]] [-Ra|<w>/<e>/<s>/n>] [-Sc|n<scale>]\n");
	GMT_Message (API, GMT_TIME_NONE, "\t[-T<title>[/<foldername>] [%s] [-W[<pen>][<attr>]] [-Z<opts>] [%s]\n", GMT_V_OPT, GMT_a_OPT);
	GMT_Message (API, GMT_TIME_NONE, "\t[%s] [%s] [%s] [%s]\n\t[%s] [%s]\n\t[%s]\n\n", GMT_bi_OPT, GMT_di_OPT, GMT_f_OPT, GMT_g_OPT, GMT_h_OPT, GMT_i_OPT, GMT_colon_OPT);

	if (level == GMT_SYNOPSIS) return (GMT_MODULE_SYNOPSIS);

	GMT_Message (API, GMT_TIME_NONE, "\n\tOPTIONS:\n");
	GMT_Option (API, "<");
	GMT_Message (API, GMT_TIME_NONE, "\t-A Altitude mode, choose among three modes:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     a Absolute altitude.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     g Altitude relative to sea surface or ground.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     s Altitude relative to seafloor or ground.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally, append fixed <altitude>, or x<scale> [g0: Clamped to sea surface or ground].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-C Append color palette name to color symbols by third column z-value.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-D File with HTML snippets to use for data description [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-E Extend feature down to the ground [no extrusion].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-F Feature type; choose from (e)vent, (s)ymbol, (t)imespan, (l)ine, (p)olygon, or (w)iggle [s].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   All features expect lon, lat in the first two columns.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Value or altitude is given in the third column (see -A and -C).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Event requires a timestamp in the next column.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Timespan requires begin and end ISO timestamps in the next two columns\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   (use NaN for unlimited begin and/or end times).\n");
	gmt_rgb_syntax (API->GMT, 'G', "Set color for symbol/polygon fill (-Gf<color>) or label (-Gn<color>).");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default polygon fill is lightorange with 75%% transparency.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Default text label color is white.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Gf- to turn off polygon fill.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Use -Gn- to turn off labels.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-I URL to an alternative icon used for the symbol [Google circle].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   If URL starts with + we will prepend http://maps.google.com/mapfiles/kml/.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Give -I- to not place any icons.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   [Default is a local icon with no directory path].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-K Allow for more KML code to be appended later [OFF].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-L Supply extended named data columns via <name1>,<name2>,... [none].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-N Control the feature labels.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   By default, -L\"label\" statements in the segment header are used. Alternatively,\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   1. Specify -N- if first non-coordinate column of data record is a single-word label (-Fe|s|t only).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   2. Specify -N+ if the rest of the data record should be used as label (-Fe|s|t only).\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   3. Append a string that may contain the format %%d for a running feature count.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   4. Give no argument to indicate no labels.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-O Append the KML code to an existing document [OFF].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Q Set properties in support of wiggle plots (-Fw):\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qa Set preferred azimuth +|-90 for wiggle direction [0], or\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qi Instead, set fixed azimuth for wiggle direction [variable].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -Qs Set wiggle scale in z-data units per map unit.  Append %s [e].\n", GMT_LEN_UNITS_DISPLAY);
	GMT_Message (API, GMT_TIME_NONE, "\t-R Issue Region tag.  Append w/e/s/n to set a particular region or append a to use the\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   actual domain of the data (single file only) [no region specified].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-S Scale for (c)ircle icon size or (n)ame label [1].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-T Append KML document title name.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   Optionally append /<foldername> to name folder when used with\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   -O and -K to organize features into groups.\n");
	GMT_Option (API, "V");
	gmt_pen_syntax (API->GMT, 'W', "Specify pen attributes for lines and polygon outlines [Default is %s].", 8);
	GMT_Message (API, GMT_TIME_NONE, "\t   Give width in pixels and append p.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t-Z Control visibility of features.  Append one or more modifiers:\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +a<alt_min>/<alt_max> inserts altitude limits [no limit].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +l<minLOD>/<maxLOD>] sets Level Of Detail when layer should be active [always active].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     layer goes inactive when there are fewer than minLOD pixels or more\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     than maxLOD pixels visible.  -1 means never invisible.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +f<minfade>/<maxfade>] sets distances over which we fade from opaque.\n");
	GMT_Message (API, GMT_TIME_NONE, "\t     to transparent [no fading].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +v turns off visibility [feature is visible].\n");
	GMT_Message (API, GMT_TIME_NONE, "\t   +o open document or folder when loaded [closed].\n");
	GMT_Option (API, "a,bi2,di,f,g,h,i,:,.");

	return (GMT_MODULE_USAGE);
}

GMT_LOCAL unsigned int parse_old_W (struct GMTAPI_CTRL *API, struct GMT2KML_CTRL *Ctrl, char *text) {
	unsigned int j = 0, n_errors = 0;
	if (text[j] == '-') {Ctrl->W.pen.cptmode = 1; j++;}
	if (text[j] == '+') {Ctrl->W.pen.cptmode = 3; j++;}
	if (text[j] && gmt_getpen (API->GMT, &text[j], &Ctrl->W.pen)) {
		gmt_pen_syntax (API->GMT, 'W', "sets pen attributes [Default pen is %s]:", 8);
		n_errors++;
	}
	return n_errors;
}

GMT_LOCAL int parse (struct GMT_CTRL *GMT, struct GMT2KML_CTRL *Ctrl, struct GMT_OPTION *options) {
	/* This parses the options provided to gmt2kml and sets parameters in Ctrl.
	 * Note Ctrl has already been initialized and non-zero default values set.
	 * Any GMT common options will override values set previously by other commands.
	 * It also replaces any file names specified as input or output with the data ID
	 * returned when registering these sources/destinations with the API.
	 */

	unsigned int n_errors = 0, pos = 0, k, n_files = 0;
	size_t n_alloc = 0;
	char buffer[GMT_LEN256] = {""}, p[GMT_LEN256] = {""}, T[4][GMT_LEN64], *c = NULL;
	struct GMT_OPTION *opt = NULL;
	struct GMTAPI_CTRL *API = GMT->parent;

	for (opt = options; opt; opt = opt->next) {	/* Process all the options given */

		switch (opt->option) {

			case '<':	/* Input files */
				if (!gmt_check_filearg (GMT, '<', opt->arg, GMT_IN, GMT_IS_TEXTSET))
					n_errors++;
				else if (n_files == 0)	/* Just keep name of first file */
					Ctrl->In.file = strdup (opt->arg);
				n_files++;
				break;

			/* Processes program-specific parameters */

			case 'A':	/* Altitude mode */
				Ctrl->A.active = true;
				switch (opt->arg[1]) {
					case 'x':
						Ctrl->A.scale = atof (&opt->arg[2]);
						Ctrl->A.get_alt = true;
						break;
					case '\0':
						Ctrl->A.get_alt = true;
						break;
					default:
						Ctrl->A.altitude = atof (&opt->arg[1]);
						break;
				}
				switch (opt->arg[0]) {
					case 'a':
						Ctrl->A.mode = KML_ABSOLUTE;
						break;
					case 'g':
						Ctrl->A.mode = (Ctrl->A.altitude != 0.0 || Ctrl->A.get_alt) ? KML_GROUND_REL : KML_GROUND;
						break;
					case 's':
						Ctrl->A.mode = (Ctrl->A.altitude != 0.0 || Ctrl->A.get_alt) ? KML_SEAFLOOR_REL : KML_SEAFLOOR;
						break;
					default:
						GMT_Message (API, GMT_TIME_NONE, "Bad altitude mode. Use a, g or s.\n");
						n_errors++;
						break;
				}
				break;
			case 'C':	/* Color table */
				Ctrl->C.active = true;
				gmt_M_str_free (Ctrl->C.file);
				if (opt->arg[0]) Ctrl->C.file = strdup (opt->arg);
				break;
			case 'D':	/* Description file */
				Ctrl->D.active = true;
				gmt_M_str_free (Ctrl->D.file);
				if (opt->arg[0]) Ctrl->D.file = strdup (opt->arg);
				break;
			case 'E':	/* Extrude feature down to the ground*/
			 	Ctrl->E.active = true;
				break;
			case 'F':	/* Feature type */
		 		Ctrl->F.active = true;
				switch (opt->arg[0]) {
					case 's':
						Ctrl->F.mode = POINT;
						break;
					case 'e':
						Ctrl->F.mode = EVENT;
						break;
					case 't':
						Ctrl->F.mode = SPAN;
						break;
					case 'l':
						Ctrl->F.mode = LINE;
						Ctrl->F.geometry = GMT_IS_LINE;
						break;
					case 'p':
						Ctrl->F.mode = POLYGON;
						Ctrl->F.geometry = GMT_IS_POLY;
						break;
					case 'w':
						Ctrl->F.mode = WIGGLE;
						Ctrl->F.geometry = GMT_IS_LINE;	/* And poly, but need LINE first so read works as line */
						break;
					default:
						GMT_Message (API, GMT_TIME_NONE, "Bad feature type. Use s, e, t, l, p or w.\n");
						n_errors++;
						break;
				}
				break;
			case 'G':	/* Set fill for symbols or polygon */
				switch (opt->arg[0]) {
					case 'f':	/* Symbol/polygon color fill */
						if (opt->arg[1] == '-')
				 			Ctrl->G.active[F_ID] = true;
						else if (!opt->arg[1] || gmt_getfill (GMT, &opt->arg[1], &Ctrl->G.fill[F_ID])) {
							gmt_fill_syntax (GMT, 'G', "(-Gf or -Gn)");
							n_errors++;
						}
						break;
					case 'n':	/* Label name color */
		 				Ctrl->G.active[N_ID] = true;
						if (opt->arg[1] == '-')
							Ctrl->t_transp = 0.0;
						else if (!opt->arg[1] || gmt_getfill (GMT, &opt->arg[1], &Ctrl->G.fill[N_ID])) {
							gmt_fill_syntax (GMT, 'G', "(-Gf or -Gn)");
							n_errors++;
						}
						break;
					default:
						gmt_fill_syntax (GMT, 'G', "(-Gf or -Gn)");
						n_errors++;
						break;
				}
				break;
			case 'I':	/* Custom icon */
	 			Ctrl->I.active = true;
				gmt_M_str_free (Ctrl->I.file);
				if (opt->arg[0] == '+')
					snprintf (buffer, GMT_LEN256, "http://maps.google.com/mapfiles/kml/%s", &opt->arg[1]);
				else if (opt->arg[0])
					strncpy (buffer, opt->arg, GMT_LEN256-1);
				Ctrl->I.file = strdup (buffer);
				break;
			case 'L':	/* Extended data */
 				Ctrl->L.active = true;
				pos = Ctrl->L.n_cols = 0;
				while ((gmt_strtok (opt->arg, ",", &pos, p))) {
					if (Ctrl->L.n_cols == n_alloc) Ctrl->L.name = gmt_M_memory (GMT, Ctrl->L.name, n_alloc += GMT_TINY_CHUNK, char *);
					Ctrl->L.name[Ctrl->L.n_cols++] = strdup (p);
				}
				break;
			case 'N':	/* Feature label */
				Ctrl->N.active = true;
				if (opt->arg[0] == '-') {	/* First non-coordinate field as label */
					Ctrl->N.mode = GET_COL_LABEL;
				}
				else if (opt->arg[0] == '+') {	/* Everything following coordinates is a label */
					Ctrl->N.mode = GET_LABEL;
				}
				else if (!opt->arg[0]) {	/* Want no label */
					Ctrl->t_transp = 0.0;
					Ctrl->N.mode = NO_LABEL;
				}
				else {
					Ctrl->N.fmt = strdup (opt->arg);
					Ctrl->N.mode = FMT_LABEL;
				}
				break;
			case 'Q':	/* Wiggle azimuth and scale settings in data units for map distance [unit] */
				Ctrl->Q.active = true;
				switch (opt->arg[0]) {
					case 'i': Ctrl->Q.mode = 1; Ctrl->Q.value[1] = atof (&opt->arg[1]); break;
					case 'q': Ctrl->Q.mode = 0; Ctrl->Q.value[0] = atof (&opt->arg[1]); break;
					case 's': strcpy (p, &opt->arg[1]); k = (unsigned int)strlen (p) - 1;
						if (!strchr (GMT_LEN_UNITS, p[k])) strcat (p, "e");	/* Force meters as default unit */
						Ctrl->Q.dmode = gmt_get_distance (GMT, p, &(Ctrl->Q.scale), &(Ctrl->Q.unit)); break;
				}
				break;
			case 'R':	/* Region setting */
				Ctrl->R2.active = GMT->common.R.active = true;
				if (opt->arg[0] == 'a')	/* Get args from data domain */
					Ctrl->R2.automatic = true;
				else if (opt->arg[0])
					n_errors += gmt_parse_R_option (GMT, opt->arg);
				break;
			case 'S':	/* Scale for symbol (c) or text (n) */
				Ctrl->S.active = true;
				if (opt->arg[0] == 'c')
					Ctrl->S.scale[F_ID] = atof (&opt->arg[1]);
				else if (opt->arg[0] == 'n')
					Ctrl->S.scale[N_ID] = atof (&opt->arg[1]);
				else {
					GMT_Message (API, GMT_TIME_NONE, "-S requires c or n, then nondimensional scale\n");
					n_errors++;
				}
				break;
			case 'T':	/* Title [and folder] */
				Ctrl->T.active = true;
				if ((c = strchr (opt->arg, '/')) != NULL) {	/* Got both title and folder */
					if (c[1]) Ctrl->T.folder = strdup (&c[1]);
					*c = '\0';
					if (opt->arg[0]) Ctrl->T.title = strdup (opt->arg);
					*c = '/';
				}
				else if (opt->arg[0]) Ctrl->T.title = strdup (opt->arg);
				break;
			case 'W':	/* Pen attributes */
				Ctrl->W.active = true;
				if (opt->arg[0] && strchr ("-+", opt->arg[0]) ) {	/* Definitively old-style args */
					if (gmt_M_compat_check (API->GMT, 5)) {	/* Sorry */
						GMT_Report (API, GMT_MSG_NORMAL, "Your -W syntax is obsolete; see program usage.\n");
						n_errors++;
					}
					else {
						GMT_Report (API, GMT_MSG_NORMAL, "Your -W syntax is obsolete; see program usage.\n");
						n_errors += parse_old_W (API, Ctrl, opt->arg);
					}
				}
				else if (opt->arg[0]) {
					if (gmt_getpen (GMT, opt->arg, &Ctrl->W.pen)) {
						gmt_pen_syntax (GMT, 'W', "sets pen attributes [Default pen is %s]:", 11);
						n_errors++;
					}
				}
				break;
			case 'Z':	/* Visibility control */
				Ctrl->Z.active = true;
				pos = 0;
				while ((gmt_strtok (&opt->arg[1], "+", &pos, p))) {
					switch (p[0]) {
						case 'a':	/* Altitude range */
							if (sscanf (&p[1], "%[^/]/%s", T[0], T[1]) != 2) {
								GMT_Message (API, GMT_TIME_NONE, "-Z+a requires 2 arguments\n");
								n_errors++;
							}
							Ctrl->Z.min[ALT] = atof (T[0]);	Ctrl->Z.max[ALT] = atof (T[1]);
							break;
						case 'l':	/* LOD */
							if (sscanf (&p[1], "%[^/]/%s", T[0], T[1]) != 2) {
								GMT_Message (API, GMT_TIME_NONE, "-Z+l requires 2 arguments\n");
								n_errors++;
							}
							Ctrl->Z.min[LOD] = atof (T[0]);	Ctrl->Z.max[LOD] = atof (T[1]);
							break;
						case 'f':	/* Fading */
							if (sscanf (&p[1], "%[^/]/%s", T[0], T[1]) != 2) {
								GMT_Message (API, GMT_TIME_NONE, "-Z+f requires 2 arguments\n");
								n_errors++;
							}
							Ctrl->Z.min[FADE] = atof (T[0]);	Ctrl->Z.max[FADE] = atof (T[1]);
							break;
						case 'v':
							Ctrl->Z.invisible = true;
							break;
						case 'o':
							Ctrl->Z.open = true;
							break;
						default:
							GMT_Message (API, GMT_TIME_NONE, "-Z unrecognized modifier +%c\n", p[0]);
							n_errors++;
							break;
					}
				}
				break;

			default:	/* Report bad options */
				n_errors += gmt_default_error (GMT, opt->option);
				break;
		}
	}
	n_errors += gmt_M_check_condition (GMT, Ctrl->C.active && !Ctrl->C.file, "Syntax error -C option: Need to supply color palette name\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->D.active && access (Ctrl->D.file, R_OK), "Syntax error -D: Cannot open HTML description file %s\n", Ctrl->D.file);
	if (GMT->common.b.active[GMT_IN] && GMT->common.b.ncol[GMT_IN] == 0) GMT->common.b.ncol[GMT_IN] = 2;
	n_errors += gmt_M_check_condition (GMT, GMT->common.b.active[GMT_OUT], "Syntax error: Cannot produce binary KML output\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->R2.automatic && n_files > 1, "Syntax error: -Ra without arguments only accepted for single table\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->S.scale[F_ID] < 0.0 || Ctrl->S.scale[N_ID] < 0.0, "Syntax error: -S takes scales > 0.0\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->t_transp < 0.0 || Ctrl->t_transp > 1.0, "Syntax error: -Q takes transparencies in range 0-1\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->N.mode == GET_LABEL && Ctrl->F.mode >= LINE, "Syntax error: -N+ not valid for lines and polygons\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->F.mode == WIGGLE && Ctrl->Q.scale <= 0.0, "Syntax error: -Fw requires -Qs\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.pen.width < 1.0, "Syntax error: -W given pen width < 1 pixel.  Use integers and append p as unit.\n");
	n_errors += gmt_M_check_condition (GMT, Ctrl->W.active && Ctrl->W.pen.cptmode && !Ctrl->C.active, "Syntax error: -W option +|-<pen> requires the -C option.\n");
	n_errors += gmt_M_check_condition (GMT, (GMT->common.b.active[GMT_IN] || GMT->common.i.active) && (Ctrl->N.mode == GET_COL_LABEL || Ctrl->N.mode == GET_LABEL), "Syntax error: Cannot use -N- or -N+ when -b or -i are used.\n");

	return (n_errors ? GMT_PARSE_ERROR : GMT_NOERROR);
}

GMT_LOCAL int kml_print (struct GMTAPI_CTRL *API, int ntabs, char *format, ...) {
	/* Message whose output depends on verbosity setting */
	int tab;
	char record[GMT_BUFSIZ] = {""};
	va_list args;

	for (tab = 0; tab < ntabs; tab++) record[tab] = '\t';
	va_start (args, format);
	/* append format to the record: */
	vsnprintf (record + ntabs, GMT_BUFSIZ - ntabs, format, args);
	va_end (args);
	assert (strlen (record) < GMT_BUFSIZ);
	GMT_Put_Record (API, GMT_WRITE_TEXT, record);
	return (GMT_NOERROR);
}

GMT_LOCAL int check_lon_lat (struct GMT_CTRL *GMT, double *lon, double *lat) {
	if (*lat < GMT->common.R.wesn[YLO] || *lat > GMT->common.R.wesn[YHI]) return (true);
	if (*lon < GMT->common.R.wesn[XLO]) *lon += 360.0;
	if (*lon > GMT->common.R.wesn[XHI]) *lon -= 360.0;
	if (*lon < GMT->common.R.wesn[XLO]) return (true);
	return (false);
}

GMT_LOCAL void print_altmode (struct GMTAPI_CTRL *API, int extrude, int fmode, int altmode, int ntabs) {
	char *RefLevel[5] = {"clampToGround", "relativeToGround", "absolute", "relativeToSeaFloor", "clampToSeaFloor"};
	if (extrude) kml_print (API, ntabs, "<extrude>1</extrude>\n");
	if (fmode) kml_print (API, ntabs, "<tessellate>1</tessellate>\n");
	if (altmode == KML_GROUND_REL || altmode == KML_ABSOLUTE) kml_print (API, ntabs, "<altitudeMode>%s</altitudeMode>\n", RefLevel[altmode]);
	if (altmode == KML_SEAFLOOR_REL || altmode == KML_SEAFLOOR) kml_print (API, ntabs, "<gx:altitudeMode>%s</gx:altitudeMode>\n", RefLevel[altmode]);
}

GMT_LOCAL void ascii_output_three (struct GMTAPI_CTRL *API, double out[], int ntabs) {
	char X[GMT_LEN256] = {""}, Y[GMT_LEN256] = {""}, Z[GMT_LEN256] = {""};
	gmt_ascii_format_col (API->GMT, X, out[GMT_X], GMT_OUT, GMT_X);
	gmt_ascii_format_col (API->GMT, Y, out[GMT_Y], GMT_OUT, GMT_Y);
	gmt_ascii_format_col (API->GMT, Z, out[GMT_Z], GMT_OUT, GMT_Z);
	kml_print (API, ntabs, "%s,%s,%s\n", X, Y, Z);
}

GMT_LOCAL void place_region_tag (struct GMTAPI_CTRL *API, double wesn[], double min[], double max[], int N) {
	char text[GMT_LEN256] = {""};
	if (gmt_M_360_range (wesn[XLO], wesn[XHI])) { wesn[XLO] = -180.0; wesn[XHI] = +180.0;}
	kml_print (API, N++, "<Region>\n");
	kml_print (API, N++, "<LatLonAltBox>\n");
	gmt_ascii_format_col (API->GMT, text, wesn[YHI], GMT_OUT, GMT_Y);
	kml_print (API, N, "<north>%s</north>\n", text);
	gmt_ascii_format_col (API->GMT, text, wesn[YLO], GMT_OUT, GMT_Y);
	kml_print (API, N, "<south>%s</south>\n", text);
	gmt_ascii_format_col (API->GMT, text, wesn[XHI], GMT_OUT, GMT_X);
	kml_print (API, N, "<east>%s</east>\n", text);
	gmt_ascii_format_col (API->GMT, text, wesn[XLO], GMT_OUT, GMT_X);
	kml_print (API, N, "<west>%s</west>\n", text);
	if (max[ALT] > min[ALT]) {
		kml_print (API, N, "<minAltitude>%g</minAltitude>\n", min[ALT]);
		kml_print (API, N, "<maxAltitude>%g</maxAltitude>\n", max[ALT]);
	}
	kml_print (API, --N, "</LatLonAltBox>\n");
	if (max[LOD] != min[LOD]) {
		kml_print (API, N++, "<Lod>\n");
		kml_print (API, N, "<minLodPixels>%ld</minLodPixels>\n", lrint (min[LOD]));
		kml_print (API, N, "<maxLodPixels>%ld</maxLodPixels>\n", lrint (max[LOD]));
		if (min[FADE] > 0.0 || max[FADE] > 0.0) {
			kml_print (API, N, "<minFadeExtent>%g</minFadeExtent>\n", min[FADE]);
			kml_print (API, N, "<maxFadeExtent>%g</maxFadeExtent>\n", max[FADE]);
		}
		kml_print (API, --N, "</Lod>\n");
	}
	kml_print (API, --N, "</Region>\n");
}

GMT_LOCAL void set_iconstyle (struct GMTAPI_CTRL *API, double *rgb, double scale, char *iconfile, int N) {
	/* No icon = no symbol */
	kml_print (API, N++, "<IconStyle>\n");
	kml_print (API, N++, "<Icon>\n");
	if (iconfile[0] != '-') kml_print (API, N, "<href>%s</href>\n", iconfile);
	kml_print (API, --N, "</Icon>\n");
	if (iconfile[0] != '-') {
		kml_print (API, N, "<scale>%g</scale>\n", scale);
		kml_print (API, N, "<color>%02x%02x%02x%02x</color>\n", gmt_M_u255 (1.0 - rgb[3]), GMT_3u255 (rgb));
	}
	kml_print (API, --N, "</IconStyle>\n");
}

GMT_LOCAL void set_linestyle (struct GMTAPI_CTRL *API, struct GMT_PEN *pen, double *rgb, int N) {
	kml_print (API, N++, "<LineStyle>\n");
	kml_print (API, N, "<color>%02x%02x%02x%02x</color>\n", gmt_M_u255 (1.0 - rgb[3]), GMT_3u255 (rgb));
	kml_print (API, N, "<width>%g</width>\n", pen->width);
	kml_print (API, --N, "</LineStyle>\n");
}

GMT_LOCAL void set_polystyle (struct GMTAPI_CTRL *API, double *rgb, int outline, int active, int N) {
	kml_print (API, N++, "<PolyStyle>\n");
	kml_print (API, N, "<color>%02x%02x%02x%02x</color>\n", gmt_M_u255 (1.0 - rgb[3]), GMT_3u255 (rgb));
	kml_print (API, N, "<fill>%d</fill>\n", !active);
	kml_print (API, N, "<outline>%d</outline>\n", outline);
	kml_print (API, --N, "</PolyStyle>\n");
}

GMT_LOCAL void get_rgb_lookup (struct GMT_CTRL *GMT, struct GMT_PALETTE *P, int index, double *rgb) {
	/* Special version of gmt_get_rgb_lookup since no interpolation can take place */

	if (index < 0) {	/* NaN, Foreground, Background */
		gmt_M_rgb_copy (rgb, P->bfn[index+3].rgb);
		P->skip = P->bfn[index+3].skip;
	}
	else if (P->data[index].skip) {		/* Set to page color for now */
		gmt_M_rgb_copy (rgb, GMT->current.setting.ps_page_rgb);
		P->skip = true;
	}
	else {	/* Return low color */
		gmt_M_memcpy (rgb, P->data[index].rgb_low, 4, double);
		P->skip = false;
	}
}

GMT_LOCAL int get_data_region (struct GMT_CTRL *GMT, struct GMT_TEXTSET *D, double wesn[]) {
	/* Because we read as textset we must determine the data extent the hard way */
	unsigned int tbl, ix, iy, way;
	uint64_t row, seg;
	char T[2][GMT_LEN64];
	double x, y, y_min = 90.0, y_max = -90.0;
	struct GMT_QUAD *Q = gmt_quad_init (GMT, 1);	/* Allocate and initialize one QUAD structure */
	ix = GMT->current.setting.io_lonlat_toggle[GMT_IN];	iy = 1 - ix;

	for (tbl = 0; tbl < D->n_tables; tbl++) {
		for (seg = 0; seg < D->table[tbl]->n_segments; seg++) {
			for (row = 0; row < D->table[tbl]->segment[seg]->n_rows; row++) {
				sscanf (D->table[tbl]->segment[seg]->data[row], "%s %s", T[ix], T[iy]);
				if (gmt_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X], gmt_scanf_arg (GMT, T[GMT_X], GMT->current.io.col_type[GMT_IN][GMT_X], &x), T[GMT_X])) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not decode longitude from %s\n", T[GMT_X]);
					gmt_M_free (GMT, Q);
					return (GMT_RUNTIME_ERROR);
				}
				if (gmt_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y], gmt_scanf_arg (GMT, T[GMT_Y], GMT->current.io.col_type[GMT_IN][GMT_Y], &y), T[GMT_Y])) {
					GMT_Report (GMT->parent, GMT_MSG_NORMAL, "Error: Could not decode latitude from %s\n", T[GMT_Y]);
					gmt_M_free (GMT, Q);
					return (GMT_RUNTIME_ERROR);
				}
				gmt_quad_add (GMT, Q, x);
				if (y < y_min) y_min = y;
				if (y > y_max) y_max = y;
			}
		}
	}
	way = gmt_quad_finalize (GMT, Q);
	wesn[XLO] = Q->min[way];	wesn[XHI] = Q->max[way];
	wesn[YLO] = y_min;		wesn[YHI] = y_max;
	gmt_M_free (GMT, Q);
	return (GMT_NOERROR);
}

GMT_LOCAL bool crossed_dateline (double this_x, double last_x) {
	if (this_x > 90.0 && this_x <= 180.0 && last_x > +180.0 && last_x < 270.0) return (true);	/* Positive lons 0-360 */
	if (last_x > 90.0 && last_x <= 180.0 && this_x > +180.0 && this_x < 270.0) return (true);	/* Positive lons 0-360 */
	if (this_x > 90.0 && this_x <= 180.0 && last_x > -180.0 && last_x < -90.0) return (true);	/* Lons in -180/+180 range */
	if (last_x > 90.0 && last_x <= 180.0 && this_x > -180.0 && this_x < -90.0) return (true);	/* Lons in -180/+180 range */
	return (false);
}

GMT_LOCAL struct KML * kml_alloc (struct GMT_CTRL *GMT, struct GMT_DATASET *D) {
	uint64_t tbl, seg, max = 0;
	struct KML *kml = gmt_M_memory (GMT, NULL, 1, struct KML);
	for (tbl = 0; tbl < D->n_tables; tbl++) for (seg = 0; seg < D->table[tbl]->n_segments; seg++) if (D->table[tbl]->segment[seg]->n_rows > max) max = D->table[tbl]->segment[seg]->n_rows;
	kml->n_alloc = 3 * max;
	kml->lon  = gmt_M_memory (GMT, NULL, kml->n_alloc, double);
	kml->lat  = gmt_M_memory (GMT, NULL, kml->n_alloc, double);
	kml->z    = gmt_M_memory (GMT, NULL, kml->n_alloc, double);
	kml->flon = gmt_M_memory (GMT, NULL, kml->n_alloc, double);
	kml->flat = gmt_M_memory (GMT, NULL, kml->n_alloc, double);
	return (kml);
}

GMT_LOCAL void kml_free (struct GMT_CTRL *GMT, struct KML ** kml) {
	gmt_M_free (GMT, (*kml)->lon);
	gmt_M_free (GMT, (*kml)->lat);
	gmt_M_free (GMT, (*kml)->z);
	gmt_M_free (GMT, (*kml)->flon);
	gmt_M_free (GMT, (*kml)->flat);
	gmt_M_free (GMT, *kml);
}

void KML_plot_object (struct GMTAPI_CTRL *API, double *x, double *y, uint64_t np, int type, int process_id, int alt_mode, int N, double z_level) {
	/* Plots a self-contained polygon or line, depending on type, using
	 * the current fill/line styles */
	static char *name[2] = {"Wiggle Anomaly", "Positive Anomaly"};
	static char *feature[5] = {"Point", "Point", "Point", "LineString", "Polygon"};
	double out[3];
	uint64_t k;
	kml_print (API, N++, "<Placemark>\n");
	kml_print (API, N, "<name>%s</name>\n", name[type-LINE]);
	kml_print (API, N, "<styleUrl>#st-%d-%d</styleUrl>\n", process_id, 0); /* It is always style 0 */
	kml_print (API, N++, "<%s>\n", feature[type]);
	print_altmode (API, 0, 1, alt_mode, N);
	if (type == POLYGON) {
		kml_print (API, N++, "<outerBoundaryIs>\n");
		kml_print (API, N++, "<LinearRing>\n");
	}
	kml_print (API, N++, "<coordinates>\n");
	for (k = 0; k < np; k++) {
		out[GMT_X] = x[k];
		out[GMT_Y] = y[k];
		out[GMT_Z] = z_level;
		ascii_output_three (API, out, N);
	}
	kml_print (API, --N, "</coordinates>\n");
	if (type == POLYGON) {
		kml_print (API, --N, "</LinearRing>\n");
		kml_print (API, --N, "</outerBoundaryIs>\n");
	}
	kml_print (API, --N, "</%s>\n", feature[type]);
	kml_print (API, --N, "</Placemark>\n");
}

GMT_LOCAL void kml_plot_wiggle (struct GMT_CTRL *GMT, struct KML *kml, double zscale, int mode, double azim[], int fill, int outline, int process_id, int amode, int N, double altitude) {
	int64_t i, np;
	double lon_len, lat_len, az = 0.0, s = 0.0, c = 0.0, lon_inc, lat_inc;
	double start_az, stop_az, daz;
	//char *RGB[2] = {"green", "red"};
	/* Here, kml->{lon, lat, z} are the kml->n_in coordinates of one section of a to-be-designed wiggle.
	 * We need to erect z, properly scaled, normal to the line and build the kml->n_out fake kml->{flon, flat}
	 * points that we can use to plot lines and polygons on Google Earth */
	/* zscale is in degrees per data units. */
	// fprintf (stderr, "# zscale = %g degrees/unit or %g nm/units or %g units/nm\n", zscale, 60.0*zscale, 1.0/(60.0*zscale));
	if (mode == 1) {	/* Want all wiggles to point in this azimuth */
		az = D2R * azim[1];
		sincos (az, &s, &c);
	}
	else {	/* Preferential azimuth for wiggles */
		start_az = (azim[0] - 90.0) * D2R;
		stop_az  = (azim[0] + 90.0) * D2R;
	}
	if (fill || outline) {
		kml->n_out = 0;
		for (i = 0; i < (int64_t)kml->n_in; i++) {
			if (mode == 0 && i < (int64_t)(kml->n_in-1)) {
				if (i == 0)
					daz = gmt_az_backaz (GMT, kml->lon[i+1], kml->lat[i+1], kml->lon[i], kml->lat[i], true);
				else
					daz = gmt_az_backaz (GMT, kml->lon[i], kml->lat[i], kml->lon[i-1], kml->lat[i-1], true);
				az = D2R * (daz + 90.0);
				while (az < start_az) az += TWO_PI;
				if (az > stop_az) az -= M_PI;
			}
			if (fabs (kml->z[i]) > 0.0) {
				if (mode == 0) sincos (az, &s, &c);
				lat_len = zscale * kml->z[i];
				lon_len = lat_len * cosd (0.5 * (kml->lat[i] + kml->lat[i+1]));
				lon_inc = lon_len * s;
				lat_inc = lat_len * c;
			}
			else
				lon_inc = lat_inc = 0.0;
			kml->flon[kml->n_out] = kml->lon[i] + lon_inc;
			kml->flat[kml->n_out] = kml->lat[i] + lat_inc;
			kml->n_out++;
		}
#if 0
		fprintf (stderr, "> segment fill -G%s\n", RGB[fill]);
		for (i = 0; i < (int64_t)kml->n_out; i++)
			fprintf (stderr, "%g\t%g\n", kml->flon[i], kml->flat[i]);
#endif
		np = kml->n_out;	/* Number of points for the outline only */
		if (fill) {
			for (i = kml->n_in - 2; i >= 0; i--, kml->n_out++) {	/* Go back to 1st point along track */
				kml->flon[kml->n_out] = kml->lon[i];
				kml->flat[kml->n_out] = kml->lat[i];
			}
		}
	}

	if (fill) /* First shade wiggles */
		KML_plot_object (GMT->parent, kml->flon, kml->flat, kml->n_out, POLYGON, process_id, amode, N, altitude);

	if (outline) /* Then draw wiggle outline */
		KML_plot_object (GMT->parent, kml->flon, kml->flat, np, LINE, process_id, amode, N, altitude);
}

/* Must free allocated memory before returning */
#define bailout(code) {gmt_M_free_options (mode); return (code);}
#define Return(code) {Free_Ctrl (GMT, Ctrl); gmt_end_module (GMT, GMT_cpy); bailout (code);}

int GMT_gmt2kml (void *V_API, int mode, void *args) {
	bool first = true, get_z = false, use_folder = false, do_description, no_dateline = false, act, no_text;
	unsigned int n_coord = 0, t1_col, t2_col, pnt_nr = 0, tbl, col, pos, ix, iy;

	uint64_t row, seg, n_tables, n_segments, n_rows;
	size_t L = 0;
	int set_nr = 0, index, N = 1, error = 0, process_id;

	char extra[GMT_BUFSIZ] = {""}, buffer[GMT_BUFSIZ] = {""}, description[GMT_BUFSIZ] = {""}, item[GMT_LEN128] = {""}, C[5][GMT_LEN64] = {"","","","",""};
	char *feature[5] = {"Point", "Point", "Point", "LineString", "Polygon"}, *Document[2] = {"Document", "Folder"};
	char *name[5] = {"Point", "Event", "Timespan", "Line", "Polygon"};
	char text[GMT_LEN256] = {""};
	char **file = NULL, *label = NULL, *header = NULL;

	double rgb[4], out[5], last_x = 0;

	struct KML *kml = NULL;
	struct GMT_OPTION *options = NULL;
	struct GMT_PALETTE *P = NULL;
	struct GMT_DATASET *Dd = NULL;
	struct GMT_TEXTSET *Dt = NULL;
	struct GMT2KML_CTRL *Ctrl = NULL;
	struct GMT_CTRL *GMT = NULL, *GMT_cpy = NULL;		/* General GMT internal parameters */
	struct GMTAPI_CTRL *API = gmt_get_api_ptr (V_API);	/* Cast from void to GMTAPI_CTRL pointer */

	/*----------------------- Standard module initialization and parsing ----------------------*/

	if (API == NULL) return (GMT_NOT_A_SESSION);
	if (mode == GMT_MODULE_PURPOSE) return (usage (API, GMT_MODULE_PURPOSE));	/* Return the purpose of program */
	options = GMT_Create_Options (API, mode, args);	if (API->error) return (API->error);	/* Set or get option list */

	if (!options || options->option == GMT_OPT_USAGE) bailout (usage (API, GMT_USAGE));/* Return the usage message */
	if (options->option == GMT_OPT_SYNOPSIS) bailout (usage (API, GMT_SYNOPSIS));	/* Return the synopsis */

	/* Parse the command-line arguments */

	GMT = gmt_begin_module (API, THIS_MODULE_LIB, THIS_MODULE_NAME, &GMT_cpy); /* Save current state */
	if (GMT_Parse_Common (API, GMT_PROG_OPTIONS, options)) Return (API->error);
	Ctrl = New_Ctrl (GMT);		/* Allocate and initialize a new control structure */
	if ((error = parse (GMT, Ctrl, options)) != 0) Return (error);

	/*---------------------------- This is the gmt2kml main code ----------------------------*/

	GMT_Report (API, GMT_MSG_VERBOSE, "Processing input table data\n");
	no_text = (gmt_input_is_bin (GMT, Ctrl->In.file) || GMT->common.i.active || Ctrl->F.mode == WIGGLE);	/* Must read as data table so no text columns are expected or considered */

	/* gmt2kml only applies to geographic data so we do a -fg implicitly here */
	gmt_set_geographic (GMT, GMT_IN);
	gmt_set_geographic (GMT, GMT_OUT);
	extra[0] = '\0';
	gmt_M_memset (out, 5, double);	/* Set to zero */
	ix = GMT->current.setting.io_lonlat_toggle[GMT_IN];	iy = 1 - ix;

	if (Ctrl->C.active) {	/* Process CPT */
		if ((P = GMT_Read_Data (API, GMT_IS_PALETTE, GMT_IS_FILE, GMT_IS_NONE, GMT_READ_NORMAL, NULL, Ctrl->C.file, NULL)) == NULL) {
			Return (API->error);
		}
		if (P->is_continuous) {
			GMT_Message (API, GMT_TIME_NONE, "Cannot use continuous color palette\n");
			Return (GMT_RUNTIME_ERROR);
		}
	}

	if (GMT_Init_IO (API, GMT_IS_TEXTSET, GMT_IS_NONE, GMT_OUT, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data output */
		Return (API->error);
	}
	if (GMT_Begin_IO (API, GMT_IS_TEXTSET, GMT_OUT, GMT_HEADER_OFF) != GMT_NOERROR) {
		Return (API->error);	/* Enables data output and sets access mode */
	}
	if (GMT_Set_Geometry (API, GMT_OUT, Ctrl->F.geometry) != GMT_NOERROR) {	/* Sets output geometry */
		Return (API->error);
	}

	/* Now we are ready to take on some input values */

	out[GMT_Z] = Ctrl->A.altitude;
	strcpy (GMT->current.setting.io_col_separator, ",");		/* Specify comma-separated output */
	strcpy (GMT->current.setting.format_float_out, "%.12g");	/* Make sure we use enough decimals */
	n_coord = (Ctrl->F.mode < LINE) ? Ctrl->F.mode + 2 : 2;		/* This is a cryptic way to determine if there are 2,3 or 4 columns... */
	if (Ctrl->F.mode == WIGGLE) n_coord = 3;	/* But here we need exactly 3 */
	get_z = (Ctrl->C.active || Ctrl->A.get_alt);
	if (get_z) n_coord++;
	t1_col = 2 + get_z;
	t2_col = 3 + get_z;
	if (Ctrl->F.mode == EVENT || Ctrl->F.mode == SPAN) GMT->current.io.col_type[GMT_IN][t1_col] = GMT->current.io.col_type[GMT_OUT][t1_col] = GMT_IS_ABSTIME;
	if (Ctrl->F.mode == SPAN)  GMT->current.io.col_type[GMT_IN][t2_col] = GMT->current.io.col_type[GMT_OUT][t2_col] = GMT_IS_ABSTIME;

	if (Ctrl->F.mode == WIGGLE) {	/* Adjust wiggle scale for units and then take inverse */
		char unit_name[GMT_LEN16] = {""};
		gmt_check_scalingopt (GMT, 'Q', Ctrl->Q.unit, unit_name);
		GMT_Report (API, GMT_MSG_VERBOSE, "Wiggle scale given as %g z-data units per %s.\n", Ctrl->Q.scale, unit_name);
		gmt_init_distaz (GMT, Ctrl->Q.unit, Ctrl->Q.dmode, GMT_MAP_DIST);	/* Initialize distance machinery */
		Ctrl->Q.scale = 1.0 / Ctrl->Q.scale;	/* Now in map-distance units (i.e, unit they appended) per users data units */
		GMT_Report (API, GMT_MSG_VERBOSE, "Wiggle scale inverted as %g %s per z-data units.\n", Ctrl->Q.scale, unit_name);
		/* Convert to degrees per user data unit - this is our scale that converts z-data to degree distance latitude */
		Ctrl->Q.scale = R2D * (Ctrl->Q.scale / GMT->current.map.dist[GMT_MAP_DIST].scale) / GMT->current.proj.mean_radius;
		GMT_Report (API, GMT_MSG_VERBOSE, "Wiggle scale inverted to yield %g degrees per z-data unit\n", Ctrl->Q.scale);
	}
	if (!GMT->common.O.active) {
		/* Create KML header */
		kml_print (API, 0, "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n");
		kml_print (API, 0, "<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
		kml_print (API, N++, "<%s>\n", Document[KML_DOCUMENT]);
		if (Ctrl->T.title != NULL && *Ctrl->T.title != '\0')
			kml_print (API, N, "<name>%s</name>\n", Ctrl->T.title);
		if (Ctrl->Z.invisible) kml_print (API, N, "<visibility>0</visibility>\n");
		if (Ctrl->Z.open) kml_print (API, N, "<open>1</open>\n");
	}

	/* Apparently, for Google Maps (not Google Earth), styles need to be outside any <Folder></Folder> pair, so we do them first */
	process_id = (int) getpid();
	kml_print (API, N++, "<Style id=\"st-%d-0\">\n", process_id);	/* Default style unless -C is used, use PID to get unique style ID in case of layer-caking -O -K */

	/* Set icon style (applies to symbols only */
	set_iconstyle (API, Ctrl->G.fill[F_ID].rgb, Ctrl->S.scale[F_ID], Ctrl->I.file, N);

	/* Set shared line and polygon style (also for extrusions) */
	if (Ctrl->F.mode == WIGGLE) {
		if (!Ctrl->G.active[F_ID]) {
			set_polystyle (API, Ctrl->G.fill[F_ID].rgb, 0, 0, N);
		}
		if (Ctrl->W.active) {
			set_linestyle (API, &Ctrl->W.pen, Ctrl->W.pen.rgb, N);
		}
	}
	else {
		set_linestyle (API, &Ctrl->W.pen, Ctrl->W.pen.rgb, N);
		set_polystyle (API, Ctrl->G.fill[F_ID].rgb, !Ctrl->W.pen.cptmode, Ctrl->G.active[F_ID], N);
	}

	/* Set style for labels */
	kml_print (API, N++, "<LabelStyle>\n");
	kml_print (API, N, "<scale>%g</scale>\n", Ctrl->S.scale[N_ID]);
	kml_print (API, N, "<color>%02x%02x%02x%02x</color>\n", gmt_M_u255 (1.0 - Ctrl->G.fill[N_ID].rgb[3]), GMT_3u255 (Ctrl->G.fill[N_ID].rgb));
	kml_print (API, --N, "</LabelStyle>\n");
	kml_print (API, --N, "</Style>\n");

	for (index = -3; Ctrl->C.active && index < (int)P->n_colors; index++) {	/* Place styles for each color in CPT */
		get_rgb_lookup (GMT, P, index, rgb);	/* For -3, -2, -1 we get the back, fore, nan colors */
		kml_print (API, N++, "<Style id=\"st-%d-%d\">\n", process_id, index + 4); /* +4 to make the first style ID = 1 */
		if (Ctrl->F.mode < LINE)	/* Set icon style (applies to symbols only */
			set_iconstyle (API, rgb, Ctrl->S.scale[F_ID], Ctrl->I.file, N);
		else if (Ctrl->F.mode == LINE)	/* Line style only */
			set_linestyle (API, &Ctrl->W.pen, rgb, N);
		else {	/* Polygons */
			if (!Ctrl->W.active) {	/* Only fill, no outline */
				set_polystyle (API, rgb, false, Ctrl->G.active[F_ID], N);
			}
			else if (Ctrl->W.pen.cptmode == 2) { /* Use -C for fill, -W for outline */
				set_polystyle (API, rgb, true, Ctrl->G.active[F_ID], N);
				set_linestyle (API, &Ctrl->W.pen, Ctrl->W.pen.rgb, N);
			}
			else if (Ctrl->W.pen.cptmode == 1) { /* Use -G for fill, -C for outline */
				set_polystyle (API, Ctrl->G.fill[F_ID].rgb, true, Ctrl->G.active[F_ID], N);
				set_linestyle (API, &Ctrl->W.pen, rgb, N);
			}
			else if (Ctrl->W.pen.cptmode == 3) { /* Use -C for fill and outline */
				set_polystyle (API, rgb, true, Ctrl->G.active[F_ID], N);
				set_linestyle (API, &Ctrl->W.pen, rgb, N);
			}
		}
		kml_print (API, --N, "</Style>\n");
	}
	index = -4;	/* Default style unless -C changes things */

	/* Now start the data within a <Folder> */
	if (!Ctrl->T.folder) {
		sprintf (buffer, "%s Features", name[Ctrl->F.mode]);
		Ctrl->T.folder = strdup (buffer);
	}
	if (GMT->common.O.active || GMT->common.K.active) use_folder = true;	/* When at least one or -O, -K is used */
	if (GMT->common.O.active) N++;	/* Due to the extra folder tag */
	if (use_folder) {
		kml_print (API, N++, "<%s>\n", Document[KML_FOLDER]);
		kml_print (API, N, "<name>%s</name>\n", Ctrl->T.folder);
		if (Ctrl->Z.invisible) kml_print (API, N, "<visibility>0</visibility>\n");
		if (Ctrl->Z.open) kml_print (API, N, "<open>1</open>\n");
	}

	if (Ctrl->D.active) {	/* Add in a description HTML snipped */
		char line[GMT_BUFSIZ];
		FILE *fp = NULL;
		if ((fp = gmt_fopen (GMT, Ctrl->D.file, "r")) == NULL) {
			GMT_Message (API, GMT_TIME_NONE, "Could not open description file %s\n", Ctrl->D.file);
			Return (GMT_RUNTIME_ERROR);
		}
		kml_print (API, N++, "<description>\n");
		kml_print (API, N++, "<![CDATA[\n");
		while (gmt_fgets (GMT, line, GMT_BUFSIZ, fp)) kml_print (API, N, "%s", line);
		gmt_fclose (GMT, fp);
		kml_print (API, --N, "]]>\n");
		kml_print (API, --N, "</description>\n");
	}

	/* If binary input or the user selected input columns with -i then call open file as GMT_IS_DATASET;
	   TEXTSET otherwise, then handle the differenes below n_tables = (binary) ? Dd->n_tables : Dt->n_tables;
	   same for n_segments, and n_rows, then additional if-test on parsing text record or using data as is. */

	if (no_text) {	/* Treat input as a data set (no texts) */
		if (GMT_Init_IO (API, GMT_IS_DATASET, Ctrl->F.geometry, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
			Return (API->error);
		}
		if ((Dd = GMT_Read_Data (API, GMT_IS_DATASET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
			Return (API->error);
		}
		if (Dd->n_columns < n_coord) {
			GMT_Report (API, GMT_MSG_NORMAL, "Input data have %d column(s) but at least %d are needed\n", (int)Dd->n_columns, n_coord);
			Return (GMT_DIM_TOO_SMALL);
		}
		n_tables = Dd->n_tables;
	}
	else {	/* Free-form text record, parse coordinates when required */
		if (GMT_Init_IO (API, GMT_IS_TEXTSET, Ctrl->F.geometry, GMT_IN, GMT_ADD_DEFAULT, 0, options) != GMT_NOERROR) {	/* Establishes data input */
			Return (API->error);
		}
		if ((Dt = GMT_Read_Data (API, GMT_IS_TEXTSET, GMT_IS_FILE, 0, GMT_READ_NORMAL, NULL, NULL, NULL)) == NULL) {
			Return (API->error);
		}
		n_tables = Dt->n_tables;
	}
	if (GMT->common.R.active && first) {	/* Issue Region tag as given on command line */
		place_region_tag (API, GMT->common.R.wesn, Ctrl->Z.min, Ctrl->Z.max, N);
		first = false;
	}
	else if (Ctrl->R2.automatic) {	/* Issue Region tag */
		double wesn[4];
		if (no_text) {	/* This information is part of the dataset header and obtained during reading */
			wesn[XLO] = Dd->min[GMT_X];	wesn[XHI] = Dd->max[GMT_X];
			wesn[YLO] = Dd->min[GMT_Y];	wesn[YHI] = Dd->max[GMT_Y];
		}
		else {	/* Must slog through the textset the hard way */
			if (get_data_region (GMT, Dt, wesn)) Return (GMT_RUNTIME_ERROR);
		}
		place_region_tag (API, wesn, Ctrl->Z.min, Ctrl->Z.max, N);
	}
	set_nr = pnt_nr = 0;

	if (Ctrl->F.mode == WIGGLE) kml = kml_alloc (GMT, Dd);

	for (tbl = 0; tbl < n_tables; tbl++) {
		if (no_text) {	/* Assign dataset n_segments and pointer to file names */
			n_segments = Dd->table[tbl]->n_segments;
			file = Dd->table[tbl]->file;
		}
		else {	/* Assign textset n_segments and pointer to file names */
			n_segments = Dt->table[tbl]->n_segments;
			file = Dt->table[tbl]->file;
		}
		if (file[GMT_IN]) {	/* Place all of this file's content in its own named folder */
			kml_print (API, N++, "<Folder>\n");
			if (!strcmp (file[GMT_IN], "<stdin>"))
				kml_print (API, N, "<name>stdin</name>\n");
			else
				kml_print (API, N, "<name>%s</name>\n", basename (file[GMT_IN]));
		}
		for (seg = 0; seg < n_segments; seg++) {	/* Process each segment in this table */
			pnt_nr = 0;
			if (no_text) {	/* Assign dataset n_rows and pointers to label and header */
				n_rows = Dd->table[tbl]->segment[seg]->n_rows;
				label = Dd->table[tbl]->segment[seg]->label;
				header = Dd->table[tbl]->segment[seg]->header;
			}
			else {	/* Assign textset n_rows and pointers to label and header */
				n_rows = Dt->table[tbl]->segment[seg]->n_rows;
				label = Dt->table[tbl]->segment[seg]->label;
				header = Dt->table[tbl]->segment[seg]->header;
			}

			/* Only point sets will be organized in folders as lines/polygons are single entities */
			if (Ctrl->F.mode < LINE) {	/* Meaning point-types, not lines or polygons */
				kml_print (API, N++, "<Folder>\n");
				if (label)
					kml_print (API, N, "<name>%s</name>\n", label);
				else
					kml_print (API, N, "<name>%s Set %d</name>\n", name[Ctrl->F.mode], set_nr);
				if (gmt_M_compat_check (GMT, 4))	/* GMT4 LEVEL: Accept either -D or -T */
					act = (gmt_parse_segment_item (GMT, header, "-D", buffer) || gmt_parse_segment_item (GMT, header, "-T", buffer));
				else
					act = (gmt_parse_segment_item (GMT, header, "-T", buffer));
				if (act)
					kml_print (API, N, "<description>%s</description>\n", description);
			}
			else if (Ctrl->F.mode < WIGGLE) {	/* Line or polygon means we lay down the placemark first*/
				if (Ctrl->C.active && gmt_parse_segment_item (GMT, header, "-Z", description)) {
					double z_val = atof (description);
					index = gmt_get_index (GMT, P, z_val);
				}
				kml_print (API, N++, "<Placemark>\n");
				if (Ctrl->N.mode == NO_LABEL) { /* Nothing */ }
				else if (Ctrl->N.mode == FMT_LABEL) {
					kml_print (API, N, "<name>");
					kml_print (API, 0, Ctrl->N.fmt, (int)set_nr);
					kml_print (API, 0, "</name>\n");
				}
				else if (label)
					kml_print (API, N, "<name>%s</name>\n", label);
				else
					kml_print (API, N, "<name>%s %d</name>\n", name[Ctrl->F.mode], set_nr);
				description[0] = 0;
				do_description = false;
				if (gmt_parse_segment_item (GMT, header, "-I", buffer)) {
					do_description = true;
					strcat (description, buffer);
				}
				if (gmt_M_compat_check (GMT, 4))	/* GMT4 LEVEL: Accept either -D or -T */
					act = (gmt_parse_segment_item (GMT, header, "-D", buffer) || gmt_parse_segment_item (GMT, header, "-T", buffer));
				else
					act = (gmt_parse_segment_item (GMT, header, "-T", buffer));
				if (act) {
					if (do_description) strcat (description, " ");
					strcat (description, buffer);
					do_description = true;
				}
				if (do_description)
					kml_print (API, N, "<description>%s</description>\n", description);
				kml_print (API, N, "<styleUrl>#st-%d-%d</styleUrl>\n", process_id, index + 4); /* +4 to make style ID  >= 0 */
				kml_print (API, N++, "<%s>\n", feature[Ctrl->F.mode]);
				print_altmode (API, Ctrl->E.active, Ctrl->F.mode, Ctrl->A.mode, N);
				if (Ctrl->F.mode == POLYGON) {
					kml_print (API, N++, "<outerBoundaryIs>\n");
					kml_print (API, N++, "<LinearRing>\n");
				}
				kml_print (API, N++, "<coordinates>\n");
			}
			if (Ctrl->F.mode == WIGGLE) {
				bool positive;
				struct GMT_DATASEGMENT *S = Dd->table[tbl]->segment[seg];
				double dz, *lon, *lat, *z;
				/* Separate loop over rows since different than the other cases */
				lon = S->coord[GMT_X];
				lat = S->coord[GMT_Y];
				z = S->coord[GMT_Z];
				if (!Ctrl->G.active[F_ID]) {	/* Wants to fill positive wiggles, must get those polygons */
					kml_print (API, N++, "<Folder>\n");
					kml_print (API, N, "<name>Positive Wiggles</name>\n");
					kml->lon[0] = lon[0];
					kml->lat[0] = lat[0];
					kml->z[0]   = z[0];
					for (row = kml->n_in = 1; row < S->n_rows; row++) {	/* Convert to inches/cm and get distance increments */
						if (kml->n_in > 0 && gmt_M_is_dnan (z[row])) {	/* Data gap, plot what we have */
							positive = (z[kml->n_in-1] > 0.0);
							if (positive) kml_plot_wiggle (GMT, kml, Ctrl->Q.scale, Ctrl->Q.mode, Ctrl->Q.value, true, false, process_id, Ctrl->A.mode, N, Ctrl->A.altitude);
							kml->n_in = 0;
						}
						else if (!gmt_M_is_dnan (z[row-1]) && (z[row]*z[row-1] < 0.0 || z[row] == 0.0)) {	/* Crossed 0, add new point and plot */
							dz = z[row] - z[row-1];
							kml->lon[kml->n_in] = (dz == 0.0) ? lon[row] : lon[row-1] + fabs (z[row-1] / dz) * (lon[row] - lon[row-1]);
							kml->lat[kml->n_in] = (dz == 0.0) ? lat[row] : lat[row-1] + fabs (z[row-1] / dz) * (lat[row] - lat[row-1]);
							kml->z[kml->n_in++] = 0.0;
							positive = (kml->z[kml->n_in-2] > 0.0);
							if (positive) kml_plot_wiggle (GMT, kml, Ctrl->Q.scale, Ctrl->Q.mode, Ctrl->Q.value, true, false, process_id, Ctrl->A.mode, N, Ctrl->A.altitude);
							kml->lon[0] = kml->lon[kml->n_in-1];
							kml->lat[0] = kml->lat[kml->n_in-1];
							kml->z[0]   = 0.0;
							kml->n_in = 1;
						}
						kml->lon[kml->n_in] = lon[row];
						kml->lat[kml->n_in] = lat[row];
						kml->z[kml->n_in]   = z[row];
						if (!gmt_M_is_dnan (z[row])) kml->n_in++;
					}
					if (kml->n_in > 1) {
						positive = (kml->z[kml->n_in-1] > 0.0);
						if (positive) kml_plot_wiggle (GMT, kml, Ctrl->Q.scale, Ctrl->Q.mode, Ctrl->Q.value, true, false, process_id, Ctrl->A.mode, N, Ctrl->A.altitude);
					}
					kml_print (API, --N, "</Folder>\n");
				}
				if (Ctrl->W.active) {	/* Draw the entire wiggle */
					gmt_M_memcpy (kml->lon, lon, S->n_rows, double);
					gmt_M_memcpy (kml->lat, lat, S->n_rows, double);
					gmt_M_memcpy (kml->z, z, S->n_rows, double);
					kml->n_in = S->n_rows;
					kml_plot_wiggle (GMT, kml, Ctrl->Q.scale, Ctrl->Q.mode, Ctrl->Q.value, false, true, process_id, Ctrl->A.mode, N, Ctrl->A.altitude);
				} 
			}
			else {
				for (row = 0; row < n_rows; row++) {
					if (no_text) {	/* Data values only, just copy as is */
						unsigned int col;
						for (col = 0; col < n_coord; col++)
							out[col] = Dd->table[tbl]->segment[seg]->data[col][row];
						if (GMT->common.R.active && check_lon_lat (GMT, &out[GMT_X], &out[GMT_Y])) continue;
						pos = 0;
						if (get_z) {
							if (Ctrl->C.active) index = gmt_get_index (GMT, P, out[GMT_Z]);
							out[GMT_Z] = Ctrl->A.get_alt ? out[GMT_Z] * Ctrl->A.scale : Ctrl->A.altitude;
						}
					}
					else {	/* Parse text records separately */
						switch (n_coord) {	/* Sort out input coordinates from remaining items which may be text */
							case 2:	/* Just lon, lat, label */
								sscanf (Dt->table[tbl]->segment[seg]->data[row], "%s %s %[^\n]", C[ix], C[iy], extra);
								break;
							case 3:	/* Just lon, lat, a, extra */
								sscanf (Dt->table[tbl]->segment[seg]->data[row], "%s %s %s %[^\n]", C[ix], C[iy], C[2], extra);
								break;
							case 4:	/* Just lon, lat, a, b, extra */
								sscanf (Dt->table[tbl]->segment[seg]->data[row], "%s %s %s %s %[^\n]", C[ix], C[iy], C[2], C[3], extra);
								break;
							case 5:	/* Just lon, lat, z, t1, t2, extra */
								sscanf (Dt->table[tbl]->segment[seg]->data[row], "%s %s %s %s %s %[^\n]",
								        C[ix], C[iy], C[2], C[3], C[4], extra);
								break;
						}
						if (gmt_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_X],
						                             gmt_scanf_arg (GMT, C[GMT_X], GMT->current.io.col_type[GMT_IN][GMT_X],
						                             &out[GMT_X]), C[GMT_X])) {
							GMT_Message (API, GMT_TIME_NONE, "Error: Could not decode longitude from %s\n", C[GMT_X]);
							Return (GMT_RUNTIME_ERROR);
						}
						if (gmt_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Y],
						                             gmt_scanf_arg (GMT, C[GMT_Y], GMT->current.io.col_type[GMT_IN][GMT_Y],
						                             &out[GMT_Y]), C[GMT_Y])) {
							GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not decode latitude from %s\n", C[GMT_Y]);
							Return (GMT_RUNTIME_ERROR);
						}
						if (GMT->common.R.active && check_lon_lat (GMT, &out[GMT_X], &out[GMT_Y])) continue;
						if (get_z) {
							if (gmt_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][GMT_Z],
							                             gmt_scanf_arg (GMT, C[GMT_Z], GMT->current.io.col_type[GMT_IN][GMT_Z],
							                             &out[GMT_Z]), C[GMT_Z])) {
								GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not decode altitude from %s\n", C[GMT_Z]);
								Return (GMT_RUNTIME_ERROR);
							}
							if (Ctrl->C.active) index = gmt_get_index (GMT, P, out[GMT_Z]);
							out[GMT_Z] = Ctrl->A.get_alt ? out[GMT_Z] * Ctrl->A.scale : Ctrl->A.altitude;
						}
						if (Ctrl->F.mode == EVENT) {
							if (gmt_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][t1_col],
							                                  gmt_scanf_arg (GMT, C[t1_col], GMT->current.io.col_type[GMT_IN][t1_col],
							                                  &out[t1_col]), C[t1_col])) {
								GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not decode time event from %s\n", C[t1_col]);
								Return (GMT_RUNTIME_ERROR);
							}
						}
						else if (Ctrl->F.mode == SPAN) {
							if (!(strcmp (C[t1_col], "NaN")))
								out[t1_col] = GMT->session.d_NaN;
							else if (gmt_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][t1_col],
							                                  gmt_scanf_arg (GMT, C[t1_col], GMT->current.io.col_type[GMT_IN][t1_col],
							                                  &out[t1_col]), C[t1_col])) {
								GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not decode time span beginning from %s\n", C[t1_col]);
								Return (GMT_RUNTIME_ERROR);
							}
							if (!(strcmp (C[t2_col], "NaN")))
								out[t2_col] = GMT->session.d_NaN;
							else if (gmt_verify_expectations (GMT, GMT->current.io.col_type[GMT_IN][t2_col],
							                                  gmt_scanf_arg (GMT, C[t2_col], GMT->current.io.col_type[GMT_IN][t2_col],
							                                  &out[t2_col]), C[t2_col])) {
								GMT_Report (API, GMT_MSG_NORMAL, "Error: Could not decode time span end from %s\n", C[t2_col]);
								Return (GMT_RUNTIME_ERROR);
							}
						}
					}
					if (Ctrl->F.mode < LINE && gmt_M_is_dnan (out[GMT_Z])) continue;	/* Symbols with NaN height are not plotted anyhow */

					pos = 0;
					if (Ctrl->F.mode < LINE) {	/* Print the information for this point */
						kml_print (API, N++, "<Placemark>\n");
						if (Ctrl->N.mode == NO_LABEL) { /* Nothing */ }
						else if (Ctrl->N.mode == GET_COL_LABEL) {
							gmt_strtok (extra, GMT_TOKEN_SEPARATORS, &pos, item);
							kml_print (API, N, "<name>%s</name>\n", item);
						}
						else if (Ctrl->N.mode == GET_LABEL)
							kml_print (API, N, "<name>%s</name>\n", extra);
						else if (Ctrl->N.mode == FMT_LABEL) {
							kml_print (API, N, "<name>");
							kml_print (API, 0, Ctrl->N.fmt, pnt_nr);
							kml_print (API, 0, "</name>\n");
						}
						else if (label && n_rows > 1)
							kml_print (API, N, "<name>%s %" PRIu64 "</name>\n", label, row);
						else if (label)
							kml_print (API, N, "<name>%s</name>\n", label);
						else
							kml_print (API, N, "<name>%s %d</name>\n", name[Ctrl->F.mode], pnt_nr);
						if (Ctrl->L.n_cols) {
							kml_print (API, N++, "<ExtendedData>\n");
							for (col = 0; col < Ctrl->L.n_cols; col++) {
								kml_print (API, N, "<Data name = \"%s\">\n", Ctrl->L.name[col]);
								kml_print (API, N++, "<value>");
								gmt_strtok (extra, GMT_TOKEN_SEPARATORS, &pos, item);
								L = strlen (item);
								if (L && item[0] == '\"' && item[L-1] == '\"') {	/* Quoted string on input, remove quotes on output */
									item[L-1] = '\0';
									kml_print (API, N, "%s", &item[1]);
								}
								else
									kml_print (API, N, "%s", item);
								kml_print (API, --N, "</value>\n");
								kml_print (API, N, "</Data>\n");
							}
							kml_print (API, --N, "</ExtendedData>\n");
						}
						if (Ctrl->F.mode == SPAN) {
							kml_print (API, N++, "<TimeSpan>\n");
							if (!gmt_M_is_dnan (out[t1_col])) {
								gmt_ascii_format_col (GMT, text, out[t1_col], GMT_OUT, t1_col);
								kml_print (API, N, "<begin>%s</begin>\n", text);
							}
							if (!gmt_M_is_dnan (out[t2_col])) {
								gmt_ascii_format_col (GMT, text, out[t2_col], GMT_OUT, t2_col);
								kml_print (API, N, "<end>%s</end>\n", text);
							}
							kml_print (API, --N, "</TimeSpan>\n");
						}
						else if (Ctrl->F.mode == EVENT) {
							kml_print (API, N++, "<TimeStamp>\n");
							gmt_ascii_format_col (GMT, text, out[t1_col], GMT_OUT, t1_col);
							kml_print (API, N, "<when>%s</when>\n", text);
							kml_print (API, --N, "</TimeStamp>\n");
						}
						kml_print (API, N, "<styleUrl>#st-%d-%d</styleUrl>\n", process_id, index + 4); /* +4 to make index a positive integer */
						kml_print (API, N++, "<%s>\n", feature[Ctrl->F.mode]);
						print_altmode (API, Ctrl->E.active, false, Ctrl->A.mode, N);
						kml_print (API, N, "<coordinates>");
						ascii_output_three (API, out, N);
						kml_print (API, N, "</coordinates>\n");
						kml_print (API, --N, "</%s>\n", feature[Ctrl->F.mode]);
						kml_print (API, --N, "</Placemark>\n");
					}
					else {	/* For lines and polygons we just output the coordinates */
						if (gmt_M_is_dnan (out[GMT_Z])) out[GMT_Z] = 0.0;	/* Google Earth can not handle lines at NaN altitude */
						ascii_output_three (API, out, N);
						if (row > 0 && no_dateline && crossed_dateline (out[GMT_X], last_x)) {
							/* GE cannot handle polygons crossing the dateline; warn for now */
							GMT_Report (API, GMT_MSG_NORMAL,
							            "Warning: At least on polygon is straddling the Dateline. Google Earth will wrap these the wrong way\n");
							GMT_Report (API, GMT_MSG_NORMAL,
							            "Split such polygons into East and West parts and plot them as separate polygons.\n");
							GMT_Report (API, GMT_MSG_NORMAL, "Use gmtconvert to help in this conversion.\n");
							no_dateline = true;
						}
						last_x = out[GMT_X];
					}
					pnt_nr++;
				}
			}
			/* End of segment */
			if (pnt_nr == 0)
				set_nr--;
			else if (Ctrl->F.mode < LINE)
				kml_print (API, --N, "</Folder>\n");
			else {
				kml_print (API, --N, "</coordinates>\n");
				if (Ctrl->F.mode == POLYGON) {
					kml_print (API, --N, "</LinearRing>\n");
					kml_print (API, --N, "</outerBoundaryIs>\n");
				}
				kml_print (API, --N, "</%s>\n", feature[Ctrl->F.mode]);
				kml_print (API, --N, "</Placemark>\n");
			}
			set_nr++;
		}
		if (file[GMT_IN]) kml_print (API, --N, "</Folder>\n");
	}
	if (use_folder) kml_print (API, --N, "</%s>\n", Document[KML_FOLDER]);
	if (!GMT->common.K.active) {
		kml_print (API, --N, "</%s>\n", Document[KML_DOCUMENT]);
		kml_print (API, 0, "</kml>\n");
	}

	if (Ctrl->F.mode == WIGGLE) kml_free (GMT, &kml);

	if (GMT_End_IO (API, GMT_OUT, 0) != GMT_NOERROR) {	/* Disables further data output */
		Return (API->error);
	}

	Return (GMT_NOERROR);	/* Garbage collection will free Dd or Dt */
}
